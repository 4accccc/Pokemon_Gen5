//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_core.c
 *	@brief	バトルレコーダーコア部分（バトルレコードプレイヤー以外の部分）
 *	@author	Toru=Nagihashi
 *	@data		2009.11.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID

//自分のモジュール
#include "br_graphic.h"
#include "br_res.h"
#include "br_proc_sys.h"
#include "br_fade.h"
#include "br_sidebar.h"

//各プロセス
#include "br_start_proc.h"
#include "br_menu_proc.h"
#include "br_record_proc.h"
#include "br_btlsubway_proc.h"
#include "br_rndmatch_proc.h"
#include "br_bvrank_proc.h"
#include "br_bvsearch_proc.h"
#include "br_codein_proc.h"
#include "br_bvsend_proc.h"
#include "br_musicallook_proc.h"
#include "br_musicalsend_proc.h"

//セーブデータ
#include "savedata/battle_rec.h"
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"

//外部参照
#include "br_core.h"

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
//-------------------------------------
///	メインワーク
//=====================================
typedef struct 
{
	//グラフィック
	BR_GRAPHIC_WORK	*p_graphic;

	//リソース管理
	BR_RES_WORK			*p_res;

	//プロセス管理
	BR_PROC_SYS			*p_procsys;

  //フェード
  BR_FADE_WORK    *p_fade;

  //サイドバー
  BR_SIDEBAR_WORK *p_sidebar;

	//引数
	BR_CORE_PARAM		*p_param;

  BR_MENU_BTLREC_DATA  btlrec_data;
} BR_CORE_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BRコアプロセス
//=====================================
static GFL_PROC_RESULT BR_CORE_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_CORE_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_CORE_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	各プロセスのための処理関数
//=====================================
//起動
static void BR_START_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_START_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//メニュー
static void BR_MENU_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_MENU_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//レコード
static void BR_RECORD_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_RECORD_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//バトルサブウェイ
static void BR_BTLSUBWAY_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_BTLSUBWAY_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//ランダムマッチ
static void BR_RNDMATCH_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_RNDMATCH_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//バトルビデオ30件
static void BR_BVRANK_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_BVRANK_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//詳しく探す
static void BR_BVSEARCH_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_BVSEARCH_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//ビデオナンバー
static void BR_CODEIN_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_CODEIN_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//ビデオ送信
static void BR_BVSEND_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_BVSEND_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//ミュージカルショット写真をみる
static void BR_MUSICALLOOK_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_MUSICALLOOK_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//ミュージカルショット写真を送る
static void BR_MUSICALSEND_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_MUSICALSEND_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	バトルレコーダーコアプロセス
//=====================================
const GFL_PROC_DATA BR_CORE_ProcData =
{	
	BR_CORE_PROC_Init,
	BR_CORE_PROC_Main,
	BR_CORE_PROC_Exit,
};

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	プロセス繋がり情報
//=====================================
static const BR_PROC_SYS_DATA sc_procdata_tbl[BR_PROCID_MAX]	=
{	
	//	BR_PROCID_START,		//起動画面
	{	
		&BR_START_ProcData,
		sizeof( BR_START_PROC_PARAM ),
		BR_START_PROC_BeforeFunc,
		BR_START_PROC_AfterFunc,
	},
	//	BR_PROCID_MENU,			//メニュー画面
	{	
		&BR_MENU_ProcData,
		sizeof( BR_MENU_PROC_PARAM ),
		BR_MENU_PROC_BeforeFunc,
		BR_MENU_PROC_AfterFunc,
	},
	//	BR_PROCID_RECORD,		//録画記録画面
	{	
		&BR_RECORD_ProcData,
		sizeof( BR_RECORD_PROC_PARAM ),
		BR_RECORD_PROC_BeforeFunc,
		BR_RECORD_PROC_AfterFunc,
	},
	//	BR_PROCID_BTLSUBWAY,//バトルサブウェイ成績画面
	{	
		&BR_BTLSUBWAY_ProcData,
		sizeof( BR_BTLSUBWAY_PROC_PARAM ),
		BR_BTLSUBWAY_PROC_BeforeFunc,
		BR_BTLSUBWAY_PROC_AfterFunc,
	},
	//	BR_PROCID_RNDMATCH,	//ランダムマッチ成績画面
	{	
		&BR_RNDMATCH_ProcData,
		sizeof( BR_RNDMATCH_PROC_PARAM ),
		BR_RNDMATCH_PROC_BeforeFunc,
		BR_RNDMATCH_PROC_AfterFunc,
	},
	//	BR_PROCID_BV_RANK,	//バトルビデオ30件画面（最新、通信対戦、サブウェイ）
	{	
		&BR_BVRANK_ProcData,
		sizeof( BR_BVRANK_PROC_PARAM ),
		BR_BVRANK_PROC_BeforeFunc,
		BR_BVRANK_PROC_AfterFunc,
	},
	//	BR_PROCID_BV_SEARCH,//詳しく探す画面
	{	
		&BR_BVSEARCH_ProcData,
		sizeof( BR_BVSEARCH_PROC_PARAM ),
		BR_BVSEARCH_PROC_BeforeFunc,
		BR_BVSEARCH_PROC_AfterFunc,
	},
	//	BR_PROCID_CODEIN		,//バトルビデオナンバー入力画面
	{	
		&BR_CODEIN_ProcData,
		sizeof( BR_CODEIN_PROC_PARAM ),
		BR_CODEIN_PROC_BeforeFunc,
		BR_CODEIN_PROC_AfterFunc,
	},
	//	BR_PROCID_BV_SEND,	//バトルビデオ送信画面
	{	
		&BR_BVSEND_ProcData,
		sizeof( BR_BVSEND_PROC_PARAM ),
		BR_BVSEND_PROC_BeforeFunc,
		BR_BVSEND_PROC_AfterFunc,
	},
	//	BR_PROCID_MUSICAL_LOOK,	//ミュージカルショット	写真を見る画面
	{	
		&BR_MUSICALLOOK_ProcData,
		sizeof( BR_MUSICALLOOK_PROC_PARAM ),
		BR_MUSICALLOOK_PROC_BeforeFunc,
		BR_MUSICALLOOK_PROC_AfterFunc,
	},
	//	BR_PROCID_MUSICAL_SEND,	//ミュージカルショット	写真を送る画面
	{	
		&BR_MUSICALSEND_ProcData,
		sizeof( BR_MUSICALSEND_PROC_PARAM ),
		BR_MUSICALSEND_PROC_BeforeFunc,
		BR_MUSICALSEND_PROC_AfterFunc,
	},
};

//=============================================================================
/**
 *					BRコアプロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダーコア	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_CORE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_CORE_WORK	*p_wk;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_RECORDER_CORE, 0x60000 );
//	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_RECORDER_CORE, 0x30000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_CORE_WORK), HEAPID_BATTLE_RECORDER_CORE );
	GFL_STD_MemClear( p_wk, sizeof(BR_CORE_WORK) );	
	p_wk->p_param		= p_param_adrs;

	//グラフィック初期化
	p_wk->p_graphic	= BR_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_BATTLE_RECORDER_CORE );
	p_wk->p_res			= BR_RES_Init( HEAPID_BATTLE_RECORDER_CORE );
	BR_RES_LoadBG( p_wk->p_res, BR_RES_BG_START_M, HEAPID_BATTLE_RECORDER_CORE );
	BR_RES_LoadBG( p_wk->p_res, BR_RES_BG_START_S, HEAPID_BATTLE_RECORDER_CORE );

	//コアプロセス初期化
  p_wk->p_procsys	= BR_PROC_SYS_Init( BR_PROCID_START, sc_procdata_tbl, 
      BR_PROCID_MAX, p_wk, HEAPID_BATTLE_RECORDER_CORE );

  //フェードプロセス
  p_wk->p_fade  = BR_FADE_Init( HEAPID_BATTLE_RECORDER_CORE );
  BR_FADE_PALETTE_Copy( p_wk->p_fade );  //BR_RES_Initでよんでいるので
  BR_FADE_PALETTE_SetColor( p_wk->p_fade, BR_FADE_COLOR_BLUE );
  if( p_wk->p_param->mode == BR_CORE_MODE_INIT )
  { 
    BR_FADE_PALETTE_TransColor( p_wk->p_fade, BR_FADE_DISPLAY_BOTH );
  }

  //サイドバー作成
  {
    GFL_CLUNIT  *p_clunit = BR_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_wk->p_sidebar = BR_SIDEBAR_Init( p_clunit, p_wk->p_fade, p_wk->p_res, HEAPID_BATTLE_RECORDER_CORE );
  }

  //メニューで使用する録画データ
  {	
    int i;
    LOAD_RESULT result;
    SAVE_CONTROL_WORK *p_sv;
    GDS_PROFILE_PTR p_profile;
    
    p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_param->p_gamedata ); 

    //設定構造体作成
    for( i = 0; i < BR_BTLREC_DATA_NUM; i++ )
    {
      BattleRec_Load( p_sv, GFL_HEAP_LOWID( HEAPID_BATTLE_RECORDER_CORE ), &result, i ); 
      if( result == LOAD_RESULT_OK )
      { 
        NAGI_Printf( "戦闘録画読み込み %d\n", i );
        p_profile = BattleRec_GDSProfilePtrGet();
        p_wk->btlrec_data.is_valid[i] = TRUE;
        p_wk->btlrec_data.p_name[i]   = GDS_Profile_CreateNameString( p_profile, HEAPID_BATTLE_RECORDER_CORE );
        p_wk->btlrec_data.sex[i]      = GDS_Profile_GetSex( p_profile );
      }
    }
  }

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダーコア	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_CORE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_CORE_WORK	*p_wk	= p_wk_adrs;

  { 
    int i;
    //設定構造体破棄
    for( i = 0; i < BR_BTLREC_DATA_NUM; i++ )
    { 
      if( p_wk->btlrec_data.is_valid[i] )
      { 
        GFL_STR_DeleteBuffer( p_wk->btlrec_data.p_name[i] );
      }
    }
	}


  //サイドバー破棄
  BR_SIDEBAR_Exit( p_wk->p_sidebar, p_wk->p_res );

  //フェードプロセス破棄
  BR_FADE_Exit( p_wk->p_fade );

	//コアプロセス破棄
	BR_PROC_SYS_Exit(	p_wk->p_procsys );

	//グラフィック破棄
	BR_RES_UnLoadBG( p_wk->p_res, BR_RES_BG_START_M );
	BR_RES_UnLoadBG( p_wk->p_res, BR_RES_BG_START_S );
	BR_RES_Exit( p_wk->p_res );
	BR_GRAPHIC_Exit( p_wk->p_graphic );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_BATTLE_RECORDER_CORE );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダーコア	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_CORE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  enum
  { 
		SEQ_INIT,
		SEQ_FADEIN,
		SEQ_FADEIN_WAIT,
		SEQ_MAIN,
		SEQ_FADEOUT,
		SEQ_FADEOUT_WAIT,
		SEQ_EXIT,
  };

	BR_CORE_WORK	*p_wk	= p_wk_adrs;

  switch( *p_seq )
	{	
	case SEQ_INIT:
    *p_seq	= SEQ_FADEIN;
		break;

	case SEQ_FADEIN:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_MAIN;
		}
		break;

	case SEQ_MAIN:
		{
			BOOL is_end;
      //フェードプロセス処理
      BR_FADE_Main( p_wk->p_fade );

      //サイドバー処理
      BR_SIDEBAR_Main( p_wk->p_sidebar );

      //プロセス処理
      BR_PROC_SYS_Main( p_wk->p_procsys );

      //描画
      BR_GRAPHIC_2D_Draw( p_wk->p_graphic );

			is_end	= BR_PROC_SYS_IsEnd( p_wk->p_procsys );

      //戦闘にいくときのみフェード
			if( is_end )
			{	
        if( p_wk->p_param->ret == BR_CORE_RETURN_BTLREC )
        { 
          *p_seq	= SEQ_FADEOUT;
        }
        else
        { 
          *p_seq  = SEQ_EXIT;
        }
			}
		}
		break;

	case SEQ_FADEOUT:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_EXIT;
		}
		break;

	case SEQ_EXIT:
		return GFL_PROC_RES_FINISH;
	}

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *					各プロセス接続用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_START_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_START_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK					*p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_sidebar  = p_wk->p_sidebar;

  if( preID == BR_PROCID_MENU )
  { 
    p_param->mode = BR_START_PROC_MODE_CLOSE;
  }
  else
  { 
      
    if( p_wk->p_param->mode == BR_CORE_MODE_INIT )
    { 
      p_param->mode = BR_START_PROC_MODE_OPEN;
    }
    else
    { 
      p_param->mode = BR_START_PROC_MODE_NONE;
    }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_START_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 

}

//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_MENU_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{	
	BR_MENU_PROC_PARAM	*p_param	= p_param_adrs;
	BR_CORE_WORK				*p_wk			= p_wk_adrs;

  //バトルから戻ってきたとき
  if( p_wk->p_param->mode == BR_CORE_MODE_RETURN )
  {
    //初期
    switch( p_wk->p_param->p_param->mode )
    { 
    case BR_MODE_BROWSE:
      p_param->menuID			= BR_BROWSE_MENUID_BTLVIDEO;
      break;

    case BR_MODE_GLOBAL_BV:
      p_param->menuID			= BR_BTLVIDEO_MENUID_RANK;
      break;

    case BR_MODE_GLOBAL_MUSICAL:
      //こない
      p_param->menuID			= BR_MUSICAL_MENUID_TOP;
      break;
    }
  }
  else
  { 
    //各プロセスから戻ってくるとき
    switch( preID )
    {	
    case BR_PROCID_RECORD:
      { 
        const BR_RECORD_PROC_PARAM	*cp_record_param	= cp_pre_param;
        if( cp_record_param->mode == BR_RECODE_PROC_MY )
        {	
          p_param->menuID			= BR_BROWSE_MENUID_BTLVIDEO;
        }
        else
        {
          p_param->menuID			= BR_BROWSE_MENUID_OTHER_RECORD;
        }
      }
      break;

    case BR_PROCID_BV_RANK:
      p_param->menuID = BR_BTLVIDEO_MENUID_RANK;
      break;

    case BR_PROCID_BV_SEARCH:
      /* fallthrough */
    case BR_PROCID_CODEIN:
      p_param->menuID = BR_BTLVIDEO_MENUID_LOOK;
      break;

    default:
      //初期
      switch( p_wk->p_param->p_param->mode )
      { 
      case BR_MODE_BROWSE:
        p_param->menuID			= BR_BROWSE_MENUID_TOP;
        break;

      case BR_MODE_GLOBAL_BV:
        p_param->menuID			= BR_BTLVIDEO_MENUID_TOP;
        break;

      case BR_MODE_GLOBAL_MUSICAL:
        p_param->menuID			= BR_MUSICAL_MENUID_TOP;
        break;
      }
    }
  }
	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->cp_btlrec  = &p_wk->btlrec_data;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_MENU_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{	
  BR_MENU_PROC_PARAM	*p_param	= p_param_adrs;
	BR_CORE_WORK				*p_wk			= p_wk_adrs;

}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_RECORD_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{	
	BR_RECORD_PROC_PARAM			*p_param	= p_param_adrs;
	BR_CORE_WORK							*p_wk			= p_wk_adrs;

	//メニュー以外から来ない

  p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
  p_param->p_procsys	= p_wk->p_procsys;
  p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_profile  = BattleRec_GDSProfilePtrGet();
  p_param->p_header   = BattleRec_HeaderPtrGet();
  switch( preID )
  { 
  case BR_PROCID_MENU:
    { 
      const BR_MENU_PROC_PARAM	*cp_menu_param	= cp_pre_param;
      p_param->mode				= cp_menu_param->next_mode;
    }
    break;

  case BR_PROCID_BV_RANK:
    { 
      //@todoとりあえず今は自分
      p_param->mode       = BR_RECODE_PROC_MY;
    }
    break;

  case BR_PROCID_CODEIN:
    { 
      //@todoとりあえず今は自分
      p_param->mode       = BR_RECODE_PROC_MY;
    }
    break;

  default:
    GF_ASSERT_MSG( 0, "メニューとランキング、コード入力以外からは来ない %d", preID );
    break;
  }

  //読み込み
  {
    LOAD_RESULT result;
    BattleRec_Load( SaveControl_GetPointer(), HEAPID_BATTLE_RECORDER_SYS, &result, p_param->mode ); 
    if( result == LOAD_RESULT_OK )
    { 
      if( p_param->mode == LOADDATA_MYREC )
      { 
        GDS_Profile_MyDataSet(p_param->p_profile, SaveControl_GetPointer());
      }
      NAGI_Printf( "戦闘録画読み込み %d\n", p_param->mode );
    }
    else
    { 
      GF_ASSERT(0);
    }
  }
  
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_RECORD_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{	
  BR_RECORD_PROC_PARAM			*p_param	= p_param_adrs;
	BR_CORE_WORK							*p_wk			= p_wk_adrs;
  
  switch( p_param->ret )
  { 
  case BR_RECORD_RETURN_BTLREC:
    p_wk->p_param->ret = BR_CORE_RETURN_BTLREC;
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_BTLSUBWAY_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{	
	BR_BTLSUBWAY_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK							*p_wk			= p_wk_adrs;
	const BR_MENU_PROC_PARAM	*cp_menu_param	= cp_pre_param;

	//メニュー以外から来ない
	GF_ASSERT_MSG( preID == BR_PROCID_MENU, "メニュー以外からは来ない %d", preID );
/*
	switch( cp_menu_param->next_data )
	{	
		
	}
*/
	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_subway   = SaveControl_DataPtrGet( SaveControl_GetPointer(), GMDATA_ID_BSUBWAY_SCOREDATA );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_BTLSUBWAY_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{	

}

//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_RNDMATCH_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{	
	BR_RNDMATCH_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK							*p_wk			= p_wk_adrs;
	const BR_MENU_PROC_PARAM	*cp_menu_param	= cp_pre_param;

	//メニュー以外から来ない
	GF_ASSERT_MSG( preID == BR_PROCID_MENU, "メニュー以外からは来ない %d", preID );

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_rndmatch = SaveData_GetRndMatch( SaveControl_GetPointer() );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_RNDMATCH_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{	

}

//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_BVRANK_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_BVRANK_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK						*p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_BVRANK_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 

}

//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_BVSEARCH_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_BVSEND_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK						*p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_BVSEARCH_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 

}

//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_CODEIN_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_CODEIN_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK						*p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_CODEIN_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 

}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_BVSEND_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_BVSEND_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK						*p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_BVSEND_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 


}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_MUSICALLOOK_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_MUSICALLOOK_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK						    *p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_graphic	= p_wk->p_graphic;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_MUSICALLOOK_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 

}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理前関数
 *
 *	@param	void *p_param_adrs	アロケートされた引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 *	@param	void *cp_pre_param	このプロセスの前の引数ワーク
 *	@param	preID								このプロセスの前のID
 */
//-----------------------------------------------------------------------------
static void BR_MUSICALSEND_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_MUSICALSEND_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK						    *p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_graphic	= p_wk->p_graphic;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_MUSICALSEND_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 

}
