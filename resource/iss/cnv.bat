set PROJECT_ROOT=C:/home/pokemon_wb/
set Path=c:\cygwin\bin;C:\WINDOWS\system32;C:/home/pokemon_wb/lib\NitroSystem\tools\win\bin;


ruby %PROJECT_ROOT%tools/exceltool/xls2xml/tab_out.rb dungeon.xls > dungeon.txt
ruby %PROJECT_ROOT%tools/exceltool/xls2xml/tab_out.rb 3d_sound.xls > 3d_sound.txt

ruby dungeon.rb 
ruby 3d_sound.rb 3d_sound.txt

nnsarc -c -l -n -i iss -S list.txt

call ./city/cnv.bat

del dungeon.txt
del 3d_sound.txt
del dungeon.bin
del 3d_sound_h01.bin
pause
