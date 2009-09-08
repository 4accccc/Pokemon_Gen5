PATH=%PATH%;c:\NitroSDK\tools\bin;c:\NitroSystem\tools\win\bin;


g2dcvtr back_bg_up.ncl
g2dcvtr back_bg_dwon.ncl 
g2dcvtr back_bg_up.nsc
g2dcvtr back_bg_down.nsc
g2dcvtr obj_gage.ncl
g2dcvtr obj_gage.nce
g2dcvtr obj_pokemon.ncl
g2dcvtr obj_pokemon.nce
g2dcvtr obj_note.ncl
g2dcvtr obj_note.nce
g2dcvtr obj_button.ncl
g2dcvtr obj_button.nce

@if errorlevel 1 goto err

echo アーカイブの作成
nnsarc -c -i -n mictest.narc -S arc_list.txt

@if errorlevel 1 goto err

echo 破棄
del *.NCLR
del *.NCGR
del *.NCER
del *.NANR
del *.NSCR


@if errorlevel 1 goto err

@goto ok

:ok
   @echo =======================
   @echo  Convert is SUCCESS !!!
   @echo =======================
   @goto end

:err
   @echo ***********************
   @echo        Error !!! 
   @echo **********************
   @goto end
:end

Pause
