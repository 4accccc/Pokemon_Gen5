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

@echo フラグ生成開始...
ruby scripts\flag.rb flags.xls flag_define.h wkdir/debug_flag.list
@echo フラグ生成終了！！

@echo ワーク生成開始
ruby scripts\flag.rb works.xls work_define.h  wkdir/debug_work.list
@echo ワーク生成終了！

PAUSE



