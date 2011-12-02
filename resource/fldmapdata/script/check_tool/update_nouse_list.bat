
:: =====================================================
:: prompt.batから自らを起動する処理
:: =====================================================
@echo off
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: 実際の処理
:: =====================================================
:EX
ruby ../../../../tools/exceltool/xls2xml/tab_out_direct_sheet.rb 未使用メッセージ解析データ.xls -n 0 -s > ./message_list/nosearch_gmm.csv

:end
@echo on
