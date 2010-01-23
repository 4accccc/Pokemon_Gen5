REM make -C %PROJECT_RSCDIR%fldmapdata > %PROJECT_ROOT%tools\script_debugger\result.txt 2>&1
make -C %PROJECT_RSCDIR%fldmapdata 2>&1 |tee %PROJECT_ROOT%tools\script_debugger\result.txt
unix2dos -D %PROJECT_ROOT%tools\script_debugger\result.txt
pause
