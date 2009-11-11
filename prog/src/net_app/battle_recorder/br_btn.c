//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_btn.c
 *	@brief	バトルレコーダーボタン管理
 *	@author	Toru=Nagihashi
 *	@data		2009.11.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID

//自分のモジュール
#include "br_btn_data.h"
#include "br_btn.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン移動タイプ
//=====================================
typedef enum
{
	BR_BTN_MOVE_PUSH,		//押されたボタンの処理					target=NULL
	BR_BTN_MOVE_HIDE,		//押されたボタンの格納する処理	target=PUSH
	BR_BTN_MOVE_APPEAR,	//ボタン出現処理								target=NULL
	BR_BTN_MOVE_TAG,		//他のタグを吐き出した後上へ登るtarget=TAG(あれば)
} BR_BTN_MOVE;
//-------------------------------------
///	ボタン移動タイプ
//=====================================
typedef enum
{
	BR_BTN_PARAM_TYPE,				//ボタンのタイプ	（BR_BTN_TYPE）
	BR_BTN_PARAM_DATA,				//タイプによったデータ
} BR_BTN_PARAM;

//-------------------------------------
///	ボタン管理システム
//=====================================
//管理スタック数
#define BR_BTN_SYS_STACK_MAX	(3)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ボタン単位
//=====================================
typedef struct 
{
	BOOL							is_use;		//使用フラグ
	GFL_CLWK					*p_clwk;	//CLWK
	const BR_BTN_DATA *cp_data;	//ボタンのデータ
} BR_BTN_WORK;

//-------------------------------------
///	バトルレコーダー　ボタン管理
//=====================================
struct _BR_BTN_SYS_WORK
{

	BR_BTN_SYS_STATE	state;	//ボタン管理の状態
	BR_BTN_SYS_INPUT	input;	//ボタン入力状態

	u32					trg_btn;			//押したボタン

	u8					btn_now_max;	//現在のボタンバッファ最大
	u8					btn_now_num;	//現在のボタン数
	u8					btn_stack_max;//ボタンスタックバッファ最大
	u8					btn_stack_num;//ボタンスタック現在数
	BR_BTN_WORK *p_btn_stack;	//スタックに積むボタンバッファ
	BR_BTN_WORK	*p_btn_now;		//現在のボタンバッファ
};
//=============================================================================
/**
 *					PRIVATE関数
*/
//=============================================================================
//-------------------------------------
///	ボタンシステムのプライベート
//=====================================
static void Br_Btn_Sys_PushStack( BR_BTN_SYS_WORK *p_wk, BR_BTN_WORK *p_btn );
static void Br_Btn_Sys_ReLoadBtn( BR_BTN_SYS_WORK *p_wk, BR_MENUID menuID );

//-------------------------------------
///	BTNの処理
//=====================================
static void BR_BTN_Init( BR_BTN_WORK *p_wk, const BR_BTN_DATA *cp_data, GFL_CLUNIT *p_unit, HEAPID heapID );
static void BR_BTN_Exit( BR_BTN_WORK *p_wk );
static BOOL BR_BTN_GetTrg( const BR_BTN_WORK *cp_wk );
static void BR_BTN_StartMove( BR_BTN_WORK *p_wk, BR_BTN_MOVE move, const BR_BTN_WORK *cp_target );
static BOOL BR_BTN_MainMove( BR_BTN_WORK *p_wk );
static void BR_BTN_Copy( const BR_BTN_WORK *cp_wk, BR_BTN_WORK *p_wk );
static u32 BR_BTN_GetParam( const BR_BTN_WORK *cp_wk, BR_BTN_PARAM param );

//=============================================================================
/**
 *					PUBILIC関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理初期化
 *
 *  @param	mode						起動モード
 *  @param	GFL_CLUNIT			ユニット
 *	@param	HEAPID heapID		ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_WORK *BR_BTN_SYS_Init( BR_MODE mode, GFL_CLUNIT *p_unit, HEAPID heapID )
{	
	BR_BTN_SYS_WORK *p_wk;

	//ワーク作成
	{	
		p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(BR_BTN_SYS_WORK) );
		GFL_STD_MemClear( p_wk, sizeof(BR_BTN_SYS_WORK) );
	}

	//リソース読み込み
	{	
		
	}

	//ボタンバッファを作成
	{	
		u32 size;
		
		p_wk->btn_now_max	= BR_BTN_DATA_SYS_GetDataMax();
		size = sizeof(BR_BTN_WORK) * p_wk->btn_now_max;
		p_wk->p_btn_now	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk->p_btn_now, size );
	}

	//スタックバッファを作成
	{	
		u32 size;

		p_wk->btn_stack_max	= BR_BTN_SYS_STACK_MAX;
		size	= sizeof(BR_BTN_WORK) * BR_BTN_SYS_STACK_MAX;
		p_wk->p_btn_stack	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk->p_btn_stack, size );
	}

	//最初に読み込むボタンを初期化
	{	
		int i;
		int max;
		const BR_BTN_DATA *	cp_data;

		BR_MENUID	startID;

		//初期メニュー取得
		startID	= BR_BTN_DATA_SYS_GetStartMenuID( mode );

		p_wk->btn_now_num	= BR_BTN_DATA_SYS_GetDataNum( mode );
		for( i = 0; i < p_wk->btn_now_num; i++ )
		{	
			cp_data	= BR_BTN_DATA_SYS_GetData( mode, i );
			BR_BTN_Init( &p_wk->p_btn_now[i], cp_data, p_unit, heapID );
		}
	}

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void BR_BTN_SYS_Exit( BR_BTN_SYS_WORK *p_wk )
{	
	//スタックバッファ破棄
	{	
		int i;
		for( i = 0; i < p_wk->btn_stack_num; i++ )
		{	
			BR_BTN_Exit( &p_wk->p_btn_stack[i] );
		}
	}
	GFL_HEAP_FreeMemory( p_wk->p_btn_stack );

	//ボタンバッファ破棄
	{	
		int i;
		for( i = 0; i < p_wk->btn_now_num; i++ )
		{	
			BR_BTN_Exit( &p_wk->p_btn_now[i] );
		}
	}
	GFL_HEAP_FreeMemory( p_wk->p_btn_now );
	
	//リソース破棄
	{	

	}

	//ワーク破棄
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理メイン処理
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void BR_BTN_SYS_Main( BR_BTN_SYS_WORK *p_wk )
{
	switch( p_wk->state )
	{	
	case BR_BTN_SYS_STATE_WAIT:	
		{	
			int i;
			BOOL is_trg	= FALSE;


			p_wk->input	= BR_BTN_SYS_INPUT_NONE;

			//ボタン押し検知
			for( i = 0; i < p_wk->btn_now_max; i++ )
			{	
				if( BR_BTN_GetTrg( &p_wk->p_btn_now[i] ) )
				{	
					p_wk->trg_btn	= i;
					is_trg				= TRUE;
					break;
				}
			}

			//ボタン動作開始
			if( is_trg )
			{	
				for( i = 0; i < p_wk->btn_now_max; i++ )
				{	
					if( p_wk->trg_btn == i )
					{	
						BR_BTN_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_PUSH, NULL );
					}
					else
					{	
						BR_BTN_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_HIDE, &p_wk->p_btn_now[p_wk->trg_btn] );
					}
				}

				p_wk->state	= BR_BTN_SYS_STATE_HANGER_MOVE;
			}
		}
		break;

	case BR_BTN_SYS_STATE_HANGER_MOVE:
		//ボタンが隠れるまでの動作
		{	
			int i;
			BOOL is_end	= FALSE;

			for( i = 0; i < p_wk->btn_now_max; i++ )
			{	
				is_end	&= BR_BTN_MainMove( &p_wk->p_btn_now[i] );
			}

			if( is_end )
			{
				p_wk->state	= BR_BTN_SYS_STATE_BTN_CHANGE;
			}
		}
		break;
	
	case BR_BTN_SYS_STATE_BTN_CHANGE:
		//決定したボタンをスタックに積み、クリア
		Br_Btn_Sys_PushStack( p_wk, &p_wk->p_btn_now[p_wk->trg_btn] );
		BR_BTN_Exit( &p_wk->p_btn_now[p_wk->trg_btn] );
		//他のボタンを読み変える
		Br_Btn_Sys_ReLoadBtn( p_wk, 0 );	//@todo
		{	
			int i;

			//ボタン出現処理
			for( i = 0; i < p_wk->btn_now_max; i++ )
			{	
				BR_BTN_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_APPEAR, NULL );
			}

			//スタックに積んだ、決定ボタンは上部へ移動
			BR_BTN_StartMove( &p_wk->p_btn_stack[p_wk->btn_stack_num-1], BR_BTN_MOVE_TAG, NULL );
		}
		p_wk->state	= BR_BTN_SYS_STATE_APPEAR_MOVE;
		break;

	case BR_BTN_SYS_STATE_APPEAR_MOVE:
		{	
			int i;
			BOOL is_end	= FALSE;

			for( i = 0; i < p_wk->btn_now_max; i++ )
			{	
				is_end	&= BR_BTN_MainMove( &p_wk->p_btn_now[i] );
			}
			for( i = 0; i < p_wk->btn_stack_max; i++ )
			{	
				is_end	&= BR_BTN_MainMove( &p_wk->p_btn_stack[i] );
			}
			if( is_end )
			{
				p_wk->state	= BR_BTN_SYS_STATE_BTN_CHANGE;
			}
		}
		break;

	case BR_BTN_SYS_STATE_INPUT:
		if( 0 )
		{	
			p_wk->input	= BR_BTN_SYS_INPUT_CHANGESEQ;
		}
		p_wk->state	= BR_BTN_SYS_STATE_WAIT;
		break;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理	入力状態取得
 *
 *	@param	cp_wk	ワーク
 *	@param	p_seq	BR_BTN_SYS_INPUT_CHANGESEQのとき、次のしーけんす
 *
 *	@return	入力の種類
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_INPUT BR_BTN_SYS_GetInput( const BR_BTN_SYS_WORK *cp_wk, u32 *p_seq )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理	状態取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	現在の状態
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_STATE BR_BTN_SYS_GetState( const BR_BTN_SYS_WORK *cp_wk )
{	
	return cp_wk->state;
}
//=============================================================================
/**
 *		ボタン管理システムのプライベート
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン管理システムにボタンを積む
 *
 *	@param	BR_BTN_SYS_WORK *p_wk		ワーク
 *	@param	*p_btn									スタックに積むボタン
 */
//-----------------------------------------------------------------------------
static void Br_Btn_Sys_PushStack( BR_BTN_SYS_WORK *p_wk, BR_BTN_WORK *p_btn )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンを破棄し、際読み込みする
 *
 *	@param	BR_BTN_SYS_WORK *p_wk ワーク
 *	@param	BR_MENUID							読み込むボタンのメニューID
 */
//-----------------------------------------------------------------------------
static void Br_Btn_Sys_ReLoadBtn( BR_BTN_SYS_WORK *p_wk, BR_MENUID menuID )
{	

}

//=============================================================================
/**
 *		ボタン１つ１つの処理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン初期化
 *
 *	@param	BR_BTN_WORK *p_wk			ワーク
 *	@param	BR_BTN_DATA *cp_data	ボタン情報
 *	@param	*p_unit								OBJ作成用ユニット
 *	@param	heapID								ヒープID
 */
//-----------------------------------------------------------------------------
static void BR_BTN_Init( BR_BTN_WORK *p_wk, const BR_BTN_DATA *cp_data, GFL_CLUNIT *p_unit, HEAPID heapID )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン破棄
 *
 *	@param	BR_BTN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_BTN_Exit( BR_BTN_WORK *p_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンをタッチしたかどうか
 *
 *	@param	const BR_BTN_WORK *cp_wk	ワーク
 *
 *	@return	TRUEならば押した	FALSEならば押していない
 */
//-----------------------------------------------------------------------------
static BOOL BR_BTN_GetTrg( const BR_BTN_WORK *cp_wk )
{	
	
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの動作開始
 *
 *	@param	BR_BTN_WORK *p_wk				ワーク
 *	@param	move										動作
 *	@param	BR_BTN_WORK *cp_target	動作に必要なターゲっト
 */
//-----------------------------------------------------------------------------
static void BR_BTN_StartMove( BR_BTN_WORK *p_wk, BR_BTN_MOVE move, const BR_BTN_WORK *cp_target )
{	
	//@todo
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンのメイン動作
 *
 *	@param	BR_BTN_WORK *p_wk		ワーク
 *
 *	@return	TRUEならばボタンの動き終了	FALSEならば動き中
 */
//-----------------------------------------------------------------------------
static BOOL BR_BTN_MainMove( BR_BTN_WORK *p_wk )
{	
	//@todo
	return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンをコピー
 *
 *	@param	const BR_BTN_WORK *cp_wk	ワーク
 *	@param	*p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void BR_BTN_Copy( const BR_BTN_WORK *cp_wk, BR_BTN_WORK *p_wk )
{	
	//@todo
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの情報を受け取り
 *
 *	@param	const BR_BTN_WORK *cp_wk	ワーク
 *	@param	param		ボタンの情報インデックス
 *
 *	@return	ボタン情報
 */
//-----------------------------------------------------------------------------
static u32 BR_BTN_GetParam( const BR_BTN_WORK *cp_wk, BR_BTN_PARAM param )
{	
	u32 ret;
	switch( param )
	{	
	case BR_BTN_PARAM_TYPE:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_DATA );
		break;

	case BR_BTN_PARAM_DATA:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_TYPE );
		break;

	default:
		ret	= 0;
		GF_ASSERT(0);
	}

	return ret;
}
