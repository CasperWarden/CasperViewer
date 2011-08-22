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

////////////////////////////////////////////////////
//////////////DATA TYPE/////////////////////////////

#include "llviewerprecompiledheaders.h"
#include "lggbeamscolors.h"

lggBeamsColors lggBeamsColors::fromLLSD(LLSD inputData)
{

	lggBeamsColors toReturn;
	
	if(inputData.has("startHue")) toReturn.startHue = (F32)inputData["startHue"].asReal();
	if(inputData.has("endHue")) toReturn.endHue = (F32)inputData["endHue"].asReal();
	if(inputData.has("rotateSpeed")) toReturn.rotateSpeed = (F32)inputData["rotateSpeed"].asReal();

	return toReturn;
}
LLSD lggBeamsColors::toLLSD()
{

	LLSD out;
	out["startHue"]=startHue;
	out["endHue"]=endHue;
	out["rotateSpeed"]=rotateSpeed;
	return out;
}

std::string lggBeamsColors::toString()
{

	return llformat("Start Hue %d\nEnd Hue is %d\nRotate Speed is %d",
		startHue,endHue,rotateSpeed
		
		);
}
lggBeamsColors::lggBeamsColors(F32 istartHue, F32 iendHue, F32 irotateSpeed):
startHue(istartHue),endHue(iendHue),rotateSpeed(irotateSpeed)
{
}
lggBeamsColors::lggBeamsColors():
startHue(0.0f),endHue(360.0f),rotateSpeed(1.0f)
{
}
lggBeamsColors::~lggBeamsColors()
{

}
