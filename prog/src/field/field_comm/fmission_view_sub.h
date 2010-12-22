//==============================================================================
/**
 * @file    fmission_view_sub.c
 * @brief   フェスミッション下画面 サブルーチン
 * @author  iwasawa 
 * @date    2010.12.21
 */
//==============================================================================

#pragma once

/*
 *  @brief  初期描画
 */
extern void FMissionView_InitialDraw( BEACON_VIEW_PTR wk );

/*
 *  @brief  メインループ内での入力チェック
 */
extern int FMissionView_CheckInput( BEACON_VIEW_PTR wk );

/*
 *  @brief  スタックからログを一件取り出し
 */
extern BOOL FMissionView_CheckStack( BEACON_VIEW_PTR wk );

/**
 *  @brief  描画状態アップデート
 */
extern void FMissionView_ViewUpdate( BEACON_VIEW_PTR wk );

/*
 *  @brief  サブシーケンス　Newログエントリーメイン
 */
extern BOOL FMissionView_SubSeqLogEntry( BEACON_VIEW_PTR wk );

