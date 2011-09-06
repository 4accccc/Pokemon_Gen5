//=============================================================================
/**
 *	@file		fmission_app_sub.h
 *	@brief  フェスミッションアプリ サブ
 *	@author Miyuki Iwasawa	
 *	@data	  2011.04.22	
 */
//=============================================================================

#pragma once

// dataの初期化
extern void FmaCmn_data_init( FMISSION_APP_WORK* wk, const s32 mode );
extern void FmaCmn_data_exit( FMISSION_APP_WORK* wk );

// 文字列の表示
extern void FmaCmn_disp_string( FMISSION_APP_WORK* wk, const s32 mode, const s32 x, const s32 y, 
								const STRBUF* strbuf, const u32 pmode, const PRINTSYS_LSB col );
extern void FmaCmn_clear_canvas( FMISSION_APP_WORK* wk, const s32 mode );
//
// emptyの取得
extern s16 FmaCmn_get_empty( FMISSION_APP_WORK *wk, const s32 num );

//
extern void FmaCmn_chg_objseq( FMISSION_APP_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
extern void FmaCmn_set_obj_visible( FMISSION_APP_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
extern BOOL FmaCmn_is_anime_end( FMISSION_APP_WORK *wk, const s32 wmode, const s32 wnum );
//
extern void FmaCmn_disp_number( FMISSION_APP_WORK *wk, const s32 wmode, const s32 src_wnum, const s32 num, const s32 snum, const s32 smode,
								const s32 x, const s32 y );
extern void FmaCmn_disp_time( FMISSION_APP_WORK *wk, const s32 wmode, const s32 src_wnum, const s32 src0, const s32 src1 );
//
extern void FmaCmn_disp_star( FMISSION_APP_WORK *wk );
//
extern s32  FmaCmn_chk_tp( FMISSION_APP_WORK *wk );

