del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR

echo g2dコンバート
g2dcvtr wb_logo_bk.nsc -bg
g2dcvtr wb_logo_bk.ncl -pcm
g2dcvtr wb_logo_bk2.nsc -bg
g2dcvtr wb_logo_bk2.ncl -pcm
g2dcvtr mist.nsc -bg
g2dcvtr mist.ncl -pcm

echo narc作成
nnsarc -c -l -n -i title -S title_arc.txt

del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
