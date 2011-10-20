//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_pokecen_table.c
 *  @brief  ポケモンセンター用スクリプトコマンドテーブル
 *	@author	Masayuki Onoue
 *	@date		2011.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//system
#include <gflib.h>
//スクリプトコマンド
#include "scrcmd_pokecen.h"
#include "system/vm_cmd.h"

//外部参照
#include "scrcmd_pokecen_table.h"

//-------------------------------------
///	テーブルをマクロで自動生成
//  テーブル名は ScriptCmdTbl_XXXXXになります。
//  XXXXXはcdatに書いたINIT_OVERLAY_CMDに与えた引数です
//=====================================
//変換マクロ
#include "array_table_macro.h"
//cdat
#include "scrcmd_pokecen_table.cdat"

