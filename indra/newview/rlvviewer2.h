/** 
 * $LicenseInfo:firstyear=2004&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef RLV_VIEWER2_H
#define RLV_VIEWER2_H

#include "llcallbacklist.h"

#include "boost/function.hpp"

// ============================================================================
// From llappearancemgr.h

typedef boost::function<void ()> nullary_func_t;
typedef boost::function<bool ()> bool_func_t;

// Call a given callable once in idle loop.
void doOnIdleOneTime(nullary_func_t callable);

// Repeatedly call a callable in idle loop until it returns true.
void doOnIdleRepeating(bool_func_t callable);

// ============================================================================

#endif // RLV_VIEWER2_H