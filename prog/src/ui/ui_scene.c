//=============================================================================
/**
 *
 *	@file		ui_scene.c
 *	@brief	シーン管理モジュール
 *	@author	hosaka genya
 *	@data		2009.07.01
 *
 */
//=============================================================================
//	2009.07.16 : メイン終了時にCleanupが呼ばれていないとサブシーケンスが初期化されなかった不具合を修正

#include <gflib.h>
#include "ui/ui_scene.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{	
	SCENE_SEQ_INIT = 0,
	SCENE_SEQ_SET_UP,
	SCENE_SEQ_MAIN,
	SCENE_SEQ_END,
	SCENE_SEQ_SET_NEXT,
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	シーン管理ワーク
//==============================================================
typedef struct UI_SCENE_CNT_tag{
	const UI_SCENE_FUNC_SET* func_set_tbl;
	void* work;
	u16 scene_id;
	u16 scene_id_next;
	u8 scene_max;
	u8 seq;
	u8 sub_seq;
	u8 padding[1];
} UI_SCENE_CNT_WORK;

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief	シーン管理モジュール 生成
 *
 *	@param	HEAPID heapID	ヒープID
 *	@param	UI_SCENE_FUNC_SET* func_set_tbl	シーンテーブル
 *	@param	scene_max	シーン数
 *	@param	work		シーン全体で参照するワーク
 *
 *	@retval	生成したワーク
 */
//-----------------------------------------------------------------------------
UI_SCENE_CNT_PTR UI_SCENE_CNT_Create( HEAPID heapID, const UI_SCENE_FUNC_SET* func_set_tbl, int scene_max, int first_scene, void* work )
{	
	UI_SCENE_CNT_PTR ptr;

	ptr = GFL_HEAP_AllocMemory( heapID, sizeof( UI_SCENE_CNT_WORK ) );
	GFL_STD_MemClear( ptr, sizeof( UI_SCENE_CNT_WORK ) );

	// メンバ初期化
	ptr->func_set_tbl	  = func_set_tbl;
	ptr->scene_max	  	= scene_max;
	ptr->work		      	= work;
  ptr->scene_id       = first_scene;
	ptr->scene_id_next  = UI_SCENE_ID_NULL;

	return ptr;
}

//-----------------------------------------------------------------------------
/**
 *	@brief	シーン管理モジュール 削除
 *
 *	@param	UI_SCENE_CNT_PTR ptr 
 *
 *	@retval	none
 */
//-----------------------------------------------------------------------------
void UI_SCENE_CNT_Delete( UI_SCENE_CNT_PTR ptr )
{	
  GF_ASSERT( ptr );

	GFL_HEAP_FreeMemory( ptr );
}

//-----------------------------------------------------------------------------
/**
 *	@brief	シーン管理モジュール 主処理
 *
 *	@param	UI_SCENE_CNT_PTR ptr 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL UI_SCENE_CNT_Main( UI_SCENE_CNT_PTR ptr )
{
	const UI_SCENE_FUNC_SET* func_set_tbl;
	
	func_set_tbl = &ptr->func_set_tbl[ ptr->scene_id ];

	switch( ptr->seq )
	{	
		// 初期化処理
		case SCENE_SEQ_INIT :
			if(	func_set_tbl->Init == NULL )
			{	
				OS_TPrintf("scene Id:%d Init Skip\n", ptr->scene_id );
				ptr->seq = SCENE_SEQ_SET_UP;
			}
			else
			{	
				if( func_set_tbl->Init( ptr, ptr->work ) )
				{	
					ptr->sub_seq = 0;
					OS_TPrintf("scene Id:%d Init End\n", ptr->scene_id );
					ptr->seq = SCENE_SEQ_SET_UP;
				}
				break;
			}
			/* FALLTHROW */

		// 前処理
		case SCENE_SEQ_SET_UP :
			if( func_set_tbl->SetUp != NULL )
			{	
				OS_TPrintf("scene Id:%d Call Setup\n", ptr->scene_id );
				func_set_tbl->SetUp( ptr, ptr->work );
			}
			ptr->seq = SCENE_SEQ_MAIN;
			/* FALLTHROW */

		// 主処理
		case SCENE_SEQ_MAIN :
			// 主処理なしは許可しない 
			GF_ASSERT( func_set_tbl->Main != NULL );

			if( func_set_tbl->Main( ptr, ptr->work ) )
			{	
				OS_TPrintf("scene Id:%d Main End\n", ptr->scene_id );

				ptr->sub_seq = 0;
				
				// 後処理
				if( func_set_tbl->CleanUp != NULL )
				{	
					OS_TPrintf("scene Id:%d Call CleanUp\n", ptr->scene_id );
					func_set_tbl->CleanUp( ptr, ptr->work );
				}

				ptr->seq = SCENE_SEQ_END;
			}
			break;

		// 終了処理
		case SCENE_SEQ_END :
			if( func_set_tbl->Exit == NULL )
			{	
				OS_TPrintf("scene Id:%d Exit Skip\n", ptr->scene_id );
				ptr->seq = SCENE_SEQ_SET_NEXT;
			}
			else
			{	
				if( func_set_tbl->Exit( ptr, ptr->work ) )
				{	
					ptr->sub_seq = 0;
					OS_TPrintf("scene Id:%d Exit End\n", ptr->scene_id );
					ptr->seq = SCENE_SEQ_SET_NEXT;
				}
				break;
			}
			/* FALLTHROW */

		// 次のシーンをセット
		case SCENE_SEQ_SET_NEXT : 
			GF_ASSERT( ptr->scene_id_next != UI_SCENE_ID_NULL );

			ptr->scene_id		= ptr->scene_id_next;
			ptr->scene_id_next	= UI_SCENE_ID_NULL; 

			if( ptr->scene_id == UI_SCENE_ID_END )
			{	
				// シーン管理終了
				return TRUE;
			}
			else
			{	
				// 次のシーンへ
				ptr->seq = 0;
				ptr->sub_seq = 0;
			}
			break;

		default : GF_ASSERT(0);
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief	次のシーンをセット
 *
 *	@param	UI_SCENE_CNT_PTR ptr
 *	@param	scene_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void UI_SCENE_CNT_SetNextScene( UI_SCENE_CNT_PTR ptr, int scene_id )
{	
	GF_ASSERT( ptr );
	GF_ASSERT( scene_id == UI_SCENE_ID_END || scene_id < ptr->scene_max );

	ptr->scene_id_next = scene_id;
}

//-----------------------------------------------------------------------------
/**
 *	@brief	現在のシーンIDを取得
 *
 *	@param	UI_SCENE_CNT_PTR ptr 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
int UI_SCENE_CNT_GetSceneID( UI_SCENE_CNT_PTR ptr )
{	
  GF_ASSERT( ptr );
	
  return ptr->scene_id;
}

//-----------------------------------------------------------------------------
/**
 *	@brief	サブシーケンサを取得
 *
 *	@param	UI_SCENE_CNT_PTR ptr 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 UI_SCENE_CNT_GetSubSeq( UI_SCENE_CNT_PTR ptr )
{	
	GF_ASSERT( ptr->seq != SCENE_SEQ_SET_UP );

	return ptr->sub_seq;
}

//-----------------------------------------------------------------------------
/**
 *	@brief	サブシーケンサをインクリメント
 *
 *	@param	UI_SCENE_CNT_PTR ptr 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void UI_SCENE_CNT_IncSubSeq( UI_SCENE_CNT_PTR ptr )
{	
	GF_ASSERT( ptr->seq != SCENE_SEQ_SET_UP );

	ptr->sub_seq++;
}


