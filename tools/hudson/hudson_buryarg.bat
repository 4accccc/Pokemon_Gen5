@echo off
REM =================================================================
REM hudson用のSLRに引数を埋め込むバッチ
REM 用途    : prompt.bat cmd hudson_buryarg.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2010/05/12
REM =================================================================

SET PATH_TOOLS=%PROJECT_ROOT%\tools\hudson

REM テストナンバー >> hudsonで指定
SET TEST_NO=%1

REM ================================
REM ユーザーネーム設定
REM ================================
SET USERNAME=hudson
@echo USERNAME = %USERNAME%

REM まずmakeする
make

REM ================================
REM TLFに書きこんでからSRLを再生成
REM ================================
buryarg.TWL %PATH_MAIN_TLF% %TEST_NO%
touch %PATH_MAIN_SRL%
make

