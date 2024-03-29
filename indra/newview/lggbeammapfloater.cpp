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

#include "lggbeammapfloater.h"

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
#include "llfocusmgr.h"
class lggPoint;
class lggBeamMapFloater;

const F32 CONTEXT_CONE_IN_ALPHA = 0.0f;
const F32 CONTEXT_CONE_OUT_ALPHA = 1.f;
const F32 CONTEXT_FADE_TIME = 0.08f;


////////////////////////////////////////////////////////////////////////////
// lggBeamMapFloater
class lggBeamMapFloater : public LLFloater, public LLFloaterSingleton<lggBeamMapFloater>
{
public:
	lggBeamMapFloater(const LLSD& seed);
	virtual ~lggBeamMapFloater();
	

	BOOL postBuild(void);
	BOOL handleMouseDown(S32 x,S32 y,MASK mask);
	void update();
	BOOL handleRightMouseDown(S32 x,S32 y,MASK mask);
	
	void setData(void* data);
	LLPanelPhoenix * empanel;

	void draw();
	void clearPoints();

	LLSD getMyDataSerialized();
	
	std::vector<lggPoint> dots;

	// UI Handlers
	static void onClickSave(void* data);
	static void onClickClear(void* data);
	static void onClickLoad(void* data);

	
private:
	static void onBackgroundChange(LLUICtrl* ctrl, void* userdata);

	F32 mContextConeOpacity;
};
class lggPoint
{
	public:
		S32 x;
		S32 y;
		LLColor4 c;
};

void lggBeamMapFloater::clearPoints()
{
	dots.clear();
	
}
void lggBeamMapFloater::draw()
{

	LLRect swatch_rect;
	LLButton* createButton = empanel->getChild<LLButton>("custom_beam_btn");

	createButton->localRectToOtherView(createButton->getLocalRect(), &swatch_rect, this);
	LLRect local_rect = getLocalRect();
	if (gFocusMgr.childHasKeyboardFocus(this) && createButton->isInVisibleChain() && mContextConeOpacity > 0.001f)
	{
		gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
		LLGLEnable(GL_CULL_FACE);
		gGL.begin(LLRender::QUADS);
		{
			gGL.color4f(0.f, 0.f, 0.f, CONTEXT_CONE_IN_ALPHA * mContextConeOpacity);
			gGL.vertex2i(swatch_rect.mLeft, swatch_rect.mTop);
			gGL.vertex2i(swatch_rect.mRight, swatch_rect.mTop);
			gGL.color4f(0.f, 0.f, 0.f, CONTEXT_CONE_OUT_ALPHA * mContextConeOpacity);
			gGL.vertex2i(local_rect.mRight, local_rect.mTop);
			gGL.vertex2i(local_rect.mLeft, local_rect.mTop);

			gGL.color4f(0.f, 0.f, 0.f, CONTEXT_CONE_OUT_ALPHA * mContextConeOpacity);
			gGL.vertex2i(local_rect.mLeft, local_rect.mTop);
			gGL.vertex2i(local_rect.mLeft, local_rect.mBottom);
			gGL.color4f(0.f, 0.f, 0.f, CONTEXT_CONE_IN_ALPHA * mContextConeOpacity);
			gGL.vertex2i(swatch_rect.mLeft, swatch_rect.mBottom);
			gGL.vertex2i(swatch_rect.mLeft, swatch_rect.mTop);

			gGL.color4f(0.f, 0.f, 0.f, CONTEXT_CONE_OUT_ALPHA * mContextConeOpacity);
			gGL.vertex2i(local_rect.mRight, local_rect.mBottom);
			gGL.vertex2i(local_rect.mRight, local_rect.mTop);
			gGL.color4f(0.f, 0.f, 0.f, CONTEXT_CONE_IN_ALPHA * mContextConeOpacity);
			gGL.vertex2i(swatch_rect.mRight, swatch_rect.mTop);
			gGL.vertex2i(swatch_rect.mRight, swatch_rect.mBottom);

			gGL.color4f(0.f, 0.f, 0.f, CONTEXT_CONE_OUT_ALPHA * mContextConeOpacity);
			gGL.vertex2i(local_rect.mLeft, local_rect.mBottom);
			gGL.vertex2i(local_rect.mRight, local_rect.mBottom);
			gGL.color4f(0.f, 0.f, 0.f, CONTEXT_CONE_IN_ALPHA * mContextConeOpacity);
			gGL.vertex2i(swatch_rect.mRight, swatch_rect.mBottom);
			gGL.vertex2i(swatch_rect.mLeft, swatch_rect.mBottom);
		}
		gGL.end();
	}

	static F32* opacity = rebind_llcontrol<F32>("PickerContextOpacity", &gSavedSettings, true);


	mContextConeOpacity = lerp(mContextConeOpacity, *opacity, LLCriticalDamp::getInterpolant(CONTEXT_FADE_TIME));


	//getChild<LLPanel>("beamshape_draw")->setBackgroundColor(getChild<LLColorSwatchCtrl>("back_color_swatch")->get());
	LLFloater::draw();
	LLRect rec  = getChild<LLPanel>("beamshape_draw")->getRect();
	
	gGL.pushMatrix();
	gGL.color4fv(LLColor4::white.mV);
	gl_circle_2d(rec.getCenterX(),rec.getCenterY(),2.0f,(S32)30,false);
	gGL.color4fv(LLColor4::black.mV);
	gl_circle_2d(rec.getCenterX(),rec.getCenterY(),30.0f,(S32)30,false);	
	gGL.color4fv(LLColor4::white.mV);
	gl_circle_2d(rec.getCenterX(),rec.getCenterY(),60.0f,(S32)30,false);	
	gGL.color4fv(LLColor4::black.mV);
	gl_circle_2d(rec.getCenterX(),rec.getCenterY(),90.0f,(S32)30,false);
	gGL.color4fv(LLColor4::white.mV);
	gl_circle_2d(rec.getCenterX(),rec.getCenterY(),120.0f,(S32)30,false);

	for(int i = 0; i < (int)dots.size();i++)
	{	
		gGL.color4fv(LLColor4::white.mV);
		gl_circle_2d(dots[i].x,dots[i].y,9.0f,(S32)30,true);

		gGL.color4fv(LLColor4::black.mV);
		gl_circle_2d(dots[i].x,dots[i].y,8.0f,(S32)30,true);
		
		gGL.color4fv(dots[i].c.mV);
		gl_circle_2d(dots[i].x,dots[i].y,7.0f,(S32)30,true);
		
		
	}
	gGL.popMatrix();
	
}

lggBeamMapFloater::~lggBeamMapFloater()
{
	//if(mCallback) mCallback->detach();
}

lggBeamMapFloater::lggBeamMapFloater(const LLSD& seed):mContextConeOpacity(0.0f)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_beamshape.xml");
	
	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}

}

BOOL lggBeamMapFloater::postBuild(void)
{
	setCanMinimize(false);
	childSetAction("beamshape_save",onClickSave,this);
	childSetAction("beamshape_clear",onClickClear,this);
	childSetAction("beamshape_load",onClickLoad,this);
	getChild<LLColorSwatchCtrl>("back_color_swatch")->setCommitCallback(onBackgroundChange);
	getChild<LLColorSwatchCtrl>("beam_color_swatch")->setColor(LLColor4::red);
	
	
	return true;
}
BOOL lggBeamMapFloater::handleMouseDown(S32 x,S32 y,MASK mask)
{
	if(y>39 && x>16 && x<394 && y<317)
	{
		lggPoint a;
		a.x=x;
		a.y=y;
		a.c= 	getChild<LLColorSwatchCtrl>("beam_color_swatch")->get();
		dots.push_back(a);
	
		llinfos << "we got clicked at (" << x << ", " << y << " and color was " << a.c << llendl;
	}
	
	return LLFloater::handleMouseDown(x,y,mask);
}
void lggBeamMapFloater::setData(void* data)
{
	empanel = (LLPanelPhoenix*)data;
	if(empanel)
	{

		gFloaterView->getParentFloater(empanel)->addDependentFloater(this);
	}

}
BOOL lggBeamMapFloater::handleRightMouseDown(S32 x,S32 y,MASK mask)
{
	std::vector<lggPoint> newDots;
	for(int i = 0; i < (int)dots.size();i++)
	{
		if(dist_vec(LLVector2(x,y),LLVector2(dots[i].x,dots[i].y)) < 7)
		{
			
		}else
		{
			newDots.push_back(dots[i]);
		}
	
	}
	dots = newDots;
		
	return LLFloater::handleMouseDown(x,y,mask);
}
void lggBeamMapFloater::onBackgroundChange(LLUICtrl* ctrl, void* userdata)
{
	lggBeamMapFloater* self = (lggBeamMapFloater*)userdata;
	
	LLColorSwatchCtrl* cctrl = (LLColorSwatchCtrl*)ctrl;
	
	if(cctrl)
	{
		self->getChild<LLPanel>("beamshape_draw")->setBackgroundColor(cctrl->get());
	}
	
}
void lggBeamMapFloater::update()
{
	
}
LLSD lggBeamMapFloater::getMyDataSerialized()
{
	LLSD out;
	LLRect r  = getChild<LLPanel>("beamshape_draw")->getRect();
	for(int i =0; i<(int)dots.size();i++)
	{
		LLSD point;
		lggPoint t = dots[i];
		LLVector3 vec = LLVector3((F32)0.0,(F32)t.x,(F32)t.y);
		vec -= LLVector3((F32)0.0,(F32)r.getCenterX(),r.getCenterY());
		
		point["offset"]= vec.getValue();
		point["color"] = t.c.getValue();

		out[i]=point;
	}
	return out;
}
void lggBeamMapFloater::onClickSave(void* data)
{
	
	lggBeamMapFloater* self = (lggBeamMapFloater*)data;
	LLRect r  = self->getChild<LLPanel>("beamshape_draw")->getRect();
	
	LLFilePicker& picker = LLFilePicker::instance();
	
	std::string path_name2(gDirUtilp->getExpandedFilename( LL_PATH_USER_SETTINGS , "beams", ""));
				
	std::string filename=path_name2 + "myNewBeam.xml";
	if(!picker.getSaveFile( LLFilePicker::FFSAVE_BEAM, filename ) )
	{return;
	}	
	
	filename = path_name2 +gDirUtilp->getBaseFileName(picker.getFirstFile());

	
	LLSD main;
	main["scale"] = 8.0f/(r.getWidth());
	main["data"]=self->getMyDataSerialized();
  
	llofstream export_file;
	export_file.open(filename);
	LLSDSerialize::toPrettyXML(main, export_file);
	export_file.close();
	gSavedSettings.setString("PhoenixBeamShape",gDirUtilp->getBaseFileName(filename,true));

	if(self->empanel)
	{

		self->empanel->refresh();
	}
	
}

void lggBeamMapFloater::onClickClear(void* data)
{
	lggBeamMapFloater* self = (lggBeamMapFloater*)data;
	
	
	self->clearPoints();
	
}
void lggBeamMapFloater::onClickLoad(void* data)
{
	
	lggBeamMapFloater* self = (lggBeamMapFloater*)data;
	
	LLFilePicker& picker = LLFilePicker::instance();
	if(!picker.getOpenFile( LLFilePicker::FFLOAD_XML ) )
	{
		return;
	}
	self->dots.clear();
	LLSD mydata;
	llifstream importer(picker.getFirstFile());
	LLSDSerialize::fromXMLDocument(mydata, importer);
	LLSD myPicture = mydata["data"];	
	F32 scale = (F32)mydata["scale"].asReal();

			
	for(int i = 0; i < myPicture.size(); i++)
	{
		LLRect rec  = self->getChild<LLPanel>("beamshape_draw")->getRect();
	
		
		LLSD beamData = myPicture[i];
		lggPoint p;
		LLVector3 vec =  beamData["offset"];
		vec *= (scale)/(8.0f/(rec.getWidth()));
		LLColor4 color = beamData["color"];
		p.c = color;
		p.x = (S32)(vec.mV[VY]+rec.getCenterX());
		p.y = (S32)(vec.mV[VZ]+rec.getCenterY());
		
						
		self->dots.push_back(p);
	}
	
}

void LggBeamMap::show(BOOL showin, void * data)
{
	//lggBeamMapFloater* beam_floater = 
	if(showin)
	{

	
		lggBeamMapFloater* beam_floater = lggBeamMapFloater::showInstance();
		beam_floater->setData(data);


	}
	//beam_floater->update();
}
