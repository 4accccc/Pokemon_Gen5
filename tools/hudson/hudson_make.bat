@echo off
REM =================================================================
REM hudson用のmakeバッチ
REM 用途    : prompt.bat cmd hudson_make.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2009/11/24
REM =================================================================

SET IPMSG_BAT=tools/hudson/hudson_ipmsg.bat

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
REM cleanしておく
REM make clean

REM IPMSGで通知(エラーレベルを上書きしてしまう)
REM %IPMSG_BAT% 1
REM ↑を使うと、↓に流れてこない...

@echo errorlevel = %ERRORLEVEL%
exit %ERRORLEVEL%

REM ================================
REM 正常終了
REM ================================
:END
  REM %IPMSG_BAT% 0
  @echo hudson make end


