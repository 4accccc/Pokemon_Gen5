FOR %%I IN (*.nsc) DO g2dcvtr %%I
nnsarc -c -l -n -i corp.narc -S datalist.scr
copy corp.naix ..\narc_cpy
copy corp.narc ..\narc_cpy
