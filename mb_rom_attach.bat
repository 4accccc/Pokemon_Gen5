REM MultiBootのアタッチ
cd ./multiboot
CALL attach_rom.bat
if not %ERRORLEVEL%==0 GOTO ERROR
cd ..

REM MultiBoot2のアタッチ
cd ./multiboot2
CALL attach_rom.bat
if not %ERRORLEVEL%==0 GOTO ERROR
cd ..


if %ERRORLEVEL%==0 GOTO END
ECHO マルチブートROMのアタッチが完了しました。

:ERROR
echo エラーが発生しました

:END
