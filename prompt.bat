@ECHO OFF

SET LANG=C-SJIS
SET HOME_TMP=%~dp0
SET PROJECT_ROOT=%HOME_TMP:\=/%
SET PROJECT_PROGDIR=%PROJECT_ROOT%prog/
SET PROJECT_ARCDIR=%PROJECT_PROGDIR%arc/
SET PROJECT_RSCDIR=%PROJECT_ROOT%resource/
SET WBLIBDIR=C:\home\pokemon_wb\lib
SET TAGFILE=c:\home\pokemon_wb\tags

REM SET PATH=%CYGNUSDIR%/native-99r1/H-i686-cygwin32/bin;%AGBDIR%/bin;%PATH%;.
REM SET PATH=%CYGNUSDIR%/thumbelf-000512/H-i686-cygwin32/bin;%PATH%
REM SET PATH=%AGBDIR%/bin;%PATH%

REM ##################################################
REM		ディレクトリパス定義
REM ##################################################
SET WBLIBDIR=%PROJECT_ROOT%lib
SET CWFolder_NITRO=C:\Program Files\Freescale\CW for NINTENDO DSi V1.2\
SET CWFOLDER_TWL=C:\Program Files\Freescale\CW for NINTENDO DSi V1.2\
SET CW_NITROSDK_ROOT=%WBLIBDIR%\TwlSDK
SET CW_TWLSDK_ROOT=%WBLIBDIR%\TwlSDK
SET LM_LICENSE_FILE=C:\Program Files\Freescale\CW for NINTENDO DSi V1.2\license.dat
SET NITROSDK_ROOT=%WBLIBDIR%\TwlSDK
SET NITROSYSTEM_ROOT=%WBLIBDIR%\TwlSystem
SET NITROWIFI_ROOT=%WBLIBDIR%\TwlWiFi
SET NITRODWC_ROOT=%WBLIBDIR%\TwlDWC

REM ####################################################
REM ハイブリッド対応の場合下記の部分を有効にしてください ハイブリッドでない時は無効にしてください
REM NITROとかくとIS-NITRO用 TWLとかくとTWL用 両方書くと両方が出来上がります
REM ####################################################
REM SET TWLSDK_PLATFORM=TWL NITRO
REM SET TARGET_PLATFORM=TWL NITRO

REM ####################################################
REM ハイブリッドでない場合下記の部分を有効にしてください ハイブリッドの時は無効にしてください
REM ####################################################
SET TWLSDK_PLATFORM=NITRO
SET TARGET_PLATFORM=NITRO

REM ####################################################
REM SDカードにデバッグログを出力する機能をONにします。ハイブリッドでDSiのみ動きます
REM ####################################################
REM SET PM_DEBUG_SD_PRINT=yes


SET TWLSYSTEM_ROOT=%WBLIBDIR%\TwlSystem
SET TWLSDK_ROOT=%WBLIBDIR%\TwlSDK
SET TWLWIFI_ROOT=%WBLIBDIR%\TwlWiFi
SET TWLDWC_ROOT=%WBLIBDIR%\TwlDWC

SET TWLVCT_ROOT=%WBLIBDIR%\libVCT
SET TWLLIBVCT_ROOT=%WBLIBDIR%\libVCT
SET TWLCRYPTO_ROOT=%WBLIBDIR%\NitroCrypto
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
PATH=c:\tools;%PROJECT_ROOT%;%PROJECT_ROOT%\tools;c:\tools\subversion\bin;c:\cygwin\bin;c:\tools\vim;C:\tools;%NITROSDK_ROOT%\tools\bin;%NITROSYSTEM_ROOT%\tools\bin;%PATH%;

REM ##################################################
REM		マクロ定義
REM ##################################################
doskey alias=doskey /macros:all
doskey his=doskey /history
doskey vi=c:\tools\vim\gvim $*
doskey he=c:\progra~1\hidemaru\hidemaru $*
doskey gort = cd %PROJECT_ROOT:/=\%
doskey gopr = cd %PROJECT_PROGDIR:/=\%
doskey gors = cd %PROJECT_RSCDIR:/=\%
doskey ms = make ./bin/ARM9-TS/Release/main.srl

REM ##################################################
REM		ユーザー個別設定の呼び出し
REM ##################################################
if exist "tools\personal\%USERNAME%_prompt.bat" call tools\personal\%USERNAME%_prompt.bat

REM 別のバッチファイルから呼び出して
REM 環境変数やパス指定だけを有効にして
REM 元のバッチファイルに戻れるようにcmd /F:ONをスルーできるようにしてます
REM prompt.bat cmd hoge.batとすると環境変数とパス指定をWB環境にした上で、
REM hoge.batを実行します
if %1.==. GOTO START
if %1==cmd ( call %2 & GOTO END )
:START
cmd /F:ON
:END

