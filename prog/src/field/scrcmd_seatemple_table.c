//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_seatemple_table.c
 *	@brief  海底神殿用スクリプトコマンドテーブル
 *	@author	Toru=Nagihashi
 *	@data		2010.10.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
//スクリプトコマンド
#include "scrcmd_seatemple.h"
//外部参照
#include "scrcmd_seatemple_table.h"

//-------------------------------------
///	テーブルをマクロで自動生成
//  テーブル名は ScriptCmdTbl_XXXXXになります。
//  XXXXXはcdatに書いたINIT_OVERLAY_CMDに与えた引数です
//=====================================
//変換マクロ
#include "array_table_macro.h"
//cdat
#include "scrcmd_seatemple_table.cdat"
