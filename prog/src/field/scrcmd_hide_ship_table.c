//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_hide_ship_table.c
 *	@brief  アジト船スクリプトコマンドテーブル
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//system
#include <gflib.h>
//スクリプトコマンド
#include "scrcmd_hide_ship.h"
//外部参照
#include "scrcmd_hide_ship_table.h"

//-------------------------------------
///	テーブルをマクロで自動生成
//  テーブル名は ScriptCmdTbl_XXXXXになります。
//  XXXXXはcdatに書いたINIT_OVERLAY_CMDに与えた引数です
//=====================================
//変換マクロ
#include "array_table_macro.h"
//cdat
#include "scrcmd_hide_ship_table.cdat"
