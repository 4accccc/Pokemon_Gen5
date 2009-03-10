//======================================================================
/**
 * @file	script_table.h
 * @brief	ステージ スクリプト処理 スクリプトテーブル
 * @author	ariizumi
 * @data	09/03/09
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "sta_act_script.h"
#include "sta_act_script_def.h"

//# は引数を文字列に ## は文字列の結合 \ でマクロの改行

//externで関数を定義し、別ソースで関数が宣言されていてもOKなように
//extern SCRIPT_FINISH_TYPE STA_SCRIPT_FuncName_Func(STA_SCRIPT_SYS *work,STA_SCRIPT_WORK *scriptWork);
#define SCRIPT_FUNC_DEF(str) extern SCRIPT_FINISH_TYPE STA_SCRIPT_ ## str ## _Func(STA_SCRIPT_SYS *work,STA_SCRIPT_WORK *scriptWork);
#include "script_table.dat"
#undef SCRIPT_FUNC_DEF

//定義された順に関数を配列に格納
//STA_SCRIPT_FuncName_Func,
#define SCRIPT_FUNC_DEF(str) STA_SCRIPT_ ## str ## _Func,
STA_SCRIPT_FUNC STA_ScriptFuncArr[] =
{
#include "script_table.dat"
};
#undef SCRIPT_FUNC_DEF
