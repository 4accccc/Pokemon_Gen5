@echo off
rem UpdateWithGmm.bat gmmdir gmm mstxt
%PROJECT_ROOT%tools\MessageStudio\bin\MessageStudioConsole.exe /UpdateWithGmm;gmmdir="%1";pairs="%2,%3" %PROJECT_RSCDIR%message\msproj\ingame_text.msproj
