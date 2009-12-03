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


echo obj
g2dcvtr mono_obj.nce -br -ncn
ntrcomp -lex -A4 -o mono_obj_lz.NCGR mono_obj.NCGR


echo bg
g2dcvtr mono_bgd_base.nsc -bg
ntrcomp -lex -A4 -o mono_bgd_base_lz.NSCR mono_bgd_base.NSCR

g2dcvtr mono_bgd_joutai.nsc -bg
ntrcomp -lex -A4 -o mono_bgd_joutai_lz.NSCR mono_bgd_joutai.NSCR

g2dcvtr mono_bgd_map.nsc -bg
ntrcomp -lex -A4 -o mono_bgd_map_lz.NSCR mono_bgd_map.NSCR

g2dcvtr mono_bgd_power.nsc -bg
ntrcomp -lex -A4 -o mono_bgd_power_lz.NSCR mono_bgd_power.NSCR

g2dcvtr mono_bgd_rireki.nsc -bg
ntrcomp -lex -A4 -o mono_bgd_rireki_lz.NSCR mono_bgd_rireki.NSCR

g2dcvtr mono_bgd_title.nsc -bg
ntrcomp -lex -A4 -o mono_bgd_title_lz.NSCR mono_bgd_title.NSCR

g2dcvtr mono_bgu_base.nsc -bg
ntrcomp -lex -A4 -o mono_bgu_base_lz.NSCR mono_bgu_base.NSCR

g2dcvtr mono_bgu_map.nsc -bg
ntrcomp -lex -A4 -o mono_bgu_map_lz.NSCR mono_bgu_map.NSCR

g2dcvtr mono_bgu_mission.nsc -bg
ntrcomp -lex -A4 -o mono_bgu_mission_lz.NSCR mono_bgu_mission.NSCR

g2dcvtr mono_bgu_power.nsc -bg
ntrcomp -lex -A4 -o mono_bgu_power_lz.NSCR mono_bgu_power.NSCR

ntrcomp -lex -A4 -o mono_bgd_lz.NCGR mono_bgd.NCGR
ntrcomp -lex -A4 -o mono_bgu_lz.NCGR mono_bgu.NCGR

g2dcvtr mono_bgd.ncl
g2dcvtr mono_bgu.ncl
g2dcvtr mono_obj.ncl

echo narcçÏê¨
nnsarc -c -l -n -i monolith -S monolith_arc.txt


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
