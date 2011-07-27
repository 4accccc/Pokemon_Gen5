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

//-----------------
// 宣言
//-----------------
// データ初期化
extern void TdCmn_init_data( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_exit_data( T_DOWNLOAD_MAIN_WORK* wk );

extern void TdCmn_init_bg_info( T_DOWNLOAD_MAIN_WORK* wk );

// 文字列
extern void TdCmn_disp_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, 
							   const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col );
extern void TdCmn_disp_string_number( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const s32 num, const s32 lnum, const u32 pmode );
extern void TdCmn_clear_canvas( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern void TdCmn_init_string2( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_exit_string2( T_DOWNLOAD_MAIN_WORK* wk );

// obj
extern void TdCmn_chg_objseq( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
extern void TdCmn_set_obj_visible( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
extern BOOL TdCmn_is_anime_end( T_DOWNLOAD_MAIN_WORK *wk, const s32 wmode, const s32 wnum );

// empty
extern s16  TdCmn_get_empty( T_DOWNLOAD_MAIN_WORK *wk, const s32 num );

// yes/no win
extern void TdCmn_init_yesnowin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern void TdCmn_exit_yesnowin( T_DOWNLOAD_MAIN_WORK* wk );
extern s32  TdCmn_chk_yesnowin( T_DOWNLOAD_MAIN_WORK* wk );
//extern void TdCmn_set_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mesid );
//extern BOOL TdCmn_main_meswin( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_disp_winmes( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 id );
extern void TdCmn_clear_frame( T_DOWNLOAD_MAIN_WORK* wk );

//
extern void TdCmn_set_cmn_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );

//
extern void TdCmn_main_bg_scroll( T_DOWNLOAD_MAIN_WORK * wk );

//
extern void TdCmn_disp_upmes( T_DOWNLOAD_MAIN_WORK * wk, const s32 num );

//
extern void TdCmn_init_framelist( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode );
extern u32  TdCmn_main_framelist( T_DOWNLOAD_MAIN_WORK * wk );
extern s32  TdCmn_is_main_framelist( T_DOWNLOAD_MAIN_WORK * wk, const u32 mode );
extern void TdCmn_exit_framelist( T_DOWNLOAD_MAIN_WORK * wk );
extern void TdCmn_calc_framelist( T_DOWNLOAD_MAIN_WORK * wk );
//
extern void TdCmn_disp_framelist_cursor( T_DOWNLOAD_MAIN_WORK * wk, const s32 num );
//
extern void TdCmn_main_chgpal( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_set_chgpal( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg );

//
extern void TdCmn_chg_info_curpos( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
//
extern void TdCmn_main_scrollbar( T_DOWNLOAD_MAIN_WORK * wk );
//
extern void TdCmn_disp_info_mode( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode );
extern void TdCmn_init_info_mode( T_DOWNLOAD_MAIN_WORK * wk );
extern s32  TdCmn_calc_scrollbar_info( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern void TdCmn_set_scrollbar_info( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 num );


