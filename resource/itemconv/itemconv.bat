del itemequip.h
del itemsym.h
del itemindex.dat
del itemname.gmm
del iteminfo.gmm
del itemdata /q
del itemtype.dat
del item_data.naix
del item_data.narc

md itemdata

itemconv_dp.exe wb_item.csv >itemconv_dp.err
itemcnv_gmm.exe wb_item.csv >itemcnv_gmm.err
rem notepad itemconv_dp.err
rem notepad itemcnv_gmm.err

nnsarc -c -l -n -i item_data itemdata

move itemequip.h %1include/item
move itemsym.h %1include/item
move itemindex.dat %1src/item
move itemname.gmm %2resource/message/src
move iteminfo.gmm %2resource/message/src
move itemtype.dat %1src/item
move itemuse_def.h %1include/item
