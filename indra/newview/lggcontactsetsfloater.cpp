/* Copyright (C) 2011 LordGregGreg Back (Greg Hendrickson)

   This is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; version 2.1 of
   the License.
 
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
#include "lggcontactsetsfloater.h"

#include "llagentdata.h"
#include "llcommandhandler.h"
#include "llfloater.h"
#include "lluictrlfactory.h"
#include "llagent.h"
#include "llpanel.h"
#include "llbutton.h"
#include "llcolorswatch.h"
#include "llcombobox.h"
#include "llview.h"
#include "llpanelphoenix.h"
#include "llhttpclient.h"
#include "llbufferstream.h"
#include "llcheckboxctrl.h"
#include "llviewercontrol.h"

#include "llui.h"
#include "llcontrol.h"
#include "llscrolllistctrl.h"
#include "llscrollingpanellist.h"
#include "lggautocorrect.h"
#include "llfilepicker.h"
#include "llfile.h"
#include "llsdserialize.h"
#include "llfloaterchat.h"
#include "llchat.h"
#include "llviewerinventory.h"
#include "llinventorymodel.h"
#include "llhost.h"
#include "llassetstorage.h"
#include "roles_constants.h"
#include "llviewertexteditor.h"

#include "llappviewer.h"
#include "llavatarnamecache.h"
#include "lluuid.h"
#include "llavatarname.h"
#include "llcallingcard.h"
#include "lluserrelations.h"
#include "llfloateravatarinfo.h"
#include "llimview.h"
#include "llviewermessage.h"
#include "llfloaterworldmap.h"
#include "llstring.h"
#include "llclipboard.h"
#include "llfloateravatarpicker.h"
#include "llfirstuse.h"
#include "llvoavatar.h"

class lggContactSetsFloater;
class lggContactSetsFloater : public LLFloater, public LLFloaterSingleton<lggContactSetsFloater>, public LLFriendObserver
{
public:
	lggContactSetsFloater(const LLSD& seed);
	virtual ~lggContactSetsFloater();

	virtual void changed(U32 mask);
	BOOL postBuild(void);
	void setData(void * data);
	BOOL handleMouseDown(S32 x,S32 y,MASK mask);
	void update();
	BOOL handleRightMouseDown(S32 x,S32 y,MASK mask);
	BOOL handleKeyHere( KEY key, MASK mask );
	BOOL handleDoubleClick(S32 x, S32 y, MASK mask);
	BOOL handleHover(S32 x, S32 y, MASK mask);
	BOOL handleUnicodeCharHere(llwchar uni_char);
	BOOL handleScrollWheel(S32 x, S32 y, S32 clicks);
	
	BOOL generateCurrentList();
	void draw();
	void drawScrollBars();
	void drawRightClick();
	void drawFilter();
	BOOL toggleSelect(LLUUID whoToToggle);
	static BOOL compareAv(LLUUID av1, LLUUID av2);

	static void onClickSettings(void* data);
	static void onClickNew(void* data);
	static void onClickDelete(void* data);
	static void onPickAvatar(const std::vector<std::string>& names, const std::vector<LLUUID>& ids, void* user_data);


	static void onBackgroundChange(LLUICtrl* ctrl, void* userdata);

	static void onNoticesChange(LLUICtrl* ctrl, void* userdata);
	static void onCheckBoxChange(LLUICtrl* ctrl, void* userdata);
	static void hitSpaceBar(LLUICtrl* ctrl, void* userdata);

	void updateGroupsList();
	void updateGroupGUIs();

	static void onSelectGroup(LLUICtrl* ctrl, void* userdata);

	LLComboBox* groupsList;
	LLColorSwatchCtrl * groupColorBox;
	LLCheckBoxCtrl* noticeBox;
	LLRect contextRect;

	LLPanelPhoenix * phpanel;
	static lggContactSetsFloater* sInstance;
private:
	
	S32 mouse_x;
	S32 mouse_y;
	F32 hovered;
	F32 scrollStarted;
	S32 maxSize;
	std::string currentFilter;
	std::string currentRightClickText;
	std::vector<LLUUID> selected;
	std::vector<LLUUID> currentList;
	std::vector<std::string> allFolders;
	std::vector<std::string> openedFolders;
	S32 scrollLoc;
	BOOL showRightClick;
	BOOL justClicked;
};

lggContactSetsFloater* lggContactSetsFloater::sInstance;

lggContactSetsFloater::~lggContactSetsFloater()
{
	LLAvatarTracker::instance().removeObserver(sInstance);
	sInstance = NULL;
}
lggContactSetsFloater::lggContactSetsFloater(const LLSD& seed)
:mouse_x(0),mouse_y(900),hovered(0.f),justClicked(FALSE),scrollLoc(0),
showRightClick(FALSE),maxSize(0),scrollStarted(0),currentFilter(""),
currentRightClickText("")
{
	if(sInstance)delete sInstance;
	sInstance = this;
	selected.clear();
	currentList.clear();
	allFolders.clear();
	openedFolders.clear();
	LLAvatarTracker::instance().addObserver(this);
	
	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_contactsets.xml");
}
void lggContactSetsFloater::changed(U32 mask)
{
	if(mask & (LLFriendObserver::ADD | LLFriendObserver::REMOVE ))
	{
		sInstance->generateCurrentList();
	}
	if(mask & (LLFriendObserver::ONLINE))
	{
		static BOOL *showOnline = rebind_llcontrol<BOOL>("PhoenixContactSetsShowOnline", &gSavedSettings, true);
		static BOOL *showOffline = rebind_llcontrol<BOOL>("PhoenixContactSetsShowOffline", &gSavedSettings, true);
		if(!(*showOffline&&*showOnline))
		{
			sInstance->generateCurrentList();
		}
	}
}
void lggContactSetsFloater::onBackgroundChange(LLUICtrl* ctrl, void* userdata)
{
	LLColorSwatchCtrl* cctrl = (LLColorSwatchCtrl*)ctrl;

	if(cctrl)
	{
		std::string *currentGroup = rebind_llcontrol<std::string>("PhoenixContactSetsSelectedGroup", &gSavedSettings, true);

		LGGContactSets::getInstance()->setGroupColor(*currentGroup,cctrl->get());

	}

}
void lggContactSetsFloater::onNoticesChange(LLUICtrl* ctrl, void* userdata)
{
	LLCheckBoxCtrl* cctrl = (LLCheckBoxCtrl*)ctrl;

	if(cctrl)
	{
		std::string *currentGroup = rebind_llcontrol<std::string>("PhoenixContactSetsSelectedGroup", &gSavedSettings, true);

		LGGContactSets::getInstance()->setNotifyForGroup(*currentGroup, ctrl->getValue().asBoolean());
		
	}

}
void lggContactSetsFloater::onCheckBoxChange(LLUICtrl* ctrl, void* userdata)
{
	LLCheckBoxCtrl* cctrl = (LLCheckBoxCtrl*)ctrl;

	if(cctrl)
	{
		sInstance->generateCurrentList();		
	}
}
void lggContactSetsFloater::onPickAvatar(const std::vector<std::string>& names,
								  const std::vector<LLUUID>& ids,
								  void* )
{
	if (names.empty()) return;
	if (ids.empty()) return;
	LGGContactSets::getInstance()->addNonFriendToList(ids[0]);
	sInstance->updateGroupsList();
	LLFirstUse::usePhoenixFriendsNonFriend();
}
void lggContactSetsFloater::updateGroupsList()
{
	std::string currentGroup = gSavedSettings.getString("PhoenixContactSetsSelectedGroup");
	LLComboBox * cb = groupsList;
	//if(	sInstance->groupsList != NULL) cb = sInstance->groupsList;

	cb->clear();
	cb->removeall();
	std::vector<std::string> groups = LGGContactSets::getInstance()->getAllGroups();
	for(int i =0;i<groups.size();i++)
	{
		cb->add(groups[i],groups[i],ADD_BOTTOM,TRUE);
	}
	if((currentGroup=="")&&(groups.size()>0))
	{
		gSavedSettings.setString("PhoenixContactSetsSelectedGroup",groups[0]);
		updateGroupGUIs();
		generateCurrentList();
	}else
	cb->setSimple(currentGroup);
}
void lggContactSetsFloater::hitSpaceBar(LLUICtrl* ctrl, void* userdata)
{
	LLCheckBoxCtrl* cctrl = (LLCheckBoxCtrl*)ctrl;

	if(cctrl)
	{
		if((sInstance->currentFilter=="" && !sInstance->showRightClick)||
			(sInstance->currentRightClickText=="" && sInstance->showRightClick))
			sInstance->justClicked=TRUE;
		else
		{
			if(!sInstance->showRightClick)
			{
				sInstance->currentFilter+=' ';
				sInstance->generateCurrentList();
			}else
			{
				sInstance->currentRightClickText+=' ';
			}
		}
	}
}
void lggContactSetsFloater::updateGroupGUIs()
{
	std::string *currentGroup = rebind_llcontrol<std::string>("PhoenixContactSetsSelectedGroup", &gSavedSettings, true);
	
	groupColorBox->set(LGGContactSets::getInstance()->getGroupColor(*currentGroup),TRUE);	
	
	groupsList->setSimple(*currentGroup);
	
	noticeBox->set(LGGContactSets::getInstance()->getNotifyForGroup(*currentGroup));
}
void lggContactSetsFloater::onSelectGroup(LLUICtrl* ctrl, void* userdata)
{
	LLComboBox* cctrl = (LLComboBox*)ctrl;

	if(cctrl)
	{
		gSavedSettings.setString("PhoenixContactSetsSelectedGroup",cctrl->getSimple());
		sInstance->updateGroupGUIs();
		sInstance->selected.clear();

	}
	sInstance->generateCurrentList();
}
BOOL lggContactSetsFloater::postBuild(void)
{
	groupsList= getChild<LLComboBox>("lgg_fg_groupCombo");
	groupsList->setCommitCallback(onSelectGroup);

	groupColorBox = getChild<LLColorSwatchCtrl>("colorswatch");
	
	childSetAction("lgg_fg_groupCreate",onClickNew,this);
	childSetAction("lgg_fg_groupDelete",onClickDelete,this);
	childSetAction("lgg_fg_openSettings",onClickSettings,this);

	groupColorBox->setCommitCallback(onBackgroundChange);

	noticeBox = getChild<LLCheckBoxCtrl>("lgg_fg_showNotices");
	noticeBox->setCommitCallback(onNoticesChange);

	getChild<LLCheckBoxCtrl>("lgg_fg_showOnline")->setCommitCallback(onCheckBoxChange);
	getChild<LLCheckBoxCtrl>("lgg_fg_showOffline")->setCommitCallback(onCheckBoxChange);
	getChild<LLCheckBoxCtrl>("lgg_fg_showOtherGroups")->setCommitCallback(onCheckBoxChange);
	getChild<LLCheckBoxCtrl>("lgg_fg_showAllFriends")->setCommitCallback(onCheckBoxChange);

	getChild<LLCheckBoxCtrl>("haxCheckbox")->setCommitCallback(hitSpaceBar);

	updateGroupsList();
	generateCurrentList();
	updateGroupGUIs();

	LLFirstUse::usePhoenixContactSet();

	return true;
}



void lggContactSetsFloater::setData(void * data)
{
	phpanel = (LLPanelPhoenix*)data;
}


void lggContactSetsFloater::drawScrollBars()
{

}
void lggContactSetsFloater::drawRightClick()
{
	std::string *currentGroup = rebind_llcontrol<std::string>("PhoenixContactSetsSelectedGroup", &gSavedSettings, true);

	if(!sInstance->hasFocus())
	{
		showRightClick=FALSE;
		return;
	}
	int heightPer = 17;
	int width = 208;
	BOOL drawRemove=FALSE;

	int extras = 5;//make sure we have room for the extra options
	BOOL canMap = FALSE;
	int isNonFriend=0;
	if(selected.size()==1)
	{
		extras+=6;//space,name,tp, profile, im, rename 
		//map
		if(LGGContactSets::getInstance()->isNonFriend(selected[0]))
		{
			isNonFriend=1;
			extras+=1;//add remove option

		}else
		if((LLAvatarTracker::instance().getBuddyInfo(selected[0])->getRightsGrantedFrom())& LLRelationship::GRANT_MAP_LOCATION)
		{
			if(LLAvatarTracker::instance().getBuddyInfo(selected[0])->isOnline())
			{
				extras+=1;
				canMap=TRUE;
			}
		}
		if(LGGContactSets::getInstance()->hasPseudonym(selected[0]))
		{
			extras+=1;//for clearing it
		}
	}
	if(selected.size()>1)
	{
		extras+=4;//name, conference, mass tp, space
		for(int i=0;i<selected.size();i++)
		{
			if(LGGContactSets::getInstance()->isNonFriend(selected[i]))
				isNonFriend++;
		}
	}
	if(*currentGroup!="All Groups" && *currentGroup != "No Groups" && *currentGroup != "")
	{
		drawRemove=TRUE;
		extras+=2;
	}
	

	std::vector<std::string> groups = LGGContactSets::getInstance()->getAllGroups(FALSE);
	if(selected.size()==0)
	{
		groups.clear();
		extras+=4;
	}
	int height = heightPer*(extras+groups.size());

	LLRect rec = sInstance->getChild<LLPanel>("draw_region")->getRect();
	gGL.color4fv(LLColor4(0,0,0,.5).mV);
	gl_rect_2d(rec);
	
	if((contextRect.mLeft+width)>rec.mRight)
	{
		contextRect.mLeft=rec.mRight-width;
	}
	if((contextRect.mTop- (height))<rec.mBottom)
	{
		contextRect.mTop= rec.mBottom+height;
	}	
	contextRect.setLeftTopAndSize(contextRect.mLeft,contextRect.mTop,width,height);
	LLRect bgRect;
	bgRect.setLeftTopAndSize(contextRect.mLeft-2,contextRect.mTop+2,width+4,contextRect.getHeight()+4);
	gGL.color4fv(LLColor4::black.mV);
	gl_rect_2d(bgRect);
	
	int top = contextRect.mTop;
	for(int i = 0;i<groups.size();i++)
	{
		LLRect addBackGround;
		addBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);		
		
		gGL.color4fv(LGGContactSets::getInstance()->getGroupColor(groups[i]).mV);
		gl_rect_2d(addBackGround);
		if(addBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(addBackGround,FALSE);
			if(justClicked)
			{
				for(int v=0;v<selected.size();v++)
				{
					LLUUID afriend = selected[v];
					LGGContactSets::getInstance()->addFriendToGroup(
						afriend,groups[i]);
				}
				selected.clear();
			}
		}

		LLFontGL::getFontSansSerif()->renderUTF8(
			std::string("Add to: "+groups[i])
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);

		top-=heightPer;
	}

	LLRect remBackGround;
	if(drawRemove)
	{
		//draw remove button
		top-=heightPer;
		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);

		gGL.color4fv(LGGContactSets::getInstance()->getGroupColor(*currentGroup).mV);
		gl_rect_2d(remBackGround);
		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				for(int v=0;v<selected.size();v++)
				{
					LLUUID afriend = selected[v];
					LGGContactSets::getInstance()->removeFriendFromGroup(
						afriend,*currentGroup);

					sInstance->generateCurrentList();
				}

				selected.clear();
			}
		}

		LLFontGL::getFontSansSerif()->renderUTF8(
			std::string("Remove From: "+*currentGroup)
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);

		top-=heightPer;
	}
	top-=heightPer;
	//specials
	if(selected.size()==1)
	{

		std::string avName("");
		LLAvatarName avatar_name;
		if(LLAvatarNameCache::get(selected[0], &avatar_name))avName=avatar_name.getLegacyName();

		LLColor4 friendColor = LGGContactSets::getInstance()->getFriendColor(selected[0],"");
		
		
		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			//gGL.color4fv(LLColor4::yellow.mV);
			//gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				//no
			}
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
			avName
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
		top-=heightPer;
		
		//rename start

		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
		//draw text block background after the :rename text
		int remWidth = LLFontGL::getFontSansSerif()->getWidth("Set Alias:");
		LLRect inTextBox;
		inTextBox.setLeftTopAndSize(remBackGround.mLeft+2+remWidth,remBackGround.mTop,
			remBackGround.getWidth()-2-remWidth,remBackGround.getHeight()-2);
		gGL.color4fv(LLColor4::white.mV);
		gl_rect_2d(inTextBox);

		//draw text in black of rightclicktext
		//if nothing set, give hints
		std::string textToDrawInRightClickBox = sInstance->currentRightClickText;
		LLColor4 textColor = LLColor4::black;
		if(textToDrawInRightClickBox=="")
		{
			textToDrawInRightClickBox="Start Typing, then click here";
			textColor=LLColor4::grey;
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
		textToDrawInRightClickBox
			, 0,
			inTextBox.mLeft,
			inTextBox.mBottom+4,
			textColor, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);

		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				//rename avatar
				if(sInstance->currentRightClickText!="")
				{
					LGGContactSets::getInstance()->setPseudonym(selected[0],sInstance->currentRightClickText);
					sInstance->updateGroupsList();
					LLFirstUse::usePhoenixContactSetRename();
					LLVOAvatar::invalidateNameTag(selected[0]);
				}
			}
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
			"Set Alias:"
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
		top-=heightPer;

		if(LGGContactSets::getInstance()->hasPseudonym(selected[0]))
		{

			remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
			if(remBackGround.pointInRect(mouse_x,mouse_y))
			{
				//draw hover effect
				gGL.color4fv(LLColor4::yellow.mV);
				gl_rect_2d(remBackGround,FALSE);
				if(justClicked)
				{
					//cler avs rename
					LGGContactSets::getInstance()->clearPseudonym(selected[0]);
					LLVOAvatar::invalidateNameTag(selected[0]);
					sInstance->generateCurrentList();
					sInstance->updateGroupsList();
				}
			}
			LLFontGL::getFontSansSerif()->renderUTF8(
				"Remove Alias"
				, 0,
				contextRect.mLeft,
				top-(heightPer/2)-2,
				LLColor4::white, LLFontGL::LEFT,
				LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
			top-=heightPer;


		}
		if(isNonFriend)
		{
			remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
			if(remBackGround.pointInRect(mouse_x,mouse_y))
			{
				//draw hover effect
				gGL.color4fv(LLColor4::yellow.mV);
				gl_rect_2d(remBackGround,FALSE);
				if(justClicked)
				{
					//cler avs rename
					LGGContactSets::getInstance()->removeNonFriendFromList(selected[0]);
					sInstance->generateCurrentList();
				}
			}
			LLFontGL::getFontSansSerif()->renderUTF8(
				"Remove From List"
				, 0,
				contextRect.mLeft,
				top-(heightPer/2)-2,
				LLColor4::white, LLFontGL::LEFT,
				LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
			top-=heightPer;
		}



		//rename stop

		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
		gGL.color4fv(friendColor.mV);
		gl_rect_2d(remBackGround);
		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				//profileclick
				LLFloaterAvatarInfo::showFromDirectory(selected[0]);
			}
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
			std::string("View Profile")
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
		top-=heightPer;

		//// map
		if(canMap)
		{
			remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
			gGL.color4fv(friendColor.mV);
			gl_rect_2d(remBackGround);
			if(remBackGround.pointInRect(mouse_x,mouse_y))
			{
				//draw hover effect
				gGL.color4fv(LLColor4::yellow.mV);
				gl_rect_2d(remBackGround,FALSE);
				if(justClicked)
				{
					//mapclick
					if( gFloaterWorldMap )
					{
						gFloaterWorldMap->trackAvatar(selected[0],avName);
						LLFloaterWorldMap::show(NULL, TRUE);
					}

				}
			}
			LLFontGL::getFontSansSerif()->renderUTF8(
				std::string("Map Avatar")
				, 0,
				contextRect.mLeft,
				top-(heightPer/2)-2,
				LLColor4::white, LLFontGL::LEFT,
				LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
			top-=heightPer;
		}

		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
		gGL.color4fv(friendColor.mV);
		gl_rect_2d(remBackGround);
		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				//im avatar
				char buffer[MAX_STRING];
				LLAvatarName avatar_name;
				if (LLAvatarNameCache::get(selected[0], &avatar_name))
				{
					snprintf(buffer, MAX_STRING, "%s", avatar_name.getLegacyName().c_str());
					gIMMgr->setFloaterOpen(TRUE);
					gIMMgr->addSession(
						buffer,
						IM_NOTHING_SPECIAL,
						selected[0]);
				}
			}
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
			std::string("Instant Message")
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
		top-=heightPer;

		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
		gGL.color4fv(friendColor.mV);
		gl_rect_2d(remBackGround);
		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				//offer tp click
				handle_lure(selected[0]);
			}
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
			std::string("Teleport Avatar")
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
		
	}
	//group of avatars
	if(selected.size()>1)
	{
		LLColor4 groupColor = LGGContactSets::getInstance()->getGroupColor(*currentGroup);
		LLDynamicArray<LLUUID> ids;
		for(int se=0;se<selected.size();se++)
		{
			LLUUID avid= selected[se];
			if(!LGGContactSets::getInstance()->isNonFriend(avid))//dont mass tp or confrence non friends
			{
				ids.push_back(avid);
			}
		}

		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				//no
			}
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
			std::string(llformat("All %d Selected",ids.size()))
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
		top-=heightPer;

		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
		gGL.color4fv(groupColor.mV);
		gl_rect_2d(remBackGround);
		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				//confrence				
				gIMMgr->setFloaterOpen(TRUE);
				gIMMgr->addSession(std::string(*currentGroup+" Conference"), IM_SESSION_CONFERENCE_START, ids[0], ids);
			}
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
			std::string(llformat("Start Conference Call (%d)",ids.size()))
			, 0, 
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
		top-=heightPer;

		remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
		gGL.color4fv(groupColor.mV);
		gl_rect_2d(remBackGround);
		if(remBackGround.pointInRect(mouse_x,mouse_y))
		{
			//draw hover effect
			gGL.color4fv(LLColor4::yellow.mV);
			gl_rect_2d(remBackGround,FALSE);
			if(justClicked)
			{
				//mass tp
				handle_lure(ids);
			}
		}
		LLFontGL::getFontSansSerif()->renderUTF8(
			std::string(llformat("Send Mass TP (%d)",ids.size()))
			, 0,
			contextRect.mLeft,
			top-(heightPer/2)-2,
			LLColor4::white, LLFontGL::LEFT,
			LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
		


	}

	top-=heightPer;
	top-=heightPer;
	remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
	if(remBackGround.pointInRect(mouse_x,mouse_y))
	{
		//draw hover effect
		gGL.color4fv(LLColor4::yellow.mV);
		gl_rect_2d(remBackGround,FALSE);
		if(justClicked)
			selected.clear();
	}
	LLFontGL::getFontSansSerif()->renderUTF8(
		std::string(llformat("Deselect All (%d)",selected.size()))
		, 0,
		contextRect.mLeft,
		top-(heightPer/2)-2,
		LLColor4::white, LLFontGL::LEFT,
		LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
	top-=heightPer;

	remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
	if(remBackGround.pointInRect(mouse_x,mouse_y))
	{
		//draw hover effect
		gGL.color4fv(LLColor4::yellow.mV);
		gl_rect_2d(remBackGround,FALSE);
		if(justClicked)
		{
			/*std::vector<LLUUID> newSelected;
			newSelected.clear();
			for(int pp=0;pp<currentList.size();pp++)//dont use snapshot, get anyhting new
			{
				newSelected.push_back(currentList[pp]);
			}*/
			sInstance->selected=sInstance->currentList;
		}
	}
	LLFontGL::getFontSansSerif()->renderUTF8(
		std::string(llformat("Select All (%d)",currentList.size()-selected.size()))
		, 0,
		contextRect.mLeft,
		top-(heightPer/2)-2,
		LLColor4::white, LLFontGL::LEFT,
		LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);

	top-=heightPer;
	top-=heightPer;
	remBackGround.setLeftTopAndSize(contextRect.mLeft,top,width,heightPer);
	if(remBackGround.pointInRect(mouse_x,mouse_y))
	{
		//draw hover effect
		gGL.color4fv(LLColor4::yellow.mV);
		gl_rect_2d(remBackGround,FALSE);
		if(justClicked)
		{
			//add new av
			LLFloaterAvatarPicker* picker = LLFloaterAvatarPicker::show(onPickAvatar, NULL, FALSE, TRUE);
			sInstance->addDependentFloater(picker);
			
		}
	}
	LLFontGL::getFontSansSerif()->renderUTF8(
		"Add New Avatar"
		, 0,
		contextRect.mLeft,
		top-(heightPer/2)-2,
		LLColor4::white, LLFontGL::LEFT,
		LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
	

	if(justClicked)
	{
		showRightClick=FALSE;
		if(selected.size()==1)selected.clear();
	}
	justClicked=FALSE;	
}
void lggContactSetsFloater::drawFilter()
{
	if(sInstance->currentFilter=="")return;
	int mySize = 40;

	LLRect rec = sInstance->getChild<LLPanel>("top_region")->getRect();
	LLRect aboveThisMess;
	aboveThisMess.setLeftTopAndSize(rec.mLeft,rec.mTop+mySize,rec.getWidth(),mySize);
	LLColor4 backGround(0,0,0,1.0);
	LLColor4 foreGround(1,1,1,1.0);
	if(aboveThisMess.pointInRect(sInstance->mouse_x,sInstance->mouse_y))
	{
		backGround=LLColor4(0,0,0,.4);
		foreGround=LLColor4(1,1,1,.4);
		gGL.color4fv(LLColor4(0,0,0,.2).mV);//for main bg
	}else
		gGL.color4fv(LLColor4(0,0,0,.8).mV);

	gl_rect_2d(aboveThisMess);
	std::string preText("Currently Using Filter: ");
	int width1 = LLFontGL::getFontSansSerif()->getWidth(preText)+8;
	int width2 = LLFontGL::getFontSansSerif()->getWidth(sInstance->currentFilter)+8;
	int tSize = 24;
	LLRect fullTextBox;
	fullTextBox.setLeftTopAndSize(aboveThisMess.mLeft+20,aboveThisMess.getCenterY()+(tSize/2),width1+width2,tSize);
	gGL.color4fv(backGround.mV);
	gl_rect_2d(fullTextBox);
	gGL.color4fv(foreGround.mV);
	gl_rect_2d(fullTextBox,FALSE);
	LLRect filterTextBox;
	filterTextBox.setLeftTopAndSize(fullTextBox.mLeft+width1,fullTextBox.mTop,width2,tSize);
	gl_rect_2d(filterTextBox);	
	
	LLFontGL::getFontSansSerif()->renderUTF8(
		preText
		, 0,
		fullTextBox.mLeft+4,
		fullTextBox.mBottom+4,
		foreGround, LLFontGL::LEFT,
		LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
	LLFontGL::getFontSansSerif()->renderUTF8(
		sInstance->currentFilter
		, 0,
		filterTextBox.mLeft+4,
		filterTextBox.mBottom+4,
		backGround, LLFontGL::LEFT,
		LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);


}
void lggContactSetsFloater::draw()
{
	LLFloater::draw();
	if(sInstance->isMinimized())return;

	LLFontGL* font = LLFontGL::getFontSansSerifSmall();
	LLFontGL* bigFont = LLFontGL::getFontSansSerifBig();
	LLFontGL* hugeFont = LLFontGL::getFontSansSerifHuge();


	static std::string *currentGroup = rebind_llcontrol<std::string>("PhoenixContactSetsSelectedGroup", &gSavedSettings, true);
	static BOOL *textNotBg = rebind_llcontrol<BOOL>("PhoenixContactSetsColorizeText",&gSavedSettings,true);
	static BOOL *barNotBg = rebind_llcontrol<BOOL>("PhoenixContactSetsColorizeBar",&gSavedSettings,true);
	static BOOL *requireCTRL = rebind_llcontrol<BOOL>("PhoenixContactSetsRequireCTRL",&gSavedSettings,true);
	static BOOL *doZoom = rebind_llcontrol<BOOL>("PhoenixContactSetsDoZoom",&gSavedSettings,true);
	static BOOL *doColorChange = rebind_llcontrol<BOOL>("PhoenixContactSetsUseColorHighlight",&gSavedSettings,true);

		
	std::vector<LLUUID> workingList;
	workingList= currentList;
	int numberOfPanels = workingList.size();//45;
	//see if we are guna draw some folders
	allFolders=LGGContactSets::getInstance()->getInnerGroups(*currentGroup);
	numberOfPanels+=allFolders.size();

	LLRect topScroll = getChild<LLPanel>("top_region")->getRect();
	LLRect bottomScroll = getChild<LLPanel>("bottom_region")->getRect();
	LLPanel * mainPanel = getChild<LLPanel>("draw_region");
	LLRect rec  = mainPanel->getRect();
	if((rec.pointInRect(mouse_x,mouse_y))&&(sInstance->hasFocus()))//||justClicked||showRightClick))
	{
		//mainPanel->setFocus(TRUE);
		//sInstance->setFocus(TRUE);
		sInstance->getChild<LLCheckBoxCtrl>("haxCheckbox")->setFocus(TRUE);
		
	}
	//LLPanel *panel = getChild<LLPanel>("draw_region");
	
	gGL.pushMatrix();
	gl_circle_2d(rec.getCenterX(),rec.getCenterY(),2.0f,(S32)30,false);
	int bMag = 35;
	if(!(*doZoom))bMag=0;
	//kinda magic numbers to compensate for max bloom effect and stuff
	float sizeV = (F32)((rec.getHeight()-143)-(((F32)numberOfPanels)*1.07f)-0)/(F32)(numberOfPanels);
	if(!(*doZoom))sizeV= (F32)((rec.getHeight()-0-(numberOfPanels*2)))/(F32)(numberOfPanels);
	maxSize=sizeV+bMag;
	int minSize = 10;
	if(!(*doZoom))minSize=24;
	if(sizeV<minSize)
	{
		//need scroll bars
		sizeV=minSize;
//#pragma region ScrollBars
		if(this->hasFocus())
		{
			LLUIImage *arrowUpImage = LLUI::getUIImage("map_avatar_above_32.tga");
			LLUIImage *arrowDownImage = LLUI::getUIImage("map_avatar_below_32.tga");
			LLColor4 active = LGGContactSets::getInstance()->getGroupColor(*currentGroup);
			LLColor4 unactive = LGGContactSets::toneDownColor(active,.5);
			static S32 *scrollSpeedSetting = rebind_llcontrol<S32>("PhoenixContactSetsScrollSpeed", &gSavedSettings, true);
			float speedFraction = ((F32)(*scrollSpeedSetting))/100.0f;

			LLColor4 useColor = unactive;
			if(mouse_y<topScroll.mTop&& mouse_y > topScroll.mBottom)
			{
				useColor=active;
				scrollLoc-=llclamp((S32)((((F32)numberOfPanels)/4.0f)*speedFraction),1,100);
			}
			if(scrollLoc>0)
			{
				gGL.color4fv(useColor.mV);
				gl_rect_2d(topScroll,true);
			}		
			
			int x=topScroll.mLeft;
			if(scrollLoc>0)
			for(;x<topScroll.mRight-topScroll.getHeight();x+=(30+topScroll.getHeight()))
			{
				gl_draw_scaled_image_with_border(x,
					topScroll.mBottom,
					topScroll.getHeight(),topScroll.getHeight(),
					arrowUpImage->getImage(),
					useColor,
					FALSE); 
			}
			int maxS =((numberOfPanels*11)+200-(rec.getHeight()));
			if(!(*doZoom))maxS=((numberOfPanels*(minSize+2))+10-(rec.getHeight()));
			useColor=unactive;
			if(mouse_y<bottomScroll.mTop && mouse_y > bottomScroll.mBottom)
			{
				useColor=active;
				scrollLoc+=llclamp((S32)((((F32)numberOfPanels)/4.0f)*speedFraction),1,100);
			}
			if(scrollLoc<maxS)
			{
				gGL.color4fv(useColor.mV);
				gl_rect_2d(bottomScroll,true);
			}
			if(scrollLoc<maxS)
				for(x=bottomScroll.mLeft;x<bottomScroll.mRight-bottomScroll.getHeight();x+=(30+bottomScroll.getHeight()))
				{
					gl_draw_scaled_image_with_border(x,
						bottomScroll.mBottom,
						bottomScroll.getHeight(),topScroll.getHeight(),
						arrowDownImage->getImage(),
						useColor,
						FALSE); 
				}
			scrollLoc = llclamp(scrollLoc,0,maxS);
		}
//#pragma endregion ScrollBars

	}
	else
	{
		scrollLoc=0;
	}
	float top=rec.mTop+scrollLoc;//sizeV+12;
	if(mouse_y<15)mouse_y=15;
	for(int f=0; f< allFolders.size();f++)
	{
		float thisSize = sizeV;
		float pi = 3.1415f;
		float piOver2Centered = pi/2+( (top-((F32)(sizeV+(40))/2.0f)-mouse_y)*.01);
		float bubble =sin((float)llclamp(piOver2Centered,0.0f,pi));//*bMag;
		thisSize+=(bubble*bMag);

		if((top-thisSize)<rec.mBottom)continue; 
		if((top-thisSize)>rec.mTop){}
		else
		{
			//draw folder stuff
			if((top)>rec.mTop){top=rec.mTop;}//draw as much as the top one as we can

			LLRect box;
			box.setLeftTopAndSize(rec.mLeft+(bMag/2)+5-((bubble*bMag)/2),llceil(top+.00001),(rec.getWidth()-bMag-10)+((bubble*bMag)/1),(int)llfloor(thisSize+.00001f));


			std::string folder= allFolders[f];

			LLColor4 color = LGGContactSets::getInstance()->getGroupColor(folder);
			
			if(*doColorChange)
				color = LGGContactSets::toneDownColor(color,((F32)bubble+.001)/(1.0f));

			gGL.color4fv(color.mV);			
			if(!(*barNotBg) && !(*textNotBg))
			{
				gl_rect_2d(box);
			}else
			{
				LLRect smallBox = box;
				smallBox.setLeftTopAndSize(box.mLeft,box.mTop,10+(bubble/2),box.getHeight());
				gl_rect_2d(smallBox);
				smallBox.setLeftTopAndSize(box.mLeft+10+(bubble/2),box.mTop,box.getWidth()-(10+(bubble/2)),box.getHeight());
				gGL.color4fv(LGGContactSets::toneDownColor(LGGContactSets::getInstance()->getDefaultColor(),(*doColorChange)?(((F32)bubble)/(1)):1).mV);
				gl_rect_2d(smallBox);
			}
			if(box.pointInRect(mouse_x,mouse_y))
			{
				gGL.color4fv(LLColor4::white.mV);
				gl_rect_2d(box,FALSE);
				if(justClicked)
				{
					justClicked=FALSE;
					gSavedSettings.setString("PhoenixContactSetsSelectedGroup",folder);
					sInstance->updateGroupGUIs();
					sInstance->selected.clear();
					sInstance->generateCurrentList();
				}
			}

			LLFontGL* useFont = font;
			if(thisSize>25)useFont = bigFont;
			if(thisSize>36)useFont = hugeFont;
			if(*doZoom)
			{
				if(thisSize>14)useFont = bigFont;
				if(thisSize>25)useFont = hugeFont;
			}

			int roomForBar = 0;
			if((*barNotBg)||(*textNotBg))roomForBar=10+(bubble/2);

			int size =llclamp(thisSize+(bubble*bMag/2),
				llmax(10.0f,llmin((((F32)box.getHeight())/1.0f),20.0f)),
				llmin(20+(bubble*bMag/2),thisSize+(bubble*bMag/2)));

			int xLoc = box.mLeft+roomForBar;//size;
			LLUIImage *selectedImage = LLUI::getUIImage("inv_folder_plain_closed.tga");
			LLRect imageBox;
			imageBox.setLeftTopAndSize(xLoc,(box.getHeight()/2)+0+box.mBottom+(size/2),size,size);			
			gl_draw_scaled_image_with_border(imageBox.mLeft,imageBox.mBottom,
				imageBox.getWidth(),imageBox.getHeight(),
				selectedImage->getImage(),
				LLColor4::white,
				FALSE);

			LLColor4 groupTextColor = LLColor4::white;
			if((*textNotBg))groupTextColor=LGGContactSets::toneDownColor(color,1.0f);

			useFont->renderUTF8(
				folder
				, 0,
				box.mLeft+roomForBar+size+2,
				top-(thisSize/2)+((*doZoom)?-2:2),
				groupTextColor, LLFontGL::LEFT,
				LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);

		}

		top-=(thisSize+1);
	}
	
	for(int p=0; p < (numberOfPanels-allFolders.size()) ; p++)
	{
		float thisSize = sizeV;
		float pi = 3.1415f;
		float piOver2Centered = pi/2+( (top-((F32)(sizeV+(40))/2.0f)-mouse_y)*.01);
		float bubble =sin((float)llclamp(piOver2Centered,0.0f,pi));//*bMag;
		thisSize+=(bubble*bMag);

		if((top-thisSize)<rec.mBottom)continue; 
		if((top-thisSize)>rec.mTop){}
		else
		{
//#pragma region DrawListItem
			if((top)>rec.mTop){top=rec.mTop;}//draw as much as the top one as we can

			LLRect box;
			box.setLeftTopAndSize(rec.mLeft+(bMag/2)+5-((bubble*bMag)/2),llceil(top+.00001),(rec.getWidth()-bMag-10)+((bubble*bMag)/1),(int)llfloor(thisSize+.00001f));

			BOOL hoveringThis=FALSE;
			if(top>mouse_y && (top-thisSize)<mouse_y)
			{
				hoveringThis=TRUE;
				
			}
			BOOL iAMSelected = FALSE;
			for(int i = 0; i < (int)selected.size();i++)
			{	
				if(selected[i]==workingList[p])
				{
					iAMSelected=TRUE;
				}
			}

			LLUUID agent_id = workingList[p];

			BOOL *showOtherGroups = rebind_llcontrol<BOOL>("PhoenixContactSetsShowOtherGroups", &gSavedSettings, true);
			std::string *cg = rebind_llcontrol<std::string>("PhoenixContactSetsSelectedGroup", &gSavedSettings, true);
			std::vector<std::string> groupsIsIn;				
			groupsIsIn= LGGContactSets::getInstance()->getFriendGroups(agent_id);

			LLColor4 color = LGGContactSets::getInstance()->getGroupColor(*cg);
			if(!LGGContactSets::getInstance()->isFriendInGroup(agent_id,*cg))
				color = LGGContactSets::getInstance()->getDefaultColor();
			if(*showOtherGroups)color = LGGContactSets::getInstance()->
				getFriendColor(agent_id,*cg);

			if(!iAMSelected&&(*doColorChange))
				color = LGGContactSets::toneDownColor(color,((F32)bubble+.001)/(1.0f));
			
			gGL.color4fv(color.mV);			
			if(!(*barNotBg) && !(*textNotBg))
			{
				gl_rect_2d(box);
			}else
			{
				LLRect smallBox = box;
				smallBox.setLeftTopAndSize(box.mLeft,box.mTop,10+(bubble/2),box.getHeight());
				gl_rect_2d(smallBox);
				smallBox.setLeftTopAndSize(box.mLeft+10+(bubble/2),box.mTop,box.getWidth()-(10+(bubble/2)),box.getHeight());
				gGL.color4fv(LGGContactSets::toneDownColor(LGGContactSets::getInstance()->getDefaultColor(),(*doColorChange)?(((F32)bubble)/(1)):1).mV);
				gl_rect_2d(smallBox);
			}

			//draw over lays (other group names)
			if(box.getHeight()>((*doZoom)?25:0))
			{
				int breathingRoom = 0;
				if(box.getHeight()>35)breathingRoom=4;

				int w =box.mLeft+breathingRoom;
				int sizePerOGroup = 40;
				for(int gr=0;gr<groupsIsIn.size();gr++)
				{
					std::string oGroupName = groupsIsIn[gr];
					sizePerOGroup=
						LLFontGL::getFontSansSerifSmall()->getWidth(oGroupName)+8;
					LLColor4 oGroupColor = LGGContactSets::getInstance()->getGroupColor(oGroupName);
					LLRect oGroupArea;
					oGroupArea.setLeftTopAndSize(w,box.mBottom+12+breathingRoom,sizePerOGroup,12+(breathingRoom/2));
					gGL.color4fv(oGroupColor.mV);			
					gl_rect_2d(oGroupArea);
					gGL.color4fv(LLColor4(1,1,1,.5).mV);			
					gl_rect_2d(oGroupArea,FALSE);
					LLFontGL::getFontSansSerifSmall()->renderUTF8(
						oGroupName
						, 0,
						w+4,
						box.mBottom+breathingRoom,
						LLColor4::white, LLFontGL::LEFT,
						LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
					if(oGroupArea.pointInRect(mouse_x,mouse_y))
					{
						gGL.color4fv(LLColor4(1,1,1,1.0).mV);			
						gl_rect_2d(oGroupArea,FALSE);
						if(justClicked)
						{
							justClicked=FALSE;
							gSavedSettings.setString("PhoenixContactSetsSelectedGroup",oGroupName);
							sInstance->updateGroupGUIs();
							sInstance->selected.clear();
							sInstance->generateCurrentList();
						}
					}
					w+=sizePerOGroup+5;

				}
			}
			//draw icons
			//not to small if we can, but not to big, but still have a good zoom effect
			int size =llclamp(thisSize+(bubble*bMag/2),
				llmax(10.0f,llmin((((F32)box.getHeight())/1.0f),20.0f)),
				llmin(20+(bubble*bMag/2),thisSize+(bubble*bMag/2)));

			std::string toolTipText="Friend is Unselected";
			std::string toDisplayToolTipText="";
			int xLoc = box.mRight-size;
			LLUIImage *selectedImage = LLUI::getUIImage("checkbox_enabled_false.tga");
			if(iAMSelected)
			{
				toolTipText="Friend is Selected";
				selectedImage = LLUI::getUIImage("checkbox_enabled_true.tga");
			}
			LLRect imageBox;

			gGL.color4fv(LLColor4::white.mV);
			
			imageBox.setLeftTopAndSize(xLoc,(box.getHeight()/2)+0+box.mBottom+(size/2),size,size);			
			gl_draw_scaled_image_with_border(imageBox.mLeft,imageBox.mBottom,
				imageBox.getWidth(),imageBox.getHeight(),
				selectedImage->getImage(),
				LLColor4::white,
				FALSE);
			if(imageBox.pointInRect(mouse_x,mouse_y))
			{
				gl_rect_2d(imageBox,FALSE);
				toDisplayToolTipText=toolTipText;
				if(justClicked&&!showRightClick)
				{
					justClicked=FALSE;
					toggleSelect(workingList[p]);
				}
			}

			LLUIImage *onlineImage = LLUI::getUIImage("icon_avatar_online.tga");
			toolTipText = "Friend is Online";
			LLColor4 overlay = LLColor4::white;
			if(LGGContactSets::getInstance()->isNonFriend(agent_id))
			{
				onlineImage = LLUI::getUIImage("icon_avatar_offline.tga");
				toolTipText="Not on your friends list.";
				overlay=LLColor4::black;
			}else
			if(!LLAvatarTracker::instance().getBuddyInfo(agent_id)->isOnline())
			{
				onlineImage = LLUI::getUIImage("icon_avatar_offline.tga");
				toolTipText="Friend is Offline";
			}
			
			imageBox.setLeftTopAndSize(xLoc-=(1+size),(box.getHeight()/2)+0+box.mBottom+(size/2),size,size);			
			gl_draw_scaled_image_with_border(imageBox.mLeft,imageBox.mBottom,
				imageBox.getWidth(),imageBox.getHeight(),
				onlineImage->getImage(), 
				overlay,
				FALSE);
			if(imageBox.pointInRect(mouse_x,mouse_y))
			{
				toDisplayToolTipText=toolTipText;
			}

			////im button
			LLUIImage *imImage = LLUI::getUIImage("icn_chatbar.tga");

			imageBox.setLeftTopAndSize(xLoc-=(1+size),(box.getHeight()/2)+0+box.mBottom+(size/2),size,size);			
			gl_draw_scaled_image_with_border(imageBox.mLeft,imageBox.mBottom,
				imageBox.getWidth(),imageBox.getHeight(),
				imImage->getImage(),
				LLColor4::white,
				FALSE); 
			if(imageBox.pointInRect(mouse_x,mouse_y))
			{
				gl_rect_2d(imageBox,FALSE);				
				toDisplayToolTipText="Instant Message";
				if(justClicked&&!showRightClick)
				{
					justClicked=FALSE;
					char buffer[MAX_STRING];
					// [Ansariel/Henri: Display name support]
					// snprintf(buffer, MAX_STRING, "%s", avlist->mAvatars[agent_id].getName().c_str());
					LLAvatarName avatar_name;
					if (LLAvatarNameCache::get(agent_id, &avatar_name))
					{
						snprintf(buffer, MAX_STRING, "%s", avatar_name.getLegacyName().c_str());
						gIMMgr->setFloaterOpen(TRUE);
						gIMMgr->addSession(
							buffer,
							IM_NOTHING_SPECIAL,
							agent_id);
					}
				}
			}
			//profile button
			LLUIImage *profileImage = LLUI::getUIImage("map_infohub.tga");
			imageBox.setLeftTopAndSize(xLoc-=(1+size),(box.getHeight()/2)+0+box.mBottom+(size/2),size,size);			
			gl_draw_scaled_image_with_border(imageBox.mLeft,imageBox.mBottom,
				imageBox.getWidth(),imageBox.getHeight(),
				profileImage->getImage(),
				LLColor4::white,
				FALSE); 
			if(imageBox.pointInRect(mouse_x,mouse_y))
			{
				gl_rect_2d(imageBox,FALSE);
				toDisplayToolTipText="View Profile";
				if(justClicked&&!showRightClick)
				{
					justClicked=FALSE;
					LLFloaterAvatarInfo::showFromDirectory(agent_id);
				}
			}

			//(if set) av has been renamed button
			if(LGGContactSets::getInstance()->hasPseudonym(agent_id))
			{
				LLUIImage *profileImage = LLUI::getUIImage("icn_voice-localchat.tga");
				imageBox.setLeftTopAndSize(xLoc-=(1+size),(box.getHeight()/2)+0+box.mBottom+(size/2),size,size);			
				gl_draw_scaled_image_with_border(imageBox.mLeft,imageBox.mBottom,
					imageBox.getWidth(),imageBox.getHeight(),
					profileImage->getImage(),
					LLColor4::white,
					FALSE); 
				if(imageBox.pointInRect(mouse_x,mouse_y))
				{
					//gl_rect_2d(imageBox,FALSE);
					toDisplayToolTipText="Name has been changed.";
					if(justClicked&&!showRightClick)
					{
						//nothing yet
					}
				}
			}
			//draw hover and selected			
			if(iAMSelected)
			{
				gGL.color4fv(LLColor4(1,1,1,1.0).mV);
				gl_rect_2d(box,FALSE);
				//gGL.color4fv(LLColor4(1,1,1,.7).mV);
				//gl_circle_2d(box.mRight-(box.getHeight()/2),box.mTop-(box.getHeight()/2),box.getHeight()/2,20,TRUE);

			}
			if(hoveringThis)
			{
				gGL.color4fv(LLColor4::yellow.mV);
				gl_rect_2d(box,FALSE);
				if(justClicked&&!showRightClick)
				{
					//if(requeireCTRL)//todo add setting
					BOOL found = FALSE;
					if((*requireCTRL)&&(!gKeyboard->getKeyDown(KEY_CONTROL)))
					{
						found = toggleSelect(workingList[p]);
						selected.clear();
					}
					if(!found)toggleSelect(workingList[p]);
				}
				
				if(showRightClick)
				{
					if(selected.size()<1)
						toggleSelect(workingList[p]);
				}
				
			}

			//draw tooltip
			if(toDisplayToolTipText!="")
			{
				int tsize = LLFontGL::getFontSansSerifSmall()->getWidth(toDisplayToolTipText)+8;
				LLRect toolRect;toolRect.setLeftTopAndSize(mouse_x-tsize,mouse_y+16,tsize,16);
				gGL.color4fv(LLColor4::black.mV);
				gl_rect_2d(toolRect);
				//gGL.color4fv(LLColor4::yellow.mV);
				gl_rect_2d(toolRect,FALSE);
				LLFontGL::getFontSansSerifSmall()->renderUTF8(toDisplayToolTipText
					, 0,toolRect.mLeft+4,
					toolRect.mBottom+2,
					LLColor4::white, LLFontGL::LEFT,
					LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
			}

			if(thisSize>8)//draw name
			{
				std::string text="";
				LLAvatarName avatar_name;
				if (LLAvatarNameCache::get(agent_id, &avatar_name))
				{
					std::string fullname;
					static S32* sPhoenixNameSystem = rebind_llcontrol<S32>("PhoenixNameSystem", &gSavedSettings, true);
					switch (*sPhoenixNameSystem)
					{
					case 0 : fullname = avatar_name.getLegacyName(); break;
					case 1 : fullname = (avatar_name.mIsDisplayNameDefault? avatar_name.mDisplayName : avatar_name.getCompleteName()); break;
					case 2 : fullname = avatar_name.mDisplayName; break;
					default : fullname = avatar_name.getCompleteName(); break;
					}

					text+="" +fullname;
				}
				LLFontGL* useFont = font;
				if(thisSize>25)useFont = bigFont;
				if(thisSize>36)useFont = hugeFont;
				if(*doZoom)
				{
					if(thisSize>14)useFont = bigFont;
					if(thisSize>25)useFont = hugeFont;
				}
				
				int roomForBar = 0;
				if((*barNotBg)||(*textNotBg))roomForBar=10+(bubble/2);

				LLColor4 nameTextColor = LLColor4::white;
				if((*textNotBg)&(groupsIsIn.size()>0))nameTextColor=LGGContactSets::toneDownColor(color,1.0f);

				useFont->renderUTF8(
					text
					, 0,
					box.mLeft+roomForBar,
					top-(thisSize/2)+((*doZoom)?-2:2),
					nameTextColor, LLFontGL::LEFT,
					LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);

				//useFont->renderUTF8(llformat(" %d ",p),0,box.mLeft+0,top-(thisSize/2),LLColor4::white,LLFontGL::LEFT,
				//	LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
			}

//#pragma endregion DrawListItem
		}

		top-=(thisSize+1);

	}

	///mouse
	if(mouse_x!=0 && mouse_y!=0 && mouse_y<rec.mTop && (gFrameTimeSeconds-2<hovered))
	{
		gGL.color4fv(LLColor4::black.mV);
		//gl_circle_2d(mouse_x,mouse_y,20.0f,((S32)(gFrameTimeSeconds))%4 + 3,false);
	}

	/*font->renderUTF8(
		llformat("Mouse is at %d , %d scoll lock is %d height is %d panels is %d",
		mouse_x,mouse_y,scrollLoc,rec.getHeight(),numberOfPanels)
		, 0,
		rec.mLeft,
		rec.mBottom+0,
		LLColor4::white, LLFontGL::LEFT,
		LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);*/

	drawFilter();
	if(showRightClick)drawRightClick();

	gGL.popMatrix();
	justClicked=FALSE;
}
BOOL lggContactSetsFloater::toggleSelect(LLUUID whoToToggle)
{
	justClicked=FALSE;
	bool found = false;
	for(int i = 0; i < (int)selected.size();i++)
	{
		if(selected[i]==whoToToggle)
			found=true;
	}
	if(!found)
		selected.push_back(whoToToggle);
	else
	{
		std::vector<LLUUID> newList;
		newList.clear();
		for(int i = 0; i < (int)selected.size();i++)
		{
			if(selected[i]!=whoToToggle)
				newList.push_back(selected[i]);
		}
		selected=newList;
	}
	return found;
	
}
BOOL lggContactSetsFloater::handleMouseDown(S32 x,S32 y,MASK mask)
{
	sInstance->justClicked=true;
	return LLFloater::handleMouseDown(x,y,mask);
}
BOOL lggContactSetsFloater::handleRightMouseDown(S32 x,S32 y,MASK mask)
{
	if(!showRightClick)
	{
		contextRect.setLeftTopAndSize(x,y,2,2);
		showRightClick=TRUE;
		sInstance->currentRightClickText="";
	}else
	{
		justClicked=TRUE;
	}
	return LLFloater::handleRightMouseDown(x,y,mask);
}
BOOL lggContactSetsFloater::handleScrollWheel(S32 x, S32 y, S32 clicks)
{
	LLRect rec  = sInstance->getChild<LLPanel>("draw_region")->getRect();

	int maxS =(((sInstance->currentList.size()+sInstance->allFolders.size())*11)+200-(rec.getHeight()));
	BOOL *doZoom = rebind_llcontrol<BOOL>("PhoenixContactSetsDoZoom",&gSavedSettings,true);
	if(!(*doZoom))maxS=(((sInstance->currentList.size()+sInstance->allFolders.size())*(26))+10-(rec.getHeight()));

	int moveAmt=12;
	if(!(*doZoom))moveAmt=26;
	

	sInstance->scrollLoc=llclamp(sInstance->scrollLoc+(clicks*moveAmt),0,maxS);
	return LLFloater::handleScrollWheel(x,y,clicks);
}
BOOL lggContactSetsFloater::handleUnicodeCharHere(llwchar uni_char)
{
	if ((uni_char < 0x20) || (uni_char == 0x7F)) // Control character or DEL
	{
		return FALSE;
	}

	if(' ' == uni_char 
		&& !gKeyboard->getKeyRepeated(' ')
		&& 
		((!sInstance->showRightClick&&sInstance->currentFilter=="")||(sInstance->showRightClick&&sInstance->currentRightClickText==""))
		)
	{
		sInstance->justClicked=TRUE;
	}else
	{
		if(gKeyboard->getKeyDown(KEY_CONTROL)&&22==(U32)uni_char)
		{
			std::string toPaste=wstring_to_utf8str(gClipboard.getPasteWString());
			if(sInstance->showRightClick)
			{

				sInstance->currentRightClickText+=toPaste;
			}else
			{
				sInstance->currentFilter+=toPaste;
			}
		}else
		if(((U32)uni_char!=27)&&((U32)uni_char!=8))
		{
			//sInstance->getChild<LLCheckBoxCtrl>("haxCheckbox")->setFocus(TRUE);
			if(!sInstance->showRightClick)
			{
				sInstance->currentFilter+=uni_char;
				sInstance->generateCurrentList();
			}else
			{
				sInstance->currentRightClickText+=uni_char;
			}
			
			return TRUE;
		}
	}

	return LLFloater::handleUnicodeCharHere(uni_char);
}
BOOL lggContactSetsFloater::handleKeyHere( KEY key, MASK mask )
{
	LLRect rec  = sInstance->getChild<LLPanel>("draw_region")->getRect();
	
	int maxS =(((sInstance->currentList.size()+sInstance->allFolders.size())*11)+200-(rec.getHeight()));
	static BOOL *doZoom = rebind_llcontrol<BOOL>("PhoenixContactSetsDoZoom",&gSavedSettings,true);
	if(!(*doZoom))maxS=(((sInstance->currentList.size()+sInstance->allFolders.size())*(26))+10-(rec.getHeight()));
	std::string localFilter = sInstance->currentFilter;
	if(sInstance->showRightClick)localFilter=sInstance->currentRightClickText;

	int curLoc = sInstance->scrollLoc;
	
	if(key==KEY_PAGE_UP)
	{
		curLoc-=rec.getHeight();
	}else if(key == KEY_UP)
	{

		if(!(*doZoom))curLoc-=26;
		else curLoc-=12;
	}else
	if(key==KEY_PAGE_DOWN)
	{
		curLoc+=rec.getHeight();
	}else if(key==KEY_DOWN)
	{
		if(!(*doZoom))curLoc+=26;
		else curLoc+=12;
	}
	if(key==KEY_ESCAPE)
	{
		if(localFilter!="")
		{
			sInstance->currentFilter="";
			sInstance->generateCurrentList();
			return TRUE;
		}
		if(sInstance->showRightClick)
		{
			sInstance->showRightClick=FALSE;
			return TRUE;
		}
	}
	if(key==KEY_RETURN)
	{
		sInstance->justClicked=TRUE;
	}
	if(key==KEY_BACKSPACE)
	{
		int length = localFilter.length();
		if(length>0)
		{
			length--;
			if(!sInstance->showRightClick)
			{
				sInstance->currentFilter=localFilter.substr(0,length);
				sInstance->generateCurrentList();
			}else
			{
				sInstance->currentRightClickText=localFilter.substr(0,length);
			}
		}
	}

	sInstance->scrollLoc=llclamp(curLoc,0,maxS);

	return LLFloater::handleKeyHere(key,mask);
}
BOOL lggContactSetsFloater::handleDoubleClick(S32 x, S32 y, MASK mask)
{
	LLRect rec  = sInstance->getChild<LLPanel>("draw_region")->getRect();
	LLRect topScroll = sInstance->getChild<LLPanel>("top_region")->getRect();
	LLRect bottomScroll = sInstance->getChild<LLPanel>("bottom_region")->getRect();

	int maxS =(((sInstance->currentList.size()+sInstance->allFolders.size())*11)+200-(rec.getHeight()));
	static BOOL *doZoom = rebind_llcontrol<BOOL>("PhoenixContactSetsDoZoom",&gSavedSettings,true);
	if(!(*doZoom))maxS=(((sInstance->currentList.size()+sInstance->allFolders.size())*(26))+10-(rec.getHeight()));
	
	if(bottomScroll.pointInRect(x,y))
	{
		sInstance->scrollLoc=maxS;
	}else if(topScroll.pointInRect(x,y))
	{
		sInstance->scrollLoc=0;
	}
	return LLFloater::handleDoubleClick(x,y,mask);
}
BOOL lggContactSetsFloater::handleHover(S32 x,S32 y,MASK mask)
{
	mouse_x=x;
	mouse_y=y;
	hovered=gFrameTimeSeconds;
	return LLFloater::handleHover(x,y,mask);
}
BOOL lggContactSetsFloater::compareAv(LLUUID av1, LLUUID av2)
{
	std::string avN1("");
	std::string avN2("");
	LLAvatarName avatar_name;
	if (LLAvatarNameCache::get(av1, &avatar_name))
	{
		std::string fullname;
		static S32* sPhoenixNameSystem = rebind_llcontrol<S32>("PhoenixNameSystem", &gSavedSettings, true);
		switch (*sPhoenixNameSystem)
		{
		case 0 : fullname = avatar_name.getLegacyName(); break;
		case 1 : fullname = (avatar_name.mIsDisplayNameDefault? avatar_name.mDisplayName : avatar_name.getCompleteName()); break;
		case 2 : fullname = avatar_name.mDisplayName; break;
		default : fullname = avatar_name.getCompleteName(); break;
		}

		avN1=fullname;
	}
	if (LLAvatarNameCache::get(av2, &avatar_name))
	{
		std::string fullname;
		static S32* sPhoenixNameSystem = rebind_llcontrol<S32>("PhoenixNameSystem", &gSavedSettings, true);
		switch (*sPhoenixNameSystem)
		{
		case 0 : fullname = avatar_name.getLegacyName(); break;
		case 1 : fullname = (avatar_name.mIsDisplayNameDefault? avatar_name.mDisplayName : avatar_name.getCompleteName()); break;
		case 2 : fullname = avatar_name.mDisplayName; break;
		default : fullname = avatar_name.getCompleteName(); break;
		}

		avN2=fullname;
	}
	LLStringUtil::toLower(avN2);
	LLStringUtil::toLower(avN1);
	
	return (avN1.compare(avN2))<0;
}
BOOL lggContactSetsFloater::generateCurrentList()
{
	static BOOL *showOnline = rebind_llcontrol<BOOL>("PhoenixContactSetsShowOnline", &gSavedSettings, true);
	static BOOL *showOffline = rebind_llcontrol<BOOL>("PhoenixContactSetsShowOffline", &gSavedSettings, true);
	static BOOL *yshowAllFriends = rebind_llcontrol<BOOL>("PhoenixContactSetsShowAllFriends", &gSavedSettings, true);
	//static BOOL *showOtherGroups = rebind_llcontrol<BOOL>("PhoenixContactSetsShowOtherGroups", &gSavedSettings, true);
	static std::string *currentGroup = rebind_llcontrol<std::string>("PhoenixContactSetsSelectedGroup", &gSavedSettings, true);

	currentList.clear();
	std::map<LLUUID, LLRelationship*>::iterator p;
	std::map<LLUUID, LLRelationship*> friends;
	LLAvatarTracker::instance().copyBuddyList(friends);

	for(p = friends.begin(); 
		p != friends.end(); p++)
	{
		LLRelationship* relation = p->second;
		if((! (*showOnline))&&(relation->isOnline()))continue;
		if((! (*showOffline))&&(!relation->isOnline()))continue;		
		if((! (*yshowAllFriends)&&(!LGGContactSets::getInstance()->isFriendInGroup(p->first,*currentGroup))))continue;
		

		currentList.push_back(p->first);
	}
	//add ppl not in friends list
	std::vector<LLUUID> nonFriends = LGGContactSets::getInstance()->getListOfNonFriends();
	//currentList.insert(currentList.end(), nonFriends.begin(), nonFriends.end());
	for(int i=0;i<nonFriends.size();i++)
	{
		if(! (*showOffline))continue;	
		if((! (*yshowAllFriends)&&(!LGGContactSets::getInstance()->isFriendInGroup(nonFriends[i],*currentGroup))))continue;

		currentList.push_back(nonFriends[i]);
	}

	//filter \o/
	if(sInstance->currentFilter!="")
	{
		std::vector<LLUUID> newList;
		std::string workingFilter = sInstance->currentFilter;
		LLStringUtil::toLower(workingFilter);
		for(int itFilter=0;itFilter<currentList.size();itFilter++)
		{

			std::string avN("");
			LLAvatarName avatar_name;
			if (LLAvatarNameCache::get(currentList[itFilter], &avatar_name))
			{
				std::string fullname;
				static S32* sPhoenixNameSystem = rebind_llcontrol<S32>("PhoenixNameSystem", &gSavedSettings, true);
				switch (*sPhoenixNameSystem)
				{
				case 0 : fullname = avatar_name.getLegacyName(); break;
				case 1 : fullname = (avatar_name.mIsDisplayNameDefault? avatar_name.mDisplayName : avatar_name.getCompleteName()); break;
				case 2 : fullname = avatar_name.mDisplayName; break;
				default : fullname = avatar_name.getCompleteName(); break;
				}
				avN=fullname;
			}

			LLStringUtil::toLower(avN);			
			if(avN.find(workingFilter)!=std::string::npos)
			{
				newList.push_back(currentList[itFilter]);
			}
		}
		currentList=newList;
	}


	std::sort(currentList.begin(),currentList.end(),&lggContactSetsFloater::compareAv);
	return TRUE;
}
void lggContactSetsFloater::onClickDelete(void* data)
{
	std::string *currentGroup = rebind_llcontrol<std::string>("PhoenixContactSetsSelectedGroup", &gSavedSettings, true);

	LGGContactSets::getInstance()->deleteGroup(*currentGroup);
	gSavedSettings.setString("PhoenixContactSetsSelectedGroup","");
	sInstance->updateGroupsList();
}
void lggContactSetsFloater::onClickNew(void* data)
{
	LLLineEditor* line =sInstance->getChild<LLLineEditor>("lgg_fg_groupNewName");
	std::string text = line->getText();
	if(text!="")
	{
		LGGContactSets::getInstance()->addGroup(text);
		line->setText(LLStringExplicit(""));
	}

	sInstance->updateGroupsList();
}
void lggContactSetsFloater::onClickSettings(void* data)
{
	lggContactSetsFloaterStart::showSettings(TRUE);
}
class lggContactSetsFloaterSettings;
class lggContactSetsFloaterSettings : public LLFloater, public LLFloaterSingleton<lggContactSetsFloaterSettings>
{
public:
	lggContactSetsFloaterSettings(const LLSD& seed);
	virtual ~lggContactSetsFloaterSettings();

	BOOL postBuild(void);
	static void onClickOk(void* data);
	static void onDefaultBackgroundChange(LLUICtrl* ctrl, void* userdata);
	static lggContactSetsFloaterSettings* sSettingsInstance;
};


lggContactSetsFloaterSettings* lggContactSetsFloaterSettings::sSettingsInstance;

lggContactSetsFloaterSettings::~lggContactSetsFloaterSettings()
{
	sSettingsInstance = NULL;
}
lggContactSetsFloaterSettings::lggContactSetsFloaterSettings(const LLSD& seed)
{
	if(sSettingsInstance)delete sSettingsInstance;
	sSettingsInstance= this;
	
	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_contactsets_settings.xml");
}
BOOL lggContactSetsFloaterSettings::postBuild(void)
{
	childSetAction("lgg_fg_okButton",onClickOk,this);
	getChild<LLColorSwatchCtrl>("colordefault")->setCommitCallback(onDefaultBackgroundChange);
	getChild<LLColorSwatchCtrl>("colordefault")->set(LGGContactSets::getInstance()->getDefaultColor());
	return TRUE;
}
void lggContactSetsFloaterSettings::onDefaultBackgroundChange(LLUICtrl* ctrl, void* userdata)
{
	LLColorSwatchCtrl* cctrl = (LLColorSwatchCtrl*)ctrl;
	if(cctrl)
	{
		LGGContactSets::getInstance()->setDefaultColor(cctrl->get());
	}
}
void lggContactSetsFloaterSettings::onClickOk(void* data)
{
	sSettingsInstance->close();
}
void lggContactSetsFloaterStart::show(BOOL showin,void * data)
{
	if(showin)
	{
		lggContactSetsFloater* fg_floater = lggContactSetsFloater::showInstance();
		fg_floater->setData(data);
	}
}
void lggContactSetsFloaterStart::showSettings(BOOL showin)
{
	if(showin)
	{
		/*lggContactSetsFloaterSettings* settings_floater =*/ lggContactSetsFloaterSettings::showInstance();
	}
}