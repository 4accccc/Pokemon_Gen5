//============================================================================================
/**
 * @file	field_func_wbnormal_file.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "field/field_const.h"

#include "field_func_random_generate.h"

#include "../field_buildmodel.h"
#include "../field_g3dmap_exwork.h"	// GFL_G3D_MAP拡張ワーク

#include "../land_data_patch.h"	// マップ情報に追加パッチをあてる


#include "savedata/save_control.h"
#include "fieldmap/buildmodel_outdoor.naix"

//============================================================================================
/**
 *
 *
 *
 * @brief	ＧＦマップエディタより作成したデータを処理する関数郡
 *  //ホワイトフォレスト・ブラックシティ用ランダム生成
 *
 *
 */
//============================================================================================
//============================================================================================
/**
 *
 *
 *
 * @brief	マップデータ読み込み
 *				※シーケンス0番はIDLING 1番はスタートＩＤＸで固定
 *
 *
 */
//============================================================================================


enum {
	FILE_LOAD_START = GFL_G3D_MAP_LOAD_START,
	FILE_LOAD,
	RND_CREATE,
	TEX_TRANS,
};

typedef enum
{
  RMM_LEFT_TOP,
  RMM_RIGHT_TOP,
  RMM_LEFT_BOTTOM,
  RMM_RIGHT_BOTTOM,
}RANDOM_MAP_MAPPOS;


BOOL FieldLoadMapData_RandomGenerate( GFL_G3D_MAP* g3Dmap, void * exWork )
{
	GFL_G3D_MAP_LOAD_STATUS* ldst;
	FLD_G3D_MAP_EXWORK* p_exwork;	// GFL_G3D_MAP拡張ワーク
  HEAPID heapID;

	// 拡張ワーク取得
	p_exwork = exWork;

  // ヒープIDを取得　パッチ読み込み用 
  heapID = FLD_G3D_MAP_EXWORK_GetHeapID( p_exwork );

	GFL_G3D_MAP_GetLoadStatusPointer( g3Dmap, &ldst );

	switch( ldst->seq ){

	case FILE_LOAD_START:
		GFL_G3D_MAP_ResetLoadStatus(g3Dmap);

		//モデルデータロード開始
		{
			u32		datID;
			GFL_G3D_MAP_GetLoadDatID( g3Dmap, &datID );
			GFL_G3D_MAP_StartFileLoad( g3Dmap, datID );
		}
		ldst->seq = FILE_LOAD;
		break;

	case FILE_LOAD:
		if( GFL_G3D_MAP_ContinueFileLoad(g3Dmap) == FALSE ){
			ldst->mdlLoaded = TRUE;
			ldst->texLoaded = TRUE;
			ldst->attrLoaded = TRUE;

			ldst->seq = RND_CREATE;
		}
		break;

	case RND_CREATE:
		//レンダー作成
		{
			void*				mem;
			WBGridMapPackHeaderSt*	fileHeader;
      int buildmodel_count;

			//ヘッダー設定
			GFL_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );
			fileHeader = (WBGridMapPackHeaderSt*)mem;
			//モデルリソース設定
			GFL_G3D_MAP_CreateResourceMdl(g3Dmap, (void*)((u32)mem + fileHeader->nsbmdOffset));

			//配置オブジェクト設定
			if( fileHeader->positionOffset != fileHeader->endPos ){
				LayoutFormat* layout = (LayoutFormat*)((u32)mem + fileHeader->positionOffset);
				PositionSt* objStatus = (PositionSt*)&layout->posData;
				GFL_G3D_MAP_GLOBALOBJ_ST status;
				int i;
        FIELD_BMODEL_MAN * bm = FLD_G3D_MAP_EXWORK_GetBModelMan(p_exwork);

        buildmodel_count = layout->count;

				for( i=0; i<buildmodel_count; i++ ){
          FIELD_BMODEL_MAN_ResistMapObject(bm, g3Dmap, &objStatus[i], i);
				}
			}

      {
        int i, j;
        int block_x, block_z;
        FIELD_BMODEL_MAN* p_bm = FLD_G3D_MAP_EXWORK_GetBModelMan( p_exwork );
        NormalVtxFormat* p_attr = (NormalVtxFormat*)((u32)mem + fileHeader->vertexOffset);
        int map_index = FLD_G3D_MAP_EXWORK_GetMapIndex( p_exwork );
        FIELD_WFBC* p_wfbc = FLD_G3D_MAP_EXWORK_GetWFBCWork( p_exwork );
        HEAPID heapID = FLD_G3D_MAP_EXWORK_GetHeapID( p_exwork );

        TOMOYA_Printf( "\nmapindex %d setup start\n", map_index );
        
        block_x = (map_index % 2) * (FIELD_WFBC_BLOCK_SIZE_X/2);
        block_z = (map_index / 2) * (FIELD_WFBC_BLOCK_SIZE_Z/2);

        for( i=0; i<(FIELD_WFBC_BLOCK_SIZE_Z/2); i++ )
        {
          for( j=0; j<(FIELD_WFBC_BLOCK_SIZE_X/2); j++ )
          {
            buildmodel_count = FIELD_WFBC_SetUpBlock( p_wfbc, p_attr, p_bm, g3Dmap, buildmodel_count, block_x + j, block_z + i, heapID );
          }
        }
      }
      
		}
		//>>GFL_G3D_MAP_SetTransVramParam( g3Dmap );	//テクスチャ転送設定
		GFL_G3D_MAP_MakeRenderObj( g3Dmap );

		// 地面アニメーションの設定
		if( FLD_G3D_MAP_EXWORK_IsGranm( p_exwork ) ){
			FIELD_GRANM_WORK* p_granm;

			p_granm = FLD_G3D_MAP_EXWORK_GetGranmWork( p_exwork );
			FIELD_GRANM_WORK_Bind( p_granm, 
					GFL_G3D_MAP_GetResourceMdl(g3Dmap), GFL_G3D_MAP_GetResourceTex(g3Dmap), 
					GFL_G3D_MAP_GetRenderObj(g3Dmap) );
		}

		ldst->seq = TEX_TRANS;
		break;

	case TEX_TRANS:
		//>>if( GFL_G3D_MAP_TransVram(g3Dmap) == FALSE ){
		{
			ldst->seq = GFL_G3D_MAP_LOAD_IDLING;
			return FALSE;
		}
		break;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 *
 *
 * @brief	３Ｄマップ情報取得
 *
 *
 *
 */
//============================================================================================
void FieldGetAttr_RandomGenerate( GFL_G3D_MAP_ATTRINFO* attrInfo, const void* mapdata, 
					const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
	fx32			grid_w, grid_x, grid_z;
	u32				grid_idx;
	u32				grid_count;
	VecFx32			pos, vecN;
	fx32			by, valD;
	NormalVtxSt*	nvs;
	WBGridMapPackHeaderSt* fileHeader = (WBGridMapPackHeaderSt*)mapdata;
	u32				attrAdrs = (u32)mapdata + fileHeader->vertexOffset;

	VEC_Set( &pos, posInBlock->x + map_width/2, posInBlock->y, posInBlock->z + map_width/2 );
	//グリッド内情報取得
	grid_count = (map_width>>FX32_SHIFT) /  FIELD_CONST_GRID_SIZE;
	grid_w = FIELD_CONST_GRID_FX32_SIZE;	//マップ幅をグリッド数で分割
	grid_idx = ( pos.z / grid_w ) * grid_count + ( pos.x / grid_w );
	grid_x = pos.x % grid_w;
	grid_z = pos.z % grid_w;

	//情報取得(軸の取り方が違うので法線ベクトルはZ反転)
	nvs = (NormalVtxSt*)(attrAdrs + sizeof(NormalVtxFormat) + grid_idx * sizeof(NormalVtxSt));

	//グリッド内三角形の判定
	if( nvs->tryangleType == 0 ){
		//0-2-1,3-1-2のパターン
		if( grid_x + grid_z < grid_w ){
			VEC_Fx16Set( &attrInfo->mapAttr[0].vecN, nvs->vecN1_x, nvs->vecN1_y, -nvs->vecN1_z );
			valD = nvs->vecN1_D;
		} else {
			VEC_Fx16Set( &attrInfo->mapAttr[0].vecN, nvs->vecN2_x, nvs->vecN2_y, -nvs->vecN2_z );
			valD = nvs->vecN2_D;
		}
	} else {
		//2-3-0,1-0-3のパターン
		if( grid_x > grid_z ){
			VEC_Fx16Set( &attrInfo->mapAttr[0].vecN, nvs->vecN1_x, nvs->vecN1_y, -nvs->vecN1_z );
			valD = nvs->vecN1_D;
		} else {
			VEC_Fx16Set( &attrInfo->mapAttr[0].vecN, nvs->vecN2_x, nvs->vecN2_y, -nvs->vecN2_z );
			valD = nvs->vecN2_D;
		}
	}
	VEC_Set( &vecN, 
			attrInfo->mapAttr[0].vecN.x, attrInfo->mapAttr[0].vecN.y, attrInfo->mapAttr[0].vecN.z );
	by = -( FX_Mul(vecN.x, posInBlock->x) + FX_Mul(vecN.z, posInBlock->z) + valD );
	attrInfo->mapAttr[0].attr = nvs->attr;

	attrInfo->mapAttr[0].height = FX_Div( by, vecN.y ) + map_height;

	attrInfo->mapAttrCount = 1;
}


