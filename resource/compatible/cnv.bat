del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.naix
del *.narc


echo g2dコンバート
g2dcvtr cc_circle.nce -br -ncn
g2dcvtr cc_back_s.nsc -bg
g2dcvtr cc_back_m.nsc -bg
g2dcvtr cc_count_m.nsc -bg
g2dcvtr cc_circle_light.ncl


echo narc作成
nnsarc -c -l -n -i compati_check -S arc_list.txt


echo g2dファイル削除
del *.NCLR
del *.NCGR
del *.NCER
del *.NANR
