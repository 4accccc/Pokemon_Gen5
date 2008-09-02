@echo off

REM SET USERNAME=none

SET HOME_TMP=%~dp0
SET PROJECT_ROOT=%HOME_TMP:\=/%
SET WBLIBDIR=%PROJECT_ROOT%lib
SET PROJECT_PROGDIR=%PROJECT_ROOT%prog/
SET PROJECT_RSCDIR=%PROJECT_ROOT%resource/
SET PROJECT_ARCDIR=%PROJECT_ROOT%prog/arc/

SET CWFOLDER_NITRO=C:\Program Files\Freescale\CW for NINTENDO DS V2.0
SET CW_NITROSDK_ROOT=%WBLIBDIR%\NitroSDK
SET LM_LICENSE_FILE=C:\Program Files\Freescale\CW for NINTENDO DS V2.0\license.dat
SET NITROSDK_ROOT=%WBLIBDIR%\NitroSDK
SET NITROSYSTEM_ROOT=%WBLIBDIR%\NitroSystem
SET NITROWIFI_ROOT=%WBLIBDIR%\NitroWifi
SET NITRODWC_ROOT=%WBLIBDIR%\NitroDwc
SET NITROVCT_ROOT=%WBLIBDIR%\libVCT
SET NITROLIBVCT_ROOT=%WBLIBDIR%\libVCT

SET GFL_MAKE_DIR = %PROJECT_ROOT%../gflib/gflib/
SET GFL_PROJECT_DIR = %PROJECT_ROOT%lib/gflib/

SET NITRO_PLATFORM=TS

SET CVSROOT=:pserver:%USERNAME%@dpcvs.gamefreak.co.jp:/Groups/deocvs
SET CVSEDITOR=c:\tools\vim\gvim

SET SVNROOT=:svn://svn-wb.gamefreak.co.jp/pokemon_wb/
SET SVN_EDITOR=c:\tools\vim\gvim

PATH=C:\tools;c:\cygwin\bin;c:\tools\vim;%WBLIBDIR%\NitroSDK\tools\bin;%WBLIBDIR%\NitroSystem\tools\win\bin;%WBLIBDIR%\..\tools;%PATH%;

REM C:
REM chdir C:\Cygwin\bin


bash --login -i -s

