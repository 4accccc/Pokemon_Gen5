//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		cygnus_flow.c
 *	@brief  シグナス受け取りフロー
 *	@author	Toru=Nagihashi
 *	@date		2012.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "gamesystem/game_data.h"
#include "system/gfl_use.h"
#include "system/bmp_winframe.h"
#include "print/wordset.h"
#include "field/zonedata.h"
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"
#include "app/name_input.h"
#include "system/net_err.h"
#include "net_app/pdwacc_gamesync_id_inline.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_memo.h"
#include "item/itemsym.h"
#include "poke_tool/poke_personal.h"

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
#include "key_system_spa.h"
#include "system\poke2dgra.h"
#include "pokeicon/pokeicon.h"
#include "item/item.h"
#include "item_icon.naix"

//自分のモジュール
#include "key_system_util.h"
#include "key_system_graphic.h"
#include "key_system_net.h"
#include "key_system_main.h"
#include "key_system_snd.h"

//セーブ
#include "savedata/save_outside.h"
#include "savedata/outside_permanent.h"
#include "field/eventwork.h"
#include "field/eventwork_def.h"
#include "savedata/cygnus_save_control.h"
#include "savedata/cygnus_data.h"
#include "savedata/dummy3ds_savedata.h"
#include "savedata/myitem_savedata.h"
#include "savedata/box_savedata.h"

//外部公開
#include "cygnus_flow.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

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
static void DataConv_Seq_DisableRecive( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_BreakSave( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_ReciveData( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void DataConv_Seq_ReciveWait( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_Demo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_DemoOne( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_Result( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//画面構築シーケンス
static void DataConv_Seq_CreateDisplayStart( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_CreateDisplayDisableRecive( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DataConv_Seq_CreateDisplayReciveWait( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
/// 画面作成・破棄
//=====================================
//メインメニュー
static void Cygnus_Display_CreateStart( void *p_wk_adrs, HEAPID heapID );
static BOOL Cygnus_Display_IsEndCreateStart( void *p_wk_adrs );
static void Cygnus_Display_DeleteStart( void *p_wk_adrs );
//受け取らない
static void Cygnus_Display_CreateDisableRecive( void *p_wk_adrs, HEAPID heapID );
static BOOL Cygnus_Display_IsEndCreateDisableRecive( void *p_wk_adrs );
static void Cygnus_Display_DeleteDisableRecive( void *p_wk_adrs );
//受取り待機
static void Cygnus_Display_CreateReciveWait( void *p_wk_adrs, HEAPID heapID );
static BOOL Cygnus_Display_IsEndCreateReciveWait( void *p_wk_adrs );
static void Cygnus_Display_DeleteReciveWait( void *p_wk_adrs );
//-------------------------------------
///	デモ
//=====================================
static CYGNUS_DEMO_WORK * CYGNUS_DEMO_Init( KEYSYS_PTC_WORK *p_ptc, const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, HEAPID heapID );
static void CYGNUS_DEMO_Start( CYGNUS_DEMO_WORK *p_wk );
static void CYGNUS_DEMO_Exit( CYGNUS_DEMO_WORK *p_wk );
static void CYGNUS_DEMO_Main( CYGNUS_DEMO_WORK *p_wk );
static BOOL CYGNUS_DEMO_IsEnd( const CYGNUS_DEMO_WORK *cp_wk );
static void CYGNUS_DEMO_Load( CYGNUS_DEMO_WORK *p_wk, POKEMON_PARAM* p_pp );
static void CYGNUS_DEMO_Release( CYGNUS_DEMO_WORK *p_wk );
static void Cygnus_Demo_EffectMain( CYGNUS_DEMO_WORK *p_wk );

//-------------------------------------
/// シグナスセーブ
//=====================================
#define CYGNUS_REWARD_POKE_NUM  (6)
#define CYGNUS_REWARD_ITEM_NUM  (6)
#define CYGNUS_REWARD_DATA_NUM  (CYGNUS_REWARD_POKE_NUM+CYGNUS_REWARD_ITEM_NUM)

static CYGNUS_SAVE_WORK * CYGNUS_SAVE_Init( GAMEDATA *p_gamedata, HEAPID heapID );
static void CYGNUS_SAVE_Exit( CYGNUS_SAVE_WORK *p_wk );

typedef enum
{
  CYGNUS_SAVE_STATUS_NONE,  //一度もやってない
  CYGNUS_SAVE_STATUS_EMPTY,  //一度はやったがない
  CYGNUS_SAVE_STATUS_EXIST, //ある
  CYGNUS_SAVE_STATUS_BREAK, //壊れている
}CYGNUS_SAVE_STATUS;
static CYGNUS_SAVE_STATUS CYGNUS_SAVE_GetStatus( const CYGNUS_SAVE_WORK *cp_wk );

static BOOL CYGNUS_SAVE_IsEffectPoke( const CYGNUS_SAVE_WORK *cp_wk );
static u32 CYGNUS_SAVE_GetEffectPoke( const CYGNUS_SAVE_WORK *cp_wk, u32 idx );

static void CYGNUS_SAVE_StartSave( CYGNUS_SAVE_WORK *p_wk );
static BOOL CYGNUS_SAVE_MainSave( CYGNUS_SAVE_WORK *p_wk );

typedef enum
{
  CYGNUS_REWARD_TYPE_NONE,
  CYGNUS_REWARD_TYPE_POKEMON,
  CYGNUS_REWARD_TYPE_ITEM,
}CYGNUS_REWARD_TYPE;
static BOOL CYGNUS_SAVE_GetReward( const CYGNUS_SAVE_WORK *cp_wk, u32 idx, CYGNUS_REWARD_TYPE *p_type, u32 *p_no );
static POKEMON_PARAM * CYGNUS_SAVE_CreatePP( const CYGNUS_SAVE_WORK *cp_wk, u32 idx, HEAPID heapID );
static BOOL CYGNUS_SAVE_GetItem( const CYGNUS_SAVE_WORK *cp_wk, u32 idx, u16 *p_item, u8 *p_num );
static u32 CYGNUS_SAVE_GetItemCount( const CYGNUS_SAVE_WORK *cp_wk, u32 idx );
static u32 CYGNUS_SAVE_GetRewardPokeMax( const CYGNUS_SAVE_WORK *cp_wk );
static u32 CYGNUS_SAVE_GetRewardItemMax( const CYGNUS_SAVE_WORK *cp_wk );
static u32 CYGNUS_SAVE_GetRewardMax( const CYGNUS_SAVE_WORK *cp_wk );

static void CYGNUS_SAVE_SetGameDataReward( CYGNUS_SAVE_WORK *p_wk, HEAPID heapID );

//-------------------------------------
/// 結果
//=====================================
static CYGNUS_RESULT_WORK * CYGNUS_RESULT_Init( const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, const CYGNUS_SAVE_WORK *cp_save, HEAPID heapID );
static void CYGNUS_RESULT_Exit( CYGNUS_RESULT_WORK *p_wk );
static void CYGNUS_RESULT_Main( CYGNUS_RESULT_WORK *p_wk );
static void CYGNUS_RESULT_Start( CYGNUS_RESULT_WORK *p_wk );
static BOOL CYGNUS_RESULT_IsEnd( const CYGNUS_RESULT_WORK *cp_wk );
static void CYGNUS_RESULT_Load( CYGNUS_RESULT_WORK *p_wk );
static void CYGNUS_RESULT_Release( CYGNUS_RESULT_WORK *p_wk );

//-------------------------------------
/// その他
//=====================================
static void Cygnus_3d_Init( KEY_SYSTEM_WORK *p_wk, HEAPID heapID );
static void Cygnus_3d_Exit( KEY_SYSTEM_WORK *p_wk );

static void Cygnus_Demo_Graphic_Set( KEY_SYSTEM_WORK *p_wk, HEAPID heapID );
static void Cygnus_Demo_Graphic_Reset( KEY_SYSTEM_WORK *p_wk );

typedef enum
{
  CYGNUS_RECIVE_NG,
  CYGNUS_RECIVE_BREAK,
  CYGNUS_RECIVE_OK,
}CYGNUS_RECIVE;
static CYGNUS_RECIVE Cygnus_IsReciveEnable( KEY_SYSTEM_WORK *p_wk, u32 *p_error_msg );
static u32 Cygnus_GetLegPos( const POKEMON_PASO_PARAM* cp_ppp, HEAPID heapID );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスフロー用設定
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_CYGNUSFLOW_Init( KEY_SYSTEM_WORK *p_wk, HEAPID heapID )
{
  p_wk->p_cyg_sv  = CYGNUS_SAVE_Init( p_wk->p_param->p_gamedata, heapID );

  p_wk->p_cyg_result  = CYGNUS_RESULT_Init( p_wk->p_graphic, p_wk->p_cyg_sv, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスフロー用破棄
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_CYGNUSFLOW_Exit( KEY_SYSTEM_WORK *p_wk )
{
  CYGNUS_RESULT_Exit( p_wk->p_cyg_result );
  CYGNUS_SAVE_Exit( p_wk->p_cyg_sv );
}
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスフロー
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
void KEYSYS_CYGNUSFLOW_SEQ_Main( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_MSG_START,
    SEQ_YESNO_START,
    SEQ_YESNO_DRAW_WAIT,
    SEQ_YESNO_WAIT,
    SEQ_NEXT_DELETE,
    SEQ_NEST_CALL,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成フロー
    KEYSYS_TEXT_Create( p_wk, HEAPID_KEY_SYSTEM );
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayStart );
    (*p_seq)++;
    break;
  case SEQ_MSG_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_01, KEYSYS_TEXT_TYPE_STREAM ); 
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
        switch( ret )
        {
        case 0:
          *p_seq  = SEQ_NEXT_DELETE;
          break;
        default:
          GF_ASSERT(0);
        case 1:
          *p_seq = SEQ_DISPLAY_DELETE;
          break;
        }
      }
    }
    break;

  case SEQ_NEXT_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_NEST_CALL:
    switch( Cygnus_IsReciveEnable( p_wk, NULL ) )
    {
    case CYGNUS_RECIVE_OK:
      //受け取れる
      KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_ReciveData );
      break;
    case CYGNUS_RECIVE_NG:
      //受け取れない
      KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_DisableRecive );
      break;
    case CYGNUS_RECIVE_BREAK:
      //壊れていた
      KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_BreakSave );
      break;
    }
    (*p_seq)++;
    break;
  case SEQ_DISPLAY_DELETE:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_DeleteDisplay );
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_TEXT_Delete( p_wk );
    KEYSYS_BG_RequestPltFade( p_wk->p_bg, KEYSYS_BG_PLTFADE_REQ_CYGNUS_TO_MAIN, 30 );
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信できない
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_DisableRecive( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_PRINT_START,
    SEQ_PRINT_WAIT,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成フロー
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayDisableRecive );
    (*p_seq)++;
    break;
  case SEQ_PRINT_START:
    {
      u32 error_msg;
      CYGNUS_RECIVE ret = Cygnus_IsReciveEnable( p_wk, &error_msg );
      GF_ASSERT( ret == CYGNUS_RECIVE_NG );
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, error_msg, KEYSYS_TEXT_TYPE_STREAM ); 
    }
    (*p_seq)++;
    break;
  case SEQ_PRINT_WAIT:
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
 *	@brief  壊れている
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_BreakSave( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_PRINT01_START,
    SEQ_PRINT01_WAIT,
    SEQ_PRINT02_START,
    SEQ_PRINT02_WAIT,
    SEQ_SAVE_START,
    SEQ_SAVE_WAIT, 
    SEQ_PRINT03_START,
    SEQ_PRINT03_WAIT,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    //最初はメインメニュー作成フロー
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayDisableRecive );
    (*p_seq)++;
    break;
  case SEQ_PRINT01_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_12, KEYSYS_TEXT_TYPE_STREAM ); 
    (*p_seq)++;
    break;
  case SEQ_PRINT01_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_PRINT02_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_13, KEYSYS_TEXT_TYPE_WAIT ); 
    (*p_seq)++;
    break;
  case SEQ_PRINT02_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_SAVE_START:
    CYGNUS_SAVE_StartSave( p_wk->p_cyg_sv );
    (*p_seq)++;
    break;
  case SEQ_SAVE_WAIT:
    if( CYGNUS_SAVE_MainSave(p_wk->p_cyg_sv) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_PRINT03_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_14, KEYSYS_TEXT_TYPE_STREAM ); 
    (*p_seq)++;
    break;
  case SEQ_PRINT03_WAIT:
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
 *	@brief  データ受信
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_ReciveData( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_RECIVE_WAIT,
    SEQ_CHECK_SP_POKE,
    SEQ_CALL_DEMO,
    SEQ_CALL_RESULT,
    SEQ_END,
  };
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_CALL_RECIVE_WAIT:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_ReciveWait );
    (*p_seq)++;
    break;
  case SEQ_CHECK_SP_POKE:
    if( CYGNUS_SAVE_IsEffectPoke(p_wk->p_cyg_sv) )
    {
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutBlack );
      (*p_seq)  = SEQ_CALL_DEMO;
    }
    else
    {
      KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutWhite );
      (*p_seq)  = SEQ_CALL_RESULT;
    }
    break;
  case SEQ_CALL_DEMO:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Demo );
    (*p_seq)++;
    break;
  case SEQ_CALL_RESULT:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_Result );
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
 *	@brief  受信待機
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_ReciveWait( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE,
    SEQ_MSG_START,
    SEQ_WAIT,
    SEQ_DISPLAY_DELETE,
    SEQ_END,
  };

  enum
  {
    RECIVE_WAIT_SYNC  = 180,
  };

  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DISPLAY_CREATE:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_CreateDisplayReciveWait );
    (*p_seq)++;
    break;
  case SEQ_MSG_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_02, KEYSYS_TEXT_TYPE_WAIT ); 
    (*p_seq)++;
    break;
  case SEQ_WAIT:
    if( p_wk->cnt++ > RECIVE_WAIT_SYNC )
    {
      p_wk->cnt = 0;
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
 *	@brief  デモ
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_Demo( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_FADEIN_S_START,

    SEQ_CALL_DEMO,
    SEQ_CHECK_END_SP_POKE,

    SEQ_FADEOUT_E_START,
    SEQ_END,
  };
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_INIT:
    Cygnus_3d_Init( p_wk, HEAPID_KEY_SYSTEM );
    Cygnus_Demo_Graphic_Set( p_wk, HEAPID_KEY_SYSTEM );
    (*p_seq)++;
    break;
  case SEQ_FADEIN_S_START:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeInBlack );
    (*p_seq)++;
    break;
  case SEQ_CALL_DEMO:
    KEYSYS_SEQ_Call( p_seqwk, DataConv_Seq_DemoOne );
    (*p_seq)++;
    break;
  case SEQ_CHECK_END_SP_POKE:
    if( CYGNUS_SAVE_GetEffectPoke(p_wk->p_cyg_sv, ++p_wk->cnt ) != 0 )
    {
      (*p_seq)  = SEQ_CALL_DEMO;
    }
    else
    {
      p_wk->cnt = 0;
      (*p_seq)++;
    }
    break;

  case SEQ_FADEOUT_E_START:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeOutWhite );
    (*p_seq)++;
    break;
  case SEQ_END:
    Cygnus_3d_Exit( p_wk );
    Cygnus_Demo_Graphic_Reset( p_wk );
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  デモ一つ
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_DemoOne( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DEMO_START,
    SEQ_DEMO_WAIT,
    SEQ_MSG_START,
    SEQ_SE_WAIT,
    SEQ_MSG_WAIT,
    SEQ_END,
  };
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_DEMO_START:
    {
      POKEMON_PARAM *p_pp = CYGNUS_SAVE_CreatePP( p_wk->p_cyg_sv, p_wk->cnt, HEAPID_KEY_SYSTEM );
      CYGNUS_DEMO_Load( p_wk->p_cyg_demo, p_pp );
      GFL_HEAP_FreeMemory( p_pp );
    }
    CYGNUS_DEMO_Start( p_wk->p_cyg_demo );
    (*p_seq)++;
    /*  fall  */
  case SEQ_DEMO_WAIT:
    CYGNUS_DEMO_Main( p_wk->p_cyg_demo );
    if( CYGNUS_DEMO_IsEnd( p_wk->p_cyg_demo ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_MSG_START:

    PMSND_PlaySE( CYGNUS_SE_RECV_PRESENT );
    KEYSYS_TEXT_CreateEx( p_wk, KEYSYS_BG_FRAME_WND_M, HEAPID_KEY_SYSTEM );
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_WND_M );
    WORDSET_RegisterPokeMonsNameNo( p_wk->p_word, 0, CYGNUS_SAVE_GetEffectPoke(p_wk->p_cyg_sv,p_wk->cnt) );
    {
      STRBUF* p_str = WORDSET_TOOL_CreateExpandStr( p_wk->p_word, p_wk->p_msg, CYGNUS_04, HEAPID_KEY_SYSTEM );
      KEYSYS_TEXT_PrintBuf( p_wk->p_text, p_str, KEYSYS_TEXT_TYPE_STREAM_WAIT );
      GFL_STR_DeleteBuffer( p_str );
    }
    (*p_seq)++;
    break;
  case SEQ_SE_WAIT:
    if( PMSND_CheckPlaySE() == FALSE )
    {
      KEYSYS_TEXT_SetMsgCursor(p_wk->p_text);
      (*p_seq)++;
    }
    /* fall */
  case SEQ_MSG_WAIT:
    if(KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ))
    {
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    CYGNUS_DEMO_Release( p_wk->p_cyg_demo );
    KEYSYS_TEXT_DeleteEx( p_wk, KEYSYS_BG_FRAME_WND_M );
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  何を貰ったか結果画面
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_Result( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_FADEIN_START,
    SEQ_MSG_START,
    SEQ_RESULT_LOAD,
    SEQ_START,
    SEQ_WAIT,
    SEQ_FINISH_MSG_START,
    SEQ_FINISHSE_WAIT,
    SEQ_FINISH_MSG_WAIT,

    SEQ_BOX_MSG_START,
    SEQ_BOX_MSG_WAIT,
    SEQ_BAG_MSG_START,
    SEQ_BAG_MSG_WAIT,

    SEQ_END,
  };
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;


  switch( *p_seq )
  {
  case SEQ_INIT:
    CYGNUS_SAVE_SetGameDataReward( p_wk->p_cyg_sv, HEAPID_KEY_SYSTEM );
    KEYSYS_BG_TransScreen( p_wk->p_bg, KEYSYS_BG_FRAME_WND_M, KEYSYS_BG_SCR_TRAY_M );
    (*p_seq)++;
    break;
  case SEQ_FADEIN_START:
    KEYSYS_SEQ_Call( p_seqwk, KeySys_Seq_FadeInWhite );
    (*p_seq)++;
    break;
  case SEQ_MSG_START:
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_03, KEYSYS_TEXT_TYPE_WAIT );
    (*p_seq)++;
    break;
  case SEQ_RESULT_LOAD:
    CYGNUS_RESULT_Load(p_wk->p_cyg_result );
    (*p_seq)++;
    break;
  case SEQ_START:
    CYGNUS_SAVE_StartSave( p_wk->p_cyg_sv );
    CYGNUS_RESULT_Start( p_wk->p_cyg_result );
    (*p_seq)++;
    break;
  case SEQ_WAIT:
    {
      BOOL is_save_end    = CYGNUS_SAVE_MainSave(p_wk->p_cyg_sv);
      BOOL is_result_end  = CYGNUS_RESULT_IsEnd(p_wk->p_cyg_result);
      CYGNUS_RESULT_Main(p_wk->p_cyg_result);

      if( is_save_end && is_result_end )
      {
        (*p_seq)++;
      }
    }
    break;
  case SEQ_FINISH_MSG_START:
    PMSND_PlaySE( KEYSYS_SE_SAVE );
    KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_05, KEYSYS_TEXT_TYPE_STREAM_WAIT );
    (*p_seq)++;
    break;

  case SEQ_FINISHSE_WAIT:
    if( PMSND_CheckPlaySE() == FALSE )
    {
      KEYSYS_TEXT_SetMsgCursor(p_wk->p_text);
      (*p_seq)++;
    }
    /* fall */

  case SEQ_FINISH_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd(p_wk->p_text) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_BOX_MSG_START:
    if( CYGNUS_SAVE_GetRewardPokeMax(p_wk->p_cyg_sv) > 0 )
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_10, KEYSYS_TEXT_TYPE_STREAM );
      (*p_seq)++;
    }
    else
    {
      (*p_seq)  = SEQ_BAG_MSG_START;
    }
    break;
  case SEQ_BOX_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd(p_wk->p_text) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_BAG_MSG_START:
    if( CYGNUS_SAVE_GetRewardItemMax(p_wk->p_cyg_sv) > 0 )
    {
      KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, CYGNUS_11, KEYSYS_TEXT_TYPE_STREAM );
      (*p_seq)++;
    }
    else
    {
      (*p_seq)  = SEQ_END;
    }
    break;
  case SEQ_BAG_MSG_WAIT:
    if( KEYSYS_TEXT_IsMsgEnd(p_wk->p_text) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    CYGNUS_RESULT_Release( p_wk->p_cyg_result );

    KEYSYS_BG_TransScreen( p_wk->p_bg, KEYSYS_BG_FRAME_WND_M, KEYSYS_BG_SCR_TITLE_M );
    KEYSYS_SEQ_Return( p_seqwk );
    break;
  default:
    GF_ASSERT(0);

  }
}

//=============================================================================
/**
 *  画面構築フロー
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  最初の画面作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_CreateDisplayStart( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    Cygnus_Display_CreateStart,
    Cygnus_Display_IsEndCreateStart,
    Cygnus_Display_DeleteStart,
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
 *	@brief  受け取れない
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_CreateDisplayDisableRecive( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    Cygnus_Display_CreateDisableRecive,
    Cygnus_Display_IsEndCreateDisableRecive,
    Cygnus_Display_DeleteDisableRecive,
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
 *	@brief  受信待機
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DataConv_Seq_CreateDisplayReciveWait( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_DISPLAY_CREATE_START,
    SEQ_DISPLAY_CREATE_WAIT,
    SEQ_END,
  };

  static const KEYSYS_DISPLAY_FUNCTION sc_display_tbl =
  {
    Cygnus_Display_CreateReciveWait,
    Cygnus_Display_IsEndCreateReciveWait,
    Cygnus_Display_DeleteReciveWait,
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
//=============================================================================
/**
 *  画面構築・破棄
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  最初の作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Cygnus_Display_CreateStart( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //タイトル
  KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, CYGNUS_00, KEYSYS_TEXT_TYPE_QUE );

  //テキスト
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_000, KEYSYS_TEXT_TYPE_QUE ); 
}
//----------------------------------------------------------------------------
/**
 *	@brief  最初の作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static BOOL Cygnus_Display_IsEndCreateStart( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;
  BOOL is_end_title  = KEYSYS_TEXT_IsMsgEnd( p_wk->p_title );
  BOOL is_end_text  = KEYSYS_TEXT_IsMsgEnd( p_wk->p_text );

  if( is_end_title && is_end_text )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  最初の作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Cygnus_Display_DeleteStart( void *p_wk_adrs )
{
}
//----------------------------------------------------------------------------
/**
 *	@brief  受け取らないの作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Cygnus_Display_CreateDisableRecive( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_000, KEYSYS_TEXT_TYPE_QUE ); 

}
//----------------------------------------------------------------------------
/**
 *	@brief  受け取らないの作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static BOOL Cygnus_Display_IsEndCreateDisableRecive( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;
  if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受け取らないの作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Cygnus_Display_DeleteDisableRecive( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信待機画面の作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Cygnus_Display_CreateReciveWait( void *p_wk_adrs, HEAPID heapID )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  //受取り待機アニメ
  {
    GFL_CLWK *p_clwk = KEYSYS_OBJ_AddClwk( &p_wk->obj, KEYSYS_CLWK_LOAD_ICON, heapID );
    GFL_CLACT_WK_SetDrawEnable( p_clwk, TRUE );
  }

  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_000, KEYSYS_TEXT_TYPE_QUE ); 

}
//----------------------------------------------------------------------------
/**
 *	@brief  受信待機画面の作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static BOOL Cygnus_Display_IsEndCreateReciveWait( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;
  if( KEYSYS_TEXT_IsMsgEnd( p_wk->p_text ) )
  {
    GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信待機画面の作成
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq                    シーケンス
 *	@param	*p_wk_adrs                ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Cygnus_Display_DeleteReciveWait( void *p_wk_adrs )
{
  KEY_SYSTEM_WORK *p_wk = p_wk_adrs;

  KEYSYS_OBJ_RemoveClwk( &p_wk->obj, KEYSYS_CLWK_LOAD_ICON );
}

//=============================================================================
/**
 *  デモ
 */
//=============================================================================
//-------------------------------------
///	シグナス用受け取りデモワーク
//=====================================
struct _CYGNUS_DEMO_WORK
{
  HEAPID  heapID;
  GFL_CLUNIT *p_clunit;
  GFL_CLWK  *p_clwk;
  BOOL      is_end;
  u32       res_plt;
  u32       res_cgx;
  u32       res_cel;
  u32       sync;
  u32       fade_sync;
  u32       seq;

  //BGパレット転送用
  u16 plt_cnt;
  u16 plt[0x10];
  u16 plt_sub[0x10];
  u16 plt_src[0x10];
  u16 plt_dst[0x10];

  //ポケモンOBJパレット転送用
  u16 plt_poke[0x10];
  u16 plt_poke_src[0x10]; //白
  u16 plt_poke_dst[0x10]; //ポケモンの色　

  s16               leg_pos;
  s16               x_ofs;

  KEYSYS_PTC_WORK         *p_ptc;

  void (*move_function)( CYGNUS_DEMO_WORK *p_wk );
};
//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ  初期化
 *
 *	@param	CYGNUS_DEMO_WORK *p_wk ワーク
 *	@param	*p_unit                 CLUNIT
 *	@param  cp_data                 データ
 *	@param	heapID                  ヒープID
 */
//-----------------------------------------------------------------------------
static CYGNUS_DEMO_WORK * CYGNUS_DEMO_Init( KEYSYS_PTC_WORK *p_ptc, const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, HEAPID heapID )
{
  CYGNUS_DEMO_WORK *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(CYGNUS_DEMO_WORK) );
  p_wk->p_ptc     = p_ptc;
  p_wk->heapID    = heapID;
  p_wk->leg_pos   = 0;
  p_wk->x_ofs     = 0;
  p_wk->p_clunit  = KEY_SYSTEM_GRAPHIC_GetClunit( cp_graphic );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デモ開始
 *
 *	@param	CYGNUS_DEMO_WORK *p_wk ワーク
 *	@param	type                    開始するデモのタイプ
 */
//-----------------------------------------------------------------------------
void CYGNUS_DEMO_Start( CYGNUS_DEMO_WORK *p_wk )
{ 
  p_wk->is_end  = FALSE;
  p_wk->seq     = 0;
  p_wk->sync    = 0;

  p_wk->move_function = Cygnus_Demo_EffectMain;
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ  破棄
 *
 *	@param	CYGNUS_DEMO_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CYGNUS_DEMO_Exit( CYGNUS_DEMO_WORK *p_wk )
{ 
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ  メイン処理
 *
 *	@param	CYGNUS_DEMO_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CYGNUS_DEMO_Main( CYGNUS_DEMO_WORK *p_wk )
{
  if( p_wk->move_function )
  { 
    p_wk->move_function( p_wk );

    if( p_wk->is_end )
    { 
      p_wk->move_function = NULL;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信デモ  終了チェック
 *
 *	@param	const CYGNUS_DEMO_WORK *cp_wk  ワーク
 *
 *	@return TRUEでデモ終了  FALSEでデモ中
 */
//-----------------------------------------------------------------------------
static BOOL CYGNUS_DEMO_IsEnd( const CYGNUS_DEMO_WORK *cp_wk )
{ 
  return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デモ  読み込み
 *
 *	@param	CYGNUS_DEMO_WORK *p_wk  ワーク
 *	@param	mons_no   モンスター番号
 *
 */
//-----------------------------------------------------------------------------
static void CYGNUS_DEMO_Load( CYGNUS_DEMO_WORK *p_wk, POKEMON_PARAM* p_pp )
{


#if 0
	//リソース読みこみ
  switch( cp_data->data.gift_type )
  { 
  case CYGNUSGIFT_TYPE_POKEMON:
#endif
    {	

      ARCHANDLE		*p_handle	= POKE2DGRA_OpenHandle( p_wk->heapID );

      p_wk->res_plt	= POKE2DGRA_OBJ_PLTT_RegisterPPP( p_handle, PP_GetPPPPointerConst(p_pp), POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, KEYSYS_PLT_OBJ_GIFT_M * 0x20, p_wk->heapID );
 
      p_wk->res_cel	= POKE2DGRA_OBJ_CELLANM_RegisterPPP( PP_GetPPPPointerConst(p_pp), POKEGRA_DIR_FRONT, APP_COMMON_MAPPING_128K, CLSYS_DRAW_MAIN, p_wk->heapID );
        
      p_wk->res_cgx	= POKE2DGRA_OBJ_CGR_RegisterPPP( p_handle, PP_GetPPPPointerConst(p_pp), POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, p_wk->heapID );

      //足元座標を保存
      //(中心からのオフセット)
      p_wk->leg_pos = Cygnus_GetLegPos( PP_GetPPPPointerConst(p_pp), p_wk->heapID ) - 48;
      p_wk->leg_pos = MATH_CLAMP( p_wk->leg_pos, 0, 48 );
 
      GFL_ARC_CloseDataHandle( p_handle );
    }
#if 0
    break;

  case CYGNUSGIFT_TYPE_ITEM:
    {	
      const GIFT_PRESENT_ITEM  *cp_item = &cp_data->data.data.item;
      ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_ITEMICON, p_wk->heapID );

      p_wk->res_plt	= GFL_CLGRP_PLTT_Register( p_handle, 
          ITEM_GetIndex( cp_item->itemNo, ITEM_GET_ICON_PAL ), CLSYS_DRAW_MAIN, KEYSYS_PLT_OBJ_GIFT_M*0x20, p_wk->heapID );

      p_wk->res_cel	= GFL_CLGRP_CELLANIM_Register( p_handle,
          NARC_item_icon_itemicon_NCER, NARC_item_icon_itemicon_NANR, p_wk->heapID );

      p_wk->res_cgx	= GFL_CLGRP_CGR_Register( p_handle,
          ITEM_GetIndex( cp_item->itemNo, ITEM_GET_ICON_CGX ), FALSE, CLSYS_DRAW_MAIN, p_wk->heapID );

      GFL_ARC_CloseDataHandle( p_handle );
      p_wk->x_ofs = 4;
      p_wk->leg_pos = 8;
    }
    break;

  default:
    GF_ASSERT( 0 );
    break;
  }
#endif

  {
    u32 plt_addr  = HW_OBJ_PLTT + KEYSYS_PLT_OBJ_GIFT_M * 0x20;
    GFL_STD_MemCopy( (void*)plt_addr, p_wk->plt_poke_dst, sizeof(u16)*0x10 );
  }

	//CLWK登録
	{

		int i;
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.pos_x  = 0;
    cldata.pos_y  = 0;
    cldata.bgpri  = 0;

		p_wk->p_clwk	=	GFL_CLACT_WK_Create( p_wk->p_clunit,
				p_wk->res_cgx,
				p_wk->res_plt,
				p_wk->res_cel,
				&cldata,
				CLSYS_DEFREND_MAIN,
				p_wk->heapID );

    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, FALSE );
    GFL_CLACT_WK_SetObjMode( p_wk->p_clwk, GX_OAM_MODE_XLU );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  デモ読み込みを破棄
 *
 *	@param	CYGNUS_DEMO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_DEMO_Release( CYGNUS_DEMO_WORK *p_wk )
{
	//CLWK破棄
  if( p_wk->p_clwk )
	{	
    GFL_CLACT_WK_Remove( p_wk->p_clwk );
	}

	//リソース破棄
	{
    GFL_CLGRP_PLTT_Release( p_wk->res_plt );
    GFL_CLGRP_CGR_Release( p_wk->res_cgx );
    GFL_CLGRP_CELLANIM_Release( p_wk->res_cel );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  映画用デモ
 *
 *	@param	CYGNUS_DEMO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Cygnus_Demo_EffectMain( CYGNUS_DEMO_WORK *p_wk )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_INIT_WAIT,

    SEQ_APPEAR_POKE_INIT,
    SEQ_APPEAR_POKE_WAIT,
    SEQ_PTC_FALL_WAIT,
    SEQ_PTC_EMIT_WAIT,

    SEQ_FADE_POKE_INIT,
    SEQ_FADE_POKE_WAIT,

    SEQ_END_WAIT,
    SEQ_END,
  };

  enum
  {
    CYGNUS_DEMO_GIFT_X            = (128),
    CYGNUS_DEMO_MOVE_GIFT_START_Y = (-96),
    CYGNUS_DEMO_MOVE_GIFT_END_Y   = (118),
    CYGNUS_DEMO_POKE_FADE_SYNC1   = (10),
    CYGNUS_DEMO_POKE_WAIT_SYNC    = (10),
    CYGNUS_DEMO_POKE_FADE_SYNC2   = (120),
    CYGNUS_DEMO_POKE_END_SYNC     = (60),
    CYGNUS_DEMO_FINISH_FADE_SYNC  = (60),
    CYGNUS_DEMO_PTC_FALL_SYNC     = (120),
  };

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    KEYSYS_PTC_LoadResource( p_wk->p_ptc, ARCID_KEY_SYSTEM, NARC_key_system_gra_key_system_spa, p_wk->heapID );
    p_wk->seq = SEQ_INIT_WAIT;
    break;

  case SEQ_INIT_WAIT:
    { 
      p_wk->sync  = 0;
      p_wk->seq   = SEQ_PTC_FALL_WAIT;
      //PTC+ポケモンOBJをアルファ

      G2_SetBlendAlpha(
          GX_BLEND_PLANEMASK_BG0,
          GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 ,
          0,
          16
          );

      {
        u32 plt_addr  = HW_OBJ_PLTT + KEYSYS_PLT_OBJ_GIFT_M * 0x20;

        //真っ白にする
        GFL_STD_MemFill16( (void*)plt_addr, GX_RGB(31,31,31), sizeof(u16)*0x10 );
        GFL_STD_MemCopy( (void*)plt_addr, p_wk->plt_poke_src, sizeof(u16)*0x10 );
      }

      //光のパーティクルが降りてくる
      KEYSYS_PTC_Start( p_wk->p_ptc, BIG_RING );
      KEYSYS_PTC_Start( p_wk->p_ptc, DOWN_BALL );
      KEYSYS_PTC_Start( p_wk->p_ptc, SPLASH_BALL );
      PMSND_PlaySE( CYGNUS_SE_MOVIE_FALL );
    }
    break;

  case SEQ_PTC_FALL_WAIT:
    if( p_wk->sync++ > CYGNUS_DEMO_PTC_FALL_SYNC )
    { 
      //PTC爆発
      PMSND_PlaySE( CYGNUS_SE_MOVIE_EMIT );
      p_wk->sync  = 0;
      p_wk->seq   = SEQ_APPEAR_POKE_INIT;
    }
    break;


  case SEQ_APPEAR_POKE_INIT:
      //ポケモンON
      { 
        GFL_CLACTPOS  pos;
        pos.x = CYGNUS_DEMO_GIFT_X;
        pos.y = CYGNUS_DEMO_MOVE_GIFT_END_Y - p_wk->leg_pos;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, CLSYS_DRAW_MAIN );
      }
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );

      p_wk->fade_sync = 0;
      p_wk->seq   = SEQ_APPEAR_POKE_WAIT;
    break;

  case SEQ_APPEAR_POKE_WAIT:
    {
      s16 ev1 = 0 + 16 * p_wk->fade_sync / CYGNUS_DEMO_POKE_FADE_SYNC1;
      s16 ev2 = 16 - 16 * p_wk->fade_sync / CYGNUS_DEMO_POKE_FADE_SYNC1;
      G2_ChangeBlendAlpha( ev1, ev2 );

      p_wk->sync++;
      if( p_wk->fade_sync++ > CYGNUS_DEMO_POKE_FADE_SYNC1 )
      { 
        p_wk->fade_sync  = 0;
        p_wk->seq = SEQ_PTC_EMIT_WAIT;
      }
    }
    break;


  case SEQ_PTC_EMIT_WAIT:
    if( p_wk->sync++ > 90 )
    { 
      //爆発終了
      G2_BlendNone();
      p_wk->sync  = 0;
      p_wk->seq   = SEQ_FADE_POKE_INIT;
    }
    break;
  case SEQ_FADE_POKE_INIT:
    if( p_wk->sync++ > CYGNUS_DEMO_POKE_WAIT_SYNC )
    { 
      p_wk->sync  = 0;
      p_wk->seq = SEQ_FADE_POKE_WAIT;
    }
    break;

  case SEQ_FADE_POKE_WAIT:
    {
      int i;
      p_wk->plt_cnt = 0 + 0x7FFF * p_wk->sync / CYGNUS_DEMO_POKE_FADE_SYNC2;

      KEYSYS_UTIL_MainPltAnmLine( NNS_GFD_DST_2D_OBJ_PLTT_MAIN, p_wk->plt_poke, p_wk->plt_cnt, KEYSYS_PLT_OBJ_GIFT_M, p_wk->plt_poke_dst, p_wk->plt_poke_src );

      //終了チェック
      if( p_wk->sync++ > CYGNUS_DEMO_POKE_FADE_SYNC2 )
      { 

        p_wk->sync  = 0;
        p_wk->seq = SEQ_END_WAIT;
      }
    }
    break;

  case SEQ_END_WAIT:
    if( p_wk->sync++ > CYGNUS_DEMO_POKE_END_SYNC )
    { 
      p_wk->sync  = 0;
      p_wk->seq = SEQ_END;
    }
    break;

  case SEQ_END:
    G2_BlendNone();
    p_wk->is_end  = TRUE;
    break;
  }
}

//=============================================================================
/**
 *  シグナスセーブ部分
 */
//=============================================================================


typedef struct
{
  CYGNUS_REWARD_TYPE type;
  u16 no;
  u8  form;
  u8  sex;
  u16 num;
  u16 is_demo;
} CYGNUS_REWARD_DATA;

//-------------------------------------
///	シグナスセーブワーク
//=====================================
struct _CYGNUS_SAVE_WORK
{
  CYGNUS_SAVE_CONTROL *p_cyg_sv_ctrl;
  GAMEDATA *p_gamedata;
  CYGNUS_SAVEDATA *p_out_sv;
  DUMMY3DS_SAVEDATA * p_in_sv;
  CYGSAVE_STATE out_state;
  u32 sv_seq;
  CYGNUS_REWARD_DATA reward_data[CYGNUS_REWARD_DATA_NUM];
  u32 reward_num;
  u32 flag;
};

static void Cygnus_Save_ConvertRewardData( CYGNUS_SAVE_WORK *p_wk, const u32 cp_row_data[], HEAPID heapID );
static BOOL Cygnus_Save_IsValidPokemon( u16 mons_no, u16 form_no, u8 sex, HEAPID heapID );
static BOOL Cygnus_Save_IsDemoPoke( u16 mons_no );
static BOOL Cygnus_Save_IsStatusFixPoke( u16 mons_no );

#ifdef PM_DEBUG
static const  CYGNUS_REWARD_DATA sc_debug_data[] =
{
  {
    CYGNUS_REWARD_TYPE_POKEMON,
    MONSNO_BORUTOROSU,
    1,
    PM_MALE,
    0,
    TRUE
  },
  {
    CYGNUS_REWARD_TYPE_POKEMON,
    MONSNO_TORUNEROSU,
    1,
    PM_MALE,
    0,
    FALSE
  },
  {
    CYGNUS_REWARD_TYPE_POKEMON,
    MONSNO_RANDOROSU,
    1,
    PM_MALE,
    0,
    FALSE
  },
  {
    CYGNUS_REWARD_TYPE_POKEMON,
    MONSNO_HUSIGIDANE,
    0,
    PM_MALE,
    0,
    FALSE
  },
  {
    CYGNUS_REWARD_TYPE_POKEMON,
    MONSNO_HITOKAGE,
    0,
    PM_MALE,
    0,
    FALSE
  },
  {
    CYGNUS_REWARD_TYPE_POKEMON,
    MONSNO_ZENIGAME,
    0,
    PM_MALE,
    0,
    FALSE
  },
  {
    CYGNUS_REWARD_TYPE_ITEM,
    1,
    0,
    0,
    1,
    FALSE,
  },
  {
    CYGNUS_REWARD_TYPE_ITEM,
    33,
    0,
    0,
    1,
    FALSE,
  },
  {
    CYGNUS_REWARD_TYPE_ITEM,
    23,
    0,
    0,
    1,
    FALSE,
  },
  {
    CYGNUS_REWARD_TYPE_ITEM,
    101,
    0,
    0,
    1,
    FALSE,
  },
  {
    CYGNUS_REWARD_TYPE_ITEM,
    102,
    0,
    0,
    1,
    FALSE,
  },
  {
    CYGNUS_REWARD_TYPE_ITEM,
    103,
    0,
    0,
    1,
    FALSE,
  },
};
#endif
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスセーブ作成
 *
 *	@param	GAMEDATA *p_gamedata  ゲームデータ
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static CYGNUS_SAVE_WORK * CYGNUS_SAVE_Init( GAMEDATA *p_gamedata, HEAPID heapID )
{
  CYGNUS_SAVE_WORK *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(CYGNUS_SAVE_WORK) );
  p_wk->p_gamedata  = p_gamedata;
  p_wk->p_cyg_sv_ctrl = CYGNUSSAVE_CONTROL_Create( heapID );
  CYGNUSSAVE_CONTROL_Load( p_wk->p_cyg_sv_ctrl );
  p_wk->p_out_sv = CYGNUSSAVE_CONTROL_GetPtr( p_wk->p_cyg_sv_ctrl );
  p_wk->p_in_sv = SaveData_GetDummy3ds( GAMEDATA_GetSaveControlWork(p_gamedata) );
  p_wk->flag  = DUMMY3DS_SAVEDATA_GetData( p_wk->p_in_sv, DUMMY3DS_SAVEDATA_PARAM_FLAG );

  {

    u32 row_data[CYGDATA_AREA_SIZE];
    u32 onetime_key = DUMMY3DS_SAVEDATA_GetData( p_wk->p_in_sv, DUMMY3DS_SAVEDATA_PARAM_ONETIME_KEY );
    p_wk->out_state = CYGNUS_SAVEDATA_Receive(
        p_wk->p_out_sv, onetime_key, row_data );

    //ナマデータをご褒美データに編集
    if( p_wk->out_state == CYGSAVE_STATE_RECEIVE )
    {
      Cygnus_Save_ConvertRewardData( p_wk, row_data, heapID );
    }
  }

#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_R|PAD_BUTTON_L) )
  {
    p_wk->out_state = CYGSAVE_STATE_BREAK;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    GFL_STD_MemCopy( sc_debug_data, p_wk->reward_data, sizeof(sc_debug_data) );
    p_wk->reward_num = NELEMS( sc_debug_data );
    p_wk->out_state = CYGSAVE_STATE_RECEIVE;
  }
#endif

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスセーブ破棄
 *
 *	@param	CYGNUS_SAVE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_SAVE_Exit( CYGNUS_SAVE_WORK *p_wk )
{
  CYGNUSSAVE_CONTROL_Delete( p_wk->p_cyg_sv_ctrl );
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  シグナスのセーブ状態取得
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk   ワーク
 *
 *	@return シグナスのセーブ状態取得
 */
//-----------------------------------------------------------------------------
static CYGNUS_SAVE_STATUS CYGNUS_SAVE_GetStatus( const CYGNUS_SAVE_WORK *cp_wk )
{
  BOOL is_played = DUMMY3DS_SAVEDATA_GetData( cp_wk->p_in_sv, DUMMY3DS_SAVEDATA_PARAM_IS_PLAYED );
  BOOL is_empty = cp_wk->reward_num == 0;
  BOOL is_access  = (cp_wk->out_state == CYGSAVE_STATE_RECEIVE);
  BOOL is_break  = (cp_wk->out_state == CYGSAVE_STATE_NON || cp_wk->out_state == CYGSAVE_STATE_BREAK);

  //壊れている
  if( is_break )
  {
    return CYGNUS_SAVE_STATUS_BREAK;
  }

  if( !is_empty && is_access )
  {
    //存在していてアクセス可能
    return CYGNUS_SAVE_STATUS_EXIST;
  }
  else
  {
    //存在してないか、アクセス不可能
    if( is_played )
    {
      //一度でもやった
      return CYGNUS_SAVE_STATUS_EMPTY;
    }
    else
    {
      //やったことない
      return CYGNUS_SAVE_STATUS_NONE;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出あるポケモンがいるかどうか
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk ワーク
 *
 *	@return TRUE いる FALSE　いない
 */
//-----------------------------------------------------------------------------
static BOOL CYGNUS_SAVE_IsEffectPoke( const CYGNUS_SAVE_WORK *cp_wk )
{
  int i;
  for( i = 0; i < cp_wk->reward_num; i++ )
  {
    if( cp_wk->reward_data[i].is_demo )
    {
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出あるポケモンのモンスターナンバーを取得
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk ワーク
 *	@param	idx インデックス
 *
 *	@return   モンスターナンバー  いない場合は０
 */
//-----------------------------------------------------------------------------
static u32 CYGNUS_SAVE_GetEffectPoke( const CYGNUS_SAVE_WORK *cp_wk, u32 idx )
{
  int i;
  int cnt = 0;
  for( i = 0; i < cp_wk->reward_num; i++ )
  {
    if( cp_wk->reward_data[i].is_demo )
    {
      if( cnt++ == idx )
      {
        return cp_wk->reward_data[i].no;
      }
    }
  }

  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスセーブ（内部も同時に行う）
 *
 *	@param	CYGNUS_SAVE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_SAVE_StartSave( CYGNUS_SAVE_WORK *p_wk )
{
  //ワンタイムパスワードを生成
  GF_ASSERT( p_wk->out_state == CYGSAVE_STATE_RECEIVE 
      || p_wk->out_state == CYGSAVE_STATE_NON
      || p_wk->out_state == CYGSAVE_STATE_BREAK );
  {
    u32 onetime_key = GFUser_GetPublicRand( GFL_STD_RAND_MAX );
    DUMMY3DS_SAVEDATA_SetData( p_wk->p_in_sv, DUMMY3DS_SAVEDATA_PARAM_ONETIME_KEY, onetime_key );
    DUMMY3DS_SAVEDATA_SetData( p_wk->p_in_sv, DUMMY3DS_SAVEDATA_PARAM_IS_PLAYED, TRUE );

    CYGNUS_SAVEDATA_SetSend( p_wk->p_out_sv, onetime_key, p_wk->flag );
    DUMMY3DS_SAVEDATA_SetData( p_wk->p_in_sv, DUMMY3DS_SAVEDATA_PARAM_FLAG, p_wk->flag );

    p_wk->sv_seq  = 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスセーブ（内部も同時に行う）
 *
 *	@param	CYGNUS_SAVE_WORK *p_wk  ワーク
 *
 *	@return TRUEセーブ終了  FALSEセーブ中
 */
//-----------------------------------------------------------------------------
static BOOL CYGNUS_SAVE_MainSave( CYGNUS_SAVE_WORK *p_wk )
{
  enum
  {
    SEQ_IN_SAVE_START,
    SEQ_IN_SAVE_LAST,
    SEQ_OUT_SAVE_START,
    SEQ_OUT_SAVE_WAIT,
    SEQ_IN_SAVE_WAIT,
    SEQ_END,
  };

  switch( p_wk->sv_seq )
  {
  case SEQ_IN_SAVE_START:
    GAMEDATA_SaveAsyncStart(p_wk->p_gamedata);
    p_wk->sv_seq++;
    /* fall */
  case SEQ_IN_SAVE_LAST:
    {
      SAVE_RESULT ret = GAMEDATA_SaveAsyncMain(p_wk->p_gamedata);
      if( ret == SAVE_RESULT_LAST )
      {
        p_wk->sv_seq++;
        /* fall  */
      }
      else
      {
        break;
      }
    }
  case SEQ_OUT_SAVE_START:
    CYGNUSSAVE_CONTROL_SaveAsyncInit( p_wk->p_cyg_sv_ctrl );
    p_wk->sv_seq++;
    /* fall */
  case SEQ_OUT_SAVE_WAIT:
    if( CYGNUSSAVE_CONTROL_SaveAsyncMain( p_wk->p_cyg_sv_ctrl ) )
    {
      p_wk->sv_seq++;
      /* fall */
    }
    else
    {
      break;
    }
  case SEQ_IN_SAVE_WAIT:
    {
      SAVE_RESULT ret = GAMEDATA_SaveAsyncMain(p_wk->p_gamedata);
      if( ret == SAVE_RESULT_OK )
      {
        p_wk->sv_seq++;
        /* fall */
      }
      else
      {
        break;
      }
    }
  case SEQ_END:
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ご褒美取得
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk ワーク
 *	@param	idx     インデックス
 *	@param	*p_type タイプ受け取り
 *	@param	*p_no   番号受け取り
 *
 *	@return TRUEご褒美がある  FALSEない
 */
//-----------------------------------------------------------------------------
static BOOL CYGNUS_SAVE_GetReward( const CYGNUS_SAVE_WORK *cp_wk, u32 idx, CYGNUS_REWARD_TYPE *p_type, u32 *p_no )
{ 
  if( cp_wk->reward_data[idx].type != CYGNUS_REWARD_TYPE_NONE )
  {
    *p_type = cp_wk->reward_data[idx].type;
    *p_no   = cp_wk->reward_data[idx].no;
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンパラム作成
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk ワーク
 *	@param	idx インデックス
 *	@param	heapID ヒープID
 *
 *	@return ポケモンパラム
 */
//-----------------------------------------------------------------------------
static POKEMON_PARAM * CYGNUS_SAVE_CreatePP( const CYGNUS_SAVE_WORK *cp_wk, u32 idx, HEAPID heapID )
{
  if( cp_wk->reward_data[idx].type == CYGNUS_REWARD_TYPE_POKEMON )
  {
    u16 mons_no = cp_wk->reward_data[idx].no;
    u8  form_no = cp_wk->reward_data[idx].form;
    u8  sex = cp_wk->reward_data[idx].sex;
    u8  lv  = 100;
    u32 id = MyStatus_GetID(GAMEDATA_GetMyStatus(cp_wk->p_gamedata));

    u32 rand = POKETOOL_CalcPersonalRandSpec( id, mons_no, form_no, sex, PTL_TOKUSEI_SPEC_BOTH, PTL_RARE_SPEC_FALSE );

    u32 pow = PTL_SETUP_POW_AUTO;

    POKEMON_PARAM *p_pp;

    //バッジの数によってレベルを帰る
    {
      static const sc_lv[]  =
      {
        5,10,10,20,20,30,30,40,40,
      };
      int cnt = MISC_GetBadgeCount( GAMEDATA_GetMiscWork(cp_wk->p_gamedata) );
      cnt = MATH_CLAMP( cnt, 0, NELEMS(sc_lv)-1 );
      lv  = sc_lv[cnt];
    }

    p_pp = PP_Create( mons_no, lv, id, heapID );

    if( Cygnus_Save_IsStatusFixPoke( mons_no ) )
    {
      pow = PTL_SETUP_POW_PACK( 20,20,20,20,20,20 );
    }

    PP_SetupEx( p_pp, mons_no, lv, id, pow, rand );
    PP_ChangeFormNo( p_pp, form_no );
    PP_SetWazaDefault( p_pp );

    if( Cygnus_Save_IsStatusFixPoke( mons_no ) )
    {
      PP_Put(p_pp, ID_PARA_seikaku, PTL_SEIKAKU_GANBARIYA);
    }

    //もし第３特性もってたら第３特性
    {
      POKEMON_PERSONAL_DATA *p_personal = POKE_PERSONAL_OpenHandle( mons_no, form_no, heapID );
      u32 tokusei3 = POKE_PERSONAL_GetParam( p_personal, POKEPER_ID_speabi3 );
      if( tokusei3 )
      {
        PP_SetTokusei3( p_pp, mons_no, form_no );
      }
      POKE_PERSONAL_CloseHandle( p_personal );
    }

    PP_Put(p_pp, ID_PARA_oyaname_raw, (u32)MyStatus_GetMyName(GAMEDATA_GetMyStatus(cp_wk->p_gamedata)));
    PP_Put(p_pp, ID_PARA_oyasex, MyStatus_GetMySex(GAMEDATA_GetMyStatus(cp_wk->p_gamedata)));
    PP_Put(p_pp, ID_PARA_get_ball, BALLID_DORIIMUBOORU );
    PP_Put(p_pp, ID_PARA_get_cassette, CasetteVersion );
    PP_Put(p_pp, ID_PARA_country_code, CasetteLanguage ); 

    POKE_MEMO_SetTrainerMemoARSearcher( PP_GetPPPPointer(p_pp) );

    PP_Renew( p_pp );

    if(PP_Get(p_pp,ID_PARA_fusei_tamago_flag, NULL)){  //不正ポケモンが出来てしまった場合
      GF_ASSERT(0);
      GFL_HEAP_FreeMemory( p_pp );
      return NULL;
    }
    return p_pp;
  }

  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アイテム取得
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk
 *	@param	idx
 *	@param	*p_item
 *	@param	*p_num 
 */
//-----------------------------------------------------------------------------
static BOOL CYGNUS_SAVE_GetItem( const CYGNUS_SAVE_WORK *cp_wk, u32 idx, u16 *p_item, u8 *p_num )
{
  if( cp_wk->reward_data[idx].type == CYGNUS_REWARD_TYPE_ITEM )
  {
    *p_item = cp_wk->reward_data[idx].no;
    *p_num  = cp_wk->reward_data[idx].num;
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  現在のインデックスのアイテムが何個目のアイテムか
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk ワーク
 *	@param	idx   インデックス
 *
 *	@return 何コメのあいてむか
 */
//-----------------------------------------------------------------------------
static u32 CYGNUS_SAVE_GetItemCount( const CYGNUS_SAVE_WORK *cp_wk, u32 idx )
{
  int i;
  int cnt = 0;
  for( i = 0; i < cp_wk->reward_num; i++ )
  {
    if( cp_wk->reward_data[i].type == CYGNUS_REWARD_TYPE_ITEM )
    {
      if( cnt++ == idx )
      {
        return cnt;
      }
    }
  }

  GF_ASSERT(0);
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ご褒美ポケモンの数取得
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk   ワーク
 *
 *	@return ポケモンの数
 */
//-----------------------------------------------------------------------------
static u32 CYGNUS_SAVE_GetRewardPokeMax( const CYGNUS_SAVE_WORK *cp_wk )
{
  int i;
  int cnt = 0;
  for( i = 0; i < cp_wk->reward_num; i++ )
  {
    if( cp_wk->reward_data[i].type == CYGNUS_REWARD_TYPE_POKEMON )
    {
      cnt++;
    }
  }

  return cnt;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ご褒美道具の数取得
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk   ワーク
 *
 *	@return 道具の数
 */
//-----------------------------------------------------------------------------
static u32 CYGNUS_SAVE_GetRewardItemMax( const CYGNUS_SAVE_WORK *cp_wk )
{
  int i;
  int cnt = 0;
  for( i = 0; i < cp_wk->reward_num; i++ )
  {
    if( cp_wk->reward_data[i].type == CYGNUS_REWARD_TYPE_ITEM )
    {
      cnt++;
    }
  }

  return cnt;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ご褒美最大数を取得
 *
 *	@param	const CYGNUS_SAVE_WORK *cp_wk   ワーク
 *
 *	@return 最大数
 */
//-----------------------------------------------------------------------------
static u32 CYGNUS_SAVE_GetRewardMax( const CYGNUS_SAVE_WORK *cp_wk )
{
  return cp_wk->reward_num;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ゲームデータにご褒美を格納
 *
 *	@param	CYGNUS_SAVE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_SAVE_SetGameDataReward( CYGNUS_SAVE_WORK *p_wk, HEAPID heapID )
{
  //ポケモン格納
  {
    BOX_MANAGER *p_box  = GAMEDATA_GetBoxManager( p_wk->p_gamedata );
    int i;

    for( i = 0; i < p_wk->reward_num; i++ )
    {
      POKEMON_PARAM *p_pp = CYGNUS_SAVE_CreatePP( p_wk, i, heapID );
      if( p_pp != NULL )
      {
        BOXDAT_PutPokemon( p_box, PP_GetPPPPointer(p_pp) );
        GFL_HEAP_FreeMemory( p_pp );
      }
    }
  }

  //道具格納
  {
    MYITEM_PTR p_myitem  = GAMEDATA_GetMyItem( p_wk->p_gamedata );
    int i;

    for( i = 0; i < p_wk->reward_num; i++ )
    {
      u16 item;
      u8 num;
      if( CYGNUS_SAVE_GetItem( p_wk, i, &item, &num ) )
      {
        MYITEM_AddItem(p_myitem, item, num, heapID );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ナマデータをご褒美データに変換
 *
 *	@param	CYGNUS_SAVE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Cygnus_Save_ConvertRewardData( CYGNUS_SAVE_WORK *p_wk, const u32 cp_row_data[], HEAPID heapID )
{
  int i;

  //SPデータを変換
  for( i = CYGDATA_SP_POKE01; i <= CYGDATA_SP_POKE08; i++ )
  {
    if( CUGNUS_DATA_IsValidMagicNumber(cp_row_data[i]) )
    {
      CYGNUSDATA_FLAG flag = CYGNUS_DATA_GetFlagByMagicNum( cp_row_data[i] );
      if( (p_wk->flag & flag) == 0 )
      {
        u16 mons_no;
        u8  form_no;
        u8  sex;
        CYGNUS_DATA_GetMonsNoByMagicNum( cp_row_data[i], &mons_no, &form_no, &sex );

        p_wk->reward_data[p_wk->reward_num].type = CYGNUS_REWARD_TYPE_POKEMON;
        p_wk->reward_data[p_wk->reward_num].no   = mons_no;
        p_wk->reward_data[p_wk->reward_num].form = form_no;
        p_wk->reward_data[p_wk->reward_num].sex  = sex;
        p_wk->reward_data[p_wk->reward_num].is_demo = Cygnus_Save_IsDemoPoke( mons_no );
        p_wk->reward_num++;

        p_wk->flag |= flag;
      }
    }
  }

  //デモポケを先頭にする
  if( p_wk->reward_num > 0 )
  {
    int j;
    //ソート
    for( i = 0; i < p_wk->reward_num-1; i++ )
    {
      for( j = p_wk->reward_num-1; j > i; j-- )
      {
        s16 idx1 = j-1;
        s16 idx2 = j;
        if( p_wk->reward_data[ idx1 ].is_demo < p_wk->reward_data[ idx2 ].is_demo )
        {
          CYGNUS_REWARD_DATA temp = p_wk->reward_data[ idx1 ];
          p_wk->reward_data[ idx1 ] = p_wk->reward_data[ idx2 ];
          p_wk->reward_data[ idx2 ] = temp;
        }
      }
    }
  }

  //ポケモン
  for( i = CYGDATA_POKE01; i <= CYGDATA_POKE06; i++  )
  {
    //ポケモンは６匹まで
    if( p_wk->reward_num >= CYGNUS_REWARD_POKE_NUM )
    {
      break;
    }
    else
    {
      u16 mons_no = CYGNUS_DATA_GET_POKE_NO( cp_row_data[i] );
      u8  form_no = CYGNUS_DATA_GET_FORM_NO( cp_row_data[i] );
      u8  sex = CYGNUS_DATA_GET_FORM_NO( cp_row_data[i] );

      if( Cygnus_Save_IsValidPokemon( mons_no, form_no, sex, heapID ) )
      {
        p_wk->reward_data[p_wk->reward_num].type = CYGNUS_REWARD_TYPE_POKEMON;
        p_wk->reward_data[p_wk->reward_num].no   = mons_no;
        p_wk->reward_data[p_wk->reward_num].form = form_no;
        p_wk->reward_data[p_wk->reward_num].sex  = sex;
        p_wk->reward_data[p_wk->reward_num].is_demo = FALSE;
        p_wk->reward_num++;
      }
    }
  }

  //道具
  for( i = CYGDATA_ITEMID01; i <= CYGDATA_ITEMID06; i++ )
  {
    if( p_wk->reward_num >= CYGNUS_REWARD_DATA_NUM )
    {
      //ないと思うけど、一応上限チェック
      break;
    }
    else
    {
      u16 item  = CYGNUS_DATA_GET_ITEM_NO( cp_row_data[i] );
      u16 num   = CYGNUS_DATA_GET_ITEM_NUM( cp_row_data[i] );
      if( ( 0 < item && item < ITEM_DATA_MAX )
          && num > 0 )
      {
        p_wk->reward_data[p_wk->reward_num].type = CYGNUS_REWARD_TYPE_ITEM;
        p_wk->reward_data[p_wk->reward_num].no   = item;
        p_wk->reward_data[p_wk->reward_num].num  = num;
        p_wk->reward_data[p_wk->reward_num].form = 0;
        p_wk->reward_data[p_wk->reward_num].sex  = 0;
        p_wk->reward_data[p_wk->reward_num].is_demo = FALSE;
        p_wk->reward_num++;
      }
    }
  } 
}
//----------------------------------------------------------------------------
/**
 *	@brief  与えられた引数が正常かチェック
 *
 *	@param	u16 mons_no モンスタ番号
 *	@param	form_no   フォルム
 *	@param	sex        性別
 *
 *	@return TRUE正常  FALSE不正
 */
//-----------------------------------------------------------------------------
static BOOL Cygnus_Save_IsValidPokemon( u16 mons_no, u16 form_no, u8 sex, HEAPID heapID )
{
  //モンスタ番号不正
  if( MONSNO_END < mons_no || mons_no == 0 )
  {
    return FALSE;
  }
  else
  {
    POKEMON_PERSONAL_DATA*  p_ppd = POKE_PERSONAL_OpenHandle( mons_no, 0, heapID );
    const u32 form_max = POKE_PERSONAL_GetParam( p_ppd, POKEPER_ID_form_max );
    const POKEPER_SEX per_sex = POKE_PERSONAL_GetParam( p_ppd, POKEPER_ID_sex );
    POKE_PERSONAL_CloseHandle( p_ppd );

    //フォルム最大数が超えていたら不正
    if( form_no > form_max )
    {
      return FALSE;
    }

    //性別ベクトルと性別が合わなかったら不正
    switch( per_sex )
    { 
    case POKEPER_SEX_MALE:
      if( sex != PM_MALE )
      {
        return FALSE;
      }
      break;
    case POKEPER_SEX_UNKNOWN:
      if( sex != PM_NEUTRAL )
      {
        return FALSE;
      }
      break;
    case POKEPER_SEX_FEMALE:
      if( sex != PM_FEMALE )
      {
        return FALSE;
      }
      break;
    default:
      if( !(sex == PM_MALE || sex == PM_FEMALE) )
      {
        return FALSE;
      }
      break;
    }

    return TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  デモをするポケモンかチェック
 *
 *	@param	u16 mons_no モンスター番号
 *
 *	@return TRUEデモをするポケモン  FALSEしないポケモン
 */
//-----------------------------------------------------------------------------
static BOOL Cygnus_Save_IsDemoPoke( u16 mons_no )
{
  static const u32 sc_demo_poke[] =
  {
    MONSNO_BORUTOROSU,
    MONSNO_TORUNEROSU,
    MONSNO_RANDOROSU,
  };
  int i;
  for( i = 0; i < NELEMS(sc_demo_poke); i++ )
  {
    if( sc_demo_poke[i] == mons_no )
    {
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ステータスを固定するポケモンをチェック
 *
 *	@param	u16 mons_no   モンスター番号
 *
 *	@return TRUE固定ポケ  FALSE固定しないポケ
 */
//-----------------------------------------------------------------------------
static BOOL Cygnus_Save_IsStatusFixPoke( u16 mons_no )
{
  static const u32 sc_fix_poke[] =
  {
    //▼伝説
    144,//フリーザー
    145,//サンダー
    146,//ファイヤー
    150,//ミュウツー
    243,//ライコウ
    244,//エンテイ
    245,//スイクン
    249,//ルギア
    250,//ホウオウ
    377,//レジロック
    378,//レジアイス
    379,//レジスチル
    380,//ラティアス
    381,//ラティオス
    382,//カイオーガ
    383,//グラードン
    384,//レックウザ
    480,//ユクシー
    481,//エムリット
    482,//アグノム
    483,//ディアルガ
    484,//パルキア
    485,//ヒードラン
    486,//レジギガス
    487,//ギラティナ
    488,//クレセリア
    638,//コバルオン
    639,//テラキオン
    640,//ビリジオン
    641,//トルネロス
    642,//ボルトロス
    643,//レシラム
    644,//ゼクロム
    645,//ランドロス
    646,//キュレム
    //▼幻
    151,//ミュウ
    251,//セレビィ
    385,//ジラーチ
    386,//デオキシス
    489,//フィオネ
    490,//マナフィ
    491,//ダークライ
    492,//シェイミ
    493,//アルセウス
    494,//ビクティニ
    647,//ケルディオ
    648,//メロエッタ
  };
  int i;
  for( i = 0; i < NELEMS(sc_fix_poke); i++ )
  {
    if( sc_fix_poke[i] == mons_no )
    {
      return TRUE;
    }
  }

  return FALSE;
}

//=============================================================================
/**
 *  結果画面
 */
//=============================================================================
#define CYGNUS_RESULT_ICON_NUM            (CYGNUS_REWARD_DATA_NUM)
#define CYGNUS_RESULT_ICON_MOVE_DISTANCE  (16)
#define CYGNUS_RESULT_ICON_MOVE_MS        (10*16)

//-------------------------------------
///	１つのアイコン
//=====================================
typedef struct
{
  u32       res_plt;
  u32       res_cgx;
  u32       res_cel;
  GFL_CLWK *p_clwk;
  GFL_CLACTPOS def_pos;
  u64       start_ms;
  BOOL      is_start;
} CYGNUS_RESULT_ICON;

//-------------------------------------
///	シグナス用結果ワーク
//=====================================
struct _CYGNUS_RESULT_WORK
{
  HEAPID  heapID;
  GFL_CLUNIT *p_clunit;
  CYGNUS_RESULT_ICON  icon[CYGNUS_RESULT_ICON_NUM];
  u32   icon_cnt;  
  BOOL  is_end;
  BOOL  is_start;
  u32   seq;
  const CYGNUS_SAVE_WORK *cp_save;
};
//-------------------------------------
///	プロトタイプ
//=====================================
static void Cygnus_Result_wk_Init( CYGNUS_RESULT_ICON *p_wk, GFL_CLUNIT *p_clunit, const CYGNUS_SAVE_WORK *cp_save, u16 idx, HEAPID heapID );
static void Cygnus_Result_wk_Exit( CYGNUS_RESULT_ICON *p_wk );
static void Cygnus_Result_wk_StartMove( CYGNUS_RESULT_ICON *p_wk );
static BOOL Cygnus_Result_wk_WaitMove( CYGNUS_RESULT_ICON *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  結果画面ワーク初期化
 *
 *	@param	const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic グラフィックシステム
 *	@param	CYGNUS_SAVE_WORK *cp_save セーブワーク
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static CYGNUS_RESULT_WORK * CYGNUS_RESULT_Init( const KEY_SYSTEM_GRAPHIC_WORK *cp_graphic, const CYGNUS_SAVE_WORK *cp_save, HEAPID heapID )
{
  CYGNUS_RESULT_WORK *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(CYGNUS_RESULT_WORK) );
  p_wk->heapID    = heapID;
  p_wk->p_clunit  = KEY_SYSTEM_GRAPHIC_GetClunit( cp_graphic );
  p_wk->cp_save   = cp_save;

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  結果ワーク破棄
 *
 *	@param	CYGNUS_RESULT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_RESULT_Exit( CYGNUS_RESULT_WORK *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  結果ワークメイン
 *
 *	@param	CYGNUS_RESULT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_RESULT_Main( CYGNUS_RESULT_WORK *p_wk )
{
  u32 max = CYGNUS_SAVE_GetRewardMax( p_wk->cp_save );

  if( p_wk->is_start == FALSE )
  {
    return;
  }

  if( p_wk->icon_cnt >= max )
  {
    p_wk->is_end  = TRUE;
  }
  else
  {
    enum
    {
      SEQ_START,
      SEQ_WAIT,
      SEQ_SE_WAIT,
    };

    switch( p_wk->seq )
    {
    case SEQ_START:
      Cygnus_Result_wk_StartMove( &p_wk->icon[p_wk->icon_cnt] );
      p_wk->seq++;
      /* fall */
    case SEQ_WAIT:
      if( Cygnus_Result_wk_WaitMove(&p_wk->icon[p_wk->icon_cnt]) )
      {
        PMSND_PlaySE( CYGNUS_SE_ICON );

        p_wk->seq++;
        p_wk->icon_cnt++;
      }
      break;
    case SEQ_SE_WAIT:
      // if( PMSND_CheckPlaySE() == FALSE )
      {
        p_wk->seq = SEQ_START;
      }
      break;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  結果ワーク開始
 *
 *	@param	CYGNUS_RESULT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_RESULT_Start( CYGNUS_RESULT_WORK *p_wk )
{
  p_wk->is_start  = TRUE;
  p_wk->is_end    = FALSE;
  p_wk->seq       = 0;
  p_wk->icon_cnt  = 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  結果ワーク  終了検知
 *
 *	@param	const CYGNUS_RESULT_WORK *cp_wk   ワーク
 *
 *	@return TRUE終了  FALSEまだ
 */
//-----------------------------------------------------------------------------
static BOOL CYGNUS_RESULT_IsEnd( const CYGNUS_RESULT_WORK *cp_wk )
{
  return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief  読み込み
 *
 *	@param	CYGNUS_RESULT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_RESULT_Load( CYGNUS_RESULT_WORK *p_wk )
{
  u32 max = CYGNUS_SAVE_GetRewardMax( p_wk->cp_save );
  int i;
  for( i = 0; i < max; i++ )
  {
    Cygnus_Result_wk_Init( &p_wk->icon[i], p_wk->p_clunit, p_wk->cp_save, i, p_wk->heapID );
  }
  p_wk->is_start = FALSE;
  p_wk->is_end = FALSE;

}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	CYGNUS_RESULT_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CYGNUS_RESULT_Release( CYGNUS_RESULT_WORK *p_wk )
{
  {
    u32 max = CYGNUS_SAVE_GetRewardMax( p_wk->cp_save );
    int i;
    for( i = 0; i < max; i++ )
    {
      Cygnus_Result_wk_Exit( &p_wk->icon[i] );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アイコンワーク
 *
 *	@param	CYGNUS_RESULT_ICON *p_wk  ワーク
 *	@param	*p_clunit 
 *	@param	CYGNUS_SAVE_WORK *cp_save
 *	@param	heapID 
 */
//-----------------------------------------------------------------------------
static void Cygnus_Result_wk_Init( CYGNUS_RESULT_ICON *p_wk, GFL_CLUNIT *p_clunit, const CYGNUS_SAVE_WORK *cp_save, u16 idx, HEAPID heapID )
{
  CYGNUS_REWARD_TYPE type;
  u32 no;
  s8 ofs_x = 0;
  s8 ofs_y = 0;
  u32 plt_ofs = 0;

  GFL_STD_MemClear( p_wk, sizeof(CYGNUS_RESULT_ICON) );

  //ご褒美の種類取得
  {
    BOOL ret = CYGNUS_SAVE_GetReward( cp_save, idx, &type, &no );
    GF_ASSERT( ret );
  }

  switch( type )
  {
  default:
    GF_ASSERT(0);
  case CYGNUS_REWARD_TYPE_POKEMON:
    //リソース読みこみ
    {	
      POKEMON_PARAM *p_pp = CYGNUS_SAVE_CreatePP( cp_save, idx, heapID );
      ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_POKEICON, heapID );

      p_wk->res_plt = GFL_CLGRP_PLTT_RegisterComp( p_handle, 
            POKEICON_GetPalArcIndex(), CLSYS_DRAW_MAIN, KEYSYS_PLT_OBJ_CYG_POKE_M*0x20, heapID );
      p_wk->res_cel = GFL_CLGRP_CELLANIM_Register( p_handle,
          POKEICON_GetCellArcIndex(), POKEICON_GetAnmArcIndex(), heapID );
      p_wk->res_cgx	= GFL_CLGRP_CGR_Register( p_handle,
          POKEICON_GetCgxArcIndex(PP_GetPPPPointer(p_pp)), FALSE, CLSYS_DRAW_MAIN, heapID );

      plt_ofs = POKEICON_GetPalNumGetByPPP( PP_GetPPPPointer(p_pp) );

      GFL_ARC_CloseDataHandle( p_handle );
      GFL_HEAP_FreeMemory( p_pp );

    }
    break;
  case CYGNUS_REWARD_TYPE_ITEM:
    {
      ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_ITEMICON, heapID );
      u32 item_cnt = CYGNUS_SAVE_GetItemCount( cp_save, idx-CYGNUS_SAVE_GetRewardPokeMax(cp_save) );

      p_wk->res_plt = GFL_CLGRP_PLTT_RegisterEx( p_handle, 
            ITEM_GetIndex( no, ITEM_GET_ICON_PAL ), CLSYS_DRAW_MAIN, (KEYSYS_PLT_OBJ_CYG_ITEM_M_00+item_cnt)*0x20, 0, 1, heapID );
      p_wk->res_cel = GFL_CLGRP_CELLANIM_Register( p_handle,
          NARC_item_icon_itemicon_NCER, NARC_item_icon_itemicon_NANR, heapID );
      p_wk->res_cgx	= GFL_CLGRP_CGR_Register( p_handle,
          ITEM_GetIndex( no, ITEM_GET_ICON_CGX ), FALSE, CLSYS_DRAW_MAIN, heapID );

      GFL_ARC_CloseDataHandle( p_handle );

      ofs_x   = 4;
      ofs_y   = 8;
    }
    break;
  }

  //CLWK登録
	{
    enum
    {

      ICON_W_NUM  = 6,
      ICON_H_NUM  = 2,

      ICON_W  = 32,
      ICON_H  = 24,

      MARGIN_H  = 16,
    };

		int i;
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    p_wk->def_pos.x  = (128+16)-(ICON_W_NUM*ICON_W/2)
      +(idx%ICON_W_NUM)*ICON_W + ofs_x;
    p_wk->def_pos.y  = (96-16)-(ICON_H_NUM*ICON_H/2)
      +(idx/ICON_W_NUM)*ICON_H + ofs_y + (idx/ICON_W_NUM == 0 ? 0: MARGIN_H);

    cldata.pos_x  = p_wk->def_pos.x;
    cldata.pos_y  = p_wk->def_pos.y - CYGNUS_RESULT_ICON_MOVE_DISTANCE;
    cldata.bgpri  = 0;

		p_wk->p_clwk	=	GFL_CLACT_WK_Create( p_clunit,
				p_wk->res_cgx,
				p_wk->res_plt,
				p_wk->res_cel,
				&cldata,
				CLSYS_DEFREND_MAIN,
				heapID );

    GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk, plt_ofs, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, FALSE );
    GFL_CLACT_WK_SetObjMode( p_wk->p_clwk, GX_OAM_MODE_XLU );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  アイコンワーク破棄
 *
 *	@param	CYGNUS_RESULT_ICON *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Cygnus_Result_wk_Exit( CYGNUS_RESULT_ICON *p_wk )
{
  GFL_CLACT_WK_Remove( p_wk->p_clwk );

	//リソース破棄
	{
    GFL_CLGRP_PLTT_Release( p_wk->res_plt );
    GFL_CLGRP_CGR_Release( p_wk->res_cgx );
    GFL_CLGRP_CELLANIM_Release( p_wk->res_cel );
	}

  GFL_STD_MemClear( p_wk, sizeof(CYGNUS_RESULT_ICON) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  移動開始
 *
 *	@param	CYGNUS_RESULT_ICON *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Cygnus_Result_wk_StartMove( CYGNUS_RESULT_ICON *p_wk )
{
  G2_SetBlendAlpha(
      GX_BLEND_PLANEMASK_NONE,/*GX_BLEND_PLANEMASK_OBJ,->全部のOBJがなっちゃう*/
      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 ,
      0,
      16
      );

  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );

  p_wk->is_start  = TRUE;

  p_wk->start_ms = OS_TicksToMilliSeconds(OS_GetTick());
}
//----------------------------------------------------------------------------
/**
 *	@brief  移動メイン
 *
 *	@param	CYGNUS_RESULT_ICON *p_wk  ワーク
 *
 *	@return TRUE　移動終了  FALSE移動中
 */
//-----------------------------------------------------------------------------
static BOOL Cygnus_Result_wk_WaitMove( CYGNUS_RESULT_ICON *p_wk )
{
  if( p_wk->is_start )
  {
    u64 now_ms  = OS_TicksToMilliSeconds(OS_GetTick()) - p_wk->start_ms;

    if( now_ms > CYGNUS_RESULT_ICON_MOVE_MS )
    {
      GFL_CLACT_WK_SetPos( p_wk->p_clwk, &p_wk->def_pos, CLSYS_DEFREND_MAIN );
      GFL_CLACT_WK_SetObjMode( p_wk->p_clwk, GX_OAM_MODE_NORMAL );
      G2_BlendNone();

      return TRUE;
    }
    else
    {
      {
        GFL_CLACTPOS  pos;
        pos.x = p_wk->def_pos.x;
        pos.y = p_wk->def_pos.y - CYGNUS_RESULT_ICON_MOVE_DISTANCE + CYGNUS_RESULT_ICON_MOVE_DISTANCE * now_ms / CYGNUS_RESULT_ICON_MOVE_MS;
        GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, CLSYS_DEFREND_MAIN );
      }
      {
        s16 ev1 = 0 + 16 * now_ms / CYGNUS_RESULT_ICON_MOVE_MS;
        s16 ev2 = 16 - 16 * now_ms / CYGNUS_RESULT_ICON_MOVE_MS;
        G2_ChangeBlendAlpha( ev1, ev2 );
      }
    }
  }
  return FALSE;
}

//=============================================================================
/**
 *  その他
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスフロー用一時システム作成
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 *	@param  HEAPID  heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void Cygnus_3d_Init( KEY_SYSTEM_WORK *p_wk, HEAPID heapID )
{
  KEY_SYSTEM_GRAPHICS_ChangeMode( p_wk->p_graphic, KEYSYS_GRAPHICS_MODE_2D_TO_3D );

  GF_ASSERT( p_wk->p_ptc == NULL );
  p_wk->p_ptc = KEYSYS_PTC_Init( heapID );

  GF_ASSERT( p_wk->p_cyg_demo == NULL );
  p_wk->p_cyg_demo  = CYGNUS_DEMO_Init( p_wk->p_ptc, p_wk->p_graphic, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  シグナスフロー用一時システム破棄
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Cygnus_3d_Exit( KEY_SYSTEM_WORK *p_wk )
{
  CYGNUS_DEMO_Exit( p_wk->p_cyg_demo );
  p_wk->p_cyg_demo  = NULL;

  KEYSYS_PTC_Exit( p_wk->p_ptc );
  p_wk->p_ptc = NULL;

  KEY_SYSTEM_GRAPHICS_ChangeMode( p_wk->p_graphic, KEYSYS_GRAPHICS_MODE_3D_TO_2D );
}
//----------------------------------------------------------------------------
/**
 *	@brief  デモ用の背景を設定
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void Cygnus_Demo_Graphic_Set( KEY_SYSTEM_WORK *p_wk, HEAPID heapID )
{
  GFL_BG_ClearScreen(KEYSYS_BG_FRAME_WND_M);

  KEYSYS_TEXT_Delete( p_wk );

  //0面は3Dで使うので、破棄。
  //テキスト面をWND_Mにする

  BmpWinFrame_GraphicSet( KEYSYS_BG_FRAME_WND_M, KEYSYS_CGR_TEXT_M, KEYSYS_PLT_BG_FRAME_M, MENU_TYPE_SYSTEM, HEAPID_KEY_SYSTEM );

  KEYSYS_BG_RequestPltFade( p_wk->p_bg, KEYSYS_BG_PLTFADE_REQ_CYGNUS_TO_DEMO, 1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  デモ用の背景から通常に戻す
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Cygnus_Demo_Graphic_Reset( KEY_SYSTEM_WORK *p_wk )
{
  GFL_ARC_UTIL_TransVramBgCharacter( ARCID_KEY_SYSTEM, NARC_key_system_gra_back00_NCGR, 
      KEYSYS_BG_FRAME_WND_M, 0, 0, FALSE, HEAPID_KEY_SYSTEM );

  KEYSYS_BG_TransScreen( p_wk->p_bg, KEYSYS_BG_FRAME_WND_M, KEYSYS_BG_SCR_TITLE_M );

  KEYSYS_TEXT_Create(p_wk,HEAPID_KEY_SYSTEM);

  KEYSYS_TEXT_Print( p_wk->p_title, p_wk->p_msg, CYGNUS_00, KEYSYS_TEXT_TYPE_QUE );
  KEYSYS_TEXT_Print( p_wk->p_text, p_wk->p_msg, KEYSYS_STR_000, KEYSYS_TEXT_TYPE_QUE );
  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_TEXT_M );
  GFL_BG_LoadScreenV_Req( KEYSYS_BG_FRAME_WND_M );

  KEYSYS_BG_RequestPltFade( p_wk->p_bg, KEYSYS_BG_PLTFADE_REQ_DEMO_TO_CYGNUS, 1 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  シグナスフローを開始して良いか
 *
 *	@param	KEY_SYSTEM_WORK *p_wk ワーク
 *	@param  p_error_msg   エラーメッセージ受け取り
 *
 *	@return TRUE受け取れる  FALSE受け取れない
 */
//-----------------------------------------------------------------------------
static CYGNUS_RECIVE Cygnus_IsReciveEnable( KEY_SYSTEM_WORK *p_wk, u32 *p_error_msg )
{
  u32 error;

  //p_error_msgを貰わないときのダミー
  if( p_error_msg == NULL )
  {
    p_error_msg = &error;
  }

  //報酬データが壊れていた
  {
    if( CYGNUS_SAVE_GetStatus(p_wk->p_cyg_sv) == CYGNUS_SAVE_STATUS_BREAK )
    {
      return CYGNUS_RECIVE_BREAK;
    }
  }

  //SWAN本編のセーブデータ（初期３匹入手済みでないとNG）
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
    EVENTWORK *p_ev = GAMEDATA_GetEventWork( p_wk->p_param->p_gamedata );
    if( SaveData_GetExistFlag(p_sv) )
    {
      //セーブあるけどずかんない
      if( !EVENTWORK_CheckEventFlag(p_ev, SYS_FLAG_FIRST_POKE_GET ) )
      {
        *p_error_msg  = CYGNUS_15;
        return CYGNUS_RECIVE_NG;
      }
    }
    else
    {
      //セーブない
      *p_error_msg  = CYGNUS_06;
      return CYGNUS_RECIVE_NG;
    }
  }


  //報酬データがない
  {
    CYGNUS_SAVE_STATUS status = CYGNUS_SAVE_GetStatus( p_wk->p_cyg_sv );
    if( status == CYGNUS_SAVE_STATUS_NONE )
    {
      *p_error_msg  = CYGNUS_07;
      return CYGNUS_RECIVE_NG;
    }
    if( status == CYGNUS_SAVE_STATUS_EMPTY )
    {
      *p_error_msg  = CYGNUS_08;
      return CYGNUS_RECIVE_NG;
    }
  }

  //ボックスに６匹分の空きがない
  {
    enum
    {
      CYGNUS_BOX_NEED_EMPTY_NUM = 6,
    };

    BOX_MANAGER *p_box  = GAMEDATA_GetBoxManager( p_wk->p_param->p_gamedata );
    const u32 empty_num = BOXDAT_GetEmptySpaceTotal( p_box );
    if( !(CYGNUS_BOX_NEED_EMPTY_NUM <= empty_num) )
    {
      *p_error_msg  = CYGNUS_09;
      return CYGNUS_RECIVE_NG;
    }
  }

  return CYGNUS_RECIVE_OK;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンの足元座標を取得
 *
 *	@param	const POKEMON_PASO_PARAM* cp_ppp  ポケモン
 *
 *	@return 足元座標(ドット単位）
 */
//-----------------------------------------------------------------------------
static u32 Cygnus_GetLegPos( const POKEMON_PASO_PARAM* cp_ppp, HEAPID heapID )
{ 
  u32 temp;
  u32 pos = 48;  //中心にしておく

  void *p_chr_buff;
  NNSG2dCharacterData *p_chr_data;
  

  p_chr_buff  =  POKE2DGRA_LoadCharacterPPP( &p_chr_data, cp_ppp, POKEGRA_DIR_FRONT, GFL_HEAP_LOWID(heapID) );
  POKEGRA_SortBGCharacter( p_chr_data, GFL_HEAP_LOWID(heapID) );

  //足元サーチ
  { 
    int i, j, k;
    u32  chara_adrs;
    u32 *p_chara_y;

    BOOL exits  = FALSE;

    for( i = POKEGRA_POKEMON_CHARA_HEIGHT-1; i >= 0; i-- )
    { 
      for( j = 0; j < POKEGRA_POKEMON_CHARA_WIDTH; j++ )
      { 
        chara_adrs = ((u32)p_chr_data->pRawData + ((i * POKEGRA_POKEMON_CHARA_WIDTH + j ) * GFL_BG_1CHRDATASIZ));

        //Yを知れればよいのでYしか検知しない
        for( k = GFL_BG_1CHRDOTSIZ-1; k >= 0; k-- )
        { 
          p_chara_y = (u32*)(chara_adrs + k * 4);
          if( *p_chara_y != 0 )
          { 
            NAGI_Printf( "足元キャラx=%d y=%d doty=%d\n", j, i, k );
            temp = i*8+k;
            if( pos < temp )
            { 
              pos = temp;
            }
            exits = TRUE;
          }
        }
      }

      //横軸を全てチェックし終わったら終了
      if( exits )
      { 
        break;
      }
    }
  }

  GFL_HEAP_FreeMemory( p_chr_buff );

  return pos;
}
