#include <gflib.h>
#include "field_main.h"
#include "arc_def.h"

//#include "test_graphic/sample_map.naix"
//#include "test_graphic/fld_map.naix"
#include "test_graphic/test3dp.naix"

#include "test_graphic/fieldmap_sample.naix"
#include "test_graphic/fieldmap_map.naix"

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

const SCENE_DATA	resistMapTbl[] = {
	{
		{	
			FILE_CUSTOM_DATA,
			2,  6, NELEMS(GSMap), MAP_WIDTH, 1024*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_XZ, 
			ARCID_GSMAP, 
			USE_GLOBAL_TEX,	(void*)&gtexData, 
			//USE_GLOBAL_OBJSET_TBL, (void*)&gobjData_Tbl,
			USE_GLOBAL_OBJSET_BIN, (void*)&gobjData_Bin,
			GSMap, 
		}, 
		{ MAP_WIDTH*1, 0, MAP_WIDTH*1 },
		&FieldBaseFunctions,
	},
	{
		{	
			FILE_MAPEDITER_DATA,
			4,  4, NELEMS(sampleMap), MAP_WIDTH, 1024*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_XZ, 
			ARCID_TEST3DP,
			NON_GLOBAL_TEX, NULL,
			USE_GLOBAL_OBJSET_TBL, (void*)&gobjData_Tbl,
			sampleMap, 
		}, 
		{ MAP_WIDTH*2, 0, MAP_WIDTH*2 },
		&FieldGridFunctions,
	},
	{
		{	
			FILE_MAPEDITER_DATA,
			2,  6, NELEMS(sampleMap2), MAP_WIDTH, 1024*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_XZ, 
			ARCID_TEST3DP2,
			NON_GLOBAL_TEX, NULL,
			USE_GLOBAL_OBJSET_TBL, (void*)&gobjData_Tbl,
			sampleMap2, 
		}, 
		{ MAP_WIDTH*1, 0, MAP_WIDTH*1 },
		&FieldNoGridFunctions,
	},
};

const unsigned int resistMapTblCount = NELEMS(resistMapTbl);










#if 0
	{
		{	
			FILE_MAPEDITER_DATA,
			1,  1, NELEMS(rasenMap), MAP_WIDTH, 92*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_Y,
			ARCID_SAMPLEMAP, rasenMap 
		}, 
		{ MAP_WIDTH/2, 0, MAP_WIDTH/2 }
	},
	{
		{	
			FILE_MAPEDITER_DATA,
			1,  1, NELEMS(rasen2Map), MAP_WIDTH, 128*FX32_ONE, FLD_G3D_MAPPER_MODE_SCROLL_Y,
			ARCID_SAMPLEMAP, rasen2Map 
		}, 
		{ MAP_WIDTH/2, 0, MAP_WIDTH/2 }
	},
	{
		{	
			FILE_MAPEDITER_DATA,
			2,  6, NELEMS(sample2Map), MAP_WIDTH, MAP_HEIGHT, FLD_G3D_MAPPER_MODE_SCROLL_XZ,
			ARCID_SAMPLEMAP, sample2Map 
		}, 
		{ MAP_WIDTH*1, 0, MAP_WIDTH*5 }
	},
	{
		{ 
			FILE_MAPEDITER_DATA,
			32, 32, NELEMS(DPworldMap), MAP_WIDTH, MAP_HEIGHT, FLD_G3D_MAPPER_MODE_SCROLL_XZ,
			ARCID_FLDMAP, DPworldMap 
		}, 
		{ MAP_WIDTH*16, 0, MAP_WIDTH*16 }
	},
	{
		{  
			FILE_MAPEDITER_DATA,
			2,  2, NELEMS(DPc01Map), MAP_WIDTH, MAP_HEIGHT, FLD_G3D_MAPPER_MODE_SCROLL_NONE,
			ARCID_FLDMAP, DPc01Map 
		}, 
		{ MAP_WIDTH*1, 0, MAP_WIDTH*1 }
	},
	{
		{  
			FILE_MAPEDITER_DATA,
			1,  2, NELEMS(DPc02Map), MAP_WIDTH, MAP_HEIGHT, FLD_G3D_MAPPER_MODE_SCROLL_NONE,
			ARCID_FLDMAP, DPc02Map 
		}, 
		{ MAP_WIDTH*1, 0, MAP_WIDTH*1 }
	},
#endif
#if 0
	{{  2,  2, MAP_WIDTH, ARCID_FLDMAP, DPc03Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
	{{  2,  2, MAP_WIDTH, ARCID_FLDMAP, DPc04Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
	{{  2,  2, MAP_WIDTH, ARCID_FLDMAP, DPc05Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
	{{  2,  2, MAP_WIDTH, ARCID_FLDMAP, DPc06Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
	{{  2,  2, MAP_WIDTH, ARCID_FLDMAP, DPc07Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
	{{  2,  2, MAP_WIDTH, ARCID_FLDMAP, DPc08Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
	{{  1,  2, MAP_WIDTH, ARCID_FLDMAP, DPc09Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
	{{  1,  1, MAP_WIDTH, ARCID_FLDMAP, DPc10Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
	{{  2,  1, MAP_WIDTH, ARCID_FLDMAP, DPc11Map }, { MAP_WIDTH*1, 0, MAP_WIDTH*1 }},
#endif


#if 0
static const FLD_G3D_MAPPER_DATA sample2Map[] = {
	{ NARC_sample_map_m_test_01_01c_nsbmd, NARC_sample_map_m_test_01_01c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_02_01c_nsbmd, NARC_sample_map_m_test_02_01c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_01_02c_nsbmd, NARC_sample_map_m_test_01_02c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_02_02c_nsbmd, NARC_sample_map_m_test_02_02c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_01_03c_nsbmd, NARC_sample_map_m_test_01_03c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_02_03c_nsbmd, NARC_sample_map_m_test_02_03c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_01_04c_nsbmd, NARC_sample_map_m_test_01_04c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_02_04c_nsbmd, NARC_sample_map_m_test_02_04c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_01_05c_nsbmd, NARC_sample_map_m_test_01_05c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_02_05c_nsbmd, NARC_sample_map_m_test_02_05c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_01_06c_nsbmd, NARC_sample_map_m_test_01_06c_nsbtx, NON_ATTR }, 
	{ NARC_sample_map_m_test_02_06c_nsbmd, NARC_sample_map_m_test_02_06c_nsbtx, NON_ATTR }, 
};

static const FLD_G3D_MAPPER_DATA rasenMap[] = {
	{ NARC_sample_map_map_rasen_nsbmd, NARC_sample_map_map_rasen_nsbtx, 
		NARC_sample_map_map_rasen_bin },
	{ NARC_sample_map_map_rasen_nsbmd, NARC_sample_map_map_rasen_nsbtx, 
		NARC_sample_map_map_rasen_bin },
	{ NARC_sample_map_map_rasen_nsbmd, NARC_sample_map_map_rasen_nsbtx, 
		NARC_sample_map_map_rasen_bin },
	{ NARC_sample_map_map_rasen_nsbmd, NARC_sample_map_map_rasen_nsbtx, 
		NARC_sample_map_map_rasen_bin },
	{ NARC_sample_map_map_rasen_nsbmd, NARC_sample_map_map_rasen_nsbtx, 
		NARC_sample_map_map_rasen_bin },
};

static const FLD_G3D_MAPPER_DATA rasen2Map[] = {
	{ NARC_sample_map_map_rasen2_nsbmd, NARC_sample_map_map_rasen2_nsbtx, 
		NARC_sample_map_map_rasen2_bin },
	{ NARC_sample_map_map_rasen2_nsbmd, NARC_sample_map_map_rasen2_nsbtx, 
		NARC_sample_map_map_rasen2_bin },
	{ NARC_sample_map_map_rasen2_nsbmd, NARC_sample_map_map_rasen2_nsbtx, 
		NARC_sample_map_map_rasen2_bin },
	{ NARC_sample_map_map_rasen2_nsbmd, NARC_sample_map_map_rasen2_nsbtx, 
		NARC_sample_map_map_rasen2_bin },
	{ NARC_sample_map_map_rasen2_nsbmd, NARC_sample_map_map_rasen2_nsbtx, 
		NARC_sample_map_map_rasen2_bin },
};

//	NARC_fld_map_ro_treec_nsbmd = 178,
//	NARC_fld_map_ro_tree2c_nsbmd = 179,
//	NARC_fld_map_ro_tree3c_nsbmd = 180,
//	NARC_fld_map_ro_mountc_nsbmd = 181,
//	NARC_fld_map_ro_seac_nsbmd = 182,
//	NARC_fld_map_ro_snowc_nsbmd = 183,

static const FLD_G3D_MAPPER_DATA DPworldMap[] = {
	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_06c_nsbmd, NARC_fld_map_map09_06c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_map10_06c_nsbmd, NARC_fld_map_map10_06c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_map11_06c_nsbmd, NARC_fld_map_map11_06c_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_06c_nsbmd, NARC_fld_map_map28_06c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_07c_nsbmd, NARC_fld_map_map09_07c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_map10_07c_nsbmd, NARC_fld_map_map10_07c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_map11_07c_nsbmd, NARC_fld_map_map11_07c_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_map23_07c_nsbmd, NARC_fld_map_map23_07c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_07c_nsbmd, NARC_fld_map_map28_07c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_map01_08c_nsbmd, NARC_fld_map_map01_08c_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_map04_08c_nsbmd, NARC_fld_map_map04_08c_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_08c_nsbmd, NARC_fld_map_map09_08c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_map23_08c_nsbmd, NARC_fld_map_map23_08c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_08c_nsbmd, NARC_fld_map_map28_08c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_09c_nsbmd, NARC_fld_map_map09_09c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_map23_09c_nsbmd, NARC_fld_map_map23_09c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_09c_nsbmd, NARC_fld_map_map28_09c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_10c_nsbmd, NARC_fld_map_map09_10c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_10c_nsbmd, NARC_fld_map_map19_10c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_map20_10c_nsbmd, NARC_fld_map_map20_10c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_map21_10c_nsbmd, NARC_fld_map_map21_10c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_10c_nsbmd, NARC_fld_map_map22_10c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_map23_10c_nsbmd, NARC_fld_map_map23_10c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_map24_10c_nsbmd, NARC_fld_map_map24_10c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_10c_nsbmd, NARC_fld_map_map28_10c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_11c_nsbmd, NARC_fld_map_map09_11c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_11c_nsbmd, NARC_fld_map_map19_11c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_map24_11c_nsbmd, NARC_fld_map_map24_11c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_11c_nsbmd, NARC_fld_map_map28_11c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_12c_nsbmd, NARC_fld_map_map09_12c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_map10_12c_nsbmd, NARC_fld_map_map10_12c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_map11_12c_nsbmd, NARC_fld_map_map11_12c_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_12c_nsbmd, NARC_fld_map_map19_12c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_map24_12c_nsbmd, NARC_fld_map_map24_12c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_12c_nsbmd, NARC_fld_map_map28_12c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_tree3c_nsbmd, NARC_fld_map_ro_tree3c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_snowc_nsbmd, NARC_fld_map_ro_snowc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_13c_nsbmd, NARC_fld_map_map19_13c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_map20_13c_nsbmd, NARC_fld_map_map20_13c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_map21_13c_nsbmd, NARC_fld_map_map21_13c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_13c_nsbmd, NARC_fld_map_map22_13c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_map23_13c_nsbmd, NARC_fld_map_map23_13c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_map24_13c_nsbmd, NARC_fld_map_map24_13c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_map25_13c_nsbmd, NARC_fld_map_map25_13c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_13c_nsbmd, NARC_fld_map_map28_13c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_map25_14c_nsbmd, NARC_fld_map_map25_14c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_14c_nsbmd, NARC_fld_map_map28_14c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_map03_15c_nsbmd, NARC_fld_map_map03_15c_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_15c_nsbmd, NARC_fld_map_map28_15c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_map06_16c_nsbmd, NARC_fld_map_map06_16c_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_map07_16c_nsbmd, NARC_fld_map_map07_16c_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_map08_16c_nsbmd, NARC_fld_map_map08_16c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_16c_nsbmd, NARC_fld_map_map09_16c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_map10_16c_nsbmd, NARC_fld_map_map10_16c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_map11_16c_nsbmd, NARC_fld_map_map11_16c_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_map13_16c_nsbmd, NARC_fld_map_map13_16c_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_map14_16c_nsbmd, NARC_fld_map_map14_16c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_map15_16c_nsbmd, NARC_fld_map_map15_16c_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_map16_16c_nsbmd, NARC_fld_map_map16_16c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_16c_nsbmd, NARC_fld_map_map17_16c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_map27_16c_nsbmd, NARC_fld_map_map27_16c_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_map28_16c_nsbmd, NARC_fld_map_map28_16c_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_map06_17c_nsbmd, NARC_fld_map_map06_17c_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_map07_17c_nsbmd, NARC_fld_map_map07_17c_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_17c_nsbmd, NARC_fld_map_map09_17c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_17c_nsbmd, NARC_fld_map_map17_17c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_map26_17c_nsbmd, NARC_fld_map_map26_17c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_map27_17c_nsbmd, NARC_fld_map_map27_17c_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_18c_nsbmd, NARC_fld_map_map05_18c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_map06_18c_nsbmd, NARC_fld_map_map06_18c_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_18c_nsbmd, NARC_fld_map_map09_18c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_18c_nsbmd, NARC_fld_map_map17_18c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_map18_18c_nsbmd, NARC_fld_map_map18_18c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_18c_nsbmd, NARC_fld_map_map19_18c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_map20_18c_nsbmd, NARC_fld_map_map20_18c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_map21_18c_nsbmd, NARC_fld_map_map21_18c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_18c_nsbmd, NARC_fld_map_map22_18c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_map26_18c_nsbmd, NARC_fld_map_map26_18c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_19c_nsbmd, NARC_fld_map_map05_19c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_map06_19c_nsbmd, NARC_fld_map_map06_19c_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_19c_nsbmd, NARC_fld_map_map09_19c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_19c_nsbmd, NARC_fld_map_map17_19c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_map21_19c_nsbmd, NARC_fld_map_map21_19c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_19c_nsbmd, NARC_fld_map_map22_19c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_map26_19c_nsbmd, NARC_fld_map_map26_19c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_20c_nsbmd, NARC_fld_map_map05_20c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_map06_20c_nsbmd, NARC_fld_map_map06_20c_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_map07_20c_nsbmd, NARC_fld_map_map07_20c_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_20c_nsbmd, NARC_fld_map_map09_20c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_map14_20c_nsbmd, NARC_fld_map_map14_20c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_map15_20c_nsbmd, NARC_fld_map_map15_20c_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_20c_nsbmd, NARC_fld_map_map17_20c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_map18_20c_nsbmd, NARC_fld_map_map18_20c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_20c_nsbmd, NARC_fld_map_map22_20c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_map26_20c_nsbmd, NARC_fld_map_map26_20c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_21c_nsbmd, NARC_fld_map_map05_21c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_21c_nsbmd, NARC_fld_map_map09_21c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_map14_21c_nsbmd, NARC_fld_map_map14_21c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_map15_21c_nsbmd, NARC_fld_map_map15_21c_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_21c_nsbmd, NARC_fld_map_map17_21c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_21c_nsbmd, NARC_fld_map_map22_21c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_map23_21c_nsbmd, NARC_fld_map_map23_21c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_map24_21c_nsbmd, NARC_fld_map_map24_21c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_map26_21c_nsbmd, NARC_fld_map_map26_21c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//22
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_map01_22c_nsbmd, NARC_fld_map_map01_22c_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_22c_nsbmd, NARC_fld_map_map05_22c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_22c_nsbmd, NARC_fld_map_map09_22c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_map10_22c_nsbmd, NARC_fld_map_map10_22c_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_map12_22c_nsbmd, NARC_fld_map_map12_22c_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_map13_22c_nsbmd, NARC_fld_map_map13_22c_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_map14_22c_nsbmd, NARC_fld_map_map14_22c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_map15_22c_nsbmd, NARC_fld_map_map15_22c_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_map16_22c_nsbmd, NARC_fld_map_map16_22c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_22c_nsbmd, NARC_fld_map_map17_22c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_22c_nsbmd, NARC_fld_map_map22_22c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_map23_22c_nsbmd, NARC_fld_map_map23_22c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_map24_22c_nsbmd, NARC_fld_map_map24_22c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_map26_22c_nsbmd, NARC_fld_map_map26_22c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//23
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_map01_23c_nsbmd, NARC_fld_map_map01_23c_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_map02_23c_nsbmd, NARC_fld_map_map02_23c_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_map03_23c_nsbmd, NARC_fld_map_map03_23c_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_map04_23c_nsbmd, NARC_fld_map_map04_23c_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_23c_nsbmd, NARC_fld_map_map05_23c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_map06_23c_nsbmd, NARC_fld_map_map06_23c_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_map07_23c_nsbmd, NARC_fld_map_map07_23c_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_map08_23c_nsbmd, NARC_fld_map_map08_23c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_23c_nsbmd, NARC_fld_map_map09_23c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_map14_23c_nsbmd, NARC_fld_map_map14_23c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_23c_nsbmd, NARC_fld_map_map19_23c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_map21_23c_nsbmd, NARC_fld_map_map21_23c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_23c_nsbmd, NARC_fld_map_map22_23c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_map26_23c_nsbmd, NARC_fld_map_map26_23c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_map27_23c_nsbmd, NARC_fld_map_map27_23c_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//24
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_map04_24c_nsbmd, NARC_fld_map_map04_24c_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_24c_nsbmd, NARC_fld_map_map05_24c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_24c_nsbmd, NARC_fld_map_map09_24c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_map14_24c_nsbmd, NARC_fld_map_map14_24c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_24c_nsbmd, NARC_fld_map_map17_24c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_map18_24c_nsbmd, NARC_fld_map_map18_24c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_24c_nsbmd, NARC_fld_map_map19_24c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_map21_24c_nsbmd, NARC_fld_map_map21_24c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_24c_nsbmd, NARC_fld_map_map22_24c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_map23_24c_nsbmd, NARC_fld_map_map23_24c_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_map24_24c_nsbmd, NARC_fld_map_map24_24c_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_map25_24c_nsbmd, NARC_fld_map_map25_24c_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_map26_24c_nsbmd, NARC_fld_map_map26_24c_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_map27_24c_nsbmd, NARC_fld_map_map27_24c_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//25
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_map01_25c_nsbmd, NARC_fld_map_map01_25c_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_map02_25c_nsbmd, NARC_fld_map_map02_25c_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_25c_nsbmd, NARC_fld_map_map05_25c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_map14_25c_nsbmd, NARC_fld_map_map14_25c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_map18_25c_nsbmd, NARC_fld_map_map18_25c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_25c_nsbmd, NARC_fld_map_map19_25c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_map20_25c_nsbmd, NARC_fld_map_map20_25c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_map21_25c_nsbmd, NARC_fld_map_map21_25c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_25c_nsbmd, NARC_fld_map_map22_25c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//26
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_map01_26c_nsbmd, NARC_fld_map_map01_26c_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_map02_26c_nsbmd, NARC_fld_map_map02_26c_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_map03_26c_nsbmd, NARC_fld_map_map03_26c_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_map04_26c_nsbmd, NARC_fld_map_map04_26c_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_26c_nsbmd, NARC_fld_map_map05_26c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_map14_26c_nsbmd, NARC_fld_map_map14_26c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_map15_26c_nsbmd, NARC_fld_map_map15_26c_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_map16_26c_nsbmd, NARC_fld_map_map16_26c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_map17_26c_nsbmd, NARC_fld_map_map17_26c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_map18_26c_nsbmd, NARC_fld_map_map18_26c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_map19_26c_nsbmd, NARC_fld_map_map19_26c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_map20_26c_nsbmd, NARC_fld_map_map20_26c_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_map21_26c_nsbmd, NARC_fld_map_map21_26c_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_map22_26c_nsbmd, NARC_fld_map_map22_26c_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//27
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_map03_27c_nsbmd, NARC_fld_map_map03_27c_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_27c_nsbmd, NARC_fld_map_map05_27c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_tree2c_nsbmd, NARC_fld_map_ro_tree2c_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_map05_28c_nsbmd, NARC_fld_map_map05_28c_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_map06_28c_nsbmd, NARC_fld_map_map06_28c_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_map07_28c_nsbmd, NARC_fld_map_map07_28c_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_map08_28c_nsbmd, NARC_fld_map_map08_28c_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_map09_28c_nsbmd, NARC_fld_map_map09_28c_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_treec_nsbmd, NARC_fld_map_ro_treec_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
	//31
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//0
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//1
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//2
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//3
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//4
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//5
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//6
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//7
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//8
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//9
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//10
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//11
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//12
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//13
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//14
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//15
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//16
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//17
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//18
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//19
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//20
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//21
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//22
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//23
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//24
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//25
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//26
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//27
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//28
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//29
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//30
	{ NARC_fld_map_ro_seac_nsbmd, NARC_fld_map_ro_seac_nsbtx, NON_ATTR },	//31
};

static const FLD_G3D_MAPPER_DATA DPc01Map[] = {
	{ NARC_fld_map_map04_23c_nsbmd, NARC_fld_map_map04_23c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map05_23c_nsbmd, NARC_fld_map_map05_23c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map04_24c_nsbmd, NARC_fld_map_map04_24c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map05_24c_nsbmd, NARC_fld_map_map05_24c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc02Map[] = {
	{ NARC_fld_map_map01_22c_nsbmd, NARC_fld_map_map01_22c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map01_23c_nsbmd, NARC_fld_map_map01_23c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc03Map[] = {
	{ NARC_fld_map_map08_23c_nsbmd, NARC_fld_map_map08_23c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map09_23c_nsbmd, NARC_fld_map_map09_23c_nsbtx, NON_ATTR },
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },
	{ NARC_fld_map_map09_24c_nsbmd, NARC_fld_map_map09_24c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc04Map[] = {
	{ NARC_fld_map_map09_16c_nsbmd, NARC_fld_map_map09_16c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map10_16c_nsbmd, NARC_fld_map_map10_16c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map09_17c_nsbmd, NARC_fld_map_map09_17c_nsbtx, NON_ATTR },
	{ NARC_fld_map_ro_mountc_nsbmd, NARC_fld_map_ro_mountc_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc05Map[] = {
	{ NARC_fld_map_map14_21c_nsbmd, NARC_fld_map_map14_21c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map15_21c_nsbmd, NARC_fld_map_map15_21c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map14_22c_nsbmd, NARC_fld_map_map14_22c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map15_22c_nsbmd, NARC_fld_map_map15_22c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc06Map[] = {
	{ NARC_fld_map_map18_25c_nsbmd, NARC_fld_map_map18_25c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map19_25c_nsbmd, NARC_fld_map_map19_25c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map18_26c_nsbmd, NARC_fld_map_map18_26c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map19_26c_nsbmd, NARC_fld_map_map19_26c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc07Map[] = {
	{ NARC_fld_map_map21_18c_nsbmd, NARC_fld_map_map21_18c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map22_18c_nsbmd, NARC_fld_map_map22_18c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map21_19c_nsbmd, NARC_fld_map_map21_19c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map22_19c_nsbmd, NARC_fld_map_map22_19c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc08Map[] = {
	{ NARC_fld_map_map26_23c_nsbmd, NARC_fld_map_map26_23c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map27_23c_nsbmd, NARC_fld_map_map27_23c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map26_24c_nsbmd, NARC_fld_map_map26_24c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map27_24c_nsbmd, NARC_fld_map_map27_24c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc09Map[] = {
	{ NARC_fld_map_map11_06c_nsbmd, NARC_fld_map_map11_06c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map11_07c_nsbmd, NARC_fld_map_map11_07c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc10Map[] = {
	{ NARC_fld_map_map26_17c_nsbmd, NARC_fld_map_map26_17c_nsbtx, NON_ATTR },
};

static const FLD_G3D_MAPPER_DATA DPc11Map[] = {
	{ NARC_fld_map_map19_13c_nsbmd, NARC_fld_map_map19_13c_nsbtx, NON_ATTR },
	{ NARC_fld_map_map20_13c_nsbmd, NARC_fld_map_map20_13c_nsbtx, NON_ATTR },
};
#endif
