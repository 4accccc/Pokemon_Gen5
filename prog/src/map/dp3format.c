//=============================================================================
/**
 * @file	dp3format.h
 * @brief   マップエディターの出力ファイル定義
 * @author	ohno_katsunmi@gamefreak.co.jp
 * @date    2008.12.11
 */
//=============================================================================

#include <gflib.h>
#include "map/dp3format.h"


//-----------------------------------------------------------------------------
/**
 * @brief      index番目の法線ベクトル構造体ポインタを得る
 * @param[in]  index    
 * @param[in]  LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval     PositionSt
 */
//-----------------------------------------------------------------------------

NormalVtxSt* DP3MAP_GetNormalVtxSt(const int index,const NormalVtxFormat* pNormalVtx)
{
    u8* pData = (u8*)pNormalVtx;
	return (NormalVtxSt*)(pData + sizeof(NormalVtxFormat) + index * sizeof(NormalVtxSt));
}

//-----------------------------------------------------------------------------
/**
 * @brief       index番目の法線ベクトルを得る
 * @param[in]   index    
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[out]  VecFx32* 返す VecFx32
 * @param[out]  VecFx32* 返す VecFx32
 * @retval      none
 */
//-----------------------------------------------------------------------------

void DP3MAP_GetNormalVtxPosition(const int index,const NormalVtxFormat* pNormalVtx,VecFx16* vc1 ,VecFx16* vc2)
{
    NormalVtxSt* norVtxSt = DP3MAP_GetNormalVtxSt(index, pNormalVtx);

    GF_ASSERT((pNormalVtx->width*pNormalVtx->height) > index );
    VEC_Fx16Set( vc1, norVtxSt->vecN1_x, norVtxSt->vecN1_y, -norVtxSt->vecN1_z);
    VEC_Fx16Set( vc2, norVtxSt->vecN2_x, norVtxSt->vecN2_y, -norVtxSt->vecN2_z);

//    VEC_Fx16Set( vc1, norVtxSt[index].vecN1_x, norVtxSt[index].vecN1_y, -norVtxSt[index].vecN1_z);
//    VEC_Fx16Set( vc2, norVtxSt[index].vecN2_x, norVtxSt[index].vecN2_y, -norVtxSt[index].vecN2_z);
}

//-----------------------------------------------------------------------------
/**
 * @brief       index番目の内積値を得る
 * @param[in]   index    
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[out]  内積１
 * @param[out]  内積２
 * @retval      none
 */
//-----------------------------------------------------------------------------

void DP3MAP_GetNormalVtxInnerProduct(const int index,const NormalVtxFormat* pNormalVtx,fx16* ip1 ,fx16* ip2)
{
    NormalVtxSt* norVtxSt = DP3MAP_GetNormalVtxSt(index, pNormalVtx);

    GF_ASSERT((pNormalVtx->width*pNormalVtx->height) > index );
    *ip1 = norVtxSt->vecN1_D;
    *ip2 = norVtxSt->vecN2_D;

//    *ip1 = norVtxSt[index].vecN1_D;
//	  *ip2 = norVtxSt[index].vecN2_D;
}

//-----------------------------------------------------------------------------
/**
 * @brief       index番目のアトリビュートBITを得る
 * @param[in]   index    
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[out]  内積１
 * @param[out]  内積２
 * @retval      none
 */
//-----------------------------------------------------------------------------

u32 DP3MAP_GetNormalVtxAttrBit(const int index,const NormalVtxFormat* pNormalVtx)
{
    NormalVtxSt* norVtxSt = DP3MAP_GetNormalVtxSt(index, pNormalVtx);

    GF_ASSERT((pNormalVtx->width*pNormalVtx->height) > index );
	return norVtxSt->attr;	
//    return norVtxSt[index].attr;
}

//-----------------------------------------------------------------------------
/**
 * @brief       index番目のtryangleTypeを得る
 * @param[in]   index    
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval      三角形の形のタイプ  ＼ = 0  ／ = 1
 */
//-----------------------------------------------------------------------------

u32 DP3MAP_GetNormalVtxTriangleType(const int index,const NormalVtxFormat* pNormalVtx)
{
    NormalVtxSt* norVtxSt = DP3MAP_GetNormalVtxSt(index, pNormalVtx);

    GF_ASSERT((pNormalVtx->width*pNormalVtx->height) > index );
    return norVtxSt->tryangleType;

//    return norVtxSt[index].tryangleType;
}

//-----------------------------------------------------------------------------
/**
 * @brief       与えられた位置の法線ベクトルと内積を返す
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[in]   posInBlock*   いる座標
 * @param[in]   map_width     ＭＡＰの長さ
 * @param[in]   map_height    ＭＡＰの高さ
 * @param[out]  nomalOut      法線ベクトル
 * @retval      内積
 */
//-----------------------------------------------------------------------------

fx32 DP3MAP_GetNormalVector( const NormalVtxFormat* pNormalVtx, const VecFx32* posInBlock,
                             const fx32 map_width, const fx32 map_height, VecFx32* normalOut )
{
	fx32			grid_w, grid_x, grid_z;
	fx32			grid_w_x, grid_w_z;
	u32				grid_idx;
	VecFx32			pos, vecN;
	fx32			by, valD;
	NormalVtxSt*	nvs;
	u32				attrAdrs = (u32)pNormalVtx;

	VEC_Set( &pos, posInBlock->x + map_width/2, posInBlock->y, posInBlock->z + map_width/2 );
	//グリッド内情報取得
	grid_w_x = map_width / pNormalVtx->width;	//マップ幅をグリッド数で分割
	grid_w_z = map_height / pNormalVtx->height;	//マップ幅をグリッド数で分割
	grid_idx = ( pos.z / grid_w_z ) * pNormalVtx->width + ( pos.x / grid_w_x );
	grid_x = pos.x % grid_w_x;
	grid_z = pos.z % grid_w_z;

	// 比率にする
	grid_x = FX_Div( grid_x, grid_w_x );
	grid_z = FX_Div( grid_z, grid_w_z );

	//情報取得(軸の取り方が違うので法線ベクトルはZ反転)
	nvs = (NormalVtxSt*)(attrAdrs + sizeof(NormalVtxFormat) + grid_idx * sizeof(NormalVtxSt));

	//グリッド内三角形の判定
	if( nvs->tryangleType == 0 ){
		//0-2-1,3-1-2のパターン
		//　｜－－－－－－｜
		//　｜　　　　　/ ｜
		//　｜　　　　/ 　｜
		//　｜　　　/ 　　｜
		//　｜　　/ 　　　｜
		//　｜　/ 　　　　｜
		//　｜/ 　　　　　｜
		//　｜－－－－－－｜
		if( grid_x + grid_z < FX32_ONE ){
			VEC_Set( normalOut, nvs->vecN1_x, nvs->vecN1_y, -nvs->vecN1_z );
			valD = nvs->vecN1_D;
		} else {
			VEC_Set( normalOut, nvs->vecN2_x, nvs->vecN2_y, -nvs->vecN2_z );
			valD = nvs->vecN2_D;
		}
	} else {
		//2-3-0,1-0-3のパターン
		//　｜－－－－－－｜
		//　｜・　　　　　｜
		//　｜　・　　　　｜
		//　｜　　・　　　｜
		//　｜　　　・　　｜
		//　｜　　　　・　｜
		//　｜　　　　　・｜
		//　｜－－－－－－｜
		if( grid_x > grid_z ){
			VEC_Set( normalOut, nvs->vecN1_x, nvs->vecN1_y, -nvs->vecN1_z );
			valD = nvs->vecN1_D;
		} else {
			VEC_Set( normalOut, nvs->vecN2_x, nvs->vecN2_y, -nvs->vecN2_z );
			valD = nvs->vecN2_D;
		}
	}
    return valD;
}

//-----------------------------------------------------------------------------
/**
 * @brief      index番目の配置データ構造体ポインタを得る
 * @param[in]  index    
 * @param[in]  LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval     PositionSt
 */
//-----------------------------------------------------------------------------

PositionSt* DP3MAP_GetLayoutPositionSt(const int index,const LayoutFormat* pLayout)
{
    PositionSt* objStatus = (PositionSt*)&pLayout->posData;
    GF_ASSERT(pLayout->count > index );
    return &objStatus[index];
}

//-----------------------------------------------------------------------------
/**
 * @brief      index番目の配置を得る
 * @param[in]  index    
 * @param[in]  LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @param[out]  VecFx32* 返す VecFx32
 * @retval  none
 */
//-----------------------------------------------------------------------------

void DP3MAP_GetLayoutPosition(const int index,const LayoutFormat* pLayout,VecFx32* vc )
{
    PositionSt* objStatus = (PositionSt*)&pLayout->posData;

    GF_ASSERT(pLayout->count > index );
    VEC_Set( vc, objStatus[index].xpos, objStatus[index].ypos, -objStatus[index].zpos );
}

//-----------------------------------------------------------------------------
/**
 * @brief      index番目の角度を得る
 * @param[in]  index    
 * @param[in]  LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval      角度
 */
//-----------------------------------------------------------------------------

u16 DP3MAP_GetLayoutRotate(const int index,const LayoutFormat* pLayout )
{
    PositionSt* objStatus = (PositionSt*)&pLayout->posData;

    GF_ASSERT(pLayout->count > index );
    return objStatus[index].rotate;
}

//-----------------------------------------------------------------------------
/**
 * @brief       index番目のビルボードフラグを得る
 * @param[in]   index
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval      TRUEならビルボード
 */
//-----------------------------------------------------------------------------

#if 0
BOOL DP3MAP_GetLayoutBillboard(const int index,const LayoutFormat* pLayout )
{
    PositionSt* objStatus = (PositionSt*)&pLayout->posData;

    GF_ASSERT(pLayout->count > index );
    return objStatus[index].billboard;
}
#endif

//-----------------------------------------------------------------------------
/**
 * @brief       index番目の配置物IDを得る
 * @param[in]   index
 * @param[in]   LayoutFormat* レイアウトバイナリの先頭ポインタ
 * @retval      配置物ID
 */
//-----------------------------------------------------------------------------

u8 DP3MAP_GetLayoutResourceID(const int index,const LayoutFormat* pLayout )
{
    PositionSt* objStatus = (PositionSt*)&pLayout->posData;

    GF_ASSERT(pLayout->count > index );
    return objStatus[index].resourceID;
}



