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

#include "field_func_wbnormal_file.h"

#include "../field_buildmodel.h"
#include "../field_g3dmap_exwork.h"	// FLD_G3D_MAP拡張ワーク

#include "mapdata_attr.h"

//============================================================================================
/**
 *
 *
 *
 * @brief	ＧＦマップエディタより作成したデータを処理する関数郡
 *
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
	FILE_LOAD_START = FLD_G3D_MAP_LOAD_START,
	FILE_LOAD,
	RND_CREATE,
};

BOOL FieldLoadMapData_WBNormalFile( FLD_G3D_MAP* g3Dmap, void * exWork )
{
	FLD_G3D_MAP_LOAD_STATUS* ldst;
	FLD_G3D_MAP_EXWORK* p_exwork;	// FLD_G3D_MAP拡張ワーク


	// 拡張ワーク取得
	p_exwork = exWork;

	FLD_G3D_MAP_GetLoadStatusPointer( g3Dmap, &ldst );

	switch( ldst->seq ){

	case FILE_LOAD_START:
		FLD_G3D_MAP_ResetLoadStatus(g3Dmap);

		//モデルデータロード開始
		{
			u32		datID;
			FLD_G3D_MAP_GetLoadDatID( g3Dmap, &datID );
			FLD_G3D_MAP_StartFileLoad( g3Dmap, datID );
		}
		ldst->seq = FILE_LOAD;
		break;

	case FILE_LOAD:
    {
      BOOL rc;
      rc = FLD_G3D_MAP_ContinueFileLoad(g3Dmap);
		  if( rc ){
        break;
		  }
    }
    ldst->mdlLoaded = TRUE;
    ldst->texLoaded = TRUE;
    ldst->attrLoaded = TRUE;

    ldst->seq = RND_CREATE;
    //break through
    
	case RND_CREATE:
		//レンダー作成
		{
			void*				mem;
			WBGridMapPackHeaderSt*	fileHeader;

			//ヘッダー設定
			FLD_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );
			fileHeader = (WBGridMapPackHeaderSt*)mem;
			//モデルリソース設定
			FLD_G3D_MAP_CreateResourceMdl(g3Dmap, (void*)((u32)mem + fileHeader->nsbmdOffset));
			//テクスチャリソース設定
			//>>FLD_G3D_MAP_CreateResourceTex(g3Dmap, (void*)((u32)mem + fileHeader->nsbtxOffset)); 
			//配置オブジェクト設定
			if( fileHeader->positionOffset != fileHeader->endPos ){
				LayoutFormat* layout = (LayoutFormat*)((u32)mem + fileHeader->positionOffset);
				PositionSt* objStatus = (PositionSt*)&layout->posData;
        FIELD_BMODEL_MAN * bm = FLD_G3D_MAP_EXWORK_GetBModelMan(p_exwork);
        FIELD_BMODEL_MAN_ResistAllMapObjects(bm, g3Dmap, objStatus, layout->count);
			}
		}
		//>>FLD_G3D_MAP_SetTransVramParam( g3Dmap );	//テクスチャ転送設定
		FLD_G3D_MAP_MakeRenderObj( g3Dmap );

		// 地面アニメーションの設定
		if( FLD_G3D_MAP_EXWORK_IsGranm( p_exwork ) ){
			FIELD_GRANM_WORK* p_granm;

			p_granm = FLD_G3D_MAP_EXWORK_GetGranmWork( p_exwork );
			FIELD_GRANM_WORK_Bind( p_granm, 
					FLD_G3D_MAP_GetResourceMdl(g3Dmap), FLD_G3D_MAP_GetResourceTex(g3Dmap), 
					FLD_G3D_MAP_GetRenderObj(g3Dmap) );
		}

    ldst->seq = FLD_G3D_MAP_LOAD_IDLING;
    return FALSE;
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
void FieldGetAttr_WBNormalFile( FLD_G3D_MAP_ATTRINFO* attrInfo, const void* mapdata, 
					const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
	WBGridMapPackHeaderSt* fileHeader = (WBGridMapPackHeaderSt*)mapdata;
	u32				attrAdrs = (u32)mapdata + fileHeader->vertexOffset;

  MAPDATA_ATR_GetAttrFunc( attrInfo, 0, attrAdrs, posInBlock, map_width, map_height );
	attrInfo->mapAttrCount = 1;
}

