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

////////////////////////////////////////////////////
//////////////DATA TYPE/////////////////////////////

#include "llviewerprecompiledheaders.h"
#include "lggircdata.h"
#include "llviewercontrol.h"
#include "llviewerobject.h"
#include "llagent.h"
#include "llvoavatar.h"

lggIrcData lggIrcData::fromLLSD(LLSD inputData)
{
	
	lggIrcData toReturn;
	if(inputData.has("ircserver")) toReturn.server = inputData["ircserver"].asString();
	if(inputData.has("ircname")) toReturn.name = inputData["ircname"].asString();
	if(inputData.has("ircport")) toReturn.port = inputData["ircport"].asString();
	if(inputData.has("ircnick")) toReturn.nick = inputData["ircnick"].asString();
	if(inputData.has("ircchannel")) toReturn.channel = inputData["ircchannel"].asString();
	if(inputData.has("ircnickpassword")) toReturn.nickPassword = inputData["ircnickpassword"].asString();
	if(inputData.has("ircchannelpassword")) toReturn.channelPassword = inputData["ircchannelpassword"].asString();
	if(inputData.has("ircserverpassword")) toReturn.serverPassword = inputData["ircserverpassword"].asString();
	if(inputData.has("ircautologin")) toReturn.autoLogin = inputData["ircautologin"].asBoolean();
	if(inputData.has("ircid")) toReturn.id = LLUUID(inputData["ircid"].asString());
	//support for legacy format
	if(inputData.has("ircpassword")) toReturn.nickPassword = inputData["ircpassword"].asString();

	return toReturn;


}
LLSD lggIrcData::toLLSD()
{
	LLSD out;
	out["ircchannel"]=channel;
	out["ircport"]=port;
	out["ircid"]=id.asString();
	out["ircnick"]=nick;
	out["ircnickpassword"]=nickPassword;
	out["ircserver"]=server;
	out["ircserverpassword"]=serverPassword;
	out["ircchannelpassword"]=channelPassword;
	out["ircautologin"]=autoLogin;
	out["ircname"]=name;
	return out;
}

std::string lggIrcData::toString()
{
	
	return llformat("Name is %s\nNick is %s\nChannel is %s\nUUID is %s\nServer is %s",
		name.c_str(),nick.c_str(),channel.c_str(),id.asString().c_str(),server.c_str());
}
lggIrcData::lggIrcData(std::string iserver, std::string iname, std::string iport,
					   std::string inick, std::string ichannel, std::string iNickPassword,
					   std::string iChannelpassword, std::string iServerPassword,
					   BOOL iautoLogin, LLUUID iid)
  : server(iserver),
	name(iname),
	port(iport),
	nick(inick),
	channel(ichannel),
	serverPassword(iServerPassword),
	channelPassword(iChannelpassword),
	nickPassword(iNickPassword),
	autoLogin(iautoLogin),
	id(iid)
{
}

lggIrcData::lggIrcData()
  : server("irc.freenode.org"),
  name("Phoenix Chat"),
  port("6667"),
  channel("#phoenixviewer"),
  serverPassword(""),
  channelPassword(""),
  nickPassword(""),
  autoLogin(TRUE),
  id(LLUUID::generateNewID())
{
	std::string first("Phoenix");
	// Prevents crash at startup where viewer object and NVPair may not exist yet.
	// In the event that either of these happen. Name is replaced with "Phoenix"
	if(gAgent.getAvatarObject())
	{
		LLNameValue* first_name = gAgent.getAvatarObject()->getNVPair("FirstName");
		// Ahh yes... nametag hacks. Check type!
		if(first_name && (first_name->getTypeEnum() == NVT_STRING))
		{
			first = first_name->getString();
		}
	}
	nick = std::string(first+LLUUID::generateNewID().asString().substr(33));
}

lggIrcData::~lggIrcData()
{
	
}
