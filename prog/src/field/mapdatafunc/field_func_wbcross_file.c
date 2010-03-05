//============================================================================================
/**
 * @file    field_func_wbcorss_file.c
 * @brief   ノーマルグリッド立体交差シグネチャー
 * @author	Miyuki Iwasawa
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "field/field_const.h"

#include "field_func_wbcross_file.h"

#include "../field_buildmodel.h"
#include "../field_g3dmap_exwork.h"	// FLD_G3D_MAP拡張ワーク
#include "field/map_attr.h"	// FLD_G3D_MAP拡張ワーク

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

enum{
 CROSS_LAYER_DEF,
 CROSS_LAYER_EX,
 CROSS_LAYER_NUM,
};


///プロトタイプ
static void fgr_WBCrossFileCore( FLD_G3D_MAP_ATTRINFO* attrInfo, const u8 idx, const u32 attrAdrs,
    const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height );

BOOL FieldLoadMapData_WBCrossFile( FLD_G3D_MAP* g3Dmap, void * exWork )
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
			WBGridCrossMapPackHeaderSt*	fileHeader;
			//ヘッダー設定
			FLD_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );
			fileHeader = (WBGridCrossMapPackHeaderSt*)mem;
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

    // 完了
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
void FieldGetAttr_WBCrossFile( FLD_G3D_MAP_ATTRINFO* attrInfo, const void* mapdata, 
					const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
  FLD_G3D_MAP_ATTRINFO attr;

	WBGridCrossMapPackHeaderSt* fileHeader = (WBGridCrossMapPackHeaderSt*)mapdata;

  MI_CpuClear8(&attr,sizeof(FLD_G3D_MAP_ATTRINFO));

  //デフォルト階層ロード
  fgr_WBCrossFileCore( &attr, CROSS_LAYER_DEF, (u32)mapdata + fileHeader->vertexOffset,
    posInBlock, map_width, map_height );
  
  //立体交差階層ロード
  fgr_WBCrossFileCore( &attr, CROSS_LAYER_EX, (u32)mapdata + fileHeader->exAttrOffset,
    posInBlock, map_width, map_height );
#if 0
  {
    int i;
    u8 target = CROSS_LAYER_DEF;
    fx32 diff[CROSS_LAYER_NUM] = {0,0};

    //Exが進入可能の時
    if(!MAPATTR_GetHitchFlag(attr.mapAttr[CROSS_LAYER_EX].attr)){
      //Defが進入不可ならExを取る
      if(MAPATTR_GetHitchFlag(attr.mapAttr[CROSS_LAYER_DEF].attr)){
        target = CROSS_LAYER_EX;
      }else{
        //両方進入可能なら、高さの差分の絶対値を見て近い方をターゲットとする
        for(i = 0;i < CROSS_LAYER_NUM;i++){
          diff[i] = posInBlock->y - attr.mapAttr[i].height;
          if(diff[i] < 0){
            diff[i] *= -1;
          }
        }
        if(diff[CROSS_LAYER_EX] < diff[CROSS_LAYER_DEF]){
          target = CROSS_LAYER_EX;
        }
      }
    }
    attrInfo->mapAttr[0] = attr.mapAttr[target];
  }
#else
  *attrInfo = attr;
#endif
  attrInfo->mapAttrCount = CROSS_LAYER_NUM;
}

/**
  @brief  アトリビュートファイルの解釈
 */
static void fgr_WBCrossFileCore( FLD_G3D_MAP_ATTRINFO* attrInfo, const u8 idx, const u32 attrAdrs,
    const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
  MAPDATA_ATR_GetAttrFunc( attrInfo, idx, attrAdrs, posInBlock, map_width, map_height );
}

