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

g2dcvtr palace_obj.nce -br -ncn
ntrcomp -lex -A4 -o palace_obj_lz.NCGR palace_obj.NCGR

g2dcvtr palace_bg.nsc -bg
g2dcvtr palace_bar.nsc -bg
g2dcvtr palace_bar2.nsc -bg
g2dcvtr palace_time.nsc -bg
ntrcomp -lex -A4 -o palace_bg_lz.NCGR palace_bg.NCGR
ntrcomp -lex -A4 -o palace_bg_lz.NSCR palace_bg.NSCR
ntrcomp -lex -A4 -o palace_bar_lz.NSCR palace_bar.NSCR
ntrcomp -lex -A4 -o palace_bar2_lz.NSCR palace_bar2.NSCR
ntrcomp -lex -A4 -o palace_time_lz.NSCR palace_time.NSCR


echo narcçÏê¨
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
