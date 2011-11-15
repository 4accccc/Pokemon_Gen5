//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_gimmick_out58_table.c
 *	@brief  OUT58に関するギミック スクリプトコマンドテーブル
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.14
 *
 *	@note   C14,R22,W21のゾーンで使用するスクリプトコマンドを集約しています
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//system
#include <gflib.h>
//スクリプトコマンド
#include "scrcmd_gimmick_out58.h"
//外部参照
#include "scrcmd_gimmick_out58_table.h"

//-------------------------------------
///	テーブルをマクロで自動生成
//  テーブル名は ScriptCmdTbl_XXXXXになります。
//  XXXXXはcdatに書いたINIT_OVERLAY_CMDに与えた引数です
//=====================================
//変換マクロ
#include "array_table_macro.h"
//cdat
#include "scrcmd_gimmick_out58_table.cdat"
