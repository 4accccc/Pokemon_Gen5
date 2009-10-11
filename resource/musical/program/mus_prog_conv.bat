SET PROJECT_ROOT=../../..
SET WBLIBDIR=%PROJECT_ROOT%/lib
SET NITROSYSTEM_ROOT=%WBLIBDIR%/NitroSystem
SET CWFOLDER=C:/Program Files/Freescale/CW for NINTENDO DSi V1.1

PATH=%NITROSYSTEM_ROOT%\tools\win\bin;%PATH%

ruby program_make.rb

cp p01/mus_prog_data.narc ./
pause