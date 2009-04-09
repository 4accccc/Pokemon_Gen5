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
//extern VMCMD_RESULT STA_SCRIPT_FuncName_Func(VMHANDLE *vmh, void *context_work);
#define SCRIPT_FUNC_DEF(str) extern VMCMD_RESULT STA_SCRIPT_ ## str ## _Func(VMHANDLE *vmh, void *context_work);
#include "script_table.dat"
#undef SCRIPT_FUNC_DEF

//============================================================================================
/**
 *	スクリプトテーブル
 */
//============================================================================================
//定義された順に関数を配列に格納
//STA_SCRIPT_FuncName_Func,
#define SCRIPT_FUNC_DEF(str) STA_SCRIPT_ ## str ## _Func,
VMCMD_FUNC STA_ScriptFuncArr[]={
#include "script_table.dat"
};
#undef SCRIPT_FUNC_DEF
