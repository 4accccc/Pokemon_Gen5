Path=c:\cygwin\bin;
cp ../../resource/sodateya/kowaza_table.xls ./data/kowaza_table.xls
cp ../../resource/personal/personal_wb.csv ./data/personal_wb.csv
cp ../../resource/trdata/trainer_wb.csv ./data/trainer_wb.csv
cp ../../resource/itemconv/wb_item.xls ./data/wb_item.xls
cp ../../resource/personal/monsno_hash.rb ./script/hash/monsno_hash.rb
cp ../../resource/personal/wazaoboe_hash.rb ./script/hash/wazaoboe_hash.rb
cp ../constant.rb ./script/personal/constant.rb
cp ../constant.rb ./script/trainer/constant.rb

cd ../exceltool
exceltabconv.exe ../waza_check/data/kowaza_table.xls
exceltabconv.exe ../waza_check/data/wb_item.xls

cd ../waza_check/script
ruby -Ks -I ./hash;./hash_conv;./personal;./trainer update_hash.rb
ruby -Ks -I ./hash;./hash_conv;./personal;./trainer main.rb
pause
