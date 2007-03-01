//=============================================================================================
/**
 * @file	g3d_light.c                                                  
 * @brief	ライト設定
 * @date	
 */
//=============================================================================================
#include "gflib.h"

//=============================================================================================
//	型宣言
//=============================================================================================
struct _GFL_G3D_LIGHTING {
	u16			num;
	u16			color;
	VecFx16		vec;
};

//=============================================================================================
/**
 *
 *
 * 管理システム
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * ライト作成
 *
 * @param	num			使用ライトナンバー
 * @param	color		ライトの色
 * @param	vec			ライト方向ベクトル
 *
 * @return	g3Dlight	ライトハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_LIGHTING*
	GFL_G3D_LightCreate
		( const u16 num, const u16 color, const VecFx16* vec, HEAPID heapID )
{
	GFL_G3D_LIGHTING*	g3Dlight;
	//管理領域確保
	g3Dlight = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_G3D_LIGHTING) );

	g3Dlight->num	= num;
	g3Dlight->color	= color;
	g3Dlight->vec	= *vec;

	return g3Dlight;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ破棄
 *
 * @param	g3Dlight	ライトハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_LightDelete
		( GFL_G3D_LIGHTING* g3Dlight )
{
	GF_ASSERT( g3Dlight );

	GFL_HEAP_FreeMemory( g3Dlight );
}

//--------------------------------------------------------------------------------------------
/**
 * ライト反映
 *
 * @param	g3Dlight	ライトハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_LightSwitching
		( GFL_G3D_LIGHTING* g3Dlight )
{
	GF_ASSERT( g3Dlight );

	//ライト設定
	GFL_G3D_sysLightSet( g3Dlight->num, &g3Dlight->vec, g3Dlight->color );
}

//--------------------------------------------------------------------------------------------
/**
 * ライト方向の取得と変更
 *
 * @param	g3Dlight	ライトハンドル
 * @param	vec			ライトベクトルの格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_LightVecGet
		( GFL_G3D_LIGHTING* g3Dlight, VecFx16* vec )
{
	GF_ASSERT( g3Dlight );
	*vec = g3Dlight->vec;
}

void
	GFL_G3D_LightVecSet
		( GFL_G3D_LIGHTING* g3Dlight, VecFx16* vec )
{
	GF_ASSERT( g3Dlight );
	g3Dlight->vec = *vec;
}

//--------------------------------------------------------------------------------------------
/**
 * カメラ色の取得と変更
 *
 * @param	g3Dlight	ライトハンドル
 * @param	color		ライト色の格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_LightColorGet
		( GFL_G3D_LIGHTING* g3Dlight, u16* color )
{
	GF_ASSERT( g3Dlight );
	*color = g3Dlight->color;
}

void
	GFL_G3D_LightColorSet
		( GFL_G3D_LIGHTING* g3Dlight, u16* color )
{
	GF_ASSERT( g3Dlight );
	g3Dlight->color = *color;
}


