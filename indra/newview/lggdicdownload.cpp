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

#include "lggdicdownload.h"

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

class lggDicDownloadFloater;
class PhoenixDicDownloader : public LLHTTPClient::Responder
{
public:
	PhoenixDicDownloader(lggDicDownloadFloater * spanel,std::string sname);
	~PhoenixDicDownloader() { }
	void completedRaw(
		U32 status,
		const std::string& reason,
		const LLChannelDescriptors& channels,
		const LLIOPipe::buffer_ptr_t& buffer);
private:
	lggDicDownloadFloater* panel;
	std::string name;
};


class lggDicDownloadFloater : public LLFloater, public LLFloaterSingleton<lggDicDownloadFloater>
{
public:
	lggDicDownloadFloater(const LLSD& seed);
	virtual ~lggDicDownloadFloater();
	BOOL postBuild(void);
	void setData(std::vector<std::string> shortNames, std::vector<std::string> longNames, void * data);
	static void onClickDownload(void* data);
	std::vector<std::string> sNames;
	std::vector<std::string> lNames;
	LLPanelPhoenix * empanel;
};
lggDicDownloadFloater::~lggDicDownloadFloater()
{
}
lggDicDownloadFloater::lggDicDownloadFloater(const LLSD& seed)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_dictionaries.xml");

	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}

}

BOOL lggDicDownloadFloater::postBuild(void)
{
	childSetAction("Phoenix_dic_download",onClickDownload,this);
	return true;
}
void lggDicDownloadFloater::setData(std::vector<std::string> shortNames, std::vector<std::string> longNames, void * data)
{
	sNames=shortNames;
	lNames=longNames;
	empanel = (LLPanelPhoenix*)data;

	LLComboBox* comboBox = getChild<LLComboBox>("Phoenix_combo_dics");
	if(comboBox != NULL) 
	{
		comboBox->removeall();
		comboBox->add("");
		for(int i = 0; i < (int)lNames.size(); i++)
		{
			comboBox->add(lNames[i]);
		}
		comboBox->setCurrentByIndex(0);
	}
}
void lggDicDownloadFloater::onClickDownload(void* data)
{
	lggDicDownloadFloater* self = (lggDicDownloadFloater*)data;
	if(self)
	{
		//std::string selection = self->childGetValue("Phoenix_combo_dics").asString();
		LLComboBox* comboBox = self->getChild<LLComboBox>("Phoenix_combo_dics");
		if(comboBox != NULL) 
		{
			int index = comboBox->getCurrentIndex();
			if(index!=0)
			{
				index--;
				std::string newDict(self->sNames[index]);
				LLHTTPClient::get("http://phoenixviewer.com/app/dics/"+newDict+".aff", new PhoenixDicDownloader(self,newDict+".aff"));
				LLHTTPClient::get("http://phoenixviewer.com/app/dics/"+newDict+".dic", new PhoenixDicDownloader(NULL,newDict+".dic"));
				
				LLButton* butt = self->getChild<LLButton>("Phoenix_dic_download");
				if(butt)
				{
					butt->setLabel(LLStringExplicit("Downloading... Please Wait"));
					butt->setEnabled(FALSE);
				}

			}
		}
	}
	
 
}

void LggDicDownload::show(BOOL showin, std::vector<std::string> shortNames, std::vector<std::string> longNames, void * data)
{
	if(showin)
	{
		lggDicDownloadFloater* dic_floater = lggDicDownloadFloater::showInstance();
		dic_floater->setData(shortNames,longNames,data);
	}
}
PhoenixDicDownloader::PhoenixDicDownloader(lggDicDownloadFloater* spanel, std::string sname):
panel(spanel),name(sname){}


void PhoenixDicDownloader::completedRaw(U32 status, const std::string& reason, const LLChannelDescriptors& channels, const LLIOPipe::buffer_ptr_t& buffer)
{
	if(status < 200 || status >= 300)
	{
		return;
	}
	LLBufferStream istr(channels, buffer.get());
	std::string dicpath(gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "dictionaries", 
		name.c_str()));
	
	
	llofstream ostr(dicpath, std::ios::binary);

	while(istr.good() && ostr.good())
		ostr << istr.rdbuf();
	ostr.close();
	if(panel)
	{
		panel->empanel->refresh();
		panel->close();
	}
	
	
}
