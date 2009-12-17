SET PROJECT_ROOT=../../..
SET WBLIBDIR=%PROJECT_ROOT%/lib
SET NITROSDK_ROOT=%WBLIBDIR%\TwlSDK
SET NITROSYSTEM_ROOT=%WBLIBDIR%\TwlSystem
SET NITROWIFI_ROOT=%WBLIBDIR%\TwlDWC
SET NITRODWC_ROOT=%WBLIBDIR%\TwlDWC
SET CWFOLDER=C:/Program Files/Freescale/CW for NINTENDO DSi V1.1

PATH=c:\tools;%PROJECT_ROOT%;%PROJECT_ROOT%\tools;c:\tools\subversion\bin;c:\cygwin\bin;c:\tools\vim;C:\tools;%NITROSDK_ROOT%\tools\bin;%NITROSYSTEM_ROOT%\tools\bin;%PATH%;


cp p01/mus_prog_data.narc ./mus_prog_data_01.narc
cp p02/mus_prog_data.narc ./mus_prog_data_02.narc
cp p03/mus_prog_data.narc ./mus_prog_data_03.narc
cp p04/mus_prog_data.narc ./mus_prog_data_04.narc
cp p05/mus_prog_data.narc ./mus_prog_data_05.narc
pause