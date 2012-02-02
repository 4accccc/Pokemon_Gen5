bannercvtr pm_b_bnr.nce
bannercvtr pm_w_bnr.nce
makebanner.TWL pokemon_WB_black.txt
makebanner.TWL pokemon_WB_white.txt
addbanner pokemon_WB_black.TWL.bnr blackbanner.srl
addbanner pokemon_WB_white.TWL.bnr whitebanner.srl

copy pokemon_WB_black.TWL.bnr ..\..\prog\spec
copy pokemon_WB_white.TWL.bnr ..\..\prog\spec

ruby multiboot_banner.rb pm_b_bnr.bin icon_b
ruby multiboot_banner.rb pm_w_bnr.bin icon_w

copy icon_b.char ..\..\prog\filetree\dl_rom
copy icon_b.plt ..\..\prog\filetree\dl_rom
copy icon_w.char ..\..\prog\filetree\dl_rom
copy icon_w.plt ..\..\prog\filetree\dl_rom
