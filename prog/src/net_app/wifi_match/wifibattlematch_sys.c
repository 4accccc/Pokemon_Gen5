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
#include "poke_tool/poke_regulation.h"
#include "sound/pm_sndsys.h"
#include "system/net_err.h"
#include "net/dreamworld_netdata.h"

//各プロセス
#include "net_app/wifi_login.h"
#include "net_app/wifi_logout.h"
#include "title/title.h"
#include "wifibattlematch_battle.h"
#include "net_app/btl_rec_sel.h"
#include "wifibattlematch_utilproc.h"
#include "wifibattlematch_subproc.h"
#include "battle_championship_core.h"

//セーブデータ
#include "savedata/battle_box_save.h"
#include "savedata/wifihistory.h"
#include "savedata/battlematch_savedata.h"
#include "savedata/my_pms_data.h"

//自分のモジュール
#include "wifibattlematch_core.h"
#include "wifibattlematch_util.h"
#include "wifibattlematch_data.h"
#include "wifibattlematch_snd.h"
#include "wifibattlematch_net.h"

//外部公開
#include "net_app/wifibattlematch.h"

//デバッグ
#include "debug/debug_nagihashi.h"
#include "wifibattlematch_debugdata.h"

//-------------------------------------
///	DEBUG
//=====================================
#ifdef PM_DEBUG

#if defined(DEBUG_ONLY_FOR_toru_nagihashi) || defined(DEBUG_ONLY_FOR_shimoyamada)
#define WBM_SYS_PRINT_ON
#endif

#ifdef WIFIMATCH_RATE_AUTO
#define WBM_SYS_BATTLE_VSTIME_1
#endif //WIFIMATCH_RATE_AUTO

#endif //PM_DEBUG

#ifdef WBM_SYS_PRINT_ON
#define WBM_SYS_Printf(...)  OS_TFPrintf( 3, __VA_ARGS__ )
#else
#define WBM_SYS_Printf(...) /*  */
#endif

//-------------------------------------
///	PROCエクスターン
//=====================================
FS_EXTERN_OVERLAY( wifibattlematch_proc );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//SAKEサーバー上には80バイトしかとっていない
SDK_COMPILER_ASSERT( sizeof(WIFIBATTLEMATCH_RECORD_DATA) == 80 );

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef struct _WBM_SYS_SUBPROC_WORK WBM_SYS_SUBPROC_WORK;


//-------------------------------------
///	システムワーク
//=====================================
typedef struct
{ 
  BOOL                        is_create_gamedata;
  WIFIBATTLEMATCH_PARAM       param;

  //プロセス管理システム
  WBM_SYS_SUBPROC_WORK        *p_subproc;

  //大会モード
  WIFIBATTLEMATCH_TYPE        type;

  //以下PROCのつなぎのために記憶しておくもの
  WIFIBATTLEMATCH_CORE_MODE   core_mode;
  BOOL  is_wificup_end;

  //以下システム層に置いておくデータ
  WIFIBATTLEMATCH_ENEMYDATA   *p_player_data; //自分の情報
  WIFIBATTLEMATCH_ENEMYDATA   *p_enemy_data;  //相手の情報
  POKEPARTY                   *p_player_btl_party;//自分で決めたパーティ
  POKEPARTY                   *p_enemy_btl_party; //相手の決めたパーティ
  POKEPARTY                   *p_player_modify_party;//レベル補正をかけた自分のパーティ
  POKEPARTY                   *p_enemy_modify_party; //レベル補正をかけた相手のパーティ
  DWCSvlResult                svl_result;         //WIFIログイン時に得るサービスロケータ
  BATTLEMATCH_BATTLE_SCORE    btl_score;          //バトルの成績
  u32                         server_time;        //サーバアクセス時間
  WIFIBATTLEMATCH_RECORD_DATA record_data;        //戦績
  BOOL                        is_err_return_login;//WIFILOGINにエラーで戻るとき
  DREAM_WORLD_SERVER_WORLDBATTLE_STATE_DATA gpf_data;//GPFサーバーから落としてきた選手証データ
  WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA   wifi_sake_data; //WIFI用サケデータ
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA    rnd_sake_data;  //ランダムマッチ用サケデータ
  WIFIBATTLEMATCH_NET_DATA    net_data;   //通信が終了しても残しておくデータ
  WIFIBATTLEMATCH_RECV_DATA   recv_data;  //サーバーから落としてきたデータ
  BOOL                        is_dirty_name;  //対戦相手の名前が不正かどうか
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
///	サブプロセス用引数の解放、破棄関数
//=====================================
//メインメニュー＋LIVE用フロープロセス
static void *BC_CORE_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc, HEAPID heapID, void *p_wk_adrs );
static BOOL BC_CORE_FreeParam(  WBM_SYS_SUBPROC_WORK *p_subproc, void *p_param_adrs, void *p_wk_adrs );
//WIFIフローメインプロセス
static void *WBM_CORE_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
static BOOL WBM_CORE_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs );
//リスト＋ステータス
static void *POKELIST_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
static BOOL POKELIST_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs );
//バトル＋デモ
static void *BATTLE_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
static BOOL BATTLE_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs );
//WIFIログイン
static void *LOGIN_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
static BOOL LOGIN_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs );
//リスト＋ステータス～バトル＋デモへのつなぎプロセス
static void *WBM_LISTAFTER_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
static BOOL WBM_LISTAFTER_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs );
//録画
static void *WBM_BTLREC_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
static BOOL WBM_BTLREC_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs );
//WIFIログアウト
static void *LOGOUT_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
static BOOL LOGOUT_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs );
//録画再生
static void *RECPLAY_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
static BOOL RECPLAY_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	データバッファ作成
//=====================================
static void DATA_CreateBuffer( WIFIBATTLEMATCH_SYS *p_wk, HEAPID heapID );
static void DATA_DeleteBuffer( WIFIBATTLEMATCH_SYS *p_wk );

//その他
static void Util_SetRecordData( WIFIBATTLEMATCH_RECORD_DATA *p_data, const POKEPARTY *cp_my_poke, const POKEPARTY *cp_you_poke, const WIFIBATTLEMATCH_ENEMYDATA *cp_you_data, const REGULATION *cp_reg, u32 cupNO, const BATTLE_SETUP_PARAM *cp_btl_param, const BATTLEMATCH_BATTLE_SCORE *cp_btl_score, WIFIBATTLEMATCH_TYPE type );

static void Util_GetRestPoke( const BATTLE_SETUP_PARAM *cp_btl_param, u8 *p_my_poke, u8 *p_you_poke );
static void Util_GetRestHp( const BATTLE_SETUP_PARAM *cp_btl_param, u8 *p_my_poke, u8 *p_you_poke );

//=============================================================================
/**
 *				  サブプロセス
 *				    ・プロセスを行き来するシステム
 */
//=============================================================================
//-------------------------------------
///	サブプロセス初期化・解放関数コールバック
//=====================================
typedef void *(*WBM_SYS_SUBPROC_ALLOC_FUNCTION)( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs );
typedef BOOL (*WBM_SYS_SUBPROC_FREE_FUNCTION)( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param, void *p_wk_adrs );

//-------------------------------------
///	サブプロセス設定構造体
//=====================================
typedef struct 
{
	FSOverlayID							    ov_id;
	const GFL_PROC_DATA			    *cp_procdata;
	WBM_SYS_SUBPROC_ALLOC_FUNCTION	alloc_func;
	WBM_SYS_SUBPROC_FREE_FUNCTION		free_func;
} WBM_SYS_SUBPROC_DATA;

//-------------------------------------
///	パブリック
//=====================================
static  WBM_SYS_SUBPROC_WORK * WBM_SYS_SUBPROC_Init( const WBM_SYS_SUBPROC_DATA *cp_procdata_tbl, u32 tbl_len, void *p_wk_adrs, HEAPID heapID );
static void WBM_SYS_SUBPROC_Exit( WBM_SYS_SUBPROC_WORK *p_wk );
static BOOL WBM_SYS_SUBPROC_Main( WBM_SYS_SUBPROC_WORK *p_wk );
static void WBM_SYS_SUBPROC_CallProc( WBM_SYS_SUBPROC_WORK *p_wk, u32 procID );
static void WBM_SYS_SUBPROC_End( WBM_SYS_SUBPROC_WORK *p_wk );
static GFL_PROC_MAIN_STATUS WBM_SYS_SUBPROC_GetStatus( const WBM_SYS_SUBPROC_WORK *cp_wk );
static u8 WBM_SYS_SUBPROC_GetPreProcID( const WBM_SYS_SUBPROC_WORK *cp_wk );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMatch_ProcData =
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
enum
{	
	SUBPROCID_MAINMENU,
	SUBPROCID_CORE,
  SUBPROCID_POKELIST,
  SUBPROCID_BATTLE,
  SUBPROCID_LOGIN,
  SUBPROCID_LISTAFTER,
  SUBPROCID_BTLREC,
  SUBPROCID_LOGOUT,
  SUBPROCID_RECPLAY,

	SUBPROCID_MAX
};
static const WBM_SYS_SUBPROC_DATA sc_subproc_data[SUBPROCID_MAX]	=
{	
	//SUBPROCID_MAINMENU,
  { 
		FS_OVERLAY_ID( battle_championship ),
		&BATTLE_CHAMPIONSHIP_CORE_ProcData,
		BC_CORE_AllocParam,
		BC_CORE_FreeParam,
  },
	//SUBPROCID_CORE
	{	
		FS_OVERLAY_ID( wifibattlematch_core ),
		&WifiBattleMatchCore_ProcData,
		WBM_CORE_AllocParam,
		WBM_CORE_FreeParam,
	},
  //SUBPROCID_POKELIST,
  { 
	  FS_OVERLAY_ID( wifibattlematch_proc ),
    &WifiBattleMatch_Sub_ProcData,
    POKELIST_AllocParam,
    POKELIST_FreeParam,
  },
  //SUBPROCID_BATTLE,
  { 
	  NO_OVERLAY_ID,
    &WifiBattleMatch_BattleLink_ProcData,
    BATTLE_AllocParam,
    BATTLE_FreeParam,
  },
  //SUBPROCID_LOGIN
  { 
    FS_OVERLAY_ID( wifi_login ),
    &WiFiLogin_ProcData,
    LOGIN_AllocParam,
    LOGIN_FreeParam,
  },
  //SUBPROCID_LISTAFTER,
  { 
		FS_OVERLAY_ID( wifibattlematch_proc ),
		&WifiBattleMatch_ListAfter_ProcData,
		WBM_LISTAFTER_AllocParam,
		WBM_LISTAFTER_FreeParam,
  },
  //SUBPROCID_BTLREC
  { 
    FS_OVERLAY_ID( btl_rec_sel ),
    &BTL_REC_SEL_ProcData,
    WBM_BTLREC_AllocParam,
    WBM_BTLREC_FreeParam,
  },
  //SUBPROCID_LOGOUT
  { 
    FS_OVERLAY_ID( wifi_login ),
    &WiFiLogout_ProcData,
    LOGOUT_AllocParam,
    LOGOUT_FreeParam,
  },
  //SUBPROCID_RECPLAY,
  { 
    NO_OVERLAY_ID,
    &BtlProcData,
    RECPLAY_AllocParam,
    RECPLAY_FreeParam,
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
  HEAPID  parentID;

  { 
    WIFIBATTLEMATCH_PARAM *p_param  = p_param_adrs;
    switch( p_param->mode )
    { 
    case WIFIBATTLEMATCH_MODE_RANDOM:
      //ランダムマッチはポケセンWIFIカウンターから入り、
      //ゲームシステム等でメモリを食っているので、HEAPID_PROCにシステムをおく
      //parentID  = HEAPID_PROC;

      //PROCはさまざまなイベントが乗る可能性があるのでAPPから貰うことにしました
      parentID  = GFL_HEAPID_APP;
      break;

    case WIFIBATTLEMATCH_MODE_MAINMENU:
      //それ以外は、タイトル画面からくるためHEAPID_APPが潤沢にあるので、
      //HEAPID_APPからもらう
      parentID  = GFL_HEAPID_APP;
      break;
    default:
      GF_ASSERT(0);
    }
  }
  
  NAGI_Printf( "work %d + %d *2\n", sizeof(WIFIBATTLEMATCH_SYS), sizeof(WIFIBATTLEMATCH_ENEMYDATA) );

	//ヒープ作成
	GFL_HEAP_CreateHeap( parentID, HEAPID_WIFIBATTLEMATCH_SYS, 0x8000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_SYS), HEAPID_WIFIBATTLEMATCH_SYS );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_SYS) );
  GFL_STD_MemCopy( p_param_adrs, &p_wk->param, sizeof(WIFIBATTLEMATCH_PARAM) );
  p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_START;

  //ゲームデータ自動生成のときは作成
  if( p_wk->param.p_game_data == NULL )
  { 
    p_wk->param.p_game_data = GAMEDATA_Create( GFL_HEAPID_APP );
    p_wk->is_create_gamedata  = TRUE;
  }

  //戦闘用パーティ作成
  p_wk->p_player_btl_party = PokeParty_AllocPartyWork( HEAPID_WIFIBATTLEMATCH_SYS );
  p_wk->p_enemy_btl_party = PokeParty_AllocPartyWork( HEAPID_WIFIBATTLEMATCH_SYS );
  p_wk->p_player_modify_party = PokeParty_AllocPartyWork( HEAPID_WIFIBATTLEMATCH_SYS );
  p_wk->p_enemy_modify_party = PokeParty_AllocPartyWork( HEAPID_WIFIBATTLEMATCH_SYS );

  WBM_SYS_Printf( "ランダムマッチ引数 使用ポケ%d ルール%d\n", p_wk->param.poke, p_wk->param.btl_rule );

  //データバッファ作成
  BattleRec_Init( HEAPID_WIFIBATTLEMATCH_SYS );
  DATA_CreateBuffer( p_wk, HEAPID_WIFIBATTLEMATCH_SYS );

  //モジュール作成
	p_wk->p_subproc = WBM_SYS_SUBPROC_Init( sc_subproc_data, SUBPROCID_MAX, p_wk, HEAPID_WIFIBATTLEMATCH_SYS );

  //開始モード
  switch( p_wk->param.mode )
  { 
  case WIFIBATTLEMATCH_MODE_MAINMENU:  //タイトルから進む、メインメニュー
    p_wk->type  = WIFIBATTLEMATCH_TYPE_WIFICUP; //まだ決まっていないが、設定しないと０のままなのでいれておく
    WBM_SYS_SUBPROC_CallProc( p_wk->p_subproc, SUBPROCID_MAINMENU );
    break;
  case WIFIBATTLEMATCH_MODE_RANDOM:    //ポケセンのWIFIカウンターからすすむ、ランダム対戦
    p_wk->type  = WIFIBATTLEMATCH_TYPE_RNDRATE; //まだ決まっていないが、設定しないと０のままなのでいれておく
    WBM_SYS_SUBPROC_CallProc( p_wk->p_subproc, SUBPROCID_LOGIN );
    break;
  default:
    GF_ASSERT( 0 );
  }

  //自分のデータ初期化
  {
    POKEPARTY *p_temoti;
    WIFIBATTLEMATCH_ENEMYDATA *p_my_data  = p_wk->p_player_data;
    switch( p_wk->param.poke )
    {
    case WIFIBATTLEMATCH_POKE_TEMOTI:
      p_temoti = GAMEDATA_GetMyPokemon(p_wk->param.p_game_data);
      GFL_STD_MemCopy( p_temoti, p_my_data->pokeparty, PokeParty_GetWorkSize() );
      break;

    case WIFIBATTLEMATCH_POKE_BTLBOX:
      { 
        SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(p_wk->param.p_game_data);
        BATTLE_BOX_SAVE *p_btlbox_sv = BATTLE_BOX_SAVE_GetBattleBoxSave( p_sv );
        p_temoti  = BATTLE_BOX_SAVE_MakePokeParty( p_btlbox_sv, GFL_HEAP_LOWID(HEAPID_WIFIBATTLEMATCH_SYS) );
        GFL_STD_MemCopy( p_temoti, p_my_data->pokeparty, PokeParty_GetWorkSize() );
        GFL_HEAP_FreeMemory( p_temoti );
      }
      break;
    }
  }

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
	WBM_SYS_SUBPROC_Exit( p_wk->p_subproc );

  //データバッファ破棄
  DATA_DeleteBuffer( p_wk );
  BattleRec_Exit();

  //パーティの破棄
  GFL_HEAP_FreeMemory( p_wk->p_enemy_modify_party );
  GFL_HEAP_FreeMemory( p_wk->p_player_modify_party );
  GFL_HEAP_FreeMemory( p_wk->p_enemy_btl_party );
  GFL_HEAP_FreeMemory( p_wk->p_player_btl_party );

  //ゲームデータを自分で作成していたら破棄
  if( p_wk->is_create_gamedata )
  { 
    GAMEDATA_Delete( p_wk->param.p_game_data );
  }

  //ランダムマッチ以外はタイトルへ戻る
  if( p_wk->param.mode != WIFIBATTLEMATCH_MODE_RANDOM )
  { 
    //GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
    OS_ResetSystem(0);
  }

  //引数自動破棄フラグを設定されていたら破棄
  if( p_wk->param.is_auto_release )
  { 
    GFL_HEAP_FreeMemory( p_param_adrs );
  }

  //プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH_SYS );

  PMSND_StopBGM();

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
		WBM_SYS_SEQ_MAIN,
		WBM_SYS_SEQ_EXIT,
	};

  WIFIBATTLEMATCH_SYS   *p_wk     = p_wk_adrs;

  //DEBUG_HEAP_PrintRestUse( GFL_HEAPID_APP );

  //シーケンス
	switch( *p_seq )
	{	
	case WBM_SYS_SEQ_MAIN:
		{
      //サブプロックのループ
			if( WBM_SYS_SUBPROC_Main( p_wk->p_subproc ) )
			{	
				*p_seq	= WBM_SYS_SEQ_EXIT;
			}

      //SAKEサーバーへのアクセス時間をカウンダウン
      if( p_wk->server_time > 0 )
      { 
        p_wk->server_time--;
      }

      //gamesystem_main上のLOCAL_PROCで動くものしか検知してくれないので、
      //自分で動かす必要がある
      { 
        const GFL_PROC_MAIN_STATUS  status  = WBM_SYS_SUBPROC_GetStatus( p_wk->p_subproc );
        if( status == GFL_PROC_MAIN_CHANGE || status == GFL_PROC_MAIN_NULL )
        { 
          NetErr_DispCall(FALSE);
        }
      }
		}
		break;

	case WBM_SYS_SEQ_EXIT:
		return GFL_PROC_RES_FINISH;
	}
	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *		サブプロセス用作成破棄
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	メインメニュー＋LIVE用フローコアの引数	作成
 *
 *	@param	HEAPID heapID			ヒープID
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return	引数
 */
//-----------------------------------------------------------------------------
static void *BC_CORE_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{	
  BATTLE_CHAMPIONSHIP_CORE_PARAM  *p_param;
  WIFIBATTLEMATCH_SYS             *p_wk     = p_wk_adrs;

  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(BATTLE_CHAMPIONSHIP_CORE_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(BATTLE_CHAMPIONSHIP_CORE_PARAM) );
  p_param->p_gamedata     = p_wk->param.p_game_data;
  p_param->p_player_data  = p_wk->p_player_data;
  p_param->p_enemy_data   = p_wk->p_enemy_data;
  p_param->p_btl_score    = &p_wk->btl_score;

  switch( WBM_SYS_SUBPROC_GetPreProcID( p_subproc ) )
  { 
  case SUBPROCID_LOGIN:
  case SUBPROCID_LOGOUT:
    //ログイン、ログアウトから着ていたら、WIFI大会（ランダムマッチはここへこないので）
    p_param->mode       = BATTLE_CHAMPIONSHIP_CORE_MODE_WIFI_MENU;
    break;

  case SUBPROCID_RECPLAY:
    //再生から着ていたら、LIVE大会メインメニュー
    p_param->mode       = BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_MENU;
    break;

  case SUBPROCID_BATTLE:
    //バトルからきていたら、LIVE大会バトル後処理
    p_param->mode       = BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_BTLEND;
    break;

  case SUBPROCID_BTLREC:
    //録画からきていたら、LIVE大会録画後処理
    p_param->mode       = BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_RECEND;
    break;

  case 0:
    //初期化値0ならば一番最初にここへきた＝メインメニュー
    p_param->mode       = BATTLE_CHAMPIONSHIP_CORE_MODE_MAIN_MEMU;
    break;

  default:
    if( p_wk->btl_score.is_error )
    { 
      //LIVE大会用エラー
      p_param->mode       = BATTLE_CHAMPIONSHIP_CORE_MODE_LIVE_FLOW_MENU;
    }
    else
    { 
      GF_ASSERT(0);
    }
    break;
  }

  if( PMSND_GetBGMsoundNo() != WBM_SND_SEQ_MAIN )
  { 
    PMSND_PlayBGM( WBM_SND_SEQ_MAIN );
  }
		
	return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	メインメニュー＋LIVE用フローコアの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL BC_CORE_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{	
  WIFIBATTLEMATCH_SYS             *p_wk     = p_wk_adrs;
  BATTLE_CHAMPIONSHIP_CORE_PARAM  *p_param  = p_param_adrs;

  switch( p_param->ret )
  { 
  case BATTLE_CHAMPIONSHIP_CORE_RET_TITLE:   //タイトルへ行く
    WBM_SYS_SUBPROC_End( p_subproc );
    break;
  case BATTLE_CHAMPIONSHIP_CORE_RET_WIFICUP: //WIFI大会へ行く
    p_wk->type  = WIFIBATTLEMATCH_TYPE_WIFICUP;
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_LOGIN );
    break;
  case BATTLE_CHAMPIONSHIP_CORE_RET_LIVEBTL: //LIVE用バトルへ行く
    p_wk->type  = WIFIBATTLEMATCH_TYPE_LIVECUP;
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_POKELIST );
    break;
  case BATTLE_CHAMPIONSHIP_CORE_RET_LIVEREC: //LIVE用録画へ行く
    p_wk->type  = WIFIBATTLEMATCH_TYPE_LIVECUP;
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_BTLREC );
    break;
  case BATTLE_CHAMPIONSHIP_CORE_RET_LIVERECPLAY: //LIVE用録画再生へ行く
    p_wk->type  = WIFIBATTLEMATCH_TYPE_LIVECUP;
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_RECPLAY );
    break;
  default:
    GF_ASSERT( 0 );
  }

	GFL_HEAP_FreeMemory( p_param );

  return TRUE;
}
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
static void *WBM_CORE_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{	

  WIFIBATTLEMATCH_CORE_PARAM  *p_param;
  WIFIBATTLEMATCH_SYS         *p_wk     = p_wk_adrs;

  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_CORE_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_CORE_PARAM) );
	p_param->p_param	      = &p_wk->param;
  p_param->mode           = p_wk->core_mode;
  p_param->retmode        = p_wk->type - WIFIBATTLEMATCH_TYPE_RNDRATE;
  p_param->p_player_data  = p_wk->p_player_data;
  p_param->p_enemy_data   = p_wk->p_enemy_data;
  p_param->p_svl_result   = &p_wk->svl_result;
  p_param->p_server_time  = &p_wk->server_time;
  p_param->p_record_data  = &p_wk->record_data;
  p_param->p_net_data     = &p_wk->net_data;
  p_param->p_recv_data    = &p_wk->recv_data;
  p_param->cp_btl_score   = &p_wk->btl_score;
  p_param->p_gpf_data     = &p_wk->gpf_data;
  p_param->p_wifi_sake_data   = &p_wk->wifi_sake_data;
  p_param->p_rnd_sake_data    = &p_wk->rnd_sake_data;
  p_param->p_is_dirty_name    = &p_wk->is_dirty_name;
  { 
    BATTLEMATCH_DATA  *p_btlmatch_sv  = SaveData_GetBattleMatch( GAMEDATA_GetSaveControlWork( p_wk->param.p_game_data ) );
    p_param->p_rndmatch     =  BATTLEMATCH_GetRndMatch( p_btlmatch_sv );
  }

  if( PMSND_GetBGMsoundNo() != WBM_SND_SEQ_MAIN )
  { 
    PMSND_PlayBGM( WBM_SND_SEQ_MAIN );
  }
		
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
static BOOL WBM_CORE_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{	
  WIFIBATTLEMATCH_SYS         *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_CORE_PARAM  *p_param  = p_param_adrs;

  //大会モード決定
  switch( p_param->retmode )
  { 
  case WIFIBATTLEMATCH_CORE_RETMODE_RATE:  //レーティングモード
    p_wk->type  = WIFIBATTLEMATCH_TYPE_RNDRATE;
    break;
  case WIFIBATTLEMATCH_CORE_RETMODE_FREE:  //フリーモード
    p_wk->type  = WIFIBATTLEMATCH_TYPE_RNDFREE;
    break;
  case WIFIBATTLEMATCH_CORE_RETMODE_WIFI: //WIFI大会
    break;
  case WIFIBATTLEMATCH_CORE_RETMODE_NONE:  //なにも選ばずに終了
    p_wk->type  = WIFIBATTLEMATCH_TYPE_RNDFREE;
    break;
  }

  //次への行き先
  switch( p_param->result )
  { 
  case WIFIBATTLEMATCH_CORE_RESULT_NEXT_BATTLE:
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_POKELIST );
    break;
    
  case WIFIBATTLEMATCH_CORE_RESULT_NEXT_REC:
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_BTLREC );
    break;
  
  case WIFIBATTLEMATCH_CORE_RESULT_ERR_NEXT_LOGIN:
    p_wk->is_err_return_login = TRUE;
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_LOGIN );
    break;
  case WIFIBATTLEMATCH_CORE_RESULT_END_WIFICUP:
    p_wk->is_wificup_end  = TRUE;
    /* fallthor */
  case WIFIBATTLEMATCH_CORE_RESULT_FINISH:
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_LOGOUT );
    break;

  default:
    GF_ASSERT( 0 );
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
static void *POKELIST_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SUBPROC_PARAM    *p_param;
  WIFIBATTLEMATCH_SYS               *p_wk   = p_wk_adrs;
  int reg_no;

  //ポケパーティの受け取りは内部でADDしてるだけなので、
  //毎回ここで初期化する
  PokeParty_InitWork( p_wk->p_player_btl_party );
  PokeParty_InitWork( p_wk->p_enemy_btl_party );


  //引数作成
  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_SUBPROC_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_SUBPROC_PARAM) );

  if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDRATE
      || p_wk->type == WIFIBATTLEMATCH_TYPE_RNDFREE )
  { 
    //ランダムマッチは規定のレギュレーションを使う

    switch( p_wk->param.btl_rule  )
    { 
    case WIFIBATTLEMATCH_BTLRULE_SINGLE:
      reg_no  = REG_RND_SINGLE;
      break;
    case WIFIBATTLEMATCH_BTLRULE_DOUBLE:
      reg_no  = REG_RND_DOUBLE;
      break;
    case WIFIBATTLEMATCH_BTLRULE_TRIPLE:
      reg_no  = REG_RND_TRIPLE;
      break;
    case WIFIBATTLEMATCH_BTLRULE_ROTATE:
      reg_no  = REG_RND_ROTATION;
      break;
    case WIFIBATTLEMATCH_BTLRULE_SHOOTER:
      reg_no  = REG_RND_TRIPLE_SHOOTER;
      break;
    default:
      GF_ASSERT(0);
    }

    p_param->regulation = (REGULATION*)PokeRegulation_LoadDataAlloc( reg_no, heapID );
  }
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_WIFICUP )
  { 
    //WIFI大会はWIFI大会用レギュレーションを使う

    SAVE_CONTROL_WORK *p_sv       = GAMEDATA_GetSaveControlWork( p_wk->param.p_game_data );
    REGULATION_SAVEDATA *p_reg_sv = SaveData_GetRegulationSaveData( p_sv );
    REGULATION_CARDDATA *p_reg    = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_WIFI );

    p_param->regulation       = RegulationData_GetRegulation( p_reg );
  }
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  { 
    //LIVE大会はLIVE大会用レギュレーションを使う

    SAVE_CONTROL_WORK *p_sv       = GAMEDATA_GetSaveControlWork( p_wk->param.p_game_data );
    REGULATION_SAVEDATA *p_reg_sv = SaveData_GetRegulationSaveData( p_sv );
    REGULATION_CARDDATA *p_reg    = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_LIVE );

    p_param->regulation       = RegulationData_GetRegulation( p_reg );
  }
  else
  { 
    GF_ASSERT(0);
  }

  p_param->p_select_party   = p_wk->p_player_btl_party;
  p_param->gameData         = p_wk->param.p_game_data;

  //自分のデータ
  { 
    int i;
    WIFIBATTLEMATCH_ENEMYDATA *p_player;
    POKEPARTY *p_party;
    POKEMON_PARAM *pp;
    p_player = p_wk->p_player_data;

    PokeParty_Copy((POKEPARTY*)p_player->pokeparty, p_wk->p_player_modify_party);
    PokeRegulation_ModifyLevelPokeParty( p_param->regulation, p_wk->p_player_modify_party );

    p_param->p_party  = p_wk->p_player_modify_party;

  }

  //敵データ
  { 
    WIFIBATTLEMATCH_ENEMYDATA *p_enemy;
    POKEPARTY *p_party;
    p_enemy = p_wk->p_enemy_data;

    PokeParty_Copy((POKEPARTY*)p_enemy->pokeparty, p_wk->p_enemy_modify_party);
    PokeRegulation_ModifyLevelPokeParty( p_param->regulation, p_wk->p_enemy_modify_party );

    p_param->enemyName      = MyStatus_GetMyName( (MYSTATUS*)p_enemy->mystatus );
    p_param->enemyPokeParty = p_wk->p_enemy_modify_party;
    p_param->enemySex       = MyStatus_GetMySex( (MYSTATUS*)p_enemy->mystatus );

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
static BOOL POKELIST_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS             *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_SUBPROC_PARAM   *p_param  = p_param_adrs;

  if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  { 
    switch( p_param->result )
    {
    case WIFIBATTLEMATCH_SUBPROC_RESULT_SUCCESS:
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_LISTAFTER );
      break;

    case WIFIBATTLEMATCH_SUBPROC_RESULT_ERROR_RETURN_LIVE:
      p_wk->btl_score.is_error  = TRUE;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_MAINMENU );
      break;

    default:
      GF_ASSERT_MSG( 0, "IRC %d\n", p_param->result );
    }
  }
  else
  { 
    switch( p_param->result)
    {
    case WIFIBATTLEMATCH_SUBPROC_RESULT_SUCCESS:
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_LISTAFTER );
      break;

    case WIFIBATTLEMATCH_SUBPROC_RESULT_ERROR_NEXT_LOGIN:
      p_wk->is_err_return_login = TRUE;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_LOGIN );
      break;
    case WIFIBATTLEMATCH_SUBPROC_RESULT_ERROR_DISCONNECT_WIFI:
      //切断された
      p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE_ERR;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_CORE );
      break;

    default:
      GF_ASSERT_MSG( 0, "WIFI %d\n", p_param->result );
    }
  }

  //ランダムマッチではレギュレーションをALLOCしたので
  //解放する
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDRATE
      || p_wk->type == WIFIBATTLEMATCH_TYPE_RNDFREE )
  { 
    GFL_HEAP_FreeMemory( p_param->regulation );
  }
  GFL_HEAP_FreeMemory( p_param );

  return TRUE;
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
static void *BATTLE_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_BATTLELINK_PARAM  *p_param;
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  REGULATION* p_reg;
  BOOL is_alloc = FALSE;

  SAVE_CONTROL_WORK *p_sv       = GAMEDATA_GetSaveControlWork( p_wk->param.p_game_data );
  REGULATION_SAVEDATA *p_reg_sv = SaveData_GetRegulationSaveData( p_sv );
  
  //デモバトル接続ワーク
  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_BATTLELINK_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_BATTLELINK_PARAM) );
  p_param->p_gamedata  = p_wk->param.p_game_data;

  //録画バッファをクリア
  BattleRec_DataClear();

  //バトルスコアをクリア
  GFL_STD_MemClear( &p_wk->btl_score, sizeof(BATTLEMATCH_BATTLE_SCORE) );

  //デモパラメータ
  p_param->p_demo_param = GFL_HEAP_AllocMemory( heapID, sizeof(COMM_BTL_DEMO_PARAM) );
	GFL_STD_MemClear( p_param->p_demo_param, sizeof(COMM_BTL_DEMO_PARAM) );
  p_param->p_demo_param->record = GAMEDATA_GetRecordPtr( p_wk->param.p_game_data );
  p_param->p_demo_param->wcs_flag = FALSE;

  //デモパラメータへの設定
  //自分
  {
    COMM_BTL_DEMO_TRAINER_DATA *p_tr;
    p_tr  = &p_param->p_demo_param->trainer_data[ COMM_BTL_DEMO_TRDATA_A ];
    p_tr->mystatus  = (MYSTATUS*)p_wk->p_player_data->mystatus;
    p_tr->party     = p_wk->p_player_btl_party;
    p_tr->server_version  = p_wk->p_player_data->btl_server_version;
  }
  //相手
  { 
    COMM_BTL_DEMO_TRAINER_DATA *p_tr;
    p_tr  = &p_param->p_demo_param->trainer_data[ COMM_BTL_DEMO_TRDATA_B ];
    p_tr->mystatus  = (MYSTATUS*)p_wk->p_enemy_data->mystatus;
    p_tr->party     = p_wk->p_enemy_btl_party;
    p_tr->server_version  = p_wk->p_enemy_data->btl_server_version;
  }

  //バトル設定パラメータ
  p_param->p_btl_setup_param	= GFL_HEAP_AllocMemory( heapID, sizeof(BATTLE_SETUP_PARAM) );
	GFL_STD_MemClear( p_param->p_btl_setup_param, sizeof(BATTLE_SETUP_PARAM) );

  GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );

	//ランダムバトルのバトル設定
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDRATE
      ||  p_wk->type == WIFIBATTLEMATCH_TYPE_RNDFREE )
  { 
    int reg_no;


    switch( p_wk->param.btl_rule )
    {
    default:
      GF_ASSERT(0);
    case WIFIBATTLEMATCH_BTLRULE_SINGLE:    ///< シングル
      reg_no  = REG_RND_SINGLE;
      BTL_SETUP_Single_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case WIFIBATTLEMATCH_BTLRULE_DOUBLE:    ///< ダブル
      reg_no  = REG_RND_DOUBLE;
      BTL_SETUP_Double_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case WIFIBATTLEMATCH_BTLRULE_TRIPLE:    ///< トリプル
      reg_no  = REG_RND_TRIPLE;
      BTL_SETUP_Triple_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case WIFIBATTLEMATCH_BTLRULE_ROTATE:  ///< ローテーション
      reg_no  = REG_RND_ROTATION;
      BTL_SETUP_Rotation_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;
    case WIFIBATTLEMATCH_BTLRULE_SHOOTER:  ///< シューター
      reg_no  = REG_RND_TRIPLE_SHOOTER;
      BTL_SETUP_Triple_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      //シューター用設定は下部で行っている
      break;
    }

    p_reg = (REGULATION*)PokeRegulation_LoadDataAlloc( reg_no, heapID );
    is_alloc  = TRUE;
  }
  //WiFI大会のバトル設定
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_WIFICUP )
  { 
    REGULATION_CARDDATA *p_reg_card    = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_WIFI );
    p_reg        = RegulationData_GetRegulation( p_reg_card );

    switch( Regulation_GetParam( p_reg, REGULATION_BATTLETYPE ) )
    {
    case REGULATION_BATTLE_SINGLE:    ///< シングル
      BTL_SETUP_Single_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case REGULATION_BATTLE_DOUBLE:    ///< ダブル
      BTL_SETUP_Double_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case REGULATION_BATTLE_TRIPLE:    ///< トリプル
      BTL_SETUP_Triple_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case REGULATION_BATTLE_ROTATION:  ///< ローテーション
      BTL_SETUP_Rotation_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    default:
      GF_ASSERT( 0 );
      //念のためシングルにしておく
      BTL_SETUP_Single_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
    }
  }
  //LIVE大会のバトル設定
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  { 
    REGULATION_CARDDATA *p_reg_card    = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_LIVE );
    p_reg        = RegulationData_GetRegulation( p_reg_card );

    switch( Regulation_GetParam( p_reg, REGULATION_BATTLETYPE ) )
    {
    case REGULATION_BATTLE_SINGLE:    ///< シングル
      BTL_SETUP_Single_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case REGULATION_BATTLE_DOUBLE:    ///< ダブル
      BTL_SETUP_Double_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case REGULATION_BATTLE_TRIPLE:    ///< トリプル
      BTL_SETUP_Triple_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    case REGULATION_BATTLE_ROTATION:  ///< ローテーション
      BTL_SETUP_Rotation_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;

    default:
      GF_ASSERT( 0 );
      //念のためシングルにしておく
      BTL_SETUP_Single_Comm( p_param->p_btl_setup_param, p_wk->param.p_game_data, 
          GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_WIFI, heapID );
      break;
    }
  }
  else 
  { 
    GF_ASSERT(0);
  }

  //バトルタイプ設定
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDFREE )
  { 
    //ランダムマッチ：フリー
    p_param->p_demo_param->battle_mode = 
      BATTLE_MODE_RANDOM_FREE_SINGLE + p_wk->param.btl_rule;
  }
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDRATE )
  { 
    //ランダムマッチ：レーティング
    p_param->p_demo_param->battle_mode = 
      BATTLE_MODE_RANDOM_RATING_SINGLE + p_wk->param.btl_rule;
  }
  else
  { 
    int  is_shooter  = Regulation_GetParam( p_reg, REGULATION_SHOOTER );
    REGULATION_BATTLE_TYPE  battle_type = Regulation_GetParam( p_reg, REGULATION_BATTLETYPE );

    if( is_shooter >= REGULATION_SHOOTER_MANUAL )
    {
        is_shooter  = REGULATION_SHOOTER_VALID;
    }

    p_param->p_demo_param->battle_mode = 
      BATTLE_MODE_COMPETITION_SINGLE + battle_type * 2 + is_shooter;
  }

  //曲の設定
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDRATE
      ||  p_wk->type == WIFIBATTLEMATCH_TYPE_RNDFREE )
  { 
    p_param->p_btl_setup_param->musicDefault  = WBM_SND_SEQ_BATTLE_RND;
    p_param->p_btl_setup_param->musicWin    = WBM_SND_SEQ_BATTLE_RND_WIN;
  }
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_WIFICUP )
  { 
    REGULATION_CARDDATA *p_reg_card    = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_WIFI );
    int bgm_type  = Regulation_GetCardParam( p_reg_card, REGULATION_CARD_BGM );
    int state_type  = Regulation_GetParam( RegulationData_GetRegulation(p_reg_card), REGULATION_STATE );

    if( bgm_type == REGULATION_CARD_BGM_WCS )
    { 
      p_param->p_btl_setup_param->musicDefault  = WBM_SND_BGM_FAINAL;
      p_param->p_btl_setup_param->musicWin    = WBM_SND_BGM_FAINAL_WIN;
    }
    else
    { 
      p_param->p_btl_setup_param->musicDefault  = WBM_SND_BGM_NORMAL;
      p_param->p_btl_setup_param->musicWin    = WBM_SND_BGM_NORMAL_WIN;
    }

    if( REGULATION_STATE_WCS_15 == state_type 
      || state_type == REGULATION_STATE_WCS_02)
    {

      p_param->p_btl_setup_param->fieldSituation.bgAttr = BATTLE_BG_ATTR_E_INDOOR;
      p_param->p_btl_setup_param->fieldSituation.bgType = BATTLE_BG_TYPE_WCS;
      p_param->p_demo_param->wcs_flag = TRUE;
    }
  }
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  { 
    REGULATION_CARDDATA *p_reg_card    = RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_LIVE );
    int bgm_type  = Regulation_GetCardParam( p_reg_card, REGULATION_CARD_BGM );
    int state_type  = Regulation_GetParam( RegulationData_GetRegulation(p_reg_card), REGULATION_STATE );

    if( bgm_type == REGULATION_CARD_BGM_WCS )
    { 
      p_param->p_btl_setup_param->musicDefault  = WBM_SND_BGM_FAINAL;
      p_param->p_btl_setup_param->musicWin    = WBM_SND_BGM_FAINAL_WIN;
    }
    else
    { 
      p_param->p_btl_setup_param->musicDefault  = WBM_SND_BGM_NORMAL;
      p_param->p_btl_setup_param->musicWin    = WBM_SND_BGM_NORMAL_WIN;
    }

    if( REGULATION_STATE_WCS_15 == state_type 
      || state_type == REGULATION_STATE_WCS_02)
    {
      p_param->p_btl_setup_param->fieldSituation.bgAttr = BATTLE_BG_ATTR_E_INDOOR;
      p_param->p_btl_setup_param->fieldSituation.bgType = BATTLE_BG_TYPE_WCS;
      p_param->p_demo_param->wcs_flag = TRUE;
    }
  }

  //ポケモン設定
  BATTLE_PARAM_SetPokeParty( p_param->p_btl_setup_param, p_wk->p_player_btl_party, BTL_CLIENT_PLAYER );

  //レギュレーションの内容を適用
  BATTLE_PARAM_SetRegulation( p_param->p_btl_setup_param, p_reg, GFL_HEAP_LOWID( heapID ) );

  //不正文字
  if( p_wk->is_dirty_name )
  {
    p_param->p_btl_setup_param->WifiBadNameFlag = 1;
  }

#ifdef WBM_SYS_BATTLE_VSTIME_1
  p_param->p_btl_setup_param->LimitTimeGame = 1;
#endif

  WBM_SYS_Printf( "vs %d\n", p_param->p_btl_setup_param->LimitTimeGame );
  WBM_SYS_Printf( "cmd %d\n",  p_param->p_btl_setup_param->LimitTimeCommand );

  //録画準備
  BTL_SETUP_AllocRecBuffer( p_param->p_btl_setup_param, heapID );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );

  //戦闘曲強制書き換え（バトルレコーダーの曲置換デバッグ用）
#ifdef PM_DEBUG
  {   
    DEBUGWIN_BTLBGM_DATA *p_debug = DEBUGWIN_BTLBGM_DATA_GetInstance();
    if( p_debug->is_use )
    { 
      p_param->p_btl_setup_param->musicDefault  = p_debug->btl_bgm;
      p_param->p_btl_setup_param->musicWin    = p_debug->win_bgm;
    }
  }
#endif //PM_DEBUG

  if( is_alloc )
  { 
    GFL_HEAP_FreeMemory( p_reg );
  }

  //デモパラメータ
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
static BOOL BATTLE_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_BATTLELINK_PARAM  *p_param  = p_param_adrs;
  BATTLE_SETUP_PARAM  *p_btl_param  = p_param->p_btl_setup_param;
  WIFIBATTLEMATCH_BATTLELINK_RESULT result  = p_param->result;

#ifdef PM_DEBUG
  (*DEBUGWIN_BATTLE_GetFlag())  = TRUE;
#endif

  //受け取り
  p_wk->btl_score.rule  = p_btl_param->rule;
  p_wk->btl_score.result  = p_btl_param->result;
  p_wk->btl_score.is_dirty  = p_btl_param->cmdIllegalFlag;
  p_wk->btl_score.result  = p_btl_param->result;
  { 
    u8 you_rest_poke;
    u8 my_rest_poke;
    u8 dummy;

    Util_GetRestPoke( p_param->p_btl_setup_param, &my_rest_poke, &you_rest_poke );
    p_wk->btl_score.enemy_rest_poke = you_rest_poke;
    
    Util_GetRestHp( p_param->p_btl_setup_param, &dummy, &p_wk->btl_score.enemy_rest_hp );

    WBM_SYS_Printf( "ボール結果 自分%d　相手%d \n", my_rest_poke, you_rest_poke);
    WBM_SYS_Printf( "HP結果 自分%d　相手%d \n", dummy, p_wk->btl_score.enemy_rest_hp);
  }

  WBM_SYS_Printf( "バトル結果 %d \n", p_wk->btl_score.result);

  //録画情報にバトル情報を設定
  BattleRec_LoadToolModule();
  BattleRec_StoreSetupParam( p_param->p_btl_setup_param );
  BattleRec_UnloadToolModule();

  //戦績を残す
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_WIFICUP )
  { 
    SAVE_CONTROL_WORK   *p_sv     = GAMEDATA_GetSaveControlWork( p_wk->param.p_game_data );
    REGULATION_SAVEDATA *p_reg_sv = SaveData_GetRegulationSaveData( p_sv );
    REGULATION_CARDDATA *p_reg_card= RegulationSaveData_GetRegulationCard( p_reg_sv, REGULATION_CARD_TYPE_WIFI );
    REGULATION          *p_reg    = RegulationData_GetRegulation( p_reg_card );
    u32                 cupNO     = Regulation_GetCardParam( p_reg_card, REGULATION_CARD_CUPNO );

    if( result == WIFIBATTLEMATCH_BATTLELINK_RESULT_SUCCESS )
    {
      Util_SetRecordData( &p_wk->record_data, p_wk->p_player_btl_party, p_wk->p_enemy_btl_party, p_wk->p_enemy_data, p_reg, cupNO, p_btl_param, &p_wk->btl_score, WIFIBATTLEMATCH_TYPE_WIFICUP );
    }
  }
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDRATE )
  { 
    REGULATION          *p_reg    = (REGULATION*)PokeRegulation_LoadDataAlloc( REG_RND_SINGLE + p_wk->param.btl_rule, HEAPID_WIFIBATTLEMATCH_SYS );
    if( result == WIFIBATTLEMATCH_BATTLELINK_RESULT_SUCCESS )
    {
      Util_SetRecordData( &p_wk->record_data, p_wk->p_player_btl_party, p_wk->p_enemy_btl_party, p_wk->p_enemy_data, p_reg, 0, p_btl_param, &p_wk->btl_score,WIFIBATTLEMATCH_TYPE_RNDRATE );
    }
    GFL_HEAP_FreeMemory( p_reg );
  }

  //破棄
  BATTLE_PARAM_Release( p_param->p_btl_setup_param );
	GFL_HEAP_FreeMemory( p_param->p_btl_setup_param );
	GFL_HEAP_FreeMemory( p_param->p_demo_param );
	GFL_HEAP_FreeMemory( p_param );

  //次のPROC
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  { 
    switch( result )
    {
    case WIFIBATTLEMATCH_BATTLELINK_RESULT_SUCCESS:
      //ライブマッチはメインメニューからくるのでライブまっちへ戻る
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_MAINMENU );
      break;
    case WIFIBATTLEMATCH_BATTLELINK_RESULT_ERROR_LIVE:
      p_wk->btl_score.is_error  = TRUE;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_MAINMENU );
      break;
    default:
      GF_ASSERT_MSG( 0, "IRC %d\n", result );
    }
  }
  else
  {
    //ランダムマッチとWIFIはCOREからくる
    switch( result )
    {
    case WIFIBATTLEMATCH_BATTLELINK_RESULT_SUCCESS:
      p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_CORE );
      break;

    case WIFIBATTLEMATCH_BATTLELINK_RESULT_ERROR_RETURN_LOGIN_WIFI:
      p_wk->is_err_return_login = TRUE;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_LOGIN );
      break;

    case WIFIBATTLEMATCH_BATTLELINK_RESULT_ERROR_DISCONNECT_WIFI:
      p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE_ERR;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_CORE );
      break;
    default:
      GF_ASSERT_MSG( 0, "WIFI %d\n", result );
    }
  }

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
static void *LOGIN_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{ 
  WIFILOGIN_PARAM *p_param;
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFILOGIN_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFILOGIN_PARAM) );

  p_param->gamedata = p_wk->param.p_game_data;
  p_param->bg       = WIFILOGIN_BG_NORMAL;
  p_param->display  = WIFILOGIN_DISPLAY_UP;
  p_param->pSvl     = &p_wk->svl_result;

  if( p_wk->param.mode == WIFIBATTLEMATCH_MODE_MAINMENU )
  {
    p_param->nsid     = WB_NET_WIFIMATCH;
  }
  else
  {
    p_param->nsid     = WB_NET_WIFIRNDMATCH;
  }

  if( p_wk->is_err_return_login )
  { 
    p_wk->is_err_return_login = FALSE;
    p_param->mode     = WIFILOGIN_MODE_ERROR;
  }
  else
  { 
    p_param->mode     = WIFILOGIN_MODE_NORMAL;
  }

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
static BOOL LOGIN_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  WIFILOGIN_PARAM  *p_param     = p_param_adrs;
  WIFILOGIN_RESULT  result      = p_param->result;

  GFL_HEAP_FreeMemory( p_param );

  switch( result )
  { 
  case WIFILOGIN_RESULT_LOGIN:
    p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_START;
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_CORE );
    break;

  case WIFILOGIN_RESULT_CANCEL:
    if( p_wk->param.mode == WIFIBATTLEMATCH_MODE_MAINMENU )
    { 
      //WIFI大会ならばメニューへ戻る
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_MAINMENU );
    }
    else
    { 
      //ランダムマッチならば終了
      WBM_SYS_SUBPROC_End( p_subproc );
    }
    break;
  default:
    GF_ASSERT_MSG( 0, "IRC %d\n", result );
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	リスト後プロセスの引数	作成
 *
 *	@param	HEAPID heapID			ヒープID
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return	引数
 */
//-----------------------------------------------------------------------------
static void *WBM_LISTAFTER_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_LISTAFTER_PARAM *p_param;
  WIFIBATTLEMATCH_SYS             *p_wk     = p_wk_adrs;
  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFIBATTLEMATCH_LISTAFTER_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_LISTAFTER_PARAM) );
  p_param->p_param        = &p_wk->param;
  p_param->p_player_btl_party = p_wk->p_player_btl_party;
  p_param->p_enemy_btl_party  = p_wk->p_enemy_btl_party;
  p_param->p_net_data        = &p_wk->net_data;
  p_param->p_recv_data    = &p_wk->recv_data;

  if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  { 
    p_param->type = WIFIBATTLEMATCH_LISTAFTER_NETTYPE_IRC;
  }
  else
  { 
    p_param->type = WIFIBATTLEMATCH_LISTAFTER_NETTYPE_WIFI;
  }

  return p_param;
}

//----------------------------------------------------------------------------
/**
 *	@brief	リスト後プロセスの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL WBM_LISTAFTER_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS               *p_wk     = p_wk_adrs;
  WIFIBATTLEMATCH_LISTAFTER_PARAM   *p_param  = p_param_adrs;
  WIFIBATTLEMATCH_LISTAFTER_RESULT  result    = p_param->result;

  GFL_HEAP_FreeMemory( p_param );

  if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  { 
    switch( result )
    { 
    case WIFIBATTLEMATCH_LISTAFTER_RESULT_SUCCESS:
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_BATTLE );
      break;
    case WIFIBATTLEMATCH_LISTAFTER_RESULT_ERROR_RETURN_LIVE:
      p_wk->btl_score.is_error  = TRUE;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_MAINMENU );
      break;
    default:
      GF_ASSERT_MSG( 0, "IRC %d\n", result );
    }
  }
  else
  { 
    switch( result )
    { 
    case WIFIBATTLEMATCH_LISTAFTER_RESULT_SUCCESS:
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_BATTLE );
      break;

    case WIFIBATTLEMATCH_LISTAFTER_RESULT_ERROR_NEXT_LOGIN:
      p_wk->is_err_return_login = TRUE;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_LOGIN );
      break;

    case WIFIBATTLEMATCH_LISTAFTER_RESULT_ERROR_DISCONNECT_WIFI:
      p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_ENDBATTLE_ERR;
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_CORE );
      break;
    default:
      GF_ASSERT_MSG( 0, "WIFI %d\n", result );
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	録画プロセスの引数	作成
 *
 *	@param	HEAPID heapID			ヒープID
 *	@param	*p_wk_adrs				ワーク
 *
 *	@return	引数
 */
//-----------------------------------------------------------------------------
static void *WBM_BTLREC_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS             *p_wk     = p_wk_adrs;
  BTL_REC_SEL_PARAM               *p_param;
  BATTLE_MODE                     battle_mode;

  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(BTL_REC_SEL_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(BTL_REC_SEL_PARAM) );

  //バトルタイプ設定
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDFREE )
  { 
    //ランダムマッチ：フリー
    battle_mode = BATTLE_MODE_RANDOM_FREE_SINGLE + p_wk->param.btl_rule;
  }
  else if( p_wk->type == WIFIBATTLEMATCH_TYPE_RNDRATE )
  { 
    //ランダムマッチ：レーティング
    battle_mode = BATTLE_MODE_RANDOM_RATING_SINGLE + p_wk->param.btl_rule;
  }
  else
  { 
    REGULATION_CARD_TYPE  type;
    if( p_wk->type == WIFIBATTLEMATCH_TYPE_WIFICUP )
    { 
      //WIFI大会
      type  = REGULATION_CARD_TYPE_WIFI;
    }
    else if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
    { 
      //LIVE大会
      type  = REGULATION_CARD_TYPE_LIVE;
    }
    else
    { 
      GF_ASSERT(0);
    }

    { 
      SAVE_CONTROL_WORK   *p_sv       = GAMEDATA_GetSaveControlWork( p_wk->param.p_game_data );
      REGULATION_SAVEDATA *p_reg_sv   = SaveData_GetRegulationSaveData( p_sv );
      REGULATION_CARDDATA *p_reg_card = RegulationSaveData_GetRegulationCard( p_reg_sv, type );
      REGULATION          *p_reg      = RegulationData_GetRegulation( p_reg_card );
      int                is_shooter  = Regulation_GetParam( p_reg, REGULATION_SHOOTER );
      REGULATION_BATTLE_TYPE  battle_type  = Regulation_GetParam( p_reg, REGULATION_BATTLETYPE );

      if( is_shooter >= REGULATION_SHOOTER_MANUAL )
      {
        is_shooter  = REGULATION_SHOOTER_VALID;
      }

      battle_mode = BATTLE_MODE_COMPETITION_SINGLE + battle_type * 2 + is_shooter;

      OS_TPrintf( "mode%d type%d sho%d\n", battle_mode, battle_type, is_shooter );
    }
  }

  p_param->gamedata     = p_wk->param.p_game_data;
  p_param->b_rec        = TRUE;
  p_param->b_sync       = FALSE;
  p_param->battle_mode  = battle_mode;;
  p_param->fight_count  = 0;
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  {
    //ライブ大会は不正録画チェックをしない
    p_param->b_correct    = TRUE;
  }
  else
  {
    p_param->b_correct    = (!p_wk->btl_score.is_dirty && !p_wk->btl_score.is_other_dirty);
  }

  //相手のサーバーが、自分のサーバーよりバージョンが上だったら録画できない
  //相手と自分のサーバーが違かったら録画できない
  if( BattleRec_ServerVersionCheck( p_wk->p_enemy_data->btl_server_version ) )
  {
    p_param->b_rec      = TRUE;
  }
  else
  {
    p_param->b_rec      = FALSE;
  }

  return p_param;
}

//----------------------------------------------------------------------------
/**
 *	@brief	録画プロセスの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL WBM_BTLREC_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS             *p_wk     = p_wk_adrs;
  BTL_REC_SEL_PARAM               *p_param  = p_param_adrs;

  GFL_HEAP_FreeMemory( p_param );

  //次のPROC
  if( p_wk->type == WIFIBATTLEMATCH_TYPE_LIVECUP )
  { 
    //ライブマッチはメインメニューからくるのでライブまっちへ戻る
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_MAINMENU );
  }
  else
  { 
    p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_ENDREC;
    WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_CORE );
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIログアウトプロセスの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static void *LOGOUT_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  WIFILOGOUT_PARAM    *p_param;

  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(WIFILOGOUT_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(WIFILOGOUT_PARAM) );

  p_param->gamedata = p_wk->param.p_game_data;
  p_param->bg       = WIFILOGIN_BG_NORMAL;
  p_param->display  = WIFILOGIN_DISPLAY_UP;

  if( p_wk->is_wificup_end )
  { 
    p_param->fade = WIFILOGIN_FADE_BLACK_IN | WIFILOGIN_FADE_WHITE_OUT;
  }

  return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIログアウトプロセスの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL LOGOUT_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{ 
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  WIFILOGOUT_PARAM    *p_param  = p_param_adrs;

  switch( p_wk->type )
  { 
  case WIFIBATTLEMATCH_TYPE_WIFICUP:
    if( p_wk->is_wificup_end )
    { 
      WBM_SYS_SUBPROC_End( p_subproc );
    }
    else
    { 
      WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_MAINMENU );
    }
    break;
  case WIFIBATTLEMATCH_TYPE_LIVECUP:
    GF_ASSERT(0);
    break;
  case WIFIBATTLEMATCH_TYPE_RNDRATE:
  case WIFIBATTLEMATCH_TYPE_RNDFREE:
    WBM_SYS_SUBPROC_End( p_subproc );
    break;
  }

  GFL_HEAP_FreeMemory( p_param );

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	録画再生プロセスの引数	破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static void *RECPLAY_AllocParam( WBM_SYS_SUBPROC_WORK *p_subproc,HEAPID heapID, void *p_wk_adrs )
{ 
  BATTLE_SETUP_PARAM  *p_param;
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  
  //バトル設定パラメータ
  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(BATTLE_SETUP_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(BATTLE_SETUP_PARAM) );

  GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
  BATTLE_PARAM_Init( p_param );
  BTL_SETUP_InitForRecordPlay( p_param, p_wk->param.p_game_data, heapID );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
  BattleRec_LoadToolModule();
  BattleRec_RestoreSetupParam( p_param, heapID );
  BattleRec_UnloadToolModule();

  GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );


  PMSND_StopBGM();
  PMSND_PlayBGM( p_param->musicDefault );

  //デモパラメータ
	return p_param;
}

//----------------------------------------------------------------------------
/**
 *	@brief	録画再生プロセスの引数  破棄
 *
 *	@param	void *p_param_adrs				引数
 *	@param	*p_wk_adrs								ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RECPLAY_FreeParam( WBM_SYS_SUBPROC_WORK *p_subproc,void *p_param_adrs, void *p_wk_adrs )
{ 
  BATTLE_SETUP_PARAM  *p_param  = p_param_adrs;
  WIFIBATTLEMATCH_SYS *p_wk     = p_wk_adrs;
  
  PMSND_StopBGM();

  GFL_HEAP_FreeMemory( p_param->playerStatus[ BTL_CLIENT_PLAYER ] );  //プレイヤーのMySatusは開放されないので
  BTL_SETUP_QuitForRecordPlay( p_param );
  GFL_HEAP_FreeMemory( p_param );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );

  WBM_SYS_SUBPROC_CallProc( p_subproc, SUBPROCID_MAINMENU );
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
  GFL_STD_MemClear( p_wk->p_player_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
  p_wk->p_enemy_data  = GFL_HEAP_AllocMemory( heapID, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );
  GFL_STD_MemClear( p_wk->p_enemy_data, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE );

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

//----------------------------------------------------------------------------
/**
 *	@brief  データ作成
 *
 *	@param	WIFIBATTLEMATCH_RECORD_DATA *p_data ワーク
 */
//-----------------------------------------------------------------------------
static void Util_SetRecordData( WIFIBATTLEMATCH_RECORD_DATA *p_data, const POKEPARTY *cp_my_poke, const POKEPARTY *cp_you_poke, const WIFIBATTLEMATCH_ENEMYDATA *cp_you_data, const REGULATION *cp_reg, u32 cupNO, const BATTLE_SETUP_PARAM *cp_btl_param, const BATTLEMATCH_BATTLE_SCORE *cp_btl_score, WIFIBATTLEMATCH_TYPE type )
{ 
  int i;
  GFL_STD_MemClear( p_data, sizeof(WIFIBATTLEMATCH_RECORD_DATA) );

  //対戦相手のプロフィール
  p_data->your_gamesyncID = MyStatus_GetProfileID( (MYSTATUS*)cp_you_data->mystatus );
  p_data->your_profileID  = cp_you_data->profileID;

  //対戦相手のポケモン
  for( i = 0; i < PokeParty_GetPokeCount(cp_you_poke); i++ )
  { 
    POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer( cp_you_poke, i );
    if( PP_Get( p_pp, ID_PARA_poke_exist, NULL ) )
    { 
      p_data->your_monsno[i] = PP_Get( p_pp, ID_PARA_monsno, NULL );
      p_data->your_form[i]   = PP_Get( p_pp, ID_PARA_form_no, NULL );
      p_data->your_lv[i]     = PP_Get( p_pp, ID_PARA_level, NULL );
      p_data->your_sex[i]    = PP_Get( p_pp, ID_PARA_sex, NULL );
    }
  }

  //自分のポケモン
  for( i = 0; i < PokeParty_GetPokeCount(cp_my_poke); i++ )
  { 
    POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer( cp_my_poke, i );
    if( PP_Get( p_pp, ID_PARA_poke_exist, NULL ) )
    { 
      p_data->my_monsno[i] = PP_Get( p_pp, ID_PARA_monsno, NULL );
      p_data->my_form[i]   = PP_Get( p_pp, ID_PARA_form_no, NULL );
      p_data->my_lv[i]     = PP_Get( p_pp, ID_PARA_level, NULL );
      p_data->my_sex[i]    = PP_Get( p_pp, ID_PARA_sex, NULL );
    }
  }

  //対戦情報
  { 
    BOOL ret;
    RTCDate date;
    RTCTime time;
    ret = DWC_GetDateTime( &date, &time );
    GF_ASSERT( ret );

    p_data->year  = date.year;
    p_data->month = date.month;
    p_data->day   = date.day;
    p_data->hour  = time.hour;
    p_data->minute= time.minute;
  }
  p_data->cupNO   = cupNO;
  p_data->result  = cp_btl_param->result;
  {
    u8 you_rest_poke;
    u8 my_rest_poke;
    Util_GetRestPoke( cp_btl_param, &my_rest_poke, &you_rest_poke );
    p_data->rest_my_poke    = my_rest_poke;
    p_data->rest_you_poke   = you_rest_poke;
  }
  p_data->btl_type= Regulation_GetParam( cp_reg, REGULATION_BATTLETYPE );
  p_data->shooter_flag  = Regulation_GetParam( cp_reg, REGULATION_SHOOTER ) ;
}


//----------------------------------------------------------------------------
/**
 *	@brief  残りポケモンを取得
 *
 *	@param	const BATTLE_SETUP_PARAM *cp_btl_param  引数
 *	@param	*p_my_poke  自分の残りポケモン
 *	@param	*p_you_poke 相手の残りポケモン
 */
//-----------------------------------------------------------------------------
static void Util_GetRestPoke( const BATTLE_SETUP_PARAM *cp_btl_param, u8 *p_my_poke, u8 *p_you_poke )
{
  int tr_no;
  int party_no;
  int my_pos;
  int client_no;
  int max;

  *p_my_poke  = 0;
  *p_you_poke = 0;

  my_pos = cp_btl_param->commPos;
  for(tr_no = 0; tr_no <= COMM_BTL_DEMO_TRDATA_B; tr_no++){
    if((tr_no & 1) == (my_pos & 1)){  //自分パーティ
      if(my_pos & 1){
        client_no = BTL_CLIENT_ENEMY1 + (tr_no & 2);
      }
      else{
        client_no = BTL_CLIENT_PLAYER + (tr_no & 2);
      }

      max = PokeParty_GetPokeCount(cp_btl_param->party[client_no] );
      for(party_no = 0; party_no < max; party_no++){
        if( cp_btl_param->party_state[client_no][party_no] != BTL_POKESTATE_DEAD )
        {
          (*p_my_poke)++;
        }
      }
    }
    else{ //敵パーティ
      if(my_pos & 1){
        client_no = BTL_CLIENT_PLAYER + (tr_no & 2);
      }
      else{
        client_no = BTL_CLIENT_ENEMY1 + (tr_no & 2);
      }
      max = PokeParty_GetPokeCount(cp_btl_param->party[client_no] );
      for(party_no = 0; party_no < max; party_no++){
        if( cp_btl_param->party_state[client_no][party_no] != BTL_POKESTATE_DEAD )
        {
          (*p_you_poke)++;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  残りHPを取得
 *
 *	@param	const BATTLE_SETUP_PARAM *cp_btl_param  パラメータ
 *	@param	*p_my_poke    自分の残りHP
 *	@param	*p_you_poke   相手の残りHP
 */
//-----------------------------------------------------------------------------
static void Util_GetRestHp( const BATTLE_SETUP_PARAM *cp_btl_param, u8 *p_my_poke, u8 *p_you_poke )
{
  int tr_no;
  int party_no;
  int my_pos;
  int client_no;

  *p_my_poke  = 0;
  *p_you_poke = 0;

  my_pos = cp_btl_param->commPos;
  for(tr_no = 0; tr_no <= COMM_BTL_DEMO_TRDATA_B; tr_no++){
    if((tr_no & 1) == (my_pos & 1)){  //自分パーティ
      if(my_pos & 1){
        client_no = BTL_CLIENT_ENEMY1 + (tr_no & 2);
      }
      else{
        client_no = BTL_CLIENT_PLAYER + (tr_no & 2);
      }
      (*p_my_poke)  = cp_btl_param->restHPRatio[client_no];
    }
    else{ //敵パーティ
      if(my_pos & 1){
        client_no = BTL_CLIENT_PLAYER + (tr_no & 2);
      }
      else{
        client_no = BTL_CLIENT_ENEMY1 + (tr_no & 2);
      }
      (*p_you_poke)  = cp_btl_param->restHPRatio[client_no];
    }
  }
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  サブプロセス
 *				    ・プロセスを行き来するシステム
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
#define WBM_SYS_SUBPROC_END   (0xFF)
//-------------------------------------
///	ワーク
//=====================================
struct _WBM_SYS_SUBPROC_WORK
{
	GFL_PROCSYS			  *p_procsys;
	void						  *p_proc_param;

	HEAPID					  heapID;
	void						  *p_wk_adrs;
	const WBM_SYS_SUBPROC_DATA			*cp_procdata_tbl;
  u32               tbl_len;
  u8                pre_procID;
	u8							  next_procID;
	u8							  now_procID;
	u8							  seq;

  GFL_PROC_MAIN_STATUS  status;
} ;
//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	WBM_SYS_SUBPROCシステム	初期化
 *
 *	@param	WBM_SYS_SUBPROC_WORK *p_wk	ワーク
 *	@param	cp_procdata_tbl			プロセス接続テーブル
 *	@param	void *p_wk_adrs			アロック関数と解放関数に渡すワーク
 *	@param	heapID							システム構築用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static WBM_SYS_SUBPROC_WORK * WBM_SYS_SUBPROC_Init( const WBM_SYS_SUBPROC_DATA *cp_procdata_tbl, u32 tbl_len, void *p_wk_adrs, HEAPID heapID )
{	
  WBM_SYS_SUBPROC_WORK *p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WBM_SYS_SUBPROC_WORK ) );
	GFL_STD_MemClear( p_wk, sizeof(WBM_SYS_SUBPROC_WORK) );
	p_wk->p_procsys				= GFL_PROC_LOCAL_boot( heapID );
	p_wk->p_wk_adrs				= p_wk_adrs;
	p_wk->cp_procdata_tbl	= cp_procdata_tbl;
  p_wk->tbl_len         = tbl_len;
	p_wk->heapID					= heapID;

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	WBM_SYS_SUBPROCシステム	破棄
 *
 *	@param	WBM_SYS_SUBPROC_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void WBM_SYS_SUBPROC_Exit( WBM_SYS_SUBPROC_WORK *p_wk )
{	
	GF_ASSERT( p_wk->p_proc_param == NULL );

	GFL_PROC_LOCAL_Exit( p_wk->p_procsys );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	WBM_SYS_SUBPROCシステム	メイン処理
 *
 *	@param	WBM_SYS_SUBPROC_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば終了	FALSEならばPROCが存在する
 */
//-----------------------------------------------------------------------------
static BOOL WBM_SYS_SUBPROC_Main( WBM_SYS_SUBPROC_WORK *p_wk )
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
    p_wk->pre_procID  = p_wk->now_procID;
		p_wk->now_procID	= p_wk->next_procID;
		p_wk->seq	= SEQ_ALLOC_PARAM;
		break;

	case SEQ_ALLOC_PARAM:
		//プロセス引数作成関数があれば呼び出し
		if( p_wk->cp_procdata_tbl[ p_wk->now_procID ].alloc_func )
		{	
			p_wk->p_proc_param	= p_wk->cp_procdata_tbl[ p_wk->now_procID ].alloc_func(
          p_wk, 
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
			p_wk->status	= GFL_PROC_LOCAL_Main( p_wk->p_procsys );
			if( GFL_PROC_MAIN_NULL == p_wk->status )
			{	
				p_wk->seq	= SEQ_FREE_PARAM;
			}
		}
		break;

	case SEQ_FREE_PARAM:
		//プロセス引数破棄関数呼び出し
		if( p_wk->cp_procdata_tbl[	p_wk->now_procID ].free_func )
		{	
			if( p_wk->cp_procdata_tbl[	p_wk->now_procID ].free_func( p_wk, p_wk->p_proc_param, p_wk->p_wk_adrs ) )
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
		if( p_wk->now_procID == p_wk->next_procID )
		{	
      GF_ASSERT_MSG( 0, "次のPROCが指定されていない\n" );
		}
    if( p_wk->next_procID == WBM_SYS_SUBPROC_END )
    { 
			p_wk->seq	= SEQ_END;
    }
		else
		{	
			p_wk->seq	= SEQ_INIT;
		}
		break;

	case SEQ_END:
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WBM_SYS_SUBPROCシステム PROC状態を取得
 *
 *	@param	const WBM_SYS_SUBPROC_WORK *cp_wk   ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static GFL_PROC_MAIN_STATUS WBM_SYS_SUBPROC_GetStatus( const WBM_SYS_SUBPROC_WORK *cp_wk )
{ 
  return cp_wk->status;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WBM_SYS_SUBPROCシステム 前回のPROCIDを取得
 *
 *	@param	const WBM_SYS_SUBPROC_WORK *cp_wk   ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static u8 WBM_SYS_SUBPROC_GetPreProcID( const WBM_SYS_SUBPROC_WORK *cp_wk )
{ 
  return cp_wk->pre_procID;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WBM_SYS_SUBPROCシステム	プロセスリクエスト
 *
 *	@param	WBM_SYS_SUBPROC_WORK *p_wk	ワーク
 *	@param	proc_id							呼ぶプロセスID
 *
 */
//-----------------------------------------------------------------------------
static void WBM_SYS_SUBPROC_CallProc( WBM_SYS_SUBPROC_WORK *p_wk, u32 procID )
{	
	p_wk->next_procID	= procID;
}


//----------------------------------------------------------------------------
/**
 *	@brief	WBM_SYS_SUBPROCシステム	プロセスの繋がり終了
 *
 *	@param	WBM_SYS_SUBPROC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void WBM_SYS_SUBPROC_End( WBM_SYS_SUBPROC_WORK *p_wk )
{ 
  p_wk->next_procID = WBM_SYS_SUBPROC_END;
}
