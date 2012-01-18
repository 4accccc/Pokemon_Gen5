
:: =====================================================
:: prompt.bat‚©‚çŽ©‚ç‚ð‹N“®‚·‚éˆ—
:: =====================================================
@echo off
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: ŽÀÛ‚Ìˆ—
:: =====================================================
:EX
@echo on
cp ../../resource/sodateya/kowaza_table.xls ./data/kowaza_table.xls
cp ../../resource/personal/personal_sw.csv ./data/personal_sw.csv
cp ../../resource/trdata/trainer_wb.csv ./data/trainer_wb.csv
cp ../../resource/itemconv/wb_item.xls ./data/wb_item.xls
cp ../../resource/personal/monsno_hash.rb ./script/hash/monsno_hash.rb
cp ../../resource/personal/wazaoboe_hash.rb ./script/hash/wazaoboe_hash.rb
cp ../../resource/shopdata/waza_oshie_hash.rb ./script/hash/waza_oshie_hash.rb
cp ../constant.rb ./script/personal/constant.rb
cp ../constant.rb ./script/trainer/constant.rb

cd ../exceltool
exceltabconv.exe ../waza_check/data/kowaza_table.xls
exceltabconv.exe ../waza_check/data/wb_item.xls
ruby ./xls2xml/tab_out_direct_sheet.rb -s ../waza_check/data/old_waza_oshie.xls > ../waza_check/data/old_waza_oshie.csv

cd ../waza_check/script

ruby -Ks -I ./hash;./hash_conv;./personal;./trainer update_hash.rb
cd ..
@echo off

:end
Pause
