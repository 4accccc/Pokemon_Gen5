@echo off
REM =================================================================
REM hudson用のmakeバッチ
REM 用途    : prompt.bat cmd hudson_make.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2009/11/24
REM =================================================================

REM main.srl
SET PATH_MAIN_SRL=%PROJECT_PROGDIR%/bin/ARM9-TS/Release/main.srl

REM 実行タイムアウト(秒) >> hudsonで指定
REM SET TIMEOUT_EXEC=60 

REM 対象DSのシリアルナンバー >> hudsonで指定
REM SET SERIAL_NO=05093474

REM 終了検出文字列(改行が来た時に先頭から判定)
SET ABORT_STRING="  **** ASSERTION FAILED ! ****"

REM ================================
REM ユーザーネーム設定
REM ================================
SET USERNAME=hudson
@echo USERNAME = %USERNAME%

REM ================================
REM ブート
REM ================================
loadrun -T %TIMEOUT_EXEC% -a %ABORT_STRING% -s %SERIAL_NO% %PATH_MAIN_SRL%
echo ErrorLevel = %ERRORLEVEL%

REM タイムアウト判定
if %ERRORLEVEL% == 205 goto _TIMEOUT:

REM 指定文字列検知で終了
if %ERRORLEVEL% == 200 goto _ABORT_STRING:

REM ================================
REM タイムアウト
REM ================================
:_TIMEOUT
  @echo timeout
  goto _END:
  
REM ================================
REM 指定文字列検知終了
REM ================================
:_ABORT_STRING
  REM エラーコードを丸々返す
  exit %ERRORLEVEL%

REM ================================
REM 正常終了
REM ================================
:_END
  @echo hudson test is SUCCESS .
  exit 0

