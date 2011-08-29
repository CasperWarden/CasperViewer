# -*- cmake -*-

# these should be moved to their own cmake file
include(Prebuilt)
use_prebuilt_binary(colladadom)
if (LINUX OR DARWIN)
	use_prebuilt_binary(pcre)
endif (LINUX OR DARWIN)
if (LINUX)
	use_prebuilt_binary(libxml)
endif (LINUX)
use_prebuilt_binary(boost)

set(LLPRIMITIVE_INCLUDE_DIRS
    ${LIBS_OPEN_DIR}/llprimitive
    )

if (WINDOWS)
    set(LLPRIMITIVE_LIBRARIES 
        llprimitive
        debug libcollada14dom22-d
        debug libboost_filesystem-d
        debug libboost_system-d
        optimized libcollada14dom22
        optimized libboost_filesystem
        optimized libboost_system
        )
else (WINDOWS)
    set(LLPRIMITIVE_LIBRARIES 
        llprimitive
        collada14dom
        minizip
        xml2
       	pcrecpp
        pcre
		boost_system
        )
endif (WINDOWS)
