//=============================================================================
/**
 *
 *	@file		t_download_sub.h
 *	@brief  トーナメントダウンロード画面(sub)
 *	@author
 *	@date
 *
 */
//=============================================================================
#pragma once

// データ初期化
extern void TdCmn_init_data( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_exit_data( T_DOWNLOAD_MAIN_WORK* wk );

// 文字列
extern void TdCmn_disp_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, 
							   const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col );
extern void TdCmn_disp_string_number( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum, const u32 pmode );
extern void TdCmn_clear_canvas( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );

// obj
extern void TdCmn_chg_objseq( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
extern void TdCmn_set_obj_visible( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
extern BOOL TdCmn_is_anime_end( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum );

// bmpoam
extern void TdCmn_disp_bmpoam( T_DOWNLOAD_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf );

// empty
extern s16  TdCmn_get_empty( T_DOWNLOAD_MAIN_WORK *wk, const s32 num );

// yes/no win
extern void TdCmn_init_yesnowin( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_exit_yesnowin( T_DOWNLOAD_MAIN_WORK* wk );
extern s32  TdCmn_chk_yesnowin( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_set_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid );
extern BOOL TdCmn_main_meswin( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_init_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern void TdCmn_disp_centermes( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid );



