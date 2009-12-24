del itemequip.h
del itemsym.h
del itemindex.dat
del itemname.gmm
del iteminfo.gmm
del itemdata /q
del itemtype.dat
del waza_mcn.dat
del itempocket_def.h
del item_data.naix
del item_data.narc
del itemsort_abc_def.h

md itemdata

itemconv_wb.exe wb_item.csv >itemconv_wb.err
itemcnv_gmm.exe wb_item.csv >itemcnv_gmm.err
ruby makelist_sort_by_abc.rb
rem notepad itemconv_dp.err
rem notepad itemcnv_gmm.err

nnsarc -c -l -n -i item_data itemdata

move itemequip.h %1include/item
move itemsym.h %1include/item
move itemindex.dat %1src/item
move itemname.gmm %2resource/message/src
move iteminfo.gmm %2resource/message/src
move itemtype.dat %1src/item
move waza_mcn.dat %1src/item
move itemuse_def.h %1include/item
move itemtype_def.h %1include/item
cp itempocket_def.h %1include/item
cp itemsort_abc_def.h %1src/app/bag
