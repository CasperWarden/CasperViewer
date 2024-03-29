# -*- cmake -*-

project(llprimitive)

include(00-Common)
include(LLCommon)
include(LLMath)
include(LLMessage)
include(LLXML)

include_directories(
    ${LLCOMMON_INCLUDE_DIRS}
    ${LLMATH_INCLUDE_DIRS}
    ${LLMESSAGE_INCLUDE_DIRS}
    ${LLXML_INCLUDE_DIRS}
    ${LIBS_PREBUILT_DIR}/include/collada
    ${LIBS_PREBUILT_DIR}/include/collada/1.4
    )

set(llprimitive_SOURCE_FILES
    llmaterialtable.cpp
    llmodel.cpp
    llprimitive.cpp
    lltextureanim.cpp
    lltextureentry.cpp
    lltreeparams.cpp
    llvolumemessage.cpp
    llvolumexml.cpp
    )

set(llprimitive_HEADER_FILES
    CMakeLists.txt

    legacy_object_types.h
    llmaterialtable.h
    llmodel.h
    llprimitive.h
    lltextureanim.h
    lltextureentry.h
    lltreeparams.h
    lltree_common.h
    llvolumemessage.h
    llvolumexml.h
    material_codes.h
    object_flags.h
    )
        
set_source_files_properties(${llprimitive_HEADER_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND llprimitive_SOURCE_FILES ${llprimitive_HEADER_FILES})

add_library (llprimitive ${llprimitive_SOURCE_FILES})
add_dependencies(llprimitive prepare)
