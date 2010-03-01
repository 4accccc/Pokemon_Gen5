g2dcvtr deposit.ncl
g2dcvtr mybox.ncl
g2dcvtr search.ncl
g2dcvtr title.ncl
g2dcvtr traderoom.ncl
g2dcvtr wifi_snap.ncl
g2dcvtr poke_view.ncl
g2dcvtr worldtrade_obj.ncl
g2dcvtr hero.ncl

g2dcvtr deposit.ncg -bg
g2dcvtr mybox.ncg -bg
g2dcvtr search.ncg -bg
g2dcvtr title.ncg -bg
g2dcvtr traderoom.ncg -bg
g2dcvtr wifi_snap.ncg -bg
g2dcvtr poke_view.ncg -bg
g2dcvtr worldtrade_obj.nce
g2dcvtr worldtrade_obj_s.nce
g2dcvtr btower.nce

g2dcvtr deposit.nsc
g2dcvtr mybox.nsc
g2dcvtr mypoke.nsc
g2dcvtr search.nsc
g2dcvtr searchpoke0.nsc
g2dcvtr searchpoke1.nsc
g2dcvtr title_base.nsc
g2dcvtr title_menu.nsc
g2dcvtr traderoom.nsc


ntrcomp -l 2 -o deposit_lz.ncgr deposit.ncgr
ntrcomp -l 2 -o mybox_lz.ncgr mybox.ncgr
ntrcomp -l 2 -o search_lz.ncgr search.ncgr
ntrcomp -l 2 -o title_lz.ncgr title.ncgr
ntrcomp -l 2 -o traderoom_lz.ncgr traderoom.ncgr
ntrcomp -l 2 -o wifi_snap_lz.ncgr wifi_snap.ncgr
ntrcomp -l 2 -o poke_view_lz.ncgr poke_view.ncgr
ntrcomp -l 2 -o worldtrade_obj_lz.ncgr worldtrade_obj.ncgr
ntrcomp -l 2 -o worldtrade_obj_lz.ncer worldtrade_obj.ncer
ntrcomp -l 2 -o worldtrade_obj_lz.nanr worldtrade_obj.nanr
ntrcomp -l 2 -o hero_lz.ncgr hero.ncgr
ntrcomp -l 2 -o worldtrade_obj_s_lz.ncer worldtrade_obj_s.ncer
ntrcomp -l 2 -o worldtrade_obj_s_lz.nanr worldtrade_obj_s.nanr
ntrcomp -l 2 -o deposit_lz.nscr deposit.nscr
ntrcomp -l 2 -o mybox_lz.nscr mybox.nscr
ntrcomp -l 2 -o mypoke_lz.nscr mypoke.nscr
ntrcomp -l 2 -o search_lz.nscr search.nscr
ntrcomp -l 2 -o searchpoke0_lz.nscr searchpoke0.nscr
ntrcomp -l 2 -o searchpoke1_lz.nscr searchpoke1.nscr
ntrcomp -l 2 -o title_base_lz.nscr title_base.nscr
ntrcomp -l 2 -o title_menu_lz.nscr title_menu.nscr
ntrcomp -l 2 -o traderoom_lz.nscr traderoom.nscr
ntrcomp -l 2 -o btower_obj_lz.ncgr btower.ncgr
ntrcomp -l 2 -o btower_obj_lz.ncer btower.ncer
ntrcomp -l 2 -o btower_obj_lz.nanr btower.nanr


nnsarc -c -i -n worldtrade.narc -S worldtrade.script
