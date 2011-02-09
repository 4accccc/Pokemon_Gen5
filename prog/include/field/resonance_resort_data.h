//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resonance_resort_data.h
 *	@brief  レゾナンスリゾートのデータ
 *	@author	Toru=Nagihashi
 *	@date		2011.02.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_beacon.h"
//=============================================================================
/**
 *	リゾート用データ
 *    
 *    １…リゾート用全受信データ
 *          全てのデータが入ったもの
 *        基本情報１
 *        基本情報２
 *        拡張情報１～MAX
 *
 *    ２…リゾート用ビーコンパックデータ
 *          ビーコンに入れるor受け取るデータ
 *        基本情報１
 *        基本情報２
 *        拡張情報N
 *
 *    ３…リゾート用代理データ
 *          GTSなどサーバーを介するときに入れるデータ
 *        基本情報１
 *        TODO
*/
//=============================================================================

//-------------------------------------
///	拡張データ
//=====================================
typedef enum
{
  RESORT_EXTEND_TYPE_01,
  RESORT_EXTEND_TYPE_02,
  RESORT_EXTEND_TYPE_03,
  RESORT_EXTEND_TYPE_04,
  RESORT_EXTEND_TYPE_05,
  RESORT_EXTEND_TYPE_06,
  RESORT_EXTEND_TYPE_07,
  RESORT_EXTEND_TYPE_08,
  RESORT_EXTEND_TYPE_MAX,
  RESORT_EXTEND_TYPE_NULL = RESORT_EXTEND_TYPE_MAX,
}RESORT_EXTEND_TYPE;

//-------------------------------------
///	定数
//=====================================
//リゾート用の基本データ１のサイズ
#define RESORT_BASIC1_DATA_SIZE   (32)
//リゾート用の基本データ２のサイズ
#define RESORT_BASIC2_DATA_SIZE   (4)
//リゾート用の拡張データ１つあたりのサイズ
#define RESORT_EXTEND_DATA_SIZE   (20)
//リゾート用全受信データの全サイズ
#define RESORT_RECV_DATA_SIZE     (RESORT_BASIC1_DATA_SIZE+RESORT_BASIC2_DATA_SIZE+RESORT_EXTEND_DATA_SIZE*RESORT_EXTEND_TYPE_MAX+4)

//-------------------------------------
///	１…リゾート用全受信データ
//=====================================
typedef struct _RESORT_RECV_DATA RESORT_RECV_DATA;

//-------------------------------------
///	２…リゾート用ビーコンパックデータ
//=====================================
typedef struct _RESORT_BEACON_DATA RESORT_BEACON_DATA;

//-------------------------------------
///	３…リゾート用代理データ
//=====================================
typedef struct _RESORT_PROXY_DATA RESORT_PROXY_DATA;


//-------------------------------------
///	１…リゾート用全受信データの外部公開
//=====================================
extern void RESORT_RECV_DATA_Clear( RESORT_RECV_DATA *p_wk );
extern BOOL RESORT_RECV_DATA_IsEmpty( const RESORT_RECV_DATA *cp_wk );
extern BOOL RESORT_RECV_DATA_IsSameOwnerBeacon( const RESORT_RECV_DATA *cp_wk, const RESORT_BEACON_DATA *cp_beacon );
extern void RESORT_RECV_DATA_MergeBeacon( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon );

//-------------------------------------
///	２…リゾート用ビーコンパックデータの外部公開
//=====================================
extern RESORT_BEACON_DATA * RESORT_BEACON_DATA_Init( HEAPID heapID );
extern void RESORT_BEACON_DATA_Exit( RESORT_BEACON_DATA *p_wk );
extern void RESORT_BEACON_DATA_SetMyData( RESORT_BEACON_DATA *p_wk, RESORT_EXTEND_TYPE type, const GAMEDATA *cp_gamedata );
extern void RESORT_BEACON_DATA_GetExtendData( const RESORT_BEACON_DATA *cp_wk, u8 *p_extend_buff, RESORT_EXTEND_TYPE *p_type );
extern void RESORT_BEACON_DATA_SetBeaconInfo( RESORT_BEACON_DATA *p_wk, const GAMEBEACON_INFO *cp_beacon_info );
extern BOOL RESORT_BEACON_DATA_IsValid( const RESORT_BEACON_DATA *cp_wk );


//=============================================================================
/**
 *	リゾート用データ受信バッファ（セーブデータにも存在）
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//受信バッファクラスのヘッダーサイズ
#define RESORT_DATA_BUFF_HEADER_SIZE (8)

//受信バッファクラスの全サイズ
#define RESORT_DATA_BUFF_SIZE( num )  (RESORT_DATA_BUFF_HEADER_SIZE+(RESORT_RECV_DATA_SIZE*num))

//-------------------------------------
///	受信バッファ
//=====================================
typedef struct _RESORT_DATA_BUFF RESORT_DATA_BUFF;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_DATA_BUFF * RESORT_DATA_BUFF_Init( HEAPID heapID, u32 num );
extern void RESORT_DATA_BUFF_Exit( RESORT_DATA_BUFF *p_wk );

extern void RESORT_DATA_BUFF_InitInstance( RESORT_DATA_BUFF *p_wk, u32 num );

extern void RESORT_DATA_BUFF_Lock( RESORT_DATA_BUFF *p_wk );
extern void RESORT_DATA_BUFF_UnLock( RESORT_DATA_BUFF *p_wk );

extern RESORT_RECV_DATA * RESORT_DATA_BUFF_GetData( RESORT_DATA_BUFF *p_wk, u32 index );
extern void RESORT_DATA_BUFF_Clear( RESORT_DATA_BUFF *p_wk, u32 index );

extern BOOL RESORT_DATA_BUFF_AddBeacon( RESORT_DATA_BUFF *p_wk, const RESORT_BEACON_DATA *cp_beacon );
