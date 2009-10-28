//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shortcut.c
 *	@brief	Yボタン登録セーブデータ
 *	@author	Toru=Nagihashi
 *	@data		2009.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "savedata/shortcut.h"
#include "savedata/save_tbl.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define DEBUG_PRINT_ON
#endif //PM_DEBUG

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	Yボタン登録用セーブデータ
//=====================================
struct _SHORTCUT
{	
	u8	data[SHORTCUT_ID_MAX];
};

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
static void ShortCut_SortNull( SHORTCUT *p_wk );


#ifdef DEBUG_PRINT_ON
static inline void DEBUG_PrintData( SHORTCUT *p_wk )
{	
	int i;
	for( i = 0; i < SHORTCUT_ID_MAX; i++ )
	{	
		NAGI_Printf( "data[%d]= %d \n", i, p_wk->data[ i ] );
	}
}
#else
#define DEBUG_PRINT_ON(x)	/*  */
#endif

//=============================================================================
/**
 *	セーブデータシステムに依存する関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SHORTCUT構造体のサイズ取得
 *
 *	@return	SHORTCUT構造体のサイズ
 */
//-----------------------------------------------------------------------------
int SHORTCUT_GetWorkSize( void )
{	
	return sizeof(SHORTCUT);
}
//----------------------------------------------------------------------------
/**
 *	@brief	SHORTCUT構造体初期化
 *
 *	@param	SHORTCUT *p_msc ワーク
 */
//-----------------------------------------------------------------------------
void SHORTCUT_Init( SHORTCUT *p_wk )
{	
	GFL_STD_MemFill( p_wk->data, SHORTCUT_ID_NULL, sizeof(u8) * SHORTCUT_ID_MAX );
}

//=============================================================================
/**
 *	セーブデータ取得のための関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	未分類データのCONST版ポインタ取得
 *
 *	@param	SAVE_CONTROL_WORK * cp_sv	セーブデータ保持ワークへのポインタ
 *
 *	@return	SHORTCUT
 */
//-----------------------------------------------------------------------------
const SHORTCUT * SaveData_GetShortCutConst( const SAVE_CONTROL_WORK * cp_sv)
{	
	return (const SHORTCUT *)SaveData_GetShortCut( (SAVE_CONTROL_WORK *)cp_sv);
}
//----------------------------------------------------------------------------
/**
 *	@brief	未分類データのポインタ取得
 *
 *	@param	SAVE_CONTROL_WORK * p_sv	セーブデータ保持ワークへのポインタ
 *
 *	@return	SHORTCUT
 */
//-----------------------------------------------------------------------------
SHORTCUT * SaveData_GetShortCut( SAVE_CONTROL_WORK * p_sv)
{	
	SHORTCUT	*p_msc;
	p_msc = SaveControl_DataPtrGet(p_sv, GMDATA_ID_SHORTCUT);
	return p_msc;
}

//=============================================================================
/**
 *	各アプリ内での、Yボタン登録関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	Yボタン登録設定
 *
 *	@param	SHORTCUT *p_wk	ワーク
 *	@param	shortcutID		登録タイプ
 *	@param	is_on		TRUEでYボタン登録	FALSEでYボタン登録解除
 */
//-----------------------------------------------------------------------------
void SHORTCUT_SetRegister( SHORTCUT *p_wk, SHORTCUT_ID shortcutID, BOOL is_on )
{	
	int i;
	if( is_on )
	{	
		//登録する処理
		//ソート済みと考え、NULLが来るまで後尾を検索
		for( i = 0; i < SHORTCUT_ID_MAX; i++ )
		{
			//すでにあったら何もしない
			if( p_wk->data[ i ] == shortcutID )
			{
				return;
			}

			//後尾発見
			if( p_wk->data[ i ] == SHORTCUT_ID_NULL )
			{	
				break;
			}
		}
		GF_ASSERT_MSG( i != SHORTCUT_ID_MAX, "SHORTCUT_IDへの登録が不十分\n" );

		p_wk->data[ i ] = shortcutID;
	}
	else
	{	
		//消す処理
		for( i = 0; i < SHORTCUT_ID_MAX; i++ )
		{	
			if( p_wk->data[ i ] == shortcutID )
			{	
				p_wk->data[ i ] = SHORTCUT_ID_NULL;
			}
		}
	}


	//NULLをソート
	ShortCut_SortNull( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	Yボタン登録状態取得
 *	
 *	@param	const SHORTCUT *cp_wk	ワーク
 *	@param	shortcutID									登録タイプ
 *
 *	@return	TRUEならば登録されている	FALSEならば登録されていない
 */
//-----------------------------------------------------------------------------
BOOL SHORTCUT_GetRegister( const SHORTCUT *cp_wk, SHORTCUT_ID shortcutID )
{	
	int i;
	//消す処理
	for( i = 0; i < SHORTCUT_ID_MAX; i++ )
	{	
		if( cp_wk->data[ i ] == shortcutID )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//=============================================================================
/**
 *	Yボタンメニューで取得、操作する関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	インデックスから登録されているものを取得
 *
 *	@param	const SHORTCUT *cp_wk		ワーク
 *	@param	idx											インデックス	
 *
 *	@return	登録されているタイプ	or SHORTCUT_ID_NULLならば登録されていない
 */
//-----------------------------------------------------------------------------
SHORTCUT_ID SHORTCUT_GetType( const SHORTCUT *cp_wk, u8 idx )
{	
	GF_ASSERT_MSG( idx < SHORTCUT_ID_MAX, "インデックスが最大を超えています\n" );
	return cp_wk->data[ idx ];
}
//----------------------------------------------------------------------------
/**
 *	@brief	登録数を取得
 *
 *	@param	const SHORTCUT *cp_wk		ワーク
 *
 *	@return	登録数
 */
//-----------------------------------------------------------------------------
u16 SHORTCUT_GetMax( const SHORTCUT *cp_wk )
{	
	int i;
	int cnt = 0;
	for( i = 0 ; i < SHORTCUT_ID_MAX; i++ )
	{	
		if( cp_wk->data[ i ] != SHORTCUT_ID_NULL )
		{	
			cnt++;
		}
	}

	return cnt;
}
//----------------------------------------------------------------------------
/**
 *	@brief	入れ替え挿入
 *					指定されたタイプを指定したインデックスに移す処理
 *
 *	@param	const SHORTCUT *cp_wk		ワーク
 *	@param	shortcutID										
 *	@param	insert_idx							挿入されるインデックス
 *
 *	@param	挿入されたインデックス（－1されていることがある）
 *
 */
//-----------------------------------------------------------------------------
u8 SHORTCUT_Insert( SHORTCUT *p_wk, SHORTCUT_ID shortcutID, u8 insert_idx )
{	
	s16 i;
	u16 next;
	s16 erase_idx;

	/*
	 *	メモ
	 *
	 *	挿入なので、上にいくときは良いが、
	 *	一端消す場合、下へいくときは、自分の分を考慮すること
	 *
	 *		
	 *	AAA	→	消すとインデックスが詰まるので・・・
	 *			←	下にいくときは-1インデックス
	 *	BBB
	 *		
	 *	CCC
	 *
	 *
	 */


	GF_ASSERT_MSG( insert_idx < SHORTCUT_ID_MAX, "インデックスが最大を超えています\n" );
	
	//自分のインデックスを取得
	for( erase_idx = 0 ; erase_idx < SHORTCUT_ID_MAX ; erase_idx++ )
	{
		if( p_wk->data[ erase_idx ] == shortcutID  )
		{	
			break;
		}
	}
	GF_ASSERT_MSG( erase_idx != SHORTCUT_ID_MAX, "見つからなかった\n" )


	//タイプを消す
	SHORTCUT_SetRegister( p_wk, shortcutID, FALSE );
	
	NAGI_Printf( "消した後\n" );
	DEBUG_PrintData( p_wk );
	
	//消した分つまるので
	//インデックス－1
	if( erase_idx < insert_idx )
	{	
		insert_idx--;
	}


	//挿入の以下を１つずつずらす
	for( i = SHORTCUT_ID_MAX-1; i >= insert_idx ; i-- )
	{	
		p_wk->data[ i ] = p_wk->data[ i - 1 ];
	}

	//挿入
	p_wk->data[ insert_idx ] = shortcutID;


	NAGI_Printf( "挿入完了\n" );
	DEBUG_PrintData( p_wk );

	return insert_idx;
}
//=============================================================================
/**
 *		PRIVATE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SHORTCUT_ID_NULLがあればデータを詰める
 *
 *	@param	SHORTCUT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void ShortCut_SortNull( SHORTCUT *p_wk )
{	
	u16 i;
	u16 next;
	u8 tmp;
	for( i = 0; i < SHORTCUT_ID_MAX; i++ )
	{
		next	= MATH_CLAMP( i+1, 1, SHORTCUT_ID_MAX-1 );
		if( p_wk->data[ i ] == SHORTCUT_ID_NULL )
		{	
			tmp	= p_wk->data[ i ];
			p_wk->data[ i ] = p_wk->data[ next ];
			p_wk->data[ next ] = tmp;
		}
	}
}
