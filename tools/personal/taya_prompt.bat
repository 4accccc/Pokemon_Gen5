REM ##################################################
REM
REM		田谷の個人用設定（prompt.batから呼び出される）
REM
REM ##################################################


doskey gowa = cd %PROJECT_RSCDIR:/=\%waza_tbl
doskey gome = cd %PROJECT_RSCDIR:/=\%message
doskey gofo = cd %PROJECT_RSCDIR:/=\%font
doskey goan = cd %PROJECT_ROOT:/=\%tools\analyze\overlaymap
doskey mk = call %PROJECT_PROGDIR:/=\%mksnd.bat
doskey rd = call %PROJECT_PROGDIR:/=\%rdsnd.bat %1

cd %PROJECT_PROGDIR:/=\%

