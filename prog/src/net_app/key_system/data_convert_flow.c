//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		data_convert_flow.c
 *	@brief  データコンバートフロー
 *	@author	Toru=Nagihashi
 *	@data		2011.07.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "gamesystem/game_data.h"
#include "system/bmp_winframe.h"
#include "print/wordset.h"
#include "field/zonedata.h"
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"
#include "app/name_input.h"
#include "system/net_err.h"
#include "net_app/pdwacc_gamesync_id_inline.h"

//アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "arc/key_system_gra.naix"
#include "arc/mb_parent.naix"
#include "msg/msg_key_system.h"
#include "msg/script/msg_common_scr.h"
#include "script_message.naix"
#include "message.naix"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"
#include "system/ds_system.h"

//自分のモジュール
#include "key_system_util.h"
#include "key_system_graphic.h"
#include "key_system_net.h"
#include "key_system_main.h"
#include "key_system_snd.h"

//セーブ
#include "savedata/save_outside.h"
#include "savedata/outside_permanent.h"

//外部公開
#include "data_convert_flow.h"

#ifdef PM_DEBUG
#define DEBUG_DATACONV_LIST_OPEN
#endif

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define DATACONV_INFO_LIST_MAX  (8)
#define DATACONV_SCROLL_MENU_CANCEL (0xFF)
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// シーケンス
//=====================================
//サブシーケンス
static void DataConv_Seq_Wireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_Wifi( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void DataConv_Seq_Save( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_SaveOuter( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_SaveInner( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_SaveInAndOut( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_InfomationList( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_Explain( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//画面構築シーケンス
static void DataConv_Seq_CreateDisplayNetMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_CreateDisplayDLPlayInfo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_CreateDisplayWifi( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_CreateDisplayInfomationList( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_CreateDisplayExplain( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//プロセス関係シーケンス
static void DataConv_Seq_SubProc_WifiLogin( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_SubProc_WifiLogout( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_SubProc_NameIn( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//その他
static BOOL DataConv_Seq_ConvComplete( KEY_SYSTEM_WORK *p_wk );
static BOOL DataConv_InfoList_GetVisible( const KEY_SYSTEM_WORK *cp_wk, u32 index );
static BOOL DataConv_InfoList_GetVisibleAny( const KEY_SYSTEM_WORK *cp_wk );
static BOOL DataConv_InfoList_GetMark( const KEY_SYSTEM_WORK *cp_wk, u32 index );

//-------------------------------------
/// 画面作成・破棄
//=====================================
//メインメニュー
static void DataConv_Display_CreateNetMenu( void *p_wk_adrs, HEAPID heapID );
static BOOL DataConv_Display_IsEndCreateNetMenu( void *p_wk_adrs );
static void DataConv_Display_DeleteNetMenu( void *p_wk_adrs );
//DLプレイ説明画面
static void DataConv_Display_CreateDlPlay( void *p_wk_adrs, HEAPID heapID );
static BOOL DataConv_Display_IsEndCreateDlPlay( void *p_wk_adrs );
static void DataConv_Display_DeleteDlPlay( void *p_wk_adrs );
//WiFIダウンロード画面
static void DataConv_Display_CreateWifi( void *p_wk_adrs, HEAPID heapID );
static BOOL DataConv_Display_IsEndCreateWifi( void *p_wk_adrs );
static void DataConv_Display_DeleteWifi( void *p_wk_adrs );
//思い出リスト画面
static void DataConv_Display_CreateInfomationList( void *p_wk_adrs, HEAPID heapID );
static BOOL DataConv_Display_IsEndCreateInfomationList( void *p_wk_adrs );
static void DataConv_Display_DeleteInfomationList( void *p_wk_adrs );
//思い出リスト説明画面
static void DataConv_Display_CreateExplain( void *p_wk_adrs, HEAPID heapID );
static BOOL DataConv_Display_IsEndCreateExplain( void *p_wk_adrs );
static void DataConv_Display_DeleteExplain( void *p_wk_adrs );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  データコンバートフロー用設定
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_DATACONVFLOW_Init( KEY_SYSTEM_WORK *p_wk, HEAPID heapID )
{
}
//----------------------------------------------------------------------------
/**
 *	@brief  データコンバートフロー用破棄
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_DATACONVFLOW_Exit( KEY_SYSTEM_WORK *p_wk )
{
  if( p_wk->p_rom_title )
  {
    GFL_STR_DeleteBuffer( p_wk->p_rom_title );
    p_wk->p_rom_title = NULL;
  }
  if( p_wk->p_rom_info )
  {
    GFL_STR_DeleteBuffer( p_wk->p_rom_info );
    p_wk->p_rom_info = NULL;
  }
}
//=============================================================================
/**
 *					フロー
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  データコンバートメインフロー
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
void KEYSYS_DATACONVFLOW_SEQ_Main( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_MAIN,
    SEQ_DISPLAY_DELETE,
    SEQ_SELECT_CALL,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayNetMenu );
    (*p_seq)++;
    break;
  case SEQ_MAIN:
    KEYSYS_MENU_Main( p_wk->p_menu );

    if( KEYSYS_MENU_IsChangeSelect( p_wk->p_menu ) )
    {
      if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_info ) )
      {
        u32 select = KEYSYS_MENU_GetSelect( p_wk->p_menu );
        if( !DataConv_InfoList_GetVisibleAny(p_wk) )
        {
          if( select == 2 )
          {
            select = 3;
          }
        }
        KEYSYS_TEXT_Print( p_wk->p_info, p_wk->p_msg, KEYSYS_STR_72+select, KEYSYS_TEXT_TYPE_QUE ); 
      }
    }

    if( KEYSYS_MENU_IsDecide( p_wk->p_menu ) )
    {
      p_wk->ret = KEYSYS_MENU_GetSelect( p_wk->p_menu );

      //もどる以外は記憶
      if( DataConv_InfoList_GetVisibleAny(p_wk) )
      {
        if( p_wk->ret < 3 )
        {
          KEYSYS_MAP_Insert( &p_wk->select, "DNM", KEYSYS_MENU_GetSelect( p_wk->p_menu ) );
        }
      }
      else
      {
        if( p_wk->ret < 2 )
        {
          KEYSYS_MAP_Insert( &p_wk->select, "DNM", KEYSYS_MENU_GetSelect( p_wk->p_menu ) );
        }
      }
      (*p_seq)++;
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;

  case SEQ_SELECT_CALL:
    if( DataConv_InfoList_GetVisibleAny(p_wk) )
    {
      switch( p_wk->ret )
      {
      case 0://ワイヤレス
        KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Wireless );
        //もう一度メインメニュー
        (*p_seq ) = 0;
        break;
      case 1://Wi-Fi
        KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Wifi );
        //もう一度メインメニュー
        (*p_seq ) = 0;
        break;
      case 2://おもいでリスト
        KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_InfomationList );
        //もう一度メインメニュー
        (*p_seq ) = 0;
        break;
      case 3://もどる
        (*p_seq)++;
        break;
      default:
        GF_ASSERT(0);
      }
    }
    else
    {
      switch( p_wk->ret )
      {
      case 0://ワイヤレス
        KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Wireless );
        //もう一度メインメニュー
        (*p_seq ) = 0;
        break;
      case 1://Wi-Fi
        KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Wifi );
        //もう一度メインメニュー
        (*p_seq ) = 0;
        break;
      case 2://もどる
        (*p_seq)++;
        break;
      default:
        GF_ASSERT(0);
      }
    }

    break;

  case SEQ_END:
    KEYSYS_BG_RequestPltFade( p_wk->p_bg, KEYSYS_BG_PLTFADE_REQ_DATACONV_TO_MAIN, 30 );
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワイヤレス
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_Wireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_YESNO_MSG_START,
    SEQ_YESNO_START,
    SEQ_YESNO_DRAW_WAIT,
    SEQ_YESNO_WAIT,

    SEQ_DISPLAY_CREATE,
    SEQ_SEND_ROM_START,
    SEQ_SEND_ROM_WAIT,
    SEQ_SEND_ROM_CANCEL_WAIT,
    SEQ_CONNECT_ROM_START,
    SEQ_CONNECT_ROM_WAIT,
    SEQ_CONNECT_ROM_RESULT,
    SEQ_CONVERT_CHECK,
    SEQ_CONVERT_YESNO_MSG,
    SEQ_CONVERT_YESNO_START,
    SEQ_CONVERT_YESNO_WAIT,
    SEQ_CONVERT_MAIN,
    SEQ_SAVE,
    SEQ_COMPLETE_MSG_START,
    SEQ_CANCEL_MSG_START,
    SEQ_COMPLETE_MSG_WAIT,
    SEQ_ERR_MESSAGE_END,

    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_INIT:
    if( DS_SYSTEM_IsAvailableWireless() )
    {
      //無線許可なら通信してよいか聞いて開始
      (*p_seq)  = SEQ_YESNO_MSG_START;
    }
    else
    {
      //無線OFFならば警告だして終了
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DisableWireless );
      (*p_seq)  = SEQ_END;
    }
    break;
  case SEQ_YESNO_MSG_START:
    KEYSYS_TEXT_Create( p_wk, HEAPID_KEY_SYSTEM );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_03, KEYSYS_TEXT_TYPE_STREAM );
    (*p_seq)++;
    break;
  case SEQ_YESNO_START:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      KEYSYS_LIST_CreateYesNo( p_wk, HEAPID_KEY_SYSTEM );
      (*p_seq)++;
    }
    break;
  case SEQ_YESNO_DRAW_WAIT:
    if( KEYSYS_LIST_Draw( p_wk->p_list ) )
    {
      GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
      (*p_seq)++;
    }
    break;
  case SEQ_YESNO_WAIT:
    {
      u32 ret = KEYSYS_LIST_Main(p_wk->p_list );
      if( ret != KEYSYS_LIST_SELECT_NULL )
      {
        KEYSYS_LIST_Delete( p_wk );
        KEYSYS_TEXT_Delete( p_wk );
        switch( ret )
        {
        case 0:
          (*p_seq)++;
          break;
        default:
          GF_ASSERT(0);
        case 1:
          *p_seq = SEQ_END;
          break;
        }
      }
    }
    break;
  case SEQ_DISPLAY_CREATE:
    KEYSYS_NET_Start( p_wk->p_net, KEYSYS_NET_TYPE_MULTIBOOT );
    p_wk->p_recv_data = NULL;
    {
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      BOOL is_save_exist = SaveData_GetExistFlag( p_sv );
      STRBUF *p_title_str;

      if( p_wk->p_rom_title )
      {
        GFL_STR_DeleteBuffer( p_wk->p_rom_title );
        p_wk->p_rom_title = NULL;
      }
      if( p_wk->p_rom_info )
      {
        GFL_STR_DeleteBuffer( p_wk->p_rom_info );
        p_wk->p_rom_info = NULL;
      }

      p_wk->p_rom_title = GFL_STR_CreateBuffer(128, HEAPID_KEY_SYSTEM );

      if( GET_VERSION() == VERSION_B2 )
      {
        p_wk->p_rom_info = GFL_MSG_CreateString( p_wk->p_msg, DATACONV_ROM_INFO_B );
      }
      else
      {
        p_wk->p_rom_info = GFL_MSG_CreateString( p_wk->p_msg, DATACONV_ROM_INFO_W );
      }
      if( is_save_exist )
      {
        MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus(p_wk->p_param->p_gamedata);
        WORDSET_RegisterNumber( p_wk->p_word, 0, MyStatus_GetID_Low(p_mystatus), 5, STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
        p_title_str = GFL_MSG_CreateString( p_wk->p_msg, DATACONV_ROM_TITLE );
      }
      else
      {
        p_title_str = GFL_MSG_CreateString( p_wk->p_msg, DATACONV_ROM_TITLE_NOSAVE );
      }
      WORDSET_ExpandStr( p_wk->p_word, p_wk->p_rom_title, p_title_str );
      GFL_STR_DeleteBuffer( p_title_str );
    }
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayDLPlayInfo );
    (*p_seq)++;
    break;
  case SEQ_SEND_ROM_START:
    //通信モードをDLPLAYに
    {
      KEYSYS_NET_SEQ_PARAM_MULTIBOOT_SEND_ROM param = {0};
      param.cp_title  = p_wk->p_rom_title;
      param.cp_info   = p_wk->p_rom_info;
      KEYSYS_NET_RequestState( p_wk->p_net, KEYSYS_NET_REQ_MULTIBOOT_SEND_ROM, &param );
    }
    (*p_seq)++;
    break;
  case SEQ_SEND_ROM_WAIT:
    if( KEYSYS_NET_GetState(p_wk->p_net ) == KEYSYS_NET_STATE_WAIT )
    {
      KEYSYS_NET_SEQ_PARAM_MULTIBOOT_SEND_ROM *p_param = KEYSYS_NET_GetParamAdrs(p_wk->p_net);
      if( p_param->result == KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM_FINISH )
      {
        *p_seq = SEQ_CONNECT_ROM_START;
      }
      else
      {
        (*p_seq) = SEQ_SEND_ROM_CANCEL_WAIT;
      }
    }
    break;
  case SEQ_SEND_ROM_CANCEL_WAIT:
    (*p_seq) = SEQ_DISPLAY_DELETE;
    break;
  case SEQ_CONNECT_ROM_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_01, KEYSYS_TEXT_TYPE_WAIT );
    {
      KEYSYS_NET_SEQ_PARAM_MULTIBOOT_CONNECT_ROM param = {0};
      KEYSYS_NET_RequestState( p_wk->p_net, KEYSYS_NET_REQ_MULTIBOOT_CONNECT_ROM, &param );
    }
    (*p_seq)++;
    break;
  case SEQ_CONNECT_ROM_WAIT:
    if( KEYSYS_NET_GetState(p_wk->p_net ) == KEYSYS_NET_STATE_WAIT )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_CONNECT_ROM_RESULT:
    {
      KEYSYS_NET_SEQ_PARAM_MULTIBOOT_CONNECT_ROM *p_param = KEYSYS_NET_GetParamAdrs(p_wk->p_net);
      switch( p_param->data_result )
      {
      case MB_PARENT_DATACONV_RESULT_NONE: //とくになにもない
        {
          p_wk->p_recv_data = p_param->p_recv_data;
          *p_seq = SEQ_CONVERT_CHECK;
        }
        break;
      case MB_PARENT_DATACONV_RESULT_TIMEOUT: //タイムアウト
        KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_02, KEYSYS_TEXT_TYPE_WAIT );
        *p_seq = SEQ_ERR_MESSAGE_END;
        break;
      case MB_PARENT_DATACONV_RESULT_BACKUP_ERROR: //CRCが違う
        KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_05, KEYSYS_TEXT_TYPE_WAIT );
        *p_seq = SEQ_ERR_MESSAGE_END;
        break;
      default:
        GF_ASSERT(0);
      case MB_PARENT_DATACONV_RESULT_CONTRY_ERROR: //国が違う
      case MB_PARENT_DATACONV_RESULT_GAME_ERROR: //ROMが違う
        KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_04, KEYSYS_TEXT_TYPE_WAIT );
        *p_seq = SEQ_ERR_MESSAGE_END;
        break;
      }
      break;
    }
  case SEQ_CONVERT_CHECK:
    {
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      if( DATA_CONVERT_SAVE_IsSameOwner( p_wk->p_data_sv, p_wk->p_recv_data, p_sv ) )
      {
        *p_seq = SEQ_CONVERT_MAIN;
      }
      else
      {
        *p_seq = SEQ_CONVERT_YESNO_MSG;
      }
    }
    break;
  case SEQ_CONVERT_YESNO_MSG:
    KEYSYS_TEXT_Exit( p_wk->p_text );
    p_wk->p_text = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 2, 4, 28, 12, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, HEAPID_KEY_SYSTEM );
    KEYSYS_TEXT_SetColor( p_wk->p_text, PRINTSYS_LSB_Make( 1, 2, 0xF ) );
    KEYSYS_TEXT_WriteWindowFrame( p_wk->p_text, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );

    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_07, KEYSYS_TEXT_TYPE_STREAM );
    *p_seq = SEQ_CONVERT_YESNO_START;
    break;
  case SEQ_CONVERT_YESNO_START:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      KEYSYS_LIST_SETUP setup;
      GFL_STD_MemClear( &setup, sizeof(KEYSYS_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.frm     = KEYSYS_BG_FRAME_TEXT_M;
      setup.font_plt = KEYSYS_PLT_BG_FONT_M;
      setup.frm_plt = KEYSYS_PLT_BG_FRAME_M;
      setup.frm_chr = KEYSYS_CGR_TEXT_M;
      setup.strID[0]  = KEYSYS_STR_58;
      setup.strID[1]  = KEYSYS_STR_59;
      setup.list_max  = 2;
      setup.is_cancel = TRUE;
      setup.cancel_idx = 1;
      setup.default_idx = 0;
      p_wk->p_list = KEYSYS_LIST_InitEx( &setup,32-10-1,24-4-1,10,4,HEAPID_KEY_SYSTEM );
      GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
      (*p_seq)++;
    }
    break;
  case SEQ_CONVERT_YESNO_WAIT:
    {
      u32 ret = KEYSYS_LIST_Main(p_wk->p_list );
      if( ret != KEYSYS_LIST_SELECT_NULL )
      {
        KEYSYS_LIST_Exit(p_wk->p_list);
        p_wk->p_list = NULL;

        switch( ret )
        {
        case 0:
          (*p_seq)++;
          break;
        default:
          GF_ASSERT(0);
        case 1:
          *p_seq = SEQ_CANCEL_MSG_START;
          break;
        }
      }
    }
    break;
  case SEQ_CONVERT_MAIN:
    {
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      p_wk->is_convert = DATA_CONVERT_SAVE_Convert( p_wk->p_data_sv, p_wk->p_recv_data, p_sv );
    }
    (*p_seq)++;
    break;
 case SEQ_SAVE:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Save );
    (*p_seq)++;
    break;

  case SEQ_COMPLETE_MSG_START:
    if( p_wk->is_convert )
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_14, KEYSYS_TEXT_TYPE_STREAM_FIX );
    }
    else
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_06, KEYSYS_TEXT_TYPE_STREAM_FIX );
    }
    *p_seq = SEQ_COMPLETE_MSG_WAIT;
    break;
    
  case SEQ_CANCEL_MSG_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_08, KEYSYS_TEXT_TYPE_STREAM_FIX );
    *p_seq = SEQ_COMPLETE_MSG_WAIT;
    break;

  case SEQ_COMPLETE_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq) = SEQ_DISPLAY_DELETE;
    }
    break;

  case SEQ_ERR_MESSAGE_END:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq) = SEQ_DISPLAY_DELETE;
    }
    break;

  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    KEYSYS_NET_Start( p_wk->p_net, KEYSYS_NET_TYPE_NONE );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  WiFiフロー
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_Wifi( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CLOSE_PROC_TO_WIFI_LOGIN,
    SEQ_DELETE_DISPLAY_TO_WIFI_LOGIN,
    SEQ_SUBPROC_WIFI_LOGIN,
    SEQ_CREATE_DISPLAY_TO_WIFI_LOGIN,
    SEQ_OPEN_PROC_TO_WIFI_LOGIN,
    SEQ_CHECK_LOGIN,

    SEQ_START_SERVER_MSG,
    SEQ_WAIT_SERVER_MSG,

    SEQ_START_PGL_SERVER,
    SEQ_WAIT_PGL_SERVER,

    SEQ_MSG_INPUT_START,
    SEQ_MSG_INPUT_WAIT,

    SEQ_CLOSE_PROC_TO_NAMEIN,
    SEQ_DELETE_DISPLAY_TO_NAMEIN,
    SEQ_SUBPROC_NAMEIN,
    SEQ_CREATE_DISPLAY_TO_NAMEIN,
    SEQ_OPEN_PROC_TO_NAMEIN,

    SEQ_CHECK_INPUT,

    SEQ_MSG_COMM_START,
    SEQ_MSG_COMM_WAIT,

    SEQ_SAVE_CONVERT,
    SEQ_CHECK_OWNER,
    SEQ_CONVERT_YESNO_MSG,
    SEQ_CONVERT_YESNO_START,
    SEQ_CONVERT_YESNO_WAIT,
    SEQ_CONVERT_MAIN,

    SEQ_SAVE,
    SEQ_COMPLETE_MSG_START,


    SEQ_MSG_RETRY_START,
    SEQ_MSG_RETRY_WAIT,

    SEQ_MSG_CANCEL_START,
    SEQ_MSG_NOSWAN_START,
    SEQ_MSG_NODATA_START,
    SEQ_MSG_END_WAIT,

    SEQ_CLOSE_PROC_TO_WIFI_LOGOUT,
    SEQ_DELETE_DISPLAY_TO_WIFI_LOGOUT,
    SEQ_SUBPROC_WIFI_LOGOUT,
    SEQ_CREATE_DISPLAY_TO_WIFI_LOGOUT,
    SEQ_OPEN_PROC_TO_WIFI_LOGOUT,
    SEQ_DELETE_DISPLAY,
    SEQ_END,

    SEQ_START_WIFI_DISABLE_MSG,
    SEQ_WAIT_WIFI_DISABLE_MSG,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_CLOSE_PROC_TO_WIFI_LOGIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutBlack );
    (*p_seq)++;
    break;
  case SEQ_DELETE_DISPLAY_TO_WIFI_LOGIN:
    (*p_seq)++;
    break;
  case SEQ_SUBPROC_WIFI_LOGIN:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_SubProc_WifiLogin );
    (*p_seq)++;
    break;
  case SEQ_CREATE_DISPLAY_TO_WIFI_LOGIN:
    if( p_wk->is_wifi_start )
    {
      p_wk->is_convert =  FALSE;

      KEYSYS_NET_SetDWCSvlResult( p_wk->p_net, &p_wk->svl );
      KEYSYS_NET_Start( p_wk->p_net, KEYSYS_NET_TYPE_WIFI_NTTP );

      KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayWifi );
    }
    else
    {
      KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayNetMenu );
    }
    (*p_seq)++;
    break;
  case SEQ_OPEN_PROC_TO_WIFI_LOGIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeInBlack );
    (*p_seq)++;
    break;
  case SEQ_CHECK_LOGIN:
    if( p_wk->is_wifi_start )
    {
      (*p_seq)++;
    }
    else
    {
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_START_SERVER_MSG:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_17, KEYSYS_TEXT_TYPE_WAIT ); 
    (*p_seq)++;
    break;
  case SEQ_WAIT_SERVER_MSG:
    {
      (*p_seq)++;
    }
    break;
  case SEQ_START_PGL_SERVER:
    {
      KEYSYS_NET_SEQ_RESULT_PLAYSTATUS  param = {0};
      KEYSYS_NET_RequestState( p_wk->p_net, KEYSYS_NET_REQ_PGL_PLAYSTATUS, &param );
    }
    (*p_seq)++;
    break;

  case SEQ_WAIT_PGL_SERVER:
    if( KEYSYS_NET_GetState(p_wk->p_net) == KEYSYS_NET_STATE_WAIT )
    {
      KEYSYS_NET_SEQ_RESULT_PLAYSTATUS *p_param = KEYSYS_NET_GetParamAdrs( p_wk->p_net );
      switch( p_param->result )
      {
      case KEYSYS_NET_RESULT_PLAYSTATUS_PDW_ERROR:
      case KEYSYS_NET_RESULT_PLAYSTATUS_RESPONSE_ERROR:
        NetErr_DispCallPushPop();
        *p_seq = SEQ_END;
        break;
      default:
        if( p_param->play_status == DREAM_WORLD_SERVER_PLAY_FINISH )
        {
          *p_seq = SEQ_START_WIFI_DISABLE_MSG;
        }
        else
        {
          (*p_seq)++;
        }
      }
    }
    break;
    
  case SEQ_MSG_INPUT_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_09, KEYSYS_TEXT_TYPE_STREAM ); 
    (*p_seq)++;
    break;
  case SEQ_MSG_INPUT_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_CLOSE_PROC_TO_NAMEIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutBlack );
    (*p_seq)++;
    break;
  case SEQ_DELETE_DISPLAY_TO_NAMEIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_SUBPROC_NAMEIN:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_SubProc_NameIn );
    (*p_seq)++;
    break;
  case SEQ_CREATE_DISPLAY_TO_NAMEIN:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayWifi );
    (*p_seq)++;
    break;
  case SEQ_OPEN_PROC_TO_NAMEIN:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeInBlack );
    (*p_seq)++;
    break;

  case SEQ_CHECK_INPUT:
    if( p_wk->p_gamesync_id_code == NULL )
    {
      *p_seq = SEQ_MSG_CANCEL_START;
    }
    else
    {
      GF_ASSERT( p_wk->p_download_data == NULL );
      p_wk->p_download_data = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, 0x80000+sizeof(gs_response) );
      GF_ASSERT( p_wk->p_download_data != NULL );
      *p_seq = SEQ_MSG_COMM_START;
    }
    break;
  case SEQ_MSG_COMM_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_13, KEYSYS_TEXT_TYPE_WAIT );
    {
      KEYSYS_NET_SEQ_PARAM_PGL_GETBW  param = {0};
      {
        param.gsid = PDWACC_MESSAGE_ConvertID( p_wk->p_gamesync_id_code );
        param.p_buffer_adrs = p_wk->p_download_data;
        param.buffer_size = 0x80000+sizeof(gs_response);
      }
      KEYSYS_NET_RequestState( p_wk->p_net, KEYSYS_NET_REQ_PGL_GETBW, &param );
    }
    (*p_seq)++;
    break;
  case SEQ_MSG_COMM_WAIT:
    if( KEYSYS_NET_GetState(p_wk->p_net) == KEYSYS_NET_STATE_WAIT )
    {
      KEYSYS_NET_SEQ_PARAM_PGL_GETBW *p_param = KEYSYS_NET_GetParamAdrs(p_wk->p_net);
      switch( p_param->result )
      {
      case KEYSYS_NET_RESULT_PGL_GETBW_RESPONSE_ERROR:
      case KEYSYS_NET_RESULT_PGL_GETBW_PDW_ERROR:
        NetErr_DispCallPushPop();
        *p_seq = SEQ_END;
        break;
      case KEYSYS_NET_RESULT_PGL_GETBW_FINISH:
        *p_seq = SEQ_SAVE_CONVERT;
        break;
      case KEYSYS_NET_RESULT_PGL_GETBW_SWANDATA:
        *p_seq = SEQ_MSG_NOSWAN_START;
        break;
      case KEYSYS_NET_RESULT_PGL_GETBW_NO_REGIST:

        if( p_wk->p_download_data )
        {
          GFL_HEAP_FreeMemory( p_wk->p_download_data );
          p_wk->p_download_data = NULL;
        }
        *p_seq = SEQ_MSG_RETRY_START;
        break;
      case KEYSYS_NET_RESULT_PGL_GETBW_NO_DATA:
        *p_seq = SEQ_MSG_NODATA_START;
        break;
      }
    }
    break;
  case SEQ_SAVE_CONVERT:
    {
      if( p_wk->p_recv_data )
      {
        GFL_HEAP_FreeMemory( p_wk->p_recv_data );
      }
      p_wk->p_recv_data = WB_SAVE_CONV_AllocConvertAllData( (u8*)p_wk->p_download_data+sizeof(gs_response), HEAPID_KEY_SYSTEM );

      GFL_HEAP_FreeMemory( p_wk->p_download_data );
      p_wk->p_download_data = NULL;
      (*p_seq)++;
    }
    break;
  case SEQ_CHECK_OWNER:
    {
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      if( DATA_CONVERT_SAVE_IsSameOwner( p_wk->p_data_sv, p_wk->p_recv_data, p_sv ) )
      {
        *p_seq = SEQ_CONVERT_MAIN;
      }
      else
      {
        *p_seq = SEQ_CONVERT_YESNO_MSG;
      }
    }
    break;
  case SEQ_CONVERT_YESNO_MSG:
    KEYSYS_TEXT_Exit( p_wk->p_text );
    p_wk->p_text = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 2, 4, 28, 12, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, HEAPID_KEY_SYSTEM );
    KEYSYS_TEXT_SetColor( p_wk->p_text, PRINTSYS_LSB_Make( 1, 2, 0xF ) );
    KEYSYS_TEXT_WriteWindowFrame( p_wk->p_text, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_07, KEYSYS_TEXT_TYPE_STREAM );
    *p_seq = SEQ_CONVERT_YESNO_START;
    break;
  case SEQ_CONVERT_YESNO_START:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      KEYSYS_LIST_SETUP setup;
      GFL_STD_MemClear( &setup, sizeof(KEYSYS_LIST_SETUP) );
      setup.p_msg   = p_wk->p_msg;
      setup.p_font  = p_wk->p_font;
      setup.p_que   = p_wk->p_que;
      setup.frm     = KEYSYS_BG_FRAME_TEXT_M;
      setup.font_plt = KEYSYS_PLT_BG_FONT_M;
      setup.frm_plt = KEYSYS_PLT_BG_FRAME_M;
      setup.frm_chr = KEYSYS_CGR_TEXT_M;
      setup.strID[0]  = KEYSYS_STR_58;
      setup.strID[1]  = KEYSYS_STR_59;
      setup.list_max  = 2;
      setup.is_cancel = TRUE;
      setup.cancel_idx = 1;
      setup.default_idx = 0;
      p_wk->p_list = KEYSYS_LIST_InitEx( &setup,32-10-1,24-4-1,10,4,HEAPID_KEY_SYSTEM );
      GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
      (*p_seq)++;
    }
    break;
  case SEQ_CONVERT_YESNO_WAIT:
    {
      u32 ret = KEYSYS_LIST_Main(p_wk->p_list );
      if( ret != KEYSYS_LIST_SELECT_NULL )
      {
        KEYSYS_LIST_Exit(p_wk->p_list);
        p_wk->p_list = NULL;

        switch( ret )
        {
        case 0:
          (*p_seq)++;
          break;
        default:
          GF_ASSERT(0);
        case 1:
          *p_seq = SEQ_MSG_CANCEL_START;
          break;
        }
      }
    }
    break;
  case SEQ_CONVERT_MAIN:
    {
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      p_wk->is_convert = DATA_CONVERT_SAVE_Convert( p_wk->p_data_sv, p_wk->p_recv_data, p_sv );
    }
    (*p_seq)++;
    break;

 case SEQ_SAVE:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Save );
    (*p_seq)++;
    break;

  case SEQ_COMPLETE_MSG_START:
    if( p_wk->is_convert )
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_14, KEYSYS_TEXT_TYPE_STREAM_FIX );
    }
    else
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_06, KEYSYS_TEXT_TYPE_STREAM_FIX );
    }
    *p_seq = SEQ_MSG_END_WAIT;
    break;


  case SEQ_MSG_RETRY_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_11, KEYSYS_TEXT_TYPE_STREAM ); 
    (*p_seq)  = SEQ_MSG_RETRY_WAIT;
    break;

  case SEQ_MSG_RETRY_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      *p_seq = SEQ_MSG_INPUT_START;
    } 
    break;

  case SEQ_MSG_CANCEL_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_08, KEYSYS_TEXT_TYPE_STREAM ); 
    (*p_seq)  = SEQ_MSG_END_WAIT;
    break;

  case SEQ_MSG_NOSWAN_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_19, KEYSYS_TEXT_TYPE_STREAM ); 
    (*p_seq)  = SEQ_MSG_END_WAIT;
    break;

  case SEQ_MSG_NODATA_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_18, KEYSYS_TEXT_TYPE_STREAM ); 
    (*p_seq)  = SEQ_MSG_END_WAIT;
    break;

  case SEQ_MSG_END_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      *p_seq = SEQ_CLOSE_PROC_TO_WIFI_LOGOUT;
    }
    break;

  case SEQ_CLOSE_PROC_TO_WIFI_LOGOUT:
    KEYSYS_NET_Start( p_wk->p_net, KEYSYS_NET_TYPE_NONE );

    if( p_wk->p_download_data )
    {
      GFL_HEAP_FreeMemory( p_wk->p_download_data );
      p_wk->p_download_data = NULL;
    }

    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutBlack );
    (*p_seq)++;
    break;
  case SEQ_DELETE_DISPLAY_TO_WIFI_LOGOUT:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_SUBPROC_WIFI_LOGOUT:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_SubProc_WifiLogout );
    (*p_seq)++;
    break;
  case SEQ_CREATE_DISPLAY_TO_WIFI_LOGOUT:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayNetMenu );
    (*p_seq)++;
    break;
  case SEQ_OPEN_PROC_TO_WIFI_LOGOUT:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeInBlack );
    (*p_seq)++;
    break;
  case SEQ_DELETE_DISPLAY:
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  case SEQ_START_WIFI_DISABLE_MSG:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_16, KEYSYS_TEXT_TYPE_STREAM );
    (*p_seq)  = SEQ_START_WIFI_DISABLE_MSG;
    break;
  case SEQ_WAIT_WIFI_DISABLE_MSG:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq)  = SEQ_END;
    }
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  セーブシーケンス
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_Save( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_SAVE_MSG,
    SEQ_SAVE_MSG_WAIT,
    SEQ_SAVE,
    SEQ_SAVE_SE_START,
    SEQ_SAVE_SE_WAIT,
    SEQ_SAVE_COMPLETE_MSG,
    SEQ_SAVE_COMPLETE_WAIT,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_SAVE_MSG:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_30, KEYSYS_TEXT_TYPE_WAIT );
    (*p_seq)++;
    break;
  case SEQ_SAVE_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE:
    if( DataConv_Seq_ConvComplete( p_wk ) )
    {
      KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_SaveInAndOut );
    }
    else
    {
      KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_SaveOuter );
    }
    (*p_seq)++;
    break;

  case SEQ_SAVE_SE_START:
    PMSND_PlaySE( KEYSYS_SE_SAVE );
    (*p_seq) = SEQ_SAVE_SE_WAIT;
    break;

  case SEQ_SAVE_SE_WAIT:
    //if( !PMSND_CheckPlaySE() )
    {
      (*p_seq) = SEQ_SAVE_COMPLETE_MSG;
    }
    break;

  case SEQ_SAVE_COMPLETE_MSG:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_45, KEYSYS_TEXT_TYPE_STREAM_FIX );
    (*p_seq) = SEQ_SAVE_COMPLETE_WAIT;
    break;

  case SEQ_SAVE_COMPLETE_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text )
        && !PMSND_CheckPlaySE() )
    {
      (*p_seq) = SEQ_END;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;

  default:
    GF_ASSERT(0);

  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  管理外セーブのみ実行
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_SaveOuter( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_SAVE_START,
    SEQ_SAVE_WAIT,
    SEQ_SAVE_COMPLETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_SAVE_START:
    OutsideSave_SaveAsyncInit( p_wk->p_outside_sv );
    p_wk->cnt = 0;
    (*p_seq)++;
    break;
  case SEQ_SAVE_WAIT:
    p_wk->cnt++;
    if( OutsideSave_SaveAsyncMain( p_wk->p_outside_sv ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE_COMPLETE:
    if( p_wk->cnt++ > KEYSYS_SAVE_MSG_SYNC )
    {
      p_wk->cnt = 0;
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  }
}//----------------------------------------------------------------------------
/**
 *	@brief  内部セーブのみ実行
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_SaveInner( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_SAVE_START,
    SEQ_SAVE_WAIT,
    SEQ_SAVE_COMPLETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_SAVE_START:
    GAMEDATA_SaveAsyncStart(p_wk->p_param->p_gamedata);
    (*p_seq)++;
    break;
  case SEQ_SAVE_WAIT:
    if( GAMEDATA_SaveAsyncMain(p_wk->p_param->p_gamedata) == SAVE_RESULT_OK )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE_COMPLETE:
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  内部＋管理外セーブ実行
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_SaveInAndOut( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_SAVE_OUTER,
    SEQ_SAVE_INNER,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_SAVE_OUTER:
    //管理外をセーブする
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_SaveOuter );
    (*p_seq)++;
    break;
  case SEQ_SAVE_INNER:
    //内部をセーブする
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_SaveInner );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  } 
}
//----------------------------------------------------------------------------
/**
 *	@brief  思い出リストフロー
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_InfomationList( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_SELECT_MAIN,
    SEQ_DISPLAY_DELETE,
    SEQ_SELECT_CALL,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayInfomationList );
    (*p_seq)++;
    break;
  case SEQ_SELECT_MAIN:
    {
      p_wk->ret = KEYSYS_SCROLL_MENU_Main( p_wk->p_scroll_menu );
      if( p_wk->ret != KEYSYS_SCROLL_MENU_SELECT_NULL )
      {
        if( p_wk->ret != DATACONV_SCROLL_MENU_CANCEL )
        {
          u32 cursor;
          u32 list;
          KEYSYS_SCROLL_MENU_GetPos( p_wk->p_scroll_menu, &cursor, &list );
          KEYSYS_MAP_Insert( &p_wk->select, "DI1", cursor );
          KEYSYS_MAP_Insert( &p_wk->select, "DI2", list );
        }
        (*p_seq)++;
      }
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_SELECT_CALL:
    switch( p_wk->ret )
    {
    case DATACONV_SCROLL_MENU_CANCEL://キャンセル
      (*p_seq)++;
      break;

    default:  //デフォルト
      KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Explain );
      //もう一度メニュー
      (*p_seq ) = 0;
    }
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  説明画面
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_Explain( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_TEXT_START,
    SEQ_TEXT_MAIN,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayExplain );
    (*p_seq)++;
    break;
  case SEQ_TEXT_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, p_wk->ret + DATACONV_LISTINFO_00, KEYSYS_TEXT_TYPE_STREAM ); 
    (*p_seq)++;
    break;
  case SEQ_TEXT_MAIN:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信方式メニュー画面の作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_CreateDisplayNetMenu( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    DataConv_Display_CreateNetMenu,
    DataConv_Display_IsEndCreateNetMenu,
    DataConv_Display_DeleteNetMenu,
    NULL,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    //最初はメインメニュー作成
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl, HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  DLプレイ説明画面
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_CreateDisplayDLPlayInfo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    DataConv_Display_CreateDlPlay,
    DataConv_Display_IsEndCreateDlPlay,
    DataConv_Display_DeleteDlPlay,
    NULL,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    //最初はメインメニュー作成
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl, HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WiFI画面
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_CreateDisplayWifi( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    DataConv_Display_CreateWifi,
    DataConv_Display_IsEndCreateWifi,
    DataConv_Display_DeleteWifi,
    NULL,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    //最初はメインメニュー作成
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl, HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  説明リスト
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_CreateDisplayInfomationList( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    DataConv_Display_CreateInfomationList,
    DataConv_Display_IsEndCreateInfomationList,
    DataConv_Display_DeleteInfomationList,
    NULL,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    //最初はメインメニュー作成
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl, HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  説明画面
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_CreateDisplayExplain( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    DataConv_Display_CreateExplain,
    DataConv_Display_IsEndCreateExplain,
    DataConv_Display_DeleteExplain,
    NULL,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE_START:
    //最初はメインメニュー作成
    KEYSYS_DISPLAY_Create( p_wk->p_display, &sc_display_tbl, HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;

  case SEQ_DISPLAY_CREATE_WAIT:
    if( KEYSYS_DISPLAY_IsComplete(p_wk->p_display) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;

  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  WIFIログイン
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_SubProc_WifiLogin( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_NEXT_PROC,
    SEQ_MAIN,
    SEQ_FINISH_PROC,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_NEXT_PROC:
    KEYSYS_ExitTemporaryModules( p_wk );
    {

      WIFILOGIN_PARAM *p_param = GFL_HEAP_AllocClearMemory( HEAPID_KEY_SYSTEM, sizeof(WIFILOGIN_PARAM) );
      p_param->gamedata     = p_wk->p_param->p_gamedata;
      p_param->bg           = WIFILOGIN_BG_NORMAL;
      p_param->display      = WIFILOGIN_DISPLAY_UP;
      p_param->pSvl         = &p_wk->svl;
      p_param->bgm          = WIFILOGIN_BGM_NORMAL;
      p_param->nsid         = WB_NET_WIFI_DATACONV;

      if( SaveData_GetExistFlag( GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata) ) )
      {
        p_param->mode         = WIFILOGIN_MODE_NORMAL;
      }
      else
      {
        p_param->mode         = WIFILOGIN_MODE_NOTSAVE;
      }

      GF_ASSERT( p_wk->p_proc_param == NULL );
      p_wk->p_proc_param = p_param;

      GFL_PROC_SysCallProc( FS_OVERLAY_ID(wifi_login),
          &WiFiLogin_ProcData, p_wk->p_proc_param );
      (*p_seq)++;
    }
    break;
  case SEQ_MAIN:
    (*p_seq)++;
    break;
  case SEQ_FINISH_PROC:
    {
      WIFILOGIN_PARAM *p_param  = p_wk->p_proc_param;
      if( p_param->result == WIFILOGIN_RESULT_LOGIN )
      {
        p_wk->is_wifi_start = TRUE;
      }
      else
      {
        p_wk->is_wifi_start = FALSE;
      }
    }
    GFL_HEAP_FreeMemory( p_wk->p_proc_param );
    p_wk->p_proc_param = NULL;

    KEYSYS_InitTemporaryModules( p_wk, HEAPID_KEY_SYSTEM );
    KEYSYS_BG_TransScreen( p_wk->p_bg, KEYSYS_BG_FRAME_WND_M, KEYSYS_BG_SCR_TITLE_M );
    KEYSYS_BG_RequestPltFade( p_wk->p_bg, KEYSYS_BG_PLTFADE_REQ_MAIN_TO_DATACONV, 1 );

    GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_KEY_SYSTEM );

    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  WIFIログアウト
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_SubProc_WifiLogout( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_NEXT_PROC,
    SEQ_MAIN,
    SEQ_FINISH_PROC,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_NEXT_PROC:
    KEYSYS_ExitTemporaryModules( p_wk );
    {
      WIFILOGOUT_PARAM *p_param = GFL_HEAP_AllocClearMemory( HEAPID_KEY_SYSTEM, sizeof(WIFILOGOUT_PARAM) );
      p_param->gamedata = p_wk->p_param->p_gamedata;
      p_param->bg       = WIFILOGIN_BG_NORMAL;
      p_param->display  = WIFILOGIN_DISPLAY_UP;
      p_param->bgm      = WIFILOGIN_BGM_NORMAL;
      p_param->fade     = WIFILOGIN_FADE_DEFAULT;
      GF_ASSERT( p_wk->p_proc_param == NULL );
      p_wk->p_proc_param = p_param;

      GFL_PROC_SysCallProc( FS_OVERLAY_ID(wifi_login),
          &WiFiLogout_ProcData, p_wk->p_proc_param );
      (*p_seq)++;
    }
    break;
  case SEQ_MAIN:
    (*p_seq)++;
    break;
  case SEQ_FINISH_PROC:
    GFL_HEAP_FreeMemory( p_wk->p_proc_param );
    p_wk->p_proc_param = NULL;

    KEYSYS_InitTemporaryModules( p_wk, HEAPID_KEY_SYSTEM );
    KEYSYS_BG_TransScreen( p_wk->p_bg, KEYSYS_BG_FRAME_WND_M, KEYSYS_BG_SCR_TITLE_M );
    KEYSYS_BG_RequestPltFade( p_wk->p_bg, KEYSYS_BG_PLTFADE_REQ_MAIN_TO_DATACONV, 1 );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  名前入力
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_SubProc_NameIn( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_NEXT_PROC,
    SEQ_MAIN,
    SEQ_FINISH_PROC,
    SEQ_END,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_NEXT_PROC:
    KEYSYS_ExitTemporaryModules( p_wk );
    {
      NAMEIN_PARAM *p_param = GFL_HEAP_AllocClearMemory( HEAPID_KEY_SYSTEM, sizeof(NAMEIN_PARAM) );
      p_param->mode = NAMEIN_GAMESYNC_ID_CODE;
      p_param->wordmax = GAMESYNC_ID_CODE_LENGTH;
      p_param->prof_flag = NAMEIN_PROF_ALL_ENABLE;
      p_param->strbuf = GFL_STR_CreateBuffer( NAMEIN_STR_BUFFER_LENGTH + 1, HEAPID_KEY_SYSTEM );

      GF_ASSERT( p_wk->p_proc_param == NULL );
      p_wk->p_proc_param = p_param;

      GFL_PROC_SysCallProc(FS_OVERLAY_ID(namein),
          &NameInputProcData, p_wk->p_proc_param );
      (*p_seq)++;
    }
    break;
  case SEQ_MAIN:
    (*p_seq)++;
    break;
  case SEQ_FINISH_PROC:
    {
      NAMEIN_PARAM *p_param = p_wk->p_proc_param;
      if( p_param->cancel )
      {
        if( p_wk->p_gamesync_id_code )
        {
          GFL_STR_DeleteBuffer( p_wk->p_gamesync_id_code );
          p_wk->p_gamesync_id_code = NULL;
        }
      }
      else
      {
        if( p_wk->p_gamesync_id_code == NULL )
        {
          p_wk->p_gamesync_id_code = GFL_STR_CreateBuffer( GAMESYNC_ID_CODE_LENGTH + EOM_SIZE, HEAPID_KEY_SYSTEM );
        }
        GFL_STR_CopyBuffer( p_wk->p_gamesync_id_code, p_param->strbuf );
      }
      GFL_STR_DeleteBuffer( p_param->strbuf );
      GFL_HEAP_FreeMemory( p_param );
    }
    p_wk->p_proc_param = NULL;

    KEYSYS_InitTemporaryModules( p_wk, HEAPID_KEY_SYSTEM );
    KEYSYS_BG_TransScreen( p_wk->p_bg, KEYSYS_BG_FRAME_WND_M, KEYSYS_BG_SCR_TITLE_M );
    KEYSYS_BG_RequestPltFade( p_wk->p_bg, KEYSYS_BG_PLTFADE_REQ_MAIN_TO_DATACONV, 1 );

    GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Return( p_seqwk );
		break;
  default:
    GF_ASSERT(0);
  }
}
//=============================================================================
/**
 * 画面作成・破棄
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  通信方法メニュー作成
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_CreateNetMenu( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //タイトル
  KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, KEYSYS_STR_68, KEYSYS_TEXT_TYPE_QUE );

  //メニュー追加
  {
    int i;
    KEYSYS_MENU_PARAM menu_param;
    GFL_STD_MemClear( &menu_param, sizeof(KEYSYS_MENU_PARAM) );
    menu_param.frm = KEYSYS_BG_FRAME_LIST_M;
    menu_param.font_plt = KEYSYS_PLT_BG_FONT_M;
    menu_param.frm_chr  = KEYSYS_CGR_MENU_M;
    menu_param.frm_plt  = KEYSYS_PLT_BG_MENU_NORMAL_M;
    menu_param.p_msg    = p_wk->p_msg;
    menu_param.p_font   = p_wk->p_font;
    menu_param.max      = 3;
    if( KEYSYS_MAP_IsExist( &p_wk->select, "DNM" ) )
    {
      menu_param.cursor   = KEYSYS_MAP_GetValue( &p_wk->select, "DNM" );
      KEYSYS_MAP_Erase( &p_wk->select, "DNM" );
    }
    else
    {
      menu_param.cursor   = 0;
    }
    for( i = 0; i < 4; i++ )
    {
      menu_param.wnd_data[i].w  = 26;
      menu_param.wnd_data[i].h  = 2;
      menu_param.wnd_data[i].x  = 3;
      menu_param.wnd_data[i].y  = 5 + (i*(menu_param.wnd_data[i].h+2+1)); //+2はフレーム分+1は間隔分
    }
    menu_param.wnd_data[0].strID = KEYSYS_STR_69;
    menu_param.wnd_data[1].strID = KEYSYS_STR_70;

    if( DataConv_InfoList_GetVisibleAny(p_wk) )
    {
      menu_param.wnd_data[2].strID = KEYSYS_STR_85;
      menu_param.wnd_data[3].strID = KEYSYS_STR_71;
      menu_param.max  = 4;
    }
    else
    {
      menu_param.wnd_data[2].strID = KEYSYS_STR_71;
    }

    p_wk->p_menu = KEYSYS_MENU_Init( &menu_param, heapID );

    //メッセージ
    KEYSYS_INFO_Create( p_wk, heapID );
    KEYSYS_TEXT_Print( p_wk->p_info, p_wk->p_msg, KEYSYS_STR_72+menu_param.cursor, KEYSYS_TEXT_TYPE_QUE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  通信方法メニュー作成完了待ち
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_Display_IsEndCreateNetMenu( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_title  = KEYSYS_TEXT_IsMsgEnd( p_wk->p_title );
  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_info );
  BOOL is_end_menu = KEYSYS_MENU_IsDrawEnd( p_wk->p_menu );

  if( is_end_text && is_end_menu )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_LIST_M );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_S );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  通信方法メニュー破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_DeleteNetMenu( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //メニュー破棄
  KEYSYS_MENU_Exit( p_wk->p_menu );
  p_wk->p_menu = NULL;    
  
  //タイトルクリア
  KEYSYS_TEXT_ClearCharacter( p_wk->p_title );

  KEYSYS_INFO_Delete( p_wk );

  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_LIST_M );

}
//----------------------------------------------------------------------------
/**
 *	@brief  DLプレイメニュー作成
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_CreateDlPlay( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  p_wk->p_text = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 2, 4, 28, 16, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, HEAPID_KEY_SYSTEM );
  KEYSYS_TEXT_SetColor( p_wk->p_text, PRINTSYS_LSB_Make( 1, 2, 0xF ) );
  KEYSYS_TEXT_WriteWindowFrame( p_wk->p_text, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M );

  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
    if( !SaveData_GetExistFlag( p_sv ) )
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, DATACONV_STR_00_2, KEYSYS_TEXT_TYPE_QUE ); 
    }
    else
    {
      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus(p_wk->p_param->p_gamedata);
      STRBUF *p_src = GFL_MSG_CreateString( p_wk->p_msg, DATACONV_STR_00 );
      STRBUF *p_dst = GFL_STR_CreateBuffer( 128, heapID );

      WORDSET_RegisterNumber( p_wk->p_word, 0, MyStatus_GetID_Low(p_mystatus), 5, STR_NUM_DISP_ZERO , STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( p_wk->p_word, p_dst, p_src );

      KEYSYS_TEXT_PrintBuf( p_wk->p_text, p_dst, KEYSYS_TEXT_TYPE_QUE ); 

      GFL_STR_DeleteBuffer( p_dst );
      GFL_STR_DeleteBuffer( p_src );    
    }
  }

  //BG読み込み
 {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_MB_PARENT, heapID );

    //----背景----
    //PLT
    GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_mb_parent_ss_NCLR,
        PALTYPE_SUB_BG, KEYSYS_PLT_BG_DLPLAY_S*0x20, KEYSYS_PLT_BG_DLPLAY_S*0x20, 0x20*3, heapID );
    GFL_ARCHDL_UTIL_TransVramPaletteEx( p_handle, NARC_mb_parent_ss_msg_NCLR,
        PALTYPE_SUB_BG, KEYSYS_PLT_BG_DLMSG_S*0x20, KEYSYS_PLT_BG_DLMSG_S*0x20, 0x20, heapID );
    //CHR
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mb_parent_ss_msg_NCGR, 
        KEYSYS_BG_FRAME_TEXT_S, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_mb_parent_ss_NCGR, 
        KEYSYS_BG_FRAME_WND_S, 0, 32*24*32, FALSE, heapID );
    //SCR
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mb_parent_ss_msg_NSCR,
        KEYSYS_BG_FRAME_TEXT_S, 0, 0, FALSE, heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_mb_parent_ss_NSCR,
        KEYSYS_BG_FRAME_WND_S, 0, 0, FALSE, heapID );
    GFL_ARC_CloseDataHandle( p_handle );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  DLプレイ画面作成完了待ち
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_Display_IsEndCreateDlPlay( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );

  if( is_end_text )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_S );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_WND_S );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  DLプレイ画面破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_DeleteDlPlay( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //前呼んでいたのが上書きされてしまったので読み直し
  {
    ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_KEY_SYSTEM, HEAPID_KEY_SYSTEM );

    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_key_system_gra_back00_NCGR, 
        KEYSYS_BG_FRAME_WND_S, 0, 0, FALSE, HEAPID_KEY_SYSTEM );

    GFL_ARC_CloseDataHandle( p_handle );


    BmpWinFrame_GraphicSet( KEYSYS_BG_FRAME_TEXT_S, KEYSYS_CGR_TEXT_S, KEYSYS_PLT_BG_FRAME_S, MENU_TYPE_SYSTEM, HEAPID_KEY_SYSTEM );
  }

  KEYSYS_TEXT_Delete( p_wk );

  GFL_BG_ClearScreen( KEYSYS_BG_FRAME_TEXT_S );
  GFL_BG_ClearScreen( KEYSYS_BG_FRAME_WND_S );

  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_S );
  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_WND_S );
}
//----------------------------------------------------------------------------
/**
 *	@brief  WiFi画面作成
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_CreateWifi( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  p_wk->p_text = KEYSYS_TEXT_Init( KEYSYS_BG_FRAME_TEXT_M, 2, 4, 28, 16, KEYSYS_PLT_BG_FONT_M, p_wk->p_font, HEAPID_KEY_SYSTEM );
  KEYSYS_TEXT_SetColor( p_wk->p_text, PRINTSYS_LSB_Make( 1, 2, 0xF ) );
  KEYSYS_TEXT_WriteWindowFrame( p_wk->p_text, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M );
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, 0, KEYSYS_TEXT_TYPE_QUE ); 

  KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, KEYSYS_STR_68, KEYSYS_TEXT_TYPE_QUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  WiFi画面作成完了待ち
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *
 *	@return TRUE作成完了  FALSE作成中
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_Display_IsEndCreateWifi( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_text = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );
  BOOL is_end_title = KEYSYS_TEXT_IsMsgEnd( p_wk->p_title );

  if( is_end_text && is_end_title )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  WiFi画面破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_DeleteWifi( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;
}

//----------------------------------------------------------------------------
/**
 *	@brief  思い出リスト画面作成
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_CreateInfomationList( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //タイトル
  KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, DATACONV_STR_15, KEYSYS_TEXT_TYPE_QUE );

  //スクロールリスト
  {
    int i;
    KEYSYS_SCROLL_MENU_SETUP setup;
    GFL_STD_MemClear( &setup, sizeof(KEYSYS_SCROLL_MENU_SETUP) );
    setup.p_msg   = p_wk->p_msg;
    setup.p_font  = p_wk->p_font;
    setup.p_up    = KEYSYS_OBJ_AddClwk( &p_wk->obj, KEYSYS_CLWK_INFO_ALLOW_U, heapID );
    setup.p_down  = KEYSYS_OBJ_AddClwk( &p_wk->obj, KEYSYS_CLWK_INFO_ALLOW_D, heapID );
    
    //項目の追加
    for( i = 0; i < DATACONV_INFO_LIST_MAX; i++ )
    {
      if( DataConv_InfoList_GetVisible(p_wk, i ) )
      {
        setup.strID[setup.list_max]  = DATACONV_LIST_00+i;
        setup.param[setup.list_max]  = i;
        if( DataConv_InfoList_GetMark( p_wk, i ) )
        {
          setup.p_mark[setup.list_max]   = KEYSYS_OBJ_AddClwk( &p_wk->obj, KEYSYS_CLWK_INFO_MARK_00+i, heapID );
;
        }
        setup.list_max++;
      }
    }

    //戻るの追加
    setup.strID[setup.list_max]  = DATACONV_LIST_CANCEL;
    setup.param[setup.list_max]  = DATACONV_SCROLL_MENU_CANCEL;
    setup.p_mark[setup.list_max]   = NULL;
    setup.list_max++;

    setup.frm = KEYSYS_BG_FRAME_LIST_M;
    setup.plt = KEYSYS_PLT_BG_MENU_NORMAL_M;
    setup.char_ofs  = KEYSYS_CGR_MENU_M;

    if( KEYSYS_MAP_IsExist( &p_wk->select, "DI1" ) )
    {
      setup.cursor_pos  = KEYSYS_MAP_GetValue( &p_wk->select, "DI1" );
      setup.list_pos    = KEYSYS_MAP_GetValue( &p_wk->select, "DI2" );
      KEYSYS_MAP_Erase( &p_wk->select, "DI1" );
      KEYSYS_MAP_Erase( &p_wk->select, "DI2" );
    }

    p_wk->p_scroll_menu = KEYSYS_SCROLL_MENU_Init( &setup, heapID );
    GX_SetVisiblePlane(GX_PLANEMASK_BG0|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3);
    p_wk->cnt = 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  思い出リスト画面待ち
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *
 *	@return TRUE作成完了  FALSE作成中
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_Display_IsEndCreateInfomationList( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  BOOL is_end_menu = KEYSYS_SCROLL_MENU_Draw( p_wk->p_scroll_menu );
  BOOL is_end_title = KEYSYS_TEXT_IsMsgEnd( p_wk->p_title );

  if( is_end_menu && is_end_title )
  {
    //どうしても１フレOBJが見えてしまうので…
    if( p_wk->cnt++ > 1 )
    {
      GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
      GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_LIST_M );
      GX_SetVisiblePlane(GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ);
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  思い出リスト画面破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_DeleteInfomationList( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  KEYSYS_SCROLL_MENU_Exit( p_wk->p_scroll_menu );
  p_wk->p_scroll_menu = NULL;

  {
    int i;
    for( i = KEYSYS_CLWK_INFO_MARK_00; i < KEYSYS_CLWK_INFO_MARK_MAX; i++ )
    {
      KEYSYS_OBJ_RemoveClwk( &p_wk->obj, i );
    }
  }

  KEYSYS_OBJ_RemoveClwk( &p_wk->obj, KEYSYS_CLWK_INFO_ALLOW_U );
  KEYSYS_OBJ_RemoveClwk( &p_wk->obj, KEYSYS_CLWK_INFO_ALLOW_D );

	GFL_BG_ClearScreen( KEYSYS_BG_FRAME_LIST_M );
  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_LIST_M );
}

//----------------------------------------------------------------------------
/**
 *	@brief  思い出リスト説明画面作成
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_CreateExplain( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //タイトル
  //KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, DATACONV_STR_15, KEYSYS_TEXT_TYPE_QUE );

  //スクロールリスト
  KEYSYS_TEXT_Create( p_wk, heapID );
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, 0, KEYSYS_TEXT_TYPE_QUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  思い出リスト説明画面待ち
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *
 *	@return TRUE作成完了  FALSE作成中
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_Display_IsEndCreateExplain( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  if( KEYSYS_TEXT_IsMsgEnd(p_wk->p_text) )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  思い出リスト説明画面破棄
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Display_DeleteExplain( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  KEYSYS_TEXT_Delete( p_wk );

  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
}


//----------------------------------------------------------------------------
/**
 *	@brief  コンバート後の処理
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 *
 *	@retval TRUE  内部セーブを変更した
 *	@retval FALSE していない
 *
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_Seq_ConvComplete( KEY_SYSTEM_WORK *p_wk )
{
  BOOL change = FALSE;

  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(p_wk->p_param->p_gamedata );

  //セーブがあるときだけ
  if( SaveData_GetExistFlag(p_sv) )
  {
    EVENTWORK *p_ev = SaveData_GetEventWork(p_sv);

    //Nポケ起動条件を見て、フラグを立てる
    if( EVENTWORK_CheckEventFlag( p_ev, FE_C05R0501_POKE22_GET )
      && DATA_CONVERT_SAVE_GetFlag(p_wk->p_data_sv, WB_SAVE_CONV_FLAG_GAME_CLEAR) )
    {
      EVENTWORK_SetEventFlag( p_ev, SYS_FLAG_NPOKE_OPEN );
      change = TRUE;
    }
    return change;
  }
  
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  おもいでリストの表示
 *
 *	@param	const KEY_SYSTEM_WORK *cp_wk  ワーク
 *	@param	index インデックス
 *
 *	@return TRUE表示してよい  ＦＡＬＳＥだめ
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_InfoList_GetVisible( const KEY_SYSTEM_WORK *cp_wk, u32 index )
{
#ifdef DEBUG_DATACONV_LIST_OPEN
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    return TRUE;
  }
#endif
  return DATA_CONVERT_SAVE_IsEventOpen( cp_wk->p_data_sv, index );
}

//----------------------------------------------------------------------------
/**
 *	@brief  おもいでリストの表示  どれかがTRUE
 *
 *	@param	const KEY_SYSTEM_WORK *cp_wk  ワーク
 *
 *	@return TRUE表示してよい  ＦＡＬＳＥだめ
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_InfoList_GetVisibleAny( const KEY_SYSTEM_WORK *cp_wk )
{
  int i;
  for( i = 0; i < DATACONV_INFO_LIST_MAX; i++ )
  {
    if( DataConv_InfoList_GetVisible( cp_wk, i ) )
    {
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  マークをつくフラグ取得
 *
 *	@param	const KEY_SYSTEM_WORK *cp_wk  ワーク
 *	@param	index   インデックス
 *
 *	@return TRUEマーク  FALSEつかない
 */
//-----------------------------------------------------------------------------
static BOOL DataConv_InfoList_GetMark( const KEY_SYSTEM_WORK *cp_wk, u32 index )
{
  EVENTWORK *p_ev = SaveData_GetEventWork(GAMEDATA_GetSaveControlWork(cp_wk->p_param->p_gamedata));
  u32 work_num = DATA_CONVERT_SAVE_GetEventWork( index );
  u16 *p_ev_wk  = EVENTWORK_GetEventWorkAdrs( p_ev, work_num );

#ifdef DEBUG_DATACONV_LIST_OPEN
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    return TRUE;
  }
#endif

  return *p_ev_wk == 1;
}
