FOR %%I IN (*.imd) DO g3dcvtr %%I -emdl
FOR %%I IN (*.imd) DO g3dcvtr %%I -etex
nnsarc -c -l -n -i sample_map.narc -S datalist.scr
copy sample_map.naix c:\home\pokemon_wb\prog\graphic_data\test_graphic
copy sample_map.narc c:\home\pokemon_wb\prog\graphic_data\test_graphic
