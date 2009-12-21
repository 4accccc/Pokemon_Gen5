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


g2dcvtr beacon_status.nsc -bg
ntrcomp -lex -A4 -o beacon_status_lz.NSCR beacon_status.NSCR
ntrcomp -lex -A4 -o beacon_status_lz.NCGR beacon_status.NCGR

g2dcvtr beacon_status.ncl

echo narcçÏê¨
nnsarc -c -l -n -i beacon_status -S arc_list.txt


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
