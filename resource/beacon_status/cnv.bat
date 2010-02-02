del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR

g2dcvtr bdetail_bgu01.nsc -bg -pcm
g2dcvtr bdetail_bgu02.nsc -bg -pcm
g2dcvtr bdetail_bgd01.nsc -bg -pcm
ntrcomp -lex -A4 -o bdetail_bgu_lz.NCGR bdetail_bgu.NCGR
ntrcomp -lex -A4 -o bdetail_bgd_lz.NCGR bdetail_bgd.NCGR
ntrcomp -lex -A4 -o bdetail_bgu01_lz.NSCR bdetail_bgu01.NSCR
ntrcomp -lex -A4 -o bdetail_bgu02_lz.NSCR bdetail_bgu02.NSCR
ntrcomp -lex -A4 -o bdetail_bgd01_lz.NSCR bdetail_bgd01.NSCR

g2dcvtr bdetail_obj.nce -br

g2dcvtr bstatus_bg01.nsc -bg -pcm
g2dcvtr bstatus_bg02.nsc -bg -pcm
g2dcvtr bstatus_bg03.nsc -bg -pcm
ntrcomp -lex -A4 -o bstatus_bg_lz.NCGR bstatus_bg.NCGR
ntrcomp -lex -A4 -o bstatus_bg01_lz.NSCR bstatus_bg01.NSCR
ntrcomp -lex -A4 -o bstatus_bg02_lz.NSCR bstatus_bg02.NSCR
ntrcomp -lex -A4 -o bstatus_bg03_lz.NSCR bstatus_bg03.NSCR

g2dcvtr bstatus_obj.nce -br
g2dcvtr bstatus_icon01.nce -br
g2dcvtr bstatus_icon02.nce -br
g2dcvtr bstatus_icon03.nce -br
g2dcvtr bstatus_icon04.nce -br
g2dcvtr bstatus_icon01.nce -br
g2dcvtr bstatus_icon01.nce -br
g2dcvtr bstatus_icon05.nce -br
g2dcvtr bstatus_icon06.nce -br
g2dcvtr bstatus_icon07.nce -br
g2dcvtr bstatus_icon08.nce -br
g2dcvtr bstatus_icon09.nce -br
g2dcvtr bstatus_icon10.nce -br
g2dcvtr bstatus_icon11.nce -br
g2dcvtr bstatus_icon12.nce -br
g2dcvtr bstatus_icon13.nce -br

g2dcvtr bstatus_panel.ncl

echo narcì¬
nnsarc -c -l -n -i beacon_status -S arc_list.txt >arc_result.txt

del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
