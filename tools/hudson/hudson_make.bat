@echo off
REM =================================================================
REM hudson用のmakeバッチ
REM 用途    : prompt.bat cmd hudson_make.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2009/11/24
REM =================================================================

SET PATH_TOOLS=C:\.hudson\jobs\wb_make\workspace\pokemon_wb\tools\hudson

REM デバイスのON/OFF は問題なく動作する。
REM devcon enable *HID*

REM IPMSGはなぜかhudsonがexit 0を拾ってしまう。

REM ================================
REM ユーザーネーム設定
REM ================================
SET USERNAME=hudson
@echo USERNAME = %USERNAME%

REM ================================
REM 初回 make
REM ================================
@echo ***** make *****
make

REM 正常終了判定
if %ERRORLEVEL% == 0 goto END:

REM ================================
REM エラー処理
REM ================================
REM cleanしておく-> 通知が遅れてしまうので一端コメントアウト
REM make clean

REM progクリーンを試す
cd prog
make clean
cd ..
make

REM 正常終了判定
if %ERRORLEVEL% == 0 goto END:

REM ================================
REM エラー終了
REM ================================
@echo errorlevel = %ERRORLEVEL%

exit %ERRORLEVEL%

REM ================================
REM 正常終了
REM ================================
:END
  @echo hudson make end


