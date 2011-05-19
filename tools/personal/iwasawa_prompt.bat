REM ##################################################
REM		岩澤用設定の呼び出し
REM ##################################################

REM ##################################################
REM		スクリプト班設定の呼び出し
REM ##################################################
if exist "tools\personal\script_prompt.bat" call tools\personal\script_prompt.bat

REM ##################################################
REM ツールパス系
doskey vi=c:\tools\vi\gvim.exe $*
doskey golib=cd c:\home\gflib\gflib\src\$*
doskey godat=cd c:\home\wb_data3d$*

doskey chp=cd %PROJECT_ROOT:/=\%prog\$*
doskey chs=cd %PROJECT_ROOT:/=\%prog\src\$*
doskey chi=cd %PROJECT_ROOT:/=\%prog\include\$*
doskey chf=cd %PROJECT_ROOT:/=\%prog\src\field\$*
doskey chfc=cd %PROJECT_ROOT:/=\%prog\src\field\field_comm\$*
doskey chr=cd %PROJECT_ROOT:/=\%resource\$*
doskey che=cd %PROJECT_ROOT:/=\%resource\fldmapdata\$*
doskey cha=cd %PROJECT_ROOT:/=\%prog\src\app\$*
doskey chg=cd %PROJECT_ROOT:/=\%prog\src\gamesystem\$*
doskey chgi=cd %PROJECT_ROOT:/=\%prog\include\gamesystem\$*

doskey mcs=%PROJECT_ROOT:/=\%lib\TwlSystem\tools\mcsserver\mcsserv.exe
doskey mcspv=%PROJECT_ROOT:/=\%tools\pmviewer\pmViewer2_twl.exe

cmd /TAB:ON
