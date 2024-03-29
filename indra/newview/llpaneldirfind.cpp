/**
 * @file llpaneldirfind.cpp
 * @brief The "All" panel in the Search directory.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llpaneldirfind.h"

// linden library includes
#include "llclassifiedflags.h"
#include "llfontgl.h"
#include "llparcel.h"
#include "llqueryflags.h"
#include "message.h"

// viewer project includes
#include "llagent.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "lllineeditor.h"
#include "llcombobox.h"
#include "llviewercontrol.h"
#include "llmenucommands.h"
#include "llmenugl.h"
#include "llpluginclassmedia.h"
#include "lltextbox.h"
#include "lluiconstants.h"
#include "llviewerimagelist.h"
#include "llviewermessage.h"
#include "llfloateravatarinfo.h"
#include "lldir.h"
#include "llviewercontrol.h"
#include "llviewerregion.h"		// for region name for search urls
#include "lluictrlfactory.h"
#include "llfloaterdirectory.h"
#include "llpaneldirbrowser.h"

#if LL_MSVC
// disable boost::lexical_cast warning
#pragma warning (disable:4702)
#endif
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

//---------------------------------------------------------------------------
// LLPanelDirFindAll - Google search appliance based search
//---------------------------------------------------------------------------

class LLPanelDirFindAll
:	public LLPanelDirFind
{
public:
	LLPanelDirFindAll(const std::string& name, LLFloaterDirectory* floater);

	/*virtual*/ void reshape(S32 width, S32 height, BOOL called_from_parent);
	/*virtual*/ void search(const std::string& search_text);
};

LLPanelDirFindAll::LLPanelDirFindAll(const std::string& name, LLFloaterDirectory* floater)
:	LLPanelDirFind(name, floater, "find_browser")
{
}

//---------------------------------------------------------------------------
// LLPanelDirFind - Base class for all browser-based search tabs
//---------------------------------------------------------------------------

LLPanelDirFind::LLPanelDirFind(const std::string& name, LLFloaterDirectory* floater, const std::string& browser_name)
:	LLPanelDirBrowser(name, floater),
	mWebBrowser(NULL),
	mBrowserName(browser_name)
{
}

BOOL LLPanelDirFind::postBuild()
{
	LLPanelDirBrowser::postBuild();

	childSetAction("back_btn", onClickBack, this);
	childSetAction("home_btn", onClickHome, this);
	childSetAction("forward_btn", onClickForward, this);
	childSetCommitCallback("search_editor", onCommitSearch, this);
	childSetAction("search_btn", onClickSearch, this);
	childSetAction("?", onClickHelp, this);

	// showcase doesn't have maturity flags -- it's all PG
	if (hasChild("incmature"))
	{
		// Teens don't get mature checkbox
		if (gAgent.wantsPGOnly())
		{
			childSetValue("incmature", FALSE);
			childSetValue("incadult", FALSE);
			childHide("incmature");
			childHide("incadult");
			childSetValue("incpg", TRUE);
			childDisable("incpg");
		}		
		
		if (!gAgent.canAccessMature())
		{
			childSetValue("incmature", FALSE);
			childDisable("incmature");
		}
		
		if (!gAgent.canAccessAdult())
		{
			childSetValue("incadult", FALSE);
			childDisable("incadult");
		}
	}
	
	
	mWebBrowser = getChild<LLMediaCtrl>(mBrowserName);
	if (mWebBrowser)
	{
		mWebBrowser->addObserver(this);

		// need to handle secondlife:///app/ URLs for direct teleports
		mWebBrowser->setTrusted( true );

		// redirect 404 pages from S3 somewhere else
		mWebBrowser->set404RedirectUrl( getString("redirect_404_url") );

		navigateToDefaultPage();
	}

	return TRUE;
}

LLPanelDirFind::~LLPanelDirFind()
{
}

// virtual
void LLPanelDirFind::draw()
{
	// enable/disable buttons depending on state
	if ( mWebBrowser )
	{
		bool enable_back = mWebBrowser->canNavigateBack();	
		childSetEnabled( "back_btn", enable_back );

		bool enable_forward = mWebBrowser->canNavigateForward();	
		childSetEnabled( "forward_btn", enable_forward );
	}

	// showcase doesn't have maturity flags -- it's all PG
	if (hasChild("incmature"))
	{
		updateMaturityCheckbox();
	}

	LLPanelDirBrowser::draw();
}

// When we show any browser-based view, we want to hide all
// the right-side XUI detail panels.
// virtual
void LLPanelDirFind::onVisibilityChange(BOOL new_visibility)
{
	if (new_visibility)
	{
		mFloaterDirectory->hideAllDetailPanels();
	}
	LLPanel::onVisibilityChange(new_visibility);
}

// virtual
void LLPanelDirFindAll::reshape(S32 width, S32 height, BOOL called_from_parent = TRUE)
{
	if ( mWebBrowser )
		mWebBrowser->navigateTo( mWebBrowser->getCurrentNavUrl() );

	LLUICtrl::reshape( width, height, called_from_parent );
}

void LLPanelDirFindAll::search(const std::string& search_text)
{
	BOOL inc_pg = childGetValue("incpg").asBoolean();
	BOOL inc_mature = childGetValue("incmature").asBoolean();
	BOOL inc_adult = childGetValue("incadult").asBoolean();
	if (!(inc_pg || inc_mature || inc_adult))
	{
		LLNotifications::instance().add("NoContentToSearch");
		return;
	}
	
	if (!search_text.empty())
	{
		std::string selected_collection = childGetValue( "Category" ).asString();
		std::string url = buildSearchURL(search_text, selected_collection, inc_pg, inc_mature, inc_adult);
		if (mWebBrowser)
		{
			mWebBrowser->navigateTo(url);
		}
	}
	else
	{
		// empty search text
		navigateToDefaultPage();
	}

	childSetText("search_editor", search_text);
}

void LLPanelDirFind::focus()
{
	childSetFocus("search_editor");
}

void LLPanelDirFind::navigateToDefaultPage()
{
	std::string start_url = getString("default_search_page");
	BOOL inc_pg = childGetValue("incpg").asBoolean();
	BOOL inc_mature = childGetValue("incmature").asBoolean();
	BOOL inc_adult = childGetValue("incadult").asBoolean();
	if (!(inc_pg || inc_mature || inc_adult))
	{
		// if nothing's checked, just go for pg; we don't notify in
		// this case because it's a default page.
		inc_pg = true;
	}
	
	start_url += getSearchURLSuffix(inc_pg, inc_mature, inc_adult);

	llinfos << "default url: "  << start_url << llendl;

	if (mWebBrowser)
	{
		mWebBrowser->navigateTo( start_url );
	}
}
// static
std::string LLPanelDirFind::buildSearchURL(const std::string& search_text, const std::string& collection, 
										   bool inc_pg, bool inc_mature, bool inc_adult)
{
	std::string url = gSavedSettings.getString("SearchURLDefault");
	if (!search_text.empty())
	{
		// Replace spaces with "+" for use by Google search appliance
		// Yes, this actually works for double-spaces
		// " foo  bar" becomes "+foo++bar" and works fine. JC
		std::string search_text_with_plus = search_text;
		std::string::iterator it = search_text_with_plus.begin();
		for ( ; it != search_text_with_plus.end(); ++it )
		{
			if ( std::isspace( *it ) )
			{
				*it = '+';
			}
		}

		// Our own special set of allowed chars (RFC1738 http://www.ietf.org/rfc/rfc1738.txt)
		// Note that "+" is one of them, so we can do "+" addition first.
		const char* allowed =   
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
			"0123456789"
			"-._~$+!*'()";
		std::string query = LLURI::escape(search_text_with_plus, allowed);

		url = gSavedSettings.getString("SearchURLQuery");
		std::string substring = "[QUERY]";
		std::string::size_type where = url.find(substring);
		if (where != std::string::npos)
		{
			url.replace(where, substring.length(), query);
		}

		// replace the collection name with the one selected from the combo box
		// std::string selected_collection = childGetValue( "Category" ).asString();
		substring = "[COLLECTION]";
		where = url.find(substring);
		if (where != std::string::npos)
		{
			url.replace(where, substring.length(), collection);
		}

	}
	url += getSearchURLSuffix(inc_pg, inc_mature, inc_adult);
	llinfos << "search url " << url << llendl;
	return url;
}
// static
std::string LLPanelDirFind::getSearchURLSuffix(bool inc_pg, bool inc_mature, bool inc_adult)
{
	std::string url = gSavedSettings.getString("SearchURLSuffix2");

	// if the mature checkbox is unchecked, modify query to remove 
	// terms with given phrase from the result set
	// This builds a value from 1-7 by or-ing together the flags, and then converts
	// it to a string. 
	std::string substring="[MATURITY]";
	S32 maturityFlag = 
		(inc_pg ? SEARCH_PG : SEARCH_NONE) |
		(inc_mature ? SEARCH_MATURE : SEARCH_NONE) |
		(inc_adult ? SEARCH_ADULT : SEARCH_NONE);
	url.replace(url.find(substring), substring.length(), boost::lexical_cast<std::string>(maturityFlag));
	
	// Include region and x/y position, not for the GSA, but
	// just to get logs on the web server for search_proxy.php
	// showing where people were standing when they searched.
	std::string region_name;
	LLViewerRegion* region = gAgent.getRegion();
	if (region)
	{
		region_name = region->getName();
	}
	// take care of spaces in names
	region_name = LLURI::escape(region_name);
	substring = "[REGION]";
	url.replace(url.find(substring), substring.length(), region_name);

	LLVector3 pos_region = gAgent.getPositionAgent();

	std::string x = llformat("%.0f", pos_region.mV[VX]);
	substring = "[X]";
	url.replace(url.find(substring), substring.length(), x);
	std::string y = llformat("%.0f", pos_region.mV[VY]);
	substring = "[Y]";
	url.replace(url.find(substring), substring.length(), y);
	std::string z = llformat("%.0f", pos_region.mV[VZ]);
	substring = "[Z]";
	url.replace(url.find(substring), substring.length(), z);

	LLUUID session_id = gAgent.getSessionID();
	std::string session_string = session_id.getString();
	substring = "[SESSION]";
	url.replace(url.find(substring), substring.length(), session_string);

	// set the currently selected language by asking the pref setting
	std::string language_string = LLUI::getLanguage();
	std::string language_tag = "[LANG]";
	url.replace( url.find( language_tag ), language_tag.length(), language_string );

	// and set the flag for the teen grid
	std::string teen_string = gAgent.isTeen() ? "y" : "n";
	std::string teen_tag = "[TEEN]";
	url.replace( url.find( teen_tag ), teen_tag.length(), teen_string );	
	
	return url;
}


// static
void LLPanelDirFind::onClickBack( void* data )
{
	LLPanelDirFind* self = ( LLPanelDirFind* )data;
	if ( self->mWebBrowser )
	{
		self->mWebBrowser->navigateBack();
	}
}

// static
void LLPanelDirFind::onClickHelp( void* data )
{
	LLNotifications::instance().add("ClickSearchHelpAll");
}

// static
void LLPanelDirFind::onClickForward( void* data )
{
	LLPanelDirFind* self = ( LLPanelDirFind* )data;
	if ( self->mWebBrowser )
	{
		self->mWebBrowser->navigateForward();
	}
}

// static
void LLPanelDirFind::onClickHome( void* data )
{
	LLPanelDirFind* self = ( LLPanelDirFind* )data;
	if ( self->mWebBrowser )
	{
		self->mWebBrowser->navigateHome();
	}
}

// static
void LLPanelDirFind::onCommitSearch(LLUICtrl*, void* data)
{
	onClickSearch(data);
}

// static
void LLPanelDirFind::onClickSearch(void* data)
{
	LLPanelDirFind* self = ( LLPanelDirFind* )data;
	std::string search_text = self->childGetText("search_editor");
	self->search(search_text);

	LLFloaterDirectory::sNewSearchCount++;
}

void LLPanelDirFind::handleMediaEvent(LLPluginClassMedia* self, EMediaEvent event)
{
	switch(event)
	{
		case MEDIA_EVENT_NAVIGATE_BEGIN:
			childSetText("status_text", getString("loading_text"));
		break;
		
		case MEDIA_EVENT_NAVIGATE_COMPLETE:
			childSetText("status_text", getString("done_text"));
		break;
		
		case MEDIA_EVENT_LOCATION_CHANGED:
			// Debugging info to console
			llinfos << self->getLocation() << llendl;
		break;

		default:
			// Having a default case makes the compiler happy.
		break;
	}
}

//---------------------------------------------------------------------------
// LLPanelDirFindAllInterface
//---------------------------------------------------------------------------

// static
LLPanelDirFindAll* LLPanelDirFindAllInterface::create(LLFloaterDirectory* floater)
{
	return new LLPanelDirFindAll("find_all_panel", floater);
}

// static
void LLPanelDirFindAllInterface::search(LLPanelDirFindAll* panel,
										const std::string& search_text)
{
	panel->search(search_text);
}

// static
void LLPanelDirFindAllInterface::focus(LLPanelDirFindAll* panel)
{
	panel->focus();
}

