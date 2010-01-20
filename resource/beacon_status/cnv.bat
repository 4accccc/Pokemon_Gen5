del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR

g2dcvtr beacon_status.nsc -bg
g2dcvtr bstatus_bg01.nsc -bg -pcm
g2dcvtr bstatus_bg02.nsc -bg -pcm
g2dcvtr bstatus_bg03.nsc -bg -pcm
ntrcomp -lex -A4 -o beacon_status_lz.NSCR beacon_status.NSCR
ntrcomp -lex -A4 -o beacon_status_lz.NCGR beacon_status.NCGR
ntrcomp -lex -A4 -o bstatus_bg_lz.NCGR bstatus_bg.NCGR
ntrcomp -lex -A4 -o bstatus_bg01_lz.NSCR bstatus_bg01.NSCR
ntrcomp -lex -A4 -o bstatus_bg02_lz.NSCR bstatus_bg02.NSCR
ntrcomp -lex -A4 -o bstatus_bg03_lz.NSCR bstatus_bg03.NSCR

g2dcvtr bstatus_obj.nce -br -pcm

g2dcvtr beacon_status.ncl

echo narcçÏê¨
nnsarc -c -l -n -i beacon_status -S arc_list.txt

del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
