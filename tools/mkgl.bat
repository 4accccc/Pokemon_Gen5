echo off
pushd %GFL_MAKE_DIR%
make %1 %2

if %1.==. GOTO NEXT
if %1==clean GOTO END

:NEXT
if %ERRORLEVEL%==0 GOTO NOERROR

:ERROR
GOTO END

:NOERROR
make wbc 

:END
popd
