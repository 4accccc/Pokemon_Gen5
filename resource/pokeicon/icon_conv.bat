rem コンバートに必要な消えてはいけないファイルをコンバート用フォルダにコピーする
copy poke_icon_32k.nce graphic\poke_icon_32k.nce
copy poke_icon_64k.nce graphic\poke_icon_64k.nce
copy poke_icon_128k.nce graphic\poke_icon_128k.nce

cd graphic

rem poke_icon.nclを各アイコン用にコピーして作成する
rem call ncl_copy.bat←たぶんやらんでもいいはず　poke_icon.nclとリンクしとけばこれでコンバートするはず

rem 共通のnceをコピーして全てのポケモン分のnceファイルを作成
rem call link.bat

rem コンバート実行
rem call cnv.bat

rem アイコンのパレット番号テーブル生成
rem picon_attr_cnv attr_list.txt

rem 上三つのバッチファイルを１つのRubyスクリプトで実行する
ruby poke_icon_conv.rb attr_list.txt ..\poke_icon.narc

rem del *.naix
rem del *.narc

echo g2dコンバート
g2dcvtr poke_icon_32k.nce -br
g2dcvtr poke_icon_64k.nce -br
g2dcvtr poke_icon_128k.nce -br

g2dcvtr poke_icon.ncl -pcm

echo narc作成
nnsarc -c -l -n -i poke_icon -S pokeicon_all.scr

echo g2dファイル削除
rem del *_????.NCGR
rem del *_????.NCER
rem del *_????.NANR
rem del poke_icon.NCLR
rem del icon_???.NCLR
rem del icon_*.ncl
del *.nce

move *.narc ../
move *.naix ../

cd..

