del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.naix
del *.narc


echo g2dコンバート
g2dcvtr p_st_bunrui_buturi.nce -br -ncn
g2dcvtr p_st_bunrui_henka.nce -br -ncn
g2dcvtr p_st_bunrui_tokusyu.nce -br -ncn
g2dcvtr p_st_type_beautiful.nce -br -ncn
g2dcvtr p_st_type_cute.nce -br -ncn
g2dcvtr p_st_type_dragon.nce -br -ncn
g2dcvtr p_st_type_ele.nce -br -ncn
g2dcvtr p_st_type_esp.nce -br -ncn
g2dcvtr p_st_type_evil.nce -br -ncn
g2dcvtr p_st_type_fight.nce -br -ncn
g2dcvtr p_st_type_fire.nce -br -ncn
g2dcvtr p_st_type_flight.nce -br -ncn
g2dcvtr p_st_type_ghost.nce -br -ncn
g2dcvtr p_st_type_grass.nce -br -ncn
g2dcvtr p_st_type_ground.nce -br -ncn
g2dcvtr p_st_type_ice.nce -br -ncn
g2dcvtr p_st_type_insect.nce -br -ncn
g2dcvtr p_st_type_intelli.nce -br -ncn
g2dcvtr p_st_type_leef.nce -br -ncn
g2dcvtr p_st_type_normal.nce -br -ncn
g2dcvtr p_st_type_poison.nce -br -ncn
g2dcvtr p_st_type_ques.nce -br -ncn
g2dcvtr p_st_type_rock.nce -br -ncn
g2dcvtr p_st_type_steel.nce -br -ncn
g2dcvtr p_st_type_strong.nce -br -ncn
g2dcvtr p_st_type_style.nce -br -ncn
g2dcvtr p_st_type_water.nce -br -ncn
g2dcvtr st_type.ncl


echo narc作成
nnsarc -c -l -n -i wazatype_icon -S wazatype_icon.txt


echo g2dファイル削除
del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
