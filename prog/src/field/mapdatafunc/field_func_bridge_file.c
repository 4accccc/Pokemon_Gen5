//============================================================================================
/**
 * @file	field_func_bridge_file.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include <gflib.h>
#include "system\main.h"

#include "field_func_bridge_file.h"

#include "../field_g3d_map.h.h"

#include "../field_g3dmap_exwork.h"	// FLD_G3D_MAP拡張ワーク

#include "height.h"

/// 全部をパックしたファイルのヘッダー
typedef struct {
    u16 DataID;         ////< DP3PACK_HEADER
    u16 dummy1;
    u32 nsbmdOffset;    ///< ファイルの先頭からnsbmdの場所までのOFFSET
    //u32 nsbtxOffset;    ///< ファイルの先頭からnsbtxの場所までのOFFSET
    u32 bhcOffset;	    ///< ファイルの先頭からbhcの場所までのOFFSET
    u32 positionOffset; ///< ファイルの先頭からポジションの場所までのOFFSET
    u32 endPos;         ///< ファイルの先頭からポジションの最後までのOFFSET
} BridgePackHeaderSt;


void CheckHeightData(const void *mem, MHI_PTR outMHI);
void SetInvalidHeightData(MHI_PTR outMap3DInfo);
extern BOOL GetHeightForBlock(const fx32 inNowY, const fx32 inX, const fx32 inZ,
		MHI_CONST_PTR inMap3DInfo,fx32 *outY);
extern int GetDPFormatHeight(const fx32 inX, const fx32 inZ, MHI_CONST_PTR inMap3DInfo, FLD_G3D_MAP_ATTRINFO* attrInfo);
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
	TEX_TRANS,
};

BOOL FieldLoadMapData_BridgeFile( FLD_G3D_MAP* g3Dmap, void * exWork )
{
	FLD_G3D_MAP_LOAD_STATUS* ldst;
	FLD_G3D_MAP_EXWORK* p_exwork;	// FLD_G3D_MAP拡張ワーク

	FLD_G3D_MAP_GetLoadStatusPointer( g3Dmap, &ldst );

	// 拡張ワーク取得
	p_exwork = exWork;

	switch( ldst->seq ){

	case FILE_LOAD_START:
		FLD_G3D_MAP_ResetLoadStatus(g3Dmap);

		SetInvalidHeightData((MHI_PTR)ldst->mOffs);
		//メモリ先頭にはデータ取得用情報を配置するため、読み込みポインタをずらす
		ldst->mOffs += sizeof(MAP_HEIGHT_INFO);

		//モデルデータロード開始
		{
			u32		datID;
			FLD_G3D_MAP_GetLoadDatID( g3Dmap, &datID );
			FLD_G3D_MAP_StartFileLoad( g3Dmap, datID );
		}
		ldst->seq = FILE_LOAD;
		break;

	case FILE_LOAD:
		if( FLD_G3D_MAP_ContinueFileLoad(g3Dmap) == FALSE ){
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
			BridgePackHeaderSt*	fileHeader;
			//ヘッダー設定
			FLD_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );
			mem = ((u8*)mem + sizeof(MAP_HEIGHT_INFO));
			fileHeader = (BridgePackHeaderSt*)mem;
			//モデルリソース設定
			FLD_G3D_MAP_CreateResourceMdl(g3Dmap, (void*)((u32)mem + fileHeader->nsbmdOffset));
			//テクスチャリソース設定
			//>>FLD_G3D_MAP_CreateResourceTex(g3Dmap, (void*)((u32)mem + fileHeader->nsbtxOffset)); 
			//配置オブジェクト設定
			if( fileHeader->positionOffset != fileHeader->endPos ){
				LayoutFormat* layout = (LayoutFormat*)((u32)mem + fileHeader->positionOffset);
				PositionSt* objStatus = (PositionSt*)&layout->posData;
        FIELD_BMODEL_MAN * bm = FLD_G3D_MAP_EXWORK_GetBModelMan(p_exwork);
				FLD_G3D_MAP_GLOBALOBJ_ST status;
				int i, count = layout->count;

				for( i=0; i<count; i++ ){
					FIELD_BMODEL_MAN_ResistMapObject( bm, g3Dmap, &objStatus[i], i );

				}
			//===========
			} else {
				//FLD_G3D_MAP_MakeTestPos( g3Dmap );
			//===========
			}

      
#if 0
    OS_Printf("DataID=%08x\n",fileHeader->DataID);         ////< DP3PACK_HEADER
    OS_Printf("dummy1=%08x\n",fileHeader->dummy1);
    OS_Printf("nsbmdOffset=%08x\n",fileHeader->nsbmdOffset);    ///< ファイルの先頭からnsbmdの場所までのOFFSET
    OS_Printf("nsbtxOffset=%08x\n",fileHeader->nsbtxOffset);    ///< ファイルの先頭からnsbtxの場所までのOFFSET
    OS_Printf("bhcOffset=%08x\n",fileHeader->bhcOffset);	    ///< ファイルの先頭からbhcの場所までのOFFSET
    OS_Printf("vertexOffset=%08x\n",fileHeader->vertexOffset);   ///< ファイルの先頭から法線＋アトリビュートの場所までのOFFSET
    OS_Printf("positionOffset=%08x\n",fileHeader->positionOffset); ///< ファイルの先頭からポジションの場所までのOFFSET
    OS_Printf("endPos=%08x\n",fileHeader->endPos);         ///< ファイルの先頭からポジションの最後までのOFFSET
#endif
			CheckHeightData(
					(void*)((u32)mem + fileHeader->bhcOffset),
					(MHI_PTR)((u8*)mem - sizeof(MAP_HEIGHT_INFO))
					);
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

		ldst->seq = TEX_TRANS;
		break;

	case TEX_TRANS:
		//>>if( FLD_G3D_MAP_TransVram(g3Dmap) == FALSE )
		{
			ldst->seq = FLD_G3D_MAP_LOAD_IDLING;
			return FALSE;
		}
		break;
	}
	return TRUE;
}

#include "../field_easytp.h"
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
void FieldGetAttr_BridgeFile( FLD_G3D_MAP_ATTRINFO* attrInfo, const void* mapdata, 
					const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
  // 090731 橋、C3には現在アトリビュートはない
  attrInfo->mapAttrCount = 0;
#if 0
	BridgePackHeaderSt * fileHeader = (void*)((u8*)mapdata + sizeof(MAP_HEIGHT_INFO));
	MHI_PTR mhi = (MHI_PTR)mapdata;
	u32 pol_count;
	pol_count = GetDPFormatHeight(posInBlock->x, posInBlock->z, mhi, attrInfo);
	attrInfo->mapAttrCount = pol_count;
#endif
#if 0
	if (FieldEasyTP_TouchDirGet() == FLDEASYTP_TCHDIR_UP) {
		fx32 outY;
		GetHeightForBlock(posInBlock->y, posInBlock->x, posInBlock->z, mhi, &outY);
	}
#endif
}

//============================================================================================
/**
 * 以下はポケモンDPからコピペ
 */
//============================================================================================
static inline fx32 Min(fx32 n1,fx32 n2)
{
	if(n1 < n2){
		return n1;
	}else{
		return n2;
	}
}

static inline fx32 Max(fx32 n1,fx32 n2)
{
	if(n1 > n2){
		return n1;
	}else{
		return n2;
	}

}

static void DEBUG_PutVector(const VecFx32 * vec)
{
	OS_Printf("(%08x,%08x,%08x)",vec->x, vec->y, vec->z);
}
//三角形の内外判定ZX射影版
static inline BOOL BG3D_CheckTriangleIObyZX(const VecFx32 inT_Vtx,const VecFx32 inVtx1,const VecFx32 inVtx2,const VecFx32 inVtx3)
{
	//外積を求める（VEC_CrossProduct未使用）
	fx32 cross;
	fx32 cross1;
	fx32 cross2;
	fx32 cross3;
	fx32 cross_total;
	cross = FX_Mul(inVtx3.z,inVtx1.x-inVtx2.x) + FX_Mul(inVtx1.z,inVtx2.x-inVtx3.x) + FX_Mul(inVtx2.z,inVtx3.x-inVtx1.x);
	cross1 = FX_Mul(inVtx2.z,inT_Vtx.x-inVtx1.x) + FX_Mul(inT_Vtx.z,inVtx1.x-inVtx2.x) + FX_Mul(inVtx1.z,inVtx2.x-inT_Vtx.x);
	cross2 = FX_Mul(inVtx3.z,inT_Vtx.x-inVtx2.x) + FX_Mul(inT_Vtx.z,inVtx2.x-inVtx3.x) + FX_Mul(inVtx2.z,inVtx3.x-inT_Vtx.x);
	cross3 = FX_Mul(inVtx1.z,inT_Vtx.x-inVtx3.x) + FX_Mul(inT_Vtx.z,inVtx3.x-inVtx1.x) + FX_Mul(inVtx3.z,inVtx1.x-inT_Vtx.x);

	if (cross < 0){
		return FALSE;
	}
	if (cross1 < 0){
		return FALSE;
	}
	if (cross2 < 0){
		return FALSE;
	}
	if (cross3 < 0){
		return FALSE;
	}
	return TRUE;
}

extern VecFx32 MoveRevise(fx32 inMoveVal,VecFx32 inNewVec,VecFx32 inOldVec);
#include "height_check.c"

extern void SetupHeightData(const char  *path, MHI_PTR outMapHeightInfo, u8 * inHeightMem);
#include "height_load.c"

