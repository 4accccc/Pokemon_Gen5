//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_gimmick_sppoke_table.c
 *	@brief  特殊ポケに関するギミック スクリプトコマンドテーブル
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//system
#include <gflib.h>
//スクリプトコマンド
#include "scrcmd_gimmick_sppoke.h"
//外部参照
#include "scrcmd_gimmick_sppoke_table.h"

//-------------------------------------
///	テーブルをマクロで自動生成
//  テーブル名は ScriptCmdTbl_XXXXXになります。
//  XXXXXはcdatに書いたINIT_OVERLAY_CMDに与えた引数です
//=====================================
//変換マクロ
#include "array_table_macro.h"
//cdat
#include "scrcmd_gimmick_sppoke_table.cdat"
