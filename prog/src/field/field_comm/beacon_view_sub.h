/**
 *  @file   beacon_view_sub.h
 *  @brief  すれ違い通信画面サブ関数群ヘッダ
 *  @author Miyuki Iwasawa
 *  @date   10.01.19
 */

#pragma once

/*
 *  @brief  初期描画
 */
extern void BeaconView_InitialDraw( BEACON_VIEW_PTR wk );

/*
 *  @brief  メインループ内での入力チェック
 */
extern int BeaconView_CheckInput( BEACON_VIEW_PTR wk );

/*
 *  @brief  リスト上下ボタン　タッチ判定
 */
extern int BeaconView_CheckInoutTouchUpDown( BEACON_VIEW_PTR wk );

/*
 *  @brief  御礼対象選択ループ内での入力チェック
 */
extern int BeaconView_CheckInputThanks( BEACON_VIEW_PTR wk );

/*
 *  @brief  通信関連特殊イベント起動チェック 
 */
extern BOOL BeaconView_CheckCommEvent( BEACON_VIEW_PTR wk );

/*
 *  @brief  特殊ポップアップ起動チェック 
 */
extern BOOL BeaconView_CheckSpecialPopup( BEACON_VIEW_PTR wk );

/*
 *  @brief  特殊Gパワー発動ポップアップ起動チェック 
 */
extern BOOL BeaconView_CheckSpecialGPower( BEACON_VIEW_PTR wk );

/*
 *  @brief  スタックからログを一件取り出し
 */
extern BOOL BeaconView_CheckStack( BEACON_VIEW_PTR wk );


//---------------------------------------------------
/*
 *  @brief  リストスクロールリクエスト
 */
//---------------------------------------------------
extern void BeaconView_ListScrollRepeatReq( BEACON_VIEW_PTR wk );


/*
 *  @brief  サブシーケンス　Newログエントリーメイン
 */
extern BOOL BeaconView_SubSeqLogEntry( BEACON_VIEW_PTR wk );

/*
 *  @brief  サブシーケンス　GPower使用メイン
 */
extern BOOL BeaconView_SubSeqGPower( BEACON_VIEW_PTR wk );

/*
 *  @brief  メニューバーアニメセット
 */
extern void BeaconView_MenuBarViewSet( BEACON_VIEW_PTR wk, MENU_ID id, MENU_STATE state );

/*
 *  @brief  メニューバーアニメウェイト
 */
extern BOOL BeaconView_MenuBarCheckAnm( BEACON_VIEW_PTR wk, MENU_ID id );

/*
 *  @brief  サブシーケンス　御礼メイン
 */
extern int BeaconView_SubSeqThanks( BEACON_VIEW_PTR wk );


/*
 *  @brief  スクロールボタンアニメセット
 */
extern void BeaconView_UpDownAnmSet( BEACON_VIEW_PTR wk, SCROLL_DIR dir );

/*
 *  @brief  スクロールボタン状態セット
 */
extern void BeaconView_UpDownViewSet( BEACON_VIEW_PTR wk );

/*
 *  @brief  ログ内カウンター収集
 */
extern void BeaconView_LogCounterGet( BEACON_VIEW_PTR wk, u8* sex_ct, u8* version_ct);

