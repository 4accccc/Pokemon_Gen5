//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wflby_3dmatrix.c
 *	@brief		広場部屋	３D座標管理関数
 *	@author		tomoya takahashi
 *	@data		2007.11.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "wflby_3dmatrix.h"

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

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief	wf2dシステムのポジション座標を３D座標に変換する
 *
 *	@param	cp_pos		ポジション
 *	@param	p_vec		３D座標
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMATRIX_GetPosVec( const WF2DMAP_POS* cp_pos, VecFx32* p_vec )
{
	p_vec->x = cp_pos->x * WFLBY_3DMATRIX_GRID_SIZ;
	p_vec->z = cp_pos->y * WFLBY_3DMATRIX_GRID_SIZ;
	p_vec->y = WFLBY_3DMATRIX_FLOOR_MAT;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３D座標をwf2dシステムのポジション座標に変換する
 *
 *	@param	cp_vec	３D座標
 *	@param	p_pos	ポジション
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMATRIX_GetVecPos( const VecFx32* cp_vec, WF2DMAP_POS* p_pos )
{
	p_pos->x = cp_vec->x / WFLBY_3DMATRIX_GRID_SIZ;
	p_pos->y = cp_vec->z / WFLBY_3DMATRIX_GRID_SIZ;
}





//----------------------------------------------------------------------------
/**
 *	@brief	汎用関数	テクスチャを展開して実データを破棄する
 *
 *	@param	pp_in		読み込んだバッファ格納先
 *	@param	p_handle	ハンドル
 *	@param	data_idx	データIDX
 *	@param	gheapID		ヒープID
 */
//-----------------------------------------------------------------------------
void WFLBY_3DMAPOBJ_TEX_LoatCutTex( GFL_G3D_RES** pp_in, ARCHANDLE* p_handle, u32 data_idx, u32 gheapID )
{
#if WB_FIX
	u32 tex_cut_size;
	void* p_file;
	NNSGfdTexKey	texKey;		// テクスチャキー
	NNSGfdTexKey	tex4x4Key;	// 4x4テクスチャキー
	NNSGfdPlttKey	plttKey;	// パレットキー
	NNSG3dResTex*	p_tex;

	// まず普通にモデルを読み込む
	p_file = GFL_ARCHDL_UTIL_Load( p_handle, data_idx, FALSE, GFL_HEAP_LOWID(gheapID) );

	// VRAMに展開
	{
		// 一時的に各ポインタを取得する
		p_tex		= NNS_G3dGetTex( p_file );
		// 転送する
		LoadVRAMTexture( p_tex );
	}

	// テクスチャキー取得
	{
		NNS_G3dTexReleaseTexKey( p_tex, &texKey, &tex4x4Key );
		plttKey = NNS_G3dPlttReleasePlttKey( p_tex );	
	}

	
	// テクスチャ部分を破棄する
	{
		tex_cut_size		= TEXRESM_TOOL_CutTexDataSizeGet( p_file );	// テクスチャを破棄した分のサイズを取得
		*pp_in		= GFL_HEAP_AllocMemory( gheapID, tex_cut_size );	// テクスチャを破棄したサイズのメモリを確保
		memcpy( *pp_in, p_file, tex_cut_size );	// メモリをコピー
	}

	// テクスチャキーを設定
	{
		p_tex		= NNS_G3dGetTex( *pp_in );
		NNS_G3dTexSetTexKey(p_tex, texKey, tex4x4Key);
		NNS_G3dPlttSetPlttKey(p_tex, plttKey);
	}


	// 一時ファイルを破棄
	GFL_HEAP_FreeMemory( p_file );
#else
	GFL_G3D_RES *g3dres;
	NNSG3dResTex*	p_tex;
	u8* texImgStartAddr;
	u32 newSize;
	
	// まず普通にモデルを読み込む
	g3dres = GFL_G3D_CreateResourceHandle(p_handle, data_idx);
	
	//VRAMに展開
	GFL_G3D_TransVramTexture(g3dres);
	
	//テクスチャイメージをVRAMへ展開し終わったので、実体を破棄
	p_tex = GFL_G3D_GetResTex(g3dres);
	GF_ASSERT(p_tex->texInfo.ofsTex != 0);
	texImgStartAddr = (u8*)p_tex + p_tex->texInfo.ofsTex;
	newSize = (u32)(texImgStartAddr - (u8*)p_file);	//ヒープの頭からテクスチャイメージまでのサイズ
	GFL_HEAP_ResizeMemory(p_file, newSize);
	
	*pp_in = g3dres;
#endif
}


//----------------------------------------------------------------------------
/**
 *	@brief	カリングチェック
 *
 *	@param	cp_mdl		モデル
 *	@param	cp_obj		描画オブジェ
 *
 *	@retval	TRUE	描画する
 *	@retval	FALSE	描画しない
 */
//-----------------------------------------------------------------------------
#if WB_FIX
BOOL WFLBY_3DMAPOBJ_MDL_BOXCheck( const GFL_G3D_OBJ* cp_mdl, const GFL_G3D_OBJSTATUS* cp_obj )
{
	VecFx32 matrix;
	VecFx32 scale;
	MtxFx33 mtx;
	MtxFx33 calc_mtx;
	u32 rot_x;
	u32 rot_y;
	u32 rot_z;

	D3DOBJ_GetMatrix( cp_obj,  &matrix.x, &matrix.y, &matrix.z );
	D3DOBJ_GetScale( cp_obj,  &scale.x, &scale.y, &scale.z );
	rot_x = D3DOBJ_GetRota( cp_obj, D3DOBJ_ROTA_WAY_X );
	rot_y = D3DOBJ_GetRota( cp_obj, D3DOBJ_ROTA_WAY_Y );
	rot_z = D3DOBJ_GetRota( cp_obj, D3DOBJ_ROTA_WAY_Z );

	// 回転行列作成
	MTX_Identity33( &mtx );
	MTX_RotX33( &calc_mtx, FX_SinIdx( rot_x ), FX_CosIdx( rot_x ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	MTX_RotZ33( &calc_mtx, FX_SinIdx( rot_z ), FX_CosIdx( rot_z ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	MTX_RotY33( &calc_mtx, FX_SinIdx( rot_y ), FX_CosIdx( rot_y ) );
	MTX_Concat33( &calc_mtx, &mtx, &mtx );
	
	// 描画チェック
	return BB_CullingCheck3DModel( cp_mdl->pModel, 
				&matrix,
				&mtx,
				&scale );
}
#endif
