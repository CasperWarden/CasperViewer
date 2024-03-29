/** 
 * @file llfasttimer.cpp
 * @brief Implementation of the fast timer.
 *
 * $LicenseInfo:firstyear=2004&license=viewergpl$
 * 
 * Copyright (c) 2004-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */
#include "linden_common.h"

#include "llfasttimer.h"

#include "llprocessor.h"


#if LL_WINDOWS
#elif LL_LINUX || LL_SOLARIS
#include <sys/time.h>
#include <sched.h>
#elif LL_DARWIN
#include <sys/time.h>
#include "lltimer.h"	// get_clock_count()
#else 
#error "architecture not supported"
#endif

//////////////////////////////////////////////////////////////////////////////
// statics


LLFastTimer::EFastTimerType LLFastTimer::sCurType = LLFastTimer::FTM_OTHER;
int LLFastTimer::sCurDepth = 0;
U64 LLFastTimer::sStart[LLFastTimer::FTM_MAX_DEPTH];
U64 LLFastTimer::sCounter[LLFastTimer::FTM_NUM_TYPES];
U64 LLFastTimer::sCountHistory[LLFastTimer::FTM_HISTORY_NUM][LLFastTimer::FTM_NUM_TYPES];
U64 LLFastTimer::sCountAverage[LLFastTimer::FTM_NUM_TYPES];
U64 LLFastTimer::sCalls[LLFastTimer::FTM_NUM_TYPES];
U64 LLFastTimer::sCallHistory[LLFastTimer::FTM_HISTORY_NUM][LLFastTimer::FTM_NUM_TYPES];
U64 LLFastTimer::sCallAverage[LLFastTimer::FTM_NUM_TYPES];
S32 LLFastTimer::sCurFrameIndex = -1;
S32 LLFastTimer::sLastFrameIndex = -1;
int LLFastTimer::sPauseHistory = 0;
int LLFastTimer::sResetHistory = 0;

F64 LLFastTimer::sCPUClockFrequency = 0.0;

#if LL_LINUX || LL_SOLARIS
U64 LLFastTimer::sClockResolution = 1000000000; // 1e9, Nanosecond resolution
#else 
U64 LLFastTimer::sClockResolution = 1000000; // 1e6, Microsecond resolution
#endif

//////////////////////////////////////////////////////////////////////////////

//
// CPU clock/other clock frequency and count functions
//

#if LL_WINDOWS

U64 get_cpu_clock_count()
{   U32  hi,lo;

    __asm   
    {
        _emit   0x0f
        _emit   0x31
        mov     lo,eax
        mov     hi,edx
    }

	U64 ret = hi;
	ret *= 4294967296L;
	ret |= lo;
    return ret;
};

#endif // LL_WINDOWS

#if LL_LINUX || LL_SOLARIS
// Try to use the MONOTONIC clock if available, this is a constant time counter
// with nanosecond resolution (but not necessarily accuracy) and attempts are made
// to synchronize this value between cores at kernel start. It should not be affected
// by CPU frequency. If not available use the REALTIME clock, but this may be affected by
// NTP adjustments or other user activity affecting the system time.
U64 get_cpu_clock_count()
{
    struct timespec tp;

#ifdef CLOCK_MONOTONIC
    clock_gettime(CLOCK_MONOTONIC,&tp);
#else
    clock_gettime(CLOCK_REALTIME,&tp);
#endif
    return (tp.tv_sec*LLFastTimer::sClockResolution)+tp.tv_nsec;        
}
#endif // (LL_LINUX || LL_SOLARIS))

#if LL_DARWIN
//
// Mac implementation of CPU clock
//
// Just use gettimeofday implementation for now

U64 get_cpu_clock_count()
{
	return get_clock_count();
}
#endif

//////////////////////////////////////////////////////////////////////////////

//static
#if (LL_DARWIN || LL_LINUX || LL_SOLARIS) && !(defined(__i386__) || defined(__amd64__))
U64 LLFastTimer::countsPerSecond() // counts per second for the *32-bit* timer
{
	return sClockResolution >> 8;
}
#else // windows or x86-mac or x86-linux or x86-solaris
U64 LLFastTimer::countsPerSecond() // counts per second for the *32-bit* timer
{
	//getCPUFrequency returns MHz and sCPUClockFrequency wants to be in Hz
	static U64 sCPUClockFrequency = U64(LLProcessorInfo().getCPUFrequency()*1000000.0);

	// we drop the low-order byte in our timers, so report a lower frequency
	return sCPUClockFrequency >> 8;
}
#endif

void LLFastTimer::reset()
{
	countsPerSecond(); // good place to calculate clock frequency

	if (sCurDepth != 0)
	{
		//llerrs << "LLFastTimer::Reset() when sCurDepth != 0" << llendl;
		static bool warned = false;	// Since LL_WARNS_ONCE is broken...
		if (warned)
		{
			LL_DEBUGS("FastTimer") << "LLFastTimer::Reset() when sCurDepth != 0 ("
								   << sCurDepth << ")" << LL_ENDL;
		}
		else
		{
			LL_WARNS("FastTimer") << "LLFastTimer::Reset() when sCurDepth != 0 ("
								  << sCurDepth << ")" << LL_ENDL;
			warned = true;
		}
	}
	if (sPauseHistory)
	{
		sResetHistory = 1;
	}
	else if (sResetHistory)
	{
		sCurFrameIndex = -1;
		sResetHistory = 0;
	}
	else if (sCurFrameIndex >= 0)
	{
		int hidx = sCurFrameIndex % FTM_HISTORY_NUM;
		for (S32 i=0; i<FTM_NUM_TYPES; i++)
		{
			sCountHistory[hidx][i] = sCounter[i];
			sCountAverage[i] = (sCountAverage[i]*sCurFrameIndex + sCounter[i]) / (sCurFrameIndex+1);
			sCallHistory[hidx][i] = sCalls[i];
			sCallAverage[i] = (sCallAverage[i]*sCurFrameIndex + sCalls[i]) / (sCurFrameIndex+1);
		}
		sLastFrameIndex = sCurFrameIndex;
	}
	else
	{
		for (S32 i=0; i<FTM_NUM_TYPES; i++)
		{
			sCountAverage[i] = 0;
			sCallAverage[i] = 0;
		}
	}
	
	sCurFrameIndex++;
	
	for (S32 i=0; i<FTM_NUM_TYPES; i++)
	{
		sCounter[i] = 0;
		sCalls[i] = 0;
	}
	sCurDepth = 0;
}

//////////////////////////////////////////////////////////////////////////////
