:: //[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
:: /**
::  *
::  *	@file		\home\master_2nd\resource\fldmapdata\flagwork\convert.bat
::  *	@brief  フラグワークコンバート
::  *	@author		Masayuki Onoue
::  *	@data		2010.10.08
::  *
::  */
:: //]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

:: =====================================================
:: prompt.batから自らを起動する処理
:: =====================================================
@echo off
if "%1" == "" goto call_cmd
goto EX

:call_cmd
../../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: 実際の処理
:: =====================================================
:EX
@echo フラグ生成開始...
ruby scripts\flag.rb flags.xls flag_define.h wkdir/debug_flag.list
@echo フラグ生成終了！！

@echo ワーク生成開始
ruby scripts\flag.rb works.xls work_define.h  wkdir/debug_work.list
@echo ワーク生成終了！

:end
Pause
@echo on
