
:: =====================================================
::	@file		sp_ng_cmd_make.bat
::  @brief  scrcmd_list.xls からfind_sp_ng_cmd.batに必要なファイルを作成する
::  @author		Masayuki Onoue
::  @data		2010.10.20
:: =====================================================

:: =====================================================
:: prompt.batから自らを起動する処理
:: =====================================================
@echo off
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: 実際の処理
:: =====================================================
:EX
::cp ../../../../prog/src/field/scrcmd_table.cdat ./scrcmd_table.cdat
cd script
cp ../../scrcmd_list.xls ./scrcmd_list.xls 
ruby ../../../../../tools/exceltool/xls2xml/tab_out.rb ./scrcmd_list.xls > ./scrcmd_list.txt
ruby sp_ng_cmd_make.rb scrcmd_list.txt
echo init_ng_cmd.txt,recover_ng_cmd.txt,reserve_ng_cmd.txt,zone_ng_cmd.txt
echo の4ファイルを生成しました。
cd ..

:end
Pause
@echo on

