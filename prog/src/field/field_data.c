//============================================================================================
/**
 */
//============================================================================================
#include <gflib.h>
#include "./fieldmap.h"
#include "arc_def.h"
#include "field_data.h"

#include "./fieldmap_resist.h"
#include "field/zonedata.h"
#include "field/areadata.h"
#include "map_matrix.h"

#include "fieldmap/area_id.h"

#include "fieldmap_ctrl_grid.h"
#include "fieldmap_ctrl_nogrid.h"
#include "fieldmap_ctrl_c3.h"

typedef struct {
	//横ブロック数, 縦ブロック数, ブロック１辺の幅, グラフィックアーカイブＩＤ, 実マップデータ
	FLDMAPPER_RESISTDATA	mapperData;
	const DEPEND_FUNCTIONS * dep_funcs;
	BOOL isMatrixMapFlag;
}SCENE_DATA;


const SCENE_DATA	resistMapTbl[];
const unsigned int resistMapTblCount;


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static u16 MapID2ResistID(u16 mapid)
{
	u16 resid;
	GF_ASSERT(mapid < ZONEDATA_GetZoneIDMax());
	resid = ZONEDATA_GetMapRscID(mapid);
	GF_ASSERT(resid < resistMapTblCount);
	return resid;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELDDATA_SetMapperData(u16 mapid, const AREADATA * areadata,
    FLDMAPPER_RESISTDATA * map_res, MAP_MATRIX *matrix_buf )
{
	u16 resid = MapID2ResistID(mapid);
	*map_res = resistMapTbl[resid].mapperData;

	if (resistMapTbl[resid].isMatrixMapFlag){
		u16 matID;
		matID = ZONEDATA_GetMatrixID( mapid );
		MAP_MATRIX_Init( matrix_buf, matID, mapid );
		map_res->sizex = MAP_MATRIX_GetMapBlockSizeWidth( matrix_buf );
		map_res->sizez = MAP_MATRIX_GetMapBlockSizeHeight( matrix_buf );
		map_res->totalSize = MAP_MATRIX_GetMapBlockTotalSize( matrix_buf );
		map_res->blocks = (const FLDMAPPER_MAPDATA*)MAP_MATRIX_GetMapResIDTable( matrix_buf );
	}
	
	//標準フィールド以外のときだけテクスチャをグローバルにしてみる
	if (map_res->g3DmapFileType != FLDMAPPER_FILETYPE_PKGS) {
    FLDMAPPER_RESIST_TEX	gTexBuffer;

		gTexBuffer.arcID = ARCID_AREA_MAPTEX;
		gTexBuffer.datID = AREADATA_GetTextureSetID(areadata);
		map_res->gtexType = FLDMAPPER_TEXTYPE_USE;
		map_res->gtexData = gTexBuffer;

    map_res->ground_anime.ita_datID = AREADATA_GetGroundITAID(areadata);
    map_res->ground_anime.itp_anm_datID = AREADATA_GetGroundITPID(areadata);
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const DEPEND_FUNCTIONS * FIELDDATA_GetFieldFunctions(u16 mapid)
{
	u16 resid = MapID2ResistID(mapid);
	return resistMapTbl[resid].dep_funcs;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
#include "test_graphic/test3dp.naix"
static const FLDMAPPER_MAPDATA sampleMap[] = {
	{ NARC_test3dp_map_a1_3dppack },
	{ NARC_test3dp_map_a2_3dppack },
	{ NARC_test3dp_map_a3_3dppack },
	{ NARC_test3dp_map_a4_3dppack },
	{ NARC_test3dp_map_b1_3dppack },
	{ NARC_test3dp_map_b2_3dppack },
	{ NARC_test3dp_map_b3_3dppack },
	{ NARC_test3dp_map_b4_3dppack },
	{ NARC_test3dp_map_c1_3dppack },
	{ NARC_test3dp_map_c2_3dppack },
	{ NARC_test3dp_map_c3_3dppack },
	{ NARC_test3dp_map_c4_3dppack },
	{ NARC_test3dp_map_d1_3dppack },
	{ NARC_test3dp_map_d2_3dppack },
	{ NARC_test3dp_map_d3_3dppack },
	{ NARC_test3dp_map_d4_3dppack },
};

//------------------------------------------------------------------
//------------------------------------------------------------------
#define	NARC_nogrid_mapblock_h01a_dummy	NARC_nogrid_mapblock_h01a_00_01_ngpack
#include "fieldmap/nogrid_mapblock.naix"
static const FLDMAPPER_MAPDATA newbridgemap[] = {
	{ NARC_nogrid_mapblock_h01a_00_00_ngpack }, //{ NARC_nogrid_mapblock_h01a_dummy },
	{ NARC_nogrid_mapblock_h01a_01_00_ngpack },

	{ NARC_nogrid_mapblock_h01a_00_01_ngpack },
	{ NARC_nogrid_mapblock_h01a_01_01_ngpack },

	{ NARC_nogrid_mapblock_h01a_00_02_ngpack },
	{ NARC_nogrid_mapblock_h01a_01_02_ngpack },
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static const FLDMAPPER_MAPDATA test_c3map[] = {
	{ NARC_nogrid_mapblock_C3_00_01_ngpack },
	{ NARC_nogrid_mapblock_C3_00_02_ngpack },
	{ NARC_nogrid_mapblock_C3_01_01_ngpack },
	{ NARC_nogrid_mapblock_C3_01_02_ngpack },
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static const FLDMAPPER_MAPDATA loopbridgemap[] = {
	{ NARC_nogrid_mapblock_m_test2_01_01_ngpack },
	{ NARC_nogrid_mapblock_m_test2_02_01_ngpack },
	{ NARC_nogrid_mapblock_m_test2_03_01_ngpack },
	{ NARC_nogrid_mapblock_m_test2_04_01_ngpack },
	{ NARC_nogrid_mapblock_m_test2_01_02_ngpack },
	{ NARC_nogrid_mapblock_m_test2_02_02_ngpack },
	{ NARC_nogrid_mapblock_m_test2_03_02_ngpack },
	{ NARC_nogrid_mapblock_m_test2_04_02_ngpack },
	{ NARC_nogrid_mapblock_m_test2_01_03_ngpack },
	{ NARC_nogrid_mapblock_m_test2_02_03_ngpack },
	{ NARC_nogrid_mapblock_m_test2_03_03_ngpack },
	{ NARC_nogrid_mapblock_m_test2_04_03_ngpack },
	{ NARC_nogrid_mapblock_m_test2_01_04_ngpack },
	{ NARC_nogrid_mapblock_m_test2_02_04_ngpack },
	{ NARC_nogrid_mapblock_m_test2_03_04_ngpack },
	{ NARC_nogrid_mapblock_m_test2_04_04_ngpack },
};

static const FLDMAPPER_MAPDATA samplebridge[] = {
	{ NARC_nogrid_mapblock_m_test_01_01_ngpack },
	{ NARC_nogrid_mapblock_m_test_02_01_ngpack },
	{ NARC_nogrid_mapblock_m_test_01_02_ngpack },
	{ NARC_nogrid_mapblock_m_test_02_02_ngpack },
	{ NARC_nogrid_mapblock_m_test_01_03_ngpack },
	{ NARC_nogrid_mapblock_m_test_02_03_ngpack },
	{ NARC_nogrid_mapblock_m_test_01_04_ngpack },
	{ NARC_nogrid_mapblock_m_test_02_04_ngpack },
	{ NARC_nogrid_mapblock_m_test_01_05_ngpack },
	{ NARC_nogrid_mapblock_m_test_02_05_ngpack },
	{ NARC_nogrid_mapblock_m_test_01_06_ngpack },
	{ NARC_nogrid_mapblock_m_test_02_06_ngpack },
};

//------------------------------------------------------------------
//------------------------------------------------------------------
//金銀形式バイナリデータサンプル
#define DATID_GSMAP_GOBJ (2)
static const FLDMAPPER_RESISTDATA_OBJBIN	gobjData_Bin = {
	ARCID_GSAREAOBJ,
	ARCID_GSOBJANMTBL,
	DATID_GSMAP_GOBJ, 

	ARCID_GSOBJ,
	ARCID_GSOBJTEX,
	ARCID_GSOBJANM,
};

#if 0
//グローバルテクスチャサンプル（金銀マップテクスチャ）
#define DATID_GSMAP_GTEX (3)
static const FLDMAPPER_RESIST_TEX	gtexData = {
	ARCID_GSTEX, DATID_GSMAP_GTEX, 
};
#endif

static const FLDMAPPER_MAPDATA GSMap[] = {
	{ 21 }, { 22 },
	{ 23 }, { 24 },
	{ 25 }, { 26 },
	{ FLDMAPPER_MAPDATA_NULL }, { 27 },
	{ FLDMAPPER_MAPDATA_NULL }, { 28 },
	{ FLDMAPPER_MAPDATA_NULL }, { 29 },
};


//------------------------------------------------------------------
//------------------------------------------------------------------
#define MAP_XZ_SIZE (512 * FX32_ONE)	//16 unit * 32 grid
#define MAP_HEIGHT (128 * FX32_ONE)

const SCENE_DATA resistMapTbl[] = {
  //RSC_GRID_DEFAULT = 0,
	{	//実験マップ グリッド移動
		{
			FLDMAPPER_FILETYPE_NORMAL,
			MAP_XZ_SIZE, 1024*FX32_ONE, 
      3,3,
      FLDMAPPER_MODE_SCROLL_XZ, 
			ARCID_FLDMAP_LANDDATA,

			1,  1, 1,		//dummy map matrix data
			NULL, 

			FLDMAPPER_TEXTYPE_NONE,	{ 0, 0 },
			FLDMAPPER_RESIST_OBJTYPE_BMODEL,	NULL,
			{0,2},	// 地面アニメーション

			FLD_MAPPER_BLOCK_MEMSIZE,	// 1ブロックメモリサイズ
		},
		&FieldMapCtrl_GridFunctions,
		TRUE,
	},
  //RSC_NOGRID_BRIDGE = 1,
	{	//実験マップ　橋
		{
			FLDMAPPER_FILETYPE_NORMAL,
			2048*FX32_ONE, 2048*FX32_ONE, 
      2,3,
      FLDMAPPER_MODE_SCROLL_NONE, 
			ARCID_FLDMAP_LAND_EX,

			2,	3, NELEMS(newbridgemap),
			newbridgemap,
		//	4,  4, NELEMS(loopbridgemap),
		//	loopbridgemap, 

			FLDMAPPER_TEXTYPE_NONE,	{ 0, 0 },
			FLDMAPPER_RESIST_OBJTYPE_BMODEL,	NULL,
			{FLDMAPPER_MAPDATA_NULL,FLDMAPPER_MAPDATA_NULL},	// 地面アニメーション

			FLD_MAPPER_BLOCK_MEMSIZE,	// 1ブロックメモリサイズ
		}, 
		&FieldMapCtrl_NoGridFunctions,
		FALSE,
	},
  //RSC_NOGRID_C3 = 2,
	{	//実験マップ　C3
		{	
			FLDMAPPER_FILETYPE_NORMAL,
			MAP_XZ_SIZE, 1024*FX32_ONE, 
      2,2,
      FLDMAPPER_MODE_SCROLL_NONE, 
			ARCID_FLDMAP_LAND_EX,

			2,  2, NELEMS(test_c3map),
			test_c3map, 

			FLDMAPPER_TEXTYPE_NONE,	{ 0, 0 },
			FLDMAPPER_RESIST_OBJTYPE_BMODEL,	NULL,
			{FLDMAPPER_MAPDATA_NULL,FLDMAPPER_MAPDATA_NULL},	// 地面アニメーション

			FLD_MAPPER_BLOCK_MEMSIZE,	// 1ブロックメモリサイズ
		}, 
		&FieldMapCtrl_C3Functions,
		FALSE,
	},
  //RSC_GRID_PALACE = 3,
	{	//実験マップ グリッド移動   パレス
		{
			FLDMAPPER_FILETYPE_NORMAL,
			MAP_XZ_SIZE, 1024*FX32_ONE, 
      3,1,
      FLDMAPPER_MODE_SCROLL_NONE, 
			ARCID_FLDMAP_LANDDATA,

			1,  1, 1,		//dummy map matrix data
			NULL, 

			FLDMAPPER_TEXTYPE_NONE,	{ 0, 0 },
			FLDMAPPER_RESIST_OBJTYPE_BMODEL,	NULL,
			{0,2},	// 地面アニメーション

			FLD_MAPPER_BLOCK_MEMSIZE,	// 1ブロックメモリサイズ
		},
		&FieldMapCtrl_GridFunctions,
		TRUE,
	},
  //RSC_GRID_RANDOMMAP = 4,
	{	//実験マップ グリッド移動   ランダム生成マップ
		{
			FLDMAPPER_FILETYPE_RANDOM,
			MAP_XZ_SIZE, 1024*FX32_ONE, 
      3,3,
      FLDMAPPER_MODE_SCROLL_XZ, 
			ARCID_FLDMAP_LANDDATA,

			1,  1, 1,		//dummy map matrix data
			NULL, 

			FLDMAPPER_TEXTYPE_NONE,	{ 0, 0 },
			FLDMAPPER_RESIST_OBJTYPE_BMODEL,	NULL,
			{0,2},	// 地面アニメーション

			FLD_MAPPER_BLOCK_MEMSIZE,	// 1ブロックメモリサイズ
		},
		&FieldMapCtrl_GridFunctions,
		TRUE,
	},
  //RSC_NOGRID_LEAGUE = 5,
	{	//実験マップ チャンピオンリーグ
		{
			FLDMAPPER_FILETYPE_NORMAL,
			1024*FX32_ONE, 1024*FX32_ONE, 
      1,1,
      FLDMAPPER_MODE_SCROLL_NONE, 
			ARCID_FLDMAP_LANDDATA,

			1,  1, 1,		//dummy map matrix data
			NULL, 

			FLDMAPPER_TEXTYPE_NONE,	{ 0, 0 },
			FLDMAPPER_RESIST_OBJTYPE_BMODEL,	NULL,
			{0,2},	// 地面アニメーション

			FLD_MAPPER_BLOCK_MEMSIZE*4,	// 1ブロックメモリサイズ
		},
		&FieldMapCtrl_GridFunctions,
		TRUE,
	},
  //RSC_GRID_UNION = 6,
	{	//実験マップ グリッド移動
		{
			FLDMAPPER_FILETYPE_NORMAL,
			MAP_XZ_SIZE, 1024*FX32_ONE, 
      1,1,
      FLDMAPPER_MODE_SCROLL_NONE, 
			ARCID_FLDMAP_LANDDATA,

			1,  1, 1,		//dummy map matrix data
			NULL, 

			FLDMAPPER_TEXTYPE_NONE,	{ 0, 0 },
			FLDMAPPER_RESIST_OBJTYPE_BMODEL,	NULL,
			{0,2},	// 地面アニメーション

			FLD_MAPPER_BLOCK_MEMSIZE,	// 1ブロックメモリサイズ
		},
		&FieldMapCtrl_GridFunctions,
		TRUE,
	},
};

const unsigned int resistMapTblCount = NELEMS(resistMapTbl);


