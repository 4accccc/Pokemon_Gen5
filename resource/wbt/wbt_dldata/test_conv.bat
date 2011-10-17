@echo off

rem ------------------------
rem 単独実行時エラー
rem ------------------------
if "%1" == "" goto NOT_EXIST

rem ------------------------
rem 正しくドラッグされた場合
rem (パスを指定しないとだめな場合があるらしい)
rem ------------------------
copy %1 %~dp1test_all_crc.bin

rem -----------
rem 終了
rem -----------
goto END

rem -----------------------
rem 単独実行時のエラー対応
rem -----------------------
:NOT_EXIST
echo "ファイルをドラッグしてください"
goto END


:END
rem pause

