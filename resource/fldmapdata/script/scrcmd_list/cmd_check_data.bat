
:: =====================================================
:: prompt.bat‚©‚çŽ©‚ç‚ð‹N“®‚·‚éˆ—
:: =====================================================
@echo off
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: ŽÀÛ‚Ìˆ—
:: =====================================================
:EX
@echo on
cp ../../../../prog/src/field/scrcmd_table.cdat ./scrcmd_table.cdat
cp ../scrcmd_list.xls ./scrcmd_list.xls 
ruby ../../../../tools/exceltool/xls2xml/tab_out.rb ./scrcmd_list.xls > ./scrcmd_list.txt
ruby scrcmd_list.rb

:end
Pause
@echo on
