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

//文字表示
#include "print/printsys.h"

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
///	ボタン公開情報
//=====================================
typedef enum
{
	BR_BTN_PARAM_TYPE,				//ボタンのタイプ	（BR_BTN_TYPE）
	BR_BTN_PARAM_DATA1,				//タイプによったデータ
	BR_BTN_PARAM_DATA2,				//タイプによったデータ
	BR_BTN_PARAM_VALID,				//おせるかどうか
} BR_BTN_PARAM;

//-------------------------------------
///	
//=====================================
#define BR_BTN_OAMFONT_OFS_X	(4*8)
#define BR_BTN_OAMFONT_OFS_Y	(1*8)

//-------------------------------------
///	ボタン管理システム
//=====================================
//管理スタック数
#define BR_BTN_SYS_STACK_MAX	(3)
//上画面にボタンがいったときの座標
#define TAG_INIT_POS_UP( n )		( 25 + ( ( 36 ) * ( 4-n ) ) )

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	汎用ボタンワーク
//=====================================
struct _BR_BTN_WORK
{
	GFL_CLWK					*p_clwk;		//CLWK
	BMPOAM_ACT_PTR		p_oamfont;	//OAMフォント
	GFL_BMP_DATA			*p_bmp;			//フォント
	const BR_BTN_DATA *cp_data;		//ボタンのデータ
	CLSYS_DRAW_TYPE		display;		//どちらに描画するか
};

//-------------------------------------
///	ボタンシステムで管理する専用のボタンワーク
//=====================================
typedef struct 
{
	BOOL							is_use;			//使用フラグ
  CLSYS_DRAW_TYPE   display;    //どちらの画面か
	BR_BTN_WORK				*p_btn;			//ボタンワーク
	const BR_BTN_DATA *cp_data;		//ボタンのデータ
} BR_BTNEX_WORK;

//-------------------------------------
///	バトルレコーダー　ボタン管理
//=====================================
struct _BR_BTN_SYS_WORK
{
	HEAPID						heapID;
	BR_RES_WORK				*p_res;		//リソース
	GFL_CLUNIT				*p_unit;	//セルユニット

	BR_BTN_SYS_STATE	state;	//ボタン管理の状態
	BR_BTN_SYS_INPUT	input;	//ボタン入力状態
	BR_BTN_TYPE				next_type;//次の動作
	BOOL							next_valid;//次のボタンが押せるかどうか
	u32								next_proc;//次のプロセス
	u32								next_mode;//次のプロセスモード

	u32								trg_btn;			//押したボタン

	u8								btn_now_max;	//現在のボタンバッファ最大
	u8								btn_now_num;	//現在のボタン数
	u8								btn_stack_max;//ボタンスタックバッファ最大
	u8								btn_stack_num;//ボタンスタック現在数
	BR_BTNEX_WORK			*p_btn_stack;	//スタックに積むボタンバッファ
	BR_BTNEX_WORK			*p_btn_now;		//現在のボタンバッファ

	BR_BTN_DATA_SYS		*p_btn_data;	//ボタンデータバッファ
	BMPOAM_SYS_PTR		p_bmpoam;	//BMPOAMシステム
};
//=============================================================================
/**
 *					PRIVATE関数
*/
//=============================================================================
//-------------------------------------
///	ボタンシステムのプライベート
//=====================================
static void Br_Btn_Sys_PushStack( BR_BTN_SYS_WORK *p_wk, BR_BTNEX_WORK *p_btn );
static BOOL Br_Btn_Sys_PopStack( BR_BTN_SYS_WORK *p_wk, BR_BTNEX_WORK *p_btn );
static void Br_Btn_Sys_ReLoadBtn( BR_BTN_SYS_WORK *p_wk, BR_MENUID menuID );

//-------------------------------------
///	BTNの処理
//=====================================
static void BR_BTNEX_Init( BR_BTNEX_WORK *p_wk, const BR_BTN_DATA *cp_data, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, const BR_RES_WORK *cp_res, HEAPID heapID );
static void BR_BTNEX_Exit( BR_BTNEX_WORK *p_wk );
static BOOL BR_BTNEX_GetTrg( const BR_BTNEX_WORK *cp_wk, u32 x, u32 y );
static void BR_BTNEX_StartMove( BR_BTNEX_WORK *p_wk, BR_BTN_MOVE move, const BR_BTNEX_WORK *cp_target );
static BOOL BR_BTNEX_MainMove( BR_BTNEX_WORK *p_wk );
static void BR_BTNEX_Copy( const BR_BTN_SYS_WORK *cp_wk, BR_BTNEX_WORK *p_dst, const BR_BTNEX_WORK *cp_src, CLSYS_DRAW_TYPE	display );
static void BR_BTNEX_SetStackPos( BR_BTNEX_WORK *p_wk, u16 stack_num );
static u32 BR_BTNEX_GetParam( const BR_BTNEX_WORK *cp_wk, BR_BTN_PARAM param );

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
 *  @param	BR_RES_WORK			リソース
 *  @param	BR_BTLREC_SET		バトル録画情報
 *	@param	HEAPID heapID		ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_WORK *BR_BTN_SYS_Init( BR_MENUID menuID, GFL_CLUNIT *p_unit, BR_RES_WORK *p_res, const BR_BTLREC_SET *cp_rec, HEAPID heapID )
{	
	BR_BTN_SYS_WORK *p_wk;

	GF_ASSERT( menuID < BR_MENUID_MAX );

	//ワーク作成
	{	
		p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(BR_BTN_SYS_WORK) );
		GFL_STD_MemClear( p_wk, sizeof(BR_BTN_SYS_WORK) );
		p_wk->p_res				= p_res;
		p_wk->p_unit			= p_unit;
		p_wk->heapID			= heapID;
		p_wk->p_bmpoam		= BmpOam_Init( heapID, p_unit);
		p_wk->p_btn_data	= BR_BTN_DATA_SYS_Init( cp_rec, heapID );
	}

	//リソース読み込み
	{	
		BR_RES_LoadOBJ( p_res, BR_RES_OBJ_BROWSE_BTN_M, heapID );
		BR_RES_LoadOBJ( p_res, BR_RES_OBJ_BROWSE_BTN_S, heapID );
	}

	//ボタンバッファを作成
	{	
		u32 size;
		
		p_wk->btn_now_max	= BR_BTN_DATA_SYS_GetDataMax(p_wk->p_btn_data);
		size = sizeof(BR_BTNEX_WORK) * p_wk->btn_now_max;
		p_wk->p_btn_now	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk->p_btn_now, size );
	}

	//スタックバッファを作成
	{	
		u32 size;

		p_wk->btn_stack_max	= BR_BTN_SYS_STACK_MAX;
		size	= sizeof(BR_BTNEX_WORK) * BR_BTN_SYS_STACK_MAX;
		p_wk->p_btn_stack	= GFL_HEAP_AllocMemory( heapID, size );
		GFL_STD_MemClear( p_wk->p_btn_stack, size );
	}

	//最初に読み込むボタンを初期化
	{	
		//読み込むボタンのリンク情報を検知し、途中のものであればスタックに積む
		{	
			u32 num	= BR_BTN_DATA_SYS_GetLinkMax( p_wk->p_btn_data, menuID );

			if( num != 0 )
			{	
				int i;
				const BR_BTN_DATA *	cp_data;
				BR_MENUID	preID;
				u32 btnID;
				BR_BTNEX_WORK	btn	= {0};

				for( i = 0; i < num; i++ )
				{	
					BR_BTN_DATA_SYS_GetLink( p_wk->p_btn_data, menuID, i, &preID, &btnID );

					cp_data	= BR_BTN_DATA_SYS_GetData( p_wk->p_btn_data, preID, btnID );
					BR_BTNEX_Init( &btn, cp_data, p_unit, p_wk->p_bmpoam, p_res, heapID );
					Br_Btn_Sys_PushStack( p_wk, &btn );
				}
			}
		}

		//読み込むもの
		{	
			int i;
			const BR_BTN_DATA *	cp_data;
			p_wk->btn_now_num	= BR_BTN_DATA_SYS_GetDataNum( p_wk->p_btn_data, menuID );

			for( i = 0; i < p_wk->btn_now_num; i++ )
			{	
				cp_data	= BR_BTN_DATA_SYS_GetData( p_wk->p_btn_data, menuID, i );
				BR_BTNEX_Init( &p_wk->p_btn_now[i], cp_data, p_unit, p_wk->p_bmpoam, p_res, heapID );
			}
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
			BR_BTNEX_Exit( &p_wk->p_btn_stack[i] );
		}
	}
	GFL_HEAP_FreeMemory( p_wk->p_btn_stack );

	//ボタンバッファ破棄
	{	
		int i;
		for( i = 0; i < p_wk->btn_now_num; i++ )
		{	
			BR_BTNEX_Exit( &p_wk->p_btn_now[i] );
		}
	}
	GFL_HEAP_FreeMemory( p_wk->p_btn_now );
	
	//リソース破棄
	{	
		BR_RES_UnLoadOBJ( p_wk->p_res, BR_RES_OBJ_BROWSE_BTN_S );
		BR_RES_UnLoadOBJ( p_wk->p_res, BR_RES_OBJ_BROWSE_BTN_M );
	}

	BR_BTN_DATA_SYS_Exit( p_wk->p_btn_data );
	BmpOam_Exit( p_wk->p_bmpoam );

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
			u32 x, y;
			BOOL is_trg	= FALSE;

			p_wk->input	= BR_BTN_SYS_INPUT_NONE;

			//ボタン押し検知
			if( GFL_UI_TP_GetPointTrg( &x, &y ) )
			{	
				for( i = 0; i < p_wk->btn_now_max; i++ )
				{	
					if( BR_BTNEX_GetTrg( &p_wk->p_btn_now[i], x, y ) )
					{	

#ifdef PM_DEBUG
						int j;
						NAGI_Printf("push ! [%d]\n", i );
						for( j = 0; j < BR_BTN_DATA_PARAM_MAX; j++  )
						{	
							NAGI_Printf("param %d\n", BR_BTN_DATA_GetParam( p_wk->p_btn_now[i].cp_data, j) );
						}
#endif	//PM_DEBUG

						p_wk->trg_btn	= i;
						is_trg				= TRUE;
						break;
					}
				}
			}

			//ボタン動作開始
			if( is_trg )
			{	
				for( i = 0; i < p_wk->btn_now_max; i++ )
				{	
					if( p_wk->trg_btn == i )
					{	
						BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_PUSH, NULL );
					}
					else
					{	
						BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_HIDE, &p_wk->p_btn_now[p_wk->trg_btn] );
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
			BOOL is_end	= TRUE;

			for( i = 0; i < p_wk->btn_now_max; i++ )
			{	
				is_end	&= BR_BTNEX_MainMove( &p_wk->p_btn_now[i] );
			}

			if( is_end )
			{
				p_wk->state	= BR_BTN_SYS_STATE_BTN_CHANGE;
			}
		}
		break;
	
	case BR_BTN_SYS_STATE_BTN_CHANGE:
		{
			BR_MENUID nextID;
			u32				next_mode;

			p_wk->next_type	= BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_TYPE );
			nextID					= BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_DATA1 );
			next_mode				= BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_DATA2 );
			p_wk->next_valid= BR_BTNEX_GetParam(&p_wk->p_btn_now[p_wk->trg_btn],BR_BTN_PARAM_VALID );

			switch( p_wk->next_type )
			{	
			case BR_BTN_TYPE_SELECT:				//選択用ボタン
				//決定したボタンをスタックに積む
				Br_Btn_Sys_PushStack( p_wk, &p_wk->p_btn_now[p_wk->trg_btn] );
				//他のボタンを読み変える
				Br_Btn_Sys_ReLoadBtn( p_wk, nextID );
				break;

			case BR_BTN_TYPE_RETURN:				//1つ前のメニューへ戻る用ボタン
				{	
					BR_BTNEX_WORK	btn;
					if( Br_Btn_Sys_PopStack( p_wk, &btn ) )
					{	
						GF_ASSERT( btn.is_use );
						//他のボタンを読み変える
						Br_Btn_Sys_ReLoadBtn( p_wk, nextID );
						BR_BTNEX_Exit( &btn );
					}
				}
				break;

			case BR_BTN_TYPE_EXIT:					//バトルレコーダー終了用ボタン
				break;

			case BR_BTN_TYPE_MYRECORD:			//自分の記録ボタン
				p_wk->next_proc	= nextID;
				p_wk->next_mode	= next_mode;
				NAGI_Printf( "自分の記録 %d %d \n", nextID, next_mode );
				break;

			case BR_BTN_TYPE_OTHERRECORD:	//誰かの記録ボタン
				p_wk->next_proc	= nextID;
				p_wk->next_mode	= next_mode;
				break;

			case BR_BTN_TYPE_DELETE_MY:		//自分の記録を消すボタン
				p_wk->next_proc	= nextID;	
				p_wk->next_mode	= next_mode;
				break;

			case BR_BTN_TYPE_DELETE_OTHER:	//誰かの記録を消すボタン
				p_wk->next_proc	= nextID;
				p_wk->next_mode	= next_mode;
				break;

			case BR_BTN_TYPE_CHANGESEQ:		//シーケンス変更ボタン
				p_wk->next_proc	= nextID;
				p_wk->next_mode	= next_mode;
				break;
			}

			{	
				int i;
				//ボタン出現処理
				for( i = 0; i < p_wk->btn_now_max; i++ )
				{	
					BR_BTNEX_StartMove( &p_wk->p_btn_now[i], BR_BTN_MOVE_APPEAR, NULL );
				}

				//スタックに積んだ、決定ボタンは上部へ移動
			//	if( p_wk->btn_stack_num != 0 )
			//	{	
			//		BR_BTNEX_StartMove( &p_wk->p_btn_stack[p_wk->btn_stack_num-1], BR_BTN_MOVE_TAG, NULL );
			//	}
			}
			p_wk->state	= BR_BTN_SYS_STATE_APPEAR_MOVE;
		}
		break;

	case BR_BTN_SYS_STATE_APPEAR_MOVE:
		{	
			int i;
			BOOL is_end	= TRUE;

			for( i = 0; i < p_wk->btn_now_max; i++ )
			{	
				is_end	&= BR_BTNEX_MainMove( &p_wk->p_btn_now[i] );
			}
			for( i = 0; i < p_wk->btn_stack_max; i++ )
			{	
				is_end	&= BR_BTNEX_MainMove( &p_wk->p_btn_stack[i] );
			}
			if( is_end )
			{		
				p_wk->state	= BR_BTN_SYS_STATE_INPUT;
			}
		}
		break;

	case BR_BTN_SYS_STATE_INPUT:
	
		p_wk->state	= BR_BTN_SYS_STATE_WAIT;

		if( p_wk->next_valid )
		{	

			switch( p_wk->next_type )
			{	
			case BR_BTN_TYPE_SELECT:				//選択用ボタン
				/* fallthrough */
			case BR_BTN_TYPE_RETURN:				//1つ前のメニューへ戻る用ボタン
				p_wk->state	= BR_BTN_SYS_STATE_WAIT;
				break;

			case BR_BTN_TYPE_EXIT:					//バトルレコーダー終了用ボタン
				p_wk->input	= BR_BTN_SYS_INPUT_EXIT;
				break;

			case BR_BTN_TYPE_MYRECORD:			//自分の記録ボタン
				p_wk->input	= BR_BTN_SYS_INPUT_CHANGESEQ;
				break;

			case BR_BTN_TYPE_OTHERRECORD:	//誰かの記録ボタン
				p_wk->input	= BR_BTN_SYS_INPUT_CHANGESEQ;
				break;

			case BR_BTN_TYPE_DELETE_MY:		//自分の記録を消すボタン
				break;

			case BR_BTN_TYPE_DELETE_OTHER:	//誰かの記録を消すボタン
				break;

			case BR_BTN_TYPE_CHANGESEQ:		//シーケンス変更ボタン			
				p_wk->input	= BR_BTN_SYS_INPUT_CHANGESEQ;
				break;
		}
		break;
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダー	ボタン管理	入力状態取得
 *
 *	@param	cp_wk	ワーク
 *	@param	p_seq	BR_BTN_SYS_INPUT_CHANGESEQのとき、次のシーケンス
 *	@param	p_param	BR_BTN_SYS_INPUT_CHANGESEQのとき、次のシーケンスのモード
 *
 *	@return	入力の種類
 */
//-----------------------------------------------------------------------------
BR_BTN_SYS_INPUT BR_BTN_SYS_GetInput( const BR_BTN_SYS_WORK *cp_wk, u32 *p_seq, u32 *p_mode )
{	
	if( p_seq )
	{	
		*p_seq	= cp_wk->next_proc;
	}
	if( p_mode )
	{	
		*p_mode	= cp_wk->next_mode;
	}

	return cp_wk->input;
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
static void Br_Btn_Sys_PushStack( BR_BTN_SYS_WORK *p_wk, BR_BTNEX_WORK *p_btn )
{	
	GF_ASSERT( p_wk->btn_stack_num - 1 < p_wk->btn_stack_max );

	//積む
	BR_BTNEX_Copy( p_wk, &p_wk->p_btn_stack[p_wk->btn_stack_num], p_btn, CLSYS_DRAW_MAIN );
	GFL_STD_MemClear( p_btn, sizeof(BR_BTNEX_WORK) );	//積んだので、元の場所の情報を消す
	
	p_wk->btn_stack_num++;

	//座標を設定 //@todo
	{	
		int i;
		for( i = 0; i < p_wk->btn_stack_num; i++ )
		{	
			BR_BTNEX_SetStackPos( &p_wk->p_btn_stack[p_wk->btn_stack_num - i - 1], i );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン管理システムからボタンを破棄する
 *
 *	@param	BR_BTN_SYS_WORK *p_wk	ワーク
 *	@param	*p_btn								スタックから受け取るボタン
 *
 *	@retval	TRUE破棄した　FALSE先頭だったので何もしなかった
 */
//-----------------------------------------------------------------------------
static BOOL Br_Btn_Sys_PopStack( BR_BTN_SYS_WORK *p_wk, BR_BTNEX_WORK *p_btn )
{	
	if( p_wk->btn_stack_num != 0 )
	{	
		p_wk->btn_stack_num--;
		*p_btn	= p_wk->p_btn_stack[p_wk->btn_stack_num];
		GFL_STD_MemClear( &p_wk->p_btn_stack[p_wk->btn_stack_num], sizeof(BR_BTNEX_WORK) );


		//座標を設定
		{	
			int i;
			for( i = 0; i < p_wk->btn_stack_num; i++ )
			{	
				BR_BTNEX_SetStackPos( &p_wk->p_btn_stack[p_wk->btn_stack_num - i - 1], i );
			}
		}

		return TRUE;
	}

	return FALSE;
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
	int i;
	for( i = 0; i < p_wk->btn_now_max; i++ )
	{	
		BR_BTNEX_Exit( &p_wk->p_btn_now[i] );
	}

	{	
		int i;
		const BR_BTN_DATA *	cp_data;

		p_wk->btn_now_num	= BR_BTN_DATA_SYS_GetDataNum( p_wk->p_btn_data, menuID );

		for( i = 0; i < p_wk->btn_now_num; i++ )
		{	
			cp_data	= BR_BTN_DATA_SYS_GetData( p_wk->p_btn_data, menuID, i );
			BR_BTNEX_Init( &p_wk->p_btn_now[i], cp_data, p_wk->p_unit, p_wk->p_bmpoam, p_wk->p_res, p_wk->heapID );
		}
	}
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
 *	@param	BR_BTNEX_WORK *p_wk			ワーク
 *	@param	BR_BTN_DATA *cp_data	ボタン情報
 *	@param	*p_unit								OBJ作成用ユニット
 *	@param	*p_bmpoam							BMPOAMシステム
 *	@param	p_res									リソース
 *	@param	heapID								ヒープID
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_Init( BR_BTNEX_WORK *p_wk, const BR_BTN_DATA *cp_data, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, const BR_RES_WORK *cp_res, HEAPID heapID )
{	
	u32 plt;

	GF_ASSERT( p_wk->is_use == FALSE );

	GFL_STD_MemClear( p_wk, sizeof(BR_BTNEX_WORK) );
	p_wk->is_use	= TRUE;
	p_wk->cp_data	= cp_data;
  p_wk->display = CLSYS_DRAW_SUB;

	//ボタン作成
	{	
		BR_RES_OBJ_DATA	res;
		GFL_FONT *p_font;
		GFL_MSGDATA *p_msg;
		GFL_CLWK_DATA	cldata;
    u32 msgID;

    //データ設定
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
    cldata.pos_x    = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_X );
    cldata.pos_y    = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_Y );
    cldata.anmseq   = BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_ANMSEQ );
    cldata.softpri  = 1;

		//リソース読み込み
		BR_RES_GetOBJRes( cp_res, BR_RES_OBJ_BROWSE_BTN_S, &res );
		p_font	= BR_RES_GetFont( cp_res );
		p_msg		= BR_RES_GetMsgData( cp_res );
		msgID		= BR_BTN_DATA_GetParam( cp_data, BR_BTN_DATA_PARAM_MSGID );

		//作成
		p_wk->p_btn		= BR_BTN_Init( &cldata, msgID, p_wk->display, p_unit, 
				p_bmpoam, p_font, p_msg, &res, GetHeapLowID( heapID ) );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン破棄
 *
 *	@param	BR_BTNEX_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_Exit( BR_BTNEX_WORK *p_wk )
{	
	if( p_wk->is_use )
	{	
    BR_BTN_Exit( p_wk->p_btn );
		GFL_STD_MemClear( p_wk, sizeof(BR_BTNEX_WORK) );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンをタッチしたかどうか
 *
 *	@param	const BR_BTNEX_WORK *cp_wk	ワーク
 *	@param	位置座標
 *
 *	@return	TRUEならば押した	FALSEならば押していない
 */
//-----------------------------------------------------------------------------
static BOOL BR_BTNEX_GetTrg( const BR_BTNEX_WORK *cp_wk, u32 x, u32 y )
{	
	if( !cp_wk->is_use )
	{	
		return FALSE;
	}

  return BR_BTN_GetTrg( cp_wk->p_btn, x, y );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの動作開始
 *
 *	@param	BR_BTNEX_WORK *p_wk				ワーク
 *	@param	move										動作
 *	@param	BR_BTNEX_WORK *cp_target	動作に必要なターゲっト
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_StartMove( BR_BTNEX_WORK *p_wk, BR_BTN_MOVE move, const BR_BTNEX_WORK *cp_target )
{	
	//@todo
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンのメイン動作
 *
 *	@param	BR_BTNEX_WORK *p_wk		ワーク
 *
 *	@return	TRUEならばボタンの動き終了	FALSEならば動き中
 */
//-----------------------------------------------------------------------------
static BOOL BR_BTNEX_MainMove( BR_BTNEX_WORK *p_wk )
{	
	//@todo
	return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンをコピー
 *
 *	@param	p_wk		ワーク					
 *	@param	p_dst		コピー先　サブ画面のボタン
 *	@param	p_src		コピー元	メイン画面のボタン
 *	@param	display	どちらの画面にコピーするか
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_Copy( const BR_BTN_SYS_WORK *cp_wk, BR_BTNEX_WORK *p_dst, const BR_BTNEX_WORK *cp_src, CLSYS_DRAW_TYPE display )
{
	u32 plt;	

	GF_ASSERT( p_dst->is_use == FALSE );
	GF_ASSERT( cp_src->is_use == TRUE );
		
	//引き継ぐもの
	{
		GFL_STD_MemClear( p_dst, sizeof(BR_BTNEX_WORK) );
		p_dst->cp_data	= cp_src->cp_data;
		p_dst->is_use		= cp_src->is_use;
		p_dst->display	= display;
	}

	//同じならば、コピー、
	//違うならば作り直し
	if( cp_src->display	== display )
	{	
		p_dst->p_btn			= cp_src->p_btn;
	}
	else
	{	
		//新しく作り直すため、消去
		{	
			BR_BTN_Exit( cp_src->p_btn );
		}

    //ボタン作成
    {	
      BR_RES_OBJ_DATA	res;
      GFL_FONT *p_font;
      GFL_MSGDATA *p_msg;
      GFL_CLWK_DATA	cldata;
      u32 msgID;

      //データ設定
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x    = BR_BTN_DATA_GetParam( p_dst->cp_data, BR_BTN_DATA_PARAM_X );
      cldata.pos_y    = BR_BTN_DATA_GetParam( p_dst->cp_data, BR_BTN_DATA_PARAM_Y );
      cldata.anmseq   = BR_BTN_DATA_GetParam( p_dst->cp_data, BR_BTN_DATA_PARAM_ANMSEQ );
      cldata.softpri  = 1;

      //リソース読み込み
			if( display == CLSYS_DRAW_MAIN )
			{	
				BR_RES_GetOBJRes( cp_wk->p_res, BR_RES_OBJ_BROWSE_BTN_M, &res );
			}
			else
			{	
				BR_RES_GetOBJRes( cp_wk->p_res, BR_RES_OBJ_BROWSE_BTN_S, &res );
			}
      p_font	= BR_RES_GetFont( cp_wk->p_res );
      p_msg		= BR_RES_GetMsgData( cp_wk->p_res );
      msgID		= BR_BTN_DATA_GetParam( p_dst->cp_data, BR_BTN_DATA_PARAM_MSGID );

      //作成
      p_dst->p_btn		= BR_BTN_Init( &cldata, msgID, display, cp_wk->p_unit, 
          cp_wk->p_bmpoam, p_font, p_msg, &res, cp_wk->heapID );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの座標を設定
 *
 *	@param	BR_BTNEX_WORK *p_wk	ワーク
 *	@param	x									X座標
 *	@param	y									Y座標
 *
 */
//-----------------------------------------------------------------------------
static void BR_BTNEX_SetStackPos( BR_BTNEX_WORK *p_wk, u16 stack_num )
{	
	GFL_CLACTPOS	pos;
	pos.x	= BR_BTN_DATA_GetParam( p_wk->cp_data, BR_BTN_DATA_PARAM_X );
	pos.y	= TAG_INIT_POS_UP( stack_num );

  BR_BTN_SetPos( p_wk->p_btn, pos.x, pos.y );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの情報を受け取り
 *
 *	@param	const BR_BTNEX_WORK *cp_wk	ワーク
 *	@param	param		ボタンの情報インデックス
 *
 *	@return	ボタン情報
 */
//-----------------------------------------------------------------------------
static u32 BR_BTNEX_GetParam( const BR_BTNEX_WORK *cp_wk, BR_BTN_PARAM param )
{	
	u32 ret;
	switch( param )
	{	
	case BR_BTN_PARAM_TYPE:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_TYPE );
		break;

	case BR_BTN_PARAM_DATA1:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_DATA1 );
		break;

	case BR_BTN_PARAM_DATA2:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_DATA2 );
		break;
	
	case BR_BTN_PARAM_VALID:
		ret = BR_BTN_DATA_GetParam( cp_wk->cp_data, BR_BTN_DATA_PARAM_VALID );
		break;


	default:
		ret	= 0;
		GF_ASSERT(0);
	}

	return ret;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					ボタン単体作成関数(各プロックで使用する)
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン初期化
 *
 *	@param	const GFL_CLWK_DATA *cp_cldata	設定情報
 *	@param	msgID														ボタンに載せる文字列
 *	@param	display													表示面
 *	@param	*p_unit													アクター登録ユニット
 *	@param	p_bmpoam												BMPOAM登録システム
 *	@param	*p_font													フォント
 *	@param	*p_msg													メッセージ
 *	@param	BR_RES_OBJ_DATA *cp_res					リソース
 *	@param	heapID													ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
BR_BTN_WORK * BR_BTN_Init( const GFL_CLWK_DATA *cp_cldata, u16 msgID, CLSYS_DRAW_TYPE display, GFL_CLUNIT *p_unit, BMPOAM_SYS_PTR p_bmpoam, GFL_FONT *p_font, GFL_MSGDATA *p_msg, const BR_RES_OBJ_DATA *cp_res, HEAPID heapID )
{	
	BR_BTN_WORK *p_wk;

	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(BR_BTN_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(BR_BTN_WORK) );
  p_wk->display = display;

	//CLWK作成
	{	
		p_wk->p_clwk	= GFL_CLACT_WK_Create( p_unit, 
				cp_res->ncg, cp_res->ncl, cp_res->nce, 
				cp_cldata, display, heapID );
	}

	//フォント
	{	
		STRBUF			*p_str;

		p_wk->p_bmp	= GFL_BMP_Create( 15, 2, GFL_BMP_16_COLOR, heapID );
		p_str	= GFL_MSG_CreateString( p_msg, msgID );
		PRINTSYS_Print( p_wk->p_bmp, 0, 0, p_str, p_font );

		GFL_STR_DeleteBuffer( p_str );
	}


	//OAMの文字を作成
	{	
		BMPOAM_ACT_DATA	actdata;
		
		GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
		actdata.bmp	= p_wk->p_bmp;
		actdata.x		= cp_cldata->pos_x + BR_BTN_OAMFONT_OFS_X;
		actdata.y		= cp_cldata->pos_y - BR_BTN_DATA_HEIGHT/2 + BR_BTN_OAMFONT_OFS_Y;
		actdata.pltt_index	= cp_res->ncl;
		actdata.soft_pri		= cp_cldata->softpri - 1;
		actdata.setSerface	= display;
		actdata.draw_type		= display;
		actdata.bg_pri			= cp_cldata->bgpri;
		p_wk->p_oamfont	= BmpOam_ActorAdd( p_bmpoam, &actdata );
		BmpOam_ActorBmpTrans(p_wk->p_oamfont);
	}

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン破棄
 *
 *	@param	BR_BTN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_BTN_Exit( BR_BTN_WORK *p_wk )
{	
	BmpOam_ActorDel( p_wk->p_oamfont );
	GFL_BMP_Delete( p_wk->p_bmp );
	GFL_CLACT_WK_Remove( p_wk->p_clwk );

	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンのタッチを検知
 *
 *	@param	const BR_BTN_WORK *cp_wk	ワーク
 *	@param	x													タッチ座標X
 *	@param	y													タッチ座標Y
 *
 *	@return	TRUEでボタントリガ	FALSEで押していない
 */
//-----------------------------------------------------------------------------
BOOL BR_BTN_GetTrg( const BR_BTN_WORK *cp_wk, u32 x, u32 y )
{	
	GFL_RECT rect;
	GFL_CLACTPOS	pos;

	GFL_CLACT_WK_GetPos( cp_wk->p_clwk, &pos, cp_wk->display );

	rect.left		= pos.x;
	rect.right	= pos.x + BR_BTN_DATA_WIDTH;
	rect.top		= pos.y - BR_BTN_DATA_HEIGHT/2;
	rect.bottom	= pos.y + BR_BTN_DATA_HEIGHT/2;

  return ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンの位置を設定
 *
 *	@param	BR_BTN_WORK *p_wk		ワーク
 *	@param	x										座標X
 *	@param	y										座標Y
 */
//-----------------------------------------------------------------------------
void BR_BTN_SetPos( BR_BTN_WORK *p_wk, u32 x, u32 y )
{	
	GFL_CLACTPOS	pos;
	pos.x	= x;
	pos.y	= y;

	GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, p_wk->display );
	BmpOam_ActorSetPos( p_wk->p_oamfont,	pos.x + BR_BTN_OAMFONT_OFS_X,
																				pos.y - BR_BTN_DATA_HEIGHT/2 + BR_BTN_OAMFONT_OFS_Y );

}
