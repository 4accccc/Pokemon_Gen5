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

extern void TdCmn_init_sub_bg( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );

// 文字列
extern void TdCmn_disp_string( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, 
							   const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col );
extern void TdCmn_clear_canvas( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern void TdCmn_init_string2( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern void TdCmn_exit_string2( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );

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
//
extern void TdCmn_set_meswin( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 id );
extern BOOL TdCmn_main_meswin( T_DOWNLOAD_MAIN_WORK* wk );
//
extern void TdCmn_disp_winmes( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 id, const s32 smode );
extern void TdCmn_clear_frame( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
//
extern void TdCmn_set_cmn_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 smode );
//
extern void TdCmn_main_bg_scroll( T_DOWNLOAD_MAIN_WORK * wk );
//
extern void TdCmn_disp_upmes( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num );
//
// フレーム
extern void TdCmn_init_framelist( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode );
extern u32  TdCmn_main_framelist( T_DOWNLOAD_MAIN_WORK * wk );
extern s32  TdCmn_is_main_framelist( T_DOWNLOAD_MAIN_WORK * wk, const u32 mode );
extern void TdCmn_exit_framelist( T_DOWNLOAD_MAIN_WORK * wk );
extern void TdCmn_calc_framelist( T_DOWNLOAD_MAIN_WORK * wk );
extern void TdCmn_disp_panel_cursor( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num );
extern void TdCmn_set_panel_cursor( T_DOWNLOAD_MAIN_WORK * wk, const s32 smode );
//
// パレットチェンジ
extern void TdCmn_main_chgpal( T_DOWNLOAD_MAIN_WORK* wk );
extern void TdCmn_set_chgpal( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg );
//
extern void TdCmn_chg_info_curpos( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern void TdCmn_chk_info_disp_scrollbar( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
//
extern void TdCmn_main_scrollbar( T_DOWNLOAD_MAIN_WORK * wk );
//
extern void TdCmn_disp_info_mode( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num, const s32 snum );
extern void TdCmn_init_info_mode( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num );

extern s32  TdCmn_calc_scrollbar_info( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
extern void TdCmn_set_scrollbar_info( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 num );
//
extern void TdCmn_disp_title_info( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 smode );
//
extern s32  TdCmn_chk_tp( T_DOWNLOAD_MAIN_WORK *wk, const s32 tpmode, const s32 mode );
//
extern void TdCmn_set_passive( T_DOWNLOAD_MAIN_WORK* wk, const BOOL bflg );
//
extern void TdCmn_main_keycursor( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );
//
extern void TdCmn_stop_anim_speed( T_DOWNLOAD_MAIN_WORK *wk, const BOOL bflg );
//
extern void TdCmn_disp_panel_string( T_DOWNLOAD_MAIN_WORK * wk, const s32 mode, const s32 num );
//
extern void TdCmn_chk_up_disp( T_DOWNLOAD_MAIN_WORK * wk, const BOOL keyflg, const BOOL tpflg, const BOOL fflg );
//
extern void TdCmn_chk_mark_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 mmin, const s32 mmax, const s32 nnum );
extern BOOL TdCmn_chk_anim( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode );


