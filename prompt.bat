@ECHO OFF

SET LANG=C-SJIS
SET HOME=c:\home
SET PROJECT_ROOT=%HOME%\pokemon_wb

REM SET PATH=%CYGNUSDIR%/native-99r1/H-i686-cygwin32/bin;%AGBDIR%/bin;%PATH%;.
REM SET PATH=%CYGNUSDIR%/thumbelf-000512/H-i686-cygwin32/bin;%PATH%
REM SET PATH=%AGBDIR%/bin;%PATH%
PATH=%PROJECT_ROOT%;c:\cygwin\bin;%PATH%;c:\tools\vim;C:\tools;c:\NitroSDK\tools\bin;c:\NitroSystem\tools\win\bin;

REM ##################################################
REM		ディレクトリパス定義
REM ##################################################
SET CWFolder_NITRO=C:\Program Files\Freescale\CW for NINTENDO DS V2.0
SET CW_NITROSDK_ROOT=C:\NEWNitro\NitroSDK
SET LM_LICENSE_FILE=C:\Program Files\Freescale\CW for NINTENDO DS V2.0\license.dat
SET NITROSYSTEM_ROOT=C:\NitroSystem\
SET NITROWIFI_ROOT=C:\NitroWifi\
SET NITRODWC_ROOT=C:\NitroDwc\
SET NITROVCT_ROOT=C:\libVct\
SET NITROLIBVCT_ROOT=C:\libVCT\


REM ##################################################
REM		マクロ定義
REM ##################################################
doskey alias=doskey /macros:all
doskey his=doskey /history
doskey vi=c:\tools\vim\gvim $*
doskey he=c:\progra~1\hidemaru\hidemaru $*
doskey ms = make ./bin/ARM9-TS/Release/main.srl

cmd /F:ON