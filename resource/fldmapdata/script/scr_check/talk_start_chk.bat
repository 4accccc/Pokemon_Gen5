
:: =====================================================
::	@file		talk_start_chk.bat
::  @brief  会話コマンドの終了チェック
::  @author	Masayuki Onoue
::  @date		2010.11.04
:: =====================================================

:: =====================================================
:: prompt.batから自らを起動する処理
:: =====================================================
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: 実際の処理
:: =====================================================
:EX
@echo check_start..
@cd script
ruby talk_start_chk.rb > ../result/talk_start_chk/result.txt
@cd ..
@echo check_end
@Pause

:end
@echo off
if "%1" == "" echo ダブルクリックでは実行できません
Pause
@echo on

