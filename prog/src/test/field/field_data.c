//============================================================================================
//============================================================================================
#include <gflib.h>
#include "field_main.h"
#include "arc_def.h"
#include "field_debug.h"
#include "field_data.h"

#include "field/zonedata.h"
#include "map_matrix.h"

//#include "test_graphic/sample_map.naix"
//#include "test_graphic/fld_map.naix"
#include "test_graphic/test3dp.naix"

#include "test_graphic/fieldmap_sample.naix"
#include "test_graphic/fieldmap_map.naix"

typedef struct {
	//横ブロック数, 縦ブロック数, ブロック１辺の幅, グラフィックアーカイブＩＤ, 実マップデータ
	FLD_G3D_MAPPER_RESIST	mapperData;
	const DEPEND_FUNCTIONS * dep_funcs;
	BOOL isMatrixMapFlag;
}SCENE_DATA;

const SCENE_DATA	resistMapTbl[];
const unsigned int resistMapTblCount;
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static int MapID2ResistID(u16 mapid)
{
	GF_ASSERT(mapid < ZONEDATA_GetZoneIDMax());
	return ZONEDATA_GetMapRscID(mapid);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELDDATA_SetMapperData(u16 mapid, FLD_G3D_MAPPER_RESIST * map_res, void * matrix_buf)
{
	mapid = MapID2ResistID(mapid);
	GF_ASSERT(mapid < resistMapTblCount);
	*map_res = resistMapTbl[mapid].mapperData;
	if (resistMapTbl[mapid].isMatrixMapFlag){
		u8 *tbl;
		const MAP_MATRIX_HEADER *matH;
		
		GFL_ARC_LoadData(matrix_buf,
			ARCID_FLDMAP_MAPMATRIX, NARC_map_matrix_wb_mat_bin);
		matH = matrix_buf;
		tbl = (u8*)matrix_buf + sizeof(MAP_MATRIX_HEADER);
		
		map_res->sizex = matH->size_h;
		map_res->sizez = matH->size_v;
		map_res->totalSize = matH->size_h * matH->size_v;
		map_res->data = (const FLD_G3D_MAPPER_DATA *)tbl;
	}
	{
		int x,z;
		for (z = 0; z < map_res->sizez; z++) {
			for (x = 0; x < map_res->sizex; x++) {
				OS_Printf("%08x ",map_res->data[map_res->sizex * z + x]);
			}
			OS_Printf("\n");
		}
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const FLD_G3D_MAPPER_RESIST * FIELDDATA_GetMapperData(u16 mapid)
{
	mapid = MapID2ResistID(mapid);
	GF_ASSERT(mapid < resistMapTblCount);
	return &resistMapTbl[mapid].mapperData;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const DEPEND_FUNCTIONS * FIELDDATA_GetFieldFunctions(u16 mapid)
{
	mapid = MapID2ResistID(mapid);
	GF_ASSERT(mapid < resistMapTblCount);
	return resistMapTbl[mapid].dep_funcs;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u16 FIELDDATA_GetMapIDMax(void)
{
	return resistMapTblCount;
}

//============================================================================================
//============================================================================================
static const FLD_G3D_MAPPER_DATA sampleMap[] = {
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

#include "test_graphic/test3dp2.naix"

static const FLD_G3D_MAPPER_DATA sampleMap2[] = {
	{ NARC_test3dp2_m_test_01_01c_3dppack },
	{ NARC_test3dp2_m_test_02_01c_3dppack },
	{ NARC_test3dp2_m_test_01_02c_3dppack },
	{ NARC_test3dp2_m_test_02_02c_3dppack },
	{ NARC_test3dp2_m_test_01_03c_3dppack },
	{ NARC_test3dp2_m_test_02_03c_3dppack },
	{ NARC_test3dp2_m_test_01_04c_3dppack },
	{ NARC_test3dp2_m_test_02_04c_3dppack },
	{ NARC_test3dp2_m_test_01_05c_3dppack },
	{ NARC_test3dp2_m_test_02_05c_3dppack },
	{ NARC_test3dp2_m_test_01_06c_3dppack },
	{ NARC_test3dp2_m_test_02_06c_3dppack },
};

#include "test_graphic/loopbridge.naix"

static const FLD_G3D_MAPPER_DATA loopbridgemap[] = {
	{ NARC_loopbridge_m_test2_01_01c_3dppack },
	{ NARC_loopbridge_m_test2_02_01c_3dppack },
	{ NARC_loopbridge_m_test2_03_01c_3dppack },
	{ NARC_loopbridge_m_test2_04_01c_3dppack },
	{ NARC_loopbridge_m_test2_01_02c_3dppack },
	{ NARC_loopbridge_m_test2_02_02c_3dppack },
	{ NARC_loopbridge_m_test2_03_02c_3dppack },
	{ NARC_loopbridge_m_test2_04_02c_3dppack },
	{ NARC_loopbridge_m_test2_01_03c_3dppack },
	{ NARC_loopbridge_m_test2_02_03c_3dppack },
	{ NARC_loopbridge_m_test2_03_03c_3dppack },
	{ NARC_loopbridge_m_test2_04_03c_3dppack },
	{ NARC_loopbridge_m_test2_01_04c_3dppack },
	{ NARC_loopbridge_m_test2_02_04c_3dppack },
	{ NARC_loopbridge_m_test2_03_04c_3dppack },
	{ NARC_loopbridge_m_test2_04_04c_3dppack },
};

static const FLD_G3D_MAPPER_DATA fldmap3dp_test[] = {
	{ 0 },
};

static const FLD_G3D_MAPPEROBJ_DATA	resistObjTbl[] = {
	{ NARC_fieldmap_sample_pc_01_h_nsbmd, NARC_fieldmap_sample_pc_01_l_nsbmd },
	{ NARC_fieldmap_sample_buil_01_h_nsbmd, NARC_fieldmap_sample_buil_01_l_nsbmd },
};

static const u16	resistDDobjTbl[] = {
	NARC_fieldmap_sample_sample_tree_nsbtx,
};

#define DATID_GSMAP_GTEX (3)
static const FLD_G3D_MAPPER_GLOBAL_TEXTURE	gtexData = {
	ARCID_GSTEX, DATID_GSMAP_GTEX, 
};

static const FLD_G3D_MAPPER_GLOBAL_OBJSET_TBL	gobjData_Tbl = {
	ARCID_FLDMAP_SAMPLE, resistObjTbl, NELEMS(resistObjTbl), 
	ARCID_FLDMAP_ACTOR, resistDDobjTbl, NELEMS(resistDDobjTbl),
};

#define DATID_GSMAP_GOBJ (2)
static const FLD_G3D_MAPPER_GLOBAL_OBJSET_BIN	gobjData_Bin = {
	ARCID_GSAREAOBJ, ARCID_GSOBJANMTBL, DATID_GSMAP_GOBJ, 
	ARCID_GSOBJ, ARCID_GSOBJTEX, ARCID_GSOBJANM,
};

static const FLD_G3D_MAPPER_DATA GSMap[] = {
	{ 21 }, { 22 },
	{ 23 }, { 24 },
	{ 25 }, { 26 },
	{ FLD_G3D_MAPPER_NOMAP }, { 27 },
	{ FLD_G3D_MAPPER_NOMAP }, { 28 },
	{ FLD_G3D_MAPPER_NOMAP }, { 29 },
};

#define MAP_WIDTH (512 * FX32_ONE)
#define MAP_HEIGHT (128 * FX32_ONE)

const SCENE_DATA resistMapTbl[] = {
	{	//実験マップ　橋
		{
			FILE_MAPEDITER_DATA,
			MAP_WIDTH, 1024*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_XZ, 
			ARCID_LOOPBRIDGE,
			NON_GLOBAL_TEX, NULL,
			USE_GLOBAL_OBJSET_TBL, (void*)&gobjData_Tbl,

			4,  4, NELEMS(loopbridgemap),
			loopbridgemap, 
		}, 
		&FieldNoGridFunctions,
		FALSE,
	},
	{	//実験マップ グリッド移動
		{
			FILE_MAPEDITER_DATA,
			MAP_WIDTH, 1024*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_XZ, 
			ARCID_FLDMAP_LANDDATA,
			NON_GLOBAL_TEX, NULL,
			USE_GLOBAL_OBJSET_TBL, (void*)&gobjData_Tbl,

			1,  1, NELEMS(fldmap3dp_test),
			NULL, 
		},
		&FieldGridFunctions,
		TRUE,
	},
	{	//実験マップ　橋
		{	
			FILE_MAPEDITER_DATA,
			MAP_WIDTH, 1024*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_XZ, 
			ARCID_TEST3DP2,
			NON_GLOBAL_TEX, NULL,
			USE_GLOBAL_OBJSET_TBL, (void*)&gobjData_Tbl,

			2,  6, NELEMS(sampleMap2),
			sampleMap2, 
		}, 
		&FieldNoGridFunctions,
		FALSE,
	},
	{
		//実験マップ　グリッド移動 金銀
		{	
			FILE_CUSTOM_DATA,
			MAP_WIDTH, 1024*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_XZ, 
			ARCID_GSMAP, 
			USE_GLOBAL_TEX,	(void*)&gtexData, 
			USE_GLOBAL_OBJSET_BIN, (void*)&gobjData_Bin,

			2,  6, NELEMS(GSMap),
			GSMap, 
		},
		&FieldGridFunctions,
		FALSE,
	},
};

const unsigned int resistMapTblCount = NELEMS(resistMapTbl);


