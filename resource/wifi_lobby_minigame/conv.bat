g2dcvtr font_boad.nce -br
g2dcvtr minigame_win_oam.nce -br
g2dcvtr result_fusen.nce -br
g2dcvtr minigame_win_bg0.nsc
g2dcvtr minigame_win_bg1.nsc
g2dcvtr minigame_win_bg2.nsc
g2dcvtr minigame_win.ncl
g2dcvtr minigame_win_ani.ncl
g2dcvtr touchpen.nce -br
g2dcvtr mini_fusen_score.nsc

nnsarc -c -i -n wlmngm_tool -S list.txt

del *.NSCR
del *.NCLR
del *.NCGR
del *.NCER
del *.NANR
