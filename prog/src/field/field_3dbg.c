//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_3dbg.c
 *	@brief		フィールド　３ＤＢＧ面
 *	@author		tomoya takahshi
 *	@date		2009.03.30
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "field_3dbg.h"



//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------

#ifdef PM_DEBUG
#endif


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	テクスチャ情報
//=====================================
typedef struct {
	u8		texsiz_s;		// GXTexSizeS
	u8		texsiz_t;		// GXTexSizeT
	u8		repeat;			// GXTexRepeat
	u8		flip;			// GXTexFlip
	u8		texfmt;			// GXTexFmt
	u8		texpltt;		// GXTexPlttColor0

	u8		load;			// 読み込みが行われているか
	u8		trans_req;	// 転送リクエスト

	GFL_G3D_RES* p_textmp;	//  テクスチャデータ
} FIELD_3DBG_TEX;

//-------------------------------------
///	描画面情報
//=====================================
typedef struct {
	fx32	size_half_x;
	fx32	size_half_y;
	fx32	z;
	fx32	base_scale;
	fx32	u0;
	fx32	u1;
	fx32	v0;
	fx32	v1;
	MtxFx33 tex_mtx;
} FIELD_3DPLANE;



//-------------------------------------
///	フィールド３ＤＢＧシステム
//=====================================
struct _FIELD_3DBG {
	u16		size_x;
	u16		size_y;
	fx32	camera_dist;
	BOOL	visible;
	s32		scroll_x;
	s32		scroll_y;
	fx32	scale_x;	
	fx32	scale_y;	
	u16		rotate;
	u8		polygon_id;
	u8		alpha;			// アルファ設定
	
	
	FIELD_3DBG_TEX	tex;
	FIELD_3DPLANE	plane;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	テクスチャ情報
//=====================================
static void FIELD_3DBG_TEX_Init( FIELD_3DBG_TEX* p_wk, ARCHANDLE* p_handle, const FIELD_3DBG_WRITE_DATA* cp_data, u32 heapID );
static void FIELD_3DBG_TEX_Delete( FIELD_3DBG_TEX* p_wk );
static void FIELD_3DBG_TEX_TexTrans( FIELD_3DBG_TEX* p_wk );
static BOOL FIELD_3DBG_TEX_IsLoad( const FIELD_3DBG_TEX* cp_wk );


//-------------------------------------
///	描画情報
//=====================================
static void FIELD_3DBG_PLANE_SetData( FIELD_3DPLANE* p_wk, const FIELD_3DBG* cp_sys, const FIELD_3DBG_TEX* cp_tex, const MtxFx44* cp_projection );
static void FIELD_3DBG_PLANE_Write( const FIELD_3DPLANE* cp_wk );


//-------------------------------------
///	描画関係
//=====================================
static void FIELD_3DBG_SetPolyAttr( const FIELD_3DBG* cp_sys );
static void FIELD_3DBG_SetTexParam( const FIELD_3DBG* cp_sys );

//----------------------------------------------------------------------------
/**
 *	@brief	３DBG描画システム	生成
 *
 *	@param	cp_setup		基本情報
 *	@param	heapID			ヒープID		
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
FIELD_3DBG* FIELD_3DBG_Create( const FIELD_3DBG_SETUP* cp_setup, u32 heapID )
{
	FIELD_3DBG* p_sys;

	// メモリ確保
	p_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_3DBG) );

	// 基本情報の設定
	p_sys->size_x		= cp_setup->size_x;
	p_sys->size_y		= cp_setup->size_y;
	p_sys->camera_dist	= cp_setup->camera_dist;
	p_sys->polygon_id	= cp_setup->polygon_id;

	// 描画ON
	p_sys->visible = TRUE;

	return p_sys;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３DBG描画システム	破棄
 *
 *	@param	p_sys		システムワーク
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_Delete( FIELD_3DBG* p_sys )
{
	GF_ASSERT( p_sys );

	// テクスチャ情報の破棄
	if( FIELD_3DBG_TEX_IsLoad( &p_sys->tex ) ){
		FIELD_3DBG_TEX_Delete( &p_sys->tex );
	}

	GFL_HEAP_FreeMemory( p_sys );
}
	 
//----------------------------------------------------------------------------
/**
 *	@brief	3DBG描画システム	描画
 *
 *	@param	p_sys			システムワーク
 *	@param	cp_camera		カメラワーク
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_Write( FIELD_3DBG* p_sys )
{
	MtxFx44 projection;
	
	if( !p_sys ){
		return ;
	}
	
	if( (!p_sys->visible) ){
		return;
	}

	//  テクスチャが読み込まれているか？
	if( !FIELD_3DBG_TEX_IsLoad( &p_sys->tex ) ){
		return;
	}

	// 描画処理
	// NNS系のジオメトリコマンド展開
	NNS_G3dGeFlushBuffer();

	// 射影行列設定
	G3_MtxMode( GX_MTXMODE_PROJECTION );
	projection = *NNS_G3dGlbGetProjectionMtx();
	G3_LoadMtx44( &projection );
	
	// カメラは単位行列
	G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );
	G3_Identity();

	G3_PushMtx();
	{
	
		// 射影情報からcameraからの距離のときの、XサイズYサイズを求める
		// texture情報からUV値を求める
		FIELD_3DBG_PLANE_SetData( &p_sys->plane, p_sys, &p_sys->tex, &projection );

		// ポリゴンアトリビュート
		FIELD_3DBG_SetPolyAttr( p_sys );
		
		// textureパラメータ
		FIELD_3DBG_SetTexParam( p_sys );

		// 板描画
		FIELD_3DBG_PLANE_Write( &p_sys->plane );
	}
	G3_PopMtx(1);
}

//----------------------------------------------------------------------------
/**
 *	@brief	3DBG描画システム	テクスチャ展開
 *
 *	@param	p_sys	システムワーク
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_VBlank( FIELD_3DBG* p_sys )
{
	if( p_sys ){
		// テクスチャの展開
		FIELD_3DBG_TEX_TexTrans( &p_sys->tex );
	}
}	



//----------------------------------------------------------------------------
/**
 *	@brief	描画情報のクリア
 *
 *	@param	p_sys	システムワーク
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_ClearWriteData( FIELD_3DBG* p_sys )
{
	GF_ASSERT( p_sys );

	// テクスチャ破棄
	if( FIELD_3DBG_TEX_IsLoad( &p_sys->tex ) ){
		FIELD_3DBG_TEX_Delete( &p_sys->tex );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	テクスチャ設定
 *
 *	@param	p_sys		システムワーク
 *	@param	p_handle	アークハンドル
 *	@param	cp_data		データ
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetWriteData( FIELD_3DBG* p_sys, ARCHANDLE* p_handle, const FIELD_3DBG_WRITE_DATA* cp_data, u32 heapID )
{
	GF_ASSERT( p_sys );
	GF_ASSERT( p_handle );
	GF_ASSERT( cp_data );
	
	// 破棄
	if( FIELD_3DBG_TEX_IsLoad( &p_sys->tex ) ){
		FIELD_3DBG_TEX_Delete( &p_sys->tex );
	}

	// 読み込み
	FIELD_3DBG_TEX_Init( &p_sys->tex, p_handle, cp_data, heapID );

	p_sys->alpha		= cp_data->alpha;			// アルファ設定

	p_sys->scale_x		= FX32_ONE;
	p_sys->scale_y		= FX32_ONE;

  p_sys->rotate = 0;
}



//----------------------------------------------------------------------------
/**
 *	@brief	描画ON・OFF
 *
 *	@param	p_sys	システム
 *	@param	flag	フラグ
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetVisible( FIELD_3DBG* p_sys, BOOL flag )
{
	GF_ASSERT( p_sys );
	p_sys->visible = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画ON/OFFの取得
 *
 *	@param	cp_sys	システムワーク
 *
 *	@retval	TRUE	表示
 *	@retval	FALSE	非表示
 */
//-----------------------------------------------------------------------------
BOOL FIELD_3DBG_GetVisible( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->visible;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３DBG　スクリーンサイズの設定
 *	
 *	@param	p_sys		システム
 *	@param	size_x		ピクセル単位のＢＧ０に描画するさいの面サイズ 
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetScreenSizeX( FIELD_3DBG* p_sys, u16 size_x )
{
	GF_ASSERT( p_sys );
	p_sys->size_x = size_x;
}

//----------------------------------------------------------------------------
/**
 *	@brief	3DBG	スクリーンサイズの取得
 *
 *	@param	cp_sys		ワーク
 *
 *	@return	スクリーンサイズ
 */
//-----------------------------------------------------------------------------
u16 FIELD_3DBG_GetScreenSizeX( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->size_x;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３DBG　スクリーンサイズの設定
 *	
 *	@param	p_sys		システム
 *	@param	size_y		ピクセル単位のＢＧ０に描画するさいの面サイズ 
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetScreenSizeY( FIELD_3DBG* p_sys, u16 size_y )
{
	GF_ASSERT( p_sys );
	p_sys->size_y = size_y;
}

//----------------------------------------------------------------------------
/**
 *	@brief	3DBG	スクリーンサイズの取得
 *
 *	@param	cp_sys		ワーク
 *
 *	@return	スクリーンサイズ
 */
//-----------------------------------------------------------------------------
u16 FIELD_3DBG_GetScreenSizeY( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->size_x;
}


//----------------------------------------------------------------------------
/**
 *	@brief	カメラからの距離を設定
 *
 *	@param	p_sys		システムワーク
 *	@param	dist		距離
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetCameraDist( FIELD_3DBG* p_sys, fx32 dist )
{
	GF_ASSERT( p_sys );

	p_sys->camera_dist = dist;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラからの距離を取得
 *
 *	@param	cp_sys		システムワーク
 *
 *	@return	距離
 */
//-----------------------------------------------------------------------------
fx32 FIELD_3DBG_GetCameraDist( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->camera_dist;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポリゴンIDの設定
 *
 *	@param	p_sys		システムワーク
 *	@param	polygon_id	ポリゴンID
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetPolygonID( FIELD_3DBG* p_sys, u8 polygon_id )
{
	GF_ASSERT( p_sys );
	p_sys->polygon_id = polygon_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポリゴンIDの取得
 *
 *	@param	cp_sys		システムワーク
 *
 *	@return	ポリゴンID
 */
//-----------------------------------------------------------------------------
u8 FIELD_3DBG_GetPolygonID( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->polygon_id;
}


//----------------------------------------------------------------------------
/**
 *	@brief	リピート設定
 *
 *	@param	p_sys		システムワーク
 *	@param	repeat		リピート
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetRepeat( FIELD_3DBG* p_sys, GXTexRepeat repeat )
{
	GF_ASSERT( p_sys );
	p_sys->tex.repeat = repeat;
}

//----------------------------------------------------------------------------
/**
 *	@brief	リピート取得
 *	
 *	@param	cp_sys		システムワーク
 *
 *	@return	リピート
 */
//-----------------------------------------------------------------------------
GXTexRepeat FIELD_3DBG_GetRepeat( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->tex.repeat;
}


//----------------------------------------------------------------------------
/**
 *	@brief	フリップ設定
 *	
 *	@param	p_sys	システムワーク
 *	@param	flip	フリップ
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetFlip( FIELD_3DBG* p_sys, GXTexFlip flip )
{
	GF_ASSERT( p_sys );
	p_sys->tex.flip = flip;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フリップ取得
 *
 *	@param	cp_sys	システムワーク
 *
 *	@return	フリップ
 */
//-----------------------------------------------------------------------------
GXTexFlip FIELD_3DBG_GetFlip( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->tex.flip;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アルファ設定
 *
 *	@param	p_sys	システムワーク
 *	@param	alpha	アルファ 
// *使用には注意が必要です。
// 半透明ポリゴンがフィールドに描画されている場所では
// 半透明にしないでください。
// 重なり方がおかしくなります。
 *
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetAlpha( FIELD_3DBG* p_sys, u8 alpha )
{
	GF_ASSERT( p_sys );
	p_sys->alpha = alpha;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アルファ取得
 *
 *	@param	cp_sys	システムワーク
 *
 *	@return	アルファ
 */
//-----------------------------------------------------------------------------
u8 FIELD_3DBG_GetAlpha( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->alpha;
}


//----------------------------------------------------------------------------
/**
 *	@brief	スクロールｘ座標
 *
 *	@param	p_sys	システムワーク
 *	@param	x		ｘ座標
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetScrollX( FIELD_3DBG* p_sys, s32 x )
{
	GF_ASSERT( p_sys );
	p_sys->scroll_x = x;
}
s32 FIELD_3DBG_GetScrollX( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->scroll_x;
}

//----------------------------------------------------------------------------
/**
 *	@brief	スクロールｙ座標
 *
 *	@param	p_sys
 *	@param	y 
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetScrollY( FIELD_3DBG* p_sys, s32 y )
{
	GF_ASSERT( p_sys );
	p_sys->scroll_y = y;
}
s32 FIELD_3DBG_GetScrollY( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->scroll_y;
}


//----------------------------------------------------------------------------
/**
 *	@brief	回転
 *
 *	@param	p_sys		システム
 *	@param	rotate		回転角度
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetRotate( FIELD_3DBG* p_sys, u16 rotate )
{
	GF_ASSERT( p_sys );
	p_sys->rotate = rotate;
}
u16 FIELD_3DBG_GetRotate( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->rotate;
}


//----------------------------------------------------------------------------
/**
 *	@brief	スケールｘ座標
 *
 *	@param	p_sys	システムワーク
 *	@param	x		ｘスケール値
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetScaleX( FIELD_3DBG* p_sys, fx32 x )
{
	GF_ASSERT( p_sys );
	p_sys->scale_x = x;
}
fx32 FIELD_3DBG_GetScaleX( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->scale_x;
}

//----------------------------------------------------------------------------
/**
 *	@brief	スケールY座標
 *
 *	@param	p_sys
 *	@param	y 
 */
//-----------------------------------------------------------------------------
void FIELD_3DBG_SetScaleY( FIELD_3DBG* p_sys, fx32 y )
{
	GF_ASSERT( p_sys );
	p_sys->scale_y = y;
}
fx32 FIELD_3DBG_GetScaleY( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->scale_y;
}





//-----------------------------------------------------------------------------
/**
 *			プライベート関数郡
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief	テクスチャ初期化
 *
 *	@param	p_wk			ワーク
 *	@param	p_handle		ハンドル
 *	@param	cp_data			データ
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void FIELD_3DBG_TEX_Init( FIELD_3DBG_TEX* p_wk, ARCHANDLE* p_handle, const FIELD_3DBG_WRITE_DATA* cp_data, u32 heapID )
{
	BOOL result;
	
	GF_ASSERT( p_wk );
	GF_ASSERT( p_handle );
	GF_ASSERT( cp_data );
	
	// 基本情報設定
	p_wk->texsiz_s	= cp_data->texsiz_s;		// GXTexSizeS
	p_wk->texsiz_t	= cp_data->texsiz_t;		// GXTexSizeT
	p_wk->repeat	= cp_data->repeat;			// GXTexRepeat
	p_wk->flip		= cp_data->flip;			// GXTexFlip
	p_wk->texfmt	= cp_data->texfmt;			// GXTexFmt
	p_wk->texpltt	= cp_data->texpltt;			// GXTexPlttColor0

	// texture読み込み
	p_wk->p_textmp = GFL_G3D_CreateResourceHandle( p_handle, cp_data->nsbtex_id );

	result = GFL_G3D_AllocVramTexture( p_wk->p_textmp );
	GF_ASSERT( result );

	// 読み込み完了
	p_wk->load			= TRUE;
	p_wk->trans_req		= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	テクスチャ破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_3DBG_TEX_Delete( FIELD_3DBG_TEX* p_wk )
{
	BOOL result;
	GF_ASSERT( p_wk->load );
	
	
	// 破棄
	result = GFL_G3D_FreeVramTexture( p_wk->p_textmp );
	GF_ASSERT(result);
	GFL_G3D_DeleteResource( p_wk->p_textmp );
	p_wk->p_textmp = NULL;


	p_wk->load = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	テクスチャVRAM転送
 *
 *	@param	cp_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_3DBG_TEX_TexTrans( FIELD_3DBG_TEX* p_wk )
{
	BOOL result;
	
	if( p_wk->trans_req ){

		result = GFL_G3D_TransOnlyTexture( p_wk->p_textmp );
		GF_ASSERT( result );
 
		p_wk->trans_req = FALSE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	テクスチャ情報が入っているか
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	入っている
 *	@retval	FALSE	入ってない
 */
//-----------------------------------------------------------------------------
static BOOL FIELD_3DBG_TEX_IsLoad( const FIELD_3DBG_TEX* cp_wk )
{
	GF_ASSERT( cp_wk );
	return cp_wk->load;
}



//----------------------------------------------------------------------------
/**
 *	@brief	面情報の計算
 *
 *	@param	p_wk			ワーク
 *	@param	cp_sys			システムワーク
 *	@param	cp_tex			textureデータ
 *	@param	cp_projection	射影行列
 */
//-----------------------------------------------------------------------------
static void FIELD_3DBG_PLANE_SetData( FIELD_3DPLANE* p_wk, const FIELD_3DBG* cp_sys, const FIELD_3DBG_TEX* cp_tex, const MtxFx44* cp_projection )
{
	fx32 u_dist;
	fx32 v_dist;
	MtxFx33 mtx_work;

	// 射影行列から、cameraからcamera_dist先の、画面高さ、幅を求める
	p_wk->size_half_y	= FX_Div( cp_sys->camera_dist, cp_projection->_11 );
	p_wk->size_half_x	= FX_Div( cp_sys->camera_dist, cp_projection->_00 );
	p_wk->z				= cp_sys->camera_dist;

	// ベーススケール値を求める
	if( (p_wk->size_half_y > p_wk->size_half_x) ){
		if( p_wk->size_half_y > p_wk->z ){
			p_wk->base_scale = FX_Div( p_wk->size_half_y, 4<<FX32_SHIFT );
		}else{
			p_wk->base_scale = FX_Div( p_wk->z, 4<<FX32_SHIFT );
		}
	}else{
		if( p_wk->size_half_x > p_wk->z ){
			p_wk->base_scale = FX_Div( p_wk->size_half_x, 4<<FX32_SHIFT );
		}else{
			p_wk->base_scale = FX_Div( p_wk->z, 4<<FX32_SHIFT );
		}
	}
	p_wk->size_half_x	= FX_Div( p_wk->size_half_x, p_wk->base_scale );
	p_wk->size_half_y	= FX_Div( p_wk->size_half_y, p_wk->base_scale );
	p_wk->z				= FX_Div( p_wk->z, p_wk->base_scale );

	// UV値を求める
	p_wk->u0 = cp_sys->scroll_x<<FX32_SHIFT;
	p_wk->v0 = cp_sys->scroll_y<<FX32_SHIFT;
	p_wk->u1 = p_wk->u0 + (cp_sys->size_x<<FX32_SHIFT);
	p_wk->v1 = p_wk->v0 + (cp_sys->size_y<<FX32_SHIFT);


	// textureの変換行列を求める
	MTX_Identity33( &p_wk->tex_mtx );
	MTX_RotZ33( &p_wk->tex_mtx, FX_SinIdx(cp_sys->rotate), FX_CosIdx(cp_sys->rotate) );
	MTX_Scale33( &mtx_work, cp_sys->scale_x, cp_sys->scale_y, FX32_ONE );
	MTX_Concat33( &p_wk->tex_mtx, &mtx_work, &p_wk->tex_mtx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	面の描画
 *		
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_3DBG_PLANE_Write( const FIELD_3DPLANE* cp_wk )
{
  // texture変換設定
  G3_MtxMode( GX_MTXMODE_TEXTURE );
  G3_Identity();
  G3_MultMtx33( &cp_wk->tex_mtx );

  // 平面の描画
  G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );
  G3_Scale( cp_wk->base_scale, cp_wk->base_scale, cp_wk->base_scale );

  G3_Begin( GX_BEGIN_QUADS );

  //平面ポリゴンなので法線ベクトルは4頂点で共用
  G3_TexCoord( cp_wk->u0, cp_wk->v0 );
  G3_Vtx( -cp_wk->size_half_x, cp_wk->size_half_y, -cp_wk->z );

  G3_TexCoord( cp_wk->u0, cp_wk->v1 );
  G3_Vtx( -cp_wk->size_half_x, -cp_wk->size_half_y, -cp_wk->z );

  G3_TexCoord( cp_wk->u1, cp_wk->v1 );
  G3_Vtx( cp_wk->size_half_x, -cp_wk->size_half_y, -cp_wk->z );

  G3_TexCoord( cp_wk->u1, cp_wk->v0 );
  G3_Vtx( cp_wk->size_half_x, cp_wk->size_half_y, -cp_wk->z );

  G3_End();
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポリゴンアトリビュート設定
 *
 *	@param	cp_sys	システム
 */
//-----------------------------------------------------------------------------
static void FIELD_3DBG_SetPolyAttr( const FIELD_3DBG* cp_sys )
{
	// マテリアル設定
	G3_MaterialColorDiffAmb( GX_RGB( 31,31,31 ), GX_RGB( 31,31,31 ), TRUE );
	
	G3_PolygonAttr( 0, GX_POLYGONMODE_MODULATE, GX_CULL_BACK, 
			cp_sys->polygon_id, cp_sys->alpha, GX_POLYGON_ATTR_MISC_FOG );
}

//----------------------------------------------------------------------------
/**
 *	@brief	テクスチャーパラメータ設定
 *
 *	@param	cp_sys	システム
 */
//-----------------------------------------------------------------------------
static void FIELD_3DBG_SetTexParam( const FIELD_3DBG* cp_sys )
{
	GF_ASSERT( cp_sys->tex.load );
	G3_TexImageParam( cp_sys->tex.texfmt,
			GX_TEXGEN_VERTEX, 
			cp_sys->tex.texsiz_s, cp_sys->tex.texsiz_t,
			cp_sys->tex.repeat, cp_sys->tex.flip, 
			cp_sys->tex.texpltt,
			NNS_GfdGetTexKeyAddr(GFL_G3D_GetTextureDataVramKey(cp_sys->tex.p_textmp)) );

	G3_TexPlttBase( NNS_GfdGetPlttKeyAddr(GFL_G3D_GetTexturePlttVramKey(cp_sys->tex.p_textmp)),
			cp_sys->tex.texfmt );
}




