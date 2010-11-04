
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
../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: 実際の処理
:: =====================================================
:EX
cd C:\home\swan\tools\musicaleditor
del we_000.bin
ruby c:\home\swan\resource\musical\eescmk.rb c:\home\swan\prog\src\musical\stage\script\musical_script_command.h we_test.esf c:/home/swan/ on

if not %ERRORLEVEL%==0 GOTO ERROR

make

if %ERRORLEVEL%==0 GOTO END

:ERROR
echo エラーが発生しました
pause

:end
Pause
@echo on


