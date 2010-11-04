
:: =====================================================
::	@file		find_sp_ng_cmd.bat
::  @brief  特殊スクリプト内で使われている、使用禁止コマンドを調査します
::  @author		Masayuki Onoue
::  @data		2010.11.04
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
@echo off
cd script
echo 特殊スクリプト内で使われている、使用禁止コマンドを調査します
echo ev_init を調査中
ruby find_splabel_ng_cmd.rb ../init_ng_cmd.txt ev_init SP > ../result/find_sp_ng_cmd/init_out.txt
echo ev_recover を調査中
ruby find_splabel_ng_cmd.rb ../recover_ng_cmd.txt ev_recover SP > ../result/find_sp_ng_cmd/recov_out.txt
echo ev_zone を調査中
ruby find_splabel_ng_cmd.rb ../zone_ng_cmd.txt ev_zone SP > ../result/find_sp_ng_cmd/zone_out.txt
echo ev_reserve を調査中
ruby find_splabel_ng_cmd.rb ../reserve_ng_cmd.txt ev_reserve NRM > ../result/find_sp_ng_cmd/reserve_out.txt
cd ..
echo result/find_sp_ng_cmd/フォルダに結果を生成しました。
@echo on

:end
Pause

