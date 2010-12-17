
:: =====================================================
::	@file		script_search.bat
::  @brief  D&Dされたファイルから、検索条件データを作成し汎用検索を行う
::  @author		Masayuki Onoue
::  @data		2010.10.20
:: =====================================================
ls
:: =====================================================
:: prompt.batから自らを起動する処理
:: =====================================================
@echo off
if "%1" == "" goto end
if "%2" == "" goto call_cmd
goto EX

:call_cmd
cd %~dp0
../../../../prompt.bat cmd %0 %1 ex
goto end

:: =====================================================
:: 実際の処理
:: =====================================================
:EX
cd script
set EXPASS=%1
:: パスの\を/に置き換える
set EXPASS=%EXPASS:\=/%
echo %~n1から条件ファイルを生成します
ruby ../../../../../tools/exceltool/xls2xml/tab_out.rb %EXPASS% > %~n1.txt
echo 検索中・・・
ruby script_search_call.rb %~n1.txt
echo 結果をresult\script_search\フォルダに出力しました
cd ..

:end
@echo off
if "%1" == "" echo ダブルクリックでは実行できません
Pause
@echo on


