REM ##################################################
REM		スクリプト班共有設定の呼び出し
REM ##################################################

REM ##################################################
REM ツールパス系
doskey mks=make -j 3
doskey sa=c:\progra~1\sakura\sakura.exe $*

REM ##################################################
REM ディレクトリ移動ショートカット系
doskey ch=cd %PROJECT_ROOT:/=\%$*
doskey che=cd %PROJECT_RSCDIR:/=\%fldmapdata\$*
doskey gose=cd %PROJECT_RSCDIR:/=\%fldmapdata\script\$*
doskey goev=cd %PROJECT_RSCDIR:/=\%fldmapdata\eventdata\$*

