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

#include "lggircprofilefloater.h"

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
#include "llpanelphoenix.h"
#include "lggbeamscolors.h"
#include "llsliderctrl.h"
#include "llfocusmgr.h"
#include "llsd.h"
#include "llimview.h"
#include "lllineeditor.h"


class lggIrcProfileFloater;

class lggIrcProfileFloater : public LLFloater, public LLFloaterSingleton<lggIrcProfileFloater>
{
public:
	lggIrcProfileFloater(const LLSD& seed);
	virtual ~lggIrcProfileFloater();	
	BOOL postBuild(void);

	void setData(LLSD idata);

	static void onClickIM(void* data);
	static void onClickClose(void* data);

protected:
	LLSD myLLSDdata;
};

lggIrcProfileFloater::~lggIrcProfileFloater()
{

}
void lggIrcProfileFloater::setData(LLSD idata)
{
	myLLSDdata=idata;

	getChild<LLLineEditor>("PhIRCProfile_WhoNick")->setValue(myLLSDdata["NICK"].asString());
	getChild<LLLineEditor>("PhIRCProfile_WhoUser")->setValue(myLLSDdata["USER"].asString());
	getChild<LLLineEditor>("PhIRCProfile_WhoHost")->setValue(myLLSDdata["HOST"].asString());
	//childSetValue("PhIRCtxt_name",myLLSDdata["REALNAME"].asString());
	//childSetText("PhIRCtxt_channels",myLLSDdata["CHANNELS"].asString());
	getChild<LLLineEditor>("PhIRCProfile_WhoName")->setValue(myLLSDdata["REALNAME"].asString());

	getChild<LLLineEditor>("PhIRCProfile_WhoChanel")->setValue(myLLSDdata["CHANNELS"].asString());
	getChild<LLLineEditor>("PhIRCProfile_WhoServer")->setValue(myLLSDdata["SERVERS"].asString());
	getChild<LLLineEditor>("PhIRCProfile_WhoIdle")->setValue(myLLSDdata["IDLE"].asString());

	setTitle("IRC Profile - "+myLLSDdata["NICK"].asString());
}

lggIrcProfileFloater::lggIrcProfileFloater(const LLSD& seed)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_IRCprofile.xml");
	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}

}

BOOL lggIrcProfileFloater::postBuild(void)
{
	//setCanMinimize(false);
	childSetAction("PhoenixIRC_Profile_IM",onClickIM,this);
	childSetAction("PhoenixIRC_Profile_Close",onClickClose,this);
	
	

// 	childSetValue("PhIRCtxt_nick",myLLSDdata["NICK"]);
// 	childSetValue("PhIRCtxt_user",myLLSDdata["USER"]);
// 	childSetValue("PhIRCtxt_host",myLLSDdata["HOST"]);
// 	childSetValue("PhIRCtxt_name",myLLSDdata["REALNAME"]);
// 	childSetValue("PhIRCtxt_channels",myLLSDdata["CHANNELS"]);
// 	childSetValue("PhIRCtxt_server",myLLSDdata["SERVERS"]);
// 	childSetValue("PhIRCtxt_idle",myLLSDdata["IDLE"]);
	// 
	// 	args["NICK"] = nick;
	// 	args["USER"] = user;
	// 	args["HOST"] = host;
	// 	args["REALNAME"] = realName;
	// 	args["CHANNELS"] = channels;
	// 	args["SERVERS"] = servers;
	// 	args["IDLE"] = idle;
	// 	args["RCHANNEL"]= REALChannel;

	return true;
}
void lggIrcProfileFloater::onClickIM(void* data)
{
	lggIrcProfileFloater* self = (lggIrcProfileFloater*)data;

	LLUUID uid;
	uid.generate(self->myLLSDdata["NICK"].asString()+"lgg"+self->myLLSDdata["RCHANNEL"].asString());

	LLUUID computed_session_id=LLIMMgr::computeSessionID(IM_PRIVATE_IRC,uid);
			
	if(!gIMMgr->hasSession(computed_session_id))
	{
		make_ui_sound("UISndNewIncomingIMSession");
		gIMMgr->addSession(
		llformat("%s",self->myLLSDdata["NICK"].asString().c_str()),IM_PRIVATE_IRC,uid);
	}else
	{

		
	}
	
	self->close();
}

void lggIrcProfileFloater::onClickClose(void* data)
{
	lggIrcProfileFloater* self = (lggIrcProfileFloater*)data;
	self->close();
}
void LggIrcProfile::show(LLSD data)
{
	lggIrcProfileFloater* beam_floater = lggIrcProfileFloater::showInstance();
	beam_floater->setData(data);
}

