Path=c:\cygwin\bin; 
cp ../../../../prog/include/item/itemsym.h ./itemsym.h
cp ../../../message/src/place_name.gmm ./place_name.gmm
cp ../../zonetable/zonetable.xls ./zonetable.xls
ruby excel_tab_out.rb ./zonetable.xls 1 ./zonetable.txt
ruby item_listup.rb
pause
