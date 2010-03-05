//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		battle_championship_sys.c
 *	@brief  バトル大会管理システム
 *	@author	Toru=Nagihashi
 *	@data		2010.03.04
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

//プロセス
#include "battle_championship_core.h"
#include "title/title.h"
#include "net_app/wifibattlematch.h"
#include "net_app/btl_rec_sel.h"

//外部公開
#include "battle_championship/battle_championship.h"

//-------------------------------------
///	DEBUG
//=====================================
#ifdef PM_DEBUG
#endif //PM_DEBUG

//-------------------------------------
///	PROCエクスターン
//=====================================

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///システム終了時の行き先	
//=====================================
typedef enum
{
  BC_SYS_NEXT_TITLE,
  BC_SYS_NEXT_WIFICUP,
} BC_SYS_NEXT;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef struct _BC_SYS_SUBPROC_WORK BC_SYS_SUBPROC_WORK;

//-------------------------------------
///	システムワーク
//=====================================
typedef struct
{ 
  //プロセス管理システム
  BC_SYS_SUBPROC_WORK          *p_subproc;

  //ゲームデータ（タイトルから直接来るので存在しないため作成）
  GAMEDATA                      *p_gamedata;

  //このシステムの開始モード
  BATTLE_CHAMPIONSHIP_MODE      mode;

  //システム終了時の行き先
  BC_SYS_NEXT                   next;
}BC_SYS_WORK;
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	プロセス用引数作成・解放
//=====================================
//チャンピオンシップコア
static void *BC_CORE_AllocParam( HEAPID heapID, void *p_wk_adrs );
static BOOL BC_CORE_FreeParam( void *p_param_adrs, void *p_wk_adrs );

//録画
static void *BC_BTLREC_AllocParam( HEAPID heapID, void *p_wk_adrs );
static BOOL BC_BTLREC_FreeParam( void *p_param_adrs, void *p_wk_adrs );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  サブプロセス
 *				    ・プロセスを行き来するシステム
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	サブプロセス初期化・解放関数コールバック
//=====================================
typedef void *(*BC_SYS_SUBPROC_ALLOC_FUNCTION)( HEAPID heapID, void *p_wk_adrs );
typedef BOOL (*BC_SYS_SUBPROC_FREE_FUNCTION)( void *p_param, void *p_wk_adrs );

//-------------------------------------
///	サブプロセス設定構造体
//=====================================
typedef struct 
{
	FSOverlayID							    ov_id;
	const GFL_PROC_DATA			    *cp_procdata;
	BC_SYS_SUBPROC_ALLOC_FUNCTION	alloc_func;
	BC_SYS_SUBPROC_FREE_FUNCTION		free_func;
} BC_SYS_SUBPROC_DATA;

//-------------------------------------
///	パブリック
//=====================================
static BC_SYS_SUBPROC_WORK * BC_SYS_SUBPROC_Init( const BC_SYS_SUBPROC_DATA *cp_procdata_tbl, u32 tbl_len, void *p_wk_adrs, HEAPID heapID );
static void BC_SYS_SUBPROC_Exit( BC_SYS_SUBPROC_WORK *p_wk );
static BOOL BC_SYS_SUBPROC_Main( BC_SYS_SUBPROC_WORK *p_wk );
static GFL_PROC_MAIN_STATUS BC_SYS_SUBPROC_GetStatus( const BC_SYS_SUBPROC_WORK *cp_wk );
static void BC_SYS_SUBPROC_CallProc( BC_SYS_SUBPROC_WORK *p_wk, u32 procID );
static void BC_SYS_SUBPROC_End( BC_SYS_SUBPROC_WORK *p_wk );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	BATTLE_CHAMPIONSHIP_ProcData =
{	
	BATTLE_CHAMPIONSHIP_PROC_Init,
	BATTLE_CHAMPIONSHIP_PROC_Main,
	BATTLE_CHAMPIONSHIP_PROC_Exit,
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
  SUBPROCID_BTLREC,
  /*
  SUBPROCID_POKELIST,
  SUBPROCID_BATTLE,
  SUBPROCID_BTLREC,
*/
	SUBPROCID_MAX
} SUBPROC_ID;
static const BC_SYS_SUBPROC_DATA sc_subproc_data[SUBPROCID_MAX]	=
{
  //SUBPROCID_CORE
  { 
    NO_OVERLAY_ID,
		&BATTLE_CHAMPIONSHIP_CORE_ProcData,
		BC_CORE_AllocParam,
		BC_CORE_FreeParam,
  },

  //SUBPROCID_BTLREC
  { 
    FS_OVERLAY_ID( btl_rec_sel ),
    &BTL_REC_SEL_ProcData,
    BC_BTLREC_AllocParam,
    BC_BTLREC_FreeParam,
  },

};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトル大会画面	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  BC_SYS_WORK   *p_wk;

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_CHAMPIONSHIP_SYS, 0x10000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BC_SYS_WORK), HEAPID_BATTLE_CHAMPIONSHIP_SYS );
	GFL_STD_MemClear( p_wk, sizeof(BC_SYS_WORK) );
  p_wk->mode  = (BATTLE_CHAMPIONSHIP_MODE)p_param_adrs;

  //ゲームデータ
  p_wk->p_gamedata  = GAMEDATA_Create( HEAPID_BATTLE_CHAMPIONSHIP_SYS );

  //モジュール作成
	p_wk->p_subproc   = BC_SYS_SUBPROC_Init( sc_subproc_data, SUBPROCID_MAX, p_wk, HEAPID_BATTLE_CHAMPIONSHIP_SYS );
  BC_SYS_SUBPROC_CallProc( p_wk->p_subproc, SUBPROCID_CORE );

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	バトル大会画面	メインプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  BC_SYS_WORK *p_wk = p_wk_adrs;

  //次の行き先指定
  switch( p_wk->next )
  { 
  case BC_SYS_NEXT_TITLE:
    GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
    break;
  case BC_SYS_NEXT_WIFICUP:
    { 
      WIFIBATTLEMATCH_PARAM *p_param;
      p_param = GFL_HEAP_AllocMemory( GFL_HEAPID_APP, sizeof(WIFIBATTLEMATCH_PARAM) );
      GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_PARAM) );
      p_param->mode             = WIFIBATTLEMATCH_MODE_WIFI;
      p_param->is_auto_release  = TRUE;
      GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(wifibattlematch_sys), &WifiBattleMatch_ProcData, p_param );
    }
    break;
  default:
    GF_ASSERT(0); //行き先が指定されていない
  }


  //モジュール破棄
	BC_SYS_SUBPROC_Exit( p_wk->p_subproc );

  //ゲームデータ
  GAMEDATA_Delete( p_wk->p_gamedata );

  //プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_BATTLE_CHAMPIONSHIP_SYS );


  return GFL_PROC_RES_FINISH;
}


//----------------------------------------------------------------------------
/**
 *	@brief	バトル大会画面	メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BATTLE_CHAMPIONSHIP_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  BC_SYS_WORK *p_wk = p_wk_adrs;
  BOOL is_end;

  //プロセス処理
  is_end	= BC_SYS_SUBPROC_Main( p_wk->p_subproc );

  if( is_end )
  {	
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *      プロセス用引数作成・解放
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  バトルチャンピオンシップ用引数作成
 *
 *	@param	HEAPID heapID   ヒープID
 *	@param	*p_wk_adrs      WBM_SUBPROC_Initで与えたワークアドレス
 *
 *	@return 作成したパラメータ
 */
//-----------------------------------------------------------------------------
static void *BC_CORE_AllocParam( HEAPID heapID, void *p_wk_adrs )
{ 
  BC_SYS_WORK *p_wk = p_wk_adrs;
  BATTLE_CHAMPIONSHIP_CORE_PARAM  *p_param;

  p_param = GFL_HEAP_AllocMemory( heapID, sizeof(BATTLE_CHAMPIONSHIP_CORE_PARAM) );
  GFL_STD_MemClear( p_param, sizeof(BATTLE_CHAMPIONSHIP_CORE_PARAM) );
  p_param->p_gamedata = p_wk->p_gamedata;

  switch( (u32)p_wk->mode )
  {
  case (u32)BATTLE_CHAMPIONSHIP_MODE_MAIN_MENU:
    p_param->mode       = BATTLE_CHAMPIONSHIP_CORE_MODE_MAIN_MEMU;
    break;
  case (u32)BATTLE_CHAMPIONSHIP_MODE_WIFI_MENU:
    p_param->mode       = BATTLE_CHAMPIONSHIP_CORE_MODE_WIFI_MENU;
    break;
  }
  return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルチャンピオンシップ用引数破棄
 *
 *	@param	void *p_param   ALLOC関数で作成した引数
 *	@param	*p_wk_adrs      WBM_SUBPROC_Initで与えたワークアドレス
 *
 *	@return TRUEで終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL BC_CORE_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{ 
  BC_SYS_WORK *p_wk = p_wk_adrs;
  BATTLE_CHAMPIONSHIP_CORE_PARAM  *p_param  = p_param_adrs;
 
  switch( p_param->ret )
  { 
  case BATTLE_CHAMPIONSHIP_CORE_RET_TITLE:   //タイトルへ行く
    p_wk->next  = BC_SYS_NEXT_TITLE;
    BC_SYS_SUBPROC_End( p_wk->p_subproc );
    break;
  case BATTLE_CHAMPIONSHIP_CORE_RET_WIFICUP: //WIFI大会へ行く
    p_wk->next  = BC_SYS_NEXT_WIFICUP;
    BC_SYS_SUBPROC_End( p_wk->p_subproc );
    break;
  case BATTLE_CHAMPIONSHIP_CORE_RET_LIVEBTL: //LIVE用バトルへ行く
    break;
  case BATTLE_CHAMPIONSHIP_CORE_RET_LIVEREC: //LIVE用録画へ行く
    break;
  }

  GFL_HEAP_FreeMemory( p_param );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  録画用引数作成
 *
 *	@param	HEAPID heapID   ヒープID
 *	@param	*p_wk_adrs      WBM_SUBPROC_Initで与えたワークアドレス
 *
 *	@return 作成したパラメータ
 */
//-----------------------------------------------------------------------------
static void *BC_BTLREC_AllocParam( HEAPID heapID, void *p_wk_adrs )
{ 
  BC_SYS_WORK         *p_wk = p_wk_adrs;
  BTL_REC_SEL_PARAM   *p_param;
  p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(BTL_REC_SEL_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(BTL_REC_SEL_PARAM) );

  p_param->gamedata   = p_wk->p_gamedata;
  p_param->b_rec      = TRUE;

  //if( p_wk->p_enemy_data->btl_server_version != p_wk->p_player_data->btl_server_version )
  { 
    p_param->b_rec        = FALSE;
  }

  return p_param;

}
//----------------------------------------------------------------------------
/**
 *	@brief  録画用引数破棄
 *
 *	@param	void *p_param   ALLOC関数で作成した引数
 *	@param	*p_wk_adrs      WBM_SUBPROC_Initで与えたワークアドレス
 *
 *	@return TRUEで終了  FALSEで続行
 */
//-----------------------------------------------------------------------------
static BOOL BC_BTLREC_FreeParam( void *p_param_adrs, void *p_wk_adrs )
{ 
  BC_SYS_WORK       *p_wk = p_wk_adrs;
  BTL_REC_SEL_PARAM *p_param  = p_param_adrs;

  GFL_HEAP_FreeMemory( p_param );

  //録画バッファ終了
 // BattleRec_Exit();

  //次のPROC
 // p_wk->core_mode = WIFIBATTLEMATCH_CORE_MODE_ENDREC;
//  WBM_SYS_SUBPROC_CallProc( p_wk->p_subproc, SUBPROCID_CORE );

  return TRUE;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  サブプロセス
 *				    ・プロセスを行き来するシステム
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
#define BC_SYS_SUBPROC_END  0xFF
//-------------------------------------
///	ワーク
//=====================================
struct _BC_SYS_SUBPROC_WORK
{
	GFL_PROCSYS			  *p_procsys;
	void						  *p_proc_param;

	HEAPID					  heapID;
	void						  *p_wk_adrs;
	const BC_SYS_SUBPROC_DATA			*cp_procdata_tbl;
  u32               tbl_len;

	u8							  next_procID;
	u8							  now_procID;
	u16							  seq;

  GFL_PROC_MAIN_STATUS  status;
} ;
//-------------------------------------
///	外部公開
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	BC_SYS_SUBPROCシステム	初期化
 *
 *	@param	BC_SYS_SUBPROC_WORK *p_wk	ワーク
 *	@param	cp_procdata_tbl			プロセス接続テーブル
 *	@param	void *p_wk_adrs			アロック関数と解放関数に渡すワーク
 *	@param	heapID							システム構築用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static BC_SYS_SUBPROC_WORK * BC_SYS_SUBPROC_Init( const BC_SYS_SUBPROC_DATA *cp_procdata_tbl, u32 tbl_len, void *p_wk_adrs, HEAPID heapID )
{	
  BC_SYS_SUBPROC_WORK *p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BC_SYS_SUBPROC_WORK ) );
	GFL_STD_MemClear( p_wk, sizeof(BC_SYS_SUBPROC_WORK) );
	p_wk->p_procsys				= GFL_PROC_LOCAL_boot( heapID );
	p_wk->p_wk_adrs				= p_wk_adrs;
	p_wk->cp_procdata_tbl	= cp_procdata_tbl;
  p_wk->tbl_len         = tbl_len;
	p_wk->heapID					= heapID;

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	BC_SYS_SUBPROCシステム	破棄
 *
 *	@param	BC_SYS_SUBPROC_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BC_SYS_SUBPROC_Exit( BC_SYS_SUBPROC_WORK *p_wk )
{	
	GF_ASSERT( p_wk->p_proc_param == NULL );

	GFL_PROC_LOCAL_Exit( p_wk->p_procsys );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BC_SYS_SUBPROCシステム	メイン処理
 *
 *	@param	BC_SYS_SUBPROC_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば終了	FALSEならばPROCが存在する
 */
//-----------------------------------------------------------------------------
static BOOL BC_SYS_SUBPROC_Main( BC_SYS_SUBPROC_WORK *p_wk )
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
		if( p_wk->now_procID == p_wk->next_procID )
		{	
      GF_ASSERT( 0 ); //次のプロセスがセットされていない
		}
		else if( p_wk->next_procID == BC_SYS_SUBPROC_END )
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
 *	@brief  BC_SYS_SUBPROCシステム PROC状態を取得
 *
 *	@param	const BC_SYS_SUBPROC_WORK *cp_wk   ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static GFL_PROC_MAIN_STATUS BC_SYS_SUBPROC_GetStatus( const BC_SYS_SUBPROC_WORK *cp_wk )
{ 
  return cp_wk->status;
}

//----------------------------------------------------------------------------
/**
 *	@brief	BC_SYS_SUBPROCシステム	プロセスリクエスト
 *
 *	@param	BC_SYS_SUBPROC_WORK *p_wk	ワーク
 *	@param	proc_id							呼ぶプロセスID
 *
 */
//-----------------------------------------------------------------------------
static void BC_SYS_SUBPROC_CallProc( BC_SYS_SUBPROC_WORK *p_wk, u32 procID )
{	
	p_wk->next_procID	= procID;
}

//----------------------------------------------------------------------------
/**
 *	@brief	BC_SYS_SUBPROCシステム	終了
 *
 *	@param	BC_SYS_SUBPROC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BC_SYS_SUBPROC_End( BC_SYS_SUBPROC_WORK *p_wk )
{ 
  p_wk->next_procID = BC_SYS_SUBPROC_END;
}
