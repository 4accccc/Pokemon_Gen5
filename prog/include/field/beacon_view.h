//==============================================================================
/**
 * @file    beacon_view.h
 * @brief   すれ違い通信状態参照画面
 * @author  matsuda
 * @date    2009.12.13(日)
 */
//==============================================================================
#pragma once


//==============================================================================
//  型定義
//==============================================================================
typedef struct _BEACON_VIEW * BEACON_VIEW_PTR;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern BEACON_VIEW_PTR BEACON_VIEW_Init(GAMESYS_WORK *gsys, FIELD_SUBSCREEN_WORK *subscreen);
extern void BEACON_VIEW_Exit(BEACON_VIEW_PTR view);
extern void BEACON_VIEW_Update(BEACON_VIEW_PTR view, BOOL bActive);
extern void BEACON_VIEW_Draw(BEACON_VIEW_PTR view);

