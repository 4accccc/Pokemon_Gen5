
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
@echo on
:: データ更新
call update_hash.bat
:: ポケウッドパーティーデータをdata以下に作成する
cd ../../resource/pokewood_party/
ruby pkwd_csv.rb ../../tools/waza_check/data/pkwd_info.csv
:: チェックツールを実行する
cd ../../tools/waza_check/script/
ruby -Ks -I ./hash;./hash_conv;./personal;./trainer waza_check_for_pkwd_poke.rb
cd ..
@echo off

:end
Pause

