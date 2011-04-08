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
extern void FMissionView_InitialDraw( FMISSION_VIEW_PTR wk );

/*
 *  @brief  メインループ内での入力チェック
 */
extern int FMissionView_CheckInput( FMISSION_VIEW_PTR wk );

/*
 *  @brief  リスト上下ボタン　タッチ判定
 */
extern int FMissionView_CheckInoutTouchUpDown( FMISSION_VIEW_PTR wk );

/*
 *  @brief  特殊ポップアップ起動チェック 
 */
extern BOOL FMissionView_CheckSpecialPopup( FMISSION_VIEW_PTR wk );

/*
 *  @brief  Gパワーポップアップ起動チェック 
 */
extern BOOL FMissionView_CheckSpecialGPower( FMISSION_VIEW_PTR wk );

/*
 *  @brief  スタックからログを一件取り出し
 */
extern BOOL FMissionView_CheckStack( FMISSION_VIEW_PTR wk );

/**
 *  @brief  描画状態アップデート
 */
extern void FMissionView_ViewUpdate( FMISSION_VIEW_PTR wk );

/*
 *  @brief  サブシーケンス　Newログエントリーメイン
 */
extern BOOL FMissionView_SubSeqLogEntry( FMISSION_VIEW_PTR wk );

/*
 *  @brief  サブシーケンス　リタイアチェックメイン
 */
extern BOOL FMissionView_SubSeqRetire( FMISSION_VIEW_PTR wk );

//---------------------------------------------------
/*
 *  @brief  リストスクロールリクエスト
 */
//---------------------------------------------------
extern void FMissionView_ListScrollRepeatReq( FMISSION_VIEW_PTR wk );

//---------------------------------------------------
/*
 *  @brief  メニューバーアニメセット
 */
//---------------------------------------------------
extern void FMissionView_MenuBarViewSet( FMISSION_VIEW_PTR wk, MENU_ID id, MENU_STATE state );

//---------------------------------------------------
/*
 *  @brief  メニューバーアニメウェイト
 */
//---------------------------------------------------
extern BOOL FMissionView_MenuBarCheckAnm( FMISSION_VIEW_PTR wk, MENU_ID id );

//---------------------------------------------------
/*
 *  @brief  スクロールボタンアニメセット
 */
//---------------------------------------------------
extern void FMissionView_UpDownAnmSet( FMISSION_VIEW_PTR wk, SCROLL_DIR dir );

//---------------------------------------------------
/*
 *  @brief  スクロールボタン状態セット
 */
//---------------------------------------------------
extern void FMissionView_UpDownViewSet( FMISSION_VIEW_PTR wk );



