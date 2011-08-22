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
#include "lggfloaterircedit.h"
#include "lggfloaterirc.h"

#include "llagentdata.h"
#include "llcommandhandler.h"
#include "llfloater.h"
#include "llsdutil.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llagent.h"
#include "llfilepicker.h"
#include "llpanel.h"
#include "lliconctrl.h"
#include "llbutton.h"
#include "llcolorswatch.h"
#include "lggbeammaps.h"

#include "llsdserialize.h"

class lggFloaterIrcEdit;

////////////////////////////////////////////////////////////////////////////
// lggBeamMapFloater
class lggFloaterIrcEdit : public LLFloater, public LLFloaterSingleton<lggFloaterIrcEdit>
{
public:
	lggPanelIRC* caller;
	lggFloaterIrcEdit(const LLSD& seed);
	virtual ~lggFloaterIrcEdit();
	

	BOOL postBuild(void);
	void update(lggIrcData dat,void* data);
	
	void draw();
	//void showInstance(lggIrcData dat);
	
	// UI Handlers
	static void onClickSave(void* data);
	//static void onClickClear(void* data);
	static void onClickCancel(void* data);

	
private:
	static void onBackgroundChange(LLUICtrl* ctrl, void* userdata);
	static void onClickHelp(void* data);
	void initHelpBtn(const std::string& name, const std::string& xml_alert);
	
};
//void lggFloaderIrcEdit::showInstance(lggIrcData dat)
//{
//	LLFloater::showInstance();
//}
void lggFloaterIrcEdit::draw()
{
	LLFloater::draw();
}

lggFloaterIrcEdit::~lggFloaterIrcEdit()
{
	//if(mCallback) mCallback->detach();
}

lggFloaterIrcEdit::lggFloaterIrcEdit(const LLSD& seed)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_IRCinfo.xml");
	
	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}

}
void lggFloaterIrcEdit::initHelpBtn(const std::string& name, const std::string& xml_alert)
{
	childSetAction(name, onClickHelp, new std::string(xml_alert));
}
void lggFloaterIrcEdit::onClickHelp(void* data)
{
	std::string* xml_alert = (std::string*)data;
	LLNotifications::instance().add(*xml_alert);
}
BOOL lggFloaterIrcEdit::postBuild(void)
{
	//setCanMinimize(false);
	childSetAction("PhoenixIRC_save",onClickSave,this);
	childSetAction("PhoenixIRC_cancel",onClickCancel,this);

	initHelpBtn("PhoenixIRC_Help",	"PhoenixHelp_IRCSettings");

	
	return true;
}
void lggFloaterIrcEdit::update(lggIrcData dat, void* data)
{
	caller = (lggPanelIRC*)data;
	childSetValue("PhoenixIRC_nick",dat.nick);
	childSetValue("PhoenixIRC_server",dat.server);
	childSetValue("PhoenixIRC_password",dat.nickPassword);
	childSetValue("PhoenixIRC_ServerPassword",dat.serverPassword);
	childSetValue("PhoenixIRC_ChanPassword",dat.channelPassword);
	childSetValue("PhoenixIRC_channel",dat.channel);
	childSetValue("PhoenixIRC_tag",dat.name);
	childSetValue("PhoenixIRC_port",dat.port);

	childSetValue("PhoenixIRC_AutoConnect",dat.autoLogin);

}
void lggFloaterIrcEdit::onClickSave(void* data)
{
	llinfos << "lggPanelIRCedit::save" << llendl;
	
	lggFloaterIrcEdit* self = (lggFloaterIrcEdit*)data;
	//LLFilePicker& picker = LLFilePicker::instance();
	lggIrcData dat(
	self->childGetValue("PhoenixIRC_server"),	
	self->childGetValue("PhoenixIRC_tag"),
	self->childGetValue("PhoenixIRC_port"),
	self->childGetValue("PhoenixIRC_nick"),	
	self->childGetValue("PhoenixIRC_channel"),
	self->childGetValue("PhoenixIRC_password"),
	self->childGetValue("PhoenixIRC_ChanPassword"),
	self->childGetValue("PhoenixIRC_ServerPassword"),
	self->childGetValue("PhoenixIRC_AutoConnect").asBoolean(),
	LLUUID::generateNewID());

	std::string path_name2(gDirUtilp->getExpandedFilename( LL_PATH_PER_SL_ACCOUNT , "IRCGroups", ""));
				
	std::string filename=path_name2 + dat.name+".xml";
		
	
	
	LLSD main=dat.toLLSD();
	llofstream export_file;
	export_file.open(filename);
	LLSDSerialize::toPrettyXML(main, export_file);
	export_file.close();
	//lggPanelIRC* instance = (lggPanelIRC*)caller;	if(instance)	instance.refresh();
	
	//gSavedSettings.setString("PhoenixBeamShape",gDirUtilp->getBaseFileName(filename,true));
	if(self->caller)
	{
		self->caller->newList();
	}
	
	self->close();
}

void lggFloaterIrcEdit::onClickCancel(void* data)
{
	lggFloaterIrcEdit* self = (lggFloaterIrcEdit*)data;
	self->close();
	
}
void LggIrcFloaterStarter::show(lggIrcData dat,void* data)
{
	
	lggFloaterIrcEdit* floater = lggFloaterIrcEdit::showInstance();
	floater->update(dat, data);
	
	//beam_floater->update();
}


