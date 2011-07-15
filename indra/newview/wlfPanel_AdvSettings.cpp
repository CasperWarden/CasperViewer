/* Copyright (c) 2010
 *
 * Modular Systems All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *   3. Neither the name Modular Systems nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MODULAR SYSTEMS AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MODULAR SYSTEMS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "llviewerprecompiledheaders.h"

#include "wlfPanel_AdvSettings.h"

#include "llbutton.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "lliconctrl.h"
#include "lloverlaybar.h"
#include "lltextbox.h"
#include "llcombobox.h"
#include "llsliderctrl.h"
#include "llstartup.h"

#include "llfloaterwindlight.h"
#include "llfloaterwater.h"

#include "lldaycyclemanager.h"
#include "llenvmanager.h"
#include "llwaterparammanager.h"
#include "llwlparamset.h"
#include "llwlparammanager.h"

// [RLVa:KB]
#include "rlvhandler.h"
// [/RLVa:KB]

BOOL firstBuildDone;
void* fixPointer;
std::string ButtonState;

wlfPanel_AdvSettings::wlfPanel_AdvSettings()
{
	setIsChrome(TRUE);
	setFocusRoot(TRUE);
	build();
}
void wlfPanel_AdvSettings::build()
{
	deleteAllChildren();
	if (!gSavedSettings.getBOOL("wlfAdvSettingsPopup"))
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, "wlfPanel_AdvSettings_expanded.xml", &getFactoryMap());
		ButtonState = "arrow_up.tga";
	}
	else
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, "wlfPanel_AdvSettings.xml", &getFactoryMap());
		ButtonState = "arrow_down.tga";
	}
}

void wlfPanel_AdvSettings::refresh()
{
// [RLVa:KB] - Checked: 2009-09-19
	if ( (rlv_handler_t::isEnabled()) && (gSavedSettings.getBOOL("wlfAdvSettingsPopup")) )
	{
		childSetEnabled("EnvAdvancedWaterButton", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		childSetEnabled("WWPresetsCombo", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		childSetEnabled("WWprev", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		childSetEnabled("WWnext", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		childSetEnabled("EnvAdvancedSkyButton", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		childSetEnabled("WLPresetsCombo", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		childSetEnabled("WLprev", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		childSetEnabled("WLnext", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
		childSetEnabled("EnvTimeSlider", !gRlvHandler.hasBehaviour(RLV_BHVR_SETENV));
	}
// [/RLVa:KB]
}

void wlfPanel_AdvSettings::refreshLists()
{
	LLEnvManagerNew& env_mgr = LLEnvManagerNew::instance();

	// Populate the combo boxes with appropriate lists of available presets.
	//actually, dont do this, its laggy and not needed to just refresh the selections
	// populateWaterPresetsList();
	// populateSkyPresetsList();
	// populateDayCyclePresetsList();

	// Select the current presets in combo boxes.
	mWaterPresetCombo->selectByValue(env_mgr.getWaterPresetName());
	mSkyPresetCombo->selectByValue(env_mgr.getSkyPresetName());
	//mDayCyclePresetCombo->selectByValue(env_mgr.getDayCycleName());
}

void wlfPanel_AdvSettings::fixPanel()
{
	if(!firstBuildDone)
	{
		llinfos << "firstbuild done" << llendl;
		firstBuildDone = TRUE;
		onClickExpandBtn(fixPointer);
	}
}

BOOL wlfPanel_AdvSettings::postBuild()
{
	childSetAction("expand", onClickExpandBtn, this);
	
	mWaterPresetCombo = getChild<LLComboBox>("WWPresetsCombo");
	mWaterPresetCombo->setCommitCallback(onChangeWWPresetName);
		
	mSkyPresetCombo = getChild<LLComboBox>("WLPresetsCombo");
	mSkyPresetCombo->setCommitCallback(onChangeWLPresetName);
	
	// mDayCyclePresetCombo = getChild<LLComboBox>("DCPresetsCombo");
	// mDayCyclePresetCombo->setCommitCallback(onChangeDCPresetName);
	
	LLEnvManagerNew::instance().setPreferencesChangeCallback(boost::bind(&wlfPanel_AdvSettings::refreshLists, this));
	LLWaterParamManager::instance()->setPresetListChangeCallback(boost::bind(&wlfPanel_AdvSettings::populateWaterPresetsList, this));
	LLWLParamManager::instance()->setPresetListChangeCallback(boost::bind(&wlfPanel_AdvSettings::populateSkyPresetsList, this));
	// LLDayCycleManager::instance().setModifyCallback(boost::bind(&wlfPanel_AdvSettings::populateDayCyclePresetsList, this));

	populateWaterPresetsList();
	populateSkyPresetsList();
	//populateDayCyclePresetsList();

	// next/prev buttons
	childSetAction("WWnext", onClickWWNext, this);
	childSetAction("WWprev", onClickWWPrev, this);
	childSetAction("WLnext", onClickWLNext, this);
	childSetAction("WLprev", onClickWLPrev, this);
	
	childSetAction("EnvAdvancedSkyButton", onOpenAdvancedSky, NULL);
	childSetAction("EnvAdvancedWaterButton", onOpenAdvancedWater, NULL);
	
	childSetCommitCallback("EnvTimeSlider", onChangeDayTime, NULL);
	
	fixPointer = this;
	return TRUE;
}

void wlfPanel_AdvSettings::draw()
{
	LLButton* expand_button = getChild<LLButton>("expand");
/*	if (expand_button)
	{
		if (expand_button->getToggleState())
		{
			expand_button->setImageOverlay("arrow_down.tga");
		}
		else
		{
			expand_button->setImageOverlay("arrow_up.tga");
		}
	} */
			expand_button->setImageOverlay(ButtonState);
	refresh();
	LLPanel::draw();
}

wlfPanel_AdvSettings::~wlfPanel_AdvSettings ()
{
}

void wlfPanel_AdvSettings::onClickExpandBtn(void* user_data)
{
	gSavedSettings.setBOOL("wlfAdvSettingsPopup",!gSavedSettings.getBOOL("wlfAdvSettingsPopup"));
	wlfPanel_AdvSettings* remotep = (wlfPanel_AdvSettings*)user_data;
	remotep->build();
	gOverlayBar->layoutButtons();
}

void wlfPanel_AdvSettings::onChangeWWPresetName(LLUICtrl* ctrl, void * userData)
{
	LLComboBox * combo_box = static_cast<LLComboBox*>(ctrl);
	
	if(combo_box->getSimple() == "")
	{
		return;
	}

	const std::string& wwset = combo_box->getSelectedValue().asString();
	if (LLWaterParamManager::instance()->hasParamSet(wwset))
	{
		LLEnvManagerNew::instance().setUseWaterPreset(wwset, true);
	}
	else
	{
		//if that failed, use region's
		// LLEnvManagerNew::instance().useRegionWater();
		LLEnvManagerNew::instance().setUseWaterPreset("Default", true);
	}
}

void wlfPanel_AdvSettings::onChangeWLPresetName(LLUICtrl* ctrl, void * userData)
{
	LLComboBox * combo_box = static_cast<LLComboBox*>(ctrl);
	
	if(combo_box->getSimple() == "")
	{
		return;
	}

	const LLWLParamKey key(combo_box->getSelectedValue().asString(), LLEnvKey::SCOPE_LOCAL);
	if (LLWLParamManager::instance()->hasParamSet(key))
	{
		LLEnvManagerNew::instance().setUseSkyPreset(key.name, true);
	}
	else
	{
		//if that failed, use region's
		LLEnvManagerNew::instance().setUseSkyPreset("Default", true);
		// LLEnvManagerNew::instance().useRegionSky();
	}
}

void wlfPanel_AdvSettings::onClickWWNext(void* user_data)
{
	wlfPanel_AdvSettings* self = (wlfPanel_AdvSettings*) user_data;
	
	LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

	// find place of current param
	std::map<std::string, LLWaterParamSet>::iterator mIt = 
		param_mgr->mParamList.find(LLEnvManagerNew::instance().getWaterPresetName());

	// if at the end, loop
	std::map<std::string, LLWaterParamSet>::iterator last = param_mgr->mParamList.end(); --last;
	if (mIt == param_mgr->mParamList.end() || mIt == last) 
	{
		mIt = param_mgr->mParamList.begin();
	}
	else
	{
		mIt++;
	}

	//param_mgr->loadPreset(mIt->first, true);
	LLEnvManagerNew::instance().setUseWaterPreset(mIt->first, true);
	self->mWaterPresetCombo->setSimple(mIt->first);
}

void wlfPanel_AdvSettings::onClickWWPrev(void* user_data)
{
	wlfPanel_AdvSettings* self = (wlfPanel_AdvSettings*) user_data;
	
	LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

	// find place of current param
	std::map<std::string, LLWaterParamSet>::iterator mIt = 
		param_mgr->mParamList.find(LLEnvManagerNew::instance().getWaterPresetName());

	if (mIt == param_mgr->mParamList.end())
	{
		mIt = param_mgr->mParamList.begin();
	}
	// if at the beginning, loop
	else if(mIt == param_mgr->mParamList.begin()) 
	{
		std::map<std::string, LLWaterParamSet>::iterator last = param_mgr->mParamList.end(); --last;
		mIt = last;
	}
	else
	{
		mIt--;
	}

	//param_mgr->loadPreset(mIt->first, true);
	LLEnvManagerNew::instance().setUseWaterPreset(mIt->first, true);
	self->mWaterPresetCombo->setSimple(mIt->first);
}

void wlfPanel_AdvSettings::onClickWLNext(void* user_data)
{
	wlfPanel_AdvSettings* self = (wlfPanel_AdvSettings*) user_data;

	LLWLParamManager* param_mgr = LLWLParamManager::instance();

	// find place of current param
	LLWLParamKey key(LLEnvManagerNew::instance().getSkyPresetName(), LLEnvKey::SCOPE_LOCAL);
	std::map<LLWLParamKey, LLWLParamSet>::iterator mIt = 
		param_mgr->mParamList.find(key);

 	// shouldn't happen unless you delete every preset but Default
	if (mIt == param_mgr->mParamList.end())
	{
		llwarns << "No more presets left!" << llendl;
		return;
	}

	// if at the end, loop
	std::map<LLWLParamKey, LLWLParamSet>::iterator last = param_mgr->mParamList.end(); --last;
	if(mIt == last) 
	{
		mIt = param_mgr->mParamList.begin();
	}
	else
	{
		mIt++;
	}
	// param_mgr->mAnimator.deactivate();
	LLEnvManagerNew::instance().setUseSkyPreset(mIt->first.name, true);
	self->mSkyPresetCombo->setSimple(mIt->first.name);
}

void wlfPanel_AdvSettings::onClickWLPrev(void* user_data)
{
	wlfPanel_AdvSettings* self = (wlfPanel_AdvSettings*) user_data;

	LLWLParamManager* param_mgr = LLWLParamManager::instance();

	// find place of current param
	LLWLParamKey key(LLEnvManagerNew::instance().getSkyPresetName(), LLEnvKey::SCOPE_LOCAL);
	std::map<LLWLParamKey, LLWLParamSet>::iterator mIt = 
		param_mgr->mParamList.find(key);

 	// shouldn't happen unless you delete every preset but Default
	if (mIt == param_mgr->mParamList.end())
	{
		llwarns << "No more presets left!" << llendl;
		return;
	}

	// if at the beginning, loop
	if(mIt == param_mgr->mParamList.begin()) 
	{
		std::map<LLWLParamKey, LLWLParamSet>::iterator last = param_mgr->mParamList.end(); --last;
		mIt = last;
	}
	else
	{
		mIt--;
	}
	// param_mgr->mAnimator.deactivate();
	//LLWLParamManager::instance()->loadPreset(mIt->first.name, true);
	LLEnvManagerNew::instance().setUseSkyPreset(mIt->first.name, true);
	self->mSkyPresetCombo->setSimple(mIt->first.name);
}

void wlfPanel_AdvSettings::onOpenAdvancedSky(void* userData)
{
	LLFloaterWindLight::show();
}

void wlfPanel_AdvSettings::onOpenAdvancedWater(void* userData)
{
	LLFloaterWater::show();
}

void wlfPanel_AdvSettings::onChangeDayTime(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldr = (LLSliderCtrl*) ctrl;

	if (sldr) {
		// deactivate animator
		// LLWLParamManager::instance()->mAnimator.deactivate();

		F32 val = sldr->getValueF32() + 0.25f;
		if(val > 1.0) 
		{
			val--;
		}

		LLWLParamManager::instance()->mAnimator.setDayTime((F64)val);
		LLWLParamManager::instance()->mAnimator.update(
			LLWLParamManager::instance()->mCurParams);
	}
}

void wlfPanel_AdvSettings::populateWaterPresetsList()
{
	mWaterPresetCombo->removeall();

	std::list<std::string> user_presets, system_presets;
	LLWaterParamManager::instance()->getPresetNames(user_presets, system_presets);

	// Add user presets first.
	for (std::list<std::string>::const_iterator it = user_presets.begin(); it != user_presets.end(); ++it)
	{
		mWaterPresetCombo->add(*it);
	}

	if (user_presets.size() > 0)
	{
		mWaterPresetCombo->addSeparator();
	}

	// Add system presets.
	for (std::list<std::string>::const_iterator it = system_presets.begin(); it != system_presets.end(); ++it)
	{
		mWaterPresetCombo->add(*it);
	}

	mWaterPresetCombo->selectByValue(LLEnvManagerNew::instance().getWaterPresetName());
}

void wlfPanel_AdvSettings::populateSkyPresetsList()
{
	mSkyPresetCombo->removeall();

	LLWLParamManager::preset_name_list_t region_presets; // unused as we don't list region presets here
	LLWLParamManager::preset_name_list_t user_presets, sys_presets;
	LLWLParamManager::instance()->getPresetNames(region_presets, user_presets, sys_presets);

	// Add user presets.
	for (LLWLParamManager::preset_name_list_t::const_iterator it = user_presets.begin(); it != user_presets.end(); ++it)
	{
		mSkyPresetCombo->add(*it);
	}

	if (!user_presets.empty())
	{
		mSkyPresetCombo->addSeparator();
	}

	// Add system presets.
	for (LLWLParamManager::preset_name_list_t::const_iterator it = sys_presets.begin(); it != sys_presets.end(); ++it)
	{
		mSkyPresetCombo->add(*it);
	}

	mSkyPresetCombo->selectByValue(LLEnvManagerNew::instance().getSkyPresetName());
}

// void wlfPanel_AdvSettings::populateDayCyclePresetsList()
// {
	// mDayCyclePresetCombo->removeall();

	// LLDayCycleManager::preset_name_list_t user_days, sys_days;
	// LLDayCycleManager::instance().getPresetNames(user_days, sys_days);

	// // Add user days.
	// for (LLDayCycleManager::preset_name_list_t::const_iterator it = user_days.begin(); it != user_days.end(); ++it)
	// {
		// mDayCyclePresetCombo->add(*it);
	// }

	// if (user_days.size() > 0)
	// {
		// mDayCyclePresetCombo->addSeparator();
	// }

	// // Add system days.
	// for (LLDayCycleManager::preset_name_list_t::const_iterator it = sys_days.begin(); it != sys_days.end(); ++it)
	// {
		// mDayCyclePresetCombo->add(*it);
	// }

	// mDayCyclePresetCombo->selectByValue(LLEnvManagerNew::instance().getDayCycleName());
// }