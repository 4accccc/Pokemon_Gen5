//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		digitalcardcheck.c
 *	@brief	デジタル選手証確認画面
 *	@author	Toru=Nagihashi
 *	@data		2010.1.26
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	ライブラリ
#include <gflib.h>

//	システム
#include "system/main.h"
#include "system/gfl_use.h"
#include "net/dreamworld_netdata.h"

//	アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "wifimatch_gra.naix"
#include "msg/msg_wifi_match.h"

//	文字表示
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

//  セーブデータ
#include "savedata/regulation.h"

//  WIFIバトルマッチのモジュール
#include "wifibattlematch_graphic.h"
#include "wifibattlematch_view.h"
#include "wifibattlematch_util.h"

//  PROC
#include "battle_championship/battle_championship.h"

//  外部公開
#include "net_app/digitalcardcheck.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(battle_championship);

//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define DEBUG_INPUT_STATUS  (2)   //0かundefだとやらない  その他は開催ステータスになる
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define DIGITALCARDCHECK_PUSH_BUTTON  (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL)

//=============================================================================
/**
 *        定義
*/
//=============================================================================
//-------------------------------------
///	メインワーク
//=====================================
typedef struct 
{
	//グラフィック設定
	WIFIBATTLEMATCH_GRAPHIC_WORK	*p_graphic;

  //リソース読み込み
  WIFIBATTLEMATCH_VIEW_RESOURCE *p_res;

	//共通で使うフォント
	GFL_FONT			            *p_font;

	//共通で使うキュー
	PRINT_QUE				          *p_que;

	//共通で使うメッセージ
	GFL_MSGDATA	          		*p_msg;

	//共通で使う単語
	WORDSET				          	*p_word;

	//上画面情報
	PLAYERINFO_WORK         	*p_playerinfo;

  //テキスト面
  WBM_TEXT_WORK             *p_text;

  //リスト
  WBM_LIST_WORK             *p_list;

  //メインシーケンス
  WBM_SEQ_WORK              *p_seq;

  //引数
  GAMEDATA                  *p_game_data;
} DIGITALCARDCHECK_WORK;

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT DIGITALCARDCHECK_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT DIGITALCARDCHECK_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT DIGITALCARDCHECK_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	WIFI大会シーケンス関数
//=====================================
static void DC_SEQFUNC_Init( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DC_SEQFUNC_SignUp( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DC_SEQFUNC_Entry( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DC_SEQFUNC_CupEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DC_SEQFUNC_Retire( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DC_SEQFUNC_NoData( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void DC_SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	モジュールを使いやすくまとめたもの
//=====================================
//プレイヤー情報
static void Util_PlayerInfo_Create( DIGITALCARDCHECK_WORK *p_wk );
static void Util_PlayerInfo_Delete( DIGITALCARDCHECK_WORK *p_wk );
static BOOL Util_PlayerInfo_Move( DIGITALCARDCHECK_WORK *p_wk );
static void Util_PlayerInfo_RenewalData( DIGITALCARDCHECK_WORK *p_wk, PLAYERINFO_WIFI_UPDATE_TYPE type );
//選択肢
typedef enum
{ 
  UTIL_LIST_TYPE_YESNO,
  UTIL_LIST_TYPE_RETURN,
  UTIL_LIST_TYPE_UNREGISTER,
}UTIL_LIST_TYPE;
static void Util_List_Create( DIGITALCARDCHECK_WORK *p_wk, UTIL_LIST_TYPE type );
static void Util_List_Delete( DIGITALCARDCHECK_WORK *p_wk );
static u32 Util_List_Main( DIGITALCARDCHECK_WORK *p_wk );

//テキスト
static void Util_Text_Print( DIGITALCARDCHECK_WORK *p_wk, u32 strID );
static BOOL Util_Text_IsEnd( DIGITALCARDCHECK_WORK *p_wk );
static void Util_Text_SetVisible( DIGITALCARDCHECK_WORK *p_wk, BOOL is_visible );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	DigitalCard_ProcData =
{	
	DIGITALCARDCHECK_PROC_Init,
	DIGITALCARDCHECK_PROC_Main,
	DIGITALCARDCHECK_PROC_Exit,
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	コアプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DIGITALCARDCHECK_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	DIGITALCARDCHECK_WORK	*p_wk;


	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_DIGITALCARD_CHECK, 0x30000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(DIGITALCARDCHECK_WORK), HEAPID_DIGITALCARD_CHECK );
	GFL_STD_MemClear( p_wk, sizeof(DIGITALCARDCHECK_WORK) );
  p_wk->p_game_data   = GAMEDATA_Create( HEAPID_DIGITALCARD_CHECK );

	//グラフィック設定
	p_wk->p_graphic	= WIFIBATTLEMATCH_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_DIGITALCARD_CHECK );

  //リソース読み込み
  p_wk->p_res     = WIFIBATTLEMATCH_VIEW_LoadResource( WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic ), WIFIBATTLEMATCH_VIEW_RES_MODE_DIGITALCARD, HEAPID_DIGITALCARD_CHECK );

	//共通モジュールの作成
	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
      GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_DIGITALCARD_CHECK );
	p_wk->p_que		= PRINTSYS_QUE_Create( HEAPID_DIGITALCARD_CHECK );
	p_wk->p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
												NARC_message_wifi_match_dat, HEAPID_DIGITALCARD_CHECK );
	p_wk->p_word	= WORDSET_CreateEx( WORDSET_DEFAULT_SETNUM, WORDSET_COUNTRY_BUFLEN, HEAPID_DIGITALCARD_CHECK );

	//モジュールの作成
  p_wk->p_text  = WBM_TEXT_Init( BG_FRAME_M_TEXT, PLT_FONT_M, PLT_TEXT_M, CGR_OFS_M_TEXT, p_wk->p_que, p_wk->p_font, HEAPID_DIGITALCARD_CHECK );
  Util_Text_SetVisible( p_wk, FALSE );
  p_wk->p_seq   = WBM_SEQ_Init( p_wk, DC_SEQFUNC_Init, HEAPID_DIGITALCARD_CHECK );

#ifdef DEBUG_INPUT_STATUS
#if DEBUG_INPUT_STATUS
  { 
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_game_data );
    REGULATION_CARDDATA*p_reg = SaveData_GetRegulationCardData( p_sv );
    Regulation_SetDebugData( p_reg );
    Regulation_SetCardParam( p_reg, REGULATION_CARD_STATUS, DEBUG_INPUT_STATUS );
  }
#endif
#endif 

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	コアプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DIGITALCARDCHECK_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
	DIGITALCARDCHECK_WORK	  *p_wk	= p_wk_adrs;

	//モジュールの破棄
  WBM_SEQ_Exit( p_wk->p_seq );
  WBM_TEXT_Exit( p_wk->p_text );
  Util_PlayerInfo_Delete( p_wk );

	//共通モジュールの破棄
	WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

  //リソース破棄
  WIFIBATTLEMATCH_VIEW_UnLoadResource( p_wk->p_res );

	//グラフィック破棄
	WIFIBATTLEMATCH_GRAPHIC_Exit( p_wk->p_graphic );

  //ゲームデータ破棄
  GAMEDATA_Delete( p_wk->p_game_data );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_DIGITALCARD_CHECK );
	
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(battle_championship), &BATTLE_CHAMPIONSHIP_ProcData, NULL );

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DIGITALCARDCHECK_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_MAIN,
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
  };

	DIGITALCARDCHECK_WORK	*p_wk	    = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;

  case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      *p_seq  = SEQ_MAIN;
    }
    break;

  case SEQ_MAIN:
    //メインシーケンス
    WBM_SEQ_Main( p_wk->p_seq );

    if( WBM_SEQ_IsEnd( p_wk->p_seq ) )
    { 
      *p_seq  = SEQ_FADEOUT_START;
    }
    break;

  case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      return GFL_PROC_RES_FINISH;
    }
  }

	//描画
	WIFIBATTLEMATCH_GRAPHIC_2D_Draw( p_wk->p_graphic );

	//プリント
	PRINTSYS_QUE_Main( p_wk->p_que );

  //テキスト
  WBM_TEXT_Main( p_wk->p_text );

  //BG
  WIFIBATTLEMATCH_VIEW_Main( p_wk->p_res );

  //文字表示
  if( p_wk->p_playerinfo )
  { 
    PLAYERINFO_PrintMain( p_wk->p_playerinfo, p_wk->p_que );
  }

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *  シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  デジタル選手証チェック  初期化処理
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DC_SEQFUNC_Init( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  DIGITALCARDCHECK_WORK	  *p_wk	    = p_wk_adrs;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_game_data );
  REGULATION_CARDDATA * p_reg = SaveData_GetRegulationCardData( p_sv );
  const u32 cup_no      = Regulation_GetCardParam( p_reg, REGULATION_CARD_CUPNO );
  const u32 cup_status  = Regulation_GetCardParam( p_reg, REGULATION_CARD_STATUS );


  if( cup_no > 0 && cup_status == DREAM_WORLD_MATCHUP_SIGNUP )
  { 
    //大会に登録して、ポケモン未登録
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_SignUp );
  }
  else if( cup_no > 0 && cup_status == DREAM_WORLD_MATCHUP_ENTRY )
  { 
    //大会に登録して、ポケモン登録済み
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_Entry );
  }
  else if( cup_no > 0 && cup_status == DREAM_WORLD_MATCHUP_END )
  { 
    //大会終了
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_CupEnd );
  }
  else if( cup_no > 0 && cup_status == DREAM_WORLD_MATCHUP_RETIRE )
  { 
    //大会を棄権した
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_Retire );
  }
  else
  { 
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_NoData );
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  デジタル選手証チェック  サインアップした
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DC_SEQFUNC_SignUp( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DRAW_PLAYERINFO,
    SEQ_WAIT_DRAW_PLAYERINFO,
    SEQ_WAIT_PUSH,
    SEQ_START_MSG_SIGNUP,
    SEQ_WAIT_MSG,
    SEQ_START_LIST_RETURN,
    SEQ_WAIT_LIST_RETURN,
    SEQ_PROC_END,
  };
  DIGITALCARDCHECK_WORK	  *p_wk	    = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_DRAW_PLAYERINFO:
    Util_PlayerInfo_Create( p_wk );
    *p_seq  = SEQ_WAIT_DRAW_PLAYERINFO;
    break;

  case SEQ_WAIT_DRAW_PLAYERINFO:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq  = SEQ_WAIT_PUSH;
    }
    break;

  case SEQ_WAIT_PUSH:
    if( GFL_UI_KEY_GetTrg() & DIGITALCARDCHECK_PUSH_BUTTON )
    { 
      *p_seq  = SEQ_START_MSG_SIGNUP;
    }
    break;

  case SEQ_START_MSG_SIGNUP:
    Util_Text_SetVisible( p_wk, TRUE );
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_00 );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_WAIT_MSG:
    if( Util_Text_IsEnd( p_wk ) )
    { 
      *p_seq  = SEQ_START_LIST_RETURN;
    }
    break;

  case SEQ_START_LIST_RETURN:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_RETURN );
    *p_seq  = SEQ_WAIT_LIST_RETURN;
    break;

  case SEQ_WAIT_LIST_RETURN:
    { 
      const u32 select = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        if( select == 0 )
        { 
          *p_seq  = SEQ_PROC_END;
        }
      }
    }
    break;
    
  case SEQ_PROC_END:
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_End );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  デジタル選手証チェック  エントリー中
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DC_SEQFUNC_Entry( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DRAW_PLAYERINFO,
    SEQ_WAIT_DRAW_PLAYERINFO,
    SEQ_WAIT_PUSH,
    SEQ_START_MSG_ENTRY,
    SEQ_START_LIST_UNREGISTER,
    SEQ_WAIT_LIST_UNREGISTER,

    SEQ_START_MSG_CONFIRM1,
    SEQ_START_LIST_CONFIRM1,
    SEQ_WAIT_LIST_CONFIRM1,

    SEQ_START_MSG_CONFIRM2,
    SEQ_START_LIST_CONFIRM2,
    SEQ_WAIT_LIST_CONFIRM2,

    SEQ_START_MSG_UNREGISTER,
    SEQ_RETIRE,
    SEQ_START_SAVE,
    SEQ_WAIT_SAVE,
    SEQ_START_MSG_UNLOCK,

    SEQ_PROC_END,
    SEQ_WAIT_MSG,
  };
  DIGITALCARDCHECK_WORK	  *p_wk	    = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_DRAW_PLAYERINFO:
    Util_PlayerInfo_Create( p_wk );
    *p_seq  = SEQ_WAIT_DRAW_PLAYERINFO;
    break;

  case SEQ_WAIT_DRAW_PLAYERINFO:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq  = SEQ_WAIT_PUSH;
    }
    break;

  case SEQ_WAIT_PUSH:
    if( GFL_UI_KEY_GetTrg() & DIGITALCARDCHECK_PUSH_BUTTON )
    { 
      *p_seq  = SEQ_START_MSG_ENTRY;
    }
    break;

  case SEQ_START_MSG_ENTRY:
    Util_Text_SetVisible( p_wk, TRUE );
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_01 );
    *p_seq  = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_UNREGISTER );
    break;

  case SEQ_START_LIST_UNREGISTER:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_UNREGISTER );
    *p_seq  = SEQ_WAIT_LIST_UNREGISTER;
    break;

  case SEQ_WAIT_LIST_UNREGISTER:
    { 
      const u32 select = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        if( select == 0 )   //参加の解除
        { 
          *p_seq  = SEQ_START_MSG_CONFIRM1;
        }
        else if( select == 1 )  //もどる
        { 
          *p_seq  = SEQ_PROC_END;
        }
      }
    }
    break;

  case SEQ_START_MSG_CONFIRM1:
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_02 );
    *p_seq  = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_CONFIRM1 );
    break;
  case SEQ_START_LIST_CONFIRM1:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_WAIT_LIST_CONFIRM1;
    break;
  case SEQ_WAIT_LIST_CONFIRM1:
    { 
      const u32 select = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        if( select == 0 )   //はい
        { 
          *p_seq  = SEQ_START_MSG_CONFIRM2;
        }
        else if( select == 1 )  //いいえ
        { 
          *p_seq  = SEQ_PROC_END;
        }
      }
    }
    break;
  case SEQ_START_MSG_CONFIRM2:
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_03 );
    *p_seq  = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_START_LIST_CONFIRM2 );
    break;
  case SEQ_START_LIST_CONFIRM2:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_YESNO );
    *p_seq  = SEQ_WAIT_LIST_CONFIRM2;
    break;
  case SEQ_WAIT_LIST_CONFIRM2:
    { 
      const u32 select = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        if( select == 0 )   //はい
        { 
          *p_seq  = SEQ_RETIRE;
        }
        else if( select == 1 )  //いいえ
        { 
          *p_seq  = SEQ_PROC_END;
        }
      }
    }
    break;    

  case SEQ_START_MSG_UNREGISTER:
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_04 );
    *p_seq  = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_RETIRE );
    break;

  case SEQ_RETIRE:
    { 
      
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_game_data );
      BATTLE_BOX_SAVE   *p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
      REGULATION_CARDDATA*p_reg = SaveData_GetRegulationCardData( p_sv );

      BATTLE_BOX_SAVE_SetLockFlg( p_bbox_save, FALSE );
      Regulation_SetCardParam( p_reg, REGULATION_CARD_STATUS, DREAM_WORLD_MATCHUP_RETIRE );
      *p_seq  = SEQ_START_SAVE;
    }
    break;
    
  case SEQ_START_SAVE:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_game_data );
      SaveControl_SaveAsyncInit(p_sv);
      *p_seq  = SEQ_WAIT_SAVE;
    }
    break;
  case SEQ_WAIT_SAVE:
    {
      SAVE_RESULT ret;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_game_data );
      ret = SaveControl_SaveAsyncMain(p_sv);
      if( ret == SAVE_RESULT_OK )
      { 
        *p_seq  = SEQ_START_MSG_UNLOCK;
      }
      else if( ret == SAVE_RESULT_NG )
      { 
        *p_seq  = SEQ_START_MSG_UNLOCK;
      }
    }
    break;
  case SEQ_START_MSG_UNLOCK:
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_05 );
    *p_seq  = SEQ_WAIT_MSG;
    WBM_SEQ_SetReservSeq( p_seqwk, SEQ_PROC_END );
    break;

  case SEQ_PROC_END:
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_End );
    break;


  case SEQ_WAIT_MSG:
    if( Util_Text_IsEnd( p_wk ) )
    {
      WBM_SEQ_NextReservSeq( p_seqwk );
    }
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  デジタル選手証チェック  大会終了した
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DC_SEQFUNC_CupEnd( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DRAW_PLAYERINFO,
    SEQ_WAIT_DRAW_PLAYERINFO,
    SEQ_WAIT_PUSH,
    SEQ_START_MSG_CUPEND,
    SEQ_WAIT_MSG,
    SEQ_START_LIST_RETURN,
    SEQ_WAIT_LIST_RETURN,
    SEQ_PROC_END,
  };
  DIGITALCARDCHECK_WORK	  *p_wk	    = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_DRAW_PLAYERINFO:
    Util_PlayerInfo_Create( p_wk );
    *p_seq  = SEQ_WAIT_DRAW_PLAYERINFO;
    break;

  case SEQ_WAIT_DRAW_PLAYERINFO:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq  = SEQ_WAIT_PUSH;
    }
    break;

  case SEQ_WAIT_PUSH:
    if( GFL_UI_KEY_GetTrg() & DIGITALCARDCHECK_PUSH_BUTTON )
    { 
      *p_seq  = SEQ_START_MSG_CUPEND;
    }
    break;

  case SEQ_START_MSG_CUPEND:
    Util_Text_SetVisible( p_wk, TRUE );
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_06 );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_WAIT_MSG:
    if( Util_Text_IsEnd( p_wk ) )
    { 
      *p_seq  = SEQ_START_LIST_RETURN;
    }
    break;

  case SEQ_START_LIST_RETURN:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_RETURN );
    *p_seq  = SEQ_WAIT_LIST_RETURN;
    break;

  case SEQ_WAIT_LIST_RETURN:
    { 
      const u32 select = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        if( select == 0 )
        { 
          *p_seq  = SEQ_PROC_END;
        }
      }
    }
    break;
    
  case SEQ_PROC_END:
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_End );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  デジタル選手証チェック  リタイア
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DC_SEQFUNC_Retire( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_DRAW_PLAYERINFO,
    SEQ_WAIT_DRAW_PLAYERINFO,
    SEQ_WAIT_PUSH,
    SEQ_START_MSG_RETIRE,
    SEQ_WAIT_MSG,
    SEQ_START_LIST_RETURN,
    SEQ_WAIT_LIST_RETURN,
    SEQ_PROC_END,
  };
  DIGITALCARDCHECK_WORK	  *p_wk	    = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_DRAW_PLAYERINFO:
    Util_PlayerInfo_Create( p_wk );
    *p_seq  = SEQ_WAIT_DRAW_PLAYERINFO;
    break;

  case SEQ_WAIT_DRAW_PLAYERINFO:
    if( Util_PlayerInfo_Move( p_wk ) )
    { 
      *p_seq  = SEQ_WAIT_PUSH;
    }
    break;

  case SEQ_WAIT_PUSH:
    if( GFL_UI_KEY_GetTrg() & DIGITALCARDCHECK_PUSH_BUTTON )
    { 
      *p_seq  = SEQ_START_MSG_RETIRE;
    }
    break;

  case SEQ_START_MSG_RETIRE:
    Util_Text_SetVisible( p_wk, TRUE );
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_07 );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_WAIT_MSG:
    if( Util_Text_IsEnd( p_wk ) )
    { 
      *p_seq  = SEQ_START_LIST_RETURN;
    }
    break;

  case SEQ_START_LIST_RETURN:
    Util_List_Create( p_wk, UTIL_LIST_TYPE_RETURN );
    *p_seq  = SEQ_WAIT_LIST_RETURN;
    break;

  case SEQ_WAIT_LIST_RETURN:
    { 
      const u32 select = Util_List_Main( p_wk );
      if( select != WBM_LIST_SELECT_NULL )
      { 
        Util_List_Delete( p_wk );
        if( select == 0 )
        { 
          *p_seq  = SEQ_PROC_END;
        }
      }
    }
    break;
    
  case SEQ_PROC_END:
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_End );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  デジタル選手証チェック  データがない
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DC_SEQFUNC_NoData( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_MSG_NODATA,
    SEQ_WAIT_MSG,
    SEQ_PROC_END,
  };
  DIGITALCARDCHECK_WORK	  *p_wk	    = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_MSG_NODATA:
    Util_Text_SetVisible( p_wk, TRUE );
    Util_Text_Print( p_wk,  WIFIMATCH_DPC_STR_08 );
    *p_seq  = SEQ_WAIT_MSG;
    break;

  case SEQ_WAIT_MSG:
    if( Util_Text_IsEnd( p_wk ) )
    { 
      *p_seq  = SEQ_PROC_END;
    }
    break;

  case SEQ_PROC_END:
    WBM_SEQ_SetNext( p_seqwk, DC_SEQFUNC_End );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  デジタル選手証チェック  シーケンス終了
 *
 *	@param	WBM_SEQ_WORK *p_seqwk       シーケンスワーク
 *	@param  p_seq                       シーケンス
 *	@param	p_wk_adrs                   ワークアドレス
 */
//-----------------------------------------------------------------------------
static void DC_SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  DIGITALCARDCHECK_WORK	  *p_wk	    = p_wk_adrs;
  WBM_SEQ_End( p_seqwk );
}

//=============================================================================
/**
 *  便利
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を表示
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_PlayerInfo_Create( DIGITALCARDCHECK_WORK *p_wk )
{ 
  if( p_wk->p_playerinfo == NULL )
  {
    MYSTATUS    *p_my;
    GFL_CLUNIT	*p_unit;
    SAVE_CONTROL_WORK *p_sv;
    BATTLE_BOX_SAVE   *p_bbox_save;

    PLAYERINFO_WIFICUP_DATA info_setup;

    const REGULATION_CARDDATA *cp_reg_card  = SaveData_GetRegulationCardData(GAMEDATA_GetSaveControlWork( p_wk->p_game_data ));


    p_my    = GAMEDATA_GetMyStatus( p_wk->p_game_data); 
    p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_sv    = GAMEDATA_GetSaveControlWork( p_wk->p_game_data );
    p_bbox_save  = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );


    //自分のデータを表示
    GFL_STD_MemClear( &info_setup, sizeof(PLAYERINFO_WIFICUP_DATA) );
    GFL_STD_MemCopy( Regulation_GetCardCupNamePointer( cp_reg_card ), info_setup.cup_name, 2*(WIFI_PLAYER_TIX_CUPNAME_MOJINUM + EOM_SIZE) );

    info_setup.start_date = GFDATE_Set( 
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_START_YEAR ),
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_START_MONTH ), 
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_START_DAY ),
          0);

    info_setup.end_date = GFDATE_Set( 
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_END_YEAR ),
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_END_MONTH ), 
        Regulation_GetCardParam( cp_reg_card, REGULATION_CARD_END_DAY ),
          0);

    info_setup.trainerID  = MyStatus_GetTrainerView( p_my );

    info_setup.rate = 0;    //@todo
    info_setup.btl_cnt = 0;//@todo

    p_wk->p_playerinfo	= PLAYERINFO_WIFI_Init( &info_setup, FALSE, p_my, p_unit, p_wk->p_res, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, p_bbox_save, TRUE, HEAPID_DIGITALCARD_CHECK );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を破棄
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_PlayerInfo_Delete( DIGITALCARDCHECK_WORK *p_wk )
{ 
  if( p_wk->p_playerinfo )
  { 
    PLAYERINFO_WIFI_Exit( p_wk->p_playerinfo );
    p_wk->p_playerinfo  = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカードをスライドイン
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk  ワーク
 *
 *	@return TRUEで完了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL Util_PlayerInfo_Move( DIGITALCARDCHECK_WORK *p_wk )
{ 
  return PLAYERINFO_MoveMain( p_wk->p_playerinfo );
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のカードのデータを更新
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk
 *	@param	type 
 */
//-----------------------------------------------------------------------------
static void Util_PlayerInfo_RenewalData( DIGITALCARDCHECK_WORK *p_wk, PLAYERINFO_WIFI_UPDATE_TYPE type )
{ 
  PLAYERINFO_WIFI_RenewalData( p_wk->p_playerinfo, type, p_wk->p_msg, p_wk->p_que, p_wk->p_font, HEAPID_DIGITALCARD_CHECK );

}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk  ワーク
 *	@param	type                            リストの種類
 */
//-----------------------------------------------------------------------------
static void Util_List_Create( DIGITALCARDCHECK_WORK *p_wk, UTIL_LIST_TYPE type )
{ 
  if( p_wk->p_list == NULL )
  { 
    WBM_LIST_SETUP  setup;
    GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
    setup.p_msg   = p_wk->p_msg;
    setup.p_font  = p_wk->p_font;
    setup.p_que   = p_wk->p_que;
    setup.frm     = BG_FRAME_M_TEXT;
    setup.font_plt= PLT_FONT_M;
    setup.frm_plt = PLT_LIST_M;
    setup.frm_chr = CGR_OFS_M_LIST;

    switch( type )
    {
    case UTIL_LIST_TYPE_YESNO:
      setup.strID[0]= WIFIMATCH_DPC_SELECT_02;
      setup.strID[1]= WIFIMATCH_DPC_SELECT_03;
      setup.list_max= 2;
      break;
    case UTIL_LIST_TYPE_RETURN:
      setup.strID[0]= WIFIMATCH_DPC_SELECT_00;
      setup.list_max= 1;
      break;
    case UTIL_LIST_TYPE_UNREGISTER:
      setup.strID[0]= WIFIMATCH_DPC_SELECT_01;
      setup.strID[1]= WIFIMATCH_DPC_SELECT_00;
      setup.list_max= 2;
      break;
    }
    p_wk->p_list  = WBM_LIST_Init( &setup, HEAPID_DIGITALCARD_CHECK );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Util_List_Delete( DIGITALCARDCHECK_WORK *p_wk )
{
  if( p_wk->p_list )
  { 
    WBM_LIST_Exit( p_wk->p_list );
    p_wk->p_list  = NULL;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  リストメイン
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk ワーク
 *
 *	@return 選択したもの
 */
//-----------------------------------------------------------------------------
static u32 Util_List_Main( DIGITALCARDCHECK_WORK *p_wk )
{ 
  if( p_wk->p_list )
  { 
    return WBM_LIST_Main( p_wk->p_list );
  }
  else
  { 
    NAGI_Printf( "List not exist !!!\n" );
    return WBM_LIST_SELECT_NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  テキストに文字出力
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk ワーク
 *	@param	strID                       文字ID
 */
//-----------------------------------------------------------------------------
static void Util_Text_Print( DIGITALCARDCHECK_WORK *p_wk, u32 strID )
{ 
  WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, strID, WBM_TEXT_TYPE_STREAM );
}
//----------------------------------------------------------------------------
/**
 *	@brief  テキストの文字出力が終わったかどうか
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk   ワーク
 *
 *	@return TRUEで描画終了  FALSEで描画中
 */
//-----------------------------------------------------------------------------
static BOOL Util_Text_IsEnd( DIGITALCARDCHECK_WORK *p_wk )
{ 
  return WBM_TEXT_IsEnd( p_wk->p_text );
}
//----------------------------------------------------------------------------
/**
 *	@brief  テキストの表示設定
 *
 *	@param	DIGITALCARDCHECK_WORK *p_wk ワーク
 *	@param	is_visible                  TRUEで表示FALSEで非表示
 */
//-----------------------------------------------------------------------------
static void Util_Text_SetVisible( DIGITALCARDCHECK_WORK *p_wk, BOOL is_visible )
{ 
  GFL_BG_SetVisible( BG_FRAME_M_TEXT, is_visible );
}
