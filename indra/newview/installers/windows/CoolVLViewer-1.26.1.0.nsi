;NSIS installer for the Cool VL Viewer
;
;When making yourself, change "TMP_DIR" to correct location

SetCompress auto
SetCompressor /solid lzma
RequestExecutionLevel admin

!define PROD_NAME "CoolVLViewer"
!define VER_MAJOR 1
!define VER_MINOR 26
!define VER_MICRO 1
!define VER_REVISION 0
!define VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_MICRO}.${VER_REVISION}"
!define TMP_DIR "C:\TMP"
!define BUILD_DIR "${TMP_DIR}\${PROD_NAME}"
;-------------------------------
;Use modern GUI
;-------------------------------
!include "MUI.nsh"

;---------
;Variables
;---------
Var MUI_TEMP
Var STARTMENU_FOLDER

;-------------------------
; General Config
;-------------------------
XPStyle on
Name "${PROD_NAME}"
Caption "${PROD_NAME} ${VERSION} - Setup"
OutFile "${TMP_DIR}\${PROD_NAME}-${VERSION}.exe"
InstallDir "$PROGRAMFILES\Snowglobe"
;LicenseData "${BUILD_DIR}\COPYING"

;------------------
;Interface Settings
;------------------

;Define custom look and images to be used
!define MUI_ABORTWARNING
;!define MUI_LANGDLL_ALWAYSSHOW
;!define MUI_ICON "install.ico"
;!define MUI_UNICON "uninstall.ico"
;!define MUI_HEADERIMAGE_BITMAP "header.bmp"
;!define MUI_HEADERIMAGE_UNBITMAP "header.bmp"
;!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
;!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

;-----------------------------
;Pages to be used in the setup
;-----------------------------

;General pages
!insertmacro MUI_PAGE_WELCOME
;!insertmacro MUI_PAGE_LICENSE "${BUILD_DIR}\COPYING"
;!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

;Start Menu Folder Page Configuration
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

;Installation progress and finish
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

;---------
;Languages
;---------

;Define installer languages
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Section

Section "Install"
  SetOutPath "$INSTDIR"
  File /r "${BUILD_DIR}\*"

  ; Delete some conflicting files
  Delete "$INSTDIR\SnowglobeRelease.exe.config"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
   ;Create Start menu shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${PROD_NAME}${VER_MAJOR}.${VER_MINOR}.${VER_MICRO}.lnk" "$OUTDIR\SnowglobeRelease.exe" "" ""
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd
