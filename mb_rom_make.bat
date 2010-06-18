REM まずバージョンファイルのコピー
cd prog/spec/
copy version_br ../version
cd ../
move version_br version
cd ../

REM マルチブートのクリーン
cd multiboot
make clean
cd ../multiboot2
make clean
cd ../resource
make
if not %ERRORLEVEL%==0 GOTO ERROR
cd ..

REM MultiBoot1の実行
cd multiboot
cd arc
make
cd ..
make
if not %ERRORLEVEL%==0 GOTO ERROR
ECHO multibootのmakeが完了しました。
ECHO 認証コードを発行してください。
ECHO 保存先はmultiboot/spec/main_r.sgn
pause
call attach_rom.bat
cd ..

REM MultiBoot2の実行
cd multiboot2
cd arc
make
cd ..
make
if not %ERRORLEVEL%==0 GOTO ERROR
ECHO multiboot2のmakeが完了しました。
ECHO 認証コードを発行してください。
ECHO 保存先はmultiboot2/spec/main_r.sgn
pause
call attach_rom.bat
cd ..

REM versionを戻す
cd prog/spec/
copy version ../version
cd ../../

if %ERRORLEVEL%==0 GOTO END
ECHO マルチブートROMの作成が完了しました。

:ERROR
echo エラーが発生しました

:END
