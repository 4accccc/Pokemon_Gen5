//======================================================================
/**
 * @file    field_beacon_message.h
 * @brief   フィールドでビーコンでメッセージをやり取りする
 * @author  ariizumi
 * @data    09/10/02
 *
 * モジュール名：FIELD_BEACON_MSG
 */
//======================================================================
#pragma once
#include "field/game_beacon_search.h"
#include "savedata/mystatus.h"

//======================================================================
//  define
//======================================================================
#pragma mark [> define

//======================================================================
//  enum
//======================================================================
#pragma mark [> enum


//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct

typedef struct _FIELD_BEACON_MSG_DATA FIELD_BEACON_MSG_DATA;
typedef struct _FIELD_BEACON_MSG_SYS FIELD_BEACON_MSG_SYS;

//======================================================================
//  proto
//======================================================================
#pragma mark [> proto


//--------------------------------------------------------------
//	データの作成・開放・初期化(名前セットのためMYSTATUSを渡す
//--------------------------------------------------------------
extern FIELD_BEACON_MSG_DATA* FIELD_BEACON_MSG_CreateData( const HEAPID heapId  , MYSTATUS *myStatus );
extern void FIELD_BEACON_MSG_DeleteData( FIELD_BEACON_MSG_DATA* fbmData );
extern void FIELD_BEACON_MSG_ResetData( FIELD_BEACON_MSG_DATA* fbmData  , MYSTATUS *myStatus );

//--------------------------------------------------------------
//	ビーコンチェック周り
//--------------------------------------------------------------
extern FIELD_BEACON_MSG_SYS* FIELD_BEACON_MSG_InitSystem( FIELD_BEACON_MSG_DATA* fbmData , const HEAPID heapId );
extern void FIELD_BEACON_MSG_ExitSystem( FIELD_BEACON_MSG_SYS* fbmSys );
extern void FIELD_BEACON_MSG_SetBeaconMessage( FIELD_BEACON_MSG_SYS *fbmSys , GBS_BEACON *beacon );
extern void FIELD_BEACON_MSG_CheckBeacon( FIELD_BEACON_MSG_SYS *fbmSys , GBS_BEACON *beacon , u8 *macAddress );

//--------------------------------------------------------------
//	メッセージ系
//--------------------------------------------------------------
extern void FIELD_BEACON_MESSAGE_SetWord( FIELD_BEACON_MSG_DATA *fbmData , u16 *word );
