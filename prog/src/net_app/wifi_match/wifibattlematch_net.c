//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_net.c
 *	@brief  WIFIバトル用ネットモジュール
 *	@author	Toru=Nagihashi
 *	@data		2009.11.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	ライブラリ
#include <gflib.h>
#include <dwc.h>

//	システム
#include "system/main.h"

//  ネットワーク
#include "net/network_define.h"
#include "net/dwc_rap.h"
#include "system/net_err.h"


//  セーブデータ
#include "savedata/system_data.h"
#include "savedata/wifilist.h"

//アトラス情報
#include "atlas_syachi2ds_v1.h"

//  外部公開
#include "wifibattlematch_net.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define DEBUG_DEBUG_NET_Printf_ON
#endif //PM_DEBUG

#if defined(DEBUG_DEBUG_NET_Printf_ON) && defined(DEBUG_ONLY_FOR_toru_nagihashi)
#define DEBUG_NET_Printf(...)  OS_FPrintf(3,__VA_ARGS__)
#else
#define DEBUG_NET_Printf(...)  /*  */
#endif  //DEBUG_DEBUG_NET_Printf_ON

//-------------------------------------
///	マクロスイッチ
//=====================================

//-------------------------------------
///	ネット定数
//=====================================
#ifdef PM_DEBUG
#define WIFIBATTLEMATCH_NET_SSL_TYPE DWC_GDB_SSL_TYPE_NONE
#else
#define WIFIBATTLEMATCH_NET_SSL_TYPE DWC_GDB_SSL_TYPE_SERVER_AUTH
#endif

#define WIFIBATTLEMATCH_NET_TABLENAME "PlayerStats_v1" 

//以下、ATLAS経由ではなく、SAKE直接アクセスの場合
#define SAKE_STAT_RECORDID              "recordid"
#define SAKE_STAT_INITIAL_PROFILE_ID    "INITIAL_PROFILE_ID"
#define SAKE_STAT_NOW_PROFILE_ID        "NOW_PROFILE_ID"
#define SAKE_STAT_LAST_LOGIN_DATETIME   "LAST_LOGIN_DATETIME"
#define SAKE_STAT_WIFICUP_POKEMON_PARTY "WIFICUP_POKEMON_PARTY"

//-------------------------------------
///	SCのシーケンス
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_SC_SEQ_INIT,
  WIFIBATTLEMATCH_SC_SEQ_LOGIN,
  WIFIBATTLEMATCH_SC_SEQ_SESSION_START,
  WIFIBATTLEMATCH_SC_SEQ_SESSION_WAIT,
  WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_START,
  WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_WAIT,
  WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA,
  WIFIBATTLEMATCH_SC_SEQ_INTENTION_START,
  WIFIBATTLEMATCH_SC_SEQ_INTENTION_WAIT,
  WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_START,
  WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_WAIT,
  WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_PARENT,
  WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD,
  WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_START,
  WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_WAIT,
  WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT,
  WIFIBATTLEMATCH_SC_SEQ_SUBMIT_REPORT_START,
  WIFIBATTLEMATCH_SC_SEQ_SUBMIT_REPORT_WAIT,
  WIFIBATTLEMATCH_SC_SEQ_EXIT,
  WIFIBATTLEMATCH_SC_SEQ_END,
} WIFIBATTLEMATCH_SC_SEQ;

//-------------------------------------
///	GDBのシーケンス
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_GDB_SEQ_INIT,
  WIFIBATTLEMATCH_GDB_SEQ_GET_START,
  WIFIBATTLEMATCH_GDB_SEQ_GET_WAIT,
  WIFIBATTLEMATCH_GDB_SEQ_GET_END,
  WIFIBATTLEMATCH_GDB_SEQ_EXIT,
  WIFIBATTLEMATCH_GDB_SEQ_END,
} WIFIBATTLEMATCH_GDB_SEQ;

//-------------------------------------
///	タイミング
//=====================================
#define WIFIBATTLEMATCH_SC_SEND_PLAYERDATA_TIMING    (1)
#define WIFIBATTLEMATCH_SC_RETURN_PLAYERDATA_TIMING  (2)
#define WIFIBATTLEMATCH_SC_REPORT_TIMING    (3)

//-------------------------------------
///	その他
//=====================================
#define TIMEOUT_MS   100  // HTTP通信のタイムアウト時間
#define PLAYER_NUM   2          // プレイヤー数
#define TEAM_NUM     0          // チーム数
#define CANCELSELECT_TIMEOUT (60*30)     //キャンセルセレクトタイムアウト
#define ASYNC_TIMEOUT (60*60)     //キャンセルセレクトタイムアウト

//-------------------------------------
///	マッチメイクキー
//=====================================
typedef enum
{
  MATCHMAKE_KEY_WBM,    //Wifiバトルマッチを示す（必ず１）
  MATCHMAKE_KEY_MODE,   //バトルモード
  MATCHMAKE_KEY_RULE,   //バトルルール
  MATCHMAKE_KEY_DEBUG,  //デバッグ用

  MATCHMAKE_KEY_MAX,
} MATCHMAKE_KEY;

#define WIFIBATTLEMATCH_NET_TIMINGSYNC_CONNECT  15

//-------------------------------------
///	受信フラグ
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER,
  WIFIBATTLEMATCH_NET_RECVFLAG_GAMEDATA,
  WIFIBATTLEMATCH_NET_RECVFLAG_CANCELREQUEST,
  WIFIBATTLEMATCH_NET_RECVFLAG_MAX
} WIFIBATTLEMATCH_NET_RECVFLAG;

//-------------------------------------
///	アトリビュート
//=====================================
#define WIFI_FILE_ATTR1			"REGCARD"
//#define WIFI_FILE_ATTR2		//ここに大会番号が入っている
#define WIFI_FILE_ATTR3			""
#define REGULATION_CARD_DATA_SIZE (sizeof(REGULATION_CARDDATA))

//-------------------------------------
///	  不正文字チェック
//=====================================
#define WIFIBATTLEMATCH_BADWORD_STRL_MAX  (10)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	プレイヤーデータ
//=====================================
typedef struct
{
  DWCScLoginCertificatePtr mCertificate;
  DWCScReportPtr mReport;
  u8 mSessionId[DWC_SC_SESSION_GUID_SIZE];
  u8 mConnectionId[DWC_SC_CONNECTION_GUID_SIZE];
  u8 mStatsAuthdata[DWC_SC_AUTHDATA_SIZE];
} DWC_SC_PLAYERDATA;

//-------------------------------------
///	書き込み情報
//=====================================
typedef struct 
{
  WIFIBATTLEMATCH_BTLRULE   btl_rule;
  BtlResult btl_result;
} DWC_SC_WRITE_DATA;
//SCのリポート作成コア関数
typedef DWCScResult (*DWCRAP_SC_CREATE_REPORT_CORE_FUNC)( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my );

//-------------------------------------
///	友達無指定ピアマッチメイク用追加キーデータ構造体
//=====================================
typedef struct
{
  int ivalue;       // キーに対応する値（int型）
  u8  keyID;        // マッチメイク用キーID
  u8 pad;           // パディング
  char keyStr[4];   // マッチメイク用キー文字列
} MATCHMAKE_KEY_WORK;

//-------------------------------------
///	ネットモジュール
//=====================================
struct _WIFIBATTLEMATCH_NET_WORK
{ 
  const DWCUserData *cp_user_data;
  WIFI_LIST *p_wifilist;

  //マッチング
  u32 seq_matchmake;
  char filter[128];
  MATCHMAKE_KEY_WORK  key_wk[ MATCHMAKE_KEY_MAX ];


  HEAPID heapID;
  u32   init_seq;
  u32   seq;
  u32   next_seq;
  BOOL  is_gdb_start;
  BOOL  is_gdb_w_start;
  BOOL  is_sc_start;
  BOOL  is_initialize;
  BOOL  is_nd_start;
  u32   wait_cnt;
  BOOL              is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_MAX];
  
  //SC
  DWC_SC_PLAYERDATA player[2];  //自分は０ 相手は１
  DWCRAP_SC_CREATE_REPORT_CORE_FUNC SC_CreateReportCoreFunc;
  DWCScResult       result;



  //gdb
  void *p_get_wk;
  DWCGdbGetRecordsCallback  gdb_get_record_callback;
  DWC_SC_WRITE_DATA report;
  DWCGdbField *p_field_buff;
  const char **pp_table_name;
  int table_name_num;
  int sake_record_id;
  DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA gdb_write_data;
  u32   magic_num;


  //以下のバッファはフレキシブルなので、メモリ確保している
  WIFIBATTLEMATCH_ENEMYDATA *p_enemy_data;

  u32 async_timeout;
  u32 cancel_select_timeout;   //CANCELSELECT_TIMEOUT
  BOOL is_stop_connect;
  BOOL is_auth;

  //マッチメイク
  DWCEvalPlayerCallback matchmake_eval_callback;

  //GPFサーバーからの受信
  NHTTP_RAP_WORK* p_nhttp;
  DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA gpf_data;

  //以下ダウンロード用
  REGULATION_CARDDATA temp_buffer;
  DWCNdFileInfo fileInfo;
  u32               percent;
  u32               recived;
  u32               contentlen;
  int               server_filenum;
  char              nd_attr2[10];

  //以下バックアップ用
  s32 init_profileID;
  s32 now_profileID;

  //以下不正文字チェック用
  STRCODE   badword_str[ WIFIBATTLEMATCH_BADWORD_STRL_MAX ];
  u16       *p_badword_arry[1];
  char      badword_result[1];
  int       badword_num;

  //以下デバッグ用データ
  BOOL is_debug;
  WIFIBATTLEMATCH_SC_DEBUG_DATA * p_debug_data;
};

//=============================================================================
/**
 *          グローバル
 */
//=============================================================================
//NdCallback内の情報保持（外からワーク渡せないので……）
static u8 s_callback_flag   = FALSE;
static DWCNdError s_callback_result = 0;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	マッチング関係
//=====================================
static void MATCHMAKE_KEY_Set( WIFIBATTLEMATCH_NET_WORK *p_wk, MATCHMAKE_KEY key, int value );
static int WIFIBATTLEMATCH_RND_FREE_Eval_Callback( int index, void* p_param_adrs );
static int WIFIBATTLEMATCH_RND_RATE_Eval_Callback( int index, void* p_param_adrs );
static int WIFIBATTLEMATCH_WIFI_Eval_Callback( int index, void* p_param_adrs );

//-------------------------------------
///	GFL_NETコールバック
//=====================================
static void DwcRap_Sc_RecvPlayerData(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle);
static void WifiBattleMatch_RecvEnemyData(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle);
static u8* WifiBattleMatch_RecvEnemyDataBuffAddr(int netID, void* p_wk_adrs, int size);
static void WifiBattleMatch_RecvMatchCancel(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle);

//-------------------------------------
///	SC関係
//=====================================
//DWCコールバック
static void DwcRap_Sc_CreateSessionCallback( DWCScResult theResult, void* theUserData );
static void DwcRap_Sc_SetReportIntentionCallback( DWCScResult theResult, void* theUserData );
static void DwcRap_Sc_SubmitReportCallback( DWCScResult theResult, void* theUserData );
//private
static void DwcRap_Sc_Finalize( WIFIBATTLEMATCH_NET_WORK *p_wk );
static DWCScResult DwcRap_Sc_CreateReport( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const DWC_SC_WRITE_DATA *cp_data, BOOL is_auth, DWCRAP_SC_CREATE_REPORT_CORE_FUNC SC_CreateReportCoreFunc );
static DWCScResult DwcRap_Sc_CreateReportDebug( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const WIFIBATTLEMATCH_SC_DEBUG_DATA * cp_data, BOOL is_auth );
//リポート作成コールバック
static DWCScResult DwcRap_Sc_CreateReportRndCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my );
static DWCScResult DwcRap_Sc_CreateReportWifiCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my );

//-------------------------------------
///	GDB関係
//=====================================
static void DwcRap_Gdb_Rnd_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param);
static void DwcRap_Gdb_Wifi_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param);
static void DwcRap_Gdb_RecordID_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param);
static void DwcRap_Gdb_Finalize( WIFIBATTLEMATCH_NET_WORK *p_wk );


#ifdef PM_DEBUG
static void print_field(DWCGdbField* field); // レコードをデバッグ出力する。
#endif

//-------------------------------------
///	ダウンロード関係
//=====================================
static void DwcRap_Nd_WaitNdCallback( WIFIBATTLEMATCH_NET_WORK *p_wk, int next_seq );
static void NdCallback(DWCNdCallbackReason reason, DWCNdError error, int servererror);

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	フィールドの型
//=====================================
static const int sc_field_type[]  =
{ 
  DWC_GDB_FIELD_TYPE_INT,//"ARENA_ELO_RATING_1V1_DOUBLE",
  DWC_GDB_FIELD_TYPE_INT,//"ARENA_ELO_RATING_1V1_ROTATE",
  DWC_GDB_FIELD_TYPE_INT,//"ARENA_ELO_RATING_1V1_SHOOTER",
  DWC_GDB_FIELD_TYPE_INT,//"ARENA_ELO_RATING_1V1_SINGLE",
  DWC_GDB_FIELD_TYPE_INT,//"ARENA_ELO_RATING_1V1_TRIPLE",
  DWC_GDB_FIELD_TYPE_INT,//"CHEATS_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"COMPLETE_MATCHES_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"DISCONNECTS_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_DOUBLE_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_DOUBLE_WIN_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_ROTATE_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_ROTATE_WIN_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_SHOOTER_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_SHOOTER_WIN_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_SINGLE_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_SINGLE_WIN_COUNTER"  ,
  DWC_GDB_FIELD_TYPE_INT,//"NUM_TRIPLE_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"NUM_TRIPLE_WIN_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"INITIAL_PROFILE_ID",
  DWC_GDB_FIELD_TYPE_INT,//"NOW_PROFILE_ID",
};
//-------------------------------------
///	マッチメイクキー文字列
//    3文字固定
//=====================================
static const char *sc_matchmake_key_str[ MATCHMAKE_KEY_MAX ] =
{ 
  "wbm",
  "mod",
  "rul",
  "deb",
};
//-------------------------------------
///	通信コマンド
//=====================================
enum
{ 
  WIFIBATTLEMATCH_NETCMD_SEND_PLAYERDATA  = GFL_NET_CMD_WIFIMATCH,
  WIFIBATTLEMATCH_NETCMD_SEND_ENEMYDATA,
  WIFIBATTLEMATCH_NETCMD_SEND_CANCELMATCH,
  WIFIBATTLEMATCH_NETCMD_MAX,
};
static const NetRecvFuncTable sc_net_recv_tbl[] = 
{
  {DwcRap_Sc_RecvPlayerData,   NULL},    ///_NETCMD_INFOSEND
  {WifiBattleMatch_RecvEnemyData, WifiBattleMatch_RecvEnemyDataBuffAddr },
  {WifiBattleMatch_RecvMatchCancel, NULL},
};


//=============================================================================
/**
 *					初期化・破棄
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_WORK * WIFIBATTLEMATCH_NET_Init( const DWCUserData *cp_user_data, WIFI_LIST *p_wifilist, HEAPID heapID )
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_NET_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_NET_WORK) );
  p_wk->cp_user_data  = cp_user_data;
  p_wk->p_wifilist    = p_wifilist;

  p_wk->magic_num = 0x573;


  p_wk->p_field_buff  = GFL_HEAP_AllocMemory( heapID, (ATLAS_RND_GetFieldNameNum() + ATLAS_WIFI_GetFieldNameNum()) * sizeof(DWCGdbField) );
  GFL_STD_MemClear(p_wk->p_field_buff, (ATLAS_RND_GetFieldNameNum() + ATLAS_WIFI_GetFieldNameNum()) * sizeof(DWCGdbField) );

  p_wk->p_enemy_data  = GFL_HEAP_AllocMemory( heapID, MATH_ROUNDUP32(WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE) );
  GFL_STD_MemClear(p_wk->p_enemy_data, MATH_ROUNDUP32(WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE) );

  GFL_NET_DebugPrintOn();

  if(!GFL_NET_IsInit())
  {
    GF_ASSERT(0);
  }

  GFL_NET_AddCommandTable( GFL_NET_CMD_WIFIMATCH, sc_net_recv_tbl, NELEMS(sc_net_recv_tbl), p_wk );
  GFL_NET_SetWifiBothNet(FALSE);


  NAGI_Printf( "Net Wk 0x%x buff 0x%x\n", p_wk, p_wk->p_enemy_data );
  NAGI_Printf( "BufferAddr 0x%x\n", WifiBattleMatch_RecvEnemyDataBuffAddr );

  return p_wk;
}

#if 0
wk 0x255d114 buff 0x0


Net Wk 0x255d114 buff 0x255d73c
BufferAddr 0x22231d1
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_Exit( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  GFL_NET_DelCommandTable( GFL_NET_CMD_WIFIMATCH );

  GFL_HEAP_FreeMemory( p_wk->p_enemy_data );
  GFL_HEAP_FreeMemory( p_wk->p_field_buff );
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_Main( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
}

//----------------------------------------------------------------------------
/**
 *	@brief  DWCのエラーチェック
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *
 *	@return TRUEエラー発生  FALSE
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_CheckError( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  if( GFL_NET_IsInit() )
  { 
    if( NetErr_App_CheckError() )
    { 
      const GFL_NETSTATE_DWCERROR* cp_error  =  GFL_NET_StateGetWifiError();

      //GDBとSCの切断とFatalエラー以外は、個別処理とするため、検知しない
      if( (cp_error->errorRet == DWC_ERROR_GDB_ANY
            || cp_error->errorRet == DWC_ERROR_SCL_ANY)
          && (cp_error->errorType != DWC_ETYPE_DISCONNECT
            && cp_error->errorType != DWC_ETYPE_FATAL) )
      { 
        return FALSE;
      }

      switch( cp_error->errorType )
      { 
      case DWC_ETYPE_LIGHT:
        /* fallthru */
      case DWC_ETYPE_SHOW_ERROR:
        //エラーコードorメッセージを表示するだけ
        NetErr_DispCallPushPop();
        GFL_NET_StateClearWifiError();
        break;

      case DWC_ETYPE_SHUTDOWN_FM:
      case DWC_ETYPE_SHUTDOWN_GHTTP:
      case DWC_ETYPE_SHUTDOWN_ND:
        //シャットダウン
        NetErr_App_ReqErrorDisp();
        return TRUE;

      case DWC_ETYPE_DISCONNECT:
        //切断
        NetErr_App_ReqErrorDisp();
        return TRUE;

      case DWC_ETYPE_FATAL:
        //Fatal
        NetErr_DispCallFatal();
        return TRUE;
      }
    }
  }

  return FALSE;

}

//----------------------------------------------------------------------------
/**
 *	@brief  初期化開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartInitialize( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  p_wk->is_initialize = TRUE;
  p_wk->init_seq  = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  初期化しているかどうかを判別
 *
 *	@param	const WIFIBATTLEMATCH_NET_WORK *cp_wk   ワーク
 *
 *	@return TRUEで初期化  FALSEで破棄
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_IsInitialize( const WIFIBATTLEMATCH_NET_WORK *cp_wk )
{ 
  return cp_wk->is_initialize;
}
//----------------------------------------------------------------------------
/**
 *	@brief  初期化待ち
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *
 *	@return TRUE処理完了  FALSE処理中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_WaitInitialize( WIFIBATTLEMATCH_NET_WORK *p_wk, SAVE_CONTROL_WORK *p_save, DWCGdbError *p_result )
{ 
  enum
  { 
    SEQ_START_GET_TABLE_ID,
    SEQ_WAIT_GET_TABLE_ID,
    SEQ_START_CREATE_RECORD,
    SEQ_WAIT_CREATE_RECORD,
    SEQ_END,
  };

  switch( p_wk->init_seq )
  { 
  case SEQ_START_GET_TABLE_ID:
    WIFIBATTLEMATCH_GDB_Start( p_wk, WIFIBATTLEMATCH_GDB_GET_RECORDID, &p_wk->sake_record_id );
    p_wk->init_seq  = SEQ_WAIT_GET_TABLE_ID;
    break;

  case SEQ_WAIT_GET_TABLE_ID:
    if( WIFIBATTLEMATCH_GDB_Process( p_wk, p_result ) )
    { 
      if( p_wk->sake_record_id != 0 )
      { 
        p_wk->init_seq  = SEQ_END;
      }
      else
      { 
        p_wk->init_seq  = SEQ_START_CREATE_RECORD;
      }
    }
    break;

  case SEQ_START_CREATE_RECORD:
    WIFIBATTLEMATCH_GDB_StartCreateRecord( p_wk );
    p_wk->init_seq  = SEQ_WAIT_CREATE_RECORD;
    break;

  case SEQ_WAIT_CREATE_RECORD:
    if( WIFIBATTLEMATCH_GDB_ProcessCreateRecord( p_wk, p_result ) )
    { 
      p_wk->init_seq  = SEQ_END;
    }
    break;

  case SEQ_END:
    DEBUG_NET_Printf( "sake テーブルID %d\n", p_wk->sake_record_id );
    p_wk->is_initialize = FALSE;
    return TRUE;
  }

  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  マッチメイク開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_MODE mode, BOOL is_rnd_rate, WIFIBATTLEMATCH_BTLRULE btl_rule )
{ 
  u32 btl_mode  = mode;
  if( mode == WIFIBATTLEMATCH_MODE_RANDOM )
  { 
    btl_mode  += is_rnd_rate;
  }

  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_WBM, TRUE );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_MODE, btl_mode );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_RULE, btl_rule );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_DEBUG, MATCHINGKEY );
  STD_TSPrintf( p_wk->filter, "wbm=%d And mod=%d And rul=%d And deb=%d", TRUE, btl_mode, btl_rule, MATCHINGKEY );
  p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_START;

  //接続評価コールバック指定
  switch( mode )
  { 
  case WIFIBATTLEMATCH_MODE_WIFI:    //WIFI大会
    p_wk->matchmake_eval_callback = WIFIBATTLEMATCH_WIFI_Eval_Callback;
    break;
  case WIFIBATTLEMATCH_MODE_LIVE:    //ライブ大会
    GF_ASSERT( 0 );
    break;
  case WIFIBATTLEMATCH_MODE_RANDOM:  //ランダム対戦　（マッチメイクのときにRANDOM＋０がフリー＋１がレーティングにしている）
    if( is_rnd_rate )
    { 
      p_wk->matchmake_eval_callback = WIFIBATTLEMATCH_RND_RATE_Eval_Callback;
    }
    else
    { 
      p_wk->matchmake_eval_callback = WIFIBATTLEMATCH_RND_FREE_Eval_Callback;
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチメイク開始  デバッグ版
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartMatchMakeDebug( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_WBM, 0 );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_MODE, 0 );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_RULE, 0 );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_DEBUG, 0xF );
  STD_TSPrintf( p_wk->filter, "wbm=%d And mod=%d And rul=%d And deb=%d", 0, 0, 0, 0xF );
  p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_START;

  p_wk->matchmake_eval_callback = WIFIBATTLEMATCH_RND_FREE_Eval_Callback;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチメイク処理
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_WaitMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
 // DEBUG_NET_Printf( "WiFiState %d \n", GFL_NET_StateGetWifiStatus() );
  switch( p_wk->seq_matchmake )
  { 
  case WIFIBATTLEMATCH_NET_SEQ_MATCH_IDLE:
    /* 何もしない  */
    break;

  case WIFIBATTLEMATCH_NET_SEQ_MATCH_START:
    if( GFL_NET_DWC_StartMatchFilter( p_wk->filter, 2 ,p_wk->matchmake_eval_callback, p_wk ) != 0 )
    {
      GFL_NET_DWC_SetVChat( FALSE );
      p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_START2;
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_MATCH_START2:
    if( GFL_NET_StateStartWifiRandomMatch_RateMode() )
    { 
      p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_WAIT;
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_MATCH_WAIT:
    {
      int ret;

      ret = GFL_NET_DWC_GetStepMatchResult();
      switch(ret)
      {
      case STEPMATCH_CONTINUE:
        /* 接続中 */
        break;

      case STEPMATCH_SUCCESS:
        p_wk->cancel_select_timeout = 0;
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CONNECT_START;
        break;

      case STEPMATCH_CANCEL:
        GF_ASSERT( 0 );
        break;

      case STEPMATCH_FAIL:
        GF_ASSERT( 0 );
        break;
      }
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_CONNECT_START:
    {
      u16 netID;
      netID = GFL_NET_SystemGetCurrentID();
      if( netID != GFL_NET_NO_PARENTMACHINE){  // 子機として接続が完了した
        if( GFL_NET_HANDLE_RequestNegotiation() )
        {
          p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CONNECT_CHILD;
        }
      }
      else
      {
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CONNECT_PARENT;
      }
      DEBUG_NET_Printf( "接続処理 %d^\n",netID );

      //相手が「やめますかはいorいいえ」中に放置することを考えて、
      //タイムアウトしたら、切断して、マッチメイクしなおす
      if( p_wk->cancel_select_timeout++ > CANCELSELECT_TIMEOUT )
      { 
        if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk, TRUE ) )
        { 
          DEBUG_NET_Printf( "ネゴシエーションタイムアウト\n" );
          p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CANCEL;
        }
      }
    }
    break;


  case WIFIBATTLEMATCH_NET_SEQ_CONNECT_CHILD:
    GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_NET_TIMINGSYNC_CONNECT);
    p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_TIMING_END;
    break;

  case WIFIBATTLEMATCH_NET_SEQ_CONNECT_PARENT:
    if( GFL_NET_HANDLE_GetNumNegotiation() != 0 )
    {
      if( GFL_NET_HANDLE_RequestNegotiation() )
      {
        GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_NET_TIMINGSYNC_CONNECT);
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_TIMING_END;
      }
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_TIMING_END:
    //相手が「やめますかはいorいいえ」中に放置することを考えて、
    //タイムアウトしたら、切断して、マッチメイクしなおす
    if( p_wk->cancel_select_timeout++ > CANCELSELECT_TIMEOUT )
    { 
      if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk, TRUE ) )
      { 
        DEBUG_NET_Printf( "ネゴシエーションタイムアウト\n" );
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CANCEL;
      }
    }

    if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_NET_TIMINGSYNC_CONNECT))
    {
      p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_END;
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_CANCEL:
    if( WIFIBATTLEMATCH_NET_SetDisConnect( p_wk, TRUE ) )
    { 
      p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_START;
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_MATCH_END:
    /* 何もしない */
    if( GFL_NET_IsParentMachine() )
    { 
      DEBUG_NET_Printf( "！親機 " );
    }
    else
    {
      DEBUG_NET_Printf( "！子機 " );
    }
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチメイクのシーケンスを取得
 *
 *	@param	const WIFIBATTLEMATCH_NET_WORK *cp_wk ワーク
 *
 *	@return シーケンス
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_SEQ WIFIBATTLEMATCH_NET_GetSeqMatchMake( const WIFIBATTLEMATCH_NET_WORK *cp_wk )
{ 
  return cp_wk->seq_matchmake;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチメイクを切断する
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	is_force  強制切断のときはTRUE　状況に合わせるときはFALSE
 *
 *	@return TRUEで処理終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_SetDisConnect( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_force )
{ 
  GFL_NET_StateWifiMatchEnd(!is_force);
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  接続を一時停止にする
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	is_stop                         TRUEで一時停止  FALSEで解除
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StopConnect( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_stop )
{ 
  p_wk->is_stop_connect = is_stop;
}

//----------------------------------------------------------------------------
/**
 *	@brief  キーを設定
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	key                             キー
 *	@param	value                           値
 */
//-----------------------------------------------------------------------------
static void MATCHMAKE_KEY_Set( WIFIBATTLEMATCH_NET_WORK *p_wk, MATCHMAKE_KEY key, int value )
{ 
  MATCHMAKE_KEY_WORK *p_key_wk = &p_wk->key_wk[ key ];

  p_key_wk->ivalue  = value;
  GFL_STD_MemCopy( sc_matchmake_key_str[ key ], p_key_wk->keyStr, 3 );
  p_key_wk->keyStr[3] = '\0';
  p_wk->key_wk[ key ].keyID  = DWC_AddMatchKeyInt( p_key_wk->keyID,
      p_key_wk->keyStr, &p_key_wk->ivalue );
  if( p_key_wk->keyID == 0 )
  {
      DEBUG_NET_Printf("AddMatchKey failed key=%d.value=%d\n",key, value);
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  接続コールバック
 *
 *	@param	int index
 *	@param	p_param_adrs 
 *
 *	@return 評価値  0以下はつながらない。　1以上は数字が高いほど接続されやすくなる値
 *	                                        （見つかったプレイヤーの評価値順）
 *	                                        ライブラリ内部で8ビット左シフトし下位8ビットに乱数をいれる
 */
//-----------------------------------------------------------------------------
static int WIFIBATTLEMATCH_RND_RATE_Eval_Callback( int index, void* p_param_adrs )
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_param_adrs;

  int value=0;
/*
  targetlv = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_LEVEL].keyStr,-1);
  targetfriend = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_IMAGE_FRIEND].keyStr,-1);
  targetmy = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_IMAGE_MY].keyStr,-1);
*/
 
  //@todo   ここに評価条件を書く
  value = 100;


  OS_TPrintf("評価コールバック %d \n",value );
  return value;

}
//フリーモード用
static int WIFIBATTLEMATCH_RND_FREE_Eval_Callback( int index, void* p_param_adrs )
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_param_adrs;

  int value=0;
/*
  targetlv = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_LEVEL].keyStr,-1);
  targetfriend = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_IMAGE_FRIEND].keyStr,-1);
  targetmy = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_IMAGE_MY].keyStr,-1);
*/
 
  //@todo   ここに評価条件を書く
  value = 100;


  OS_TPrintf("評価コールバック %d \n",value );
  return value;

}
//WIFI大会用
static int WIFIBATTLEMATCH_WIFI_Eval_Callback( int index, void* p_param_adrs )
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_param_adrs;

  int value=0;
/*
  targetlv = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_LEVEL].keyStr,-1);
  targetfriend = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_IMAGE_FRIEND].keyStr,-1);
  targetmy = DWC_GetMatchIntValue(index,pWork->aMatchKey[_MATCHKEY_IMAGE_MY].keyStr,-1);
*/
 
  //@todo   ここに評価条件を書く
  value = 100;


  OS_TPrintf("評価コールバック %d \n",value );
  return value;

}

//=============================================================================
/**
 *    統計・競争関係
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  SCの開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param  WIFIBATTLEMATCH_BTLRULE rule              ルール
 *	@param  BtlResult result          対戦結果
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_SC_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_MODE mode, WIFIBATTLEMATCH_BTLRULE rule, BtlResult result )
{ 
  if( GFL_NET_IsParentMachine() )
  { 
    DEBUG_NET_Printf( "！親機 ルール%d 結果%d\n", rule, result );
  }
  else
  { 
    DEBUG_NET_Printf( "！子機 ルール%d 結果%d\n", rule, result );
  }

  p_wk->is_sc_start = TRUE;
  p_wk->seq = 0;
  p_wk->report.btl_rule   = rule;
  p_wk->report.btl_result = result;
  p_wk->is_debug  = FALSE;
  p_wk->is_auth   = GFL_NET_IsParentMachine();
  
  switch( mode )
  { 
  case WIFIBATTLEMATCH_MODE_WIFI:    //WIFI大会
    p_wk->SC_CreateReportCoreFunc = DwcRap_Sc_CreateReportWifiCore;
    break;
  case WIFIBATTLEMATCH_MODE_LIVE:    //ライブ大会
    GF_ASSERT(0);
    break;
  case WIFIBATTLEMATCH_MODE_RANDOM:  //ランダム対戦　（マッチメイクのときにRANDOM＋０がフリー＋１がレーティングにしている）
    p_wk->SC_CreateReportCoreFunc = DwcRap_Sc_CreateReportRndCore;
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  SCの開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param  WIFIBATTLEMATCH_BTLRULE rule              ルール
 *	@param  BtlResult result          対戦結果
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_SC_StartDebug( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_SC_DEBUG_DATA * p_data, BOOL is_auth )
{ 

  p_wk->is_sc_start   = TRUE;
  p_wk->seq           = 0;
  p_wk->is_debug      = TRUE;
  p_wk->is_auth = is_auth;
  p_wk->p_debug_data  = p_data;
}
//----------------------------------------------------------------------------
/**
 *	@brief  SCの処理中
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param  DWCScResult               結果
 *	@retval TRUEならば処理終了  FALSEならば処理進行中or不正エラー
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_SC_Process( WIFIBATTLEMATCH_NET_WORK *p_wk, DWCScResult *p_result )
{ 
  DWCScResult ret;
  
  *p_result = DWC_SC_RESULT_NO_ERROR;

  //@todo 失敗したら相手に失敗を送る必要がある？
  if( p_wk->is_sc_start )
  { 
    switch( p_wk->seq )
    { 
    //初期化
    case WIFIBATTLEMATCH_SC_SEQ_INIT:
      p_wk->wait_cnt  = 0;
      { 
        ret = DWC_GdbInitialize( GAME_ID, p_wk->cp_user_data, DWC_GDB_SSL_TYPE_SERVER_AUTH );
        if( *p_result != DWC_GDB_ERROR_NONE )
        { 
          *p_result = ret;
          p_wk->is_gdb_start  = FALSE;
          DwcRap_Gdb_Finalize( p_wk );
          return FALSE;
        }
      }

      ret = DWC_ScInitialize( GAME_ID );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "SC:Init\n" );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_LOGIN;
      break;

    //認証情報取得
    case WIFIBATTLEMATCH_SC_SEQ_LOGIN:
      ret = DWC_ScGetLoginCertificate( &p_wk->player[0].mCertificate );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "SC:Login\n" );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SESSION_START;
      break;

    //セッション開始  （ホストが行い、クライアントにセッションIDを伝播する）
    case WIFIBATTLEMATCH_SC_SEQ_SESSION_START:
      if( GFL_NET_IsParentMachine() )
      { 
        p_wk->wait_cnt++;
        ret = DWC_ScCreateSessionAsync( DwcRap_Sc_CreateSessionCallback, TIMEOUT_MS, p_wk );
        if( ret != DWC_SC_RESULT_NO_ERROR )
        { 
          *p_result = ret;
          p_wk->is_sc_start  = FALSE;
          DwcRap_Sc_Finalize( p_wk );
          return FALSE;
        }
        DEBUG_NET_Printf( "SC:Session parent\n" );
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SESSION_WAIT;
      }
      else
      { 
        //クライアントはセッションを行わず次へ
        DEBUG_NET_Printf( "SC:Session child\n" );
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_START;
      }
      break;

    //セッション終了待ち
    case WIFIBATTLEMATCH_SC_SEQ_SESSION_WAIT:
      if( p_wk->wait_cnt == 0 )
      { 
        DEBUG_NET_Printf( "SC:Session process parent\n" );
        GFL_STD_MemCopy( DWC_ScGetSessionId(), p_wk->player[0].mSessionId, DWC_SC_SESSION_GUID_SIZE );
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_START;
      }
      else
      { 
        DWC_ScProcess();
        if( p_wk->result != DWC_SC_RESULT_NO_ERROR )
        {
          *p_result = p_wk->result;
          p_wk->is_sc_start  = FALSE;
          DwcRap_Sc_Finalize( p_wk );
          return FALSE;
        }
      }
      break;

    //ホストがクライアントへセッションIDを送るためのタイミングとり
   case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_START:
      GFL_STD_MemCopy( DWC_ScGetConnectionId(), p_wk->player[0].mConnectionId, DWC_SC_CONNECTION_GUID_SIZE );

      GFL_NET_HANDLE_TimingSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_SEND_PLAYERDATA_TIMING );
      p_wk->seq  = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_WAIT;
      break;

    //タイミング待ち
    case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimingSync( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_SEND_PLAYERDATA_TIMING ) )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA;
      }
      break;

    //ホストがクライアントへ自分のデータを送る
    case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA:
      if( GFL_NET_IsParentMachine() )
      { 
        //自分を送信
        if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_NETCMD_SEND_PLAYERDATA, 
              sizeof(DWC_SC_PLAYERDATA), &p_wk->player[0]) )
        { 
          DEBUG_NET_Printf( "SC:Senddata parent\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INTENTION_START;
        }
      }
      else
      { 
        //受け取り
        if( p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] )
        { 
          p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] = FALSE;
          //ホスト以外の人は、ホストがCreateSessionしたsessionIdを設定する
          ret = DWC_ScSetSessionId( p_wk->player[1].mSessionId );
          GFL_STD_MemCopy( DWC_ScGetSessionId(), p_wk->player[0].mSessionId, DWC_SC_SESSION_GUID_SIZE );
          if( ret != DWC_SC_RESULT_NO_ERROR )
          {
            *p_result = ret;
            p_wk->is_sc_start  = FALSE;
            DwcRap_Sc_Finalize( p_wk );
            return FALSE;
          }
          DEBUG_NET_Printf( "SC:Senddata child\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INTENTION_START;
        }
      }
      break;

    //送信するレポートの種類を通知
    case WIFIBATTLEMATCH_SC_SEQ_INTENTION_START:
      p_wk->wait_cnt++;
      //第一引数は親のみ正しいという値なのか？
      ret = DWC_ScSetReportIntentionAsync( p_wk->is_auth, DwcRap_Sc_SetReportIntentionCallback, TIMEOUT_MS, p_wk );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      } 
      DEBUG_NET_Printf( "SC:intention start\n" );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INTENTION_WAIT;
      break;

    //通知待ち
    case WIFIBATTLEMATCH_SC_SEQ_INTENTION_WAIT:
      if( p_wk->wait_cnt == 0 )
      { 
        DEBUG_NET_Printf( "SC:intention_wait\n" );
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_START;
      }
      else
      { 
        DWC_ScProcess();
        if( p_wk->result != DWC_SC_RESULT_NO_ERROR )
        {
          *p_result = p_wk->result;
          p_wk->is_sc_start  = FALSE;
          DwcRap_Sc_Finalize( p_wk );
          return FALSE;
        }
      }
      break;

    //クライアントの情報をホストがもらうためのタイミングとり
    case WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_START:
      GFL_NET_HANDLE_TimingSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_RETURN_PLAYERDATA_TIMING );
      p_wk->seq  = WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_WAIT;
      break;

    //タイミング待ち
    case WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimingSync( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_RETURN_PLAYERDATA_TIMING ) )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_PARENT;
      }
      break;

    //お互いのCCID（Intention ConnectionID）を交換する  まずは親データを子に送信
    case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_PARENT:
      if( GFL_NET_IsParentMachine() )
      { 
        if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_NETCMD_SEND_PLAYERDATA, 
              sizeof(DWC_SC_PLAYERDATA), &p_wk->player[0]) )
        { 
          DEBUG_NET_Printf( "SC:send data ccid parent\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD;
        }
      }
      else
      { 
        if( p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] )
        { 
          p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] = FALSE;
          DEBUG_NET_Printf( "SC:send data ccid parent\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD;
        }
      }
      break;

    //次は子のデータを送信
    case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD:
      if( !GFL_NET_IsParentMachine() )
      { 
        if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_NETCMD_SEND_PLAYERDATA, 
              sizeof(DWC_SC_PLAYERDATA), &p_wk->player[0]) )
        { 
          DEBUG_NET_Printf( "SC:send data ccid child\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_START;
        }
      }
      else
      { 
        if( p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] )
        { 
          p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] = FALSE;
          DEBUG_NET_Printf( "SC:send data ccid child\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_START;
        }
      }
      break;

    //受信後、レポート作成のためのタイミングとり
    case WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_START:
      GFL_NET_HANDLE_TimingSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_REPORT_TIMING );
      p_wk->seq  = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_WAIT;
      break;

    //タイミングまち
    case WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimingSync( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_REPORT_TIMING ) )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT;
      }
      break;

    //レポート作成
    case WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT:
      //レポート作成
      if( !p_wk->is_debug )
      { 
        ret = DwcRap_Sc_CreateReport( &p_wk->player[0], &p_wk->player[1], &p_wk->report, p_wk->is_auth, p_wk->SC_CreateReportCoreFunc );
      }
      else
      { 
        ret = DwcRap_Sc_CreateReportDebug( &p_wk->player[0], &p_wk->player[1], p_wk->p_debug_data, p_wk->is_auth );
      }

      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "SC:create report\n" );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SUBMIT_REPORT_START;
      break;

    //レポート送信開始
    case WIFIBATTLEMATCH_SC_SEQ_SUBMIT_REPORT_START:
      p_wk->wait_cnt++;
      ret = DWC_ScSubmitReportAsync( p_wk->player[0].mReport, p_wk->is_auth,
            DwcRap_Sc_SubmitReportCallback, TIMEOUT_MS, p_wk );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "SC:submit report start\n" );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SUBMIT_REPORT_WAIT;
      break;

    //レポート送信中
    case WIFIBATTLEMATCH_SC_SEQ_SUBMIT_REPORT_WAIT:
      if( p_wk->wait_cnt == 0 )
      { 
        DEBUG_NET_Printf( "SC:submit report wait\n" );
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_EXIT;
      }
      else
      { 
        DWC_ScProcess();
        if( p_wk->result != DWC_SC_RESULT_NO_ERROR )
        {
          *p_result = p_wk->result;
          p_wk->is_sc_start  = FALSE;
          DwcRap_Sc_Finalize( p_wk );
          return FALSE;
        }
      }
      break;

    //終了！
    case WIFIBATTLEMATCH_SC_SEQ_EXIT:
      DEBUG_NET_Printf( "SC:exit\n" );
      DwcRap_Sc_Finalize( p_wk );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_END;
      break;

    case WIFIBATTLEMATCH_SC_SEQ_END:
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  修復タイプ
 *
 *	@param	DWCScResult error エラー
 *
 *	@return 修理タイプ
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_SC_GetErrorRepairType( DWCScResult error )
{ 
  switch( error )
  { 
  case DWC_SC_RESULT_NO_ERROR :              ///< エラーは発生しませんでした
    return WIFIBATTLEMATCH_NET_ERROR_NONE;

  case DWC_SC_RESULT_NO_AVAILABILITY_CHECK:     ///< 初期化の前に、標準の GameSpy 可用性チェックが実行されませんでした
    GF_ASSERT(0);
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_INVALID_PARAMETERS:        ///< インターフェース関数に渡されたパラメータが無効です
    GF_ASSERT(0);
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_NOT_INITIALIZED:           ///< SDK が初期化されませんでした
    GF_ASSERT(0);
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL;

  case DWC_SC_RESULT_CORE_NOT_INITIALIZED:      ///< コアが初期化されませんでした
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

  case DWC_SC_RESULT_OUT_OF_MEMORY:             ///< SDK はリソースにメモリを割り当てられませんでした
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_CALLBACK_PENDING:          ///< アプリケーションに対し、操作が保留中であることを伝えます
    GF_ASSERT( 0 );
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_HTTP_ERROR:                ///< バックエンドが適正な HTTP によるレスポンスに失敗した場合のエラー発生
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

  case DWC_SC_RESULT_UNKNOWN_RESPONSE:          ///< SDK が結果を理解できない場合のエラー発生
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_RESPONSE_INVALID:          ///< SDK がバックエンドからのレスポンスを読み取れない場合のエラー発生
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_INVALID_DATATYPE:          ///< 無効なデータタイプ
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

  case DWC_SC_RESULT_REPORT_INCOMPLETE:         ///< レポートが不完全です
    GF_ASSERT(0);
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

  case DWC_SC_RESULT_REPORT_INVALID:            ///< レポートの一部またはすべてが無効です
    GF_ASSERT(0);
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

  case DWC_SC_RESULT_SUBMISSION_FAILED:         ///< レポートの送信に失敗しました
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

  case DWC_SC_RESULT_UNKNOWN_ERROR:             ///< SDK が認識できないエラー
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  default:
    GF_ASSERT(0);
    return 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  セッション作成コールバック
 *
 *	@param	DWCScResult theResult 結果
 *	@param	theUserData           自分が渡したアドレス
 */
//-----------------------------------------------------------------------------
static void DwcRap_Sc_CreateSessionCallback( DWCScResult theResult, void* theUserData )
{
  WIFIBATTLEMATCH_NET_WORK  *p_wk = theUserData;

  if (theResult == DWC_SC_RESULT_NO_ERROR)
  {
    DEBUG_NET_Printf( "[CreateSessionCallback]Session ID: %s\n", DWC_ScGetSessionId() );
    DEBUG_NET_Printf( "[CreateSessionCallback]Connection ID: %s\n", DWC_ScGetConnectionId() );
  }
  else
  {
    p_wk->result = theResult;
  }

  p_wk->wait_cnt--;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レポート送信意思通知コールバック
 *
 *	@param	theResult   結果
 *	@param	theUserData 自分で与えたアドレス
 */
//-----------------------------------------------------------------------------
static void DwcRap_Sc_SetReportIntentionCallback(DWCScResult theResult, void* theUserData)
{
  WIFIBATTLEMATCH_NET_WORK  *p_wk = theUserData;
  DWC_SC_PLAYERDATA   *p_player_data  = &p_wk->player[0];

  if (theResult == DWC_SC_RESULT_NO_ERROR)
  {
    GFL_STD_MemCopy( DWC_ScGetConnectionId(),p_player_data->mConnectionId, DWC_SC_CONNECTION_GUID_SIZE );
    DEBUG_NET_Printf( "[SetReportIntentionCallback]Connection ID: %s\n", DWC_ScGetConnectionId() );
  }
  else
  {
    DEBUG_NET_Printf( "SetReportIntentionCallbackエラー %d\n", theResult );
    p_wk->result = theResult;
  }
  p_wk->wait_cnt--;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レポート送信コールバック
 *
 *	@param	theResult   結果
 *	@param	theUserData 自分で与えたアドレス
 */
//-----------------------------------------------------------------------------
static void DwcRap_Sc_SubmitReportCallback(DWCScResult theResult, void* theUserData)
{
  WIFIBATTLEMATCH_NET_WORK  *p_wk = theUserData;

  if (theResult == DWC_SC_RESULT_NO_ERROR)
  {
    DEBUG_NET_Printf( "[SubmitReportCallback]success\n" );
  }
  else
  {
    p_wk->result = theResult;
  }

  p_wk->wait_cnt--;

}

//----------------------------------------------------------------------------
/**
 *	@brief  終了時のシステム破棄関数
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Sc_Finalize( WIFIBATTLEMATCH_NET_WORK *p_wk )
{
  if( p_wk->seq > WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT )
  {
    DEBUG_NET_Printf( "destroy report\n" );
    DWC_ScDestroyReport( p_wk->player[0].mReport );
  }

  if( p_wk->seq >= WIFIBATTLEMATCH_SC_SEQ_INIT )
  {
    DEBUG_NET_Printf( "sc shutdown\n" );
    DWC_ScShutdown();
    DWC_GdbShutdown();           // 簡易データベースライブラリの終了
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レポートに書き込む処理
 *
 *	@param	DWC_SC_PLAYERDATA *p_player プレイヤー
 *	@param	DWC_SC_WRITE_DATA *cp_data  書き込む情報
 *
 *	@return 結果
 */
//-----------------------------------------------------------------------------
static DWCScResult DwcRap_Sc_CreateReport( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const DWC_SC_WRITE_DATA *cp_data, BOOL is_auth, DWCRAP_SC_CREATE_REPORT_CORE_FUNC SC_CreateReportCoreFunc )
{ 
  DWCScResult ret;

  //レポート作成
  ret = DWC_ScCreateReport( ATLAS_RULE_SET_VERSION, PLAYER_NUM, TEAM_NUM, &p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  {
    DEBUG_NET_Printf( "Report:Create%d\n",ret );
    return ret;
  }
  
  // レポートにグローバルデータを書き込む
  ret = DWC_ScReportBeginGlobalData( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  {
    DEBUG_NET_Printf( "Report:Grobal%d\n",ret );
    return ret;
  }

  // レポートにプレイヤーデータを書き込む
  ret = DWC_ScReportBeginPlayerData( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:Player%d\n",ret );
    return ret;
  }

  //自分のレポートを作成
  ret = DWC_ScReportBeginNewPlayer( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:New%d\n",ret );
    return ret;
  }

  { 
      
    DWCScGameResult game_result;

    switch( cp_data->btl_result )
    { 
    case BTL_RESULT_RUN_ENEMY:
      /* fallthrough */
    case BTL_RESULT_WIN:
      game_result = DWC_SC_GAME_RESULT_WIN;
      break;
    case BTL_RESULT_RUN:
      /* fallthrough */
    case BTL_RESULT_LOSE:
      game_result = DWC_SC_GAME_RESULT_LOSS;
      break;
    case BTL_RESULT_DRAW:
      game_result = DWC_SC_GAME_RESULT_DRAW;
      break;
    default:
      GF_ASSERT_MSG(0, "バトル結果不正値 %d\n", cp_data->btl_result );
    }
    ret = DWC_ScReportSetPlayerData(
                p_my->mReport,
                0,
                p_my->mConnectionId,
                0,
                game_result,
                p_my->mCertificate,
                p_my->mStatsAuthdata );

    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      DEBUG_NET_Printf( "Report:SetData%d\n",ret );
      return ret;
    }
  }

  //成績書き込み
  ret = SC_CreateReportCoreFunc( p_my, cp_data, TRUE );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:Core%d\n",ret );
    return ret;
  }

  //相手のレポートを作成    相手のなのでデータは逆になる
  ret = DWC_ScReportBeginNewPlayer( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:New%d\n",ret );
    return ret;
  }

  { 
    DWCScGameResult game_result;

    //相手はぎゃくになる
    switch( cp_data->btl_result )
    { 
    case BTL_RESULT_RUN_ENEMY:
      /* fallthrough */
    case BTL_RESULT_WIN:
      game_result = DWC_SC_GAME_RESULT_LOSS;
      break;
    case BTL_RESULT_RUN:
      /* fallthrough */
    case BTL_RESULT_LOSE:
      game_result = DWC_SC_GAME_RESULT_WIN;
      break;
    case BTL_RESULT_DRAW:
      game_result = DWC_SC_GAME_RESULT_DRAW;
      break;
    default:
      GF_ASSERT_MSG(0, "バトル結果不正値 %d\n", cp_data->btl_result );
    }
    ret = DWC_ScReportSetPlayerData(
                p_my->mReport,
                1,
                p_other->mConnectionId,
                0,
                game_result,
                p_other->mCertificate,
                p_other->mStatsAuthdata );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      DEBUG_NET_Printf( "Report:SetData%d\n",ret );
      return ret;
    }
  }

  //成績書き込み
  ret = SC_CreateReportCoreFunc( p_my, cp_data, FALSE );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:Core%d\n",ret );
    return ret;
  }

  // レポートにチームデータを書き込む
  ret = DWC_ScReportBeginTeamData( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:Team%d\n",ret );
    return ret;
  }

  //　レポートの終端を宣言する
  ret = DWC_ScReportEnd( p_my->mReport, is_auth, DWC_SC_GAME_STATUS_COMPLETE );
  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レポートに書き込む処理
 *
 *	@param	DWC_SC_PLAYERDATA *p_player プレイヤー
 *	@param	DWC_SC_WRITE_DATA *cp_data  書き込む情報
 *
 *	@return 結果
 */
//-----------------------------------------------------------------------------
static DWCScResult DwcRap_Sc_CreateReportDebug( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const WIFIBATTLEMATCH_SC_DEBUG_DATA * cp_data, BOOL is_auth )
{ 
  DWCScResult ret;

  //レポート作成
  ret = DWC_ScCreateReport( ATLAS_RULE_SET_VERSION, PLAYER_NUM, TEAM_NUM, &p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  {
    DEBUG_NET_Printf( "Report:Create%d\n",ret );
    return ret;
  }
  
  // レポートにグローバルデータを書き込む
  ret = DWC_ScReportBeginGlobalData( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  {
    DEBUG_NET_Printf( "Report:Grobal%d\n",ret );
    return ret;
  }

  // レポートにプレイヤーデータを書き込む
  ret = DWC_ScReportBeginPlayerData( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:Player%d\n",ret );
    return ret;
  }

  //自分のレポートを作成
  ret = DWC_ScReportBeginNewPlayer( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:New%d\n",ret );
    return ret;
  }

  { 
    ret = DWC_ScReportSetPlayerData(
                p_my->mReport,
                0,
                p_my->mConnectionId,
                0,
                cp_data->my_result,
                p_my->mCertificate,
                p_my->mStatsAuthdata );

    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      DEBUG_NET_Printf( "Report:SetData%d\n",ret );
      return ret;
    }
  }

  //成績書き込み
  //シングル
  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_SINGLE, cp_data->my_single_rate );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SINGLE_WIN_COUNTER, cp_data->my_single_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
      return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SINGLE_LOSE_COUNTER, cp_data->my_single_lose );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  //ダブル
  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_DOUBLE, cp_data->my_double_rate );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_DOUBLE_WIN_COUNTER, cp_data->my_double_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_DOUBLE_LOSE_COUNTER, cp_data->my_double_lose );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  //トリプル
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_TRIPLE, cp_data->my_triple_rate );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_TRIPLE_WIN_COUNTER, cp_data->my_triple_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_TRIPLE_LOSE_COUNTER, cp_data->my_triple_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
  //ローテ
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_ROTATE, cp_data->my_rot_rate );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_ROTATE_WIN_COUNTER, cp_data->my_rot_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_ROTATE_LOSE_COUNTER, cp_data->my_rot_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }

  //シューター
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_SHOOTER, cp_data->my_shooter_rate );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SHOOTER_WIN_COUNTER, cp_data->my_shooter_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SHOOTER_LOSE_COUNTER, cp_data->my_shooter_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
  //その他
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_CHEATS_COUNTER, cp_data->my_cheat );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }


  //相手のレポートを作成    相手のなのでデータは逆になる
  ret = DWC_ScReportBeginNewPlayer( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:New%d\n",ret );
    return ret;
  }

  { 

    ret = DWC_ScReportSetPlayerData(
                p_my->mReport,
                1,
                p_other->mConnectionId,
                0,
                cp_data->you_result,
                p_other->mCertificate,
                p_other->mStatsAuthdata );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      DEBUG_NET_Printf( "Report:SetData%d\n",ret );
      return ret;
    }
  }

  //成績書き込み
  //成績書き込み
  //シングル
  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_SINGLE, cp_data->you_single_rate );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SINGLE_WIN_COUNTER, cp_data->you_single_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
      return ret;
  }
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SINGLE_LOSE_COUNTER, cp_data->you_single_lose );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  //ダブル
  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_DOUBLE, cp_data->you_double_rate );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_DOUBLE_WIN_COUNTER, cp_data->you_double_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_DOUBLE_LOSE_COUNTER, cp_data->you_double_lose );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  //トリプル
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_TRIPLE, cp_data->you_triple_rate );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_TRIPLE_WIN_COUNTER, cp_data->you_triple_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_TRIPLE_LOSE_COUNTER, cp_data->you_triple_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
  //ローテ
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_ROTATE, cp_data->you_rot_rate );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_ROTATE_WIN_COUNTER, cp_data->you_rot_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_ROTATE_LOSE_COUNTER, cp_data->you_rot_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }

  //シューター
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_SHOOTER, cp_data->you_shooter_rate );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SHOOTER_WIN_COUNTER, cp_data->you_shooter_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SHOOTER_LOSE_COUNTER, cp_data->you_shooter_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
  //その他
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_CHEATS_COUNTER, cp_data->you_cheat );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }

  // レポートにチームデータを書き込む
  ret = DWC_ScReportBeginTeamData( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:Team%d\n",ret );
    return ret;
  }

  //　レポートの終端を宣言する
  ret = DWC_ScReportEnd( p_my->mReport, is_auth, DWC_SC_GAME_STATUS_COMPLETE );
  return ret;
}


//----------------------------------------------------------------------------
/**
 *	@brief  レポート作成
 *
 *	@param	DWC_SC_PLAYERDATA *p_my           書き込むレポートの持ち主
 *	@param  const DWC_SC_WRITE_DATA *cp_data  書き込むデータ
 *	@param	is_my                             TRUEならば自分、FALSEならば相手
 *
 *	@return 書き込み結果
 */
//-----------------------------------------------------------------------------
static DWCScResult DwcRap_Sc_CreateReportRndCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my )
{ 
  const BtlResult result  = cp_data->btl_result;
  BOOL is_win;
  DWCScResult ret;

  //もし相手の情報を書き込むならば、結果を反対にする
  is_win = (cp_data->btl_result == BTL_RESULT_WIN || cp_data->btl_result == BTL_RESULT_RUN_ENEMY);  
  if( !is_my )
  {
    is_win  ^= 1;
  }

  DEBUG_NET_Printf( "-----------Report_start-------------\n" );
  DEBUG_NET_Printf( "レポート先%d\n", is_my );
  DEBUG_NET_Printf( "勝ち%d\n", is_win );
  DEBUG_NET_Printf( "負け%d\n", !is_win );
  DEBUG_NET_Printf( "-----------Report_end-------------\n" );

  //ルールによって送るものが違う
  switch( cp_data->btl_rule )
  { 
  case BTL_RULE_SINGLE:    ///< シングル
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_SINGLE, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SINGLE_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SINGLE_LOSE_COUNTER, !is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  case BTL_RULE_DOUBLE:    ///< ダブル
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_DOUBLE, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_DOUBLE_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_DOUBLE_LOSE_COUNTER, !is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  case BTL_RULE_TRIPLE:    ///< トリプル
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_TRIPLE, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_TRIPLE_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_TRIPLE_LOSE_COUNTER, !is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  case BTL_RULE_ROTATION:  ///< ローテーション
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_ROTATE, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_ROTATE_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_ROTATE_LOSE_COUNTER, !is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  default:
    GF_ASSERT(0);
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レポート作成  WIFI版
 *
 *	@param	DWC_SC_PLAYERDATA *p_my           書き込むレポートの持ち主
 *	@param  const DWC_SC_WRITE_DATA *cp_data  書き込むデータ
 *	@param	is_my                             TRUEならば自分、FALSEならば相手
 *
 *	@return 書き込み結果
 */
//-----------------------------------------------------------------------------
static DWCScResult DwcRap_Sc_CreateReportWifiCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my )
{ 
  const BtlResult result  = cp_data->btl_result;
  BOOL is_win;
  DWCScResult ret;

  //もし相手の情報を書き込むならば、結果を反対にする
  is_win = (cp_data->btl_result == BTL_RESULT_WIN || cp_data->btl_result == BTL_RESULT_RUN_ENEMY);  
  if( !is_my )
  {
    is_win  ^= 1;
  }

  DEBUG_NET_Printf( "-----------Report_start-------------\n" );
  DEBUG_NET_Printf( "レポート先%d\n", is_my );
  DEBUG_NET_Printf( "勝ち%d\n", is_win );
  DEBUG_NET_Printf( "負け%d\n", !is_win );
  DEBUG_NET_Printf( "-----------Report_end-------------\n" );

  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_WIFICUP, 1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_WIFICUP_WIN_COUNTER, is_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_WIFICUP_LOSE_COUNTER, !is_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }

  return ret;
}


//=============================================================================
/**
 *        コールバック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤーデータ受信コールバック
 *
 *	@param	int netID     ネットID
 *	@param	int size      サイズ
 *	@param	void* pData   データ
 *	@param	pWk           自分であたえたアドレス
 *	@param	pNetHandle    ネットハンドル
 */
//-----------------------------------------------------------------------------
static void DwcRap_Sc_RecvPlayerData(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle)
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_wk_adrs;

  if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
  {
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    return;//自分のは今は受け取らない
  }

  DEBUG_NET_Printf( "プレイヤーデータ受信コールバック！netID=%d size=%d \n", netID, size );

  GFL_STD_MemCopy( cp_data_adrs, &p_wk->player[1], sizeof(DWC_SC_PLAYERDATA));

  DEBUG_NET_Printf( "[Other]Session ID: %s\n", p_wk->player[1].mSessionId );
  DEBUG_NET_Printf( "[Other]Connection ID: %s\n", p_wk->player[1].mConnectionId );
  p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  敵データ受信コールバック
 *
 *	@param	int netID     ネットID
 *	@param	int size      サイズ
 *	@param	void* pData   データ
 *	@param	pWk           自分であたえたアドレス
 *	@param	pNetHandle    ネットハンドル
 */
//-----------------------------------------------------------------------------
static void WifiBattleMatch_RecvEnemyData(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle)
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_wk_adrs;

  if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
  {
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    return;//自分のは今は受け取らない
  }

  DEBUG_NET_Printf( "敵データ受信コールバック！netID=%d size=%d \n", netID, size );

  p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_GAMEDATA] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファアドレス
 *
 *	@param	netID ネットID
 *	@param	pWork ワーク
 *	@param	size  サイズ
 *
 *	@return バッファアドレス
 */
//-----------------------------------------------------------------------------
static u8* WifiBattleMatch_RecvEnemyDataBuffAddr(int netID, void* p_wk_adrs, int size)
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_wk_adrs;

  GF_ASSERT_MSG( p_wk->p_enemy_data, "wk 0x%x buff 0x%x\n", p_wk, p_wk->p_enemy_data );

  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    GF_ASSERT( 0 );
    return NULL;//自分のは受け取らない
  }
  return (u8 *)(p_wk->p_enemy_data);
}
//----------------------------------------------------------------------------
/**
 *	@brief  マッチングキャンセルフラグコールバック
 *
 *	@param	int netID     ネットID
 *	@param	int size      サイズ
 *	@param	void* pData   データ
 *	@param	pWk           自分であたえたアドレス
 *	@param	pNetHandle    ネットハンドル
 */
//-----------------------------------------------------------------------------
static void WifiBattleMatch_RecvMatchCancel(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle)
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_wk_adrs;

  if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
  {
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    return;//自分のは今は受け取らない
  }

  DEBUG_NET_Printf( "キャンセル受信コールバック！netID=%d size=%d \n", netID, size );

  p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_CANCELREQUEST] = TRUE;

}

//=============================================================================
/**
 *    データベース
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  DB開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_GDB_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_GDB_GETTYPE type, void *p_wk_adrs )
{ 
  p_wk->is_gdb_start  = TRUE;
  p_wk->seq           = 0;
  p_wk->p_get_wk      = p_wk_adrs;


  switch( type )
  { 
  case WIFIBATTLEMATCH_GDB_GET_RND_SCORE:
    p_wk->pp_table_name  = ATLAS_RND_GetFieldName();
    p_wk->table_name_num= ATLAS_RND_GetFieldNameNum();
    p_wk->gdb_get_record_callback = DwcRap_Gdb_Rnd_GetRecordsCallback;
    break;

  case WIFIBATTLEMATCH_GDB_GET_WIFI_SCORE:
    p_wk->pp_table_name  = ATLAS_WIFI_GetFieldName();
    p_wk->table_name_num= ATLAS_WIFI_GetFieldNameNum();
    p_wk->gdb_get_record_callback = DwcRap_Gdb_Wifi_GetRecordsCallback;
    break;

  case WIFIBATTLEMATCH_GDB_GET_RECORDID:
    { 
      static const char *sc_get_record_id_tbl[]  =
      { 
        SAKE_STAT_RECORDID
      };

      p_wk->pp_table_name  = sc_get_record_id_tbl;
      p_wk->table_name_num= NELEMS( sc_get_record_id_tbl );
      p_wk->gdb_get_record_callback = DwcRap_Gdb_RecordID_GetRecordsCallback;
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  DBの処理中
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param  DWCScResult               結果
 *	@retval TRUEならば処理終了  FALSEならば処理進行中or不正エラー
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_GDB_Process( WIFIBATTLEMATCH_NET_WORK *p_wk, DWCGdbError *p_result )
{ 
  DWCError    error;
  DWCGdbState state;

  *p_result = DWC_GDB_ERROR_NONE;

  if( p_wk->is_gdb_start )  
  { 
    switch( p_wk->seq )
    { 
    case WIFIBATTLEMATCH_GDB_SEQ_INIT:
      *p_result = DWC_GdbInitialize( GAME_ID, p_wk->cp_user_data, WIFIBATTLEMATCH_NET_SSL_TYPE );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "GDB:Init\n" );
      p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_GET_START;
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_GET_START:
      *p_result = DWC_GdbGetMyRecordsAsync( WIFIBATTLEMATCH_NET_TABLENAME, p_wk->pp_table_name, p_wk->table_name_num, p_wk->gdb_get_record_callback, p_wk->p_get_wk );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "GDB:Get start\n" );
      p_wk->seq= WIFIBATTLEMATCH_GDB_SEQ_GET_WAIT;
      p_wk->async_timeout = 0;
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_GET_WAIT:
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          GF_ASSERT(0);
          DwcRap_Gdb_Finalize( p_wk );
          return FALSE;
        }

        state = DWC_GdbGetState();
        if( DWC_GDB_STATE_IN_ASYNC_PROCESS == state )
        { 
          DWC_GdbProcess();
        }
        else
        { 
          if( state != DWC_GDB_STATE_IDLE )
          { 
            GF_ASSERT(0);
            DwcRap_Gdb_Finalize( p_wk );
            return FALSE;
          }
          else
          { 
            DEBUG_NET_Printf( "GDB:Get wait\n" );
            p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_GET_END;
          }
        }
      }
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_GET_END:
      { 
        DWCGdbAsyncResult ret;
        ret = DWC_GdbGetAsyncResult();
        if( ret != DWC_GDB_ASYNC_RESULT_SUCCESS )
        { 
          GF_ASSERT_MSG(0, "GDB:AsyncResult %d\n", ret);
          p_wk->is_gdb_start  = FALSE;
          DwcRap_Gdb_Finalize( p_wk );
          return FALSE;
        }
      }
      DEBUG_NET_Printf( "GDB:Get end\n" );
      p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_EXIT;
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_EXIT:
      DwcRap_Gdb_Finalize( p_wk );
      DEBUG_NET_Printf( "GDB:Get exit\n" );
      p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_END;
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_END:
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了時のシステム破棄関数
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_Finalize( WIFIBATTLEMATCH_NET_WORK *p_wk )
{
  DEBUG_NET_Printf( "GDB_ShotDown!" );
  DWC_GdbShutdown();           // 簡易データベースライブラリの終了
}

//----------------------------------------------------------------------------
/**
 *	@brief  レコード取得コールバック  ランダムマッチ版
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_Rnd_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param)
{
    int i,j;
    WIFIBATTLEMATCH_GDB_RND_SCORE_DATA *p_data = user_param;
    GFL_STD_MemClear( p_data, sizeof(WIFIBATTLEMATCH_GDB_RND_SCORE_DATA) );

    for (i = 0; i < record_num; i++)
    {
      DEBUG_NET_Printf("!!!=====gdb_Print:======\n");
      for (j = 0; j < ATLAS_RND_GetFieldNameNum(); j++)   // user_param -> field_num
      {
        DWCGdbField* field  = &records[i][j];

        if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_SINGLE_WIN_COUNTER ) ) )
        { 
          p_data->single_win  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_SINGLE_LOSE_COUNTER) ) )
        { 
          p_data->single_lose  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_DOUBLE_WIN_COUNTER ) ) )
        { 
          p_data->double_win  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_DOUBLE_LOSE_COUNTER) ) )
        { 
          p_data->double_lose  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_ROTATE_WIN_COUNTER ) ) )
        { 
          p_data->rot_win  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_ROTATE_LOSE_COUNTER) ) )
        { 
          p_data->rot_lose  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_TRIPLE_WIN_COUNTER ) ) )
        { 
          p_data->triple_win  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_TRIPLE_LOSE_COUNTER) ) )
        { 
          p_data->triple_lose  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_SHOOTER_WIN_COUNTER ) ) )
        { 
          p_data->shooter_win  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_SHOOTER_LOSE_COUNTER) ) )
        { 
          p_data->shooter_lose  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_SINGLE ) ) )
        { 
          p_data->single_rate  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_DOUBLE) ) )
        { 
          p_data->double_rate  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_ROTATE) ) )
        { 
          p_data->rot_rate  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_TRIPLE) ) )
        { 
          p_data->triple_rate  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_SHOOTER) ) )
        { 
          p_data->shooter_rate  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(DISCONNECTS_COUNTER) ) )
        { 
          p_data->disconnect  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(CHEATS_COUNTER) ) )
        { 
          p_data->cheat  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_INITIAL_PROFILE_ID ) )
        { 
          p_data->init_profileID  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_NOW_PROFILE_ID ) )
        { 
          p_data->now_profileID  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_LAST_LOGIN_DATETIME ) )
        { 
        }
        print_field( field );
        DEBUG_NET_Printf(" ");
      }
      DEBUG_NET_Printf("!!!====================\n");
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レコード取得コールバック  WiFI大会版
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_Wifi_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param)
{
    int i,j;
    WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA *p_data = user_param;
    GFL_STD_MemClear( p_data, sizeof(WIFIBATTLEMATCH_GDB_RND_SCORE_DATA) );

    for (i = 0; i < record_num; i++)
    {
      DEBUG_NET_Printf("!!!=====gdb_Print:======\n");
      for (j = 0; j < ATLAS_WIFI_GetFieldNameNum(); j++)   // user_param -> field_num
      {
        DWCGdbField* field  = &records[i][j];

        if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_WIFICUP_WIN_COUNTER ) ) )
        { 
          p_data->win  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_WIFICUP_LOSE_COUNTER) ) )
        { 
          p_data->lose  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_WIFICUP ) ) )
        { 
          p_data->rate  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(DISCONNECTS_WIFICUP_COUNTER) ) )
        { 
          p_data->disconnect  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(CHEATS_WIFICUP_COUNTER) ) )
        { 
          p_data->cheat  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(CHEATS_WIFICUP_COUNTER) ) )
        { 
          p_data->cheat  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_WIFICUP_POKEMON_PARTY ) )
        { 
          DWCGdbBinaryData  *p_binary = &field->value.binary_data;
          GF_ASSERT_MSG( p_binary->size <= WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE, "not %d <= %d" , p_binary->size, WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE );
          GFL_STD_MemCopy( p_binary->data, p_data->pokeparty, p_binary->size );
        }
        print_field( field );
        DEBUG_NET_Printf(" ");
      }
      DEBUG_NET_Printf("!!!====================\n");
    }
}
//----------------------------------------------------------------------------
/**
 *	@brief  レコード取得コールバック  レコードIDのみ
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_RecordID_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param)
{ 
    int i,j;
    int *p_data = user_param;

    for (i = 0; i < record_num; i++)
    {
      DEBUG_NET_Printf("!!!=====gdb_Print:======\n" );
      for (j = 0; j < 1; j++)   // user_param -> field_num
      {
        DWCGdbField* field  = &records[i][j];

        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORDID ) )
        { 
          *p_data  = field->value.int_s32;
        }
        print_field( field );
        DEBUG_NET_Printf(" ");
      }
      DEBUG_NET_Printf("!!!====================\n");
    }
}


#ifdef PM_DEBUG
static void print_field(DWCGdbField* field) // レコードをデバッグ出力する。
{
    DEBUG_NET_Printf("%s[", field->name);
    switch (field->type)
    {
    case DWC_GDB_FIELD_TYPE_BYTE:
        DEBUG_NET_Printf("BYTE]:%d\n", field->value.int_u8);
        break;
    case DWC_GDB_FIELD_TYPE_SHORT:
        DEBUG_NET_Printf("SHORT]:%d\n", field->value.int_s16);
        break;
    case DWC_GDB_FIELD_TYPE_INT:
        DEBUG_NET_Printf("INT]:%d\n", field->value.int_s32);
        break;
    case DWC_GDB_FIELD_TYPE_FLOAT:
        DEBUG_NET_Printf("FLOAT]:%f\n", field->value.float_f64);
        break;
    case DWC_GDB_FIELD_TYPE_ASCII_STRING:
        DEBUG_NET_Printf("ASCII]:%s\n", field->value.ascii_string);
        break;
    case DWC_GDB_FIELD_TYPE_UNICODE_STRING:
        DEBUG_NET_Printf("UNICODE]\n");
        break;
    case DWC_GDB_FIELD_TYPE_BOOLEAN:
        DEBUG_NET_Printf("BOOL]:%d\n", field->value.boolean);
        break;
    case DWC_GDB_FIELD_TYPE_DATE_AND_TIME:
        DEBUG_NET_Printf("TIME]:%lld\n", field->value.datetime);
        break;
    case DWC_GDB_FIELD_TYPE_BINARY_DATA:
        DEBUG_NET_Printf("BINARY]:%d\n", field->value.binary_data.size);
        break;
    }
}
#endif
//----------------------------------------------------------------------------
/**
 *	@brief  GDBへデータ書き込み
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	type                            データタイプ
 *	@param	void *cp_wk_adrs                書き込むデータ
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_GDB_StartWrite( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_GDB_WRITETYPE type, const void *cp_wk_adrs )
{ 

  p_wk->is_gdb_w_start  = TRUE;
  p_wk->seq             = 0;
  switch( type )
  { 
  case WIFIBATTLEMATCH_GDB_WRITE_DEBUGALL:
    { 
      int i;
      const WIFIBATTLEMATCH_GDB_RND_SCORE_DATA *cp_data  = cp_wk_adrs;

      p_wk->table_name_num  = ATLAS_RND_GetFieldNameNum();
      for( i = 0; i < p_wk->table_name_num; i++ )
      { 
        p_wk->p_field_buff[i].name  = (char*)ATLAS_RND_GetFieldName()[i];
        p_wk->p_field_buff[i].type  = sc_field_type[i];
        p_wk->p_field_buff[i].value.int_s32 = cp_data->arry[i];
      }
    }
    break;

  case WIFIBATTLEMATCH_GDB_WRITE_POKEPARTY:
    p_wk->table_name_num  = 1;
    p_wk->p_field_buff[0].name  = SAKE_STAT_WIFICUP_POKEMON_PARTY;
    p_wk->p_field_buff[0].type  = DWC_GDB_FIELD_TYPE_BINARY_DATA;
    p_wk->p_field_buff[0].value.binary_data.data = (u8*)cp_wk_adrs;
    p_wk->p_field_buff[0].value.binary_data.size = WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE;
    break;

  case WIFIBATTLEMATCH_GDB_WRITE_WIFI_SCORE:
    { 
      const WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA *cp_data  = cp_wk_adrs;

      p_wk->p_field_buff[0].name  = ATLAS_GET_KEY_NAME( ARENA_ELO_RATING_1V1_WIFICUP );
      p_wk->p_field_buff[0].type  = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[0].value.int_s32 = cp_data->rate;
      p_wk->p_field_buff[1].name  = ATLAS_GET_KEY_NAME( CHEATS_WIFICUP_COUNTER );
      p_wk->p_field_buff[1].type  = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[1].value.int_s32 = cp_data->cheat;
      p_wk->p_field_buff[2].name  = ATLAS_GET_KEY_NAME( DISCONNECTS_WIFICUP_COUNTER );
      p_wk->p_field_buff[2].type  = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[2].value.int_s32 = cp_data->disconnect;
      p_wk->p_field_buff[3].name  = ATLAS_GET_KEY_NAME( NUM_WIFICUP_LOSE_COUNTER );
      p_wk->p_field_buff[3].type  = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[3].value.int_s32 = cp_data->lose;
      p_wk->p_field_buff[4].name  = ATLAS_GET_KEY_NAME( NUM_WIFICUP_WIN_COUNTER );
      p_wk->p_field_buff[4].type  = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[4].value.int_s32 = cp_data->win;

      p_wk->table_name_num  = 5;
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  GDBへデータ書き込みメイン
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	DWCUserData *cp_user_data       ユーザーデータ
 *	@param	*p_result                       エラー値
 *
 *	@return TRUEで終了  FALSEで破棄
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_GDB_ProcessWrite( WIFIBATTLEMATCH_NET_WORK *p_wk, DWCGdbError *p_result )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
    SEQ_EXIT,
  };

  DWCError    error;
  DWCGdbState state;

  *p_result = DWC_GDB_ERROR_NONE;

  if( p_wk->is_gdb_w_start )  
  { 
    switch( p_wk->seq )
    { 
    case SEQ_INIT:
      *p_result = DWC_GdbInitialize( GAME_ID, p_wk->cp_user_data, WIFIBATTLEMATCH_NET_SSL_TYPE );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_w_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return TRUE;
      }
      DEBUG_NET_Printf( "GDB:Init\n" );
      p_wk->seq = SEQ_START;
      break;

    case SEQ_START:
      *p_result = DWC_GdbUpdateRecordAsync( WIFIBATTLEMATCH_NET_TABLENAME, p_wk->sake_record_id, p_wk->p_field_buff, p_wk->table_name_num );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_w_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return TRUE;
      }
      DEBUG_NET_Printf( "GDB:Get start\n" );
      p_wk->async_timeout = 0;
      p_wk->seq= SEQ_WAIT;
      break;

    case SEQ_WAIT:
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          GF_ASSERT(0);
          DwcRap_Gdb_Finalize( p_wk );
          return TRUE;
        }

        state = DWC_GdbGetState();
        if( DWC_GDB_STATE_IN_ASYNC_PROCESS == state )
        { 
          DWC_GdbProcess();
        }
        else
        { 
          if( state != DWC_GDB_STATE_IDLE )
          { 
            GF_ASSERT(0);
            DwcRap_Gdb_Finalize( p_wk );
            return TRUE;
          }
          else
          { 
            DEBUG_NET_Printf( "GDB:Get wait\n" );
            p_wk->seq = SEQ_END;
          }
        }
      }
      break;

    case SEQ_END:
      { 
        DWCGdbAsyncResult ret;
        ret = DWC_GdbGetAsyncResult();
        if( ret != DWC_GDB_ASYNC_RESULT_SUCCESS )
        { 
          GF_ASSERT_MSG(0, "GDB:AsyncResult %d\n", ret);
          p_wk->is_gdb_w_start  = FALSE;
          DwcRap_Gdb_Finalize( p_wk );
          return TRUE;
        }
      }
      DEBUG_NET_Printf( "GDB:Get end\n" );
      p_wk->seq = SEQ_EXIT;
      break;

    case SEQ_EXIT:
      DwcRap_Gdb_Finalize( p_wk );
      DEBUG_NET_Printf( "GDB:Get exit\n" );
      DEBUG_NET_Printf( "sake テーブルID %d\n", p_wk->sake_record_id );
      return TRUE;

    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラー修正タイプを取得
 *
 *	@param	DWCGdbError error   エラー
 *
 *	@return エラー修正タイプ
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_GDB_GetErrorRepairType( DWCGdbError error )
{ 
  OS_TPrintf( "WIFIBATTLEMATCH_GDB_GetErrorRepairType %d\n", error );
  switch( error )
  { 
    case DWC_GDB_ERROR_NONE :                 ///< 正常に終了しました。
      return WIFIBATTLEMATCH_NET_ERROR_NONE;

    case DWC_GDB_ERROR_OUT_OF_MEMORY :        ///< case DWC_Initで与えたアロケータからメモリを確保できませんでした。
      GF_ASSERT(0);
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

    case DWC_GDB_ERROR_NOT_AVAILABLE :        ///< 利用可能ではない
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

    case DWC_GDB_ERROR_CORE_SHUTDOWN :        ///< 
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

    case DWC_GDB_ERROR_NOT_LOGIN :            ///< WiFiConnectionに正しくログインしていません。
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

    case DWC_GDB_ERROR_NOT_AUTHENTICATED :    ///< WiFiConnectionに正しくログインしていません。
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

    case DWC_GDB_ERROR_BAD_INPUT :            ///<
    case DWC_GDB_ERROR_BAD_TABLENAME :        ///<
    case DWC_GDB_ERROR_BAD_FIELDS :           ///<
    case DWC_GDB_ERROR_BAD_NUM_FIELDS :       ///<
    case DWC_GDB_ERROR_BAD_FIELD_NAME :      ///<
    case DWC_GDB_ERROR_BAD_FIELD_TYPE :      ///<
    case DWC_GDB_ERROR_BAD_FIELD_VALUE :     ///<
    case DWC_GDB_ERROR_BAD_OFFSET :          ///<
    case DWC_GDB_ERROR_BAD_MAX :             ///<
    case DWC_GDB_ERROR_BAD_RECORDIDS :       ///<
    case DWC_GDB_ERROR_BAD_NUM_RECORDIDS :   ///<
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

    case DWC_GDB_ERROR_UNKNOWN :             ///<
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

    case DWC_GDB_ERROR_ALREADY_INITIALIZED : ///< 既に初期化されています。
      GF_ASSERT(0);
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

    case DWC_GDB_ERROR_NOT_INITIALIZED :     ///< 初期化が行われていません。
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETRY;

    case DWC_GDB_ERROR_IN_ASYNC_PROCESS :    ///< 非同期処理中です。
      return WIFIBATTLEMATCH_NET_ERROR_NONE;

    case DWC_GDB_ERROR_GHTTP :               ///< GHTTPのエラーが発生しました。
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

    case DWC_GDB_ERROR_IN_DWC_ERROR :         ///< DWCのエラーが起こっています。エラーの対処を行ってください。
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

    default:
      GF_ASSERT(0);
      return 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レコードテーブルを作成
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_GDB_StartCreateRecord( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  p_wk->is_gdb_start  = TRUE;
  p_wk->seq           = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レコードテーブルを作成
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	*p_result   エラー結果
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_GDB_ProcessCreateRecord( WIFIBATTLEMATCH_NET_WORK *p_wk, DWCGdbError *p_result )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_CREATE_START,
    SEQ_CREATE_WAIT,
    SEQ_CREATE_END,
    SEQ_EXIT,
  };
  DWCGdbState state;

  if( p_wk->is_gdb_start )  
  { 
    switch( p_wk->seq )
    { 
    case SEQ_INIT:
	    *p_result = DWC_GdbInitialize( GAME_ID, p_wk->cp_user_data, WIFIBATTLEMATCH_NET_SSL_TYPE );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return TRUE;
      }
      DEBUG_NET_Printf( "INIT:Init\n" );
      p_wk->seq = SEQ_CREATE_START;
      break;


	  case SEQ_CREATE_START:
      //新規開始のときは、まっさらな状態で作成
	    *p_result = DWC_GdbCreateRecordAsync( WIFIBATTLEMATCH_NET_TABLENAME,
	                                      NULL, 
	                                      0, &p_wk->sake_record_id );
	    if( *p_result != DWC_GDB_ERROR_NONE )
	    { 
        DEBUG_NET_Printf( "INIT:Create Start\n" );
        p_wk->is_gdb_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return TRUE;
	    }
	    p_wk->async_timeout = 0;
	    p_wk->seq = SEQ_CREATE_WAIT;
	    break;
	
	  case SEQ_CREATE_WAIT:
	    { 
	      state = DWC_GdbGetState();
	
	      if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
	      { 
	        GF_ASSERT(0);

          p_wk->is_gdb_start  = FALSE;
          DwcRap_Gdb_Finalize( p_wk );
          return TRUE;
	      }
	
	      if( DWC_GDB_STATE_IN_ASYNC_PROCESS == state )
	      { 
	        DWC_GdbProcess();
	      }
	      else
	      { 
	        if( state != DWC_GDB_STATE_IDLE )
	        { 
            DEBUG_NET_Printf( "INIT:Create Wait\n" );
	          GF_ASSERT(0);

            p_wk->is_gdb_start  = FALSE;
	          DwcRap_Gdb_Finalize( p_wk );
	          return TRUE;
	        }
	        else
	        { 
            DEBUG_NET_Printf( "INIT:Create Wait\n" );
	          p_wk->seq = SEQ_CREATE_END;
	        }
	      }
	    }
	
	    break;
	
	  case SEQ_CREATE_END:
	    { 
	      DWCGdbAsyncResult ret;
	      ret = DWC_GdbGetAsyncResult();
	      if( ret != DWC_GDB_ASYNC_RESULT_SUCCESS )
	      { 
	        GF_ASSERT_MSG(0, "INIT:Create %d\n", ret);
          p_wk->is_gdb_start  = FALSE;
	        DwcRap_Gdb_Finalize( p_wk );
	        return TRUE;
	      }
	    }
	    DEBUG_NET_Printf( "INIT:Create end\n" );
      p_wk->seq     = SEQ_EXIT;
      break;

    case SEQ_EXIT:
      DwcRap_Gdb_Finalize( p_wk );
      p_wk->is_gdb_start  = FALSE;
	    DEBUG_NET_Printf( "ITNI:Back exit\n" );
      DEBUG_NET_Printf( "作成! sake テーブルID %d\n", p_wk->sake_record_id );
	    return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  敵情報
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *	@return TRUEで送信  FALSEで受信
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_StartEnemyData( WIFIBATTLEMATCH_NET_WORK *p_wk, const void *cp_buff )
{ 
  NetID netID;

  //相手にのみ送信
  netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
  netID = netID == 0? 1: 0;
  return GFL_NET_SendDataEx( GFL_NET_HANDLE_GetCurrentHandle(), netID, WIFIBATTLEMATCH_NETCMD_SEND_ENEMYDATA, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE, cp_buff, TRUE, TRUE, TRUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  敵情報受信
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	*p_data                         データ受け取り
 *
 *	@return TRUE受信  FALSE受信中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_WaitEnemyData( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_ENEMYDATA **pp_data )
{ 
  *pp_data  = p_wk->p_enemy_data;
  return p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_GAMEDATA];
}

//----------------------------------------------------------------------------
/**
 *	@brief  初期化開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
extern void WIFIBATTLEMATCH_NET_StartDownloadDigCard( WIFIBATTLEMATCH_NET_WORK *p_wk, int cup_no )
{ 
  p_wk->is_nd_start = TRUE;
  p_wk->seq = 0;
  GFL_STD_MemClear( p_wk->nd_attr2, 10 );
  STD_TSPrintf( p_wk->nd_attr2, "%d", cup_no );
}
//----------------------------------------------------------------------------
/**
 *	@brief  Wifiからダウンロード
 *  @param  p_wk    ワーク
 *  @retval WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RETを参照
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET WIFIBATTLEMATCH_NET_WaitDownloadDigCard( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_ATTR,
    SEQ_FILENUM,
    SEQ_FILELIST,
    SEQ_GET_FILE,
    SEQ_GETTING_FILE,

    SEQ_CANCEL,
    SEQ_DOWNLOAD_COMPLETE,
    SEQ_END,

    SEQ_WAIT_CALLBACK         = 100,
  };

  if( !p_wk->is_nd_start )
  { 
    return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_SUCCESS;
  }

  switch( p_wk->seq )
  { 
  case SEQ_INIT:
    if( DWC_NdInitAsync( NdCallback, GF_DWC_ND_LOGIN, WIFI_ND_LOGIN_PASSWD ) == FALSE )
    {
      OS_TPrintf( "DWC_NdInitAsync: Failed\n" );
      //@todo
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
    }
    DwcRap_Nd_WaitNdCallback( p_wk, SEQ_ATTR );
    break;

  case SEQ_ATTR:
    // ファイル属性の設定
    if( DWC_NdSetAttr(WIFI_FILE_ATTR1, p_wk->nd_attr2, WIFI_FILE_ATTR3) == FALSE )
    {
      OS_TPrintf( "DWC_NdSetAttr: Failed\n." );
      GF_ASSERT(0);//@todo
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
    }
    p_wk->seq = SEQ_FILENUM;
    break;

//-------------------------------------
///	サーバーのファイルチェック
//=====================================
  case SEQ_FILENUM:
    // サーバーにおかれているファイルの数を得る
    if( DWC_NdGetFileListNumAsync( &p_wk->server_filenum ) == FALSE )
    {
      OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
      GF_ASSERT( 0 ); //@todo
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
    }
    DwcRap_Nd_WaitNdCallback( p_wk, SEQ_FILELIST );
    break;

  case SEQ_FILELIST:
    //ファイルがなかった場合
    DEBUG_NET_Printf( "サーバーにレギュレーションがあった数 %d\n", p_wk->server_filenum );
    if( p_wk->server_filenum == 1 )
    { 
      //ファイルが１つだけの場合決定
      OS_TPrintf( "server_filenum %d\n", p_wk->server_filenum );
      if( DWC_NdGetFileListAsync( &p_wk->fileInfo, 0, 1 ) == FALSE)
      {
        OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
        GF_ASSERT(0);
        return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
      }
      DwcRap_Nd_WaitNdCallback( p_wk, SEQ_GET_FILE );
    }
    else
    { 
      DEBUG_NET_Printf( "サーバーにレギュレーションがあった数 %d\n", p_wk->server_filenum );
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
    }
    break;

//-------------------------------------
///	ファイル読み込み開始
//=====================================
  case SEQ_GET_FILE:
    // ファイル読み込み開始
    if(DWC_NdGetFileAsync( &p_wk->fileInfo, (char*)&p_wk->temp_buffer, REGULATION_CARD_DATA_SIZE) == FALSE){
      OS_TPrintf( "DWC_NdGetFileAsync: Failed.\n" );
      GF_ASSERT(0);
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
    }
    s_callback_flag   = FALSE;
    s_callback_result = DWC_ND_ERROR_NONE;
    p_wk->seq = SEQ_GETTING_FILE;
    break;
    
  case SEQ_GETTING_FILE:
    // ファイル読み込み中
    DWC_NdProcess();
    if( s_callback_flag == FALSE )
    {
      // 進行度を表示
      if(DWC_NdGetProgress( &p_wk->recived, &p_wk->contentlen ) == TRUE)
      {
        if(p_wk->percent != (p_wk->recived*100)/p_wk->contentlen)
        {
          p_wk->percent = (p_wk->recived*100)/p_wk->contentlen;
          OS_Printf( "Download %d/100\n", p_wk->percent );
        }
      }
    }
    else if( s_callback_result != DWC_ND_ERROR_NONE)
    {
      GF_ASSERT(0);
      OS_Printf("DWC_NdGetProgressでエラー\n");
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
    }
    else
    {
      // ファイル読み込み終了
      if( !DWC_NdCleanupAsync() ){  //FALSEの場合コールバックが呼ばれない
        OS_Printf("DWC_NdCleanupAsyncに失敗\n");
        return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
      }
      DwcRap_Nd_WaitNdCallback( p_wk, SEQ_CANCEL );
    }
    break;

//-------------------------------------
///	終了・キャンセル処理
//=====================================
  case SEQ_CANCEL:
    if( DWC_NdCancelAsync() == FALSE )
    {
      p_wk->seq = SEQ_DOWNLOAD_COMPLETE;
    }
    else 
    {
      OS_Printf("download cancel\n");
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
    }
    break;

  case SEQ_DOWNLOAD_COMPLETE:
    DEBUG_NET_Printf( "●regulation card data\n" );
    DEBUG_NET_Printf( "cup no %d\n", p_wk->temp_buffer.no );
    DEBUG_NET_Printf( "status %d\n", p_wk->temp_buffer.status );


    p_wk->seq  = SEQ_END;
    break;

  case SEQ_END:
    return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_SUCCESS;

//-------------------------------------
///	コールバック待ち処理  
//　　WIFI_DOWNLOAD_WaitNdCallbackを使ってください
//=====================================
  case SEQ_WAIT_CALLBACK:
    //コールバック処理を待つ
    DWC_NdProcess();
    if( s_callback_flag )
    { 
      s_callback_flag = FALSE;
      if( s_callback_result != DWC_ND_ERROR_NONE)
      { 
        return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
      }
      else
      { 
        p_wk->seq  = p_wk->next_seq;
      }
    }
    break;
  }

  return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_DOWNLOADING;
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信したデジタル選手証を取得
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	*p_recv   取得バッファ
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_GetDownloadDigCard( const WIFIBATTLEMATCH_NET_WORK *cp_wk, REGULATION_CARDDATA *p_recv )
{ 
  Regulation_PrintDebug( &cp_wk->temp_buffer );
  GFL_STD_MemCopy( &cp_wk->temp_buffer, p_recv, sizeof(REGULATION_CARDDATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  コールバック待ち
 *
 *	@param	*p_wk                     ワーク
 *	@param	next_seq                  コールバック成功後へいくシーケンス
 */
//-----------------------------------------------------------------------------
static void DwcRap_Nd_WaitNdCallback( WIFIBATTLEMATCH_NET_WORK *p_wk, int next_seq )
{ 
  s_callback_flag   = FALSE;
  s_callback_result = DWC_ND_ERROR_NONE;
  p_wk->next_seq    = next_seq;
  p_wk->seq         = 100;
}

/*-------------------------------------------------------------------------*
 * Name        : NdCallback
 * Description : ND用コールバック
 * Arguments   : None.
 * Returns     : None.
 *-------------------------------------------------------------------------*/
static void NdCallback(DWCNdCallbackReason reason, DWCNdError error, int servererror)
{
  DEBUG_NET_Printf("NdCallback: Called\n");
  switch(reason) {
  case DWC_ND_CBREASON_GETFILELISTNUM:
    DEBUG_NET_Printf("DWC_ND_CBREASON_GETFILELISTNUM\n");
    break;
  case DWC_ND_CBREASON_GETFILELIST:
    DEBUG_NET_Printf("DWC_ND_CBREASON_GETFILELIST\n");
    break;
  case DWC_ND_CBREASON_GETFILE:
    DEBUG_NET_Printf("DWC_ND_CBREASON_GETFILE\n");
    break;
  case DWC_ND_CBREASON_INITIALIZE:
    DEBUG_NET_Printf("DWC_ND_CBREASON_INITIALIZE\n");
    break;
  }
	
  switch(error) {
  case DWC_ND_ERROR_NONE:
    DEBUG_NET_Printf("DWC_ND_NOERR\n");
    break;
  case DWC_ND_ERROR_ALLOC:
    DEBUG_NET_Printf("DWC_ND_MEMERR\n");
    break;
  case DWC_ND_ERROR_BUSY:
    DEBUG_NET_Printf("DWC_ND_BUSYERR\n");
    break;
  case DWC_ND_ERROR_HTTP:
    DEBUG_NET_Printf("DWC_ND_HTTPERR\n");
    // ファイル数の取得でＨＴＴＰエラーが発生した場合はダウンロードサーバに繋がっていない可能性が高い
    if( reason == DWC_ND_CBREASON_GETFILELISTNUM )
      {
          DEBUG_NET_Printf( "It is not possible to connect download server.\n." );
          ///	OS_Terminate();
      }
    break;
  case DWC_ND_ERROR_BUFFULL:
    DEBUG_NET_Printf("DWC_ND_BUFFULLERR\n");
    break;
  case DWC_ND_ERROR_DLSERVER:
    DEBUG_NET_Printf("DWC_ND_SERVERERR\n");
    break;
  case DWC_ND_ERROR_CANCELED:
    DEBUG_NET_Printf("DWC_ND_CANCELERR\n");
    break;
  }
  OS_Printf("errorcode = %d\n", servererror);
  s_callback_flag   = TRUE;
  s_callback_result = error;

  //NdCleanupCallback();
}
//=============================================================================
/**
 *    GPFサーバからの受信
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  GPFサーバからの受信 スタート
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartRecvGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk, MYSTATUS *p_mystatus, HEAPID heapID )
{ 
  GF_ASSERT( p_wk->p_nhttp == NULL );
  p_wk->seq = 0;
  p_wk->p_nhttp = NHTTP_RAP_Init( heapID, MyStatus_GetProfileID(p_mystatus));
}
//----------------------------------------------------------------------------
/**
 *	@brief  GPFサーバーからの受信 メイン処理
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *
 *	@return TRUEで受信  FALSEで処理中
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_RECV_GPFDATA_RET WIFIBATTLEMATCH_NET_WaitRecvGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk, NHTTPError *p_error )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_START_CONNECT,
    SEQ_WAIT_CONNECT,

    SEQ_END,
  };

  if( p_wk->p_nhttp == NULL )
  { 
    return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_SUCCESS;
  }

  switch( p_wk->seq )
  { 
  case SEQ_INIT:
    if (NHTTP_RAP_ConectionCreate( NHTTPRAP_URL_BATTLE_DOWNLOAD, p_wk->p_nhttp ))
    {
      p_wk->seq  = SEQ_START_CONNECT;
    }
    break;

  case SEQ_START_CONNECT:
    if(NHTTP_RAP_StartConnect(p_wk->p_nhttp))
    {
      p_wk->seq  = SEQ_WAIT_CONNECT;
    }
    break;

  case SEQ_WAIT_CONNECT:
    *p_error  = NHTTP_RAP_Process(p_wk->p_nhttp);
    if(NHTTP_ERROR_NONE == *p_error)
    {
      u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(p_wk->p_nhttp);
      DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA* pDream = (DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA*)&pEvent[sizeof(gs_response)];


      NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
      DEBUG_NET_Printf("ID %x\n",pDream->WifiMatchUpID);
      DEBUG_NET_Printf("FLG %x\n",pDream->GPFEntryFlg);
      DEBUG_NET_Printf("ST %d\n",pDream->WifiMatchUpState);

      DEBUG_NET_Printf("終了\n");

      p_wk->gpf_data  = *pDream;
      p_wk->seq  = SEQ_END;
    }
    else if( NHTTP_ERROR_BUSY != *p_error )
    { 
      return WIFIBATTLEMATCH_RECV_GPFDATA_RET_ERROR;
    }

    break;


  case SEQ_END :
    if(p_wk->p_nhttp)
    {
      NHTTP_RAP_End(p_wk->p_nhttp);
      p_wk->p_nhttp  = NULL;
    }
    return WIFIBATTLEMATCH_RECV_GPFDATA_RET_SUCCESS;
  }

  return WIFIBATTLEMATCH_RECV_GPFDATA_RET_UPDATE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  GPFサーバーからの受信 受信したデータを受け取り
 *
 *	@param	const WIFIBATTLEMATCH_NET_WORK *cp_wk ワーク
 *	@param	*p_recv 受け取りバッファ
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_GetRecvGpfData( const WIFIBATTLEMATCH_NET_WORK *cp_wk, DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA *p_recv )
{ 
  *p_recv = cp_wk->gpf_data;
}

//----------------------------------------------------------------------------
/**
 *	@brief  GPFサーバーへ書き込み 初期化
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk                        ワーク
 *	@param	*p_mystatus                                           自分のステータス
 *	@param	DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA *cp_send    書き込みデータ
 *	@param	heapID                                                ヒープID
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartSendGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk, MYSTATUS *p_mystatus, const DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA *cp_send, HEAPID heapID )
{ 
  GF_ASSERT( p_wk->p_nhttp == NULL );
  p_wk->p_nhttp = NHTTP_RAP_Init( heapID, MyStatus_GetProfileID(p_mystatus));
  p_wk->gdb_write_data  = *cp_send;
  p_wk->seq = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  GPFサーバーへ書き込み メイン処理
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	*p_error                        エラー値
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_SEND_GPFDATA_RET WIFIBATTLEMATCH_NET_WaitSendGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk, NHTTPError *p_error )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_START_CONNECT,
    SEQ_WAIT_CONNECT,

    SEQ_END,
  };

  if( p_wk->p_nhttp == NULL )
  { 
    return TRUE;
  }

  switch( p_wk->seq )
  { 
  case SEQ_INIT:
    if (NHTTP_RAP_ConectionCreate( NHTTPRAP_URL_BATTLE_DOWNLOAD, p_wk->p_nhttp ))
    {
      NHTTP_AddPostDataRaw(NHTTP_RAP_GetHandle(p_wk->p_nhttp), &p_wk->gdb_write_data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA) );
      p_wk->seq  = SEQ_START_CONNECT;
    }
    break;

  case SEQ_START_CONNECT:
    if(NHTTP_RAP_StartConnect(p_wk->p_nhttp))
    {
      p_wk->seq  = SEQ_WAIT_CONNECT;
    }
    break;

  case SEQ_WAIT_CONNECT:
    *p_error  = NHTTP_RAP_Process(p_wk->p_nhttp);
    if(NHTTP_ERROR_NONE == *p_error)
    {
      u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(p_wk->p_nhttp);
      NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
      DEBUG_NET_Printf("GPFサーバーへの書き込み終了\n");

      p_wk->seq  = SEQ_END;
    }
    else if( NHTTP_ERROR_BUSY != *p_error )
    { 
      return WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR;
    }

    break;


  case SEQ_END :
    if(p_wk->p_nhttp)
    {
      NHTTP_RAP_End(p_wk->p_nhttp);
      p_wk->p_nhttp  = NULL;
    }
    return WIFIBATTLEMATCH_SEND_GPFDATA_RET_SUCCESS;
  }

  return WIFIBATTLEMATCH_SEND_GPFDATA_RET_UPDATE;
}
//=============================================================================
/**
 *    不正文字
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  不正文字チェックをスタート
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	STRCODE *cp_str                 チェックするSTRCODE
 *	@param	str_len                         STRCODEの長さ
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartBadWord( WIFIBATTLEMATCH_NET_WORK *p_wk, const STRCODE *cp_str, u32 str_len )
{ 
  BOOL ret;
/*
  # 文字コードはUnicode（リトルエンディアンUTF16）を使用してください。
  それ以外の文字コードを使用している場合は、Unicodeに変換してください。
  # スクリーンネームにUnicode及びIPLフォントにない独自の文字を使用している場合は、スペースに置き換えてください。
  # 終端は"\0\0"（u16で0x0000）である必要があります。
  # 配列内の全ての文字列の合計が501文字まで（各文字列の終端を含む）にする必要があります。
  # 配列内の文字列にNULLを指定することはできません。 
  */
  { 
    int i;
    GF_ASSERT( str_len < WIFIBATTLEMATCH_BADWORD_STRL_MAX );
    for( i = 0; i < str_len; i++ )
    { 
      if( GFL_STR_GetEOMCode() == cp_str[i] )
      {
        p_wk->badword_str[i]  = 0x0000;
      }
      else
      { 
        p_wk->badword_str[i]  = cp_str[i];
      }
    }
    p_wk->p_badword_arry[0] = p_wk->badword_str;
  }
  p_wk->badword_num = 0;


  ret = DWC_CheckProfanityExAsync( (const u16 **)p_wk->p_badword_arry,
                             1,
                             NULL,
                             0,
                             p_wk->badword_result,
                             &p_wk->badword_num,
                             DWC_PROF_REGION_ALL );
  GF_ASSERT( ret );
}
//----------------------------------------------------------------------------
/**
 *	@brief  不正文字チェックメイン
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk    ワーク
 *	@param	*p_is_bad_word                    TRUEならば不正文字  FALSEならば正常文字
 *
 *	@return TRUE処理終了  FALSE処理中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_WaitBadWord( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL *p_is_bad_word )
{ 
  BOOL ret;
  ret = DWC_CheckProfanityProcess() == DWC_PROF_STATE_SUCCESS;

  if( ret == TRUE )
  { 
    *p_is_bad_word  = p_wk->badword_num;
  }
  return ret;
}
