g2dcvtr unionbg.ncl
g2dcvtr unionobj.ncl
g2dcvtr unionbg.ncg -bg
g2dcvtr unionbg1.nsc
g2dcvtr unionbg1_base.nsc
g2dcvtr unionobj.nce


ntrcomp -l 2 -o unionbg_lz.ncgr unionbg.ncgr
ntrcomp -l 2 -o unionbg1_lz.nscr unionbg1.nscr
ntrcomp -l 2 -o unionbg1_base_lz.nscr unionbg1_base.nscr
ntrcomp -l 2 -o unionobj_lz.ncgr union_obj.ncgr
ntrcomp -l 2 -o unionobj_lz.ncer unionobj.ncer
ntrcomp -l 2 -o unionobj_lz.nanr unionobj.nanr

nnsarc -c -i -n unionroom.narc -S unionroom.script

