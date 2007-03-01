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
struct _GFL_G3D_LIGHTSET {
	GFL_G3D_LIGHT	light[4];
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
 * ライトセット作成
 *
 * @param	light		ライト設定データポインタ
 * @param	lightCount	ライト数
 *
 * @return	g3Dlightset	ライトセットハンドル
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_LIGHTSET*
	GFL_G3D_LightCreate
		( const GFL_G3D_LIGHTSET_SETUP* setUp, HEAPID heapID )
{
	GFL_G3D_LIGHTSET*	g3Dlightset;
	int	i;

	//管理領域確保
	g3Dlightset = GFL_HEAP_AllocMemoryClear( heapID, sizeof(GFL_G3D_LIGHTSET) );

	//ライトセットを取得。セットアップ後のcolor=0は存在しないライトとみなす。
	for( i=0; i<setUp->lightCount; i++ ){
		g3Dlightset->light[ setUp->lightData[i].num ] = setUp->lightData[i].data;
	}
	return g3Dlightset;
}

//--------------------------------------------------------------------------------------------
/**
 * ライトセット破棄
 *
 * @param	g3Dlightset	ライトセットハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_LightDelete
		( GFL_G3D_LIGHTSET* g3Dlightset )
{
	GF_ASSERT( g3Dlightset );

	GFL_HEAP_FreeMemory( g3Dlightset );
}

//--------------------------------------------------------------------------------------------
/**
 * ライトセット反映
 *
 * @param	g3Dlightset	ライトセットハンドル
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_LightSwitching
		( GFL_G3D_LIGHTSET* g3Dlightset )
{
	VecFx16	initVec = { -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) };
	int	i;

	GF_ASSERT( g3Dlightset );

	//ライト設定
	for( i=0; i<4; i++ ){
		if( g3Dlightset->light[i].color ){
			GFL_G3D_sysLightSet( i, &g3Dlightset->light[i].vec, g3Dlightset->light[i].color );
		} else {
			GFL_G3D_sysLightSet( i, &initVec, 0 );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ライト方向の取得と変更
 *
 * @param	g3Dlightset	ライトセットハンドル
 * @param	idx			ライトＩＮＤＥＸ
 * @param	vec			ライトベクトルの格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_LightVecGet
		( GFL_G3D_LIGHTSET* g3Dlightset, u8 idx, VecFx16* vec )
{
	GF_ASSERT( g3Dlightset );
	*vec = g3Dlightset->light[ idx ].vec;
}

void
	GFL_G3D_LightVecSet
		( GFL_G3D_LIGHTSET* g3Dlightset, u8 idx, VecFx16* vec )
{
	GF_ASSERT( g3Dlightset );
	g3Dlightset->light[ idx ].vec = *vec;
}

//--------------------------------------------------------------------------------------------
/**
 * ライト色の取得と変更
 *
 * @param	g3Dlightset	ライトセットハンドル
 * @param	idx			ライトＩＮＤＥＸ
 * @param	color		ライト色の格納もしくは参照ワークポインタ	
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_LightColorGet
		( GFL_G3D_LIGHTSET* g3Dlightset, u8 idx, u16* color )
{
	GF_ASSERT( g3Dlightset );
	*color = g3Dlightset->light[ idx ].color;
}

void
	GFL_G3D_LightColorSet
		( GFL_G3D_LIGHTSET* g3Dlightset, u8 idx, u16* color )
{
	GF_ASSERT( g3Dlightset );
	g3Dlightset->light[ idx ].color = *color;
}


