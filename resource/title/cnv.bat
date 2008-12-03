del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR

echo g2dコンバート
g2dcvtr wb_logo_bk.nsc -bg
g2dcvtr wb_logo_bk.ncl

echo narc作成
nnsarc -c -l -n -i title -S title_arc.txt

del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
