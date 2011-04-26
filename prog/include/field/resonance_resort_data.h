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
 *  共通データ
 */
//=============================================================================
//-------------------------------------
///	
//=====================================
typedef enum
{
  RESORT_MAPCHANGE_FLAG_NONE,
  RESORT_MAPCHANGE_FLAG_GO,
  RESORT_MAPCHANGE_FLAG_RET,
} RESORT_MAPCHANGE_FLAG;


//=============================================================================
/**
 *  データ初期化
 */
//=============================================================================
extern void RESORT_DATA_Boot( const SAVE_CONTROL_WORK* ctrl );

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
extern BOOL RESORT_INSTITUTION_DATA_IsEmpty( const RESORT_INSTITUTION_DATA *cp_wk );

typedef enum
{
  RESORT_INSTITUTION_DATA_PARAM_ID,
  RESORT_INSTITUTION_DATA_PARAM_LV,
  RESORT_INSTITUTION_DATA_PARAM_EXP,
  RESORT_INSTITUTION_DATA_PARAM_WORK_00,
  RESORT_INSTITUTION_DATA_PARAM_WORK_01,
  RESORT_INSTITUTION_DATA_PARAM_BIT,
}RESORT_INSTITUTION_DATA_PARAM;
extern u32 RESORT_INSTITUTION_DATA_GetData( const RESORT_INSTITUTION_DATA *cp_wk, RESORT_INSTITUTION_DATA_PARAM param );
extern void RESORT_INSTITUTION_DATA_SetData( RESORT_INSTITUTION_DATA *p_wk, RESORT_INSTITUTION_DATA_PARAM param, u32 data );


typedef enum
{
  RESORT_INSTITUTION_DATA_BIT_SHOP_START,
  RESORT_INSTITUTION_DATA_BIT_SHOP_01 = RESORT_INSTITUTION_DATA_BIT_SHOP_START,
  RESORT_INSTITUTION_DATA_BIT_SHOP_02,
  RESORT_INSTITUTION_DATA_BIT_SHOP_03,
  RESORT_INSTITUTION_DATA_BIT_SHOP_04,
  RESORT_INSTITUTION_DATA_BIT_SHOP_05,
  RESORT_INSTITUTION_DATA_BIT_SHOP_06,
  RESORT_INSTITUTION_DATA_BIT_SHOP_END,

  RESORT_INSTITUTION_DATA_BIT_MAX,
}RESORT_INSTITUTION_DATA_BIT;
extern BOOL RESORT_INSTITUTION_DATA_IsBit( const RESORT_INSTITUTION_DATA *cp_wk, RESORT_INSTITUTION_DATA_BIT bit );
extern void RESORT_INSTITUTION_DATA_SetBit( RESORT_INSTITUTION_DATA *p_wk, RESORT_INSTITUTION_DATA_BIT bit, BOOL enable );


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
  RESORT_EXTEND_TYPE_09,
  RESORT_EXTEND_TYPE_MAX,
  RESORT_EXTEND_TYPE_NULL = RESORT_EXTEND_TYPE_MAX,
}RESORT_EXTEND_TYPE;
//-------------------------------------
///	定数
//=====================================
//リゾート用の基本データ１のサイズ
#define RESORT_BASIC1_DATA_SIZE   (60)
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
#define RESORT_EXTEND9_DATA_SIZE   (16)

//口癖の文字数
#define RESORT_PHRASE_LEN         (8)

//リゾート用全受信データの全サイズ
#define RESORT_RECV_DATA_SIZE     (RESORT_BASIC1_DATA_SIZE+RESORT_BASIC2_DATA_SIZE+RESORT_EXTEND_DATA_SIZE*RESORT_EXTEND_TYPE_MAX+RESORT_INSTITUTION_DATA_SIZE+20)

//リゾート用代理データの全サイズ
#define RESORT_PROXY_DATA_SIZE    (8)

//メダルビットのバイト数
#define RESORT_RECV_MEDAL_BIT_BYTE (12)

#define RESORT_PEOPLE_ID_NULL (0xFFFF)

#define RESORT_PEOPLE_PRECENT_ID_BP (1000)

//-------------------------------------
///	リゾート用データの取得引数
//=====================================
typedef u16 RESORT_DATE;

#define RESORT_DATE_GET_YEAR( x )   ((x>>9)&7)
#define RESORT_DATE_GET_MONTH( x )  ((x>>5)&4)
#define RESORT_DATE_GET_DAY( x )    ((x)&5)
#define RESORT_DATE_SET( y,m,d)     ((y&7)<<9|(m&4)<<5|(d&5))

//-------------------------------------
///	リゾート用データの取得引数
//=====================================
typedef enum 
{
  //全人物データ共通
  RESORT_DATA_PARAM_PEOPLE_ID,
  RESORT_DATA_PARAM_PLACE_ID,
  RESORT_DATA_PARAM_SEX,
  RESORT_DATA_PARAM_TRAINERVIEW,
  RESORT_DATA_PARAM_NAME,          //p_ret_adrsから取得
  RESORT_DATA_PARAM_INTRODUCTION, //p_ret_adrsから取得
  RESORT_DATA_PARAM_WANT_JOB, //なりたい職業（RESORT_RESIDENT_TYPE_xxx）

  RESORT_DATA_PARAM_PHRASE, //p_ret_adrsから取得
  RESORT_DATA_PARAM_WANT_SPOT,

  //以下RECVとRESIDENT版にある
  //基本データ
  RESORT_DATA_PARAM_NATION,
  RESORT_DATA_PARAM_AREA,
  RESORT_DATA_PARAM_WND_COLOR,
  RESORT_DATA_PARAM_PLAY_HOUR,
  RESORT_DATA_PARAM_PLAY_MIN,
  RESORT_DATA_PARAM_QANSWER,  //p_ret_adrsから取得
  RESORT_DATA_PARAM_TRAINER_ID,
  RESORT_DATA_PARAM_VER,
  RESORT_DATA_PARAM_LANG,
  RESORT_DATA_PARAM_PASS_CNT,
  RESORT_DATA_PARAM_COLOR,
  //準基本データ
  RESORT_DATA_PARAM_CONTINUE,
  RESORT_DATA_PARAM_PERSONAL,
  RESORT_DATA_PARAM_SPECIAL,
  RESORT_DATA_PARAM_RESORT_QANSWER_00,
  RESORT_DATA_PARAM_RESORT_QANSWER_01,
  RESORT_DATA_PARAM_RESORT_QANSWER_02,
  RESORT_DATA_PARAM_RESORT_QANSWER_03,
  //生成データ
  RESORT_DATA_PARAM_PRESENT_ID,
  RESORT_DATA_PARAM_PRESENT_NUM,
  RESORT_DATA_PARAM_MEET_YEAR,
  RESORT_DATA_PARAM_MEET_MONTH,
  RESORT_DATA_PARAM_MEET_DAY,
  RESORT_DATA_PARAM_MEET_HOUR,
  RESORT_DATA_PARAM_MEET_MIN,
  RESORT_DATA_PARAM_COMM_ID,
  RESORT_DATA_PARAM_LOCK,
  RESORT_DATA_PARAM_UPDATE,
  RESORT_DATA_PARAM_KEEP,
  RESORT_DATA_PARAM_DAY_FLAG,
  RESORT_DATA_PARAM_TYPE, //RESORT_RESIDENT_TYPE_xxx
  RESORT_DATA_PARAM_EVENT_FLAG,
  RESORT_DATA_PARAM_EXTEND_BIT,
  RESORT_DATA_PARAM_KOSEI,
  //拡張データ1
  RESORT_DATA_PARAM_EXT1_MEDAL_BIT,  //p_ret_adrsから取得 12byte
  RESORT_DATA_PARAM_EXT1_RECOMMEND_MEDAL_ID,
  RESORT_DATA_PARAM_EXT1_RECOMMEND_DATE,
  RESORT_DATA_PARAM_EXT1_MEDAL_NUM,
  //拡張データ2
  RESORT_DATA_PARAM_EXT2_RANK_00,
  RESORT_DATA_PARAM_EXT2_RANK_01,
  RESORT_DATA_PARAM_EXT2_RANK_02,
  RESORT_DATA_PARAM_EXT2_RANK_03,
  //拡張データ3
  RESORT_DATA_PARAM_EXT3_RANK_04,
  RESORT_DATA_PARAM_EXT3_RANK_05,
  RESORT_DATA_PARAM_EXT3_RANK_06,
  RESORT_DATA_PARAM_EXT3_RANK_07,
  //拡張データ4
  RESORT_DATA_PARAM_EXT4_GOSSIP_00,
  RESORT_DATA_PARAM_EXT4_GOSSIP_01,
  RESORT_DATA_PARAM_EXT4_GOSSIP_02,
  RESORT_DATA_PARAM_EXT4_GOSSIP_03,
  RESORT_DATA_PARAM_EXT4_GOSSIP_04,
  RESORT_DATA_PARAM_EXT4_GOSSIP_05,
  RESORT_DATA_PARAM_EXT4_GOSSIP_06,
  RESORT_DATA_PARAM_EXT4_GOSSIP_07,
  RESORT_DATA_PARAM_EXT4_GOSSIP_08,
  RESORT_DATA_PARAM_EXT4_GOSSIP_09,
  RESORT_DATA_PARAM_EXT4_GOSSIP_10,
  RESORT_DATA_PARAM_EXT4_GOSSIP_11,
  RESORT_DATA_PARAM_EXT4_GOSSIP_12,
  RESORT_DATA_PARAM_EXT4_GOSSIP_13,
  RESORT_DATA_PARAM_EXT4_GOSSIP_14,
  RESORT_DATA_PARAM_EXT4_GOSSIP_15,
  //拡張データ5
  RESORT_DATA_PARAM_EXT5_MONSNO,
  RESORT_DATA_PARAM_EXT5_FORM,
  RESORT_DATA_PARAM_EXT5_SEX,
  RESORT_DATA_PARAM_EXT5_RARE,
  RESORT_DATA_PARAM_EXT5_TOKUSEI,
  RESORT_DATA_PARAM_EXT5_WAZA_00,
  RESORT_DATA_PARAM_EXT5_WAZA_01,
  RESORT_DATA_PARAM_EXT5_WAZA_02,
  RESORT_DATA_PARAM_EXT5_WAZA_03,
  RESORT_DATA_PARAM_EXT5_ITEM,
  //拡張データ6
  RESORT_DATA_PARAM_EXT6_MONSNO,
  RESORT_DATA_PARAM_EXT6_FORM,
  RESORT_DATA_PARAM_EXT6_SEX,
  RESORT_DATA_PARAM_EXT6_RARE,
  RESORT_DATA_PARAM_EXT6_TOKUSEI,
  RESORT_DATA_PARAM_EXT6_WAZA_00,
  RESORT_DATA_PARAM_EXT6_WAZA_01,
  RESORT_DATA_PARAM_EXT6_WAZA_02,
  RESORT_DATA_PARAM_EXT6_WAZA_03,
  RESORT_DATA_PARAM_EXT6_ITEM,
  //拡張データ7
  RESORT_DATA_PARAM_EXT7_MONSNO,
  RESORT_DATA_PARAM_EXT7_FORM,
  RESORT_DATA_PARAM_EXT7_SEX,
  RESORT_DATA_PARAM_EXT7_RARE,
  RESORT_DATA_PARAM_EXT7_TOKUSEI,
  RESORT_DATA_PARAM_EXT7_WAZA_00,
  RESORT_DATA_PARAM_EXT7_WAZA_01,
  RESORT_DATA_PARAM_EXT7_WAZA_02,
  RESORT_DATA_PARAM_EXT7_WAZA_03,
  RESORT_DATA_PARAM_EXT7_ITEM,
  //拡張データ8
  RESORT_DATA_PARAM_EXT8_RECORD_00,
  RESORT_DATA_PARAM_EXT8_RECORD_01,
  RESORT_DATA_PARAM_EXT8_RECORD_02,
  RESORT_DATA_PARAM_EXT8_RECORD_03,
  RESORT_DATA_PARAM_EXT8_RECORD_DATE_00,
  RESORT_DATA_PARAM_EXT8_RECORD_DATE_01,
  RESORT_DATA_PARAM_EXT8_RECORD_DATE_02,
  RESORT_DATA_PARAM_EXT8_RECORD_DATE_03,
  //拡張データ9
  RESORT_DATA_PARAM_EXT9_PHRASE, 

} RESORT_DATA_PARAM;

//-------------------------------------
///	何の通信からきたかのID
//=====================================
typedef enum 
{
  RESORT_COMM_TYPE_NULL,
  RESORT_COMM_TYPE_BEACON,
  RESORT_COMM_TYPE_CGEAR_IRC,
  RESORT_COMM_TYPE_UNION,
  RESORT_COMM_TYPE_RNDMATCH,
  RESORT_COMM_TYPE_GTSNEGO,
  RESORT_COMM_TYPE_HYLINK,
} RESORT_COMM_TYPE;


//-------------------------------------
///	リゾート用受信データの更新フラグ
//    ここを増やしたら、RESORT_RECV_DATAのupdateのbitを増やすこと
//=====================================
typedef enum
{
  RESORT_RECV_DATA_UPDATE_NONE,         //更新していない    
  RESORT_RECV_DATA_UPDATE_FIRST,        //初回更新        優先度：高
  RESORT_RECV_DATA_UPDATE_MERGE_VIEW,   //見た目更新    
  RESORT_RECV_DATA_UPDATE_MERGE_DATA,   //データ更新      優先度：低

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
extern BOOL RESORT_RECV_DATA_IsExtend( const RESORT_RECV_DATA *cp_wk,  RESORT_EXTEND_TYPE type );
extern BOOL RESORT_RECV_DATA_IsSameOwnerBeacon( const RESORT_RECV_DATA *cp_wk, const RESORT_BEACON_DATA *cp_beacon );
extern BOOL RESORT_RECV_DATA_IsSameOwnerRecv( const RESORT_RECV_DATA *cp_wk, const RESORT_RECV_DATA *cp_recv );
extern BOOL RESORT_RECV_DATA_MergeBeacon( RESORT_RECV_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon );
extern BOOL RESORT_RECV_DATA_MergeRecv( RESORT_RECV_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, RESORT_COMM_TYPE type );
extern BOOL RESORT_RECV_DATA_IsValid( const RESORT_RECV_DATA *cp_wk );

extern u32 RESORT_RECV_DATA_GetData( const RESORT_RECV_DATA *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs );
extern void RESORT_RECV_DATA_SetData( RESORT_RECV_DATA *p_wk, RESORT_DATA_PARAM param, u32 data );

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
extern u32 RESORT_BEACON_DATA_GetTrainerID( const RESORT_BEACON_DATA *cp_wk );
extern BOOL RESORT_BEACON_DATA_IsValid( const RESORT_BEACON_DATA *cp_wk );

//-------------------------------------
///	２…リゾート用代理データの外部公開
//=====================================
extern RESORT_PROXY_DATA *RESORT_PROXY_DATA_Init( HEAPID heapID );
extern void RESORT_PROXY_DATA_Exit( RESORT_PROXY_DATA *p_wk );
extern void RESORT_PROXY_DATA_InitInstance( RESORT_PROXY_DATA *p_wk );
extern BOOL RESORT_PROXY_DATA_IsEmpty( const RESORT_PROXY_DATA *cp_wk );

extern void RESORT_PROXY_DATA_SetCommData( RESORT_PROXY_DATA *p_wk, u16 type );

extern u32 RESORT_PROXY_DATA_GetData( const RESORT_PROXY_DATA *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs );
extern void RESORT_PROXY_DATA_SetData( RESORT_PROXY_DATA *p_wk, RESORT_DATA_PARAM param, u32 data );

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

//拡張バッファのサイズ
#define RECEPTION_BUFF_RECV_MAX (8)

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
extern BOOL RESORT_RECV_BUFF_AddRecv( RESORT_RECV_BUFF *p_wk, const RESORT_RECV_DATA *cp_recv, RESORT_COMM_TYPE type );


//=============================================================================
/**
 *  住民データ
 */
//=============================================================================
#define RESORT_RESIDENT_DATA_SIZE RESORT_RECV_DATA_SIZE

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

#define RESORT_RESIDENT_DATA_IsExtend( x, y ) RESORT_RECV_DATA_IsExtend( x, y );
#define RESORT_RESIDENT_DATA_GetData( x, y, z ) RESORT_RECV_DATA_GetData( x, y, z )
#define RESORT_RESIDENT_DATA_SetData( x, y, z )  RESORT_RECV_DATA_SetData( x, y, z )
#define RESORT_RESIDENT_DATA_IsSameOwnerBeacon( x,y ) RESORT_RECV_DATA_IsSameOwnerBeacon(x,y)
#define RESORT_RESIDENT_DATA_IsSameOwnerRecv( x,y ) RESORT_RECV_DATA_IsSameOwnerRecv(x,y)
#define RESORT_RESIDENT_DATA_MergeBeacon(x,y) RESORT_RECV_DATA_MergeBeacon(x,y)
#define RESORT_RESIDENT_DATA_MergeRecv(x,y,z) RESORT_RECV_DATA_MergeRecv(x,y,z)

extern void RESORT_RESIDENT_DATA_SetRecv( RESORT_RESIDENT_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID, u16 type );
extern RESORT_INSTITUTION_DATA * RESORT_RESIDENT_DATA_GetInstitution( RESORT_RESIDENT_DATA *p_wk );
extern void RESORT_RESIDENT_DATA_JobChange( RESORT_RESIDENT_DATA *p_wk );

//=============================================================================
/**
 *  滞在者データ
 */
//=============================================================================
#define RESORT_VISITOR_DATA_SIZE  (RESORT_BASIC1_DATA_SIZE+RESORT_BASIC2_DATA_SIZE+RESORT_EXTEND4_DATA_SIZE+8)
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
extern void RESORT_VISITOR_DATA_SetRecv( RESORT_VISITOR_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID, u16 type );
extern void RESORT_VISITOR_DATA_SetResident( RESORT_VISITOR_DATA *p_wk, const RESORT_RESIDENT_DATA *cp_resident, u16 peopleID, u16 placeID );

extern u32 RESORT_VISITOR_DATA_GetData( const RESORT_VISITOR_DATA *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs );
extern void RESORT_VISITOR_DATA_SetData( RESORT_VISITOR_DATA *p_wk, RESORT_DATA_PARAM param, u32 data );

//=============================================================================
/**
 *  秘書データ
 */
//=============================================================================
#define RESORT_SECRETARY_DATA_SIZE  (RESORT_BASIC1_DATA_SIZE+RESORT_BASIC2_DATA_SIZE+8)

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

extern u32 RESORT_SECRETARY_DATA_GetData( const RESORT_SECRETARY_DATA *cp_wk, RESORT_DATA_PARAM param, void *p_ret_adrs );
extern void RESORT_SECRETARY_DATA_SetData( RESORT_SECRETARY_DATA *p_wk, RESORT_DATA_PARAM param, u32 data );
extern void RESORT_SECRETARY_DATA_SetRecv( RESORT_SECRETARY_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, u16 peopleID, u16 placeID, u16 type );

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
#define RESORT_PEOPLE_ALL_DATA_SIZE       (RESORT_RECV_DATA_SIZE*RESORT_RESIDENT_DATA_NUM+RESORT_VISITOR_DATA_SIZE*RESORT_VISITOR_DATA_NUM+RESORT_SECRETARY_DATA_SIZE*RESORT_SECRETARY_DATA_NUM+RESORT_RECV_DATA_SIZE)

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
extern u32 RESORT_PEOPLE_ALL_DATA_GetResidentCount( const RESORT_PEOPLE_ALL_DATA *p_wk );
extern RESORT_VISITOR_DATA * RESORT_PEOPLE_ALL_DATA_GetVisitor( RESORT_PEOPLE_ALL_DATA *p_wk, u32 index );
extern RESORT_VISITOR_DATA * RESORT_PEOPLE_ALL_DATA_SearchEmptyVisitor( RESORT_PEOPLE_ALL_DATA *p_wk );
extern RESORT_SECRETARY_DATA * RESORT_PEOPLE_ALL_DATA_GetSecretary( RESORT_PEOPLE_ALL_DATA *p_wk, u32 index );
extern u32 RESORT_PEOPLE_ALL_DATA_GetSecretaryMax( const RESORT_PEOPLE_ALL_DATA *cp_wk );
extern RESORT_SECRETARY_DATA * RESORT_PEOPLE_ALL_DATA_SearchEmptySecretary( RESORT_PEOPLE_ALL_DATA *p_wk );

extern BOOL RESORT_PEOPLE_ALL_DATA_MergeBeacon( RESORT_PEOPLE_ALL_DATA *p_wk, const RESORT_BEACON_DATA *cp_beacon );
extern BOOL RESORT_PEOPLE_ALL_DATA_MergeRecv( RESORT_PEOPLE_ALL_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, RESORT_COMM_TYPE type );

extern void RESORT_PEOPLE_ALL_DATA_UpdateDay( RESORT_PEOPLE_ALL_DATA *p_wk );

extern RESORT_RECV_DATA * RESORT_PEOPLE_ALL_DATA_GetOwnerData( RESORT_PEOPLE_ALL_DATA *p_wk );
//=============================================================================
/**
 *	リゾート街データ
*/
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//実際の文字列数    EOMなし
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
#define RESORT_CITY_DATA_SIZE       (376)

#define RESORT_TRAINER_ID_STORE_SIZE (32)

//-------------------------------------
///	引数
//=====================================
typedef enum
{
  RESORT_CITY_DATA_PARAM_CITY_NAME, //p_ret_adrs
  RESORT_CITY_DATA_PARAM_AUTHER_NAME, //p_ret_adrs
  RESORT_CITY_DATA_PARAM_LV,
  RESORT_CITY_DATA_PARAM_EXP,
  RESORT_CITY_DATA_PARAM_RND_SEED,
  RESORT_CITY_DATA_PARAM_MAX
}RESORT_CITY_DATA_PARAM;

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

extern u32 RESORT_CITY_DATA_GetData( const RESORT_CITY_DATA *cp_wk, RESORT_CITY_DATA_PARAM param, void *p_ret_adrs );
extern void RESORT_CITY_DATA_SetData( RESORT_CITY_DATA *p_wk, RESORT_CITY_DATA_PARAM param, u32 data );

extern void RESORT_CITY_DATA_UpdateDay( RESORT_CITY_DATA *p_wk );
extern void RESORT_CITY_DATA_AddTrainerID( RESORT_CITY_DATA *p_wk, u32 trainerID );
extern BOOL RESORT_CITY_DATA_CheckTrainerID( const RESORT_CITY_DATA *cp_wk, u32 trainerID );

extern u32 RESORT_DATA_GetRnd( u32 ofs, u32 max );
extern u32 RESORT_DATA_GetRndRecv( RESORT_RECV_DATA *p_wk, u32 max );

//=============================================================================
/**
 *　  デバッグ
*/
//=============================================================================
#ifdef PM_DEBUG
extern void DEBUG_RESORT_DATA_SetPermission( BOOL enable );
#endif //PM_DEBUG
