#include "llviewerprecompiledheaders.h"

#include "hippolimits.h"

#include "hippogridmanager.h"

#include "llerror.h"

#include "llviewercontrol.h"		// gSavedSettings
#include "llagent.h"
#include "llviewerregion.h"

HippoLimits *gHippoLimits = 0;


HippoLimits::HippoLimits()
{
	setLimits();
}


void HippoLimits::setLimits()
{
	if (gHippoGridManager->getConnectedGrid()->getPlatform() == HippoGridInfo::PLATFORM_SECONDLIFE) {
		setSecondLifeLimits();
	} else {
		setOpenSimLimits();
	}
}


void HippoLimits::setOpenSimLimits()
{
	mMaxAgentGroups = gHippoGridManager->getConnectedGrid()->getMaxAgentGroups();
	if (mMaxAgentGroups < 0) mMaxAgentGroups = 50;
	mMaxPrimScale = 256.0f;
	mMaxHeight = 10000.0f;
	if (gHippoGridManager->getConnectedGrid()->isRenderCompat()) {
		llinfos << "Using rendering compatible OpenSim limits." << llendl;
		mMinHoleSize = 0.05f;
		mMaxHollow = 0.95f;
	} else {
		llinfos << "Using Hippo OpenSim limits." << llendl;
		mMinHoleSize = 0.01f;
		mMaxHollow = 0.99f;
	}
}

void HippoLimits::setSecondLifeLimits()
{
	llinfos << "Using Second Life limits." << llendl;
	
	//KC: new server defined max groups
	mMaxAgentGroups = gHippoGridManager->getConnectedGrid()->getMaxAgentGroups();
	if (mMaxAgentGroups < 0)
	{
		mMaxAgentGroups = DEFAULT_MAX_AGENT_GROUPS;
	}
	
	mMaxHeight = 4096.0f;
	mMinHoleSize = 0.05f;
	mMaxHollow = 0.95f;

	// Check if we have caps yet for the new region, defer till we do if needed
	LLViewerRegion* cur_region = gAgent.getRegion();
	if (cur_region)
	{
		if (!cur_region->capabilitiesReceived())
		{
			// set limit back to 10m till get caps
			mMaxPrimScale = 10.0f;
			gSavedSettings.setBOOL("Phoenix64mPrimSupport", FALSE);
			cur_region->setCapabilitiesReceivedCallback(boost::bind(&HippoLimits::onRegionCapsReceived, _1));
		}
		else
		{
			doSecondLifeMeshCheck();
		}
	}
}

//static
void HippoLimits::onRegionCapsReceived(const LLUUID& region_id)
{
	if (region_id == gAgent.getRegion()->getRegionID())
	{
		gHippoLimits->doSecondLifeMeshCheck();
	}
}

void HippoLimits::doSecondLifeMeshCheck()
{
	// check if the region has mesh for 64m prim support
	if (!gAgent.getRegion()->getCapability("GetMesh").empty())
	{
		mMaxPrimScale = 64.0f;
		gSavedSettings.setBOOL("Phoenix64mPrimSupport", TRUE);
	}
	else
	{
		mMaxPrimScale = 10.0f;
		gSavedSettings.setBOOL("Phoenix64mPrimSupport", FALSE);
	}
}

