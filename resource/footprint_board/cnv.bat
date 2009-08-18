del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.naix
del *.narc


echo g2dコンバート
g2dcvtr a_board_eff.nce -br
g2dcvtr a_board_sita.nsc -bg
g2dcvtr a_board_sita_bg.nsc -bg
g2dcvtr ashiato_board.nsc -bg
g2dcvtr ashiato_board_bg.nsc -bg
g2dcvtr ashiato_frame.nce -br -ncn
g2dcvtr ashiato_gage.nce -br -ncn
g2dcvtr foot_dummy.nce -br -ncn
g2dcvtr wifi_mark.nce -br -ncn
g2dcvtr a_board_font_s.ncl
g2dcvtr a_board_font_b.ncl


echo g3dコンバート
g3dcvtr a_board1.imd
g3dcvtr a_board2.imd
g3dcvtr ashiato.imd


echo narc作成
nnsarc -c -l -n -i footprint_board -S footprint_board.txt


echo g2dファイル削除
del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.nsbmd
