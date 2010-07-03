//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_core.c
 *	@brief	バトルレコーダーコア部分
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
#include "src\field\field_sound.h"

//自分のモジュール
#include "br_graphic.h"
#include "br_res.h"
#include "br_proc_sys.h"
#include "br_fade.h"
#include "br_sidebar.h"
#include "br_net.h"
#include "br_snd.h"

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
#include "br_bvdelete_proc.h"
#include "br_bvsave_proc.h"
#include "br_musicallook_proc.h"
#include "br_musicalsend_proc.h"

//セーブデータ
#include "savedata/battlematch_savedata.h"
#include "savedata/battle_rec.h"
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"

//デバッグ
#include "debug/debug_nagihashi.h"

//外部参照
#include "br_core.h"

//オーバーレイ
FS_EXTERN_OVERLAY( battle_recorder_browse );
FS_EXTERN_OVERLAY( battle_recorder_musical_look );
FS_EXTERN_OVERLAY( battle_recorder_musical_send );
FS_EXTERN_OVERLAY( gds_comm );

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

  //通信
  BR_NET_WORK     *p_net;

	//引数
	BR_CORE_PARAM		*p_param;
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
//ビデオ消去
static void BR_BVDELETE_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_BVDELETE_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//ビデオ保存
static void BR_BVSAVE_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_BVSAVE_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//ミュージカルショット写真をみる
static void BR_MUSICALLOOK_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_MUSICALLOOK_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//ミュージカルショット写真を送る
static void BR_MUSICALSEND_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
static void BR_MUSICALSEND_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	その他
//=====================================
static void Br_Core_CheckSaveData( BR_SAVE_INFO *p_saveinfo, BOOL is_onece, GAMEDATA *p_gamedata, HEAPID heapID );

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
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_START_PROC_BeforeFunc,
		BR_START_PROC_AfterFunc,
	},
	//	BR_PROCID_MENU,			//メニュー画面
	{	
		&BR_MENU_ProcData,
		sizeof( BR_MENU_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_MENU_PROC_BeforeFunc,
		BR_MENU_PROC_AfterFunc,
	},
	//	BR_PROCID_RECORD,		//録画記録画面
	{	
		&BR_RECORD_ProcData,
		sizeof( BR_RECORD_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_RECORD_PROC_BeforeFunc,
		BR_RECORD_PROC_AfterFunc,
	},
	//	BR_PROCID_BTLSUBWAY,//バトルサブウェイ成績画面
	{	
		&BR_BTLSUBWAY_ProcData,
		sizeof( BR_BTLSUBWAY_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_BTLSUBWAY_PROC_BeforeFunc,
		BR_BTLSUBWAY_PROC_AfterFunc,
	},
	//	BR_PROCID_RNDMATCH,	//ランダムマッチ成績画面
	{	
		&BR_RNDMATCH_ProcData,
		sizeof( BR_RNDMATCH_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_RNDMATCH_PROC_BeforeFunc,
		BR_RNDMATCH_PROC_AfterFunc,
	},
	//	BR_PROCID_BV_RANK,	//バトルビデオ30件画面（最新、通信対戦、サブウェイ）
	{	
		&BR_BVRANK_ProcData,
		sizeof( BR_BVRANK_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_BVRANK_PROC_BeforeFunc,
		BR_BVRANK_PROC_AfterFunc,
	},
	//	BR_PROCID_BV_SEARCH,//詳しく探す画面
	{	
		&BR_BVSEARCH_ProcData,
		sizeof( BR_BVSEARCH_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_BVSEARCH_PROC_BeforeFunc,
		BR_BVSEARCH_PROC_AfterFunc,
	},
	//	BR_PROCID_CODEIN		,//バトルビデオナンバー入力画面
	{	
		&BR_CODEIN_ProcData,
		sizeof( BR_CODEIN_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_CODEIN_PROC_BeforeFunc,
		BR_CODEIN_PROC_AfterFunc,
	},
	//	BR_PROCID_BV_SEND,	//バトルビデオ送信画面
	{	
		&BR_BVSEND_ProcData,
		sizeof( BR_BVSEND_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_BVSEND_PROC_BeforeFunc,
		BR_BVSEND_PROC_AfterFunc,
	},
	//BR_PROCID_BV_DELETE,	  //バトルビデオ消去画面
  { 
		&BR_BVDELETE_ProcData,
		sizeof( BR_BVDELETE_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_BVDELETE_PROC_BeforeFunc,
		BR_BVDELETE_PROC_AfterFunc,
  },
  //BR_PROCID_BV_SAVE,    //バトルビデオ保存画面
  { 
		&BR_BVSAVE_ProcData,
		sizeof( BR_BVSAVE_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_browse ),
		BR_BVSAVE_PROC_BeforeFunc,
		BR_BVSAVE_PROC_AfterFunc,
  },
	//	BR_PROCID_MUSICAL_LOOK,	//ミュージカルショット	写真を見る画面
	{	
		&BR_MUSICALLOOK_ProcData,
		sizeof( BR_MUSICALLOOK_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_musical_look ),
		BR_MUSICALLOOK_PROC_BeforeFunc,
		BR_MUSICALLOOK_PROC_AfterFunc,
	},
	//	BR_PROCID_MUSICAL_SEND,	//ミュージカルショット	写真を送る画面
	{	
		&BR_MUSICALSEND_ProcData,
		sizeof( BR_MUSICALSEND_PROC_PARAM ),
    FS_OVERLAY_ID( battle_recorder_musical_send ),
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
  BR_GRAPHIC_SETUP_TYPE graphic_type;

	//ヒープ作成
  //主にミュージカルショットのために多くとっています
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_RECORDER_CORE, 0x90000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_CORE_WORK), HEAPID_BATTLE_RECORDER_CORE );
	GFL_STD_MemClear( p_wk, sizeof(BR_CORE_WORK) );	
	p_wk->p_param		= p_param_adrs;
  p_wk->p_param->p_param->result  = BR_RESULT_RETURN;

	//グラフィック初期化
  if( p_wk->p_param->p_param->mode == BR_MODE_GLOBAL_MUSICAL )
  { 
    graphic_type  = BR_GRAPHIC_SETUP_3D;
  }
  else
  { 
    graphic_type  = BR_GRAPHIC_SETUP_2D;
  }
	p_wk->p_graphic	= BR_GRAPHIC_Init( graphic_type, GX_DISP_SELECT_MAIN_SUB, HEAPID_BATTLE_RECORDER_CORE );

  //初期リソース読み込み
  {
    MISC  *p_misc = GAMEDATA_GetMiscWork( p_wk->p_param->p_param->p_gamedata );
    p_wk->p_res			= BR_RES_Init( 
        MISC_GetBattleRecorderColor( p_misc ),
        p_wk->p_param->p_param->mode == BR_MODE_BROWSE,
        HEAPID_BATTLE_RECORDER_CORE );
  }
	BR_RES_LoadBG( p_wk->p_res, BR_RES_BG_START_M, HEAPID_BATTLE_RECORDER_CORE );
	BR_RES_LoadBG( p_wk->p_res, BR_RES_BG_START_S, HEAPID_BATTLE_RECORDER_CORE );

	//コアプロセス初期化
  p_wk->p_procsys	= BR_PROC_SYS_Init( BR_PROCID_START, sc_procdata_tbl, 
      BR_PROCID_MAX, p_wk, &p_wk->p_param->p_data->proc_recovery, HEAPID_BATTLE_RECORDER_CORE );

  //フェードプロセス(パレットフェードを使うのでリソースを読んだあとにコピーしたり色設定したりしている)
  p_wk->p_fade  = BR_FADE_Init( HEAPID_BATTLE_RECORDER_CORE );
  BR_FADE_PALETTE_Copy( p_wk->p_fade );  //BR_RES_InitでリソースをVRAMに読んでいるので、そこからコピー
  BR_FADE_PALETTE_SetColor( p_wk->p_fade, BR_RES_GetFadeColor( p_wk->p_res ) );
  if( p_wk->p_param->mode == BR_CORE_MODE_INIT )
  { 
    BR_FADE_PALETTE_TransColor( p_wk->p_fade, BR_FADE_DISPLAY_BOTH );
  }

  //サイドバー作成
  {
    GFL_CLUNIT  *p_clunit = BR_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_wk->p_sidebar = BR_SIDEBAR_Init( p_clunit, p_wk->p_fade, p_wk->p_res, HEAPID_BATTLE_RECORDER_CORE );
  }

  //メニューで使用する録画データをチェック
  Br_Core_CheckSaveData( &p_wk->p_param->p_data->rec_saveinfo,
      TRUE, p_wk->p_param->p_param->p_gamedata, HEAPID_BATTLE_RECORDER_CORE );

  //GDSのみ通信ON
  { 
    if( p_wk->p_param->p_param->mode != BR_MODE_BROWSE )
    { 
      p_wk->p_net = BR_NET_Init( 
          p_wk->p_param->p_param->p_gamedata,
          p_wk->p_param->p_param->p_svl,
          HEAPID_BATTLE_RECORDER_CORE );
    }
    else
    { 
      //このオーバーレイはgdsモードのときに読まれるが、
      //battle_rec操作があるため、通信を使わないときにでも
      //オーバーレイに置く
      GFL_OVERLAY_Load( FS_OVERLAY_ID(gds_comm) );
    }

    //ブラウザモードでも通信アイコンをだす
    if( GFL_NET_IsInit() )
    { 

      GFL_NET_ReloadIconTopOrBottom( FALSE, HEAPID_BATTLE_RECORDER_CORE );
    }
  }

  //ブラウザモードのみボリュームを下げる
  //バトルからの戻りのときはbr_main.cのBR_BATTLE_FreeParamで行っているので行わない
  if( p_wk->p_param->p_param->mode == BR_MODE_BROWSE )
  { 
    if( p_wk->p_param->mode != BR_CORE_MODE_RETURN )
    { 
      FIELD_SOUND *p_fld_snd  = GAMEDATA_GetFieldSound( p_wk->p_param->p_param->p_gamedata );
      FSND_HoldBGMVolume_inApp( p_fld_snd );
    }
  }
  else
  { 
    //それ以外のモードはGDSの曲をかける
    PMSND_PlayBGM( BR_SND_BGM_MAIN );
  }

  //戦闘から来た場合は、ALPHAをONにし、サイドバー状態を開いた状態にする
  if( p_wk->p_param->mode == BR_CORE_MODE_RETURN )
  { 
    BR_SIDEBAR_SetShakePos( p_wk->p_sidebar );
    BR_SIDEBAR_StartShake( p_wk->p_sidebar );
    BR_FADE_ALPHA_SetAlpha( p_wk->p_fade, BR_FADE_DISPLAY_BOTH, 0 );
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

  //ブラウザモードのみボリュームを元に戻す
  if( p_wk->p_param->p_param->mode == BR_MODE_BROWSE )
  { 
    FIELD_SOUND *p_fld_snd  = GAMEDATA_GetFieldSound( p_wk->p_param->p_param->p_gamedata );
    FSND_ReleaseBGMVolume_inApp( p_fld_snd );
  }

  //ネット破棄
  if( p_wk->p_net )
  { 
    if( BR_NET_IsLastDisConnectError( p_wk->p_net ) )
    { 
      p_wk->p_param->p_param->result  = BR_RESULT_NET_ERROR;
    }
    BR_NET_Exit( p_wk->p_net );
    p_wk->p_net = NULL;
    NAGI_Printf( "ネット切断\n" );
  }
  else
  { 
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( gds_comm ) );
  }

  //サイドバー破棄
  BR_SIDEBAR_Exit( p_wk->p_sidebar, p_wk->p_res );

  //フェードプロセス破棄
  BR_FADE_Exit( p_wk->p_fade );

	//コアプロセス破棄
	BR_PROC_SYS_Exit(	p_wk->p_procsys );


  //RES破棄前に色保存
  if( p_wk->p_param->p_param->mode == BR_MODE_BROWSE )
  { 
    MISC  *p_misc = GAMEDATA_GetMiscWork( p_wk->p_param->p_param->p_gamedata );
    MISC_SetBattleRecorderColor( p_misc, BR_RES_GetColor( p_wk->p_res ) );
  }

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
      //フェードプロセス処理
      BR_FADE_Main( p_wk->p_fade );

      //サイドバー処理
      BR_SIDEBAR_Main( p_wk->p_sidebar );

      //プロセス処理
      BR_PROC_SYS_Main( p_wk->p_procsys );

      //描画
      BR_GRAPHIC_2D_Draw( p_wk->p_graphic );

      //通信
      if( p_wk->p_net )
      { 
        BR_NET_Main( p_wk->p_net );
      }

      //終了
			if( BR_PROC_SYS_IsEnd( p_wk->p_procsys ) )
			{	
        *p_seq	= SEQ_FADEOUT;
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

  p_param->fade_type = BR_FADE_TYPE_ALPHA_BG012OBJ;
  p_param->p_result   = &p_wk->p_param->p_param->result;
  p_param->p_btn_recovery = &p_wk->p_param->p_data->btn_recovery;

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
        switch( cp_record_param->mode )
        {	
        case BR_RECODE_PROC_MY:
          p_param->menuID			= BR_BROWSE_MENUID_BTLVIDEO;
          break;
        case BR_RECODE_PROC_OTHER_00:
        case BR_RECODE_PROC_OTHER_01:
        case BR_RECODE_PROC_OTHER_02:
          p_param->menuID			= BR_BROWSE_MENUID_OTHER_RECORD;
          break;
        case BR_RECODE_PROC_DOWNLOAD_RANK:
          GF_ASSERT( 0 ); //ここはこない
          p_param->menuID			= BR_BTLVIDEO_MENUID_RANK;
          break;
        case BR_RECODE_PROC_DOWNLOAD_NUMBER:
          GF_ASSERT( 0 ); //ここはこない
          p_param->menuID			= BR_BTLVIDEO_MENUID_RANK;
          break;
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

    case BR_PROCID_MUSICAL_SEND:
      { 
        const BR_MUSICALSEND_PROC_PARAM *cp_musicalsend_param = cp_pre_param;
        if( cp_musicalsend_param->ret == BR_MUSICALSEND_RET_RETURN )
        { 
          p_param->fade_type = BR_FADE_TYPE_MASTERBRIGHT_AND_ALPHA;
        }
        p_param->menuID			= BR_MUSICAL_MENUID_TOP;
      }
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
  p_param->cp_saveinfo = &p_wk->p_param->p_data->rec_saveinfo;
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

  //「バトルビデオを送る」からの戻りは
  //はい、いいえではなく、その前に戻る
  {
    u16 stack_num = p_param->p_btn_recovery->stack_num - 1;
    if( p_param->p_btn_recovery->stack[ stack_num ].menuID == BR_MENUID_BVSEND_YESNO )
    { 
      p_param->p_btn_recovery->stack_num--;
    }
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
static void BR_RECORD_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{	
	BR_RECORD_PROC_PARAM			*p_param	= p_param_adrs;
	BR_CORE_WORK							*p_wk			= p_wk_adrs;

	//メニュー以外から来ない

  p_param->p_res			  = p_wk->p_res;
  p_param->p_fade       = p_wk->p_fade;
  p_param->p_procsys	  = p_wk->p_procsys;
  p_param->p_unit			  = BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_net        = p_wk->p_net;
  p_param->p_gamedata   = p_wk->p_param->p_param->p_gamedata;
  p_param->p_record     = &p_wk->p_param->p_data->record;
  p_param->is_return    = FALSE;
  p_param->cp_rec_saveinfo  = &p_wk->p_param->p_data->rec_saveinfo;
  p_param->cp_is_recplay_finish = &p_wk->p_param->p_data->is_recplay_finish;

  switch( preID )
  { 
  case BR_PROCID_MENU:
    { 
      const BR_MENU_PROC_PARAM	*cp_menu_param	= cp_pre_param;
      p_param->mode				= cp_menu_param->next_mode;
      p_param->p_outline  = NULL;

      p_wk->p_param->p_data->record_mode  = p_param->mode;

      //メモリへバトルビデオ読み込み
      {
        GAMEDATA  *p_gamedata = p_wk->p_param->p_param->p_gamedata;
        SAVE_CONTROL_WORK *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_gamedata );

        LOAD_RESULT result;
        BattleRec_Load( p_sv_ctrl, HEAPID_BATTLE_RECORDER_SYS, &result, p_param->mode ); 
        if( result == LOAD_RESULT_OK )
        { 
          if( p_param->mode == LOADDATA_MYREC )
          { 

            GDS_Profile_MyDataSet( BattleRec_GDSProfilePtrGet(), p_gamedata);
          }
          NAGI_Printf( "戦闘録画読み込み %d\n", p_param->mode );
        }
        else
        { 
          GF_ASSERT(0);
        }
      }
    }
    break;

  case BR_PROCID_BV_RANK:
    { 
      const BR_BVRANK_PROC_PARAM  *cp_rank_param  = cp_pre_param;
      p_param->mode       = BR_RECODE_PROC_DOWNLOAD_RANK;
      p_param->p_outline  = &p_wk->p_param->p_data->outline;

      p_wk->p_param->p_data->record_mode  = p_param->mode;
    }
    break;

  case BR_PROCID_CODEIN:
    { 
      const BR_CODEIN_PROC_PARAM  *cp_codein_param  = cp_pre_param;
      p_param->mode         = BR_RECODE_PROC_DOWNLOAD_NUMBER;
      p_param->video_number = cp_codein_param->video_number;
      p_param->p_outline    = NULL;

      p_wk->p_param->p_data->record_mode  = p_param->mode;
    }
    break;

  case BR_PROCID_BV_SAVE:
    //保存からの戻り
    //一旦BRS上に読み込むが、そのあとセーブチェックをするため
    //BRS上は外部セーブデータの可能性がある
    //だが、そのあと再度読み直しているのでBRS上に録画データがあるとみなしてよい
    { 
      const BR_BVSAVE_PROC_PARAM  *cp_bvsave_param  = cp_pre_param;
      p_param->mode				    = p_wk->p_param->p_data->record_mode;
      p_param->p_outline      = &p_wk->p_param->p_data->outline;
      p_param->video_number   = cp_bvsave_param->video_number;
      p_param->is_return      = TRUE;
    }
    break;
    
  case BR_PROC_SYS_RECOVERY_ID:
    //戦闘からの復帰時
    { 
      //戦闘から帰ってきたということは、録画データはBRS上にある
      p_param->mode				    = p_wk->p_param->p_data->record_mode;
      p_param->p_outline      = &p_wk->p_param->p_data->outline;
      p_param->is_return      = TRUE;
    }
    break;

  default:
    GF_ASSERT_MSG( 0, "メニューとランキング、コード入力、保存、復帰以外からは来ない %d", preID );
    break;
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
  SAVE_CONTROL_WORK *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_param->p_gamedata );

	//メニュー以外から来ない
	GF_ASSERT_MSG( preID == BR_PROCID_MENU, "メニュー以外からは来ない %d", preID );
	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_subway   = SaveControl_DataPtrGet( p_sv_ctrl, GMDATA_ID_BSUBWAY_SCOREDATA );
  p_param->p_gamedata = p_wk->p_param->p_param->p_gamedata;
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
  SAVE_CONTROL_WORK *p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_param->p_gamedata );

	//メニュー以外から来ない
	GF_ASSERT_MSG( preID == BR_PROCID_MENU, "メニュー以外からは来ない %d", preID );

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_gamedata = p_wk->p_param->p_param->p_gamedata;
  {
    BATTLEMATCH_DATA  *p_btlmatch_sv  = SaveData_GetBattleMatch( p_sv_ctrl );
    p_param->p_rndmatch = BATTLEMATCH_GetRndMatch( p_btlmatch_sv );
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

  p_param->p_outline      = &p_wk->p_param->p_data->outline;
  p_param->p_list_pos     = &p_wk->p_param->p_data->rank_pos;

  if( preID == BR_PROCID_MENU )
  { 
    const BR_MENU_PROC_PARAM  *cp_menu_param = cp_pre_param;
    p_param->mode       = cp_menu_param->next_mode;

    p_wk->p_param->p_data->rank_mode = p_param->mode;

    //カーソル位置はレコード画面とランク画面の間しか保持しない（厳密にはバトルの行き戻りも）ので、
    //他から来た場合クリアする
    GFL_STD_MemClear( p_param->p_list_pos, sizeof(BR_LIST_POS) );
  }
  else if( preID == BR_PROCID_BV_SEARCH )
  {
    const BR_BVSEARCH_PROC_PARAM  *cp_search_param = cp_pre_param;
    p_param->mode         = BR_BVRANK_MODE_SEARCH;
    p_param->search_data  = cp_search_param->search_data;
    
    p_wk->p_param->p_data->rank_mode = p_param->mode;

    //カーソル位置はレコード画面とランク画面の間しか保持しない（厳密にはバトルの行き戻りも）ので、
    //他から来た場合クリアする
    GFL_STD_MemClear( p_param->p_list_pos, sizeof(BR_LIST_POS) );
  }
  else if( preID == BR_PROCID_RECORD )
  {
    p_param->mode         = p_wk->p_param->p_data->rank_mode;
    p_param->is_return    = TRUE;
  }
  else
  { 
    GF_ASSERT_MSG( 0, "%d", preID );
  }

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_net      = p_wk->p_net;
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
	BR_BVSEARCH_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK					  	*p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->p_gamedata = p_wk->p_param->p_param->p_gamedata;
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
  p_param->p_net      = p_wk->p_net;
  p_param->p_gamedata = p_wk->p_param->p_param->p_gamedata;
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
static void BR_BVDELETE_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_BVDELETE_PROC_PARAM	*p_param	= p_param_adrs;
	BR_CORE_WORK						*p_wk			= p_wk_adrs;

  if( preID == BR_PROCID_MENU )
  { 
    const BR_MENU_PROC_PARAM	*cp_menu_param	= cp_pre_param;
    p_param->mode				= cp_menu_param->next_mode;
    p_param->p_res			= p_wk->p_res;
    p_param->p_fade     = p_wk->p_fade;
    p_param->p_procsys	= p_wk->p_procsys;
    p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
    p_param->p_gamedata = p_wk->p_param->p_param->p_gamedata;
    p_param->cp_rec_saveinfo = &p_wk->p_param->p_data->rec_saveinfo;
  }
  else
  { 
    GF_ASSERT(0);
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
static void BR_BVDELETE_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 
  BR_BVDELETE_PROC_PARAM	*p_param	= p_param_adrs;
	BR_CORE_WORK						*p_wk			= p_wk_adrs;

  //消していたら、セーブデータをリロードする
  if( p_param->is_delete )
  { 
    Br_Core_CheckSaveData( &p_wk->p_param->p_data->rec_saveinfo,
      FALSE, p_wk->p_param->p_param->p_gamedata, HEAPID_BATTLE_RECORDER_CORE );
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
static void BR_BVSAVE_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_BVSAVE_PROC_PARAM	    *p_param	= p_param_adrs;
	BR_CORE_WORK						  *p_wk			= p_wk_adrs;
  const BR_RECORD_PROC_PARAM	*cp_record_param	= cp_pre_param;

  GF_ASSERT( preID == BR_PROCID_RECORD );

  p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
  p_param->p_net      = p_wk->p_net;
  p_param->p_procsys	= p_wk->p_procsys;
  p_param->p_unit			= BR_GRAPHIC_GetClunit( p_wk->p_graphic );
  p_param->video_number = cp_record_param->video_number;
  p_param->p_gamedata = p_wk->p_param->p_param->p_gamedata;
  p_param->cp_rec_saveinfo = &p_wk->p_param->p_data->rec_saveinfo;
  p_param->is_secure  = cp_record_param->is_secure;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロック処理後関数
 *
 *	@param	void *p_param_adrs	引数ワーク
 *	@param	*p_wk_adrs					メインワーク
 */
//-----------------------------------------------------------------------------
static void BR_BVSAVE_PROC_AfterFunc( void *p_param_adrs, void *p_wk_adrs )
{ 
  BR_BVSAVE_PROC_PARAM	*p_param	= p_param_adrs;
	BR_CORE_WORK						*p_wk			= p_wk_adrs;

  //保存していたら、セーブデータをリロードする
  if( p_param->is_save )
  { 
    //セーブ状況をチェック
    Br_Core_CheckSaveData( &p_wk->p_param->p_data->rec_saveinfo,
      FALSE, p_wk->p_param->p_param->p_gamedata, HEAPID_BATTLE_RECORDER_CORE );

    //セーブ状況をチェックするとBRS上を上書いてしまうので、
    //セーブしたもので再度BTS上へ展開する
    {
      LOAD_RESULT result;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_param->p_gamedata );
      BattleRec_Load( p_sv, GFL_HEAP_LOWID( HEAPID_BATTLE_RECORDER_CORE ), &result, p_param->save_idx ); 
      if( result == LOAD_RESULT_OK )
      { 
      }
    }
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
static void BR_MUSICALLOOK_PROC_BeforeFunc( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID )
{ 
	BR_MUSICALLOOK_PROC_PARAM		*p_param	= p_param_adrs;
	BR_CORE_WORK						    *p_wk			= p_wk_adrs;

	p_param->p_res			= p_wk->p_res;
  p_param->p_fade     = p_wk->p_fade;
	p_param->p_procsys	= p_wk->p_procsys;
	p_param->p_graphic	= p_wk->p_graphic;
  p_param->p_net      = p_wk->p_net;
  p_param->p_sidebar  = p_wk->p_sidebar;
  p_param->p_gamedata = p_wk->p_param->p_param->p_gamedata;
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
  p_param->p_net      = p_wk->p_net;
  p_param->p_sidebar  = p_wk->p_sidebar;
  p_param->p_gamedata = p_wk->p_param->p_param->p_gamedata;
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
//=============================================================================
/**
 *    その他
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  外部セーブデータをチェック
 *	        （外部セーブデータをチェックするときはBRSを上書きしてしまうことに注意）
 *
 *	@param	BR_SAVE_INFO *p_saveinfo  セーブデータ状況ワーク
 *	@param  is_onece                  TRUEで一度チェックしていると行わないFALSEでいつでも行なう
 *	@param	*p_gamedata               ゲームデータ
 *	@param	heapID                    テンポラリヒープID
 */
//-----------------------------------------------------------------------------
static void Br_Core_CheckSaveData( BR_SAVE_INFO *p_saveinfo, BOOL is_onece, GAMEDATA *p_gamedata, HEAPID heapID )
{ 
  BOOL  is_check  = TRUE;

  if( is_onece )
  { 
    is_check  = !p_saveinfo->is_check;
  }

  if( is_check )
  { 
	  int i;
	  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
    GDS_PROFILE_PTR p_profile;
    LOAD_RESULT result;

    //もし一度チェックしていたら解放
    if( p_saveinfo->is_check )
    { 
      for( i = 0; i < BR_SAVEDATA_NUM; i++ )
      { 
        if( p_saveinfo->p_name[i] )
        { 
          GFL_STR_DeleteBuffer( p_saveinfo->p_name[i] );
          p_saveinfo->p_name[i] = NULL;
        }
      }
    }

    //今までの情報をクリア
    GFL_STD_MemClear( p_saveinfo, sizeof(BR_SAVE_INFO) );
	
	  //バトルビデオのセーブ状況を取得
	  for( i = 0; i < BR_SAVEDATA_NUM; i++ )
	  {	
	    BattleRec_Load( p_sv, GFL_HEAP_LOWID( heapID ), &result, i ); 
	    if( result == LOAD_RESULT_OK )
	    { 
	      NAGI_Printf( "戦闘録画読み込み %d\n", i );
	      p_profile = BattleRec_GDSProfilePtrGet();

        if( i == 0 )
        { 
          //自分の録画データは自分の情報を常に最新にしておくため設定する
          GDS_Profile_MyDataSet(p_profile, p_gamedata );
        }


        //設定
	      p_saveinfo->is_valid[i] = TRUE;

        //解放はbr_main.cで行っています
        p_saveinfo->p_name[i]   = GDS_Profile_CreateNameString( p_profile, HEAPID_BATTLE_RECORDER_SYS );
        DEBUG_STRBUF_Print( p_saveinfo->p_name[i] );

        //性別取得
	      p_saveinfo->sex[i]      = GDS_Profile_GetSex( p_profile );

        //ビデオ番号取得
        p_saveinfo->video_number[i]  = RecHeader_ParamGet( BattleRec_HeaderPtrGet(), RECHEAD_IDX_DATA_NUMBER, 0 );
	    }
	  }
	
	  //ミュージカルのセーブ状況を取得
	  {
	    MUSICAL_SAVE* p_musical = MUSICAL_SAVE_GetMusicalSave( GAMEDATA_GetSaveControlWork(p_gamedata) );
      p_saveinfo->is_musical_valid  = MUSICAL_SAVE_IsValidMusicalShotData( p_musical );
    }
	
	  //チェックフラグを立てる
	  p_saveinfo->is_check  = TRUE;
  }
}
