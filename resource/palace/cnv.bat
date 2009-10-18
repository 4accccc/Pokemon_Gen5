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
g2dcvtr debug_areano.nce -br -ncn

g2dcvtr palace_obj.nce -br -ncn
ntrcomp -lex -A4 -o palace_obj_lz.NCGR palace_obj.NCGR

g2dcvtr palace_bg.nsc -bg
ntrcomp -lex -A4 -o palace_bg_lz.NCGR palace_bg.NCGR
ntrcomp -lex -A4 -o palace_bg_lz.NSCR palace_bg.NSCR


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
