Path=c:\cygwin\bin; 
cp ../../../../prog/include/item/itemsym.h ./copy/itemsym.h
cp ../../../message/src/place_name.gmm ./copy/place_name.gmm
cp ../../zonetable/zonetable.xls ./copy/zonetable.xls
cd ./script
ruby ./excel_tab_out.rb ../copy/zonetable.xls 1 ../copy/zonetable.txt
ruby ./item_listup.rb
pause
