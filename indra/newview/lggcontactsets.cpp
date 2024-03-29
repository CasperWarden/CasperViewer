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
#include "lggcontactsets.h"
#include "llsdserialize.h"
#include "llboost.h"
#include "llcontrol.h"
#include "llviewercontrol.h"
#include "llnotifications.h"
#include "lldir.h"
#include "llcallingcard.h"
#include "llavatarnamecache.h"

LGGContactSets* LGGContactSets::sInstance;

LGGContactSets::LGGContactSets()
{
	sInstance = this;
	sInstance->loadFromDisk();
}

LGGContactSets::~LGGContactSets()
{
	sInstance = NULL;
}

LGGContactSets* LGGContactSets::getInstance()
{
	if(sInstance)return sInstance;
	else
	{
		sInstance = new LGGContactSets();
		return sInstance;
	}
}
LLColor4 LGGContactSets::toneDownColor(LLColor4 inColor, float strength)
{
	if(strength<.4)strength=.4;
	return LLColor4(LLColor3(inColor),strength);
	return inColor;
}
bool LGGContactSets::callbackAliasReset(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	if (option == 0)
	{
		LGGContactSets::getInstance()->clearPseudonym(notification["payload"]["agent_id"].asUUID());
	}
	else
	{

	}
	return false;
}
void LGGContactSets::save()
{
	saveToDisk(mContactSets);
}
std::string LGGContactSets::getFileName()
{
	std::string path=gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "");

	if (!path.empty())
	{
		path = gDirUtilp->getExpandedFilename(LL_PATH_PER_SL_ACCOUNT, "settings_friends_groups.xml");
	}
	return path;  
}
std::string LGGContactSets::getDefaultFileName()
{
	std::string path=gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "");

	if (!path.empty())
	{
		path = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "settings_friends_groups.xml");
	}
	return path;  
}
LLSD LGGContactSets::exportGroup(std::string groupName)
{
	LLSD toReturn;
	if(mContactSets.has(groupName))
	{
		toReturn["groupname"]=groupName;
		toReturn["color"]=mContactSets[groupName]["color"];
		toReturn["notices"]=mContactSets[groupName]["notices"];
		toReturn["friends"]=mContactSets[groupName]["friends"];
	}
	return toReturn;
}
LLSD LGGContactSets::getContactSets()
{
	//loadFromDisk();
	return mContactSets;
}
void LGGContactSets::loadFromDisk()
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
		saveToDisk(mContactSets);
	}
	else
	{
		llifstream file;
		file.open(filename.c_str());
		if (file.is_open())
		{
			LLSDSerialize::fromXML(mContactSets, file);
		}
		file.close();
	}	
}
void LGGContactSets::saveToDisk(LLSD newSettings)
{
	mContactSets=newSettings;
	std::string filename=getFileName();
	llofstream file;
	file.open(filename.c_str());
	LLSDSerialize::toPrettyXML(mContactSets, file);
	file.close();
}
void LGGContactSets::runTest()
{
	
mContactSets.clear();
}
BOOL LGGContactSets::saveGroupToDisk(std::string groupName, std::string fileName)
{
	if(mContactSets.has(groupName))
	{
		llofstream file;
		file.open(fileName.c_str());
		LLSDSerialize::toPrettyXML(exportGroup(groupName), file);
		file.close();
		return TRUE;
	}
	return FALSE;
}
LLSD LGGContactSets::getExampleLLSD()
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

LLColor4 LGGContactSets::getGroupColor(std::string groupName)
{
	if(groupName!="" && groupName!="All Groups" && groupName !="globalSettings" && groupName!="No Groups" && groupName!="ReNamed" && groupName!="Non Friends")
		if(mContactSets[groupName].has("color"))
			return LLColor4(mContactSets[groupName]["color"]);
	return getDefaultColor();
};
LLColor4 LGGContactSets::getFriendColor(
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
				if(mContactSets[groups[i]].has("color"))
				{
					toReturn= LLColor4(mContactSets[groups[i]]["color"]);
					if(isNonFriend(friend_id))toReturn=toneDownColor(toReturn,.8);
				}
			}
		}
	}
	if(lowest==9999)
	if(isFriendInGroup(friend_id,ignoredGroupName)  && ignoredGroupName!="globalSettings" && ignoredGroupName!="Non Friends" &&ignoredGroupName!="All Groups" && ignoredGroupName!="No Groups" &&ignoredGroupName!="ReNamed" &&ignoredGroupName!="")
		if(mContactSets[ignoredGroupName].has("color"))
			return LLColor4(mContactSets[ignoredGroupName]["color"]);
	return toReturn;
}
BOOL LGGContactSets::hasFriendColorThatShouldShow(LLUUID friend_id)
{
	static BOOL* sPhoenixColorContactSetsChat = rebind_llcontrol<BOOL>("PhoenixContactSetsColorizeChat", &gSavedSettings, true);
	if(!(*sPhoenixColorContactSetsChat))return FALSE;
	if(getFriendColor(friend_id)==getDefaultColor())return FALSE;
	return TRUE;
}
LLColor4 LGGContactSets::getDefaultColor()
{
	LLColor4 toReturn= LLColor4::grey;
	if(mContactSets.has("globalSettings"))
		if(mContactSets["globalSettings"].has("defaultColor"))
			toReturn = LLColor4(mContactSets["globalSettings"]["defaultColor"]);
	return toReturn;
}
void LGGContactSets::setDefaultColor(LLColor4 dColor)
{
	mContactSets["globalSettings"]["defaultColor"]=dColor.getValue();
}
std::vector<std::string> LGGContactSets::getInnerGroups(std::string groupName)
{
	std::vector<std::string> toReturn;
	toReturn.clear();
	static BOOL *useFolders = rebind_llcontrol<BOOL>("PhoenixContactSetsShowFolders",&gSavedSettings,true);
	static BOOL *showOnline = rebind_llcontrol<BOOL>("PhoenixContactSetsShowOnline", &gSavedSettings, true);
	static BOOL *showOffline = rebind_llcontrol<BOOL>("PhoenixContactSetsShowOffline", &gSavedSettings, true);

	if(!(*useFolders))return toReturn;

	std::set<std::string> newGroups;
	newGroups.clear();
	if(groupName!="All Groups")newGroups.insert("All Groups");
	std::vector<LLUUID> freindsInGroup = getFriendsInGroup(groupName);
	for(int fn = 0; fn<freindsInGroup.size();fn++)
	{
		LLUUID friend_id = freindsInGroup[fn];
		BOOL online = LLAvatarTracker::instance().isBuddyOnline(friend_id);
		if(online && !(*showOnline))continue;
		if(!online && !(*showOffline))continue;
		
		std::vector<std::string> innerGroups = getFriendGroups(friend_id);
		for(int inIter=0;inIter<innerGroups.size();inIter++)
		{
			std::string innerGroupName = innerGroups[inIter];
			if(groupName!=innerGroupName)
				newGroups.insert(innerGroupName);
		}
	}

	std::copy(newGroups.begin(), newGroups.end(), std::back_inserter(toReturn));
	return toReturn;
}
std::vector<std::string> LGGContactSets::getFriendGroups(LLUUID friend_id)
{
	std::vector<std::string> toReturn;
	toReturn.clear();

	LLSD::map_const_iterator loc_it = mContactSets.beginMap();
	LLSD::map_const_iterator loc_end = mContactSets.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const std::string& groupName = (*loc_it).first;
		if(groupName!="" && groupName !="globalSettings" && groupName!="All Groups" && groupName!="No Groups" && groupName!="ReNamed" && groupName!="Non Friends" && groupName!="extraAvs" && groupName!="pseudonym")
			if(mContactSets[groupName]["friends"].has(friend_id.asString()))
				toReturn.push_back(groupName);
	}
	return toReturn;
}
std::vector<LLUUID> LGGContactSets::getFriendsInGroup(std::string groupName)
{
	std::vector<LLUUID> toReturn;
	toReturn.clear();
	if(groupName=="All Groups")return getFriendsInAnyGroup();
	if(groupName=="No Groups")return toReturn;
	if(groupName=="pseudonym"||groupName=="ReNamed")return getListOfPseudonymAvs();
	if(groupName=="Non Friends")return getListOfNonFriends();

	LLSD friends = mContactSets[groupName]["friends"];	
	LLSD::map_const_iterator loc_it = friends.beginMap();
	LLSD::map_const_iterator loc_end = friends.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const LLSD& friendID = (*loc_it).first;

		toReturn.push_back(friendID.asUUID());
	}	

	return toReturn;
}
std::vector<std::string> LGGContactSets::getAllGroups(BOOL extraGroups)
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

	LLSD::map_const_iterator loc_it = mContactSets.beginMap();
	LLSD::map_const_iterator loc_end = mContactSets.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const std::string& groupName = (*loc_it).first;
		if((groupName!="globalSettings")&&(groupName!="ReNamed")&&(groupName!="Non Friends")&&(groupName!="")&&(groupName!="extraAvs")&&(groupName!="pseudonym")&&(groupName!="All Groups")&&groupName!="No Groups")
			toReturn.push_back(groupName);
	}


	return toReturn;
}
std::vector<LLUUID> LGGContactSets::getFriendsInAnyGroup()
{
	std::set<LLUUID> friendsInAnyGroup;
	std::vector<std::string> groups = getAllGroups(FALSE);
	for(int g=0;g<groups.size();g++)
	{
		LLSD friends = mContactSets[groups[g]]["friends"];	
		LLSD::map_const_iterator loc_it = friends.beginMap();
		LLSD::map_const_iterator loc_end = friends.endMap();
		for ( ; loc_it != loc_end; ++loc_it)
		{
			const LLSD& friendID = (*loc_it).first;
			friendsInAnyGroup.insert(friendID);
		}
	}
	return std::vector<LLUUID>(friendsInAnyGroup.begin(),friendsInAnyGroup.end());
}
BOOL LGGContactSets::isFriendInAnyGroup(LLUUID friend_id)
{
	std::vector<std::string> groups = getAllGroups(FALSE);
	for(int g=0;g<groups.size();g++)
	{
		if(mContactSets[groups[g]]["friends"].has(friend_id.asString()))
			return TRUE;
	}
	return FALSE;
}
BOOL LGGContactSets::isFriendInGroup(LLUUID friend_id, std::string groupName)
{	
	if(groupName=="All Groups") return isFriendInAnyGroup(friend_id);
	if(groupName=="No Groups") return !isFriendInAnyGroup(friend_id);
	if(groupName=="ReNamed") return hasPseudonym(friend_id);
	if(groupName=="Non Friends") return isNonFriend(friend_id);
	return mContactSets[groupName]["friends"].has(friend_id.asString());
}
BOOL LGGContactSets::notifyForFriend(LLUUID friend_id)
{
	BOOL notify = FALSE;
	std::vector<std::string> groups = getFriendGroups(friend_id);
	for(int i =0;i<groups.size();i++)
	{
		if(mContactSets[groups[i]]["notify"].asBoolean())return TRUE;
	}
	return notify;
}
void LGGContactSets::addFriendToGroup(LLUUID friend_id, std::string groupName)
{
	if(friend_id.notNull() && groupName!="" && groupName !="globalSettings" && groupName!="No Groups" && groupName!="All Groups" && groupName!="ReNamed" && groupName!="Non Friends")
	{
		mContactSets[groupName]["friends"][friend_id.asString()]="";
		save();
	}
}
void LGGContactSets::addNonFriendToList(LLUUID non_friend_id)
{
	mContactSets["extraAvs"][non_friend_id.asString()]="";
	save();
}
void LGGContactSets::removeNonFriendFromList(LLUUID non_friend_id)
{
	if(mContactSets["extraAvs"].has(non_friend_id.asString()))
	{
		mContactSets["extraAvs"].erase(non_friend_id.asString());
		if(!LLAvatarTracker::instance().isBuddy(non_friend_id))
		{
			clearPseudonym(non_friend_id);
			removeFriendFromAllGroups(non_friend_id);
		}
		save();
	}
}
void LGGContactSets::removeFriendFromAllGroups(LLUUID friend_id)
{
	std::vector<std::string> groups = getFriendGroups(friend_id);
	for(int i=0;i<groups.size();i++)
	{
		removeFriendFromGroup(friend_id,groups[i]);
	}
}
BOOL LGGContactSets::isNonFriend(LLUUID non_friend_id)
{
	if(LLAvatarTracker::instance().isBuddy(non_friend_id))return FALSE;
	if(mContactSets["extraAvs"].has(non_friend_id.asString()))return TRUE;
	return FALSE;
}
std::vector<LLUUID> LGGContactSets::getListOfNonFriends()
{
	std::vector<LLUUID> toReturn;
	toReturn.clear();

	LLSD friends = mContactSets["extraAvs"];	
	LLSD::map_const_iterator loc_it = friends.beginMap();
	LLSD::map_const_iterator loc_end = friends.endMap();
	for ( ; loc_it != loc_end; ++loc_it)
	{
		const LLSD& friendID = (*loc_it).first;
		if(friendID.asUUID().notNull())
			if(!LLAvatarTracker::instance().isBuddy(friendID))
				toReturn.push_back(friendID.asUUID());
	}	

	return toReturn;
}
std::vector<LLUUID> LGGContactSets::getListOfPseudonymAvs()
{
	std::vector<LLUUID> toReturn;
	toReturn.clear();

	LLSD friends = mContactSets["pseudonym"];	
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
void LGGContactSets::setPseudonym(LLUUID friend_id, std::string pseudonym)
{
	mContactSets["pseudonym"][friend_id.asString()]=pseudonym;
	save();
}
std::string LGGContactSets::getPseudonym(LLUUID friend_id)
{
	if(mContactSets["pseudonym"].has(friend_id.asString()))
	{
		return mContactSets["pseudonym"][friend_id.asString()];
	}
	return "";
}
void LGGContactSets::clearPseudonym(LLUUID friend_id)
{
	if(mContactSets["pseudonym"].has(friend_id.asString()))
	{
		mContactSets["pseudonym"].erase(friend_id.asString());
		LLAvatarNameCache::fetch(friend_id);//update
		save();
	}
}
BOOL LGGContactSets::hasPseudonym(LLUUID friend_id)
{
	if(getPseudonym(friend_id)!="")return TRUE;
	return FALSE;
}
BOOL LGGContactSets::hasDisplayNameRemoved(LLUUID friend_id)
{
	return (getPseudonym(friend_id)=="--- ---");
}
BOOL LGGContactSets::hasVisuallyDiferentPseudonym(LLUUID friend_id)
{
	return (hasPseudonym(friend_id) && (!hasDisplayNameRemoved(friend_id)));
}
void LGGContactSets::removeDisplayName(LLUUID friend_id)
{
	setPseudonym(friend_id,"--- ---");
}
void LGGContactSets::removeFriendFromGroup(LLUUID friend_id, std::string groupName)
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
		if(mContactSets[groupName]["friends"].has(friend_id.asString()))
		{
			mContactSets[groupName]["friends"].erase(friend_id.asString());
			save();
		}
	}
}
void LGGContactSets::addGroup(std::string groupName)
{

	if(groupName!="")
	{
		mContactSets[groupName]["color"] = LLColor4::red.getValue();
		save();
	}
}
void LGGContactSets::deleteGroup(std::string groupName)
{
	if(mContactSets.has(groupName))
	{
		mContactSets.erase(groupName);
		save();
	}
}
void LGGContactSets::setNotifyForGroup(std::string groupName, BOOL notify)
{
	if(groupName=="All Groups" || groupName =="globalSettings" || groupName == "" || groupName =="No Groups"||groupName=="ReNamed"||groupName=="Non Friends")return;

	if(mContactSets.has(groupName))
	{
		mContactSets[groupName]["notify"]=notify;
		save();
	}
}
BOOL LGGContactSets::getNotifyForGroup(std::string groupName)
{
	if(mContactSets.has(groupName))
	{
		if(mContactSets[groupName].has("notify"))
		{
			return mContactSets[groupName]["notify"].asBoolean();
		}
	}
	return FALSE;
}
void LGGContactSets::setGroupColor(std::string groupName, LLColor4 color)
{
	if(groupName=="All Groups" ||   groupName =="globalSettings" || groupName == "" || groupName =="No Groups"||groupName=="ReNamed"||groupName=="Non Friends")return;

	if(mContactSets.has(groupName))
	{
		mContactSets[groupName]["color"]=color.getValue();
		save();
	}
}
