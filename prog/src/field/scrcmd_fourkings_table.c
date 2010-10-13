//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_fourkings_table.c
 *	@brief  四天王部屋用スクリプトコマンドテーブル
 *	@author		Toru=Nagihashi
 *	@data		2010.10.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
//スクリプトコマンド
#include "scrcmd_fourkings.h"
//外部参照
#include "scrcmd_fourkings_table.h"

//-------------------------------------
///	テーブルをマクロで自動生成
//  テーブル名は ScriptCmdTbl_XXXXXになります。
//  XXXXXはcdatに書いたINIT_OVERLAY_CMDに与えた引数です
//=====================================
//変換マクロ
#include "array_table_macro.h"
//cdat
#include "scrcmd_fourkings_table.cdat"
