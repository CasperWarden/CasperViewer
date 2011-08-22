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
#ifndef IRC_THREAD_IRC22
#define IRC_THREAD_IRC22


#include "llthread.h"
#include "lggircdata.h"
#include "llviewerprecompiledheaders.h"
#include "irc.h"
#include "llimpanel.h"
#include "llchat.h"

class MsgListener : public LLThread
{
public:
	MsgListener(IRC * conn2, std::string chan2, LLUUID mid2);
	~MsgListener();	
	/*virtual*/ void run(void);
	/*virtual*/ void shutdown(void);
	
protected:
	std::string chan;
	IRC * conn;
	LLUUID mid;

};

class whoisresponce
{
public:
	std::string nick;
	std::string user;
	std::string host;
	std::string realName;
	std::string channels;
	std::string servers;
	std::string idle;
	std::string away;
		
	std::string REALChannel;


	whoisresponce(){user=channels=servers=idle=away=nick=host=realName="";}
	void newOne(){user=channels=servers=idle=away=nick=host=realName="";}
	void done();//need to make this one
	static void callbackProfile(const LLSD& notification, const LLSD& response);
	
};

class lggIrcThread
{
public:
	lggIrcThread(lggIrcData data);
	~lggIrcThread();
	
	MsgListener * listener;
	whoisresponce whoR;
	LLChat chat;

	LLFloaterIMPanel* floater;

	std::vector<LLUUID> getParticipants();
	void displayPrivateIm(std::string msg, std::string name);
	void sendPrivateImToID(std::string msg, LLUUID id);
	void run(void);
	void join();
	void notifyStuff();
	void stopRun(void);
	void whois(std::string user);
	void whois(LLUUID who);
	void setData(lggIrcData dat);
	lggIrcData getData() const;
	void actionDisp(std::string name, std::string msg);
	void msg(std::string message); 
	//void msg(std::string message, std::string name, bool notify);
	void msg(std::string message, LLColor4 color, bool notify);
	void msg(std::string message, std::string name, LLColor4 color,bool notify);
	void sendChat(std::string chat);
	void updateNames();
	int ircresponce(char* params, irc_reply_data* hostd, void* conn);
	std::string makelower(std::string str);
	int PrivMessageResponce( char * params, irc_reply_data * hostd, void * conn);
	int NoticeMessageResponce( char * params, irc_reply_data * hostd, void * conn);
	int channelJoinedResponce( char * params, irc_reply_data * hostd, void * conn);
	int JoinMessageResponce( char * params, irc_reply_data * hostd, void * conn);
	int PartMessageResponce( char * params, irc_reply_data * hostd, void * conn);
	int QuitMessageResponce( char * params, irc_reply_data * hostd, void * conn);
	int NickMessageResponce( char * params, irc_reply_data * hostd, void * conn);
	int ModeMessageResponce( char * params, irc_reply_data * hostd, void * conn);
	int KickMessageResponce( char * params, irc_reply_data * hostd, void * conn);

	int RPL_WHOISUSER( char * params, irc_reply_data * hostd, void * conn);
	int RPL_WHOISCHANNELS( char * params, irc_reply_data * hostd, void * conn);
	int RPL_WHOISSERVER( char * params, irc_reply_data * hostd, void * conn);
	int RPL_AWAY( char * params, irc_reply_data * hostd, void * conn);
	int RPL_WHOISIDLE( char * params, irc_reply_data * hostd, void * conn);
	int RPL_ENDOFWHOIS( char * params, irc_reply_data * hostd, void * conn);
	
	std::string stripColorCodes(std::string input);
	std::string getChannel();
	LLUUID getMID();
	static lggIrcThread* findInstance(IRC* conn);
	friend bool operator==(const lggIrcThread &a, const lggIrcThread &b);		// Return a == b
	friend bool operator!=(const lggIrcThread &a, const lggIrcThread &b);		// Return a == b
	friend bool operator<(const lggIrcThread &a, const lggIrcThread &b);		// Return a < b	
	friend bool operator>(const lggIrcThread &a, const lggIrcThread &b);

protected:
	lggIrcData mData; // User data associated with this thread
	IRC * conn;
	BOOL dataGotten;
private:
	static std::map<IRC*,lggIrcThread*> sInstances;
	static std::map<MsgListener*,lggIrcThread*> msInstances;

};

inline bool operator==(const lggIrcThread &a, const lggIrcThread &b)
{
	return (  a.getData().id == b.getData().id);
}

inline bool operator!=(const lggIrcThread &a, const lggIrcThread &b)
{
	return ( !(a==b) );
}
inline bool operator<(const lggIrcThread &a, const lggIrcThread &b)
{
	return (  a.getData().name < b.getData().name);
}
inline bool operator>(const lggIrcThread &a, const lggIrcThread &b)
{
	return (  a.getData().name > b.getData().name);
}
#endif
