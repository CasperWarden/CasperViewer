/** 
 * @file kcwlinterface.h
 * @brief Windlight Interface
 *
 * Copyright (C) 2010, Kadah Coba
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
 * The Phoenix Viewer Project, http://www.phoenixviewer.com/
 */

#include "llviewerprecompiledheaders.h"
#include "llmemory.h"	// LLSingleton<>
#include "lltimer.h"	// LLEventTimer
#include "llnotifications.h"
#include "lluuid.h"
#include <set>
#include <string>

class LLParcel;

class KCWindlightInterface : public LLSingleton<KCWindlightInterface>,LLEventTimer
{
public:
	KCWindlightInterface();
	void ParcelChange();
	virtual BOOL tick();
	void ApplySettings(const LLSD& settings);
	void ResetToRegion();
	//bool ChatCommand(std::string message, std::string from_name, LLUUID source_id, LLUUID owner_id);
	void LoadFromPacel(LLParcel *parcel);
	void onClickWLStatusButton();
	bool WLset;
	
private:
	bool callbackParcelWL(const LLSD& notification, const LLSD& response);
	bool callbackParcelWLClear(const LLSD& notification, const LLSD& response);
	bool AllowedLandOwners(const LLUUID& agent_id);
	LLUUID getOwnerID(LLParcel *parcel);
	std::string getOwnerName(LLParcel *parcel);

	std::set<LLUUID> mAllowedLand;
	LLNotificationPtr mSetWLNotification;
	LLNotificationPtr mClearWLNotification;
	S32 mLastParcelID;
	std::string mLastParcelDesc; //used to check if its changed
	LLSD mCurrentSettings;
};
