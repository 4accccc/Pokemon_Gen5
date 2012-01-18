REM まずバージョンファイルのコピー
cd prog/spec/
copy version_br ../version
cd ../
move version_br version
cd ../

CALL mb_release_make.bat

REM versionを戻す
cd prog/spec/
copy version ../version
cd ../../

