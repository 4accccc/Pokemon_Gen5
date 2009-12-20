del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.naix
del *.narc
del *.nsbmd

g3dcvtr g_egg.imd -o g_egg.nsbmd
g3dcvtr g_panel02_c.imd -o g_panel02_c.nsbmd
g3dcvtr g_panel03_c.imd -o g_panel03_c.nsbmd
g3dcvtr g_panel04_c.imd -o g_panel04_c.nsbmd
g3dcvtr g_panel05_c.imd -o g_panel05_c.nsbmd
g3dcvtr g_egg_kage.imd -o g_egg_kage.nsbmd
g3dcvtr g_egg_cursor1.imd -o g_egg_cursor1.nsbmd
g3dcvtr g_egg_cursor2.imd -o g_egg_cursor2.nsbmd
g3dcvtr g_egg_cursor3.imd -o g_egg_cursor3.nsbmd
g3dcvtr g_egg_cursor4.imd -o g_egg_cursor4.nsbmd
g3dcvtr g_egg_cursor5.imd -o g_egg_cursor5.nsbmd
g3dcvtr g_egg_cursor1.ica -o g_egg_cursor1.nsbca
g3dcvtr g_egg_cursor2.ica -o g_egg_cursor2.nsbca
g3dcvtr g_egg_cursor3.ica -o g_egg_cursor3.nsbca
g3dcvtr g_egg_cursor4.ica -o g_egg_cursor4.nsbca
g3dcvtr g_egg_cursor5.ica -o g_egg_cursor5.nsbca


g2dcvtr guruguru_bg.ncl
g2dcvtr guruguru_bg.ncg -bg
g2dcvtr guruguru_bg0.nsc
g2dcvtr guruguru_bg1.nsc
g2dcvtr guruguru_sub.ncl
g2dcvtr guruguru_sub_bg.ncg -bg
g2dcvtr guruguru_sub_bg.nsc
g2dcvtr guruguru_but.ncg -bg
g2dcvtr guruguru_but0.nsc
g2dcvtr guruguru_but1.nsc
g2dcvtr guruguru_but2.nsc


nnsarc -i -c -l -n guru2 -S arc_list.txt

del *.NCLR
del *.NCGR
del *.NSCR
del *.NCER
del *.NANR
del *.nsbmd
del *.nsbca
