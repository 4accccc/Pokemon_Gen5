//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_pokewood_table.c
 *	@brief  ポケウッド用スクリプトコマンドテーブル
 *	@author	tamada  GAME FREAK inc.
 *  @date   2011.06.30
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
//スクリプトコマンド
#include "scrcmd_pokewood.h"
//外部参照
#include "scrcmd_pokewood_table.h"

//-------------------------------------
///	テーブルをマクロで自動生成
//  テーブル名は ScriptCmdTbl_XXXXXになります。
//  XXXXXはcdatに書いたINIT_OVERLAY_CMDに与えた引数です
//=====================================
//変換マクロ
#include "array_table_macro.h"
//cdat
#include "scrcmd_pokewood_table.cdat"
