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
  WIFIBATTLEMATCH_NET_ERRORTYPE_DWC,
  WIFIBATTLEMATCH_NET_ERRORTYPE_SC,
  WIFIBATTLEMATCH_NET_ERRORTYPE_GDB,
  WIFIBATTLEMATCH_NET_ERRORTYPE_MY,
} WIFIBATTLEMATCH_NET_ERRORTYPE;
//-------------------------------------
/// 自分のシステムのエラー結果
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_NET_RESULT_NONE,
  WIFIBATTLEMATCH_NET_RESULT_TIMEOUT,
} WIFIBATTLEMATCH_NET_RESULT;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

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
    s32 arry[20];
  };
} WIFIBATTLEMATCH_GDB_RND_SCORE_DATA;

//-------------------------------------
///	PDFから落としてきたデータ
//=====================================
typedef struct 
{
  char  status;
  short no;

  //登録ポケモン？
  //todo

} WIFIBATTLEMATCH_PDF_DATA;


//-------------------------------------
///	エラー
//=====================================
/*typedef struct {
  WIFIBATTLEMATCH_NET_ERRORTYPE type;
  DWCGdbError                   gdb;
  DWCScResult                   sc;
  WIFIBATTLEMATCH_NET_RESULT    my;
} WIFIBATTLEMATCH_NET_ERRORTYPE;
*/
//エラー解決タイプ
typedef enum
{
  WIFIBATTLEMATCH_NET_ERROR_NONE,               //正常
  WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY,       //もう一度
  WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT,  //切断しログインからやり直し
  WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL,       //電源切断
} WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE;


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
extern WIFIBATTLEMATCH_NET_WORK * WIFIBATTLEMATCH_NET_Init( const DWCUserData *cp_user_data, WIFI_LIST *p_wifilist, HEAPID heapID );
extern void WIFIBATTLEMATCH_NET_Exit( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern void WIFIBATTLEMATCH_NET_Main( WIFIBATTLEMATCH_NET_WORK *p_wk );

//-------------------------------------
///	エラー
//=====================================
extern BOOL WIFIBATTLEMATCH_NET_CheckError( WIFIBATTLEMATCH_NET_WORK *p_wk );

//-------------------------------------
///	初回処理(必要のない場合は内部で自動的にきる)
//=====================================
extern void WIFIBATTLEMATCH_NET_StartInitialize( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern BOOL WIFIBATTLEMATCH_NET_WaitInitialize( WIFIBATTLEMATCH_NET_WORK *p_wk, SAVE_CONTROL_WORK *p_save, DWCGdbError *p_result );
extern BOOL WIFIBATTLEMATCH_NET_IsInitialize( const WIFIBATTLEMATCH_NET_WORK *cp_wk );

//-------------------------------------
///	マッチング
//=====================================
//マッチング
extern void WIFIBATTLEMATCH_NET_StartMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_MODE mode, BOOL is_rnd_rate, WIFIBATTLEMATCH_BTLRULE btl_rule );
extern BOOL WIFIBATTLEMATCH_NET_WaitMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern WIFIBATTLEMATCH_NET_SEQ WIFIBATTLEMATCH_NET_GetSeqMatchMake( const WIFIBATTLEMATCH_NET_WORK *cp_wk );

extern void WIFIBATTLEMATCH_NET_StartMatchMakeDebug( WIFIBATTLEMATCH_NET_WORK *p_wk );

//切断
extern BOOL WIFIBATTLEMATCH_NET_SetDisConnect( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_force );

//キャンセル
extern void WIFIBATTLEMATCH_NET_StopConnect( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_stop );

//-------------------------------------
///	統計・競争関係（SC）
//=====================================
extern void WIFIBATTLEMATCH_SC_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_BTLRULE rule, BtlResult result );
extern BOOL WIFIBATTLEMATCH_SC_Process( WIFIBATTLEMATCH_NET_WORK *p_wk, DWCScResult *p_result );

extern WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_SC_GetErrorRepairType( DWCScResult error );

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
///	データベース関係（GDB)
//=====================================
typedef enum
{ 
  WIFIBATTLEMATCH_GDB_GET_RND_SCORE,
}WIFIBATTLEMATCH_GDB_GETTYPE;
//WIFIBATTLEMATCH_GDB_GET_RND_SCORE
extern void WIFIBATTLEMATCH_GDB_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_GDB_GETTYPE type, void *p_wk_adrs );
extern BOOL WIFIBATTLEMATCH_GDB_Process( WIFIBATTLEMATCH_NET_WORK *p_wk, DWCGdbError *p_result );

extern void WIFIBATTLEMATCH_GDB_StartWrite( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_GDB_GETTYPE type, const void *cp_wk_adrs );
extern BOOL WIFIBATTLEMATCH_GDB_ProcessWrite( WIFIBATTLEMATCH_NET_WORK *p_wk, DWCGdbError *p_result );

extern WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_GDB_GetErrorRepairType( DWCGdbError error );

//-------------------------------------
///	相手のデータ受信
//=====================================
extern BOOL WIFIBATTLEMATCH_NET_StartEnemyData( WIFIBATTLEMATCH_NET_WORK *p_wk, const void *cp_buff );
extern BOOL WIFIBATTLEMATCH_NET_WaitEnemyData( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_ENEMYDATA **pp_data );


//-------------------------------------
///	GPFサーバー　ダウンロード（SC)
//=====================================
extern void WIFIBATTLEMATCH_NET_StartDownloadDigCard( WIFIBATTLEMATCH_NET_WORK *p_wk );
extern BOOL WIFIBATTLEMATCH_NET_WaitDownloadDigCard( WIFIBATTLEMATCH_NET_WORK *p_wk );
