//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resonance_resort_data.h
 *	@brief  レゾナンスリゾートのデータ
 *	@author	Toru=Nagihashi
 *	@date		2011.02.07
 *
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_beacon.h"
#include "src/field/resonance_resort/resort_people_def.h"
//=============================================================================
/**
 *	リゾート用受信データ
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
#define RESORT_EXTEND_DATA_SIZE   (16)
#define RESORT_EXTEND1_DATA_SIZE   (16)
#define RESORT_EXTEND2_DATA_SIZE   (16)
#define RESORT_EXTEND3_DATA_SIZE   (16)
#define RESORT_EXTEND4_DATA_SIZE   (16)
#define RESORT_EXTEND5_DATA_SIZE   (16)
#define RESORT_EXTEND6_DATA_SIZE   (16)
#define RESORT_EXTEND7_DATA_SIZE   (16)
#define RESORT_EXTEND8_DATA_SIZE   (16)


//リゾート用全受信データの全サイズ
#define RESORT_RECV_DATA_SIZE     (RESORT_BASIC1_DATA_SIZE+RESORT_BASIC2_DATA_SIZE+RESORT_EXTEND_DATA_SIZE*RESORT_EXTEND_TYPE_MAX+28)

//リゾート用代理データの全サイズ
#define RESORT_PROXY_DATA_SIZE    (8)

#define RESORT_PEOPLE_ID_NULL (0xFFFF)
//-------------------------------------
///	リゾート用受信データの取得引数
//=====================================
typedef enum 
{
  RESORT_RECV_DATA_PARAM_PEOPLE_ID,
  RESORT_RECV_DATA_PARAM_PLACE_ID,
  RESORT_RECV_DATA_PARAM_LOCK,
  RESORT_RECV_DATA_PARAM_UPDATE,
  RESORT_RECV_DATA_PARAM_SEX,
  RESORT_RECV_DATA_PARAM_TRAINERVIEW,
  RESORT_RECV_DATA_PARAM_NAME,          //p_ret_adrsから取得
  RESORT_RECV_DATA_PARAM_KEEP,
} RESORT_RECV_DATA_PARAM;

//-------------------------------------
///	リゾート用受信データの更新フラグ
//    ここを増やしたら、RESORT_RECV_DATAのupdateのbitを増やすこと
//=====================================
typedef enum
{
  RESORT_RECV_DATA_UPDATE_NONE,
  RESORT_RECV_DATA_UPDATE_FIRST,  //初期アップデート
  RESORT_RECV_DATA_UPDATE_REPLACE,//置き換え（初期アップデート）
  RESORT_RECV_DATA_UPDATE_MERGE,  //マージした

  RESORT_RECV_DATA_UPDATE_MAX,
} RESORT_RECV_DATA_UPDATE;


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
extern RESORT_RECV_DATA * RESORT_RECV_DATA_Init( HEAPID heapID );
extern void RESORT_RECV_DATA_Exit( RESORT_RECV_DATA *p_wk );
extern void RESORT_RECV_DATA_SetMyData( RESORT_RECV_DATA *p_wk, const GAMEDATA *cp_gamedata );
extern void RESORT_RECV_DATA_InitInstance( RESORT_RECV_DATA *p_wk );
extern BOOL RESORT_RECV_DATA_IsEmpty( const RESORT_RECV_DATA *cp_wk );
extern BOOL RESORT_RECV_DATA_IsSameOwnerBeacon( const RESORT_RECV_DATA *cp_wk, const RESORT_BEACON_DATA *cp_beacon );
extern void RESORT_RECV_DATA_MergeBeacon( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon );
extern void RESORT_RECV_DATA_MergeRecv( RESORT_RECV_DATA *p_wk, const RESORT_RECV_DATA *cp_recv );
extern BOOL RESORT_RECV_DATA_IsValid( const RESORT_RECV_DATA *cp_wk );

extern u32 RESORT_RECV_DATA_GetData( const RESORT_RECV_DATA *cp_wk, RESORT_RECV_DATA_PARAM param, void *p_ret_adrs );
extern void RESORT_RECV_DATA_SetData( RESORT_RECV_DATA *p_wk, RESORT_RECV_DATA_PARAM param, u32 data );

#ifdef PM_DEBUG
extern void RESORT_RECV_DATA_SetDebugData( RESORT_RECV_DATA *p_wk, const GAMEDATA *cp_gamedata );
#endif

//-------------------------------------
///	２…リゾート用ビーコンパックデータの外部公開
//=====================================
extern RESORT_BEACON_DATA * RESORT_BEACON_DATA_Init( HEAPID heapID );
extern void RESORT_BEACON_DATA_Exit( RESORT_BEACON_DATA *p_wk );
extern void RESORT_BEACON_DATA_SetMyData( RESORT_BEACON_DATA *p_wk, RESORT_EXTEND_TYPE type, const GAMEDATA *cp_gamedata );
extern void RESORT_BEACON_DATA_GetExtendData( const RESORT_BEACON_DATA *cp_wk, u8 *p_extend_buff, RESORT_EXTEND_TYPE *p_type );
extern void RESORT_BEACON_DATA_SetBeaconInfo( RESORT_BEACON_DATA *p_wk, const GAMEBEACON_INFO *cp_beacon_info );
extern BOOL RESORT_BEACON_DATA_IsValid( const RESORT_BEACON_DATA *cp_wk );

//-------------------------------------
///	２…リゾート用代理データの外部公開
//=====================================
extern RESORT_PROXY_DATA *RESORT_PROXY_DATA_Init( HEAPID heapID );
extern void RESORT_PROXY_DATA_Exit( RESORT_PROXY_DATA *p_wk );
extern void RESORT_PROXY_DATA_InitInstance( RESORT_PROXY_DATA *p_wk );
extern BOOL RESORT_PROXY_DATA_IsEmpty( const RESORT_PROXY_DATA *cp_wk );

extern void RESORT_PROXY_DATA_SetCommData( RESORT_PROXY_DATA *p_wk, u16 type );

typedef enum
{
  RESORT_PROXY_DATA_PARAM_PEOPLEID,
  RESORT_PROXY_DATA_PARAM_PLACEID,
}RESORT_PROXY_DATA_PARAM;
extern u16 RESORT_PROXY_DATA_GetData( const RESORT_PROXY_DATA *cp_wk, RESORT_PROXY_DATA_PARAM param );
extern void RESORT_PROXY_DATA_SetData( RESORT_PROXY_DATA *p_wk, RESORT_PROXY_DATA_PARAM param, u32 data );

//=============================================================================
/**
 *	リゾート代理データバッファ（セーブデータしか存在しない）
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//受信バッファクラスのヘッダーサイズ
#define RESORT_PROXY_BUFF_HEADER_SIZE (4)

//受信バッファクラスの全サイズ
#define RESORT_PROXY_BUFF_SIZE( num )  (RESORT_PROXY_BUFF_HEADER_SIZE+(RESORT_PROXY_DATA_SIZE*num))

//-------------------------------------
///	受信バッファ
//=====================================
typedef struct _RESORT_PROXY_BUFF RESORT_PROXY_BUFF;

//-------------------------------------
///	外部公開
//=====================================
extern void RESORT_PROXY_BUFF_InitInstance( RESORT_PROXY_BUFF *p_wk, u32 num );

extern void RESORT_PROXY_BUFF_Clear( RESORT_PROXY_BUFF *p_wk, u32 index );

extern BOOL RESORT_PROXY_BUFF_Add( RESORT_PROXY_BUFF *p_wk, const RESORT_PROXY_DATA *cp_proxy );
extern u32 RESORT_PROXY_BUFF_GetMax( const RESORT_PROXY_BUFF *cp_wk );
extern RESORT_PROXY_DATA * RESORT_PROXY_BUFF_GetData( RESORT_PROXY_BUFF *p_wk, u32 index );


//=============================================================================
/**
 *	リゾート受信データバッファ（セーブデータにも存在）
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//受信バッファクラスのヘッダーサイズ
#define RESORT_RECV_BUFF_HEADER_SIZE (8)

//受信バッファクラスの全サイズ
#define RESORT_RECV_BUFF_SIZE( num )  (RESORT_RECV_BUFF_HEADER_SIZE+(RESORT_RECV_DATA_SIZE*num))

//-------------------------------------
///	受信バッファ
//=====================================
typedef struct _RESORT_RECV_BUFF RESORT_RECV_BUFF;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_RECV_BUFF * RESORT_RECV_BUFF_Init( HEAPID heapID, u32 num );
extern void RESORT_RECV_BUFF_Exit( RESORT_RECV_BUFF *p_wk );

extern void RESORT_RECV_BUFF_InitInstance( RESORT_RECV_BUFF *p_wk, u32 num );

extern void RESORT_RECV_BUFF_Lock( RESORT_RECV_BUFF *p_wk );
extern void RESORT_RECV_BUFF_UnLock( RESORT_RECV_BUFF *p_wk );
extern u32 RESORT_RECV_BUFF_GetMax( const RESORT_RECV_BUFF *cp_wk );

extern RESORT_RECV_DATA * RESORT_RECV_BUFF_GetData( RESORT_RECV_BUFF *p_wk, u32 index );
extern void RESORT_RECV_BUFF_Clear( RESORT_RECV_BUFF *p_wk, u32 index );

extern BOOL RESORT_RECV_BUFF_AddBeacon( RESORT_RECV_BUFF *p_wk, const RESORT_BEACON_DATA *cp_beacon );
extern BOOL RESORT_RECV_BUFF_AddRecv( RESORT_RECV_BUFF *p_wk, const RESORT_RECV_DATA *cp_recv );


//=============================================================================
/**
 *  住民データ
 */
//=============================================================================

//-------------------------------------
///	住民データ
//    (住民データと受信データは同じ)
//=====================================
typedef RESORT_RECV_DATA RESORT_RESIDENT_DATA;

//-------------------------------------
///	外部公開
//=====================================
#define RESORT_RESIDENT_DATA_Init( x ) RESORT_RECV_DATA_Init( x )
#define RESORT_RESIDENT_DATA_Exit( x ) RESORT_RECV_DATA_Exit( x )
#define RESORT_RESIDENT_DATA_InitInstance( x ) RESORT_RECV_DATA_InitInstance( x )
#define RESORT_RESIDENT_DATA_IsEmpty( x ) RESORT_RECV_DATA_IsEmpty( x )
#define RESORT_RESIDENT_DATA_GetData( x, y, z ) RESORT_RECV_DATA_GetData( x, y, z )
#define RESORT_RESIDENT_DATA_SetData( x, y, z )  RESORT_RECV_DATA_SetData( x, y, z )

extern void RESORT_RESIDENT_DATA_SetRecv( RESORT_RESIDENT_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID );

//=============================================================================
/**
 *  滞在者データ
 */
//=============================================================================
#define RESORT_VISITOR_DATA_SIZE  (RESORT_BASIC1_DATA_SIZE+RESORT_BASIC2_DATA_SIZE+RESORT_EXTEND4_DATA_SIZE+4)
//-------------------------------------
///	滞在者データ
//=====================================
typedef struct _RESORT_VISITOR_DATA RESORT_VISITOR_DATA;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_VISITOR_DATA * RESORT_VISITOR_DATA_Init( HEAPID heapID );
extern void RESORT_VISITOR_DATA_Exit( RESORT_VISITOR_DATA *p_wk );
extern void RESORT_VISITOR_DATA_InitInstance( RESORT_VISITOR_DATA *p_wk );

extern BOOL RESORT_VISITOR_DATA_IsEmpty( const RESORT_VISITOR_DATA *cp_wk );
extern void RESORT_VISITOR_DATA_SetRecv( RESORT_VISITOR_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID );
extern void RESORT_VISITOR_DATA_SetResident( RESORT_VISITOR_DATA *p_wk, const RESORT_RESIDENT_DATA *cp_resident, u16 peopleID, u16 placeID );

typedef enum
{
  RESORT_VISITOR_DATA_PARAM_PEOPLE_ID,
  RESORT_VISITOR_DATA_PARAM_PLACE_ID,
  RESORT_VISITOR_DATA_PARAM_NAME,
  RESORT_VISITOR_DATA_PARAM_SEX,
  RESORT_VISITOR_DATA_PARAM_TRAINERVIEW,
}RESORT_VISITOR_DATA_PARAM;

extern u32 RESORT_VISITOR_DATA_GetData( const RESORT_VISITOR_DATA *cp_wk, RESORT_VISITOR_DATA_PARAM param, void *p_ret_adrs );
extern void RESORT_VISITOR_DATA_SetData( RESORT_VISITOR_DATA *p_wk, RESORT_VISITOR_DATA_PARAM param, u32 data );

//=============================================================================
/**
 *  秘書データ
 */
//=============================================================================
#define RESORT_SECRETARY_DATA_SIZE  (RESORT_BASIC1_DATA_SIZE+RESORT_BASIC2_DATA_SIZE+4)

//-------------------------------------
///	秘書データ
//=====================================
typedef struct _RESORT_SECRETARY_DATA RESORT_SECRETARY_DATA;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_SECRETARY_DATA * RESORT_SECRETARY_DATA_Init( HEAPID heapID );
extern void RESORT_SECRETARY_DATA_Exit( RESORT_SECRETARY_DATA *p_wk );
extern void RESORT_SECRETARY_DATA_InitInstance( RESORT_SECRETARY_DATA *p_wk );

extern BOOL RESORT_SECRETARY_DATA_IsEmpty( const RESORT_SECRETARY_DATA *cp_wk );

typedef enum
{
  RESORT_SECRETARY_DATA_PARAM_PEOPLE_ID,
  RESORT_SECRETARY_DATA_PARAM_PLACE_ID,
  RESORT_SECRETARY_DATA_PARAM_NAME,
  RESORT_SECRETARY_DATA_PARAM_SEX,
  RESORT_SECRETARY_DATA_PARAM_TRAINERVIEW,
}RESORT_SECRETARY_DATA_PARAM;
extern u32 RESORT_SECRETARY_DATA_GetData( const RESORT_SECRETARY_DATA *cp_wk, RESORT_SECRETARY_DATA_PARAM param, void *p_ret_adrs );
extern void RESORT_SECRETARY_DATA_SetData( RESORT_SECRETARY_DATA *p_wk, RESORT_SECRETARY_DATA_PARAM param, u32 data );
extern void RESORT_SECRETARY_DATA_SetRecv( RESORT_SECRETARY_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID );

//=============================================================================
/**
 *  施設データ
 */
//=============================================================================
//施設ワークサイズ
#define RESORT_INSTITUTION_DATA_SIZE  (12)

//-------------------------------------
///	施設データ
//=====================================
typedef struct _RESORT_INSTITUTION_DATA RESORT_INSTITUTION_DATA;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_INSTITUTION_DATA * RESORT_INSTITUTION_DATA_Init( HEAPID heapID );
extern void RESORT_INSTITUTION_DATA_Exit( RESORT_INSTITUTION_DATA *p_wk );
extern void RESORT_INSTITUTION_DATA_InitInstance( RESORT_INSTITUTION_DATA *p_wk );

//=============================================================================
/**
 *	リゾート人物全データ
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//住人データ数
#define RESORT_RESIDENT_DATA_NUM    (8)
//滞在者データ数
#define RESORT_VISITOR_DATA_NUM     (6)
//秘書データ数
#define RESORT_SECRETARY_DATA_NUM   (5)

//人物データ総サイズ
#define RESORT_PEOPLE_ALL_DATA_SIZE       (RESORT_RECV_DATA_SIZE*RESORT_RESIDENT_DATA_NUM+RESORT_VISITOR_DATA_SIZE*RESORT_VISITOR_DATA_NUM+RESORT_SECRETARY_DATA_SIZE*RESORT_SECRETARY_DATA_NUM)

//-------------------------------------
///	街データ
//=====================================
typedef struct _RESORT_PEOPLE_ALL_DATA RESORT_PEOPLE_ALL_DATA;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_PEOPLE_ALL_DATA * RESORT_PEOPLE_ALL_DATA_Init( HEAPID heapID );
extern void RESORT_PEOPLE_ALL_DATA_Exit( RESORT_PEOPLE_ALL_DATA *p_wk );
extern void RESORT_PEOPLE_ALL_DATA_InitInstance( RESORT_PEOPLE_ALL_DATA *p_wk );
extern RESORT_RESIDENT_DATA * RESORT_PEOPLE_ALL_DATA_GetResident( RESORT_PEOPLE_ALL_DATA *p_wk, u32 index );
extern RESORT_RESIDENT_DATA * RESORT_PEOPLE_ALL_DATA_SearchEmptyResident( RESORT_PEOPLE_ALL_DATA *p_wk );
extern RESORT_VISITOR_DATA * RESORT_PEOPLE_ALL_DATA_GetVisitor( RESORT_PEOPLE_ALL_DATA *p_wk, u32 index );
extern RESORT_VISITOR_DATA * RESORT_PEOPLE_ALL_DATA_SearchEmptyVistor( RESORT_PEOPLE_ALL_DATA *p_wk );
extern RESORT_SECRETARY_DATA * RESORT_PEOPLE_ALL_DATA_GetSecretary( RESORT_PEOPLE_ALL_DATA *p_wk, u32 index );
extern RESORT_SECRETARY_DATA * RESORT_PEOPLE_ALL_DATA_SearchEmptySecretary( RESORT_PEOPLE_ALL_DATA *p_wk );


//=============================================================================
/**
 *	リゾート街データ
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//文字列数    EOMなし
#if PM_LANG == LANG_JAPAN
#define RESORT_CITY_NAME_LEN  (10)
#define RESORT_MY_NAME_LEN    (10)
#else
#define RESORT_CITY_NAME_LEN  (20)
#define RESORT_MY_NAME_LEN    (20)
#endif


//施設データ数
#define RESORT_INSTITUTION_DATA_NUM (8)

//街データ総サイズ
#define RESORT_CITY_DATA_SIZE       (372+RESORT_INSTITUTION_DATA_SIZE*RESORT_INSTITUTION_DATA_NUM)

//

//-------------------------------------
///	街データ
//=====================================
typedef struct _RESORT_CITY_DATA RESORT_CITY_DATA;

//-------------------------------------
///	外部公開
//=====================================
extern RESORT_CITY_DATA * RESORT_CITY_DATA_Init( HEAPID heapID );
extern void RESORT_CITY_DATA_Exit( RESORT_CITY_DATA *p_wk );
extern void RESORT_CITY_DATA_InitInstance( RESORT_CITY_DATA *p_wk );


