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
REM 初回 make
REM ================================
@echo ***** make *****
make
@echo errorlevel = %ERRORLEVEL%

REM 終了判定
if %ERRORLEVEL% == 0 goto END:

REM ================================
REM エラー処理
REM ================================
REM make cleanして再度make
@echo ***** make clean *****
make clean
make
REM エラーコードを丸々返す（cleanでもmakeが通らなかったらhudsonに通知)
exit %ERRORLEVEL%

REM ================================
REM 正常終了
REM ================================
:END
  @echo hudson make end


