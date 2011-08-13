/* Copyright (C) 2011 LordGregGreg Back
   
   This is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
 
   This is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
 
   You should have received a copy of the GNU Lesser General Public
   License along with the viewer; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include "llviewerprecompiledheaders.h"
#include "lggfriendsgroups.h"
#include "llsdserialize.h"
#include "llboost.h"
#include "llcontrol.h"
#include "llviewercontrol.h"
#include "llnotifications.h"

LGGFriendsGroups* LGGFriendsGroups::sInstance;

LGGFriendsGroups::LGGFriendsGroups()
{
	sInstance = this;
	sInstance->loadFromDisk();
}

LGGFriendsGroups::~LGGFriendsGroups()
{
	sInstance = NULL;
}

LGGFriendsGroups* LGGFriendsGroups::getInstance()
{
	if(sInstance)return sInstance;
	else
	{
		sInstance = new LGGFriendsGroups();
		return sInstance;
	}
}
LLColor4 LGGFriendsGroups::toneDownColor(LLColor4 inColor, float strength)
{
	if(strength<.4)strength=.4;
	return LLColor4(LLColor3(inColor),strength);
	return inColor;
}

void LGGFriendsGroups::save()
{
	saveToDisk(mFriendsGroups);
}
std::string LGGFriendsGroups::getFileName()
{
	std::string path=gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "");

	if (!path.empty())
	{
		path = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "settings_friends_groups.xml");
	}
	return path;  
}
std::string LGGFriendsGroups::getDefaultFileName()
{
	std::string path=gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "");

	if (!path.empty())
	{
		path = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "settings_friends_groups.xml");
	}
	return path;  
}
LLSD LGGFriendsGroups::exportGroup(std::string groupName)
{
	LLSD toReturn;
	if(mFriendsGroups.has(groupName))
	{
		toReturn["groupname"]=groupName;
		toReturn["color"]=mFriendsGroups[groupName]["color"];
		toReturn["notices"]=mFriendsGroups[groupName]["notices"];
		toReturn["friends"]=mFriendsGroups[groupName]["friends"];
	}
	return toReturn;
}
LLSD LGGFriendsGroups::getFriendsGroups()
{
	//loadFromDisk();
	return mFriendsGroups;
}
void LGGFriendsGroups::loadFromDisk()
{
	std::string filename=getFileName();
	if (filename.empty())
	{
		llinfos << "no valid user directory." << llendl; 
	}
	if(!gDirUtilp->fileExists(filename))
	{
		std::string defaultName = getDefaultFileName();
		llinfos << " user settings file doesnt exist, going to try and read default one from "<<defaultName.c_str()<< llendl;

		if(gDirUtilp->fileExists(defaultName))
		{
			LLSD blankllsd;
			llifstream file;
			file.open(defaultName.c_str());
			if (file.is_open())
			{
				LLSDSerialize::fromXMLDocument(blankllsd, file);
			}
			file.close();
			saveToDisk(blankllsd);
		}else
		saveToDisk(mFriendsGroups);
	}
	else
	{
		llifstream file;
		file.open(filename.c_str());
		if (file.is_open())
		{
			LLSDSerialize::fromXML(mFriendsGroups, file);
		}
		file.close();
	}	
}
void LGGFriendsGroups::saveToDisk(LLSD newSettings)
{
	mFriendsGroups=newSettings;
	std::string filename=getFileName();
	llofstream file;
	file.open(filename.c_str());
	LLSDSerialize::toPrettyXML(mFriendsGroups, file);
	file.close();
}
void LGGFriendsGroups::runTest()
{
	
mFriendsGroups.clear();
}
BOOL LGGFriendsGroups::saveGroupToDisk(std::string groupName, std::string fileName)
{
	if(mFriendsGroups.has(groupName))
	{
		llofstream file;
		file.open(fileName.c_str());
		LLSDSerialize::toPrettyXML(exportGroup(groupName), file);
		file.close();
		return TRUE;
	}
	return FALSE;
}
LLSD LGGFriendsGroups::getExampleLLSD()
{
	LLSD toReturn;

	LLSD listone;
	LLSD listtwo;

	LLSD itemOne;
	itemOne["wrong"]="wrong1";
	itemOne["right"]="right1";
	listone[0]=itemOne;

	LLSD itemTwo;
	itemTwo["wrong"]="wrong2";
	itemTwo["right"]="right2";
	listone[1]=itemTwo;

	toReturn["listOne"]=listone;


	itemOne["wrong"]="secondwrong1";
	itemOne["right"]="secondright1";
	listone[0]=itemOne;

	itemTwo["wrong"]="secondwrong2";
	itemTwo["right"]="secondright2";
	listone[1]=itemTwo;

	toReturn["listTwo"]=listone;	

	return toReturn;
}

LLColor4 LGGFriendsGroups::getGroupColor(std::string groupName)
{
	if(groupName!="")
		if(mFriendsGroups[groupName].has("color"))
			return LLColor4(mFriendsGroups[groupName]["color"]);
	return getDefaultColor();
};
LLColor4 LGGFriendsGroups::getFriendColor(
	LLUUID friend_id, std::string ignoredGroupName)
{
	LLColor4 toReturn = getDefaultColor();
	int lowest = 9999;
	std::vector<std::string> groups = getFriendGroups(friend_id);
	for(int i =0;i<groups.size();i++)
	{
		if(groups[i]!=ignoredGroupName)
		{
			int membersNum = getFriendsInGroup(groups[i]).size();
			if(membersNum<lowest)
			{
				lowest=membersNum;
				toReturn= LLColor4(mFriendsGroups[groups[i]]["color"]);
			}
		}
	}
	if(lowest==9999)
	if(isFriendInGroup(friend_id,ignoredGroupName))
		return LLColor4(mFriendsGroups[ignoredGroupName]["color"]);
	return toReturn;
}
LLColor4 LGGFriendsGroups::getDefaultColor()
{
	LLColor4 toReturn= LLColor4::grey;
	if(mFriendsGroups.has("globalSettings"))
		if(mFriendsGroups["globalSettings"].has("defaultColor"))
			toReturn = mFriendsGroups["globalSettings"]["defaultColor"];
	return toReturn;
}
void LGGFriendsGroups::setDefaultColor(LLColor4 dColor)
{
	mFriendsGroups["globalSettings"]["defaultColor"]=dColor.getValue();
}
std::vector<std::string> LGGFriendsGroups::getFriendGroups(LLUUID friend_id)
{
	std::vector<std::string> toReturn;
	toReturn.clear();

	LLSD::map_const_iterator loc_it = mFriendsGroups.beginMap();
	LLSD::map_const_iterator loc_end = mFriendsGroups.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const std::string& groupName = (*loc_it).first;
		if(mFriendsGroups[groupName]["friends"].has(friend_id.asString()))
			toReturn.push_back(groupName);
	}
	return toReturn;
}
std::vector<std::string> LGGFriendsGroups::getAllGroups()
{
	std::vector<std::string> toReturn;
	toReturn.clear();

	LLSD::map_const_iterator loc_it = mFriendsGroups.beginMap();
	LLSD::map_const_iterator loc_end = mFriendsGroups.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const std::string& groupName = (*loc_it).first;
		if((groupName!="globalSettings")&&(groupName!=""))
			toReturn.push_back(groupName);
	}


	return toReturn;
}
std::vector<LLUUID> LGGFriendsGroups::getFriendsInGroup(std::string groupName)
{
	std::vector<LLUUID> toReturn;
	toReturn.clear();

	LLSD friends = mFriendsGroups[groupName]["friends"];	
	LLSD::map_const_iterator loc_it = friends.beginMap();
	LLSD::map_const_iterator loc_end = friends.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const LLSD& friendID = (*loc_it).first;

		toReturn.push_back(friendID.asUUID());
	}	

	return toReturn;
}
BOOL LGGFriendsGroups::isFriendInGroup(LLUUID friend_id, std::string groupName)
{	
	return mFriendsGroups[groupName]["friends"].has(friend_id.asString());
}
BOOL LGGFriendsGroups::notifyForFriend(LLUUID friend_id)
{
	BOOL notify = FALSE;
	std::vector<std::string> groups = getFriendGroups(friend_id);
	for(int i =0;i<groups.size();i++)
	{
		if(mFriendsGroups[groups[i]]["notify"].asBoolean())return TRUE;
	}
	return notify;
}
void LGGFriendsGroups::addFriendToGroup(LLUUID friend_id, std::string groupName)
{
	if(friend_id.notNull() && groupName!="")
	{
		mFriendsGroups[groupName]["friends"][friend_id.asString()]="";
		save();
	}
}
void LGGFriendsGroups::removeFriendFromGroup(LLUUID friend_id, std::string groupName)
{
	if(friend_id.notNull() && groupName!="")
	{	
		mFriendsGroups[groupName]["friends"].erase(friend_id.asString());
		save();
	}
}
void LGGFriendsGroups::addGroup(std::string groupName)
{

	if(groupName!="")
	{
		mFriendsGroups[groupName]["color"] = LLColor4::red.getValue();
		save();
	}
}
void LGGFriendsGroups::deleteGroup(std::string groupName)
{
	if(mFriendsGroups.has(groupName))
	{
		mFriendsGroups.erase(groupName);
		save();
	}
}
void LGGFriendsGroups::setNotifyForGroup(std::string groupName, BOOL notify)
{
	if(mFriendsGroups.has(groupName))
	{
		mFriendsGroups[groupName]["notify"]=notify;
		save();
	}

}
void LGGFriendsGroups::setGroupColor(std::string groupName, LLColor4 color)
{
	if(mFriendsGroups.has(groupName))
	{
		mFriendsGroups[groupName]["color"]=color.getValue();
		save();
	}
}
