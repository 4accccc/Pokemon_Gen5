Path=c:\cygwin\bin; 
cp ../../../../prog/include/item/itemsym.h ./item/copy/itemsym.h
cp ../../../message/src/place_name.gmm ./item/copy/place_name.gmm
cp ../../zonetable/zonetable.xls ./item/copy/zonetable.xls
cd ./item/script
ruby ./excel_tab_out.rb ../copy/zonetable.xls 1 ../copy/zonetable.txt
ruby ./item_listup.rb
pause
