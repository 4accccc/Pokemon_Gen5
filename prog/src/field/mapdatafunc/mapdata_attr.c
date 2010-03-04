//=============================================================================
/**
 * @file	mapdata_attr.c
 * @brief   マップデータ高さアトリビュート取得
 * @author	Saito @gamefreak.co.jp
 */
//=============================================================================

#include "wb_attr_data.h"
#include "field/field_const.h"   //for FIELD_CONST_GRID_SIZE
#include "map/dp3format.h"       //for NormalVtxFormat
#include "mapdata_attr.h"

//データ
#include "wb_bin.cdat"

//----------------------------------------------------------------------------
/**
 *	@brief	高さ　アトリビュート取得関数
 */
//-----------------------------------------------------------------------------
void MAPDATA_ATR_GetAttrFunc( FLD_G3D_MAP_ATTRINFO* attrInfo, const u8 idx, const u32 attrAdrs,
    const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
	fx32			grid_w, grid_x, grid_z;
	u32				grid_idx;
	u32				grid_count;
	VecFx32			pos, vecN;
	fx32			by, valD;
	
  WB_NORMALVTXST_TR1 *nvs;
  WB_NORMALVTXST_TR2 *ex_nvs;

  const fx16 *nrm_tbl = (const fx16*)WB_NORMAL_TBL;
  const fx32 *d_tbl = (const fx32*)WB_PLANE_D_TBL;
/**
  if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_R){
    OS_Printf("GetAttr idx%d = y=%08x\n",idx,posInBlock->y);
  }
*/
	VEC_Set( &pos, posInBlock->x + map_width/2, posInBlock->y, posInBlock->z + map_width/2 );
	//グリッド内情報取得
	grid_count = (map_width>>FX32_SHIFT) /  FIELD_CONST_GRID_SIZE;
	grid_w = FIELD_CONST_GRID_FX32_SIZE;	//マップ幅をグリッド数で分割
	grid_idx = ( pos.z / grid_w ) * grid_count + ( pos.x / grid_w );
	grid_x = pos.x % grid_w;
	grid_z = pos.z % grid_w;

	//情報取得(軸の取り方が違うので法線ベクトルはZ反転)
	nvs = (WB_NORMALVTXST_TR1*)(attrAdrs + sizeof(NormalVtxFormat) + grid_idx * sizeof(WB_NORMALVTXST_TR1));

  {
    u8 flag;
    int nrm_idx, d_idx;
    flag = WB_NORMALVTXST_GetTryangleFlag(nvs);
    //クワッド型かトライアングル型かで分岐
    if ( flag == WB_NORMALVTX_TRYANGLE_ONE )   //三角形１つ＝クワッド型
    {
      nrm_idx = nvs->vecN1_x;
      d_idx = nvs->vecN1_D;
      //法線ベクトルテーブルから法線を取得
      VEC_Fx16Set( &attrInfo->mapAttr[idx].vecN, nrm_tbl[nrm_idx], nrm_tbl[nrm_idx+1], -nrm_tbl[nrm_idx+2] );
      //ＤテーブルからＤ値を取得
      valD = d_tbl[d_idx];
    }
    else if( flag == WB_NORMALVTX_TRYANGLE_TWO_INDEX )     //三角形2つ
    {
      int ex_idx;
      int ofs;
      ex_idx = WB_NORMALVTXST_GetTR2Index( nvs );
      {
        NormalVtxFormat *vfmt;
        vfmt = (NormalVtxFormat*)(attrAdrs);
        ofs = vfmt->width * vfmt->height * sizeof(WB_NORMALVTXST_TR1);
      }
      NOZOMU_Printf("idx %d tryangle two ex_idx=%d\n", grid_idx, ex_idx);

      //データの先頭アドレスを計算
      ex_nvs = (WB_NORMALVTXST_TR2*)(attrAdrs + sizeof(NormalVtxFormat) + ofs + ex_idx* sizeof(WB_NORMALVTXST_TR2));
      
      GF_ASSERT_MSG(ex_nvs->tryangleFlag == WB_NORMALVTX_TRYANGLE_TWO, \
          " ex tryangleFlag error %d grididx = %d\n",ex_nvs->tryangleFlag, grid_idx);

      if( nvs->tryangleType == 0 )
      {
        //0-2-1,3-1-2のパターン
        if( grid_x + grid_z < grid_w )
        {
			    nrm_idx = ex_nvs->vecN1_x;
          d_idx = ex_nvs->vecN1_D;
		    }
        else
        {
			    nrm_idx = ex_nvs->vecN2_x;
          d_idx = ex_nvs->vecN2_D;
		    }
      }
      else
      {
        //2-3-0,1-0-3のパターン
        if( grid_x > grid_z )
        {
          nrm_idx = ex_nvs->vecN1_x;
          d_idx = ex_nvs->vecN1_D;
        }
        else
        {
          nrm_idx = ex_nvs->vecN2_x;
          d_idx = ex_nvs->vecN2_D;
        }
      }
      VEC_Fx16Set( &attrInfo->mapAttr[idx].vecN, nrm_tbl[nrm_idx], nrm_tbl[nrm_idx+1], -nrm_tbl[nrm_idx+2] );
			valD = d_tbl[d_idx];
    }
    else
    {
      GF_ASSERT_MSG(0,"tryangleFlag error %d", flag);
      VEC_Fx16Set( &attrInfo->mapAttr[idx].vecN, 0, FX16_ONE, 0 );
      valD = 0;
    }
  }
    
	VEC_Set( &vecN, 
			attrInfo->mapAttr[idx].vecN.x, attrInfo->mapAttr[idx].vecN.y, attrInfo->mapAttr[idx].vecN.z );
	by = -( FX_Mul(vecN.x, posInBlock->x) + FX_Mul(vecN.z, posInBlock->z) + valD );
	attrInfo->mapAttr[idx].attr = nvs->attr;

	attrInfo->mapAttr[idx].height = FX_Div( by, vecN.y ) + map_height;
}

