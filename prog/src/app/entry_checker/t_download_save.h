//=============================================================================
/**
 *
 *	@file		t_download_save.h
 *	@brief  	トーナメントダウンロード画面(save関連)
 *	@author
 *	@date
 *
 */
//=============================================================================
#pragma once

//-----------------
// 定義
//-----------------
enum{
	T_DOWNLOAD_SAVE_STR_NAME 		= 0,		// トーナメント名
	T_DOWNLOAD_SAVE_STR_EXPLAIN_00,				// 内容説明
	T_DOWNLOAD_SAVE_STR_EXPLAIN_01,				// レギュレーション説明
};

enum{
	T_DOWNLOAD_SAVE_ITEM_POKE		= 0,		// ポケモン
	T_DOWNLOAD_SAVE_ITEM_ITEM,					// アイテム
	T_DOWNLOAD_SAVE_ITEM_SKILL,					// スキル
};

//-----------------
// 宣言
//-----------------
extern s32  TdSave_init_data( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdSave_exit_data( T_DOWNLOAD_MAIN_WORK* wk );

extern s32  TdSave_init_list_data( T_DOWNLOAD_MAIN_WORK* wk );

extern void TdSave_get_str( T_DOWNLOAD_MAIN_WORK* wk, const WBTDL_DATA* wbtdl, const s32 mode, STRBUF* strbuf );
extern s32  TdSave_get_item_num( T_DOWNLOAD_MAIN_WORK* wk, const WBTDL_DATA* wbtdl, const s32 mode );
extern s32  TdSave_get_item_id( T_DOWNLOAD_MAIN_WORK* wk, const WBTDL_DATA* wbtdl, const s32 mode, const s32 inum );

extern void TdSave_init_save( T_DOWNLOAD_MAIN_WORK* wk, const s32 src, const s32 dest );
extern BOOL TdSave_main_save( T_DOWNLOAD_MAIN_WORK* wk );

extern void TdSave_init_timeicon( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdSave_main_timeicon( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdSave_exit_timeicon( T_DOWNLOAD_MAIN_WORK* wk );

extern s32  TdSave_get_save_pos( T_DOWNLOAD_MAIN_WORK* wk );
extern BOOL TdSave_chk_save_list( T_DOWNLOAD_MAIN_WORK * wk, const s32 num );
//
extern s32  TdSave_get_slotpos( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern s32  TdSave_get_flag_curpos( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 curpos );


