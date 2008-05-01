//============================================================================================
/**
 * @file	map3d.c
 * @brief	É}ÉbÉvê∂ê¨
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"
#include "calc_ph.h"

//------------------------------------------------------------------
/**
 * @brief	å^êÈåæ
 */
//------------------------------------------------------------------
#define EXOBJ_MAX	(64)
#define EXOBJ_NULL	(0xffff)

typedef struct {
	u16	id;
	u16	count;
}MAPOBJ_HEADER;

typedef struct {
	u8	vtxY0;	
	u8	vtxY1;
	u8	vtxY2;
	u8	vtxY3;
}MAP_GRIDMAKE_DATA;

typedef struct {
	u16		type;
	VecFx16	vtx0;
	VecFx16	vtx1;
	VecFx16	vtx2;
	VecFx32	vecN;
	fx32	valD;
}MAP_PLANE_DATA;

typedef struct {
	MAP_PLANE_DATA	triangle[2];
	u32				planeType;
	u8				passage;

}MAP_GRID_DATA;

struct _SCENE_MAP {
	GFL_G3D_SCENE*		g3Dscene;
	u16					unitIdx;
	u16					resIdx;
	u16					objIdx;
	MAPOBJ_HEADER		floor;
	MAP_GRID_DATA*		floorGrid;
	MAPOBJ_HEADER		extraObject[EXOBJ_MAX];
	u16					anmTimer;
};

typedef struct {
	const GFL_G3D_SCENEOBJ_DATA*	data;
	u16								dataCount;
	const char*						attr;
	const MAP_GRIDMAKE_DATA*		grid;
}MAPDATA;

typedef struct { 
	int			seq;

}EXMAPOBJ_WORK;//setup.cÇ≈éwíËÇ≥ÇÍÇƒÇ¢ÇÈÉTÉCÉYÇ…é˚ÇﬂÇÈÇÊÇ§Ç…íçà”

enum {
	QUAD_TYPE_PLANE = 0,
	QUAD_TYPE_SLIDE,
	TRIANGLE_TYPE_021_312,
	TRIANGLE_TYPE_230_103,
};

SCENE_MAP*		DEBUG_sceneMap;
#define mapScale		(FX32_ONE*1)
#define mapSizeX		(32)
#define mapSizeZ		(32)
#define mapGrid			(mapScale*16)
#define defaultMapX		(-mapGrid*(mapSizeX/2)+mapGrid/2)
#define defaultMapZ		(-mapGrid*(mapSizeZ/2)+mapGrid/2)
#define mapHeight		(FX16_ONE)

static void AddExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj, int objID, VecFx32* trans );
static void RemoveExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj );
static const	GFL_G3D_SCENEOBJ_DATA mapTest[4];
static const	MAPDATA extraObjectDataTbl[5];

static void	CreateMapGraphicData( SCENE_MAP* sceneMap, int mapID );
static void	DeleteMapGraphicData( SCENE_MAP* sceneMap );
static void CreateMapGridData( SCENE_MAP* sceneMap, int mapID, HEAPID heapID );
static void DeleteMapGridData( SCENE_MAP* sceneMap );

static void moveMapDebug( GFL_G3D_SCENEOBJ* sceneObj, void* work );
//------------------------------------------------------------------
/**
 * @brief	ÉZÉbÉgÉAÉbÉv
 */
//------------------------------------------------------------------
#include "graphic_data/test_graphic/test3d.naix"
#include "graphic_data/test_graphic/fld_map.naix"
#include "graphic_data/test_graphic/build_model_id.h"

enum {
	G3DRES_MAP_FLOOR = 0,
	G3DRES_EFFECT_WALL,
	G3DRES_FIELD_TEX1,
	G3DRES_FIELD_TEX2,
	G3DRES_TOWER_RED,
	G3DRES_TOWER_BLUE,
	G3DRES_TOWER_GREEN,
	G3DRES_TOWER_YELLOW,
	G3DRES_MAP_FLOOR0,
	G3DRES_MAP_FLOOR1,
	G3DRES_MAP_FLOOR2,
	G3DRES_MAP_FLOOR3,
	G3DRES_BLD_BUILD0,
	G3DRES_BLD_BUILD1,
	G3DRES_BLD_BUILD2,
	G3DRES_BLD_BUILD3,
	G3DRES_BLD_DOOR0,
	G3DRES_BLD_DOOR1,
	G3DRES_BLD_DOOR2,
	G3DRES_BLD_DOOR3,
};
//ÇRÇcÉOÉâÉtÉBÉbÉNÉäÉ\Å[ÉXÉeÅ[ÉuÉã
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{ ARCID_FLDMAP, NARC_fld_map_map14_16c_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_test_wall_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_field_tex1_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_field_tex2_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_tower_red_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_tower_blue_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_tower_green_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_SAMPLE, NARC_haruka_tower_yellow_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDMAP, NARC_fld_map_map21_18c_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDMAP, NARC_fld_map_map21_19c_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDMAP, NARC_fld_map_map22_18c_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDMAP, NARC_fld_map_map22_19c_nsbmd, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDBLD, BMID_PC, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDBLD, BMID_T5_S01, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDBLD, BMID_C4_H01A, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDBLD, BMID_T5_S02, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDBLD, BMID_T5_O01, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDBLD, BMID_T5_O01B, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDBLD, BMID_C4_DOOR1, GFL_G3D_UTIL_RESARC },
	{ ARCID_FLDBLD, BMID_P_DOOR, GFL_G3D_UTIL_RESARC },
};

//---------------------
enum {
	G3DOBJ_MAP_FLOOR = 0,
	G3DOBJ_EFFECT_WALL,
	G3DOBJ_FIELD_TEX1,
	G3DOBJ_FIELD_TEX2,
	G3DOBJ_TOWER_RED,
	G3DOBJ_TOWER_BLUE,
	G3DOBJ_TOWER_GREEN,
	G3DOBJ_TOWER_YELLOW,
	G3DOBJ_MAP_FLOOR0,
	G3DOBJ_MAP_FLOOR1,
	G3DOBJ_MAP_FLOOR2,
	G3DOBJ_MAP_FLOOR3,
	G3DOBJ_BLD_BUILD0,
	G3DOBJ_BLD_BUILD1,
	G3DOBJ_BLD_BUILD2,
	G3DOBJ_BLD_BUILD3,
	G3DOBJ_BLD_DOOR0,
	G3DOBJ_BLD_DOOR1,
	G3DOBJ_BLD_DOOR2,
	G3DOBJ_BLD_DOOR3,
};
//ÇRÇcÉIÉuÉWÉFÉNÉgíËã`ÉeÅ[ÉuÉã
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{ G3DRES_MAP_FLOOR, 0, G3DRES_MAP_FLOOR, NULL, 0 },
	{ G3DRES_EFFECT_WALL, 0, G3DRES_EFFECT_WALL, NULL, 0 },
	{ G3DRES_FIELD_TEX1, 0, G3DRES_FIELD_TEX1, NULL, 0 },
	{ G3DRES_FIELD_TEX2, 0, G3DRES_FIELD_TEX2, NULL, 0 },
	{ G3DRES_TOWER_RED, 0, G3DRES_TOWER_RED, NULL, 0 },
	{ G3DRES_TOWER_BLUE, 0, G3DRES_TOWER_BLUE, NULL, 0 },
	{ G3DRES_TOWER_GREEN, 0, G3DRES_TOWER_GREEN, NULL, 0 },
	{ G3DRES_TOWER_YELLOW, 0, G3DRES_TOWER_YELLOW, NULL, 0 },
	{ G3DRES_MAP_FLOOR0, 0, G3DRES_MAP_FLOOR0, NULL, 0 },
	{ G3DRES_MAP_FLOOR1, 0, G3DRES_MAP_FLOOR1, NULL, 0 },
	{ G3DRES_MAP_FLOOR2, 0, G3DRES_MAP_FLOOR2, NULL, 0 },
	{ G3DRES_MAP_FLOOR3, 0, G3DRES_MAP_FLOOR3, NULL, 0 },
	{ G3DRES_BLD_BUILD0, 0, G3DRES_BLD_BUILD0, NULL, 0 },
	{ G3DRES_BLD_BUILD1, 0, G3DRES_BLD_BUILD1, NULL, 0 },
	{ G3DRES_BLD_BUILD2, 0, G3DRES_BLD_BUILD2, NULL, 0 },
	{ G3DRES_BLD_BUILD3, 0, G3DRES_BLD_BUILD3, NULL, 0 },
	{ G3DRES_BLD_DOOR0, 0, G3DRES_BLD_DOOR0, NULL, 0 },
	{ G3DRES_BLD_DOOR1, 0, G3DRES_BLD_DOOR1, NULL, 0 },
	{ G3DRES_BLD_DOOR2, 0, G3DRES_BLD_DOOR2, NULL, 0 },
	{ G3DRES_BLD_DOOR3, 0, G3DRES_BLD_DOOR3, NULL, 0 },
};

//---------------------
//g3Dscene èâä˙ê›íËÉeÅ[ÉuÉãÉfÅ[É^
static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl, NELEMS(g3Dutil_resTbl),
	g3Dutil_objTbl, NELEMS(g3Dutil_objTbl),
};

static const GFL_G3D_SCENEOBJ_DATA mapGraphicData0[] = {
	{	G3DOBJ_MAP_FLOOR, 0, 1, 31, FALSE, TRUE,
		{	{ 0, -FX32_ONE*80, 0 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD0, 0, 1, 31, FALSE, TRUE,
		{	{ 0x89000, 0x21000, 0x93000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD1, 0, 1, 31, FALSE, TRUE,
		{	{ 0xffff7000, 0x20000, 0xfff29000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfff34000, 0x20000, 0xfff2d000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xa0000, 0x20000, 0xfff2d000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfff6d000, 0x20000, 0x99000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD3, 0, 1, 31, FALSE, TRUE,
		{	{ 0, 0, 0 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR0, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfffc8000, 0, 0xfff90000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR1, 0, 1, 31, FALSE, TRUE,
		{	{ 0x28000, 0, 0xfff90000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ 0x1000, 0x23000, 0xfff37000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfff36000, 0x23000, 0xfff3b000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xa2000, 0x23000, 0xfff3b000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfff6f000, 0x23000, 0xa6000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR3, 0, 1, 31, FALSE, TRUE,
		{	{ 0x88000, 0x23000, 0xa6000 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};

static const GFL_G3D_SCENEOBJ_DATA mapGraphicData1[] = {
	{	G3DOBJ_MAP_FLOOR0, 0, 1, 31, FALSE, TRUE,
		{	{ -FX32_ONE*256, -FX32_ONE*32, -FX32_ONE*256 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveMapDebug,
	},
	{	G3DOBJ_MAP_FLOOR1, 0, 1, 31, FALSE, TRUE,
		{	{ -FX32_ONE*256, -FX32_ONE*32, FX32_ONE*256 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveMapDebug,
	},
	{	G3DOBJ_MAP_FLOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ FX32_ONE*256, -FX32_ONE*32, -FX32_ONE*256 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveMapDebug,
	},
	{	G3DOBJ_MAP_FLOOR3, 0, 1, 31, FALSE, TRUE,
		{	{ FX32_ONE*256, -FX32_ONE*32, FX32_ONE*256 },
			{ mapScale, mapScale, mapScale },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveMapDebug,
	},
};

static const char mapAttr0[] = {
"Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°"	//16
"Å°ÅüÅüÅüÅüÇQÅ†Å†Å†Å†Å†ÇQÅüÅüÅüÅüÅüÅüÅüÇQÇQÇQÇQÇQÅüÅüÅüÅüÅ°Å°Å°Å°"	//
"Å°ÅüÅüÅüÅüÇQÅ†Å†Å†Å†Å†ÇQÅüÅüÅüÅüÅüÅüÅüÇQÇQÇQÇQÇQÅüÅüÅüÅüÅ°Å°Å°Å°"	//
"Å°ÅüÅüÅüÅüÇQÅ†Å†Å†Å†Å†ÇQÅüÅüÅüÅüÅüÅüÅüÇQÇQÇQÇQÇQÅüÅüÅüÅüÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å†Å†Å†Å°Å°Å°Å†Å†Å†Å°Å°ÇQÇQÇQÇQÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å†Å†Å†Å°Å†Å°Å†Å†Å†Å°Å°ÇQÇQÇQÇQÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÅ°Å°Å°Å°Å°Å°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°Å°Å°Å°Å°Å°ÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÅ°ÇPÇPÇPÇPÅ°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°ÇPÇPÇPÇPÅ°ÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÅ°ÇPÅ°Å°Å°Å°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°Å°Å°Å°ÇPÅ°ÇQÇQÅ°Å°Å°Å°"	//
"Å°ÇQÇQÇQÇQÅ°ÇPÅ°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°ÇPÅ°ÇQÇQÅ°Å°Å°Å°"	//
"ÇQÇQÇQÇQÇQÅ°ÇPÅ°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°ÇPÅ°ÇQÇQÅ°Å°Å°Å°"	//
"ÇQÇQÇQÇQÇQÅ°ÇPÅ®ÇOÇOÇOÇOÇOÇOÅüÅüÅüÇOÇOÇOÇOÇOÇOÅ©ÇPÅ°ÇQÇQÅ°Å°Å°Å°"	//
"ÇQÇQÇQÇQÇQÅ°ÇPÅ®ÇOÇOÇOÇOÇOÇOÅüÅüÅüÇOÇOÇOÇOÇOÇOÅ©ÇPÅ°ÇQÇQÇQÇQÇQÇQ"	//
"ÇQÇQÇQÇQÇQÅ°ÇPÅ®ÇOÇOÇOÇOÇOÇOÅüÅüÅüÇOÇOÇOÇOÇOÇOÅ©ÇPÅ°ÇQÇQÇQÇQÇQÇQ"	//
"ÇQÇQÇQÇQÇQÅ°ÇPÅ°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°ÇPÅ°ÇQÇQÇQÇQÇQÇQ"	//
"Å°Å°Å°ÇQÇQÅ°ÇPÅ°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°ÇPÅ°ÇQÇQÇQÇQÇQÇQ"	//
"Å°Å°Å°ÇQÇQÅ°ÇPÅ°Å°Å°Å°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°Å°Å°Å°ÇPÅ°ÇQÇQÇQÇQÇQÇQ"	//
"Å°Å°Å°ÇQÇQÅ°ÇPÇPÇPÇPÅ°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°ÇPÇPÇPÇPÅ°ÇQÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°ÇQÇQÅ°Å°Å°Å°Å°Å°ÇOÇOÇOÇOÇOÇOÇOÇOÇOÅ°Å°Å°Å°Å°Å°ÇQÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°ÇQÇQÇQÇQÇQÇQÅ°Å°Å°Å°Å™Å™Å™Å™Å™Å°Å°Å°Å°ÅüÅüÅüÅüÅüÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°ÇQÇQÅüÅüÅüÅüÅ°Å°Å°Å°Å™Å™Å™Å™Å™Å°Å°Å°Å°ÅüÅüÅüÅüÅüÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°ÇQÇQÅüÅüÅüÅüÅ†Å†Å†Å†ÇQÇQÇQÇQÇQÇQÇQÇQÇQÅüÅüÅüÅüÅüÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°ÇQÇQÅüÅüÅüÅüÅ†Å†Å†Å†Å†ÇQÇQÇQÇQÇQÇQÇQÇQÅüÅüÅüÅüÅüÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°ÇQÇQÇQÇQÇQÇQÅ†Å†Å†Å†Å†ÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°ÇQÇQÇQÇQÇQÇQÇQÅ†Å†Å†Å†ÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°ÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°Å°Å°ÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÇQÅ°Å°Å°"	//
"Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°Å°"	//
};

static const MAP_GRIDMAKE_DATA mapGrid0[] = {
//0//
{3,3,3,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},
{3,3,3,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},
{3,3,3,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},
{3,3,3,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//1//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//2//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//3//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//4//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//5//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//6//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//7//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//8//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,1},{2,2,1,1},{2,2,1,1},{2,2,1,1},{2,2,1,1},{2,2,1,1},{2,2,1,1},
{2,2,1,1},{2,2,1,1},{2,2,1,1},{2,2,1,1},{2,2,1,1},{2,2,1,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//9//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,1,2,1},{1,1,1,0},{1,1,0,0},{1,1,0,0},{1,1,0,0},{1,1,0,0},{1,1,0,0},
{1,1,0,0},{1,1,0,0},{1,1,0,0},{1,1,0,0},{1,1,0,0},{1,2,1,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//10//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,1},{2,2,1,1},{2,2,1,1},
{2,2,1,1},{2,1,1,1},{1,0,1,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},{1,2,1,1},{2,2,1,1},{2,2,1,1},
{2,2,1,1},{2,2,1,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//11//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,1,1,1},
{1,1,1,1},{1,1,1,1},{1,0,1,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},{1,1,1,1},{1,1,1,1},{1,1,1,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//12//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,1,1,0},
{1,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,0},{1,1,0,0},{1,1,0,0},{1,1,0,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//13//
{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//14//
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
//15//
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},
//16//
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
//17//
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
//18//
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
//19//
{2,2,3,3},{2,2,3,3},{2,2,3,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
//20//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,1},
{0,0,1,1},{0,0,1,1},{0,0,1,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,1},{0,0,1,1},{0,0,1,1},{0,1,1,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
//21//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,1,1,1},
{1,1,1,1},{1,1,1,1},{1,0,1,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},{1,1,1,1},{1,1,1,1},{1,1,1,1},
{1,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,3},{2,2,3,3},{2,2,3,3},
//22//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,1,2,2},{1,1,2,2},{1,1,2,2},
{1,1,2,2},{1,1,2,1},{1,0,1,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,1,0,1},{1,1,1,2},{1,1,2,2},{1,1,2,2},
{1,1,2,2},{1,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//23//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,1,2,1},{1,0,1,1},{0,0,1,1},{0,0,1,1},{0,0,1,1},{0,0,1,1},{0,0,1,1},
{0,0,1,1},{0,0,1,1},{0,0,1,1},{0,0,1,1},{0,1,1,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//24//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,1,2,2},{1,1,2,2},{1,1,2,2},{1,1,2,2},{1,1,2,2},{1,1,2,2},{1,1,2,2},
{1,1,2,2},{1,1,2,2},{1,1,2,2},{1,1,2,2},{1,1,2,2},{1,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//25//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//26//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//27//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//28//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//29//
{3,3,3,3},{3,3,3,3},{3,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//30//
{3,3,3,3},{3,3,3,3},{3,2,3,3},{2,2,3,3},{2,2,3,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},
{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,3,2,3},{3,3,3,3},{3,3,3,3},
//31//
{3,3,3,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},{3,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},
{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},
{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},
{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,2,3,3},{2,3,3,3},{3,3,3,3},{3,3,3,3},
};

static const MAPDATA mapData[] = {
	{ mapGraphicData0, NELEMS(mapGraphicData0), mapAttr0, mapGrid0 },
	{ mapGraphicData1, NELEMS(mapGraphicData1), mapAttr0, mapGrid0 },
};

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvê∂ê¨
 */
//------------------------------------------------------------------
SCENE_MAP*	Create3Dmap( GFL_G3D_SCENE* g3Dscene, HEAPID heapID )
{
	SCENE_MAP* sceneMap = GFL_HEAP_AllocClearMemory( heapID, sizeof(SCENE_MAP) );
	int	i;

	//ÇRÇcÉfÅ[É^ÉZÉbÉgÉAÉbÉv
	sceneMap->g3Dscene = g3Dscene;
	sceneMap->unitIdx = GFL_G3D_SCENE_AddG3DutilUnit( g3Dscene, &g3Dutil_setup );
	sceneMap->resIdx = GFL_G3D_SCENE_GetG3DutilUnitResIdx( g3Dscene, sceneMap->unitIdx );
	sceneMap->objIdx = GFL_G3D_SCENE_GetG3DutilUnitObjIdx( g3Dscene, sceneMap->unitIdx );

	//É}ÉbÉvçÏê¨
	{
		int mapID = 0;

		CreateMapGraphicData( sceneMap, mapID );
		CreateMapGridData( sceneMap, mapID, heapID );
	}

	for( i=0; i<EXOBJ_MAX; i++ ){
		sceneMap->extraObject[i].id = EXOBJ_NULL;
		sceneMap->extraObject[i].count = 0;
	}
	sceneMap->anmTimer = 0;

	DEBUG_sceneMap = sceneMap;
	return sceneMap;
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvîjä¸
 */
//------------------------------------------------------------------
void	Delete3Dmap( SCENE_MAP* sceneMap )
{
	int	i;

	for( i=0; i<EXOBJ_MAX; i++ ){
		if( sceneMap->extraObject[i].id != EXOBJ_NULL ){
			RemoveExtraObject( sceneMap, &sceneMap->extraObject[i] );
		}
	}
	DeleteMapGridData( sceneMap );
	DeleteMapGraphicData( sceneMap );
	GFL_G3D_SCENE_DelG3DutilUnit( sceneMap->g3Dscene, sceneMap->unitIdx );
	GFL_HEAP_FreeMemory( sceneMap );
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvÉIÉuÉWÉFÉNÉgí«â¡
 */
//------------------------------------------------------------------
int		AddObject3Dmap( SCENE_MAP* sceneMap, int objID, VecFx32* trans )
{
	int	i;

	for( i=0; i<EXOBJ_MAX; i++ ){
		if( sceneMap->extraObject[i].id == EXOBJ_NULL ){
			AddExtraObject( sceneMap, &sceneMap->extraObject[i], objID, trans );
			return i;
		}
	}
	GF_ASSERT(0);
	return 0;
}	

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvÉIÉuÉWÉFÉNÉgîjä¸
 */
//------------------------------------------------------------------
void	RemoveObject3Dmap( SCENE_MAP* sceneMap, int mapobjID )
{
	if( sceneMap->extraObject[mapobjID].id != EXOBJ_NULL ){
		RemoveExtraObject( sceneMap, &sceneMap->extraObject[mapobjID] );
		sceneMap->extraObject[mapobjID].id = EXOBJ_NULL;
	}
}	

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvÉIÉuÉWÉFÉNÉgï`âÊÉXÉCÉbÉ`ê›íË
 */
//------------------------------------------------------------------
void	Set3DmapDrawSw( SCENE_MAP* sceneMap, int mapobjID, BOOL* sw )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneMap->g3Dscene, 
													sceneMap->extraObject[mapobjID].id );
	GFL_G3D_SCENEOBJ_SetDrawSW( g3DsceneObj, sw );
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvÉIÉuÉWÉFÉNÉgï`âÊÉXÉCÉbÉ`éÊìæ
 */
//------------------------------------------------------------------
void	Get3DmapDrawSw( SCENE_MAP* sceneMap, int mapobjID, BOOL* sw )
{
	GFL_G3D_SCENEOBJ* g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneMap->g3Dscene, 
													sceneMap->extraObject[mapobjID].id );
	GFL_G3D_SCENEOBJ_GetDrawSW( g3DsceneObj, sw );
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvê∂ê¨ÉÅÉCÉì
 */
//------------------------------------------------------------------
static inline u16 GET_MAPCODE( const u16* mapdata, int x, int z )
{
	u16	tmpdata;

	if(( x < 0 )||( x >= mapSizeX )||( z < 0 )||( z >= mapSizeZ )){
		return  'Å°';
	}
	tmpdata = mapdata[ z * mapSizeX + x ];
	return  (( tmpdata & 0x00ff ) << 8 ) + (( tmpdata & 0xff00 ) >> 8 );
}

//------------------------------------------------------------------
/**
 * @brief		ÇRÇcÉ}ÉbÉvÉfÅ[É^ÉZÉbÉgÉAÉbÉv
 */
//------------------------------------------------------------------
static void	CreateMapGraphicData( SCENE_MAP* sceneMap, int mapID )
{
	GFL_G3D_SCENEOBJ_DATA*	pdata;

	GF_ASSERT( mapID < NELEMS(mapData) );

	sceneMap->floor.count = mapData[mapID].dataCount;
	sceneMap->floor.id = GFL_G3D_SCENEOBJ_Add(	sceneMap->g3Dscene, mapData[mapID].data,
												sceneMap->floor.count, sceneMap->objIdx );
}

//------------------------------------------------------------------
static void	DeleteMapGraphicData( SCENE_MAP* sceneMap )
{
	GFL_G3D_SCENEOBJ_Remove(	sceneMap->g3Dscene, 
								sceneMap->floor.id, 
								sceneMap->floor.count ); 
}


//------------------------------------------------------------------
/**
 * @brief		ÇRÇcÉ}ÉbÉvÉIÉuÉWÉFÉNÉgÉfÅ[É^ÉZÉbÉgÉAÉbÉv
 */
//------------------------------------------------------------------
static void AddExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj, int objID, VecFx32* trans )
{
	int	i;
	GFL_G3D_SCENEOBJ* g3DsceneObj;
	VecFx32	transTmp;

	exobj->count = extraObjectDataTbl[objID].dataCount;
	exobj->id = GFL_G3D_SCENEOBJ_Add(	sceneMap->g3Dscene, 
										extraObjectDataTbl[objID].data,
										exobj->count, 
										sceneMap->objIdx );

	for( i=0; i<exobj->count; i++ ){
		g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneMap->g3Dscene, exobj->id + i );
		GFL_G3D_SCENEOBJ_GetPos( g3DsceneObj, &transTmp );
		transTmp.x = trans->x;
		transTmp.z = trans->z;
		GFL_G3D_SCENEOBJ_SetPos( g3DsceneObj, &transTmp );
	}
}

//------------------------------------------------------------------
static void RemoveExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj )
{
	GFL_G3D_SCENEOBJ_Remove( sceneMap->g3Dscene, exobj->id, exobj->count ); 
}


//------------------------------------------------------------------
/**
 * @brief		ÇRÇcÉ}ÉbÉvÉIÉuÉWÉFÉNÉgÉfÅ[É^ìÆçÏ
 */
//------------------------------------------------------------------
static void moveExtraObject( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	EXMAPOBJ_WORK*	exw = (EXMAPOBJ_WORK*)work;

	if( exw->seq == 3 ){
		return;
	}
	if( exw->seq == 2 ){
		u8	blendAlphaEnd = 31;
		GFL_G3D_SCENEOBJ_SetBlendAlpha( sceneObj, &blendAlphaEnd );
		exw->seq++;
	}
	{
		VecFx32	transTmp;
		fx32	heightLimit = 0;

		GFL_G3D_SCENEOBJ_GetPos( sceneObj, &transTmp );
		if( transTmp.y < heightLimit ){	//çÇÇ≥É`ÉFÉbÉN
			transTmp.y += FX32_ONE/16;
			if( transTmp.y >= heightLimit ){
				transTmp.y = heightLimit;
				exw->seq++;
			}
			GFL_G3D_SCENEOBJ_SetPos( sceneObj, &transTmp );
		}
	}
	{
		u8	blendAlphaTmp;
		u8	alphaLimit = 20;

		GFL_G3D_SCENEOBJ_GetBlendAlpha( sceneObj, &blendAlphaTmp );
		if( blendAlphaTmp < alphaLimit ){	//ìßñæìxÉ`ÉFÉbÉN
			blendAlphaTmp++;
			if( blendAlphaTmp >= alphaLimit ){
				blendAlphaTmp = alphaLimit;
				exw->seq++;
			}
			GFL_G3D_SCENEOBJ_SetBlendAlpha( sceneObj, &blendAlphaTmp );
		}
	}
}

static void moveMapDebug( GFL_G3D_SCENEOBJ* sceneObj, void* work )
{
	VecFx32	trans;
	VecFx32	vecMove = { 0, 0, 0 };
	BOOL mvf = FALSE;

	if( GFL_UI_KEY_CheckCont( PAD_BUTTON_A ) == TRUE ){
		vecMove.y = -FX32_ONE;
		mvf = TRUE;
	} else if( GFL_UI_KEY_CheckCont( PAD_BUTTON_B ) == TRUE ){
		vecMove.y = FX32_ONE;
		mvf = TRUE;
	} else if( GFL_UI_KEY_CheckCont( PAD_KEY_UP ) == TRUE ){
		vecMove.z = -FX32_ONE;
		mvf = TRUE;
	} else if( GFL_UI_KEY_CheckCont( PAD_KEY_DOWN ) == TRUE ){
		vecMove.z = FX32_ONE;
		mvf = TRUE;
	} else if( GFL_UI_KEY_CheckCont( PAD_KEY_LEFT ) == TRUE ){
		vecMove.x = -FX32_ONE;
		mvf = TRUE;
	} else if( GFL_UI_KEY_CheckCont( PAD_KEY_RIGHT ) == TRUE ){
		vecMove.x = FX32_ONE;
		mvf = TRUE;
	}
	if( mvf == TRUE ){
		GFL_G3D_SCENEOBJ_GetPos( sceneObj, &trans );
		VEC_Add( &trans, &vecMove, &trans );
		GFL_G3D_SCENEOBJ_SetPos( sceneObj, &trans );
		OS_Printf("mvpos  {%x,%x,%x}\n", trans.x, trans.y, trans.z );
	}
}

//------------------------------------------------------------------
/**
 * @brief		ÉfÅ[É^
 */
//------------------------------------------------------------------
static const GFL_G3D_SCENEOBJ_DATA extraObject1[] = {
	{	G3DOBJ_EFFECT_WALL, 0, 1, 8, TRUE, TRUE,
		{	{ 0, -FX32_ONE*64, 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveExtraObject,
	},
};

static const GFL_G3D_SCENEOBJ_DATA towerRed[] = {
	{	G3DOBJ_TOWER_RED, 0, 1, 8, TRUE, TRUE,
		{	{ 0, 0, 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveExtraObject,
	},
};

static const GFL_G3D_SCENEOBJ_DATA towerBlue[] = {
	{	G3DOBJ_TOWER_BLUE, 0, 1, 8, TRUE, TRUE,
		{	{ 0, 0, 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveExtraObject,
	},
};

static const GFL_G3D_SCENEOBJ_DATA towerGreen[] = {
	{	G3DOBJ_TOWER_GREEN, 0, 1, 8, TRUE, TRUE,
		{	{ 0, 0, 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveExtraObject,
	},
};

static const GFL_G3D_SCENEOBJ_DATA towerYellow[] = {
	{	G3DOBJ_TOWER_YELLOW, 0, 1, 8, TRUE, TRUE,
		{	{ 0, 0, 0 },
			{ FX32_ONE, FX32_ONE, FX32_ONE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveExtraObject,
	},
};

static const	MAPDATA extraObjectDataTbl[5] = {
	{ extraObject1, NELEMS(extraObject1) },
	{ towerRed, NELEMS(towerRed) },
	{ towerBlue, NELEMS(towerBlue) },
	{ towerGreen, NELEMS(towerGreen) },
	{ towerYellow, NELEMS(towerYellow) },
};

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvçÇÇ≥ÉfÅ[É^çÏê¨
 */
//------------------------------------------------------------------
static void MakeGridData( MAP_GRID_DATA* floor, int gridx, int gridz, 
							int y0, int y1, int y2, int y3 );
static void MakeTriangleData
		( MAP_PLANE_DATA* triData, VecFx32* posRef, VecFx16* vtx0, VecFx16* vtx1, VecFx16* vtx2 );
static BOOL GetPlaneVtxHeight( u16 mapCode, int* y );

static void CreateMapGridData( SCENE_MAP* sceneMap, int mapID, HEAPID heapID )
{
	int	i, x, z;
	u16	mapCode;
	int	height, height2;
	const u16* attr = (const u16*)mapData->attr;
	const MAP_GRIDMAKE_DATA* grid = mapData->grid;

	sceneMap->floorGrid = GFL_HEAP_AllocClearMemory( heapID, 
								sizeof(MAP_GRID_DATA)*mapSizeX*mapSizeZ );

	for( z=0; z<mapSizeZ; z++ ){
		for( x=0; x<mapSizeX; x++ ){
			i = z * mapSizeX + x;

			mapCode = GET_MAPCODE( attr, x, z );

			MakeGridData( sceneMap->floorGrid, x, z, 
					grid[i].vtxY0, grid[i].vtxY1, grid[i].vtxY2, grid[i].vtxY3 );

			switch( mapCode ){
			default:
			case 'ÇO':
			case 'ÇP':
			case 'ÇQ':
			case 'ÇR':
			case 'Å®':	//äKíi
			case 'Å©':	//äKíi
			case 'Å™':	//äKíi
			case 'Å´':	//äKíi
				sceneMap->floorGrid[i].passage = 0;
				sceneMap->floorGrid[i].passage = 0;
				break;
			case 'Å°':	//äR
			case 'Åü':	//â∆
			case 'Å†':	//êÖ
				//MakeGridData( sceneMap->floorGrid, x, z, 0, 0, 0, 0 );
				sceneMap->floorGrid[i].passage = 1;
				break;
			}
		}
	}
}

static void DeleteMapGridData( SCENE_MAP* sceneMap )
{
	GFL_HEAP_FreeMemory( sceneMap->floorGrid );
}

static void	GetGroundGridData( const VecFx32* pos, int* x, int* z, u16* offset )
{
	*x = (pos->x + mapGrid * mapSizeX/2)/mapGrid;
	*z = (pos->z + mapGrid * mapSizeZ/2)/mapGrid;
	*offset = *z * mapSizeX + *x;
}

//------------------------------------------------------------------
static void inline posGridCalc( int x, int z, VecFx16* vtx, VecFx32* dst )
{
	VecFx32	tmp;

	tmp.x = (x - mapSizeX/2) * FX16_ONE + vtx->x;
	tmp.y = vtx->y;
	tmp.z = (z - mapSizeZ/2) * FX16_ONE + vtx->z;

	dst->x = FX_Mul( mapGrid, tmp.x );
	dst->y = FX_Mul( mapGrid, tmp.y );
	dst->z = FX_Mul( mapGrid, tmp.z );
}

static void MakeGridData( MAP_GRID_DATA* floor, int gridx, int gridz, 
							int y0, int y1, int y2, int y3 )
{
	MAP_GRID_DATA* grid = &floor[ gridz * mapSizeX + gridx ];
	VecFx32	posRef;
	VecFx16	vtx0, vtx1, vtx2, vtx3;

	//ñ@ê¸ÉxÉNÉgÉãéZèoópÅAí∏ì_ï˚å¸ÉfÅ[É^çÏê¨Åiï`âÊÇ…Ç‡égÇ¶ÇÈÅj
	vtx0.x = 0;
	vtx0.y = y0 * mapHeight;
	vtx0.z = 0;

	vtx1.x = FX16_ONE;
	vtx1.y = y1 * mapHeight;
	vtx1.z = 0;

	vtx2.x = 0;
	vtx2.y = y2 * mapHeight;
	vtx2.z = FX16_ONE;

	vtx3.x = FX16_ONE;
	vtx3.y = y3 * mapHeight;
	vtx3.z = FX16_ONE;

	if(( y0 == y3 )&&( y1 == y2 )&&( y0 == y1 )){
		grid->planeType = QUAD_TYPE_PLANE;
	} else if( y0 == y3 ){
		grid->planeType = TRIANGLE_TYPE_230_103;
	} else if( y1 == y2 ){
		grid->planeType = TRIANGLE_TYPE_021_312;
	} else {
		grid->planeType = QUAD_TYPE_SLIDE;
	}
	//ÉfÅ[É^äiî[(ñ@ê¸Ç™è„ÇÃèÍçáÇÕç∂âÒÇËÇ…í∏ì_ê›íË)
	if( grid->planeType == TRIANGLE_TYPE_021_312 ){
		//0-2-1 triangleê›íË
		posGridCalc( gridx, gridz, &vtx0, &posRef );	//vtx0ÇÃé¿ç¿ïWåvéZ
		MakeTriangleData( &grid->triangle[0], &posRef, &vtx0, &vtx2, &vtx1 );
		//3-1-2 triangleê›íË
		posGridCalc( gridx, gridz, &vtx3, &posRef );	//vtx3ÇÃé¿ç¿ïWåvéZ
		MakeTriangleData( &grid->triangle[1], &posRef, &vtx3, &vtx1, &vtx2 );
	} else {
		//2-3-0 triangleê›íË
		posGridCalc( gridx, gridz, &vtx2, &posRef );	//vtx2ÇÃé¿ç¿ïWåvéZ
		MakeTriangleData( &grid->triangle[0], &posRef, &vtx2, &vtx3, &vtx0 );
		//1-0-3 triangleê›íË
		posGridCalc( gridx, gridz, &vtx1, &posRef );	//vtx1ÇÃé¿ç¿ïWåvéZ
		MakeTriangleData( &grid->triangle[1], &posRef, &vtx1, &vtx0, &vtx3 );
	}
}

//------------------------------------------------------------------
static void MakeTriangleData
		( MAP_PLANE_DATA* triData, VecFx32* posRef, VecFx16* vtx0, VecFx16* vtx1, VecFx16* vtx2 )
{
	VecFx16	vec0, vec1, vecN;

	triData->vtx0 = *vtx0;
	triData->vtx1 = *vtx1;
	triData->vtx2 = *vtx2;

	VEC_Fx16Subtract( vtx1, vtx0, &vec0 );
	VEC_Fx16Subtract( vtx2, vtx0, &vec1 );
	VEC_Fx16CrossProduct( &vec0, &vec1, &vecN );
	VEC_Fx16Normalize( &vecN, &vecN );

	VEC_Set( &triData->vecN, (fx32)vecN.x, (fx32)vecN.y, (fx32)vecN.z );

	triData->valD = -(	  FX_Mul( triData->vecN.x, posRef->x ) 
						+ FX_Mul( triData->vecN.y, posRef->y ) 
						+ FX_Mul( triData->vecN.z, posRef->z ) ); 
}

//------------------------------------------------------------------
static BOOL GetPlaneVtxHeight( u16 mapCode, int* y )
{
	switch( mapCode ){
	case 'ÇO':
	case 'ÇP':
	case 'ÇQ':
	case 'ÇR':
		*y = mapCode - 'ÇO';
		return TRUE;
	}
	*y = 0;
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvçÇÇ≥ÉfÅ[É^éÊìæ
 */
//------------------------------------------------------------------
static BOOL	CheckGetGroundOutSideData( const int x, const int z )
{
	if(( x < 0 )||( x >= mapSizeX )||( z < 0 )||( z >= mapSizeZ ) ){ 
		return TRUE;
	} else {
		return FALSE;
	}
}

static BOOL	GetGroundTriangleID( MAP_GRID_DATA* floor, const VecFx32* pos, u16* gridOffs, u16* ID )
{
	int		gridx, gridz;
	fx32	fx, fz;

	GetGroundGridData( pos, &gridx, &gridz, gridOffs );
	fx = ( pos->x + mapGrid * mapSizeX/2 )%mapGrid;
	fz = ( pos->z + mapGrid * mapSizeZ/2 )%mapGrid;

	if( CheckGetGroundOutSideData( gridx, gridz ) == TRUE ){
		return FALSE;
	}
	//ÉOÉäÉbÉhì‡éOäpå`ÇÃîªíË
	if( floor[ *gridOffs ].planeType == TRIANGLE_TYPE_021_312 ){
		//0-2-1,3-1-2ÇÃÉpÉ^Å[Éì
		if( fx + fz < mapGrid ){
			*ID = 0;
			return TRUE;	//0-1-2éOäpå`ì‡
		} else {
			*ID = 1;
			return TRUE;	//3-2-1éOäpå`ì‡
		}
	} else {
		//2-3-0,1-0-3ÇÃÉpÉ^Å[Éì
		if( fx < fz ){
			*ID = 0;
			return TRUE;	//2-0-3éOäpå`ì‡
		} else {
			*ID = 1;
			return TRUE;	//1-3-0éOäpå`ì‡
		}
	}
	return FALSE;
}

BOOL	CheckGroundOutRange( SCENE_MAP* sceneMap, const VecFx32* pos )
{
	int	gridx, gridz;
	u16	gridOffs;

	GetGroundGridData( pos, &gridx, &gridz, &gridOffs );

	if( CheckGetGroundOutSideData( gridx, gridz ) == TRUE ){
		return FALSE;
	}
	if( sceneMap->floorGrid[ gridOffs ].passage != 0 ){
		return FALSE;
	}
	return TRUE;
}

BOOL	GetGroundPlaneData( SCENE_MAP* sceneMap, const VecFx32* pos, VecFx32* vecN, fx32* valD )
{
	u16		gridOffs, triangleID;

	if( GetGroundTriangleID( sceneMap->floorGrid, pos, &gridOffs, &triangleID ) == FALSE ){
		return FALSE;
	}
	*vecN	= sceneMap->floorGrid[ gridOffs ].triangle[ triangleID ].vecN;
	*valD	= sceneMap->floorGrid[ gridOffs ].triangle[ triangleID ].valD;

	return TRUE;
}

void	GetGroundPlaneVecN( SCENE_MAP* sceneMap, const VecFx32* pos, VecFx32* vecN )
{
	u16		gridOffs, triangleID;

	if( GetGroundTriangleID( sceneMap->floorGrid, pos, &gridOffs, &triangleID ) == FALSE ){
		vecN->x = 0;
		vecN->y = FX32_ONE;
		vecN->z = 0;
		return;
	}
	*vecN = sceneMap->floorGrid[ gridOffs ].triangle[ triangleID ].vecN;
}

void	GetGroundPlaneHeight( SCENE_MAP* sceneMap, const VecFx32* pos, fx32* height ) 
{
	u16		gridOffs, triangleID;
	VecFx32 vecN;
	fx32	by, valD;

	if( GetGroundTriangleID( sceneMap->floorGrid, pos, &gridOffs, &triangleID ) == FALSE ){
		*height = 0;
		return;
	}
	vecN = sceneMap->floorGrid[ gridOffs ].triangle[ triangleID ].vecN;
	valD = sceneMap->floorGrid[ gridOffs ].triangle[ triangleID ].valD;

	by = -( FX_Mul( vecN.x, pos->x ) + FX_Mul( vecN.z, pos->z ) + valD );
	*height = FX_Div( by, vecN.y );
}

void	GetGroundMoveVec
	( SCENE_MAP* sceneMap, const VecFx32* pos, const VecFx32* vecDir, VecFx32* vecMove )
{
	VecFx32 vecN, vecH, vecV;

	//ïΩñ ÇÃñ@ê¸ÉxÉNÉgÉãÇ…ÇÊÇËà⁄ìÆÉxÉNÉgÉãÇ…êÇíºÇ≈éŒñ Ç…ï¿çsÇ»ÉxÉNÉgÉãÇéZèo
	GetGroundPlaneVecN( sceneMap, pos, &vecN );				//ïΩñ ÇÃñ@ê¸ÇéÊìæ
	VEC_CrossProduct( &vecN, vecDir, &vecH );		//ïΩñ è„ÇÃêÖïΩÉxÉNÉgÉãéZèo
	VEC_CrossProduct( &vecN, &vecH, &vecV );		//ïΩñ è„ÇÃéŒñ ÉxÉNÉgÉãéZèo

	if( VEC_DotProduct( &vecV, vecDir ) < 0 ){
		//ãtï˚å¸Ç÷ÇÃÉxÉNÉgÉãÇ™èoÇƒÇµÇ‹Ç¡ÇΩèÍçáèCê≥
		VEC_Set( vecMove, -vecV.x, -vecV.y, -vecV.z );
	} else {
		VEC_Set( vecMove, vecV.x, vecV.y, vecV.z );
	}
	VEC_Normalize( vecMove, vecMove );
}

void	GetGroundMovePos
	( SCENE_MAP* sceneMap, const VecFx32* posNow, const VecFx32* vecMoveXZ, VecFx32* posNext ) 
{
	VecFx32	vecMove;

	//à⁄ìÆï˚å¸ÇÃínå`Ç…âàÇ¡ÇΩíPà ÉxÉNÉgÉãéÊìæ
	GetGroundMoveVec( sceneMap, posNow, vecMoveXZ, &vecMove );
	//à⁄ìÆÉxÉNÉgÉãéÊìæ
	VEC_Mult( VEC_Mag( vecMoveXZ ), &vecMove, &vecMove );
	//à⁄ìÆà íuÇÃåvéZ
	VEC_Add( posNow, &vecMove, posNext );
	GetGroundPlaneHeight( sceneMap, posNext, &posNext->y );
}

//------------------------------------------------------------------
//âºÅBÇ¢Ç∏ÇÍè¡Ç∑
BOOL	DEBUG_CheckGroundOutRange( const VecFx32* pos )
{
	return CheckGroundOutRange( DEBUG_sceneMap, pos );
}
BOOL	DEBUG_CheckGroundMove( const VecFx32* posNow, const VecFx32* vecMove, VecFx32* posNext )
{
	GetGroundMovePos( DEBUG_sceneMap, posNow, vecMove, posNext ); 
	//îÕàÕäOÉRÉìÉgÉçÅ[ÉãÅiñ{ìñÇÕäOïîä÷êîÇ≈Ç‚ÇÈÅj
	return CheckGroundOutRange( DEBUG_sceneMap, posNext );
}
void	GLOBAL_GetGroundPlaneVecN( const VecFx32* pos, VecFx32* vecN )
{
	GetGroundPlaneVecN( DEBUG_sceneMap, pos, vecN );
}

void	GLOBAL_GetGroundPlaneHeight( const VecFx32* pos, fx32* height )
{
	GetGroundPlaneHeight( DEBUG_sceneMap, pos, height );
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvÉäÉ\Å[ÉXÉfÅ[É^éÊìæ
 */
//------------------------------------------------------------------
static void GetMapTexAddress
			( const SCENE_MAP* sceneMap, const u16 idx, u32* texData, u32* texPltt )
{
	NNSG3dTexKey	texKey;
	NNSG3dPlttKey	plttKey;

	texKey = GFL_G3D_GetTextureDataVramKey
		( GFL_G3D_SCENE_GetG3DutilResHandle( sceneMap->g3Dscene, idx + sceneMap->resIdx ) );
	plttKey = GFL_G3D_GetTexturePlttVramKey
		( GFL_G3D_SCENE_GetG3DutilResHandle( sceneMap->g3Dscene, idx + sceneMap->resIdx ) );

	*texData = NNS_GfdGetTexKeyAddr( texKey );
	*texPltt = NNS_GfdGetPlttKeyAddr( plttKey );
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvÉfÉBÉXÉvÉåÉC
 */
//------------------------------------------------------------------
void	Draw3Dmap( SCENE_MAP* sceneMap, GFL_G3D_CAMERA* g3Dcamera )
{
	u32			texData, texPltt, texData1, texPltt1, texData2, texPltt2;
	VecFx32		camPos, camUp, target;

	//ÉeÉNÉXÉ`ÉÉê›íËéÊìæ
	GetMapTexAddress( sceneMap, G3DRES_FIELD_TEX1, &texData1, &texPltt1 );
	GetMapTexAddress( sceneMap, G3DRES_FIELD_TEX2, &texData2, &texPltt2 );
	//ÉJÉÅÉâê›íËéÊìæ
	GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
	GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
	GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

	//ínå`ï`âÊ
	{
		int			x, z;
		fx32		px, pz;
		int			offset;
		VecFx16		*pVtx0, *pVtx1, *pVtx2, *pVtx3;
		fx32		s0, t0, s1, t1, s2, t2;
		GXRgb		vtx0col, vtx1col, vtx2col;
		fx16		col;
		fx16		diff0, diff1, diff2;
		GXTexFmt	texFmt;
		int			i;
		VecFx16		vecN;

		G3X_Reset();
		G3_LookAt( &camPos, &camUp, &target, NULL);
#if 0
		{
			G3_MtxMode(GX_MTXMODE_TEXTURE);
			G3_Identity();
			// Use an identity matrix for the texture matrix for simplicity
			G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
		}
#endif
		//É}ÉeÉäÉAÉãê›íË
		G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), GX_RGB(16, 16, 16), TRUE );
		G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE );
		G3_PolygonAttr(GX_LIGHTMASK_0, GX_POLYGONMODE_MODULATE, GX_CULL_BACK, 63, 31, 0);

		G3_Scale( mapGrid, mapGrid, mapGrid );

		for( z=0; z<mapSizeZ; z++ ){
			for( x=0; x<mapSizeX; x++ ){
	
				px = (x - mapSizeX/2) * FX16_ONE; 
				pz = (z - mapSizeZ/2) * FX16_ONE;
				offset = z * mapSizeX + x;
	
				G3_PushMtx();
	
				G3_Translate( px, 0, pz );
	
				//G3_MultMtx33(const MtxFx33* m);
				
				G3_Begin( GX_BEGIN_TRIANGLES );
	
				for( i=0; i<2; i++ ){
					pVtx0 = &sceneMap->floorGrid[offset].triangle[i].vtx0;
					pVtx1 = &sceneMap->floorGrid[offset].triangle[i].vtx1;
					pVtx2 = &sceneMap->floorGrid[offset].triangle[i].vtx2;
					//vecN.x  = 0;//(fx16)sceneMap->floorGrid[offset].triangle[i].vecN.x;
					//vecN.y  = FX16_ONE;// (fx16)sceneMap->floorGrid[offset].triangle[i].vecN.y;
					//vecN.z  = 0;//(fx16)sceneMap->floorGrid[offset].triangle[i].vecN.z;
#if 0	
					if(( pVtx0->y == pVtx1->y )&&( pVtx0->y == pVtx2->y )){
#else
					if( pVtx0->y > pVtx1->y ){
						diff0 = pVtx0->y - pVtx1->y;
					} else if( pVtx0->y < pVtx1->y ){
						diff0 = pVtx1->y - pVtx0->y;
					} else {
						diff0 = 0;
					}
					if( pVtx1->y > pVtx2->y ){
						diff1 = pVtx1->y - pVtx2->y;
					} else if( pVtx1->y < pVtx2->y ){
						diff1 = pVtx2->y - pVtx1->y;
					} else {
						diff1 = 0;
					}
					if( pVtx2->y > pVtx0->y ){
						diff2 = pVtx2->y - pVtx0->y;
					} else if( pVtx2->y < pVtx0->y ){
						diff2 = pVtx0->y - pVtx2->y;
					} else {
						diff2 = 0;
					}
					if(( diff0 <= mapHeight )&&( diff1 <= mapHeight )&&( diff2 <= mapHeight )){
#endif
						//ïΩñ ÅiëêÅj
						texFmt	= GX_TEXFMT_PLTT16;// use 16 colors palette texture
						texData = texData1;
						texPltt = texPltt1;
						s0 = 0;
						t0 = 0;
						s1 = 0;
						t1 = 64 * FX32_ONE;
						s2 = 64 * FX32_ONE;
						t2 = 0;
					} else {
						//éŒñ ÅiìyÅj
						texFmt	= GX_TEXFMT_PLTT4;// use 4 colors palette texture
						texData = texData2;
						texPltt = texPltt2;
						s0 = 0;
						t0 = 0;
						s1 = 0;
						t1 = 64 * FX32_ONE;
						s2 = 64 * FX32_ONE;
						t2 = 0;
					}
					G3_TexImageParam(	texFmt, GX_TEXGEN_TEXCOORD,// use texcoord
										GX_TEXSIZE_S64, GX_TEXSIZE_T64,// 64 pixels
										GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE,
										GX_TEXPLTTCOLOR0_USE, texData );
					G3_TexPlttBase( texPltt, texFmt );
	
					//gxSt0 = GX_ST( pVtx0->x * 64, pVtx0->z * 64 );
					//gxSt1 = GX_ST( pVtx1->x * 64, pVtx1->z * 64 );
					//gxSt2 = GX_ST( pVtx2->x * 64, pVtx2->z * 64 );
					vtx0col = GX_RGB(16, 16, 16);
					vtx1col = GX_RGB(24, 24, 24);
					vtx2col = GX_RGB(31, 31, 31);
	
					//ÉâÉCÉgÇégópÇµÇ»Ç¢ÇÃÇ≈Normal(ñ@ê¸ÉxÉNÉgÉãê›íË)ÉRÉ}ÉìÉhÇÕî≠çsÇµÇ»Ç¢
					G3_Color( vtx0col );
					G3_TexCoord( s0, t0 );
					//G3_Normal( vecN.x, vecN.y, vecN.z );
					G3_Vtx( pVtx0->x, pVtx0->y, pVtx0->z );
	
					G3_Color( vtx1col );
					G3_TexCoord( s1, t1 );
					//G3_Normal( vecN.x, vecN.y, vecN.z );
					G3_Vtx( pVtx1->x, pVtx1->y, pVtx1->z );
	
					G3_Color( vtx2col );
					G3_TexCoord( s2, t2 );
					//G3_Normal( vecN.x, vecN.y, vecN.z );
					G3_Vtx( pVtx2->x, pVtx2->y, pVtx2->z );
				}
				G3_End();
	
				G3_PopMtx(1);
			}
		}
	}
	//êÖïΩê¸ï`âÊ
	{
		fx16 waterLine;

		sceneMap->anmTimer += 0x400;
		waterLine = 1 * FX_SinIdx( sceneMap->anmTimer );

		G3X_Reset();
		G3_LookAt( &camPos, &camUp, &target, NULL);

		//É}ÉeÉäÉAÉãê›íË
		G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), GX_RGB(16, 16, 16), TRUE );
		G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE );
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_BACK, 63, 8, 0);
	
		G3_Scale( FX32_ONE*64*32, FX32_ONE, FX32_ONE*64*32 );
		G3_PushMtx();
		G3_Translate( 0, -FX16_ONE*4, 0 );
	
		G3_Begin( GX_BEGIN_QUADS );
	
		//ÉâÉCÉgÇégópÇµÇ»Ç¢ÇÃÇ≈Normal(ñ@ê¸ÉxÉNÉgÉãê›íË)ÉRÉ}ÉìÉhÇÕî≠çsÇµÇ»Ç¢
		G3_Color( GX_RGB(0, 0, 16) );
		G3_Vtx( -FX16_ONE, waterLine, FX16_ONE );
		G3_Color( GX_RGB(0, 0, 31) );
		G3_Vtx( FX16_ONE, waterLine, FX16_ONE );
		G3_Color( GX_RGB(0, 0, 16) );
		G3_Vtx( FX16_ONE, waterLine, -FX16_ONE );
		G3_Color( GX_RGB(0, 0, 31) );
		G3_Vtx( -FX16_ONE, waterLine, -FX16_ONE );
	
		G3_End();
		G3_PopMtx(1);
	}
	//Å´å„Ç…G3D_SystemÇ≈çsÇ§ÇÃÇ≈ÅAÇ±Ç±Ç≈ÇÕÇ‚ÇÁÇ»Ç¢
	//G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvçÇÇ≥ïœçX
 */
//------------------------------------------------------------------
static void	GetGridVtxY( MAP_GRID_DATA* floor, const int gridx, const int gridz, 
							fx16* y0, fx16* y1, fx16* y2, fx16* y3 )
{
	MAP_GRID_DATA* grid = &floor[ gridz * mapSizeX + gridx ];

	if( grid->planeType == TRIANGLE_TYPE_021_312 ){
		*y0 = grid->triangle[0].vtx0.y;
		*y1 = grid->triangle[0].vtx2.y;
		*y2 = grid->triangle[1].vtx2.y;
		*y3 = grid->triangle[1].vtx0.y;
	} else {
		*y0 = grid->triangle[0].vtx2.y;
		*y1 = grid->triangle[1].vtx0.y;
		*y2 = grid->triangle[0].vtx0.y;
		*y3 = grid->triangle[1].vtx2.y;
	}
}

static u32	GetGridPlaneData( MAP_GRID_DATA* floor, const int gridx, const int gridz )
{
	MAP_GRID_DATA* grid = &floor[ gridz * mapSizeX + gridx ];

	return  grid->planeType;
}

static void	GetGridVtxYLen( MAP_GRID_DATA* floor, const int gridx, const int gridz, 
							int* y0, int* y1, int* y2, int* y3 )
{
	fx16 fy0, fy1, fy2, fy3;

	GetGridVtxY( floor, gridx, gridz, &fy0, &fy1, &fy2, &fy3 );
	*y0 = fy0/mapHeight;
	*y1 = fy1/mapHeight;
	*y2 = fy2/mapHeight;
	*y3 = fy3/mapHeight;
}

static void	SetGridAround( MAP_GRID_DATA* floor, const int gridx, const int gridz )
{
	int y0, y1, y2, y3;
	int ty0, ty1, ty2, ty3;

	GetGridVtxYLen( floor, gridx, gridz, &y0, &y1, &y2, &y3 );

	if( CheckGetGroundOutSideData( gridx-1, gridz-1 ) == FALSE ){
		GetGridVtxYLen( floor, gridx-1, gridz-1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( floor, gridx-1, gridz-1, ty0, ty1, ty2, y0 );
	}
	if( CheckGetGroundOutSideData( gridx, gridz-1 ) == FALSE ){
		GetGridVtxYLen( floor, gridx, gridz-1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( floor, gridx, gridz-1, ty0, ty1, y0, y1 );
	}
	if( CheckGetGroundOutSideData( gridx+1, gridz-1 ) == FALSE ){
		GetGridVtxYLen( floor, gridx+1, gridz-1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( floor, gridx+1, gridz-1, ty0, ty1, y1, ty3 );
	}
	if( CheckGetGroundOutSideData( gridx-1, gridz ) == FALSE ){
		GetGridVtxYLen( floor, gridx-1, gridz, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( floor, gridx-1, gridz, ty0, y0, ty2, y2 );
	}
	if( CheckGetGroundOutSideData( gridx+1, gridz ) == FALSE ){
		GetGridVtxYLen( floor, gridx+1, gridz, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( floor, gridx+1, gridz, y1, ty1, y3, ty3 );
	}
	if( CheckGetGroundOutSideData( gridx-1, gridz+1 ) == FALSE ){
		GetGridVtxYLen( floor, gridx-1, gridz+1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( floor, gridx-1, gridz+1, ty0, y2, ty2, ty3 );
	}
	if( CheckGetGroundOutSideData( gridx, gridz+1 ) == FALSE ){
		GetGridVtxYLen( floor, gridx, gridz+1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( floor, gridx, gridz+1, y2, y3, ty2, ty3 );
	}
	if( CheckGetGroundOutSideData( gridx+1, gridz+1 ) == FALSE ){
		GetGridVtxYLen( floor, gridx+1, gridz+1, &ty0, &ty1, &ty2, &ty3 );
		MakeGridData( floor, gridx+1, gridz+1, y3, ty1, ty2, ty3 );
	}
}

static void	SetGridUp( MAP_GRID_DATA* floor, const int gridx, const int gridz,
							const int y0, const int y1, const int y2, const int y3 )
{
	int	ymax;

	//OS_Printf("ëŒè€ínå` y0 = %x, y1 = %x, y2 = %x, y3 = %x\n", y0, y1, y2, y3 );
	ymax = y0;
	if( ymax < y1 ){ ymax = y1; }
	if( ymax < y2 ){ ymax = y2; }
	if( ymax < y3 ){ ymax = y3; }

	if( GetGridPlaneData( floor, gridx, gridz ) == QUAD_TYPE_PLANE ){
		if( ymax < (32-1) ){
			ymax++;	//óLå¯ílÇÕÅAfx16è„à Ç™3bitïùÇ»ÇÃÇ≈(fx16/4)íPà ÇæÇ∆-32<y<32
		}
	}
	MakeGridData( floor, gridx, gridz, ymax, ymax, ymax, ymax );
	SetGridAround( floor, gridx, gridz );
}

static void	SetGridDown( MAP_GRID_DATA* floor, const fx32 gridx, const fx32 gridz,
							const int y0, const int y1, const int y2, const int y3 )
{
	int	ymin;

	//OS_Printf("ëŒè€ínå` y0 = %x, y1 = %x, y2 = %x, y3 = %x\n", y0, y1, y2, y3 );
	ymin = y0;
	if( ymin > y1 ){ ymin = y1; }
	if( ymin > y2 ){ ymin = y2; }
	if( ymin > y3 ){ ymin = y3; }

	if( GetGridPlaneData( floor, gridx, gridz ) == QUAD_TYPE_PLANE ){
		if( ymin > -(32-1) ){
			ymin--;	//óLå¯ílÇÕÅAfx16è„à Ç™3bitïùÇ»ÇÃÇ≈(fx16/4)íPà ÇæÇ∆-32<y<32
		}
	}
	MakeGridData( floor, gridx, gridz, ymin, ymin, ymin, ymin );
	SetGridAround( floor, gridx, gridz );
}

static BOOL	CheckGridVtxUp( MAP_GRID_DATA* floor, const int gridx, const int gridz )
{
	int		ymax, y0, y1, y2, y3;
	BOOL	result = TRUE;

	GetGridVtxYLen( floor, gridx, gridz, &y0, &y1, &y2, &y3 );
	ymax = y0;
	if( ymax < y1 ){ ymax = y1; }
	if( ymax < y2 ){ ymax = y2; }
	if( ymax < y3 ){ ymax = y3; }

	if( y0 < (ymax - 1) ){
		y0 = ymax - 1;
		result = FALSE;
	}
	if( y1 < (ymax - 1) ){
		y1 = ymax - 1;
		result = FALSE;
	}
	if( y2 < (ymax - 1) ){
		y2 = ymax - 1;
		result = FALSE;
	}
	if( y3 < (ymax - 1) ){
		y3 = ymax - 1;
		result = FALSE;
	}
	if( result == FALSE ){
		MakeGridData( floor, gridx, gridz, y0, y1, y2, y3 );
		SetGridAround( floor, gridx, gridz );
	}
	return 	result;
}

static BOOL	CheckGridVtxDown( MAP_GRID_DATA* floor, const int gridx, const int gridz )
{
	int		ymin, y0, y1, y2, y3;
	BOOL	result = TRUE;

	GetGridVtxYLen( floor, gridx, gridz, &y0, &y1, &y2, &y3 );
	ymin = y0;
	if( ymin > y1 ){ ymin = y1; }
	if( ymin > y2 ){ ymin = y2; }
	if( ymin > y3 ){ ymin = y3; }

	if( y0 > (ymin + 1) ){
		y0 = ymin + 1;
		result = FALSE;
	}
	if( y1 > (ymin + 1) ){
		y1 = ymin + 1;
		result = FALSE;
	}
	if( y2 > (ymin + 1) ){
		y2 = ymin + 1;
		result = FALSE;
	}
	if( y3 > (ymin + 1) ){
		y3 = ymin + 1;
		result = FALSE;
	}
	if( result == FALSE ){
		MakeGridData( floor, gridx, gridz, y0, y1, y2, y3 );
		SetGridAround( floor, gridx, gridz );
	}
	return 	result;
}

void	SetMapGroundUp( SCENE_MAP* sceneMap, VecFx32* pos )
{
	int		gridx, gridz;
	int		y0, y1, y2, y3;
	u16		gridOffs;

//	OS_Printf("ínå`ÉAÉbÉv\n");
	GetGroundGridData( pos, &gridx, &gridz, &gridOffs );
	GetGridVtxYLen( sceneMap->floorGrid, gridx, gridz, &y0, &y1, &y2, &y3 );

	SetGridUp( sceneMap->floorGrid, gridx, gridz, y0, y1, y2, y3 );
#if 0
	{
		BOOL endFlag = FALSE;

		while( endFlag == FALSE ){
			int x, z;
			endFlag = TRUE;

			for( z=0; z<mapSizeZ; z++ ){
				for( x=0; x<mapSizeX; x++ ){
					if( CheckGridVtxUp( x, z ) == FALSE ){
						endFlag = FALSE;
					}
				}
			}
		}
	}
#endif
}

void	SetMapGroundDown( SCENE_MAP* sceneMap, VecFx32* pos )
{
	int		gridx, gridz;
	int		y0, y1, y2, y3;
	u16		gridOffs;

//	OS_Printf("ínå`É_ÉEÉì\n");
	GetGroundGridData( pos, &gridx, &gridz, &gridOffs );
	GetGridVtxYLen( sceneMap->floorGrid, gridx, gridz, &y0, &y1, &y2, &y3 );

	SetGridDown( sceneMap->floorGrid, gridx, gridz, y0, y1, y2, y3 );
#if 0
	{
		BOOL endFlag = FALSE;

		while( endFlag == FALSE ){
			int x, z;
			endFlag = TRUE;

			for( z=0; z<mapSizeZ; z++ ){
				for( x=0; x<mapSizeX; x++ ){
					if( CheckGridVtxDown( x, z ) == FALSE ){
						endFlag = FALSE;
					}
				}
			}
		}
	}
#endif
}

//------------------------------------------------------------------
/**
 * @brief	ÇRÇcÉ}ÉbÉvÇ∆ÉåÉCÇ∆ÇÃìñÇΩÇËç¿ïWéÊìæ
 */
//------------------------------------------------------------------
static BOOL checkOnTriangle( VecFx32* pos, 
		VecFx32* posRef0, VecFx32* posRef1, VecFx32* posRef2, VecFx32* vecN )
{
	VecFx32 vecEdge, vecVtx, vecTmp;
	fx32	scalar0, scalar1, scalar2;
			
	//àÍíËâÒÇËÇ…í∏ì_ÇåãÇ—ïÑçÜÇämîFÇ∑ÇÈ
	VEC_Subtract( pos, posRef0, &vecVtx );
	VEC_Subtract( posRef1, posRef0, &vecEdge );
	VEC_CrossProduct( &vecVtx, &vecEdge, &vecTmp );
	scalar0 = VEC_DotProduct( &vecTmp, vecN );

	VEC_Subtract( pos, posRef1, &vecVtx );
	VEC_Subtract( posRef2, posRef1, &vecEdge );
	VEC_CrossProduct( &vecVtx, &vecEdge, &vecTmp );
	scalar1 = VEC_DotProduct( &vecTmp, vecN );

	VEC_Subtract( pos, posRef2, &vecVtx );
	VEC_Subtract( posRef0, posRef2, &vecEdge );
	VEC_CrossProduct( &vecVtx, &vecEdge, &vecTmp );
	scalar2 = VEC_DotProduct( &vecTmp, vecN );

	if(( scalar0 > 0 )&&( scalar1 > 0 )&&( scalar2 > 0 )){
		return TRUE;
	}
	if(( scalar0 < 0 )&&( scalar1 < 0 )&&( scalar2 < 0 )){
		return TRUE;
	}
	return FALSE;
}

BOOL GetRayPosOnMap
	( SCENE_MAP* sceneMap, const VecFx32* posRay, const VecFx32* vecRay, VecFx32* dst )
{
	VecFx32 vecRayXZ, vecLength, posStart, pos;
	fx32	limitLength;
	int		x, z;
	u16		offset, prevOffset; 

	//ÉåÉCÇÃï˚å¸ê¨ï™ÇXZï˚å¸ÇæÇØî≤Ç´èoÇµÇƒê≥ãKâª
	VEC_Set( &vecRayXZ, vecRay->x, 0, vecRay->z );
	VEC_Normalize( &vecRayXZ, &vecRayXZ );

	VEC_Set( &posStart, posRay->x, 0, posRay->z );		//åüèoäJénínì_
	prevOffset = 0xffff;	//åüèoèâä˙âª
	vecLength.x = 0;
	vecLength.y = 0;
	vecLength.z = 0;
	{
		fx32 x_x = FX_Mul( mapSizeX * FX32_ONE, mapSizeX * FX32_ONE ); 
		fx32 z_z = FX_Mul( mapSizeZ * FX32_ONE, mapSizeZ * FX32_ONE ); 
		fx32 xz = FX_Sqrt( x_x + z_z );

		limitLength = FX_Mul( mapGrid, xz );
	}
	while( VEC_Mag( &vecLength ) <= limitLength ){
		VEC_Add( &posStart, &vecLength, &pos );

		GetGroundGridData( &pos, &x, &z, &offset );
		if( CheckGetGroundOutSideData( x, z ) == FALSE ){

			if( offset != prevOffset ){	//ìØÇ∂ÉfÅ[É^(offset)ÇÕÉpÉX
				MAP_GRID_DATA*	grid = &sceneMap->floorGrid[ offset ];
				VecFx32 posRef0, posRef1, posRef2, vecN, posCalc;
				BOOL result;
				int i;

				for( i=0; i<2; i++ ){
					//ÉOÉäÉbÉhì‡éOäpå`ÇPÇ™çÏÇÈïΩñ Ç∆ÇÃåç∑îªíË
					posGridCalc( x, z, &grid->triangle[i].vtx0, &posRef0 );
					posGridCalc( x, z, &grid->triangle[i].vtx1, &posRef1 );
					posGridCalc( x, z, &grid->triangle[i].vtx2, &posRef2 );
					vecN = grid->triangle[i].vecN;
	
					result = GFL_G3D_Calc_GetClossPointRayPlane
								( posRay, vecRay, &posRef0, &vecN, &posCalc, 0 );
					if( result == GFL_G3D_CALC_TRUE ){
						if( checkOnTriangle(&posCalc, &posRef0, &posRef1, &posRef2, &vecN)==TRUE ){
							*dst = posCalc;
							return TRUE;
						}
					}
				}
				prevOffset = offset;
			}
		}
		VEC_Add( &vecLength, &vecRayXZ, &vecLength );
	}
	//ÉåÉìÉWÉIÅ[ÉoÅ[Ç≈åüèoé∏îs
	//à”ñ°ÇÕÇ»Ç¢Ç™îOÇÃÇΩÇﬂdstÇ…ñ‚ëËÇ™î≠ê∂ÇµÇ»Ç¢ÇÊÇ§Ç»ílÇì¸ÇÍÇÈ
	dst->x = 0;
	dst->y = 0;
	dst->z = 0;
	return FALSE;
}


