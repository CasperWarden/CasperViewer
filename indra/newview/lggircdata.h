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
#ifndef  LGGIRCDATA
#define LGGIRCDATA

#include "llviewerprecompiledheaders.h"

#include "llfile.h"
#include "llsdserialize.h"
class lggIrcData
{
	public:
		lggIrcData(std::string iserver,
		std::string iname,
		std::string iport,
		std::string inick,
		std::string ichannel,
		std::string iNickPassword,
		std::string iChannelpassword,
		std::string iServerPassword,
		BOOL iautoLogin,
		LLUUID iid);
		lggIrcData();

		~lggIrcData();
		std::string server;
		std::string name;
		std::string port;
		std::string nick;
		std::string channel;
		std::string serverPassword;
		std::string nickPassword;
		std::string channelPassword;
		BOOL autoLogin;
		LLUUID id;
	public:
		LLSD toLLSD();
		static lggIrcData fromLLSD(LLSD inputData);
		//static lggIrcData newDefault();
		std::string toString();
		// List sorted by name.
		friend bool operator==(const lggIrcData &a, const lggIrcData &b);		// Return a == b
		friend bool operator<(const lggIrcData &a, const lggIrcData &b);		// Return a < b	
};
inline bool operator==(const lggIrcData &a, const lggIrcData &b)
{
	return (  a.id == b.id);
}
inline bool operator<(const lggIrcData &a, const lggIrcData &b)
{
	return (  a.name < b.name);
}


#endif
