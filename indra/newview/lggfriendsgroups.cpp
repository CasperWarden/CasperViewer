/* Copyright (C) 2011 Greg Hendrickson (LordGregGreg Back)
   
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
#include "lldir.h"

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
	std::string path=gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "");

	if (!path.empty())
	{
		path = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "settings_friends_groups.xml");
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
	if(groupName!="" && groupName!="All Groups" && groupName!="No Groups" && groupName!="ReNamed" && groupName!="Non Friends")
		if(mFriendsGroups[groupName].has("color"))
			return LLColor4(mFriendsGroups[groupName]["color"]);
	return getDefaultColor();
};
LLColor4 LGGFriendsGroups::getFriendColor(
	LLUUID friend_id, std::string ignoredGroupName)
{
	LLColor4 toReturn = getDefaultColor();
	if(ignoredGroupName=="No Groups") return toReturn;
	int lowest = 9999;
	std::vector<std::string> groups = getFriendGroups(friend_id);
	for(int i =0;i<groups.size();i++)
	{
		if(groups[i]!=ignoredGroupName)
		{
			int membersNum = getFriendsInGroup(groups[i]).size();
			if(membersNum==0)continue;
			if(membersNum<lowest)
			{
				lowest=membersNum;
				toReturn= LLColor4(mFriendsGroups[groups[i]]["color"]);
				if(isNonFriend(friend_id))toReturn=toneDownColor(toReturn,.8);
			}
		}
	}
	if(lowest==9999)
	if(isFriendInGroup(friend_id,ignoredGroupName) &&ignoredGroupName!="Non Friends" &&ignoredGroupName!="All Groups" && ignoredGroupName!="No Groups" &&ignoredGroupName!="ReNamed" &&ignoredGroupName!="")
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
		if(groupName!="" && groupName!="All Groups" && groupName!="No Groups" && groupName!="ReNamed" && groupName!="Non Friends")
			if(mFriendsGroups[groupName]["friends"].has(friend_id.asString()))
				toReturn.push_back(groupName);
	}
	return toReturn;
}
std::vector<std::string> LGGFriendsGroups::getAllGroups(BOOL extraGroups)
{
	std::vector<std::string> toReturn;
	toReturn.clear();
	if(extraGroups)
	{
		if(getAllGroups(FALSE).size()>0)
		{
			toReturn.push_back("All Groups");
			toReturn.push_back("No Groups");
		}
		if(getListOfPseudonymAvs().size()>0)
			toReturn.push_back("ReNamed");
		if(getListOfNonFriends().size()>0)
			toReturn.push_back("Non Friends");
	}

	LLSD::map_const_iterator loc_it = mFriendsGroups.beginMap();
	LLSD::map_const_iterator loc_end = mFriendsGroups.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const std::string& groupName = (*loc_it).first;
		if((groupName!="globalSettings")&&(groupName!="ReNamed")&&(groupName!="Non Friends")&&(groupName!="")&&(groupName!="extraAvs")&&(groupName!="pseudonym")&&(groupName!="All Groups")&&groupName!="No Groups")
			toReturn.push_back(groupName);
	}


	return toReturn;
}
std::vector<LLUUID> LGGFriendsGroups::getFriendsInAnyGroup()
{
	std::vector<LLUUID> toReturn;
	toReturn.clear();
	std::vector<std::string> groups = getAllGroups(FALSE);
	for(int g=0;g<groups.size();g++)
	{
		LLSD friends = mFriendsGroups[groups[g]]["friends"];	
		LLSD::map_const_iterator loc_it = friends.beginMap();
		LLSD::map_const_iterator loc_end = friends.endMap();
		for ( ; loc_it != loc_end; ++loc_it)
		{
			const LLSD& friendID = (*loc_it).first;
			if(std::find(toReturn.begin(), toReturn.end(), friendID)!=toReturn.end())
			{
				toReturn.push_back(friendID.asUUID());
			}
		}
	}
	return toReturn;
}
std::vector<LLUUID> LGGFriendsGroups::getFriendsInGroup(std::string groupName)
{
	std::vector<LLUUID> toReturn;
	toReturn.clear();
	if(groupName=="All Groups")return getFriendsInAnyGroup();
	if(groupName=="No Groups")return toReturn;
	if(groupName=="Pseudonym")return getListOfPseudonymAvs();
	if(groupName=="Non Friends")return getListOfNonFriends();

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
BOOL LGGFriendsGroups::isFriendInAnyGroup(LLUUID friend_id)
{
	std::vector<std::string> groups = getAllGroups(FALSE);
	for(int g=0;g<groups.size();g++)
	{
		if(mFriendsGroups[groups[g]]["friends"].has(friend_id.asString()))
			return TRUE;
	}
	return FALSE;
}
BOOL LGGFriendsGroups::isFriendInGroup(LLUUID friend_id, std::string groupName)
{	
	if(groupName=="All Groups") return isFriendInAnyGroup(friend_id);
	if(groupName=="No Groups") return !isFriendInAnyGroup(friend_id);
	if(groupName=="ReNamed") return hasPseudonym(friend_id);
	if(groupName=="Non Friends") return isNonFriend(friend_id);
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
	if(friend_id.notNull() && groupName!="" && groupName!="No Groups" && groupName!="All Groups" && groupName!="ReNamed" && groupName!="Non Friends")
	{
		mFriendsGroups[groupName]["friends"][friend_id.asString()]="";
		save();
	}
}
void LGGFriendsGroups::addNonFriendToList(LLUUID non_friend_id)
{
	mFriendsGroups["extraAvs"][non_friend_id.asString()]="";
	save();
}
void LGGFriendsGroups::removeNonFriendFromList(LLUUID non_friend_id)
{
	if(mFriendsGroups["extraAvs"].has(non_friend_id.asString()))
	{
		mFriendsGroups["extraAvs"].erase(non_friend_id.asString());
		clearPseudonym(non_friend_id);
		removeFriendFromAllGroups(non_friend_id);
		save();
	}
}
void LGGFriendsGroups::removeFriendFromAllGroups(LLUUID friend_id)
{
	std::vector<std::string> groups = getFriendGroups(friend_id);
	for(int i=0;i<groups.size();i++)
	{
		removeFriendFromGroup(friend_id,groups[i]);
	}
}
BOOL LGGFriendsGroups::isNonFriend(LLUUID non_friend_id)
{
	if(mFriendsGroups["extraAvs"].has(non_friend_id.asString()))return TRUE;
	return FALSE;
}
std::vector<LLUUID> LGGFriendsGroups::getListOfNonFriends()
{
	std::vector<LLUUID> toReturn;
	toReturn.clear();

	LLSD friends = mFriendsGroups["extraAvs"];	
	LLSD::map_const_iterator loc_it = friends.beginMap();
	LLSD::map_const_iterator loc_end = friends.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const LLSD& friendID = (*loc_it).first;
		if(friendID.asUUID().notNull())
			toReturn.push_back(friendID.asUUID());
	}	

	return toReturn;
}
std::vector<LLUUID> LGGFriendsGroups::getListOfPseudonymAvs()
{
	std::vector<LLUUID> toReturn;
	toReturn.clear();

	LLSD friends = mFriendsGroups["pseudonym"];	
	LLSD::map_const_iterator loc_it = friends.beginMap();
	LLSD::map_const_iterator loc_end = friends.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const LLSD& friendID = (*loc_it).first;
		if(friendID.asUUID().notNull())
			toReturn.push_back(friendID.asUUID());
	}	

	return toReturn;
}
void LGGFriendsGroups::setPseudonym(LLUUID friend_id, std::string pseudonym)
{
	mFriendsGroups["pseudonym"][friend_id.asString()]=pseudonym;
	save();
}
std::string LGGFriendsGroups::getPseudonym(LLUUID friend_id)
{
	if(mFriendsGroups["pseudonym"].has(friend_id.asString()))
	{
		return mFriendsGroups["pseudonym"][friend_id.asString()];
	}
	return "";
}
void LGGFriendsGroups::clearPseudonym(LLUUID friend_id)
{
	if(mFriendsGroups["pseudonym"].has(friend_id.asString()))
	{
		mFriendsGroups["pseudonym"].erase(friend_id.asString());
		save();
	}
}
BOOL LGGFriendsGroups::hasPseudonym(LLUUID friend_id)
{
	if(getPseudonym(friend_id)!="")return TRUE;
	return FALSE;
}
void LGGFriendsGroups::removeFriendFromGroup(LLUUID friend_id, std::string groupName)
{
	if(groupName=="extraAvs"||groupName=="Non Friends")
	{
		return removeNonFriendFromList(friend_id);
	}
	if(groupName=="ReNamed")
	{
		return clearPseudonym(friend_id);
	}
	if(friend_id.notNull() && groupName!="")
	{	
		if(mFriendsGroups[groupName]["friends"].has(friend_id.asString()))
		{
			mFriendsGroups[groupName]["friends"].erase(friend_id.asString());
			save();
		}
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
	if(groupName=="All Groups" || groupName == "" || groupName =="No Groups"||groupName=="ReNamed"||groupName=="Non Friends")return;

	if(mFriendsGroups.has(groupName))
	{
		mFriendsGroups[groupName]["notify"]=notify;
		save();
	}
}
BOOL LGGFriendsGroups::getNotifyForGroup(std::string groupName)
{
	if(mFriendsGroups.has(groupName))
	{
		if(mFriendsGroups[groupName].has("notify"))
		{
			return mFriendsGroups[groupName]["notify"].asBoolean();
		}
	}
	return FALSE;
}
void LGGFriendsGroups::setGroupColor(std::string groupName, LLColor4 color)
{
	if(groupName=="All Groups" || groupName == "" || groupName =="No Groups"||groupName=="ReNamed"||groupName=="Non Friends")return;

	if(mFriendsGroups.has(groupName))
	{
		mFriendsGroups[groupName]["color"]=color.getValue();
		save();
	}
}
