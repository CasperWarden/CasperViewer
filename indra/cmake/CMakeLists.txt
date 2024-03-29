# -*- cmake -*-

include(00-Common)

project(cmake)

set(cmake_SOURCE_FILES
    CMakeLists.txt

    00-Common.cmake
    APR.cmake
    Audio.cmake
    BerkeleyDB.cmake
    Boost.cmake
    BuildVersion.cmake
    CARes.cmake
    CURL.cmake
    CMakeCopyIfDifferent.cmake
    CopyWinLibs.cmake
    CSharpMacros.cmake
    DBusGlib.cmake
    DirectX.cmake
    DownloadPrebuilt.cmake.in
    ELFIO.cmake
    EXPAT.cmake
    FindAPR.cmake
    FindBerkeleyDB.cmake
    FindCARes.cmake
    FindELFIO.cmake
    FindFMOD.cmake
    FindGooglePerfTools.cmake
    FindHunSpell.cmake
    FindMono.cmake
    FindMT.cmake
    FindMySQL.cmake
    FindNDOF.cmake
    FindOpenJPEG.cmake
    FindXmlRpcEpi.cmake
    FindZLIB.cmake
    FMOD.cmake
    FreeType.cmake
    GLOD.cmake
    GStreamer010Plugin.cmake
    GooglePerfTools.cmake
    Growl.cmake
    HUNSPELL.cmake
    JPEG.cmake
    LLAddBuildTest.cmake
    LLAudio.cmake
    LLCharacter.cmake
    LLCommon.cmake
    LLConvexDecomposition.cmake
    LLCrashLogger.cmake
    LLDatabase.cmake
    LLImage.cmake
    LLImageJ2COJ.cmake
    LLInventory.cmake
    LLKDU.cmake
    LLMath.cmake
    LLMessage.cmake
    LLPlugin.cmake
    LLPrimitive.cmake
    LLRender.cmake
    LLScene.cmake
    LLUI.cmake
    LLVFS.cmake
    LLWindow.cmake
    LLXML.cmake
    LScript.cmake
    Linking.cmake
    MonoEmbed.cmake
    MySQL.cmake
    NDOF.cmake
    OPENAL.cmake
    OpenGL.cmake
    OpenJPEG.cmake
    OpenSSL.cmake
    PNG.cmake
    Python.cmake
    Prebuilt.cmake
    RunBuildTest.cmake
    TemplateCheck.cmake
    Tut.cmake
    UI.cmake
    UnixInstall.cmake
    Variables.cmake
    Versions.cmake
    XmlRpcEpi.cmake
    ZLIB.cmake
    )

source_group("Shared Rules" FILES ${cmake_SOURCE_FILES})

set(master_SOURCE_FILES
    ../CMakeLists.txt
    ../develop.py
    )

if (SERVER)
  list(APPEND master_SOURCE_FILES ../Server.cmake)
endif (SERVER)

source_group("Master Rules" FILES ${master_SOURCE_FILES})

set_source_files_properties(${cmake_SOURCE_FILES} ${master_SOURCE_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

add_library(cmake
            cmake_dummy.cpp
            ${cmake_SOURCE_FILES}
            ${master_SOURCE_FILES}
            )
