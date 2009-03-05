echo "アーカイブリスト生成"
perl wflby_gradata_conv.pl


echo "全ファイルコンパイル"
call convlist.bat



nnsarc -c -i -n wifi_lobby -S list.txt

copy wflby_3dmapobj_data.c ..\..\prog\src\net_app\wifi_lobby\map_conv\

del wflby_3dmapobj_data.c
del convlist.bat
