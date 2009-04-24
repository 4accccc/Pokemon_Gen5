@echo off
set PATH=\cygwin\bin;%PATH%;

REM ---------------------------------------------------------
REM コンバート元フォルダ
REM ---------------------------------------------------------
set GS_SRC_DIR=C:\gshome\pokemon_gs\src\application\e_mail\

REM ---------------------------------------------------------
REM コンバート先フォルダ
REM ---------------------------------------------------------
set WB_DST_DIR=c:\home\pokemon_wb\prog\src\app\e_mail\

REM ---------------------------------------------------------
REM コンバートするファイル
REM ---------------------------------------------------------
set SRC_FILES=email.c email.dat email_enter.c email_input.c email_main.c email_snd.h email_snd_def.h email_test.c email_tool.c

REM ---------------------------------------------------------
REM コンバート本体
REM ---------------------------------------------------------

REM 開始メッセージ
echo "コンバートを開始します"

REM コンバート
for %%I in ( %SRC_FILES% ) do  (
REM echo "%%Iをコンバートします"
perl source_convert.pl %GS_SRC_DIR%%%I %WB_DST_DIR%%%I
)

REM	終了メッセージ
echo "全てのコンバートが終了しました"
PAUSE
