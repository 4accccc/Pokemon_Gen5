//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_net.h
 *	@brief  WIFIバトル用ネットモジュール
 *	@author	Toru=Nagihashi
 *	@date		2009.11.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "sc/dwc_sc.h"
#include "gdb/dwc_gdb.h"
#include "battle/battle.h"
#include "wifibattlematch_data.h"
#include "pm_define.h"
#include "net_app/wifibattlematch.h"
#include "savedata/regulation.h"
#include "net/nhttp_rap.h"
#include "savedata/wifilist.h"
#include "system/net_err.h"
#include "net/nhttp_rap_evilcheck.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	マッチシーケンス
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_NET_SEQ_MATCH_IDLE,
  WIFIBATTLEMATCH_NET_SEQ_MATCH_START,
  WIFIBATTLEMATCH_NET_SEQ_MATCH_START2,
  WIFIBATTLEMATCH_NET_SEQ_MATCH_WAIT,

  WIFIBATTLEMATCH_NET_SEQ_CONNECT_START,
  WIFIBATTLEMATCH_NET_SEQ_CONNECT_CHILD,
  WIFIBATTLEMATCH_NET_SEQ_CONNECT_PARENT,
  WIFIBATTLEMATCH_NET_SEQ_TIMING_END,

  WIFIBATTLEMATCH_NET_SEQ_CANCEL,

  WIFIBATTLEMATCH_NET_SEQ_MATCH_END,
} WIFIBATTLEMATCH_NET_SEQ;

//-------------------------------------
///	エラーの種類
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_NET_ERRORTYPE_NONE,
  WIFIBATTLEMATCH_NET_ERRORTYPE_SC,
  WIFIBATTLEMATCH_NET_ERRORTYPE_GDB,
  WIFIBATTLEMATCH_NET_ERRORTYPE_ND,
  WIFIBATTLEMATCH_NET_ERRORTYPE_NHTTP,
  WIFIBATTLEMATCH_NET_ERRORTYPE_SYS,
} WIFIBATTLEMATCH_NET_ERRORTYPE;
//-------------------------------------
/// 自分のシステムのエラー結果
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_NET_SYSERROR_NONE,    //エラーは発生していない
  WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, //アプリケーションタイムアウトが発生した
  WIFIBATTLEMATCH_NET_SYSERROR_NO_RECORD, //自分のサケテーブルIDが取得＆作成できない
} WIFIBATTLEMATCH_NET_SYSERROR;

//-------------------------------------
///	エラー解決タイプ
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_NET_ERROR_NONE,               //正常
  WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN,      //復帰可能地点まで戻る
  WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT,  //切断しログインからやり直し
  WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL,       //電源切断
} WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	時刻型
//=====================================
typedef struct 
{
  u8  year;                      // 年 ( 0 ~ 99 )
  u8  month;                     // 月 ( 1 ~ 12 )
  u8  day;                       // 日 ( 1 ~ 31 )
  u8  hour;                      // 時 ( 0 ~ 23 )
  u8  minute;                    // 分 ( 0 ~ 59 )
  u8  second;                    // 秒 ( 0 ~ 59 )
} WBM_NET_DATETIME;

//-------------------------------------
///	ポケモンデータ
//=====================================
typedef struct
{ 
  char  data[128];
}WIFIBATTLEMATCH_ND_POKEMON;
typedef struct 
{
	///	現在保持しているポケモン数
	int poke_count;
	///	ポケモンデータ
	WIFIBATTLEMATCH_ND_POKEMON member[TEMOTI_POKEMAX];
} WIFIBATTLEMATCH_ND_POKEPARTY;

//-------------------------------------
///	SAKEからのデータ受け取り
//=====================================
typedef struct
{ 
  union
  { 
    struct
    { 
      s32 double_rate;
      s32 rot_rate;
      s32 shooter_rate;
      s32 single_rate;
      s32 triple_rate;
      s32 wificup_rate;
      s32 cheat;
      s32 complete;
      s32 disconnect;

      s32 double_lose;
      s32 double_win;
      s32 rot_lose;
      s32 rot_win;
      s32 shooter_lose;
      s32 shooter_win;
      s32 single_lose;
      s32 single_win;
      s32 triple_lose;
      s32 triple_win;
      s32 init_profileID;
      s32 now_profileID;
    };
    s32 arry[21];
  };
} WIFIBATTLEMATCH_GDB_RND_SCORE_DATA;

//-------------------------------------
///	SAKEからのデータ受け取り
//=====================================
#define WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE (1334)
typedef struct
{ 
  s32 cheat;
  s32 disconnect;
  s32 win;
  s32 lose;
  s32 rate;
  u8  pokeparty[ WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE ];
} WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA;


//-------------------------------------
///	ネットモジュール
//=====================================
typedef struct _WIFIBATTLEMATCH_NET_WORK WIFIBATTLEMATCH_NET_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク作成
//=====================================
extern WIFIBATTLEMATCH_NET_WORK * WIFIBATTLEMATCH_NET_Init( GAMEDATA *p_gamedata, DWCSvlResult *p_svl_result, HEAPID heapID );
extern void WIFIBATTLEMATCH_NET_Exit( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern void WIFIBATTLEMATCH_NET_Main( WIFIBATTLEMATCH_NET_WORK *p_wk );

//-------------------------------------
///	エラー
//=====================================
extern WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_NET_CheckErrorRepairType( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_heavy );

//-------------------------------------
///	初回処理(必要のない場合は内部で自動的にきる)
//=====================================
extern void WIFIBATTLEMATCH_NET_StartInitialize( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern BOOL WIFIBATTLEMATCH_NET_WaitInitialize( WIFIBATTLEMATCH_NET_WORK *p_wk, SAVE_CONTROL_WORK *p_save );

//-------------------------------------
///	マッチング
//=====================================
typedef struct 
{
  u32 rate;
  u32 disconnect;
  u32 cup_no;
} WIFIBATTLEMATCH_MATCH_KEY_DATA;
extern void WIFIBATTLEMATCH_NET_StartMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_MODE mode, BOOL is_rnd_rate, WIFIBATTLEMATCH_BTLRULE btl_rule, const WIFIBATTLEMATCH_MATCH_KEY_DATA *cp_data );
extern BOOL WIFIBATTLEMATCH_NET_WaitMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern WIFIBATTLEMATCH_NET_SEQ WIFIBATTLEMATCH_NET_GetSeqMatchMake( const WIFIBATTLEMATCH_NET_WORK *cp_wk );

extern void WIFIBATTLEMATCH_NET_StartMatchMakeDebug( WIFIBATTLEMATCH_NET_WORK *p_wk );

//切断
extern BOOL WIFIBATTLEMATCH_NET_SetDisConnect( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_force );

//キャンセル
extern void WIFIBATTLEMATCH_NET_StopConnect( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_stop );

//-------------------------------------
///	ATLAS統計・競争関係（SC）
//=====================================
extern void WIFIBATTLEMATCH_SC_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_MODE mode, WIFIBATTLEMATCH_BTLRULE rule, BtlResult result );
extern BOOL WIFIBATTLEMATCH_SC_Process( WIFIBATTLEMATCH_NET_WORK *p_wk );

typedef struct
{
  DWCScGameResult my_result;
  u8 my_single_win;
  u8 my_single_lose;
  u8 my_double_win;
  u8 my_double_lose;
  u8 my_rot_win;
  u8 my_rot_lose;
  u8 my_triple_win;
  u8 my_triple_lose;
  u8 my_shooter_win;
  u8 my_shooter_lose;
  u8 my_single_rate;
  u8 my_double_rate;
  u8 my_rot_rate;
  u8 my_triple_rate;
  u8 my_shooter_rate;
  u8 my_cheat;
  DWCScGameResult you_result;  
  u8 you_single_win;
  u8 you_single_lose;
  u8 you_double_win;
  u8 you_double_lose;
  u8 you_rot_win;
  u8 you_rot_lose;
  u8 you_triple_win;
  u8 you_triple_lose;
  u8 you_shooter_win;
  u8 you_shooter_lose;
  u8 you_single_rate;
  u8 you_double_rate;
  u8 you_rot_rate;
  u8 you_triple_rate;
  u8 you_shooter_rate;
  u8 you_cheat;
} WIFIBATTLEMATCH_SC_DEBUG_DATA;
extern void WIFIBATTLEMATCH_SC_StartDebug( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_SC_DEBUG_DATA * p_data, BOOL is_auth );

//-------------------------------------
///	SAKEデータベース関係（GDB)
//=====================================
//読み取り
typedef enum
{ 
  WIFIBATTLEMATCH_GDB_GET_RND_SCORE,
  WIFIBATTLEMATCH_GDB_GET_WIFI_SCORE,
  WIFIBATTLEMATCH_GDB_GET_RECORDID,
  WIFIBATTLEMATCH_GDB_GET_PARTY,
  WIFIBATTLEMATCH_GDB_GET_LOGIN_DATE, //DATETIMEの受け取りバッファを設定してください

  WIFIBATTLEMATCH_GDB_GET_DEBUGALL  = WIFIBATTLEMATCH_GDB_GET_RND_SCORE,
}WIFIBATTLEMATCH_GDB_GETTYPE;

//自分のデータから取得する場合
#define WIFIBATTLEMATCH_GDB_MYRECORD  (0xFFFFFFFF)

extern void WIFIBATTLEMATCH_GDB_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, u32 recordID, WIFIBATTLEMATCH_GDB_GETTYPE type, void *p_wk_adrs );
extern BOOL WIFIBATTLEMATCH_GDB_Process( WIFIBATTLEMATCH_NET_WORK *p_wk );

//書き込み
typedef enum
{ 
  WIFIBATTLEMATCH_GDB_WRITE_POKEPARTY,  //ポケモンのバッファは外側のものを使います
  WIFIBATTLEMATCH_GDB_WRITE_WIFI_SCORE,
  WIFIBATTLEMATCH_GDB_WRITE_LOGIN_DATE, //データは、いりません（内部でログイン時間をバッファに貯めます）
  WIFIBATTLEMATCH_GDB_WRITE_MYINFO, //データは、いりません＋LOGIN_DATEも設定します

  WIFIBATTLEMATCH_GDB_WRITE_DEBUGALL,
}WIFIBATTLEMATCH_GDB_WRITETYPE;
extern void WIFIBATTLEMATCH_GDB_StartWrite( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_GDB_WRITETYPE type, const void *cp_wk_adrs );
extern BOOL WIFIBATTLEMATCH_GDB_ProcessWrite( WIFIBATTLEMATCH_NET_WORK *p_wk );

//作成
//  作成したあとは上記関数でWIFIBATTLEMATCH_GDB_WRITE_MYINFOをかきこんでください
extern void WIFIBATTLEMATCH_GDB_StartCreateRecord( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern BOOL WIFIBATTLEMATCH_GDB_ProcessCreateRecord( WIFIBATTLEMATCH_NET_WORK *p_wk );

//レコードID取得
extern u32 WIFIBATTLEMATCH_GDB_GetRecordID( const WIFIBATTLEMATCH_NET_WORK *cp_wk );

//-------------------------------------
///	相手のデータ受信  お互いのデータを送る  SENDDATA系
//=====================================
//お互いの情報を送りあう
extern BOOL WIFIBATTLEMATCH_NET_StartEnemyData( WIFIBATTLEMATCH_NET_WORK *p_wk, const void *cp_buff );
extern BOOL WIFIBATTLEMATCH_NET_WaitEnemyData( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_ENEMYDATA **pp_data );
//ポケパーテイを送りあう
extern BOOL WIFIBATTLEMATCH_NET_SendPokeParty( WIFIBATTLEMATCH_NET_WORK *p_wk, const POKEPARTY *cp_party );
extern BOOL WIFIBATTLEMATCH_NET_RecvPokeParty( WIFIBATTLEMATCH_NET_WORK *p_wk, POKEPARTY *p_party );
//不正カウンタを送り合う
extern BOOL WIFIBATTLEMATCH_NET_SendDirtyCnt( WIFIBATTLEMATCH_NET_WORK *p_wk, const u32 *cp_dirty_cnt );
extern BOOL WIFIBATTLEMATCH_NET_RecvDirtyCnt( WIFIBATTLEMATCH_NET_WORK *p_wk, u32 *p_dirty_cnt );
//-------------------------------------
///	ダウンロードサーバー  (ND)
//=====================================
typedef enum
{ 
  WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_DOWNLOADING, //取得中
  WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_SUCCESS, //受信成功
  WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_EMPTY,   //存在しなかった
  WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR,   //エラー
}WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET;

extern void WIFIBATTLEMATCH_NET_StartDownloadDigCard( WIFIBATTLEMATCH_NET_WORK *p_wk, int cup_no );
extern WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET WIFIBATTLEMATCH_NET_WaitDownloadDigCard( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern void WIFIBATTLEMATCH_NET_GetDownloadDigCard( const WIFIBATTLEMATCH_NET_WORK *cp_wk, REGULATION_CARDDATA *p_recv );

//-------------------------------------
///	GPFサーバー            (HTTP)
//=====================================
//受信
typedef enum
{
  WIFIBATTLEMATCH_RECV_GPFDATA_RET_UPDATE,
  WIFIBATTLEMATCH_RECV_GPFDATA_RET_SUCCESS,
  WIFIBATTLEMATCH_RECV_GPFDATA_RET_ERROR,
} WIFIBATTLEMATCH_RECV_GPFDATA_RET;

extern void WIFIBATTLEMATCH_NET_StartRecvGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk, HEAPID heapID );
extern WIFIBATTLEMATCH_RECV_GPFDATA_RET WIFIBATTLEMATCH_NET_WaitRecvGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern void WIFIBATTLEMATCH_NET_GetRecvGpfData( const WIFIBATTLEMATCH_NET_WORK *cp_wk, DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA *p_recv );

//書き込み
typedef enum
{
  WIFIBATTLEMATCH_SEND_GPFDATA_RET_UPDATE,
  WIFIBATTLEMATCH_SEND_GPFDATA_RET_SUCCESS,
  WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR,
} WIFIBATTLEMATCH_SEND_GPFDATA_RET;

extern void WIFIBATTLEMATCH_NET_StartSendGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk, const DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA *cp_send, HEAPID heapID );
extern WIFIBATTLEMATCH_SEND_GPFDATA_RET WIFIBATTLEMATCH_NET_WaitSendGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk );


//-------------------------------------
///	不正文字チェック
//=====================================
extern void WIFIBATTLEMATCH_NET_StartBadWord( WIFIBATTLEMATCH_NET_WORK *p_wk, const STRCODE *cp_str, u32 str_len );
extern BOOL WIFIBATTLEMATCH_NET_WaitBadWord( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL *p_is_bad_word );

//-------------------------------------
///	ポケモン不正チェック  (NHTTP)
//=====================================
//結果コード
typedef enum
{
  WIFIBATTLEMATCH_NET_EVILCHECK_RET_UPDATE,
  WIFIBATTLEMATCH_NET_EVILCHECK_RET_SUCCESS,
  WIFIBATTLEMATCH_NET_EVILCHECK_RET_ERROR,
} WIFIBATTLEMATCH_NET_EVILCHECK_RET;
//データ
typedef struct 
{
  u8 status_code;
  u32 poke_result[TEMOTI_POKEMAX];
  s8 sign[ NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN ];
} WIFIBATTLEMATCH_NET_EVILCHECK_DATA;

typedef enum
{ 
  WIFIBATTLEMATCH_NET_EVILCHECK_TYPE_PP,
  WIFIBATTLEMATCH_NET_EVILCHECK_TYPE_PARTY,
} WIFIBATTLEMATCH_NET_EVILCHECK_TYPE;

extern void WIFIBATTLEMATCH_NET_StartEvilCheck( WIFIBATTLEMATCH_NET_WORK *p_wk, const void *cp_void, WIFIBATTLEMATCH_NET_EVILCHECK_TYPE type );
extern WIFIBATTLEMATCH_NET_EVILCHECK_RET WIFIBATTLEMATCH_NET_WaitEvilCheck( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_NET_EVILCHECK_DATA *p_data );
