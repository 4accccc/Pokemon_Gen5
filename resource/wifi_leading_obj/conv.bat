del *.NCLR
del *.NCGR
del *.NCER
del *.NANR

g2dcvtr hero_gts.nce -br
g2dcvtr heroine_gts.nce -br

rename hero.NCGR hero_gts.NCGR
rename heroine.NCGR heroine_gts.NCGR

g2dcvtr hero_simple.nce -br
g2dcvtr heroine_simple.nce -br

rename hero.NCGR hero_simple.NCGR
rename heroine.NCGR heroine_simple.NCGR

g2dcvtr hero.nce -br
g2dcvtr heroine.nce -br
g2dcvtr hero_ine_kage.nce -br
