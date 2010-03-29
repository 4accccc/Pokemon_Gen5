//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_proc.h
 *	@brief	バトルレコーダープロセス繋がり管理
 *	@author	Toru=Nagihashi
 *	@date		2009.11.12
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "system/main.h"  //HEAPID

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//一番最初のBR_PROC_SYS_BEFORE_FUNCTION関数ではpreIDからこの値が帰ってくる
#define BR_PROC_SYS_NONE_ID	    (0xFFFFFFFF)
//復帰時は以下のパラメータがpreIDから帰ってくる
#define BR_PROC_SYS_RECOVERY_ID	(0xFFFFFFFE)

#define BR_PROC_SYS_STACK_MAX	  (5)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス繋がり管理ワーク
//=====================================
typedef struct _BR_PROC_SYS BR_PROC_SYS;

//-------------------------------------
///	引数作成、破棄関数
//=====================================
typedef void (*BR_PROC_SYS_BEFORE_FUNCTION)( void *p_param_adrs, void *p_wk_adrs, const void *cp_pre_param, u32 preID );
typedef void (*BR_PROC_SYS_AFTER_FUNCTION)( void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	プロセステーブル作成構造体
//=====================================
typedef struct 
{
	const GFL_PROC_DATA						*cp_proc_data;	//プロセステーブル
	u32														param_size;			//引数のサイズ
  FSOverlayID                   ov_id;          //オーバーレイID
	BR_PROC_SYS_BEFORE_FUNCTION		before_func;		//プロセス開始前関数
	BR_PROC_SYS_AFTER_FUNCTION		after_func;			//プロセス終了後関数
} BR_PROC_SYS_DATA;

//-------------------------------------
///	プロセス復帰データ
//=====================================
typedef struct 
{
  u32	  stackID[BR_PROC_SYS_STACK_MAX];
  u32		stack_num;
} BR_PROC_SYS_RECOVERY_DATA;


//=============================================================================
/**
 *					PUBLIC関数
*/
//=============================================================================
//-------------------------------------
///	システム
//=====================================
extern BR_PROC_SYS * BR_PROC_SYS_Init( u16 startID, const BR_PROC_SYS_DATA *cp_procdata_tbl, u16 tbl_max, void *p_wk_adrs, BR_PROC_SYS_RECOVERY_DATA *p_recovery, HEAPID heapID );
extern void BR_PROC_SYS_Exit( BR_PROC_SYS *p_wk );
extern void BR_PROC_SYS_Main( BR_PROC_SYS *p_wk );
extern BOOL BR_PROC_SYS_IsEnd( const BR_PROC_SYS *cp_wk );

//-------------------------------------
///	PROCの中で使う関数
//		各PROCの引数にBR_PROC_SYSを渡し、以下のを使い操作してください
//=====================================
extern void * BR_PROC_SYS_GetParam( const BR_PROC_SYS *cp_wk ); //使わなくなった
extern HEAPID BR_PROC_SYS_GetHeapID( const BR_PROC_SYS *cp_wk );
extern void BR_PROC_SYS_Pop( BR_PROC_SYS *p_wk );
extern void BR_PROC_SYS_Push( BR_PROC_SYS *p_wk, u16 procID );

extern void BR_PROC_SYS_Interruput( BR_PROC_SYS *p_wk );
