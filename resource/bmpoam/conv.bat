del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.naix
del *.narc
del *.nsbtx
del *.nsbmd
del *.nsbca

echo g2dコンバート
g2dcvtr bmpoam_dummy32x16.nce -ncn


echo narc作成
nnsarc -c -l -n -i bmpoam -S bmpoam.lst


del *.N??R
del *.nsbtx
del *.nsbmd
del *.nsbca
