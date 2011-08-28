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

#include "lggbeamcolormapfloater.h"

#include "llagentdata.h"
#include "llcommandhandler.h"
#include "llfloater.h"
#include "llsdutil.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llagent.h"
#include "llfilepicker.h"
#include "llpanel.h"
#include "lliconctrl.h"
#include "llbutton.h"
#include "llcolorswatch.h"
#include "lggbeammaps.h"


#include "llsdserialize.h"
#include "llpanelphoenix.h"
#include "lggbeamscolors.h"
#include "llsliderctrl.h"
#include "llfocusmgr.h"

F32 convertXToHue(S32 place)
{return ((place-6)/396.0f)*720.0f;}
S32 convertHueToX(F32 place)
{return llround((place/720.0f)*396.0f)+6;}
F32 hueToRgb1 ( F32 val1In, F32 val2In, F32 valHUeIn )
{
	if ( valHUeIn < 0.0f ) valHUeIn += 1.0f;
	if ( valHUeIn > 1.0f ) valHUeIn -= 1.0f;
	if ( ( 6.0f * valHUeIn ) < 1.0f ) return ( val1In + ( val2In - val1In ) * 6.0f * valHUeIn );
	if ( ( 2.0f * valHUeIn ) < 1.0f ) return ( val2In );
	if ( ( 3.0f * valHUeIn ) < 2.0f ) return ( val1In + ( val2In - val1In ) * ( ( 2.0f / 3.0f ) - valHUeIn ) * 6.0f );
	return ( val1In );
}

void hslToRgb1 ( F32 hValIn, F32 sValIn, F32 lValIn, F32& rValOut, F32& gValOut, F32& bValOut )
{
	if ( sValIn < 0.00001f )
	{
		rValOut = lValIn;
		gValOut = lValIn;
		bValOut = lValIn;
	}
	else
	{
		F32 interVal1;
		F32 interVal2;

		if ( lValIn < 0.5f )
			interVal2 = lValIn * ( 1.0f + sValIn );
		else
			interVal2 = ( lValIn + sValIn ) - ( sValIn * lValIn );

		interVal1 = 2.0f * lValIn - interVal2;

		rValOut = hueToRgb1 ( interVal1, interVal2, hValIn + ( 1.f / 3.f ) );
		gValOut = hueToRgb1 ( interVal1, interVal2, hValIn );
		bValOut = hueToRgb1 ( interVal1, interVal2, hValIn - ( 1.f / 3.f ) );
	}
}


class lggBeamColorMapFloater;
const F32 CONTEXT_CONE_IN_ALPHA = 0.0f;
const F32 CONTEXT_CONE_OUT_ALPHA = 1.f;
const F32 CONTEXT_FADE_TIME = 0.08f;

////////////////////////////////////////////////////////////////////////////
// lggBeamMapFloater
class lggBeamColorMapFloater : public LLFloater, public LLFloaterSingleton<lggBeamColorMapFloater>
{
public:
	lggBeamColorMapFloater(const LLSD& seed);
	virtual ~lggBeamColorMapFloater();
	
	void fixOrder();

	BOOL postBuild(void);
	BOOL handleMouseDown(S32 x,S32 y,MASK mask);
	BOOL handleRightMouseDown(S32 x,S32 y,MASK mask);
	void update();
	
	void setData(void* data);

	void draw();
	
	LLSD getMyDataSerialized();
	
	// UI Handlers
	static void onClickSave(void* data);
	static void onClickCancel(void* data);
	static void onClickLoad(void* data);

	static void onClickSlider(LLUICtrl* crtl, void* userdata);


	
	
protected:
	F32 mContextConeOpacity;
	LLPanelPhoenix * empanel;
	lggBeamsColors myData; 
	LLSliderCtrl* mColorSlider;

};
void lggBeamColorMapFloater::onClickSlider(LLUICtrl* crtl, void* userdata)
{
	lggBeamColorMapFloater* self = (lggBeamColorMapFloater*)userdata;
	self->fixOrder();
}
void lggBeamColorMapFloater::draw()
{
	//set the color of the preview thing
	LLColorSwatchCtrl* colorctrl = getChild<LLColorSwatchCtrl>("BeamColor_Preview");
	LLColor4 bColor = LLColor4(lggBeamMaps::beamColorFromData(myData));
	colorctrl->set(bColor,TRUE);
	
	//Try draw rectangle attach beam
	LLRect swatch_rect;
	LLButton* createButton = empanel->getChild<LLButton>("BeamColor_new");
	
	createButton->localRectToOtherView(createButton->getLocalRect(), &swatch_rect, this);
	LLRect local_rect = getLocalRect();
	if (gFocusMgr.childHasKeyboardFocus(this) && empanel->isInVisibleChain() && mContextConeOpacity > 0.001f)
	{
		gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
		LLGLEnable(GL_CULL_FACE);
		gGL.begin(LLRender::QUADS);
		{
			F32 r = bColor.mV[0];
			F32 g = bColor.mV[1];
			F32 b = bColor.mV[2];

			gGL.color4f(r, g, b, CONTEXT_CONE_IN_ALPHA * mContextConeOpacity);
			gGL.vertex2i(swatch_rect.mLeft, swatch_rect.mTop);
			gGL.vertex2i(swatch_rect.mRight, swatch_rect.mTop);
			gGL.color4f(r, g, b, CONTEXT_CONE_OUT_ALPHA * mContextConeOpacity);
			gGL.vertex2i(local_rect.mRight, local_rect.mTop);
			gGL.vertex2i(local_rect.mLeft, local_rect.mTop);

			gGL.color4f(r, g, b, CONTEXT_CONE_OUT_ALPHA * mContextConeOpacity);
			gGL.vertex2i(local_rect.mLeft, local_rect.mTop);
			gGL.vertex2i(local_rect.mLeft, local_rect.mBottom);
			gGL.color4f(r, g, b, CONTEXT_CONE_IN_ALPHA * mContextConeOpacity);
			gGL.vertex2i(swatch_rect.mLeft, swatch_rect.mBottom);
			gGL.vertex2i(swatch_rect.mLeft, swatch_rect.mTop);

			gGL.color4f(r, g, b, CONTEXT_CONE_OUT_ALPHA * mContextConeOpacity);
			gGL.vertex2i(local_rect.mRight, local_rect.mBottom);
			gGL.vertex2i(local_rect.mRight, local_rect.mTop);
			gGL.color4f(r, g, b, CONTEXT_CONE_IN_ALPHA * mContextConeOpacity);
			gGL.vertex2i(swatch_rect.mRight, swatch_rect.mTop);
			gGL.vertex2i(swatch_rect.mRight, swatch_rect.mBottom);

			gGL.color4f(r, g, b, CONTEXT_CONE_OUT_ALPHA * mContextConeOpacity);
			gGL.vertex2i(local_rect.mLeft, local_rect.mBottom);
			gGL.vertex2i(local_rect.mRight, local_rect.mBottom);
			gGL.color4f(r, g, b, CONTEXT_CONE_IN_ALPHA * mContextConeOpacity);
			gGL.vertex2i(swatch_rect.mRight, swatch_rect.mBottom);
			gGL.vertex2i(swatch_rect.mLeft, swatch_rect.mBottom);
		}
		gGL.end();
	}

	static F32* opacity = rebind_llcontrol<F32>("PickerContextOpacity", &gSavedSettings, true);

	mContextConeOpacity = lerp(mContextConeOpacity, *opacity, LLCriticalDamp::getInterpolant(CONTEXT_FADE_TIME));


	//Draw Base Stuff
	LLFloater::draw();
	
	//Draw hues and pointers at color
	gGL.pushMatrix();
	F32 r, g, b;
	LLColor4 output;
	for (int i  = 0;i <= 720;i++)
	{
		int hi =i%360;
		hslToRgb1((hi/360.0f), 1.0f, 0.5f, r, g, b);
		output.set(r, g, b);

		gl_line_2d(
			convertHueToX(i),201,
			convertHueToX(i),277,output);

	}
	S32 X1 = convertHueToX(myData.startHue);
	S32 X2 = convertHueToX(myData.endHue);
	LLFontGL* font = LLFontGL::getFontSansSerifSmall();

	gGL.color4fv(LLColor4::white.mV);
	gl_circle_2d(X1,238,9.0f,(S32)30,false);

	gGL.color4fv(LLColor4::black.mV);
	gl_circle_2d(X1,238,8.0f,(S32)30,false);

	gGL.color4fv(LLColor4::white.mV);
	gl_circle_2d(X1,238,7.0f,(S32)30,false);

	gl_line_2d(X1+1,210,X1+1,266,LLColor4::white);
	gl_line_2d(X1-1,210,X1-1,266,LLColor4::white);
	gl_line_2d(X1,210,X1,266,LLColor4::black);

	gl_line_2d(X1-25,238+1,X1+25,238+1,LLColor4::white);
	gl_line_2d(X1-25,238-1,X1+25,238-1,LLColor4::white);
	gl_line_2d(X1-25,238,X1+25,238,LLColor4::black);

	font->renderUTF8(
		"Start Hue", 0,
		X1, 
		212,
		LLColor4::white, LLFontGL::HCENTER,
		LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);


	gGL.color4fv(LLColor4::white.mV);
	gl_circle_2d(X2,238,9.0f,(S32)30,false);

	gGL.color4fv(LLColor4::black.mV);
	gl_circle_2d(X2,238,8.0f,(S32)30,false);

	gGL.color4fv(LLColor4::white.mV);
	gl_circle_2d(X2,238,7.0f,(S32)30,false);

	gl_line_2d(X2+1,210,X2+1,266,LLColor4::white);
	gl_line_2d(X2-1,210,X2-1,266,LLColor4::white);
	gl_line_2d(X2,210,X2,266,LLColor4::black);

	gl_line_2d(X2-25,238+1,X2+25,238+1,LLColor4::white);
	gl_line_2d(X2-25,238-1,X2+25,238-1,LLColor4::white);
	gl_line_2d(X2-25,238,X2+25,238,LLColor4::black);

	font->renderUTF8(
		"End Hue", 0,
		X2, 
		212,
		LLColor4::white, LLFontGL::HCENTER,
		LLFontGL::BASELINE, LLFontGL::DROP_SHADOW);
	
	gGL.popMatrix();
	
}

lggBeamColorMapFloater::~lggBeamColorMapFloater()
{
	//colorUpdater->shutdown();
	//delete colorUpdater;
	//if(mCallback) mCallback->detach();
}

lggBeamColorMapFloater::lggBeamColorMapFloater(const LLSD& seed):mContextConeOpacity(0.0f)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_beamcolor.xml");
	
	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}

}

BOOL lggBeamColorMapFloater::postBuild(void)
{
	setCanMinimize(false);
	childSetAction("BeamColor_Save",onClickSave,this);
	childSetAction("BeamColor_Load",onClickLoad,this);
	childSetAction("BeamColor_Cancel",onClickCancel,this);
	mColorSlider = getChild<LLSliderCtrl>("BeamColor_Speed",TRUE,FALSE);
	mColorSlider->setCommitCallback(&lggBeamColorMapFloater::onClickSlider);
	mColorSlider->setCallbackUserData(this);

	fixOrder();

	//colorUpdater = new lggBeamColorUpdater(myData,this);
	//colorUpdater->start();
	
	return true;
}
BOOL lggBeamColorMapFloater::handleMouseDown(S32 x,S32 y,MASK mask)
{
	//6, 277
		//402 201
	
	if(y>201 &&  y<277)
	{
		if(x<6)
		{
			myData.startHue=0.0f;
		}else if( x>402)
		{
			myData.endHue=720.0f;
		}else
			myData.startHue  = convertXToHue(x);
		
	
		fixOrder();
	}
	
	llinfos << "we got clicked at (" << x << ", " << y << " yay! " << llendl;

	
	return LLFloater::handleMouseDown(x,y,mask);
}
BOOL lggBeamColorMapFloater::handleRightMouseDown(S32 x,S32 y,MASK mask)
{
	if(y>201 &&  y<277)
	{
		if(x<6)
		{
			myData.startHue=0.0f;
		}else if (x>402)
		{
			myData.endHue=720.0f;
		}else
			myData.endHue  = convertXToHue(x);
		fixOrder();
	}
	llinfos << "we got right clicked at (" << x << ", " << y << " yay! " << llendl;


	return LLFloater::handleRightMouseDown(x,y,mask);
}

void lggBeamColorMapFloater::fixOrder()
{
	myData.rotateSpeed = mColorSlider->getValueF32();
	myData.rotateSpeed /= 100.0f;
	
	if(myData.endHue < myData.startHue)
	{
		F32 temp = myData.startHue;
		myData.startHue = myData.endHue;
		myData.endHue = temp;
	}
}


void lggBeamColorMapFloater::setData(void* data)
{
	empanel = (LLPanelPhoenix*)data;
	if(empanel)
	{
		gFloaterView->getParentFloater(empanel)->addDependentFloater(this);
	}

	

}
void lggBeamColorMapFloater::update()
{
	
}
LLSD lggBeamColorMapFloater::getMyDataSerialized()
{
 	
 	return myData.toLLSD();
}
void lggBeamColorMapFloater::onClickSave(void* data)
{
	
	lggBeamColorMapFloater* self = (lggBeamColorMapFloater*)data;
	LLFilePicker& picker = LLFilePicker::instance();
	
	std::string path_name2(gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "beamsColors", ""));
				
	std::string filename=path_name2 + "myNewBeamColor.xml";
	if(!picker.getSaveFile( LLFilePicker::FFSAVE_BEAM, filename ) )
	{return;
	}	
	
	filename = path_name2 +gDirUtilp->getBaseFileName(picker.getFirstFile());

	
	LLSD main= self->getMyDataSerialized();
  
	llofstream export_file;
	export_file.open(filename);
	LLSDSerialize::toPrettyXML(main, export_file);
	export_file.close();

	gSavedSettings.setString("PhoenixBeamColorFile",gDirUtilp->getBaseFileName(filename,true));

	if(self->empanel != NULL)
	{
		self->empanel->refresh();
	}
	self->close();
}

void lggBeamColorMapFloater::onClickCancel(void* data)
{
	lggBeamColorMapFloater* self = (lggBeamColorMapFloater*)data;
	
	
	self->close();
	
}
void lggBeamColorMapFloater::onClickLoad(void* data)
{
	
	lggBeamColorMapFloater* self = (lggBeamColorMapFloater*)data;
	
	LLFilePicker& picker = LLFilePicker::instance();
	if(!picker.getOpenFile( LLFilePicker::FFLOAD_XML ) )
	{
		return; 
	}
	LLSD minedata;
	llifstream importer(picker.getFirstFile());
	LLSDSerialize::fromXMLDocument(minedata, importer);
	
	self->myData = lggBeamsColors::fromLLSD(minedata);
	self->childSetValue("BeamColor_Speed",self->myData.rotateSpeed*100);

	
}

void LggBeamColorMap::show(BOOL showin, void * data)
{
	if(showin)
	{
		lggBeamColorMapFloater* beam_floater = lggBeamColorMapFloater::showInstance();
		beam_floater->setData(data);

	}
}
