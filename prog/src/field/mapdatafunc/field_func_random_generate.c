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

static void FieldFuncRandom_CheckMapPos( GFL_G3D_MAP* g3Dmap , VecFx32* p_pos );
static const u8 FieldFuncRandom_GetBilduingHeight( const u8 idxTop , const u8 idxLeft );
static const u32 FieldFuncRandom_GetBilduingResId( u32 cyty_type, const u8 height );

BOOL FieldLoadMapData_RandomGenerate( GFL_G3D_MAP* g3Dmap, void * exWork )
{
	GFL_G3D_MAP_LOAD_STATUS* ldst;
	FLD_G3D_MAP_EXWORK* p_exwork;	// GFL_G3D_MAP拡張ワーク

	// 拡張ワーク取得
	p_exwork = exWork;

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

			//ヘッダー設定
			GFL_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );
			fileHeader = (WBGridMapPackHeaderSt*)mem;
			//モデルリソース設定
			GFL_G3D_MAP_CreateResourceMdl(g3Dmap, (void*)((u32)mem + fileHeader->nsbmdOffset));
			//テクスチャリソース設定
			//>>GFL_G3D_MAP_CreateResourceTex(g3Dmap, (void*)((u32)mem + fileHeader->nsbtxOffset)); 
			//配置オブジェクト設定
			if( fileHeader->positionOffset != fileHeader->endPos )
			{
        FIELD_WFBC* p_wfbc = FLD_G3D_MAP_EXWORK_GetWFBCWork( p_exwork );
        u16 level = FIELD_WFBC_GetPeopleNum( p_wfbc );
        u16 cyty_type = FIELD_WFBC_GetType(  p_wfbc );

				LayoutFormat* layout = (LayoutFormat*)((u32)mem + fileHeader->positionOffset);
				PositionSt* objStatus = (PositionSt*)&layout->posData;
				GFL_G3D_MAP_GLOBALOBJ_ST status;
				int i, count = layout->count;
        FIELD_BMODEL_MAN * bm = FLD_G3D_MAP_EXWORK_GetBModelMan(p_exwork);
        const u8 mapIndex = FLD_G3D_MAP_EXWORK_GetMapIndex(p_exwork);
        VecFx32 pos;
        u8 x,z;
        fx32 top,left;
        u8  idxTop,idxLeft;

        FieldFuncRandom_CheckMapPos( g3Dmap , &pos );

        //top = pos.z - (256<<FX32_SHIFT) - (144<<FX32_SHIFT);  // ハーフブロックと配置モデルのローカル座標分ずらす
        //left = pos.x - (256<<FX32_SHIFT) - (144<<FX32_SHIFT);
        top = -(144<<FX32_SHIFT);  // ハーフブロックと配置モデルのローカル座標分ずらす
        left = -(144<<FX32_SHIFT);
        
        switch(mapIndex)
        {
        case RMM_LEFT_TOP:
          idxTop = 0;
          idxLeft = 0;
          break;

        case RMM_RIGHT_TOP:
          idxTop = 0;
          idxLeft = 3;
          break;

        case RMM_LEFT_BOTTOM:
          idxTop = 3;
          idxLeft = 0;
          break;

        case RMM_RIGHT_BOTTOM:
          idxTop = 3;
          idxLeft = 3;
          break;
        }
        i=0;
				for( x=0; x<3; x++ )
				{
  				for( z=0; z<3; z++ )
  				{
            u8 height_base = FieldFuncRandom_GetBilduingHeight(idxTop+z,idxLeft+x);
            s8 height = height_base + level - 10;
            if( height_base > 5 )
            {
              if( height > 10 )
              {
                height = 10;
              }
            }
            else if( height_base > 0 )
            {
              height = height_base;
            }
            
            if( height > 0 )
            {
              const u32 resId = FieldFuncRandom_GetBilduingResId( cyty_type, height );
              PositionSt objStatus;
              objStatus.resourceID = resId;
              objStatus.rotate = 0;
              objStatus.billboard = 0;
              objStatus.xpos = left+FX32_CONST( x*172.0f );
              objStatus.ypos = 0;
              objStatus.zpos = - (top+FX32_CONST( z*172.0f)); // ResistMapObject内で-反転されるので
              //OS_TPrintf( "x=%d z=%d  pos x[%d] z[%d] \n", x, z, FX_Whole(objStatus.xpos), FX_Whole(-objStatus.zpos) );
              FIELD_BMODEL_MAN_ResistMapObject( bm, g3Dmap, &objStatus, i );
  					  i++;
  					}
          }
        }
			}

      // 地面を配置
      {
#if 0
        static const u8 GroundMapData[ 4 ][ 16 ] = 
        {
          // マップインデックス00
          {
            0xff,0xff,0xff,0xff,
            0xff,0x00,0x00,0x00,
            0xff,0x00,0x00,0x00,
            0xff,0x00,0x00,0x00,
          },
          // マップインデックス01
          {
            0xff,0xff,0xff,0xff,
            0x00,0x00,0xff,0xff,
            0x00,0x00,0x00,0xff,
            0x00,0x00,0x00,0xff,
          },
          // マップインデックス02
          {
            0xff,0x00,0x00,0x00,
            0xff,0x00,0xff,0x00,
            0xff,0xff,0x00,0x00,
            0xff,0xff,0xff,0xff,
          },
          // マップインデックス03
          {
            0x00,0xff,0x00,0xff,
            0x00,0x00,0x00,0xff,
            0x00,0x00,0x00,0xff,
            0xff,0xff,0xff,0xff,
          },
        };
#else
        static const u8 GroundMapData[ 4 ][ 16 ] = 
        {
          // マップインデックス00
          {
            0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,
            0xff,0x00,0xff,0xff,
            0xff,0xff,0xff,0xff,
          },
          // マップインデックス01
          {
            0xff,0x02,0xff,0xff,
            0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,
          },
          // マップインデックス02
          {
            0xff,0xff,0xff,0xff,    
            0xff,0xff,0x01,0xff,
            0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,
          },
          // マップインデックス03
          {
            0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,
            0xff,0xff,0xff,0xff,
          },
        };
#endif
        int i, j;
        int index;
        const u8 mapIndex = FLD_G3D_MAP_EXWORK_GetMapIndex(p_exwork);
        FIELD_BMODEL_MAN * bm = FLD_G3D_MAP_EXWORK_GetBModelMan(p_exwork);
        
        //マップインデックスの地面を配置 
        for( i=0; i<4; i++ )
        {
          for( j=0; j<4; j++ )
          {
            index = (i * 4) + j;
            
            if( GroundMapData[ mapIndex ][ index ] != 0xff )
            {
              u32 resId = NARC_output_buildmodel_outdoor_bc_block_00_nsbmd + GroundMapData[ mapIndex ][ index ];
              PositionSt objStatus;
              objStatus.resourceID = resId + (3*GFUser_GetPublicRand(3));
              objStatus.rotate = 0;
              objStatus.billboard = 0;
              objStatus.xpos = (j*(16*8)) << FX32_SHIFT;
              objStatus.ypos = 0;
              objStatus.zpos = -(i*(16*8)) << FX32_SHIFT; // ResistMapObject内で-反転されるので
              TOMOYA_Printf( "x=%d z=%d  pos x[%d] z[%d] \n", FX_Whole(objStatus.xpos), FX_Whole(-objStatus.zpos), 0, 0 );
              FIELD_BMODEL_MAN_ResistMapObject( bm, g3Dmap, &objStatus, 10+index );
  					}
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


//MAPの位置をチェック
static void FieldFuncRandom_CheckMapPos( GFL_G3D_MAP* g3Dmap , VecFx32* p_pos )
{
  GFL_G3D_MAP_GetTrans( g3Dmap , p_pos );
  OS_Printf("[%f:%f:%f]\n",FX_FX32_TO_F32(p_pos->x),FX_FX32_TO_F32(p_pos->y),FX_FX32_TO_F32(p_pos->z));
}

static const u8 FieldFuncRandom_GetBilduingHeight( const u8 idxTop , const u8 idxLeft )
{
#if 0
  static const u8 idxArr[6][6] =
  {
    {1,3,6,5,4,2},
    {2,5,7,8,6,3},
    {4,7,10,9,7,5},
    {3,6,8,10,6,4},
    {2,4,5,6,5,3},
    {1,2,4,3,2,1},
  };
#else
  static const u8 idxArr[6][6] =
  {
    {0,0,0,0,0,0},
    {0,8,0,0,8,0},
    {0,0,10,7,0,0},
    {0,0,7,10,0,0},
    {0,8,0,0,8,0},
    {0,0,0,0,0,0},
  };
#endif

  GF_ASSERT( idxTop < 6 );
  GF_ASSERT( idxLeft < 6 );
  
  return idxArr[idxTop][idxLeft];
}

static const u32 FieldFuncRandom_GetBilduingResId( u32 cyty_type, const u8 height )
{
  if( cyty_type == FIELD_WFBC_CORE_TYPE_BLACK_CITY )
  {
    return NARC_output_buildmodel_outdoor_bc_build_01_nsbmd + height-1;
  }
  else
  {
    return NARC_output_buildmodel_outdoor_wf_tree_01_nsbmd + height-1;
  }
  
}



