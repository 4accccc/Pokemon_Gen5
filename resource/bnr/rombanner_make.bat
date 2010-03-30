ntexconv pm_b_bnr.bmp -no -bg -bgb -bgnc -f palette16
ntexconv pm_w_bnr.bmp -no -bg -bgb -bgnc -f palette16
makebanner pokemon_WB_black.txt
makebanner pokemon_WB_white.txt
makebanner.TWL pokemon_WB_black.txt
makebanner.TWL pokemon_WB_white.txt
addbanner pokemon_WB_black.bnr blackbanner.srl
addbanner pokemon_WB_white.bnr whitebanner.srl
copy pokemon_WB_black.TWL.bnr ..\..\prog\spec
copy pokemon_WB_white.TWL.bnr ..\..\prog\spec
