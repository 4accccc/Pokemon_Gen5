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
 *  @brief  スタックからログを一件取り出し
 */
extern BOOL BeaconView_CheckStack( BEACON_VIEW_PTR wk );


