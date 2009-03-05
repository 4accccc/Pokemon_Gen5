copy ..\wifi_union_obj\wf* .\
copy ..\wifi_2dchar\pl_girl01* .\
copy ..\wifi_2dchar\pl_boy01* .\

g2dcvtr only_front00.nce -br -ncn
g2dcvtr only_front01.nce -br -ncn
g2dcvtr only_front02.nce -br -ncn
g2dcvtr only_front03.nce -br -ncn
g2dcvtr only_front04.nce -br -ncn
g2dcvtr only_front05.nce -br -ncn
g2dcvtr only_front06.nce -br -ncn
g2dcvtr only_front07.nce -br -ncn
g2dcvtr only_front08.nce -br -ncn
g2dcvtr only_front09.nce -br -ncn
g2dcvtr only_front10.nce -br -ncn
g2dcvtr only_front11.nce -br -ncn
g2dcvtr only_front12.nce -br -ncn
g2dcvtr only_front13.nce -br -ncn
g2dcvtr only_front14.nce -br -ncn
g2dcvtr only_front15.nce -br -ncn
g2dcvtr only_front_girl.nce -br
g2dcvtr only_front_boy.nce -br



nnsarc -c -i -n unionobj2d_onlyfront -S conv_only_frontlist.txt

del *.NSCR
del *.NCLR
del *.NCGR
del *.NCER
del *.NANR
