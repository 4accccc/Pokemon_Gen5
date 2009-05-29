//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_aura_main.c
 *	@brief	赤外線ミニゲーム
 *	@author	Toru=Nagihashi
 *	@data		2009.05.11
 *
 *	このプロセスは、各アプリケーションプロセスを繋ぐ役割と
 *	アプリケーション間の情報のやりとりをするために存在する。
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>

//	system
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"

//	module
#include "net_app/compatible_irc_sys.h"

//	proc
#include "net_app/irc_menu.h"
#include "net_app/irc_aura.h"
#include "net_app/irc_rhythm.h"
#include "net_app/irc_result.h"

//	mine
#include "net_app/irc_compatible.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#ifdef defined DEBUG_ONLY_FOR_toru_nagihashi
#define DEBUG_RETURN_TO_PROG	//戻るボタンを押すと次へ進む
#endif	//DEBUG_ONLY_FOR_toru_nagihashi
#endif	//PM_DEBUG

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	サブプロック移動
//=====================================
typedef struct {
	GFL_PROCSYS			*p_procsys;
	u32							seq;
	void						*p_proc_param;
} SUBPROC_WORK;

//-------------------------------------
///	相性診断メインワーク
//=====================================
typedef struct _IRC_COMPATIBLE_MAIN_WORK IRC_COMPATIBLE_MAIN_WORK;
typedef void (*SEQ_FUNCTION)( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq );
struct _IRC_COMPATIBLE_MAIN_WORK
{
	//ネットモジュール
	COMPATIBLE_IRC_SYS	*p_irc;

	//サブプロセスモジュール
	SUBPROC_WORK				subproc;

	//相性診断パラメータ
	IRC_COMPATIBLE_PARAM	*p_param;

	//シーケンス管理
	SEQ_FUNCTION				seq_function;
	u16									seq;
	u16									dummy;
	BOOL								is_end;


	//PROC間データ
	IRCMENU_SELECT			select;	//メニューで選んだもの
	IRCAURA_RESULT			aura_result;
	IRCRHYTHM_RESULT		rhythm_result;
	u8									score;
	u8									dummy2[3];
	BOOL								is_init;
	MYSTATUS						*p_you_status;

};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//SEQ
static void SEQ_Change( IRC_COMPATIBLE_MAIN_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( IRC_COMPATIBLE_MAIN_WORK *p_wk );
//SEQ_FUNC
static void SEQFUNC_Start( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_End( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_MenuProc( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_CompatibleProc( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_DebugCompatibleProc( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq );
//PROCMODE
static void SUBPROC_Init( SUBPROC_WORK *p_wk, HEAPID heapID );
static BOOL SUBPROC_Main( SUBPROC_WORK *p_wk );
static void SUBPROC_Exit( SUBPROC_WORK *p_wk );
static BOOL SUBPROC_CallProcReq( SUBPROC_WORK *p_wk, u32 proc_id, HEAPID heapID, void *p_wk_adrs );
//PROCCHANE
typedef void *(*SUBPROC_ALLOC_FUNCTION)( HEAPID heapID, void *p_wk_adrs );
typedef void (*SUBPROC_FREE_FUNCTION)( void *p_param, void *p_wk_adrs );
static void *SUBPROC_ALLOC_Menu( HEAPID heapID, void *p_wk_adrs );
static void SUBPROC_FREE_Menu( void *p_param_adrs, void *p_wk_adrs );
static void *SUBPROC_ALLOC_Aura( HEAPID heapID, void *p_wk_adrs );
static void SUBPROC_FREE_Aura( void *p_param_adrs, void *p_wk_adrs );
static void *SUBPROC_ALLOC_Rhythm( HEAPID heapID, void *p_wk_adrs );
static void SUBPROC_FREE_Rhythm( void *p_param_adrs, void *p_wk_adrs );
static void *SUBPROC_ALLOC_Result( HEAPID heapID, void *p_wk_adrs );
static void SUBPROC_FREE_Result( void *p_param_adrs, void *p_wk_adrs );
//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	赤外線ミニゲームメニュー用プロックデータ
//=====================================
const GFL_PROC_DATA IrcCompatible_ProcData	= 
{	
	IRC_COMPATIBLE_PROC_Init,
	IRC_COMPATIBLE_PROC_Main,
	IRC_COMPATIBLE_PROC_Exit,
};
//-------------------------------------
///	PROC呼び出し
//=====================================
FS_EXTERN_OVERLAY(irc_menu);
FS_EXTERN_OVERLAY(irc_aura);
FS_EXTERN_OVERLAY(irc_rhythm);
FS_EXTERN_OVERLAY(irc_result);

//-------------------------------------
///	PROC移動データ
//=====================================
typedef enum
{	
	SUBPROCID_MENU,
	SUBPROCID_AURA,
	SUBPROCID_RHYTHM,
	SUBPROCID_RESULT,

	SUBPROCID_MAX,
	SUBPROCID_NULL	= SUBPROCID_MAX,
} SUBPROC_ID;
static const struct
{	
	FSOverlayID		ov_id;
	const GFL_PROC_DATA	*cp_procdata;
	SUBPROC_ALLOC_FUNCTION	alloc_func;
	SUBPROC_FREE_FUNCTION		free_func;
} sc_proc_data_tbl[SUBPROCID_MAX]	=
{	
	{	
		FS_OVERLAY_ID(irc_menu),
		&IrcMenu_ProcData,
		SUBPROC_ALLOC_Menu,
		SUBPROC_FREE_Menu,
	},
	{	
		FS_OVERLAY_ID(irc_aura),
		&IrcAura_ProcData,
		SUBPROC_ALLOC_Aura,
		SUBPROC_FREE_Aura,
	},
	{	
		FS_OVERLAY_ID(irc_rhythm),
		&IrcRhythm_ProcData,
		SUBPROC_ALLOC_Rhythm,
		SUBPROC_FREE_Rhythm,
	},
	{	
		FS_OVERLAY_ID(irc_result),
		&IrcResult_ProcData,
		SUBPROC_ALLOC_Result,
		SUBPROC_FREE_Result,
	}
};



//=============================================================================
/**
 *			PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_COMPATIBLE_MAIN_WORK	*p_wk;

//デバッグ時はNULLのときでも動く
//（デバッグからはGAMESYS_WORKを渡せないため）
#ifndef PM_DEBUG
	GF_ASSERT( p_param );
#endif //PM_DEBUG

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCCOMPATIBLE_SYSTEM, 0x08000 );
	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(IRC_COMPATIBLE_MAIN_WORK), HEAPID_IRCCOMPATIBLE_SYSTEM );
	GFL_STD_MemClear( p_wk, sizeof(IRC_COMPATIBLE_MAIN_WORK) );
	p_wk->p_param	= p_param;

	//通信相手データ用バッファ作成
	p_wk->p_you_status	= GFL_HEAP_AllocMemory( HEAPID_IRCCOMPATIBLE_SYSTEM, MyStatus_GetWorkSize() );
	GFL_STD_MemClear( p_wk->p_you_status, MyStatus_GetWorkSize() );

	//モジュール作成
	SUBPROC_Init( &p_wk->subproc, HEAPID_IRCCOMPATIBLE_SYSTEM );

	//0xFFFFFFFFは応急処理（仮）5月ROM焼きのあと、
	//毎回接続しなおす処理に直す。その際、最初の接続時にマックアドレスを貰いその人としか
	//繋がらないような処理にする
	p_wk->p_irc	= COMPATIBLE_IRC_CreateSystem( 0xFFFFFFFF, HEAPID_IRCCOMPATIBLE_SYSTEM );

	p_wk->is_init	= TRUE;

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
	if( p_wk->p_param->is_only_play )
	{	
		SEQ_Change( p_wk, SEQFUNC_DebugCompatibleProc );
	}
	else
#endif //DEBUG_IRC_COMPATIBLE_ONLYPLAY
	{	
		SEQ_Change( p_wk, SEQFUNC_Start );
	}

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセス破棄処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_COMPATIBLE_MAIN_WORK	*p_wk;

	p_wk	= p_work;

	//モジュール破棄
	COMPATIBLE_IRC_DeleteSystem( p_wk->p_irc );
	SUBPROC_Exit( &p_wk->subproc );
	
	//バッファ破棄
	GFL_HEAP_FreeMemory( p_wk->p_you_status );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );
	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_IRCCOMPATIBLE_SYSTEM );
	
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセスメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	BOOL ret;
	IRC_COMPATIBLE_MAIN_WORK	*p_wk;
	p_wk	= p_work;

	ret	= SUBPROC_Main( &p_wk->subproc );

	if( !ret )
	{	
		p_wk->seq_function( p_wk, &p_wk->seq );
	}

	if( p_wk->is_end )
	{
		return GFL_PROC_RES_FINISH;
	}
	else
	{	
		return GFL_PROC_RES_CONTINUE;
	}
}

//=============================================================================
/**
 *				SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQチェンジ
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk	ワーク
 *	@param	SEQ_FUNCTION										SEQ関数
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( IRC_COMPATIBLE_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC終了
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( IRC_COMPATIBLE_MAIN_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}
//=============================================================================
/**
 *				SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	開始シーケンス
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Start( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
	//	SEQ_NET_INIT,
		SEQ_CHANGE_MENU,
	};

	switch( *p_seq )
	{	
/*	case SEQ_NET_INIT:
		if( COMPATIBLE_IRC_InitWait( p_wk->p_irc ) )
		{	
			*p_seq	= SEQ_CHANGE_MENU;
		}
		break;
	*/	
	case SEQ_CHANGE_MENU:
		SEQ_Change( p_wk, SEQFUNC_MenuProc );
		break;
	};
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了シーケンス
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_NET_EXIT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_NET_EXIT:
		if( COMPATIBLE_IRC_ExitWait( p_wk->p_irc ) )
		{	
			*p_seq	= SEQ_END;
		}
		break;
		
	case SEQ_END:
		SEQ_End( p_wk );
		break;
	};
}

//----------------------------------------------------------------------------
/**
 *	@brief	メニューシーケンス
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk
 *	@param	*p_seq 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_MenuProc( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_PROC_MENU,
		SEQ_SELECT_MENU,
	};

	switch( *p_seq )
	{	
	case SEQ_PROC_MENU:
		if( SUBPROC_CallProcReq( &p_wk->subproc, SUBPROCID_MENU, HEAPID_IRCCOMPATIBLE_SYSTEM, p_wk ) )
		{	
			*p_seq	= SEQ_SELECT_MENU;
		}
		break;

	case SEQ_SELECT_MENU:
		switch( p_wk->select )
		{	
		case IRCMENU_SELECT_COMPATIBLE:
			SEQ_Change( p_wk, SEQFUNC_CompatibleProc );
			break;
		case IRCMENU_SELECT_RETURN:
			SEQ_Change( p_wk, SEQFUNC_End );
			break;
		default:
			GF_ASSERT(0);
		}
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	相性診断ゲームシーケンス
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_CompatibleProc( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_PROC_RHYTHM,
		SEQ_PROC_RHYTHM_RESULT,
		SEQ_PROC_AURA,
		SEQ_PROC_AURA_RESULT,
		SEQ_PROC_RESULT,
		SEQ_CHANGE_MENU,
	};

	switch( *p_seq )
	{	
	case SEQ_INIT:
		p_wk->score	= 0;
		*p_seq	= SEQ_PROC_RHYTHM;
		break;

	case SEQ_PROC_RHYTHM:
		if( SUBPROC_CallProcReq( &p_wk->subproc, SUBPROCID_RHYTHM, HEAPID_IRCCOMPATIBLE_SYSTEM, p_wk ) )
		{	
			*p_seq	= SEQ_PROC_RHYTHM_RESULT;
		}
		break;

	case SEQ_PROC_RHYTHM_RESULT:
#ifdef DEBUG_RETURN_TO_PROG
		if(1)
#else
		if( p_wk->rhythm_result == IRCRHYTHM_RESULT_CLEAR )
#endif	//DEBUG_RETURN_TO_PROG
		{	
			*p_seq	= SEQ_PROC_AURA;
		}
		else
		{	
			*p_seq	= SEQ_CHANGE_MENU;
		}
		break;

	case SEQ_PROC_AURA:
		if( SUBPROC_CallProcReq( &p_wk->subproc, SUBPROCID_AURA, HEAPID_IRCCOMPATIBLE_SYSTEM, p_wk ) )
		{	
			*p_seq	= SEQ_PROC_AURA_RESULT;
		}
		break;

	case SEQ_PROC_AURA_RESULT:
#ifdef DEBUG_RETURN_TO_PROG
		if(1)
#else
		if( p_wk->aura_result == IRCAURA_RESULT_CLEAR )
#endif //DEBUG_RETURN_TO_PROG
		{	
			*p_seq	= SEQ_PROC_RESULT;
		}
		else
		{	
			*p_seq	= SEQ_CHANGE_MENU;
		}
		break;

	case SEQ_PROC_RESULT:
		if( SUBPROC_CallProcReq( &p_wk->subproc, SUBPROCID_RESULT, HEAPID_IRCCOMPATIBLE_SYSTEM, p_wk ) )
		{	
			*p_seq	= SEQ_CHANGE_MENU;
		}
		break;

	case SEQ_CHANGE_MENU:
		SEQ_Change( p_wk, SEQFUNC_MenuProc );
		break;
	};
}
#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ相性PROCゲーム
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DebugCompatibleProc( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_PROC_RHYTHM,
		SEQ_PROC_AURA,
		SEQ_PROC_RESULT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_INIT:
		p_wk->score			= 0;
		*p_seq	= SEQ_PROC_RHYTHM;
		break;

	case SEQ_PROC_RHYTHM:
		if( SUBPROC_CallProcReq( &p_wk->subproc, SUBPROCID_RHYTHM, HEAPID_IRCCOMPATIBLE_SYSTEM, p_wk ) )
		{	
			*p_seq	= SEQ_PROC_AURA;
		}
		break;

	case SEQ_PROC_AURA:
		if( SUBPROC_CallProcReq( &p_wk->subproc, SUBPROCID_AURA, HEAPID_IRCCOMPATIBLE_SYSTEM, p_wk ) )
		{	
			*p_seq	= SEQ_PROC_RESULT;
		}
		break;

	case SEQ_PROC_RESULT:
		if( SUBPROC_CallProcReq( &p_wk->subproc, SUBPROCID_RESULT, HEAPID_IRCCOMPATIBLE_SYSTEM, p_wk ) )
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		SEQ_End( p_wk );
		break;
	};
}
#endif //DEBUG_IRC_COMPATIBLE_ONLYPLAY
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
 *	@param	heapID							システム構築用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_Init( SUBPROC_WORK *p_wk, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(SUBPROC_WORK) );
	p_wk->p_procsys	= GFL_PROC_LOCAL_boot( heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	SUBPROCシステム	メイン処理
 *
 *	@param	SUBPROC_WORK *p_wk	ワーク
 *
 *	@retval	TRUE	プロセスが存在する
 *	@retval	FALSE	プロセスが存在しない
 */
//-----------------------------------------------------------------------------
static BOOL SUBPROC_Main( SUBPROC_WORK *p_wk )
{	
	return GFL_PROC_LOCAL_Main( p_wk->p_procsys );
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
	GFL_PROC_LOCAL_Exit( p_wk->p_procsys );
	GFL_STD_MemClear( p_wk, sizeof(SUBPROC_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	SUBPROCシステム	プロセスリクエスト
 *
 *	@param	SUBPROC_WORK *p_wk	ワーク
 *	@param	proc_id							呼ぶプロセスID
 *	@param	heapID							プロセスへの引き数作成用ヒープID
 *	@param	*p_wk_adrs					プロセスへの引数作成用に渡す情報
 *
 *	@retval	TRUE	プロセスが終了した
 *	@retval	FALSE	まだプロセスが終了していない
 */
//-----------------------------------------------------------------------------
static BOOL SUBPROC_CallProcReq( SUBPROC_WORK *p_wk, u32 proc_id, HEAPID heapID, void *p_wk_adrs )
{	
	enum
	{	
		SEQ_ALLOC_PARAM,
		SEQ_CALL_PROC,
		SEQ_FREE_PARAM,
		SEQ_END,
	};

	switch( p_wk->seq )
	{	
	case SEQ_ALLOC_PARAM:
		p_wk->p_proc_param	= sc_proc_data_tbl[ proc_id ].alloc_func( heapID, p_wk_adrs );
		p_wk->seq	= SEQ_CALL_PROC;
		break;

	case SEQ_CALL_PROC:
		GFL_PROC_LOCAL_CallProc( p_wk->p_procsys, sc_proc_data_tbl[	proc_id ].ov_id,
				sc_proc_data_tbl[	proc_id ].cp_procdata, p_wk->p_proc_param );
		p_wk->seq	= SEQ_FREE_PARAM;
		break;

	case SEQ_FREE_PARAM:
		sc_proc_data_tbl[	proc_id ].free_func( p_wk->p_proc_param, p_wk_adrs );
		p_wk->seq	= SEQ_END;
		break;

	case SEQ_END:
		p_wk->seq	= 0;
		return TRUE;
	}

	return FALSE;
}
//=============================================================================
/**
 *				PROC移動用パラメータ作成
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	メニュープロック用パラメータ作成
 *
 *	@param	heapID													ヒープID
 *	@param	p_wk_adrs												ワークアドレス
 *
 *	@return	オーラチェックに渡すパラメータ
 */
//-----------------------------------------------------------------------------
static void *SUBPROC_ALLOC_Menu( HEAPID heapID, void *p_wk_adrs )
{	
	IRC_COMPATIBLE_MAIN_WORK *p_wk;
	IRC_MENU_PARAM	*p_param;

	p_wk	= p_wk_adrs;

	p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(IRC_MENU_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(IRC_MENU_PARAM));
	p_param->p_gamesys	= p_wk->p_param->p_gamesys;
	p_param->p_irc			= p_wk->p_irc;
	p_param->p_you_status	= p_wk->p_you_status;

	if( p_wk->is_init )
	{	
		p_param->mode				= IRCMENU_MODE_INIT;
		p_wk->is_init				= FALSE;
	}
	else
	{	
		p_param->mode				= IRCMENU_MODE_RETURN;
	}
	
	return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニュープロック用パラメータ破棄
 *
 *	@param	p_param													パラメータアドレス
 *	@param	p_wk_adrs												ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_FREE_Menu( void *p_param_adrs, void *p_wk_adrs )
{	
	IRC_COMPATIBLE_MAIN_WORK *p_wk;
	IRC_MENU_PARAM	*p_param;
	
	p_wk		= p_wk_adrs;
	p_param	= p_param_adrs;


	p_wk->select	= p_param->select;

	GFL_HEAP_FreeMemory( p_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	オーラチェックプロック用パラメータ作成
 *
 *	@param	heapID													ヒープID
 *	@param	p_wk_adrs												ワークアドレス
 *
 *	@return	オーラチェックに渡すパラメータ
 */
//-----------------------------------------------------------------------------
static void *SUBPROC_ALLOC_Aura( HEAPID heapID, void *p_wk_adrs )
{	
	IRC_COMPATIBLE_MAIN_WORK *p_wk;
	IRC_AURA_PARAM	*p_param;

	p_wk	= p_wk_adrs;

	p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(IRC_AURA_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(IRC_AURA_PARAM)) ;
	p_param->p_gamesys	= p_wk->p_param->p_gamesys;
	p_param->p_irc			= p_wk->p_irc;
	p_param->p_you_status	= p_wk->p_you_status;

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
	p_param->is_only_play	= p_wk->p_param->is_only_play;
#endif //DEBUG_IRC_COMPATIBLE_ONLYPLAY

	return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	オーラチェックプロック用パラメータ破棄
 *
 *	@param	p_param													パラメータアドレス
 *	@param	p_wk_adrs												ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_FREE_Aura( void *p_param_adrs, void *p_wk_adrs )
{	
	IRC_COMPATIBLE_MAIN_WORK *p_wk;
	IRC_AURA_PARAM	*p_param;
	
	p_wk		= p_wk_adrs;
	p_param	= p_param_adrs;

	p_wk->aura_result	= p_param->result;
	p_wk->score				+= p_param->score;

	GFL_HEAP_FreeMemory( p_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムチェックプロック用パラメータ作成
 *
 *	@param	heapID													ヒープID
 *	@param	p_wk_adrs												ワークアドレス
 *
 *	@return	リズムチェックに渡すパラメータ
 */
//-----------------------------------------------------------------------------
static void *SUBPROC_ALLOC_Rhythm( HEAPID heapID, void *p_wk_adrs )
{	
	IRC_COMPATIBLE_MAIN_WORK *p_wk;
	IRC_RHYTHM_PARAM	*p_param;

	p_wk	= p_wk_adrs;

	p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(IRC_RHYTHM_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(IRC_RHYTHM_PARAM)) ;
	p_param->p_gamesys	= p_wk->p_param->p_gamesys;
	p_param->p_irc			= p_wk->p_irc;
	p_param->p_you_status	= p_wk->p_you_status;

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
	p_param->is_only_play	= p_wk->p_param->is_only_play;
#endif //DEBUG_IRC_COMPATIBLE_ONLYPLAY

	return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムチェックプロック用パラメータ破棄
 *
 *	@param	p_param													パラメータアドレス
 *	@param	p_wk_adrs												ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_FREE_Rhythm( void *p_param_adrs, void *p_wk_adrs )
{	
	IRC_COMPATIBLE_MAIN_WORK *p_wk;
	IRC_RHYTHM_PARAM	*p_param;
	
	p_wk		= p_wk_adrs;
	p_param	= p_param_adrs;

	p_wk->rhythm_result	= p_param->result;
	p_wk->score					+= p_param->score;

	GFL_HEAP_FreeMemory( p_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果診断プロック用パラメータ作成
 *
 *	@param	heapID													ヒープID
 *	@param	p_wk_adrs												ワークアドレス
 *
 *	@return	リズムチェックに渡すパラメータ
 */
//-----------------------------------------------------------------------------
static void *SUBPROC_ALLOC_Result( HEAPID heapID, void *p_wk_adrs )
{	
	IRC_COMPATIBLE_MAIN_WORK *p_wk;
	IRC_RESULT_PARAM	*p_param;

	p_wk	= p_wk_adrs;

	p_param	= GFL_HEAP_AllocMemory( heapID, sizeof(IRC_RESULT_PARAM) );
	GFL_STD_MemClear( p_param, sizeof(IRC_RESULT_PARAM)) ;
	p_param->p_gamesys	= p_wk->p_param->p_gamesys;
	p_param->p_irc			= p_wk->p_irc;
	p_param->p_you_status	= p_wk->p_you_status;
	p_param->score			= p_wk->score / 2;

	return p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果診断プロック用パラメータ破棄
 *
 *	@param	p_param													パラメータアドレス
 *	@param	p_wk_adrs												ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void SUBPROC_FREE_Result( void *p_param_adrs, void *p_wk_adrs )
{	
	IRC_COMPATIBLE_MAIN_WORK *p_wk;
	IRC_RESULT_PARAM	*p_param;
	
	p_wk		= p_wk_adrs;
	p_param	= p_param_adrs;

	GFL_HEAP_FreeMemory( p_param );
}
