
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
echo バトルサブウェイデータ（ポケモンの技）をチェック中
ruby -Ks -I ./hash;./hash_conv;./personal;./trainer;./eggwaza_check btl_subway_poke_waza_check_main.rb
echo バトルサブウェイデータ（ポケモンの技）をチェック完了
cd ..
@echo on

:end
Pause
