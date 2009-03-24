del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.naix
del *.narc


echo g2dコンバート
g2dcvtr shop_bg1.nsc
g2dcvtr shop_bg2.nsc
g2dcvtr shop_bg.ncg -bg
g2dcvtr shop_bg.ncl
g2dcvtr shop_sel.nce -br
g2dcvtr shop_arw.nce -br


echo narc作成
nnsarc -c -l -n -i shop_gra -S shop.txt


echo g2dファイル削除
del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
