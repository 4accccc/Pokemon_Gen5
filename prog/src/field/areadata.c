//============================================================================================
/**
 * @file	areadata.c
 * @date	2008.12.18
 * @author	tamada GAME FREAK inc.
 * @brief	エリアデータアクセス
 */
//============================================================================================
#include <gflib.h>

#include "field/areadata.h"

#include "system/file_util.h"
#include "arc/arc_def.h"
#include "arc/fieldmap/area_id.h"

#include "arc/fieldmap/area_map_ita.naix"
#include "arc/fieldmap/area_map_itp.naix"

//------------------------------------------------------------------
//------------------------------------------------------------------
struct AREADATA{
	u16 model_set_index;		//配置モデルセット指定
	u16 tex_set_index;			//テクスチャセット指定
	u8  anm_ita_id;	  		//地形ITAアニメセット指定
  u8  anm_itp_id;	  		//地形ITPアニメセット指定
	u8  inner_outer;			//金銀追加：配置モデル種類指定（屋内・屋外）
	u8 light_type;				//ライト指定
  u8 edgemark_type;      ///<エッジマーキング指定
  u8 bbd_color;        ///< ビルボードモデルの色指定
};

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  AREA_ID_OUT_MAX = AREA_ID_IN01 - 4,
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline u16 check_area_range(u16 area_id)
{
	GF_ASSERT(area_id < AREA_ID_MAX);
	if (area_id < AREA_ID_MAX) {
		return area_id;
	} else {
		return 0;
	}
}
#define	CHECK_AREA_RANGE(value)	{value = check_area_range(value);}

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 AREADATA_GetAreaIDMax(void)
{
	return AREA_ID_MAX;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void dumpAreaData(const AREADATA * areadata)
{
  OS_Printf("sizeof AREADATA=%d\n", sizeof(AREADATA) );
	OS_Printf("model_set_index=%04x ",areadata->model_set_index);		//配置モデルセット指定
	OS_Printf("tex_set_index=%04x ",areadata->tex_set_index);			//テクスチャセット指定
	OS_Printf("anm_ita_id=%02x ",areadata->anm_ita_id);	  		//地形ITAアニメセット指定
  OS_Printf("anm_itp_id=%02x \n",areadata->anm_itp_id);	  		//地形ITPアニメセット指定
	OS_Printf("inner_outer=%02x ",areadata->inner_outer);			//金銀追加：配置モデル種類指定（屋内・屋外）
	OS_Printf("light_type=%02x ",areadata->light_type);				//ライト指定
  OS_Printf("edgemark_type=%02x\n",areadata->edgemark_type);      ///<エッジマーキング指定
  OS_Printf("bbd_color=%02x\n",areadata->bbd_color);      ///<ビルボードモデル色指定
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief   エリアデータの取得
 * @param   heapID
 * @return  AREADATA
 */
//------------------------------------------------------------------
AREADATA * AREADATA_Create(u16 heapID, u16 area_id, u32 season_id)
{
  AREADATA * areadata;
  if (AREADATA_HasSeason(area_id))
  {
    area_id += season_id;
  }
  CHECK_AREA_RANGE(area_id);
  areadata = GFL_FILE_LoadDataAllocOfs(
      ARCID_AREADATA,
      heapID,
      sizeof(AREADATA) * area_id,
      sizeof(AREADATA) );
  dumpAreaData(areadata);
  return areadata;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void AREADATA_Delete(AREADATA * areadata)
{
  GFL_HEAP_FreeMemory( areadata );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 AREADATA_GetModelSetID(const AREADATA * areadata)
{
  return areadata->model_set_index;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 AREADATA_GetTextureSetID(const AREADATA * areadata)
{
  return areadata->tex_set_index;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u32 AREADATA_GetGroundITAID(const AREADATA * areadata)
{
  if (areadata->anm_ita_id == 0xff )
  {
    return AREADATA_NO_ANIME_DATA;
  }
  else
  {
    return areadata->anm_ita_id;
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u32 AREADATA_GetGroundITPID(const AREADATA * areadata)
{
  if (areadata->anm_itp_id == 0xff)
  {
    return AREADATA_NO_ANIME_DATA;
  }
  else
  {
    return areadata->anm_itp_id;
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u8 AREADATA_GetInnerOuterSwitch(const AREADATA * areadata)
{
  return areadata->inner_outer;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u8 AREADATA_GetLightType(const AREADATA * areadata)
{
  return areadata->light_type;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u8 AREADATA_GetEdgeMarkingType(const AREADATA * areadata)
{
  return areadata->edgemark_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビルボードモデルカラー
 */
//-----------------------------------------------------------------------------
u8 AREADATA_GetBBDColor(const AREADATA * areadata)
{
  return areadata->bbd_color;
}

//------------------------------------------------------------------
//四季のある/なし
//------------------------------------------------------------------
BOOL AREADATA_HasSeason(u16 area_id)
{
	if (AREA_ID_OUT01 <= area_id && area_id < AREA_ID_OUT_MAX) return TRUE;
	return FALSE;
}

//------------------------------------------------------------------
//四季のある場所で、変化が顕著な場所かそうでないか？
//------------------------------------------------------------------
BOOL AREADATA_HasSeasonDiff(u16 area_id)
{
  if( !AREADATA_HasSeason(area_id)){
    return FALSE;
  }
	if (AREA_ID_OUT09 >= area_id && area_id <= AREA_ID_OUT11) return TRUE;
	return FALSE;
}

