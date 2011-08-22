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

#ifndef LL_LLFLOATERIRC_H
#define LL_LLFLOATERIRC_H

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class llfloaterirc
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "lluuid.h"
#include "llfloater.h"
#include <map>

class LLUICtrl;
class LLTextBox;
class LLScrollListCtrl;
class LLButton;

class lggPanelIRC : public LLPanel, public LLSimpleListener
{
public:
	lggPanelIRC();
	virtual ~lggPanelIRC();

	void newList();

protected:
	// initialize based on the type
	BOOL postBuild();
	
	/*virtual*/ bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata);
	// highlight_id is a group id to highlight
	void enableButtons();

	static void onIrcList(LLUICtrl* ctrl, void* userdata);
	static void onBtnNewIrc(void* userdata);
	static void onBtnEdit(void* userdata);
	static void onBtnRefresh(void* userdata);
	static void onBtnIM(void* userdata);
	static void onBtnRemove(void* userdata);
	static void onDoubleClickGroup(void* userdata);
	static void onClickHelp(void* data);

	void newirc();
	void editirc();
	void startirc();
	void startIM();
	void removeirc();
	void initHelpBtn(const std::string& name, const std::string& xml_alert);

	
};


#endif // LL_LLFLOATERIRCS_H
