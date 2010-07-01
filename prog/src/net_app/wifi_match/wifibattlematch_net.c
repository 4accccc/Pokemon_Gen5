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
#include "gamesystem/game_data.h"

//  ネットワーク
#include "net/network_define.h"
#include "net/dwc_rap.h"
#include "net/dwc_rapcommon.h"
#include "system/net_err.h"
#include "net/dwc_error.h"
#include "net/nhttp_rap.h"
#include "net/nhttp_rap_evilcheck.h"
#include "net/dwc_tool.h"

#include "msg/msg_wifi_system.h"

//  セーブデータ
#include "savedata/system_data.h"
#include "savedata/wifilist.h"

//  アトラス情報
#include "atlas_syachi2ds_v1.h"

//  外部公開
#include "wifibattlematch_net.h"

//  デバッグ用にタイミングを知らせるためSEを鳴らすので
#ifdef PM_DEBUG
#include "sound/pm_sndsys.h"
#include "wifibattlematch_snd.h"
#endif

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define DEBUG_DEBUG_NET_Printf_ON //担当者のみのプリント表示をON
#define DEBUGWIN_OWNER_NET_ONLY   //担当者は自分としかつながらなくなる
#define DEBUG_SC_RELEASE_CHECK    //SCの開放チェック
#define DEBUG_GDB_RELEASE_CHECK   //GDBの開放チェック
//#define DEBUG_EVAL_FREE           //評価値計算で必ず繋がるようにする
#endif //PM_DEBUG

//担当者は自分しかつながらなくなる
#ifdef DEBUGWIN_OWNER_NET_ONLY
#if defined(DEBUG_ONLY_FOR_shimoyamada)
#undef  MATCHINGKEY
#define MATCHINGKEY 0xFF
#elif defined(DEBUG_ONLY_FOR_toru_nagihashi)
#undef  MATCHINGKEY
#define MATCHINGKEY 0xFE
#endif  //defined
#endif // DEBUGWIN_OWNER_NET_ONLY

//担当者のみのプリント表示
#ifdef DEBUG_DEBUG_NET_Printf_ON

#if defined(DEBUG_ONLY_FOR_toru_nagihashi)
#define DEBUG_NET_Printf(...)  OS_TFPrintf(1,__VA_ARGS__)
#elif defined(DEBUG_ONLY_FOR_shimoyamada)
#define DEBUG_NET_Printf(...)  OS_TPrintf(__VA_ARGS__)
#else //defined(DEBUG_ONLY_FOR_～
#define DEBUG_NET_Printf(...)  /*  */
#endif  //defined(DEBUG_ONLY_FOR_～

#else //DEBUG_DEBUG_NET_Printf_ON

#ifdef WIFIMATCH_RATE_AUTO
#undef DEBUGPRINT_ON
#define DEBUG_NET_Printf(...)  OS_TFPrintf(1,__VA_ARGS__)
#else
#define DEBUG_NET_Printf(...)  /*  */
#endif
#endif  //DEBUG_DEBUG_NET_Printf_ON

//SCの開放チェックチェック
#ifdef DEBUG_SC_RELEASE_CHECK
static int sc_release_cnt   = 0;
#define DEBUG_SC_RELEASE_Increment {DEBUG_NET_Printf( "sc_release cnt=%d\n", ++sc_release_cnt );}  
#define DEBUG_SC_RELEASE_Decrement {DEBUG_NET_Printf( "sc_release cnt=%d\n", --sc_release_cnt );}  
#else //DEBUG_SC_RELEASE_CHECK
#define DEBUG_SC_RELEASE_Increment /*  */
#define DEBUG_SC_RELEASE_Decrement /*  */
#endif //DEBUG_SC_RELEASE_CHECK

//GDBの開放チェック
#ifdef DEBUG_GDB_RELEASE_CHECK
static int gdb_release_cnt   = 0;
#define DEBUG_GDB_RELEASE_Increment {DEBUG_NET_Printf( "gdb_release cnt=%d\n", ++gdb_release_cnt );}  
#define DEBUG_GDB_RELEASE_Decrement {DEBUG_NET_Printf( "gdb_release cnt=%d\n", --gdb_release_cnt );}  
#else //DEBUG_GDB_RELEASE_CHECK
#define DEBUG_GDB_RELEASE_Increment /*  */
#define DEBUG_GDB_RELEASE_Decrement /*  */
#endif //DEBUG_GDB_RELEASE_CHECK



//#define SC_DIVIDE_SESSION
//-------------------------------------
///	マクロスイッチ
//=====================================

//-------------------------------------
///	ネット定数
//=====================================
#define WIFIBATTLEMATCH_NET_SSL_TYPE DWC_SSL_TYPE_SERVER_AUTH

#define WIFIBATTLEMATCH_NET_TABLENAME "PlayerStats_v1" 

//以下、ATLAS経由ではなく、SAKE直接アクセスの場合
#define SAKE_STAT_RECORDID              "recordid"
#define SAKE_STAT_INITIAL_PROFILE_ID    "INITIAL_PROFILE_ID"
#define SAKE_STAT_NOW_PROFILE_ID        "NOW_PROFILE_ID"
#define SAKE_STAT_LAST_LOGIN_DATETIME   "LAST_LOGIN_DATETIME"
#define SAKE_STAT_WIFICUP_POKEMON_PARTY "WIFICUP_POKEMON_PARTY"
#define SAKE_STAT_MYSTATUS              "MYSTATUS"
#define SAKE_STAT_RECORD_DATA_01        "RECORD_DATA_01"
#define SAKE_STAT_RECORD_DATA_02        "RECORD_DATA_02"
#define SAKE_STAT_RECORD_DATA_03        "RECORD_DATA_03"
#define SAKE_STAT_RECORD_DATA_04        "RECORD_DATA_04"
#define SAKE_STAT_RECORD_DATA_05        "RECORD_DATA_05"
#define SAKE_STAT_RECORD_DATA_06        "RECORD_DATA_06"
#define SAKE_STAT_RECORD_SAVE_IDX       "RECORD_SAVE_IDX"

static const char* scp_stat_record_data_tbl[] =
{ 
  SAKE_STAT_RECORD_DATA_01,
  SAKE_STAT_RECORD_DATA_02,
  SAKE_STAT_RECORD_DATA_03,
  SAKE_STAT_RECORD_DATA_04,
  SAKE_STAT_RECORD_DATA_05,
  SAKE_STAT_RECORD_DATA_06,
};

//-------------------------------------
///	SCのシーケンス
//=====================================
typedef enum 
{
  WIFIBATTLEMATCH_SC_SEQ_SEND_TIMING_INIT,
  WIFIBATTLEMATCH_SC_SEQ_SEND_TIMING_WAIT,
  WIFIBATTLEMATCH_SC_SEQ_SEND_INIT,
  WIFIBATTLEMATCH_SC_SEQ_SEND_WAIT,
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
  WIFIBATTLEMATCH_SC_SEQ_END_TIMING_START,
  WIFIBATTLEMATCH_SC_SEQ_END_TIMING_WAIT,
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

  WIFIBATTLEMATCH_GDB_SEQ_CANCEL_START,
  WIFIBATTLEMATCH_GDB_SEQ_CANCEL_WAIT,
  WIFIBATTLEMATCH_GDB_SEQ_CANCEL_END,
} WIFIBATTLEMATCH_GDB_SEQ;

//-------------------------------------
///	タイミング
//=====================================
#define WIFIBATTLEMATCH_SC_SEND_PLAYERDATA_TIMING     (1)
#define WIFIBATTLEMATCH_SC_RETURN_PLAYERDATA_TIMING   (2)
#define WIFIBATTLEMATCH_SC_REPORT_TIMING              (3)
#define WIFIBATTLEMATCH_SC_DIRTY_TIMING               (4)
#define WIFIBATTLEMATCH_SC_END_TIMING                 (5)
#define WIFIBATTLEMATCH_NET_TIMINGSYNC_CONNECT        (15)

//-------------------------------------
///	その他
//=====================================
#define TIMEOUT_MS            100       // HTTP通信のタイムアウト時間
#define PLAYER_NUM            2          // プレイヤー数
#define TEAM_NUM              0          // チーム数
#define CANCELSELECT_TIMEOUT (60*60)     //キャンセルセレクトタイムアウト
#define ASYNC_TIMEOUT (120*60)     //非同期用タイムアウト
#define DISCONNECTS_WAIT_SYNC      (30)  //切断同期用

#define RECV_BUFFER_SIZE  (0x1000)

#define WBM_GDB_FIELD_TABLE_MAX (38)

//-------------------------------------
///	マッチメイク
//=====================================
//マッチメイクキー
typedef enum
{
  MATCHMAKE_KEY_BTL_MODE,
  MATCHMAKE_KEY_BTL_RULE,
  MATCHMAKE_KEY_DEBUG,
  MATCHMAKE_KEY_RATE,
  MATCHMAKE_KEY_CUPNO,
  MATCHMAKE_KEY_DISCONNECT,
  MATCHMAKE_KEY_BTLCNT,
  MATCHMAKE_KEY_VERLANG,

  MATCHMAKE_KEY_MAX,
} MATCHMAKE_KEY;

//マッチメイクタイミングシンク

//マッチメイク評価計算式の定数
#define MATCHMAKE_EVAL_CALC_RATE_STANDARD       (1000)  //定数A
#define MATCHMAKE_EVAL_CALC_WEIGHT              (1)     //定数B
#define MATCHMAKE_EVAL_CALC_DISCONNECT_WEIGHT   (32)    //定数C
#define MATCHMAKE_EVAL_CALC_DISCONNECT_REVISE   (5)     //定数D
#define MATCHMAKE_EVAL_CALC_DISCONNECT_RATE     (50)    //定数E

#ifdef DEBUG_EVAL_FREE
#undef MATCHMAKE_EVAL_CALC_RATE_STANDARD
#define MATCHMAKE_EVAL_CALC_RATE_STANDARD (65535)
#endif //DEBUG_EVAL_FREE

//-------------------------------------
///	受信フラグ
//=====================================
typedef enum
{
  WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER,
  WIFIBATTLEMATCH_NET_RECVFLAG_GAMEDATA,
  WIFIBATTLEMATCH_NET_RECVFLAG_CANCELREQUEST,
  WIFIBATTLEMATCH_NET_RECVFLAG_POKEPARTY,
  WIFIBATTLEMATCH_NET_RECVFLAG_DIRTYCNT,
  WIFIBATTLEMATCH_NET_RECVFLAG_FLAG,

  WIFIBATTLEMATCH_NET_RECVFLAG_MAX
} WIFIBATTLEMATCH_NET_RECVFLAG;

//-------------------------------------
///	アトリビュート
//=====================================
#define WIFI_FILE_ATTR1			"REGCARD_J"
//#define WIFI_FILE_ATTR2		//ここに大会番号が入っている
#define WIFI_FILE_ATTR3			""
#define REGULATION_CARD_DATA_SIZE (sizeof(REGULATION_CARDDATA))

//-------------------------------------
///	デバッグ
//=====================================
#include "wifibattlematch_debug.h"

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	書き込み情報
//=====================================
typedef struct 
{
  WIFIBATTLEMATCH_BTLRULE   btl_rule;
  BtlResult btl_result;
  u32      is_dirty;
} DWC_SC_WRITE_DATA;
//SCのリポート作成コア関数
typedef DWCScResult (*DWCRAP_SC_CREATE_REPORT_CORE_FUNC)( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect );

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
///	エラー
//=====================================
typedef struct 
{
  DWCScResult                 sc_err;
  DWCGdbError                 gdb_err;
  DWCGdbState                 gdb_state;
  DWCGdbAsyncResult           gdb_result;
  DWCNdError                  nd_err;
  NHTTPError                  nhttp_err;
  WIFIBATTLEMATCH_NET_SYSERROR  sys_err;
  WIFIBATTLEMATCH_NET_ERRORTYPE type;
} WIFIBATTLEMATCH_NETERR_WORK;


typedef void (*DWCGdbGetRecordsCallbackEx)(int record_num, DWCGdbField** records, void* user_param, int field_num);

//-------------------------------------
///	ネットモジュール
//=====================================
struct _WIFIBATTLEMATCH_NET_WORK
{ 
  const DWCUserData *cp_user_data;
  WIFI_LIST         *p_wifilist;
  GAMEDATA          *p_gamedata;
  s32               pid;
  DWCSvlResult      *p_svl_result;
  s32               timing_no;

  //エラー
  WIFIBATTLEMATCH_NETERR_WORK error;
  
  //日時
  WBM_NET_DATETIME  datetime;

  //マッチング
  u32 seq_matchmake;
  char filter[128];
  MATCHMAKE_KEY_WORK  key_wk[ MATCHMAKE_KEY_MAX ];


  HEAPID heapID;
  u32   init_seq;
  u32   seq;
  u32   next_seq;
  u32   wait_cnt;
  BOOL  is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_MAX];
  u32   recv_flag;
  u32   disconnect_seq;
  
  //SC
//  DWC_SC_PLAYERDATA player[2];  //自分は０ 相手は１
  DWCRAP_SC_CREATE_REPORT_CORE_FUNC SC_CreateReportCoreFunc;
  DWCScResult       result;
  BOOL is_sc_error;
  BOOL is_session;
  BOOL is_sc_you_recv;
  BOOL is_send_report;
  BOOL is_sc_start;

  //ポケモン不正検査で使う
  u32             poke_max;

  //gdb
  void *p_get_wk;
  DWCGdbGetRecordsCallbackEx  gdb_get_record_callback;
  DWC_SC_WRITE_DATA report;
  DWCGdbField *p_field_buff;
  const char **pp_table_name;
  int table_name_num;
  u32   magic_num;
  u32 async_timeout;
  u32 cancel_select_timeout;   //CANCELSELECT_TIMEOUT
  BOOL is_auth;
  WIFIBATTLEMATCH_SC_REPORT_TYPE report_type;
  int get_recordID;
  u16 playername_unicode[PERSON_NAME_SIZE + EOM_SIZE];
  WIFIBATTLEMATCH_GDB_RND_RECORD_DATA record_data;

  //SENDDATA系
  u8 recv_buffer[RECV_BUFFER_SIZE];//受信バッファ

  //マッチメイク
  DWCEvalPlayerCallback matchmake_eval_callback;

  //GPFサーバー
  NHTTP_RAP_WORK* p_nhttp;
  DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA gpf_data;
  DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA gdb_write_data;

  //以下ダウンロード用
  REGULATION_CARDDATA temp_buffer;
  DWCNdFileInfo fileInfo;
  u32               percent;
  u32               recived;
  u32               contentlen;
  char              nd_attr2[10];

  //以下バックアップ用
  s32 init_profileID;
  s32 now_profileID;
  
  WIFIBATTLEMATCH_NET_DATA *p_data;

  //以下不正文字チェック用
  DWC_TOOL_BADWORD_WORK badword;
  MYSTATUS              *p_badword_mystatus;

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
static u8 s_cleanup_callback_flag   = FALSE;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void WifiBattleMatch_SetDateTime( WBM_NET_DATETIME *p_wk );
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
static u8* WifiBattleMatch_RecvBuffAddr(int netID, void* p_wk_adrs, int size);
static void WifiBattleMatch_RecvMatchCancel(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle);
static void WifiBattleMatch_RecvPokeParty(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle);
static void WifiBattleMatch_RecvDirtyCnt(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle);
static void WifiBattleMatch_RecvFlag(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle);

//-------------------------------------
///	SC関係
//=====================================
//DWCコールバック
static void DwcRap_Sc_CreateSessionCallback( DWCScResult theResult, void* theUserData );
static void DwcRap_Sc_SetReportIntentionCallback( DWCScResult theResult, void* theUserData );
static void DwcRap_Sc_SubmitReportCallback( DWCScResult theResult, void* theUserData );
//private
static void DwcRap_Sc_Finalize( WIFIBATTLEMATCH_NET_WORK *p_wk );
static DWCScResult DwcRap_Sc_CreateReport( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const DWC_SC_WRITE_DATA *cp_data, BOOL is_auth, DWCRAP_SC_CREATE_REPORT_CORE_FUNC SC_CreateReportCoreFunc, BOOL is_disconnect, WIFIBATTLEMATCH_SC_REPORT_TYPE report_type, BOOL is_sc_you_recv );
static DWCScResult DwcRap_Sc_CreateReportDebug( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const WIFIBATTLEMATCH_SC_DEBUG_DATA * cp_data, BOOL is_auth );
//リポート作成コールバック
static DWCScResult DwcRap_Sc_CreateReportRndCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect );
static DWCScResult DwcRap_Sc_CreateReportWifiCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect );
static DWCScResult DwcRap_Sc_CreateReportRndCoreBtlAfter( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect );
static DWCScResult DwcRap_Sc_CreateReportWifiCoreBtlAfter( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect );
static DWCScResult DwcRap_Sc_CreateReportRndCoreBtlError( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect );
static DWCScResult DwcRap_Sc_CreateReportWifiCoreBtlError( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect );
//-------------------------------------
///	GDB関係
//=====================================
static void DwcRap_Gdb_GetCallback(int record_num, DWCGdbField** records, void* user_param);

static void DwcRap_Gdb_Rnd_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num );
static void DwcRap_Gdb_Wifi_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num );
static void DwcRap_Gdb_RecordID_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num);
static void DwcRap_Gdb_PokeParty_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num);
static void DwcRap_Gdb_LoginDate_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num );
static void DwcRap_Gdb_SakeAll_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num);
static void DwcRap_Gdb_Finalize( WIFIBATTLEMATCH_NET_WORK *p_wk );
static void DwcRap_Gdb_SetMyInfo( WIFIBATTLEMATCH_NET_WORK *p_wk );


#ifdef PM_DEBUG
static void print_field(DWCGdbField* field); // レコードをデバッグ出力する。
#else
#define print_field(...) /*  */
#endif

//-------------------------------------
///	ダウンロード関係
//=====================================
static void DwcRap_Nd_WaitNdCallback( WIFIBATTLEMATCH_NET_WORK *p_wk, int next_seq );
static void DwcRap_Nd_CleanUpNdCallback( WIFIBATTLEMATCH_NET_WORK *p_wk, int next_seq );
static void NdCallback(DWCNdCallbackReason reason, DWCNdError error, int servererror);

//-------------------------------------
///	エラー関係
//=====================================
static void WIFIBATTLEMATCH_NETERR_Clear( WIFIBATTLEMATCH_NETERR_WORK *p_wk );
static void WIFIBATTLEMATCH_NETERR_Main( WIFIBATTLEMATCH_NETERR_WORK *p_wk );
static void WIFIBATTLEMATCH_NETERR_SetSysError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, WIFIBATTLEMATCH_NET_SYSERROR error, int line );
static void WIFIBATTLEMATCH_NETERR_SetScError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCScResult sc_err, int line );
static void WIFIBATTLEMATCH_NETERR_SetGdbError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCGdbError gdb_err, int line );
static void WIFIBATTLEMATCH_NETERR_SetGdbState( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCGdbState state, int line );
static void WIFIBATTLEMATCH_NETERR_SetGdbResult( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCGdbAsyncResult result, int line );
static void WIFIBATTLEMATCH_NETERR_SetNdError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCNdError nd_err, int line );
static void WIFIBATTLEMATCH_NETERR_SetNhttpError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, NHTTPError nhttp_err, int line );
static WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_SC_GetErrorRepairType( DWCScResult error );
static WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_GDB_GetErrorRepairType( DWCGdbError error, DWCGdbState gdb_state, DWCGdbAsyncResult gdb_result );
static WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_ND_GetErrorRepairType( DWCNdError nd_err );
static WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_SYS_GetErrorRepairType( WIFIBATTLEMATCH_NET_SYSERROR  sys_err );
static void WIFIBATTLEMATCH_NETERR_ClearError( WIFIBATTLEMATCH_NETERR_WORK *p_wk );


//-------------------------------------
///	その他
//=====================================
static BOOL UTIL_IsClear( void *p_adrs, u32 size );

static void WifiBattleMatch_ErrDisconnectCallback(void* p_wk_adrs, int code, int type, int ret );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	マッチメイクキー文字列
//    3文字固定
//=====================================
static const char *sc_matchmake_key_str[ MATCHMAKE_KEY_MAX ] =
{ 
  "mod",
  "rul",
  "deb",
  "rat",
  "cup",
  "dis",
  "cnt",
  "ver",
};
//-------------------------------------
///	通信コマンド
//=====================================
enum
{ 
  WIFIBATTLEMATCH_NETCMD_SEND_PLAYERDATA  = GFL_NET_CMD_WIFIMATCH,
  WIFIBATTLEMATCH_NETCMD_SEND_ENEMYDATA,
  WIFIBATTLEMATCH_NETCMD_SEND_CANCELMATCH,
  WIFIBATTLEMATCH_NETCMD_SEND_POKEPARTY,
  WIFIBATTLEMATCH_NETCMD_SEND_DIRTYCNT,
  WIFIBATTLEMATCH_NETCMD_SEND_FLAG,
  WIFIBATTLEMATCH_NETCMD_MAX,
};
static const NetRecvFuncTable sc_net_recv_tbl[] = 
{
  {DwcRap_Sc_RecvPlayerData,   NULL},    ///_NETCMD_INFOSEND
  {WifiBattleMatch_RecvEnemyData, WifiBattleMatch_RecvBuffAddr },
  {WifiBattleMatch_RecvMatchCancel, NULL},
  {WifiBattleMatch_RecvPokeParty, WifiBattleMatch_RecvBuffAddr },
  {WifiBattleMatch_RecvDirtyCnt, WifiBattleMatch_RecvBuffAddr },
  {WifiBattleMatch_RecvFlag, NULL },
};

//-------------------------------------
///	サケ全情報
//=====================================
static const char *sc_get_debugall_tbl[]  =
{ 
  "ARENA_ELO_RATING_1V1_DOUBLE",
  "ARENA_ELO_RATING_1V1_ROTATE",
  "ARENA_ELO_RATING_1V1_SHOOTER",
  "ARENA_ELO_RATING_1V1_SINGLE",
  "ARENA_ELO_RATING_1V1_TRIPLE",
  "CHEATS_COUNTER",
  "COMPLETE_MATCHES_COUNTER",
  "DISCONNECTS_COUNTER",
  "NUM_DOUBLE_LOSE_COUNTER",
  "NUM_DOUBLE_WIN_COUNTER",
  "NUM_ROTATE_LOSE_COUNTER",
  "NUM_ROTATE_WIN_COUNTER",
  "NUM_SHOOTER_LOSE_COUNTER",
  "NUM_SHOOTER_WIN_COUNTER",
  "NUM_SINGLE_LOSE_COUNTER",
  "NUM_SINGLE_WIN_COUNTER",
  "NUM_TRIPLE_LOSE_COUNTER",
  "NUM_TRIPLE_WIN_COUNTER",
  "ARENA_ELO_RATING_1V1_WIFICUP",
  "CHEATS_WIFICUP_COUNTER",
  "DISCONNECTS_WIFICUP_COUNTER",
  "NUM_WIFICUP_LOSE_COUNTER",
  "NUM_WIFICUP_WIN_COUNTER",
  "WIFICUP_POKEMON_PARTY",
  "LAST_LOGIN_DATETIME",
  "MYSTATUS",
  "RECORD_DATA_01",
  "RECORD_DATA_02",
  "RECORD_DATA_03",
  "RECORD_DATA_04",
  "RECORD_DATA_05",
  "RECORD_DATA_06",
  "RECORD_SAVE_IDX",
  "recordid",
  "ownerid",
  "INITIAL_PROFILE_ID",
  "NOW_PROFILE_ID",
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
 *  @param  GAMEDATA        WiFIListやMystatus取得用
 *  @param  DWCSvlResult    WiFILoginからもらったSVL。不正サーバーを使わないならばNULLでもよい
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_WORK * WIFIBATTLEMATCH_NET_Init( WIFIBATTLEMATCH_NET_DATA *p_data, GAMEDATA *p_gamedata, DWCSvlResult *p_svl_result, HEAPID heapID )
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_NET_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_NET_WORK) );

  p_wk->p_wifilist    = GAMEDATA_GetWiFiList( p_gamedata );
  p_wk->cp_user_data  = WifiList_GetMyUserInfo( p_wk->p_wifilist );
  p_wk->pid           = MyStatus_GetProfileID( GAMEDATA_GetMyStatus( p_gamedata ) );
  p_wk->p_gamedata    = p_gamedata;
  p_wk->p_svl_result  = p_svl_result;
  p_wk->p_data        = p_data;

  p_wk->magic_num = 0x573;

#ifdef DEBUG_ONLY_FOR_toru_nagihashi
  GFL_NET_DebugGetPrintOn();
#endif

  p_wk->p_field_buff  = GFL_HEAP_AllocMemory( heapID, WBM_GDB_FIELD_TABLE_MAX * sizeof(DWCGdbField) );
  GFL_STD_MemClear(p_wk->p_field_buff, WBM_GDB_FIELD_TABLE_MAX * sizeof(DWCGdbField) );

#ifdef PM_DEBUG
  GFL_NET_DebugPrintOn();
#endif //PM_DEBUG

  if(!GFL_NET_IsInit())
  {
    GF_ASSERT(0);
  }

  GFL_NET_AddCommandTable( GFL_NET_CMD_WIFIMATCH, sc_net_recv_tbl, NELEMS(sc_net_recv_tbl), p_wk );
  GFL_NET_SetWifiBothNet(FALSE);

  GFL_NET_DWC_SetErrDisconnectCallback( WifiBattleMatch_ErrDisconnectCallback, p_wk );

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
  GFL_NET_DWC_SetErrDisconnectCallback( NULL, NULL );

  GFL_NET_DelCommandTable( GFL_NET_CMD_WIFIMATCH );

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
 *	@brief  タイミングとり
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	timing_no                         タイミングNO
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartTiming( WIFIBATTLEMATCH_NET_WORK *p_wk, int timing_no )
{ 
  p_wk->timing_no = timing_no;
  GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), p_wk->timing_no, WB_NET_WIFIMATCH);
}

//----------------------------------------------------------------------------
/**
 *	@brief  同期待ち
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *
 *	@return TRUEで同期をとった  FALSEでとっていない
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_WaitTiming( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  return GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),p_wk->timing_no,WB_NET_WIFIMATCH);
}

//----------------------------------------------------------------------------
/**
 *	@brief  DWCのエラー修復タイプを取得
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *
 *	@return 修復タイプを取得
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_NET_CheckErrorRepairType( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_heavy, BOOL is_timeout )
{ 
  WIFIBATTLEMATCH_NETERR_WORK           *p_error  = &p_wk->error;
  WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE repair  = WIFIBATTLEMATCH_NET_ERROR_NONE;


  //※１はエラー画面の後によんでしまうとまずいためエラー画面の前で呼ぶ
  //->エラー画面でエラーここにくる→内部でdisconnect
  //  →disconnect内でエラークリア


  //DWCのエラー
  if( GFL_NET_IsInit() )
  { 
    //※１
    if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE )
    {
      GFL_NET_SetAutoErrorCheck(FALSE);
      GFL_NET_SetNoChildErrorCheck(FALSE);
    }

    //下記関数はdev_wifilibのオーバーレイにあるので、GFL_NETが解放されるとよばれなくなる
    switch( GFL_NET_DWC_ERROR_ReqErrorDisp(is_heavy, is_timeout) )
    { 
    case GFL_NET_DWC_ERROR_RESULT_NONE:
      repair  = WIFIBATTLEMATCH_NET_ERROR_NONE;
      break;

    case GFL_NET_DWC_ERROR_RESULT_PRINT_MSG:
      repair  = WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;
      break;

    case GFL_NET_DWC_ERROR_RESULT_RETURN_PROC:
      repair  = WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
      break;

    case GFL_NET_DWC_ERROR_RESULT_FATAL:
      repair  = WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL;
      break;

    case GFL_NET_DWC_ERROR_RESULT_TIMEOUT:
      repair  = WIFIBATTLEMATCH_NET_ERROR_REPAIR_TIMEOUT;
      break;
    }
    if( repair != GFL_NET_DWC_ERROR_RESULT_NONE )
    {
      DEBUG_NET_Printf( "DWCのエラー検知 %d\n",repair );
    }

    //エラー画面とエラーの解決をすでにしているので抜ける
    return repair;
  }

  //個別のエラー
  if( repair == WIFIBATTLEMATCH_NET_ERROR_NONE )
  { 
    BOOL is_clear = FALSE;
    switch( p_error->type )
    { 
    case WIFIBATTLEMATCH_NET_ERRORTYPE_NONE:
      repair  = WIFIBATTLEMATCH_NET_ERROR_NONE;
      break;

    case WIFIBATTLEMATCH_NET_ERRORTYPE_SC:
      repair  = WIFIBATTLEMATCH_SC_GetErrorRepairType( p_error->sc_err );
      break;

    case WIFIBATTLEMATCH_NET_ERRORTYPE_GDB:
      repair  = WIFIBATTLEMATCH_GDB_GetErrorRepairType( p_error->gdb_err, p_error->gdb_state, p_error->gdb_result );
      break;

    case WIFIBATTLEMATCH_NET_ERRORTYPE_ND:
      repair  = WIFIBATTLEMATCH_NET_ERROR_NONE;
      break;

    case WIFIBATTLEMATCH_NET_ERRORTYPE_NHTTP:
      //シーケンスの戻り値でエラーを返すのでここでは行わない
      repair  = WIFIBATTLEMATCH_NET_ERROR_NONE;//WIFIBATTLEMATCH_ND_GetErrorRepairType( p_error->nd_err );
      break;

    case WIFIBATTLEMATCH_NET_ERRORTYPE_SYS:
      repair  = WIFIBATTLEMATCH_SYS_GetErrorRepairType( p_error->sys_err );
      break;

    default:
      repair  =  WIFIBATTLEMATCH_NET_ERROR_NONE;
      break;
    }

    if( repair != GFL_NET_DWC_ERROR_RESULT_NONE )
    {
      GFL_NET_SetAutoErrorCheck(FALSE);
      GFL_NET_SetNoChildErrorCheck(FALSE);
      DEBUG_NET_Printf( "ライブラリ個別のエラー検知 %d\n",repair );
    }

    switch( repair )
    { 
    case WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN:      //復帰可能地点まで戻る
      GFL_NET_StateResetError();
      GFL_NET_StateClearWifiError();
      NetErr_DispCallPushPop();
      NetErr_ErrWorkInit();
      break;
    case WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT:  //切断しログインからやり直し

      //  もしDWCのエラーがなかった場合エラーを立てる
      //  システムエラーやGDBの結果等一部はDWCのエラーがたたないので
      if( NET_ERR_CHECK_NONE == NetErr_App_CheckError() )
      {
        GFL_NET_StateSetWifiError(0, 0, 0, ERRORCODE_SYSTEM );
        NetErr_ErrorSet();
      }

      NetErr_App_ReqErrorDisp();
      break;
    case WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL:       //電源切断
      NetErr_DispCallFatal();
      break;
    }

    if( repair != WIFIBATTLEMATCH_NET_ERROR_NONE )
    {
      WIFIBATTLEMATCH_NETERR_ClearError( p_error );
    }
  }

  return repair;
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
  p_wk->seq       = 0;
  p_wk->init_seq  = 0;
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
WIFIBATTLEMATCH_GDB_RESULT WIFIBATTLEMATCH_NET_WaitInitialize( WIFIBATTLEMATCH_NET_WORK *p_wk )
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
    WIFIBATTLEMATCH_GDB_Start( p_wk, WIFIBATTLEMATCH_GDB_MYRECORD, WIFIBATTLEMATCH_GDB_GET_RECORDID, &p_wk->p_data->sake_recordID );
    p_wk->init_seq  = SEQ_WAIT_GET_TABLE_ID;
    break;

  case SEQ_WAIT_GET_TABLE_ID:
    {
      WIFIBATTLEMATCH_GDB_RESULT  res = WIFIBATTLEMATCH_GDB_Process( p_wk );
      if( res == WIFIBATTLEMATCH_GDB_RESULT_SUCCESS )
      { 
        if( p_wk->p_data->sake_recordID != 0 )
        { 
          p_wk->init_seq  = SEQ_END;
        }
        else
        { 
          DEBUG_NET_Printf( "テーブルIDが0だったので、新規作成します\n" );
          p_wk->init_seq  = SEQ_START_CREATE_RECORD;
        }
      }
      else if( res == WIFIBATTLEMATCH_GDB_RESULT_ERROR )
      {
        return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
      }
    }
    break;

  case SEQ_START_CREATE_RECORD:
    WIFIBATTLEMATCH_GDB_StartCreateRecord( p_wk );
    p_wk->init_seq  = SEQ_WAIT_CREATE_RECORD;
    break;

  case SEQ_WAIT_CREATE_RECORD:
    {
      WIFIBATTLEMATCH_GDB_RESULT  res = WIFIBATTLEMATCH_GDB_ProcessCreateRecord( p_wk );
      if( res == WIFIBATTLEMATCH_GDB_RESULT_SUCCESS )
      { 
        p_wk->init_seq  = SEQ_END;
      }
      else if( res == WIFIBATTLEMATCH_GDB_RESULT_ERROR )
      {
        return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
      }
    }
    break;


  case SEQ_END:
    DEBUG_NET_Printf( "sake テーブルID %d\n", p_wk->p_data->sake_recordID );
    return WIFIBATTLEMATCH_GDB_RESULT_SUCCESS;
  }

  return WIFIBATTLEMATCH_GDB_RESULT_UPDATE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチメイク開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_TYPE mode, BOOL is_rnd_rate, WIFIBATTLEMATCH_BTLRULE btl_rule, const WIFIBATTLEMATCH_MATCH_KEY_DATA *cp_data )
{ 
  u32 btl_mode  = mode;
  if( mode == WIFIBATTLEMATCH_MODE_RANDOM )
  { 
    btl_mode  += is_rnd_rate;
  }

  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_BTL_MODE, btl_mode );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_BTL_RULE, btl_rule );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_DEBUG, MATCHINGKEY );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_RATE, cp_data->rate );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_DISCONNECT, cp_data->disconnect );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_CUPNO, cp_data->cup_no );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_BTLCNT, cp_data->btlcnt );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_VERLANG, PM_VERSION | (PM_LANG << 16) ); //@todo forupper もしアッパーバージョンでバトルサーバーバージョンが上がったらこの値を見て繋がらないようにする
  STD_TSPrintf( p_wk->filter, "mod=%d And rul=%d And deb=%d", btl_mode, btl_rule, MATCHINGKEY );
  OS_TFPrintf( 3, "%s\n", p_wk->filter );
  p_wk->seq_matchmake = 0;
  p_wk->async_timeout = 0;
  p_wk->cancel_select_timeout = 0;

  //接続評価コールバック指定
  switch( mode )
  { 
  case WIFIBATTLEMATCH_TYPE_WIFICUP:    //WIFI大会
    p_wk->matchmake_eval_callback = WIFIBATTLEMATCH_WIFI_Eval_Callback;
    break;
  case WIFIBATTLEMATCH_TYPE_LIVECUP:    //ライブ大会
    GF_ASSERT( 0 );
    break;
  case WIFIBATTLEMATCH_TYPE_RNDRATE:  //ランダムレート対戦
    p_wk->matchmake_eval_callback = WIFIBATTLEMATCH_RND_RATE_Eval_Callback;
    break;
  case WIFIBATTLEMATCH_TYPE_RNDFREE:  //ランダムフリー対戦
    p_wk->matchmake_eval_callback = WIFIBATTLEMATCH_RND_FREE_Eval_Callback;
    break;
  default:
    GF_ASSERT(0);
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
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_BTL_MODE, 0 );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_BTL_RULE, 0 );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_DEBUG, MATCHINGKEY );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_RATE, 0 );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_DISCONNECT, 0 );
  MATCHMAKE_KEY_Set( p_wk, MATCHMAKE_KEY_CUPNO, 0 );
  STD_TSPrintf( p_wk->filter, "mod=%d And rul=%d And deb=%d", 0, 0, MATCHINGKEY );
  p_wk->seq_matchmake = 0;

  p_wk->matchmake_eval_callback = WIFIBATTLEMATCH_RND_FREE_Eval_Callback;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチメイク処理
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *  @ret  終了コード
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_MATCHMAKE_STATE WIFIBATTLEMATCH_NET_WaitMatchMake( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  enum
  {
    WIFIBATTLEMATCH_NET_SEQ_MATCH_START,
    WIFIBATTLEMATCH_NET_SEQ_MATCH_START2,
    WIFIBATTLEMATCH_NET_SEQ_MATCH_WAIT,

    WIFIBATTLEMATCH_NET_SEQ_CONNECT_START,
    WIFIBATTLEMATCH_NET_SEQ_CONNECT_CHILD,
    WIFIBATTLEMATCH_NET_SEQ_CONNECT_PARENT,
    WIFIBATTLEMATCH_NET_SEQ_TIMING_END,

    WIFIBATTLEMATCH_NET_SEQ_CANCEL,

    WIFIBATTLEMATCH_NET_SEQ_MATCH_END,
  };

  //DEBUG_NET_Printf( "WiFiState %d \n", GFL_NET_StateGetWifiStatus() );
  switch( p_wk->seq_matchmake )
  { 
  case WIFIBATTLEMATCH_NET_SEQ_MATCH_START:
    {
      if( GFL_NET_DWC_StartMatchFilter( p_wk->filter, 2, p_wk->matchmake_eval_callback, p_wk ) != 0 )
      {
        GFL_NET_StateStartWifiRandomMatch_RateMode();
        GFL_NET_DWC_SetVChat( FALSE );
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_START2;
      }
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_MATCH_START2:
    if( 1 )
    { 
      p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_WAIT;
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_MATCH_WAIT:
    {
      int ret;
#if 1
      if(GFL_NET_STATE_MATCHED == GFL_NET_StateGetWifiStatus())
      {
        p_wk->cancel_select_timeout = 0;
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CONNECT_START;
        GFL_NET_SetAutoErrorCheck(TRUE);
        GFL_NET_SetNoChildErrorCheck(TRUE);
      }
#else
      ret = GFL_NET_DWC_GetStepMatchResult();
      switch(ret)
      {
      case STEPMATCH_CONNECT:
      case STEPMATCH_CONTINUE:
        /* 接続中 */
        break;

      case STEPMATCH_SUCCESS:
        p_wk->cancel_select_timeout = 0;
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CONNECT_START;
        break;

      case STEPMATCH_CANCEL:
      case STEPMATCH_FAIL:
        return WIFIBATTLEMATCH_NET_MATCHMAKE_STATE_FAILED;

      default:
        GF_ASSERT_MSG(0,"StepMatchResult=[%d]\n",ret);
      }
#endif
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_CONNECT_START:
    {
      if( GFL_NET_SystemGetCurrentID() != GFL_NET_NO_PARENTMACHINE )
      {  // 子機として接続が完了した
        if( GFL_NET_HANDLE_RequestNegotiation() )
        {
          p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CONNECT_CHILD;
        }

        if( p_wk->cancel_select_timeout++ > CANCELSELECT_TIMEOUT )
        { 
          DEBUG_NET_Printf( "マッチングネゴシエーションタイムアウトline %d\n", __LINE__ );

          WIFIBATTLEMATCH_NET_StartDisConnect( p_wk );
          p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CANCEL;
        }
      }
      else
      {
        p_wk->cancel_select_timeout = 0;
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CONNECT_PARENT;
      }
      DEBUG_NET_Printf( "接続処理 %d\n",GFL_NET_SystemGetCurrentID() );
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_CONNECT_CHILD:
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_NET_TIMINGSYNC_CONNECT, WB_NET_WIFIMATCH);
    p_wk->cancel_select_timeout = 0;
    p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_TIMING_END;
    break;

  case WIFIBATTLEMATCH_NET_SEQ_CONNECT_PARENT:
    if( p_wk->cancel_select_timeout++ > CANCELSELECT_TIMEOUT )
    { 
      DEBUG_NET_Printf( "マッチングネゴシエーションタイムアウトline %d\n", __LINE__ );
      WIFIBATTLEMATCH_NET_StartDisConnect( p_wk );
      p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CANCEL;
    }
    else if( GFL_NET_HANDLE_GetNumNegotiation() != 0 )
    {
      if( GFL_NET_HANDLE_RequestNegotiation() )
      {
        GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_NET_TIMINGSYNC_CONNECT, WB_NET_WIFIMATCH);
        p_wk->cancel_select_timeout = 0;
        p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_TIMING_END;
      }
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_TIMING_END:
    if( p_wk->cancel_select_timeout++ > CANCELSELECT_TIMEOUT )
    { 
      DEBUG_NET_Printf( "マッチングネゴシエーションタイムアウトline %d\n", __LINE__ );
      WIFIBATTLEMATCH_NET_StartDisConnect( p_wk );
      p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_CANCEL;
    }
    else if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_NET_TIMINGSYNC_CONNECT,WB_NET_WIFIMATCH))
    {
      p_wk->seq_matchmake = WIFIBATTLEMATCH_NET_SEQ_MATCH_END;
    }
    break;

  case WIFIBATTLEMATCH_NET_SEQ_CANCEL:
    if( WIFIBATTLEMATCH_NET_WaitDisConnect( p_wk ) )
    {
      //マッチングタイムアウトのときがあるので

      { 
        const GFL_NETSTATE_DWCERROR* cp_error  =  GFL_NET_StateGetWifiError();
        if( cp_error->errorUser == ERRORCODE_TIMEOUT )
        {
          GFL_NET_StateClearWifiError();  //WIFIエラー詳細情報クリア
          GFL_NET_StateResetError();      //NET内エラー情報クリア
          NetErr_ErrWorkInit();           //エラーシステム情報クリア
          GFL_NET_StateWifiMatchEnd(TRUE);
        }
      }
      return WIFIBATTLEMATCH_NET_MATCHMAKE_STATE_FAILED;
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
    return WIFIBATTLEMATCH_NET_MATCHMAKE_STATE_SUCCESS;
  }

  return WIFIBATTLEMATCH_NET_MATCHMAKE_STATE_UPDATE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチメイクを強制切断する  （エラーが起きたときなどに使う）
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_SetDisConnectForce( WIFIBATTLEMATCH_NET_WORK *p_wk )
{
  GFL_NET_SetAutoErrorCheck(FALSE);
  GFL_NET_SetNoChildErrorCheck(FALSE);

  GFL_NET_StateWifiMatchEnd(TRUE);
}


//----------------------------------------------------------------------------
/**
 *	@brief  マッチング同期切断開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartDisConnect( WIFIBATTLEMATCH_NET_WORK *p_wk )
{
  p_wk->disconnect_seq  = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マッチング同期切断処理中
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *
 *	@return TRUEで切断終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_WaitDisConnect( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  enum
  {
    SEQ_START_INIT_TIMEING,
    SEQ_WAIT_INIT_TIMEING,
    SEQ_SET_DISCONNECT_FLAG,
    SEQ_WAIT_CNT,
    SEQ_DISCONNECT,
    SEQ_END,
  };

  switch( p_wk->disconnect_seq )
  {
  case SEQ_START_INIT_TIMEING:
    WIFIBATTLEMATCH_NET_StartTiming( p_wk, WIFIBATTLEMATCH_NET_TIMINGSYNC_DISCONNECT_INIT );
    p_wk->disconnect_seq++;
    break;
  case SEQ_WAIT_INIT_TIMEING:
    if( WIFIBATTLEMATCH_NET_WaitTiming( p_wk ) )
    {
      p_wk->disconnect_seq++;
    }
    break;
  case SEQ_SET_DISCONNECT_FLAG:
    GFL_NET_SetAutoErrorCheck(FALSE);
    GFL_NET_SetNoChildErrorCheck(FALSE);
    p_wk->wait_cnt  = 0;
    p_wk->disconnect_seq++;
    break;
  case SEQ_WAIT_CNT:
    if( p_wk->wait_cnt++> DISCONNECTS_WAIT_SYNC )
    {
      p_wk->disconnect_seq++;
    }
    break;
  case SEQ_DISCONNECT:
    GFL_NET_StateWifiMatchEnd(TRUE);
    p_wk->disconnect_seq++;
    break;
  case SEQ_END:
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分がキャンセルしたことを相手へ通知する
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *
 *	@return TRUEで送信完了  FALSEで送信継続
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_SendMatchCancel( WIFIBATTLEMATCH_NET_WORK *p_wk )
{
  static const int sc_temp  = 0;

  {
    if( GFL_NET_DWC_IsMatched() )
    {
      return GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), 
          WIFIBATTLEMATCH_NETCMD_SEND_CANCELMATCH, sizeof(int), &sc_temp );
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  相手がキャンセルしたかチェック
 *
 *	@param	const WIFIBATTLEMATCH_NET_WORK *cp_wk ワーク
 *
 *	@return TRUEで相手がキャンセルした  FALSEでしていない
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_RecvMatchCancel( WIFIBATTLEMATCH_NET_WORK *p_wk )
{
  BOOL  ret;
  ret = p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_CANCELREQUEST];

  if( ret )
  {
    p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_CANCELREQUEST] = FALSE;
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ログイン時間から、現在の日時を取り出す
 *
 *	@param	WBM_NET_DATETIME *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void WifiBattleMatch_SetDateTime( WBM_NET_DATETIME *p_wk )
{ 
  RTCDate date;
  RTCTime time;
  BOOL ret  = DWC_GetDateTime(&date, &time);
  GF_ASSERT( ret );

  GFL_STD_MemClear( p_wk, sizeof(WBM_NET_DATETIME) );

  p_wk->year  = date.year;
  p_wk->month = date.month;
  p_wk->day   = date.day;
  p_wk->hour  = time.hour;
  p_wk->minute= time.minute;
  p_wk->second= time.second;
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
  p_key_wk->keyID  = DWC_AddMatchKeyInt( p_key_wk->keyID,
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
  //今のところランダムマッチレーティングとWIFI大会は同じ評価計算
  return WIFIBATTLEMATCH_WIFI_Eval_Callback( index, p_param_adrs );
}
//フリーモード用
static int WIFIBATTLEMATCH_RND_FREE_Eval_Callback( int index, void* p_param_adrs )
{ 
  //フリーモードは誰とでも同程度繋がる
  DEBUG_NET_Printf( "フリーモード評価コールバック！\n" );
  return 1000;

}
//WIFI大会用
static int WIFIBATTLEMATCH_WIFI_Eval_Callback( int index, void* p_param_adrs )
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_param_adrs;

  int value=0;
  int rate, disconnect, cup, btlcnt;

  int you_rate      = DWC_GetMatchIntValue( index, p_wk->key_wk[MATCHMAKE_KEY_RATE].keyStr, -1 );
  disconnect= DWC_GetMatchIntValue(index, p_wk->key_wk[MATCHMAKE_KEY_DISCONNECT].keyStr, -1 );
  btlcnt    = DWC_GetMatchIntValue(index, p_wk->key_wk[MATCHMAKE_KEY_BTLCNT].keyStr, -1 );
  cup       = DWC_GetMatchIntValue( index, p_wk->key_wk[MATCHMAKE_KEY_CUPNO].keyStr, -1 );

  //指標キーが反映されていないので、無視する
  if( you_rate == -1 && disconnect == -1 && cup == -1 && btlcnt == -1 )
  { 
    DEBUG_NET_Printf( "▼指標キーが反映されていない\n" );
    return 0;
  }

  if( cup == p_wk->key_wk[MATCHMAKE_KEY_CUPNO].ivalue )
  {
    int my_disconnect_rate;
    int you_disconnect_rate;
    int disconnect_rate;

    //切断比率を求める
    you_disconnect_rate = MATCHMAKE_EVAL_CALC_DISCONNECT_RATE *
      disconnect / (btlcnt+MATCHMAKE_EVAL_CALC_DISCONNECT_REVISE);

    my_disconnect_rate  = MATCHMAKE_EVAL_CALC_DISCONNECT_RATE * 
      p_wk->key_wk[MATCHMAKE_KEY_DISCONNECT].ivalue / 
      (p_wk->key_wk[MATCHMAKE_KEY_BTLCNT].ivalue + MATCHMAKE_EVAL_CALC_DISCONNECT_REVISE);

    //差を求める
    rate        = you_rate;
    rate        -= p_wk->key_wk[MATCHMAKE_KEY_RATE].ivalue;
    rate        = MATH_ABS( rate );
    disconnect_rate  = you_disconnect_rate - my_disconnect_rate;
    disconnect_rate  = MATH_ABS( disconnect_rate );

    //評価計算式
    //マッチング評価値  ＝　（定数A - レーティング差） ＊ 定数B　ー　切断比率差 ＊ 定数C
    value = (MATCHMAKE_EVAL_CALC_RATE_STANDARD - rate) * MATCHMAKE_EVAL_CALC_WEIGHT
      - disconnect_rate * MATCHMAKE_EVAL_CALC_DISCONNECT_WEIGHT;

    value = MATH_IMax( value, 0 );
    DEBUG_NET_Printf( "▼---------------評価--------------▼\n" );
    DEBUG_NET_Printf( "自分の情報 レート%d 切断回数 %d 試合回数%d カップ%d\n", 
        p_wk->key_wk[MATCHMAKE_KEY_RATE].ivalue, p_wk->key_wk[MATCHMAKE_KEY_DISCONNECT].ivalue, 
        p_wk->key_wk[MATCHMAKE_KEY_BTLCNT].ivalue, p_wk->key_wk[MATCHMAKE_KEY_CUPNO].ivalue );
    DEBUG_NET_Printf( "相手の情報 レート%d 切断回数 %d 試合回数%d カップ%d\n", 
        you_rate, disconnect, 
        btlcnt, cup );

    DEBUG_NET_Printf( "計算結果 評価値%d レート%d 切断比率(自) %d切断比率（相）%d \n", value, rate, my_disconnect_rate, you_disconnect_rate );
    DEBUG_NET_Printf( "▲---------------------------------▲\n" );
  }
  else
  { 
    value = 0;
    DEBUG_NET_Printf( "▼評価値！ %d cupが異なった %d<>%d\n", value, p_wk->key_wk[MATCHMAKE_KEY_CUPNO].ivalue, cup );
  }

  return value;

}

//=============================================================================
/**
 *    統計・競争関係
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  レポート送信の開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param  WIFIBATTLEMATCH_BTLRULE rule              ルール
 *	@param  BtlResult result          対戦結果
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_SC_StartReport( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_SC_REPORT_TYPE type, WIFIBATTLEMATCH_TYPE mode, WIFIBATTLEMATCH_BTLRULE rule, const BATTLEMATCH_BATTLE_SCORE *cp_btl_score, BOOL is_error )
{ 
  p_wk->is_debug  = FALSE;
  p_wk->seq = 0;
  p_wk->is_auth   = TRUE;//GFL_NET_IsParentMachine();

  //タイミングシンクをするのでここでのフラグ消去はOK
  p_wk->is_recv[ WIFIBATTLEMATCH_NET_RECVFLAG_FLAG ]  = FALSE;
  p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] = FALSE;
  p_wk->report_type = type;
  p_wk->is_sc_error = is_error;
  p_wk->is_session  = FALSE;
  p_wk->is_sc_you_recv  = FALSE;
  p_wk->is_send_report  = FALSE;

  if( type == WIFIBATTLEMATCH_SC_REPORT_TYPE_BTL_AFTER )
  { 
    switch( mode )
    { 
    case WIFIBATTLEMATCH_TYPE_WIFICUP:    //WIFI大会
      p_wk->SC_CreateReportCoreFunc = DwcRap_Sc_CreateReportWifiCoreBtlAfter;
      break;
    case WIFIBATTLEMATCH_TYPE_RNDRATE:    //ランダムマッチレーティング
      p_wk->SC_CreateReportCoreFunc = DwcRap_Sc_CreateReportRndCoreBtlAfter;
      break;
    default:
      GF_ASSERT(0);
      break;
    }
  }
  else if( type == WIFIBATTLEMATCH_SC_REPORT_TYPE_BTL_SCORE )
  { 
    if( p_wk->is_sc_error )
    {
      //戦闘中にエラーしたということなので、スコア等はあげず、自分の切断カウンターのみ操作
      switch( mode )
      { 
      case WIFIBATTLEMATCH_TYPE_WIFICUP:    //WIFI大会
        p_wk->SC_CreateReportCoreFunc = DwcRap_Sc_CreateReportWifiCoreBtlError;
        break;
      case WIFIBATTLEMATCH_TYPE_RNDRATE:    //ランダムマッチレーティング
        p_wk->SC_CreateReportCoreFunc = DwcRap_Sc_CreateReportRndCoreBtlError;
        break;
      default:
        GF_ASSERT(0);
        break;
      }
    }
    else
    {
      if( GFL_NET_IsParentMachine() )
      { 
        DEBUG_NET_Printf( "！親機 ルール%d 結果%d\n", rule, cp_btl_score->result );
      }
      else
      { 
        DEBUG_NET_Printf( "！子機 ルール%d 結果%d\n", rule, cp_btl_score->result );
      }

      p_wk->report.btl_rule   = rule;
      p_wk->report.btl_result = cp_btl_score->result;
      p_wk->report.is_dirty   = cp_btl_score->is_dirty;

      switch( mode )
      { 
      case WIFIBATTLEMATCH_TYPE_WIFICUP:    //WIFI大会
        p_wk->SC_CreateReportCoreFunc = DwcRap_Sc_CreateReportWifiCore;
        break;
      case WIFIBATTLEMATCH_TYPE_RNDRATE:    //ランダムマッチレーティング
        p_wk->SC_CreateReportCoreFunc = DwcRap_Sc_CreateReportRndCore;
        break;
      default:
        GF_ASSERT(0);
        break;
      }
    }
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
  p_wk->seq           = 0;
  p_wk->is_debug      = TRUE;
  p_wk->is_auth = is_auth;
  p_wk->p_debug_data  = p_data;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レポート送信の処理中
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param  BOOL *is_send_report  レポート送信を行ったか
 *	@retval WIFIBATTLEMATCH_NET_SC_STATE列挙を参照
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_SC_STATE WIFIBATTLEMATCH_SC_ProcessReport( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL *p_is_send_report )
{ 
  DWCScResult ret;

  if( p_is_send_report )
  {
    *p_is_send_report = p_wk->is_send_report;
  }
  { 
    const GFL_NETSTATE_DWCERROR* cp_error  =  GFL_NET_StateGetWifiError();
    //マッチング相手と切断した場合
    if( cp_error->errorUser == ERRORCODE_TIMEOUT
        || cp_error->errorUser == ERRORCODE_DISCONNECT )
    { 
      DEBUG_NET_Printf( "タイムアウト!!!!\n" );
      p_wk->is_sc_error = TRUE;
    }

    //DWCレベルで切断エラーだった場合
    if( NetErr_App_CheckError() == NET_ERR_CHECK_HEAVY ) 
    {
      if( cp_error->errorType == DWC_ETYPE_DISCONNECT
          || cp_error->errorType == DWC_ETYPE_SHUTDOWN_FM )
      {
        DwcRap_Sc_Finalize( p_wk );
        return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
      }
    }

    if( p_wk->is_sc_error )
    {
      //もしエラーなのに自分がセッションを作成していなかった場合は
      //セッション作成に戻る
      //(バトル前レポート以外)
      if( p_wk->report_type != WIFIBATTLEMATCH_SC_REPORT_TYPE_BTL_AFTER &&
          p_wk->is_session == FALSE && 
           WIFIBATTLEMATCH_SC_SEQ_SESSION_WAIT < p_wk->seq && p_wk->seq < WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA )
      {

        p_wk->is_sc_you_recv  = FALSE;
        DEBUG_NET_Printf( "セッションがないので作りなおします!\n" );
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SESSION_START;
      }
    }

    switch( p_wk->seq )
    { 
    case WIFIBATTLEMATCH_SC_SEQ_SEND_TIMING_INIT:

      if( p_wk->is_sc_error )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INIT;
      }
      else
      { 
        GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_SC_DIRTY_TIMING, WB_NET_WIFIMATCH);
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_TIMING_WAIT;
      }
      break;

    case WIFIBATTLEMATCH_SC_SEQ_SEND_TIMING_WAIT:
      if( p_wk->is_sc_error)
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INIT;
      }
      else
      { 
        if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_SC_DIRTY_TIMING,WB_NET_WIFIMATCH) || p_wk->is_sc_error )
        { 
          //不正フラグを贈りあう
           
            p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_INIT;
        }
      }
      break;

    case WIFIBATTLEMATCH_SC_SEQ_SEND_INIT:
      if( p_wk->is_sc_error )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INIT;
      }
      else
      { 
        if( WIFIBATTLEMATCH_NET_SendBtlDirtyFlag( p_wk, &p_wk->report.is_dirty )
            || p_wk->is_sc_error )
        { 
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_WAIT;
        }
      }
      break;

    case WIFIBATTLEMATCH_SC_SEQ_SEND_WAIT:
      if( p_wk->is_sc_error )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INIT;
      }
      else
      { 
        if( WIFIBATTLEMATCH_NET_RecvBtlDirtyFlag( p_wk, &p_wk->report.is_dirty )
            || p_wk->is_sc_error )
        { 
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INIT;
        }
      }
      break;

    //初期化
    case WIFIBATTLEMATCH_SC_SEQ_INIT:
      p_wk->wait_cnt  = 0;
      p_wk->is_sc_start = TRUE;
      DEBUG_NET_Printf( "sc is_sc_start == true\n" );
      DEBUG_NET_Printf( "SC:sc Init\n" );
      ret = DWC_ScInitialize( GAME_ID,DWC_SSL_TYPE_SERVER_AUTH );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, ret, __LINE__ );
        DwcRap_Sc_Finalize( p_wk );
        return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
      }
      DEBUG_NET_Printf( "SC:Init end\n" );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_LOGIN;
      break;

    //認証情報取得
    case WIFIBATTLEMATCH_SC_SEQ_LOGIN:
#ifdef SC_DIVIDE_SESSION
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_START;
#else
      ret = DWC_ScGetLoginCertificate( &p_wk->p_data->sc_player[0].mCertificate );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, ret, __LINE__ );
        DwcRap_Sc_Finalize( p_wk );
        return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
      }
      DEBUG_NET_Printf( "SC:Login\n" );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SESSION_START;
#endif 
      break;

    //セッション開始  （ホストが行い、クライアントにセッションIDを伝播する）
    case WIFIBATTLEMATCH_SC_SEQ_SESSION_START:
      if( GFL_NET_IsParentMachine() || p_wk->is_sc_error )
      { 
        p_wk->wait_cnt++;
        ret = DWC_ScCreateSessionAsync( DwcRap_Sc_CreateSessionCallback, TIMEOUT_MS, p_wk );
        p_wk->is_session  = TRUE;
        if( ret != DWC_SC_RESULT_NO_ERROR )
        { 
          WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, ret, __LINE__ );
          DwcRap_Sc_Finalize( p_wk );
          return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
        }
        DEBUG_NET_Printf( "SC:Session parent\n" );
        p_wk->async_timeout = 0;
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
        GFL_STD_MemCopy( DWC_ScGetSessionId(), p_wk->p_data->sc_player[0].mSessionId, DWC_SC_SESSION_GUID_SIZE );
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_START;
      }
      else
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ ); 
          DwcRap_Sc_Finalize( p_wk );
          return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
        }

        DWC_ScProcess();
        if( p_wk->result != DWC_SC_RESULT_NO_ERROR )
        {
          WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, p_wk->result, __LINE__ );
          DwcRap_Sc_Finalize( p_wk );
          return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
        }
      }
      break;

    //ホストがクライアントへセッションIDを送るためのタイミングとり
   case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_START:
      if( p_wk->is_sc_error )
      { 
        //切れている場合はインテンションまで飛ぶ
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INTENTION_START;
      }
      else
      { 
        GFL_STD_MemCopy( DWC_ScGetConnectionId(), p_wk->p_data->sc_player[0].mConnectionId, DWC_SC_CONNECTION_GUID_SIZE );

        GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_SEND_PLAYERDATA_TIMING, WB_NET_WIFIMATCH );
        p_wk->seq  = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_WAIT;
      }
      break;

    //タイミング待ち
    case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_SEND_PLAYERDATA_TIMING, WB_NET_WIFIMATCH ) || p_wk->is_sc_error )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA;
      }
      break;

    //ホストがクライアントへ自分のデータを送る
    case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA:
      if( GFL_NET_IsParentMachine() || p_wk->is_sc_error )
      { 
        //自分を送信
        if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_NETCMD_SEND_PLAYERDATA, 
              sizeof(DWC_SC_PLAYERDATA), &p_wk->p_data->sc_player[0]) || p_wk->is_sc_error )
        { 
          DEBUG_NET_Printf( "SC:Senddata parent\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INTENTION_START;
        }
      }
      else
      { 
        //受け取り
        if( p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] || p_wk->is_sc_error )
        { 
          p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] = FALSE;
          //ホスト以外の人は、ホストがCreateSessionしたsessionIdを設定する
          ret = DWC_ScSetSessionId( p_wk->p_data->sc_player[1].mSessionId );
          GFL_STD_MemCopy( DWC_ScGetSessionId(), p_wk->p_data->sc_player[0].mSessionId, DWC_SC_SESSION_GUID_SIZE );
          if( ret != DWC_SC_RESULT_NO_ERROR )
          {
            WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, ret, __LINE__ );
            DwcRap_Sc_Finalize( p_wk );
            return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
          }
          DEBUG_NET_Printf( "SC:Senddata child\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_INTENTION_START;
        }
      }
      break;

    //送信するレポートの種類を通知
    case WIFIBATTLEMATCH_SC_SEQ_INTENTION_START:
      p_wk->wait_cnt++;
      ret = DWC_ScSetReportIntentionAsync( p_wk->is_auth, DwcRap_Sc_SetReportIntentionCallback, TIMEOUT_MS, p_wk );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, ret, __LINE__ );

        DwcRap_Sc_Finalize( p_wk );
        return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
      } 
      DEBUG_NET_Printf( "SC:intention start\n" );
      p_wk->async_timeout = 0;
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
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ ); 
          DwcRap_Sc_Finalize( p_wk );
          return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
        }

        DWC_ScProcess();
        if( p_wk->result != DWC_SC_RESULT_NO_ERROR )
        {
          WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, p_wk->result, __LINE__ );
          DwcRap_Sc_Finalize( p_wk );
          return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
        }
      }
      break;

    //クライアントの情報をホストがもらうためのタイミングとり
    case WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_START:
      if( p_wk->is_sc_error )
      { 
        //切れている場合はレポート作成まで飛ぶ
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT;
      }
      else
      { 
        GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_RETURN_PLAYERDATA_TIMING, WB_NET_WIFIMATCH );
        p_wk->seq  = WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_WAIT;
      }
      break;

    //タイミング待ち
    case WIFIBATTLEMATCH_SC_SEQ_RETURN_PLAYERDATA_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_RETURN_PLAYERDATA_TIMING, WB_NET_WIFIMATCH ) || p_wk->is_sc_error )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_PARENT;
      }
      break;

    //お互いのCCID（Intention ConnectionID）を交換する  まずは親データを子に送信
    case WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_PARENT:
      if( GFL_NET_IsParentMachine() )
      { 
        if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_NETCMD_SEND_PLAYERDATA, 
              sizeof(DWC_SC_PLAYERDATA), &p_wk->p_data->sc_player[0]) || p_wk->is_sc_error )
        { 
          DEBUG_NET_Printf( "SC:send data ccid parent\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD;
        }
      }
      else
      { 
        if( p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] || p_wk->is_sc_error )
        { 
          if( !p_wk->is_sc_error )
          {
            p_wk->is_sc_you_recv  = TRUE;
          }

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
              sizeof(DWC_SC_PLAYERDATA), &p_wk->p_data->sc_player[0]) || p_wk->is_sc_error )
        { 
          DEBUG_NET_Printf( "SC:send data ccid child\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_START;
        }
      }
      else
      { 
        if( p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] || p_wk->is_sc_error )
        { 
          if( !p_wk->is_sc_error )
          {
            p_wk->is_sc_you_recv  = TRUE;
          }

          p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_PLAYER] = FALSE;
          DEBUG_NET_Printf( "SC:send data ccid child\n" );
          p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_START;
        }
      }
      break;

    //受信後、レポート作成のためのタイミングとり
    case WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_START:
      GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_REPORT_TIMING , WB_NET_WIFIMATCH);
      p_wk->seq  = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_WAIT;
      break;

    //タイミングまち
    case WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_REPORT_TIMING, WB_NET_WIFIMATCH )  || p_wk->is_sc_error )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT;
      }
      break;

    //レポート作成
    case WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT:
      //レポート作成
#ifdef PM_DEBUG
      if( !p_wk->is_debug )
#else
      if(1)
#endif
      { 
        ret = DwcRap_Sc_CreateReport( &p_wk->p_data->sc_player[0], &p_wk->p_data->sc_player[1], &p_wk->report, p_wk->is_auth, p_wk->SC_CreateReportCoreFunc, p_wk->is_sc_error, p_wk->report_type, p_wk->is_sc_you_recv );
      }
      else
      { 
        ret = DwcRap_Sc_CreateReportDebug( &p_wk->p_data->sc_player[0], &p_wk->p_data->sc_player[1], p_wk->p_debug_data, p_wk->is_auth );
      }

      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, ret, __LINE__ );
        DwcRap_Sc_Finalize( p_wk );
        return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
      }
      DEBUG_NET_Printf( "SC:create report\n" );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_SUBMIT_REPORT_START;
      break;

    //レポート送信開始
    case WIFIBATTLEMATCH_SC_SEQ_SUBMIT_REPORT_START:
      p_wk->wait_cnt++;
      ret = DWC_ScSubmitReportAsync( p_wk->p_data->sc_player[0].mReport, p_wk->is_auth,
            DwcRap_Sc_SubmitReportCallback, TIMEOUT_MS, p_wk );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, ret, __LINE__ );
        DwcRap_Sc_Finalize( p_wk );
        return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
      }
      DEBUG_NET_Printf( "SC:submit report start\n" );
      p_wk->async_timeout = 0;
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
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ ); 
          DwcRap_Sc_Finalize( p_wk );
          return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
        }

        DWC_ScProcess();
        if( p_wk->result != DWC_SC_RESULT_NO_ERROR )
        {
          WIFIBATTLEMATCH_NETERR_SetScError( &p_wk->error, p_wk->result, __LINE__ );
          DwcRap_Sc_Finalize( p_wk );
          return WIFIBATTLEMATCH_NET_SC_STATE_FAILED;
        }
      }
      break;

    //終了！
    case WIFIBATTLEMATCH_SC_SEQ_EXIT:
#ifdef PM_DEBUG
      if( *DEBUGWIN_REPORTMARK_GetFlag() )
      {
        PMSND_PlaySE( WBM_SND_SE_MATCHING_OK );
      }
#endif //PM_DEBUG

      p_wk->is_send_report  = TRUE;
      DEBUG_NET_Printf( "SC:exit\n" );
      DwcRap_Sc_Finalize( p_wk );
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_END_TIMING_START;
      break;

    case WIFIBATTLEMATCH_SC_SEQ_END_TIMING_START:
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_SC_END_TIMING, WB_NET_WIFIMATCH);
      p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_END_TIMING_WAIT;
      break;
    case WIFIBATTLEMATCH_SC_SEQ_END_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),WIFIBATTLEMATCH_SC_END_TIMING,WB_NET_WIFIMATCH) || p_wk->is_sc_error )
      { 
        p_wk->seq = WIFIBATTLEMATCH_SC_SEQ_END;
      }
      break;
    case WIFIBATTLEMATCH_SC_SEQ_END:
      return WIFIBATTLEMATCH_NET_SC_STATE_SUCCESS;
    }
  }

  return WIFIBATTLEMATCH_NET_SC_STATE_UPDATE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レポート中に受け取った不正フラグを取得
 *
 *	@param	const WIFIBATTLEMATCH_NET_WORK *cp_wk   ワーク
 *
 *	@return 不正フラグ
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_SC_GetDirtyFlag( const WIFIBATTLEMATCH_NET_WORK *cp_wk )
{
  return cp_wk->recv_flag;
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

#ifdef PM_DEBUG
#if 0
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    theResult = DWC_SC_RESULT_HTTP_ERROR;
  }
#endif
#endif 

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
  DWC_SC_PLAYERDATA   *p_player_data  = &p_wk->p_data->sc_player[0];

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
  if( p_wk->is_sc_start == TRUE )
  {
    if( p_wk->seq > WIFIBATTLEMATCH_SC_SEQ_CREATE_REPORT )
    {
      DEBUG_NET_Printf( "destroy report\n" );
      DWC_ScDestroyReport( p_wk->p_data->sc_player[0].mReport );
    }

    if( p_wk->seq >= WIFIBATTLEMATCH_SC_SEQ_INIT )
    {
      DEBUG_NET_Printf( "sc shutdown\n" );
      DWC_ScShutdown();
    }

    p_wk->is_sc_start = FALSE;
    DEBUG_NET_Printf( "sc is_sc_start == false\n" );
  }
  DEBUG_NET_Printf( "sc end shutdown\n" );
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
static DWCScResult DwcRap_Sc_CreateReport( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const DWC_SC_WRITE_DATA *cp_data, BOOL is_auth, DWCRAP_SC_CREATE_REPORT_CORE_FUNC SC_CreateReportCoreFunc, BOOL is_disconnect, WIFIBATTLEMATCH_SC_REPORT_TYPE report_type, BOOL is_sc_you_recv )
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

    if( is_disconnect || report_type == WIFIBATTLEMATCH_SC_REPORT_TYPE_BTL_AFTER )
    {
      game_result = DWC_SC_GAME_RESULT_NONE;
    }
    else
    {
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

#ifdef PM_DEBUG
      if( *DEBUGWIN_ATLASDIRTY_GetFlag() )
      {
        if( game_result == DWC_SC_GAME_RESULT_LOSS )
        {
          game_result = DWC_SC_GAME_RESULT_WIN;
        }
        else if( game_result == DWC_SC_GAME_RESULT_WIN )
        {
          game_result = DWC_SC_GAME_RESULT_LOSS;
        }
      }
#endif //PM_DEBUG

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
  ret = SC_CreateReportCoreFunc( p_my, cp_data, TRUE, is_disconnect );
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

  if( is_sc_you_recv == TRUE )
  { 
    DWCScGameResult game_result;

    if( report_type == WIFIBATTLEMATCH_SC_REPORT_TYPE_BTL_AFTER )
    { 
      game_result = DWC_SC_GAME_RESULT_NONE;
    }
    else if( is_disconnect )
    {
      game_result = DWC_SC_GAME_RESULT_DISCONNECT;
    }
    else
    {
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
    }

#ifdef PM_DEBUG
      if( *DEBUGWIN_ATLASDIRTY_GetFlag() )
      {
        if( game_result == DWC_SC_GAME_RESULT_LOSS )
        {
          game_result = DWC_SC_GAME_RESULT_WIN;
        }
        else if( game_result == DWC_SC_GAME_RESULT_WIN )
        {
          game_result = DWC_SC_GAME_RESULT_LOSS;
        }
      }
#endif //PM_DEBUG

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

    //成績書き込み
    ret = SC_CreateReportCoreFunc( p_my, cp_data, FALSE, is_disconnect );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      DEBUG_NET_Printf( "Report:Core%d\n",ret );
      return ret;
    }
  }

  // レポートにチームデータを書き込む
  ret = DWC_ScReportBeginTeamData( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:Team%d\n",ret );
    return ret;
  }

  //　レポートの終端を宣言する
  if( is_disconnect )
  {
    ret = DWC_ScReportEnd( p_my->mReport, is_auth, DWC_SC_GAME_STATUS_PARTIAL );
  }
  else
  { 
    ret = DWC_ScReportEnd( p_my->mReport, is_auth, DWC_SC_GAME_STATUS_COMPLETE );
  }
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
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SINGLE_WIN_COUNTER, cp_data->you_single_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
      return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SINGLE_LOSE_COUNTER, cp_data->you_single_lose );
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
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_DOUBLE_WIN_COUNTER, cp_data->you_double_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_DOUBLE_LOSE_COUNTER, cp_data->you_double_lose );
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
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_TRIPLE_WIN_COUNTER, cp_data->you_triple_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_TRIPLE_LOSE_COUNTER, cp_data->you_triple_lose );
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
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_ROTATE_WIN_COUNTER, cp_data->you_rot_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_ROTATE_LOSE_COUNTER, cp_data->you_rot_lose );
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
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SHOOTER_WIN_COUNTER, cp_data->you_shooter_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SHOOTER_LOSE_COUNTER, cp_data->you_shooter_lose );
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
static DWCScResult DwcRap_Sc_CreateReportRndCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect )
{ 
  const BtlResult result  = cp_data->btl_result;
  BOOL is_win = FALSE;
  BOOL is_lose  = FALSE;
  BOOL is_inverse = FALSE;
  DWCScResult ret;

  switch( cp_data->btl_result )
  {
  case BTL_RESULT_RUN:         ///< 逃げた
  case BTL_RESULT_LOSE:        ///< 負けた
    is_win  = FALSE;
    is_lose = TRUE;
    is_inverse  = TRUE;
    break;
  case BTL_RESULT_RUN_ENEMY:   ///< 相手が逃げた
  case BTL_RESULT_WIN:         ///< 勝った
    is_win  = TRUE;
    is_lose = FALSE;
    is_inverse  = TRUE;
    break;
  case BTL_RESULT_DRAW:        ///< ひきわけ
    is_win  = FALSE;
    is_lose = FALSE;
    break;
  case BTL_RESULT_CAPTURE:     ///< 捕まえた（野生のみ）
  default:
    GF_ASSERT( 0 );
  case BTL_RESULT_COMM_ERROR:  ///< 通信エラーによる
    is_win  = FALSE;
    is_lose = FALSE;
    break;
  }

#ifdef PM_DEBUG
  if( *DEBUGWIN_ATLASDIRTY_GetFlag() )
  {
    if( is_inverse )
    {
      is_win  ^= 1;
      is_lose ^= 1;
    }
  }
#endif //PM_DEBUG

  //もし相手の情報を書き込むならば、結果を反対にする
  if( !is_my && is_inverse )
  {
    is_win  ^= 1;
    is_lose ^= 1;
  }

  DEBUGWIN_REPORT_SetData( is_my, is_win, is_lose, cp_data->is_dirty != 0, -1  );

  DEBUG_NET_Printf( "-----------Report_start-------------\n" );
  DEBUG_NET_Printf( "レポート先%d\n", is_my );
  DEBUG_NET_Printf( "勝ち%d\n", is_win );
  DEBUG_NET_Printf( "負け%d\n", is_lose );
  DEBUG_NET_Printf( "不正%d\n", cp_data->is_dirty );
  DEBUG_NET_Printf( "-----------Report_end-------------\n" );

  //ルールによって送るものが違う
  switch( cp_data->btl_rule )
  { 
  case WIFIBATTLEMATCH_BTLRULE_SINGLE:    ///< シングル
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_SINGLE, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SINGLE_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SINGLE_LOSE_COUNTER, is_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  case WIFIBATTLEMATCH_BTLRULE_DOUBLE:    ///< ダブル
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_DOUBLE, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_DOUBLE_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_DOUBLE_LOSE_COUNTER, is_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  case WIFIBATTLEMATCH_BTLRULE_TRIPLE:    ///< トリプル
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_TRIPLE, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_TRIPLE_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_TRIPLE_LOSE_COUNTER, is_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  case WIFIBATTLEMATCH_BTLRULE_ROTATE:  ///< ローテーション
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_ROTATE, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_ROTATE_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_ROTATE_LOSE_COUNTER, is_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  case WIFIBATTLEMATCH_BTLRULE_SHOOTER:  ///< シューター
    ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_SHOOTER, 1 );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SHOOTER_WIN_COUNTER, is_win );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SHOOTER_LOSE_COUNTER, is_lose );
    if( ret != DWC_SC_RESULT_NO_ERROR )
    { 
      return ret;
    }
    break;

  default:
    GF_ASSERT(0);
  }

  //共通の送るもの
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_CHEATS_COUNTER, cp_data->is_dirty != 0 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_ADD_DISCONNECTS_COUNTER, -1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
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
static DWCScResult DwcRap_Sc_CreateReportWifiCore( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect )
{ 
  const BtlResult result  = cp_data->btl_result;
  BOOL is_win = FALSE;
  BOOL is_lose  = FALSE;
  BOOL is_inverse = FALSE;
  DWCScResult ret;

  switch( cp_data->btl_result )
  {
  case BTL_RESULT_RUN:         ///< 逃げた
  case BTL_RESULT_LOSE:        ///< 負けた
    is_win  = FALSE;
    is_lose = TRUE;
    is_inverse = TRUE;
    break;
  case BTL_RESULT_RUN_ENEMY:   ///< 相手が逃げた
  case BTL_RESULT_WIN:         ///< 勝った
    is_win  = TRUE;
    is_lose = FALSE;
    is_inverse = TRUE;
    break;
  case BTL_RESULT_DRAW:        ///< ひきわけ
    is_win  = FALSE;
    is_lose = FALSE;
    break;
  case BTL_RESULT_CAPTURE:     ///< 捕まえた（野生のみ）
  default:
    GF_ASSERT( 0 );
  case BTL_RESULT_COMM_ERROR:  ///< 通信エラーによる
    is_win  = FALSE;
    is_lose = FALSE;
    break;
  }


#ifdef PM_DEBUG
  if( *DEBUGWIN_ATLASDIRTY_GetFlag() )
  {
    if( is_inverse )
    {
      is_win  ^= 1;
      is_lose ^= 1;
    }
  }
#endif //PM_DEBUG

  //もし相手の勝利情報を書き込むならば、結果を反対にする
  if( !is_my && is_inverse )
  {
    is_win  ^= 1;
    is_lose ^= 1;
  }


  DEBUGWIN_REPORT_SetData( is_my, is_win, is_lose, cp_data->is_dirty != 0, -1  );

  DEBUG_NET_Printf( "-----------Report_start-------------\n" );
  DEBUG_NET_Printf( "レポート先%d\n", is_my );
  DEBUG_NET_Printf( "勝ち%d\n", is_win );
  DEBUG_NET_Printf( "負け%d\n", is_lose );
  DEBUG_NET_Printf( "不正%d\n", cp_data->is_dirty );
  DEBUG_NET_Printf( "-----------Report_end-------------\n" );

  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_WIFICUP, 1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_WIFICUP_WIN_COUNTER, is_win );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_WIFICUP_LOSE_COUNTER, is_lose );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_CHEATS_WIFICUP_COUNTER, cp_data->is_dirty != 0 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_ADD_DISCONNECTS_WIFICUP_COUNTER, -1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  {
    return ret;
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レポート作成  ランダムマッチバトル前版
 *
 *	@param	DWC_SC_PLAYERDATA *p_my           書き込むレポートの持ち主
 *	@param  const DWC_SC_WRITE_DATA *cp_data  書き込むデータ
 *	@param	is_my                             TRUEならば自分、FALSEならば相手
 *
 *	@return 書き込み結果
 */
//-----------------------------------------------------------------------------
static DWCScResult DwcRap_Sc_CreateReportRndCoreBtlAfter( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect )
{ 
  DWCScResult ret;
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_DISCONNECTS_COUNTER, 1 );
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レポート作成  WIFI大会バトル前版
 *
 *	@param	DWC_SC_PLAYERDATA *p_my           書き込むレポートの持ち主
 *	@param  const DWC_SC_WRITE_DATA *cp_data  書き込むデータ
 *	@param	is_my                             TRUEならば自分、FALSEならば相手
 *
 *	@return 書き込み結果
 */
//-----------------------------------------------------------------------------
static DWCScResult DwcRap_Sc_CreateReportWifiCoreBtlAfter( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect )
{ 
  DWCScResult ret;
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_DISCONNECTS_WIFICUP_COUNTER, 1 );
  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レポート作成  ランダムマッチバトル切断エラー版
 *
 *	@param	DWC_SC_PLAYERDATA *p_my           書き込むレポートの持ち主
 *	@param  const DWC_SC_WRITE_DATA *cp_data  書き込むデータ
 *	@param	is_my                             TRUEならば自分、FALSEならば相手
 *
 *	@return 書き込み結果
 */
//-----------------------------------------------------------------------------
static DWCScResult DwcRap_Sc_CreateReportRndCoreBtlError( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect )
{
  DWCScResult ret;

  DEBUGWIN_REPORT_SetData( is_my, 0, 0, 0, -1  );

  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_ADD_DISCONNECTS_COUNTER, -1 );
  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  レポート作成  WIFI大会バトル切断エラー版
 *
 *	@param	DWC_SC_PLAYERDATA *p_my           書き込むレポートの持ち主
 *	@param  const DWC_SC_WRITE_DATA *cp_data  書き込むデータ
 *	@param	is_my                             TRUEならば自分、FALSEならば相手
 *
 *	@return 書き込み結果
 */
//-----------------------------------------------------------------------------
static DWCScResult DwcRap_Sc_CreateReportWifiCoreBtlError( DWC_SC_PLAYERDATA *p_my, const DWC_SC_WRITE_DATA *cp_data, BOOL is_my, BOOL is_disconnect )
{
  DWCScResult ret;

  DEBUGWIN_REPORT_SetData( is_my, 0, 0, 0, -1  );
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_ADD_DISCONNECTS_WIFICUP_COUNTER, -1 );
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

  GFL_STD_MemCopy( cp_data_adrs, &p_wk->p_data->sc_player[1], sizeof(DWC_SC_PLAYERDATA));

  DEBUG_NET_Printf( "[Other]Session ID: %s\n", p_wk->p_data->sc_player[1].mSessionId );
  DEBUG_NET_Printf( "[Other]Connection ID: %s\n", p_wk->p_data->sc_player[1].mConnectionId );
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
  const WIFIBATTLEMATCH_ENEMYDATA *cp_recv = cp_data_adrs;

  //相手と自分のマイステータスをゲームデータに格納する
  MYSTATUS *p_mystatus  = GAMEDATA_GetMyStatusPlayer( p_wk->p_gamedata, netID );
  GFL_STD_MemCopy( cp_recv->mystatus, p_mystatus, MyStatus_GetWorkSize() );
  DEBUG_NET_Printf( "MyStatusをセット！netID=%d size=%d \n", netID, size );

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
static u8* WifiBattleMatch_RecvBuffAddr(int netID, void* p_wk_adrs, int size)
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_wk_adrs;

  GF_ASSERT_MSG( size < RECV_BUFFER_SIZE, "recv_buff error!! size = 0x%x <> buff= 0x%x\n", size, RECV_BUFFER_SIZE );

  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    GF_ASSERT( 0 );
    return NULL;//自分のは受け取らない
  }
  return p_wk->recv_buffer;
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

//----------------------------------------------------------------------------
/**
 *	@brief  ポケパーティ受信コールバック
 *
 *	@param	int netID     ネットID
 *	@param	int size      サイズ
 *	@param	void* pData   データ
 *	@param	pWk           自分であたえたアドレス
 *	@param	pNetHandle    ネットハンドル
 */
//-----------------------------------------------------------------------------
static void WifiBattleMatch_RecvPokeParty(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle)
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

  DEBUG_NET_Printf( "ポケパーティ受信コールバック！netID=%d size=%d \n", netID, size );

  p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_POKEPARTY] = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  不正カウンタ受信コールバック
 *
 *	@param	int netID     ネットID
 *	@param	int size      サイズ
 *	@param	void* pData   データ
 *	@param	pWk           自分であたえたアドレス
 *	@param	pNetHandle    ネットハンドル
 */
//-----------------------------------------------------------------------------
static void WifiBattleMatch_RecvDirtyCnt(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle)
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

  DEBUG_NET_Printf( "不正カウンタ受信コールバック！netID=%d size=%d \n", netID, size );

  p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_DIRTYCNT] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フラグ受信コールバック
 *
 *	@param	int netID           ネットID
 *	@param	int size            サイズ
 *	@param	void* cp_data_adrs  データ
 *	@param	p_wk_adrs           自分で与えたアドレス
 *	@param	p_net_handle        ネットハンドル
 */
//-----------------------------------------------------------------------------
static void WifiBattleMatch_RecvFlag(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle)
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_wk_adrs;
  u32 data  = *(u32*)cp_data_adrs;

  if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
  {
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    return;//自分のは今は受け取らない
  }

  DEBUG_NET_Printf( "フラグ受信コールバック！netID=%d size=%d \n", netID, size, data );

  p_wk->recv_flag = data;
  p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_FLAG] = TRUE;
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
void WIFIBATTLEMATCH_GDB_Start( WIFIBATTLEMATCH_NET_WORK *p_wk, int recordID, WIFIBATTLEMATCH_GDB_GETTYPE type, void *p_wk_adrs )
{ 
  p_wk->seq           = 0;
  p_wk->p_get_wk      = p_wk_adrs;
  p_wk->get_recordID = recordID;
  DEBUG_NET_Printf( "GDB:request[%d]\n",type );


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

  case WIFIBATTLEMATCH_GDB_GET_PARTY:
    { 
      static const char *sc_get_pokemon_tbl[]  =
      { 
        SAKE_STAT_WIFICUP_POKEMON_PARTY
      };

      p_wk->pp_table_name  = sc_get_pokemon_tbl;
      p_wk->table_name_num= NELEMS( sc_get_pokemon_tbl );
      p_wk->gdb_get_record_callback = DwcRap_Gdb_PokeParty_GetRecordsCallback;
    }
    break;

  case WIFIBATTLEMATCH_GDB_GET_LOGIN_DATE:
    { 
      static const char *sc_get_datetime_tbl[]  =
      { 
        SAKE_STAT_LAST_LOGIN_DATETIME
      };

      p_wk->pp_table_name  = sc_get_datetime_tbl;
      p_wk->table_name_num= NELEMS( sc_get_datetime_tbl );
      p_wk->gdb_get_record_callback = DwcRap_Gdb_LoginDate_GetRecordsCallback;
    }
    break;
#ifdef PM_DEBUG
  case WIFIBATTLEMATCH_GDB_GET_DEBUGALL:
    {
      p_wk->pp_table_name  = sc_get_debugall_tbl;
      p_wk->table_name_num= NELEMS( sc_get_debugall_tbl );
      p_wk->gdb_get_record_callback = DwcRap_Gdb_SakeAll_GetRecordsCallback;
    }
    break;
#endif //PM_DEBUG
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
WIFIBATTLEMATCH_GDB_RESULT WIFIBATTLEMATCH_GDB_Process( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  DWCGdbError error;
  DWCGdbState state;

  { 
    switch( p_wk->seq )
    { 
    case WIFIBATTLEMATCH_GDB_SEQ_INIT:
      error = DWC_GdbInitialize( GAME_ID, p_wk->cp_user_data, WIFIBATTLEMATCH_NET_SSL_TYPE );
      if( error != DWC_GDB_ERROR_NONE )
      { 
        WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ );
        DwcRap_Gdb_Finalize( p_wk );
        return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
      }
      DEBUG_NET_Printf( "GDB:Init\n" );
      p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_GET_START;
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_GET_START:
      GF_ASSERT( p_wk->table_name_num < WBM_GDB_FIELD_TABLE_MAX );
      if( p_wk->get_recordID == WIFIBATTLEMATCH_GDB_MYRECORD )
      { 
        //自分のレコード取得の場合
        error = DWC_GdbGetMyRecordsAsync( WIFIBATTLEMATCH_NET_TABLENAME, p_wk->pp_table_name, p_wk->table_name_num, DwcRap_Gdb_GetCallback, p_wk );
      }
      else
      { 
        //レコードID指定して取得の場合
        error = DWC_GdbGetRecordsAsync( WIFIBATTLEMATCH_NET_TABLENAME,
                                   &p_wk->get_recordID,
                                   1,
                                   p_wk->pp_table_name, 
                                   p_wk->table_name_num, 
                                   DwcRap_Gdb_GetCallback, p_wk);
      }
      if( error != DWC_GDB_ERROR_NONE )
      { 
        WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ );
        DwcRap_Gdb_Finalize( p_wk );
        return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
      }
      DEBUG_NET_Printf( "GDB:Get start\n" );
      p_wk->seq= WIFIBATTLEMATCH_GDB_SEQ_GET_WAIT;
      p_wk->async_timeout = 0;
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_GET_WAIT:
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ ); 
          //非同期処理は開始されているので、
          //キャンセルを行なう
          p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_CANCEL_START;
          break;
        }

        DWC_GdbProcess();

        state = DWC_GdbGetState();
        if( state == DWC_GDB_STATE_UNINITIALIZED || 
            state == DWC_GDB_STATE_ERROR_OCCURED )
        { 
          //非同期処理は開始されているので、
          //キャンセルを行なう
          WIFIBATTLEMATCH_NETERR_SetGdbState( &p_wk->error, state, __LINE__ );
          p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_CANCEL_START;
          break;
        }
        
        if( state == DWC_GDB_STATE_IDLE )
        { 
          DEBUG_NET_Printf( "GDB:Get wait\n" );
          p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_GET_END;
        }
      }
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_GET_END:
      { 
        DWCGdbAsyncResult ret;
        ret = DWC_GdbGetAsyncResult();
        if( ret == DWC_GDB_ASYNC_RESULT_NONE )
        {
          WIFIBATTLEMATCH_NETERR_SetGdbResult( &p_wk->error, ret, __LINE__ );
          p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_CANCEL_START;
          break;
        }
        else if( ret != DWC_GDB_ASYNC_RESULT_SUCCESS )
        { 
          WIFIBATTLEMATCH_NETERR_SetGdbResult( &p_wk->error, ret, __LINE__ );
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
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
      return WIFIBATTLEMATCH_GDB_RESULT_SUCCESS;


    case WIFIBATTLEMATCH_GDB_SEQ_CANCEL_START:
      {

        error = DWC_GdbCancelRequest();
        DEBUG_NET_Printf( "GDB:Start Cancel\n" );
        if( error == DWC_GDB_ERROR_NONE )
        {
          p_wk->async_timeout = 0;
          p_wk->seq= WIFIBATTLEMATCH_GDB_SEQ_CANCEL_WAIT;
        }
        else
        {
          WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ );
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }
      }
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_CANCEL_WAIT:
      DEBUG_NET_Printf( "GDB:Wait Cancel\n" );
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ ); 
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }

        DWC_GdbProcess();

        state = DWC_GdbGetState();
        if( state == DWC_GDB_STATE_UNINITIALIZED || 
            state == DWC_GDB_STATE_ERROR_OCCURED )
        { 
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }
        
        if( state == DWC_GDB_STATE_IDLE )
        { 
          p_wk->seq = WIFIBATTLEMATCH_GDB_SEQ_CANCEL_END;
        }
      }
      break;

    case WIFIBATTLEMATCH_GDB_SEQ_CANCEL_END:
      DwcRap_Gdb_Finalize( p_wk );
      return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
    }
  }

  return WIFIBATTLEMATCH_GDB_RESULT_UPDATE;
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
  DEBUG_NET_Printf( "GDB_ShotDown finalize !\n" );
  DWC_GdbShutdown();           // 簡易データベースライブラリの終了
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を設定フィールドに追加
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_SetMyInfo( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  MYSTATUS  *p_mystatus = GAMEDATA_GetMyStatus(p_wk->p_gamedata);

  p_wk->p_field_buff[0].name  = SAKE_STAT_MYSTATUS;
  p_wk->p_field_buff[0].type  = DWC_GDB_FIELD_TYPE_BINARY_DATA;
  p_wk->p_field_buff[0].value.binary_data.data = (u8*)p_mystatus;
  p_wk->p_field_buff[0].value.binary_data.size = MYSTATUS_SAVE_SIZE;

  WifiBattleMatch_SetDateTime( &p_wk->datetime );
  p_wk->p_field_buff[1].name  = SAKE_STAT_LAST_LOGIN_DATETIME;
  p_wk->p_field_buff[1].type  = DWC_GDB_FIELD_TYPE_BINARY_DATA;
  p_wk->p_field_buff[1].value.binary_data.data = (u8*)&p_wk->datetime;
  p_wk->p_field_buff[1].value.binary_data.size = sizeof(WBM_NET_DATETIME);

  p_wk->p_field_buff[2].name  = SAKE_STAT_INITIAL_PROFILE_ID;
  p_wk->p_field_buff[2].type  = DWC_GDB_FIELD_TYPE_INT;
  p_wk->p_field_buff[2].value.int_s32 = p_wk->pid;

  p_wk->p_field_buff[3].name  = SAKE_STAT_NOW_PROFILE_ID;
  p_wk->p_field_buff[3].type  = DWC_GDB_FIELD_TYPE_INT;
  p_wk->p_field_buff[3].value.int_s32 = p_wk->cp_user_data->gs_profile_id;
  
  p_wk->table_name_num  = 4;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レコード取得コールバック  大元
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_GetCallback(int record_num, DWCGdbField** records, void* user_param)
{ 
  WIFIBATTLEMATCH_NET_WORK *p_wk  = user_param;

  //設定したコールバックを呼ぶ
  p_wk->gdb_get_record_callback( record_num, records, p_wk->p_get_wk, p_wk->table_name_num );

  //自分のレコードからの取得の場合で、レコードIDの受信があれば、
  //格納
  if( p_wk->get_recordID == WIFIBATTLEMATCH_GDB_MYRECORD )
  { 
    int i,j;

    for (i = 0; i < record_num; i++)
    {
      for (j = 0; j < p_wk->table_name_num; j++)   // user_param -> field_num
      {
        DWCGdbField* field  = &records[i][j];

        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORDID ) )
        { 
          p_wk->p_data->sake_recordID  = field->value.int_s32;
          DEBUG_NET_Printf("recordID取得 %d!\n", p_wk->p_data->sake_recordID );
        }
      }
    }
  }
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
static void DwcRap_Gdb_Rnd_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num )
{
    int i,j;
    WIFIBATTLEMATCH_GDB_RND_SCORE_DATA *p_data = user_param;
//    GFL_STD_MemClear( p_data, sizeof(WIFIBATTLEMATCH_GDB_RND_SCORE_DATA) );
//    エラー時コールバックがまわると0クリアしてしまう

    for (i = 0; i < record_num; i++)
    {
      DEBUG_NET_Printf("!!!=====gdb_Print:======\n");
      for (j = 0; j < field_num; j++)   // user_param -> field_num
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
        else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_SAVE_IDX ) )
        { 
          p_data->record_save_idx  = field->value.int_s32;
        }
        else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORDID ) )
        { 
          p_data->recordID  = field->value.int_s32;
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
static void DwcRap_Gdb_Wifi_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num )
{
    int i,j;
    WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA *p_data = user_param;
//    GFL_STD_MemClear( p_data, sizeof(WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA) );
//    エラー時コールバックが回ると0クリアしてしまう

    DEBUG_NET_Printf("!!!=====gdb_Print[%d]:======\n", record_num);
    for (i = 0; i < record_num; i++)
    {
  
      for (j = 0; j < field_num; j++)   // user_param -> field_num
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
        else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_SAVE_IDX ) )
        { 
          p_data->record_save_idx  = field->value.int_s32;
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
static void DwcRap_Gdb_RecordID_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num)
{ 
    int i,j;
    int *p_data = user_param;

      DEBUG_NET_Printf("!!!=====gdb_Print[%d]:======\n", record_num );
    for (i = 0; i < record_num; i++)
    {
      for (j = 0; j < field_num; j++)   // user_param -> field_num
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
//----------------------------------------------------------------------------
/**
 *	@brief  レコード取得コールバック  ポケモンパーティのみ
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_PokeParty_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num )
{ 
    int i,j;
    void *p_data = user_param;

    for (i = 0; i < record_num; i++)
    {
      DEBUG_NET_Printf("!!!=====gdb_Print:======\n" );
      for (j = 0; j < field_num; j++)   // user_param -> field_num
      {
        DWCGdbField* field  = &records[i][j];

        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_WIFICUP_POKEMON_PARTY ) )
        { 
          GFL_STD_MemCopy( field->value.binary_data.data, p_data, field->value.binary_data.size );
        }
        print_field( field );
        DEBUG_NET_Printf(" ");
      }
      DEBUG_NET_Printf("!!!====================\n");
    }
}
//----------------------------------------------------------------------------
/**
 *	@brief  レコード取得コールバック  ログイン時間のみ
 *
 *	@param	record_num  レコード数
 *	@param	records     レコードのアドレス
 *	@param	user_param  自分で設定したワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_LoginDate_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num)
{ 
    int i,j;
    WBM_NET_DATETIME *p_data = user_param;

    for (i = 0; i < record_num; i++)
    {
      DEBUG_NET_Printf("!!!=====gdb_Print:======\n" );
      for (j = 0; j < field_num; j++)   // user_param -> field_num
      {
        DWCGdbField* field  = &records[i][j];

        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_LAST_LOGIN_DATETIME ) )
        { 
          GFL_STD_MemCopy( field->value.binary_data.data, p_data, field->value.binary_data.size );
        }
        print_field( field );
        DEBUG_NET_Printf(" ");
      }
      DEBUG_NET_Printf("!!!====================\n");
    }
}


//----------------------------------------------------------------------------
/**
 *	@brief  サケの全データ取得コールバック
 *
 *	@param	record_num
 *	@param	records
 *	@param	user_param
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_SakeAll_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param, int field_num)
{
    int i,j;
    WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *p_data = user_param;
    GFL_STD_MemClear( p_data, sizeof(WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA) );

    DwcRap_Gdb_LoginDate_GetRecordsCallback( record_num, records, &p_data->datetime, field_num );
    DwcRap_Gdb_Wifi_GetRecordsCallback( record_num, records, &p_data->wifi, field_num );
    DwcRap_Gdb_Rnd_GetRecordsCallback( record_num, records, &p_data->rnd, field_num );

    //大会履歴取得
    for (i = 0; i < record_num; i++)
    {
      DEBUG_NET_Printf("!!!=====gdb_Print:======\n" );
      for (j = 0; j < field_num; j++)   // user_param -> field_num
      {
        DWCGdbField* field  = &records[i][j];
        
        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_MYSTATUS ) )
        {
          GFL_STD_MemCopy( field->value.binary_data.data, &p_data->mystatus, field->value.binary_data.size );
        }
        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_DATA_01 ) )
        { 
          GFL_STD_MemCopy( field->value.binary_data.data, &p_data->record_data[0], field->value.binary_data.size );
        }
        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_DATA_02 ) )
        { 
          GFL_STD_MemCopy( field->value.binary_data.data, &p_data->record_data[1], field->value.binary_data.size );
        }
        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_DATA_03 ) )
        { 
          GFL_STD_MemCopy( field->value.binary_data.data, &p_data->record_data[2], field->value.binary_data.size );
        }
        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_DATA_04 ) )
        { 
          GFL_STD_MemCopy( field->value.binary_data.data, &p_data->record_data[3], field->value.binary_data.size );
        }
        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_DATA_05 ) )
        { 
          GFL_STD_MemCopy( field->value.binary_data.data, &p_data->record_data[4], field->value.binary_data.size );
        }
        if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_DATA_06 ) )
        { 
          GFL_STD_MemCopy( field->value.binary_data.data, &p_data->record_data[5], field->value.binary_data.size );
        }
        else if( !GFL_STD_StrCmp( field->name, SAKE_STAT_RECORD_SAVE_IDX ) )
        { 
          p_data->record_save_idx  = field->value.int_s32;
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

  p_wk->seq             = 0;
  DEBUG_NET_Printf( "GDBW: request[%d]\n", type );
  GF_ASSERT( p_wk->p_data->sake_recordID != 0 );

  switch( type )
  { 
#ifdef PM_DEBUG
  case WIFIBATTLEMATCH_GDB_WRITE_DEBUGALL:
    { 
      int i;
      const WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *cp_data  = cp_wk_adrs;
      p_wk->table_name_num  = 33;

      //ダブルレート
      p_wk->p_field_buff[0].name          = (char*)sc_get_debugall_tbl[0];
      p_wk->p_field_buff[0].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[0].value.int_s32 = cp_data->rnd.double_rate;
      //ローテーションレート
      p_wk->p_field_buff[1].name          = (char*)sc_get_debugall_tbl[1];
      p_wk->p_field_buff[1].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[1].value.int_s32 = cp_data->rnd.rot_rate;
      //シューターレート
      p_wk->p_field_buff[2].name          = (char*)sc_get_debugall_tbl[2];
      p_wk->p_field_buff[2].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[2].value.int_s32 = cp_data->rnd.shooter_rate;
      //シングルレート
      p_wk->p_field_buff[3].name          = (char*)sc_get_debugall_tbl[3];
      p_wk->p_field_buff[3].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[3].value.int_s32 = cp_data->rnd.single_rate;
      //トリプルレート
      p_wk->p_field_buff[4].name          = (char*)sc_get_debugall_tbl[4];
      p_wk->p_field_buff[4].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[4].value.int_s32 = cp_data->rnd.triple_rate;
      //チート
      p_wk->p_field_buff[5].name          = (char*)sc_get_debugall_tbl[5];
      p_wk->p_field_buff[5].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[5].value.int_s32 = cp_data->rnd.cheat;
      //マッチ
      p_wk->p_field_buff[6].name          = (char*)sc_get_debugall_tbl[6];
      p_wk->p_field_buff[6].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[6].value.int_s32 = cp_data->rnd.complete;
      //切断
      p_wk->p_field_buff[7].name          = (char*)sc_get_debugall_tbl[7];
      p_wk->p_field_buff[7].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[7].value.int_s32 = cp_data->rnd.disconnect;
      //ダブル負け
      p_wk->p_field_buff[8].name          = (char*)sc_get_debugall_tbl[8];
      p_wk->p_field_buff[8].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[8].value.int_s32 = cp_data->rnd.double_lose;
      //ダブル勝ち
      p_wk->p_field_buff[9].name          = (char*)sc_get_debugall_tbl[9];
      p_wk->p_field_buff[9].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[9].value.int_s32 = cp_data->rnd.double_win;
      //ロット負け
      p_wk->p_field_buff[10].name          = (char*)sc_get_debugall_tbl[10];
      p_wk->p_field_buff[10].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[10].value.int_s32 = cp_data->rnd.rot_lose;
      //ロット勝ち
      p_wk->p_field_buff[11].name          = (char*)sc_get_debugall_tbl[11];
      p_wk->p_field_buff[11].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[11].value.int_s32 = cp_data->rnd.rot_win;
      //シューター負け
      p_wk->p_field_buff[12].name          = (char*)sc_get_debugall_tbl[12];
      p_wk->p_field_buff[12].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[12].value.int_s32 = cp_data->rnd.shooter_lose;
      //シューター勝ち
      p_wk->p_field_buff[13].name          = (char*)sc_get_debugall_tbl[13];
      p_wk->p_field_buff[13].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[13].value.int_s32 = cp_data->rnd.shooter_win;
      //シングル負け
      p_wk->p_field_buff[14].name          = (char*)sc_get_debugall_tbl[14];
      p_wk->p_field_buff[14].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[14].value.int_s32 = cp_data->rnd.single_lose;
      //シングル勝ち
      p_wk->p_field_buff[15].name          = (char*)sc_get_debugall_tbl[15];
      p_wk->p_field_buff[15].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[15].value.int_s32 = cp_data->rnd.single_win;
      //トリプル負け
      p_wk->p_field_buff[16].name          = (char*)sc_get_debugall_tbl[16];
      p_wk->p_field_buff[16].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[16].value.int_s32 = cp_data->rnd.triple_lose;
      //とるぷる勝ち
      p_wk->p_field_buff[17].name          = (char*)sc_get_debugall_tbl[17];
      p_wk->p_field_buff[17].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[17].value.int_s32 = cp_data->rnd.triple_win;
      //WIFIレート
      p_wk->p_field_buff[18].name          = (char*)sc_get_debugall_tbl[18];
      p_wk->p_field_buff[18].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[18].value.int_s32 = cp_data->wifi.rate;
      //WIFIチート
      p_wk->p_field_buff[19].name          = (char*)sc_get_debugall_tbl[19];
      p_wk->p_field_buff[19].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[19].value.int_s32 = cp_data->wifi.cheat;
      //WIFI切断
      p_wk->p_field_buff[20].name          = (char*)sc_get_debugall_tbl[20];
      p_wk->p_field_buff[20].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[20].value.int_s32 = cp_data->wifi.disconnect;
      //WIFIまけ
      p_wk->p_field_buff[21].name          = (char*)sc_get_debugall_tbl[21];
      p_wk->p_field_buff[21].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[21].value.int_s32 = cp_data->wifi.lose;
      //WIFIかち
      p_wk->p_field_buff[22].name          = (char*)sc_get_debugall_tbl[22];
      p_wk->p_field_buff[22].type          = DWC_GDB_FIELD_TYPE_INT;
      p_wk->p_field_buff[22].value.int_s32 = cp_data->wifi.win;
      //パーティ
      p_wk->p_field_buff[23].name          = (char*)sc_get_debugall_tbl[23];
      p_wk->p_field_buff[23].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[23].value.binary_data.data = (u8*)cp_data->wifi.pokeparty;
      p_wk->p_field_buff[23].value.binary_data.size = WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE;

      //ログイン時間
      p_wk->p_field_buff[24].name          = (char*)sc_get_debugall_tbl[24];
      p_wk->p_field_buff[24].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[24].value.binary_data.data = (u8*)&cp_data->datetime;
      p_wk->p_field_buff[24].value.binary_data.size = sizeof(WBM_NET_DATETIME);
      //MYSTATUS
      p_wk->p_field_buff[25].name          = (char*)sc_get_debugall_tbl[25];
      p_wk->p_field_buff[25].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[25].value.binary_data.data = (u8*)cp_data->mystatus;
      p_wk->p_field_buff[25].value.binary_data.size = MYSTATUS_SAVE_SIZE;
      //戦績０１
      p_wk->p_field_buff[26].name          = (char*)sc_get_debugall_tbl[26];
      p_wk->p_field_buff[26].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[26].value.binary_data.data = (u8*)&cp_data->record_data[0];
      p_wk->p_field_buff[26].value.binary_data.size = sizeof(WIFIBATTLEMATCH_RECORD_DATA);
      //戦績０２
      p_wk->p_field_buff[27].name          = (char*)sc_get_debugall_tbl[27];
      p_wk->p_field_buff[27].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[27].value.binary_data.data = (u8*)&cp_data->record_data[1];
      p_wk->p_field_buff[27].value.binary_data.size = sizeof(WIFIBATTLEMATCH_RECORD_DATA);
      //戦績０３
      p_wk->p_field_buff[28].name          = (char*)sc_get_debugall_tbl[28];
      p_wk->p_field_buff[28].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[28].value.binary_data.data = (u8*)&cp_data->record_data[2];
      p_wk->p_field_buff[28].value.binary_data.size = sizeof(WIFIBATTLEMATCH_RECORD_DATA);
      //戦績０４
      p_wk->p_field_buff[29].name          = (char*)sc_get_debugall_tbl[29];
      p_wk->p_field_buff[29].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[29].value.binary_data.data = (u8*)&cp_data->record_data[3];
      p_wk->p_field_buff[29].value.binary_data.size = sizeof(WIFIBATTLEMATCH_RECORD_DATA);
      //戦績０５
      p_wk->p_field_buff[30].name          = (char*)sc_get_debugall_tbl[30];
      p_wk->p_field_buff[30].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[30].value.binary_data.data = (u8*)&cp_data->record_data[4];
      p_wk->p_field_buff[30].value.binary_data.size = sizeof(WIFIBATTLEMATCH_RECORD_DATA);
      //戦績０６
      p_wk->p_field_buff[31].name          = (char*)sc_get_debugall_tbl[31];
      p_wk->p_field_buff[31].type          = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[31].value.binary_data.data = (u8*)&cp_data->record_data[5];
      p_wk->p_field_buff[31].value.binary_data.size = sizeof(WIFIBATTLEMATCH_RECORD_DATA);
      //戦績インデックス
      p_wk->p_field_buff[32].name          = (char*)sc_get_debugall_tbl[32];
      p_wk->p_field_buff[32].type          = DWC_GDB_FIELD_TYPE_BYTE;
      p_wk->p_field_buff[32].value.int_u8  = cp_data->record_save_idx;
    }
    break;
#endif

  case WIFIBATTLEMATCH_GDB_WRITE_POKEPARTY:

    p_wk->p_field_buff[0].name  = ATLAS_GET_STAT_NAME( ARENA_ELO_RATING_1V1_WIFICUP );
    p_wk->p_field_buff[0].type  = DWC_GDB_FIELD_TYPE_INT;
    p_wk->p_field_buff[0].value.int_s32 = WIFIBATTLEMATCH_GDB_DEFAULT_RATEING;

    p_wk->p_field_buff[1].name  = ATLAS_GET_STAT_NAME( CHEATS_WIFICUP_COUNTER );
    p_wk->p_field_buff[1].type  = DWC_GDB_FIELD_TYPE_INT;
    p_wk->p_field_buff[1].value.int_s32 = 0;

    p_wk->p_field_buff[2].name  = ATLAS_GET_STAT_NAME( DISCONNECTS_WIFICUP_COUNTER );
    p_wk->p_field_buff[2].type  = DWC_GDB_FIELD_TYPE_INT;
    p_wk->p_field_buff[2].value.int_s32 = 0;

    p_wk->p_field_buff[3].name  = ATLAS_GET_STAT_NAME( NUM_WIFICUP_LOSE_COUNTER );
    p_wk->p_field_buff[3].type  = DWC_GDB_FIELD_TYPE_INT;
    p_wk->p_field_buff[3].value.int_s32 = 0;

    p_wk->p_field_buff[4].name  = ATLAS_GET_STAT_NAME( NUM_WIFICUP_WIN_COUNTER );
    p_wk->p_field_buff[4].type  = DWC_GDB_FIELD_TYPE_INT;
    p_wk->p_field_buff[4].value.int_s32 = 0;

    p_wk->p_field_buff[5].name  = SAKE_STAT_WIFICUP_POKEMON_PARTY;
    p_wk->p_field_buff[5].type  = DWC_GDB_FIELD_TYPE_BINARY_DATA;
    p_wk->p_field_buff[5].value.binary_data.data = (u8*)cp_wk_adrs;
    p_wk->p_field_buff[5].value.binary_data.size = WIFIBATTLEMATCH_GDB_WIFI_POKEPARTY_SIZE;
    p_wk->table_name_num  = 6;

    break;
  case WIFIBATTLEMATCH_GDB_WRITE_LOGIN_DATE:
    { 
      WifiBattleMatch_SetDateTime( &p_wk->datetime );

      p_wk->table_name_num  = 1;
      p_wk->p_field_buff[0].name  = SAKE_STAT_LAST_LOGIN_DATETIME;
      p_wk->p_field_buff[0].type  = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[0].value.binary_data.data = (u8*)&p_wk->datetime;
      p_wk->p_field_buff[0].value.binary_data.size = sizeof(WBM_NET_DATETIME);
    }
    break;
  case WIFIBATTLEMATCH_GDB_WRITE_MYINFO:
    DwcRap_Gdb_SetMyInfo( p_wk );
    break;
  
  case WIFIBATTLEMATCH_GDB_WRITE_RECORD:     //戦績データを書き込み
    { 
      const WIFIBATTLEMATCH_GDB_RND_RECORD_DATA  *cp_data  = cp_wk_adrs;
      p_wk->record_data = *cp_data;

      p_wk->table_name_num  = 2;
      //戦績
      p_wk->p_field_buff[0].name  = (char*)scp_stat_record_data_tbl[ p_wk->record_data.save_idx ];
      p_wk->p_field_buff[0].type  = DWC_GDB_FIELD_TYPE_BINARY_DATA;
      p_wk->p_field_buff[0].value.binary_data.data = (u8*)&p_wk->record_data.record_data;
      p_wk->p_field_buff[0].value.binary_data.size = sizeof(WIFIBATTLEMATCH_RECORD_DATA);
      //インデックス
      p_wk->p_field_buff[1].name          = SAKE_STAT_RECORD_SAVE_IDX;
      p_wk->p_field_buff[1].type          = DWC_GDB_FIELD_TYPE_BYTE;
      p_wk->p_field_buff[1].value.int_u8  = (p_wk->record_data.save_idx + 1 ) % NELEMS(scp_stat_record_data_tbl);

      DEBUG_NET_Printf( "戦績データを送信開始 sake%d 今%d 次IDX%d\n", p_wk->p_data->sake_recordID, p_wk->record_data.save_idx,p_wk->p_field_buff[1].value.int_u8 );
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
 *
 *	@return TRUEで終了  FALSEで破棄
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_GDB_RESULT WIFIBATTLEMATCH_GDB_ProcessWrite( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
    SEQ_EXIT,
    SEQ_NONE,

    SEQ_CANCEL_START,
    SEQ_CANCEL_WAIT,
    SEQ_CANCEL_END,
  };

  DWCGdbState state;
  DWCGdbError error;

  { 
    switch( p_wk->seq )
    { 
    case SEQ_INIT:
      error = DWC_GdbInitialize( GAME_ID, p_wk->cp_user_data, WIFIBATTLEMATCH_NET_SSL_TYPE );
      if( error != DWC_GDB_ERROR_NONE )
      { 
        WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ );
        DwcRap_Gdb_Finalize( p_wk );
        return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
      }
      DEBUG_NET_Printf( "GDBW:Init\n" );
      p_wk->seq = SEQ_START;
      break;

    case SEQ_START:
      GF_ASSERT( p_wk->table_name_num < WBM_GDB_FIELD_TABLE_MAX );
      error = DWC_GdbUpdateRecordAsync( WIFIBATTLEMATCH_NET_TABLENAME, p_wk->p_data->sake_recordID, p_wk->p_field_buff, p_wk->table_name_num );
      if( error != DWC_GDB_ERROR_NONE )
      { 
        WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ );
        DwcRap_Gdb_Finalize( p_wk );
        return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
      }
      DEBUG_NET_Printf( "GDBW:Get start\n" );
      p_wk->async_timeout = 0;
      p_wk->seq= SEQ_WAIT;
      break;

    case SEQ_WAIT:
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ );
          p_wk->seq = SEQ_CANCEL_START;
          break;
        }

        DWC_GdbProcess();

        state = DWC_GdbGetState();
        if( state == DWC_GDB_STATE_UNINITIALIZED || 
            state == DWC_GDB_STATE_ERROR_OCCURED )
        { 
          WIFIBATTLEMATCH_NETERR_SetGdbState( &p_wk->error, state, __LINE__ );
          p_wk->seq = SEQ_CANCEL_START;
          break;
        }

        if( state == DWC_GDB_STATE_IDLE )
        { 
          DEBUG_NET_Printf( "GDBW:Get wait end\n" );
          p_wk->seq = SEQ_END;
        }
      }
      break;

    case SEQ_END:
      { 
        DWCGdbAsyncResult ret;
        ret = DWC_GdbGetAsyncResult();
        if( ret == DWC_GDB_ASYNC_RESULT_NONE )
        {
          WIFIBATTLEMATCH_NETERR_SetGdbResult( &p_wk->error, ret, __LINE__ );
          p_wk->seq = SEQ_CANCEL_START;
          break;
        }else 
        if( ret != DWC_GDB_ASYNC_RESULT_SUCCESS )
        { 
          DwcRap_Gdb_Finalize( p_wk );
          WIFIBATTLEMATCH_NETERR_SetGdbResult( &p_wk->error, ret, __LINE__ );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }
      }
      DEBUG_NET_Printf( "GDBW:Get end\n" );
      p_wk->seq = SEQ_EXIT;
      break;

    case SEQ_EXIT:
      DwcRap_Gdb_Finalize( p_wk );
      DEBUG_NET_Printf( "GDBW:Get exit\n" );
      DEBUG_NET_Printf( "sake テーブルID %d\n", p_wk->p_data->sake_recordID );
      p_wk->seq = SEQ_NONE;
      /* fallthrough */

    case SEQ_NONE:
      return WIFIBATTLEMATCH_GDB_RESULT_SUCCESS;

    case SEQ_CANCEL_START:
      {

        error = DWC_GdbCancelRequest();
        DEBUG_NET_Printf( "GDB:Start Cancel\n" );
        if( error == DWC_GDB_ERROR_NONE )
        {
          p_wk->async_timeout = 0;
          p_wk->seq= SEQ_CANCEL_WAIT;
        }
        else
        {
          WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ );
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }
      }
      break;

    case SEQ_CANCEL_WAIT:
      DEBUG_NET_Printf( "GDB:Wait Cancel\n" );
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ );
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }

        DWC_GdbProcess();

        state = DWC_GdbGetState();
        if( state == DWC_GDB_STATE_UNINITIALIZED || 
            state == DWC_GDB_STATE_ERROR_OCCURED )
        { 
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }

        if( state == DWC_GDB_STATE_IDLE )
        { 
          p_wk->seq = SEQ_CANCEL_END;
        }
      }
      break;

    case SEQ_CANCEL_END:
      DwcRap_Gdb_Finalize( p_wk );
      return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
    }
  }

  return WIFIBATTLEMATCH_GDB_RESULT_UPDATE;
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
  p_wk->seq           = 0;
  DwcRap_Gdb_SetMyInfo( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  レコードテーブルを作成
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_GDB_RESULT WIFIBATTLEMATCH_GDB_ProcessCreateRecord( WIFIBATTLEMATCH_NET_WORK *p_wk)
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_CREATE_START,
    SEQ_CREATE_WAIT,
    SEQ_EXIT,

    SEQ_CANCEL_START,
    SEQ_CANCEL_WAIT,
    SEQ_CANCEL_END,
  };
  DWCGdbState state;
  DWCGdbError error; 

  { 
    switch( p_wk->seq )
    { 
    case SEQ_INIT:
	    error = DWC_GdbInitialize( GAME_ID, p_wk->cp_user_data, WIFIBATTLEMATCH_NET_SSL_TYPE );
      if( error != DWC_GDB_ERROR_NONE )
      { 
        WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ );
        return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
      }

      DEBUG_NET_Printf( "INIT:Init\n" );
      p_wk->seq = SEQ_CREATE_START;
      break;

	  case SEQ_CREATE_START:
	    error = DWC_GdbCreateRecordAsync( WIFIBATTLEMATCH_NET_TABLENAME,
	                                      p_wk->p_field_buff, 
	                                      p_wk->table_name_num, &p_wk->p_data->sake_recordID );
	    if( error != DWC_GDB_ERROR_NONE )
	    { 
        WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ ); 
        return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
	    }
      DEBUG_NET_Printf( "INIT:Create Start\n" );
	    p_wk->async_timeout = 0;
	    p_wk->seq = SEQ_CREATE_WAIT;
      break;
	
	  case SEQ_CREATE_WAIT:
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ );
          p_wk->seq = SEQ_CANCEL_START;
          break;
        }

        DWC_GdbProcess();

        state = DWC_GdbGetState();
        if( state == DWC_GDB_STATE_UNINITIALIZED || 
            state == DWC_GDB_STATE_ERROR_OCCURED )
        { 

          WIFIBATTLEMATCH_NETERR_SetGdbState( &p_wk->error, state, __LINE__ );
          p_wk->seq = SEQ_CANCEL_START;
          break;
        }

        if( state == DWC_GDB_STATE_IDLE )
        { 
          DEBUG_NET_Printf( "Init:Get wait end\n" );
          p_wk->seq = SEQ_EXIT;
        }
      }
	    break;

    case SEQ_EXIT:
      {
        { 
          DWCGdbAsyncResult ret;
          ret = DWC_GdbGetAsyncResult();
          if( ret == DWC_GDB_ASYNC_RESULT_NONE )
          {
            WIFIBATTLEMATCH_NETERR_SetGdbResult( &p_wk->error, ret, __LINE__ );
            p_wk->seq = SEQ_CANCEL_START;
            break;
          }
          else if( ret != DWC_GDB_ASYNC_RESULT_SUCCESS )
          { 
            DwcRap_Gdb_Finalize( p_wk );
            WIFIBATTLEMATCH_NETERR_SetGdbResult( &p_wk->error, ret, __LINE__ );
            return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
          }
        }
        if( p_wk->p_data->sake_recordID == 0 )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_NO_RECORD, __LINE__ );  
        }
        DwcRap_Gdb_Finalize( p_wk );
        DEBUG_NET_Printf( "INIT:Back exit\n" );
        DEBUG_NET_Printf( "作成! sake テーブルID %d\n", p_wk->p_data->sake_recordID );
        return WIFIBATTLEMATCH_GDB_RESULT_SUCCESS;
      }

    case SEQ_CANCEL_START:
      {
        error = DWC_GdbCancelRequest();
        DEBUG_NET_Printf( "GDB:Start Cancel\n" );
        if( error == DWC_GDB_ERROR_NONE )
        {
          p_wk->async_timeout = 0;
          p_wk->seq= SEQ_CANCEL_WAIT;
        }
        else
        {
          WIFIBATTLEMATCH_NETERR_SetGdbError( &p_wk->error, error, __LINE__ );
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }
      }
      break;

    case SEQ_CANCEL_WAIT:
      DEBUG_NET_Printf( "GDB:Wait Cancel\n" );
      { 
        if( p_wk->async_timeout++ > ASYNC_TIMEOUT )
        { 
          WIFIBATTLEMATCH_NETERR_SetSysError( &p_wk->error, WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT, __LINE__ );
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }

        DWC_GdbProcess();

        state = DWC_GdbGetState();
        if( state == DWC_GDB_STATE_UNINITIALIZED || 
            state == DWC_GDB_STATE_ERROR_OCCURED )
        { 
          DwcRap_Gdb_Finalize( p_wk );
          return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
        }

        if( state == DWC_GDB_STATE_IDLE )
        { 
          p_wk->seq = SEQ_CANCEL_END;
        }
      }
      break;

    case SEQ_CANCEL_END:
      DwcRap_Gdb_Finalize( p_wk );
      return WIFIBATTLEMATCH_GDB_RESULT_ERROR;
    }
  }

  return WIFIBATTLEMATCH_GDB_RESULT_UPDATE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  SAKEのレコードIDを取得
 *
 *	@param	const WIFIBATTLEMATCH_NET_WORK *cp_wk ワーク
 *
 *	@return レコードID
 */
//-----------------------------------------------------------------------------
u32 WIFIBATTLEMATCH_GDB_GetRecordID( const WIFIBATTLEMATCH_NET_WORK *cp_wk )
{ 
  return cp_wk->p_data->sake_recordID;
}


//=============================================================================
/**
 *    お互いの情報を送る  SENDDATA系
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  敵情報
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *	@return TRUEで送信  FALSEで失敗
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
  BOOL ret  = p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_GAMEDATA];

  if( ret )
  { 

    p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_GAMEDATA]  = FALSE;
    *pp_data  = (WIFIBATTLEMATCH_ENEMYDATA *)p_wk->recv_buffer;
  }

  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のポケパーティを送る
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	POKEPARTY *cp_party   送る自分のポケパーティ
 *
 *	@return TRUEで送信  FALSEで失敗
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_SendPokeParty( WIFIBATTLEMATCH_NET_WORK *p_wk, const POKEPARTY *cp_party )
{ 
  NetID netID;


  //相手にのみ送信
  netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
  netID = netID == 0? 1: 0;
  return GFL_NET_SendDataEx( GFL_NET_HANDLE_GetCurrentHandle(), netID, WIFIBATTLEMATCH_NETCMD_SEND_POKEPARTY, PokeParty_GetWorkSize(), cp_party, TRUE, TRUE, TRUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  相手のポケパーティ受信待ち
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	*p_party                        相手からもらったポケパーティ
 *
 *	@return TRUE受信  FALSE受信中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_RecvPokeParty( WIFIBATTLEMATCH_NET_WORK *p_wk, POKEPARTY *p_party )
{ 
  BOOL ret;
  ret = p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_POKEPARTY];

  if( ret )
  { 
    p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_POKEPARTY] = FALSE;
    GFL_STD_MemCopy( p_wk->recv_buffer, p_party, PokeParty_GetWorkSize() );
  }
  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  不正カウントを送り合う
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	dirty_cnt                       不正カウンタ
 *
 *	@return TRUEで送信  FALSEで送信していない
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_SendDirtyCnt( WIFIBATTLEMATCH_NET_WORK *p_wk, const u32 *cp_dirty_cnt )
{ 
  NetID netID;


  //相手にのみ送信
  netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
  netID = netID == 0? 1: 0;
  return GFL_NET_SendDataEx( GFL_NET_HANDLE_GetCurrentHandle(), netID, WIFIBATTLEMATCH_NETCMD_SEND_DIRTYCNT, sizeof(u32), cp_dirty_cnt, TRUE, TRUE, TRUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  不正カウントを受信する
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	*p_dirty_cnt                    受信した不正カウンタ
 *
 *	@return TRUEで受信  FALSEで受信中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_RecvDirtyCnt( WIFIBATTLEMATCH_NET_WORK *p_wk, u32 *p_dirty_cnt )
{ 
  BOOL ret;
  ret = p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_DIRTYCNT];

  if( ret )
  { 
    p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_DIRTYCNT]  = FALSE;
    GFL_STD_MemCopy( p_wk->recv_buffer, p_dirty_cnt, sizeof(u32) );
  }
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトル不正フラグを送信する
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	dirty_flag                      不正フラグ
 *
 *	@return TRUEで送信  FALSEで送信していない
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_SendBtlDirtyFlag( WIFIBATTLEMATCH_NET_WORK *p_wk, const u32 *cp_dirty_flag )
{
  return GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), WIFIBATTLEMATCH_NETCMD_SEND_FLAG, sizeof(u32), cp_dirty_flag );
}

//----------------------------------------------------------------------------
/**
 *	@brief  バトル不正フラグを受信する
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	*p_dirty_flag                   受信した不正フラグ
 *
 *	@return TRUEで受信  FALSEで受信中
 */
//-----------------------------------------------------------------------------
BOOL WIFIBATTLEMATCH_NET_RecvBtlDirtyFlag( WIFIBATTLEMATCH_NET_WORK *p_wk, u32 *p_dirty_flag )
{
  BOOL ret;
  ret = p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_FLAG];

  if( ret )
  { 
    p_wk->is_recv[WIFIBATTLEMATCH_NET_RECVFLAG_FLAG]  = FALSE;
    *p_dirty_flag = p_wk->recv_flag;
  }
  return ret;
}

//=============================================================================
/**
 *    ダウンロード系  （ND系）
 */
//=============================================================================
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
  p_wk->seq = 0;
  GFL_STD_MemClear( p_wk->nd_attr2, 10 );

  GFL_STD_MemClear( &p_wk->fileInfo, sizeof(DWCNdFileInfo) );

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
    SEQ_FILELIST,
    SEQ_GET_FILE,
    SEQ_GETTING_FILE,

    SEQ_DOWNLOAD_COMPLETE,
    SEQ_END,
    SEQ_EMPTY_END,
    SEQ_ERROR_END,

    SEQ_WAIT_CALLBACK         = 100,
    SEQ_WAIT_CLEANUP_CALLBACK = 200,
  };

  switch( p_wk->seq )
  { 
  case SEQ_INIT:


    if( DWC_NdInitAsync( NdCallback, GF_DWC_ND_LOGIN, WIFI_ND_LOGIN_PASSWD ) == FALSE )
    {
      DEBUG_NET_Printf( "DWC_NdInitAsync: Failed\n" );
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
    }
    else
    {
      DwcRap_Nd_WaitNdCallback( p_wk, SEQ_ATTR );
    }
    break;

  case SEQ_ATTR:
    // ファイル属性の設定
    if( DWC_NdSetAttr(WIFI_FILE_ATTR1, p_wk->nd_attr2, WIFI_FILE_ATTR3) == FALSE )
    {
      DEBUG_NET_Printf( "DWC_NdSetAttr: Failed\n." );
      DwcRap_Nd_CleanUpNdCallback( p_wk, SEQ_ERROR_END );
    }
    else
    {
      p_wk->seq = SEQ_FILELIST;
    }
    break;

//-------------------------------------
///	サーバーのファイルチェック
//=====================================
  case SEQ_FILELIST:
    //強制的に１つをとりにいく
    { 
      if( DWC_NdGetFileListAsync( &p_wk->fileInfo, 0, 1 ) == FALSE)
      {
        DEBUG_NET_Printf( "DWC_NdGetFileListNumAsync: Failed.\n" );
        DwcRap_Nd_CleanUpNdCallback( p_wk, SEQ_ERROR_END );
      }
      else
      {
        DwcRap_Nd_WaitNdCallback( p_wk, SEQ_GET_FILE );
      }
    }
    break;

//-------------------------------------
///	ファイル読み込み開始
//=====================================
  case SEQ_GET_FILE:
    if( !UTIL_IsClear( &p_wk->fileInfo, sizeof(DWCNdFileInfo) ) )
    { 
      //存在していた場合ファイル読み込み開始
      if(DWC_NdGetFileAsync( &p_wk->fileInfo, (char*)&p_wk->temp_buffer, REGULATION_CARD_DATA_SIZE) == FALSE){
        DEBUG_NET_Printf( "DWC_NdGetFileAsync: Failed.\n" );
        DwcRap_Nd_CleanUpNdCallback( p_wk, SEQ_ERROR_END );
      }
      else
      {
        s_callback_flag   = FALSE;
        s_callback_result = DWC_ND_ERROR_NONE;
        p_wk->seq = SEQ_GETTING_FILE;
      }
    }
    else
    {
      //存在していなかった場合、
      if( !DWC_NdCleanupAsync() ){  //FALSEの場合コールバックが呼ばれない
        return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_EMPTY;
      }
      else
      {
        DwcRap_Nd_WaitNdCallback( p_wk, SEQ_EMPTY_END );
      }

    }
    break;
    
  case SEQ_GETTING_FILE:
    // ファイル読み込み中
    DWC_NdProcess();

    if( p_wk->async_timeout++ > WIFI_ND_TIMEOUT_SYNC )
    {
      GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_USER_TIMEOUT );
      DwcRap_Nd_CleanUpNdCallback( p_wk, SEQ_ERROR_END );
    }

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
      OS_Printf("DWC_NdGetProgressでエラー\n");
      DwcRap_Nd_CleanUpNdCallback( p_wk, SEQ_ERROR_END );
    }
    else
    {
      // ファイル読み込み終了
      if( !DWC_NdCleanupAsync() ){  //FALSEの場合コールバックが呼ばれない
        OS_Printf("DWC_NdCleanupAsyncに失敗\n");
        return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;
      }
      else
      {
        DwcRap_Nd_WaitNdCallback( p_wk, SEQ_DOWNLOAD_COMPLETE );
      }
    }
    break;

//-------------------------------------
///	終了・キャンセル処理
//=====================================
  case SEQ_DOWNLOAD_COMPLETE:
    DEBUG_NET_Printf( "●regulation card data\n" );
    DEBUG_NET_Printf( "cup no %d\n", p_wk->temp_buffer.no );
    DEBUG_NET_Printf( "status %d\n", p_wk->temp_buffer.status );

    p_wk->seq  = SEQ_END;
    break;

  case SEQ_END:
    //CRCチェック
    if( Regulation_CheckCrc( &p_wk->temp_buffer ) )
    { 
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_SUCCESS;
    }
    else
    { 
      return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_EMPTY;
    }


  case SEQ_EMPTY_END:
    return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_EMPTY;

  case SEQ_ERROR_END:
    return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_ERROR;

//-------------------------------------
///	コールバック待ち処理  
//　　WIFI_DOWNLOAD_WaitNdCallbackを使ってください
//=====================================
  case SEQ_WAIT_CALLBACK:
    //コールバック処理を待つ
    DWC_NdProcess();
    if(s_callback_result != DWC_ND_ERROR_NONE)
    {
      DwcRap_Nd_CleanUpNdCallback( p_wk, SEQ_ERROR_END );
    }
    if( s_callback_flag )
    { 
      s_callback_flag = FALSE;
      if( s_callback_result == DWC_ND_ERROR_NONE)
      { 
        p_wk->seq  = p_wk->next_seq;
      }
    }
    break;

  case SEQ_WAIT_CLEANUP_CALLBACK:
    DWC_NdProcess();
    if( s_cleanup_callback_flag )
    { 
      s_cleanup_callback_flag = FALSE;
      p_wk->seq  = p_wk->next_seq;
    }
    break;
  }

  return WIFIBATTLEMATCH_NET_DOWNLOAD_DIGCARD_RET_DOWNLOADING;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ０クリアされているかどうか
 *
 *	@param	void *p_adrs  アドレス
 *	@param	size          バイトサイズ
 *
 *	@return TRUEで０クリアされている  FALSEでされていない
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_IsClear( void *p_adrs, u32 size )
{
  int i;
  u32 *p_ptr  = (u32*)p_adrs;

  for( i = 0; i < size/4; i++ )
  {
    if( *(p_ptr + i) != 0 )
    {
      return FALSE;
    }
  }

  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  エラー時に切断の前に呼ばれるコールバック
 *
 *	@param	p_wk_adrs ワークアドレス
 *	@param	code      エラーコード
 *	@param	type      エラータイプ
 *	@param	ret       エラーリターン
 */
//-----------------------------------------------------------------------------
static void WifiBattleMatch_ErrDisconnectCallback(void* p_wk_adrs, int code, int type, int ret )
{
  WIFIBATTLEMATCH_NET_WORK *p_wk  = p_wk_adrs;

  DEBUG_NET_Printf( "Call ErrDisconnectCallback code=%d type=%d ret=%d\n", code, type, ret );
  switch( type )
  {
    //切断エラー
  case DWC_ETYPE_SHUTDOWN_FM:
  case DWC_ETYPE_DISCONNECT:
    //NHTTPが解放していないならば先に呼ぶ

    if(p_wk->p_nhttp)
    {
      NHTTP_RAP_ErrorClean(p_wk->p_nhttp);
      NHTTP_RAP_End(p_wk->p_nhttp);
      p_wk->p_nhttp  = NULL;
    }

    //SCが使われているならば呼ぶ
    DwcRap_Sc_Finalize( p_wk );
  }
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
//----------------------------------------------------------------------------
/**
 *	@brief  クリーンアップ
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	next_seq  次のシーケンス
 *	@param  false_seq 失敗したときのシーケンス
 *
 */
//-----------------------------------------------------------------------------
static void DwcRap_Nd_CleanUpNdCallback( WIFIBATTLEMATCH_NET_WORK *p_wk, int next_seq )
{
  s_callback_flag   = FALSE;
  s_callback_result = DWC_ND_ERROR_NONE;
  p_wk->next_seq    = next_seq;
  p_wk->seq         = 200;

  if( !DWC_NdCleanupAsync(  ) ){  //FALSEの場合コールバックが呼ばれない
    OS_Printf("DWC_NdCleanupAsyncに失敗\n");
    p_wk->next_seq    = next_seq;
  }
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
  s_cleanup_callback_flag = TRUE;

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
void WIFIBATTLEMATCH_NET_StartRecvGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk, HEAPID heapID )
{ 
  GF_ASSERT( p_wk->p_nhttp == NULL );
  p_wk->seq = 0;
  p_wk->p_nhttp = NHTTP_RAP_Init( heapID, MyStatus_GetProfileID(GAMEDATA_GetMyStatus(p_wk->p_gamedata)), p_wk->p_svl_result);
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
WIFIBATTLEMATCH_RECV_GPFDATA_RET WIFIBATTLEMATCH_NET_WaitRecvGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_START_CONNECT,
    SEQ_WAIT_CONNECT,

    SEQ_END,
    SEQ_DIRTY_END,
  };

  NHTTPError error;

  if( p_wk->p_nhttp == NULL )
  { 
    return WIFIBATTLEMATCH_RECV_GPFDATA_RET_ERROR;
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
    if(NHTTP_RAP_StartConnect(p_wk->p_nhttp)==NHTTP_ERROR_NONE)
    {
      p_wk->seq  = SEQ_WAIT_CONNECT;
    }
    else
    {
      if(p_wk->p_nhttp)
      {
        NHTTP_RAP_End(p_wk->p_nhttp);
        p_wk->p_nhttp  = NULL;
      }
      return WIFIBATTLEMATCH_RECV_GPFDATA_RET_ERROR;
    }
    break;

  case SEQ_WAIT_CONNECT:
    {
      int response = NHTTP_RAP_GetGetResultCode(p_wk->p_nhttp);

      switch( response )
      {
      case 502:
        p_wk->seq  = SEQ_DIRTY_END;

        NetErr_App_ReqErrorDispForce( WIFI_GSYNC_ERR011 );
        return WIFIBATTLEMATCH_RECV_GPFDATA_RET_UPDATE;

      case 503:
        p_wk->seq  = SEQ_DIRTY_END;
        NetErr_App_ReqErrorDispForce( WIFI_GSYNC_ERR012 );
        return WIFIBATTLEMATCH_RECV_GPFDATA_RET_UPDATE;
      }
    }

    error  = NHTTP_RAP_Process(p_wk->p_nhttp);
    if(NHTTP_ERROR_NONE == error)
    {
      u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(p_wk->p_nhttp);
      DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA* pDream = (DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA*)&pEvent[sizeof(gs_response)];


      NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
      DEBUG_NET_Printf("ID %x\n",pDream->WifiMatchUpID);
      DEBUG_NET_Printf("FLG %x\n",pDream->GPFEntryFlg);
      DEBUG_NET_Printf("ST %d\n",pDream->WifiMatchUpState);
      DEBUG_NET_Printf("Sign %d\n",pDream->signin);

      DEBUG_NET_Printf("終了\n");

      p_wk->gpf_data  = *pDream;

      if( ((gs_response*)pEvent)->ret_cd == DREAM_WORLD_SERVER_ERROR_NONE
        ||  ((gs_response*)pEvent)->ret_cd == DREAM_WORLD_SERVER_ALREADY_EXISTS )
      { 
        p_wk->seq  = SEQ_END;
      }
      else
      { 

        NetErr_App_ReqErrorDispForce( WIFI_GSYNC_ERR001 + ((gs_response*)pEvent)->ret_cd -1 );
        p_wk->seq  = SEQ_DIRTY_END;
      }
    }
    else if( NHTTP_ERROR_BUSY != error )
    { 
      if(p_wk->p_nhttp)
      {
        NHTTP_RAP_End(p_wk->p_nhttp);
        p_wk->p_nhttp  = NULL;
      }
      return WIFIBATTLEMATCH_RECV_GPFDATA_RET_ERROR;
    }

    break;

  case SEQ_DIRTY_END:
    GFL_STD_MemClear( &p_wk->gpf_data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA) );
    if(p_wk->p_nhttp)
    {
      NHTTP_RAP_ErrorClean( p_wk->p_nhttp );
      NHTTP_RAP_End(p_wk->p_nhttp);
      p_wk->p_nhttp  = NULL;
    }
    return WIFIBATTLEMATCH_RECV_GPFDATA_RET_DIRTY;

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
 *	@param	DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA *cp_send    書き込みデータ
 *	@param	heapID                                                ヒープID
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartSendGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk, const DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA *cp_send, HEAPID heapID )
{ 
  GF_ASSERT( p_wk->p_nhttp == NULL );
  p_wk->p_nhttp = NHTTP_RAP_Init( heapID, MyStatus_GetProfileID(GAMEDATA_GetMyStatus(p_wk->p_gamedata)), p_wk->p_svl_result);
  p_wk->gdb_write_data  = *cp_send;
  p_wk->seq = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  GPFサーバーへ書き込み メイン処理
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_SEND_GPFDATA_RET WIFIBATTLEMATCH_NET_WaitSendGpfData( WIFIBATTLEMATCH_NET_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_START_CONNECT,
    SEQ_WAIT_CONNECT,

    SEQ_DIRTY_END,  
    SEQ_END,
  };
  NHTTPError error;

  if( p_wk->p_nhttp == NULL )
  { 
    return WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR;
  }

  switch( p_wk->seq )
  { 
  case SEQ_INIT:
    if (NHTTP_RAP_ConectionCreate( NHTTPRAP_URL_BATTLE_UPLOAD, p_wk->p_nhttp ))
    {
      NHTTP_AddPostDataRaw(NHTTP_RAP_GetHandle(p_wk->p_nhttp), &p_wk->gdb_write_data, sizeof(DREAM_WORLD_SERVER_WORLDBATTLE_SET_DATA) );
      p_wk->seq  = SEQ_START_CONNECT;
    }
    break;

  case SEQ_START_CONNECT:
    if(NHTTP_RAP_StartConnect(p_wk->p_nhttp)==NHTTP_ERROR_NONE)
    {
      p_wk->seq  = SEQ_WAIT_CONNECT;
    }
    else
    {
      if(p_wk->p_nhttp)
      {
        NHTTP_RAP_End(p_wk->p_nhttp);
        p_wk->p_nhttp  = NULL;
      }
      return WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR;
    }
    break;

  case SEQ_WAIT_CONNECT:
    {
      int response = NHTTP_RAP_GetGetResultCode(p_wk->p_nhttp);
      switch( response )
      {
      case 502:
        p_wk->seq  = SEQ_DIRTY_END;
        NetErr_App_ReqErrorDispForce( WIFI_GSYNC_ERR011 );
        return WIFIBATTLEMATCH_SEND_GPFDATA_RET_UPDATE;

      case 503:
        p_wk->seq  = SEQ_DIRTY_END;
        NetErr_App_ReqErrorDispForce( WIFI_GSYNC_ERR012 );
        return WIFIBATTLEMATCH_SEND_GPFDATA_RET_UPDATE;
      }
    }


    error  = NHTTP_RAP_Process(p_wk->p_nhttp);
    if(NHTTP_ERROR_NONE == error)
    {
      u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(p_wk->p_nhttp);
      NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
      DEBUG_NET_Printf("GPFサーバーへの書き込み終了\n");



      if( ((gs_response*)pEvent)->ret_cd == DREAM_WORLD_SERVER_ERROR_NONE
          ||  ((gs_response*)pEvent)->ret_cd == DREAM_WORLD_SERVER_ALREADY_EXISTS )
      {  
        p_wk->seq  = SEQ_END;
      }
      else
      {
        NetErr_App_ReqErrorDispForce( WIFI_GSYNC_ERR001 + ((gs_response*)pEvent)->ret_cd -1 );
        p_wk->seq  = SEQ_DIRTY_END;
      }
    }
    else if( NHTTP_ERROR_BUSY != error )
    { 
      WIFIBATTLEMATCH_NETERR_SetNhttpError( &p_wk->error, error, __LINE__ );

      if(p_wk->p_nhttp)
      {
        NHTTP_RAP_End(p_wk->p_nhttp);
        p_wk->p_nhttp  = NULL;
      }

      return WIFIBATTLEMATCH_SEND_GPFDATA_RET_ERROR;
    }
    break;

  case SEQ_DIRTY_END:
    if(p_wk->p_nhttp)
    {
      NHTTP_RAP_ErrorClean( p_wk->p_nhttp );
      NHTTP_RAP_End(p_wk->p_nhttp);
      p_wk->p_nhttp  = NULL;
    }
    return WIFIBATTLEMATCH_SEND_GPFDATA_RET_DIRTY;

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
 *	@param	STRBUF *p_str                   チェックするSTRBUF
 *	@param	HEAPID                          ヒープID
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartBadWord( WIFIBATTLEMATCH_NET_WORK *p_wk, MYSTATUS *p_mystatus, HEAPID heapID )
{ 
  p_wk->p_badword_mystatus  = p_mystatus;
  {

    STRBUF *p_word_check  = MyStatus_CreateNameString( p_mystatus, p_wk->heapID );
    DWC_TOOL_BADWORD_Start( &p_wk->badword, p_word_check, heapID );

    GFL_STR_DeleteBuffer( p_word_check );
  }
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
  ret = DWC_TOOL_BADWORD_Wait( &p_wk->badword, p_is_bad_word );

  if( ret == TRUE && *p_is_bad_word == TRUE )
  { 
    STRBUF  *p_modifiname = DWC_TOOL_CreateBadNickName( p_wk->heapID );

    MyStatus_SetMyNameFromString( p_wk->p_badword_mystatus, p_modifiname);

    GFL_STR_DeleteBuffer(p_modifiname);
    NAGI_Printf( "わるもしでした\n" );
  }

  return ret;
}

//=============================================================================
/**
 *    ポケモン不正チェック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン不正チェック  開始
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk  ワーク
 *	@param	POKEPARTY *cp_party チェックするポケモンパーティ
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NET_StartEvilCheck( WIFIBATTLEMATCH_NET_WORK *p_wk, const void *cp_void, WIFIBATTLEMATCH_NET_EVILCHECK_TYPE type )
{ 
  p_wk->seq = 0;

  p_wk->p_nhttp = NHTTP_RAP_Init( p_wk->heapID, MyStatus_GetProfileID(GAMEDATA_GetMyStatus(p_wk->p_gamedata)), p_wk->p_svl_result);

  switch( type )
  { 
  case WIFIBATTLEMATCH_NET_EVILCHECK_TYPE_PP:
    NHTTP_RAP_PokemonEvilCheckCreate( p_wk->p_nhttp, p_wk->heapID, POKETOOL_GetWorkSize(), NHTTP_POKECHK_RANDOMMATCH);
    NHTTP_RAP_PokemonEvilCheckAdd( p_wk->p_nhttp, (void*)cp_void, POKETOOL_GetWorkSize() );
    p_wk->poke_max  = 1;
    break;

  case WIFIBATTLEMATCH_NET_EVILCHECK_TYPE_PARTY:
    { 
      int i;
      const POKEPARTY *cp_party = (const POKEPARTY*)cp_void;
      POKEMON_PARAM *p_pp;

      NHTTP_RAP_PokemonEvilCheckCreate( p_wk->p_nhttp, p_wk->heapID, POKETOOL_GetWorkSize() * PokeParty_GetPokeCount(cp_party), NHTTP_POKECHK_RANDOMMATCH);
      for( i = 0; i < PokeParty_GetPokeCount(cp_party); i++ )
      { 
        p_pp  = PokeParty_GetMemberPointer( cp_party, i );
        NHTTP_RAP_PokemonEvilCheckAdd( p_wk->p_nhttp, p_pp, POKETOOL_GetWorkSize() );

        DEBUG_NET_Printf( "monsno %d\n", PP_Get( p_pp, ID_PARA_monsno, NULL) );
        DEBUG_NET_Printf( "rnd %d\n", PP_Get( p_pp, ID_PARA_personal_rnd, NULL) );
        DEBUG_NET_Printf( "crc %d\n", GFL_STD_CrcCalc( p_pp, POKETOOL_GetWorkSize() ) );
      }
      p_wk->poke_max  = PokeParty_GetPokeCount(cp_party);
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン不正チェック  処理待ち
 *
 *	@param	WIFIBATTLEMATCH_NET_WORK *p_wk ワーク
 *	@param  WIFIBATTLEMATCH_NET_EVILCHECK_DATA  データ
 *
 *	@return WIFIBATTLEMATCH_NET_EVILCHECK_RET
 */
//-----------------------------------------------------------------------------
WIFIBATTLEMATCH_NET_EVILCHECK_RET WIFIBATTLEMATCH_NET_WaitEvilCheck( WIFIBATTLEMATCH_NET_WORK *p_wk, WIFIBATTLEMATCH_NET_EVILCHECK_DATA *p_data )
{ 
  enum
  { 
    SEQ_EVILCHECL_START,
    SEQ_EVILCHECL_WAIT,
    SEQ_END,
  };

  switch( p_wk->seq )
  { 
  case SEQ_EVILCHECL_START:
    { 
      BOOL ret;
      ret = NHTTP_RAP_PokemonEvilCheckConectionCreate( p_wk->p_nhttp );

      if( ret )
      {
        if( NHTTP_RAP_StartConnect( p_wk->p_nhttp ) == NHTTP_ERROR_NONE )
        {
          p_wk->seq++;
        }
        else
        {
          if(p_wk->p_nhttp)
          {
            NHTTP_RAP_End(p_wk->p_nhttp);
            p_wk->p_nhttp  = NULL;
            p_wk->seq = 0;
          }
          return WIFIBATTLEMATCH_NET_EVILCHECK_RET_ERROR;
        }
        DEBUG_NET_Printf( "不正チェック開始\n" );
      }
      else
      {
        if(p_wk->p_nhttp)
        {
          NHTTP_RAP_End(p_wk->p_nhttp);
          p_wk->p_nhttp  = NULL;
          p_wk->seq = 0;
        }
        return WIFIBATTLEMATCH_NET_EVILCHECK_RET_ERROR;
      }

    }
    break;

  case SEQ_EVILCHECL_WAIT:
    { 
      NHTTPError error;
      error = NHTTP_RAP_Process( p_wk->p_nhttp );
      if( NHTTP_ERROR_NONE == error )
      {
        p_wk->seq++;
      }
      else if( NHTTP_ERROR_BUSY != error )
      { 
        WIFIBATTLEMATCH_NETERR_SetNhttpError( &p_wk->error, error, __LINE__ );
        NHTTP_RAP_PokemonEvilCheckDelete(p_wk->p_nhttp);
        if(p_wk->p_nhttp)
        {
          NHTTP_RAP_End(p_wk->p_nhttp);
          p_wk->p_nhttp  = NULL;
          p_wk->seq = 0;
        }
        return WIFIBATTLEMATCH_NET_EVILCHECK_RET_ERROR;
      }
    }
    break;

  case SEQ_END:
    { 
      int i;
      void *p_buff;
      const s8 *cp_sign;
      p_buff  = NHTTP_RAP_GetRecvBuffer(p_wk->p_nhttp);

      p_data->status_code  = NHTTP_RAP_EVILCHECK_GetStatusCode( p_buff );
      
      for( i = 0; i < p_wk->poke_max; i++ )
      { 
        p_data->poke_result[i] = NHTTP_RAP_EVILCHECK_GetPokeResult( p_buff, i );
      }

      if( p_data->status_code == 0 )
      { 
        NAGI_Printf( "不正チェック通過してサインもらいました！\n" );
        cp_sign  = NHTTP_RAP_EVILCHECK_GetSign( p_buff, p_wk->poke_max );
        GFL_STD_MemCopy( cp_sign, p_data->sign, NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN );
#ifdef PM_DEBUG
        { 
          int i;
          for( i = 0; i < NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN; i++ )
          { 
            DEBUG_NET_Printf( "%d ", p_data->sign[i] );
            if(i % 0x10 == 0xF )
            { 
              DEBUG_NET_Printf( "\n" );
            }
          }
        }
#endif
      }

      NHTTP_RAP_PokemonEvilCheckDelete(p_wk->p_nhttp);
      if(p_wk->p_nhttp)
      {
        NHTTP_RAP_End(p_wk->p_nhttp);
        p_wk->p_nhttp  = NULL;
      }
    }
    return WIFIBATTLEMATCH_NET_EVILCHECK_RET_SUCCESS;
  }

  return WIFIBATTLEMATCH_NET_EVILCHECK_RET_UPDATE;
}
//=============================================================================
/**
 *    エラー処理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  エラーをクリア
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_Clear( WIFIBATTLEMATCH_NETERR_WORK *p_wk )
{ 
  GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_NETERR_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラーメイン
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_Main( WIFIBATTLEMATCH_NETERR_WORK *p_wk )
{ 

}
//----------------------------------------------------------------------------
/**
 *	@brief  システムエラーを設定
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 *	@param	type                              システムエラーの種類
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_SetSysError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, WIFIBATTLEMATCH_NET_SYSERROR error, int line )
{ 
  DEBUG_NET_Printf( "!!!Sys Error!!!=%d line=%d\n\n", error, line );
  p_wk->sys_err = error;
  p_wk->type    = WIFIBATTLEMATCH_NET_ERRORTYPE_SYS;
  
  GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_USER_TIMEOUT );
}
//----------------------------------------------------------------------------
/**
 *	@brief  SCエラーを設定
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 *	@param	sc_err                            SCエラーの種類
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_SetScError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCScResult sc_err, int line )
{ 
  DEBUG_NET_Printf( "!!!Sc Error!!!=%d line=%d\n", sc_err, line );
  p_wk->sc_err  = sc_err;
  p_wk->type    = WIFIBATTLEMATCH_NET_ERRORTYPE_SC;
}
//----------------------------------------------------------------------------
/**
 *	@brief  GDBエラーを設定
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 *	@param	gdb_err                           GDBエラーの種類
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_SetGdbError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCGdbError gdb_err, int line )
{ 
  DEBUG_NET_Printf( "!!!Gdb Error!!=%d line=%d\n", gdb_err, line );
  p_wk->gdb_err = gdb_err;
  p_wk->type    = WIFIBATTLEMATCH_NET_ERRORTYPE_GDB;
}
//----------------------------------------------------------------------------
/**
 *	@brief  GDBの状態を設定
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 *	@param	state                             GDBの状態
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_SetGdbState( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCGdbState state, int line )
{ 
  DEBUG_NET_Printf( "!!!Gdb State!!=%d line=%d\n", state, line );
  p_wk->gdb_state = state;
  p_wk->type      = WIFIBATTLEMATCH_NET_ERRORTYPE_GDB;
}
//----------------------------------------------------------------------------
/**
 *	@brief  GDBの結果を設定
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 *	@param	state                             GDBの結果
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_SetGdbResult( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCGdbAsyncResult result, int line )
{ 
  DEBUG_NET_Printf( "!!!Gdb Result!!=%d err=%d line=%d\n", result, DWC_GetLastError(NULL), line );
  p_wk->gdb_result  = result;
  p_wk->type        = WIFIBATTLEMATCH_NET_ERRORTYPE_GDB;
}
//----------------------------------------------------------------------------
/**
 *	@brief  NDエラーを設定
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 *	@param	nd_err                            エラー
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_SetNdError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, DWCNdError nd_err, int line )
{ 
  DEBUG_NET_Printf( "!!!ND Error!!=%d line=%d\n", nd_err, line );
  p_wk->nd_err  = nd_err;
  p_wk->type    = WIFIBATTLEMATCH_NET_ERRORTYPE_ND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  NHTTPエラーを設定
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk   ワーク
 *	@param	nhttp_err                           NHTTPエラー
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_SetNhttpError( WIFIBATTLEMATCH_NETERR_WORK *p_wk, NHTTPError nhttp_err, int line )
{ 
  DEBUG_NET_Printf( "!!!NHTTP Error!!=%d line=%d\n", nhttp_err, line );
  p_wk->nhttp_err = nhttp_err;
  p_wk->type    = WIFIBATTLEMATCH_NET_ERRORTYPE_NHTTP;
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
static WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_SC_GetErrorRepairType( DWCScResult error )
{ 
//  DEBUG_NET_Printf( "WIFIBATTLEMATCH_GDB_GetErrorRepairType err%d \n", error);

  switch( error )
  { 
  case DWC_SC_RESULT_NO_ERROR :              ///< エラーは発生しませんでした
    return WIFIBATTLEMATCH_NET_ERROR_NONE;

  case DWC_SC_RESULT_NO_AVAILABILITY_CHECK:     ///< 初期化の前に、標準の GameSpy 可用性チェックが実行されませんでした
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_INVALID_PARAMETERS:        ///< インターフェース関数に渡されたパラメータが無効です
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_NOT_INITIALIZED:           ///< SDK が初期化されませんでした
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_CORE_NOT_INITIALIZED:      ///< コアが初期化されませんでした
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_OUT_OF_MEMORY:             ///< SDK はリソースにメモリを割り当てられませんでした
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_CALLBACK_PENDING:          ///< アプリケーションに対し、操作が保留中であることを伝えます
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_HTTP_ERROR:                ///< バックエンドが適正な HTTP によるレスポンスに失敗した場合のエラー発生
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_UNKNOWN_RESPONSE:          ///< SDK が結果を理解できない場合のエラー発生
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_RESPONSE_INVALID:          ///< SDK がバックエンドからのレスポンスを読み取れない場合のエラー発生
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  case DWC_SC_RESULT_INVALID_DATATYPE:          ///< 無効なデータタイプ
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;

  case DWC_SC_RESULT_REPORT_INCOMPLETE:         ///< レポートが不完全です
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;

  case DWC_SC_RESULT_REPORT_INVALID:            ///< レポートの一部またはすべてが無効です
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;

  case DWC_SC_RESULT_SUBMISSION_FAILED:         ///< レポートの送信に失敗しました
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;

  case DWC_SC_RESULT_UNKNOWN_ERROR:             ///< SDK が認識できないエラー
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

  default:
    GF_ASSERT(0);
    return 0;
  }
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
static WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_GDB_GetErrorRepairType( DWCGdbError error, DWCGdbState gdb_state, DWCGdbAsyncResult gdb_result )
{ 
 // DEBUG_NET_Printf( "WIFIBATTLEMATCH_GDB_GetErrorRepairType err%d state%d resul%d\n", error, gdb_state, gdb_result );

  //-------------------------------------
  ///	エラーのチェック
  //=====================================
  switch( error )
  { 
    case DWC_GDB_ERROR_NONE :                 ///< 正常に終了しました。
      //  resukt と stateのチェック
      break;

    case DWC_GDB_ERROR_OUT_OF_MEMORY :        ///< case DWC_Initで与えたアロケータからメモリを確保できませんでした。
    case DWC_GDB_ERROR_NOT_AVAILABLE :        ///< 利用可能ではない
    case DWC_GDB_ERROR_CORE_SHUTDOWN :        ///< 
    case DWC_GDB_ERROR_NOT_LOGIN :            ///< WiFiConnectionに正しくログインしていません。
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
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;

    case DWC_GDB_ERROR_UNKNOWN :             ///<
    case DWC_GDB_ERROR_ALREADY_INITIALIZED : ///< 既に初期化されています。
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;

    case DWC_GDB_ERROR_NOT_INITIALIZED :     ///< 初期化が行われていません。
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;

    case DWC_GDB_ERROR_IN_ASYNC_PROCESS :    ///< 非同期処理中です。
      //  resukt と stateのチェック
      break;

    case DWC_GDB_ERROR_GHTTP :               ///< GHTTPのエラーが発生しました。
    case DWC_GDB_ERROR_IN_DWC_ERROR :         ///< DWCのエラーが起こっています。エラーの対処を行ってください。
      return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  }

  //-------------------------------------
  ///	状態のチェック
  //=====================================
  switch( gdb_state )
  { 
  case DWC_GDB_STATE_UNINITIALIZED:    ///< 未初期化状態
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN; 
  case DWC_GDB_STATE_IDLE:             ///< 初期化が終了し、非同期処理が行われていない状態
      //  stateのチェック
    break;
  case DWC_GDB_STATE_IN_ASYNC_PROCESS: ///< 非同期処理を行っている状態
      //  stateのチェック
    break;
  case DWC_GDB_STATE_ERROR_OCCURED:     ///< エラーが発生した状態
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT; 
  }

  //-------------------------------------
  ///	結果のチェック
  //=====================================
  switch( gdb_result )
  { 
  case DWC_GDB_ASYNC_RESULT_NONE:                  ///< 非同期処理がまだ終了していません。
    // 正常
    break;
  case DWC_GDB_ASYNC_RESULT_SUCCESS:               ///< 成功しました。
    // 正常
    break;
  case DWC_GDB_ASYNC_RESULT_SECRET_KEY_INVALID:   ///< 無効な秘密鍵
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_GDB_ASYNC_RESULT_SERVICE_DISABLED:     ///< サービスが無効
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_GDB_ASYNC_RESULT_CONNECTION_TIMEOUT:   ///< タイムアウト
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;
  case DWC_GDB_ASYNC_RESULT_CONNECTION_ERROR:     ///< 接続エラー
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_GDB_ASYNC_RESULT_MALFORMED_RESPONSE:   ///< 間違った形式の応答
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_GDB_ASYNC_RESULT_OUT_OF_MEMORY:        ///< メモリ不足
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_GDB_ASYNC_RESULT_DATABASE_UNAVAILABLE: ///< データベースが使用できない
  case DWC_GDB_ASYNC_RESULT_LOGIN_TICKET_INVALID: ///< ログインチケットが無効
  case DWC_GDB_ASYNC_RESULT_LOGIN_TICKET_EXPIRED: ///< ログインチケットが期限切れ
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_GDB_ASYNC_RESULT_TABLE_NOT_FOUND:      ///< テーブルが見つからない
  case DWC_GDB_ASYNC_RESULT_RECORD_NOT_FOUND:     ///< レコードが見つからない
  case DWC_GDB_ASYNC_RESULT_FIELD_NOT_FOUND:      ///< フィールド名が見つからない
  case DWC_GDB_ASYNC_RESULT_FIELD_TYPE_INVALID:   ///< フィールドタイプが不正
  case DWC_GDB_ASYNC_RESULT_NO_PERMISSION:        ///< パーミッションが異なる
  case DWC_GDB_ASYNC_RESULT_RECORD_LIMIT_REACHED: ///< レコード上限に達した
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;
  case DWC_GDB_ASYNC_RESULT_ALREADY_RATED:        ///< 既に行った
  case DWC_GDB_ASYNC_RESULT_NOT_RATEABLE:         ///< 評価ではない
  case DWC_GDB_ASYNC_RESULT_NOT_OWNED:            ///< 所有者ではない
  case DWC_GDB_ASYNC_RESULT_FILTER_INVALID:       ///< フィルタが無効
  case DWC_GDB_ASYNC_RESULT_SORT_INVALID:         ///< ソートが無効
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;
  case DWC_GDB_ASYNC_RESULT_UNKNOWN_ERROR:        ///< 不明なエラー
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_GDB_ASYNC_RESULT_BAD_HTTP_METHOD:      ///< HTTPの方式が悪い
  case DWC_GDB_ASYNC_RESULT_BAD_FILE_COUNT:       ///< ファイル数が悪い
  case DWC_GDB_ASYNC_RESULT_MISSING_PARAMETER:    ///< パラメータが欠けている
  case DWC_GDB_ASYNC_RESULT_FILE_NOT_FOUND:       ///< ファイルが見つからない
  case DWC_GDB_ASYNC_RESULT_FILE_TOO_LARGE:       ///< ファイルが巨大
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;
  case DWC_GDB_ASYNC_RESULT_FILE_UNKNOWN_ERROR:   ///< ファイルの不明なエラー
  case DWC_GDB_ASYNC_RESULT_GHTTP_ERROR:          ///< GHTTPのエラーが発生しました。
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_GDB_ASYNC_RESULT_TARGET_FILTER_INVALID:///< ターゲットフィルタが無効
  case DWC_GDB_ASYNC_RESULT_SERVER_ERROR:         ///< サーバーエラー
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  }

  return WIFIBATTLEMATCH_NET_ERROR_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラー修正タイプを取得
 *
 *	@param	DWCNdError                  nd_err   エラー
 *
 *	@return エラー修正タイプ
 */
//-----------------------------------------------------------------------------
static WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_ND_GetErrorRepairType( DWCNdError nd_err )
{

  switch( nd_err )
  { 
  case DWC_ND_ERROR_NONE:     ///< 実行中の処理が完了した
    return WIFIBATTLEMATCH_NET_ERROR_NONE;
  case DWC_ND_ERROR_ALLOC:    ///< メモリ確保に失敗した
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_ND_ERROR_BUSY:     ///< 本来その関数を呼び出せないSTATEで呼び出した
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_ND_ERROR_HTTP:     ///< HTTP通信にエラーが発生した
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_ND_ERROR_BUFFULL:  ///< (内部エラー)ダウンロードバッファが足りない
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_ND_ERROR_PARAM:	   ///< 不正なパラメータによるエラーが発生した
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;
  case DWC_ND_ERROR_CANCELED: ///< 処理のキャンセルが完了した
    return WIFIBATTLEMATCH_NET_ERROR_NONE;
  case DWC_ND_ERROR_DLSERVER: ///< サーバがエラーコードを返してきた
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  case DWC_ND_ERROR_FATAL:	   ///< 未定義の致命的なエラーが発生した
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_FATAL;
  default:
    return WIFIBATTLEMATCH_NET_ERROR_NONE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラー修正タイプを取得
 *
 *	@param	DWCNdError                  nd_err   エラー
 *
 *	@return エラー修正タイプ
 */
//-----------------------------------------------------------------------------
static WIFIBATTLEMATCH_NET_ERROR_REPAIR_TYPE WIFIBATTLEMATCH_SYS_GetErrorRepairType( WIFIBATTLEMATCH_NET_SYSERROR  sys_err )
{ 
  switch( sys_err )
  { 
  case WIFIBATTLEMATCH_NET_SYSERROR_NONE:    //エラーは発生していない
    return WIFIBATTLEMATCH_NET_ERROR_NONE;
  case WIFIBATTLEMATCH_NET_SYSERROR_TIMEOUT: //アプリケーションタイムアウトが発生した
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_RETURN;
  case WIFIBATTLEMATCH_NET_SYSERROR_NO_RECORD:  //レコードIDが取得できなかった
    return WIFIBATTLEMATCH_NET_ERROR_REPAIR_DISCONNECT;
  default:
    return WIFIBATTLEMATCH_NET_ERROR_NONE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  エラーをクリア
 *
 *	@param	WIFIBATTLEMATCH_NETERR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void WIFIBATTLEMATCH_NETERR_ClearError( WIFIBATTLEMATCH_NETERR_WORK *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_NETERR_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信アイコンを表示
 *
 *	@param	BOOL is_top  TRUEならば上画面  FALSE下画面
 */
//-----------------------------------------------------------------------------
void WIFIBATTLEMATCH_NETICON_SetDraw( WIFIBATTLEMATCH_NET_WORK *p_wk, BOOL is_visible )
{ 
  if( is_visible )
  { 
    GFL_NET_ReloadIconTopOrBottom( TRUE, p_wk->heapID );
  }
  else
  { 
    GFL_NET_WirelessIconEasyEnd();
  }
}
