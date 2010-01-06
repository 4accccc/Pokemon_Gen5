//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_sys.c
 *	@brief  各プロセスのつなぎ
 *	@author	Toru=Nagihashi
 *	@data		2009.11.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID
#include "poke_tool/pokeparty.h"
#include "gamesystem/btl_setup.h"
#include "print/str_tool.h"
#include "net/network_define.h"
#include "system/net_err.h"
#include "poke_tool/poke_regulation.h"

//各プロセス
#include "battle/battle.h"
#include "net_app/wifi_login.h"

//セーブデータ
#include "savedata/battle_box_save.h"
#include "savedata/wifihistory.h"

//自分のモジュール
#include "wifibattlematch_core.h"
#include "wifibattlematch_subproc.h"
#include "wifibattlematch_data.h"

//外部公開
#include "net_app/wifibattlematch.h"

//-------------------------------------
///	PROCエクスターン
//=====================================

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define WIFIBATTLEMATCH_MEMBER_NUM  (2)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	サブプロック移動
//=====================================
typedef void *(*SUBPROC_ALLOC_FUNCTION)( HEAPID heapID, void *p_wk_adrs );
typedef BOOL (*SUBPROC_FREE_FUNCTION)( void *p_param, void *p_wk_adrs );
typedef struct 
{
	FSOverlayID							ov_id;
	const GFL_PROC_DATA			*cp_procdata;
	SUBPROC_ALLOC_FUNCTION	alloc_func;
	SUBPROC_FREE_FUNCTION		free_func;
} SUBPROC_DATA;
typedef struct {
	GFL_PROCSYS			*p_procsys;
	u32							seq;
	void						*p_proc_param;
	SUBPROC_DATA		*p_data;

	HEAPID					heapID;
	void						*p_wk_adrs;
	const SUBPROC_DATA			*cp_procdata_tbl;

	u32							next_procID;
	u32							now_procID;
} SUBPROC_WORK;
//-------------------------------------
///	システムワーク
//=====================================
typedef struct
{ 
  WIFIBATTLEMATCH_PARAM *p_param;
  SUBPROC_WORK          subproc;

  //以下システム層に置いておくデータ
  WIFIBATTLEMATCH_ENEMYDATA   *p_player_data;
  WIFIBATTLEMATCH_ENEMYDATA   *p_enemy_data;

  //コアモード
  WIFIBATTLEMATCH_CORE_MODE     core_mode;
  WIFIBATTLEMATCH_CORE_RETMODE  core_ret;

  //バトルの結果
  BtlResult                 btl_result;
  BtlRule                   btl_rule;

  //バトル用に選んだパーティ
  POKEPARTY                 *p_btl_party;

  u32 sub_seq;
} WIFIBATTLEMATCH_SYS;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	サブプロセス
//=====================================
static void SUBPROC_Init( SUBPROC_WORK *p_wk, const SUBPROC_DATA *cp_procdata_tbl, void *p_wk_adrs, HEAPID heapID );
static BOOL SUBPROC_Main( SUBPROC_WORK *p_wk );
static void SUBPROC_Exit( SUBPROC_WORK *p_wk );
static void SUBPROC_CallProc( SUBPROC_WORK *p_wk, u32 procID );

//-------------------------------------
///	サブプロセス用引数の解放、破棄関数
//=====================================
//バトルマッチ
static void *WBM_CORE_AllocParam( HEAPID heapID, void *p_wk_adrs );
static BOOL WBM_CORE_FreeParam( void *p_param_adrs, void *p_wk_adrs );
//リスト
static void *POKELIST_AllocParam( HEAPID heapID, void *p_wk_adrs );
static BOOL POKELIST_FreeParam( void *p_param_adrs, void *p_wk_adrs );
//バトル
static void *BATTLE_AllocParam( HEAPID heapID, void *p_wk_adrs );
static BOOL BATTLE_FreeParam( void *p_param_adrs, void *p_wk_adrs );
//WIFIログイン
static void *LOGIN_AllocParam( HEAPID heapID, void *p_wk_adrs );
static BOOL LOGIN_FreeParam( void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	データバッファ作成
//=====================================
static void DATA_CreateBuffer( WIFIBATTLEMATCH_SYS *p_wk, HEAPID heapID );
static void DATA_DeleteBuffer( WIFIBATTLEMATCH_SYS *p_wk );

//-------------------------------------
///	致命的なエラーをチェック
//=====================================
static void ERROR_CheckFatalMain( WIFIBATTLEMATCH_SYS *p_wk );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMaptch_ProcData =
{	
	WIFIBATTLEMATCH_PROC_Init,
	WIFIBATTLEMATCH_PROC_Main,
	WIFIBATTLEMATCH_PROC_Exit,
};

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	サブプロセス移動データ
//=====================================
typedef enum
{	
	SUBPROCID_CORE,
  SUBPROCID_POKELIST,
  SUBPROCID_BATTLE,
  SUBPROCID_LOGIN,

	SUBPROCID_MAX
} SUBPROC_ID;
static const SUBPROC_DATA sc_subproc_data[SUBPROCID_MAX]	=
{	
	//SUBPROCID_CORE
	{	
		FS_OVERLAY_ID( wifibattlematch_core ),
		&WifiBattleMaptchCore_ProcData,
		WBM_CORE_AllocParam,
		WBM_CORE_FreeParam,
	},
  //SUBPROCID_POKELIST,
  { 
	  NO_OVERLAY_ID,
    &WifiBattleMaptch_Sub_ProcData,
    POKELIST_AllocParam,
    POKELIST_FreeParam,
  },
  //SUBPROCID_BATTLE,
  { 
	  NO_OVERLAY_ID,
    &BtlProcData,
    BATTLE_AllocParam,
    BATTLE_FreeParam,
  },
  //SUBPROCID_LOGIN
  { 
    FS_OVERLAY_ID(wifi_login),
    &WiFiLogin_ProcData,
    LOGIN_AllocParam,
    LOGIN_FreeParam,
  },
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  WIFIBATTLEMATCH_SYS   *p_wk;

  
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH_SYS, 0x15000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_SYS), HEAPID_WIFIBATTLEMATCH_SYS );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_SYS) );
  p_wk->p_param   = p_param_adrs;
  OS_Printf( "ランダムマッチ引数 使用ポケ%d ルール%d\n", p_wk->p_param->poke, p_wk->p_param->btl_rule );

  p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_START;

  p_wk->p_btl_party = PokeParty_AllocPartyWork( HEAPID_WIFIBATTLEMATCH_SYS );

  //データバッファ作成
  DATA_CreateBuffer( p_wk, HEAPID_WIFIBATTLEMATCH_SYS );

  //自分のデータ設定( @todo レートとポケパーティはまだ )
  { 
    WIFIBATTLEMATCH_ENEMYDATA *p_player;

    p_player  = p_wk->p_player_data;
    {
      MYSTATUS  *p_my;
      p_my  = GAMEDATA_GetMyStatus(p_wk->p_param->p_game_data);
      STRTOOL_Copy( MyStatus_GetMyName(p_my), p_player->name, PERSON_NAME_SIZE+EOM_SIZE );
      p_player->sex           = MyStatus_GetMySex(p_my);
      p_player->trainer_view  = MyStatus_GetTrainerView(p_my);

    }
    { 
      WIFI_HISTORY *p_wifi_histroy;
      SAVE_CONTROL_WORK *p_sv;
      p_sv              = GAMEDATA_GetSaveControlWork(p_wk->p_param->p_game_data);
      p_wifi_histroy    = SaveData_GetWifiHistory(p_sv);
      p_player->nation  = WIFIHISTORY_GetMyNation(p_wifi_histroy); 
      p_player->area    = WIFIHISTORY_GetMyArea(p_wifi_histroy); 
    }
    {
      POKEPARTY *p_temoti;
      switch( p_wk->p_param->poke )
      {
      case WIFIBATTLEMATCH_POKE_TEMOTI:
        p_temoti = GAMEDATA_GetMyPokemon(p_wk->p_param->p_game_data);
        GFL_STD_MemCopy( p_temoti, p_player->pokeparty, PokeParty_GetWorkSize() );
        break;

      case WIFIBATTLEMATCH_POKE_BTLBOX:
        { 
          SAVE_CONTROL_WORK*	p_sv	= GAMEDATA_GetSaveControlWork(p_wk->p_param->p_game_data);
          BATTLE_BOX_SAVE *p_btlbox_sv = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
          p_temoti  = BATTLE_BOX_SAVE_MakePokeParty( p_btlbox_sv, GFL_HEAP_LOWID(HEAPID_WIFIBATTLEMATCH_SYS) );
          GFL_STD_MemCopy( p_temoti, p_player->pokeparty, PokeParty_GetWorkSize() );
          GFL_HEAP_FreeMemory( p_temoti );
        }
        break;
      }
    }
  }

  //モジュール作成
	SUBPROC_Init( &p_wk->subproc, sc_subproc_data, p_wk, HEAPID_WIFIBATTLEMATCH_SYS );
  SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_LOGIN );

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  WIFIBATTLEMATCH_SYS   *p_wk     = p_wk_adrs;

	//モジュール破棄
	SUBPROC_Exit( &p_wk->subproc );

  //データバッファ破棄
  DATA_DeleteBuffer( p_wk );

  //ワーク破棄
  GFL_HEAP_FreeMemory( p_wk->p_btl_party );

  //プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_SYS );

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
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
	enum
	{	
		WBM_SYS_SEQ_INIT,
		WBM_SYS_SEQ_FADEIN,
		WBM_SYS_SEQ_FADEIN_WAIT,
		WBM_SYS_SEQ_MAIN,
		WBM_SYS_SEQ_FADEOUT,
		WBM_SYS_SEQ_FADEOUT_WAIT,
		WBM_SYS_SEQ_EXIT,
	};

  WIFIBATTLEMATCH_SYS   *p_wk     = p_wk_adrs;

  //シーケンス
	switch( *p_seq )
	{	
	case WBM_SYS_SEQ_INIT:
		*p_seq	= WBM_SYS_SEQ_FADEIN;
		break;

	case WBM_SYS_SEQ_FADEIN:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= WBM_SYS_SEQ_FADEIN_WAIT;
		break;

	case WBM_SYS_SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= WBM_SYS_SEQ_MAIN;
		}
		break;

	case WBM_SYS_SEQ_MAIN:
		{
			BOOL is_end;
			is_end	= SUBPROC_Main( &p_wk->subproc );

			if( is_end )
			{	
				*p_seq	= WBM_SYS_SEQ_FADEOUT;
			}
		}
		break;

	case WBM_SYS_SEQ_FADEOUT:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= WBM_SYS_SEQ_FADEOUT_WAIT;
		break;

	case WBM_SYS_SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= WBM_SYS_SEQ_EXIT;
		}
		break;

	case WBM_SYS_SEQ_EXIT:
		return GFL_PROC_RES_FINISH;
	}

  //エラー検知
  ERROR_CheckFatalMain( p_wk );
	return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *			SUBPROCシステム
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SUBPROCシステム	初期化
 *
 *	@param	SUBPROC_WORK *p_wk	ワーク
 *	@param	cp_procdata_tbl			プロセス接続テーブル
 *	@param	void *p_wk_adrs			アロック関数と解放関数に渡すワーク
 *	@param	heapID							システム構築用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_Init( SUBPROC_WORK *p_wk, const SUBPROC_DATA *cp_procdata_tbl, void *p_wk_adrs, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(SUBPROC_WORK) );
	p_wk->p_procsys				= GFL_PROC_LOCAL_boot( heapID );
	p_wk->p_wk_adrs				= p_wk_adrs;
	p_wk->cp_procdata_tbl	= cp_procdata_tbl;
	p_wk->heapID					= heapID;
}

//----------------------------------------------------------------------------
/**
 *	@brief	SUBPROCシステム	メイン処理
 *
 *	@param	SUBPROC_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば終了	FALSEならばPROCが存在する
 */
//-----------------------------------------------------------------------------
static BOOL SUBPROC_Main( SUBPROC_WORK *p_wk )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_ALLOC_PARAM,
		SEQ_MAIN,
		SEQ_FREE_PARAM,
    SEQ_NEXT,
		SEQ_END,
	};

	switch( p_wk->seq )
	{	
	case SEQ_INIT:
		p_wk->now_procID	= p_wk->next_procID;
		p_wk->seq	= SEQ_ALLOC_PARAM;
		break;

	case SEQ_ALLOC_PARAM:
		//プロセス引数作成関数があれば呼び出し
		if( p_wk->cp_procdata_tbl[ p_wk->now_procID ].alloc_func )
		{	
			p_wk->p_proc_param	= p_wk->cp_procdata_tbl[ p_wk->now_procID ].alloc_func(
					p_wk->heapID, p_wk->p_wk_adrs );
		}
		else
		{	
			p_wk->p_proc_param	= NULL;
		}

		//プロック呼び出し
		GFL_PROC_LOCAL_CallProc( p_wk->p_procsys, p_wk->cp_procdata_tbl[	p_wk->now_procID ].ov_id,
					p_wk->cp_procdata_tbl[	p_wk->now_procID ].cp_procdata, p_wk->p_proc_param );

		p_wk->seq	= SEQ_MAIN;
		break;

	case SEQ_MAIN:
		{	
			GFL_PROC_MAIN_STATUS result;
			result	= GFL_PROC_LOCAL_Main( p_wk->p_procsys );
			if( GFL_PROC_MAIN_NULL == result )
			{	
				p_wk->seq	= SEQ_FREE_PARAM;
			}
		}
		break;

	case SEQ_FREE_PARAM:
		//プロセス引数破棄関数呼び出し
		if( p_wk->cp_procdata_tbl[	p_wk->now_procID ].free_func )
		{	
			if( p_wk->cp_procdata_tbl[	p_wk->now_procID ].free_func( p_wk->p_proc_param, p_wk->p_wk_adrs ) )
      { 
        p_wk->p_proc_param	= NULL;
				p_wk->seq	= SEQ_NEXT;
      }
		}
    else
    { 
				p_wk->seq	= SEQ_NEXT;
    }
    break;

  case SEQ_NEXT:
		//もし次のプロセスがあれば呼び出し
		//なければ終了
		if( p_wk->now_procID	!= p_wk->next_procID )
		{	
			p_wk->seq	= SEQ_INIT;
		}
		else
		{	
			p_wk->seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		return TRUE;
	}

	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SUBPROCシステム	破棄
 *
 *	@param	SUBPROC_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_Exit( SUBPROC_WORK *p_wk )
{	
	GF_ASSERT( p_wk->p_proc_param == NULL );

	GFL_PROC_LOCAL_Exit( p_wk->p_procsys );
	GFL_STD_MemClear( p_wk, sizeof(SUBPROC_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	SUBPROCシステム	プロセスリクエスト
 *
 *	@param	SUBPROC_WORK *p_wk	ワーク
 *	@param	proc_id							呼ぶプロセスID
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_CallProc( SUBPROC_WORK *p_wk, u32 procID )
{	
	p_wk->next_procID	= procID;
}

//=============================================================================
/**
 *		サブプロセス用作成破棄
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIマッチングコアの引数	作成
 *
 *	@param	HEAPID heapID			ヒープID
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return	引数
 */
//-----------------------------------------------------------------------------
static void *WBM_CORE_AllocParam( HEAPID heapID, void *p_wk_adrs )
{	

  WIFIBATTLEMATCH_CORE_PARAM  *p_param;
  WIFIBATTLEMATCH_SYS         *p_wk     = p_wk_adrs;

  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_CORE_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_CORE_PARAM) );
	p_param->p_param	      = p_wk->p_param;
  p_param->mode           = p_wk->core_mode;
  p_param->retmode        = p_wk->core_ret;
  p_param->btl_result     = p_wk->btl_result;
  p_param->btl_rule       = p_wk->btl_rule;
  p_param->p_player_data  = p_wk->p_player_data;
  p_param->p_enemy_data   = p_wk->p_enemy_data;
  p_param->p_rndmatch     = SaveData_GetRndMatch( GAMEDATA_GetSaveControlWork( p_wk->p_param->p_game_data ) );
		
	return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIマッチングコアの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL WBM_CORE_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{	
  WIFIBATTLEMATCH_SYS         *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_param_adrs;

  p_wk->core_ret = p_param->retmode;
  switch( p_param->result )
  { 
  case WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE:
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_POKELIST );
    break;
  }

	GFL_HEAP_FreeMemory( p_param );

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	リストの引数	作成
 *
 *	@param	HEAPID heapID			ヒープID
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return	引数
 */
//-----------------------------------------------------------------------------
static void *POKELIST_AllocParam( HEAPID heapID, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SUBPROC_PARAM    *p_param;
  WIFIBATTLEMATCH_SYS               *p_wk   = p_wk_adrs;
  int reg_no;

  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_SUBPROC_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_SUBPROC_PARAM) );
	

  switch( p_wk->p_param->btl_rule  )
  { 
  case BTL_RULE_SINGLE:
    reg_no  = REG_RND_SINGLE;
    break;
  case BTL_RULE_DOUBLE:
    reg_no  = REG_RND_DOUBLE;
    break;
  case BTL_RULE_TRIPLE:
    reg_no  = REG_RND_TRIPLE;
    break;
  case BTL_RULE_ROTATION:
    reg_no  = REG_RND_ROTATION;
    break;
  }

  p_param->regulation = (REGULATION*)PokeRegulation_LoadDataAlloc( reg_no, heapID );
  p_param->p_party    = p_wk->p_btl_party;
  p_param->gameData   = p_wk->p_param->p_game_data;

  //自分のデータ
  { 
    int i;
    WIFIBATTLEMATCH_ENEMYDATA *p_player;
    POKEPARTY *p_party;
    POKEMON_PARAM *pp;

    p_player = p_wk->p_player_data;
    p_party  = (POKEPARTY*)p_player->pokeparty;
    for( i = 0; i < PokeParty_GetPokeCount( p_party ); i++ )
    {
      pp = PokeParty_GetMemberPointer( p_party, i );
      p_param->ppp[i] = PP_GetPPPPointer( pp );
    }
  }

  //敵データ
  { 
    WIFIBATTLEMATCH_ENEMYDATA *p_enemy;
    POKEPARTY *p_party;

    p_enemy = p_wk->p_enemy_data;
    p_param->enemyName      = p_enemy->name;
    p_param->enemyPokeParty = (POKEPARTY*)p_enemy->pokeparty;
    p_param->enemySex       = p_enemy->sex;
  }

	return p_param;

}
//----------------------------------------------------------------------------
/**
 *	@brief  リストの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL POKELIST_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_SUBPROC_PARAM       *p_param  = p_param_adrs;

  switch( p_wk->sub_seq )
  { 
  case 0:
    GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),17);
    p_wk->sub_seq++;
    break;

  case 1:
    if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),17) )
    { 
      GFL_HEAP_FreeMemory( p_param->regulation );
      SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_BATTLE );
      GFL_HEAP_FreeMemory( p_param );
      p_wk->sub_seq = 0;
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルの引数	作成
 *
 *	@param	HEAPID heapID			ヒープID
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return	引数
 */
//-----------------------------------------------------------------------------
static void *BATTLE_AllocParam( HEAPID heapID, void *p_wk_adrs )
{ 
  BATTLE_SETUP_PARAM  *p_param;
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;

  GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );

  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(BATTLE_SETUP_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(BATTLE_SETUP_PARAM) );
		
  switch( p_wk->p_param->btl_rule )
  {
  case BTL_RULE_SINGLE:    ///< シングル
    BTL_SETUP_Single_Comm( p_param, p_wk->p_param->p_game_data, 
        GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, heapID );
    break;

  case BTL_RULE_DOUBLE:    ///< ダブル
    BTL_SETUP_Double_Comm( p_param, p_wk->p_param->p_game_data, 
        GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, heapID );
    break;

  case BTL_RULE_TRIPLE:    ///< トリプル
    BTL_SETUP_Triple_Comm( p_param, p_wk->p_param->p_game_data, 
        GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, heapID );
    break;

  case BTL_RULE_ROTATION:  ///< ローテーション
    BTL_SETUP_Rotation_Comm( p_param, p_wk->p_param->p_game_data, 
        GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, heapID );
    break;
  }

  BATTLE_PARAM_SetPokeParty( p_param, p_wk->p_btl_party, BTL_CLIENT_PLAYER ); 

  GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL);

	return p_param;

}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL BATTLE_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  BATTLE_SETUP_PARAM  *p_param  = p_param_adrs;

  //受け取り
  p_wk->btl_result  = p_param->result;
  p_wk->btl_rule  = p_param->rule;

  OS_FPrintf( 3, "バトル結果 %d \n", p_wk->btl_result);

  //破棄
  BATTLE_PARAM_Release( p_param );
	GFL_HEAP_FreeMemory( p_param );

  //次のPROC
  p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE;
  SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_CORE );


  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIログインの引数	作成
 *
 *	@param	HEAPID heapID			ヒープID
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return	引数
 */
//-----------------------------------------------------------------------------
static void *LOGIN_AllocParam( HEAPID heapID, void *p_wk_adrs )
{ 
  WIFILOGIN_PARAM *p_param;
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_PARAM) );

  p_param->gamedata = p_wk->p_param->p_game_data;

  return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ログインの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL LOGIN_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  WIFILOGIN_PARAM  *p_param     = p_param_adrs;
  WIFILOGIN_RESULT  result      = p_param->result;

  GFL_HEAP_FreeMemory( p_param );

  switch( result )
  { 
  case WIFILOGIN_RESULT_LOGIN:
    p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_START;
    SUBPROC_CallProc( &p_wk->subproc, SUBPROCID_CORE );
    break;

  case WIFILOGIN_RESULT_CANCEL:
    break;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データバッファ作成
 *
 *	@param	WIFIBATTLEMATCH_SYS *p_wk ワーク
 *	@param  heapID                    heapID
 */
//-----------------------------------------------------------------------------
static void DATA_CreateBuffer( WIFIBATTLEMATCH_SYS *p_wk, HEAPID heapID )
{ 
  p_wk->p_player_data = GFL_HEAP_AllocMemory( heapID, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
  p_wk->p_enemy_data  = GFL_HEAP_AllocMemory( heapID, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データバッファ破棄
 *
 *	@param	WIFIBATTLEMATCH_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DATA_DeleteBuffer( WIFIBATTLEMATCH_SYS *p_wk )
{ 
  GFL_HEAP_FreeMemory( p_wk->p_player_data );
  GFL_HEAP_FreeMemory( p_wk->p_enemy_data );
}
//=============================================================================
/**
 *  致命的なエラーをチェック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  致命的なエラーをチェック
 *
 *	@param	WIFIBATTLEMATCH_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void ERROR_CheckFatalMain( WIFIBATTLEMATCH_SYS *p_wk )
{ 
  BOOL is_check  = NetErr_App_CheckError();

  if( is_check )
  { 
  }
}
