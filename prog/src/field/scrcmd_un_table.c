//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\scrcmd_un_table.c
 *	@brief    国連用スクリプトコマンドテーブル
 *	@author		Toru=Nagihashi
 *	@data		2010.10.12
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
//スクリプトコマンド
#include "scrcmd_un.h"
//外部参照
#include "scrcmd_un_table.h"

//-------------------------------------
///	テーブルをマクロで自動生成
//  テーブル名は ScriptCmdTbl_XXXXXになります。
//  XXXXXはcdatに書いたINIT_OVERLAY_CMDに与えた引数です
//=====================================
//変換マクロ
#include "array_table_macro.h"
//cdat
#include "scrcmd_un_table.cdat"
