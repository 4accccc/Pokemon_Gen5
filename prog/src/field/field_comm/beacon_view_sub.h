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
 *  @brief  スタックからログを一件取り出し
 */
extern BOOL BeaconView_CheckStack( BEACON_VIEW_PTR wk );

/*
 *  @brief  メニューバーアニメセット
 */
extern void BeaconView_MenuBarViewSet( BEACON_VIEW_PTR wk, MENU_ID id, MENU_STATE state );

/*
 *  @brief  メニューバーアニメウェイト
 */
extern BOOL BeaconView_MenuBarCheckAnm( BEACON_VIEW_PTR wk, MENU_ID id );

