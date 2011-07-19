//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mb_parent_dataconv_sys.ｃ
 *	@brief  データコンバート用マルチブート親機メイン処理
 *	@author	Toru=Nagihashi
 *	@data		2011.07.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/main.h"
#include "system/machine_use.h"
#include "system/gfl_use.h"
#include "system/net_err.h"
#include "system/ds_system.h"
#include "gamesystem/msgspeed.h"
#include "net/wih.h"
#include "net/network_define.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "savedata/misc.h"
#include "app/app_menu_common.h"
#include "sound/pm_sndsys.h"

#include "arc_def.h"
#include "net_err.naix"
#include "msg/msg_net_err.h"

#include "multiboot/mb_parent_sys.h"
#include "multiboot/mb_comm_sys.h"
#include "multiboot/comm/mbp.h"
#include "multiboot/mb_util_msg.h"
#include "multiboot/mb_local_def.h"
#include "test/ariizumi/ari_debug.h"
#include "debug/debug_str_conv.h"

#include "multiboot/mb_parent_dataconv_sys.h"

FS_EXTERN_OVERLAY(dev_wireless);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///SENDMAIN	
//=====================================
typedef enum
{
  MP_PARENT_SENDMAIN_RET_CONTINUE,
  MP_PARENT_SENDMAIN_RET_COMPLETE,
  MP_PARENT_SENDMAIN_RET_CANCEL,
} MP_PARENT_SENDMAIN_RET;

#define MB_PARENT_FIRST_TIMEOUT (60*30) //通常5秒以内で接続するのができなかった。

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メインワーク
//=====================================
struct _MB_PARENT_DATACONV
{
  HEAPID heapID;
  MB_COMM_WORK    *p_comm;
  MBGameRegistry  *p_game_registry;  //MB配信用のデータ
  u32             timeout_cnt;

  u32             seq;
  u32             is_wait;
  u32             is_cancel;
  u32             is_send_rom;
  u32             is_wh_init;
  MB_COMM_INIT_DATA init_data;
  BOOL  (*state)( MB_PARENT_DATACONV *p_wk, u32 *p_seq );

  MB_PARENT_DATACONV_RESULT result; 
};


static BOOL WhCallBackFlg = FALSE;

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//state
static BOOL Mb_Parent_DataConv_State_SendRom( MB_PARENT_DATACONV *p_wk, u32 *p_seq );
static BOOL Mb_Parent_DataConv_State_ConnectRom( MB_PARENT_DATACONV *p_wk, u32 *p_seq );

//
static MBGameRegistry * Mp_Parent_MBGameRegistry_Create( const STRBUF *cp_title_str, const STRBUF *p_info_str, HEAPID heapID );
static void Mp_Parent_MBGameRegistry_Delete( MBGameRegistry *p_registry );
static MP_PARENT_SENDMAIN_RET Mp_Parent_MBGameRegistry_SendMain( MB_PARENT_DATACONV *p_wk );
static BOOL MP_PARENT_WhCallBack( BOOL bResult );

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
MB_PARENT_DATACONV * MB_PARENT_DATACONV_Init( HEAPID heapID )
{
  MB_PARENT_DATACONV *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(MB_PARENT_DATACONV));
  p_wk->heapID = heapID;
  p_wk->p_comm = MB_COMM_CreateSystem( heapID );


  GFL_UI_SleepDisable( GFL_UI_SLEEP_MB );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	MB_PARENT_DATACONV *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MB_PARENT_DATACONV_Exit( MB_PARENT_DATACONV *p_wk )
{
  GFL_UI_SleepEnable( GFL_UI_SLEEP_MB );
  GFL_NET_WirelessIconEasyEnd();

  if( p_wk->p_game_registry != NULL )
  {
    Mp_Parent_MBGameRegistry_Delete( p_wk->p_game_registry );
    p_wk->p_game_registry = NULL;
  }

  MB_COMM_DeleteSystem( p_wk->p_comm );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	MB_PARENT_DATACONV *p_wk　ワーク
 */
//-----------------------------------------------------------------------------
void MB_PARENT_DATACONV_Main( MB_PARENT_DATACONV *p_wk )
{
  MB_COMM_UpdateSystem( p_wk->p_comm );

  if( p_wk->state )
  {
    if( p_wk->state( p_wk, &p_wk->seq ) )
    {
      p_wk->state = NULL;
      p_wk->seq = 0;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リクエスト
 *
 *	@param	MB_PARENT_DATACONV *p_wk  ワーク
 *	@param	req   リクエストの種類
 *
 *	@return　TRUEリクエストを受け付けた FALSEリトライしてください
 */
//-----------------------------------------------------------------------------
BOOL MB_PARENT_DATACONV_ReqestState( MB_PARENT_DATACONV *p_wk, MB_PARENT_DATACONV_REQ req )
{
  switch( req )
  {
  case MB_PARENT_DATACONV_REQ_SEND_ROM: //ROM送信
    GF_ASSERT( p_wk->state == NULL );
    p_wk->state = Mb_Parent_DataConv_State_SendRom;
    break;
  case MB_PARENT_DATACONV_REQ_SEND_ROM_WAIT: //ROM送信停止中
    if( p_wk->state == Mb_Parent_DataConv_State_SendRom )
    {
      p_wk->is_wait = TRUE;
    }
    break;
  case MB_PARENT_DATACONV_REQ_SEND_ROM_CANCEL: //ROM送信キャンセル
    if( p_wk->state == Mb_Parent_DataConv_State_SendRom )
    {
      p_wk->is_cancel = TRUE;
    }
    break;
  case MB_PARENT_DATACONV_REQ_CONNECT_ROM: //送信したROMと通信開始
    GF_ASSERT( p_wk->state == NULL );
    p_wk->state = Mb_Parent_DataConv_State_ConnectRom;
    break;
  default:
    GF_ASSERT(0);
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リクエスト完了チェック
 *
 *	@param	const MB_PARENT_DATACONV *cp_wk   ワーク
 *
 *	@return　TRUEリクエスト完了 FALSE続行中
 */
//-----------------------------------------------------------------------------
BOOL MB_PARENT_DATACONV_IsEndRequest( const MB_PARENT_DATACONV *cp_wk )
{
  return cp_wk->state == NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief ロムイメージをセットアップ
 *
 *	@param	MB_PARENT_DATACONV *p_wk  ワーク
 *	@param	STRBUF *cp_title          タイトル文字列
 *	@param	STRBUF *cp_info           説明文文字列
 */
//-----------------------------------------------------------------------------
void MB_PARENT_DATACONV_SetUpRomImage( MB_PARENT_DATACONV *p_wk, const STRBUF *cp_title, const STRBUF *cp_info )
{
  GF_ASSERT( p_wk->p_game_registry == NULL );
  p_wk->p_game_registry = Mp_Parent_MBGameRegistry_Create( cp_title, cp_info, p_wk->heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  通信結果を取得
 *
 *	@param	const MB_PARENT_DATACONV *cp_wk   ワーク
 *
 *	@return 通信結果
 */
//-----------------------------------------------------------------------------
MB_PARENT_DATACONV_RESULT MB_PARENT_DATACONV_GetResult( const MB_PARENT_DATACONV *cp_wk )
{
  return cp_wk->result;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データを取得
 *
 *	@param	MB_PARENT_DATACONV *p_wk  ワーク
 *
 *	@return 受信データ
 */
//-----------------------------------------------------------------------------
void *MB_PARENT_DATACONV_GetRecvData( MB_PARENT_DATACONV *p_wk )
{
  return MB_COMM_GetCommonData( p_wk->p_comm );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ROM転送
 *
 *	@param	MB_PARENT_DATACONV *p_wk  ワーク
 *	@param	*p_seq  ローカルシーケンス
 *
 *	@return TRUEで終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Mb_Parent_DataConv_State_SendRom( MB_PARENT_DATACONV *p_wk, u32 *p_seq )
{
  enum
  {
    SEQ_INIT_START,
    SEQ_INIT_WAIT,
    SEQ_SEARCH_CH,
    SEQ_SEND_IMAGE_START,
    SEQ_SEND_IMAGE_WAIT,
    SEQ_EXIT_START,
    SEQ_EXIT_WAIT,
    SEQ_END,
  };

  //エラーチェック
  if( p_wk->is_wh_init )
  {
    if( WH_GetSystemState() == WH_SYSSTATE_ERROR ||
        WH_GetSystemState() == WH_SYSSTATE_FATAL )
    {
      p_wk->is_send_rom= FALSE;
      //work->isNetErrMb = TRUE;
      if( WH_Finalize() == TRUE )
      {
        MBP_ClearBuffer();
        WH_FreeMemory();
        return TRUE;
      }
      else
      {
        //Finalize終了までループ
        return FALSE;
      }
    }
  }

  switch( *p_seq )
  {
  case SEQ_INIT_START:
    p_wk->is_cancel = FALSE;
    p_wk->is_wait   = FALSE;
    p_wk->is_send_rom = FALSE;
    p_wk->is_wh_init = TRUE;

    GFL_OVERLAY_Load( FS_OVERLAY_ID(dev_wireless));

    WhCallBackFlg = FALSE;
    WH_Initialize( p_wk->heapID, &MP_PARENT_WhCallBack, FALSE );
    
    GFL_NET_WirelessIconEasy_HoldLCD( TRUE, p_wk->heapID );
    //GFL_UI_SoftResetSetFunc( MB_PARENT_SoftResetCallBack , p_wk );
    (*p_seq)++;
    break;
  case SEQ_INIT_WAIT:
    if( WH_GetSystemState() == WH_SYSSTATE_IDLE )
    {
      if( WH_StartMeasureChannel() == TRUE )
      {
        (*p_seq)++;
      }
    }
    break;
  case SEQ_SEARCH_CH:
    if( WH_GetSystemState() == WH_SYSSTATE_MEASURECHANNEL )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_SEND_IMAGE_START:
    MBP_Init( p_wk->heapID, SYACHI_NETWORK_GGID, MB_TGID_AUTO );
    MBP_Start( p_wk->p_game_registry, WH_GetMeasureChannel() );
    (*p_seq)++;
    break;
  case SEQ_SEND_IMAGE_WAIT:
    {
      MP_PARENT_SENDMAIN_RET ret = Mp_Parent_MBGameRegistry_SendMain( p_wk );
      switch( ret )
      {
      case MP_PARENT_SENDMAIN_RET_CONTINUE:
        /*  処理続行のためなにもしない  */
        break;
      case MP_PARENT_SENDMAIN_RET_COMPLETE:
        p_wk->is_send_rom = TRUE;
        (*p_seq)++;
        break;
      case MP_PARENT_SENDMAIN_RET_CANCEL:
        (*p_seq)++;
        break;
      default:
        GF_ASSERT(0);
      }
    }
    break;
  case SEQ_EXIT_START:
    WH_End( &MP_PARENT_WhCallBack );
    (*p_seq)++;
    break;
  case SEQ_EXIT_WAIT:
    if( WhCallBackFlg == TRUE )
    {
      GFL_UI_SoftResetSetFunc( NULL , NULL );
      WH_FreeMemory();
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(dev_wireless));
    p_wk->is_wh_init = FALSE;
    return TRUE;
  }

  GFL_NET_WirelessIconEasy_SetLevel(3-WM_GetLinkLevel());
  GFL_NET_WirelessIconEasyFunc();

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ROMと通信
 *
 *	@param	MB_PARENT_DATACONV *p_wk  ワーク
 *	@param	*p_seq  ローカルシーケンス
 *
 *	@return TRUEで終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL Mb_Parent_DataConv_State_ConnectRom( MB_PARENT_DATACONV *p_wk, u32 *p_seq )
{
  enum
  {
    SEQ_INIT_START,
    SEQ_INIT_WAIT,
    SEQ_CONNECT_START,
    SEQ_CONNECT_WAIT,
    SEQ_SEND_INITDATA,
    SEQ_WAIT,
    SEQ_EXIT_START,
    SEQ_EXIT_WAIT,
    SEQ_END,
  };

  switch( *p_seq )
  {
  case SEQ_INIT_START:
    MB_COMM_InitComm( p_wk->p_comm );
    GFL_NET_ReloadIconTopOrBottom(FALSE , p_wk->heapID );
    (*p_seq)++;
    break;
  case SEQ_INIT_WAIT:
    if( MB_COMM_IsInitComm( p_wk->p_comm ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_CONNECT_START:
    MB_COMM_InitParent( p_wk->p_comm );
    (*p_seq)++;
    break;
  case SEQ_CONNECT_WAIT:
    if( p_wk->timeout_cnt++ >= MB_PARENT_FIRST_TIMEOUT )
    {
      p_wk->result = MB_PARENT_DATACONV_RESULT_TIMEOUT;
      MB_COMM_ExitComm( p_wk->p_comm );
      (*p_seq) = SEQ_END;
    }
    else if( MB_COMM_IsSendEnable( p_wk->p_comm ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_SEND_INITDATA:
    if( MB_COMM_Send_InitData( p_wk->p_comm, &p_wk->init_data ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_WAIT:
    if( MB_COMM_GetChildState(p_wk->p_comm) == MCCS_END_GAME_ERROR ) //ROMorCRCが違う
    {
      p_wk->result = MB_PARENT_DATACONV_RESULT_GAME_ERROR;
      MB_COMM_ReqDisconnect( p_wk->p_comm );
      (*p_seq) = SEQ_EXIT_WAIT;
    }
    else if( MB_COMM_GetChildState(p_wk->p_comm) == MCCS_END_GAME_ERROR_BACKUP_LOAD ) //セーブ読めない
    {
      p_wk->result = MB_PARENT_DATACONV_RESULT_BACKUP_ERROR;
      MB_COMM_ReqDisconnect( p_wk->p_comm );
      (*p_seq) = SEQ_EXIT_WAIT;
    }
    else if( MB_COMM_GetChildState(p_wk->p_comm) == MCCS_END_GAME_ERROR_ANOTHER_COUNTRY ) //国が違う
    {
      p_wk->result = MB_PARENT_DATACONV_RESULT_CONTRY_ERROR;
      MB_COMM_ReqDisconnect( p_wk->p_comm );
      (*p_seq) = SEQ_EXIT_WAIT;
    }
    else if( MB_COMM_IsPostCommonData(p_wk->p_comm) )
    {
      OS_TPrintf( "受け取った\n" );
      if( MB_COMM_Send_Flag( p_wk->p_comm , MCFT_PERMIT_FINISH_COMMON_DATA, 0 ) == TRUE )
      {
        (*p_seq)++;
      }
    }
    break;

  case SEQ_EXIT_START:
    if( MB_COMM_Send_Flag( p_wk->p_comm , MCFT_MOVIE_FINISH_MACHINE, 0 ) == TRUE )
    {
      MB_COMM_ReqDisconnect( p_wk->p_comm );
      (*p_seq)++;
    }
    break;

  case SEQ_EXIT_WAIT:
    if( MB_COMM_IsDisconnect( p_wk->p_comm ) )
    {
      MB_COMM_ExitComm( p_wk->p_comm );
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    return TRUE;
  default:
    GF_ASSERT(0);
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MBゲームレジストリを作成
 *
 *	@param	const STRBUF *cp_title_str  タイトル文
 *	@param	STRBUF *p_info_str          説明文
 *	@param	heapID                      ヒープID
 *
 *	@return　MBゲームレジストリ
 */
//-----------------------------------------------------------------------------
static MBGameRegistry * Mp_Parent_MBGameRegistry_Create( const STRBUF *cp_title_str, const STRBUF *cp_info_str, HEAPID heapID )
{
  int i;
  MBGameRegistry *p_registry = GFL_HEAP_AllocMemory( heapID, sizeof(MBGameRegistry) );

  p_registry->romFilePathp = "/dl_rom/child2.srl";
  p_registry->gameNamep = GFL_HEAP_AllocClearMemory( heapID, MB_GAME_NAME_LENGTH*2 );
  {
    u16 title_len = GFL_STR_GetBufferLength( cp_title_str );
    GFL_STD_MemCopy16( GFL_STR_GetStringCodePointer( cp_title_str ) ,
                     p_registry->gameNamep,
                     title_len*2 );
    p_registry->gameNamep[title_len] = 0x0000;
  }
  p_registry->gameIntroductionp = GFL_HEAP_AllocClearMemory( heapID, MB_GAME_INTRO_LENGTH*2 );
  {
    u16 info_len = GFL_STR_GetBufferLength( cp_info_str );
    GFL_STD_MemCopy16( GFL_STR_GetStringCodePointer( cp_info_str ),
                     p_registry->gameIntroductionp,
                     info_len*2 );
    p_registry->gameIntroductionp[info_len] = 0x0000;
  }

  //改行コード変換
  for( i=0;i<MB_GAME_INTRO_LENGTH;i++ )
  {
    if( p_registry->gameIntroductionp[i] == 0xFFFE )
    {
      p_registry->gameIntroductionp[i] = 0x000a;
    }
    else if( p_registry->gameIntroductionp[i] >= L'０' &&
        p_registry->gameIntroductionp[i] <= L'９')
    {
      p_registry->gameIntroductionp[i] = p_registry->gameIntroductionp[i]-L'０'+L'0';
    }
  }
  //全角数字変換
  for( i=0;i<MB_GAME_NAME_LENGTH;i++ )
  {
    if( p_registry->gameNamep[i] == L'Ｉ' )
    {
      p_registry->gameNamep[i] = L'I';
    }
    else
    if( p_registry->gameNamep[i] == L'Ｄ' )
    {
      p_registry->gameNamep[i] = L'D';
    }
    else
    if( p_registry->gameNamep[i] >= L'０' &&
        p_registry->gameNamep[i] <= L'９')
    {
      p_registry->gameNamep[i] = p_registry->gameNamep[i]-L'０'+L'0';
    }
  }

  if( GET_VERSION() == VERSION_W2 )
  {
    p_registry->iconCharPathp     = "/dl_rom/icon_w.char";
    p_registry->iconPalettePathp  = "/dl_rom/icon_w.plt";
  }
  else
  {
    p_registry->iconCharPathp     = "/dl_rom/icon_b.char";
    p_registry->iconPalettePathp  = "/dl_rom/icon_b.plt";
  }
  p_registry->ggid = SYACHI_NETWORK_GGID; //TODO
  p_registry->maxPlayerNum  = 2;

  return p_registry;
}
//----------------------------------------------------------------------------
/**
 *	@brief  MBゲームレジストリを破棄
 *
 *	@param	MBGameRegistry *p_registry MBゲームレジストリ
 */
//-----------------------------------------------------------------------------
static void Mp_Parent_MBGameRegistry_Delete( MBGameRegistry *p_registry )
{
  GFL_HEAP_FreeMemory( p_registry->gameIntroductionp );
  GFL_HEAP_FreeMemory( p_registry->gameNamep );
  GFL_HEAP_FreeMemory( p_registry );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ロム送信処理
 
 *	@param	MB_PARENT_DATACONV *p_wk  ワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static MP_PARENT_SENDMAIN_RET Mp_Parent_MBGameRegistry_SendMain( MB_PARENT_DATACONV *p_wk )
{
#if PM_DEBUG
  {
    const u16 mbpState = MBP_GetState();
    static u16 befState = 0xFFFF;
    if( befState != mbpState )
    {
      OS_TFPrintf(3,"MBState[%d]->[%d]\n",befState,mbpState);
      befState = mbpState;
    }
  }
#endif

  //以下サンプル流用
  switch (MBP_GetState())
  {
    //-----------------------------------------
    // アイドル状態
  case MBP_STATE_IDLE:
    //ココには来ない
    break;
    //-----------------------------------------
    // 子機からのエントリー受付中
  case MBP_STATE_ENTRY:
    {
      if( p_wk->is_cancel == TRUE )
      {
        MBP_Cancel();
        break;
      }
      if( p_wk->is_wait == FALSE )
      {    
        if (MBP_IsBootableAll())
        {
          // ブート開始
          MBP_StartRebootAll();
        }
      }
    }
    break;

    //-----------------------------------------
    // プログラム配信処理
  case MBP_STATE_DATASENDING:
    {
//ここは来ない！
    }
    break;

    //-----------------------------------------
    // リブート処理
  case MBP_STATE_REBOOTING:
    {
    }
    break;

    //-----------------------------------------
    // 再接続処理
  case MBP_STATE_COMPLETE:
    {
      // 全員無事に接続完了したらマルチブート処理は終了し
      // 通常の親機として無線を再起動する。
      return MP_PARENT_SENDMAIN_RET_COMPLETE;
    }
    break;

    //-----------------------------------------
    // エラー発生
  case MBP_STATE_ERROR:
    {
      // 通信をキャンセルする
      MBP_Cancel();
    }
    break;

    //-----------------------------------------
    // 通信キャンセル処理中
  case MBP_STATE_CANCEL:
    // None
    break;

    //-----------------------------------------
    // 通信異常終了
  case MBP_STATE_STOP:
    return MP_PARENT_SENDMAIN_RET_CANCEL;
  }
 
  return MP_PARENT_SENDMAIN_RET_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  コールバック
 *
 *	@param	BOOL bResult 
 *
 *	@return 
 */
//-----------------------------------------------------------------------------
static BOOL MP_PARENT_WhCallBack( BOOL bResult )
{
  WhCallBackFlg = TRUE;
  return TRUE;
}
