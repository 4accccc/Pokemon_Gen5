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
g2dcvtr mini_fusen_ccwin.ncg -bg
g2dcvtr mini_fusen_ccwin.nsc
g2dcvtr mini_fusen_bottom.ncg -bg
g2dcvtr mini_fusen_bottom.nsc
g2dcvtr mini_fusen_bottom.ncl
g2dcvtr mini_fusen_top.ncl
g2dcvtr mini_fusen_pokefusen.ncl
g2dcvtr mini_fusen_akafusen.ncg -bg
g2dcvtr mini_fusen_akafusen.nsc
g2dcvtr mini_fusen_kiirofusen.ncg -bg
g2dcvtr mini_fusen_kiirofusen.nsc
g2dcvtr mini_fusen_midorifusen.ncg -bg
g2dcvtr mini_fusen_midorifusen.nsc
g2dcvtr mini_fusen_bg.ncg -bg
g2dcvtr mini_fusen_bg.nsc
g2dcvtr mini_fusen_pipe2p.nsc
g2dcvtr mini_fusen_pipe3p.nsc
g2dcvtr mini_fusen_pipe4p.nsc
g2dcvtr mini_fusen_window.ncg -bg
g2dcvtr mini_fusen_window.nsc
g2dcvtr mini_fusen_obj.nce -br -ncn
g2dcvtr mini_fusen_obj.ncl
g2dcvtr mini_fusen_ccobj.nce -br -ncn
g2dcvtr mini_fusen_ccobj.ncl
g2dcvtr aim.nce -br -ncn
g2dcvtr aim.ncl


echo g3dコンバート
g3dcvtr "sonans-polygon.imd"
g3dcvtr "sonans.imd" -etex
g3dcvtr "sonans_daiza.imd"
g3dcvtr "sonans_pipe_g.imd"
g3dcvtr "sonans_pipe_r.imd"
g3dcvtr "sonans_pipe_b.imd"
g3dcvtr "sonans_pipe_y.imd"
g3dcvtr "sonans-ball1.ica"
g3dcvtr "sonans-ball2.ica"
g3dcvtr "sonans-ball3.ica"
g3dcvtr "sonans-ball4.ica"
g3dcvtr "sonans-ball1r.imd"
g3dcvtr "sonans-ball2r.imd"
g3dcvtr "sonans-ball3r.imd"
g3dcvtr "sonans-ball4r.imd"
g3dcvtr "sonans-ball1g.imd"
g3dcvtr "sonans-ball2g.imd"
g3dcvtr "sonans-ball3g.imd"
g3dcvtr "sonans-ball4g.imd"
g3dcvtr "sonans-ball1b.imd"
g3dcvtr "sonans-ball2b.imd"
g3dcvtr "sonans-ball3b.imd"
g3dcvtr "sonans-ball4b.imd"
g3dcvtr "sonans-ball1y.imd"
g3dcvtr "sonans-ball2y.imd"
g3dcvtr "sonans-ball3y.imd"
g3dcvtr "sonans-ball4y.imd"


echo narc作成
nnsarc -c -l -n -i balloon_gra -S balloon_gra.lst
etd.exe balloon_gra.naix

del *.N??R
del *.nsbtx
del *.nsbmd
del *.nsbca
del *.naix
