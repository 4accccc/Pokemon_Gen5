@echo off
REM =================================================================
REM hudson用のmakeバッチ
REM 用途    : prompt.bat cmd hudson_make.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2009/11/24
REM =================================================================

REM ================================
REM ユーザーネーム設定
REM ================================
SET USERNAME=hudson
@echo USERNAME = %USERNAME%

REM ================================
REM ブート
REM ================================
nitrorun %PROJECT_PROGDIR%/bin/ARM9-TS/Release/main.srl

REM 終了判定
if %ERRORLEVEL% == 0 goto END:

REM エラーコードを丸々返す
exit %ERRORLEVEL%

REM ================================
REM 正常終了
REM ================================
:END
  @echo hudson make end


