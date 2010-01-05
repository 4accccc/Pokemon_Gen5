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
doskey che=cd %PROJECT_ROOT:/=\%prog\src\fielddata\$*
doskey chr=cd %PROJECT_ROOT:/=\%resource\$*
doskey chf=cd %PROJECT_ROOT:/=\%resource\fldmapdata\$*
