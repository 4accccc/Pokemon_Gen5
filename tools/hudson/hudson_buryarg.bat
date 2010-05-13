@echo off
REM =================================================================
REM hudson用のSLRに引数を埋め込むバッチ
REM 用途    : prompt.bat cmd hudson_buryarg.bat でhudsonから呼び出し
REM 制作者  : hosaka_genya
REM 日付    : 2010/05/12
REM =================================================================

REM main.srl
SET PATH_MAIN_SRL=%PROJECT_PROGDIR%bin/ARM9-TS.HYB/Release/main.srl
SET PATH_MAIN_TLF=%PROJECT_PROGDIR%bin/ARM9-TS.HYB/Release/main.tlf

REM テストナンバー >> hudsonで指定
SET TEST_NO=%1
@echo TEST_NO = %TEST_NO%

REM ================================
REM ユーザーネーム設定
REM ================================
SET USERNAME=hudson
@echo USERNAME = %USERNAME%

REM ================================
REM TLFに書きこんでからSRLを再生成
REM ================================
buryarg.TWL %PATH_MAIN_TLF% %TEST_NO%
touch %PATH_MAIN_SRL%
make

