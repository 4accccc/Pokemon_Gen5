@echo off
if "%1"=="rsc" (
	make -C %PROJECT_RSCDIR% %2 %3
)
if "%1"=="prog" (
	make -C %PROJECT_PROGDIR% %2 %3
)
if "%1"=="arc" (
	make -C %PROJECT_ARCDIR% %2 %3
)
if "%1"=="test" (
	make -C %PROJECT_ROOT% %2 %3
)
if "%1"=="top" (
	make -C %PROJECT_ROOT% %2 %3 |tee %PROJECT_PROGDIR%\warning.txt
)

if "%1"=="scr" (
  make -C %PROJECT_RSCDIR%fldmapdata/script
  make -C %PROJECT_ARCDIR%
  make -C %PROJECT_PROGDIR%
)

if "%1"=="filter" (
	ruby %PROJECT_ROOT%\tools\warning\filter.rb %PROJECT_PROGDIR%\warning.txt
)

if "%1"=="" (
	make -C %PROJECT_PROGDIR% -j 3 %2 %3 |tee %PROJECT_PROGDIR%\warning.txt
)

