
:: =====================================================
:: prompt.batから自らを起動する処理
:: =====================================================
@echo off
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: 実際の処理
:: =====================================================
:EX
@echo off
call update_hash.bat
cd ../waza_check/script
ruby -Ks -I ./hash level_waza_check.rb
echo チェック完了
cd ..
@echo on

:end
Pause
