

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
cd ../waza_check/
call update_hash.bat
cd ../trdata_check/
echo 全国ポケモン所持トレーナー検索中
ruby -Ks -I ./../hash;./../waza_check/script/hash;./../waza_check/script/hash_conv;./../waza_check/script/personal;./../waza_check/script/trainer;./../../resource/fldmapdata/eventdata/check_tool zenkoku_tr_check.rb
echo 検索完了
@echo on

:end
Pause


