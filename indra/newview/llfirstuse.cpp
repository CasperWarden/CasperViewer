/** 
 * @file llfirstuse.cpp
 * @brief Methods that spawn "first-use" dialogs
 *
 * $LicenseInfo:firstyear=2003&license=viewergpl$
 * 
 * Copyright (c) 2003-2009, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llfirstuse.h"

// library includes
#include "indra_constants.h"

// viewer includes
#include "llagent.h"	// for gAgent.inPrelude()
#include "llnotify.h"
#include "llviewercontrol.h"
#include "llui.h"
#include "llappviewer.h"
#include "lltracker.h"

// static
std::set<std::string> LLFirstUse::sConfigVariables;

// static
void LLFirstUse::addConfigVariable(const std::string& var)
{
	//Don't add the warning, now that we're storing the default in the settings_default.xml file
	//gSavedSettings.addWarning(var);
	sConfigVariables.insert(var);
}

// static
void LLFirstUse::disableFirstUse()
{
	// Set all first-use warnings to disabled
	for (std::set<std::string>::iterator iter = sConfigVariables.begin();
		 iter != sConfigVariables.end(); ++iter)
	{
		gSavedSettings.setWarning(*iter, FALSE);
	}
}

// static
void LLFirstUse::resetFirstUse()
{
	// Set all first-use warnings to disabled
	for (std::set<std::string>::iterator iter = sConfigVariables.begin();
		 iter != sConfigVariables.end(); ++iter)
	{
		gSavedSettings.setWarning(*iter, TRUE);
	}
}
// static
void LLFirstUse::usePhoenixContactSet()
{
	if (gSavedSettings.getWarning("FirstPhoenixContactSetOpen"))
	{
		gSavedSettings.setWarning("FirstPhoenixContactSetOpen", FALSE);
		LLNotifications::instance().add("FirstPhoenixContactSetOpen");
	}
}

//static
void LLFirstUse::usePhoenixFriendsNonFriend()
{
	if (gSavedSettings.getWarning("FirstPhoenixContactSetNonFriend"))
	{
		gSavedSettings.setWarning("FirstPhoenixContactSetNonFriend", FALSE);
		LLNotifications::instance().add("FirstPhoenixContactSetNonFriend");
	}
}

// static 
void LLFirstUse::usePhoenixContactSetRename()
{
	if (gSavedSettings.getWarning("FirstPhoenixContactSetRename"))
	{
		gSavedSettings.setWarning("FirstPhoenixContactSetRename", FALSE);
		LLNotifications::instance().add("FirstPhoenixContactSetRename");
	}
}

// Called whenever the viewer detects that your balance went up
void LLFirstUse::useBalanceIncrease(S32 delta)
{
	if (gSavedSettings.getWarning("FirstBalanceIncrease"))
	{
		gSavedSettings.setWarning("FirstBalanceIncrease", FALSE);

		LLSD args;
		args["AMOUNT"] = llformat("%d",delta);
		LLNotifications::instance().add("FirstBalanceIncrease", args);
	}
}


// Called whenever the viewer detects your balance went down
void LLFirstUse::useBalanceDecrease(S32 delta)
{
	if (gSavedSettings.getWarning("FirstBalanceDecrease"))
	{
		gSavedSettings.setWarning("FirstBalanceDecrease", FALSE);

		LLSD args;
		args["AMOUNT"] = llformat("%d",-delta);
		LLNotifications::instance().add("FirstBalanceDecrease", args);
	}
}


// static
void LLFirstUse::useSit()
{
	// Our orientation island uses sitting to teach vehicle driving
	// so just never show this message. JC
	//if (gSavedSettings.getWarning("FirstSit"))
	//{
	//	gSavedSettings.setWarning("FirstSit", FALSE);
        //
	//	LLNotifications::instance().add("FirstSit");
	//}
}

// static
void LLFirstUse::useMap()
{
	if (gSavedSettings.getWarning("FirstMap"))
	{
		gSavedSettings.setWarning("FirstMap", FALSE);

		LLNotifications::instance().add("FirstMap");
	}
}

// static
void LLFirstUse::useGoTo()
{
	// nothing for now JC
}

// static
void LLFirstUse::useBuild()
{
	if (gSavedSettings.getWarning("FirstBuild"))
	{
		gSavedSettings.setWarning("FirstBuild", FALSE);

		LLNotifications::instance().add("FirstBuild");
	}
}

// static
void LLFirstUse::useLeftClickNoHit()
{ 
	if (gSavedSettings.getWarning("FirstLeftClickNoHit"))
	{
		gSavedSettings.setWarning("FirstLeftClickNoHit", FALSE);

		LLNotifications::instance().add("FirstLeftClickNoHit");
	}
}

// static
void LLFirstUse::useTeleport()
{
	if (gSavedSettings.getWarning("FirstTeleport"))
	{
		LLVector3d teleportDestination = LLTracker::getTrackedPositionGlobal();
		if(teleportDestination != LLVector3d::zero)
		{
			gSavedSettings.setWarning("FirstTeleport", FALSE);

		        LLNotifications::instance().add("FirstTeleport");
		}
	}
}

// static
void LLFirstUse::useOverrideKeys()
{
	// Our orientation island uses key overrides to teach vehicle driving
	// so don't show this message until you get off OI. JC
	if (!gAgent.inPrelude())
	{
		if (gSavedSettings.getWarning("FirstOverrideKeys"))
		{
			gSavedSettings.setWarning("FirstOverrideKeys", FALSE);

			LLNotifications::instance().add("FirstOverrideKeys");
		}
	}
}

// static
void LLFirstUse::useAttach()
{
	// nothing for now
}

// static
void LLFirstUse::useAppearance()
{
	if (gSavedSettings.getWarning("FirstAppearance"))
	{
		gSavedSettings.setWarning("FirstAppearance", FALSE);

		LLNotifications::instance().add("FirstAppearance");
	}
}

// static
void LLFirstUse::useInventory()
{
	if (gSavedSettings.getWarning("FirstInventory"))
	{
		gSavedSettings.setWarning("FirstInventory", FALSE);

		LLNotifications::instance().add("FirstInventory");
	}
}


// static
void LLFirstUse::useSandbox()
{
	if (gSavedSettings.getWarning("FirstSandbox"))
	{
		gSavedSettings.setWarning("FirstSandbox", FALSE);

		LLSD args;
		args["HOURS"] = llformat("%d",SANDBOX_CLEAN_FREQ);
		args["TIME"] = llformat("%d",SANDBOX_FIRST_CLEAN_HOUR);
		LLNotifications::instance().add("FirstSandbox", args);
	}
}

// static
void LLFirstUse::useFlexible()
{
	if (gSavedSettings.getWarning("FirstFlexible"))
	{
		gSavedSettings.setWarning("FirstFlexible", FALSE);

		LLNotifications::instance().add("FirstFlexible");
	}
}

// static
void LLFirstUse::useDebugMenus()
{
	if (gSavedSettings.getWarning("FirstDebugMenus"))
	{
		gSavedSettings.setWarning("FirstDebugMenus", FALSE);

		LLNotifications::instance().add("FirstDebugMenus");
	}
}

// static
void LLFirstUse::useSculptedPrim()
{
	if (gSavedSettings.getWarning("FirstSculptedPrim"))
	{
		gSavedSettings.setWarning("FirstSculptedPrim", FALSE);

		LLNotifications::instance().add("FirstSculptedPrim");
		
	}
}

// static 
void LLFirstUse::useMedia()
{
	if (gSavedSettings.getWarning("FirstMedia"))
	{
		gSavedSettings.setWarning("FirstMedia", FALSE);

		LLNotifications::instance().add("FirstMedia");
	}
}
