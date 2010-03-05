//============================================================================================
/**
 * @file	field_func_nogrid_file.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include <gflib.h>
#include "system\main.h"

#include "field_func_nogrid_file.h"


#include "../field_g3dmap_exwork.h"	// FLD_G3D_MAP拡張ワーク

#include "height.h"

#include "mapdata_attr.h"


/// 全部をパックしたファイルのヘッダー
typedef struct {
    u16 DataID;         ////< NOGRIDPACK_HEADER
    u16 dummy1;
    u32 nsbmdOffset;    ///< ファイルの先頭からnsbmdの場所までのOFFSET
    u32 positionOffset; ///< ファイルの先頭からポジションの場所までのOFFSET
    u32 endPos;         ///< ファイルの先頭からポジションの最後までのOFFSET
} NoGridPackHeaderSt;


//============================================================================================
/**
 *
 *
 *
 * @brief	データを処理する関数群
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

BOOL FieldLoadMapData_NoGridFile( FLD_G3D_MAP* g3Dmap, void * exWork )
{
	FLD_G3D_MAP_LOAD_STATUS* ldst;
	FLD_G3D_MAP_EXWORK* p_exwork;	// FLD_G3D_MAP拡張ワーク

	FLD_G3D_MAP_GetLoadStatusPointer( g3Dmap, &ldst );

	// 拡張ワーク取得
	p_exwork = exWork;

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
    // ロード完了待ち
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
			NoGridPackHeaderSt*	fileHeader;
			//ヘッダー設定
			FLD_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );
			fileHeader = (NoGridPackHeaderSt*)mem;

			//モデルリソース設定
			FLD_G3D_MAP_CreateResourceMdl(g3Dmap, (void*)((u32)mem + fileHeader->nsbmdOffset));
			//配置オブジェクト設定
			if( fileHeader->positionOffset != fileHeader->endPos ){
				LayoutFormat* layout = (LayoutFormat*)((u32)mem + fileHeader->positionOffset);
				PositionSt* objStatus = (PositionSt*)&layout->posData;
        FIELD_BMODEL_MAN * bm = FLD_G3D_MAP_EXWORK_GetBModelMan(p_exwork);
        FIELD_BMODEL_MAN_ResistAllMapObjects(bm, g3Dmap, objStatus, layout->count);
			}
    }

      
		FLD_G3D_MAP_MakeRenderObj( g3Dmap );



		// 地面アニメーションの設定
		if( FLD_G3D_MAP_EXWORK_IsGranm( p_exwork ) ){
			FIELD_GRANM_WORK* p_granm;

			p_granm = FLD_G3D_MAP_EXWORK_GetGranmWork( p_exwork );
			FIELD_GRANM_WORK_Bind( p_granm, 
					FLD_G3D_MAP_GetResourceMdl(g3Dmap), FLD_G3D_MAP_GetResourceTex(g3Dmap), 
					FLD_G3D_MAP_GetRenderObj(g3Dmap) );
		}

    //　完了
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
void FieldGetAttr_NoGridFile( FLD_G3D_MAP_ATTRINFO* attrInfo, const void* mapdata, 
					const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
  // アトリビュートも高さもない！
  attrInfo->mapAttrCount = 0;
}

