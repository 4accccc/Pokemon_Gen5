g3dcvtr earth_mark_1.imd
g3dcvtr earth_mark_2.imd
g3dcvtr earth_mark_3.imd
g3dcvtr wifi_earth.imd

g2dcvtr world_watch1.nsc
g2dcvtr world_watch2.nsc
g2dcvtr world_watch3.nsc
g2dcvtr world_watch4.nsc
g2dcvtr world_watch5.nsc
g2dcvtr world_watch_frame.nsc

g2dcvtr world_w0anim.ncl
g2dcvtr world_w2anim.ncl
g2dcvtr world_w3anim.ncl
g2dcvtr world_watch2a.nsc
g2dcvtr world_watch2b.nsc
g2dcvtr world_watch_wall.nsc
g2dcvtr world_watch_roll.nsc

g2dcvtr world_w_balloon00.nce -br
g2dcvtr world_w_balloon01.nce -br
g2dcvtr world_w_balloon02.nce -br
g2dcvtr world_w_balloon03.nce -br
g2dcvtr world_w_balloon04.nce -br
g2dcvtr world_w_balloon05.nce -br
g2dcvtr world_w_balloon06.nce -br
g2dcvtr world_w_balloon07.nce -br
g2dcvtr world_w_balloon08.nce -br
g2dcvtr world_w_balloon09.nce -br
g2dcvtr world_w_balloon10.nce -br
g2dcvtr world_w_balloon11.nce -br


nnsarc -c -i -n worldtimer -S list.txt


del *.NSCR
del *.NCLR
del *.NCGR
del *.NCER
del *.NANR
del *.nsbmd
del *.nsbca
del *.nsbta
