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
doskey golib=cd c:\home\gflib\$*

doskey chs=cd %PROJECT_ROOT:/=\%prog\src\$*
doskey che=cd %PROJECT_ROOT:/=\%prog\src\fielddata\$*
