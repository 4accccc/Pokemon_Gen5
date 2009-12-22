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
#define DEBUG_NET_PRINT_ON
#endif //PM_DEBUG

#if defined(DEBUG_NET_PRINT_ON) && defined(DEBUG_ONLY_FOR_toru_nagihashi)
#define DEBUG_NET_Printf(...)  OS_FPrintf(3,__VA_ARGS__)
#else
#define DEBUG_NET_Printf(...)  /*  */
#endif  //DEBUG_NET_PRINT_ON

//-------------------------------------
///	ネット定数
//=====================================
#ifdef PM_DEBUG
#define WIFIBATTLEMATCH_NET_SSL_TYPE DWC_GDB_SSL_TYPE_NONE
#else
#define WIFIBATTLEMATCH_NET_SSL_TYPE DWC_GDB_SSL_TYPE_SERVER_AUTH
#endif

#define WIFIBATTLEMATCH_NET_TABLENAME "PlayerStats_v1" 

#define SAKE_STAT_INITIAL_PROFILE_ID  "INITIAL_PROFILE_ID"
#define SAKE_STAT_NOW_PROFILE_ID      "NOW_PROFILE_ID"
#define SAKE_STAT_LAST_LOGIN_DATETIME "LAST_LOGIN_DATETIME"

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
  MATCHMAKE_KEY_BTL,  //ランダム
  MATCHMAKE_KEY_DEBUG,  //ダミー

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
  BtlRule   btl_rule;
  BtlResult btl_result;
} DWC_SC_WRITE_DATA;

//-------------------------------------
///	友達無指定ピアマッチメイク用追加キーデータ構造体
//=====================================
typedef struct
{
  int ivalue;       // キーに対応する値（int型）
  u8  keyID;        // マッチメイク用キーID
  u8 pad;           // パディング
  char keyStr[3];   // マッチメイク用キー文字列
} MATCHMAKE_KEY_WORK;

//-------------------------------------
///	ネットモジュール
//=====================================
struct _WIFIBATTLEMATCH_NET_WORK
{ 
  const DWCUserData *cp_user_data;
  WIFI_LIST *p_wifilist;

  u32 seq_matchmake;
  char filter[128];
  char where[128];
  char orderby[128];
  MATCHMAKE_KEY_WORK  key_wk[ MATCHMAKE_KEY_MAX ];
  HEAPID heapID;

  u32   seq;
  BOOL  is_gdb_start;
  BOOL  is_gdb_w_start;
  BOOL  is_sc_start;
  BOOL  is_initialize;
  u32   wait_cnt;
  
  DWC_SC_PLAYERDATA player[2];  //自分は０ 相手は１
  BOOL              is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_MAX];
  DWCScResult       result;

  void *p_get_wk;
  DWC_SC_WRITE_DATA report;
  DWCGdbField *p_field_buff;

  WIFIBATTLEMATCH_ENEMYDATA *p_enemy_data[2]; //0が送信バッファ  1が受信バッファ
  BOOL is_recv_enemy_data;

  u32 async_timeout;
  u32 cancel_select_timeout;   //CANCELSELECT_TIMEOUT
  BOOL is_stop_connect;
  BOOL is_auth;


  //以下バックアップ用
  s32 init_profileID;
  s32 now_profileID;
  int backup_recordID;
  BOOL is_backup;
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA  backup;

  //以下デバッグ用データ
  BOOL is_debug;
  WIFIBATTLEMATCH_SC_DEBUG_DATA * p_debug_data;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void MATCHMAKE_KEY_Set( WIFIBATTLEMATCH_NET_WORK *p_wk, MATCHMAKE_KEY key, int value );
static int WIFIBATTLEMATCH_Eval_Callback( int index, void* p_param_adrs );

//-------------------------------------
///	SGFL_NETコールバック
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
static DWCScResult DwcRap_Sc_CreateReport( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const DWC_SC_WRITE_DATA *cp_data, BOOL is_auth );
static DWCScResult DwcRap_Sc_CreateReportDebug( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const WIFIBATTLEMATCH_SC_DEBUG_DATA * cp_data, BOOL is_auth );
static DWCScResult DwcRap_Sc_CreateReportCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my );

//-------------------------------------
///	GDB関係
//=====================================
static void DwcRap_Gdb_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param);
static void DwcRap_Gdb_GetRecordsCallbackAndNew(int record_num, DWCGdbField** records, void* user_param);
static void DwcRap_Gdb_GetRecordsProfileIDCallback(int record_num, DWCGdbField** records, void* user_param);
static void DwcRap_Gdb_Finalize( WIFIBATTLEMATCH_NET_WORK *p_wk );
#ifdef PM_DEBUG
static void print_field(DWCGdbField* field); // レコードをデバッグ出力する。
#endif

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
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_DOUBLE_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_DOUBLE_WIN_COUNTER",
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_ROTATE_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_ROTATE_WIN_COUNTER",
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_SHOOTER_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_SHOOTER_WIN_COUNTER",
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_SINGLE_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_SINGLE_WIN_COUNTER"  ,
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_TRIPLE_LOSE_COUNTER",
  DWC_GDB_FIELD_TYPE_SHORT,//"NUM_TRIPLE_WIN_COUNTER",
  DWC_GDB_FIELD_TYPE_INT,//"INITIAL_PROFILE_ID",
  DWC_GDB_FIELD_TYPE_INT,//"NOW_PROFILE_ID",
};

static const int sc_default_value[]  =
{ 
  1500,//"ARENA_ELO_RATING_1V1_DOUBLE",
  1500,//"ARENA_ELO_RATING_1V1_ROTATE",
  1500,//"ARENA_ELO_RATING_1V1_SHOOTER",
  1500,//"ARENA_ELO_RATING_1V1_SINGLE",
  1500,//"ARENA_ELO_RATING_1V1_TRIPLE",
  0,//"CHEATS_COUNTER",
  0,//"COMPLETE_MATCHES_COUNTER",
  0,//"DISCONNECTS_COUNTER",
  0,//"NUM_DOUBLE_LOSE_COUNTER",
  0,//"NUM_DOUBLE_WIN_COUNTER",
  0,//"NUM_ROTATE_LOSE_COUNTER",
  0,//"NUM_ROTATE_WIN_COUNTER",
  0,//"NUM_SHOOTER_LOSE_COUNTER",
  0,//"NUM_SHOOTER_WIN_COUNTER",
  0,//"NUM_SINGLE_LOSE_COUNTER",
  0,//"NUM_SINGLE_WIN_COUNTER"  ,
  0,//"NUM_TRIPLE_LOSE_COUNTER",
  0,//"NUM_TRIPLE_WIN_COUNTER",
  0,//"INITIAL_PROFILE_ID",
  0,//"NOW_PROFILE_ID",
};

//-------------------------------------
///	マッチメイクキー文字列
//=====================================
static const char *sc_matchmake_key_str[ MATCHMAKE_KEY_MAX ] =
{ 
  "bt",
  "db",
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
static const NetRecvFuncTable sc_net_recv_tbl[WIFIBATTLEMATCH_NETCMD_MAX] = 
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


  p_wk->p_field_buff  = GFL_HEAP_AllocClearMemory( heapID, ATLAS_GetFieldNameNum() * sizeof(DWCGdbField) );

  { 
    p_wk->p_enemy_data[0]  = GFL_HEAP_AllocMemory( heapID, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
    GFL_STD_MemClear( p_wk->p_enemy_data[0], WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
    p_wk->p_enemy_data[1]  = GFL_HEAP_AllocMemory( heapID, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
    GFL_STD_MemClear( p_wk->p_enemy_data[1], WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );

  }
  GFL_NET_DebugPrintOn();

  if(!GFL_NET_IsInit())
  {
    GF_ASSERT(0);
  }

  GFL_NET_SetWifiBothNet(FALSE);
  GFL_NET_AddCommandTable( GFL_NET_CMD_WIFIMATCH, sc_net_recv_tbl, WIFIBATTLEMATCH_NETCMD_MAX, p_wk );


  return p_wk;
}

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

  GFL_HEAP_FreeMemory( p_wk->p_field_buff );
  GFL_HEAP_FreeMemory( p_wk->p_enemy_data[1] );
  GFL_HEAP_FreeMemory( p_wk->p_enemy_data[0] );

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
 *	@brief  初期化開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartInitialize( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  p_wk->is_initialize = TRUE;
  p_wk->seq = 0;
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
  return cp_wk->is_initialize && cp_wk->seq > 5;
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
    SEQ_INIT,
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
    SEQ_CHECK,
    SEQ_SEARCH_START,
    SEQ_SEARCH_WAIT,
    SEQ_SEARCH_END,
    SEQ_CREATE_START,
    SEQ_CREATE_WAIT,
    SEQ_CREATE_END,
    SEQ_BACKUP_START,
    SEQ_BACKUP_WAIT,
    SEQ_BACKUP_END,
    SEQ_EXIT,
  };
  DWCGdbState state;

  if( p_wk->is_initialize )
  { 
	  *p_result = DWC_GDB_ERROR_NONE;
	
	  switch( p_wk->seq )
	  { 
	  case SEQ_INIT:
	    *p_result = DWC_GdbInitialize( GAME_ID, p_wk->cp_user_data, WIFIBATTLEMATCH_NET_SSL_TYPE );
	    if( *p_result != DWC_GDB_ERROR_NONE )
	    { 
        p_wk->is_initialize = FALSE;
	      DwcRap_Gdb_Finalize( p_wk );
	      return FALSE;
	    }
	    DEBUG_NET_Printf( "INIT:Init\n" );
	    p_wk->seq = SEQ_START;
	    break;
	
	  case SEQ_START:
	    *p_result = DWC_GdbGetMyRecordsAsync( WIFIBATTLEMATCH_NET_TABLENAME, ATLAS_GetFieldName(), ATLAS_GetFieldNameNum(), DwcRap_Gdb_GetRecordsProfileIDCallback, p_wk );
	    if( *p_result != DWC_GDB_ERROR_NONE )
	    { 
        p_wk->is_initialize = FALSE;
	      DwcRap_Gdb_Finalize( p_wk );
	      return FALSE;
	    }
	    DEBUG_NET_Printf( "INIT:GetMy init\n" );
	    p_wk->async_timeout = 0;
	    p_wk->seq= SEQ_WAIT;
	    break;
	
	  case SEQ_WAIT:
	    { 
	      state = DWC_GdbGetState();
	
	      if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
	      { 
	        GF_ASSERT(0);
	      }
	
	      if( DWC_GDB_STATE_IN_ASYNC_PROCESS == state )
	      { 
	        DWC_GdbProcess();
	      }
	      else
	      { 
	        if( state != DWC_GDB_STATE_IDLE )
	        { 
	          GF_ASSERT(0);
            p_wk->is_initialize = FALSE;
	          DwcRap_Gdb_Finalize( p_wk );
	          return FALSE;
	        }
	        else
	        { 
	          DEBUG_NET_Printf( "INIT:GetMy wait\n" );
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
	        GF_ASSERT_MSG(0, "INIT:AsyncResult %d\n", ret);
          p_wk->is_initialize = FALSE;
	        DwcRap_Gdb_Finalize( p_wk );
	        return FALSE;
	      }
	    }
	    DEBUG_NET_Printf( "INIT:GetMy end\n" );
	    p_wk->seq = SEQ_CHECK;
	    break;
	
	  case SEQ_CHECK:
	    if( p_wk->init_profileID == 0 )
	    { 
	      //始めてor新しいDSでランダムマッチを開始した
	      //
	      //自分のプロファイルIDでデータを検索し、
	      //あったら、以前のデータを移行。
	      //なかったら、通常の初期化をする
	      p_wk->seq = SEQ_SEARCH_START;
	    }
	    else
	    { 
	      p_wk->seq = SEQ_EXIT;
	    }
	    break;
	
	  case SEQ_SEARCH_START:
	    {
	      DWCGdbSearchCond search_cond  =
	      { 
	        NULL, //WHERE     あとで入れる
	        NULL, //ORDERBY   あとで入れる
	        0,    //結果の何番目から取得するか
	        10,
	        NULL,
	        0,
	        NULL,
	        0,
	        TRUE
	      };
	
	      SYSTEMDATA  *p_systemdata  = SaveData_GetSystemData( p_save );
	
	      const s32 init_profileID  = SYSTEMDATA_GetDpwInfo( p_systemdata );
	      const s32 now_profileID   = WifiList_GetMyGSID( p_wk->p_wifilist );
        DEBUG_NET_Printf( "INITIAL_PROFILE_ID = %d And NOW_PROFILE_ID = %d", init_profileID, now_profileID );
	
	      STD_TSPrintf( p_wk->where, "INITIAL_PROFILE_ID = %d", init_profileID );
	      STD_TSPrintf( p_wk->orderby, "LAST_LOGIN_DATETIME ASC" );
	      search_cond.filter  = p_wk->where;
	      search_cond.sort    = p_wk->orderby;
        p_wk->is_backup = FALSE;
	      *p_result = DWC_GdbSearchRecordsAsync( WIFIBATTLEMATCH_NET_TABLENAME,
	                                      ATLAS_GetFieldName(), ATLAS_GetFieldNameNum(),
	                                      &search_cond,
	                                      DwcRap_Gdb_GetRecordsCallbackAndNew,
	                                      p_wk );
	    }
	    if( *p_result != DWC_GDB_ERROR_NONE )
	    { 
        p_wk->is_initialize = FALSE;
	      DwcRap_Gdb_Finalize( p_wk );
	      return FALSE;
	    }
	    DEBUG_NET_Printf( "INIT:GetSearch start\n" );
	    p_wk->async_timeout = 0;
	    p_wk->seq= SEQ_SEARCH_WAIT;
	    break;
	
	  case SEQ_SEARCH_WAIT:
	    { 
	      state = DWC_GdbGetState();
	
	      if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
	      { 
	        GF_ASSERT(0);
	      }
	
	      if( DWC_GDB_STATE_IN_ASYNC_PROCESS == state )
	      { 
	        DWC_GdbProcess();
	      }
	      else
	      { 
	        if( state != DWC_GDB_STATE_IDLE )
	        { 
	          GF_ASSERT(0);
            p_wk->is_initialize = FALSE;
	          DwcRap_Gdb_Finalize( p_wk );
	          return FALSE;
	        }
	        else
	        { 
	          DEBUG_NET_Printf( "INIT:GetSearch wait\n" );
	          p_wk->seq = SEQ_SEARCH_END;
	        }
	      }
	    }
	    break;
	
	  case SEQ_SEARCH_END:
	    { 
	      DWCGdbAsyncResult ret;
	      ret = DWC_GdbGetAsyncResult();
	      if( ret != DWC_GDB_ASYNC_RESULT_SUCCESS )
	      { 
	        GF_ASSERT_MSG(0, "INIT:GetSearch AsyncResult %d\n", ret);
          p_wk->is_initialize = FALSE;
	        DwcRap_Gdb_Finalize( p_wk );
	        return FALSE;
	      }
	    }
	    DEBUG_NET_Printf( "INIT:GetSearch end\n" );
	    p_wk->seq = SEQ_CREATE_START;
	    break;
	
	  case SEQ_CREATE_START:

      //DSが新しくなった場合、新規のテーブルに昔のDSで使ってたデータを移行
      if( p_wk->is_backup )
      { 
        int i;
	      for( i = 0; i < ATLAS_GetFieldNameNum(); i++ )
	      { 
	        p_wk->p_field_buff[i].name  = (char*)ATLAS_GetFieldName()[i];
	        p_wk->p_field_buff[i].type  = sc_field_type[i];
	        p_wk->p_field_buff[i].value.int_s32 = p_wk->backup.arry[i];
	      }

        DEBUG_NET_Printf( "DS本体を交換しました\n" );

	    }
      else
      { 
        //新規の場合、デフォルト値をいれる
        int i;
        SYSTEMDATA  *p_systemdata  = SaveData_GetSystemData( p_save );
	      const s32 init_profileID  = SYSTEMDATA_GetDpwInfo( p_systemdata );
	      const s32 now_profileID   = WifiList_GetMyGSID( p_wk->p_wifilist );
	      for( i = 0; i < ATLAS_GetFieldNameNum(); i++ )
	      { 
	        p_wk->p_field_buff[i].name          = (char*)ATLAS_GetFieldName()[i];
	        p_wk->p_field_buff[i].type          = sc_field_type[i];

          if( !GFL_STD_StrCmp( p_wk->p_field_buff[i].name, SAKE_STAT_INITIAL_PROFILE_ID ) )
          { 
            p_wk->p_field_buff[i].value.int_s32 = init_profileID;
          }
          else if( !GFL_STD_StrCmp( p_wk->p_field_buff[i].name, SAKE_STAT_NOW_PROFILE_ID ) )
          { 
            p_wk->p_field_buff[i].value.int_s32 = now_profileID;
          }
          else
          { 
            p_wk->p_field_buff[i].value.int_s32 = sc_default_value[i];
          }
        }
        DEBUG_NET_Printf( "新規に開始しました\n" );
      }

      //新規開始のときは、まっさらな状態で作成
	    *p_result = DWC_GdbCreateRecordAsync( WIFIBATTLEMATCH_NET_TABLENAME,
	                                      p_wk->p_field_buff, 
	                                      ATLAS_GetFieldNameNum(), &p_wk->backup_recordID );
	    if( *p_result != DWC_GDB_ERROR_NONE )
	    { 
        DEBUG_NET_Printf( "INIT:Create Start\n" );
        p_wk->is_initialize = FALSE;
	      DwcRap_Gdb_Finalize( p_wk );
	      return FALSE;
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
            p_wk->is_initialize = FALSE;
	          DwcRap_Gdb_Finalize( p_wk );
	          return FALSE;
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
          p_wk->is_initialize = FALSE;
	        DwcRap_Gdb_Finalize( p_wk );
	        return FALSE;
	      }
	    }
	    DEBUG_NET_Printf( "INIT:Create end\n" );
	    //p_wk->seq   = SEQ_BACKUP_START;
      p_wk->seq     = SEQ_EXIT;
	    break;
#if 0
	  case SEQ_BACKUP_START:
	    { 
        int i;
	      for( i = 0; i < ATLAS_GetFieldNameNum(); i++ )
	      { 
	        p_wk->p_field_buff[i].name  = (char*)ATLAS_GetFieldName()[i];
	        p_wk->p_field_buff[i].type  = sc_field_type[i];
	        p_wk->p_field_buff[i].value.int_s32 = p_wk->backup.arry[i];
	      }
	    }
	
	    *p_result = DWC_GdbUpdateRecordAsync( WIFIBATTLEMATCH_NET_TABLENAME, p_wk->backup_recordID, p_wk->p_field_buff, ATLAS_GetFieldNameNum() );
	    if( *p_result != DWC_GDB_ERROR_NONE )
	    { 
        DEBUG_NET_Printf( "INIT:Backup end\n" );
        p_wk->is_initialize = FALSE;
	      DwcRap_Gdb_Finalize( p_wk );
	      return FALSE;
	    }
	    p_wk->seq   = SEQ_BACKUP_WAIT;
	    break;
	
	  case SEQ_BACKUP_WAIT:
	     { 
	      state = DWC_GdbGetState();
	
	      if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
	      { 
	        GF_ASSERT(0);
	      }
	
	      if( DWC_GDB_STATE_IN_ASYNC_PROCESS == state )
	      { 
	        DWC_GdbProcess();
	      }
	      else
	      { 
	        if( state != DWC_GDB_STATE_IDLE )
	        { 
	          GF_ASSERT(0);
            p_wk->is_initialize = FALSE;
	          DwcRap_Gdb_Finalize( p_wk );
	          return FALSE;
	        }
	        else
	        { 
            DEBUG_NET_Printf( "INIT:Backup wait\n" );
	          p_wk->seq   = SEQ_BACKUP_END;
	        }
	      }
	    }
	    break;
	
	  case SEQ_BACKUP_END:
	    { 
	      DWCGdbAsyncResult ret;
	      ret = DWC_GdbGetAsyncResult();
	      if( ret != DWC_GDB_ASYNC_RESULT_SUCCESS )
	      { 
	        GF_ASSERT_MSG(0, "INIT:Backup AsyncResult %d\n", ret);
          p_wk->is_initialize = FALSE;
	        DwcRap_Gdb_Finalize( p_wk );
	        return FALSE;
	      }
	    }
	    DEBUG_NET_Printf( "INIT:Backup end\n" );
	    p_wk->seq   = SEQ_EXIT;
	    break;
#endif
	  case SEQ_EXIT:
      p_wk->is_initialize = FALSE;
	    DwcRap_Gdb_Finalize( p_wk );
	    DEBUG_NET_Printf( "ITNI:Back exit\n" );
	    return TRUE;
	  }
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
void WIFIBATTLEMATCH_NET_StartMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk, BtlRule btl_rule )
{ 
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_DEBUG, MATCHINGKEY );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_BTL, btl_rule );
  STD_TSPrintf( p_wk->filter, "bt=%d And db=%d", btl_rule, MATCHINGKEY );
  p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_START;
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
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_DEBUG, 0xF );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_BTL, 0 );
  STD_TSPrintf( p_wk->filter, "bt=%d And db=%d", 0, 0xF );
  p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_START;
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
    if( GFL_NET_DWC_StartMatchFilter( p_wk->filter, 2 ,WIFIBATTLEMATCH_Eval_Callback, p_wk ) != 0 )
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
 *	@brief  エラー処理
 *
 *	@param	const WIFIBATTLEMATCH_NET_WORK *cp_wk   ワーク
 *
 *	@return TRUEでエラー  FALSEで通常
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_IsError( const WIFIBATTLEMATCH_NET_WORK *cp_wk )
{ 
  //@todo
  return FALSE;
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
  GFL_STD_MemCopy( sc_matchmake_key_str[ key ], p_key_wk->keyStr, 2 );
  p_wk->key_wk[ key ].keyID  = DWC_AddMatchKeyInt( p_key_wk->keyID,
      p_key_wk->keyStr, &p_key_wk->ivalue );
  if( p_key_wk->keyID == 0 )
  {
      NAGI_Printf("AddMatchKey failed key=%d.value=%d\n",key, value);
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
static int WIFIBATTLEMATCH_Eval_Callback( int index, void* p_param_adrs )
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
 *	@param  BtlRule rule              ルール
 *	@param  BtlResult result          対戦結果
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_SC_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, BtlRule rule, BtlResult result )
{ 
  if( GFL_NET_IsParentMachine() )
  { 
    NAGI_Printf( "！親機 ルール%d 結果%d\n", rule, result );
  }
  else
  { 
    NAGI_Printf( "！子機 ルール%d 結果%d\n", rule, result );
  }

  p_wk->is_sc_start = TRUE;
  p_wk->seq = 0;
  p_wk->report.btl_rule  = rule;
  p_wk->report.btl_result  = result;
  p_wk->is_debug  = FALSE;
  p_wk->is_auth   = GFL_NET_IsParentMachine();
}
//----------------------------------------------------------------------------
/**
 *	@brief  SCの開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param  BtlRule rule              ルール
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
        ret = DwcRap_Sc_CreateReport( &p_wk->player[0], &p_wk->player[1], &p_wk->report, p_wk->is_auth );
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
static DWCScResult DwcRap_Sc_CreateReport( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const DWC_SC_WRITE_DATA *cp_data, BOOL is_auth )
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
  ret = DwcRap_Sc_CreateReportCore( p_my, cp_data, TRUE );
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
  ret = DwcRap_Sc_CreateReportCore( p_my, cp_data, FALSE );
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
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SINGLE_WIN_COUNTER, cp_data->my_single_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
      return ret;
  }
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SINGLE_LOSE_COUNTER, cp_data->my_single_lose );
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
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_DOUBLE_WIN_COUNTER, cp_data->my_double_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_DOUBLE_LOSE_COUNTER, cp_data->my_double_lose );
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
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_TRIPLE_WIN_COUNTER, cp_data->my_triple_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_TRIPLE_LOSE_COUNTER, cp_data->my_triple_lose );
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
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_ROTATE_WIN_COUNTER, cp_data->my_rot_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_ROTATE_LOSE_COUNTER, cp_data->my_rot_lose );
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
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SHOOTER_WIN_COUNTER, cp_data->my_shooter_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddShortValue( p_my->mReport, KEY_NUM_SHOOTER_LOSE_COUNTER, cp_data->my_shooter_lose );
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
static DWCScResult DwcRap_Sc_CreateReportCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my )
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

  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    GF_ASSERT( 0 );
    return NULL;//自分のは受け取らない
  }
  return (u8 *)p_wk->p_enemy_data[1];
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
  p_wk->seq       = 0;
  p_wk->p_get_wk  = p_wk_adrs;
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
      *p_result = DWC_GdbGetMyRecordsAsync( WIFIBATTLEMATCH_NET_TABLENAME, ATLAS_GetFieldName(), ATLAS_GetFieldNameNum(), DwcRap_Gdb_GetRecordsCallback, p_wk->p_get_wk );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "GDB:Get start\n" );
      p_wk->seq= WIFIBATTLEMATCH_GDB_SEQ_GET_WAIT;
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_GET_WAIT:
      { 
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
 *	@brief  レコード取得コールバック  + 新規orバックアップフラグ
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_GetRecordsCallbackAndNew(int record_num, DWCGdbField** records, void* user_param)
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = user_param;
  if( record_num > 0 )
  { 
    p_wk->is_backup = TRUE;
  }
  DwcRap_Gdb_GetRecordsCallback(record_num, records, &p_wk->backup);
}

//----------------------------------------------------------------------------
/**
 *	@brief  レコード取得コールバック
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param)
{
    int i,j;
    WIFIBATTLEMATCH_GDB_RND_SCORE_DATA *p_data = user_param;
    GFL_STD_MemClear( p_data, sizeof(WIFIBATTLEMATCH_GDB_RND_SCORE_DATA) );

    for (i = 0; i < record_num; i++)
    {
      DEBUG_NET_Printf("!!!=====gdb_Print:======\n");
      for (j = 0; j < ATLAS_GetFieldNameNum(); j++)   // user_param -> field_num
      {
        DWCGdbField* field  = &records[i][j];

        if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_SINGLE_WIN_COUNTER ) ) )
        { 
          p_data->single_win  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_SINGLE_LOSE_COUNTER) ) )
        { 
          p_data->single_lose  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_DOUBLE_WIN_COUNTER ) ) )
        { 
          p_data->double_win  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_DOUBLE_LOSE_COUNTER) ) )
        { 
          p_data->double_lose  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_ROTATE_WIN_COUNTER ) ) )
        { 
          p_data->rot_win  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_ROTATE_LOSE_COUNTER) ) )
        { 
          p_data->rot_lose  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_TRIPLE_WIN_COUNTER ) ) )
        { 
          p_data->triple_win  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_TRIPLE_LOSE_COUNTER) ) )
        { 
          p_data->triple_lose  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_SHOOTER_WIN_COUNTER ) ) )
        { 
          p_data->shooter_win  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(NUM_SHOOTER_LOSE_COUNTER) ) )
        { 
          p_data->shooter_lose  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_SINGLE ) ) )
        { 
          p_data->single_rate  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_DOUBLE) ) )
        { 
          p_data->double_rate  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_ROTATE) ) )
        { 
          p_data->rot_rate  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_TRIPLE) ) )
        { 
          p_data->triple_rate  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(ARENA_ELO_RATING_1V1_SHOOTER) ) )
        { 
          p_data->shooter_rate  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(COMPLETE_MATCHES_COUNTER) ) )
        { 
          p_data->disconnect  = field->value.int_s16;
        }
        else if( !GFL_STD_StrCmp( field->name, ATLAS_GET_STAT_NAME(CHEATS_COUNTER) ) )
        { 
          p_data->cheat  = field->value.int_s16;
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
 *	@brief  レコード取得コールバック  初期化チェック版
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_GetRecordsProfileIDCallback(int record_num, DWCGdbField** records, void* user_param)
{ 
  int i,j;
  WIFIBATTLEMATCH_NET_WORK *p_wk  = user_param;
  p_wk->init_profileID  = 0;
  p_wk->now_profileID = 0;


  for (i = 0; i < record_num; i++)
  {
    DEBUG_NET_Printf( "自分のレコードを発見！\n" );
    for (j = 0; j < ATLAS_GetFieldNameNum(); j++)   // user_param -> field_num
    {
      DWCGdbField* field  = &records[i][j];

      if( !GFL_STD_StrCmp( field->name, SAKE_STAT_INITIAL_PROFILE_ID ) )
      { 
        p_wk->init_profileID  = field->value.int_s32;
      }
      else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_NOW_PROFILE_ID ) )
      { 
        p_wk->now_profileID  = field->value.int_s32;
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  GDBへデータ書き込み
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	type                            データタイプ
 *	@param	void *cp_wk_adrs                書き込むデータ
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_GDB_StartWrite( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_GDB_GETTYPE type, const void *cp_wk_adrs )
{ 
  p_wk->is_gdb_w_start  = TRUE;
  p_wk->seq             = 0;

  { 


    int i;
    const WIFIBATTLEMATCH_GDB_RND_SCORE_DATA *cp_data  = cp_wk_adrs;

    for( i = 0; i < ATLAS_GetFieldNameNum(); i++ )
    { 
      p_wk->p_field_buff[i].name  = (char*)ATLAS_GetFieldName()[i];
      p_wk->p_field_buff[i].type  = sc_field_type[i];
      p_wk->p_field_buff[i].value.int_s32 = cp_data->arry[i];
    }

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
        return FALSE;
      }
      DEBUG_NET_Printf( "GDB:Init\n" );
      p_wk->seq = SEQ_START;
      break;

    case SEQ_START:
      *p_result = DWC_GdbUpdateRecordAsync( WIFIBATTLEMATCH_NET_TABLENAME, 0, p_wk->p_field_buff, ATLAS_GetFieldNameNum() );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_w_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return FALSE;
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
          return FALSE;
        }
      }
      DEBUG_NET_Printf( "GDB:Get end\n" );
      p_wk->seq = SEQ_EXIT;
      break;

    case SEQ_EXIT:
      DwcRap_Gdb_Finalize( p_wk );
      DEBUG_NET_Printf( "GDB:Get exit\n" );
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
  *pp_data  = p_wk->p_enemy_data[1];
  return p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_GAMEDATA];
}
