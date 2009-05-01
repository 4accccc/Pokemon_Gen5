//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_ground_anime.c
 *	@brief	フィールド地面アニメーション管理システム
 *	@author	tomoya takahshi
 *	@data		2009.04.30
 *
 *	モジュール名：FIELD_GRANM
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "field_ground_anime.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	地面アニメーションワーク
// 
//=====================================
//NNSG3dAnmObjのメモリサイズ	プラチナの１ブロック内のマテリアル最大数分
#define FIELD_GRANM_WORK_ANIMEOBJ_WORK_MATNODE_MAX	( 40 )
#define FIELD_GRANM_WORK_ANIMEOBJ_WORK_SIZE	( sizeof(NNSG3dAnmObj) + (2*FIELD_GRANM_WORK_ANIMEOBJ_WORK_MATNODE_MAX) )


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	フィールド地面アニメーションワーク
//=====================================
struct _FIELD_GRANM_WORK
{
	u32 bind:1;									// レンダーオブジェに関連付けられているか。
	u32 pad:31;									// 予備
	NNSG3dAnmObj* p_anmobj;			// アニメーションオブジェクト
	void*					p_anmres;			// アニメーションリソース
};


//-------------------------------------
///	フィールド地面アニメーション管理システム
//=====================================
struct _FIELD_GRANM 
{
	FIELD_GRANM_WORK* p_wkbuf;
	u16								wkmax;
	
	// アニメーションフレーム情報
	u16								anime_flag;
	fx32							anime_frame;
	fx32							anime_speed;

	// エリアアニメ情報
	void* p_anmfile;
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	フィールド地面アニメーションワーク
//=====================================
static void FIELD_GRANM_Work_Init( FIELD_GRANM_WORK* p_wk, void* p_anmres, u32 heapID );
static void FIELD_GRANM_Work_Exit( FIELD_GRANM_WORK* p_wk );
static void FIELD_GRANM_Work_Bind( FIELD_GRANM_WORK* p_wk, const GFL_G3D_RES* cp_mdlres, const GFL_G3D_RES* cp_texres, NNSG3dRenderObj* p_rendobj );
static void FIELD_GRANM_Work_Release( FIELD_GRANM_WORK* p_wk );
static fx32 FIELD_GRANM_Work_SetAnimeFrame( FIELD_GRANM_WORK* p_wk, fx32 frame );



//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメ管理システム　	生成
 *
 *	@param	arcID					アニメリソースアーカイブID
 *	@param	dataID				アニメリソースアーカイブデータID
 *	@param	block_num			ブロック数
 *	@param	heapID				ヒープID
 *
 *	@return	地面アニメ管理システム
 */
//-----------------------------------------------------------------------------
FIELD_GRANM* FIELD_GRANM_Create( u32 arcID, u32 dataID, u32 block_num, u32 heapID )
{
	FIELD_GRANM* p_sys;
	int i;
	void* p_anmres;

	p_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_GRANM) );

	// アニメーションファイル読み込み
	p_sys->p_anmfile = GFL_ARC_UTIL_Load( arcID, dataID, FALSE, heapID );
	p_anmres = NNS_G3dGetAnmByIdx( p_sys->p_anmfile, 0 );
	
	// 地面アニメ管理ワークバッファ作成
	p_sys->p_wkbuf	= GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_GRANM_WORK)*block_num );
	p_sys->wkmax		= block_num;
	for( i=0; i<p_sys->wkmax; i++ )
	{
		FIELD_GRANM_Work_Init( &p_sys->p_wkbuf[i], p_anmres, heapID );
	}

	// オートアニメーションフラグ設定
	p_sys->anime_flag		= TRUE;
	p_sys->anime_speed	= FX32_ONE;
	
	return p_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションシステム破棄
 *
 *	@param	p_sys			システム
 */
//-----------------------------------------------------------------------------
void FIELD_GRANM_Delete( FIELD_GRANM* p_sys )
{
	int i;

	for( i=0; i<p_sys->wkmax; i++ )
	{
		FIELD_GRANM_Work_Exit( &p_sys->p_wkbuf[i] );
	}
	GFL_HEAP_FreeMemory( p_sys->p_wkbuf );
	GFL_HEAP_FreeMemory( p_sys->p_anmfile );
	GFL_HEAP_FreeMemory( p_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションシステム　メイン
 *
 *	@param	p_sys			システム
 */
//-----------------------------------------------------------------------------
void FIELD_GRANM_Main( FIELD_GRANM* p_sys )
{
	int i;

	// オートアニメーション処理
	if( p_sys->anime_flag )
	{
		p_sys->anime_frame += p_sys->anime_speed;
	}

	// フレーム数設定
	for( i=0; i<p_sys->wkmax; i++ )
	{
		// FIELD_GRANM_Work_SetAnimeFrameの中で、
		// アニメーションフレームの最大値で、値をループさせています。
		p_sys->anime_frame = FIELD_GRANM_Work_SetAnimeFrame( &p_sys->p_wkbuf[i], p_sys->anime_frame );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションシステム　地面アニメーションワークの取得
 *
 *	@param	cp_sys		システムワーク
 *	@param	idx				地面ブロックインデックス
 *
 *	@return	地面アニメーションワーク
 */
//-----------------------------------------------------------------------------
FIELD_GRANM_WORK* FIELD_GRANM_GetWork( const FIELD_GRANM* cp_sys, u32 idx )
{
	GF_ASSERT( cp_sys );
	GF_ASSERT( idx < cp_sys->wkmax );

	return &cp_sys->p_wkbuf[idx];
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションスピード設定
 *
 *	@param	p_sys			システムワーク
 *	@param	speed			アニメーションスピード
 */
//-----------------------------------------------------------------------------
void FIELD_GRANM_SetAnimeSpeed( FIELD_GRANM* p_sys, fx32 speed )
{
	p_sys->anime_speed = speed;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーションスピードの取得
 *
 *	@param	cp_sys	システムワーク
 *
 *	@return	アニメーションスピード
 */
//-----------------------------------------------------------------------------
fx32 FIELD_GRANM_GetAnimeSpeed( const FIELD_GRANM* cp_sys )
{
	return cp_sys->anime_speed;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションフラグの設定
 *
 *	@param	p_sys		システムワーク
 *	@param	flag		フラグ
 */
//-----------------------------------------------------------------------------
void FIELD_GRANM_SetAutoAnime( FIELD_GRANM* p_sys, BOOL flag )
{
	p_sys->anime_flag = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オートアニメーションフラグの取得
 *
 *	@param	cp_sys	システムワーク
 *		
 *	@retval	TRUE	オートアニメーション中
 *	@retval	FALSE	アニメーション停止中
 */
//-----------------------------------------------------------------------------
BOOL FIELD_GRANM_GetAutoAnime( const FIELD_GRANM* cp_sys )
{
	return  cp_sys->anime_flag;
}



//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションワーク　レンダーオブジェに関連付ける
 *
 *	@param	p_wk				ワーク
 *	@param	cp_mdlres		モデルリソース
 *	@param	cp_texres		テクスチャリソース					itp以外はNULLも可能
 *	@param	p_rendobj		関連付けるレンダーオブジェ
 */
//-----------------------------------------------------------------------------
void FIELD_GRANM_WORK_Bind( FIELD_GRANM_WORK* p_wk, const GFL_G3D_RES* cp_mdlres, const GFL_G3D_RES* cp_texres, NNSG3dRenderObj* p_rendobj )
{
	FIELD_GRANM_Work_Bind( p_wk, cp_mdlres, cp_texres, p_rendobj );
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションワーク	レンダーオブジェとの関連付けを解除
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_GRANM_WORK_Release( FIELD_GRANM_WORK* p_wk )
{
	FIELD_GRANM_Work_Release( p_wk );
}





//-----------------------------------------------------------------------------
/**
 *				プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションワークの初期化
 *
 *	@param	p_wk				ワーク
 *	@param	p_anmres		アニメーションリソース
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void FIELD_GRANM_Work_Init( FIELD_GRANM_WORK* p_wk, void* p_anmres, u32 heapID )
{
	GF_ASSERT( p_wk->p_anmobj == NULL );
	p_wk->p_anmobj = GFL_HEAP_AllocClearMemory( heapID, FIELD_GRANM_WORK_ANIMEOBJ_WORK_SIZE );	
	p_wk->p_anmres = p_anmres;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションワークの破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_GRANM_Work_Exit( FIELD_GRANM_WORK* p_wk )
{
	GF_ASSERT( p_wk->p_anmobj );
	GFL_HEAP_FreeMemory( p_wk->p_anmobj );
	p_wk->p_anmobj = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションワーク　レンダーオブジェに関連付ける
 *
 *	@param	p_wk					ワーク
 *	@param	cp_mdlres			モデルリソース
 *	@param	cp_texres			テクスチャリソース					itp以外はNULLも可能
 *	@param	p_rendobj			関連付けるレンダーオブジェ
 */
//-----------------------------------------------------------------------------
static void FIELD_GRANM_Work_Bind( FIELD_GRANM_WORK* p_wk, const GFL_G3D_RES* cp_mdlres, const GFL_G3D_RES* cp_texres, NNSG3dRenderObj* p_rendobj )
{
	const NNSG3dResMdl* cp_mdl;
	const NNSG3dResTex* cp_tex;

	GF_ASSERT( cp_mdlres );
	GF_ASSERT( p_rendobj );

	
	// モデル情報取得
	cp_mdl = NNS_G3dGetMdlByIdx( NNS_G3dGetMdlSet( GFL_G3D_GetResourceFileHeader( cp_mdlres ) ), 0 );

	// モデル内マテリアル数チェック
	GF_ASSERT( cp_mdl->info.numMat < FIELD_GRANM_WORK_ANIMEOBJ_WORK_MATNODE_MAX );

	if( cp_texres )
	{
		cp_tex = GFL_G3D_GetResTex( cp_texres );
	}
	else
	{
		cp_tex = NULL;
	}
	
	// アニメーションオブジェの初期化
	GFL_STD_MemClear( p_wk->p_anmobj, FIELD_GRANM_WORK_ANIMEOBJ_WORK_SIZE );
	NNS_G3dAnmObjInit( p_wk->p_anmobj, p_wk->p_anmres, cp_mdl, cp_tex );

	// レンダーオブジェに関連付け
	NNS_G3dRenderObjAddAnmObj( p_rendobj, p_wk->p_anmobj );

	p_wk->bind = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションワーク	レンダーオブジェとの関連付け解除
 *
 *	@param	p_wk			ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_GRANM_Work_Release( FIELD_GRANM_WORK* p_wk )
{
	p_wk->bind = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地面アニメーションワーク　フレーム設定
 *
 *	@param	p_wk			ワーク
 *	@param	frame			フレーム
 *
 *	@return	設定フレーム
 */
//-----------------------------------------------------------------------------
static fx32 FIELD_GRANM_Work_SetAnimeFrame( FIELD_GRANM_WORK* p_wk, fx32 frame )
{
	if( p_wk->bind )
	{
		fx32 max_frame;
		
		max_frame = NNS_G3dAnmObjGetNumFrame( p_wk->p_anmobj );
		frame			= frame % max_frame;
		
		NNS_G3dAnmObjSetFrame( p_wk->p_anmobj, frame );
	}

	return frame;
}



