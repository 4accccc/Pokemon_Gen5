
:: =====================================================
::	@file		move_code_check_sp.bat
::  @brief  move codeが正しく設定されているかチェックするツール
::  @author		Masayuki Onoue
::  @data		2010.11.19
:: =====================================================

:: =====================================================
:: prompt.batから自らを起動する処理
:: =====================================================
@if "%1" == "" goto call_cmd
goto EX

:call_cmd
@../../../../prompt.bat cmd %0 ex
goto end

:: =====================================================
:: 実際の処理
:: =====================================================
:EX
set XLSFILE= ./move_code/fldmmdl_list.xls
set MMDLLIST= .\move_code\mdllist.csv
set TYPE_TABLE= .\move_code\anm_type_table.rb

@echo データのコピー...
@cp ..\..\..\fldmmdl\fldmmdl_list.xls %XLSFILE%
@cp ..\..\..\fldmmdl\rbdefine .\rbdefine.rb
..\..\..\..\tools\exceltool\ExcelSeetConv.exe -o %MMDLLIST% -p 1 -s csv %XLSFILE%
:: モデルパラメータの生成
@echo 定義ファイルの生成...
ruby mkdef.rb %MMDLLIST% %TYPE_TABLE%

:: 調べる
@echo 解析中...
ruby move_code_check.rb mvcmd_result_sp.txt /sp
@echo 解析終了！！
@Pause
:end


