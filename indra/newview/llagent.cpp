/** 
 * @file llagent.cpp
 * @brief LLAgent class implementation
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

#include "stdtypes.h"
#include "stdenums.h"

#include "cofmgr.h"
#include "llagent.h" 
#include "llcamera.h"
#include "llcoordframe.h"
#include "indra_constants.h"
#include "llmath.h"
#include "llcriticaldamp.h"
#include "llfocusmgr.h"
#include "llglheaders.h"
#include "llparcel.h"
#include "llpermissions.h"
#include "llregionhandle.h"
#include "m3math.h"
#include "m4math.h"
#include "message.h"
#include "llquaternion.h"
#include "v3math.h"
#include "v4math.h"
#include "llsmoothstep.h"
#include "llsdutil.h"
//#include "vmath.h"

#include "imageids.h"
#include "llbox.h"
#include "llbutton.h"
#include "llcallingcard.h"
#include "llchatbar.h"
#include "llconsole.h"
#include "lldrawable.h"
#include "llface.h"
#include "llfirstuse.h"
#include "llfloater.h"
#include "llfloateractivespeakers.h"
#include "llfloateravatarinfo.h"
#include "llfloaterbuildoptions.h"
#include "llfloatercamera.h"
#include "llfloaterchat.h"
#include "llfloatercustomize.h"
#include "llfloaterdirectory.h"
#include "llfloatergroupinfo.h"
#include "llfloatergroups.h"
#include "llfloaterland.h"
#include "llfloatermap.h"
#include "llfloatermute.h"
#include "llfloatersnapshot.h"
#include "llfloatertools.h"
#include "llfloaterworldmap.h"
#include "llgroupmgr.h"
#include "llhomelocationresponder.h"
#include "llhudeffectlookat.h"
#include "llhudmanager.h"
#include "llinventorymodel.h"
#include "llinventoryview.h"
#include "lljoystickbutton.h"
#include "llmenugl.h"
#include "llmorphview.h"
#include "llmoveview.h"
#include "llnotify.h"
#include "llquantize.h"
#include "llsdutil.h"
#include "llselectmgr.h"
#include "llsky.h"
#include "llrendersphere.h"
#include "llstatusbar.h"
#include "llstartup.h"
#include "llimview.h"
#include "lltexturestats.h"
#include "lltool.h"
#include "lltoolcomp.h"
#include "lltoolfocus.h"
#include "lltoolgrab.h"
#include "lltoolmgr.h"
#include "lltoolpie.h"
#include "lltoolview.h"
#include "llui.h"			// for make_ui_sound
#include "llurldispatcher.h"
#include "llviewercamera.h"
#include "llviewerinventory.h"
#include "llviewermediafocus.h"
#include "llviewermenu.h"
#include "llviewernetwork.h"
#include "llviewerobjectlist.h"
#include "llviewerparcelmgr.h"
#include "llviewerparceloverlay.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llviewerwindow.h"
#include "llviewerdisplay.h"
#include "llvoavatar.h"
#include "llvoground.h"
#include "llvosky.h"
#include "llwearable.h"
#include "llwearablelist.h"
#include "llworld.h"
#include "llworldmap.h"
#include "pipeline.h"
#include "roles_constants.h"
#include "llviewercontrol.h"
#include "llappviewer.h"
#include "llviewerjoystick.h"
#include "llfollowcam.h"
#include "llfloaterteleporthistory.h"
#include "jc_lslviewerbridge.h"
#include "llenvmanager.h"
#include "hippolimits.h"

#include "llfloaterstats.h"
#include "floaterao.h"
#include "llworldmapmessage.h"
// [RLVa:KB] - Checked: 2010-09-27 (RLVa-1.1.3b)
#include "rlvhandler.h"
#include "rlvinventory.h"
#include "llattachmentsmgr.h"
// [/RLVa:KB]
#include "kcwlinterface.h"

using namespace LLVOAvatarDefines;

extern LLMenuBarGL* gMenuBarView;

//drone wandering constants
const F32 MAX_WANDER_TIME = 20.f;						// seconds
const F32 MAX_HEADING_HALF_ERROR = 0.2f;				// radians
const F32 WANDER_MAX_SLEW_RATE = 2.f * DEG_TO_RAD;		// radians / frame
const F32 WANDER_TARGET_MIN_DISTANCE = 10.f;			// meters

// Autopilot constants
const F32 AUTOPILOT_HEADING_HALF_ERROR = 10.f * DEG_TO_RAD;	// radians
const F32 AUTOPILOT_MAX_SLEW_RATE = 1.f * DEG_TO_RAD;		// radians / frame
const F32 AUTOPILOT_STOP_DISTANCE = 2.f;					// meters
const F32 AUTOPILOT_HEIGHT_ADJUST_DISTANCE = 8.f;			// meters
const F32 AUTOPILOT_MIN_TARGET_HEIGHT_OFF_GROUND = 1.f;	// meters
const F32 AUTOPILOT_MAX_TIME_NO_PROGRESS = 1.5f;		// seconds

// face editing constants
const LLVector3d FACE_EDIT_CAMERA_OFFSET(0.4f, -0.05f, 0.07f);
const LLVector3d FACE_EDIT_TARGET_OFFSET(0.f, 0.f, 0.05f);

// Mousewheel camera zoom
const F32 MIN_ZOOM_FRACTION = 0.25f;
const F32 INITIAL_ZOOM_FRACTION = 1.f;
const F32 MAX_ZOOM_FRACTION = 8.f;
const F32 METERS_PER_WHEEL_CLICK = 1.f;

const F32 MAX_TIME_DELTA = 1.f;

const F32 CAMERA_ZOOM_HALF_LIFE = 0.07f;	// seconds
const F32 FOV_ZOOM_HALF_LIFE = 0.07f;	// seconds

const F32 CAMERA_FOCUS_HALF_LIFE = 0.f;//0.02f;
const F32 CAMERA_LAG_HALF_LIFE = 0.25f;
const F32 MIN_CAMERA_LAG = 0.5f;
const F32 MAX_CAMERA_LAG = 5.f;

const F32 CAMERA_COLLIDE_EPSILON = 0.1f;
const F32 MIN_CAMERA_DISTANCE = 0.1f;
const F32 AVATAR_ZOOM_MIN_X_FACTOR = 0.55f;
const F32 AVATAR_ZOOM_MIN_Y_FACTOR = 0.7f;
const F32 AVATAR_ZOOM_MIN_Z_FACTOR = 1.15f;

const F32 MAX_CAMERA_DISTANCE_FROM_AGENT = 50.f;

const F32 MAX_CAMERA_SMOOTH_DISTANCE = 50.0f;

const F32 HEAD_BUFFER_SIZE = 0.3f;
const F32 CUSTOMIZE_AVATAR_CAMERA_ANIM_SLOP = 0.2f;

const F32 LAND_MIN_ZOOM = 0.15f;
const F32 AVATAR_MIN_ZOOM = 0.5f;
const F32 OBJECT_MIN_ZOOM = 0.02f;

const F32 APPEARANCE_MIN_ZOOM = 0.39f;
const F32 APPEARANCE_MAX_ZOOM = 8.f;

// fidget constants
const F32 MIN_FIDGET_TIME = 8.f; // seconds
const F32 MAX_FIDGET_TIME = 20.f; // seconds

const S32 MAX_NUM_CHAT_POSITIONS = 10;
const F32 GROUND_TO_AIR_CAMERA_TRANSITION_TIME = 0.5f;
const F32 GROUND_TO_AIR_CAMERA_TRANSITION_START_TIME = 0.5f;

const F32 MAX_VELOCITY_AUTO_LAND_SQUARED = 4.f * 4.f;

const F32 MAX_FOCUS_OFFSET = 20.f;

const F32 OBJECT_EXTENTS_PADDING = 0.5f;

const F32 MIN_RADIUS_ALPHA_SIZZLE = 0.5f;

const F64 CHAT_AGE_FAST_RATE = 3.0;

// The agent instance.
LLAgent gAgent;

//
// Statics
//

BOOL LLAgent::phoenixPhantom = 0;
BOOL LLAgent::ignorePrejump = 0;

BOOL LLAgent::sFirstPersonBtnState;
BOOL LLAgent::sMouselookBtnState;
BOOL LLAgent::sThirdPersonBtnState;
BOOL LLAgent::sBuildBtnState;
BOOL LLAgent::PhoenixForceFly;

BOOL LLAgent::lure_show = FALSE;
std::string LLAgent::lure_name;
LLVector3d LLAgent::lure_posglobal;
U16 LLAgent::lure_global_x;
U16 LLAgent::lure_global_y;
int LLAgent::lure_x;
int LLAgent::lure_y;
int LLAgent::lure_z;
std::string LLAgent::lure_maturity;

const F32 LLAgent::TYPING_TIMEOUT_SECS = 5.f;

std::map<std::string, std::string> LLAgent::sTeleportErrorMessages;
std::map<std::string, std::string> LLAgent::sTeleportProgressMessages;

class LLAgentFriendObserver : public LLFriendObserver
{
public:
	LLAgentFriendObserver() {}
	virtual ~LLAgentFriendObserver() {}
	virtual void changed(U32 mask);
};

void LLAgentFriendObserver::changed(U32 mask)
{
	// if there's a change we're interested in.
	if((mask & (LLFriendObserver::POWERS)) != 0)
	{
		gAgent.friendsChanged();
	}
}

// ************************************************************
// Enabled this definition to compile a 'hacked' viewer that
// locally believes the end user has godlike powers.
// #define HACKED_GODLIKE_VIEWER
// For a toggled version, see viewer.h for the
// TOGGLE_HACKED_GODLIKE_VIEWER define, instead.
// ************************************************************

// Constructors and Destructors

// JC - Please try to make this order match the order in the header
// file.  Otherwise it's hard to find variables that aren't initialized.
//-----------------------------------------------------------------------------
// LLAgent()
//-----------------------------------------------------------------------------
LLAgent::LLAgent() :
	mDrawDistance( DEFAULT_FAR_PLANE ),

	mGroupPowers(0),
	mHideGroupTitle(FALSE),
	mGroupID(),

	mMapOriginX(0.F),
	mMapOriginY(0.F),
	mMapWidth(0),
	mMapHeight(0),

	mLookAt(NULL),
	mPointAt(NULL),

	mHUDTargetZoom(1.f),
	mHUDCurZoom(1.f),
	mInitialized(FALSE),
	mNumPendingQueries(0),
	mActiveCacheQueries(NULL),
	mForceMouselook(FALSE),

	mDoubleTapRunTimer(),
	mDoubleTapRunMode(DOUBLETAP_NONE),

	mbAlwaysRun(false),
	mbRunning(false),

	mAgentAccess(gSavedSettings),
	mTeleportState( TELEPORT_NONE ),
	mRegionp(NULL),

	mAgentOriginGlobal(),
	mPositionGlobal(),

	mDistanceTraveled(0.F),
	mLastPositionGlobal(LLVector3d::zero),

	mAvatarObject(NULL),

	mRenderState(0),
	mTypingTimer(),

	mCameraMode( CAMERA_MODE_THIRD_PERSON ),
	mLastCameraMode( CAMERA_MODE_THIRD_PERSON ),
	mViewsPushed(FALSE),

	mCustomAnim(FALSE),
	mShowAvatar(TRUE),
	mCameraAnimating( FALSE ),
	mAnimationCameraStartGlobal(),
	mAnimationFocusStartGlobal(),
	mAnimationTimer(),
	mAnimationDuration(0.33f),
	
	mCameraFOVZoomFactor(0.f),
	mCameraCurrentFOVZoomFactor(0.f),
	mCameraFocusOffset(),
	mCameraFOVDefault(DEFAULT_FIELD_OF_VIEW),

	mCameraOffsetDefault(),
	mCameraCollidePlane(),

	mCurrentCameraDistance(2.f),		// meters, set in init()
	mTargetCameraDistance(2.f),
	mCameraZoomFraction(1.f),			// deprecated
	mThirdPersonHeadOffset(0.f, 0.f, 1.f),
	mSitCameraEnabled(FALSE),
	mCameraSmoothingLastPositionGlobal(),
	mCameraSmoothingLastPositionAgent(),
	mCameraSmoothingStop(FALSE),

	mCameraUpVector(LLVector3::z_axis), // default is straight up

	mFocusOnAvatar(TRUE),
	mFocusGlobal(),
	mFocusTargetGlobal(),
	mFocusObject(NULL),
	mFocusObjectDist(0.f),
	mFocusObjectOffset(),
	mFocusDotRadius( 0.1f ),			// meters
	mTrackFocusObject(TRUE),
	mUIOffset(0.f),

	mFrameAgent(),

	mIsBusy(FALSE),

	mAtKey(0), // Either 1, 0, or -1... indicates that movement-key is pressed
	mWalkKey(0), // like AtKey, but causes less forward thrust
	mLeftKey(0),
	mUpKey(0),
	mYawKey(0.f),
	mPitchKey(0.f),

	mOrbitLeftKey(0.f),
	mOrbitRightKey(0.f),
	mOrbitUpKey(0.f),
	mOrbitDownKey(0.f),
	mOrbitInKey(0.f),
	mOrbitOutKey(0.f),

	mPanUpKey(0.f),
	mPanDownKey(0.f),
	mPanLeftKey(0.f),
	mPanRightKey(0.f),
	mPanInKey(0.f),
	mPanOutKey(0.f),

	mControlFlags(0x00000000),
	mbFlagsDirty(FALSE),
	mbFlagsNeedReset(FALSE),

	mbJump(FALSE),

	mAutoPilot(FALSE),
	mAutoPilotFlyOnStop(FALSE),
	mAutoPilotTargetGlobal(),
	mAutoPilotStopDistance(1.f),
	mAutoPilotUseRotation(FALSE),
	mAutoPilotTargetFacing(LLVector3::zero),
	mAutoPilotTargetDist(0.f),
	mAutoPilotNoProgressFrameCount(0),
	mAutoPilotRotationThreshold(0.f),
	mAutoPilotFinishedCallback(NULL),
	mAutoPilotCallbackData(NULL),
	
	mCapabilities(),

	mEffectColor(0.f, 1.f, 1.f, 1.f),

	mHaveHomePosition(FALSE),
	mHomeRegionHandle( 0 ),
	mNearChatRadius(CHAT_NORMAL_RADIUS / 2.f),

	mNextFidgetTime(0.f),
	mCurrentFidget(0),
	mFirstLogin(FALSE),
	mGenderChosen(FALSE),

	mAgentWearablesUpdateSerialNum(0),
	mWearablesLoaded(FALSE),
	mTextureCacheQueryID(0),
	mAppearanceSerialNum(0),
	mbTeleportKeepsLookAt(false)
{
	U32 i;
	for (i = 0; i < TOTAL_CONTROLS; i++)
	{
		mControlsTakenCount[i] = 0;
		mControlsTakenPassedOnCount[i] = 0;
	}

	mActiveCacheQueries = new S32[BAKED_NUM_INDICES];
	for (i = 0; i < (U32)BAKED_NUM_INDICES; i++)
	{
		mActiveCacheQueries[i] = 0;
	}

	mFollowCam.setMaxCameraDistantFromSubject( MAX_CAMERA_DISTANCE_FROM_AGENT );
	//PhoenixForceFly = gSavedSettings.getBOOL("PhoenixAlwaysFly");
	//gSavedSettings.getControl("PhoenixAlwaysFly")->getSignal()->connect(&updatePhoenixForceFly);
}
void LLAgent::updatePhoenixForceFly(const LLSD &data)
{
	PhoenixForceFly = data.asBoolean();
}

void LLAgent::updateIgnorePrejump(const LLSD &data)
{
	ignorePrejump = data.asBoolean();
}
// Requires gSavedSettings to be initialized.
//-----------------------------------------------------------------------------
// init()
//-----------------------------------------------------------------------------
void LLAgent::init()
{
	mDrawDistance = gSavedSettings.getF32("RenderFarClip");

	// *Note: this is where LLViewerCamera::getInstance() used to be constructed.

	LLViewerCamera::getInstance()->setView(DEFAULT_FIELD_OF_VIEW);
	// Leave at 0.1 meters until we have real near clip management
	LLViewerCamera::getInstance()->setNear(0.1f);
	LLViewerCamera::getInstance()->setFar(mDrawDistance);			// if you want to change camera settings, do so in camera.h
	LLViewerCamera::getInstance()->setAspect( gViewerWindow->getDisplayAspectRatio() );		// default, overridden in LLViewerWindow::reshape
	LLViewerCamera::getInstance()->setViewHeightInPixels(768);			// default, overridden in LLViewerWindow::reshape

	setFlying( gSavedSettings.getBOOL("FlyingAtExit") );

	mCameraFocusOffsetTarget = LLVector4(gSavedSettings.getVector3("CameraOffsetBuild"));
	mCameraOffsetDefault = gSavedSettings.getVector3("CameraOffsetDefault");
	mCameraCollidePlane.clearVec();
	mCurrentCameraDistance = mCameraOffsetDefault.magVec() * gSavedSettings.getF32("CameraOffsetScale");
	mTargetCameraDistance = mCurrentCameraDistance;
	mCameraZoomFraction = 1.f;
	mTrackFocusObject = gSavedSettings.getBOOL("TrackFocusObject");

//	LLDebugVarMessageBox::show("Camera Lag", &CAMERA_FOCUS_HALF_LIFE, 0.5f, 0.01f);

	mEffectColor = gSavedSettings.getColor4("EffectColor");
	ignorePrejump = gSavedSettings.getBOOL("PhoenixIgnoreFinishAnimation");
	gSavedSettings.getControl("PhoenixIgnoreFinishAnimation")->getSignal()->connect((boost::function<void (const LLSD &)>) &updateIgnorePrejump);
	PhoenixForceFly = gSavedSettings.getBOOL("PhoenixAlwaysFly");
	gSavedSettings.getControl("PhoenixAlwaysFly")->getSignal()->connect((boost::function<void (const LLSD &)>) &updatePhoenixForceFly);
	mBlockSpam=gSavedSettings.getBOOL("PhoenixBlockSpam");
	mInitialized = TRUE;
}

//-----------------------------------------------------------------------------
// cleanup()
//-----------------------------------------------------------------------------
void LLAgent::cleanup()
{
	setSitCamera(LLUUID::null);
	mAvatarObject = NULL;
	if(mLookAt)
	{
		mLookAt->markDead() ;
		mLookAt = NULL;
	}
	if(mPointAt)
	{
		mPointAt->markDead() ;
		mPointAt = NULL;
	}
	mRegionp = NULL;
	setFocusObject(NULL);
}

//-----------------------------------------------------------------------------
// LLAgent()
//-----------------------------------------------------------------------------
LLAgent::~LLAgent()
{
	cleanup();

	delete [] mActiveCacheQueries;
	mActiveCacheQueries = NULL;

	// *Note: this is where LLViewerCamera::getInstance() used to be deleted.
}

// Change camera back to third person, stop the autopilot,
// deselect stuff, etc.
//-----------------------------------------------------------------------------
// resetView()
//-----------------------------------------------------------------------------
void LLAgent::resetView(BOOL reset_camera, BOOL change_camera)
{
	if (mAutoPilot)
	{
		stopAutoPilot(TRUE);
	}

	if (!gNoRender)
	{
		//LLSelectMgr::getInstance()->unhighlightAll();

		// By popular request, keep land selection while walking around. JC
		// LLViewerParcelMgr::getInstance()->deselectLand();

		// force deselect when walking and attachment is selected
		// this is so people don't wig out when their avatar moves without animating
		if (LLSelectMgr::getInstance()->getSelection()->isAttachment())
		{
			LLSelectMgr::getInstance()->unhighlightAll();
			LLSelectMgr::getInstance()->deselectAll();
		}

		// Hide all popup menus
		gMenuHolder->hideMenus();
	}

	static BOOL* sFreezeTime = rebind_llcontrol<BOOL>("FreezeTime", &gSavedSettings, true);

	if (change_camera && !(*sFreezeTime))
	{
		changeCameraToDefault();
		
		if (LLViewerJoystick::getInstance()->getOverrideCamera())
		{
			handle_toggle_flycam();
		}

		// reset avatar mode from eventual residual motion
		if (LLToolMgr::getInstance()->inBuildMode())
		{
			LLViewerJoystick::getInstance()->moveAvatar(true);
		}

		gFloaterTools->close();
		
		gViewerWindow->showCursor();

		// Switch back to basic toolset
		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
	}


	if (reset_camera && !(*sFreezeTime))
	{
		if (!gViewerWindow->getLeftMouseDown() && cameraThirdPerson())
		{
			// leaving mouse-steer mode
			LLVector3 agent_at_axis = getAtAxis();
			agent_at_axis -= projected_vec(agent_at_axis, getReferenceUpVector());
			agent_at_axis.normalize();
			gAgent.resetAxes(lerp(getAtAxis(), agent_at_axis, LLCriticalDamp::getInterpolant(0.3f)));
		}

		setFocusOnAvatar(TRUE, ANIMATE);
	}

	mHUDTargetZoom = 1.f;
}

// Handle any actions that need to be performed when the main app gains focus
// (such as through alt-tab).
//-----------------------------------------------------------------------------
// onAppFocusGained()
//-----------------------------------------------------------------------------
void LLAgent::onAppFocusGained()
{
	if (CAMERA_MODE_MOUSELOOK == mCameraMode && gSavedSettings.getBOOL("PhoenixLeaveMouselookOnFocus"))
	{
		changeCameraToDefault();
		LLToolMgr::getInstance()->clearSavedTool();
	}
}


void LLAgent::ageChat()
{
	if (mAvatarObject.notNull())
	{
		// get amount of time since I last chatted
		F64 elapsed_time = (F64)mAvatarObject->mChatTimer.getElapsedTimeF32();
		// add in frame time * 3 (so it ages 4x)
		mAvatarObject->mChatTimer.setAge(elapsed_time + (F64)gFrameDTClamped * (CHAT_AGE_FAST_RATE - 1.0));
	}
}

// Allow camera to be moved somewhere other than behind avatar.
//-----------------------------------------------------------------------------
// unlockView()
//-----------------------------------------------------------------------------
void LLAgent::unlockView()
{
	if (getFocusOnAvatar())
	{
		if (mAvatarObject.notNull())
		{
			setFocusGlobal( LLVector3d::zero, mAvatarObject->mID );
		}
		setFocusOnAvatar(FALSE, FALSE);	// no animation
	}
}


//-----------------------------------------------------------------------------
// moveAt()
//-----------------------------------------------------------------------------
void LLAgent::moveAt(S32 direction, bool reset)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mAtKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_AT_POS | AGENT_CONTROL_FAST_AT);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_AT_NEG | AGENT_CONTROL_FAST_AT);
	}

	if (reset)
	{
		resetView();
	}
}

//-----------------------------------------------------------------------------
// moveAtNudge()
//-----------------------------------------------------------------------------
void LLAgent::moveAtNudge(S32 direction)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mWalkKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_NUDGE_AT_POS);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_NUDGE_AT_NEG);
	}

	resetView();
}

//-----------------------------------------------------------------------------
// moveLeft()
//-----------------------------------------------------------------------------
void LLAgent::moveLeft(S32 direction)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mLeftKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_LEFT_POS | AGENT_CONTROL_FAST_LEFT);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_LEFT_NEG | AGENT_CONTROL_FAST_LEFT);
	}

	resetView();
}

//-----------------------------------------------------------------------------
// moveLeftNudge()
//-----------------------------------------------------------------------------
void LLAgent::moveLeftNudge(S32 direction)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mLeftKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_NUDGE_LEFT_POS);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_NUDGE_LEFT_NEG);
	}

	resetView();
}

//-----------------------------------------------------------------------------
// moveUp()
//-----------------------------------------------------------------------------
void LLAgent::moveUp(S32 direction)
{
	// age chat timer so it fades more quickly when you are intentionally moving
	ageChat();

	setKey(direction, mUpKey);

	if (direction > 0)
	{
		setControlFlags(AGENT_CONTROL_UP_POS | AGENT_CONTROL_FAST_UP);
	}
	else if (direction < 0)
	{
		setControlFlags(AGENT_CONTROL_UP_NEG | AGENT_CONTROL_FAST_UP);

		if(!gSavedSettings.getBOOL("PhoenixCrouchToggleStatus"))
			resetView();
		else if(!gSavedSettings.getBOOL("PhoenixCrouchToggle"))
			resetView();
	}


}

//-----------------------------------------------------------------------------
// moveYaw()
//-----------------------------------------------------------------------------
void LLAgent::moveYaw(F32 mag, bool reset_view)
{
	mYawKey = mag;

	if (mag > 0)
	{
		setControlFlags(AGENT_CONTROL_YAW_POS);
	}
	else if (mag < 0)
	{
		setControlFlags(AGENT_CONTROL_YAW_NEG);
	}

	if (reset_view)
	{
		resetView();
	}
}

//-----------------------------------------------------------------------------
// movePitch()
//-----------------------------------------------------------------------------
void LLAgent::movePitch(F32 mag)
{
	mPitchKey = mag;

	if (mag > 0)
	{
		setControlFlags(AGENT_CONTROL_PITCH_POS );
	}
	else if (mag < 0)
	{
		setControlFlags(AGENT_CONTROL_PITCH_NEG);
	}
}


// Does this parcel allow you to fly?
BOOL LLAgent::canFly()
{
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0c)
	if (gRlvHandler.hasBehaviour(RLV_BHVR_FLY)) return FALSE;
// [/RLVa:KB]
	if (isGodlike()) return TRUE;
	//LGG always fly code
	if(PhoenixForceFly) return TRUE;
	LLViewerRegion* regionp = getRegion();
	if (regionp && regionp->getBlockFly()) return FALSE;
	
	LLParcel* parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
	if (!parcel) return FALSE;

	// Allow owners to fly on their own land.
	if (LLViewerParcelMgr::isParcelOwnedByAgent(parcel, GP_LAND_ALLOW_FLY))
	{
		return TRUE;
	}

	return parcel->getAllowFly();
}

//-----------------------------------------------------------------------------
// setPhantom()  lgg
//-----------------------------------------------------------------------------
void LLAgent::setPhantom(BOOL phantom)
{
	phoenixPhantom = phantom;
}
//-----------------------------------------------------------------------------
// getPhantom()  lgg
//-----------------------------------------------------------------------------
BOOL LLAgent::getPhantom()
{
	return phoenixPhantom;
}

//-----------------------------------------------------------------------------
// setFlying()
//----------------------------------------------------------------------------
void LLAgent::setFlying(BOOL fly)
{
	if (mAvatarObject.notNull())
	{
		if(mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_STANDUP) != mAvatarObject->mSignaledAnimations.end())
		{
			return;
		}

		// don't allow taking off while sitting
		if (fly && mAvatarObject->mIsSitting)
		{
			return;
		}
	}

	if (fly)
	{
// [RLVa:KB] - Checked: 2009-07-05 (RLVa-1.0.0c)
		if (gRlvHandler.hasBehaviour(RLV_BHVR_FLY))
		{
			return;
		}
// [/RLVa:KB]

		BOOL was_flying = getFlying();
		if (!canFly() && !was_flying)
		{
			// parcel doesn't let you start fly
			// gods can always fly
			// and it's OK if you're already flying
			make_ui_sound("UISndBadKeystroke");
			return;
		}
		if( !was_flying )
		{
			LLViewerStats::getInstance()->incStat(LLViewerStats::ST_FLY_COUNT);
		}
		setControlFlags(AGENT_CONTROL_FLY);
		gSavedSettings.setBOOL("FlyBtnState", TRUE);
	}
	else
	{
		clearControlFlags(AGENT_CONTROL_FLY);
		gSavedSettings.setBOOL("FlyBtnState", FALSE);
	}
	mbFlagsDirty = TRUE;
}


// UI based mechanism of setting fly state
//-----------------------------------------------------------------------------
// toggleFlying()
//-----------------------------------------------------------------------------
void LLAgent::toggleFlying()
{
	BOOL fly = !(mControlFlags & AGENT_CONTROL_FLY);

	setFlying( fly );
	resetView();
}


//-----------------------------------------------------------------------------
// togglePhantom()
//-----------------------------------------------------------------------------
void LLAgent::togglePhantom()
{
	BOOL phan = !(phoenixPhantom);

	setPhantom( phan );
}

//-----------------------------------------------------------------------------
// setRegion()
//-----------------------------------------------------------------------------
void LLAgent::setRegion(LLViewerRegion *regionp)
{
	bool teleport = true;

	llassert(regionp);
	if (mRegionp != regionp)
	{
		// std::string host_name;
		// host_name = regionp->getHost().getHostName();

		std::string ip = regionp->getHost().getString();
		llinfos << "Moving agent into region: " << regionp->getName()
				<< " located at " << ip << llendl;
		if (mRegionp)
		{
			// We've changed regions, we're now going to change our agent coordinate frame.
			mAgentOriginGlobal = regionp->getOriginGlobal();
			LLVector3d agent_offset_global = mRegionp->getOriginGlobal();

			LLVector3 delta;
			delta.setVec(regionp->getOriginGlobal() - mRegionp->getOriginGlobal());

			setPositionAgent(getPositionAgent() - delta);

			LLVector3 camera_position_agent = LLViewerCamera::getInstance()->getOrigin();
			LLViewerCamera::getInstance()->setOrigin(camera_position_agent - delta);

			// Update all of the regions.
			LLWorld::getInstance()->updateAgentOffset(agent_offset_global);

			// Hack to keep sky in the agent's region, otherwise it may get deleted - DJS 08/02/02
			// *TODO: possibly refactor into gSky->setAgentRegion(regionp)? -Brad
			if (gSky.mVOSkyp)
			{
				gSky.mVOSkyp->setRegion(regionp);
			}
			if (gSky.mVOGroundp)
			{
				gSky.mVOGroundp->setRegion(regionp);
			}

			// Notify windlight managers
			teleport = (gAgent.getTeleportState() != LLAgent::TELEPORT_NONE);
		}
		else
		{
			// First time initialization.
			// We've changed regions, we're now going to change our agent coordinate frame.
			mAgentOriginGlobal = regionp->getOriginGlobal();

			LLVector3 delta;
			delta.setVec(regionp->getOriginGlobal());

			setPositionAgent(getPositionAgent() - delta);
			LLVector3 camera_position_agent = LLViewerCamera::getInstance()->getOrigin();
			LLViewerCamera::getInstance()->setOrigin(camera_position_agent - delta);

			// Update all of the regions.
			LLWorld::getInstance()->updateAgentOffset(mAgentOriginGlobal);
		}
	}
	mRegionp = regionp;

	// Must shift hole-covering water object locations because local
	// coordinate frame changed.
	LLWorld::getInstance()->updateWaterObjects();

	// keep a list of regions we've been too
	// this is just an interesting stat, logged at the dataserver
	// we could trake this at the dataserver side, but that's harder
	U64 handle = regionp->getHandle();
	mRegionsVisited.insert(handle);

	LLSelectMgr::getInstance()->updateSelectionCenter();

	if (teleport)
	{
		LLEnvManagerNew::instance().onTeleport();
	}
	else
	{
		LLEnvManagerNew::instance().onRegionCrossing();
	}

	gHippoLimits->setLimits();
}


//-----------------------------------------------------------------------------
// getRegion()
//-----------------------------------------------------------------------------
LLViewerRegion *LLAgent::getRegion() const
{
	return mRegionp;
}


const LLHost& LLAgent::getRegionHost() const
{
	if (mRegionp)
	{
		return mRegionp->getHost();
	}
	else
	{
		return LLHost::invalid;
	}
}

//-----------------------------------------------------------------------------
// getSLURL()
// returns empty() if getRegion() == NULL
//-----------------------------------------------------------------------------
std::string LLAgent::getSLURL() const
{
	std::string slurl;
	LLViewerRegion *regionp = getRegion();
	if (regionp)
	{
		LLVector3d agentPos = getPositionGlobal();
		S32 x = llround( (F32)fmod( agentPos.mdV[VX], (F64)REGION_WIDTH_METERS ) );
		S32 y = llround( (F32)fmod( agentPos.mdV[VY], (F64)REGION_WIDTH_METERS ) );
		S32 z = llround( (F32)agentPos.mdV[VZ] );
		slurl = LLURLDispatcher::buildSLURL(regionp->getName(), x, y, z);
	}
	return slurl;
}

//-----------------------------------------------------------------------------
// inPrelude()
//-----------------------------------------------------------------------------
BOOL LLAgent::inPrelude()
{
	return mRegionp && mRegionp->isPrelude();
}


//-----------------------------------------------------------------------------
// canManageEstate()
//-----------------------------------------------------------------------------

BOOL LLAgent::canManageEstate() const
{
	return mRegionp && mRegionp->canManageEstate();
}

//-----------------------------------------------------------------------------
// sendMessage()
//-----------------------------------------------------------------------------
void LLAgent::sendMessage()
{
	if (gDisconnected)
	{
		llwarns << "Trying to send message when disconnected!" << llendl;
		return;
	}
	if (!mRegionp)
	{
		llerrs << "No region for agent yet!" << llendl;
	}
	gMessageSystem->sendMessage(mRegionp->getHost());
}


//-----------------------------------------------------------------------------
// sendReliableMessage()
//-----------------------------------------------------------------------------
void LLAgent::sendReliableMessage()
{
	if (gDisconnected)
	{
		lldebugs << "Trying to send message when disconnected!" << llendl;
		return;
	}
	if (!mRegionp)
	{
		lldebugs << "LLAgent::sendReliableMessage No region for agent yet, not sending message!" << llendl;
		return;
	}
	gMessageSystem->sendReliable(mRegionp->getHost());
}

//-----------------------------------------------------------------------------
// getVelocity()
//-----------------------------------------------------------------------------
LLVector3 LLAgent::getVelocity() const
{
	if (mAvatarObject.notNull())
	{
		return mAvatarObject->getVelocity();
	}
	else
	{
		return LLVector3::zero;
	}
}


//-----------------------------------------------------------------------------
// setPositionAgent()
//-----------------------------------------------------------------------------
void LLAgent::setPositionAgent(const LLVector3 &pos_agent)
{
	if (!pos_agent.isFinite())
	{
		llerrs << "setPositionAgent is not a number" << llendl;
	}

	if (mAvatarObject.notNull() && mAvatarObject->getParent())
	{
		LLVector3 pos_agent_sitting;
		LLVector3d pos_agent_d;
		LLViewerObject *parent = (LLViewerObject*)mAvatarObject->getParent();

		pos_agent_sitting = mAvatarObject->getPosition() * parent->getRotation() + parent->getPositionAgent();
		pos_agent_d.setVec(pos_agent_sitting);

		mFrameAgent.setOrigin(pos_agent_sitting);
		mPositionGlobal = pos_agent_d + mAgentOriginGlobal;
	}
	else
	{
		mFrameAgent.setOrigin(pos_agent);

		LLVector3d pos_agent_d;
		pos_agent_d.setVec(pos_agent);
		mPositionGlobal = pos_agent_d + mAgentOriginGlobal;
	}
}

//-----------------------------------------------------------------------------
// slamLookAt()
//-----------------------------------------------------------------------------
void LLAgent::slamLookAt(const LLVector3 &look_at)
{
	LLVector3 look_at_norm = look_at;
	look_at_norm.mV[VZ] = 0.f;
	look_at_norm.normalize();
	resetAxes(look_at_norm);
}

//-----------------------------------------------------------------------------
// getPositionGlobal()
//-----------------------------------------------------------------------------
const LLVector3d &LLAgent::getPositionGlobal() const
{
	if (mAvatarObject.notNull() && !mAvatarObject->mDrawable.isNull())
	{
		mPositionGlobal = getPosGlobalFromAgent(mAvatarObject->getRenderPosition());
	}
	else
	{
		mPositionGlobal = getPosGlobalFromAgent(mFrameAgent.getOrigin());
	}

	return mPositionGlobal;
}

//-----------------------------------------------------------------------------
// getPositionAgent()
//-----------------------------------------------------------------------------
const LLVector3 &LLAgent::getPositionAgent()
{
	if(mAvatarObject.notNull() && !mAvatarObject->mDrawable.isNull())
	{
		mFrameAgent.setOrigin(mAvatarObject->getRenderPosition());	
	}

	return mFrameAgent.getOrigin();
}

//-----------------------------------------------------------------------------
// getRegionsVisited()
//-----------------------------------------------------------------------------
S32 LLAgent::getRegionsVisited() const
{
	return mRegionsVisited.size();
}

//-----------------------------------------------------------------------------
// getDistanceTraveled()
//-----------------------------------------------------------------------------
F64 LLAgent::getDistanceTraveled() const
{
	return mDistanceTraveled;
}


//-----------------------------------------------------------------------------
// getPosAgentFromGlobal()
//-----------------------------------------------------------------------------
LLVector3 LLAgent::getPosAgentFromGlobal(const LLVector3d &pos_global) const
{
	LLVector3 pos_agent;
	pos_agent.setVec(pos_global - mAgentOriginGlobal);
	return pos_agent;
}


//-----------------------------------------------------------------------------
// getPosGlobalFromAgent()
//-----------------------------------------------------------------------------
LLVector3d LLAgent::getPosGlobalFromAgent(const LLVector3 &pos_agent) const
{
	LLVector3d pos_agent_d;
	pos_agent_d.setVec(pos_agent);
	return pos_agent_d + mAgentOriginGlobal;
}


//-----------------------------------------------------------------------------
// resetAxes()
//-----------------------------------------------------------------------------
void LLAgent::resetAxes()
{
	mFrameAgent.resetAxes();
}


// Copied from LLCamera::setOriginAndLookAt
// Look_at must be unit vector
//-----------------------------------------------------------------------------
// resetAxes()
//-----------------------------------------------------------------------------
void LLAgent::resetAxes(const LLVector3 &look_at)
{
	LLVector3	skyward = getReferenceUpVector();

	// if look_at has zero length, fail
	// if look_at and skyward are parallel, fail
	//
	// Test both of these conditions with a cross product.
	LLVector3 cross(look_at % skyward);
	if (cross.isNull())
	{
		llinfos << "LLAgent::resetAxes cross-product is zero" << llendl;
		return;
	}

	// Make sure look_at and skyward are not parallel
	// and neither are zero length
	LLVector3 left(skyward % look_at);
	LLVector3 up(look_at % left);

	mFrameAgent.setAxes(look_at, left, up);
}


//-----------------------------------------------------------------------------
// rotate()
//-----------------------------------------------------------------------------
void LLAgent::rotate(F32 angle, const LLVector3 &axis) 
{ 
	mFrameAgent.rotate(angle, axis); 
}


//-----------------------------------------------------------------------------
// rotate()
//-----------------------------------------------------------------------------
void LLAgent::rotate(F32 angle, F32 x, F32 y, F32 z) 
{ 
	mFrameAgent.rotate(angle, x, y, z); 
}


//-----------------------------------------------------------------------------
// rotate()
//-----------------------------------------------------------------------------
void LLAgent::rotate(const LLMatrix3 &matrix) 
{ 
	mFrameAgent.rotate(matrix); 
}


//-----------------------------------------------------------------------------
// rotate()
//-----------------------------------------------------------------------------
void LLAgent::rotate(const LLQuaternion &quaternion) 
{ 
	mFrameAgent.rotate(quaternion); 
}


//-----------------------------------------------------------------------------
// getReferenceUpVector()
//-----------------------------------------------------------------------------
LLVector3 LLAgent::getReferenceUpVector()
{
	// this vector is in the coordinate frame of the avatar's parent object, or the world if none
	LLVector3 up_vector = LLVector3::z_axis;
	if (mAvatarObject.notNull() && 
		mAvatarObject->getParent() &&
		mAvatarObject->mDrawable.notNull())
	{
		U32 camera_mode = mCameraAnimating ? mLastCameraMode : mCameraMode;
		// and in third person...
		if (camera_mode == CAMERA_MODE_THIRD_PERSON)
		{
			// make the up vector point to the absolute +z axis
			up_vector = up_vector * ~((LLViewerObject*)mAvatarObject->getParent())->getRenderRotation();
		}
		else if (camera_mode == CAMERA_MODE_MOUSELOOK)
		{
			// make the up vector point to the avatar's +z axis
			up_vector = up_vector * mAvatarObject->mDrawable->getRotation();
		}
	}

	return up_vector;
}


// Radians, positive is forward into ground
//-----------------------------------------------------------------------------
// pitch()
//-----------------------------------------------------------------------------
void LLAgent::pitch(F32 angle)
{
	// don't let user pitch if pointed almost all the way down or up
	mFrameAgent.pitch(clampPitchToLimits(angle));
}


// Radians, positive is forward into ground
//-----------------------------------------------------------------------------
// clampPitchToLimits()
//-----------------------------------------------------------------------------
F32 LLAgent::clampPitchToLimits(F32 angle)
{
	// A dot B = mag(A) * mag(B) * cos(angle between A and B)
	// so... cos(angle between A and B) = A dot B / mag(A) / mag(B)
	//                                  = A dot B for unit vectors

	LLVector3 skyward = getReferenceUpVector();

	F32			look_down_limit = 179.f * DEG_TO_RAD;
	F32			look_up_limit = 1.f * DEG_TO_RAD;

	F32 angle_from_skyward = acos( mFrameAgent.getAtAxis() * skyward );
	/*
	if (mAvatarObject.notNull() && mAvatarObject->mIsSitting)
	{
		look_down_limit = 130.f * DEG_TO_RAD;
	}
	else
	{
		look_down_limit = 170.f * DEG_TO_RAD;
	}
	*/
	// clamp pitch to limits
	if ((angle >= 0.f) && (angle_from_skyward + angle > look_down_limit))
	{
		angle = look_down_limit - angle_from_skyward;
	}
	else if ((angle < 0.f) && (angle_from_skyward + angle < look_up_limit))
	{
		angle = look_up_limit - angle_from_skyward;
	}
   
    return angle;
}


//-----------------------------------------------------------------------------
// roll()
//-----------------------------------------------------------------------------
void LLAgent::roll(F32 angle)
{
	mFrameAgent.roll(angle);
}


//-----------------------------------------------------------------------------
// yaw()
//-----------------------------------------------------------------------------
void LLAgent::yaw(F32 angle)
{
	if (!rotateGrabbed())
	{
		mFrameAgent.rotate(angle, getReferenceUpVector());
	}
}


// Returns a quat that represents the rotation of the agent in the absolute frame
//-----------------------------------------------------------------------------
// getQuat()
//-----------------------------------------------------------------------------
LLQuaternion LLAgent::getQuat() const
{
	return mFrameAgent.getQuaternion();
}


//-----------------------------------------------------------------------------
// calcFocusOffset()
//-----------------------------------------------------------------------------
LLVector3 LLAgent::calcFocusOffset(LLViewerObject *object, LLVector3 original_focus_point, S32 x, S32 y)
{
	LLMatrix4 obj_matrix = object->getRenderMatrix();
	LLQuaternion obj_rot = object->getRenderRotation();
	LLVector3 obj_pos = object->getRenderPosition();

	BOOL is_avatar = object->isAvatar();
	// if is avatar - don't do any funk heuristics to position the focal point
	// see DEV-30589
	if (is_avatar)
	{
		return original_focus_point - obj_pos;
	}

	
	LLQuaternion inv_obj_rot = ~obj_rot; // get inverse of rotation
	LLVector3 object_extents = object->getScale();
	// make sure they object extents are non-zero
	object_extents.clamp(0.001f, F32_MAX);

	// obj_to_cam_ray is unit vector pointing from object center to camera, in the coordinate frame of the object
	LLVector3 obj_to_cam_ray = obj_pos - LLViewerCamera::getInstance()->getOrigin();
	obj_to_cam_ray.rotVec(inv_obj_rot);
	obj_to_cam_ray.normalize();

	// obj_to_cam_ray_proportions are the (positive) ratios of 
	// the obj_to_cam_ray x,y,z components with the x,y,z object dimensions.
	LLVector3 obj_to_cam_ray_proportions;
	obj_to_cam_ray_proportions.mV[VX] = llabs(obj_to_cam_ray.mV[VX] / object_extents.mV[VX]);
	obj_to_cam_ray_proportions.mV[VY] = llabs(obj_to_cam_ray.mV[VY] / object_extents.mV[VY]);
	obj_to_cam_ray_proportions.mV[VZ] = llabs(obj_to_cam_ray.mV[VZ] / object_extents.mV[VZ]);

	// find the largest ratio stored in obj_to_cam_ray_proportions
	// this corresponds to the object's local axial plane (XY, YZ, XZ) that is *most* facing the camera
	LLVector3 longest_object_axis;
	// is x-axis longest?
	if (obj_to_cam_ray_proportions.mV[VX] > obj_to_cam_ray_proportions.mV[VY] 
		&& obj_to_cam_ray_proportions.mV[VX] > obj_to_cam_ray_proportions.mV[VZ])
	{
		// then grab it
		longest_object_axis.setVec(obj_matrix.getFwdRow4());
	}
	// is y-axis longest?
	else if (obj_to_cam_ray_proportions.mV[VY] > obj_to_cam_ray_proportions.mV[VZ])
	{
		// then grab it
		longest_object_axis.setVec(obj_matrix.getLeftRow4());
	}
	// otherwise, use z axis
	else
	{
		longest_object_axis.setVec(obj_matrix.getUpRow4());
	}

	// Use this axis as the normal to project mouse click on to plane with that normal, at the object center.
	// This generates a point behind the mouse cursor that is approximately in the middle of the object in
	// terms of depth.  
	// We do this to allow the camera rotation tool to "tumble" the object by rotating the camera.
	// If the focus point were the object surface under the mouse, camera rotation would introduce an undesirable
	// eccentricity to the object orientation
	LLVector3 focus_plane_normal(longest_object_axis);
	focus_plane_normal.normalize();

	LLVector3d focus_pt_global;
	gViewerWindow->mousePointOnPlaneGlobal(focus_pt_global, x, y, gAgent.getPosGlobalFromAgent(obj_pos), focus_plane_normal);
	LLVector3 focus_pt = gAgent.getPosAgentFromGlobal(focus_pt_global);

	// find vector from camera to focus point in object space
	LLVector3 camera_to_focus_vec = focus_pt - LLViewerCamera::getInstance()->getOrigin();
	camera_to_focus_vec.rotVec(inv_obj_rot);

	// find vector from object origin to focus point in object coordinates
	LLVector3 focus_offset_from_object_center = focus_pt - obj_pos;
	// convert to object-local space
	focus_offset_from_object_center.rotVec(inv_obj_rot);

	// We need to project the focus point back into the bounding box of the focused object.
	// Do this by calculating the XYZ scale factors needed to get focus offset back in bounds along the camera_focus axis
	LLVector3 clip_fraction;

	// for each axis...
	for (U32 axis = VX; axis <= VZ; axis++)
	{
		//...calculate distance that focus offset sits outside of bounding box along that axis...
		//NOTE: dist_out_of_bounds keeps the sign of focus_offset_from_object_center 
		F32 dist_out_of_bounds;
		if (focus_offset_from_object_center.mV[axis] > 0.f)
		{
			dist_out_of_bounds = llmax(0.f, focus_offset_from_object_center.mV[axis] - (object_extents.mV[axis] * 0.5f));
		}
		else
		{
			dist_out_of_bounds = llmin(0.f, focus_offset_from_object_center.mV[axis] + (object_extents.mV[axis] * 0.5f));
		}

		//...then calculate the scale factor needed to push camera_to_focus_vec back in bounds along current axis
		if (llabs(camera_to_focus_vec.mV[axis]) < 0.0001f)
		{
			// don't divide by very small number
			clip_fraction.mV[axis] = 0.f;
		}
		else
		{
			clip_fraction.mV[axis] = dist_out_of_bounds / camera_to_focus_vec.mV[axis];
		}
	}

	LLVector3 abs_clip_fraction = clip_fraction;
	abs_clip_fraction.abs();

	// find axis of focus offset that is *most* outside the bounding box and use that to
	// rescale focus offset to inside object extents
	if (abs_clip_fraction.mV[VX] > abs_clip_fraction.mV[VY]
		&& abs_clip_fraction.mV[VX] > abs_clip_fraction.mV[VZ])
	{
		focus_offset_from_object_center -= clip_fraction.mV[VX] * camera_to_focus_vec;
	}
	else if (abs_clip_fraction.mV[VY] > abs_clip_fraction.mV[VZ])
	{
		focus_offset_from_object_center -= clip_fraction.mV[VY] * camera_to_focus_vec;
	}
	else
	{
		focus_offset_from_object_center -= clip_fraction.mV[VZ] * camera_to_focus_vec;
	}

	// convert back to world space
	focus_offset_from_object_center.rotVec(obj_rot);
	
	// now, based on distance of camera from object relative to object size
	// push the focus point towards the near surface of the object when (relatively) close to the objcet
	// or keep the focus point in the object middle when (relatively) far
	// NOTE: leave focus point in middle of avatars, since the behavior you want when alt-zooming on avatars
	// is almost always "tumble about middle" and not "spin around surface point"
	if (!is_avatar) 
	{
		LLVector3 obj_rel = original_focus_point - object->getRenderPosition();
		
		//now that we have the object relative position, we should bias toward the center of the object 
		//based on the distance of the camera to the focus point vs. the distance of the camera to the focus

		F32 relDist = llabs(obj_rel * LLViewerCamera::getInstance()->getAtAxis());
		F32 viewDist = dist_vec(obj_pos + obj_rel, LLViewerCamera::getInstance()->getOrigin());


		LLBBox obj_bbox = object->getBoundingBoxAgent();
		F32 bias = 0.f;

		// virtual_camera_pos is the camera position we are simulating by backing the camera off
		// and adjusting the FOV
		LLVector3 virtual_camera_pos = gAgent.getPosAgentFromGlobal(mFocusTargetGlobal + (getCameraPositionGlobal() - mFocusTargetGlobal) / (1.f + mCameraFOVZoomFactor));

		// if the camera is inside the object (large, hollow objects, for example)
		// leave focus point all the way to destination depth, away from object center
		if(!obj_bbox.containsPointAgent(virtual_camera_pos))
		{
			// perform magic number biasing of focus point towards surface vs. planar center
			bias = clamp_rescale(relDist/viewDist, 0.1f, 0.7f, 0.0f, 1.0f);
			obj_rel = lerp(focus_offset_from_object_center, obj_rel, bias);
		}
			
		focus_offset_from_object_center = obj_rel;
	}

	return focus_offset_from_object_center;
}

//-----------------------------------------------------------------------------
// calcCameraMinDistance()
//-----------------------------------------------------------------------------
BOOL LLAgent::calcCameraMinDistance(F32 &obj_min_distance)
{
	BOOL soft_limit = FALSE; // is the bounding box to be treated literally (volumes) or as an approximation (avatars)

	if (!mFocusObject || mFocusObject->isDead())
	{
		obj_min_distance = 0.f;
		return TRUE;
	}

	if (mFocusObject->mDrawable.isNull())
	{
#ifdef LL_RELEASE_FOR_DOWNLOAD
		llwarns << "Focus object with no drawable!" << llendl;
#else
		mFocusObject->dump();
		llerrs << "Focus object with no drawable!" << llendl;
#endif
		obj_min_distance = 0.f;
		return TRUE;
	}
	
	LLQuaternion inv_object_rot = ~mFocusObject->getRenderRotation();
	LLVector3 target_offset_origin = mFocusObjectOffset;
	LLVector3 camera_offset_target(getCameraPositionAgent() - getPosAgentFromGlobal(mFocusTargetGlobal));

	// convert offsets into object local space
	camera_offset_target.rotVec(inv_object_rot);
	target_offset_origin.rotVec(inv_object_rot);

	// push around object extents based on target offset
	LLVector3 object_extents = mFocusObject->getScale();
	if (mFocusObject->isAvatar())
	{
		// fudge factors that lets you zoom in on avatars a bit more (which don't do FOV zoom)
		object_extents.mV[VX] *= AVATAR_ZOOM_MIN_X_FACTOR;
		object_extents.mV[VY] *= AVATAR_ZOOM_MIN_Y_FACTOR;
		object_extents.mV[VZ] *= AVATAR_ZOOM_MIN_Z_FACTOR;
		soft_limit = TRUE;
	}
	LLVector3 abs_target_offset = target_offset_origin;
	abs_target_offset.abs();

	LLVector3 target_offset_dir = target_offset_origin;
	F32 object_radius = mFocusObject->getVObjRadius();

	BOOL target_outside_object_extents = FALSE;

	for (U32 i = VX; i <= VZ; i++)
	{
		if (abs_target_offset.mV[i] * 2.f > object_extents.mV[i] + OBJECT_EXTENTS_PADDING)
		{
			target_outside_object_extents = TRUE;
		}
		if (camera_offset_target.mV[i] > 0.f)
		{
			object_extents.mV[i] -= target_offset_origin.mV[i] * 2.f;
		}
		else
		{
			object_extents.mV[i] += target_offset_origin.mV[i] * 2.f;
		}
	}

	// don't shrink the object extents so far that the object inverts
	object_extents.clamp(0.001f, F32_MAX);

	// move into first octant
	LLVector3 camera_offset_target_abs_norm = camera_offset_target;
	camera_offset_target_abs_norm.abs();
	// make sure offset is non-zero
	camera_offset_target_abs_norm.clamp(0.001f, F32_MAX);
	camera_offset_target_abs_norm.normalize();

	// find camera position relative to normalized object extents
	LLVector3 camera_offset_target_scaled = camera_offset_target_abs_norm;
	camera_offset_target_scaled.mV[VX] /= object_extents.mV[VX];
	camera_offset_target_scaled.mV[VY] /= object_extents.mV[VY];
	camera_offset_target_scaled.mV[VZ] /= object_extents.mV[VZ];

	if (camera_offset_target_scaled.mV[VX] > camera_offset_target_scaled.mV[VY] && 
		camera_offset_target_scaled.mV[VX] > camera_offset_target_scaled.mV[VZ])
	{
		if (camera_offset_target_abs_norm.mV[VX] < 0.001f)
		{
			obj_min_distance = object_extents.mV[VX] * 0.5f;
		}
		else
		{
			obj_min_distance = object_extents.mV[VX] * 0.5f / camera_offset_target_abs_norm.mV[VX];
		}
	}
	else if (camera_offset_target_scaled.mV[VY] > camera_offset_target_scaled.mV[VZ])
	{
		if (camera_offset_target_abs_norm.mV[VY] < 0.001f)
		{
			obj_min_distance = object_extents.mV[VY] * 0.5f;
		}
		else
		{
			obj_min_distance = object_extents.mV[VY] * 0.5f / camera_offset_target_abs_norm.mV[VY];
		}
	}
	else
	{
		if (camera_offset_target_abs_norm.mV[VZ] < 0.001f)
		{
			obj_min_distance = object_extents.mV[VZ] * 0.5f;
		}
		else
		{
			obj_min_distance = object_extents.mV[VZ] * 0.5f / camera_offset_target_abs_norm.mV[VZ];
		}
	}

	LLVector3 object_split_axis;
	LLVector3 target_offset_scaled = target_offset_origin;
	target_offset_scaled.abs();
	target_offset_scaled.normalize();
	target_offset_scaled.mV[VX] /= object_extents.mV[VX];
	target_offset_scaled.mV[VY] /= object_extents.mV[VY];
	target_offset_scaled.mV[VZ] /= object_extents.mV[VZ];

	if (target_offset_scaled.mV[VX] > target_offset_scaled.mV[VY] && 
		target_offset_scaled.mV[VX] > target_offset_scaled.mV[VZ])
	{
		object_split_axis = LLVector3::x_axis;
	}
	else if (target_offset_scaled.mV[VY] > target_offset_scaled.mV[VZ])
	{
		object_split_axis = LLVector3::y_axis;
	}
	else
	{
		object_split_axis = LLVector3::z_axis;
	}

	LLVector3 camera_offset_object(getCameraPositionAgent() - mFocusObject->getPositionAgent());

	// length projected orthogonal to target offset
	F32 camera_offset_dist = (camera_offset_object - target_offset_dir * (camera_offset_object * target_offset_dir)).magVec();

	// calculate whether the target point would be "visible" if it were outside the bounding box
	// on the opposite of the splitting plane defined by object_split_axis;
	BOOL exterior_target_visible = FALSE;
	if (camera_offset_dist > object_radius)
	{
		// target is visible from camera, so turn off fov zoom
		exterior_target_visible = TRUE;
	}

	F32 camera_offset_clip = camera_offset_object * object_split_axis;
	F32 target_offset_clip = target_offset_dir * object_split_axis;

	// target has moved outside of object extents
	// check to see if camera and target are on same side 
	if (target_outside_object_extents)
	{
		if (camera_offset_clip > 0.f && target_offset_clip > 0.f)
		{
			return FALSE;
		}
		else if (camera_offset_clip < 0.f && target_offset_clip < 0.f)
		{
			return FALSE;
		}
	}

	// clamp obj distance to diagonal of 10 by 10 cube
	obj_min_distance = llmin(obj_min_distance, 10.f * F_SQRT3);

	obj_min_distance += LLViewerCamera::getInstance()->getNear() + (soft_limit ? 0.1f : 0.2f);
	
	return TRUE;
}

F32 LLAgent::getCameraZoomFraction()
{
	// 0.f -> camera zoomed all the way out
	// 1.f -> camera zoomed all the way in
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		// already [0,1]
		return mHUDTargetZoom;
	}
	else if (gSavedSettings.getBOOL("PhoenixDisableMinZoomDist"))
	{
		return mCameraZoomFraction;
	}
	else if (mFocusOnAvatar && cameraThirdPerson())
	{
		return clamp_rescale(mCameraZoomFraction, MIN_ZOOM_FRACTION, MAX_ZOOM_FRACTION, 1.f, 0.f);
	}
	else if (cameraCustomizeAvatar())
	{
		F32 distance = (F32)mCameraFocusOffsetTarget.magVec();
		return clamp_rescale(distance, APPEARANCE_MIN_ZOOM, APPEARANCE_MAX_ZOOM, 1.f, 0.f );
	}
	else
	{
		F32 min_zoom;
		//const F32 DIST_FUDGE = 16.f; // meters
		F32 max_zoom = 65535.f*4.f;//llmin(mDrawDistance - DIST_FUDGE, 
						//		LLWorld::getInstance()->getRegionWidthInMeters() - DIST_FUDGE,
						//		MAX_CAMERA_DISTANCE_FROM_AGENT);

		F32 distance = (F32)mCameraFocusOffsetTarget.magVec();
		if (mFocusObject.notNull())
		{
			if (mFocusObject->isAvatar())
			{
				min_zoom = AVATAR_MIN_ZOOM;
			}
			else
			{
				min_zoom = OBJECT_MIN_ZOOM;
			}
		}
		else
		{
			min_zoom = LAND_MIN_ZOOM;
		}

		return clamp_rescale(distance, min_zoom, max_zoom, 1.f, 0.f);
	}
}

void LLAgent::setCameraZoomFraction(F32 fraction)
{
	// 0.f -> camera zoomed all the way out
	// 1.f -> camera zoomed all the way in
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	BOOL disable_min = gSavedSettings.getBOOL("PhoenixDisableMinZoomDist");
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		mHUDTargetZoom = fraction;
	}
	else if (mFocusOnAvatar && cameraThirdPerson() && !disable_min)
	{
		mCameraZoomFraction = rescale(fraction, 0.f, 1.f, MAX_ZOOM_FRACTION, MIN_ZOOM_FRACTION);
	}
	else if (cameraCustomizeAvatar())
	{
		LLVector3d camera_offset_dir = mCameraFocusOffsetTarget;
		camera_offset_dir.normalize();
		mCameraFocusOffsetTarget = camera_offset_dir * rescale(fraction, 0.f, 1.f, APPEARANCE_MAX_ZOOM, APPEARANCE_MIN_ZOOM);
	}
	else
	{
		
		F32 min_zoom = LAND_MIN_ZOOM;
		//const F32 DIST_FUDGE = 16.f; // meters
		//F32 max_zoom = llmin(mDrawDistance - DIST_FUDGE, 
		//						LLWorld::getInstance()->getRegionWidthInMeters() - DIST_FUDGE,
		//						MAX_CAMERA_DISTANCE_FROM_AGENT);

		if(!disable_min)
		{
			if (mFocusObject.notNull())
			{
				if (mFocusObject->isAvatar())
				{
					min_zoom = AVATAR_MIN_ZOOM;
				}
				else
				{
					min_zoom = OBJECT_MIN_ZOOM;
				}
			}
		}
		else
		{
			min_zoom = 0.f;
		}

		LLVector3d camera_offset_dir = mCameraFocusOffsetTarget;
		camera_offset_dir.normalize();
		//mCameraFocusOffsetTarget = camera_offset_dir * rescale(fraction, 0.f, 1.f, max_zoom, min_zoom);
		mCameraFocusOffsetTarget = camera_offset_dir * rescale(fraction, 0.f, 65535.*4., 1.f, min_zoom);
	}
	startCameraAnimation();
}


//-----------------------------------------------------------------------------
// cameraOrbitAround()
//-----------------------------------------------------------------------------
void LLAgent::cameraOrbitAround(const F32 radians)
{
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		// do nothing for hud selection
	}
	else if (mFocusOnAvatar && (mCameraMode == CAMERA_MODE_THIRD_PERSON || mCameraMode == CAMERA_MODE_FOLLOW))
	{
		mFrameAgent.rotate(radians, getReferenceUpVector());
	}
	else
	{
		mCameraFocusOffsetTarget.rotVec(radians, 0.f, 0.f, 1.f);
		
		cameraZoomIn(1.f);
	}
}


//-----------------------------------------------------------------------------
// cameraOrbitOver()
//-----------------------------------------------------------------------------
void LLAgent::cameraOrbitOver(const F32 angle)
{
	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		// do nothing for hud selection
	}
	else if (mFocusOnAvatar && mCameraMode == CAMERA_MODE_THIRD_PERSON)
	{
		pitch(angle);
	}
	else
	{
		LLVector3 camera_offset_unit(mCameraFocusOffsetTarget);
		camera_offset_unit.normalize();

		F32 angle_from_up = acos( camera_offset_unit * getReferenceUpVector() );

		LLVector3d left_axis;
		left_axis.setVec(LLViewerCamera::getInstance()->getLeftAxis());
		F32 new_angle = llclamp(angle_from_up - angle, 1.f * DEG_TO_RAD, 179.f * DEG_TO_RAD);
		mCameraFocusOffsetTarget.rotVec(angle_from_up - new_angle, left_axis);

		cameraZoomIn(1.f);
	}
}

//-----------------------------------------------------------------------------
// cameraZoomIn()
//-----------------------------------------------------------------------------
void LLAgent::cameraZoomIn(const F32 fraction)
{
	if (gDisconnected)
	{
		return;
	}

	LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
	if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
	{
		// just update hud zoom level
		mHUDTargetZoom /= fraction;
		return;
	}


	LLVector3d	camera_offset(mCameraFocusOffsetTarget);
	LLVector3d	camera_offset_unit(mCameraFocusOffsetTarget);
	F32 min_zoom = 0.f;//LAND_MIN_ZOOM;
	F32 current_distance = (F32)camera_offset_unit.normalize();
	F32 new_distance = current_distance * fraction;

	
	//Phoenix:
	//So many darned limits!
	//~Zwag
	// Don't move through focus point
	
        if (!gSavedSettings.getBOOL("PhoenixDisableMinZoomDist"))
        {
		if (mFocusObject)
		{
			LLVector3 camera_offset_dir((F32)camera_offset_unit.mdV[VX], (F32)camera_offset_unit.mdV[VY], (F32)camera_offset_unit.mdV[VZ]);

			if (mFocusObject->isAvatar())
			{
				calcCameraMinDistance(min_zoom);
			}
			else
			{
				min_zoom = OBJECT_MIN_ZOOM;
			}
		}
	new_distance = llmax(new_distance, min_zoom); 
	}

		// Don't zoom too far back
		const F32 DIST_FUDGE = 16.f; // meters
	F32 max_distance = /*llmin(mDrawDistance*/ INT_MAX - DIST_FUDGE//, 
							 /*LLWorld::getInstance()->getRegionWidthInMeters() - DIST_FUDGE )*/;

	if (new_distance > max_distance)
	{
		new_distance = max_distance;

		/*
		// Unless camera is unlocked
		if (!LLViewerCamera::sDisableCameraConstraints)
		{
			return;
		}
		*/
	}
	/*
	if( cameraCustomizeAvatar() )
	{
		new_distance = llclamp( new_distance, APPEARANCE_MIN_ZOOM, APPEARANCE_MAX_ZOOM );
	}
	*/

	mCameraFocusOffsetTarget = new_distance * camera_offset_unit;
}

//-----------------------------------------------------------------------------
// cameraOrbitIn()
//-----------------------------------------------------------------------------
void LLAgent::cameraOrbitIn(const F32 meters)
{
	if (mFocusOnAvatar && mCameraMode == CAMERA_MODE_THIRD_PERSON)
	{
		F32 camera_offset_dist = llmax(0.001f, mCameraOffsetDefault.magVec() * gSavedSettings.getF32("CameraOffsetScale"));
		
		mCameraZoomFraction = (mTargetCameraDistance - meters) / camera_offset_dist;

		static BOOL* sFreezeTime = rebind_llcontrol<BOOL>("FreezeTime", &gSavedSettings, true);

		if (!(*sFreezeTime) && mCameraZoomFraction < MIN_ZOOM_FRACTION && meters > 0.f)
		{
			// No need to animate, camera is already there.
			changeCameraToMouselook(FALSE);
		}

		mCameraZoomFraction = llclamp(mCameraZoomFraction, MIN_ZOOM_FRACTION, MAX_ZOOM_FRACTION);
	}
	else
	{
		LLVector3d	camera_offset(mCameraFocusOffsetTarget);
		LLVector3d	camera_offset_unit(mCameraFocusOffsetTarget);
		F32 current_distance = (F32)camera_offset_unit.normalize();
		F32 new_distance = current_distance - meters;
		/*
		F32 min_zoom = LAND_MIN_ZOOM;
		
		// Don't move through focus point
		if (mFocusObject.notNull())
		{
			if (mFocusObject->isAvatar())
			{
				min_zoom = AVATAR_MIN_ZOOM;
			}
			else
			{
				min_zoom = OBJECT_MIN_ZOOM;
			}
		}

		new_distance = llmax(new_distance, min_zoom);

		// Don't zoom too far back
		const F32 DIST_FUDGE = 16.f; // meters
		F32 max_distance = llmin(mDrawDistance - DIST_FUDGE, 
								 LLWorld::getInstance()->getRegionWidthInMeters() - DIST_FUDGE );

		if (new_distance > max_distance)
		{
			// Unless camera is unlocked
			if (!gSavedSettings.getBOOL("DisableCameraConstraints"))
			{
				return;
			}
		}

		if( CAMERA_MODE_CUSTOMIZE_AVATAR == getCameraMode() )
		{
			new_distance = llclamp( new_distance, APPEARANCE_MIN_ZOOM, APPEARANCE_MAX_ZOOM );
		}
		*/
		// Compute new camera offset
		mCameraFocusOffsetTarget = new_distance * camera_offset_unit;
		cameraZoomIn(1.f);
	}
}


//-----------------------------------------------------------------------------
// cameraPanIn()
//-----------------------------------------------------------------------------
void LLAgent::cameraPanIn(F32 meters)
{
	LLVector3d at_axis;
	at_axis.setVec(LLViewerCamera::getInstance()->getAtAxis());

	mFocusTargetGlobal += meters * at_axis;
	mFocusGlobal = mFocusTargetGlobal;
	// don't enforce zoom constraints as this is the only way for users to get past them easily
	updateFocusOffset();
	// NOTE: panning movements expect the camera to move exactly with the focus target, not animated behind -Nyx
	mCameraSmoothingLastPositionGlobal = calcCameraPositionTargetGlobal();
}

//-----------------------------------------------------------------------------
// cameraPanLeft()
//-----------------------------------------------------------------------------
void LLAgent::cameraPanLeft(F32 meters)
{
	LLVector3d left_axis;
	left_axis.setVec(LLViewerCamera::getInstance()->getLeftAxis());

	mFocusTargetGlobal += meters * left_axis;
	mFocusGlobal = mFocusTargetGlobal;

	// disable smoothing for camera pan, which causes some residents unhappiness
	mCameraSmoothingStop = TRUE;
	
	cameraZoomIn(1.f);
	updateFocusOffset();
	// NOTE: panning movements expect the camera to move exactly with the focus target, not animated behind - Nyx
	mCameraSmoothingLastPositionGlobal = calcCameraPositionTargetGlobal();
}

//-----------------------------------------------------------------------------
// cameraPanUp()
//-----------------------------------------------------------------------------
void LLAgent::cameraPanUp(F32 meters)
{
	LLVector3d up_axis;
	up_axis.setVec(LLViewerCamera::getInstance()->getUpAxis());

	mFocusTargetGlobal += meters * up_axis;
	mFocusGlobal = mFocusTargetGlobal;

	// disable smoothing for camera pan, which causes some residents unhappiness
	mCameraSmoothingStop = TRUE;

	cameraZoomIn(1.f);
	updateFocusOffset();
	// NOTE: panning movements expect the camera to move exactly with the focus target, not animated behind -Nyx
	mCameraSmoothingLastPositionGlobal = calcCameraPositionTargetGlobal();
}

//-----------------------------------------------------------------------------
// setKey()
//-----------------------------------------------------------------------------
void LLAgent::setKey(const S32 direction, S32 &key)
{
	if (direction > 0)
	{
		key = 1;
	}
	else if (direction < 0)
	{
		key = -1;
	}
	else
	{
		key = 0;
	}
}


//-----------------------------------------------------------------------------
// getControlFlags()
//-----------------------------------------------------------------------------
U32 LLAgent::getControlFlags()
{
/*
	// HACK -- avoids maintenance of control flags when camera mode is turned on or off,
	// only worries about it when the flags are measured
	if (mCameraMode == CAMERA_MODE_MOUSELOOK) 
	{
		if ( !(mControlFlags & AGENT_CONTROL_MOUSELOOK) )
		{
			mControlFlags |= AGENT_CONTROL_MOUSELOOK;
		}
	}
*/
        if(LLAgent::ignorePrejump)
                return mControlFlags | AGENT_CONTROL_FINISH_ANIM;
        else
                return mControlFlags;
}

//-----------------------------------------------------------------------------
// setControlFlags()
//-----------------------------------------------------------------------------
void LLAgent::setControlFlags(U32 mask)
{
	U32 old_flags = mControlFlags;
	mControlFlags |= mask;
	mbFlagsDirty = mControlFlags ^ old_flags;
}


//-----------------------------------------------------------------------------
// clearControlFlags()
//-----------------------------------------------------------------------------
void LLAgent::clearControlFlags(U32 mask)
{
	U32 old_flags = mControlFlags;
	mControlFlags &= ~mask;
	if (old_flags != mControlFlags)
	{
		mbFlagsDirty = TRUE;
	}
}

//-----------------------------------------------------------------------------
// controlFlagsDirty()
//-----------------------------------------------------------------------------
BOOL LLAgent::controlFlagsDirty() const
{
	return mbFlagsDirty;
}

//-----------------------------------------------------------------------------
// enableControlFlagReset()
//-----------------------------------------------------------------------------
void LLAgent::enableControlFlagReset()
{
	mbFlagsNeedReset = TRUE;
}

//-----------------------------------------------------------------------------
// resetControlFlags()
//-----------------------------------------------------------------------------
void LLAgent::resetControlFlags()
{
	if (mbFlagsNeedReset)
	{
		mbFlagsNeedReset = FALSE;
		mbFlagsDirty = FALSE;
		// reset all of the ephemeral flags
		// some flags are managed elsewhere
		mControlFlags &= AGENT_CONTROL_AWAY | AGENT_CONTROL_FLY | AGENT_CONTROL_MOUSELOOK;
	}
}

//-----------------------------------------------------------------------------
// setAFK()
//-----------------------------------------------------------------------------
void LLAgent::setAFK()
{
	// Drones can't go AFK
	if (gNoRender)
	{
		return;
	}

	if (!gAgent.getRegion())
	{
		// Don't set AFK if we're not talking to a region yet.
		return;
	}

	if (!(mControlFlags & AGENT_CONTROL_AWAY))
	{
		sendAnimationRequest(ANIM_AGENT_AWAY, ANIM_REQUEST_START);
		setControlFlags(AGENT_CONTROL_AWAY | AGENT_CONTROL_STOP);
		gAwayTimer.start();
		if (gAFKMenu)
		{
			//*TODO:Translate
			gAFKMenu->setLabel(std::string("Set Not Away"));
		}
	}
}

//-----------------------------------------------------------------------------
// clearAFK()
//-----------------------------------------------------------------------------
void LLAgent::clearAFK()
{
	gAwayTriggerTimer.reset();

	// Gods can sometimes get into away state (via gestures)
	// without setting the appropriate control flag. JC
	LLVOAvatar* av = mAvatarObject;
	if (mControlFlags & AGENT_CONTROL_AWAY
		|| (av
			&& (av->mSignaledAnimations.find(ANIM_AGENT_AWAY) != av->mSignaledAnimations.end())))
	{
		sendAnimationRequest(ANIM_AGENT_AWAY, ANIM_REQUEST_STOP);
		clearControlFlags(AGENT_CONTROL_AWAY);
		if (gAFKMenu)
		{
			//*TODO:Translate
			gAFKMenu->setLabel(std::string("Set Away"));
		}
	}
}

//-----------------------------------------------------------------------------
// getAFK()
//-----------------------------------------------------------------------------
BOOL LLAgent::getAFK() const
{
	return (mControlFlags & AGENT_CONTROL_AWAY) != 0;
}

//-----------------------------------------------------------------------------
// setBusy()
//-----------------------------------------------------------------------------
void LLAgent::setBusy()
{
	sendAnimationRequest(ANIM_AGENT_BUSY, ANIM_REQUEST_START);
	mIsBusy = TRUE;
	if (gBusyMenu)
	{
		//*TODO:Translate
		gBusyMenu->setLabel(std::string("Set Not Busy"));
	}
	LLFloaterMute::getInstance()->updateButtons();
}

//-----------------------------------------------------------------------------
// clearBusy()
//-----------------------------------------------------------------------------
void LLAgent::clearBusy()
{
	mIsBusy = FALSE;
	sendAnimationRequest(ANIM_AGENT_BUSY, ANIM_REQUEST_STOP);
	if (gBusyMenu)
	{
		//*TODO:Translate
		gBusyMenu->setLabel(std::string("Set Busy"));
	}
	LLFloaterMute::getInstance()->updateButtons();
}

//-----------------------------------------------------------------------------
// getBusy()
//-----------------------------------------------------------------------------
BOOL LLAgent::getBusy() const
{
	return mIsBusy;
}


//-----------------------------------------------------------------------------
// startAutoPilotGlobal()
//-----------------------------------------------------------------------------
void LLAgent::startAutoPilotGlobal(const LLVector3d &target_global, const std::string& behavior_name, const LLQuaternion *target_rotation, void (*finish_callback)(BOOL, void *),  void *callback_data, F32 stop_distance, F32 rot_threshold)
{
	if (!gAgent.getAvatarObject())
	{
		return;
	}
	
	mAutoPilotFinishedCallback = finish_callback;
	mAutoPilotCallbackData = callback_data;
	mAutoPilotRotationThreshold = rot_threshold;
	mAutoPilotBehaviorName = behavior_name;

	LLVector3d delta_pos( target_global );
	delta_pos -= getPositionGlobal();
	F64 distance = delta_pos.magVec();
	LLVector3d trace_target = target_global;

	trace_target.mdV[VZ] -= 10.f;

	LLVector3d intersection;
	LLVector3 normal;
	LLViewerObject *hit_obj;
	F32 heightDelta = LLWorld::getInstance()->resolveStepHeightGlobal(NULL, target_global, trace_target, intersection, normal, &hit_obj);

	if (stop_distance > 0.f)
	{
		mAutoPilotStopDistance = stop_distance;
	}
	else
	{
		// Guess at a reasonable stop distance.
		mAutoPilotStopDistance = fsqrtf( distance );
		if (mAutoPilotStopDistance < 0.5f) 
		{
			mAutoPilotStopDistance = 0.5f;
		}
	}

	mAutoPilotFlyOnStop = getFlying();

	if (distance > 30.0)
	{
		setFlying(TRUE);
	}

	if ( distance > 1.f && heightDelta > (sqrtf(mAutoPilotStopDistance) + 1.f))
	{
		setFlying(TRUE);
		mAutoPilotFlyOnStop = TRUE;
	}

	mAutoPilot = TRUE;
	mAutoPilotTargetGlobal = target_global;

	// trace ray down to find height of destination from ground
	LLVector3d traceEndPt = target_global;
	traceEndPt.mdV[VZ] -= 20.f;

	LLVector3d targetOnGround;
	LLVector3 groundNorm;
	LLViewerObject *obj;

	LLWorld::getInstance()->resolveStepHeightGlobal(NULL, target_global, traceEndPt, targetOnGround, groundNorm, &obj);
	F64 target_height = llmax((F64)gAgent.getAvatarObject()->getPelvisToFoot(), target_global.mdV[VZ] - targetOnGround.mdV[VZ]);

	// clamp z value of target to minimum height above ground
	mAutoPilotTargetGlobal.mdV[VZ] = targetOnGround.mdV[VZ] + target_height;
	mAutoPilotTargetDist = (F32)dist_vec(gAgent.getPositionGlobal(), mAutoPilotTargetGlobal);
	if (target_rotation)
	{
		mAutoPilotUseRotation = TRUE;
		mAutoPilotTargetFacing = LLVector3::x_axis * *target_rotation;
		mAutoPilotTargetFacing.mV[VZ] = 0.f;
		mAutoPilotTargetFacing.normalize();
	}
	else
	{
		mAutoPilotUseRotation = FALSE;
	}

	mAutoPilotNoProgressFrameCount = 0;
}


//-----------------------------------------------------------------------------
// startFollowPilot()
//-----------------------------------------------------------------------------
void LLAgent::startFollowPilot(const LLUUID &leader_id)
{
	if (!mAutoPilot) return;

	mLeaderID = leader_id;
	if ( mLeaderID.isNull() ) return;

	LLViewerObject* object = gObjectList.findObject(mLeaderID);
	if (!object) 
	{
		mLeaderID = LLUUID::null;
		return;
	}

	startAutoPilotGlobal(object->getPositionGlobal());
}


//-----------------------------------------------------------------------------
// stopAutoPilot()
//-----------------------------------------------------------------------------
void LLAgent::stopAutoPilot(BOOL user_cancel)
{
	if (mAutoPilot)
	{
		mAutoPilot = FALSE;
		if (mAutoPilotUseRotation && !user_cancel)
		{
			resetAxes(mAutoPilotTargetFacing);
		}
		//NB: auto pilot can terminate for a reason other than reaching the destination
		if (mAutoPilotFinishedCallback)
		{
			mAutoPilotFinishedCallback(!user_cancel && dist_vec(gAgent.getPositionGlobal(), mAutoPilotTargetGlobal) < mAutoPilotStopDistance, mAutoPilotCallbackData);
		}
		mLeaderID = LLUUID::null;

		// If the user cancelled, don't change the fly state
		if (!user_cancel)
		{
			setFlying(mAutoPilotFlyOnStop);
		}
		setControlFlags(AGENT_CONTROL_STOP);

		if (user_cancel && !mAutoPilotBehaviorName.empty())
		{
			if (mAutoPilotBehaviorName == "Sit")
				LLNotifications::instance().add("CancelledSit");
			else if (mAutoPilotBehaviorName == "Attach")
				LLNotifications::instance().add("CancelledAttach");
			else
				LLNotifications::instance().add("Cancelled");
		}
	}
}


// Returns necessary agent pitch and yaw changes, radians.
//-----------------------------------------------------------------------------
// autoPilot()
//-----------------------------------------------------------------------------
void LLAgent::autoPilot(F32 *delta_yaw)
{
	if (mAutoPilot)
	{
		if (!mLeaderID.isNull())
		{
			LLViewerObject* object = gObjectList.findObject(mLeaderID);
			if (!object) 
			{
				stopAutoPilot();
				return;
			}
			mAutoPilotTargetGlobal = object->getPositionGlobal();
		}
		
		if (mAvatarObject.isNull())
		{
			return;
		}

		if (mAvatarObject->mInAir)
		{
			setFlying(TRUE);
		}
	
		LLVector3 at;
		at.setVec(mFrameAgent.getAtAxis());
		LLVector3 target_agent = getPosAgentFromGlobal(mAutoPilotTargetGlobal);
		LLVector3 direction = target_agent - getPositionAgent();

		F32 target_dist = direction.magVec();

		if (target_dist >= mAutoPilotTargetDist)
		{
			mAutoPilotNoProgressFrameCount++;
			if (mAutoPilotNoProgressFrameCount > AUTOPILOT_MAX_TIME_NO_PROGRESS * gFPSClamped)
			{
				stopAutoPilot();
				return;
			}
		}

		mAutoPilotTargetDist = target_dist;

		// Make this a two-dimensional solution
		at.mV[VZ] = 0.f;
		direction.mV[VZ] = 0.f;

		at.normalize();
		F32 xy_distance = direction.normalize();

		F32 yaw = 0.f;
		if (mAutoPilotTargetDist > mAutoPilotStopDistance)
		{
			yaw = angle_between(mFrameAgent.getAtAxis(), direction);
		}
		else if (mAutoPilotUseRotation)
		{
			// we're close now just aim at target facing
			yaw = angle_between(at, mAutoPilotTargetFacing);
			direction = mAutoPilotTargetFacing;
		}

		yaw = 4.f * yaw / gFPSClamped;

		// figure out which direction to turn
		LLVector3 scratch(at % direction);

		if (scratch.mV[VZ] > 0.f)
		{
			setControlFlags(AGENT_CONTROL_YAW_POS);
		}
		else
		{
			yaw = -yaw;
			setControlFlags(AGENT_CONTROL_YAW_NEG);
		}

		*delta_yaw = yaw;

		// Compute when to start slowing down and when to stop
		F32 stop_distance = mAutoPilotStopDistance;
		F32 slow_distance;
		if (getFlying())
		{
			slow_distance = llmax(6.f, mAutoPilotStopDistance + 5.f);
			stop_distance = llmax(2.f, mAutoPilotStopDistance);
		}
		else
		{
			slow_distance = llmax(3.f, mAutoPilotStopDistance + 2.f);
		}

		// If we're flying, handle autopilot points above or below you.
		if (getFlying() && xy_distance < AUTOPILOT_HEIGHT_ADJUST_DISTANCE)
		{
			if (mAvatarObject.notNull())
			{
				F64 current_height = mAvatarObject->getPositionGlobal().mdV[VZ];
				F32 delta_z = (F32)(mAutoPilotTargetGlobal.mdV[VZ] - current_height);
				F32 slope = delta_z / xy_distance;
				if (slope > 0.45f && delta_z > 6.f)
				{
					setControlFlags(AGENT_CONTROL_FAST_UP | AGENT_CONTROL_UP_POS);
				}
				else if (slope > 0.002f && delta_z > 0.5f)
				{
					setControlFlags(AGENT_CONTROL_UP_POS);
				}
				else if (slope < -0.45f && delta_z < -6.f && current_height > AUTOPILOT_MIN_TARGET_HEIGHT_OFF_GROUND)
				{
					setControlFlags(AGENT_CONTROL_FAST_UP | AGENT_CONTROL_UP_NEG);
				}
				else if (slope < -0.002f && delta_z < -0.5f && current_height > AUTOPILOT_MIN_TARGET_HEIGHT_OFF_GROUND)
				{
					setControlFlags(AGENT_CONTROL_UP_NEG);
				}
			}
		}

		//  calculate delta rotation to target heading
		F32 delta_target_heading = angle_between(mFrameAgent.getAtAxis(), mAutoPilotTargetFacing);

		if (xy_distance > slow_distance && yaw < (F_PI / 10.f))
		{
			// walking/flying fast
			setControlFlags(AGENT_CONTROL_FAST_AT | AGENT_CONTROL_AT_POS);
		}
		else if (mAutoPilotTargetDist > mAutoPilotStopDistance)
		{
			// walking/flying slow
			if (at * direction > 0.9f)
			{
				setControlFlags(AGENT_CONTROL_AT_POS);
			}
			else if (at * direction < -0.9f)
			{
				setControlFlags(AGENT_CONTROL_AT_NEG);
			}
		}

		// check to see if we need to keep rotating to target orientation
		if (mAutoPilotTargetDist < mAutoPilotStopDistance)
		{
			setControlFlags(AGENT_CONTROL_STOP);
			if(!mAutoPilotUseRotation || (delta_target_heading < mAutoPilotRotationThreshold))
			{
				stopAutoPilot();
			}
		}
	}
}


//-----------------------------------------------------------------------------
// propagate()
//-----------------------------------------------------------------------------
void LLAgent::propagate(const F32 dt)
{
	// Update UI based on agent motion
	LLFloaterMove *floater_move = LLFloaterMove::getInstance();
	if (floater_move)
	{
		floater_move->mForwardButton   ->setToggleState( mAtKey > 0 || mWalkKey > 0 );
		floater_move->mBackwardButton  ->setToggleState( mAtKey < 0 || mWalkKey < 0 );
		floater_move->mSlideLeftButton ->setToggleState( mLeftKey > 0 );
		floater_move->mSlideRightButton->setToggleState( mLeftKey < 0 );
		floater_move->mTurnLeftButton  ->setToggleState( mYawKey > 0.f );
		floater_move->mTurnRightButton ->setToggleState( mYawKey < 0.f );
		floater_move->mMoveUpButton    ->setToggleState( mUpKey > 0 );
		floater_move->mMoveDownButton  ->setToggleState( mUpKey < 0 );
	}

	// handle rotation based on keyboard levels
	const F32 YAW_RATE = 90.f * DEG_TO_RAD;				// radians per second
	yaw(YAW_RATE * mYawKey * dt);

	const F32 PITCH_RATE = 90.f * DEG_TO_RAD;			// radians per second
	pitch(PITCH_RATE * mPitchKey * dt);
	
	// handle auto-land behavior
	if (mAvatarObject.notNull())
	{
		BOOL in_air = mAvatarObject->mInAir;
		LLVector3 land_vel = getVelocity();
		land_vel.mV[VZ] = 0.f;

		if (!in_air 
			&& mUpKey < 0 
			&& land_vel.magVecSquared() < MAX_VELOCITY_AUTO_LAND_SQUARED
			&& gSavedSettings.getBOOL("AutomaticFly"))
		{
			// land automatically
			setFlying(FALSE);
		}
	}

	// clear keys
	mAtKey = 0;
	mWalkKey = 0;
	mLeftKey = 0;
	mUpKey = 0;
	mYawKey = 0.f;
	mPitchKey = 0;
}

//-----------------------------------------------------------------------------
// updateAgentPosition()
//-----------------------------------------------------------------------------
void LLAgent::updateAgentPosition(const F32 dt, const F32 yaw_radians, const S32 mouse_x, const S32 mouse_y)
{
	propagate(dt);

	// static S32 cameraUpdateCount = 0;

	rotate(yaw_radians, 0, 0, 1);
	
	//
	// Check for water and land collision, set underwater flag
	//

	updateLookAt(mouse_x, mouse_y);
}

//-----------------------------------------------------------------------------
// updateLookAt()
//-----------------------------------------------------------------------------
void LLAgent::updateLookAt(const S32 mouse_x, const S32 mouse_y)
{
	static LLVector3 last_at_axis;


	if (mAvatarObject.isNull())
	{
		return;
	}

	LLQuaternion av_inv_rot = ~mAvatarObject->mRoot.getWorldRotation();
	LLVector3 root_at = LLVector3::x_axis * mAvatarObject->mRoot.getWorldRotation();

	if 	((gViewerWindow->getMouseVelocityStat()->getCurrent() < 0.01f) &&
		(root_at * last_at_axis > 0.95f ))
	{
		LLVector3 vel = mAvatarObject->getVelocity();
		if (vel.magVecSquared() > 4.f)
		{
			setLookAt(LOOKAT_TARGET_IDLE, mAvatarObject, vel * av_inv_rot);
		}
		else
		{
			// *FIX: rotate mframeagent by sit object's rotation?
			LLQuaternion look_rotation = mAvatarObject->mIsSitting ? mAvatarObject->getRenderRotation() : mFrameAgent.getQuaternion(); // use camera's current rotation
			LLVector3 look_offset = LLVector3(2.f, 0.f, 0.f) * look_rotation * av_inv_rot;
			setLookAt(LOOKAT_TARGET_IDLE, mAvatarObject, look_offset);
		}
		last_at_axis = root_at;
		return;
	}

	last_at_axis = root_at;
	
	if (CAMERA_MODE_CUSTOMIZE_AVATAR == getCameraMode())
	{
		setLookAt(LOOKAT_TARGET_NONE, mAvatarObject, LLVector3(-2.f, 0.f, 0.f));	
	}
	else
	{
		// Move head based on cursor position
		ELookAtType lookAtType = LOOKAT_TARGET_NONE;
		LLVector3 headLookAxis;
		LLCoordFrame frameCamera = *((LLCoordFrame*)LLViewerCamera::getInstance());

		if (cameraMouselook())
		{
			lookAtType = LOOKAT_TARGET_MOUSELOOK;
		}
		else if (cameraThirdPerson())
		{
			// range from -.5 to .5
			F32 x_from_center = 
				((F32) mouse_x / (F32) gViewerWindow->getWindowWidth() ) - 0.5f;
			F32 y_from_center = 
				((F32) mouse_y / (F32) gViewerWindow->getWindowHeight() ) - 0.5f;

			frameCamera.yaw( - x_from_center * gSavedSettings.getF32("YawFromMousePosition") * DEG_TO_RAD);
			frameCamera.pitch( - y_from_center * gSavedSettings.getF32("PitchFromMousePosition") * DEG_TO_RAD);
			lookAtType = LOOKAT_TARGET_FREELOOK;
		}

		headLookAxis = frameCamera.getAtAxis();
		// RN: we use world-space offset for mouselook and freelook
		//headLookAxis = headLookAxis * av_inv_rot;
		setLookAt(lookAtType, mAvatarObject, headLookAxis);
	}
}

// friends and operators

std::ostream& operator<<(std::ostream &s, const LLAgent &agent)
{
	// This is unfinished, but might never be used. 
	// We'll just leave it for now; we can always delete it.
	s << " { "
	  << "  Frame = " << agent.mFrameAgent << "\n"
	  << " }";
	return s;
}


// ------------------- Beginning of legacy LLCamera hack ----------------------
// This section is included for legacy LLCamera support until
// it is no longer needed.  Some legacy code must exist in 
// non-legacy functions, and is labeled with "// legacy" comments.

//-----------------------------------------------------------------------------
// setAvatarObject()
//-----------------------------------------------------------------------------
void LLAgent::setAvatarObject(LLVOAvatar *avatar)			
{ 
	mAvatarObject = avatar;

	if (!avatar)
	{
		llinfos << "Setting LLAgent::mAvatarObject to NULL" << llendl;
		return;
	}

	if (!mLookAt)
	{
		mLookAt = (LLHUDEffectLookAt *)LLHUDManager::getInstance()->createViewerEffect(LLHUDObject::LL_HUD_EFFECT_LOOKAT);
	}
	if (!mPointAt)
	{
		mPointAt = (LLHUDEffectPointAt *)LLHUDManager::getInstance()->createViewerEffect(LLHUDObject::LL_HUD_EFFECT_POINTAT);
	}
	
	if (!mLookAt.isNull())
	{
		mLookAt->setSourceObject(avatar);
	}
	if (!mPointAt.isNull())
	{
		mPointAt->setSourceObject(avatar);
	}

	sendAgentWearablesRequest();
}

// TRUE if your own avatar needs to be rendered.  Usually only
// in third person and build.
//-----------------------------------------------------------------------------
// needsRenderAvatar()
//-----------------------------------------------------------------------------
BOOL LLAgent::needsRenderAvatar()
{
	if (cameraMouselook() && !LLVOAvatar::sVisibleInFirstPerson)
	{
		return FALSE;
	}

	return mShowAvatar && mGenderChosen;
}

// TRUE if we need to render your own avatar's head.
BOOL LLAgent::needsRenderHead()
{
	return (LLVOAvatar::sVisibleInFirstPerson && LLPipeline::sReflectionRender) || (mShowAvatar && !cameraMouselook());
}

//-----------------------------------------------------------------------------
// startTyping()
//-----------------------------------------------------------------------------
void LLAgent::startTyping()
{
	mTypingTimer.reset();

	if (getRenderState() & AGENT_STATE_TYPING)
	{
		// already typing, don't trigger a different animation
		return;
	}
	setRenderState(AGENT_STATE_TYPING);

	if (mChatTimer.getElapsedTimeF32() < 2.f)
	{
		LLViewerObject* chatter = gObjectList.findObject(mLastChatterID);
		if (chatter && chatter->isAvatar())
		{
			gAgent.setLookAt(LOOKAT_TARGET_RESPOND, chatter, LLVector3::zero);
		}
	}

	if (gSavedSettings.getBOOL("PlayTypingAnim"))
	{
		sendAnimationRequest(ANIM_AGENT_TYPE, ANIM_REQUEST_START);
	}

	if (gSavedSettings.getBOOL("PhoenixVoiceAnimWhileTyping")){
		sendAnimationRequest(LLUUID("37694185-3107-d418-3a20-0181424e542d"), ANIM_REQUEST_START);
	}

	gChatBar->sendChatFromViewer("", CHAT_TYPE_START, FALSE);
}

//-----------------------------------------------------------------------------
// stopTyping()
//-----------------------------------------------------------------------------
void LLAgent::stopTyping()
{
	if (mRenderState & AGENT_STATE_TYPING)
	{
		clearRenderState(AGENT_STATE_TYPING);
		sendAnimationRequest(ANIM_AGENT_TYPE, ANIM_REQUEST_STOP);
		if(gSavedSettings.getBOOL("PhoenixVoiceAnimWhileTyping")) sendAnimationRequest(LLUUID("37694185-3107-d418-3a20-0181424e542d"), ANIM_REQUEST_STOP);
		gChatBar->sendChatFromViewer("", CHAT_TYPE_STOP, FALSE);
	}
}

//-----------------------------------------------------------------------------
// setRenderState()
//-----------------------------------------------------------------------------
void LLAgent::setRenderState(U8 newstate)
{
	mRenderState |= newstate;
}

//-----------------------------------------------------------------------------
// clearRenderState()
//-----------------------------------------------------------------------------
void LLAgent::clearRenderState(U8 clearstate)
{
	mRenderState &= ~clearstate;
}


//-----------------------------------------------------------------------------
// getRenderState()
//-----------------------------------------------------------------------------
U8 LLAgent::getRenderState()
{
	if (gNoRender || gKeyboard == NULL)
	{
		return 0;
	}

	static LLCachedControl<BOOL> PhoenixVoiceAnimWhileTyping("PhoenixVoiceAnimWhileTyping", 0);
	if((mRenderState & AGENT_STATE_TYPING) && PhoenixVoiceAnimWhileTyping){ // If we are typing and voice anim should be played
		LLVOAvatar* avatarp = gAgent.getAvatarObject();
		if (avatarp)
		{
			bool isplaying=false;
			LLVOAvatar::AnimSourceIterator ai;
			for(ai = avatarp->mAnimationSources.begin(); ai != avatarp->mAnimationSources.end(); ++ai) // Loop through playing anims
			{
				if(ai->second==LLUUID("37694185-3107-d418-3a20-0181424e542d")){ //Are we playing this animation at the moment?
					isplaying=true;
				}
			}
			if(!isplaying){ //If not, start it again
				sendAnimationRequest(LLUUID("37694185-3107-d418-3a20-0181424e542d"), ANIM_REQUEST_START);
			}
		}
	}


	// *FIX: don't do stuff in a getter!  This is infinite loop city!
	if ((mTypingTimer.getElapsedTimeF32() > TYPING_TIMEOUT_SECS) 
		&& (mRenderState & AGENT_STATE_TYPING))
	{
		stopTyping();
	}
	
	if ((!LLSelectMgr::getInstance()->getSelection()->isEmpty() && LLSelectMgr::getInstance()->shouldShowSelection())
		|| LLToolMgr::getInstance()->getCurrentTool()->isEditing() )
	{
		setRenderState(AGENT_STATE_EDITING);
	}
	else
	{
		clearRenderState(AGENT_STATE_EDITING);
	}

	return mRenderState;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static const LLFloaterView::skip_list_t& get_skip_list()
{
	static LLFloaterView::skip_list_t skip_list;
	skip_list.insert(LLFloaterMap::getInstance());
	static BOOL *sPhoenixShowStatusBarInMouselook = rebind_llcontrol<BOOL>("PhoenixShowStatusBarInMouselook", &gSavedSettings, true);
	if(*sPhoenixShowStatusBarInMouselook)
	{
		skip_list.insert(LLFloaterStats::getInstance());
	}
	return skip_list;
}

//-----------------------------------------------------------------------------
// endAnimationUpdateUI()
//-----------------------------------------------------------------------------
void LLAgent::endAnimationUpdateUI()
{
	if (mCameraMode == mLastCameraMode)
	{
		// We're already done endAnimationUpdateUI for this transition.
		return;
	}

	// clean up UI from mode we're leaving
	if ( mLastCameraMode == CAMERA_MODE_MOUSELOOK )
	{
		// show mouse cursor
		gViewerWindow->showCursor();
		// show menus
		gMenuBarView->setVisible(TRUE);
		gStatusBar->setVisibleForMouselook(true);

		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);

		// Only pop if we have pushed...
		if (TRUE == mViewsPushed)
		{
			mViewsPushed = FALSE;
			gFloaterView->popVisibleAll(get_skip_list());
		}

		gAgent.setLookAt(LOOKAT_TARGET_CLEAR);
		if( gMorphView )
		{
			gMorphView->setVisible( FALSE );
		}

		// Disable mouselook-specific animations
		if (mAvatarObject.notNull())
		{
			if( mAvatarObject->isAnyAnimationSignaled(AGENT_GUN_AIM_ANIMS, NUM_AGENT_GUN_AIM_ANIMS) )
			{
				if (mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_AIM_RIFLE_R) != mAvatarObject->mSignaledAnimations.end())
				{
					sendAnimationRequest(ANIM_AGENT_AIM_RIFLE_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_HOLD_RIFLE_R, ANIM_REQUEST_START);
				}
				if (mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_AIM_HANDGUN_R) != mAvatarObject->mSignaledAnimations.end())
				{
					sendAnimationRequest(ANIM_AGENT_AIM_HANDGUN_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_HOLD_HANDGUN_R, ANIM_REQUEST_START);
				}
				if (mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_AIM_BAZOOKA_R) != mAvatarObject->mSignaledAnimations.end())
				{
					sendAnimationRequest(ANIM_AGENT_AIM_BAZOOKA_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_HOLD_BAZOOKA_R, ANIM_REQUEST_START);
				}
				if (mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_AIM_BOW_L) != mAvatarObject->mSignaledAnimations.end())
				{
					sendAnimationRequest(ANIM_AGENT_AIM_BOW_L, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_HOLD_BOW_L, ANIM_REQUEST_START);
				}
			}
		}
	}
	else
	if(	mLastCameraMode == CAMERA_MODE_CUSTOMIZE_AVATAR )
	{
		// make sure we ask to save changes

		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);

		// HACK: If we're quitting, and we were in customize avatar, don't
		// let the mini-map go visible again. JC
        if (!LLAppViewer::instance()->quitRequested())
		{
			LLFloaterMap::getInstance()->popVisible();
		}

		if( gMorphView )
		{
			gMorphView->setVisible( FALSE );
		}

		if (mAvatarObject.notNull())
		{
			if(mCustomAnim)
			{
				sendAnimationRequest(ANIM_AGENT_CUSTOMIZE, ANIM_REQUEST_STOP);
				sendAnimationRequest(ANIM_AGENT_CUSTOMIZE_DONE, ANIM_REQUEST_START);

				mCustomAnim = FALSE ;
			}
			
		}
		setLookAt(LOOKAT_TARGET_CLEAR);
	}

	//---------------------------------------------------------------------
	// Set up UI for mode we're entering
	//---------------------------------------------------------------------
	if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		// hide menus
		gMenuBarView->setVisible(FALSE);
		gStatusBar->setVisibleForMouselook(false);

		// clear out camera lag effect
		mCameraLag.clearVec();

		// JC - Added for always chat in third person option
		gFocusMgr.setKeyboardFocus(NULL);

		LLToolMgr::getInstance()->setCurrentToolset(gMouselookToolset);

		mViewsPushed = TRUE;

		gFloaterView->pushVisibleAll(FALSE, get_skip_list());

		if( gMorphView )
		{
			gMorphView->setVisible(FALSE);
		}

		gIMMgr->setFloaterOpen( FALSE );
		gConsole->setVisible( TRUE );

		if (mAvatarObject.notNull())
		{
			// Trigger mouselook-specific animations
			if( mAvatarObject->isAnyAnimationSignaled(AGENT_GUN_HOLD_ANIMS, NUM_AGENT_GUN_HOLD_ANIMS) )
			{
				if (mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_HOLD_RIFLE_R) != mAvatarObject->mSignaledAnimations.end())
				{
					sendAnimationRequest(ANIM_AGENT_HOLD_RIFLE_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_AIM_RIFLE_R, ANIM_REQUEST_START);
				}
				if (mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_HOLD_HANDGUN_R) != mAvatarObject->mSignaledAnimations.end())
				{
					sendAnimationRequest(ANIM_AGENT_HOLD_HANDGUN_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_AIM_HANDGUN_R, ANIM_REQUEST_START);
				}
				if (mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_HOLD_BAZOOKA_R) != mAvatarObject->mSignaledAnimations.end())
				{
					sendAnimationRequest(ANIM_AGENT_HOLD_BAZOOKA_R, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_AIM_BAZOOKA_R, ANIM_REQUEST_START);
				}
				if (mAvatarObject->mSignaledAnimations.find(ANIM_AGENT_HOLD_BOW_L) != mAvatarObject->mSignaledAnimations.end())
				{
					sendAnimationRequest(ANIM_AGENT_HOLD_BOW_L, ANIM_REQUEST_STOP);
					sendAnimationRequest(ANIM_AGENT_AIM_BOW_L, ANIM_REQUEST_START);
				}
			}
			if (mAvatarObject->getParent())
			{
				LLVector3 at_axis = LLViewerCamera::getInstance()->getAtAxis();
				LLViewerObject* root_object = (LLViewerObject*)mAvatarObject->getRoot();
				if (root_object->flagCameraDecoupled())
				{
					resetAxes(at_axis);
				}
				else
				{
					resetAxes(at_axis * ~((LLViewerObject*)mAvatarObject->getParent())->getRenderRotation());
				}
			}
		}

	}
	else if (mCameraMode == CAMERA_MODE_CUSTOMIZE_AVATAR)
	{
		LLToolMgr::getInstance()->setCurrentToolset(gFaceEditToolset);

		LLFloaterMap::getInstance()->pushVisible(FALSE);
		/*
		LLView *view;
		for (view = gFloaterView->getFirstChild(); view; view = gFloaterView->getNextChild())
		{
			view->pushVisible(FALSE);
		}
		*/

		if( gMorphView )
		{
			gMorphView->setVisible( TRUE );
		}

		// freeze avatar
		if (mAvatarObject.notNull())
		{
			mPauseRequest = mAvatarObject->requestPause();
		}
	}

	if (getAvatarObject())
	{
		getAvatarObject()->updateAttachmentVisibility(mCameraMode);
	}

	gFloaterTools->dirty();

	// Don't let this be called more than once if the camera
	// mode hasn't changed.  --JC
	mLastCameraMode = mCameraMode;

}


//-----------------------------------------------------------------------------
// updateCamera()
//-----------------------------------------------------------------------------
void LLAgent::updateCamera()
{
	//Ventrella - changed camera_skyward to the new global "mCameraUpVector"
	mCameraUpVector = LLVector3::z_axis;
	//LLVector3	camera_skyward(0.f, 0.f, 1.f);
	//end Ventrella

	U32 camera_mode = mCameraAnimating ? mLastCameraMode : mCameraMode;

	validateFocusObject();

	if (mAvatarObject.notNull() && 
		mAvatarObject->mIsSitting &&
		camera_mode == CAMERA_MODE_MOUSELOOK)
	{
		//Ventrella
		//changed camera_skyward to the new global "mCameraUpVector"
		mCameraUpVector = mCameraUpVector * mAvatarObject->getRenderRotation();
		//end Ventrella
	}

	if (cameraThirdPerson() && mFocusOnAvatar && LLFollowCamMgr::getActiveFollowCamParams())
	{
		changeCameraToFollow();
	}

	//Ventrella
	//NOTE - this needs to be integrated into a general upVector system here within llAgent. 
	if ( camera_mode == CAMERA_MODE_FOLLOW && mFocusOnAvatar )
	{
		mCameraUpVector = mFollowCam.getUpVector();
	}
	//end Ventrella

	if (mSitCameraEnabled)
	{
		if (mSitCameraReferenceObject->isDead())
		{
			setSitCamera(LLUUID::null);
		}
	}

	// Update UI with our camera inputs
	LLFloaterCamera::getInstance()->mRotate->setToggleState(
		mOrbitRightKey > 0.f,	// left
		mOrbitUpKey > 0.f,		// top
		mOrbitLeftKey > 0.f,	// right
		mOrbitDownKey > 0.f);	// bottom

	LLFloaterCamera::getInstance()->mZoom->setToggleState( 
		mOrbitInKey > 0.f,		// top
		mOrbitOutKey > 0.f);	// bottom

	LLFloaterCamera::getInstance()->mTrack->setToggleState(
		mPanLeftKey > 0.f,		// left
		mPanUpKey > 0.f,		// top
		mPanRightKey > 0.f,		// right
		mPanDownKey > 0.f);		// bottom

	// Handle camera movement based on keyboard.
	const F32 ORBIT_OVER_RATE = 90.f * DEG_TO_RAD;			// radians per second
	const F32 ORBIT_AROUND_RATE = 90.f * DEG_TO_RAD;		// radians per second
	const F32 PAN_RATE = 5.f;								// meters per second

	if( mOrbitUpKey || mOrbitDownKey )
	{
		F32 input_rate = mOrbitUpKey - mOrbitDownKey;
		cameraOrbitOver( input_rate * ORBIT_OVER_RATE / gFPSClamped );
	}

	if( mOrbitLeftKey || mOrbitRightKey)
	{
		F32 input_rate = mOrbitLeftKey - mOrbitRightKey;
		cameraOrbitAround( input_rate * ORBIT_AROUND_RATE / gFPSClamped );
	}

	if( mOrbitInKey || mOrbitOutKey )
	{
		F32 input_rate = mOrbitInKey - mOrbitOutKey;
		
		LLVector3d to_focus = gAgent.getPosGlobalFromAgent(LLViewerCamera::getInstance()->getOrigin()) - calcFocusPositionTargetGlobal();
		F32 distance_to_focus = (F32)to_focus.magVec();
		// Move at distance (in meters) meters per second
		cameraOrbitIn( input_rate * distance_to_focus / gFPSClamped );
	}

	if( mPanInKey || mPanOutKey )
	{
		F32 input_rate = mPanInKey - mPanOutKey;
		cameraPanIn( input_rate * PAN_RATE / gFPSClamped );
	}

	if( mPanRightKey || mPanLeftKey )
	{
		F32 input_rate = mPanRightKey - mPanLeftKey;
		cameraPanLeft( input_rate * -PAN_RATE / gFPSClamped );
	}

	if( mPanUpKey || mPanDownKey )
	{
		F32 input_rate = mPanUpKey - mPanDownKey;
		cameraPanUp( input_rate * PAN_RATE / gFPSClamped );
	}

	// Clear camera keyboard keys.
	mOrbitLeftKey		= 0.f;
	mOrbitRightKey		= 0.f;
	mOrbitUpKey			= 0.f;
	mOrbitDownKey		= 0.f;
	mOrbitInKey			= 0.f;
	mOrbitOutKey		= 0.f;

	mPanRightKey		= 0.f;
	mPanLeftKey			= 0.f;
	mPanUpKey			= 0.f;
	mPanDownKey			= 0.f;
	mPanInKey			= 0.f;
	mPanOutKey			= 0.f;

	// lerp camera focus offset
	mCameraFocusOffset = lerp(mCameraFocusOffset, mCameraFocusOffsetTarget, LLCriticalDamp::getInterpolant(CAMERA_FOCUS_HALF_LIFE));

	//Ventrella
	if ( mCameraMode == CAMERA_MODE_FOLLOW )
	{
		if ( mAvatarObject.notNull() )
		{
			//--------------------------------------------------------------------------------
			// this is where the avatar's position and rotation are given to followCam, and 
			// where it is updated. All three of its attributes are updated: (1) position, 
			// (2) focus, and (3) upvector. They can then be queried elsewhere in llAgent.
			//--------------------------------------------------------------------------------
			// *TODO: use combined rotation of frameagent and sit object
			LLQuaternion avatarRotationForFollowCam = mAvatarObject->mIsSitting ? mAvatarObject->getRenderRotation() : mFrameAgent.getQuaternion();

			LLFollowCamParams* current_cam = LLFollowCamMgr::getActiveFollowCamParams();
			if (current_cam)
			{
				mFollowCam.copyParams(*current_cam);
				mFollowCam.setSubjectPositionAndRotation( mAvatarObject->getRenderPosition(), avatarRotationForFollowCam );
				mFollowCam.update();
				LLViewerJoystick::getInstance()->setCameraNeedsUpdate(true);
			}
			else
			{
				changeCameraToThirdPerson(TRUE);
			}
		}
	}
	// end Ventrella

	BOOL hit_limit;
	LLVector3d camera_pos_global;
	LLVector3d camera_target_global = calcCameraPositionTargetGlobal(&hit_limit);
	mCameraVirtualPositionAgent = getPosAgentFromGlobal(camera_target_global);
	LLVector3d focus_target_global = calcFocusPositionTargetGlobal();

	// perform field of view correction
	mCameraFOVZoomFactor = calcCameraFOVZoomFactor();
	camera_target_global = focus_target_global + (camera_target_global - focus_target_global) * (1.f + mCameraFOVZoomFactor);

	mShowAvatar = TRUE; // can see avatar by default

	// Adjust position for animation
	if (mCameraAnimating)
	{
		F32 time = mAnimationTimer.getElapsedTimeF32();

		// yet another instance of critically damped motion, hooray!
		// F32 fraction_of_animation = 1.f - pow(2.f, -time / CAMERA_ZOOM_HALF_LIFE);

		// linear interpolation
		F32 fraction_of_animation = time / mAnimationDuration;

		BOOL isfirstPerson = mCameraMode == CAMERA_MODE_MOUSELOOK;
		BOOL wasfirstPerson = mLastCameraMode == CAMERA_MODE_MOUSELOOK;
		F32 fraction_animation_to_skip;

		if (mAnimationCameraStartGlobal == camera_target_global)
		{
			fraction_animation_to_skip = 0.f;
		}
		else
		{
			LLVector3d cam_delta = mAnimationCameraStartGlobal - camera_target_global;
			fraction_animation_to_skip = HEAD_BUFFER_SIZE / (F32)cam_delta.magVec();
		}
		F32 animation_start_fraction = (wasfirstPerson) ? fraction_animation_to_skip : 0.f;
		F32 animation_finish_fraction =  (isfirstPerson) ? (1.f - fraction_animation_to_skip) : 1.f;
	
		if (fraction_of_animation < animation_finish_fraction)
		{
			if (fraction_of_animation < animation_start_fraction || fraction_of_animation > animation_finish_fraction )
			{
				mShowAvatar = FALSE;
			}

			// ...adjust position for animation
			F32 smooth_fraction_of_animation = llsmoothstep(0.0f, 1.0f, fraction_of_animation);
			camera_pos_global = lerp(mAnimationCameraStartGlobal, camera_target_global, smooth_fraction_of_animation);
			mFocusGlobal = lerp(mAnimationFocusStartGlobal, focus_target_global, smooth_fraction_of_animation);
		}
		else
		{
			// ...animation complete
			mCameraAnimating = FALSE;

			camera_pos_global = camera_target_global;
			mFocusGlobal = focus_target_global;

			endAnimationUpdateUI();
			mShowAvatar = TRUE;
		}

		if (getAvatarObject() && mCameraMode != CAMERA_MODE_MOUSELOOK)
		{
			getAvatarObject()->updateAttachmentVisibility(mCameraMode);
		}
	}
	else 
	{
		camera_pos_global = camera_target_global;
		mFocusGlobal = focus_target_global;
		mShowAvatar = TRUE;
	}

	// smoothing
	if (TRUE)
	{
		LLVector3d agent_pos = getPositionGlobal();
		LLVector3d camera_pos_agent = camera_pos_global - agent_pos;
		// Sitting on what you're manipulating can cause camera jitter with smoothing. 
		// This turns off smoothing while editing. -MG
		mCameraSmoothingStop |= (BOOL)LLToolMgr::getInstance()->inBuildMode();
		
		if (cameraThirdPerson() && !mCameraSmoothingStop)
		{
			const F32 SMOOTHING_HALF_LIFE = 0.02f;
			
			F32 smoothing = LLCriticalDamp::getInterpolant(gSavedSettings.getF32("CameraPositionSmoothing") * SMOOTHING_HALF_LIFE, FALSE);
					
			if (!mFocusObject)  // we differentiate on avatar mode 
			{
				// for avatar-relative focus, we smooth in avatar space -
				// the avatar moves too jerkily w/r/t global space to smooth there.

				LLVector3d delta = camera_pos_agent - mCameraSmoothingLastPositionAgent;
				if (delta.magVec() < MAX_CAMERA_SMOOTH_DISTANCE)  // only smooth over short distances please
				{
					camera_pos_agent = lerp(mCameraSmoothingLastPositionAgent, camera_pos_agent, smoothing);
					camera_pos_global = camera_pos_agent + agent_pos;
				}
			}
			else
			{
				LLVector3d delta = camera_pos_global - mCameraSmoothingLastPositionGlobal;
				if (delta.magVec() < MAX_CAMERA_SMOOTH_DISTANCE) // only smooth over short distances please
				{
					camera_pos_global = lerp(mCameraSmoothingLastPositionGlobal, camera_pos_global, smoothing);
				}
			}
		}
								 
		mCameraSmoothingLastPositionGlobal = camera_pos_global;
		mCameraSmoothingLastPositionAgent = camera_pos_agent;
		mCameraSmoothingStop = FALSE;
	}

	
	mCameraCurrentFOVZoomFactor = lerp(mCameraCurrentFOVZoomFactor, mCameraFOVZoomFactor, LLCriticalDamp::getInterpolant(FOV_ZOOM_HALF_LIFE));

//	llinfos << "Current FOV Zoom: " << mCameraCurrentFOVZoomFactor << " Target FOV Zoom: " << mCameraFOVZoomFactor << " Object penetration: " << mFocusObjectDist << llendl;

	F32 ui_offset = 0.f;
	if( CAMERA_MODE_CUSTOMIZE_AVATAR == mCameraMode ) 
	{
		ui_offset = calcCustomizeAvatarUIOffset( camera_pos_global );
	}


	LLVector3 focus_agent = getPosAgentFromGlobal(mFocusGlobal);
	
	mCameraPositionAgent	= getPosAgentFromGlobal(camera_pos_global);

	// Move the camera

	//Ventrella
	LLViewerCamera::getInstance()->updateCameraLocation(mCameraPositionAgent, mCameraUpVector, focus_agent);
	//LLViewerCamera::getInstance()->updateCameraLocation(mCameraPositionAgent, camera_skyward, focus_agent);
	//end Ventrella

	//RN: translate UI offset after camera is oriented properly
	LLViewerCamera::getInstance()->translate(LLViewerCamera::getInstance()->getLeftAxis() * ui_offset);
	
	// Change FOV
	LLViewerCamera::getInstance()->setView(LLViewerCamera::getInstance()->getDefaultFOV() / (1.f + mCameraCurrentFOVZoomFactor));

	// follow camera when in customize mode
	if (cameraCustomizeAvatar())	
	{
		setLookAt(LOOKAT_TARGET_FOCUS, NULL, mCameraPositionAgent);
	}

	// update the travel distance stat
	// this isn't directly related to the camera
	// but this seemed like the best place to do this
	LLVector3d global_pos = getPositionGlobal(); 
	if (! mLastPositionGlobal.isExactlyZero())
	{
		LLVector3d delta = global_pos - mLastPositionGlobal;
		mDistanceTraveled += delta.magVec();
	}
	mLastPositionGlobal = global_pos;
	
	if (LLVOAvatar::sVisibleInFirstPerson && mAvatarObject.notNull() && !mAvatarObject->mIsSitting && cameraMouselook())
	{
		LLVector3 head_pos = mAvatarObject->mHeadp->getWorldPosition() + 
			LLVector3(0.08f, 0.f, 0.05f) * mAvatarObject->mHeadp->getWorldRotation() + 
			LLVector3(0.1f, 0.f, 0.f) * mAvatarObject->mPelvisp->getWorldRotation();
		LLVector3 diff = mCameraPositionAgent - head_pos;
		diff = diff * ~mAvatarObject->mRoot.getWorldRotation();

		LLJoint* torso_joint = mAvatarObject->mTorsop;
		LLJoint* chest_joint = mAvatarObject->mChestp;
		LLVector3 torso_scale = torso_joint->getScale();
		LLVector3 chest_scale = chest_joint->getScale();

		// shorten avatar skeleton to avoid foot interpenetration
		if (!mAvatarObject->mInAir)
		{
			LLVector3 chest_offset = LLVector3(0.f, 0.f, chest_joint->getPosition().mV[VZ]) * torso_joint->getWorldRotation();
			F32 z_compensate = llclamp(-diff.mV[VZ], -0.2f, 1.f);
			F32 scale_factor = llclamp(1.f - ((z_compensate * 0.5f) / chest_offset.mV[VZ]), 0.5f, 1.2f);
			torso_joint->setScale(LLVector3(1.f, 1.f, scale_factor));

			LLJoint* neck_joint = mAvatarObject->mNeckp;
			LLVector3 neck_offset = LLVector3(0.f, 0.f, neck_joint->getPosition().mV[VZ]) * chest_joint->getWorldRotation();
			scale_factor = llclamp(1.f - ((z_compensate * 0.5f) / neck_offset.mV[VZ]), 0.5f, 1.2f);
			chest_joint->setScale(LLVector3(1.f, 1.f, scale_factor));
			diff.mV[VZ] = 0.f;
		}

		mAvatarObject->mPelvisp->setPosition(mAvatarObject->mPelvisp->getPosition() + diff);

		mAvatarObject->mRoot.updateWorldMatrixChildren();

		for (LLVOAvatar::attachment_map_t::iterator iter = mAvatarObject->mAttachmentPoints.begin(); 
			 iter != mAvatarObject->mAttachmentPoints.end(); )
		{
			LLVOAvatar::attachment_map_t::iterator curiter = iter++;
			LLViewerJointAttachment* attachment = curiter->second;
			for (LLViewerJointAttachment::attachedobjs_vec_t::iterator attachment_iter = attachment->mAttachedObjects.begin();
				 attachment_iter != attachment->mAttachedObjects.end();
				 ++attachment_iter)
			{
				LLViewerObject *attached_object = (*attachment_iter);
				if (attached_object && !attached_object->isDead() && attached_object->mDrawable.notNull())
				{
					// clear any existing "early" movements of attachment
					attached_object->mDrawable->clearState(LLDrawable::EARLY_MOVE);
					gPipeline.updateMoveNormalAsync(attached_object->mDrawable);
					attached_object->updateText();
				}
			}
		}

		torso_joint->setScale(torso_scale);
		chest_joint->setScale(chest_scale);
	}
}

void LLAgent::updateFocusOffset()
{
	validateFocusObject();
	if (mFocusObject.notNull())
	{
		LLVector3d obj_pos = getPosGlobalFromAgent(mFocusObject->getRenderPosition());
		mFocusObjectOffset.setVec(mFocusTargetGlobal - obj_pos);
	}
}

void LLAgent::validateFocusObject()
{
	if (mFocusObject.notNull() && 
		(mFocusObject->isDead()))
	{
		mFocusObjectOffset.clearVec();
		clearFocusObject();
		mCameraFOVZoomFactor = 0.f;
	}
}

//-----------------------------------------------------------------------------
// calcCustomizeAvatarUIOffset()
//-----------------------------------------------------------------------------
F32 LLAgent::calcCustomizeAvatarUIOffset( const LLVector3d& camera_pos_global )
{
	F32 ui_offset = 0.f;

	if( gFloaterCustomize )
	{
		const LLRect& rect = gFloaterCustomize->getRect();

		// Move the camera so that the avatar isn't covered up by this floater.
		F32 fraction_of_fov = 0.5f - (0.5f * (1.f - llmin(1.f, ((F32)rect.getWidth() / (F32)gViewerWindow->getWindowWidth()))));
		F32 apparent_angle = fraction_of_fov * LLViewerCamera::getInstance()->getView() * LLViewerCamera::getInstance()->getAspect();  // radians
		F32 offset = tan(apparent_angle);

		if( rect.mLeft < (gViewerWindow->getWindowWidth() - rect.mRight) )
		{
			// Move the avatar to the right (camera to the left)
			ui_offset = offset;
		}
		else
		{
			// Move the avatar to the left (camera to the right)
			ui_offset = -offset;
		}
	}
	F32 range = (F32)dist_vec(camera_pos_global, gAgent.getFocusGlobal());
	mUIOffset = lerp(mUIOffset, ui_offset, LLCriticalDamp::getInterpolant(0.05f));
	return mUIOffset * range;
}

//-----------------------------------------------------------------------------
// calcFocusPositionTargetGlobal()
//-----------------------------------------------------------------------------
LLVector3d LLAgent::calcFocusPositionTargetGlobal()
{
	if (mFocusObject.notNull() && mFocusObject->isDead())
	{
		clearFocusObject();
	}

	// Ventrella
	if ( mCameraMode == CAMERA_MODE_FOLLOW && mFocusOnAvatar )
	{
		mFocusTargetGlobal = gAgent.getPosGlobalFromAgent(mFollowCam.getSimulatedFocus());
		return mFocusTargetGlobal;
	}// End Ventrella 
	else if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		LLVector3d at_axis(1.0, 0.0, 0.0);
		LLQuaternion agent_rot = mFrameAgent.getQuaternion();
		if (mAvatarObject.notNull() && mAvatarObject->getParent())
		{
			LLViewerObject* root_object = (LLViewerObject*)mAvatarObject->getRoot();
			if (!root_object->flagCameraDecoupled())
			{
				agent_rot *= ((LLViewerObject*)(mAvatarObject->getParent()))->getRenderRotation();
			}
		}
		at_axis = at_axis * agent_rot;
		mFocusTargetGlobal = calcCameraPositionTargetGlobal() + at_axis;
		return mFocusTargetGlobal;
	}
	else if (mCameraMode == CAMERA_MODE_CUSTOMIZE_AVATAR)
	{
		return mFocusTargetGlobal;
	}
	else if (!mFocusOnAvatar)
	{
		if (mFocusObject.notNull() && !mFocusObject->isDead() && mFocusObject->mDrawable.notNull())
		{
			LLDrawable* drawablep = mFocusObject->mDrawable;
			
			if (mTrackFocusObject &&
				drawablep && 
				drawablep->isActive())
			{
				if (!mFocusObject->isAvatar())
				{
					if (mFocusObject->isSelected())
					{
						gPipeline.updateMoveNormalAsync(drawablep);
					}
					else
					{
						if (drawablep->isState(LLDrawable::MOVE_UNDAMPED))
						{
							gPipeline.updateMoveNormalAsync(drawablep);
						}
						else
						{
							gPipeline.updateMoveDampedAsync(drawablep);
						}
					}
				}
			}
			// if not tracking object, update offset based on new object position
			else
			{
				updateFocusOffset();
			}
			LLVector3 focus_agent = mFocusObject->getRenderPosition() + mFocusObjectOffset;
			mFocusTargetGlobal.setVec(getPosGlobalFromAgent(focus_agent));
		}
		return mFocusTargetGlobal;
	}
	else if (mSitCameraEnabled && mAvatarObject.notNull() && mAvatarObject->mIsSitting && mSitCameraReferenceObject.notNull())
	{
		// sit camera
		LLVector3 object_pos = mSitCameraReferenceObject->getRenderPosition();
		LLQuaternion object_rot = mSitCameraReferenceObject->getRenderRotation();

		LLVector3 target_pos = object_pos + (mSitCameraFocus * object_rot);
		return getPosGlobalFromAgent(target_pos);
	}
	else
	{
		return getPositionGlobal() + calcThirdPersonFocusOffset();
	}
}

LLVector3d LLAgent::calcThirdPersonFocusOffset()
{
	// ...offset from avatar
	LLVector3d focus_offset;
	focus_offset.setVec(gSavedSettings.getVector3("FocusOffsetDefault"));

	LLQuaternion agent_rot = mFrameAgent.getQuaternion();
	if (!mAvatarObject.isNull() && mAvatarObject->getParent())
	{
		agent_rot *= ((LLViewerObject*)(mAvatarObject->getParent()))->getRenderRotation();
	}

	focus_offset = focus_offset * agent_rot;
	return focus_offset;
}

void LLAgent::setupSitCamera()
{
	// agent frame entering this function is in world coordinates
	if (mAvatarObject.notNull() && mAvatarObject->getParent())
	{
		LLQuaternion parent_rot = ((LLViewerObject*)mAvatarObject->getParent())->getRenderRotation();
		// slam agent coordinate frame to proper parent local version
		LLVector3 at_axis = mFrameAgent.getAtAxis();
		at_axis.mV[VZ] = 0.f;
		at_axis.normalize();
		resetAxes(at_axis * ~parent_rot);
	}
}

//-----------------------------------------------------------------------------
// getCameraPositionAgent()
//-----------------------------------------------------------------------------
const LLVector3 &LLAgent::getCameraPositionAgent() const
{
	return LLViewerCamera::getInstance()->getOrigin();
}

//-----------------------------------------------------------------------------
// getCameraPositionGlobal()
//-----------------------------------------------------------------------------
LLVector3d LLAgent::getCameraPositionGlobal() const
{
	return getPosGlobalFromAgent(LLViewerCamera::getInstance()->getOrigin());
}

//-----------------------------------------------------------------------------
// calcCameraFOVZoomFactor()
//-----------------------------------------------------------------------------
F32	LLAgent::calcCameraFOVZoomFactor()
{
	LLVector3 camera_offset_dir;
	camera_offset_dir.setVec(mCameraFocusOffset);

	if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		return 0.f;
	}
	else if (mFocusObject.notNull() && !mFocusObject->isAvatar())
	{
		// don't FOV zoom on mostly transparent objects
		LLVector3 focus_offset = mFocusObjectOffset;
		F32 obj_min_dist = 0.f;
		if (!gSavedSettings.getBOOL("PhoenixDisableMinZoomDist"))
			calcCameraMinDistance(obj_min_dist);
		F32 current_distance = llmax(0.001f, camera_offset_dir.magVec());

		mFocusObjectDist = obj_min_dist - current_distance;

		F32 new_fov_zoom = llclamp(mFocusObjectDist / current_distance, 0.f, 1000.f);
		return new_fov_zoom;
	}
	else // focusing on land or avatar
	{
		// keep old field of view until user changes focus explicitly
		return mCameraFOVZoomFactor;
		//return 0.f;
	}
}

//-----------------------------------------------------------------------------
// calcCameraPositionTargetGlobal()
//-----------------------------------------------------------------------------
LLVector3d LLAgent::calcCameraPositionTargetGlobal(BOOL *hit_limit)
{
	// Compute base camera position and look-at points.
	//F32			camera_land_height;
	LLVector3d	frame_center_global = mAvatarObject.isNull() ? getPositionGlobal() 
															 : getPosGlobalFromAgent(mAvatarObject->mRoot.getWorldPosition());
		
	LLVector3   upAxis = getUpAxis();
	BOOL		isConstrained = FALSE;
	LLVector3d	head_offset;
	head_offset.setVec(mThirdPersonHeadOffset);

	LLVector3d camera_position_global;

	// Ventrella 
	if ( mCameraMode == CAMERA_MODE_FOLLOW && mFocusOnAvatar )
	{
		camera_position_global = gAgent.getPosGlobalFromAgent(mFollowCam.getSimulatedPosition());
	}// End Ventrella
	else if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		if (mAvatarObject.isNull() || mAvatarObject->mDrawable.isNull())
		{
			llwarns << "Null avatar drawable!" << llendl;
			return LLVector3d::zero;
		}
		head_offset.clearVec();
		if (mAvatarObject->mIsSitting && mAvatarObject->getParent())
		{
			mAvatarObject->updateHeadOffset();
			head_offset.mdV[VX] = mAvatarObject->mHeadOffset.mV[VX];
			head_offset.mdV[VY] = mAvatarObject->mHeadOffset.mV[VY];
			head_offset.mdV[VZ] = mAvatarObject->mHeadOffset.mV[VZ] + 0.1f;
			const LLMatrix4& mat = ((LLViewerObject*) mAvatarObject->getParent())->getRenderMatrix();
			camera_position_global = getPosGlobalFromAgent
								((mAvatarObject->getPosition()+
								 LLVector3(head_offset)*mAvatarObject->getRotation()) * mat);
		}
		else
		{
			head_offset.mdV[VZ] = mAvatarObject->mHeadOffset.mV[VZ];
			if (mAvatarObject->mIsSitting)
			{
				head_offset.mdV[VZ] += 0.1;
			}
			camera_position_global = getPosGlobalFromAgent(mAvatarObject->getRenderPosition());//frame_center_global;
			head_offset = head_offset * mAvatarObject->getRenderRotation();
			camera_position_global = camera_position_global + head_offset;
		}
	}
	else if (mCameraMode == CAMERA_MODE_THIRD_PERSON && mFocusOnAvatar)
	{
		LLVector3 local_camera_offset;
		F32 camera_distance = 0.f;

		if (mSitCameraEnabled 
			&& mAvatarObject.notNull() 
			&& mAvatarObject->mIsSitting 
			&& mSitCameraReferenceObject.notNull())
		{
			// sit camera
			LLVector3 object_pos = mSitCameraReferenceObject->getRenderPosition();
			LLQuaternion object_rot = mSitCameraReferenceObject->getRenderRotation();

			LLVector3 target_pos = object_pos + (mSitCameraPos * object_rot);

			camera_position_global = getPosGlobalFromAgent(target_pos);
		}
		else
		{
			local_camera_offset = mCameraZoomFraction * mCameraOffsetDefault * gSavedSettings.getF32("CameraOffsetScale");
			
			// are we sitting down?
			if (mAvatarObject.notNull() && mAvatarObject->getParent())
			{
				LLQuaternion parent_rot = ((LLViewerObject*)mAvatarObject->getParent())->getRenderRotation();
				// slam agent coordinate frame to proper parent local version
				LLVector3 at_axis = mFrameAgent.getAtAxis() * parent_rot;
				at_axis.mV[VZ] = 0.f;
				at_axis.normalize();
				resetAxes(at_axis * ~parent_rot);

				local_camera_offset = local_camera_offset * mFrameAgent.getQuaternion() * parent_rot;
			}
			else
			{
				local_camera_offset = mFrameAgent.rotateToAbsolute( local_camera_offset );
			}

			if (!mCameraCollidePlane.isExactlyZero() && (mAvatarObject.isNull() || !mAvatarObject->mIsSitting))
			{
				LLVector3 plane_normal;
				plane_normal.setVec(mCameraCollidePlane.mV);

				F32 offset_dot_norm = local_camera_offset * plane_normal;
				if (llabs(offset_dot_norm) < 0.001f)
				{
					offset_dot_norm = 0.001f;
				}
				
				camera_distance = local_camera_offset.normalize();

				F32 pos_dot_norm = getPosAgentFromGlobal(frame_center_global + head_offset) * plane_normal;
				
				// if agent is outside the colliding half-plane
				if (pos_dot_norm > mCameraCollidePlane.mV[VW])
				{
					// check to see if camera is on the opposite side (inside) the half-plane
					if (offset_dot_norm + pos_dot_norm < mCameraCollidePlane.mV[VW])
					{
						// diminish offset by factor to push it back outside the half-plane
						camera_distance *= (pos_dot_norm - mCameraCollidePlane.mV[VW] - CAMERA_COLLIDE_EPSILON) / -offset_dot_norm;
					}
				}
				else
				{
					if (offset_dot_norm + pos_dot_norm > mCameraCollidePlane.mV[VW])
					{
						camera_distance *= (mCameraCollidePlane.mV[VW] - pos_dot_norm - CAMERA_COLLIDE_EPSILON) / offset_dot_norm;
					}
				}
			}
			else
			{
				camera_distance = local_camera_offset.normalize();
			}

			mTargetCameraDistance = camera_distance;//llmax(camera_distance, MIN_CAMERA_DISTANCE);

			if (mTargetCameraDistance != mCurrentCameraDistance)
			{
				F32 camera_lerp_amt = LLCriticalDamp::getInterpolant(CAMERA_ZOOM_HALF_LIFE);

				mCurrentCameraDistance = lerp(mCurrentCameraDistance, mTargetCameraDistance, camera_lerp_amt);
			}

			// Make the camera distance current
			local_camera_offset *= mCurrentCameraDistance;

			// set the global camera position
			LLVector3d camera_offset;
			
			LLVector3 av_pos = mAvatarObject.isNull() ? LLVector3::zero : mAvatarObject->getRenderPosition();
			camera_offset.setVec( local_camera_offset );
			camera_position_global = frame_center_global + head_offset + camera_offset;

			if (mAvatarObject.notNull())
			{
				LLVector3d camera_lag_d;
				F32 lag_interp = LLCriticalDamp::getInterpolant(CAMERA_LAG_HALF_LIFE);
				LLVector3 target_lag;
				LLVector3 vel = getVelocity();

				// lag by appropriate amount for flying
				F32 time_in_air = mAvatarObject->mTimeInAir.getElapsedTimeF32();
				if(!mCameraAnimating && mAvatarObject->mInAir && time_in_air > GROUND_TO_AIR_CAMERA_TRANSITION_START_TIME)
				{
					LLVector3 frame_at_axis = mFrameAgent.getAtAxis();
					frame_at_axis -= projected_vec(frame_at_axis, getReferenceUpVector());
					frame_at_axis.normalize();

					//transition smoothly in air mode, to avoid camera pop
					F32 u = (time_in_air - GROUND_TO_AIR_CAMERA_TRANSITION_START_TIME) / GROUND_TO_AIR_CAMERA_TRANSITION_TIME;
					u = llclamp(u, 0.f, 1.f);

					lag_interp *= u;

					if (gViewerWindow->getLeftMouseDown() && gViewerWindow->getLastPick().mObjectID == mAvatarObject->getID())
					{
						// disable camera lag when using mouse-directed steering
						target_lag.clearVec();
					}
					else
					{
						target_lag = vel * gSavedSettings.getF32("DynamicCameraStrength") / 30.f;
					}

					mCameraLag = lerp(mCameraLag, target_lag, lag_interp);

					F32 lag_dist = mCameraLag.magVec();
					if (lag_dist > MAX_CAMERA_LAG)
					{
						mCameraLag = mCameraLag * MAX_CAMERA_LAG / lag_dist;
					}

					// clamp camera lag so that avatar is always in front
					F32 dot = (mCameraLag - (frame_at_axis * (MIN_CAMERA_LAG * u))) * frame_at_axis;
					if (dot < -(MIN_CAMERA_LAG * u))
					{
						mCameraLag -= (dot + (MIN_CAMERA_LAG * u)) * frame_at_axis;
					}
				}
				else
				{
					mCameraLag = lerp(mCameraLag, LLVector3::zero, LLCriticalDamp::getInterpolant(0.15f));
				}

				camera_lag_d.setVec(mCameraLag);
				camera_position_global = camera_position_global - camera_lag_d;
			}
		}
	}
	else
	{
		LLVector3d focusPosGlobal = calcFocusPositionTargetGlobal();
		// camera gets pushed out later wrt mCameraFOVZoomFactor...this is "raw" value
		camera_position_global = focusPosGlobal + mCameraFocusOffset;
	}

	/*
	if (!gSavedSettings.getBOOL("DisableCameraConstraints") && !gAgent.isGodlike())
	{
		LLViewerRegion* regionp = LLWorld::getInstance()->getRegionFromPosGlobal(
			camera_position_global);
		bool constrain = true;
		if(regionp && regionp->canManageEstate())
		{
			constrain = false;
		}
		if(constrain)
		{
			F32 max_dist = ( CAMERA_MODE_CUSTOMIZE_AVATAR == mCameraMode ) ?
				APPEARANCE_MAX_ZOOM : mDrawDistance;

			LLVector3d camera_offset = camera_position_global
				- gAgent.getPositionGlobal();
			F32 camera_distance = (F32)camera_offset.magVec();

			if(camera_distance > max_dist)
			{
				camera_position_global = gAgent.getPositionGlobal() + 
					(max_dist / camera_distance) * camera_offset;
				isConstrained = TRUE;
			}
		}

// JC - Could constrain camera based on parcel stuff here.
//			LLViewerRegion *regionp = LLWorld::getInstance()->getRegionFromPosGlobal(camera_position_global);
//			
//			if (regionp && !regionp->mParcelOverlay->isBuildCameraAllowed(regionp->getPosRegionFromGlobal(camera_position_global)))
//			{
//				camera_position_global = last_position_global;
//
//				isConstrained = TRUE;
//			}
	}

	// Don't let camera go underground
	F32 camera_min_off_ground = getCameraMinOffGround();

	camera_land_height = LLWorld::getInstance()->resolveLandHeightGlobal(camera_position_global);

	if (camera_position_global.mdV[VZ] < camera_land_height + camera_min_off_ground)
	{
		camera_position_global.mdV[VZ] = camera_land_height + camera_min_off_ground;
		isConstrained = TRUE;
	}
	*/

	if (hit_limit)
	{
		*hit_limit = isConstrained;
	}

	return camera_position_global;
}


//-----------------------------------------------------------------------------
// handleScrollWheel()
//-----------------------------------------------------------------------------
void LLAgent::handleScrollWheel(S32 clicks)
{
	if ( mCameraMode == CAMERA_MODE_FOLLOW && gAgent.getFocusOnAvatar())
	{
		if ( ! mFollowCam.getPositionLocked() ) // not if the followCam position is locked in place
		{
			mFollowCam.zoom( clicks ); 
			if ( mFollowCam.isZoomedToMinimumDistance() )
			{
				changeCameraToMouselook(FALSE);
			}
		}
	}
	else
	{
		LLObjectSelectionHandle selection = LLSelectMgr::getInstance()->getSelection();
		const F32 ROOT_ROOT_TWO = sqrt(F_SQRT2);

		// Block if camera is animating
		if (mCameraAnimating)
		{
			return;
		}

		if (selection->getObjectCount() && selection->getSelectType() == SELECT_TYPE_HUD)
		{
			F32 zoom_factor = (F32)pow(0.8, -clicks);
			cameraZoomIn(zoom_factor);
		}
		else if (mFocusOnAvatar && mCameraMode == CAMERA_MODE_THIRD_PERSON)
		{
			if(gKeyboard->getKeyDown(KEY_CONTROL))
			{
				//mCameraOffsetDefault.mV[2]+=clicks/10.0f;;
				mThirdPersonHeadOffset.mV[2]+=clicks/10.0f; 
				
			}else if(gKeyboard->getKeyDown(KEY_SHIFT))
			{
				 gSavedSettings.setVector3("FocusOffsetDefault",
					gSavedSettings.getVector3("FocusOffsetDefault") + LLVector3(0.0f,0.0f,clicks/10.0f));
			}else
			{
			F32 current_zoom_fraction = mTargetCameraDistance / (mCameraOffsetDefault.magVec() * gSavedSettings.getF32("CameraOffsetScale"));
			current_zoom_fraction *= 1.f - pow(ROOT_ROOT_TWO, clicks);
			cameraOrbitIn(current_zoom_fraction * mCameraOffsetDefault.magVec() * gSavedSettings.getF32("CameraOffsetScale"));
		}
		}
		else
		{
			F32 current_zoom_fraction = (F32)mCameraFocusOffsetTarget.magVec();
			cameraOrbitIn(current_zoom_fraction * (1.f - pow(ROOT_ROOT_TWO, clicks)));
		}
	}
}


//-----------------------------------------------------------------------------
// getCameraMinOffGround()
//-----------------------------------------------------------------------------
F32 LLAgent::getCameraMinOffGround()
{
	if (mCameraMode == CAMERA_MODE_MOUSELOOK)
	{
		return 0.f;
	}
	else
	{
		if (gSavedSettings.getBOOL("DisableCameraConstraints"))
		{
			return -1000.f;
		}
		else
		{
			return 0.5f;
		}
	}
}


//-----------------------------------------------------------------------------
// resetCamera()
//-----------------------------------------------------------------------------
void LLAgent::resetCamera()
{
	// Remove any pitch from the avatar
	LLVector3 at = mFrameAgent.getAtAxis();
	at.mV[VZ] = 0.f;
	at.normalize();
	gAgent.resetAxes(at);
	// have to explicitly clear field of view zoom now
	mCameraFOVZoomFactor = 0.f;

	updateCamera();
}

//-----------------------------------------------------------------------------
// changeCameraToMouselook()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToMouselook(BOOL animate)
{
	if (LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	// visibility changes at end of animation
	gViewerWindow->getWindow()->resetBusyCount();

	// Menus should not remain open on switching to mouselook...
	LLMenuGL::sMenuContainer->hideMenus();
	
	// unpause avatar animation
	mPauseRequest = NULL;

	LLToolMgr::getInstance()->setCurrentToolset(gMouselookToolset);

	//gSavedSettings.setBOOL("FirstPersonBtnState",	FALSE);
	//gSavedSettings.setBOOL("MouselookBtnState",		TRUE);
	//gSavedSettings.setBOOL("ThirdPersonBtnState",	FALSE);
	//gSavedSettings.setBOOL("BuildBtnState",			FALSE);
	LLAgent::sFirstPersonBtnState = FALSE;
	LLAgent::sMouselookBtnState = TRUE;
	LLAgent::sThirdPersonBtnState = FALSE;
	LLAgent::sBuildBtnState = FALSE;

	if (mAvatarObject.notNull())
	{
		mAvatarObject->stopMotion( ANIM_AGENT_BODY_NOISE );
		mAvatarObject->stopMotion( ANIM_AGENT_BREATHE_ROT );
	}

	//gViewerWindow->stopGrab();
	LLSelectMgr::getInstance()->deselectAll();
	gViewerWindow->hideCursor();
	gViewerWindow->moveCursorToCenter();

	if( mCameraMode != CAMERA_MODE_MOUSELOOK )
	{
		gFocusMgr.setKeyboardFocus( NULL );

		if (gSavedPerAccountSettings.getBOOL("PhoenixAONoStandsInMouselook"))
			LLFloaterAO::stopAOMotion(LLFloaterAO::getCurrentStandId(), TRUE);

		mLastCameraMode = mCameraMode;
		mCameraMode = CAMERA_MODE_MOUSELOOK;
		U32 old_flags = mControlFlags;
		setControlFlags(AGENT_CONTROL_MOUSELOOK);
		if (old_flags != mControlFlags)
		{
			mbFlagsDirty = TRUE;
		}

		if (animate)
		{
			startCameraAnimation();
		}
		else
		{
			mCameraAnimating = FALSE;
			endAnimationUpdateUI();
		}
	}
}


//-----------------------------------------------------------------------------
// changeCameraToDefault()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToDefault()
{
	if (LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	if (LLFollowCamMgr::getActiveFollowCamParams())
	{
		changeCameraToFollow();
	}
	else
	{
		changeCameraToThirdPerson();
	}
}


// Ventrella
//-----------------------------------------------------------------------------
// changeCameraToFollow()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToFollow(BOOL animate)
{
	if (LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	if( mCameraMode != CAMERA_MODE_FOLLOW )
	{
		if (mCameraMode == CAMERA_MODE_MOUSELOOK)
		{
			animate = FALSE;
		}
		startCameraAnimation();

		mLastCameraMode = mCameraMode;
		mCameraMode = CAMERA_MODE_FOLLOW;

		// bang-in the current focus, position, and up vector of the follow cam
		mFollowCam.reset( mCameraPositionAgent, LLViewerCamera::getInstance()->getPointOfInterest(), LLVector3::z_axis );
		
		if (gBasicToolset)
		{
			LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		}

		if (mAvatarObject.notNull())
		{
			mAvatarObject->mPelvisp->setPosition(LLVector3::zero);
			mAvatarObject->startMotion( ANIM_AGENT_BODY_NOISE );
			mAvatarObject->startMotion( ANIM_AGENT_BREATHE_ROT );
		}

		//gSavedSettings.setBOOL("FirstPersonBtnState",	FALSE);
		//gSavedSettings.setBOOL("MouselookBtnState",		FALSE);
		//gSavedSettings.setBOOL("ThirdPersonBtnState",	TRUE);
		//gSavedSettings.setBOOL("BuildBtnState",			FALSE);
		LLAgent::sFirstPersonBtnState = FALSE;
		LLAgent::sMouselookBtnState = FALSE;
		LLAgent::sThirdPersonBtnState = TRUE;
		LLAgent::sBuildBtnState = FALSE;

		// unpause avatar animation
		mPauseRequest = NULL;

		U32 old_flags = mControlFlags;
		clearControlFlags(AGENT_CONTROL_MOUSELOOK);
		if (old_flags != mControlFlags)
		{
			mbFlagsDirty = TRUE;
		}

		if (animate)
		{
			startCameraAnimation();
		}
		else
		{
			mCameraAnimating = FALSE;
			endAnimationUpdateUI();
		}
	}
}

//-----------------------------------------------------------------------------
// changeCameraToThirdPerson()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToThirdPerson(BOOL animate)
{
	if (LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	gViewerWindow->getWindow()->resetBusyCount();

	mCameraZoomFraction = INITIAL_ZOOM_FRACTION;

	//gSavedSettings.setVector3("FocusOffsetDefault",LLVector3(1.0f,0.0f,1.0f));
	mCameraOffsetDefault = gSavedSettings.getVector3("CameraOffsetDefault");
	mThirdPersonHeadOffset=LLVector3(0.0f,0.0f,1.0f);


	if (mAvatarObject.notNull())
	{
		if (!mAvatarObject->mIsSitting)
		{
			mAvatarObject->mPelvisp->setPosition(LLVector3::zero);
		}
		mAvatarObject->startMotion( ANIM_AGENT_BODY_NOISE );
		mAvatarObject->startMotion( ANIM_AGENT_BREATHE_ROT );
	}

	//gSavedSettings.setBOOL("FirstPersonBtnState",	FALSE);
	//gSavedSettings.setBOOL("MouselookBtnState",		FALSE);
	//gSavedSettings.setBOOL("ThirdPersonBtnState",	TRUE);
	//gSavedSettings.setBOOL("BuildBtnState",			FALSE);
	LLAgent::sFirstPersonBtnState = FALSE;
	LLAgent::sMouselookBtnState = FALSE;
	LLAgent::sThirdPersonBtnState = TRUE;
	LLAgent::sBuildBtnState = FALSE;

	LLVector3 at_axis;

	// unpause avatar animation
	mPauseRequest = NULL;

	if( mCameraMode != CAMERA_MODE_THIRD_PERSON )
	{
		if (gBasicToolset)
		{
			LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		}

		mCameraLag.clearVec();
		if (mCameraMode == CAMERA_MODE_MOUSELOOK)
		{
			mCurrentCameraDistance = MIN_CAMERA_DISTANCE;
			mTargetCameraDistance = MIN_CAMERA_DISTANCE;
			animate = FALSE;
		}
		mLastCameraMode = mCameraMode;
		mCameraMode = CAMERA_MODE_THIRD_PERSON;
		U32 old_flags = mControlFlags;
		clearControlFlags(AGENT_CONTROL_MOUSELOOK);
		if (old_flags != mControlFlags)
		{
			mbFlagsDirty = TRUE;
		}

	}

	// Remove any pitch from the avatar
	if (mAvatarObject.notNull() && mAvatarObject->getParent())
	{
		LLQuaternion obj_rot = ((LLViewerObject*)mAvatarObject->getParent())->getRenderRotation();
		at_axis = LLViewerCamera::getInstance()->getAtAxis();
		at_axis.mV[VZ] = 0.f;
		at_axis.normalize();
		resetAxes(at_axis * ~obj_rot);
	}
	else
	{
		at_axis = mFrameAgent.getAtAxis();
		at_axis.mV[VZ] = 0.f;
		at_axis.normalize();
		resetAxes(at_axis);
	}


	if (animate)
	{
		startCameraAnimation();
	}
	else
	{
		mCameraAnimating = FALSE;
		endAnimationUpdateUI();
	}
}

//-----------------------------------------------------------------------------
// changeCameraToCustomizeAvatar()
//-----------------------------------------------------------------------------
void LLAgent::changeCameraToCustomizeAvatar(BOOL avatar_animate, BOOL camera_animate)
{
	if (LLViewerJoystick::getInstance()->getOverrideCamera())
	{
		return;
	}

	if(gSavedSettings.getBOOL("PhoenixAppearanceForceStand"))
// [RLVa:KB] - Checked: 2009-07-10 (RLVa-1.0.0g)
	if ( (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && (mAvatarObject.notNull()) && (mAvatarObject->mIsSitting) )
	{
		return;
	}
// [/RLVa:KB]
	if(gSavedSettings.getBOOL("PhoenixAppearanceForceStand"))
	setControlFlags(AGENT_CONTROL_STAND_UP); // force stand up
	gViewerWindow->getWindow()->resetBusyCount();

	if (gFaceEditToolset)
	{
		LLToolMgr::getInstance()->setCurrentToolset(gFaceEditToolset);
	}

	//gSavedSettings.setBOOL("FirstPersonBtnState", FALSE);
	//gSavedSettings.setBOOL("MouselookBtnState", FALSE);
	//gSavedSettings.setBOOL("ThirdPersonBtnState", FALSE);
	//gSavedSettings.setBOOL("BuildBtnState", FALSE);
	LLAgent::sFirstPersonBtnState = FALSE;
	LLAgent::sMouselookBtnState = FALSE;
	LLAgent::sThirdPersonBtnState = FALSE;
	LLAgent::sBuildBtnState = FALSE;

	if (camera_animate)
	{
		startCameraAnimation();
	}

	// Remove any pitch from the avatar
	//LLVector3 at = mFrameAgent.getAtAxis();
	//at.mV[VZ] = 0.f;
	//at.normalize();
	//gAgent.resetAxes(at);

	if( mCameraMode != CAMERA_MODE_CUSTOMIZE_AVATAR )
	{
		mLastCameraMode = mCameraMode;
		mCameraMode = CAMERA_MODE_CUSTOMIZE_AVATAR;
		U32 old_flags = mControlFlags;
		clearControlFlags(AGENT_CONTROL_MOUSELOOK);
		if (old_flags != mControlFlags)
		{
			mbFlagsDirty = TRUE;
		}

		gFocusMgr.setKeyboardFocus( NULL );
		gFocusMgr.setMouseCapture( NULL );

		LLVOAvatar::onCustomizeStart();
	}

	if (mAvatarObject.notNull())
	{
		if(avatar_animate  && gSavedSettings.getBOOL("PhoenixAppearanceAnimate"))
		{
			// Remove any pitch from the avatar
			LLVector3 at = mFrameAgent.getAtAxis();
			at.mV[VZ] = 0.f;
			at.normalize();
			gAgent.resetAxes(at);

			sendAnimationRequest(ANIM_AGENT_CUSTOMIZE, ANIM_REQUEST_START);
			mCustomAnim = TRUE ;
			mAvatarObject->startMotion(ANIM_AGENT_CUSTOMIZE);
			LLMotion* turn_motion = mAvatarObject->findMotion(ANIM_AGENT_CUSTOMIZE);

			if (turn_motion)
			{
				mAnimationDuration = turn_motion->getDuration() + CUSTOMIZE_AVATAR_CAMERA_ANIM_SLOP;

			}
			else
			{
				mAnimationDuration = gSavedSettings.getF32("ZoomTime");
			}
		}



		gAgent.setFocusGlobal(LLVector3d::zero);
	}
	else
	{
		mCameraAnimating = FALSE;
		endAnimationUpdateUI();
	}

}


//
// Focus point management
//

//-----------------------------------------------------------------------------
// startCameraAnimation()
//-----------------------------------------------------------------------------
void LLAgent::startCameraAnimation()
{
	mAnimationCameraStartGlobal = getCameraPositionGlobal();
	mAnimationFocusStartGlobal = mFocusGlobal;
	mAnimationTimer.reset();
	mCameraAnimating = TRUE;
	mAnimationDuration = gSavedSettings.getF32("ZoomTime");
}

//-----------------------------------------------------------------------------
// stopCameraAnimation()
//-----------------------------------------------------------------------------
void LLAgent::stopCameraAnimation()
{
	mCameraAnimating = FALSE;
}

void LLAgent::clearFocusObject()
{
	if (mFocusObject.notNull())
	{
		startCameraAnimation();

		setFocusObject(NULL);
		mFocusObjectOffset.clearVec();
	}
}

void LLAgent::setFocusObject(LLViewerObject* object)
{
	mFocusObject = object;
}

// Focus on a point, but try to keep camera position stable.
//-----------------------------------------------------------------------------
// setFocusGlobal()
//-----------------------------------------------------------------------------
void LLAgent::setFocusGlobal(const LLPickInfo& pick)
{
	LLViewerObject* objectp = gObjectList.findObject(pick.mObjectID);

	if (objectp)
	{
		// focus on object plus designated offset
		// which may or may not be same as pick.mPosGlobal
		setFocusGlobal(objectp->getPositionGlobal() + LLVector3d(pick.mObjectOffset), pick.mObjectID);
	}
	else
	{
		// focus directly on point where user clicked
		setFocusGlobal(pick.mPosGlobal, pick.mObjectID);
	}
}


void LLAgent::setFocusGlobal(const LLVector3d& focus, const LLUUID &object_id)
{
	setFocusObject(gObjectList.findObject(object_id));
	LLVector3d old_focus = mFocusTargetGlobal;
	LLViewerObject *focus_obj = mFocusObject;

	// if focus has changed
	if (old_focus != focus)
	{
		if (focus.isExactlyZero())
		{
			if (mAvatarObject.notNull())
			{
				mFocusTargetGlobal = getPosGlobalFromAgent(mAvatarObject->mHeadp->getWorldPosition());
			}
			else
			{
				mFocusTargetGlobal = getPositionGlobal();
			}
			mCameraFocusOffsetTarget = getCameraPositionGlobal() - mFocusTargetGlobal;
			mCameraFocusOffset = mCameraFocusOffsetTarget;
			setLookAt(LOOKAT_TARGET_CLEAR);
		}
		else
		{
			mFocusTargetGlobal = focus;
			if (!focus_obj)
			{
				mCameraFOVZoomFactor = 0.f;
			}

			mCameraFocusOffsetTarget = gAgent.getPosGlobalFromAgent(mCameraVirtualPositionAgent) - mFocusTargetGlobal;

			startCameraAnimation();

			if (focus_obj)
			{
				if (focus_obj->isAvatar())
				{
					setLookAt(LOOKAT_TARGET_FOCUS, focus_obj);
				}
				else
				{
					setLookAt(LOOKAT_TARGET_FOCUS, focus_obj, (getPosAgentFromGlobal(focus) - focus_obj->getRenderPosition()) * ~focus_obj->getRenderRotation());
				}
			}
			else
			{
				setLookAt(LOOKAT_TARGET_FOCUS, NULL, getPosAgentFromGlobal(mFocusTargetGlobal));
			}
		}
	}
	else // focus == mFocusTargetGlobal
	{
		if (focus.isExactlyZero())
		{
			if (mAvatarObject.notNull())
			{
				mFocusTargetGlobal = getPosGlobalFromAgent(mAvatarObject->mHeadp->getWorldPosition());
			}
			else
			{
				mFocusTargetGlobal = getPositionGlobal();
			}
		}
		mCameraFocusOffsetTarget = (getCameraPositionGlobal() - mFocusTargetGlobal) / (1.f + mCameraFOVZoomFactor);;
		mCameraFocusOffset = mCameraFocusOffsetTarget;
	}

	if (mFocusObject.notNull())
	{
		// for attachments, make offset relative to avatar, not the attachment
		if (mFocusObject->isAttachment())
		{
			while (mFocusObject.notNull()		// DEV-29123 - can crash with a messed-up attachment
				&& !mFocusObject->isAvatar())
			{
				mFocusObject = (LLViewerObject*) mFocusObject->getParent();
			}
			setFocusObject((LLViewerObject*)mFocusObject);
		}
		updateFocusOffset();
	}
}

// Used for avatar customization
//-----------------------------------------------------------------------------
// setCameraPosAndFocusGlobal()
//-----------------------------------------------------------------------------
void LLAgent::setCameraPosAndFocusGlobal(const LLVector3d& camera_pos, const LLVector3d& focus, const LLUUID &object_id)
{
	LLVector3d old_focus = mFocusTargetGlobal;

	F64 focus_delta_squared = (old_focus - focus).magVecSquared();
	const F64 ANIM_EPSILON_SQUARED = 0.0001;
	if( focus_delta_squared > ANIM_EPSILON_SQUARED )
	{
		startCameraAnimation();

		if( CAMERA_MODE_CUSTOMIZE_AVATAR == mCameraMode ) 
		{
			// Compensate for the fact that the camera has already been offset to make room for LLFloaterCustomize.
			mAnimationCameraStartGlobal -= LLVector3d(LLViewerCamera::getInstance()->getLeftAxis() * calcCustomizeAvatarUIOffset( mAnimationCameraStartGlobal ));
		}
	}
	
	//LLViewerCamera::getInstance()->setOrigin( gAgent.getPosAgentFromGlobal( camera_pos ) );
	setFocusObject(gObjectList.findObject(object_id));
	mFocusTargetGlobal = focus;
	mCameraFocusOffsetTarget = camera_pos - focus;
	mCameraFocusOffset = mCameraFocusOffsetTarget;

	if (mFocusObject)
	{
		if (mFocusObject->isAvatar())
		{
			setLookAt(LOOKAT_TARGET_FOCUS, mFocusObject);
		}
		else
		{
			setLookAt(LOOKAT_TARGET_FOCUS, mFocusObject, (getPosAgentFromGlobal(focus) - mFocusObject->getRenderPosition()) * ~mFocusObject->getRenderRotation());
		}
	}
	else
	{
		setLookAt(LOOKAT_TARGET_FOCUS, NULL, getPosAgentFromGlobal(mFocusTargetGlobal));
	}

	if( mCameraAnimating )
	{
		const F64 ANIM_METERS_PER_SECOND = 10.0;
		const F64 MIN_ANIM_SECONDS = 0.5;
		F64 anim_duration = llmax( MIN_ANIM_SECONDS, sqrt(focus_delta_squared) / ANIM_METERS_PER_SECOND );
		setAnimationDuration( (F32)anim_duration );
	}

	updateFocusOffset();
}

//-----------------------------------------------------------------------------
// setSitCamera()
//-----------------------------------------------------------------------------
void LLAgent::setSitCamera(const LLUUID &object_id, const LLVector3 &camera_pos, const LLVector3 &camera_focus)
{
	BOOL camera_enabled = !object_id.isNull();

	if (camera_enabled)
	{
		LLViewerObject *reference_object = gObjectList.findObject(object_id);
		if (reference_object)
		{
			//convert to root object relative?
			mSitCameraPos = camera_pos;
			mSitCameraFocus = camera_focus;
			mSitCameraReferenceObject = reference_object;
			mSitCameraEnabled = TRUE;
		}
	}
	else
	{
		mSitCameraPos.clearVec();
		mSitCameraFocus.clearVec();
		mSitCameraReferenceObject = NULL;
		mSitCameraEnabled = FALSE;
	}
}

//-----------------------------------------------------------------------------
// setFocusOnAvatar()
//-----------------------------------------------------------------------------
void LLAgent::setFocusOnAvatar(BOOL focus_on_avatar, BOOL animate)
{
	if (focus_on_avatar != mFocusOnAvatar)
	{
		if (animate)
		{
			startCameraAnimation();
		}
		else
		{
			stopCameraAnimation();
		}
	}
	
	//RN: when focused on the avatar, we're not "looking" at it
	// looking implies intent while focusing on avatar means
	// you're just walking around with a camera on you...eesh.
	if (!mFocusOnAvatar && focus_on_avatar)
	{
		setFocusGlobal(LLVector3d::zero);
		mCameraFOVZoomFactor = 0.f;
		if (mCameraMode == CAMERA_MODE_THIRD_PERSON)
		{
			LLVector3 at_axis;
			if (mAvatarObject.notNull() && mAvatarObject->getParent())
			{
				LLQuaternion obj_rot = ((LLViewerObject*)mAvatarObject->getParent())->getRenderRotation();
				at_axis = LLViewerCamera::getInstance()->getAtAxis();
				at_axis.mV[VZ] = 0.f;
				at_axis.normalize();
				resetAxes(at_axis * ~obj_rot);
			}
			else
			{
				at_axis = LLViewerCamera::getInstance()->getAtAxis();
				at_axis.mV[VZ] = 0.f;
				at_axis.normalize();
				resetAxes(at_axis);
			}
		}
	}
	// unlocking camera from avatar
	else if (mFocusOnAvatar && !focus_on_avatar)
	{
		// keep camera focus point consistent, even though it is now unlocked
		setFocusGlobal(getPositionGlobal() + calcThirdPersonFocusOffset(), gAgent.getID());
	}
	
	mFocusOnAvatar = focus_on_avatar;
}

//-----------------------------------------------------------------------------
// heardChat()
//-----------------------------------------------------------------------------
void LLAgent::heardChat(const LLUUID& id)
{
	// log text and voice chat to speaker mgr
	// for keeping track of active speakers, etc.
	LLLocalSpeakerMgr::getInstance()->speakerChatted(id);

	// don't respond to your own voice
	if (id == getID()) return;
	
	if (ll_rand(2) == 0) 
	{
		LLViewerObject *chatter = gObjectList.findObject(mLastChatterID);
		setLookAt(LOOKAT_TARGET_AUTO_LISTEN, chatter, LLVector3::zero);
	}			

	mLastChatterID = id;
	mChatTimer.reset();
}

//-----------------------------------------------------------------------------
// lookAtLastChat()
//-----------------------------------------------------------------------------
void LLAgent::lookAtLastChat()
{
	// Block if camera is animating or not in normal third person camera mode
	if (mCameraAnimating || !cameraThirdPerson())
	{
		return;
	}

	LLViewerObject *chatter = gObjectList.findObject(mLastChatterID);
	if (chatter)
	{
		LLVector3 delta_pos;
		if (chatter->isAvatar())
		{
			LLVOAvatar *chatter_av = (LLVOAvatar*)chatter;
			if (mAvatarObject.notNull() && chatter_av->mHeadp)
			{
				delta_pos = chatter_av->mHeadp->getWorldPosition() - mAvatarObject->mHeadp->getWorldPosition();
			}
			else
			{
				delta_pos = chatter->getPositionAgent() - getPositionAgent();
			}
			delta_pos.normalize();

			setControlFlags(AGENT_CONTROL_STOP);

			changeCameraToThirdPerson();

			LLVector3 new_camera_pos = mAvatarObject->mHeadp->getWorldPosition();
			LLVector3 left = delta_pos % LLVector3::z_axis;
			left.normalize();
			LLVector3 up = left % delta_pos;
			up.normalize();
			new_camera_pos -= delta_pos * 0.4f;
			new_camera_pos += left * 0.3f;
			new_camera_pos += up * 0.2f;
			if (chatter_av->mHeadp)
			{
				setFocusGlobal(getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition()), mLastChatterID);
				mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - gAgent.getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition());
			}
			else
			{
				setFocusGlobal(chatter->getPositionGlobal(), mLastChatterID);
				mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();
			}
			setFocusOnAvatar(FALSE, TRUE);
		}
		else
		{
			delta_pos = chatter->getRenderPosition() - getPositionAgent();
			delta_pos.normalize();

			setControlFlags(AGENT_CONTROL_STOP);

			changeCameraToThirdPerson();

			LLVector3 new_camera_pos = mAvatarObject->mHeadp->getWorldPosition();
			LLVector3 left = delta_pos % LLVector3::z_axis;
			left.normalize();
			LLVector3 up = left % delta_pos;
			up.normalize();
			new_camera_pos -= delta_pos * 0.4f;
			new_camera_pos += left * 0.3f;
			new_camera_pos += up * 0.2f;

			setFocusGlobal(chatter->getPositionGlobal(), mLastChatterID);
			mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();
			setFocusOnAvatar(FALSE, TRUE);
		}
	}
}

void LLAgent::lookAtObject(LLUUID object_id, ECameraPosition camera_pos)
{
	// Block if camera is animating or not in normal third person camera mode
	if (mCameraAnimating || !cameraThirdPerson())
	{
		return;
	}

	LLViewerObject *chatter = gObjectList.findObject(object_id);
	if (chatter)
	{
		LLVector3 delta_pos;
		if (chatter->isAvatar())
		{
			LLVOAvatar *chatter_av = (LLVOAvatar*)chatter;
			if (!mAvatarObject.isNull() && chatter_av->mHeadp)
			{
				delta_pos = chatter_av->mHeadp->getWorldPosition() - mAvatarObject->mHeadp->getWorldPosition();
			}
			else
			{
				delta_pos = chatter->getPositionAgent() - getPositionAgent();
			}
			delta_pos.normVec();

			setControlFlags(AGENT_CONTROL_STOP);

			changeCameraToThirdPerson();

			LLVector3 new_camera_pos = mAvatarObject->mHeadp->getWorldPosition();
			LLVector3 left = delta_pos % LLVector3::z_axis;
			left.normVec();
			LLVector3 up = left % delta_pos;
			up.normVec();
			new_camera_pos -= delta_pos * 0.4f;
			new_camera_pos += left * 0.3f;
			new_camera_pos += up * 0.2f;

			F32 radius = chatter_av->getVObjRadius();
			LLVector3d view_dist(radius, radius, 0.0f);

			if (chatter_av->mHeadp)
			{
				setFocusGlobal(getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition()), object_id);
				mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - gAgent.getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition());

				switch(camera_pos)
				{
					case CAMERA_POSITION_SELF:
						mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - gAgent.getPosGlobalFromAgent(chatter_av->mHeadp->getWorldPosition());
						break;
					case CAMERA_POSITION_OBJECT:
						mCameraFocusOffsetTarget =  view_dist;
						break;
				}
			}
			else
			{
				setFocusGlobal(chatter->getPositionGlobal(), object_id);
				mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();

				switch(camera_pos)
				{
					case CAMERA_POSITION_SELF:
						mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();
						break;
					case CAMERA_POSITION_OBJECT:
						mCameraFocusOffsetTarget = view_dist;
						break;
				}
			}
			setFocusOnAvatar(FALSE, TRUE);
		}
		else
		{
			delta_pos = chatter->getRenderPosition() - getPositionAgent();
			delta_pos.normVec();

			setControlFlags(AGENT_CONTROL_STOP);

			changeCameraToThirdPerson();

			LLVector3 new_camera_pos = mAvatarObject->mHeadp->getWorldPosition();
			LLVector3 left = delta_pos % LLVector3::z_axis;
			left.normVec();
			LLVector3 up = left % delta_pos;
			up.normVec();
			new_camera_pos -= delta_pos * 0.4f;
			new_camera_pos += left * 0.3f;
			new_camera_pos += up * 0.2f;

			setFocusGlobal(chatter->getPositionGlobal(), object_id);

			switch(camera_pos)
			{
				case CAMERA_POSITION_SELF:
					mCameraFocusOffsetTarget = getPosGlobalFromAgent(new_camera_pos) - chatter->getPositionGlobal();
					break;
				case CAMERA_POSITION_OBJECT:
					F32 radius = chatter->getVObjRadius();
					LLVector3d view_dist(radius, radius, 0.0f);
					mCameraFocusOffsetTarget = view_dist;
					break;
			}

			setFocusOnAvatar(FALSE, TRUE);
		}
	}
}

const F32 SIT_POINT_EXTENTS = 0.2f;

void LLAgent::setStartPosition( U32 location_id )
{
  LLViewerObject          *object;

  if ( !(gAgentID == LLUUID::null) )
  {
    // we've got an ID for an agent viewerobject
    object = gObjectList.findObject(gAgentID);
    if (object)
    {
      // we've got the viewer object
      // Sometimes the agent can be velocity interpolated off of
      // this simulator.  Clamp it to the region the agent is
      // in, a little bit in on each side.
      const F32 INSET = 0.5f; //meters
      const F32 REGION_WIDTH = LLWorld::getInstance()->getRegionWidthInMeters();

      LLVector3 agent_pos = getPositionAgent();
      LLVector3 agent_look_at = mFrameAgent.getAtAxis();

      if (mAvatarObject.notNull())
      {
	// the z height is at the agent's feet
	agent_pos.mV[VZ] -= 0.5f * mAvatarObject->mBodySize.mV[VZ];
      }

      agent_pos.mV[VX] = llclamp( agent_pos.mV[VX], INSET, REGION_WIDTH - INSET );
      agent_pos.mV[VY] = llclamp( agent_pos.mV[VY], INSET, REGION_WIDTH - INSET );

      // Don't let them go below ground, or too high.
      agent_pos.mV[VZ] = llclamp( agent_pos.mV[VZ],
				  mRegionp->getLandHeightRegion( agent_pos ),
				  LLWorld::getInstance()->getRegionMaxHeight() );
      // Send the CapReq

      LLSD body;

      std::string url = gAgent.getRegion()->getCapability("HomeLocation");
      std::ostringstream strBuffer;
      if( url.empty() )
      {
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_SetStartLocationRequest);
	msg->nextBlockFast( _PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, getID());
	msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
	msg->nextBlockFast( _PREHASH_StartLocationData);
	// corrected by sim
	msg->addStringFast(_PREHASH_SimName, "");
	msg->addU32Fast(_PREHASH_LocationID, location_id);
	msg->addVector3Fast(_PREHASH_LocationPos, agent_pos);
	msg->addVector3Fast(_PREHASH_LocationLookAt,mFrameAgent.getAtAxis());
	
	// Reliable only helps when setting home location.  Last
	// location is sent on quit, and we don't have time to ack
	// the packets.
	msg->sendReliable(mRegionp->getHost());

	const U32 HOME_INDEX = 1;
	if( HOME_INDEX == location_id )
	  {
	    setHomePosRegion( mRegionp->getHandle(), getPositionAgent() );
	  }
      }
      else
      {
	strBuffer << location_id;
	body["HomeLocation"]["LocationId"] = strBuffer.str();

	strBuffer.str("");
	strBuffer << agent_pos.mV[VX];
	body["HomeLocation"]["LocationPos"]["X"] = strBuffer.str();

	strBuffer.str("");
	strBuffer << agent_pos.mV[VY];
	body["HomeLocation"]["LocationPos"]["Y"] = strBuffer.str();

	strBuffer.str("");
	strBuffer << agent_pos.mV[VZ];
	body["HomeLocation"]["LocationPos"]["Z"] = strBuffer.str();

	strBuffer.str("");
	strBuffer << agent_look_at.mV[VX];
	body["HomeLocation"]["LocationLookAt"]["X"] = strBuffer.str();

	strBuffer.str("");
	strBuffer << agent_look_at.mV[VY];
	body["HomeLocation"]["LocationLookAt"]["Y"] = strBuffer.str();

	strBuffer.str("");
	strBuffer << agent_look_at.mV[VZ];
	body["HomeLocation"]["LocationLookAt"]["Z"] = strBuffer.str();

	LLHTTPClient::post( url, body, new LLHomeLocationResponder() );
      }
    }
    else
    {
      llinfos << "setStartPosition - Can't find agent viewerobject id " << gAgentID << llendl;
    }
  }
}

void LLAgent::requestStopMotion( LLMotion* motion )
{
	// Notify all avatars that a motion has stopped.
	// This is needed to clear the animation state bits
	LLUUID anim_state = motion->getID();
	onAnimStop(motion->getID());

	// if motion is not looping, it could have stopped by running out of time
	// so we need to tell the server this
//	llinfos << "Sending stop for motion " << motion->getName() << llendl;
	sendAnimationRequest( anim_state, ANIM_REQUEST_STOP );
}

void LLAgent::onAnimStop(const LLUUID& id)
{
	// handle automatic state transitions (based on completion of animation playback)
	if (id == ANIM_AGENT_STAND)
	{
		stopFidget();
	}
	else if (id == ANIM_AGENT_AWAY)
	{
		//clearAFK();
// [RLVa:KB] - Checked: 2009-10-19 (RLVa-1.1.0g) | Added: RLVa-1.1.0g
#ifdef RLV_EXTENSION_CMD_ALLOWIDLE
		if (!gRlvHandler.hasBehaviour(RLV_BHVR_ALLOWIDLE))
			clearAFK();
#else
		clearAFK();
#endif // RLV_EXTENSION_CMD_ALLOWIDLE
// [/RLVa:KB]
	}
	else if (id == ANIM_AGENT_STANDUP)
	{
                // send stand up command
                if(!gSavedSettings.getBOOL("PhoenixIgnoreFinishAnimation"))
                {
                        setControlFlags(AGENT_CONTROL_FINISH_ANIM);
                }

		// now trigger dusting self off animation
		if (mAvatarObject.notNull() && !mAvatarObject->mBelowWater && rand() % 3 == 0)
			sendAnimationRequest( ANIM_AGENT_BRUSH, ANIM_REQUEST_START );
	}
	else if (id == ANIM_AGENT_PRE_JUMP || id == ANIM_AGENT_LAND || id == ANIM_AGENT_MEDIUM_LAND)
	{
                if(!gSavedSettings.getBOOL("PhoenixIgnoreFinishAnimation"))
                {
                        setControlFlags(AGENT_CONTROL_FINISH_ANIM);
                }
	}
}

BOOL LLAgent::isGodlike() const
{
	return mAgentAccess.isGodlike();
}

U8 LLAgent::getGodLevel() const
{
	return mAgentAccess.getGodLevel();
}

bool LLAgent::wantsPGOnly() const
{
	return mAgentAccess.wantsPGOnly();
}

bool LLAgent::canAccessMature() const
{
	return mAgentAccess.canAccessMature();
}

bool LLAgent::canAccessAdult() const
{
	return mAgentAccess.canAccessAdult();
}

bool LLAgent::canAccessMaturityInRegion( U64 region_handle ) const
{
	LLViewerRegion *regionp = LLWorld::getInstance()->getRegionFromHandle( region_handle );
	if( regionp )
	{
		switch( regionp->getSimAccess() )
		{
			case SIM_ACCESS_MATURE:
				if( !canAccessMature() )
					return false;
				break;
			case SIM_ACCESS_ADULT:
				if( !canAccessAdult() )
					return false;
				break;
			default:
				// Oh, go on and hear the silly noises.
				break;
		}
	}
	
	return true;
}

bool LLAgent::canAccessMaturityAtGlobal( LLVector3d pos_global ) const
{
	U64 region_handle = to_region_handle_global( pos_global.mdV[0], pos_global.mdV[1] );
	return canAccessMaturityInRegion( region_handle );
}

bool LLAgent::prefersPG() const
{
	return mAgentAccess.prefersPG();
}

bool LLAgent::prefersMature() const
{
	return mAgentAccess.prefersMature();
}
	
bool LLAgent::prefersAdult() const
{
	return mAgentAccess.prefersAdult();
}

bool LLAgent::isTeen() const
{
	return mAgentAccess.isTeen();
}

bool LLAgent::isMature() const
{
	return mAgentAccess.isMature();
}

bool LLAgent::isAdult() const
{
	return mAgentAccess.isAdult();
}

void LLAgent::setTeen(bool teen)
{
	mAgentAccess.setTeen(teen);
}

//static 
int LLAgent::convertTextToMaturity(char text)
{
	return LLAgentAccess::convertTextToMaturity(text);
}

bool LLAgent::sendMaturityPreferenceToServer(int preferredMaturity)
{
	// Update agent access preference on the server
	std::string url = getRegion()->getCapability("UpdateAgentInformation");
	if (!url.empty())
	{
		// Set new access preference
		LLSD access_prefs = LLSD::emptyMap();
		if (preferredMaturity == SIM_ACCESS_PG)
		{
			access_prefs["max"] = "PG";
		}
		else if (preferredMaturity == SIM_ACCESS_MATURE)
		{
			access_prefs["max"] = "M";
		}
		if (preferredMaturity == SIM_ACCESS_ADULT)
		{
			access_prefs["max"] = "A";
		}
		
		LLSD body = LLSD::emptyMap();
		body["access_prefs"] = access_prefs;
		llinfos << "Sending access prefs update to " << (access_prefs["max"].asString()) << " via capability to: "
		<< url << llendl;
		LLHTTPClient::post(url, body, new LLHTTPClient::Responder());    // Ignore response
		return true;
	}
	return false;
}

BOOL LLAgent::getAdminOverride() const	
{ 
	return mAgentAccess.getAdminOverride(); 
}

void LLAgent::setMaturity(char text)
{
	mAgentAccess.setMaturity(text);
}

void LLAgent::setAdminOverride(BOOL b)	
{ 
	mAgentAccess.setAdminOverride(b);
}

void LLAgent::setGodLevel(U8 god_level)	
{ 
	mAgentAccess.setGodLevel(god_level);
}

void LLAgent::setAOTransition()
{
	mAgentAccess.setTransition();
}

const LLAgentAccess& LLAgent::getAgentAccess()
{
	return mAgentAccess;
}


void LLAgent::buildFullname(std::string& name) const
{
	if (mAvatarObject.notNull())
	{
		name = mAvatarObject->getFullname();
	}
}

void LLAgent::buildFullnameAndTitle(std::string& name) const
{
	if (isGroupMember())
	{
		name = mGroupTitle;
		name += ' ';
	}
	else
	{
		name.erase(0, name.length());
	}

	if (mAvatarObject.notNull())
	{
		name += mAvatarObject->getFullname();
	}
}

BOOL LLAgent::isInGroup(const LLUUID& group_id) const
{
	if (isGodlike())
		return true;

	S32 count = mGroups.count();
	for(S32 i = 0; i < count; ++i)
	{
		if(mGroups.get(i).mID == group_id)
		{
			return TRUE;
		}
	}
	return FALSE;
}

// This implementation should mirror LLAgentInfo::hasPowerInGroup
BOOL LLAgent::hasPowerInGroup(const LLUUID& group_id, U64 power) const
{
	if (isGodlike())
		return true;

	// GP_NO_POWERS can also mean no power is enough to grant an ability.
	if (GP_NO_POWERS == power) return FALSE;

	S32 count = mGroups.count();
	for(S32 i = 0; i < count; ++i)
	{
		if(mGroups.get(i).mID == group_id)
		{
			return (BOOL)((mGroups.get(i).mPowers & power) > 0);
		}
	}
	return FALSE;
}

BOOL LLAgent::hasPowerInActiveGroup(U64 power) const
{
	return (mGroupID.notNull() && (hasPowerInGroup(mGroupID, power)));
}

U64 LLAgent::getPowerInGroup(const LLUUID& group_id) const
{
	if (isGodlike())
		return GP_ALL_POWERS;
	
	S32 count = mGroups.count();
	for(S32 i = 0; i < count; ++i)
	{
		if(mGroups.get(i).mID == group_id)
		{
			return (mGroups.get(i).mPowers);
		}
	}

	return GP_NO_POWERS;
}

BOOL LLAgent::getGroupData(const LLUUID& group_id, LLGroupData& data) const
{
	S32 count = mGroups.count();
	for(S32 i = 0; i < count; ++i)
	{
		if(mGroups.get(i).mID == group_id)
		{
			data = mGroups.get(i);
			return TRUE;
		}
	}
	return FALSE;
}

S32 LLAgent::getGroupContribution(const LLUUID& group_id) const
{
	S32 count = mGroups.count();
	for(S32 i = 0; i < count; ++i)
	{
		if(mGroups.get(i).mID == group_id)
		{
			S32 contribution = mGroups.get(i).mContribution;
			return contribution;
		}
	}
	return 0;
}

BOOL LLAgent::setGroupContribution(const LLUUID& group_id, S32 contribution)
{
	S32 count = mGroups.count();
	for(S32 i = 0; i < count; ++i)
	{
		if(mGroups.get(i).mID == group_id)
		{
			mGroups.get(i).mContribution = contribution;
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessage("SetGroupContribution");
			msg->nextBlock("AgentData");
			msg->addUUID("AgentID", gAgentID);
			msg->addUUID("SessionID", gAgentSessionID);
			msg->nextBlock("Data");
			msg->addUUID("GroupID", group_id);
			msg->addS32("Contribution", contribution);
			sendReliableMessage();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL LLAgent::setUserGroupFlags(const LLUUID& group_id, BOOL accept_notices, BOOL list_in_profile)
{
	S32 count = mGroups.count();
	for(S32 i = 0; i < count; ++i)
	{
		if(mGroups.get(i).mID == group_id)
		{
			mGroups.get(i).mAcceptNotices = accept_notices;
			mGroups.get(i).mListInProfile = list_in_profile;
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessage("SetGroupAcceptNotices");
			msg->nextBlock("AgentData");
			msg->addUUID("AgentID", gAgentID);
			msg->addUUID("SessionID", gAgentSessionID);
			msg->nextBlock("Data");
			msg->addUUID("GroupID", group_id);
			msg->addBOOL("AcceptNotices", accept_notices);
			msg->nextBlock("NewData");
			msg->addBOOL("ListInProfile", list_in_profile);
			sendReliableMessage();
			return TRUE;
		}
	}
	return FALSE;
}

// utility to build a location string
void LLAgent::buildLocationString(std::string& str)
{
// [RLVa:KB] - Checked: 2009-07-04 (RLVa-1.0.0a)
	if (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC))
	{
		str = RlvStrings::getString(RLV_STRING_HIDDEN);
		return;
	}
// [/RLVa:KB]

	const LLVector3& agent_pos_region = getPositionAgent();
	S32 pos_x = S32(agent_pos_region.mV[VX]);
	S32 pos_y = S32(agent_pos_region.mV[VY]);
	S32 pos_z = S32(agent_pos_region.mV[VZ]);

	// Round the numbers based on the velocity
	LLVector3 agent_velocity = getVelocity();
	F32 velocity_mag_sq = agent_velocity.magVecSquared();

	const F32 FLY_CUTOFF = 6.f;		// meters/sec
	const F32 FLY_CUTOFF_SQ = FLY_CUTOFF * FLY_CUTOFF;
	const F32 WALK_CUTOFF = 1.5f;	// meters/sec
	const F32 WALK_CUTOFF_SQ = WALK_CUTOFF * WALK_CUTOFF;

	if (velocity_mag_sq > FLY_CUTOFF_SQ)
	{
		pos_x -= pos_x % 4;
		pos_y -= pos_y % 4;
	}
	else if (velocity_mag_sq > WALK_CUTOFF_SQ)
	{
		pos_x -= pos_x % 2;
		pos_y -= pos_y % 2;
	}

	// create a defult name and description for the landmark
	std::string buffer;
	if( LLViewerParcelMgr::getInstance()->getAgentParcelName().empty() )
	{
		// the parcel doesn't have a name
		buffer = llformat("%.32s (%d, %d, %d)",
						  getRegion()->getName().c_str(),
						  pos_x, pos_y, pos_z);
	}
	else
	{
		// the parcel has a name, so include it in the landmark name
		buffer = llformat("%.32s, %.32s (%d, %d, %d)",
						  LLViewerParcelMgr::getInstance()->getAgentParcelName().c_str(),
						  getRegion()->getName().c_str(),
						  pos_x, pos_y, pos_z);
	}
	str = buffer;
}

LLQuaternion LLAgent::getHeadRotation()
{
	if (mAvatarObject.isNull() || !mAvatarObject->mPelvisp || !mAvatarObject->mHeadp)
	{
		return LLQuaternion::DEFAULT;
	}

	if (!gAgent.cameraMouselook())
	{
		return mAvatarObject->getRotation();
	}

	// We must be in mouselook
	LLVector3 look_dir( LLViewerCamera::getInstance()->getAtAxis() );
	LLVector3 up = look_dir % mFrameAgent.getLeftAxis();
	LLVector3 left = up % look_dir;

	LLQuaternion rot(look_dir, left, up);
	if (mAvatarObject->getParent())
	{
		rot = rot * ~mAvatarObject->getParent()->getRotation();
	}

	return rot;
}

void LLAgent::sendAnimationRequests(LLDynamicArray<LLUUID> &anim_ids, EAnimRequest request)
{
	if (gAgentID.isNull())
	{
		return;
	}

	S32 num_valid_anims = 0;

	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_AgentAnimation);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, getID());
	msg->addUUIDFast(_PREHASH_SessionID, getSessionID());

	for (S32 i = 0; i < anim_ids.count(); i++)
	{
		if (anim_ids[i].isNull())
		{
			continue;
		}
		msg->nextBlockFast(_PREHASH_AnimationList);
		msg->addUUIDFast(_PREHASH_AnimID, (anim_ids[i]) );
		msg->addBOOLFast(_PREHASH_StartAnim, (request == ANIM_REQUEST_START) ? TRUE : FALSE);
		num_valid_anims++;
	}

	msg->nextBlockFast(_PREHASH_PhysicalAvatarEventList);
	msg->addBinaryDataFast(_PREHASH_TypeData, NULL, 0);
	if (num_valid_anims)
	{
		sendReliableMessage();
	}
}

void LLAgent::sendAnimationRequest(const LLUUID &anim_id, EAnimRequest request)
{
	if (gAgentID.isNull() || anim_id.isNull() || !mRegionp)
	{
		return;
	}

	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_AgentAnimation);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, getID());
	msg->addUUIDFast(_PREHASH_SessionID, getSessionID());

	msg->nextBlockFast(_PREHASH_AnimationList);
	msg->addUUIDFast(_PREHASH_AnimID, (anim_id) );
	msg->addBOOLFast(_PREHASH_StartAnim, (request == ANIM_REQUEST_START) ? TRUE : FALSE);

	msg->nextBlockFast(_PREHASH_PhysicalAvatarEventList);
	msg->addBinaryDataFast(_PREHASH_TypeData, NULL, 0);
	sendReliableMessage();
}

void LLAgent::sendWalkRun(bool running)
{
	LLMessageSystem* msgsys = gMessageSystem;
	if (msgsys)
	{
		msgsys->newMessageFast(_PREHASH_SetAlwaysRun);
		msgsys->nextBlockFast(_PREHASH_AgentData);
		msgsys->addUUIDFast(_PREHASH_AgentID, getID());
		msgsys->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msgsys->addBOOLFast(_PREHASH_AlwaysRun, BOOL(running) );
		sendReliableMessage();
	}
}

void LLAgent::friendsChanged()
{
	LLCollectProxyBuddies collector;
	LLAvatarTracker::instance().applyFunctor(collector);
	mProxyForAgents = collector.mProxy;
}

BOOL LLAgent::isGrantedProxy(const LLPermissions& perm)
{
	return (mProxyForAgents.count(perm.getOwner()) > 0);
}

BOOL LLAgent::allowOperation(PermissionBit op,
							 const LLPermissions& perm,
							 U64 group_proxy_power,
							 U8 god_minimum)
{
	// Check god level.
	if (getGodLevel() >= god_minimum) return TRUE;

	if (!perm.isOwned()) return FALSE;

	// A group member with group_proxy_power can act as owner.
	BOOL is_group_owned;
	LLUUID owner_id;
	perm.getOwnership(owner_id, is_group_owned);
	LLUUID group_id(perm.getGroup());
	LLUUID agent_proxy(getID());

	if (is_group_owned)
	{
		if (hasPowerInGroup(group_id, group_proxy_power))
		{
			// Let the member assume the group's id for permission requests.
			agent_proxy = owner_id;
		}
	}
	else
	{
		// Check for granted mod permissions.
		if ((PERM_OWNER != op) && isGrantedProxy(perm))
		{
			agent_proxy = owner_id;
		}
	}

	// This is the group id to use for permission requests.
	// Only group members may use this field.
	LLUUID group_proxy = LLUUID::null;
	if (group_id.notNull() && isInGroup(group_id))
	{
		group_proxy = group_id;
	}

	// We now have max ownership information.
	if (PERM_OWNER == op)
	{
		// This this was just a check for ownership, we can now return the answer.
		return (agent_proxy == owner_id);
	}

	return perm.allowOperationBy(op, agent_proxy, group_proxy);
}


void LLAgent::getName(std::string& name)
{
	name.clear();

	if (mAvatarObject.notNull())
	{
		LLNameValue *first_nv = mAvatarObject->getNVPair("FirstName");
		LLNameValue *last_nv = mAvatarObject->getNVPair("LastName");
		if (first_nv && last_nv)
		{
			name = first_nv->printData() + " " + last_nv->printData();
		}
		else
		{
			llwarns << "Agent is missing FirstName and/or LastName nv pair." << llendl;
		}
	}
	else
	{
		name = gSavedSettings.getString("FirstName") + " " + gSavedSettings.getString("LastName");
	}
}

const LLColor4 &LLAgent::getEffectColor()
{
	return mEffectColor;
}

void LLAgent::setEffectColor(const LLColor4 &color)
{
	mEffectColor = color;
}

void LLAgent::initOriginGlobal(const LLVector3d &origin_global)
{
	mAgentOriginGlobal = origin_global;
}

void update_group_floaters(const LLUUID& group_id)
{
	LLFloaterGroupInfo::refreshGroup(group_id);

	// update avatar info
	LLFloaterAvatarInfo* fa = LLFloaterAvatarInfo::getInstance(gAgent.getID());
	if(fa)
	{
		fa->resetGroupList();
	}

	if (gIMMgr)
	{
		// update the talk view
		gIMMgr->refresh();
	}

	gAgent.fireEvent(new LLEvent(&gAgent, "new group"), "");
}

// static
void LLAgent::processAgentDropGroup(LLMessageSystem *msg, void **)
{
	LLUUID	agent_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );

	if (agent_id != gAgentID)
	{
		llwarns << "processAgentDropGroup for agent other than me" << llendl;
		return;
	}

	LLUUID	group_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_GroupID, group_id );

	// Remove the group if it already exists remove it and add the new data to pick up changes.
	LLGroupData gd;
	gd.mID = group_id;
	S32 index = gAgent.mGroups.find(gd);
	if (index != -1)
	{
		gAgent.mGroups.remove(index);
		if (gAgent.getGroupID() == group_id)
		{
			gAgent.mGroupID.setNull();
			gAgent.mGroupPowers = 0;
			gAgent.mGroupName.clear();
			gAgent.mGroupTitle.clear();
		}
		
		// refresh all group information
		gAgent.sendAgentDataUpdateRequest();

		LLGroupMgr::getInstance()->clearGroupData(group_id);
		// close the floater for this group, if any.
		LLFloaterGroupInfo::closeGroup(group_id);
		// refresh the group panel of the search window, if necessary.
		LLFloaterDirectory::refreshGroup(group_id);
	}
	else
	{
		llwarns << "processAgentDropGroup, agent is not part of group " << group_id << llendl;
	}
}

class LLAgentDropGroupViewerNode : public LLHTTPNode
{
	virtual void post(
		LLHTTPNode::ResponsePtr response,
		const LLSD& context,
		const LLSD& input) const
	{

		if (
			!input.isMap() ||
			!input.has("body") )
		{
			//what to do with badly formed message?
			response->statusUnknownError(400);
			response->result(LLSD("Invalid message parameters"));
		}

		LLSD body = input["body"];
		if ( body.has("body") ) 
		{
			//stupid message system doubles up the "body"s
			body = body["body"];
		}

		if (
			body.has("AgentData") &&
			body["AgentData"].isArray() &&
			body["AgentData"][0].isMap() )
		{
			llinfos << "VALID DROP GROUP" << llendl;

			//there is only one set of data in the AgentData block
			LLSD agent_data = body["AgentData"][0];
			LLUUID agent_id;
			LLUUID group_id;

			agent_id = agent_data["AgentID"].asUUID();
			group_id = agent_data["GroupID"].asUUID();

			if (agent_id != gAgentID)
			{
				llwarns
					<< "AgentDropGroup for agent other than me" << llendl;

				response->notFound();
				return;
			}

			// Remove the group if it already exists remove it
			// and add the new data to pick up changes.
			LLGroupData gd;
			gd.mID = group_id;
			S32 index = gAgent.mGroups.find(gd);
			if (index != -1)
			{
				gAgent.mGroups.remove(index);
				if (gAgent.getGroupID() == group_id)
				{
					gAgent.mGroupID.setNull();
					gAgent.mGroupPowers = 0;
					gAgent.mGroupName.clear();
					gAgent.mGroupTitle.clear();
				}
		
				// refresh all group information
				gAgent.sendAgentDataUpdateRequest();

				LLGroupMgr::getInstance()->clearGroupData(group_id);
				// close the floater for this group, if any.
				LLFloaterGroupInfo::closeGroup(group_id);
				// refresh the group panel of the search window,
				//if necessary.
				LLFloaterDirectory::refreshGroup(group_id);
			}
			else
			{
				llwarns
					<< "AgentDropGroup, agent is not part of group "
					<< group_id << llendl;
			}

			response->result(LLSD());
		}
		else
		{
			//what to do with badly formed message?
			response->statusUnknownError(400);
			response->result(LLSD("Invalid message parameters"));
		}
	}
};

LLHTTPRegistration<LLAgentDropGroupViewerNode>
	gHTTPRegistrationAgentDropGroupViewerNode(
		"/message/AgentDropGroup");

// static
void LLAgent::processAgentGroupDataUpdate(LLMessageSystem *msg, void **)
{
	LLUUID	agent_id;

	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );

	if (agent_id != gAgentID)
	{
		llwarns << "processAgentGroupDataUpdate for agent other than me" << llendl;
		return;
	}	
	
	S32 count = msg->getNumberOfBlocksFast(_PREHASH_GroupData);
	LLGroupData group;
	S32 index = -1;
	bool need_floater_update = false;
	for(S32 i = 0; i < count; ++i)
	{
		msg->getUUIDFast(_PREHASH_GroupData, _PREHASH_GroupID, group.mID, i);
		msg->getUUIDFast(_PREHASH_GroupData, _PREHASH_GroupInsigniaID, group.mInsigniaID, i);
		msg->getU64(_PREHASH_GroupData, "GroupPowers", group.mPowers, i);
		msg->getBOOL(_PREHASH_GroupData, "AcceptNotices", group.mAcceptNotices, i);
		msg->getS32(_PREHASH_GroupData, "Contribution", group.mContribution, i);
		msg->getStringFast(_PREHASH_GroupData, _PREHASH_GroupName, group.mName, i);
		
		if(group.mID.notNull())
		{
			need_floater_update = true;
			// Remove the group if it already exists remove it and add the new data to pick up changes.
			index = gAgent.mGroups.find(group);
			if (index != -1)
			{
				gAgent.mGroups.remove(index);
			}
			gAgent.mGroups.put(group);
		}
		if (need_floater_update)
		{
			update_group_floaters(group.mID);
		}
	}

}

class LLAgentGroupDataUpdateViewerNode : public LLHTTPNode
{
	virtual void post(
		LLHTTPNode::ResponsePtr response,
		const LLSD& context,
		const LLSD& input) const
	{
		LLSD body = input["body"];
		if(body.has("body"))
			body = body["body"];
		LLUUID agent_id = body["AgentData"][0]["AgentID"].asUUID();

		if (agent_id != gAgentID)
		{
			llwarns << "processAgentGroupDataUpdate for agent other than me" << llendl;
			return;
		}	

		LLSD group_data = body["GroupData"];

		LLSD::array_iterator iter_group =
			group_data.beginArray();
		LLSD::array_iterator end_group =
			group_data.endArray();
		int group_index = 0;
		for(; iter_group != end_group; ++iter_group)
		{

			LLGroupData group;
			S32 index = -1;
			bool need_floater_update = false;

			group.mID = (*iter_group)["GroupID"].asUUID();
			group.mPowers = ll_U64_from_sd((*iter_group)["GroupPowers"]);
			group.mAcceptNotices = (*iter_group)["AcceptNotices"].asBoolean();
			group.mListInProfile = body["NewGroupData"][group_index]["ListInProfile"].asBoolean();
			group.mInsigniaID = (*iter_group)["GroupInsigniaID"].asUUID();
			group.mName = (*iter_group)["GroupName"].asString();
			group.mContribution = (*iter_group)["Contribution"].asInteger();

			group_index++;

			if(group.mID.notNull())
			{
				need_floater_update = true;
				// Remove the group if it already exists remove it and add the new data to pick up changes.
				index = gAgent.mGroups.find(group);
				if (index != -1)
				{
					gAgent.mGroups.remove(index);
				}
				gAgent.mGroups.put(group);
			}
			if (need_floater_update)
			{
				update_group_floaters(group.mID);
			}
		}
	}
};

LLHTTPRegistration<LLAgentGroupDataUpdateViewerNode >
	gHTTPRegistrationAgentGroupDataUpdateViewerNode ("/message/AgentGroupDataUpdate"); 

// static
void LLAgent::processAgentDataUpdate(LLMessageSystem *msg, void **)
{
	LLUUID	agent_id;

	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );

	if (agent_id != gAgentID)
	{
		llwarns << "processAgentDataUpdate for agent other than me" << llendl;
		return;
	}

	msg->getStringFast(_PREHASH_AgentData, _PREHASH_GroupTitle, gAgent.mGroupTitle);
	LLUUID active_id;
	msg->getUUIDFast(_PREHASH_AgentData, _PREHASH_ActiveGroupID, active_id);


	if(active_id.notNull())
	{
		gAgent.mGroupID = active_id;
		msg->getU64(_PREHASH_AgentData, "GroupPowers", gAgent.mGroupPowers);
		msg->getString(_PREHASH_AgentData, _PREHASH_GroupName, gAgent.mGroupName);
	}
	else
	{
		gAgent.mGroupID.setNull();
		gAgent.mGroupPowers = 0;
		gAgent.mGroupName.clear();
	}		

	update_group_floaters(active_id);
}

// static
void LLAgent::processScriptControlChange(LLMessageSystem *msg, void **)
{
	S32 block_count = msg->getNumberOfBlocks("Data");
	for (S32 block_index = 0; block_index < block_count; block_index++)
	{
		BOOL take_controls;
		U32	controls;
		BOOL passon;
		U32 i;
		msg->getBOOL("Data", "TakeControls", take_controls, block_index);
		if (take_controls)
		{
			// take controls
			msg->getU32("Data", "Controls", controls, block_index );
			msg->getBOOL("Data", "PassToAgent", passon, block_index );
			U32 total_count = 0;
			for (i = 0; i < TOTAL_CONTROLS; i++)
			{
				if (controls & ( 1 << i))
				{
					if (passon)
					{
						gAgent.mControlsTakenPassedOnCount[i]++;
					}
					else
					{
						gAgent.mControlsTakenCount[i]++;
					}
					total_count++;
				}
			}
		
			// Any control taken?  If so, might be first time.
			if (total_count > 0)
			{
				LLFirstUse::useOverrideKeys();
			}
		}
		else
		{
			// release controls
			msg->getU32("Data", "Controls", controls, block_index );
			msg->getBOOL("Data", "PassToAgent", passon, block_index );
			for (i = 0; i < TOTAL_CONTROLS; i++)
			{
				if (controls & ( 1 << i))
				{
					if (passon)
					{
						gAgent.mControlsTakenPassedOnCount[i]--;
						if (gAgent.mControlsTakenPassedOnCount[i] < 0)
						{
							gAgent.mControlsTakenPassedOnCount[i] = 0;
						}
					}
					else
					{
						gAgent.mControlsTakenCount[i]--;
						if (gAgent.mControlsTakenCount[i] < 0)
						{
							gAgent.mControlsTakenCount[i] = 0;
						}
					}
				}
			}
		}
	}
}

/*
// static
void LLAgent::processControlTake(LLMessageSystem *msg, void **)
{
	U32	controls;
	msg->getU32("Data", "Controls", controls );
	U32 passon;
	msg->getBOOL("Data", "PassToAgent", passon );

	S32 i;
	S32 total_count = 0;
	for (i = 0; i < TOTAL_CONTROLS; i++)
	{
		if (controls & ( 1 << i))
		{
			if (passon)
			{
				gAgent.mControlsTakenPassedOnCount[i]++;
			}
			else
			{
				gAgent.mControlsTakenCount[i]++;
			}
			total_count++;
		}
	}

	// Any control taken?  If so, might be first time.
	if (total_count > 0)
	{
		LLFirstUse::useOverrideKeys();
	}
}

// static
void LLAgent::processControlRelease(LLMessageSystem *msg, void **)
{
	U32	controls;
	msg->getU32("Data", "Controls", controls );
	U32 passon;
	msg->getBOOL("Data", "PassToAgent", passon );

	S32 i;
	for (i = 0; i < TOTAL_CONTROLS; i++)
	{
		if (controls & ( 1 << i))
		{
			if (passon)
			{
				gAgent.mControlsTakenPassedOnCount[i]--;
				if (gAgent.mControlsTakenPassedOnCount[i] < 0)
				{
					gAgent.mControlsTakenPassedOnCount[i] = 0;
				}
			}
			else
			{
				gAgent.mControlsTakenCount[i]--;
				if (gAgent.mControlsTakenCount[i] < 0)
				{
					gAgent.mControlsTakenCount[i] = 0;
				}
			}
		}
	}
}
*/

//static
void LLAgent::processAgentCachedTextureResponse(LLMessageSystem *mesgsys, void **user_data)
{
	gAgent.mNumPendingQueries--;

	LLVOAvatar* avatarp = gAgent.getAvatarObject();
	if (!avatarp || avatarp->isDead())
	{
		llwarns << "No avatar for user in cached texture update!" << llendl;
		return;
	}

	if (gAgent.cameraCustomizeAvatar())
	{
		// ignore baked textures when in customize mode
		return;
	}

	S32 query_id;
	mesgsys->getS32Fast(_PREHASH_AgentData, _PREHASH_SerialNum, query_id);

	S32 num_texture_blocks = mesgsys->getNumberOfBlocksFast(_PREHASH_WearableData);


	S32 num_results = 0;
	for (S32 texture_block = 0; texture_block < num_texture_blocks; texture_block++)
	{
		LLUUID texture_id;
		U8 texture_index;

		mesgsys->getUUIDFast(_PREHASH_WearableData, _PREHASH_TextureID, texture_id, texture_block);
		mesgsys->getU8Fast(_PREHASH_WearableData, _PREHASH_TextureIndex, texture_index, texture_block);

		if (texture_id.notNull() 
			&& (S32)texture_index < BAKED_NUM_INDICES 
			&& gAgent.mActiveCacheQueries[ texture_index ] == query_id)
		{
			//llinfos << "Received cached texture " << (U32)texture_index << ": " << texture_id << llendl;
			avatarp->setCachedBakedTexture(getTextureIndex((EBakedTextureIndex)texture_index), texture_id);
			//avatarp->setTETexture( LLVOAvatar::sBakedTextureIndices[texture_index], texture_id );
			gAgent.mActiveCacheQueries[ texture_index ] = 0;
			num_results++;
		}
	}

	llinfos << "Received cached texture response for " << num_results << " textures." << llendl;

	avatarp->updateMeshTextures();

	if (gAgent.mNumPendingQueries == 0)
	{
		// RN: not sure why composites are disabled at this point
		avatarp->setCompositeUpdatesEnabled(TRUE);
		gAgent.sendAgentSetAppearance();
	}
}

BOOL LLAgent::anyControlGrabbed() const
{
	U32 i;
	for (i = 0; i < TOTAL_CONTROLS; i++)
	{
		if (gAgent.mControlsTakenCount[i] > 0)
			return TRUE;
		if (gAgent.mControlsTakenPassedOnCount[i] > 0)
			return TRUE;
	}
	return FALSE;
}

BOOL LLAgent::isControlGrabbed(S32 control_index) const
{
	return mControlsTakenCount[control_index] > 0;
}

void LLAgent::forceReleaseControls()
{
	gMessageSystem->newMessage("ForceScriptControlRelease");
	gMessageSystem->nextBlock("AgentData");
	gMessageSystem->addUUID("AgentID", getID());
	gMessageSystem->addUUID("SessionID", getSessionID());
	sendReliableMessage();
}

void LLAgent::setHomePosRegion( const U64& region_handle, const LLVector3& pos_region)
{
	mHaveHomePosition = TRUE;
	mHomeRegionHandle = region_handle;
	mHomePosRegion = pos_region;
}

BOOL LLAgent::getHomePosGlobal( LLVector3d* pos_global )
{
	if(!mHaveHomePosition)
	{
		return FALSE;
	}
	F32 x = 0;
	F32 y = 0;
	from_region_handle( mHomeRegionHandle, &x, &y);
	pos_global->setVec( x + mHomePosRegion.mV[VX], y + mHomePosRegion.mV[VY], mHomePosRegion.mV[VZ] );
	return TRUE;
}

void LLAgent::clearVisualParams(void *data)
{
	LLVOAvatar* avatarp = gAgent.getAvatarObject();
	if (avatarp)
	{
		avatarp->clearVisualParamWeights();
		avatarp->updateVisualParams();
	}
}

//---------------------------------------------------------------------------
// Teleport
//---------------------------------------------------------------------------

// teleportCore() - stuff to do on any teleport
// protected
bool LLAgent::teleportCore(bool is_local)
{
	if(TELEPORT_NONE != mTeleportState)
	{
		llwarns << "Attempt to teleport when already teleporting." << llendl;
		//return false; //This seems to fix getting stuck in TPs in the first place. --Liny
		teleportCancel();
	}

#if 0
	// This should not exist. It has been added, removed, added, and now removed again.
	// This change needs to come from the simulator. Otherwise, the agent ends up out of
	// sync with other viewers. Discuss in DEV-14145/VWR-6744 before reenabling.

	// Stop all animation before actual teleporting 
	LLVOAvatar* avatarp = gAgent.getAvatarObject();
        if (avatarp)
	{
		for ( LLVOAvatar::AnimIterator anim_it= avatarp->mPlayingAnimations.begin();
		      anim_it != avatarp->mPlayingAnimations.end();
		      ++anim_it)
               {
                       avatarp->stopMotion(anim_it->first);
               }
               avatarp->processAnimationStateChanges();
       }
#endif

	// Don't call LLFirstUse::useTeleport because we don't know
	// yet if the teleport will succeed.  Look in 
	// process_teleport_location_reply

	// close the map and find panels so we can see our destination
	LLFloaterWorldMap::hide(NULL);
	LLFloaterDirectory::hide(NULL);

	// hide land floater too - it'll be out of date
	// NO
	// LLFloaterLand::hideInstance();
	if (!LLFloaterLand::isOpen())
	{
		LLViewerParcelMgr::getInstance()->deselectLand();
	}

	LLViewerMediaFocus::getInstance()->setFocusFace(false, NULL, 0, NULL);

	// Close all pie menus, deselect land, etc.
	// Don't change the camera until we know teleport succeeded. JC
	resetView(FALSE);

	// local logic
	LLViewerStats::getInstance()->incStat(LLViewerStats::ST_TELEPORT_COUNT);
	if (is_local)
	{
		gAgent.setTeleportState( LLAgent::TELEPORT_LOCAL );
	}
	else
	{
		gTeleportDisplay = TRUE;
		gAgent.setTeleportState( LLAgent::TELEPORT_START );

		//release geometry from old location
		gPipeline.resetVertexBuffers();
		if (gSavedSettings.getBOOL("RenderFarClipStepping"))
		{
			F32 draw_distance = gSavedSettings.getF32("RenderFarClip");
			if (gSavedDrawDistance < draw_distance)
			{
				gSavedDrawDistance = draw_distance;
			}
			gSavedSettings.setF32("SavedRenderFarClip", gSavedDrawDistance);
			gSavedSettings.setF32("RenderFarClip", 32.0f);
		}

		// bit of a hack -KC
		KCWindlightInterface::instance().setTPing(true);
	}
	
	if(gSavedSettings.getBOOL("PhoenixPlayTpSound"))
		make_ui_sound("UISndTeleportOut");
	
	// MBW -- Let the voice client know a teleport has begun so it can leave the existing channel.
	// This was breaking the case of teleporting within a single sim.  Backing it out for now.
//	gVoiceClient->leaveChannel();
	
	return true;
}

void LLAgent::teleportRequest(
	const U64& region_handle,
	const LLVector3& pos_local,
	bool look_at_from_camera)
{
	LLViewerRegion* regionp = getRegion();

	bool is_local = (region_handle == to_region_handle(getPositionGlobal()));
	if(regionp && teleportCore(is_local))
	{
		llinfos << "TeleportLocationRequest: '" << region_handle << "':" << pos_local
				<< llendl;
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessage("TeleportLocationRequest");
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msg->nextBlockFast(_PREHASH_Info);
		msg->addU64("RegionHandle", region_handle);
		msg->addVector3("Position", pos_local);
		//Chalice - 2 dTP modes: 0 - standard, 1 - TP AV with cam Z axis rotation.
		LLVector3 look_at;
		if (gSavedSettings.getBOOL("PhoenixDoubleClickTeleportMode") == 0)
		{
			LLVOAvatar* avatarp = gAgent.getAvatarObject();
			look_at=avatarp->getRotation().packToVector3();
		}
		else
		{
			look_at = LLViewerCamera::getInstance()->getAtAxis();
		}
		msg->addVector3("LookAt", look_at);
		sendReliableMessage();
	}
}

// Landmark ID = LLUUID::null means teleport home
void LLAgent::teleportViaLandmark(const LLUUID& landmark_asset_id)
{
// [RLVa:KB] - Checked: 2009-07-07 (RLVa-1.0.0d)
	if ( (rlv_handler_t::isEnabled()) &&
		 ( (gRlvHandler.hasBehaviour(RLV_BHVR_TPLM)) || 
		   ((gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) && (mAvatarObject.notNull()) && (mAvatarObject->mIsSitting)) ))
	{
		RlvNotifications::notifyBlockedTeleport();
		return;
	}
// [/RLVa:KB]

	LLViewerRegion *regionp = getRegion();
	if(regionp && teleportCore())
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_TeleportLandmarkRequest);
		msg->nextBlockFast(_PREHASH_Info);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msg->addUUIDFast(_PREHASH_LandmarkID, landmark_asset_id);
		sendReliableMessage();
	}
}

void LLAgent::teleportViaLure(const LLUUID& lure_id, BOOL godlike)
{
	LLViewerRegion* regionp = getRegion();
	if(regionp && teleportCore())
	{
		U32 teleport_flags = 0x0;
		if (godlike)
		{
			teleport_flags |= TELEPORT_FLAGS_VIA_GODLIKE_LURE;
			teleport_flags |= TELEPORT_FLAGS_DISABLE_CANCEL;
		}
		else
		{
			teleport_flags |= TELEPORT_FLAGS_VIA_LURE;
		}

		// send the message
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_TeleportLureRequest);
		msg->nextBlockFast(_PREHASH_Info);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		msg->addUUIDFast(_PREHASH_LureID, lure_id);
		// teleport_flags is a legacy field, now derived sim-side:
		msg->addU32("TeleportFlags", teleport_flags);
		sendReliableMessage();
	}	
}


// James Cook, July 28, 2005
void LLAgent::teleportCancel()
{
	LLViewerRegion* regionp = getRegion();
	if(regionp)
	{
		// send the message
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessage("TeleportCancel");
		msg->nextBlockFast(_PREHASH_Info);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());
		sendReliableMessage();
	}	
	gTeleportDisplay = FALSE;
	gAgent.setTeleportState( LLAgent::TELEPORT_NONE );
}


void LLAgent::teleportViaLocation(const LLVector3d& pos_global, bool go_to)
{
// [RLVa:KB] - Alternate: Snowglobe-1.2.4 | Checked: 2010-03-02 (RLVa-1.1.1a) | Modified: RLVa-1.2.0a
	if (rlv_handler_t::isEnabled()) 
	{
		// If we're getting teleported due to @tpto we should disregard any @tploc=n or @unsit=n restrictions from the same object
		if ( (gRlvHandler.hasBehaviourExcept(RLV_BHVR_TPLOC, gRlvHandler.getCurrentObject())) ||
		     ( (mAvatarObject.notNull()) && (mAvatarObject->mIsSitting) && 
			   (gRlvHandler.hasBehaviourExcept(RLV_BHVR_UNSIT, gRlvHandler.getCurrentObject()))) )
		{
			RlvNotifications::notifyBlockedTeleport();
			return;
		}

		if ( (gRlvHandler.getCurrentCommand()) && (RLV_BHVR_TPTO == gRlvHandler.getCurrentCommand()->getBehaviourType()) )
		{
			gRlvHandler.setCanCancelTp(false);
		}
	}
// [/RLVa:KB]

	LLViewerRegion* regionp = getRegion();
//	LLSimInfo* info = LLWorldMap::getInstance()->simInfoFromPosGlobal(pos_global);
	bool isLocal = regionp->getHandle() == to_region_handle_global((F32)pos_global.mdV[VX], (F32)pos_global.mdV[VY]);
	bool ml = gSavedSettings.getBOOL("PhoenixUseBridgeMoveToTarget");
	bool tpchat = gSavedSettings.getBOOL("PhoenixDoubleClickTeleportChat");

	LLVector3 offset = LLVector3(0.f,0.f,0.f);

	U64 handle = to_region_handle(pos_global);
	LLSimInfo* info = LLWorldMap::getInstance()->simInfoFromHandle(handle);
	if(regionp && info)
	{
		LLVector3d region_origin = info->getGlobalOrigin();
		LLVector3 pos_local(
			(F32)(pos_global.mdV[VX] - region_origin.mdV[VX]),
			(F32)(pos_global.mdV[VY] - region_origin.mdV[VY]),
			(F32)(pos_global.mdV[VZ]));
		teleportRequest(handle, pos_local);
	}
	else if(regionp && 
		teleportCore(regionp->getHandle() == to_region_handle_global((F32)pos_global.mdV[VX], (F32)pos_global.mdV[VY])))
	{
		llwarns << "Using deprecated teleportlocationrequest." << llendl; 
		// send the message
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_TeleportLocationRequest);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, getID());
		msg->addUUIDFast(_PREHASH_SessionID, getSessionID());

		msg->nextBlockFast(_PREHASH_Info);
		F32 width = regionp->getWidth();
		LLVector3 pos(fmod((F32)pos_global.mdV[VX], width),
					  fmod((F32)pos_global.mdV[VY], width),
					  (F32)pos_global.mdV[VZ]);
		F32 region_x = (F32)(pos_global.mdV[VX]);
		F32 region_y = (F32)(pos_global.mdV[VY]);
		U64 region_handle = to_region_handle_global(region_x, region_y);
		msg->addU64Fast(_PREHASH_RegionHandle, region_handle);
		msg->addVector3Fast(_PREHASH_Position, pos);
		pos.mV[VX] += 1;
		LLVector3 look_at;
		//Chalice - 2 dTP modes: 0 - standard, 1 - TP AV with cam Z axis rotation.
		if (gSavedSettings.getBOOL("PhoenixDoubleClickTeleportMode") == 0)
		{
			LLVOAvatar* avatarp = gAgent.getAvatarObject();
			look_at=avatarp->getRotation().packToVector3();
		}
		else
		{
			look_at = LLViewerCamera::getInstance()->getAtAxis();
		}
		msg->addVector3Fast(_PREHASH_LookAt, look_at);
		sendReliableMessage();
	}
	if(isLocal)
	{
		F32 width = regionp->getWidth();
		LLVector3 pos_local(fmod((F32)pos_global.mdV[VX], width),
						fmod((F32)pos_global.mdV[VY], width),
						(F32)pos_global.mdV[VZ]);

		std::stringstream strstr;
		strstr << std::setiosflags(std::ios::fixed) << std::setprecision(6); // delicious iomanip
		strstr << "<" << pos_local.mV[VX] << ", " << pos_local.mV[VY] << ", "  << pos_local.mV[VZ] << ">";

		if(tpchat)
		{
			gMessageSystem->newMessage("ScriptDialogReply");
			gMessageSystem->nextBlock("AgentData");
			gMessageSystem->addUUID("AgentID", gAgent.getID());
			gMessageSystem->addUUID("SessionID", gAgent.getSessionID());
			gMessageSystem->nextBlock("Data");
			gMessageSystem->addUUID("ObjectID", gAgent.getID());
			gMessageSystem->addS32("ChatChannel", gSavedSettings.getS32("PhoenixDoubleClickTeleportChannel"));
			gMessageSystem->addS32("ButtonIndex", 1);
			std::stringstream strstr;
			strstr << std::setiosflags(std::ios::fixed) << std::setprecision(6); // delicious iomanip
			strstr << "<" << pos_local.mV[VX] << ", " << pos_local.mV[VY] << ", "  << pos_local.mV[VZ] << ">";		 
			gMessageSystem->addString("ButtonLabel",strstr.str());
			gAgent.sendReliableMessage();
		}
		if(ml)
		{
			gAgent.setControlFlags(AGENT_CONTROL_STAND_UP); //GIT UP
			JCLSLBridge::instance().bridgetolsl("move|"+strstr.str(),NULL); //o i c wut u did thar.
			//I presume this will be the new format instead of a naked vector on a specified channel... -tG
		}
	}
}

// Teleport to global position, but keep facing in the same direction 
void LLAgent::teleportViaLocationLookAt(const LLVector3d& pos_global)
{
// [RLVa:KB] - Checked: 2010-10-07 (RLVa-1.2.1f) | Added: RLVa-1.2.1f
	// RELEASE-RLVa: [SL-2.2.0] Make sure this isn't used for anything except double-click teleporting
	if ( (rlv_handler_t::isEnabled()) && (!RlvUtil::isForceTp()) && 
		 ((gRlvHandler.hasBehaviour(RLV_BHVR_SITTP)) || (!gRlvHandler.canStand())) )
	{
		RlvNotifications::notifyBlockedTeleport();
		return;
	}
// [/RLVa:KB]

	mbTeleportKeepsLookAt = true;
	setFocusOnAvatar(FALSE, ANIMATE);	// detach camera form avatar, so it keeps direction
	U64 region_handle = to_region_handle(pos_global);
	LLVector3 pos_local = (LLVector3)(pos_global - from_region_handle(region_handle));
	teleportRequest(region_handle, pos_local, getTeleportKeepsLookAt());
}

void LLAgent::setTeleportState(ETeleportState state)
{
	mTeleportState = state;

	static BOOL* sFreezeTime = rebind_llcontrol<BOOL>("FreezeTime", &gSavedSettings, true);

	if (mTeleportState > TELEPORT_NONE && (*sFreezeTime))
	{
		LLFloaterSnapshot::hide(0);
	}
	if (mTeleportState == TELEPORT_NONE)
	{
		mbTeleportKeepsLookAt = false;
	}
	// OGPX : Only compute a 'slurl' in non-OGP mode. In OGP, set it to regionuri in floaterteleport.
	if ((mTeleportState == TELEPORT_MOVING)&& (!gSavedSettings.getBOOL("OpenGridProtocol")))
	{
		// We're outa here. Save "back" slurl.
		mTeleportSourceSLURL = getSLURL();
	}
// [RLVa:KB] - Alternate: Snowglobe-1.2.4 | Version: 1.23.4 | Checked: 2009-07-07 (RLVa-1.0.0d) | Added: RLVa-0.2.0b
	if ( (rlv_handler_t::isEnabled()) && (TELEPORT_NONE == mTeleportState) )
	{
		gRlvHandler.setCanCancelTp(true);
	}
// [/RLVa:KB]
}

void LLAgent::stopCurrentAnimations()
{
	// This function stops all current overriding animations on this
	// avatar, propagating this change back to the server.

	LLVOAvatar* avatarp = gAgent.getAvatarObject();
	if (avatarp)
	{
		for ( LLVOAvatar::AnimIterator anim_it =
			      avatarp->mPlayingAnimations.begin();
		      anim_it != avatarp->mPlayingAnimations.end();
		      anim_it++)
		{
			if (anim_it->first ==
			    ANIM_AGENT_SIT_GROUND_CONSTRAINED)
			{
				// don't cancel a ground-sit anim, as viewers
				// use this animation's status in
				// determining whether we're sitting. ick.
			}
			else
			{
				// stop this animation locally
				avatarp->stopMotion(anim_it->first, TRUE);
				// ...and tell the server to tell everyone.
				sendAnimationRequest(anim_it->first, ANIM_REQUEST_STOP);
			}
		}

		// re-assert at least the default standing animation, because
		// viewers get confused by avs with no associated anims.
		sendAnimationRequest(ANIM_AGENT_STAND,
				     ANIM_REQUEST_START);
	}
}

void LLAgent::fidget()
{
	if (!getAFK())
	{
		F32 curTime = mFidgetTimer.getElapsedTimeF32();
		if (curTime > mNextFidgetTime)
		{
			// pick a random fidget anim here
			S32 oldFidget = mCurrentFidget;

			mCurrentFidget = ll_rand(NUM_AGENT_STAND_ANIMS);

			if (mCurrentFidget != oldFidget)
			{
				LLAgent::stopFidget();

				
				switch(mCurrentFidget)
				{
				case 0:
					mCurrentFidget = 0;
					break;
				case 1:
					sendAnimationRequest(ANIM_AGENT_STAND_1, ANIM_REQUEST_START);
					mCurrentFidget = 1;
					break;
				case 2:
					sendAnimationRequest(ANIM_AGENT_STAND_2, ANIM_REQUEST_START);
					mCurrentFidget = 2;
					break;
				case 3:
					sendAnimationRequest(ANIM_AGENT_STAND_3, ANIM_REQUEST_START);
					mCurrentFidget = 3;
					break;
				case 4:
					sendAnimationRequest(ANIM_AGENT_STAND_4, ANIM_REQUEST_START);
					mCurrentFidget = 4;
					break;
				}
			}

			// calculate next fidget time
			mNextFidgetTime = curTime + ll_frand(MAX_FIDGET_TIME - MIN_FIDGET_TIME) + MIN_FIDGET_TIME;
		}
	}
}

void LLAgent::stopFidget()
{
	LLDynamicArray<LLUUID> anims;
	anims.put(ANIM_AGENT_STAND_1);
	anims.put(ANIM_AGENT_STAND_2);
	anims.put(ANIM_AGENT_STAND_3);
	anims.put(ANIM_AGENT_STAND_4);

	gAgent.sendAnimationRequests(anims, ANIM_REQUEST_STOP);
}


void LLAgent::requestEnterGodMode()
{
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_RequestGodlikePowers);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_RequestBlock);
	msg->addBOOLFast(_PREHASH_Godlike, TRUE);
	msg->addUUIDFast(_PREHASH_Token, LLUUID::null);

	// simulators need to know about your request
	sendReliableMessage();
}

void LLAgent::requestLeaveGodMode()
{
	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_RequestGodlikePowers);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_RequestBlock);
	msg->addBOOLFast(_PREHASH_Godlike, FALSE);
	msg->addUUIDFast(_PREHASH_Token, LLUUID::null);

	// simulator needs to know about your request
	sendReliableMessage();
}

// wearables
LLAgent::createStandardWearablesAllDoneCallback::~createStandardWearablesAllDoneCallback()
{
	gAgent.createStandardWearablesAllDone();
}

LLAgent::sendAgentWearablesUpdateCallback::~sendAgentWearablesUpdateCallback()
{
	gAgent.sendAgentWearablesUpdate();
}

LLAgent::addWearableToAgentInventoryCallback::addWearableToAgentInventoryCallback(
	LLPointer<LLRefCount> cb, S32 index, LLWearable* wearable, U32 todo) :
	mIndex(index),
	mWearable(wearable),
	mTodo(todo),
	mCB(cb)
{
}

void LLAgent::addWearableToAgentInventoryCallback::fire(const LLUUID& inv_item)
{
	if (inv_item.isNull())
		return;

	gAgent.addWearabletoAgentInventoryDone(mIndex, inv_item, mWearable);

	if (mTodo & CALL_UPDATE)
	{
		gAgent.sendAgentWearablesUpdate();
	}
	if (mTodo & CALL_RECOVERDONE)
	{
		gAgent.recoverMissingWearableDone();
	}
	/*
	 * Do this for every one in the loop
	 */
	if (mTodo & CALL_CREATESTANDARDDONE)
	{
		gAgent.createStandardWearablesDone(mIndex);
	}
	if (mTodo & CALL_MAKENEWOUTFITDONE)
	{
		gAgent.makeNewOutfitDone(mIndex);
	}
}

void LLAgent::addWearabletoAgentInventoryDone(
	S32 index,
	const LLUUID& item_id,
	LLWearable* wearable)
{
	if (item_id.isNull())
		return;

	LLUUID old_item_id = mWearableEntry[index].mItemID;
	mWearableEntry[index].mItemID = item_id;
	mWearableEntry[index].mWearable = wearable;
	if (old_item_id.notNull())
		gInventory.addChangedMask(LLInventoryObserver::LABEL, old_item_id);
	gInventory.addChangedMask(LLInventoryObserver::LABEL, item_id);
	LLViewerInventoryItem* item = gInventory.getItem(item_id);
	if(item && wearable)
	{
		// We're changing the asset id, so we both need to set it
		// locally via setAssetUUID() and via setTransactionID() which
		// will be decoded on the server. JC
		item->setAssetUUID(wearable->getID());
		item->setTransactionID(wearable->getTransactionID());
		gInventory.addChangedMask(LLInventoryObserver::INTERNAL, item_id);
		item->updateServer(FALSE);
	}
	gInventory.notifyObservers();
}

void LLAgent::sendAgentWearablesUpdate()
{
	// First make sure that we have inventory items for each wearable
	S32 i;
	for(i=0; i < WT_COUNT; ++i)
	{
		LLWearable* wearable = mWearableEntry[ i ].mWearable;
		if (wearable)
		{
			if( mWearableEntry[ i ].mItemID.isNull() )
			{
				LLPointer<LLInventoryCallback> cb =
					new addWearableToAgentInventoryCallback(
						LLPointer<LLRefCount>(NULL),
						i,
						wearable,
						addWearableToAgentInventoryCallback::CALL_NONE);
				addWearableToAgentInventory(cb, wearable);
			}
			else
			{
				gInventory.addChangedMask( LLInventoryObserver::LABEL,
						mWearableEntry[i].mItemID );
			}
		}
	}

	// Then make sure the inventory is in sync with the avatar.
	gInventory.notifyObservers();

	// This isn't the proper place to be doing this, but it's a good "catch-all"
	LLCOFMgr::instance().synchWearables();

	// Send the AgentIsNowWearing 
	gMessageSystem->newMessageFast(_PREHASH_AgentIsNowWearing);

	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, getSessionID());

	LL_DEBUGS("Wearables") << "sendAgentWearablesUpdate()" << LL_ENDL;
	for(i=0; i < WT_COUNT; ++i)
	{
		gMessageSystem->nextBlockFast(_PREHASH_WearableData);

		U8 type_u8 = (U8)i;
		gMessageSystem->addU8Fast(_PREHASH_WearableType, type_u8 );

		LLWearable* wearable = mWearableEntry[ i ].mWearable;
		if( wearable )
		{
			LL_DEBUGS("Wearables") << "Sending wearable " << wearable->getName() << " mItemID = " << mWearableEntry[ i ].mItemID << LL_ENDL; 
			LLUUID item_id = mWearableEntry[i].mItemID;
			const LLViewerInventoryItem *item = gInventory.getItem(item_id);
			if (item && item->getIsLinkType())
			{
				// Get the itemID that this item points to.  i.e. make sure
				// we are storing baseitems, not their links, in the database.
				item_id = item->getLinkedUUID();
			}
			gMessageSystem->addUUIDFast(_PREHASH_ItemID, item_id);
		}
		else
		{
			LL_DEBUGS("Wearables") << "Not wearing wearable type " << LLWearable::typeToTypeName((EWearableType)i) << LL_ENDL;
			gMessageSystem->addUUIDFast(_PREHASH_ItemID, LLUUID::null );
		}

		LL_DEBUGS("Wearables") << "       " << LLWearable::typeToTypeLabel((EWearableType)i) << " : " << (wearable ? wearable->getID() : LLUUID::null) << LL_ENDL;
	}
	gAgent.sendReliableMessage();
}

void LLAgent::saveWearable( EWearableType type, BOOL send_update )
{
	LLWearable* old_wearable = mWearableEntry[(S32)type].mWearable;
	if( old_wearable && (old_wearable->isDirty() || old_wearable->isOldVersion()) )
	{
		LLWearable* new_wearable = gWearableList.createCopyFromAvatar( old_wearable );
		mWearableEntry[(S32)type].mWearable = new_wearable;

		LLInventoryItem* item = gInventory.getItem(mWearableEntry[(S32)type].mItemID);
		if( item )
		{
			// Update existing inventory item
			LLPointer<LLViewerInventoryItem> template_item =
				new LLViewerInventoryItem(item->getUUID(),
										  item->getParentUUID(),
										  item->getPermissions(),
										  new_wearable->getID(),
										  new_wearable->getAssetType(),
										  item->getInventoryType(),
										  item->getName(),
										  item->getDescription(),
										  item->getSaleInfo(),
										  item->getFlags(),
										  item->getCreationDate());
			template_item->setTransactionID(new_wearable->getTransactionID());
			template_item->updateServer(FALSE);
			gInventory.updateItem(template_item);
		}
		else
		{
			// Add a new inventory item (shouldn't ever happen here)
			U32 todo = addWearableToAgentInventoryCallback::CALL_NONE;
			if (send_update)
			{
				todo |= addWearableToAgentInventoryCallback::CALL_UPDATE;
			}
			LLPointer<LLInventoryCallback> cb =
				new addWearableToAgentInventoryCallback(
					LLPointer<LLRefCount>(NULL),
					(S32)type,
					new_wearable,
					todo);
			addWearableToAgentInventory(cb, new_wearable);
			return;
		}
		
		getAvatarObject()->wearableUpdated( type );

		if( send_update )
		{
			sendAgentWearablesUpdate();
		}
	}
}

void LLAgent::saveWearableAs(
	EWearableType type,
	const std::string& new_name,
	BOOL save_in_lost_and_found)
{
	if(!isWearableCopyable(type))
	{
		llwarns << "LLAgent::saveWearableAs() not copyable." << llendl;
		return;
	}
	LLWearable* old_wearable = getWearable(type);
	if(!old_wearable)
	{
		llwarns << "LLAgent::saveWearableAs() no old wearable." << llendl;
		return;
	}
	LLInventoryItem* item = gInventory.getItem(mWearableEntry[type].mItemID);
	if(!item)
	{
		llwarns << "LLAgent::saveWearableAs() no inventory item." << llendl;
		return;
	}
	std::string trunc_name(new_name);
	LLStringUtil::truncate(trunc_name, DB_INV_ITEM_NAME_STR_LEN);
	LLWearable* new_wearable = gWearableList.createCopyFromAvatar(
		old_wearable,
		trunc_name);
	LLPointer<LLInventoryCallback> cb =
		new addWearableToAgentInventoryCallback(
			LLPointer<LLRefCount>(NULL),
			type,
			new_wearable,
			addWearableToAgentInventoryCallback::CALL_UPDATE);
	LLUUID category_id;
	if (save_in_lost_and_found)
	{
		category_id = gInventory.findCategoryUUIDForType(
			LLAssetType::AT_LOST_AND_FOUND);
	}
	else
	{
		// put in same folder as original
		category_id = item->getParentUUID();
	}

	copy_inventory_item(
		gAgent.getID(),
		item->getPermissions().getOwner(),
		item->getUUID(),
		category_id,
		new_name,
		cb);

/*
	LLWearable* old_wearable = getWearable( type );
	if( old_wearable )
	{
		std::string old_name = old_wearable->getName();
		old_wearable->setName( new_name );
		LLWearable* new_wearable = gWearableList.createCopyFromAvatar( old_wearable );
		old_wearable->setName( old_name );
			
		LLUUID category_id;
		LLInventoryItem* item = gInventory.getItem( mWearableEntry[ type ].mItemID );
		if( item )
		{
			new_wearable->setPermissions(item->getPermissions());
			if (save_in_lost_and_found)
			{
				category_id = gInventory.findCategoryUUIDForType(LLAssetType::AT_LOST_AND_FOUND);
			}
			else
			{
				// put in same folder as original
				category_id = item->getParentUUID();
			}
			LLInventoryView* view = LLInventoryView::getActiveInventory();
			if(view)
			{
				view->getPanel()->setSelection(item->getUUID(), TAKE_FOCUS_NO);
			}
		}

		mWearableEntry[ type ].mWearable = new_wearable;
		LLPointer<LLInventoryCallback> cb =
			new addWearableToAgentInventoryCallback(
				LLPointer<LLRefCount>(NULL),
				type,
				addWearableToAgentInventoryCallback::CALL_UPDATE);
		addWearableToAgentInventory(cb, new_wearable, category_id);
	}
*/
}

void LLAgent::revertWearable( EWearableType type )
{
	LLWearable* wearable = mWearableEntry[(S32)type].mWearable;
	if( wearable )
	{
		wearable->writeToAvatar( TRUE );
	}
	sendAgentSetAppearance();
}

void LLAgent::revertAllWearables()
{
	for( S32 i=0; i < WT_COUNT; i++ )
	{
		revertWearable( (EWearableType)i );
	}
}

void LLAgent::saveAllWearables()
{
	//if(!gInventory.isLoaded())
	//{
	//	return;
	//}

	for( S32 i=0; i < WT_COUNT; i++ )
	{
		saveWearable( (EWearableType)i, FALSE );
	}
	sendAgentWearablesUpdate();
}

// Called when the user changes the name of a wearable inventory item that is currenlty being worn.
void LLAgent::setWearableName( const LLUUID& item_id, const std::string& new_name )
{
	for( S32 i=0; i < WT_COUNT; i++ )
	{
		if( mWearableEntry[i].mItemID == item_id )
		{
			LLWearable* old_wearable = mWearableEntry[i].mWearable;
			llassert( old_wearable );

			std::string old_name = old_wearable->getName();
			old_wearable->setName( new_name );
			LLWearable* new_wearable = gWearableList.createCopy( old_wearable );
			LLInventoryItem* item = gInventory.getItem(item_id);
			if(item)
			{
				new_wearable->setPermissions(item->getPermissions());
			}
			old_wearable->setName( old_name );

			mWearableEntry[i].mWearable = new_wearable;
			sendAgentWearablesUpdate();
			break;
		}
	}
}


BOOL LLAgent::isWearableModifiable(EWearableType type)
{
	LLUUID item_id = getWearableItem(type);
	if(!item_id.isNull())
	{
		LLInventoryItem* item = gInventory.getItem(item_id);
		if(item && item->getPermissions().allowModifyBy(gAgent.getID(),
														gAgent.getGroupID()))
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL LLAgent::isWearableCopyable(EWearableType type)
{
	LLUUID item_id = getWearableItem(type);
	if(!item_id.isNull())
	{
		LLInventoryItem* item = gInventory.getItem(item_id);
		if(item && item->getPermissions().allowCopyBy(gAgent.getID(),
													  gAgent.getGroupID()))
		{
			return TRUE;
		}
	}
	return FALSE;
}

U32 LLAgent::getWearablePermMask(EWearableType type)
{
	LLUUID item_id = getWearableItem(type);
	if(!item_id.isNull())
	{
		LLInventoryItem* item = gInventory.getItem(item_id);
		if(item)
		{
			return item->getPermissions().getMaskOwner();
		}
	}
	return PERM_NONE;
}

LLInventoryItem* LLAgent::getWearableInventoryItem(EWearableType type)
{
	LLUUID item_id = getWearableItem(type);
	LLInventoryItem* item = NULL;
	if(item_id.notNull())
	{
		 item = gInventory.getItem(item_id);
	}
	return item;
}

LLWearable* LLAgent::getWearableFromWearableItem( const LLUUID& item_id )
{
	for( S32 i=0; i < WT_COUNT; i++ )
	{
		if( mWearableEntry[i].mItemID == item_id )
		{
			return mWearableEntry[i].mWearable;
		}
	}
	return NULL;
}


void LLAgent::sendAgentWearablesRequest()
{
	gMessageSystem->newMessageFast(_PREHASH_AgentWearablesRequest);
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID() );
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID() );
	sendReliableMessage();
}

// Used to enable/disable menu items.
// static
BOOL LLAgent::selfHasWearable( void* userdata )
{
	EWearableType type = (EWearableType)(intptr_t)userdata;
	return gAgent.getWearable( type ) != NULL;
}

BOOL LLAgent::isWearingItem( const LLUUID& item_id )
{
	const LLUUID& base_item_id = gInventory.getLinkedItemID(item_id);
	return (getWearableFromWearableItem(base_item_id) != NULL);
}

// static
void LLAgent::processAgentInitialWearablesUpdate( LLMessageSystem* mesgsys, void** user_data )
{
	// We should only receive this message a single time.  Ignore subsequent AgentWearablesUpdates
	// that may result from AgentWearablesRequest having been sent more than once. 
	static bool first = true;
	if (!first) return;
	first = false;

	LLUUID agent_id;
	gMessageSystem->getUUIDFast(_PREHASH_AgentData, _PREHASH_AgentID, agent_id );

	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if( avatar && (agent_id == avatar->getID()) )
	{
		gMessageSystem->getU32Fast(_PREHASH_AgentData, _PREHASH_SerialNum, gAgent.mAgentWearablesUpdateSerialNum );
		
		S32 num_wearables = gMessageSystem->getNumberOfBlocksFast(_PREHASH_WearableData);
		if( num_wearables < 4 )
		{
			// Transitional state.  Avatars should always have at least their body parts (hair, eyes, shape and skin).
			// The fact that they don't have any here (only a dummy is sent) implies that this account existed
			// before we had wearables, or that the database has gotten messed up.
			return;
		}
		//else
		//{
		//	 // OGPX HACK: OGP authentication does not pass back login-flags, 
		//   // thus doesn't check for "gendered" flag
		//	 // so this isn't an ideal place for this because the check in idle_startup in STATE_WEARABLES_WAIT
		//	 // is happening *before* this call. That causes the welcomechoosesex dialog to be displayed
		//	 // but I'm torn on removing this commented out code because I'm unsure how the initial wearables 
		//   // code will work out. 
		//	 gAgent.setGenderChosen(TRUE);
		//}

		//lldebugs << "processAgentInitialWearablesUpdate()" << llendl;
		// Add wearables
		LLUUID asset_id_array[ WT_COUNT ];
		S32 i;
		for( i=0; i < num_wearables; i++ )
		{
			U8 type_u8 = 0;
			gMessageSystem->getU8Fast(_PREHASH_WearableData, _PREHASH_WearableType, type_u8, i );
			if( type_u8 >= WT_COUNT )
			{
				continue;
			}
			EWearableType type = (EWearableType) type_u8;

			LLUUID item_id;
			gMessageSystem->getUUIDFast(_PREHASH_WearableData, _PREHASH_ItemID, item_id, i );

			LLUUID asset_id;
			gMessageSystem->getUUIDFast(_PREHASH_WearableData, _PREHASH_AssetID, asset_id, i );
			if( asset_id.isNull() )
			{
				LLWearable::removeFromAvatar( type, FALSE );
			}
			else
			{
				LLAssetType::EType asset_type = LLWearable::typeToAssetType( type );
				if( asset_type == LLAssetType::AT_NONE )
				{
					continue;
				}

				gAgent.mWearableEntry[type].mItemID = item_id;
				asset_id_array[type] = asset_id;
			}

			LL_DEBUGS("Wearables") << "       " << LLWearable::typeToTypeLabel(type) << " " << asset_id << " item id " << gAgent.mWearableEntry[type].mItemID.asString() << LL_ENDL;
		}

		LLCOFMgr::instance().fetchCOF();

		// now that we have the asset ids...request the wearable assets
		for( i = 0; i < WT_COUNT; i++ )
		{
			LL_DEBUGS("Wearables") << "      fetching " << asset_id_array[i] << LL_ENDL;
			if( !gAgent.mWearableEntry[i].mItemID.isNull() )
			{
				gWearableList.getAsset( 
					asset_id_array[i],
					LLStringUtil::null,
					LLWearable::typeToAssetType( (EWearableType) i ), 
					LLAgent::onInitialWearableAssetArrived, (void*)(intptr_t)i );
			}
		}

		// Not really sure where else to put this
		gIdleCallbacks.addFunction(&LLAttachmentsMgr::onIdle, NULL);
	}
}

// A single wearable that the avatar was wearing on start-up has arrived from the database.
// static
void LLAgent::onInitialWearableAssetArrived( LLWearable* wearable, void* userdata )
{
	EWearableType type = (EWearableType)(intptr_t)userdata;

	LLVOAvatar* avatar = gAgent.getAvatarObject();
	if( !avatar )
	{
		return;
	}

	if( wearable )
	{
		llassert( type == wearable->getType() );
		gAgent.mWearableEntry[ type ].mWearable = wearable;

		// disable composites if initial textures are baked
		avatar->setupComposites();
		gAgent.queryWearableCache();

		wearable->writeToAvatar( FALSE );
		avatar->setCompositeUpdatesEnabled(TRUE);
		gInventory.addChangedMask( LLInventoryObserver::LABEL, gAgent.mWearableEntry[type].mItemID );
	}
	else
	{
		// Somehow the asset doesn't exist in the database.
		gAgent.recoverMissingWearable( type );
	}

	gInventory.notifyObservers();

	// Have all the wearables that the avatar was wearing at log-in arrived?
	if( !gAgent.mWearablesLoaded )
	{
		gAgent.mWearablesLoaded = TRUE;
		for( S32 i = 0; i < WT_COUNT; i++ )
		{
			if( !gAgent.mWearableEntry[i].mItemID.isNull() && !gAgent.mWearableEntry[i].mWearable )
			{
				gAgent.mWearablesLoaded = FALSE;
				break;
			}
		}
	}

	if( gAgent.mWearablesLoaded )
	{
		// Make sure that the server's idea of the avatar's wearables actually match the wearables.
		gAgent.sendAgentSetAppearance();

		// Check to see if there are any baked textures that we hadn't uploaded before we logged off last time.
		// If there are any, schedule them to be uploaded as soon as the layer textures they depend on arrive.
		if( !gAgent.cameraCustomizeAvatar() )
		{
			avatar->requestLayerSetUploads();
		}
	}
}

// Normally, all wearables referred to "AgentWearablesUpdate" will correspond to actual assets in the
// database.  If for some reason, we can't load one of those assets, we can try to reconstruct it so that
// the user isn't left without a shape, for example.  (We can do that only after the inventory has loaded.)
void LLAgent::recoverMissingWearable( EWearableType type )
{
	// Try to recover by replacing missing wearable with a new one.
	LLNotifications::instance().add("ReplacedMissingWearable");
	lldebugs << "Wearable " << LLWearable::typeToTypeLabel( type ) << " could not be downloaded.  Replaced inventory item with default wearable." << llendl;
	LLWearable* new_wearable = gWearableList.createNewWearable(type);

	S32 type_s32 = (S32) type;
	mWearableEntry[type_s32].mWearable = new_wearable;
	new_wearable->writeToAvatar( TRUE );

	// Add a new one in the lost and found folder.
	// (We used to overwrite the "not found" one, but that could potentially
	// destory content.) JC
	LLUUID lost_and_found_id = 
		gInventory.findCategoryUUIDForType(LLAssetType::AT_LOST_AND_FOUND);
	LLPointer<LLInventoryCallback> cb =
		new addWearableToAgentInventoryCallback(
			LLPointer<LLRefCount>(NULL),
			type_s32,
			new_wearable,
			addWearableToAgentInventoryCallback::CALL_RECOVERDONE);
	addWearableToAgentInventory( cb, new_wearable, lost_and_found_id, TRUE);
}

void LLAgent::recoverMissingWearableDone()
{
	// Have all the wearables that the avatar was wearing at log-in arrived or been fabricated?
	mWearablesLoaded = TRUE;
	for( S32 i = 0; i < WT_COUNT; i++ )
	{
		if( !mWearableEntry[i].mItemID.isNull() && !mWearableEntry[i].mWearable )
		{
			mWearablesLoaded = FALSE;
			break;
		}
	}

	if( mWearablesLoaded )
	{
		// Make sure that the server's idea of the avatar's wearables actually match the wearables.
		sendAgentSetAppearance();
	}
	else
	{
		gInventory.addChangedMask( LLInventoryObserver::LABEL, LLUUID::null );
		gInventory.notifyObservers();
	}
}

void LLAgent::createStandardWearables(BOOL female)
{
	llwarns << "Creating Standard " << (female ? "female" : "male" )
			<< " Wearables" << llendl;

	if (mAvatarObject.isNull())
	{
		return;
	}

	if(female) mAvatarObject->setSex(SEX_FEMALE);
	else mAvatarObject->setSex(SEX_MALE);

	BOOL create[WT_COUNT] = 
	{
		TRUE,  //WT_SHAPE
		TRUE,  //WT_SKIN
		TRUE,  //WT_HAIR
		TRUE,  //WT_EYES
		TRUE,  //WT_SHIRT
		TRUE,  //WT_PANTS
		TRUE,  //WT_SHOES
		TRUE,  //WT_SOCKS
		FALSE, //WT_JACKET
		FALSE, //WT_GLOVES
		TRUE,  //WT_UNDERSHIRT
		TRUE,  //WT_UNDERPANTS
		FALSE,  //WT_SKIRT
		FALSE, //WT_ALPHA
		FALSE,  //WT_TATTOO
		FALSE   //WT_PHYSICS
	};

	for( S32 i=0; i < WT_COUNT; i++ )
	{
		bool once = false;
		LLPointer<LLRefCount> donecb = NULL;
		if( create[i] )
		{
			if (!once)
			{
				once = true;
				donecb = new createStandardWearablesAllDoneCallback;
			}
			llassert( mWearableEntry[i].mWearable == NULL );
			LLWearable* wearable = gWearableList.createNewWearable((EWearableType)i);
			mWearableEntry[i].mWearable = wearable;
			// no need to update here...
			LLPointer<LLInventoryCallback> cb =
				new addWearableToAgentInventoryCallback(
					donecb,
					i,
					wearable,
					addWearableToAgentInventoryCallback::CALL_CREATESTANDARDDONE);
			addWearableToAgentInventory(cb, wearable, LLUUID::null, FALSE);
		}
	}
}
void LLAgent::createStandardWearablesDone(S32 index)
{
	LLWearable* wearable = mWearableEntry[index].mWearable;

	if (wearable)
	{
		wearable->writeToAvatar(TRUE);
	}
}

void LLAgent::createStandardWearablesAllDone()
{
	// ... because sendAgentWearablesUpdate will notify inventory
	// observers.
	mWearablesLoaded = TRUE; 
	sendAgentWearablesUpdate();
	sendAgentSetAppearance();

	// Treat this as the first texture entry message, if none received yet
	mAvatarObject->onFirstTEMessageReceived();
}

void LLAgent::makeNewOutfit( 
	const std::string& new_folder_name,
	const LLDynamicArray<S32>& wearables_to_include,
	const LLDynamicArray<S32>& attachments_to_include,
	BOOL rename_clothing)
{
	if (mAvatarObject.isNull())
	{
		return;
	}

	BOOL fUseLinks = !gSavedSettings.getBOOL("UseInventoryLinks");
	BOOL fUseOutfits = gSavedSettings.getBOOL("UseOutfitFolders");

	LLAssetType::EType typeDest = (fUseOutfits) ? LLAssetType::AT_MY_OUTFITS : LLAssetType::AT_CLOTHING;
	LLAssetType::EType typeFolder = (fUseOutfits) ? LLAssetType::AT_OUTFIT : LLAssetType::AT_NONE;

	// First, make a folder for the outfit.
	LLUUID folder_id = gInventory.createNewCategory(gInventory.findCategoryUUIDForType(typeDest), typeFolder, new_folder_name);

	bool found_first_item = false;

	///////////////////
	// Wearables

	if( wearables_to_include.count() )
	{
		// Then, iterate though each of the wearables and save copies of them in the folder.
		S32 i;
		S32 count = wearables_to_include.count();
		LLPointer<LLRefCount> cbdone = NULL;
		for( i = 0; i < count; ++i )
		{
			S32 index = wearables_to_include[i];
			LLWearable* old_wearable = mWearableEntry[ index ].mWearable;
			if( old_wearable )
			{
				LLViewerInventoryItem* item = gInventory.getItem(mWearableEntry[index].mItemID);
				if (fUseOutfits)
				{
					std::string strOrdering = llformat("@%d", item->getWearableType() * 100);

					link_inventory_item(
						gAgent.getID(),
						item->getLinkedUUID(),
						folder_id,
						item->getName(),
						strOrdering,
						LLAssetType::AT_LINK,
						LLPointer<LLInventoryCallback>(NULL));
				}
				else
				{
					std::string new_name = item->getName();
					if (rename_clothing)
					{
						new_name = new_folder_name;
						new_name.append(" ");
						new_name.append(old_wearable->getTypeLabel());
						LLStringUtil::truncate(new_name, DB_INV_ITEM_NAME_STR_LEN);
					}

					if (fUseLinks || isWearableCopyable((EWearableType)index))
					{
						LLWearable* new_wearable = gWearableList.createCopy(old_wearable);
						if (rename_clothing)
						{
							new_wearable->setName(new_name);
						}

						S32 todo = addWearableToAgentInventoryCallback::CALL_NONE;
						if (!found_first_item)
						{
							found_first_item = true;
							/* set the focus to the first item */
							todo |= addWearableToAgentInventoryCallback::CALL_MAKENEWOUTFITDONE;
							/* send the agent wearables update when done */
							cbdone = new sendAgentWearablesUpdateCallback;
						}
						LLPointer<LLInventoryCallback> cb =
							new addWearableToAgentInventoryCallback(
								cbdone,
								index,
								new_wearable,
								todo);
						if (isWearableCopyable((EWearableType)index))
						{
							copy_inventory_item(
								gAgent.getID(),
								item->getPermissions().getOwner(),
								item->getLinkedUUID(),
								folder_id,
								new_name,
								cb);
						}
						else
						{
							move_inventory_item(
								gAgent.getID(),
								gAgent.getSessionID(),
								item->getLinkedUUID(),
								folder_id,
								new_name,
								cb);
						}
					}
					else
					{
						link_inventory_item(
							gAgent.getID(),
							item->getLinkedUUID(),
							folder_id,
							item->getName(),		// Apparently, links cannot have arbitrary names...
							item->getDescription(),
							LLAssetType::AT_LINK,
							LLPointer<LLInventoryCallback>(NULL));
					}
				}
			}
		}
		gInventory.notifyObservers();
	}


	///////////////////
	// Attachments

	if( attachments_to_include.count() )
	{
		for( S32 i = 0; i < attachments_to_include.count(); i++ )
		{
			S32 attachment_pt = attachments_to_include[i];
			LLViewerJointAttachment* attachment = get_if_there(mAvatarObject->mAttachmentPoints, attachment_pt, (LLViewerJointAttachment*)NULL );
			if(!attachment) continue;
			for (LLViewerJointAttachment::attachedobjs_vec_t::iterator attachment_iter = attachment->mAttachedObjects.begin();
				 attachment_iter != attachment->mAttachedObjects.end();
				 ++attachment_iter)
 			{
				LLViewerObject *attached_object = (*attachment_iter);
				if (!attached_object) continue;
				const LLUUID& item_id = attached_object->getAttachmentItemID();
				if (item_id.isNull()) continue;
				LLInventoryItem* item = gInventory.getItem(item_id);
				if (!item) continue;
				if (fUseOutfits)
				{
					link_inventory_item(
						gAgent.getID(),
						item->getLinkedUUID(),
						folder_id,
						item->getName(),
						item->getDescription(),
						LLAssetType::AT_LINK,
						LLPointer<LLInventoryCallback>(NULL));
				}
				else
				{
					if (fUseLinks || item->getPermissions().allowCopyBy(gAgent.getID()))
					{
						const LLUUID& old_folder_id = item->getParentUUID();

						move_inventory_item(
							gAgent.getID(),
							gAgent.getSessionID(),
							item->getLinkedUUID(),
							folder_id,
							item->getName(),
							LLPointer<LLInventoryCallback>(NULL));

						if (item->getPermissions().allowCopyBy(gAgent.getID()))
						{
							copy_inventory_item(
								gAgent.getID(),
								item->getPermissions().getOwner(),
								item->getLinkedUUID(),
								old_folder_id,
								item->getName(),
								LLPointer<LLInventoryCallback>(NULL));
						}
					}
					else
					{
						link_inventory_item(
							gAgent.getID(),
							item->getLinkedUUID(),
							folder_id,
							item->getName(),
							item->getDescription(),
							LLAssetType::AT_LINK,
							LLPointer<LLInventoryCallback>(NULL));
					}
				}
			}
		}
	} 
}

void LLAgent::makeNewOutfitDone(S32 index)
{
	LLUUID first_item_id = mWearableEntry[index].mItemID;
	// Open the inventory and select the first item we added.
	if( first_item_id.notNull() )
	{
		LLInventoryView* view = LLInventoryView::getActiveInventory();
		if(view)
		{
			view->getPanel()->setSelection(first_item_id, TAKE_FOCUS_NO);
		}
	}
}


void LLAgent::addWearableToAgentInventory(
	LLPointer<LLInventoryCallback> cb,
	LLWearable* wearable,
	const LLUUID& category_id,
	BOOL notify)
{
	create_inventory_item(
		gAgent.getID(),
		gAgent.getSessionID(),
		category_id,
		wearable->getTransactionID(),
		wearable->getName(),
		wearable->getDescription(),
		wearable->getAssetType(),
		LLInventoryType::IT_WEARABLE,
		wearable->getType(),
		wearable->getPermissions().getMaskNextOwner(),
		cb);
}

//-----------------------------------------------------------------------------
// sendAgentSetAppearance()
//-----------------------------------------------------------------------------
void LLAgent::sendAgentSetAppearance()
{
	if (mAvatarObject.isNull()) return;

	if (mNumPendingQueries > 0 && !gAgent.cameraCustomizeAvatar()) 
	{
		return;
	}


	llinfos << "TAT: Sent AgentSetAppearance: " << mAvatarObject->getBakedStatusForPrintout() << llendl;
	//dumpAvatarTEs( "sendAgentSetAppearance()" );

	LLMessageSystem* msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_AgentSetAppearance);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, getID());
	msg->addUUIDFast(_PREHASH_SessionID, getSessionID());

	// correct for the collision tolerance (to make it look like the 
	// agent is actually walking on the ground/object)
	// NOTE -- when we start correcting all of the other Havok geometry 
	// to compensate for the COLLISION_TOLERANCE ugliness we will have 
	// to tweak this number again
	LLVector3 body_size = mAvatarObject->mBodySize;
        body_size.mV[VX] = body_size.mV[VX] + gSavedPerAccountSettings.getF32("PhoenixAvatarXModifier");
        body_size.mV[VY] = body_size.mV[VY] + gSavedPerAccountSettings.getF32("PhoenixAvatarYModifier");
        body_size.mV[VZ] = body_size.mV[VZ] + gSavedPerAccountSettings.getF32("PhoenixAvatarZModifier");
	msg->addVector3Fast(_PREHASH_Size, body_size);	

	// To guard against out of order packets
	// Note: always start by sending 1.  This resets the server's count. 0 on the server means "uninitialized"
	mAppearanceSerialNum++;
	msg->addU32Fast(_PREHASH_SerialNum, mAppearanceSerialNum );

	// is texture data current relative to wearables?
	// KLW - TAT this will probably need to check the local queue.
	BOOL textures_current = !mAvatarObject->hasPendingBakedUploads() && mWearablesLoaded;

	for(U8 baked_index = 0; baked_index < BAKED_NUM_INDICES; baked_index++ )
	{
		const ETextureIndex texture_index = getTextureIndex((EBakedTextureIndex)baked_index);

		// if we're not wearing a skirt, we don't need the texture to be baked
		if (texture_index == TEX_SKIRT_BAKED && !mAvatarObject->isWearingWearableType(WT_SKIRT))
		{
			continue;
		}

		// IMG_DEFAULT_AVATAR means not baked
		if (!mAvatarObject->isTextureDefined(texture_index))
		{
			textures_current = FALSE;
			break;
		}
	}

	// only update cache entries if we have all our baked textures
	if (textures_current)
	{
		llinfos << "TAT: Sending cached texture data" << llendl;
		for (U8 baked_index = 0; baked_index < BAKED_NUM_INDICES; baked_index++)
		{
			const LLVOAvatarDictionary::WearableDictionaryEntry *wearable_dict = LLVOAvatarDictionary::getInstance()->getWearable((EBakedTextureIndex)baked_index);
			LLUUID hash;
			for (U8 i=0; i < wearable_dict->mWearablesVec.size(); i++)
			{
				// EWearableType wearable_type = gBakedWearableMap[baked_index][wearable_num];
				const EWearableType wearable_type = wearable_dict->mWearablesVec[i];
				const LLWearable* wearable = getWearable(wearable_type);
				if (wearable)
				{
					hash ^= wearable->getID();
				}
			}
			if (hash.notNull())
			{
				hash ^= wearable_dict->mHashID;
			}

			const ETextureIndex texture_index = getTextureIndex((EBakedTextureIndex)baked_index);

			msg->nextBlockFast(_PREHASH_WearableData);
			msg->addUUIDFast(_PREHASH_CacheID, hash);
			msg->addU8Fast(_PREHASH_TextureIndex, (U8)texture_index);
		}
		msg->nextBlockFast(_PREHASH_ObjectData);
		mAvatarObject->packTEMessage( gMessageSystem, TRUE );
	}
	else
	{
		// If the textures aren't baked, send NULL for texture IDs
		// This means the baked texture IDs on the server will be untouched.
		// Once all textures are baked, another AvatarAppearance message will be sent to update the TEs
		msg->nextBlockFast(_PREHASH_ObjectData);
		gMessageSystem->addBinaryDataFast(_PREHASH_TextureEntry, NULL, 0);
	}

	BOOL send_v1_message = gSavedSettings.getBOOL("PhoenixDontSendAvPhysicsParms");
	BOOL wearing_physics = (getWearable(WT_PHYSICS) != NULL);
	S32 transmitted_params = 0;
	for (LLViewerVisualParam* param = (LLViewerVisualParam*)mAvatarObject->getFirstVisualParam();
		 param;
		 param = (LLViewerVisualParam*)mAvatarObject->getNextVisualParam())
	{
		// Do not transmit params of group
		//  VISUAL_PARAM_GROUP_TWEAKABLE_NO_TRANSMIT. If we're
		//  sending the version 1 compatible message, then strip out   
		//  V2-only parameters that have IDs of 1100 or higher.   
		if ((param->getGroup() == VISUAL_PARAM_GROUP_TWEAKABLE) &&
			((param->getID() < 1100) || 
				(!send_v1_message && wearing_physics)))
		{
			msg->nextBlockFast(_PREHASH_VisualParam );
			
			// We don't send the param ids.  Instead, we assume that the receiver has the same params in the same sequence.
			F32 param_value;
#ifdef OLD_BREAST_PHYSICS
			if(param->getID() == 507)
				param_value = mAvatarObject->getActualBoobGrav();
			else
#endif
				param_value = param->getWeight();
			const U8 new_weight = F32_to_U8(param_value, param->getMinWeight(), param->getMaxWeight());
			msg->addU8Fast(_PREHASH_ParamValue, new_weight );
			transmitted_params++;
		}
	}

	llinfos << "Avatar XML num VisualParams transmitted = " << transmitted_params << llendl;
	sendReliableMessage();
}

void LLAgent::sendAgentDataUpdateRequest()
{
	gMessageSystem->newMessageFast(_PREHASH_AgentDataUpdateRequest);
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	sendReliableMessage();
}

void LLAgent::removeWearable( EWearableType type )
{
	LLWearable* old_wearable = mWearableEntry[ type ].mWearable;

	if ( (gAgent.isTeen())
		 && (type == WT_UNDERSHIRT || type == WT_UNDERPANTS))
	{
		// Can't take off underclothing in simple UI mode or on PG accounts
		return;
	}

// [RLVa:KB] - Checked: 2009-07-07 (RLVa-1.1.3b)
	if ( (rlv_handler_t::isEnabled()) && (!gRlvWearableLocks.canRemove(type)) )
	{
		return;
	}
// [/RLVa:KB]

	if( old_wearable )
	{
		if( old_wearable->isDirty() )
		{
			LLSD payload;
			payload["wearable_type"] = (S32)type;
			// Bring up view-modal dialog: Save changes? Yes, No, Cancel
			LLNotifications::instance().add("WearableSave", LLSD(), payload, &LLAgent::onRemoveWearableDialog);
			return;
		}
		else
		{
			removeWearableFinal( type );
		}
	}
}

// static 
bool LLAgent::onRemoveWearableDialog(const LLSD& notification, const LLSD& response )
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	EWearableType type = (EWearableType)notification["payload"]["wearable_type"].asInteger();
	switch( option )
	{
	case 0:  // "Save"
		gAgent.saveWearable( type );
		gAgent.removeWearableFinal( type );
		break;

	case 1:  // "Don't Save"
		gAgent.removeWearableFinal( type );
		break;

	case 2: // "Cancel"
		break;

	default:
		llassert(0);
		break;
	}
	return false;
}

// Called by removeWearable() and onRemoveWearableDialog() to actually do the removal.
void LLAgent::removeWearableFinal( EWearableType type )
{
	LLWearable* old_wearable = mWearableEntry[ type ].mWearable;

	gInventory.addChangedMask( LLInventoryObserver::LABEL, mWearableEntry[type].mItemID );

	mWearableEntry[ type ].mWearable = NULL;
	mWearableEntry[ type ].mItemID.setNull();

	queryWearableCache();

	if( old_wearable )
	{
		old_wearable->removeFromAvatar( TRUE );
	}

	// Update the server
	sendAgentWearablesUpdate(); 
	sendAgentSetAppearance();
	gInventory.notifyObservers();
}

void LLAgent::copyWearableToInventory( EWearableType type )
{
	LLWearable* wearable = mWearableEntry[ type ].mWearable;
	if( wearable )
	{
		// Save the old wearable if it has changed.
		if( wearable->isDirty() )
		{
			wearable = gWearableList.createCopyFromAvatar( wearable );
			mWearableEntry[ type ].mWearable = wearable;
		}

		// Make a new entry in the inventory.  (Put it in the same folder as the original item if possible.)
		LLUUID category_id;
		LLInventoryItem* item = gInventory.getItem( mWearableEntry[ type ].mItemID );
		if( item )
		{
			category_id = item->getParentUUID();
			wearable->setPermissions(item->getPermissions());
		}
		LLPointer<LLInventoryCallback> cb =
			new addWearableToAgentInventoryCallback(
				LLPointer<LLRefCount>(NULL),
				type,
				wearable);
		addWearableToAgentInventory(cb, wearable, category_id);
	}
}


// A little struct to let setWearable() communicate more than one value with onSetWearableDialog().
struct LLSetWearableData
{
	LLSetWearableData( const LLUUID& new_item_id, LLWearable* new_wearable ) :
		mNewItemID( new_item_id ), mNewWearable( new_wearable ) {}
	LLUUID				mNewItemID;
	LLWearable*			mNewWearable;
};

BOOL LLAgent::needsReplacement(EWearableType  wearableType, S32 remove)
{
	return TRUE;
	/*if (remove) return TRUE;
	
	return getWearable(wearableType) ? TRUE : FALSE;*/
}

// Assumes existing wearables are not dirty.
void LLAgent::setWearableOutfit( 
	const LLInventoryItem::item_array_t& items,
	const LLDynamicArray< LLWearable* >& wearables,
	BOOL remove )
{
	lldebugs << "setWearableOutfit() start" << llendl;

	BOOL wearables_to_remove[WT_COUNT];
	wearables_to_remove[WT_SHAPE]		= FALSE;
	wearables_to_remove[WT_SKIN]		= FALSE;
	wearables_to_remove[WT_HAIR]		= FALSE;
	wearables_to_remove[WT_EYES]		= FALSE;
// [RLVa:KB] - Checked: 2009-07-06 (RLVa-1.1.3b) | Added: RLVa-0.2.2a
	wearables_to_remove[WT_SHIRT]		= remove && gRlvWearableLocks.canRemove(WT_SHIRT);
	wearables_to_remove[WT_PANTS]		= remove && gRlvWearableLocks.canRemove(WT_PANTS);
	wearables_to_remove[WT_SHOES]		= remove && gRlvWearableLocks.canRemove(WT_SHOES);
	wearables_to_remove[WT_SOCKS]		= remove && gRlvWearableLocks.canRemove(WT_SOCKS);
	wearables_to_remove[WT_JACKET]		= remove && gRlvWearableLocks.canRemove(WT_JACKET);
	wearables_to_remove[WT_GLOVES]		= remove && gRlvWearableLocks.canRemove(WT_GLOVES);
	wearables_to_remove[WT_UNDERSHIRT]	= (!gAgent.isTeen()) && remove && gRlvWearableLocks.canRemove(WT_UNDERSHIRT);
	wearables_to_remove[WT_UNDERPANTS]	= (!gAgent.isTeen()) && remove && gRlvWearableLocks.canRemove(WT_UNDERPANTS);
	wearables_to_remove[WT_SKIRT]		= remove && gRlvWearableLocks.canRemove(WT_SKIRT);
	wearables_to_remove[WT_ALPHA]		= remove && gRlvWearableLocks.canRemove(WT_ALPHA);
	wearables_to_remove[WT_TATTOO]		= remove && gRlvWearableLocks.canRemove(WT_TATTOO);
	wearables_to_remove[WT_PHYSICS]     = remove && gRlvWearableLocks.canRemove(WT_PHYSICS);
// [/RLVa:KB]

	S32 count = wearables.count();
	llassert( items.count() == count );

	S32 i;
	for( i = 0; i < count; i++ )
	{
		LLWearable* new_wearable = wearables[i];
		LLPointer<LLInventoryItem> new_item = items[i];

		EWearableType type = new_wearable->getType();
		wearables_to_remove[type] = FALSE;

		LLWearable* old_wearable = mWearableEntry[ type ].mWearable;
		if( old_wearable )
		{
			const LLUUID& old_item_id = mWearableEntry[ type ].mItemID;
			if( (old_wearable->getID() == new_wearable->getID()) &&
				(old_item_id == new_item->getLinkedUUID()) )
			{
				lldebugs << "No change to wearable asset and item: " << LLWearable::typeToTypeName( type ) << llendl;
				continue;
			}

			gInventory.addChangedMask(LLInventoryObserver::LABEL, old_item_id);

			// Assumes existing wearables are not dirty.
			if( old_wearable->isDirty() )
			{
				llassert(0);
				continue;
			}
		}

		mWearableEntry[ type ].mItemID = new_item->getLinkedUUID();
		mWearableEntry[ type ].mWearable = new_wearable;
	}

	std::vector<LLWearable*> wearables_being_removed;

	for( i = 0; i < WT_COUNT; i++ )
	{
		if( wearables_to_remove[i] )
		{
			wearables_being_removed.push_back(mWearableEntry[ i ].mWearable);
			mWearableEntry[ i ].mWearable = NULL;
			
			gInventory.addChangedMask(LLInventoryObserver::LABEL, mWearableEntry[ i ].mItemID);
			mWearableEntry[ i ].mItemID.setNull();
		}
	}

	gInventory.notifyObservers();

	queryWearableCache();

	std::vector<LLWearable*>::iterator wearable_iter;

	for( wearable_iter = wearables_being_removed.begin(); 
		wearable_iter != wearables_being_removed.end();
		++wearable_iter)
	{
		LLWearable* wearablep = *wearable_iter;
		if (wearablep)
		{
			wearablep->removeFromAvatar( TRUE );
		}
	}

	for( i = 0; i < count; i++ )
	{
		wearables[i]->writeToAvatar( TRUE );
	}

	// Start rendering & update the server
	mWearablesLoaded = TRUE; 
	sendAgentWearablesUpdate();
	sendAgentSetAppearance();

	lldebugs << "setWearableOutfit() end" << llendl;
}


// User has picked "wear on avatar" from a menu.
void LLAgent::setWearable( LLInventoryItem* new_item, LLWearable* new_wearable )
{
	EWearableType type = new_wearable->getType();

	LLWearable* old_wearable = mWearableEntry[ type ].mWearable;

// [RLVa:KB] - Checked: 2009-07-07 (RLVa-1.0.0d)
	// Block if: we can't wear on that layer; or we're already wearing something there we can't take off
	if ( (rlv_handler_t::isEnabled()) && (!gRlvWearableLocks.canWear(type)) )
	{
		return;
	}
// [/RLVa:KB]

	if( old_wearable )
	{
		const LLUUID& old_item_id = mWearableEntry[ type ].mItemID;
		if( (old_wearable->getID() == new_wearable->getID()) &&
			(old_item_id == new_item->getUUID()) )
		{
			lldebugs << "No change to wearable asset and item: " << LLWearable::typeToTypeName( type ) << llendl;
			return;
		}

		if( old_wearable->isDirty() )
		{
			// Bring up modal dialog: Save changes? Yes, No, Cancel
			LLSD payload;
			payload["item_id"] = new_item->getUUID();
			LLNotifications::instance().add( "WearableSave", LLSD(), payload, boost::bind(LLAgent::onSetWearableDialog, _1, _2, new_wearable));
			return;
		}
	}

	setWearableFinal( new_item, new_wearable );
}

// static 
bool LLAgent::onSetWearableDialog( const LLSD& notification, const LLSD& response, LLWearable* wearable )
{
	S32 option = LLNotification::getSelectedOption(notification, response);
	LLInventoryItem* new_item = gInventory.getItem( notification["payload"]["item_id"].asUUID());
	if( !new_item )
	{
		delete wearable;
		return false;
	}

	switch( option )
	{
	case 0:  // "Save"
		gAgent.saveWearable( wearable->getType() );
		gAgent.setWearableFinal( new_item, wearable );
		break;

	case 1:  // "Don't Save"
		gAgent.setWearableFinal( new_item, wearable );
		break;

	case 2: // "Cancel"
		break;

	default:
		llassert(0);
		break;
	}

//	Don't delete the wearable we were passed here, because it's the one we're using now -Zi
//	Fixes PHOE-59, PHOE-1047 (partly). This might introduce a minor memory leak, but it's
//	better than crashing for the rest of the V1.x lifetime.
//	delete wearable;
	return false;
}

// Called from setWearable() and onSetWearableDialog() to actually set the wearable.
void LLAgent::setWearableFinal( LLInventoryItem* new_item, LLWearable* new_wearable )
{
	EWearableType type = new_wearable->getType();

	// Replace the old wearable with a new one.
	llassert( new_item->getAssetUUID() == new_wearable->getID() );
	LLUUID old_item_id = mWearableEntry[ type ].mItemID;
	mWearableEntry[ type ].mItemID = new_item->getLinkedUUID();
	mWearableEntry[ type ].mWearable = new_wearable;

	if (old_item_id.notNull())
	{
		gInventory.addChangedMask( LLInventoryObserver::LABEL, old_item_id );
		gInventory.notifyObservers();
	}

	//llinfos << "LLVOAvatar::setWearable()" << llendl;
	queryWearableCache();
	new_wearable->writeToAvatar( TRUE );

	// Update the server
	sendAgentWearablesUpdate();
	sendAgentSetAppearance();
}

void LLAgent::queryWearableCache()
{
	if (!mWearablesLoaded)
	{
		return;
	}

	// Look up affected baked textures.
	// If they exist:
	//		disallow updates for affected layersets (until dataserver responds with cache request.)
	//		If cache miss, turn updates back on and invalidate composite.
	//		If cache hit, modify baked texture entries.
	//
	// Cache requests contain list of hashes for each baked texture entry.
	// Response is list of valid baked texture assets. (same message)

	gMessageSystem->newMessageFast(_PREHASH_AgentCachedTexture);
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, getSessionID());
	gMessageSystem->addS32Fast(_PREHASH_SerialNum, mTextureCacheQueryID);

	S32 num_queries = 0;
	for (U8 baked_index = 0; baked_index < BAKED_NUM_INDICES; baked_index++ )
	{
		const LLVOAvatarDictionary::WearableDictionaryEntry *wearable_dict = LLVOAvatarDictionary::getInstance()->getWearable((EBakedTextureIndex)baked_index);
		LLUUID hash;
		for (U8 i=0; i < wearable_dict->mWearablesVec.size(); i++)
		{
			// EWearableType wearable_type = gBakedWearableMap[baked_index][wearable_num];
			const EWearableType wearable_type = wearable_dict->mWearablesVec[i];
			const LLWearable* wearable = getWearable(wearable_type);
			if (wearable)
			{
				hash ^= wearable->getID();
			}
		}
		if (hash.notNull())
		{
			hash ^= wearable_dict->mHashID;
			num_queries++;
			// *NOTE: make sure at least one request gets packed

			//llinfos << "Requesting texture for hash " << hash << " in baked texture slot " << baked_index << llendl;
			gMessageSystem->nextBlockFast(_PREHASH_WearableData);
			gMessageSystem->addUUIDFast(_PREHASH_ID, hash);
			gMessageSystem->addU8Fast(_PREHASH_TextureIndex, (U8)baked_index);
		}

		mActiveCacheQueries[ baked_index ] = mTextureCacheQueryID;
	}

	llinfos << "Requesting texture cache entry for " << num_queries << " baked textures" << llendl;
	gMessageSystem->sendReliable(getRegion()->getHost());
	mNumPendingQueries++;
	mTextureCacheQueryID++;
}

// User has picked "remove from avatar" from a menu.
// static
void LLAgent::userRemoveWearable( void* userdata )
{
	EWearableType type = (EWearableType)(intptr_t)userdata;
	
	if( !(type==WT_SHAPE || type==WT_SKIN || type==WT_HAIR || type==WT_EYES) ) //&&
		//!((!gAgent.isTeen()) && ( type==WT_UNDERPANTS || type==WT_UNDERSHIRT )) )
	{
		gAgent.removeWearable( type );
	}
}

void LLAgent::userRemoveAllClothes( void* userdata )
{
	// We have to do this up front to avoid having to deal with the case of multiple wearables being dirty.
	if( gFloaterCustomize )
	{
		gFloaterCustomize->askToSaveIfDirty( LLAgent::userRemoveAllClothesStep2, NULL );
	}
	else
	{
		LLAgent::userRemoveAllClothesStep2( TRUE, NULL );
	}
}

void LLAgent::userRemoveAllClothesStep2( BOOL proceed, void* userdata )
{
	if( proceed )
	{
		gAgent.removeWearable( WT_SHIRT );
		gAgent.removeWearable( WT_PANTS );
		gAgent.removeWearable( WT_SHOES );
		gAgent.removeWearable( WT_SOCKS );
		gAgent.removeWearable( WT_JACKET );
		gAgent.removeWearable( WT_GLOVES );
		gAgent.removeWearable( WT_UNDERSHIRT );
		gAgent.removeWearable( WT_UNDERPANTS );
		gAgent.removeWearable( WT_SKIRT );
		gAgent.removeWearable( WT_ALPHA );
		gAgent.removeWearable( WT_TATTOO );
		gAgent.removeWearable( WT_PHYSICS );
	}
}

void LLAgent::userRemoveAllAttachments( void* userdata )
{
	LLVOAvatar* avatarp = gAgent.getAvatarObject();
	if(!avatarp)
	{
		llwarns << "No avatar found." << llendl;
		return;
	}

	llvo_vec_t objects_to_remove;
	
	for (LLVOAvatar::attachment_map_t::iterator iter = avatarp->mAttachmentPoints.begin(); 
		 iter != avatarp->mAttachmentPoints.end();)
	{
		LLVOAvatar::attachment_map_t::iterator curiter = iter++;
		LLViewerJointAttachment* attachment = curiter->second;
		for (LLViewerJointAttachment::attachedobjs_vec_t::iterator attachment_iter = attachment->mAttachedObjects.begin();
			 attachment_iter != attachment->mAttachedObjects.end();
			 ++attachment_iter)
		{
			LLViewerObject *attached_object = (*attachment_iter);
			//KC: skip the bridge attachment point
			if (ATTACHMENT_ID_FROM_STATE(attached_object->getState()) == (S32)JCLSLBridge::PH_BRIDGE_POINT) continue;
//			if (attached_object)
// [RLVa:KB] - Checked: 2010-09-28 (RLVa-1.1.3b) | Modified: RLVa-1.1.3b
			if ( (attached_object) && ((!rlv_handler_t::isEnabled()) || (!gRlvAttachmentLocks.isLockedAttachment(attached_object)))
				/*&& (attached_object->getAttachmentItemID() != JCLSLBridge::instance().mBridge->getUUID())*/ //KC: dont remove our bridge
			)
// [/RLVa:KB]
			{
				llinfos << "bridge trying to get removed with everything, keeping" << llendl;
				objects_to_remove.push_back(attached_object);
			}
		}
	}
	userRemoveMultipleAttachments(objects_to_remove);
}

void LLAgent::observeFriends()
{
	if(!mFriendObserver)
	{
		mFriendObserver = new LLAgentFriendObserver;
		LLAvatarTracker::instance().addObserver(mFriendObserver);
		friendsChanged();
	}
}

void LLAgent::parseTeleportMessages(const std::string& xml_filename)
{
	LLXMLNodePtr root;
	BOOL success = LLUICtrlFactory::getLayeredXMLNode(xml_filename, root);

	if (!success || !root || !root->hasName( "teleport_messages" ))
	{
		llerrs << "Problem reading teleport string XML file: " 
			   << xml_filename << llendl;
		return;
	}

	for (LLXMLNode* message_set = root->getFirstChild();
		 message_set != NULL;
		 message_set = message_set->getNextSibling())
	{
		if ( !message_set->hasName("message_set") ) continue;

		std::map<std::string, std::string> *teleport_msg_map = NULL;
		std::string message_set_name;

		if ( message_set->getAttributeString("name", message_set_name) )
		{
			//now we loop over all the string in the set and add them
			//to the appropriate set
			if ( message_set_name == "errors" )
			{
				teleport_msg_map = &sTeleportErrorMessages;
			}
			else if ( message_set_name == "progress" )
			{
				teleport_msg_map = &sTeleportProgressMessages;
			}
		}

		if ( !teleport_msg_map ) continue;

		std::string message_name;
		for (LLXMLNode* message_node = message_set->getFirstChild();
			 message_node != NULL;
			 message_node = message_node->getNextSibling())
		{
			if ( message_node->hasName("message") && 
				 message_node->getAttributeString("name", message_name) )
			{
				(*teleport_msg_map)[message_name] =
					message_node->getTextContents();
			} //end if ( message exists and has a name)
		} //end for (all message in set)
	}//end for (all message sets in xml file)
}


void LLAgent::setLastRegionData(std::string regionName, LLVector3 agentCoords)
{
	mLastRegion = regionName;
	mLastCoordinates = agentCoords;
}

std::string LLAgent::getLastRegion()
{
	return mLastRegion;
}

LLVector3 LLAgent::getLastCoords()
{
	return mLastCoordinates;
}

// Combines userRemoveAllAttachments() and userAttachMultipleAttachments() logic to
// get attachments into desired state with minimal number of adds/removes.
//void LLAgentWearables::userUpdateAttachments(LLInventoryModel::item_array_t& obj_item_array)
// [SL:KB] - Patch: Appearance-SyncAttach | Checked: 2010-09-22 (Catznip-2.2.0a) | Added: Catznip-2.2.0a
void LLAgent::userUpdateAttachments(LLInventoryModel::item_array_t& obj_item_array, bool fAttachOnly)
// [/SL:KB]
{
	// Possible cases:
	// already wearing but not in request set -> take off.
	// already wearing and in request set -> leave alone.
	// not wearing and in request set -> put on.

	LLVOAvatar* pAvatar = gAgent.getAvatarObject();
	if (!pAvatar) return;

	std::set<LLUUID> requested_item_ids;
	std::set<LLUUID> current_item_ids;
	for (S32 i=0; i<obj_item_array.count(); i++)
		requested_item_ids.insert(obj_item_array[i].get()->getLinkedUUID());

	// Build up list of objects to be removed and items currently attached.
	llvo_vec_t objects_to_remove;
	for (LLVOAvatar::attachment_map_t::iterator iter = pAvatar->mAttachmentPoints.begin(); 
		 iter != pAvatar->mAttachmentPoints.end();)
	{
		LLVOAvatar::attachment_map_t::iterator curiter = iter++;
		LLViewerJointAttachment* attachment = curiter->second;
		for (LLViewerJointAttachment::attachedobjs_vec_t::iterator attachment_iter = attachment->mAttachedObjects.begin();
			 attachment_iter != attachment->mAttachedObjects.end();
			 ++attachment_iter)
		{
			LLViewerObject *objectp = (*attachment_iter);
			if (objectp)
			{
				LLUUID object_item_id = objectp->getAttachmentItemID();
				if (ATTACHMENT_ID_FROM_STATE(objectp->getState()) == (S32)JCLSLBridge::PH_BRIDGE_POINT)
				{
					if (JCLSLBridge::instance().mBridge && object_item_id == JCLSLBridge::instance().mBridge->getUUID()) //KC: keep the bridge on
					{
						llinfos << "bridge trying to get removed, keeping" << llendl;
						current_item_ids.insert(object_item_id);
					}
					else //KC: remove anything else from the bridge point
					{
						objects_to_remove.push_back(objectp);
					}
				}
				else if (requested_item_ids.find(object_item_id) != requested_item_ids.end())
				{
					// Object currently worn, was requested.
					// Flag as currently worn so we won't have to add it again.
					current_item_ids.insert(object_item_id);
				}
				
				else
				{
					// object currently worn, not requested.
					objects_to_remove.push_back(objectp);
				}
			}
		}
	}

	LLInventoryModel::item_array_t items_to_add;
	for (LLInventoryModel::item_array_t::iterator it = obj_item_array.begin();
		 it != obj_item_array.end();
		 ++it)
	{
		LLUUID linked_id = (*it).get()->getLinkedUUID();
		if (current_item_ids.find(linked_id) != current_item_ids.end())
		{
			// Requested attachment is already worn.
		}
		else
		{
			// Requested attachment is not worn yet.
			items_to_add.push_back(*it);
		}
	}
	// S32 remove_count = objects_to_remove.size();
	// S32 add_count = items_to_add.size();
	// llinfos << "remove " << remove_count << " add " << add_count << llendl;

	// Remove everything in objects_to_remove
//	userRemoveMultipleAttachments(objects_to_remove);
// [SL:KB] - Patch: Appearance-SyncAttach | Checked: 2010-09-22 (Catznip-2.2.0a) | Added: Catznip-2.2.0a
	if (!fAttachOnly)
	{
		userRemoveMultipleAttachments(objects_to_remove);
	}
// [/SL:KB]

	// Add everything in items_to_add
	userAttachMultipleAttachments(items_to_add);
}

void LLAgent::userRemoveMultipleAttachments(llvo_vec_t& objects_to_remove)
{
	if (!gAgent.getAvatarObject()) return;

// [RLVa:KB] - Checked: 2010-03-04 (RLVa-1.1.3b) | Modified: RLVa-1.2.0a
	// RELEASE-RLVa: [SL-2.0.0] Check our callers and verify that erasing elements from the passed vector won't break random things
	if ( (rlv_handler_t::isEnabled()) && (gRlvAttachmentLocks.hasLockedAttachmentPoint(RLV_LOCK_REMOVE)) )
	{
		llvo_vec_t::iterator itObj = objects_to_remove.begin();
		while (itObj != objects_to_remove.end())
		{
			const LLViewerObject* pAttachObj = *itObj;
			if (gRlvAttachmentLocks.isLockedAttachment(pAttachObj))
			{
				itObj = objects_to_remove.erase(itObj);

				// Fall-back code: re-add the attachment if it got removed from COF somehow (compensates for possible bugs elsewhere)
				LLInventoryModel::cat_array_t folders; LLInventoryModel::item_array_t items;
				LLLinkedItemIDMatches f(pAttachObj->getAttachmentItemID());
				gInventory.collectDescendentsIf(LLCOFMgr::instance().getCOF(), folders, items, LLInventoryModel::EXCLUDE_TRASH, f);
				RLV_ASSERT( 0 != items.count() );
				if (0 == items.count())
					LLCOFMgr::instance().addAttachment(pAttachObj->getAttachmentItemID());
			}
			else
			{
				++itObj;
			}
		}
	}
// [/RLVa:KB]

	if (objects_to_remove.empty())
		return;

	gMessageSystem->newMessage("ObjectDetach");
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	
	for (llvo_vec_t::iterator it = objects_to_remove.begin();
		 it != objects_to_remove.end();
		 ++it)
	{
		LLViewerObject *objectp = *it;
		gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
		gMessageSystem->addU32Fast(_PREHASH_ObjectLocalID, objectp->getLocalID());
	}
	gMessageSystem->sendReliable(gAgent.getRegionHost());
}

void LLAgent::userAttachMultipleAttachments(LLInventoryModel::item_array_t& obj_item_array)
{
// [RLVa:KB] - Checked: 2010-03-04 (RLVa-1.2.0a) | Modified: RLVa-1.2.0a
	// RELEASE-RLVa: [SL-2.0.0] Check our callers and verify that erasing elements from the passed vector won't break random things
	if ( (rlv_handler_t::isEnabled()) && (gRlvAttachmentLocks.hasLockedAttachmentPoint(RLV_LOCK_ANY)) )
	{
		// Fall-back code: everything should really already have been pruned before we get this far
		for (S32 idxItem = obj_item_array.count() - 1; idxItem >= 0; idxItem--)
		{
			const LLInventoryItem* pItem = obj_item_array.get(idxItem).get();
			if (!gRlvAttachmentLocks.canAttach(pItem))
			{
				obj_item_array.remove(idxItem);
				RLV_ASSERT(false);
			}
		}
	}
// [/RLVa:KB]

	// Build a compound message to send all the objects that need to be rezzed.
	S32 obj_count = obj_item_array.count();

	// Limit number of packets to send
	const S32 MAX_PACKETS_TO_SEND = 10;
	const S32 OBJECTS_PER_PACKET = 4;
	const S32 MAX_OBJECTS_TO_SEND = MAX_PACKETS_TO_SEND * OBJECTS_PER_PACKET;
	if( obj_count > MAX_OBJECTS_TO_SEND )
	{
		obj_count = MAX_OBJECTS_TO_SEND;
	}
				
	// Create an id to keep the parts of the compound message together
	LLUUID compound_msg_id;
	compound_msg_id.generate();
	LLMessageSystem* msg = gMessageSystem;

	for(S32 i = 0; i < obj_count; ++i)
	{
		if( 0 == (i % OBJECTS_PER_PACKET) )
		{
			// Start a new message chunk
			msg->newMessageFast(_PREHASH_RezMultipleAttachmentsFromInv);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_HeaderData);
			msg->addUUIDFast(_PREHASH_CompoundMsgID, compound_msg_id );
			msg->addU8Fast(_PREHASH_TotalObjects, obj_count );
			msg->addBOOLFast(_PREHASH_FirstDetachAll, false );
		}

		const LLInventoryItem* item = obj_item_array.get(i).get();
		msg->nextBlockFast(_PREHASH_ObjectData );
		msg->addUUIDFast(_PREHASH_ItemID, item->getLinkedUUID());
		msg->addUUIDFast(_PREHASH_OwnerID, item->getPermissions().getOwner());
		msg->addU8Fast(_PREHASH_AttachmentPt, 0 | ATTACHMENT_ADD);	// Wear at the previous or default attachment point
		pack_permissions_slam(msg, item->getFlags(), item->getPermissions());
		msg->addStringFast(_PREHASH_Name, item->getName());
		msg->addStringFast(_PREHASH_Description, item->getDescription());

		if( (i+1 == obj_count) || ((OBJECTS_PER_PACKET-1) == (i % OBJECTS_PER_PACKET)) )
		{
			// End of message chunk
			msg->sendReliable( gAgent.getRegion()->getHost() );
		}
	}
}

// OGPX - This code will change when capabilities get refactored.
// Right now this is used for capabilities that we get from OGP agent domain
void LLAgent::setCapability(const std::string& name, const std::string& url)
{
#if 0 // OGPX : I think (hope?) we don't need this
	  //    but I'm leaving it here commented out because I'm not quite
	  //    sure why the region capabilities code had it wedged in setCap call
	  //    Maybe the agent domain capabilities will need something like this as well

	if (name == "EventQueueGet")
	{
		delete mEventPoll;
		mEventPoll = NULL;
		mEventPoll = new LLEventPoll(url, getHost());
	}
	else if (name == "UntrustedSimulatorMessage")
	{
		LLHTTPSender::setSender(mHost, new LLCapHTTPSender(url));
	}
	else
#endif
	{
		mCapabilities[name] = url;
	}
}

//OGPX : Agent Domain capabilities...  this needs to be refactored
std::string LLAgent::getCapability(const std::string& name) const
{
	CapabilityMap::const_iterator iter = mCapabilities.find(name);
	if (iter == mCapabilities.end())
	{
		return "";
	}
	return iter->second;
}

void LLAgent::showLureDestination(const std::string fromname, const int global_x, const int global_y, const int x, const int y, const int z, const std::string maturity)
{
	const LLVector3d posglobal = LLVector3d(F64(global_x), F64(global_y), F64(0));
	LLSimInfo* siminfo;
	siminfo = LLWorldMap::getInstance()->simInfoFromPosGlobal(posglobal);
	std::string sim_name;
	LLWorldMap::getInstance()->simNameFromPosGlobal( posglobal, sim_name );
	
	if(siminfo)
	{
		
		llinfos << fromname << "'s teleport lure is to " << sim_name.c_str() << " (" << maturity << ")" << llendl;
		std::string url = LLURLDispatcher::buildSLURL(sim_name.c_str(), S32(x), S32(y), S32(z));
		std::string msg;
		msg = llformat("%s's teleport lure is to %s", fromname.c_str(), url.c_str());
		if(maturity != "")
			msg.append(llformat(" (%s)", maturity.c_str()));
		LLChat chat(msg);
		LLFloaterChat::addChat(chat);
	}
	else
	{
		LLAgent::lure_show = TRUE;
		LLAgent::lure_name = fromname;
		LLAgent::lure_posglobal = posglobal;
		LLAgent::lure_global_x = U16(global_x / 256);
		LLAgent::lure_global_y = U16(global_y / 256);
		LLAgent::lure_x = x;
		LLAgent::lure_y = y;
		LLAgent::lure_z = z;
		LLAgent::lure_maturity = maturity;
		LLWorldMapMessage::getInstance()->sendMapBlockRequest(lure_global_x, lure_global_y, lure_global_x, lure_global_y, true);
	}
}

void LLAgent::onFoundLureDestination()
{
	LLAgent::lure_show = FALSE;
	LLSimInfo* siminfo;
	siminfo = LLWorldMap::getInstance()->simInfoFromPosGlobal(LLAgent::lure_posglobal);
	std::string sim_name;
	LLWorldMap::getInstance()->simNameFromPosGlobal( LLAgent::lure_posglobal, sim_name );

	if(siminfo && (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC)))
	{
		llinfos << LLAgent::lure_name << " is offering a TP to " << sim_name.c_str() << " (" << LLAgent::lure_maturity << ")" << llendl;
		std::string url = LLURLDispatcher::buildSLURL(sim_name.c_str(), S32(LLAgent::lure_x), S32(LLAgent::lure_y), S32(LLAgent::lure_z));
		std::string msg;
		msg = llformat("%s is offering a TP to %s", LLAgent::lure_name.c_str(), url.c_str());
		if(LLAgent::lure_maturity != "")
			msg.append(llformat(" (%s)", LLAgent::lure_maturity.c_str()));
		LLChat chat(msg);
		LLFloaterChat::addChat(chat);
	}
}

// EOF

