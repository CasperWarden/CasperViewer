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
 

#ifndef GROUP_HANDLER_IRC22
#define GROUP_HANDLER_IRC22

//using namespace std;

#include "llviewerprecompiledheaders.h"
#include "lggircthread.h"
#include "lggircdata.h"
#include "lggfloaterirc.h"

class lggIrcGroupHandler
{
	public:

		lggIrcGroupHandler() {}
		~lggIrcGroupHandler() { }
	public:
		lggPanelIRC * listPanel;
		void setListPanel(lggPanelIRC * ilistPanel);
		void fillListPanel();
		std::list<lggIrcThread*> activeThreads;
		std::vector<lggIrcData> getFileNames();
		lggIrcData	getIrcGroupInfo(std::string filename);
		lggIrcData	getIrcGroupInfoByID(LLUUID id); 
		void deleteIrcGroup(std::string filename);		
		void deleteIrcGroupByID(LLUUID id);
		void startUpAutoRunIRC();
		BOOL sendWhoisToAll(LLUUID who);
		void startUpIRCListener(lggIrcData dat);
		void endDownIRCListener(LLUUID id);
		lggIrcThread* getThreadByID(LLUUID id);
		void sendIrcChatByID(LLUUID id, std::string msg);
		bool trySendPrivateImToID(std::string msg, LLUUID id,BOOL testingForExistance);

		
};



extern lggIrcGroupHandler glggIrcGroupHandler;

#endif

