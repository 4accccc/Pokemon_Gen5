@echo off
REM =================================================================
REM hudson用のmakeバッチ
REM 用途    : prompt.bat cmd hudson_make.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2009/11/24
REM =================================================================

REM main.srl
SET PATH_MAIN_SRL=%PROJECT_PROGDIR%bin/ARM9-TS/Release/main.srl

REM 実行タイムアウト(秒) >> hudsonで指定
REM SET TIMEOUT_EXEC=60 

REM 対象DSのシリアルナンバー >> hudsonで指定
REM SET SERIAL_NO=05113644

REM 終了検出文字列(改行が来た時に先頭から判定)
SET ABORT_STRING="  **** ASSERTION FAILED ! ****"

REM ================================
REM ユーザーネーム設定
REM ================================
SET USERNAME=hudson
@echo USERNAME = %USERNAME%

@echo loadpath = %PATH_MAIN_SRL%

REM ================================
REM ブート
REM ================================
REM buryarg %PATH_MAIN_SRL% 1

loadrun -T %TIMEOUT_EXEC% -a %ABORT_STRING% -s %SERIAL_NO% %PATH_MAIN_SRL%
echo ErrorLevel = %ERRORLEVEL%

REM デバイスが見つからなかった
if %ERRORLEVEL% == -1 goto _ERROR_END:

REM タイムアウト判定
if %ERRORLEVEL% == 205 goto _END:

REM 指定文字列検知で終了
if %ERRORLEVEL% == 200 goto _ERROR_END:
  
REM ================================
REM 指定文字列検知終了
REM ================================
:_ERROR_END
  REM エラーコードを丸々返す
  @echo hudson test is FAILED .
  pause
  exit %ERRORLEVEL%

REM ================================
REM 正常終了
REM ================================
:_END
  @echo hudson test is SUCCESS .
  pause
  exit 0

