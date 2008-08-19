//============================================================================================
/**
 * @file	map3d.c
 * @brief	É}ÉbÉvê∂ê¨
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"
#include "calc_ph.h"
#include "arc/arc_def.h"

//------------------------------------------------------------------
/**
 * @brief	å^êÈåæ
 */
//------------------------------------------------------------------
#define MAP_BLOCK_COUNT	(4)
#define EXOBJ_MAX		(64)
#define EXOBJ_NULL		(0xffff)

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
	u8				planeType;
	u8				passage;
}MAP_GRID_DATA;

typedef struct {
	int					idx;
	MAPOBJ_HEADER		floor;
	MAP_GRID_DATA*		floorGrid;
	VecFx32				trans;
}MAP_BLOCK_DATA;

struct _SCENE_MAP {
	GFL_G3D_SCENE*		g3Dscene;
	u16					unitIdx;
	u16					resIdx;
	u16					objIdx;
	MAP_BLOCK_DATA		mapBlock[MAP_BLOCK_COUNT];
//	MAPOBJ_HEADER		floor;
//	MAP_GRID_DATA*		floorGrid;
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

#define MAP3D_HEIGHT_ONE	(FX16_ONE)

static void AddExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj, int objID, VecFx32* trans );
static void RemoveExtraObject( SCENE_MAP* sceneMap, MAPOBJ_HEADER* exobj );
static const	GFL_G3D_SCENEOBJ_DATA mapTest[4];
static const	MAPDATA extraObjectDataTbl[5];

static void	CreateMapGraphicData( SCENE_MAP* sceneMap, MAP_BLOCK_DATA* mapBlock, int mapID );
static void	DeleteMapGraphicData( SCENE_MAP* sceneMap, MAP_BLOCK_DATA* mapBlock );
static void CreateMapGridData( MAP_BLOCK_DATA* mapBlock, int mapID, HEAPID heapID );
static void DeleteMapGridData( MAP_BLOCK_DATA* mapBlock );

static void moveMapDebug( GFL_G3D_SCENEOBJ* sceneObj, void* work );
//------------------------------------------------------------------
/**
 * @brief	ÉZÉbÉgÉAÉbÉv
 */
//------------------------------------------------------------------
#include "arc/test_graphic/test3d.naix"
#include "arc/test_graphic/fld_map.naix"
#include "arc/test_graphic/build_model_id.h"

enum {
	G3DRES_MAP_FLOOR = 0,
	G3DRES_MAP_FLOOR_TEX,
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
	{ ARCID_FLDMAP, NARC_fld_map_map14_16c_nsbtx, GFL_G3D_UTIL_RESARC },
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
	{ G3DRES_MAP_FLOOR, 0, G3DRES_MAP_FLOOR_TEX, NULL, 0 },
	{ G3DRES_EFFECT_WALL, 0, G3DRES_EFFECT_WALL, NULL, 0 },
	{ G3DRES_FIELD_TEX1, 0, G3DRES_FIELD_TEX1, NULL, 0 },
	{ G3DRES_FIELD_TEX2, 0, G3DRES_FIELD_TEX2, NULL, 0 },
	{ G3DRES_TOWER_RED, 0, G3DRES_TOWER_RED, NULL, 0 },
	{ G3DRES_TOWER_BLUE, 0, G3DRES_TOWER_BLUE, NULL, 0 },
	{ G3DRES_TOWER_GREEN, 0, G3DRES_TOWER_GREEN, NULL, 0 },
	{ G3DRES_TOWER_YELLOW, 0, G3DRES_TOWER_YELLOW, NULL, 0 },
	{ G3DRES_MAP_FLOOR0, 0, G3DRES_MAP_FLOOR_TEX, NULL, 0 },
	{ G3DRES_MAP_FLOOR1, 0, G3DRES_MAP_FLOOR_TEX, NULL, 0 },
	{ G3DRES_MAP_FLOOR2, 0, G3DRES_MAP_FLOOR_TEX, NULL, 0 },
	{ G3DRES_MAP_FLOOR3, 0, G3DRES_MAP_FLOOR_TEX, NULL, 0 },
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
			{ MAP3D_SCALE, MAP3D_SCALE, MAP3D_SCALE },
			//{ FX32_ONE*2, FX32_ONE, FX32_ONE*2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD0, 0, 1, 31, FALSE, TRUE,
		{	{ 0x89000, 0x21800, 0x93000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD1, 0, 1, 31, FALSE, TRUE,
		{	{ 0xffff7000, 0x21800, 0xfff29000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfff34000, 0x21800, 0xfff2d000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xa0000, 0x21800, 0xfff2d000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfff6d000, 0x21800, 0x99000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_BUILD3, 0, 1, 31, FALSE, TRUE,
		{	{ 0, 0x1000, 0 },
			{ MAP3D_SCALE/2, MAP3D_SCALE/2, MAP3D_SCALE/2 },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR0, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfffc8000, 0, 0xfff90000 },
			{ MAP3D_SCALE, MAP3D_SCALE, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR1, 0, 1, 31, FALSE, TRUE,
		{	{ 0x28000, 0, 0xfff90000 },
			{ MAP3D_SCALE, MAP3D_SCALE, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ 0x1000, 0x22000, 0xfff36000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfff36000, 0x22000, 0xfff3a000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xa2000, 0x22000, 0xfff3a000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ 0xfff6f000, 0x22000, 0xa6000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
	{	G3DOBJ_BLD_DOOR3, 0, 1, 31, FALSE, TRUE,
		{	{ 0x88000, 0x22000, 0xa6000 },
			{ MAP3D_SCALE, MAP3D_SCALE/2, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},NULL,
	},
};
#if 0
static const GFL_G3D_SCENEOBJ_DATA mapGraphicData1[] = {
	{	G3DOBJ_MAP_FLOOR0, 0, 1, 31, FALSE, TRUE,
		{	{ -FX32_ONE*256, -FX32_ONE*32, -FX32_ONE*256 },
			{ MAP3D_SCALE, MAP3D_SCALE, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveMapDebug,
	},
	{	G3DOBJ_MAP_FLOOR1, 0, 1, 31, FALSE, TRUE,
		{	{ -FX32_ONE*256, -FX32_ONE*32, FX32_ONE*256 },
			{ MAP3D_SCALE, MAP3D_SCALE, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveMapDebug,
	},
	{	G3DOBJ_MAP_FLOOR2, 0, 1, 31, FALSE, TRUE,
		{	{ FX32_ONE*256, -FX32_ONE*32, -FX32_ONE*256 },
			{ MAP3D_SCALE, MAP3D_SCALE, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveMapDebug,
	},
	{	G3DOBJ_MAP_FLOOR3, 0, 1, 31, FALSE, TRUE,
		{	{ FX32_ONE*256, -FX32_ONE*32, FX32_ONE*256 },
			{ MAP3D_SCALE, MAP3D_SCALE, MAP3D_SCALE },
			{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },
		},moveMapDebug,
	},
};
#endif

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
{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},
{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},
{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,2},{3,3,2,3},{3,3,3,3},{3,3,3,3},{3,3,3,3},
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
{2,2,2,2},{2,1,2,1},{1,1,1,0},{1,1,0,0},{1,1,0,0},{1,1,0,0},{1,1,0,0},{0,0,0,0},
{1,1,0,0},{1,1,0,0},{1,1,0,0},{1,1,0,0},{1,1,0,1},{1,2,1,2},{2,2,2,2},{2,2,2,2},
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
{3,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,2,2,2},{2,1,2,1},{1,1,1,1},{1,0,1,0},
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
//	{ mapGraphicData1, NELEMS(mapGraphicData1), mapAttr0, mapGrid0 },
};

static const VecFx32 mapTransOffs[] = {
#if 1
	{ (FX32_ONE*512)*0, 0, (FX32_ONE*512)*0 },
	{ (FX32_ONE*512)*1, 0, (FX32_ONE*512)*0 },
	{ (FX32_ONE*512)*0, 0, (FX32_ONE*512)*1 },
	{ (FX32_ONE*512)*1, 0, (FX32_ONE*512)*1 },
#else
	{ (FX32_ONE*512)*0, 0, (FX32_ONE*512)*0 },
	{ (FX32_ONE*512)*1, 0, (FX32_ONE*512)*0 },
	{ (FX32_ONE*512)*2, 0, (FX32_ONE*512)*0 },
	{ (FX32_ONE*512)*0, 0, (FX32_ONE*512)*1 },
	{ (FX32_ONE*512)*1, 0, (FX32_ONE*512)*1 },
	{ (FX32_ONE*512)*2, 0, (FX32_ONE*512)*1 },
	{ (FX32_ONE*512)*0, 0, (FX32_ONE*512)*2 },
	{ (FX32_ONE*512)*1, 0, (FX32_ONE*512)*2 },
	{ (FX32_ONE*512)*2, 0, (FX32_ONE*512)*2 },
#endif
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

		for( i=0; i<MAP_BLOCK_COUNT; i++ ){
			sceneMap->mapBlock[i].idx = i;
			CreateMapGraphicData( sceneMap, &sceneMap->mapBlock[i], mapID );
			CreateMapGridData( &sceneMap->mapBlock[i], mapID, heapID );
			sceneMap->mapBlock[i].trans.x = mapTransOffs[i].x;
			sceneMap->mapBlock[i].trans.y = mapTransOffs[i].y;
			sceneMap->mapBlock[i].trans.z = mapTransOffs[i].z;
			{
				GFL_G3D_SCENEOBJ* g3DsceneObj;
				VecFx32 trans;
				int j;

				for( j=0; j<sceneMap->mapBlock[i].floor.count; j++ ){
					g3DsceneObj = GFL_G3D_SCENEOBJ_Get( sceneMap->g3Dscene, 
													sceneMap->mapBlock[i].floor.id + j );
					GFL_G3D_SCENEOBJ_GetPos( g3DsceneObj, &trans );
					VEC_Add( &trans, &sceneMap->mapBlock[i].trans, &trans );
					GFL_G3D_SCENEOBJ_SetPos( g3DsceneObj, &trans );
				}
			}
		}
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
	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		DeleteMapGridData( &sceneMap->mapBlock[i] );
		DeleteMapGraphicData( sceneMap, &sceneMap->mapBlock[i] );
	}
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

	if(( x < 0 )||( x >= MAP3D_GRID_SIZEX )||( z < 0 )||( z >= MAP3D_GRID_SIZEZ )){
		return  'Å°';
	}
	tmpdata = mapdata[ z * MAP3D_GRID_SIZEX + x ];
	return  (( tmpdata & 0x00ff ) << 8 ) + (( tmpdata & 0xff00 ) >> 8 );
}

//------------------------------------------------------------------
/**
 * @brief		ÇRÇcÉ}ÉbÉvÉfÅ[É^ÉZÉbÉgÉAÉbÉv
 */
//------------------------------------------------------------------
static void	CreateMapGraphicData( SCENE_MAP* sceneMap, MAP_BLOCK_DATA* mapBlock, int mapID )
{
	GFL_G3D_SCENEOBJ_DATA*	pdata;

	GF_ASSERT( mapID < NELEMS(mapData) );

	mapBlock->floor.count = mapData[mapID].dataCount;
	mapBlock->floor.id = GFL_G3D_SCENEOBJ_Add(	sceneMap->g3Dscene, 
												mapData[mapID].data,
												mapData[mapID].dataCount,
												sceneMap->objIdx );
}

//------------------------------------------------------------------
static void	DeleteMapGraphicData( SCENE_MAP* sceneMap, MAP_BLOCK_DATA* mapBlock )
{
	GFL_G3D_SCENEOBJ_Remove(	sceneMap->g3Dscene, 
								mapBlock->floor.id, 
								mapBlock->floor.count ); 
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
static void MakeGridData( MAP_GRID_DATA* floor, u16 gridx, u16 gridz, 
							int y0, int y1, int y2, int y3 );
static void MakeTriangleData
		( MAP_PLANE_DATA* triData, VecFx32* posRef, VecFx16* vtx0, VecFx16* vtx1, VecFx16* vtx2 );

static void CreateMapGridData( MAP_BLOCK_DATA* mapBlock, int mapID, HEAPID heapID )
{
	int	i;
	u16	x, z;
	u16	mapCode;
	int	height, height2;
	const u16* attr = (const u16*)mapData[mapID].attr;
	const MAP_GRIDMAKE_DATA* grid = mapData[mapID].grid;

	mapBlock->floorGrid = GFL_HEAP_AllocClearMemory( heapID, 
								sizeof(MAP_GRID_DATA) * MAP3D_GRID_SIZEX * MAP3D_GRID_SIZEZ );

	for( z=0; z<MAP3D_GRID_SIZEZ; z++ ){
		for( x=0; x<MAP3D_GRID_SIZEX; x++ ){
			i = z * MAP3D_GRID_SIZEX + x;

			mapCode = GET_MAPCODE( attr, x, z );

			MakeGridData( mapBlock->floorGrid, x, z, 
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
				mapBlock->floorGrid[i].passage = 0;
				mapBlock->floorGrid[i].passage = 0;
				break;
			case 'Å°':	//äR
			case 'Åü':	//â∆
			case 'Å†':	//êÖ
				mapBlock->floorGrid[i].passage = 1;
				break;
			}
		}
	}
}

static void DeleteMapGridData( MAP_BLOCK_DATA* mapBlock )
{
	GFL_HEAP_FreeMemory( mapBlock->floorGrid );
}

//------------------------------------------------------------------
static void inline posGridCalc( u16 x, u16 z, VecFx16* vtx, VecFx32* dst )
{
	VecFx32	tmp;

	tmp.x = x * FX16_ONE + vtx->x;
	tmp.y = vtx->y;
	tmp.z = z * FX16_ONE + vtx->z;

	dst->x = FX_Mul( MAP3D_GRID_LEN, tmp.x );
	dst->y = FX_Mul( MAP3D_GRID_LEN, tmp.y );
	dst->z = FX_Mul( MAP3D_GRID_LEN, tmp.z );
}

static void MakeGridData( MAP_GRID_DATA* floor, u16 gridx, u16 gridz, 
							int y0, int y1, int y2, int y3 )
{
	MAP_GRID_DATA* grid = &floor[ gridz * MAP3D_GRID_SIZEX + gridx ];
	VecFx32	posRef;
	VecFx16	vtx0, vtx1, vtx2, vtx3;

	//ñ@ê¸ÉxÉNÉgÉãéZèoópÅAí∏ì_ï˚å¸ÉfÅ[É^çÏê¨Åiï`âÊÇ…Ç‡égÇ¶ÇÈÅj
	vtx0.x = 0;
	vtx0.y = y0 * MAP3D_HEIGHT_ONE;
	vtx0.z = 0;

	vtx1.x = FX16_ONE;
	vtx1.y = y1 * MAP3D_HEIGHT_ONE;
	vtx1.z = 0;

	vtx2.x = 0;
	vtx2.y = y2 * MAP3D_HEIGHT_ONE;
	vtx2.z = FX16_ONE;

	vtx3.x = FX16_ONE;
	vtx3.y = y3 * MAP3D_HEIGHT_ONE;
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
		posGridCalc( gridx, gridz, &vtx0, &posRef );	//vtx0ÇÃÉçÅ[ÉJÉãé¿ç¿ïWåvéZ
		MakeTriangleData( &grid->triangle[0], &posRef, &vtx0, &vtx2, &vtx1 );
		//3-1-2 triangleê›íË
		posGridCalc( gridx, gridz, &vtx3, &posRef );	//vtx3ÇÃÉçÅ[ÉJÉãé¿ç¿ïWåvéZ
		MakeTriangleData( &grid->triangle[1], &posRef, &vtx3, &vtx1, &vtx2 );
	} else {
		//2-3-0 triangleê›íË
		posGridCalc( gridx, gridz, &vtx2, &posRef );	//vtx2ÇÃÉçÅ[ÉJÉãé¿ç¿ïWåvéZ
		MakeTriangleData( &grid->triangle[0], &posRef, &vtx2, &vtx3, &vtx0 );
		//1-0-3 triangleê›íË
		posGridCalc( gridx, gridz, &vtx1, &posRef );	//vtx1ÇÃÉçÅ[ÉJÉãé¿ç¿ïWåvéZ
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
/**
 * @brief	ÇRÇcÉ}ÉbÉvçÇÇ≥ÉfÅ[É^éÊìæ
 */
//------------------------------------------------------------------
static BOOL	GetGroundGridPos
		( SCENE_MAP* sceneMap, const VecFx32* pos, u16* blockID, VecFx32* blockPos )
{
	VecFx32 vecTop, vecGrid;
	VecFx32 vecDefault = {0,0,0};
	fx32	mapLengthX = MAP3D_GRID_SIZEX*MAP3D_GRID_LEN;
	fx32	mapLengthZ = MAP3D_GRID_SIZEZ*MAP3D_GRID_LEN;
	int i;

	for( i=0; i<MAP_BLOCK_COUNT; i++ ){
		vecTop.x = sceneMap->mapBlock[i].trans.x - mapLengthX/2;
		vecTop.y = 0;
		vecTop.z = sceneMap->mapBlock[i].trans.z - mapLengthZ/2;

		if(	(pos->x >= vecTop.x)&&(pos->x < vecTop.x+mapLengthX)
			&&(pos->z >= vecTop.z)&&(pos->z < vecTop.z+mapLengthZ) ){

			*blockID = i;
			VEC_Subtract( &vecDefault, &vecTop, &vecGrid );
			VEC_Add( &vecGrid, pos, blockPos );
			return TRUE;
		}
	}
	return FALSE;
}

static void	GetGroundGridData( const VecFx32* blockPos, u16* gridx, u16* gridz, u16* offset )
{
	*gridx = blockPos->x/MAP3D_GRID_LEN;
	*gridz = blockPos->z/MAP3D_GRID_LEN;
	*offset = *gridz * MAP3D_GRID_SIZEX + *gridx;
}

static void	GetGroundTriangleID( const VecFx32* pos, const MAP_GRID_DATA* floor, u16* ID )
{
	fx32 fx, fz;

	fx = pos->x%MAP3D_GRID_LEN;
	fz = pos->z%MAP3D_GRID_LEN;

	//ÉOÉäÉbÉhì‡éOäpå`ÇÃîªíË
	if( floor->planeType == TRIANGLE_TYPE_021_312 ){
		//0-2-1,3-1-2ÇÃÉpÉ^Å[Éì
		if( fx + fz < MAP3D_GRID_LEN ){
			*ID = 0;
		} else {
			*ID = 1;
		}
	} else if( floor->planeType == TRIANGLE_TYPE_230_103 ){
		//2-3-0,1-0-3ÇÃÉpÉ^Å[Éì
		if( fx < fz ){
			*ID = 0;
		} else {
			*ID = 1;
		}
	} else {
		//ïΩñ 
		*ID = 0;
	}
}

BOOL	CheckGroundOutRange( SCENE_MAP* sceneMap, const VecFx32* pos )
{
	u16		blockID, gridx, gridz, gridOffs;
	VecFx32 blockPos;

	if( GetGroundGridPos( sceneMap, pos, &blockID, &blockPos ) == FALSE ){
		return FALSE;
	}
	GetGroundGridData( &blockPos, &gridx, &gridz, &gridOffs );

	if( sceneMap->mapBlock[blockID].floorGrid[ gridOffs ].passage != 0 ){
		return FALSE;
	}
	return TRUE;
}

BOOL	GetGroundPlaneData( SCENE_MAP* sceneMap, const VecFx32* pos, VecFx32* vecN, fx32* valD )
{
	u16		blockID, gridx, gridz, gridOffs, triangleID;
	VecFx32 blockPos;

	if( GetGroundGridPos( sceneMap, pos, &blockID, &blockPos ) == FALSE ){
		return FALSE;
	}
	GetGroundGridData( &blockPos, &gridx, &gridz, &gridOffs );
	GetGroundTriangleID( &blockPos, &sceneMap->mapBlock[blockID].floorGrid[gridOffs], &triangleID );

	*vecN	= sceneMap->mapBlock[blockID].floorGrid[ gridOffs ].triangle[ triangleID ].vecN;
	*valD	= sceneMap->mapBlock[blockID].floorGrid[ gridOffs ].triangle[ triangleID ].valD;

	return TRUE;
}

void	GetGroundPlaneVecN( SCENE_MAP* sceneMap, const VecFx32* pos, VecFx32* vecN )
{
	u16		blockID, gridx, gridz, gridOffs, triangleID;
	VecFx32 blockPos;

	if( GetGroundGridPos( sceneMap, pos, &blockID, &blockPos ) == FALSE ){
		vecN->x = 0;
		vecN->y = FX32_ONE;
		vecN->z = 0;
		return;
	}
	GetGroundGridData( &blockPos, &gridx, &gridz, &gridOffs );
	GetGroundTriangleID( &blockPos, &sceneMap->mapBlock[blockID].floorGrid[gridOffs], &triangleID );

	*vecN = sceneMap->mapBlock[blockID].floorGrid[ gridOffs ].triangle[ triangleID ].vecN;
}

void	GetGroundPlaneHeight( SCENE_MAP* sceneMap, const VecFx32* pos, fx32* height ) 
{
	u16		blockID, gridx, gridz, gridOffs, triangleID;
	VecFx32 blockPos;
	VecFx32 vecN;
	fx32	by, valD;

	if( GetGroundGridPos( sceneMap, pos, &blockID, &blockPos ) == FALSE ){
		*height = 0;
		return;
	}
	GetGroundGridData( &blockPos, &gridx, &gridz, &gridOffs );
	GetGroundTriangleID( &blockPos, &sceneMap->mapBlock[blockID].floorGrid[gridOffs], &triangleID );

	vecN = sceneMap->mapBlock[blockID].floorGrid[ gridOffs ].triangle[ triangleID ].vecN;
	valD = sceneMap->mapBlock[blockID].floorGrid[ gridOffs ].triangle[ triangleID ].valD;

	by = -( FX_Mul( vecN.x, blockPos.x ) + FX_Mul( vecN.z, blockPos.z ) + valD );
	*height = FX_Div( by, vecN.y );
}

void	GetGroundMoveVec
	( SCENE_MAP* sceneMap, const VecFx32* pos, const VecFx32* vecDir, VecFx32* vecMove )
{
	VecFx32 vecN, posNext;
	fx32 by, valD;

	GetGroundPlaneVecN( sceneMap, pos, &vecN );				//ïΩñ ÇÃñ@ê¸ÇéÊìæ
	VEC_Add( pos, vecDir, &posNext );

	valD = -( FX_Mul(vecN.x,pos->x) + FX_Mul(vecN.y,pos->y) + FX_Mul(vecN.z,pos->z) ); 
	by = -( FX_Mul( vecN.x, posNext.x ) + FX_Mul( vecN.z, posNext.z ) + valD );
	posNext.y = FX_Div( by, vecN.y );

	VEC_Subtract( &posNext, pos, vecMove );

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
 * @brief	ÇRÇcÉ}ÉbÉvÇ∆ÉåÉCÇ∆ÇÃìñÇΩÇËç¿ïWéÊìæ
 */
//------------------------------------------------------------------
static void inline posGridCalc2( int x, int z, VecFx16* vtx, VecFx32* dst )
{
	VecFx32	tmp;

	tmp.x = (x - MAP3D_GRID_SIZEX/2) * FX16_ONE + vtx->x;
	tmp.y = vtx->y;
	tmp.z = (z - MAP3D_GRID_SIZEZ/2) * FX16_ONE + vtx->z;

	dst->x = FX_Mul( MAP3D_GRID_LEN, tmp.x );
	dst->y = FX_Mul( MAP3D_GRID_LEN, tmp.y );
	dst->z = FX_Mul( MAP3D_GRID_LEN, tmp.z );
}

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
	VecFx32 vecRayXZ, vecLength, posStart, pos, blockPos;
	fx32	limitLength;
	u16		x, z, offset, prevOffset, blockID, prevBlockID;

	//ÉåÉCÇÃï˚å¸ê¨ï™ÇXZï˚å¸ÇæÇØî≤Ç´èoÇµÇƒê≥ãKâª
	VEC_Set( &vecRayXZ, vecRay->x, 0, vecRay->z );
	VEC_Normalize( &vecRayXZ, &vecRayXZ );

	VEC_Set( &posStart, posRay->x, 0, posRay->z );		//åüèoäJénínì_
	prevBlockID = 0xffff;	//åüèoèâä˙âª
	prevOffset = 0xffff;	//åüèoèâä˙âª
	vecLength.x = 0;
	vecLength.y = 0;
	vecLength.z = 0;
	{
		fx32 x_x = FX_Mul( MAP3D_GRID_SIZEX * FX32_ONE, MAP3D_GRID_SIZEX * FX32_ONE ); 
		fx32 z_z = FX_Mul( MAP3D_GRID_SIZEZ * FX32_ONE, MAP3D_GRID_SIZEZ * FX32_ONE ); 
		fx32 xz = FX_Sqrt( x_x + z_z );

		limitLength = FX_Mul( MAP3D_GRID_LEN, xz );
	}
	while( VEC_Mag( &vecLength ) <= limitLength ){
		VEC_Add( &posStart, &vecLength, &pos );

		if( GetGroundGridPos( sceneMap, &pos, &blockID, &blockPos ) == TRUE ){
			GetGroundGridData( &blockPos, &x, &z, &offset );
	
			if(( blockID != prevBlockID )||( offset != prevOffset )){	//ìØÇ∂ÉfÅ[É^ÇÕÉpÉX
				MAP_GRID_DATA*	grid = &sceneMap->mapBlock[blockID].floorGrid[offset];
				VecFx32 posRef0, posRef1, posRef2, vecN, posCalc;
				VecFx32* transOffs =  &sceneMap->mapBlock[blockID].trans;
				BOOL result;
				int i;

				for( i=0; i<2; i++ ){
					//ÉOÉäÉbÉhì‡éOäpå`ÇPÇ™çÏÇÈïΩñ Ç∆ÇÃåç∑îªíË
					posGridCalc2( x, z, &grid->triangle[i].vtx0, &posRef0 );
					posGridCalc2( x, z, &grid->triangle[i].vtx1, &posRef1 );
					posGridCalc2( x, z, &grid->triangle[i].vtx2, &posRef2 );
					vecN = grid->triangle[i].vecN;
					VEC_Add( &posRef0, transOffs, &posRef0 );
					VEC_Add( &posRef1, transOffs, &posRef1 );
					VEC_Add( &posRef2, transOffs, &posRef2 );
	
					result = GFL_G3D_Calc_GetClossPointRayPlane
								( posRay, vecRay, &posRef0, &vecN, &posCalc, 0 );
					if( result == GFL_G3D_CALC_TRUE ){
						if( checkOnTriangle(&posCalc, &posRef0, &posRef1, &posRef2, &vecN)==TRUE ){
							*dst = posCalc;
							//OS_Printf("pos {%x,%x,%x}, blockID %x, gridx %x, gridz %x\n", 
							//			pos.x, pos.y, pos.z, blockID, x, z );
							return TRUE;
						}
					}
				}
				prevBlockID = blockID;
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


