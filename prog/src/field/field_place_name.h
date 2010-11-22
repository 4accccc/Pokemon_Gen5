///////////////////////////////////////////////////////////////////////////////
/**
 * @file   field_place_name.h
 * @brief  地名表示ウィンドウ
 * @author obata_toshihiro
 * @date   2009.07
 */
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "field/field_msgbg.h"
#include "gamesystem/gamesystem.h"


#ifdef PM_DEBUG
extern BOOL PlaceNameEnable; // FALSE にすると動作しなくなる
extern BOOL PlaceNameLong;  //TRUEにすると固定最大文字列
extern u32 PlaceNameMoveType;  //動きの種類 0,上下同時1,上、下2下、上
extern BOOL PlaceNameShakeFlag;  //動きの揺れ追加
extern BOOL PlaceNameAlphaFlag;  //アルファの種類  
extern u32 PROCESS_TIME_FADEIN2; //入ってくるときの速度
extern u32 PROCESS_TIME_WAIT_LAUNCH; //待つときの時間
extern u32 PROCESS_TIME_WAIT_FADEOUT; //出て行く時の速度
extern u32 ALPHA_VALUE_1;
extern u32 ALPHA_VALUE_2;
extern BOOL PlaceNameVersionFlag;    //0がブラック１がホワイト
#endif 

typedef struct _FIELD_PLACE_NAME FIELD_PLACE_NAME;

// 生成・破棄
extern FIELD_PLACE_NAME* FIELD_PLACE_NAME_Create( GAMESYS_WORK* gameSystem, HEAPID heapID, FLDMSGBG* msgbg );
extern void FIELD_PLACE_NAME_Delete( FIELD_PLACE_NAME* system );
// メイン・描画
extern void FIELD_PLACE_NAME_Process( FIELD_PLACE_NAME* system );
extern void FIELD_PLACE_NAME_Draw( const FIELD_PLACE_NAME* system );
// 制御
extern void FIELD_PLACE_NAME_DisplayOnStanderdRule( FIELD_PLACE_NAME* system, u32 zoneID );
extern void FIELD_PLACE_NAME_DisplayOnPlaceNameFlag( FIELD_PLACE_NAME* system, u32 zoneID );
extern void FIELD_PLACE_NAME_DisplayContinue( FIELD_PLACE_NAME* system, u32 zoneID );
extern void FIELD_PLACE_NAME_DisplayForce( FIELD_PLACE_NAME* system, u32 zoneID );
extern void FIELD_PLACE_NAME_Hide( FIELD_PLACE_NAME* system );
extern void FIELD_PLACE_NAME_RecoverBG( FIELD_PLACE_NAME* system );

#ifdef PM_DEBUG
extern void FIELD_PLACE_NAME_DebugChangeVersion( FIELD_PLACE_NAME* system );
#endif 
