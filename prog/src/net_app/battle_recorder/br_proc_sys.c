//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_proc.c
 *	@brief	バトルレコーダープロセス繋がり管理
 *	@author	Toru=Nagihashi
 *	@data		2009.11.12
 *
 *	スタック式プロセス繋がり管理です。
 *	外で設定したテーブルのインデックスを渡すことで、開始し、
 *	プロセス内で、PUSHorPOPすることで、繋がっていきます。
 *	引数は保持されず、以前のPROCIDのみ保存します
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID

//外部情報
#include "br_proc_sys.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define DEBUG_BR_PROCSYS_PRINT_ON //担当者のみのプリントON
#endif //PM_DEBUG

//担当者のみのPRINTオン
#ifdef DEBUG_BR_PROCSYS_PRINT_ON
#if defined( DEBUG_ONLY_FOR_toru_nagihashi )
#define BR_PROCSYS_Printf(...)  OS_TFPrintf(1,__VA_ARGS__)
#elif defined( DEBUG_ONLY_FOR_shimoyamada )
#define BR_PROCSYS_Printf(...)  OS_TPrintf(__VA_ARGS__)
#endif  //defined
#endif //DEBUG_BR_PROCSYS_PRINT_ON
//定義されていないときは、なにもない
#ifndef BR_PROCSYS_Printf
#define BR_PROCSYS_Printf(...)  /*  */ 
#endif //BR_PROCSYS_Printf

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス情報	
//=====================================
typedef struct _BR_PROC_SYS_WORK
{
	BOOL											is_use;
	void											*p_param;
	const BR_PROC_SYS_DATA		*cp_data;
	u32												procID;
} BR_PROC_WORK;

//-------------------------------------
///	プロセス繋がり管理ワーク
//=====================================
struct _BR_PROC_SYS
{
	GFL_PROCSYS					*p_procsys;
	const BR_PROC_SYS_DATA	*cp_procdata_tbl;
	u16									tbl_max;
	void								*p_wk_adrs;
	HEAPID							heapID;
	BOOL								is_end;
	u32									seq;

	BR_PROC_WORK				prev_proc;
	BR_PROC_WORK				now_proc;
	BR_PROC_WORK				next_proc;
	u32									stackID[BR_PROC_SYS_STACK_MAX];
	u32									stack_num;

  BR_PROC_SYS_RECOVERY_DATA *p_recovery;
  BOOL                is_recovery;
};
//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
static void BR_PROC_Set( BR_PROC_WORK *p_wk, u16 procID, const BR_PROC_SYS_DATA	*cp_data );
static void BR_PROC_Clear( BR_PROC_WORK *p_wk );

//=============================================================================
/**
 *					システム
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	プロセス管理	初期化
 *
 *	@param	u16 startID		開始PROC
 *	@param	BR_PROC_SYS_DATA *cp_procdata_tbl	プロセステーブル
 *	@param	tbl_max												テーブル最大
 *	@param	p_wk_adrs											ワークアドレス（ALLOC,FREE_FUNCに渡す値）
 *	@param  p_recovery                    復帰データ
 *	@param	heapID												ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_PROC_SYS * BR_PROC_SYS_Init( u16 startID, const BR_PROC_SYS_DATA *cp_procdata_tbl, u16 tbl_max, void *p_wk_adrs, BR_PROC_SYS_RECOVERY_DATA *p_recovery, HEAPID heapID )
{	
	BR_PROC_SYS *p_wk;

	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(BR_PROC_SYS) );
	GFL_STD_MemClear( p_wk, sizeof(BR_PROC_SYS) );
	p_wk->heapID					= heapID;
	p_wk->tbl_max					= tbl_max;
	p_wk->p_wk_adrs				= p_wk_adrs;
	p_wk->cp_procdata_tbl	= cp_procdata_tbl;
  p_wk->p_recovery      = p_recovery;

	p_wk->p_procsys				= GFL_PROC_LOCAL_boot( heapID );


	BR_PROC_Clear( &p_wk->now_proc );
	BR_PROC_Clear( &p_wk->next_proc );
	BR_PROC_Clear( &p_wk->prev_proc );

  if( p_wk->p_recovery->stack_num )
  {
    //復帰データがあった場合は復帰
    int i;
    for( i = 0; i < p_wk->p_recovery->stack_num; i++ )
    { 
      BR_PROC_SYS_Push( p_wk, p_wk->p_recovery->stackID[i] );
    }

    //復帰したのでデータクリア
    GFL_STD_MemClear( p_wk->p_recovery, sizeof(BR_PROC_SYS_RECOVERY_DATA) );

    //リカバリーフラグON
    p_wk->is_recovery = TRUE;
  }
  else
  { 
    //なかった場合
    //最初のプロセスを積む
    BR_PROC_SYS_Push( p_wk, startID );
  }
	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロセス管理	破棄
 *
 *	@param	BR_PROC_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_PROC_SYS_Exit( BR_PROC_SYS *p_wk )
{	
	GFL_PROC_LOCAL_Exit( p_wk->p_procsys );
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロセス管理	メイン処理
 *
 *	@param	BR_PROC_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_PROC_SYS_Main( BR_PROC_SYS *p_wk )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_PROC_BEFORE,
		SEQ_PROC_MAIN,
		SEQ_PROC_AFTER,
		SEQ_END,
	};
	BR_PROC_WORK	*p_now	= &p_wk->now_proc;
	BR_PROC_WORK	*p_prev	= &p_wk->prev_proc;
	BR_PROC_WORK	*p_next	= &p_wk->next_proc;


	switch( p_wk->seq )
	{	
	case SEQ_INIT:

    BR_PROCSYS_Printf( "prev %d\n", p_prev->procID );
    BR_PROCSYS_Printf( "now %d\n", p_now->procID );
    BR_PROCSYS_Printf( "next %d\n", p_next->procID );

		//データを保存する
		*p_prev	= *p_now;
		*p_now	=	*p_next;

		p_wk->seq	= SEQ_PROC_BEFORE;
		break;

	case SEQ_PROC_BEFORE:
		GF_ASSERT_HEAVY( p_now->is_use );
		GF_ASSERT_HEAVY( p_now->cp_data->before_func );
		//前処理関数を呼ぶ
		p_now->p_param	= GFL_HEAP_AllocMemory( p_wk->heapID, p_now->cp_data->param_size );
    GFL_STD_MemClear( p_now->p_param, p_now->cp_data->param_size );
    {
      u32 preID;
      if( p_wk->is_recovery )
      { 
        preID = BR_PROC_SYS_RECOVERY_ID;
      }
      else
      { 
        preID = p_prev->procID;
      }
      p_now->cp_data->before_func( p_now->p_param, p_wk->p_wk_adrs, p_prev->p_param, preID );

      //フラグを渡し終えたので復帰OFF
      p_wk->is_recovery = FALSE;
    }
		//以前のデータのパラメータを消す
		if( p_prev->p_param )
		{	
			GFL_HEAP_FreeMemory( p_prev->p_param );
		}

		//プロセスを呼ぶ
		GFL_PROC_LOCAL_CallProc( p_wk->p_procsys, p_now->cp_data->ov_id,
					p_now->cp_data->cp_proc_data, p_now->p_param );	

		p_wk->seq	= SEQ_PROC_MAIN;
		break;

	case SEQ_PROC_MAIN:
		//プロセス処理中
		{	
			GFL_PROC_MAIN_STATUS	result;
			result	= GFL_PROC_LOCAL_Main( p_wk->p_procsys );
			if( result == GFL_PROC_MAIN_NULL )
			{	
				p_wk->seq	= SEQ_PROC_AFTER;
			}
		}
		break;

	case SEQ_PROC_AFTER:
		GF_ASSERT_HEAVY( p_now->is_use );
		//後処理関数を呼ぶ
		if( p_now->cp_data->after_func )
		{	
			p_now->cp_data->after_func( p_now->p_param, p_wk->p_wk_adrs );
		}

		//もしスタックに積んであったら、次のプロセスへ
		//スタックに積んでなかったら、終了
		if( p_wk->stack_num > 0 )
		{	
			p_wk->seq	= SEQ_INIT;
		}
		else
		{	
			//以前のデータのパラメータを消す
			if( p_now->p_param )
			{	
				GFL_HEAP_FreeMemory( p_now->p_param );
			}
			p_wk->seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		p_wk->is_end	= TRUE;
		break;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロセス管理	終了チェック
 *
 *	@param	const BR_PROC_SYS *cp_wk		ワーク
 *
 *	@return	TRUEで終了	FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL BR_PROC_SYS_IsEnd( const BR_PROC_SYS *cp_wk )
{	
	return cp_wk->is_end;
}
//=============================================================================
/**
 *						PROCの中で使う
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	PROC内関数	パラメータ取得
 *
 *	@param	BR_PROC_SYS *p_wk ワーク
 *
 *	@return	パラメータ取得
 */
//-----------------------------------------------------------------------------
void * BR_PROC_SYS_GetParam( const BR_PROC_SYS *cp_wk )
{	
	return cp_wk->now_proc.p_param;
}
//----------------------------------------------------------------------------
/**
 *	@brief	PROC内関数	ヒープID取得
 *
 *	@param	const BR_PROC_SYS *cp_wk	ワーク
 *
 *	@return	HEAPID	ヒープID
 */
//-----------------------------------------------------------------------------
HEAPID BR_PROC_SYS_GetHeapID( const BR_PROC_SYS *cp_wk )
{	
	return cp_wk->heapID;
}
//----------------------------------------------------------------------------
/**
 *	@brief	PROC内関数	スタックから降ろす
 *
 *	@param	BR_PROC_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_PROC_SYS_Pop( BR_PROC_SYS *p_wk )
{	

  if( p_wk->stack_num > 0 )
  { 
    p_wk->stack_num--;

    if( p_wk->stack_num != 0 )
    { 
      const u16 procID	= p_wk->stackID[ p_wk->stack_num -1 ];
      const BR_PROC_SYS_DATA	*cp_data	= &p_wk->cp_procdata_tbl[procID];
      BR_PROC_WORK	proc;

      BR_PROC_Set( &proc, procID, cp_data );

      p_wk->next_proc	= proc;
    }
    else
    {	
      BR_PROC_Clear( &p_wk->next_proc );
    }
	}
	else
	{	
		BR_PROC_Clear( &p_wk->next_proc );
	}

#ifdef PM_DEBUG
  { 
    int i;
    BR_PROCSYS_Printf( "Pop was called !!\n" );
    BR_PROCSYS_Printf( "積まれているSTACK\n" );
    for( i = 0; i < p_wk->stack_num; i++ )
    { 
      BR_PROCSYS_Printf( "%d ", p_wk->stackID[ i ] );
    }
    BR_PROCSYS_Printf( "\n" );
  }
#endif 

}
//----------------------------------------------------------------------------
/**
 *	@brief	PROC内関数	スタックに積む
 *
 *	@param	BR_PROC_SYS *p_wk ワーク
 *	@param	procID			スタックに積むID
 */
//-----------------------------------------------------------------------------
void BR_PROC_SYS_Push( BR_PROC_SYS *p_wk, u16 procID )
{	
	BR_PROC_WORK	proc;
	const BR_PROC_SYS_DATA	*cp_data	= &p_wk->cp_procdata_tbl[procID];
	
	GF_ASSERT_HEAVY( procID < p_wk->tbl_max );

	//PROC作成
  BR_PROC_Set( &proc, procID, cp_data );

	//スタックに積む
	GF_ASSERT_HEAVY( p_wk->stack_num < BR_PROC_SYS_STACK_MAX );
	p_wk->stackID[ p_wk->stack_num ]	= procID;
	p_wk->stack_num++;

	//プロセスつなぎ情報
	p_wk->next_proc	= proc;

#ifdef PM_DEBUG
  { 
    int i;
    BR_PROCSYS_Printf( "Push was called !! %d \n", procID );
    BR_PROCSYS_Printf( "積まれているSTACK\n" );
    for( i = 0; i < p_wk->stack_num; i++ )
    { 
      BR_PROCSYS_Printf( "%d ", p_wk->stackID[ i ] );
    }
    BR_PROCSYS_Printf( "\n" );
  }
#endif 

}

//----------------------------------------------------------------------------
/**
 *	@brief  COREPROCを中断し、mainに一旦帰る
 *
 *	@param	BR_PROC_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_PROC_SYS_Interruput( BR_PROC_SYS *p_wk )
{ 
  int i;

  //復帰データ保存
  GFL_STD_MemCopy( p_wk->stackID, p_wk->p_recovery->stackID, sizeof(u32)*BR_PROC_SYS_STACK_MAX );
  p_wk->p_recovery->stack_num = p_wk->stack_num;

  //全部POP
  BR_PROC_SYS_Abort( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  COREPROCを強制終了させる
 *
 *	@param	BR_PROC_SYS *p_wk wワーク
 */
//-----------------------------------------------------------------------------
void BR_PROC_SYS_Abort( BR_PROC_SYS *p_wk )
{ 
  //全部POP
  while( p_wk->stack_num > 0 )
  { 
    BR_PROC_SYS_Pop( p_wk );
  }
}
//=============================================================================
/**
 *							PRIVATE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  プログラム情報設定
 *
 *	@param	BR_PROC_WORK *p_wk          ワーク
 *	@param  procID                    　プロセスID
 *	@param	BR_PROC_SYS_DATA	*cp_data  データ
 */
//-----------------------------------------------------------------------------
static void BR_PROC_Set( BR_PROC_WORK *p_wk, u16 procID, const BR_PROC_SYS_DATA	*cp_data )
{ 
  GFL_STD_MemClear( p_wk, sizeof(BR_PROC_WORK) );
  p_wk->is_use		= TRUE;
  p_wk->p_param	  = NULL;	//引数は実際の移動時に作成される
  p_wk->procID		= procID;
  p_wk->cp_data	  = cp_data;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロセス情報を消去
 *
 *	@param	BR_PROC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_PROC_Clear( BR_PROC_WORK *p_wk )
{	
	GFL_STD_MemClear( p_wk, sizeof(BR_PROC_WORK) );
	p_wk->procID	= BR_PROC_SYS_NONE_ID;
}
