rem コンバートに必要な消えてはいけないファイルをコンバート用フォルダにコピーする
copy poke_icon_32k.nce graphic\poke_icon_32k.nce
copy poke_icon_64k.nce graphic\poke_icon_64k.nce
copy poke_icon_128k.nce graphic\poke_icon_128k.nce

cd graphic

rem poke_icon.nclを各アイコン用にコピーして作成する
call ncl_copy.bat

rem 共通のnceをコピーして全てのポケモン分のnceファイルを作成
call link.bat

rem コンバート実行
call cnv.bat

rem アイコンのパレット番号テーブル生成
picon_attr_cnv attr_list.txt

echo g2dファイル削除
del *_????.NCGR
del *_????.NCER
del *_????.NANR
del poke_icon.NCLR
del icon_???.NCLR
del icon_*.ncl
del *.nce

move *.narc ../
move *.naix ../

cd..

