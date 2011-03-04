REM ##################################################
REM
REM		尾上の個人用設定（prompt.batから呼び出される）
REM
REM ##################################################
PATH=C:\Program Files\SlikSvn\bin;%PATH%
doskey chs=cd %PROJECT_ROOT:/=\%prog\src\$*
doskey chr=cd %PROJECT_ROOT:/=\%resource\$*

cmd /TAB:ON
