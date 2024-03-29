/** 
 * @file llfloaterwater.cpp
 * @brief LLFloaterWater class definition
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
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

#include "llfloaterwater.h"

#include "pipeline.h"
#include "llsky.h"

#include "llsliderctrl.h"
#include "llspinctrl.h"
#include "llcolorswatch.h"
#include "llcheckboxctrl.h"
#include "lltexturectrl.h"
#include "lluictrlfactory.h"
#include "llviewercamera.h"
#include "llcombobox.h"
#include "lllineeditor.h"
#include "llfloaterdaycycle.h"
#include "llboost.h"
#include "llmultisliderctrl.h"

#include "llagent.h"
#include "llinventorymodel.h"
#include "llviewerinventory.h"

#include "v4math.h"
#include "llviewerdisplay.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llsavedsettingsglue.h"

#include "llwaterparamset.h"
#include "llwaterparammanager.h"
#include "llpostprocess.h"

#undef max

LLFloaterWater* LLFloaterWater::sWaterMenu = NULL;

std::set<std::string> LLFloaterWater::sDefaultPresets;

LLFloaterWater::LLFloaterWater() : LLFloater(std::string("water floater"))
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_water.xml");
	
	// add the combo boxes
	mWaterPresetCombo = getChild<LLComboBox>("WaterPresetsCombo");

	if (mWaterPresetCombo != NULL)
	{
		populateWaterPresetsList();
		mWaterPresetCombo->setCommitCallback(onChangePresetName);
	}


	std::string def_water = getString("WLDefaultWaterNames");

	// no editing or deleting of the blank string
	sDefaultPresets.insert("");
	boost_tokenizer tokens(def_water, boost::char_separator<char>(":"));
	for (boost_tokenizer::iterator token_iter = tokens.begin(); token_iter != tokens.end(); ++token_iter)
	{
		std::string tok(*token_iter);
		sDefaultPresets.insert(tok);
	}

	// load it up
	initCallbacks();
}

LLFloaterWater::~LLFloaterWater()
{
}

void LLFloaterWater::initCallbacks(void) {

	// help buttons
	initHelpBtn("WaterFogColorHelp", "HelpWaterFogColor");
	initHelpBtn("WaterFogDensityHelp", "HelpWaterFogDensity");
	initHelpBtn("WaterUnderWaterFogModHelp", "HelpUnderWaterFogMod");
	initHelpBtn("WaterGlowHelp", "HelpWaterGlow");	
	initHelpBtn("WaterNormalScaleHelp", "HelpWaterNormalScale");
	initHelpBtn("WaterFresnelScaleHelp", "HelpWaterFresnelScale");
	initHelpBtn("WaterFresnelOffsetHelp", "HelpWaterFresnelOffset");

	initHelpBtn("WaterBlurMultiplierHelp", "HelpWaterBlurMultiplier");
	initHelpBtn("WaterScaleBelowHelp", "HelpWaterScaleBelow");
	initHelpBtn("WaterScaleAboveHelp", "HelpWaterScaleAbove");

	initHelpBtn("WaterNormalMapHelp", "HelpWaterNormalMap");
	initHelpBtn("WaterWave1Help", "HelpWaterWave1");
	initHelpBtn("WaterWave2Help", "HelpWaterWave2");

	LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

	childSetCommitCallback("WaterFogColor", onWaterFogColorMoved, &param_mgr->mFogColor);

	// 
	childSetCommitCallback("WaterGlow", onColorControlAMoved, &param_mgr->mFogColor);

	// fog density
	childSetCommitCallback("WaterFogDensity", onExpFloatControlMoved, &param_mgr->mFogDensity);
	childSetCommitCallback("WaterUnderWaterFogMod", onFloatControlMoved, &param_mgr->mUnderWaterFogMod);

	// blue density
	childSetCommitCallback("WaterNormalScaleX", onVector3ControlXMoved, &param_mgr->mNormalScale);
	childSetCommitCallback("WaterNormalScaleY", onVector3ControlYMoved, &param_mgr->mNormalScale);
	childSetCommitCallback("WaterNormalScaleZ", onVector3ControlZMoved, &param_mgr->mNormalScale);

	// fresnel
	childSetCommitCallback("WaterFresnelScale", onFloatControlMoved, &param_mgr->mFresnelScale);
	childSetCommitCallback("WaterFresnelOffset", onFloatControlMoved, &param_mgr->mFresnelOffset);

	// scale above/below
	childSetCommitCallback("WaterScaleAbove", onFloatControlMoved, &param_mgr->mScaleAbove);
	childSetCommitCallback("WaterScaleBelow", onFloatControlMoved, &param_mgr->mScaleBelow);

	// blur mult
	childSetCommitCallback("WaterBlurMult", onFloatControlMoved, &param_mgr->mBlurMultiplier);

	// Load/save
	//childSetAction("WaterLoadPreset", onLoadPreset, mWaterPresetCombo);
	childSetAction("WaterNewPreset", onNewPreset, mWaterPresetCombo);
	childSetAction("WaterDeletePreset", onDeletePreset, mWaterPresetCombo);
	childSetCommitCallback("WaterSavePreset", onSavePreset, this);

	// wave direction
	childSetCommitCallback("WaterWave1DirX", onVector2ControlXMoved, &param_mgr->mWave1Dir);
	childSetCommitCallback("WaterWave1DirY", onVector2ControlYMoved, &param_mgr->mWave1Dir);
	childSetCommitCallback("WaterWave2DirX", onVector2ControlXMoved, &param_mgr->mWave2Dir);
	childSetCommitCallback("WaterWave2DirY", onVector2ControlYMoved, &param_mgr->mWave2Dir);

	LLTextureCtrl* textCtrl = getChild<LLTextureCtrl>("WaterNormalMap");
	textCtrl->setDefaultImageAssetID(DEFAULT_WATER_NORMAL);
	childSetCommitCallback("WaterNormalMap", onNormalMapPicked, NULL);

	// next/prev buttons
	childSetAction("next", onClickNext, this);
	childSetAction("prev", onClickPrev, this);
}

void LLFloaterWater::onClickHelp(void* data)
{
	LLFloaterWater* self = LLFloaterWater::instance();

	const std::string* xml_alert = (std::string*)data;
	LLNotifications::instance().add(self->contextualNotification(*xml_alert));
}

void LLFloaterWater::initHelpBtn(const std::string& name, const std::string& xml_alert)
{
	childSetAction(name, onClickHelp, new std::string(xml_alert));
}

bool LLFloaterWater::newPromptCallback(const LLSD& notification, const LLSD& response)
{
	std::string text = response["message"].asString();
	S32 option = LLNotification::getSelectedOption(notification, response);

	if(text == "")
	{
		return false;
	}

	if(option == 0) {
		LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

		// add the current parameters to the list
		// see if it's there first
		// if not there, add a new one
		if(!LLWaterParamManager::instance()->hasParamSet(text)) 
		{
			param_mgr->addParamSet(text, param_mgr->mCurParams);
			sWaterMenu->mWaterPresetCombo->add(text);
			sWaterMenu->mWaterPresetCombo->sortByName();

			sWaterMenu->mWaterPresetCombo->selectByValue(text);

			param_mgr->savePreset(text);

			LLEnvManagerNew::instance().setUseWaterPreset(text);

		// otherwise, send a message to the user
		} 
		else 
		{
			LLNotifications::instance().add("ExistsWaterPresetAlert");
		}
	}
	return false;
}

void LLFloaterWater::syncMenu()
{
	bool err;

	LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

	LLWaterParamSet & current_params = param_mgr->mCurParams;

	if (mWaterPresetCombo->getSelectedItemLabel() != LLEnvManagerNew::instance().getWaterPresetName())
	{
		mWaterPresetCombo->selectByValue(LLEnvManagerNew::instance().getWaterPresetName());
	}

	// blue horizon
	param_mgr->mFogColor = current_params.getVector4(param_mgr->mFogColor.mName, err);

	LLColor4 col = param_mgr->getFogColor();
	childSetValue("WaterGlow", col.mV[3]);
	col.mV[3] = 1.0f;
	LLColorSwatchCtrl* colCtrl = sWaterMenu->getChild<LLColorSwatchCtrl>("WaterFogColor");

	colCtrl->set(col);

	// fog and wavelets
	param_mgr->mFogDensity.mExp = 
		log(current_params.getFloat(param_mgr->mFogDensity.mName, err)) / 
		log(param_mgr->mFogDensity.mBase);
	param_mgr->setDensitySliderValue(param_mgr->mFogDensity.mExp);
	childSetValue("WaterFogDensity", param_mgr->mFogDensity.mExp);
	
	param_mgr->mUnderWaterFogMod.mX = 
		current_params.getFloat(param_mgr->mUnderWaterFogMod.mName, err);
	childSetValue("WaterUnderWaterFogMod", param_mgr->mUnderWaterFogMod.mX);

	param_mgr->mNormalScale = current_params.getVector3(param_mgr->mNormalScale.mName, err);
	childSetValue("WaterNormalScaleX", param_mgr->mNormalScale.mX);
	childSetValue("WaterNormalScaleY", param_mgr->mNormalScale.mY);
	childSetValue("WaterNormalScaleZ", param_mgr->mNormalScale.mZ);

	// Fresnel
	param_mgr->mFresnelScale.mX = current_params.getFloat(param_mgr->mFresnelScale.mName, err);
	childSetValue("WaterFresnelScale", param_mgr->mFresnelScale.mX);
	param_mgr->mFresnelOffset.mX = current_params.getFloat(param_mgr->mFresnelOffset.mName, err);
	childSetValue("WaterFresnelOffset", param_mgr->mFresnelOffset.mX);

	// Scale Above/Below
	param_mgr->mScaleAbove.mX = current_params.getFloat(param_mgr->mScaleAbove.mName, err);
	childSetValue("WaterScaleAbove", param_mgr->mScaleAbove.mX);
	param_mgr->mScaleBelow.mX = current_params.getFloat(param_mgr->mScaleBelow.mName, err);
	childSetValue("WaterScaleBelow", param_mgr->mScaleBelow.mX);

	// blur mult
	param_mgr->mBlurMultiplier.mX = current_params.getFloat(param_mgr->mBlurMultiplier.mName, err);
	childSetValue("WaterBlurMult", param_mgr->mBlurMultiplier.mX);

	// wave directions
	param_mgr->mWave1Dir = current_params.getVector2(param_mgr->mWave1Dir.mName, err);
	childSetValue("WaterWave1DirX", param_mgr->mWave1Dir.mX);
	childSetValue("WaterWave1DirY", param_mgr->mWave1Dir.mY);

	param_mgr->mWave2Dir = current_params.getVector2(param_mgr->mWave2Dir.mName, err);
	childSetValue("WaterWave2DirX", param_mgr->mWave2Dir.mX);
	childSetValue("WaterWave2DirY", param_mgr->mWave2Dir.mY);

	LLTextureCtrl* textCtrl = sWaterMenu->getChild<LLTextureCtrl>("WaterNormalMap");
	textCtrl->setImageAssetID(param_mgr->getNormalMapID());
}


// static
LLFloaterWater* LLFloaterWater::instance()
{
	if (!sWaterMenu)
	{
		sWaterMenu = new LLFloaterWater();
		sWaterMenu->open();
		sWaterMenu->setFocus(TRUE);
	}
	return sWaterMenu;
}
void LLFloaterWater::show()
{
	if (!sWaterMenu)
	{
		LLFloaterWater* water = instance();
		water->syncMenu();

		// comment in if you want the menu to rebuild each time
		//LLUICtrlFactory::getInstance()->buildFloater(water, "floater_water.xml");
		//water->initCallbacks();
	}
	else
	{
		if (sWaterMenu->getVisible())
		{
			sWaterMenu->close();
		}
		else
		{
			sWaterMenu->open();
		}
	}
}

bool LLFloaterWater::isOpen()
{
	if (sWaterMenu != NULL) {
		return true;
	}
	return false;
}

// virtual
void LLFloaterWater::onClose(bool app_quitting)
{
	if (sWaterMenu)
	{
		sWaterMenu->setVisible(FALSE);
	}
}

// vector control callbacks
void LLFloaterWater::onVector3ControlXMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterVector3Control * vectorControl = static_cast<WaterVector3Control *>(userData);

	vectorControl->mX = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

// vector control callbacks
void LLFloaterWater::onVector3ControlYMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterVector3Control * vectorControl = static_cast<WaterVector3Control *>(userData);

	vectorControl->mY = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

// vector control callbacks
void LLFloaterWater::onVector3ControlZMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterVector3Control * vectorControl = static_cast<WaterVector3Control *>(userData);

	vectorControl->mZ = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}


// vector control callbacks
void LLFloaterWater::onVector2ControlXMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterVector2Control * vectorControl = static_cast<WaterVector2Control *>(userData);

	vectorControl->mX = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

// vector control callbacks
void LLFloaterWater::onVector2ControlYMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterVector2Control * vectorControl = static_cast<WaterVector2Control *>(userData);

	vectorControl->mY = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

// color control callbacks
void LLFloaterWater::onColorControlRMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterColorControl * colorControl = static_cast<WaterColorControl *>(userData);

	colorControl->mR = sldrCtrl->getValueF32();

	// move i if it's the max
	if(colorControl->mR >= colorControl->mG 
		&& colorControl->mR >= colorControl->mB 
		&& colorControl->mHasSliderName)
	{
		colorControl->mI = colorControl->mR;
		std::string name = colorControl->mSliderName;
		name.append("I");
		
		sWaterMenu->childSetValue(name, colorControl->mR);
	}

	colorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onColorControlGMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterColorControl * colorControl = static_cast<WaterColorControl *>(userData);

	colorControl->mG = sldrCtrl->getValueF32();

	// move i if it's the max
	if(colorControl->mG >= colorControl->mR 
		&& colorControl->mG >= colorControl->mB
		&& colorControl->mHasSliderName)
	{
		colorControl->mI = colorControl->mG;
		std::string name = colorControl->mSliderName;
		name.append("I");

		sWaterMenu->childSetValue(name, colorControl->mG);

	}

	colorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onColorControlBMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterColorControl * colorControl = static_cast<WaterColorControl *>(userData);

	colorControl->mB = sldrCtrl->getValueF32();

	// move i if it's the max
	if(colorControl->mB >= colorControl->mR
		&& colorControl->mB >= colorControl->mG
		&& colorControl->mHasSliderName)
	{
		colorControl->mI = colorControl->mB;
		std::string name = colorControl->mSliderName;
		name.append("I");

		sWaterMenu->childSetValue(name, colorControl->mB);
	}

	colorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onColorControlAMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterColorControl * colorControl = static_cast<WaterColorControl *>(userData);

	colorControl->mA = sldrCtrl->getValueF32();

	colorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}


void LLFloaterWater::onColorControlIMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterColorControl * colorControl = static_cast<WaterColorControl *>(userData);

	colorControl->mI = sldrCtrl->getValueF32();
	
	// only for sliders where we pass a name
	if(colorControl->mHasSliderName) 
	{
		// set it to the top
		F32 maxVal = std::max(std::max(colorControl->mR, colorControl->mG), colorControl->mB);
		F32 iVal;

		iVal = colorControl->mI;

		// get the names of the other sliders
		std::string rName = colorControl->mSliderName;
		rName.append("R");
		std::string gName = colorControl->mSliderName;
		gName.append("G");
		std::string bName = colorControl->mSliderName;
		bName.append("B");

		// handle if at 0
		if(iVal == 0)
		{
			colorControl->mR = 0;
			colorControl->mG = 0;
			colorControl->mB = 0;
		
		// if all at the start
		// set them all to the intensity
		}
		else if (maxVal == 0)
		{
			colorControl->mR = iVal;
			colorControl->mG = iVal;
			colorControl->mB = iVal;
		}
		else
		{
			// add delta amounts to each
			F32 delta = (iVal - maxVal) / maxVal;
			colorControl->mR *= (1.0f + delta);
			colorControl->mG *= (1.0f + delta);
			colorControl->mB *= (1.0f + delta);
		}

		// set the sliders to the new vals
		sWaterMenu->childSetValue(rName, colorControl->mR);
		sWaterMenu->childSetValue(gName, colorControl->mG);
		sWaterMenu->childSetValue(bName, colorControl->mB);
	}

	// now update the current parameters and send them to shaders
	colorControl->update(LLWaterParamManager::instance()->mCurParams);
	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onExpFloatControlMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterExpFloatControl * expFloatControl = static_cast<WaterExpFloatControl *>(userData);

	F32 val = sldrCtrl->getValueF32();
	expFloatControl->mExp = val;
	LLWaterParamManager::instance()->setDensitySliderValue(val);

	expFloatControl->update(LLWaterParamManager::instance()->mCurParams);
	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onFloatControlMoved(LLUICtrl* ctrl, void* userData)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);
	WaterFloatControl * floatControl = static_cast<WaterFloatControl *>(userData);

	floatControl->mX = sldrCtrl->getValueF32() / floatControl->mMult;

	floatControl->update(LLWaterParamManager::instance()->mCurParams);
	LLWaterParamManager::instance()->propagateParameters();
}
void LLFloaterWater::onWaterFogColorMoved(LLUICtrl* ctrl, void* userData)
{
	LLColorSwatchCtrl* swatch = static_cast<LLColorSwatchCtrl*>(ctrl);
	WaterColorControl * colorControl = static_cast<WaterColorControl *>(userData);	
	*colorControl = swatch->get();

	colorControl->update(LLWaterParamManager::instance()->mCurParams);
	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onBoolToggle(LLUICtrl* ctrl, void* userData)
{
	LLCheckBoxCtrl* cbCtrl = static_cast<LLCheckBoxCtrl*>(ctrl);

	bool value = cbCtrl->get();
	(*(static_cast<BOOL *>(userData))) = value;
}

void LLFloaterWater::onNormalMapPicked(LLUICtrl* ctrl, void* userData)
{
	LLTextureCtrl* textCtrl = static_cast<LLTextureCtrl*>(ctrl);
	LLUUID textID = textCtrl->getImageAssetID();
	LLWaterParamManager::instance()->setNormalMapID(textID);
}

void LLFloaterWater::onNewPreset(void* userData)
{
	LLNotifications::instance().add("NewWaterPreset", LLSD(),  LLSD(), newPromptCallback);
}

class KVFloaterWaterNotecardCreatedCallback : public LLInventoryCallback
{
public:
	void fire(const LLUUID& inv_item);
};

void KVFloaterWaterNotecardCreatedCallback::fire(const LLUUID& inv_item)
{
	LLWaterParamManager * param_mgr = LLWaterParamManager::instance();
	param_mgr->mCurParams.mInventoryID = inv_item;
	param_mgr->setParamSet(param_mgr->mCurParams.mName, param_mgr->mCurParams);
	//param_mgr->mParamList[param_mgr->mCurParams.mName].mInventoryID = inv_item;
	LL_INFOS("WindLight") << "Created inventory item " << inv_item << LL_ENDL;
	param_mgr->savePresetToNotecard(param_mgr->mCurParams.mName);
}

void LLFloaterWater::onSavePreset(LLUICtrl* ctrl, void* userData)
{
	// don't save the empty name
	if(sWaterMenu->mWaterPresetCombo->getSelectedItemLabel() == "")
	{
		return;
	}

	if (ctrl->getValue().asString() == "save_inventory_item")
	{	
		// Check if this is already a notecard and that its not in the trash or purged.
		LLUUID trash_id;
		trash_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_TRASH);
		if(LLWaterParamManager::instance()->mCurParams.mInventoryID.notNull() 
			&& !gInventory.isObjectDescendentOf(LLWaterParamManager::instance()->mCurParams.mInventoryID, trash_id)
			&& gInventory.isObjectDescendentOf(LLWaterParamManager::instance()->mCurParams.mInventoryID, gAgent.getInventoryRootID())
		)
		{
			LLNotifications::instance().add("KittyWLSaveNotecardAlert", LLSD(), LLSD(), saveNotecardCallback);
		}
		else
		{
			// Make sure we have a ".ww" extension.
			std::string name = sWaterMenu->mWaterPresetCombo->getSelectedItemLabel();
			if(name.length() > 2 && name.compare(name.length() - 3, 3, ".ww") != 0)
			{
				name += ".ww";
			}
			LLPointer<KVFloaterWaterNotecardCreatedCallback> cb = new KVFloaterWaterNotecardCreatedCallback();
			// Create a notecard and then save it.
			create_inventory_item(gAgent.getID(), 
								  gAgent.getSessionID(),
								  LLUUID::null,
								  LLTransactionID::tnull,
								  name,
								  "Water settings (Imprudence compatible)",
								  LLAssetType::AT_NOTECARD,
								  LLInventoryType::IT_NOTECARD,
								  NOT_WEARABLE,
								  PERM_ITEM_UNRESTRICTED,
								  cb);
			
		}
	}
	else
	{
		LLWaterParamManager::instance()->mCurParams.mName = 
			sWaterMenu->mWaterPresetCombo->getSelectedItemLabel();

		// check to see if it's a default and shouldn't be overwritten
		std::set<std::string>::iterator sIt = sDefaultPresets.find(
			sWaterMenu->mWaterPresetCombo->getSelectedItemLabel());
		if(sIt != sDefaultPresets.end() && !gSavedSettings.getBOOL("WaterEditPresets")) 
		{
			LLNotifications::instance().add("WLNoEditDefault");
			return;
		}

		LLNotifications::instance().add("WLSavePresetAlert", LLSD(), LLSD(), saveAlertCallback);
	}
}

bool LLFloaterWater::saveNotecardCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	// if they choose save, do it.  Otherwise, don't do anything
	if(option == 0) 
	{
		LLWaterParamManager * param_mgr = LLWaterParamManager::instance();
		param_mgr->setParamSet(param_mgr->mCurParams.mName, param_mgr->mCurParams);
		param_mgr->savePresetToNotecard(param_mgr->mCurParams.mName);
	}
	return false;
}

bool LLFloaterWater::saveAlertCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	// if they choose save, do it.  Otherwise, don't do anything
	if(option == 0) 
	{
		LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

		param_mgr->setParamSet(
			param_mgr->mCurParams.mName, 
			param_mgr->mCurParams);

		// comment this back in to save to file
		param_mgr->savePreset(param_mgr->mCurParams.mName);
	}
	return false;
}

void LLFloaterWater::onDeletePreset(void* userData)
{
	if(sWaterMenu->mWaterPresetCombo->getSelectedValue().asString() == "")
	{
		return;
	}

	LLSD args;
	args["SKY"] = sWaterMenu->mWaterPresetCombo->getSelectedValue().asString();
	LLNotifications::instance().add("WLDeletePresetAlert", args, LLSD(), deleteAlertCallback);
}

bool LLFloaterWater::deleteAlertCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	// if they choose delete, do it.  Otherwise, don't do anything
	if(option == 0) 
	{
		LLFloaterDayCycle* day_cycle = NULL;
		LLComboBox* key_combo = NULL;
		LLMultiSliderCtrl* mult_sldr = NULL;

		if(LLFloaterDayCycle::isOpen()) 
		{
			day_cycle = LLFloaterDayCycle::instance();
			key_combo = day_cycle->getChild<LLComboBox>("WaterKeyPresets");
			mult_sldr = day_cycle->getChild<LLMultiSliderCtrl>("WaterDayCycleKeys");
		}

		std::string name = sWaterMenu->mWaterPresetCombo->getSelectedValue().asString();

		// check to see if it's a default and shouldn't be deleted
		std::set<std::string>::iterator sIt = sDefaultPresets.find(name);
		if(sIt != sDefaultPresets.end()) 
		{
			LLNotifications::instance().add("WaterNoEditDefault");
			return false;
		}

		LLWaterParamManager::instance()->removeParamSet(name, true);
		
		// remove and choose another
		S32 new_index = sWaterMenu->mWaterPresetCombo->getCurrentIndex();

		sWaterMenu->mWaterPresetCombo->remove(name);

		if(key_combo != NULL) 
		{
			key_combo->remove(name);

			// remove from slider, as well
			day_cycle->deletePreset(name);
		}

		// pick the previously selected index after delete
		if(new_index > 0) 
		{
			new_index--;
		}
		
		if(sWaterMenu->mWaterPresetCombo->getItemCount() > 0) 
		{
			sWaterMenu->mWaterPresetCombo->setCurrentByIndex(new_index);
		}
	}
	return false;
}


void LLFloaterWater::onChangePresetName(LLUICtrl* ctrl, void * userData)
{
	LLComboBox * combo_box = static_cast<LLComboBox*>(ctrl);
	
	if(combo_box->getSimple() == "")
	{
		return;
	}

	const std::string& wwset = combo_box->getSelectedValue().asString();
	if (LLWaterParamManager::instance()->hasParamSet(wwset))
	{
		LLEnvManagerNew::instance().setUseWaterPreset(wwset);
	}
	else
	{
		//if that failed, use region's
		// LLEnvManagerNew::instance().useRegionWater();
		LLEnvManagerNew::instance().setUseWaterPreset("Default");
	}
	sWaterMenu->syncMenu();
}

void LLFloaterWater::onClickNext(void* user_data)
{
	S32 index = sWaterMenu->mWaterPresetCombo->getCurrentIndex();
	index++;
	if (index == sWaterMenu->mWaterPresetCombo->getItemCount())
		index = 0;
	sWaterMenu->mWaterPresetCombo->setCurrentByIndex(index);

	LLFloaterWater::onChangePresetName(sWaterMenu->mWaterPresetCombo, sWaterMenu);
}

void LLFloaterWater::onClickPrev(void* user_data)
{
	S32 index = sWaterMenu->mWaterPresetCombo->getCurrentIndex();
	if (index == 0)
		index = sWaterMenu->mWaterPresetCombo->getItemCount();
	index--;
	sWaterMenu->mWaterPresetCombo->setCurrentByIndex(index);

	LLFloaterWater::onChangePresetName(sWaterMenu->mWaterPresetCombo, sWaterMenu);
}

void LLFloaterWater::populateWaterPresetsList()
{
	mWaterPresetCombo->removeall();

	std::list<std::string> presets;
	LLWaterParamManager::instance()->getPresetNames(presets);

	for (std::list<std::string>::const_iterator it = presets.begin(); it != presets.end(); ++it)
	{
		mWaterPresetCombo->add(*it);
	}

	mWaterPresetCombo->selectByValue(LLEnvManagerNew::instance().getWaterPresetName());
}
