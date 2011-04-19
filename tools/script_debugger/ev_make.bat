if "%1"=="callme" goto callme
prompt.bat cmd tools\script_debugger\ev_make.bat callme
goto end

:callme
SET SCRIPT_DEBUGGER_MODE=yes
make -C %PROJECT_RSCDIR%fldmapdata > %PROJECT_ROOT%tools\script_debugger\result.txt 2>&1
@REM make -C %PROJECT_RSCDIR%fldmapdata 2>&1 |tee %PROJECT_ROOT%tools\script_debugger\result.txt
@REM echo %ERRORLEVEL%
if ERRORLEVEL 1 goto error
goto end

:error
unix2dos -D %PROJECT_ROOT%tools\script_debugger\result.txt
exit 1

:end
