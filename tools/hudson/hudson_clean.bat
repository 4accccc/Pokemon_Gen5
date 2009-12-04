@echo off
REM =================================================================
REM hudson > make clean > make clean 用バッチ
REM 用途    : prompt.bat cmd hudson_clean.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2009/11/24
REM =================================================================

SET PATH_TOOLS=C:\.hudson\jobs\wb_make\workspace\pokemon_wb\tools\hudson

REM ================================
REM ユーザーネーム設定
REM ================================
SET USERNAME=hudson
@echo USERNAME = %USERNAME%

REM ================================
REM  make clean
REM ================================
@echo ***** make clean *****
make clean

REM 正常終了判定
if %ERRORLEVEL% == 0 goto END:

REM ================================
REM エラー処理
REM ================================
@echo errorlevel = %ERRORLEVEL%
exit %ERRORLEVEL%

REM ================================
REM 正常終了
REM ================================
:END
  @echo hudson make end




