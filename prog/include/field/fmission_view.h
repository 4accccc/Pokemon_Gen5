//==============================================================================
/**
 * @file    fmission_view.h
 * @brief   フェスミッションログ画面
 * @author  iwasawa 
 * @date    2011.03.04
 */
//==============================================================================
#pragma once

FS_EXTERN_OVERLAY(beacon_view_common);
FS_EXTERN_OVERLAY(fmission_view);

//==============================================================================
//  型定義
//==============================================================================
typedef struct _FMISSION_VIEW * FMISSION_VIEW_PTR;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern FMISSION_VIEW_PTR FMISSION_VIEW_Init(GAMESYS_WORK *gsys, FIELD_SUBSCREEN_WORK *subscreen);
extern void FMISSION_VIEW_Exit(FMISSION_VIEW_PTR view);
extern void FMISSION_VIEW_Update(FMISSION_VIEW_PTR view, BOOL bActive);
extern void FMISSION_VIEW_Draw(FMISSION_VIEW_PTR view);
extern GMEVENT* FMISSION_VIEW_EventCheck(FMISSION_VIEW_PTR wk, BOOL bEvReqOK );

