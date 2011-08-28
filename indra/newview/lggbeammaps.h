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

#include "llhudeffecttrail.h"
#include "llviewerprecompiledheaders.h"
#include "lggbeamscolors.h"
#include "llframetimer.h"
class lggBeamData
{
	public:
		LLVector3d p;
		LLColor4U c;
	
};
class lggBeamMaps
{
	public:
		lggBeamMaps():lastFileName(""),scale(0.0f),duration(0.25f),sPartsNow(FALSE),sBeamLastAt(LLVector3d::zero){}
		~lggBeamMaps() {}
	public:
		F32		setUpAndGetDuration();
		void	fireCurrentBeams(LLPointer<LLHUDEffectSpiral>, LLColor4U rgb);
		void	forceUpdate();
		static LLColor4U beamColorFromData(lggBeamsColors data);
		LLColor4U getCurrentColor(LLColor4U agentColor);
		std::vector<std::string> getFileNames();
		std::vector<std::string> getColorsFileNames();
		void stopBeamChat();
		void updateBeamChat(LLVector3d currentPos);
	private:
		LLSD	getPic(std::string filename); 
		std::string lastFileName;
		std::string lastColorFileName;
		BOOL		sPartsNow;
		LLVector3d sBeamLastAt;
		lggBeamsColors lastColorsData;
		F32 duration;
		F32 scale;
		std::vector<lggBeamData> dots;     
};


extern lggBeamMaps gLggBeamMaps;
