del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.nsbtx
del *.nsbmd
del *.nsbca
del *.ntfp
del *.ntft

echo g2dコンバート

echo g3dコンバート
g3dcvtr "cube.imd"

echo ntf
ntexconv block_tex16.bmp -b -f palette16

echo narc作成
nnsarc -c -l -n -i palace -S palace_arc.txt

del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.nsbtx
del *.nsbmd
del *.nsbca
del *.ntfp
del *.ntft
