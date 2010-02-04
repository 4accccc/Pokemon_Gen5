//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		dwcrap_sc_gdb.c
 *	@brief  DWC統計・競争＆データベースラップルーチン
 *	@author	Toru=Nagihashi
 *	@data		2009.11.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include <dwc.h>

//ネットワーク
#include "net/network_define.h"

//アトラス情報
#include "atlas_syachi2ds_v1.h"

//外部公開
#include "dwcrap_sc_gdb.h"

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

#if defined(DEBUG_NET_PRINT_ON) | defined(DEBUG_ONLY_FOR_toru_nagihashi)
#define DEBUG_NET_Printf(...)  OS_FPrintf(3,__VA_ARGS__)
#else
#define DEBUG_NET_Printf(...)  /*  */
#endif  //DEBUG_NET_PRINT_ON

//-------------------------------------
///	SCのシーケンス
//=====================================
typedef enum 
{
  DWCRAP_SC_SEQ_INIT,
  DWCRAP_SC_SEQ_LOGIN,
  DWCRAP_SC_SEQ_SESSION_START,
  DWCRAP_SC_SEQ_SESSION_WAIT,
  DWCRAP_SC_SEQ_SEND_PLAYERDATA_TIMING_START,
  DWCRAP_SC_SEQ_SEND_PLAYERDATA_TIMING_WAIT,
  DWCRAP_SC_SEQ_SEND_PLAYERDATA,
  DWCRAP_SC_SEQ_INTENTION_START,
  DWCRAP_SC_SEQ_INTENTION_WAIT,
  DWCRAP_SC_SEQ_RETURN_PLAYERDATA_TIMING_START,
  DWCRAP_SC_SEQ_RETURN_PLAYERDATA_TIMING_WAIT,
  DWCRAP_SC_SEQ_SEND_PLAYERDATA_CCID_PARENT,
  DWCRAP_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD,
  DWCRAP_SC_SEQ_CREATE_REPORT_TIMING_START,
  DWCRAP_SC_SEQ_CREATE_REPORT_TIMING_WAIT,
  DWCRAP_SC_SEQ_CREATE_REPORT,
  DWCRAP_SC_SEQ_SUBMIT_REPORT_START,
  DWCRAP_SC_SEQ_SUBMIT_REPORT_WAIT,
  DWCRAP_SC_SEQ_EXIT,
  DWCRAP_SC_SEQ_END,
} DWCRAP_SC_SEQ;

//-------------------------------------
///	GDBのシーケンス
//=====================================
typedef enum 
{
  DWCRAP_GDB_SEQ_INIT,
  DWCRAP_GDB_SEQ_GET_START,
  DWCRAP_GDB_SEQ_GET_WAIT,
  DWCRAP_GDB_SEQ_GET_END,
  DWCRAP_GDB_SEQ_EXIT,
  DWCRAP_GDB_SEQ_END,
} DWCRAP_GDB_SEQ;

//-------------------------------------
///	タイミング
//=====================================
#define DWCRAP_SC_SEND_PLAYERDATA_TIMING    (1)
#define DWCRAP_SC_RETURN_PLAYERDATA_TIMING  (2)
#define DWCRAP_SC_REPORT_TIMING    (3)


//-------------------------------------
///	その他
//=====================================
#define TIMEOUT_MS   100  // HTTP通信のタイムアウト時間
#define PLAYER_NUM   2          // プレイヤー数
#define TEAM_NUM     0          // チーム数

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
  int dummy;
} DWC_SC_WRITE_DATA;


//-------------------------------------
///	ワーク
//=====================================
struct _DWCRAP_SC_GDB_WORK
{ 
  u32   seq;
  BOOL  is_gdb_start;
  BOOL  is_sc_start;
  u32   wait_cnt;
  
  DWC_SC_PLAYERDATA player[2];  //自分は０ 相手は１
  BOOL              is_recv;
  DWCScResult       result;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	SC関係
//=====================================
//DWCコールバック
static void DwcRap_Sc_CreateSessionCallback( DWCScResult theResult, void* theUserData );
static void DwcRap_Sc_SetReportIntentionCallback( DWCScResult theResult, void* theUserData );
static void DwcRap_Sc_SubmitReportCallback( DWCScResult theResult, void* theUserData );
//private
static void DwcRap_Sc_Finalize( DWCRAP_SC_GDB_WORK *p_wk );
static DWCScResult DwcRap_Sc_CreateReport( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const DWC_SC_WRITE_DATA *cp_data );
//GFL_NETコールバック
static void DwcRap_Sc_RecvPlayerData(const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_net_handle);
//-------------------------------------
///	GDB関係
//=====================================
static void DwcRap_Gdb_GetRecordsCallback(int record_num, DWCGdbField** records, void* user_param);
static void DwcRap_Gdb_Finalize( DWCRAP_SC_GDB_WORK *p_wk );
#ifdef PM_DEBUG
static void print_field(DWCGdbField* field); // レコードをデバッグ出力する。
#endif

//=============================================================================
/**
 *          データ
 */
//=============================================================================
//-------------------------------------
///	通信コマンド
//=====================================
enum
{ 
  DWCRAP_SC_NETCMD_SEND_PLAYERDATA  = GFL_NET_CMD_WIFIMATCH,
  DWCRAP_SC_NETCMD_MAX,
};
static const NetRecvFuncTable sc_net_recv_tbl[DWCRAP_SC_NETCMD_MAX] = 
{
  {DwcRap_Sc_RecvPlayerData,   NULL},    ///_NETCMD_INFOSEND
};

//-------------------------------------
///	SAKEサーバーに問い合わせるためのキー名
//=====================================
static const char* sc_field_names[] = 
{
  "ARENA_ELO_RATING_1V1",
  "TOTAL_MATCHES_COUNTER",
  "DISCONNECTS_COUNTER",
  "COMPLETE_MATCHES_COUNTER",
  "DISCONNECT_RATE",
  "NUM_SINGLE_MATCH_COUNTER",
  "ARENA_ELO_RATING_1V1_DOUBLE",
};

//=============================================================================
/**
 *    ワーク作成・破棄
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
DWCRAP_SC_GDB_WORK * DWCRAP_SC_GDB_Init( HEAPID heapID )
{ 
  DWCRAP_SC_GDB_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(DWCRAP_SC_GDB_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(DWCRAP_SC_GDB_WORK) );

  if(!GFL_NET_IsInit())
  {
    GF_ASSERT(0);
  }

  GFL_NET_AddCommandTable( GFL_NET_CMD_WIFIMATCH, sc_net_recv_tbl, DWCRAP_SC_NETCMD_MAX, p_wk );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	DWCRAP_SC_GDB_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void DWCRAP_SC_GDB_Exit( DWCRAP_SC_GDB_WORK *p_wk )
{ 
  GFL_NET_DelCommandTable( GFL_NET_CMD_WIFIMATCH );
  GFL_HEAP_FreeMemory( p_wk );
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
 *	@param	DWCRAP_SC_GDB_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void DWCRAP_SC_Start( DWCRAP_SC_GDB_WORK *p_wk )
{ 
  if( GFL_NET_IsParentMachine() )
  { 
    NAGI_Printf( "！親機\n" );
  }
  else
  { 
    NAGI_Printf( "！子機\n" );
  }

  p_wk->is_sc_start = TRUE;
  p_wk->seq = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  SCの処理中
 *
 *	@param	DWCRAP_SC_GDB_WORK *p_wk  ワーク
 *	@param  DWCScResult               結果
 *	@retval TRUEならば処理終了  FALSEならば処理進行中or不正エラー
 */
//-----------------------------------------------------------------------------
BOOL DWCRAP_SC_Process( DWCRAP_SC_GDB_WORK *p_wk, const DWCUserData *cp_user_data, DWCScResult *p_result )
{ 
  DWCScResult ret;
  
  *p_result = DWC_SC_RESULT_NO_ERROR;

  //@todo 失敗したら相手に失敗を送る必要がある？
  if( p_wk->is_sc_start )
  { 
    switch( p_wk->seq )
    { 
    //初期化
    case DWCRAP_SC_SEQ_INIT:
      p_wk->wait_cnt  = 0;
      { 
        ret = DWC_GdbInitialize( GAME_ID, cp_user_data, DWC_GDB_SSL_TYPE_NONE );
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
      p_wk->seq = DWCRAP_SC_SEQ_LOGIN;
      break;

    //認証情報取得
    case DWCRAP_SC_SEQ_LOGIN:
      ret = DWC_ScGetLoginCertificate( &p_wk->player[0].mCertificate );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "SC:Login\n" );
      p_wk->seq = DWCRAP_SC_SEQ_SESSION_START;
      break;

    //セッション開始  （ホストが行い、クライアントにセッションIDを伝播する）
    case DWCRAP_SC_SEQ_SESSION_START:
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
        p_wk->seq = DWCRAP_SC_SEQ_SESSION_WAIT;
      }
      else
      { 
        //クライアントはセッションを行わず次へ
        DEBUG_NET_Printf( "SC:Session child\n" );
        p_wk->seq = DWCRAP_SC_SEQ_SEND_PLAYERDATA_TIMING_START;
      }
      break;

    //セッション終了待ち
    case DWCRAP_SC_SEQ_SESSION_WAIT:
      if( p_wk->wait_cnt == 0 )
      { 
        DEBUG_NET_Printf( "SC:Session process parent\n" );
        GFL_STD_MemCopy( DWC_ScGetSessionId(), p_wk->player[0].mSessionId, DWC_SC_SESSION_GUID_SIZE );
        p_wk->seq = DWCRAP_SC_SEQ_SEND_PLAYERDATA_TIMING_START;
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
   case DWCRAP_SC_SEQ_SEND_PLAYERDATA_TIMING_START:
      GFL_STD_MemCopy( DWC_ScGetConnectionId(), p_wk->player[0].mConnectionId, DWC_SC_CONNECTION_GUID_SIZE );

      GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_SEND_PLAYERDATA_TIMING, WB_NET_WIFIMATCH );
      p_wk->seq  = DWCRAP_SC_SEQ_SEND_PLAYERDATA_TIMING_WAIT;
      break;

    //タイミング待ち
    case DWCRAP_SC_SEQ_SEND_PLAYERDATA_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_SEND_PLAYERDATA_TIMING, WB_NET_WIFIMATCH ) )
      { 
        p_wk->seq = DWCRAP_SC_SEQ_SEND_PLAYERDATA;
      }
      break;

    //ホストがクライアントへ自分のデータを送る
    case DWCRAP_SC_SEQ_SEND_PLAYERDATA:
      if( GFL_NET_IsParentMachine() )
      { 
        //自分を送信
        if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_NETCMD_SEND_PLAYERDATA, 
              sizeof(DWC_SC_PLAYERDATA), &p_wk->player[0]) )
        { 
          DEBUG_NET_Printf( "SC:Senddata parent\n" );
          p_wk->seq = DWCRAP_SC_SEQ_INTENTION_START;
        }
      }
      else
      { 
        //受け取り
        if( p_wk->is_recv )
        { 
          p_wk->is_recv = FALSE;
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
          p_wk->seq = DWCRAP_SC_SEQ_INTENTION_START;
        }
      }
      break;

    //送信するレポートの種類を通知
    case DWCRAP_SC_SEQ_INTENTION_START:
      p_wk->wait_cnt++;
      //第一引数は親のみ正しいという値なのか？
      ret = DWC_ScSetReportIntentionAsync( GFL_NET_IsParentMachine(), DwcRap_Sc_SetReportIntentionCallback, TIMEOUT_MS, p_wk );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      } 
      DEBUG_NET_Printf( "SC:intention start\n" );
      p_wk->seq = DWCRAP_SC_SEQ_INTENTION_WAIT;
      break;

    //通知待ち
    case DWCRAP_SC_SEQ_INTENTION_WAIT:
      if( p_wk->wait_cnt == 0 )
      { 
        DEBUG_NET_Printf( "SC:intention_wait\n" );
        p_wk->seq = DWCRAP_SC_SEQ_RETURN_PLAYERDATA_TIMING_START;
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
    case DWCRAP_SC_SEQ_RETURN_PLAYERDATA_TIMING_START:
      GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_RETURN_PLAYERDATA_TIMING, WB_NET_WIFIMATCH );
      p_wk->seq  = DWCRAP_SC_SEQ_RETURN_PLAYERDATA_TIMING_WAIT;
      break;

    //タイミング待ち
    case DWCRAP_SC_SEQ_RETURN_PLAYERDATA_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_RETURN_PLAYERDATA_TIMING, WB_NET_WIFIMATCH ) )
      { 
        p_wk->seq = DWCRAP_SC_SEQ_SEND_PLAYERDATA_CCID_PARENT;
      }
      break;

    //お互いのCCID（Intention ConnectionID）を交換する  まずは親データを子に送信
    case DWCRAP_SC_SEQ_SEND_PLAYERDATA_CCID_PARENT:
      if( GFL_NET_IsParentMachine() )
      { 
        if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_NETCMD_SEND_PLAYERDATA, 
              sizeof(DWC_SC_PLAYERDATA), &p_wk->player[0]) )
        { 
          DEBUG_NET_Printf( "SC:send data ccid parent\n" );
          p_wk->seq = DWCRAP_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD;
        }
      }
      else
      { 
        if( p_wk->is_recv )
        { 
          p_wk->is_recv = FALSE;
          DEBUG_NET_Printf( "SC:send data ccid parent\n" );
          p_wk->seq = DWCRAP_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD;
        }
      }
      break;

    //次は子のデータを送信
    case DWCRAP_SC_SEQ_SEND_PLAYERDATA_CCID_CHILD:
      if( !GFL_NET_IsParentMachine() )
      { 
        if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_NETCMD_SEND_PLAYERDATA, 
              sizeof(DWC_SC_PLAYERDATA), &p_wk->player[0]) )
        { 
          DEBUG_NET_Printf( "SC:send data ccid child\n" );
          p_wk->seq = DWCRAP_SC_SEQ_CREATE_REPORT_TIMING_START;
        }
      }
      else
      { 
        if( p_wk->is_recv )
        { 
          p_wk->is_recv = FALSE;
          DEBUG_NET_Printf( "SC:send data ccid child\n" );
          p_wk->seq = DWCRAP_SC_SEQ_CREATE_REPORT_TIMING_START;
        }
      }
      break;

    //受信後、レポート作成のためのタイミングとり
    case DWCRAP_SC_SEQ_CREATE_REPORT_TIMING_START:
      GFL_NET_HANDLE_TimeSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_REPORT_TIMING, WB_NET_WIFIMATCH );
      p_wk->seq  = DWCRAP_SC_SEQ_CREATE_REPORT_TIMING_WAIT;
      break;

    //タイミングまち
    case DWCRAP_SC_SEQ_CREATE_REPORT_TIMING_WAIT:
      if( GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(), DWCRAP_SC_REPORT_TIMING, WB_NET_WIFIMATCH ) )
      { 
        p_wk->seq = DWCRAP_SC_SEQ_CREATE_REPORT;
      }
      break;

    //レポート作成
    case DWCRAP_SC_SEQ_CREATE_REPORT:
      //レポート作成
      ret = DwcRap_Sc_CreateReport( &p_wk->player[0], &p_wk->player[1], NULL );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "SC:create report\n" );
      p_wk->seq = DWCRAP_SC_SEQ_SUBMIT_REPORT_START;
      break;

    //レポート送信開始
    case DWCRAP_SC_SEQ_SUBMIT_REPORT_START:
      p_wk->wait_cnt++;
      ret = DWC_ScSubmitReportAsync( p_wk->player[0].mReport, GFL_NET_IsParentMachine(),
            DwcRap_Sc_SubmitReportCallback, TIMEOUT_MS, p_wk );
      if( ret != DWC_SC_RESULT_NO_ERROR )
      { 
        *p_result = ret;
        p_wk->is_sc_start  = FALSE;
        DwcRap_Sc_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "SC:submit report start\n" );
      p_wk->seq = DWCRAP_SC_SEQ_SUBMIT_REPORT_WAIT;
      break;

    //レポート送信中
    case DWCRAP_SC_SEQ_SUBMIT_REPORT_WAIT:
      if( p_wk->wait_cnt == 0 )
      { 
        DEBUG_NET_Printf( "SC:submit report wait\n" );
        p_wk->seq = DWCRAP_SC_SEQ_EXIT;
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
    case DWCRAP_SC_SEQ_EXIT:
      DEBUG_NET_Printf( "SC:exit\n" );
      DwcRap_Sc_Finalize( p_wk );
      p_wk->seq = DWCRAP_SC_SEQ_END;
      break;

    case DWCRAP_SC_SEQ_END:
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
  DWCRAP_SC_GDB_WORK  *p_wk = theUserData;

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
  DWCRAP_SC_GDB_WORK  *p_wk = theUserData;
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
  DWCRAP_SC_GDB_WORK  *p_wk = theUserData;

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
 *	@param	DWCRAP_SC_GDB_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Sc_Finalize( DWCRAP_SC_GDB_WORK *p_wk )
{
  if( p_wk->seq > DWCRAP_SC_SEQ_CREATE_REPORT )
  {
    DEBUG_NET_Printf( "destroy report\n" );
    DWC_ScDestroyReport( p_wk->player[0].mReport );
  }

  if( p_wk->seq >= DWCRAP_SC_SEQ_INIT )
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
static DWCScResult DwcRap_Sc_CreateReport( DWC_SC_PLAYERDATA *p_my, DWC_SC_PLAYERDATA *p_other, const DWC_SC_WRITE_DATA *cp_data )
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

  ret = DWC_ScReportSetPlayerData(
                p_my->mReport,
                0,
                p_my->mConnectionId,
                0,
                GFL_NET_IsParentMachine() ? DWC_SC_GAME_RESULT_WIN : DWC_SC_GAME_RESULT_LOSS,
                p_my->mCertificate,
                p_my->mStatsAuthdata );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:SetData%d\n",ret );
    return ret;
  }

  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1, 1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:AddInt%d\n",ret );
    return ret;
  }
  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_DOUBLE, 1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:AddInt%d\n",ret );
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SINGLE_MATCH_COUNTER, 1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:AddInt%d\n",ret );
    return ret;
  }
  ret = DWC_ScReportAddStringValue( p_my->mReport,  ARENA_FFA_PLACE, GFL_NET_IsParentMachine() ? "1" : "2" );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:AddInt%d\n",ret );
    return ret;
  }

  //相手のレポートを作成    相手のなのでデータは逆になる
  ret = DWC_ScReportBeginNewPlayer( p_my->mReport );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:New%d\n",ret );
    return ret;
  }

  ret = DWC_ScReportSetPlayerData(
                p_my->mReport,
                1,
                p_other->mConnectionId,
                0,
                GFL_NET_IsParentMachine() ? DWC_SC_GAME_RESULT_LOSS : DWC_SC_GAME_RESULT_WIN,
                p_other->mCertificate,
                p_other->mStatsAuthdata );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:SetData%d\n",ret );
    return ret;
  }

  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1, 1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:AddInt%d\n",ret );
    return ret;
  }
  ret = DWC_ScReportAddByteValue( p_my->mReport, KEY_ARENA_GAMETYPE_1V1_DOUBLE, 1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:AddInt%d\n",ret );
    return ret;
  }
  ret = DWC_ScReportAddIntValue( p_my->mReport, KEY_NUM_SINGLE_MATCH_COUNTER, 1 );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:AddInt%d\n",ret );
    return ret;
  }
  ret = DWC_ScReportAddStringValue( p_my->mReport, ARENA_FFA_PLACE, GFL_NET_IsParentMachine() ? "2" : "1" );
  if( ret != DWC_SC_RESULT_NO_ERROR )
  { 
    DEBUG_NET_Printf( "Report:AddInt%d\n",ret );
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
  ret = DWC_ScReportEnd( p_my->mReport, GFL_NET_IsParentMachine(), DWC_SC_GAME_STATUS_COMPLETE );
  return ret;
}


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
  DWCRAP_SC_GDB_WORK *p_wk  = p_wk_adrs;

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
  p_wk->is_recv = TRUE;
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
 *	@param	DWCRAP_SC_GDB_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void DWCRAP_GDB_Start( DWCRAP_SC_GDB_WORK *p_wk )
{ 
  p_wk->is_gdb_start  = TRUE;
  p_wk->seq = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  DBの処理中
 *
 *	@param	DWCRAP_SC_GDB_WORK *p_wk  ワーク
 *	@param  DWCScResult               結果
 *	@retval TRUEならば処理終了  FALSEならば処理進行中or不正エラー
 */
//-----------------------------------------------------------------------------
BOOL DWCRAP_GDB_Process( DWCRAP_SC_GDB_WORK *p_wk, const DWCUserData *cp_user_data, DWCGdbError *p_result )
{ 
  DWCError    error;
  DWCGdbState state;

  *p_result = DWC_GDB_ERROR_NONE;

  if( p_wk->is_gdb_start )  
  { 
    switch( p_wk->seq )
    { 
    case DWCRAP_GDB_SEQ_INIT:
      //@todo製品版は DWC_GDB_SSL_TYPE_SERVER_AUTH
      *p_result = DWC_GdbInitialize( GAME_ID, cp_user_data, DWC_GDB_SSL_TYPE_NONE );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "GDB:Init\n" );
      p_wk->seq = DWCRAP_GDB_SEQ_GET_START;
      break;

    case DWCRAP_GDB_SEQ_GET_START:
      *p_result = DWC_GdbGetMyRecordsAsync( "PlayerStats_v1", sc_field_names, NELEMS(sc_field_names), DwcRap_Gdb_GetRecordsCallback, p_wk );
      if( *p_result != DWC_GDB_ERROR_NONE )
      { 
        p_wk->is_gdb_start  = FALSE;
        DwcRap_Gdb_Finalize( p_wk );
        return FALSE;
      }
      DEBUG_NET_Printf( "GDB:Get start\n" );
      p_wk->seq= DWCRAP_GDB_SEQ_GET_WAIT;
      break;

    case DWCRAP_GDB_SEQ_GET_WAIT:
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
            p_wk->seq = DWCRAP_GDB_SEQ_GET_END;
          }
        }
      }
      break;

    case DWCRAP_GDB_SEQ_GET_END:
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
      p_wk->seq = DWCRAP_GDB_SEQ_EXIT;
      break;

    case DWCRAP_GDB_SEQ_EXIT:
      DwcRap_Gdb_Finalize( p_wk );
      DEBUG_NET_Printf( "GDB:Get exit\n" );
      p_wk->seq = DWCRAP_GDB_SEQ_END;
      break;

    case DWCRAP_GDB_SEQ_END:
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了時のシステム破棄関数
 *
 *	@param	DWCRAP_SC_GDB_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DwcRap_Gdb_Finalize( DWCRAP_SC_GDB_WORK *p_wk )
{
  DEBUG_NET_Printf( "GDB_ShotDown!" );
  DWC_GdbShutdown();           // 簡易データベースライブラリの終了
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

    for (i = 0; i < record_num; i++)
    {
        DEBUG_NET_Printf("!!!=====gdb_Print:======\n");
        for (j = 0; j < NELEMS(sc_field_names); j++)   // user_param -> field_num
        {
            print_field(&records[i][j]);
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
