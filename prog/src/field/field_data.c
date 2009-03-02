//============================================================================================
/**
 */
//============================================================================================
#include <gflib.h>
#include "./fieldmap_local.h"
#include "arc_def.h"
#include "field_data.h"

#include "./fieldmap_resist.h"
#include "field/zonedata.h"
#include "field/areadata.h"
#include "map_matrix.h"

#include "fieldmap/area_id.h"

typedef struct {
	//横ブロック数, 縦ブロック数, ブロック１辺の幅, グラフィックアーカイブＩＤ, 実マップデータ
	FLDMAPPER_RESISTDATA	mapperData;
	const DEPEND_FUNCTIONS * dep_funcs;
	BOOL isMatrixMapFlag;
}SCENE_DATA;


const SCENE_DATA	resistMapTbl[];
const unsigned int resistMapTblCount;

static FLDMAPPER_RESIST_TEX	gTexBuffer;

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static int MapID2ResistID(u16 mapid)
{
	u16 resid;
	GF_ASSERT(mapid < ZONEDATA_GetZoneIDMax());
	resid = ZONEDATA_GetMapRscID(mapid);
	GF_ASSERT(resid < resistMapTblCount);
	return resid;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELDDATA_SetMapperData(
	u16 mapid, FLDMAPPER_RESISTDATA * map_res, void * matrix_buf)
{
	u16 area_id = ZONEDATA_GetAreaID(mapid);
	u16 resid = MapID2ResistID(mapid);
	*map_res = resistMapTbl[resid].mapperData;

	if (resistMapTbl[resid].isMatrixMapFlag){
		u8 *tbl;
		u32 matID;
		const MAP_MATRIX_HEADER *matH;
		
		matID = ZONEDATA_GetMatrixID( mapid );
		GFL_ARC_LoadData( matrix_buf, ARCID_FLDMAP_MAPMATRIX, matID );
		
		matH = matrix_buf;
		tbl = (u8*)matrix_buf + sizeof(MAP_MATRIX_HEADER);
		
		map_res->sizex = matH->size_h;
		map_res->sizez = matH->size_v;
		map_res->totalSize = matH->size_h * matH->size_v;
		map_res->blocks = (const FLDMAPPER_MAPDATA *)tbl;

	}
	//標準フィールド以外のときだけテクスチャをグローバルにしてみる
	if (map_res->g3DmapFileType != FLDMAPPER_FILETYPE_PKGS && area_id != AREA_ID_FIELD) {
		gTexBuffer.arcID = ARCID_AREA_MAPTEX;
		gTexBuffer.datID = AREADATA_GetTextureSetID(area_id);
		TAMADA_Printf("Load Area Texture %d\n", gTexBuffer.datID);
		map_res->gtexType = FLDMAPPER_RESIST_TEXTYPE_USE;
		map_res->gtexData = &gTexBuffer;
	}
	{
		TAMADA_Printf("ZONE_ID:%d AREA_ID:%d\n",mapid, area_id);
		TAMADA_Printf("ModelSet=%d, TexSet=%d, AnmSet=%d, ",
				AREADATA_GetModelSetID(area_id),
				AREADATA_GetTextureSetID(area_id),
				AREADATA_GetAnimeSetID(area_id));
		TAMADA_Printf("INOUT=%d, Light=%d\n",
				AREADATA_GetInnerOuterSwitch(area_id),
				AREADATA_GetLightType(area_id));
	}
#if 1
	{
		int x,z;
		for (z = 0; z < map_res->sizez; z++) {
			for (x = 0; x < map_res->sizex; x++) {
				TAMADA_Printf("%08x ",map_res->blocks[map_res->sizex * z + x]);
			}
			TAMADA_Printf("\n");
		}
	}
#endif
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const FLDMAPPER_RESISTDATA * FIELDDATA_GetMapperData(u16 mapid)
{
	u16 resid = MapID2ResistID(mapid);
	return &resistMapTbl[resid].mapperData;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const DEPEND_FUNCTIONS * FIELDDATA_GetFieldFunctions(u16 mapid)
{
	u16 resid = MapID2ResistID(mapid);
	return resistMapTbl[resid].dep_funcs;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u16 FIELDDATA_GetMapIDMax(void)
{
	return resistMapTblCount;
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
#define	NARC_nogrid_mapblock_h01a_dummy	NARC_nogrid_mapblock_h01a_00_01_3dppack
#include "fieldmap/nogrid_mapblock.naix"
static const FLDMAPPER_MAPDATA newbridgemap[] = {
	{ NARC_nogrid_mapblock_h01a_dummy },
	{ NARC_nogrid_mapblock_h01a_01_00_3dppack },
	{ NARC_nogrid_mapblock_h01a_02_00_3dppack },

	{ NARC_nogrid_mapblock_h01a_00_01_3dppack },
	{ NARC_nogrid_mapblock_h01a_01_01_3dppack },
	{ NARC_nogrid_mapblock_h01a_dummy },

	{ NARC_nogrid_mapblock_h01a_00_02_3dppack },
	{ NARC_nogrid_mapblock_h01a_01_02_3dppack },
	{ NARC_nogrid_mapblock_h01a_dummy },

	{ NARC_nogrid_mapblock_h01a_00_03_3dppack },
	{ NARC_nogrid_mapblock_h01a_01_03_3dppack },
	{ NARC_nogrid_mapblock_h01a_dummy },

	{ NARC_nogrid_mapblock_h01a_00_04_3dppack },
	{ NARC_nogrid_mapblock_h01a_01_04_3dppack },
	{ NARC_nogrid_mapblock_h01a_02_04_3dppack },
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static const FLDMAPPER_MAPDATA test_c3map[] = {
	{ NARC_nogrid_mapblock_C3_00_01_3dppack },
	{ NARC_nogrid_mapblock_C3_00_02_3dppack },
	{ NARC_nogrid_mapblock_C3_01_01_3dppack },
	{ NARC_nogrid_mapblock_C3_01_02_3dppack },
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static const FLDMAPPER_MAPDATA loopbridgemap[] = {
	{ NARC_nogrid_mapblock_m_test2_01_01_3dppack },
	{ NARC_nogrid_mapblock_m_test2_02_01_3dppack },
	{ NARC_nogrid_mapblock_m_test2_03_01_3dppack },
	{ NARC_nogrid_mapblock_m_test2_04_01_3dppack },
	{ NARC_nogrid_mapblock_m_test2_01_02_3dppack },
	{ NARC_nogrid_mapblock_m_test2_02_02_3dppack },
	{ NARC_nogrid_mapblock_m_test2_03_02_3dppack },
	{ NARC_nogrid_mapblock_m_test2_04_02_3dppack },
	{ NARC_nogrid_mapblock_m_test2_01_03_3dppack },
	{ NARC_nogrid_mapblock_m_test2_02_03_3dppack },
	{ NARC_nogrid_mapblock_m_test2_03_03_3dppack },
	{ NARC_nogrid_mapblock_m_test2_04_03_3dppack },
	{ NARC_nogrid_mapblock_m_test2_01_04_3dppack },
	{ NARC_nogrid_mapblock_m_test2_02_04_3dppack },
	{ NARC_nogrid_mapblock_m_test2_03_04_3dppack },
	{ NARC_nogrid_mapblock_m_test2_04_04_3dppack },
};

static const FLDMAPPER_MAPDATA samplebridge[] = {
	{ NARC_nogrid_mapblock_m_test_01_01_3dppack },
	{ NARC_nogrid_mapblock_m_test_02_01_3dppack },
	{ NARC_nogrid_mapblock_m_test_01_02_3dppack },
	{ NARC_nogrid_mapblock_m_test_02_02_3dppack },
	{ NARC_nogrid_mapblock_m_test_01_03_3dppack },
	{ NARC_nogrid_mapblock_m_test_02_03_3dppack },
	{ NARC_nogrid_mapblock_m_test_01_04_3dppack },
	{ NARC_nogrid_mapblock_m_test_02_04_3dppack },
	{ NARC_nogrid_mapblock_m_test_01_05_3dppack },
	{ NARC_nogrid_mapblock_m_test_02_05_3dppack },
	{ NARC_nogrid_mapblock_m_test_01_06_3dppack },
	{ NARC_nogrid_mapblock_m_test_02_06_3dppack },
};

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "fieldmap/all_build_model.naix"
static const FLDMAPPER_RESISTOBJDATA resistObjTbl2[] = {
	{ NARC_all_build_model_gate_01_nsbmd, NON_LOWQ},
	{ NARC_all_build_model_pc_01_nsbmd, NON_LOWQ},
	{ NARC_all_build_model_t2_build01_nsbmd, NON_LOWQ},
	{ NARC_all_build_model_t2_house01_nsbmd, NON_LOWQ},
};

#include "test_graphic/fieldmap_sample.naix"	//resistDDobjTblのために残している
#if 0
static const FLDMAPPER_RESISTOBJDATA	resistObjTbl[] = {
	// high quality model, low quality model のセットを登録する
	// high quality model, NON_LOWQでもOKらしい
	{ NARC_fieldmap_sample_pc_01_h_nsbmd, NARC_fieldmap_sample_pc_01_l_nsbmd },
	{ NARC_fieldmap_sample_buil_01_h_nsbmd, NARC_fieldmap_sample_buil_01_l_nsbmd },
};
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------
static const u16	resistDDobjTbl[] = {
	NARC_fieldmap_sample_sample_tree_nsbtx,
};

static const FLDMAPPER_RESISTDATA_OBJTBL	gobjData_Tbl = {
	ARCID_ALLBUILDMODEL, resistObjTbl2, NELEMS(resistObjTbl2), 
	//ARCID_FLDMAP_SAMPLE, resistObjTbl, NELEMS(resistObjTbl), 
	ARCID_FLDMAP_ACTOR, resistDDobjTbl, NELEMS(resistDDobjTbl),
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

//グローバルテクスチャサンプル（金銀マップテクスチャ）
#define DATID_GSMAP_GTEX (3)
static const FLDMAPPER_RESIST_TEX	gtexData = {
	ARCID_GSTEX, DATID_GSMAP_GTEX, 
};

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
	{	//実験マップ　橋
		{
			FLDMAPPER_FILETYPE_NORMAL,
			MAP_XZ_SIZE, 1024*FX32_ONE, FLDMAPPER_MODE_SCROLL_XZ, 
			ARCID_FLDMAP_LAND_EX,
			FLDMAPPER_RESIST_TEXTYPE_NONE,	NULL,
			FLDMAPPER_RESIST_OBJTYPE_TBL,	(void*)&gobjData_Tbl,

			3,	5, NELEMS(newbridgemap),
			newbridgemap,
		//	4,  4, NELEMS(loopbridgemap),
		//	loopbridgemap, 
		}, 
		&FieldNoGridFunctions,
		FALSE,
	},
	{	//実験マップ グリッド移動
		{
			FLDMAPPER_FILETYPE_NORMAL,
			MAP_XZ_SIZE, 1024*FX32_ONE, FLDMAPPER_MODE_SCROLL_XZ, 
			ARCID_FLDMAP_LANDDATA,
			FLDMAPPER_RESIST_TEXTYPE_NONE,	NULL,
			FLDMAPPER_RESIST_OBJTYPE_TBL,	(void*)&gobjData_Tbl,

			1,  1, 1,		//dummy map matrix data
			NULL, 
		},
		&FieldGridFunctions,
		TRUE,
	},
	{	//実験マップ　橋
		{	
			FLDMAPPER_FILETYPE_NORMAL,
			MAP_XZ_SIZE, 1024*FX32_ONE, FLDMAPPER_MODE_SCROLL_XZ, 
			ARCID_FLDMAP_LAND_EX,
			FLDMAPPER_RESIST_TEXTYPE_NONE,	NULL,
			FLDMAPPER_RESIST_OBJTYPE_TBL,	(void*)&gobjData_Tbl,

			2,  6, NELEMS(samplebridge),
			samplebridge, 
		}, 
		&FieldNoGridFunctions,
		FALSE,
	},
	{
		//実験マップ　グリッド移動 金銀
		{	
			FLDMAPPER_FILETYPE_PKGS,
			MAP_XZ_SIZE, 1024*FX32_ONE, FLDMAPPER_MODE_SCROLL_XZ, 
			ARCID_GSMAP, 
			FLDMAPPER_RESIST_TEXTYPE_USE,	&gtexData, 
			FLDMAPPER_RESIST_OBJTYPE_BIN,	(void*)&gobjData_Bin,

			2,  6, NELEMS(GSMap),
			GSMap, 
		},
		&FieldGridFunctions,
		FALSE,
	},
	{	//実験マップ　C3
		{	
			FLDMAPPER_FILETYPE_NORMAL,
			MAP_XZ_SIZE, 1024*FX32_ONE, FLDMAPPER_MODE_SCROLL_XZ, 
			ARCID_FLDMAP_LAND_EX,
			FLDMAPPER_RESIST_TEXTYPE_NONE,	NULL,
			FLDMAPPER_RESIST_OBJTYPE_TBL,	(void*)&gobjData_Tbl,

			2,  2, NELEMS(test_c3map),
			test_c3map, 
		}, 
		&FieldTestC3Functions,
		FALSE,
	},
};

const unsigned int resistMapTblCount = NELEMS(resistMapTbl);


