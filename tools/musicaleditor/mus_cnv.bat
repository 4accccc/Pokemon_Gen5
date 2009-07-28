@ECHO OFF

SET LANG=C-SJIS
SET HOME_TMP=%~dp0
SET PROJECT_ROOT=c:\home\pokemon_wb\
SET PROJECT_PROGDIR=%PROJECT_ROOT%prog/
SET PROJECT_ARCDIR=%PROJECT_PROGDIR%arc/
SET PROJECT_RSCDIR=%PROJECT_ROOT%resource/
SET WBLIBDIR=C:\home\pokemon_wb\lib
SET TAGFILE=c:\home\pokemon_wb\tags

REM ##################################################
REM		ディレクトリパス定義
REM ##################################################
SET WBLIBDIR=%PROJECT_ROOT%lib
SET CWFolder_NITRO=C:\Program Files\Freescale\CW for NINTENDO DS V2.0
SET CW_NITROSDK_ROOT=%WBLIBDIR%\NitroSDK
SET LM_LICENSE_FILE=C:\Program Files\Freescale\CW for NINTENDO DS V2.0\license.dat
SET NITROSDK_ROOT=%WBLIBDIR%\NitroSDK
SET NITROSYSTEM_ROOT=%WBLIBDIR%\NitroSystem
SET NITROWIFI_ROOT=%WBLIBDIR%\NitroWifi
SET NITRODWC_ROOT=%WBLIBDIR%\NitroDWC
SET NITROVCT_ROOT=%WBLIBDIR%\libVCT
SET NITROLIBVCT_ROOT=%WBLIBDIR%\libVCT
SET NITROCRYPTO_ROOT=%WBLIBDIR%\NitroCrypto
SET GFL_MAKE_DIR=C:/home/gflib/gflib/
SET GFL_PROJECT_DIR=%PROJECT_ROOT%lib/gflib/
SET BMDS_SDK_ROOT=%PROJECT_ROOT%lib/BMDS_SDK/

SET SVN_EDITOR=c:\tools\vim\gvim

REM ##################################################
REM		パスの追加
REM ##################################################
PATH=%PROJECT_ROOT%;C:\tools;c:\cygwin\bin;c:\tools\vim;%NITROSDK_ROOT%\tools\bin;%NITROSYSTEM_ROOT%\tools\win\bin;%PATH%;

REM ##################################################
REM		ユーザー個別設定の呼び出し
REM ##################################################

@ECHO ON

cd C:\home\pokemon_wb\tools\musicaleditor
del we_000.bin
ruby c:\home\pokemon_wb\resource\musical\script\eescmk.rb c:\home\pokemon_wb\prog\src\musical\stage\script\musical_script_command.h mus_test.esf c:/home/pokemon_wb/ mus_001_ on
ruby c:\home\pokemon_wb\resource\musical\script\eescmk.rb c:\home\pokemon_wb\prog\src\musical\stage\script\musical_script_command.h july_script.esf c:/home/pokemon_wb/ mus_002_ on

if not %ERRORLEVEL%==0 GOTO ERROR

make

if %ERRORLEVEL%==0 GOTO END

:ERROR
echo エラーが発生しました
pause
exit

:END
copy *.bin ..\..\resource\musical\script\
copy *.s ..\..\resource\musical\script\

pause