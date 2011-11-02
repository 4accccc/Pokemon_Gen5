REM MultiBootのクリーン
cd ./multiboot
make clean
cd ..
REM MultiBoot2のクリーン
cd ./multiboot2
make clean
cd ..

REM リソースをmake
cd ./resource
make
if not %ERRORLEVEL%==0 GOTO ERROR
cd ..

REM MultiBootの実行
cd ./multiboot
cd ./arc
make
cd ..
make
if not %ERRORLEVEL%==0 GOTO ERROR
cd ..

REM MultiBoot2の実行
cd ./multiboot2
cd ./arc
make
cd ..
make
if not %ERRORLEVEL%==0 GOTO ERROR
cd ..

if %ERRORLEVEL%==0 GOTO END
ECHO マルチブートROMの作成が完了しました。

:ERROR
echo エラーが発生しました

:END
