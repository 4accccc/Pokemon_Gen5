//============================================================================================
/**
 * @file	field_buildmodel.c
 * @brief		配置モデルの制御
 * @author	tamada GAMEFREAK inc.
 * @date	2009.04.20
 */
//============================================================================================

#include <gflib.h>

#include "arc_def.h"
#include "field/zonedata.h"
#include "field/areadata.h"

#include "field/field_const.h"
#include "field_buildmodel.h"
#include "field_bmanime.h"

#include "field_g3d_mapper.h"		//下記ヘッダに必要

#include "system/el_scoreboard.h"

#include "savedata/save_control.h"    //テストでランダムマップの都市の種類を取得
#include "savedata/randommap_save.h"  //テストでランダムマップの都市の種類を取得

typedef EL_SCOREBOARD_TEX ELBOARD_TEX;

#include "arc/fieldmap/area_id.h"
#include "arc/fieldmap/buildmodel_info.naix"
#include "arc/fieldmap/buildmodel_outdoor.naix"
#include "arc/fieldmap/buildmodel_indoor.naix"

#include "map/dp3format.h"

//============================================================================================
//============================================================================================

enum {
  GFL_G3D_MAP_OBJST_MAX = 32,
  GFL_G3D_MAP_OBJID_NULL = 0xffffffff,
};
//------------------------------------------------------------------
//------------------------------------------------------------------
enum { GLOBAL_OBJ_ANMCOUNT	= 4 };

enum { BMODEL_USE_MAX = 4 };

enum{ 
  FILEID_BMODEL_ANIMEDATA = NARC_buildmodel_info_buildmodel_anime_bin,
  FILEID_BMINFO_OUTDOOR = NARC_buildmodel_info_buildmodel_outdoor_bin,
  FILEID_BMINFO_INDOOR = NARC_buildmodel_info_buildmodel_indoor_bin,
};

typedef enum {
  BM_ANMMODE_NOTHING = 0,
  BM_ANMMODE_ETERNAL,
  BM_ANMMODE_STOP,
  BM_ANMMODE_TEMPORARY,
}BM_ANMMODE;

#define GLOBAL_OBJ_COUNT	(64)

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  BMODEL_ID_MAX = 400,  //とりあえず
  BMODEL_ENTRY_MAX = 128,  //とりあえず

  BMODEL_ID_NG  = BMODEL_ID_MAX,
  BMODEL_ENTRY_NG = BMODEL_ENTRY_MAX,

  BMANIME_NULL_ID  = 0xffff,
  BM_SUBMODEL_NULL_ID = 0xffff,

  //電光掲示板で表示する文字列の長さ
  BMODEL_ELBOARD_STR_LEN = 64 * 2,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct _FIELD_BMANIME_DATA
{ 
  u8 anm_type;  //BMANIME_TYPE  アニメの種類指定
  u8 prg_type;  //動作プログラムの種類指定
  u8 anm_count; //アニメカウント
  u8 set_count; //

  ///アニメアーカイブ指定ID
  u16 IDs[GLOBAL_OBJ_ANMCOUNT];
}FIELD_BMANIME_DATA;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u16 anime_id;
  u16 prog_id;
  u16 submodel_id;
}BM_INFO_BIN;

typedef struct {
  BMODEL_ID bm_id;
  FIELD_BMANIME_DATA animeData;
  u16 prog_id;
  BMODEL_ID sub_bm_id;
}BMINFO;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  BMODEL_ID     bm_id;
  GFL_G3D_RES*  g3DresMdl;            //モデルリソース(High Q)
  GFL_G3D_RES*  g3DresTex;            //テクスチャリソース
  GFL_G3D_RES*  g3DresAnm[GLOBAL_OBJ_ANMCOUNT];  //アニメリソース
  const FIELD_BMANIME_DATA * anmData;
}OBJ_RES;

typedef struct {
  GFL_G3D_OBJ*  g3Dobj;              //オブジェクトハンドル
  const OBJ_RES * res;
  BM_ANMMODE anmMode[GLOBAL_OBJ_ANMCOUNT];
}OBJ_HND;

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _FIELD_BMODEL {
  OBJ_HND objHdl;
  GFL_G3D_OBJSTATUS g3dObjStatus;
};

//------------------------------------------------------------------
/**
 * GFL_G3D_MAPが保持する表示オブジェクト情報へのリンク
 */
//------------------------------------------------------------------
struct _G3DMAPOBJST{
  GFL_G3D_MAP * g3Dmap;
  u32 entryNoBackup;
  u16 index;
  u16 viewFlag;
  GFL_G3D_MAP_GLOBALOBJ_ST * objSt;
};

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _FIELD_BMODEL_MAN
{
	HEAPID heapID;

	u8 BMIDToEntryTable[BMODEL_ID_MAX];

	u16 entryCount;
  BMINFO bmInfo[BMODEL_ENTRY_MAX];
	BMODEL_ID entryToBMIDTable[BMODEL_ENTRY_MAX];

	ARCID mdl_arc_id;
  
  STRBUF * elb_str[FIELD_BMODEL_ELBOARD_ID_MAX];
  ELBOARD_TEX * elb_tex[FIELD_BMODEL_ELBOARD_ID_MAX];

	GFL_G3D_MAP_GLOBALOBJ	g3dMapObj;						//共通オブジェクト

	u32	objRes_Count;		  		//共通オブジェクトリソース数
	OBJ_RES * objRes;					//共通オブジェクトリソース

  u32 objHdl_Count;
  OBJ_HND * objHdl;

  FIELD_BMODEL * entryObj[BMODEL_USE_MAX];

  G3DMAPOBJST g3DmapObjSt[GFL_G3D_MAP_OBJST_MAX * 9];
};

//============================================================================================
//
//    デバッグ用関数
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline BMODEL_ID BMIDAssert(BMODEL_ID bm_id)
{
  if (bm_id >= BMODEL_ID_MAX)
  {
    OS_TPrintf("BuildModel ID over %d\n",bm_id );
    GF_ASSERT(0);
    bm_id = 0;
  }
  return bm_id;
}
#define BMID_ASSERT(bm_id) {bm_id = BMIDAssert(bm_id);}

//------------------------------------------------------------------
//------------------------------------------------------------------
static inline u8 EntryNoAssert(const FIELD_BMODEL_MAN * man, u8 entryNo)
{
  if (entryNo >= BMODEL_ENTRY_MAX || entryNo >= man->entryCount)
  {
    OS_TPrintf("BuildModel Entry No Over(%d)\n",entryNo);
    GF_ASSERT(0);
    entryNo = 0;
  }
  return entryNo;
}

#define ENTRYNO_ASSERT(man, entryNo) {entryNo = EntryNoAssert(man, entryNo);}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void makeBMIDToEntryTable(FIELD_BMODEL_MAN * man);
static u16 calcArcIndex(u16 area_id);

static void loadEntryToBMIDTable(FIELD_BMODEL_MAN * man, u16 arc_id, u16 file_id);

static void BMINFO_Load(FIELD_BMODEL_MAN * man, u16 file_id);
static void BMINFO_init(BMINFO * bmInfo);
static BOOL BMINFO_isDoor(const BMINFO * bmInfo);

static BMIDtoEntryNo(const FIELD_BMODEL_MAN * man, BMODEL_ID id);

//------------------------------------------------------------------
//------------------------------------------------------------------

static void createAllResource(FIELD_BMODEL_MAN * man);
static void deleteAllResource(FIELD_BMODEL_MAN * man);

static void createFullTimeObjHandle(FIELD_BMODEL_MAN * man, GFL_G3D_MAP_GLOBALOBJ * g3dMapObj);
static void deleteFullTimeObjHandle(FIELD_BMODEL_MAN * man, GFL_G3D_MAP_GLOBALOBJ * g3dMapObj);

static void OBJHND_initialize( OBJ_HND * objHdl, const OBJ_RES* objRes);
static void OBJHND_finalize( OBJ_HND * objHdl );
static void OBJHND_animate( OBJ_HND * objHdl );
static void OBJHND_setAnime( OBJ_HND * objHdl, u32 anmNo, BMANM_REQUEST req);
static BOOL OBJHND_getAnimeStatus(OBJ_HND * objHdl, u32 anmNo);

static void DEBUG_dumpBMAnimeData(const FIELD_BMANIME_DATA * data);
static u32 BMANIME_getCount(const FIELD_BMANIME_DATA * data);
static void BMANIME_init(FIELD_BMANIME_DATA * data);
//------------------------------------------------------------------
//アニメデータの取得処理
//------------------------------------------------------------------
static const FIELD_BMANIME_DATA * getLoadedAnimeData(FIELD_BMODEL_MAN * man, u16 bm_id);
//------------------------------------------------------------------
//  テクスチャ情報の登録処理
//------------------------------------------------------------------
extern void FIELD_BMANIME_DATA_entryTexData(FIELD_BMODEL_MAN* man, const FIELD_BMANIME_DATA * data,
    const GFL_G3D_RES * g3DresTex);

static G3DMAPOBJST * G3DMAPOBJST_create(
    FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST * status, int idx);
static void G3DMAPOBJST_init(FIELD_BMODEL_MAN * man, G3DMAPOBJST * obj);
static void G3DMAPOBJST_deleteByObject(FIELD_BMODEL_MAN * man, G3DMAPOBJST * obj);
static void G3DMAPOBJST_deleteByG3Dmap(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap);

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 配置モデルマネジャー生成
 * @param heapID
 */
//------------------------------------------------------------------
FIELD_BMODEL_MAN * FIELD_BMODEL_MAN_Create(HEAPID heapID)
{	
  int i;
	FIELD_BMODEL_MAN * man = GFL_HEAP_AllocMemory(heapID, sizeof(FIELD_BMODEL_MAN));
	man->heapID = heapID;

  for (i = 0; i < FIELD_BMODEL_ELBOARD_ID_MAX; i ++) 
  { 
    man->elb_str[i] = GFL_STR_CreateBuffer(BMODEL_ELBOARD_STR_LEN, heapID);
    man->elb_tex[i] = NULL;
  }

  for (i = 0; i < BMODEL_USE_MAX; i++)
  {
    man->entryObj[i] = NULL;
  }
  for (i = 0; i < NELEMS(man->g3DmapObjSt); i++)
  {
    G3DMAPOBJST_init( man, &man->g3DmapObjSt[i] );
  }

	man->g3dMapObj.gobj = NULL;
	man->g3dMapObj.gobjCount = 0;
	man->g3dMapObj.gobjIDexchange = NULL;

	man->g3dMapObj.gddobj = NULL;
	man->g3dMapObj.gddobjCount = 0;
	man->g3dMapObj.gddobjIDexchange = NULL;
	
  man->objRes_Count = 0;
  man->objRes = NULL;

  man->objHdl_Count = 0;
  man->objHdl = NULL;

	return man;
}

//------------------------------------------------------------------
/**
 * @brief 配置モデルマネジャー消去
 * @param man 配置モデルマネジャーへのポインタ
 */
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_Delete(FIELD_BMODEL_MAN * man)
{	
  int i;
  for (i = 0; i < FIELD_BMODEL_ELBOARD_ID_MAX; i ++) 
  { 
    GFL_STR_DeleteBuffer(man->elb_str[i]);
    if (man->elb_tex[i]) 
    { 
      ELBOARD_TEX_Delete(man->elb_tex[i]);
    }
  }
  deleteFullTimeObjHandle( man, &man->g3dMapObj );
  deleteAllResource( man );

	GFL_HEAP_FreeMemory(man);
}

//------------------------------------------------------------------
/**
 * @brief 配置モデルマネジャーメイン処理
 */
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_Main(FIELD_BMODEL_MAN * man)
{ 
  int i;
  for (i = 0; i < FIELD_BMODEL_ELBOARD_ID_MAX; i++)
  { 
    if (man->elb_tex[i])
    { 
      ELBOARD_TEX_Main(man->elb_tex[i]);
    }
  }

  for (i = 0; i < BMODEL_USE_MAX; i++)
  {
    if (man->entryObj[i])
    {
      OBJ_HND * objHdl = &man->entryObj[i]->objHdl;
      OBJHND_animate( objHdl );
    }
  }

  //アニメーションコントロール（暫定でフレームループさせているだけ）

  for( i=0; i<man->objHdl_Count; i++ ){
    OBJ_HND * objHdl = &man->objHdl[i];
    OBJHND_animate( objHdl );
  }

}

//------------------------------------------------------------------
/**
 * @brief   配置モデルマネジャー：描画処理
 */
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_Draw(FIELD_BMODEL_MAN * man)
{
  int i;
  for (i = 0; i < BMODEL_USE_MAX; i++)
  {
    if (man->entryObj[i])
    {
      FIELD_BMODEL_Draw(man->entryObj[i]);
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief 配置モデルデータ読み込み処理
 * @param man 配置モデルマネジャーへのポインタ
 */
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_Load(FIELD_BMODEL_MAN * man, u16 zoneid, const AREADATA * areadata)
{	
	u16 area_id = ZONEDATA_GetAreaID(zoneid);
	u16 bmlist_index = calcArcIndex(area_id);
  ARCID bmlist_arc_id;
  u16 model_info_dataid;
  int i;

	GFL_STD_MemClear(man->BMIDToEntryTable, sizeof(man->BMIDToEntryTable));
	GFL_STD_MemFill16(man->entryToBMIDTable, BMODEL_ENTRY_NG, sizeof(man->entryToBMIDTable));
  for (i = 0; i < BMODEL_ENTRY_MAX; i++)
  { 
    BMINFO_init( &man->bmInfo[i] );
  }

  if (AREADATA_GetInnerOuterSwitch(areadata) != 0) 
  {	
    man->mdl_arc_id = ARCID_BMODEL_OUTDOOR;
    model_info_dataid = FILEID_BMINFO_OUTDOOR;
    bmlist_arc_id = ARCID_BMODEL_IDX_OUTDOOR;
  }
  else
  {	
    man->mdl_arc_id = ARCID_BMODEL_INDOOR;
    model_info_dataid = FILEID_BMINFO_INDOOR;
    bmlist_arc_id = ARCID_BMODEL_IDX_INDOOR;
  }

  //エリア別配置モデルIDリストの読み込み
  loadEntryToBMIDTable(man, bmlist_arc_id, bmlist_index);

	//ID→登録順序の変換データテーブル生成
	makeBMIDToEntryTable(man);

  //必要な配置モデルデータの読み込み
  BMINFO_Load(man, model_info_dataid);

  createAllResource( man );
  createFullTimeObjHandle( man, &man->g3dMapObj );
}

//------------------------------------------------------------------
/**
 * @brief 配置モデルIDを登録済み配置モデルのインデックスに変換する
 * @param man 配置モデルマネジャーへのポインタ
 */
//------------------------------------------------------------------
u8 FIELD_BMODEL_MAN_GetEntryIndex(const FIELD_BMODEL_MAN * man, BMODEL_ID id)
{	
  return BMIDtoEntryNo( man, id );
}

static BMIDtoEntryNo(const FIELD_BMODEL_MAN * man, BMODEL_ID id)
{
	u8 entryNo;
  BMID_ASSERT( id );
	entryNo = man->BMIDToEntryTable[id];
	return entryNo;
}

//------------------------------------------------------------------
/**
 * @brief 配置モデルIDから対応するアニメデータを取得する
 * @param bm_id   配置モデルID
 */
//------------------------------------------------------------------
static const FIELD_BMANIME_DATA * getLoadedAnimeData(FIELD_BMODEL_MAN * man, u16 bm_id)
{ 
  u16 entryNo = man->BMIDToEntryTable[bm_id];
  BMID_ASSERT( bm_id );
  ENTRYNO_ASSERT( man, entryNo );
  return &man->bmInfo[entryNo].animeData;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL hasSubModel(const FIELD_BMODEL_MAN * man, u16 bm_id)
{
  u16 entryNo = BMIDtoEntryNo(man, bm_id);
  BMODEL_ID submodel_id = man->bmInfo[entryNo].sub_bm_id;
  BMID_ASSERT( submodel_id );
  return (submodel_id != BM_SUBMODEL_NULL_ID);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_BMODEL_MAN_GetSubModel(const FIELD_BMODEL_MAN * man, 
    u16 bm_id, VecFx32 * ofs, u32 * entryNo)
{
  u16 submodel_id;
  const BMINFO * bmInfo;
  if (man->mdl_arc_id != ARCID_BMODEL_OUTDOOR)
  {
    return FALSE;
  }
  bmInfo = &man->bmInfo[ BMIDtoEntryNo(man, bm_id) ];
  submodel_id = bmInfo->sub_bm_id;
  if (submodel_id == BM_SUBMODEL_NULL_ID)
  {
    return FALSE;
  }
  *entryNo = BMIDtoEntryNo(man, submodel_id);
  switch (submodel_id) {
  default:
  case NARC_output_buildmodel_outdoor_door01_nsbmd:
    VEC_Set(ofs, 0, 0, (FIELD_CONST_GRID_SIZE * 2 - 6) << FX32_SHIFT );
    break;
  case NARC_output_buildmodel_outdoor_door_pc_nsbmd:
    VEC_Set(ofs, 0, 0, (FIELD_CONST_GRID_SIZE) << FX32_SHIFT );
    break;
  }
  return TRUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_releaseBuildModel(FIELD_BMODEL_MAN * man, FIELD_BMODEL * bmodel)
{
  int i;
  for (i = 0; i < BMODEL_USE_MAX; i++)
  {
    if (man->entryObj[i] == bmodel)
    {
      man->entryObj[i] = NULL;
      return;
    }
  }
  GF_ASSERT(0);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_EntryBuildModel(FIELD_BMODEL_MAN * man, FIELD_BMODEL * bmodel)
{
  int i;
  for (i = 0; i < BMODEL_USE_MAX; i++)
  {
    if (man->entryObj[i] == NULL)
    {
      man->entryObj[i] = bmodel;
      return;
    }
  }
  GF_ASSERT(0);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GFL_G3D_MAP_GLOBALOBJ * FIELD_BMODEL_MAN_GetGlobalObjects(FIELD_BMODEL_MAN * man)
{
  return &man->g3dMapObj;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief エリア別配置モデルIDリストの読み込み
 * @param man
 * @param arc_id  エリア別配置モデルリストのアーカイブ指定ID（屋内or屋外）
 * @param file_id  エリアIDから変換したファイル指定ID
 *
 * entryCount/entryToBMIDTableを書き換える
 */
//------------------------------------------------------------------
static void loadEntryToBMIDTable(FIELD_BMODEL_MAN * man, u16 arc_id, u16 file_id)
{ 
  ARCHANDLE * handle = GFL_ARC_OpenDataHandle(arc_id, man->heapID);

  {
    u16 data_max = GFL_ARC_GetDataFileCntByHandle(handle);
    if (data_max < file_id)
    {	
      GF_ASSERT_MSG(0, "配置モデルリストデータがありません(%d<%d)\n", data_max, file_id);
      file_id = 0;		//とりあえずハングアップ回避
    }
  }

	{	
		u16 size = GFL_ARC_GetDataSizeByHandle(handle, file_id);
		man->entryCount = size / sizeof(BMODEL_ID);
		if(size > sizeof(man->entryToBMIDTable))
		{	
			GF_ASSERT_MSG(0, "配置モデルリストデータがオーバー（size=%d ARCINDEX=%d)\n", size, file_id);
			man->entryCount = BMODEL_ENTRY_MAX;	//とりあえずハングアップ回避
			size = sizeof(man->entryToBMIDTable);
		}
		//TAMADA_Printf("entryCount=%d\n", man->entryCount);
		GFL_ARC_LoadDataOfsByHandle(handle, file_id, 0, size, man->entryToBMIDTable);
	}
	
	GFL_ARC_CloseDataHandle(handle);
}

//------------------------------------------------------------------
//BMODEL_IDで引くBMIDToEntryTableにはentryToBMIDTableへのオフセットを格納
//------------------------------------------------------------------
static void makeBMIDToEntryTable(FIELD_BMODEL_MAN * man)
{
	u8 entryNo;
	for (entryNo = 0; entryNo < man->entryCount; entryNo++)
	{	
		BMODEL_ID bm_id = man->entryToBMIDTable[entryNo];
		man->BMIDToEntryTable[bm_id] = entryNo;
	}
}

//------------------------------------------------------------------
/**
 * @brief エリアIDから配置モデル情報へのインデックス取得
 * @param area_id 
 * @return  u16 情報アーカイブの指定ID
 *
 * @todo  エリアデータの並びに依存して計算しているので移行を考えること！
 */
//------------------------------------------------------------------
static u16 calcArcIndex(u16 area_id)
{	
	if (area_id >= AREA_ID_IN01)
	{	
		return area_id - AREA_ID_IN01;
	}
	if (area_id >= AREA_ID_OUT01)
	{	
		return (area_id - AREA_ID_OUT01) / 4;
	}
	return 0;	//とりあえず
}


//============================================================================================
//
//
//
//    配置モデルアニメデータ
//
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 保持しているアニメIDの数を返す
 * @param data  アニメデータへのポインタ
 * @param u32 保持しているアニメIDの数
 */
//------------------------------------------------------------------
static u32 BMANIME_getCount(const FIELD_BMANIME_DATA * data)
{ 
  u32 i,count;
  for (i = 0, count = 0; i < GLOBAL_OBJ_ANMCOUNT; i++)
  { 
    if (data->IDs[i] != BMANIME_NULL_ID)
    { 
      count ++;
    }
  }
  return count;
}
//------------------------------------------------------------------
/**
 * @brief FIELD_BMANIME_DATAの初期化
 */
//------------------------------------------------------------------
static void BMANIME_init(FIELD_BMANIME_DATA * data)
{ 
  static const FIELD_BMANIME_DATA init = {  
    BMANIME_TYPE_NONE,  //アニメ適用の種類指定
    BMANIME_PROG_TYPE_NONE,  //動作プログラムの種類指定
    0,  //アニメカウント（仮）
    0,  //セットカウント（仮）
    //アニメアーカイブ指定ID
    { BMANIME_NULL_ID, BMANIME_NULL_ID, BMANIME_NULL_ID, BMANIME_NULL_ID, }

  };
  *data = init;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
#ifdef  PM_DEBUG
static void DEBUG_dumpBMAnimeData(const FIELD_BMANIME_DATA * data)
{ 
  static const char * animetype[] ={  
    "BMANIME_TYPE_NONE",
    "BMANIME_TYPE_ETERNAL",
    "BMANIME_TYPE_EVENT",
    "BMANIME_TYPE_ERROR",
  };
  int i;
  int type = data->anm_type;
  if (type >= BMANIME_TYPE_MAX) type = 3;
  TAMADA_Printf("FIELD_BMANIME_DATA:");
  TAMADA_Printf("%s, %d\n", animetype[data->anm_type], type);
  TAMADA_Printf("%d %d %d\n",data->prg_type, data->anm_count, data->set_count);
  for (i = 0; i < GLOBAL_OBJ_ANMCOUNT; i++)
  {
    TAMADA_Printf("%04x ", data->IDs[i]);
  }
  TAMADA_Printf("\n");
}
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------
static void BMINFO_Load(FIELD_BMODEL_MAN * man, u16 file_id)
{
  ARCHANDLE * handle = GFL_ARC_OpenDataHandle(ARCID_BMODEL_INFO, man->heapID);
  BM_INFO_BIN * infobin = GFL_HEAP_AllocMemory(man->heapID, sizeof(BM_INFO_BIN));
  u8 entryNo;
  enum {
    BM_INFO_SIZE = sizeof(BM_INFO_BIN),
    BM_ANMDATA_SIZE = sizeof(FIELD_BMANIME_DATA),
  };

  for (entryNo = 0; entryNo < man->entryCount; entryNo++)
  { 
    BMINFO * bmInfo = &man->bmInfo[entryNo];
    u16 anm_id;
    bmInfo->bm_id = man->entryToBMIDTable[entryNo];
    GFL_ARC_LoadDataOfsByHandle(handle, file_id, bmInfo->bm_id * BM_INFO_SIZE, BM_INFO_SIZE, infobin);
    bmInfo->prog_id = infobin->prog_id;
    bmInfo->sub_bm_id = infobin->submodel_id;
    anm_id = infobin->anime_id;
    if (anm_id == 0xffff) continue;
    GFL_ARC_LoadDataOfsByHandle(handle, FILEID_BMODEL_ANIMEDATA,
        anm_id * BM_ANMDATA_SIZE, BM_ANMDATA_SIZE,
        &bmInfo->animeData);
    //DEBUG_dumpBMAnimeData(&man->animeData[entryNo]);
  }
  GFL_HEAP_FreeMemory(infobin);
  GFL_ARC_CloseDataHandle(handle);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void BMINFO_init(BMINFO * bmInfo)
{
  bmInfo->bm_id = 0;
  BMANIME_init(&bmInfo->animeData);
  bmInfo->prog_id = 0;
  bmInfo->sub_bm_id = BM_SUBMODEL_NULL_ID;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL BMINFO_isDoor(const BMINFO * bmInfo)
{
  return (bmInfo->prog_id == 1);
}

//============================================================================================
//
//
//    配置モデルへの電光掲示板反映
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 電光掲示板用文字列の登録処理（直接文字列オブジェクトを渡す）
 * @param man 配置モデルマネジャーへのポインタ
 * @param id  電光掲示板指定ID
 */
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_EntryELString(const FIELD_BMODEL_MAN* man,
    FIELD_BMODEL_ELBOARD_ID id,
    const STRBUF* str)
{ 
  GF_ASSERT(id < FIELD_BMODEL_ELBOARD_ID_MAX);

  GFL_STR_CopyBuffer(man->elb_str[id], str);
}

//------------------------------------------------------------------
/**
 * @brief 電光掲示板用文字列の登録処理（メッセージアーカイブ指定型）
 * @param man 配置モデルマネジャーへのポインタ
 * @param id  電光掲示板指定ID
 * @param msg_arc_id  メッセージアーカイブ指定ID
 * @param str_id  アーカイブ内メッセージ指定ID
 */
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_EntryELStringID(const FIELD_BMODEL_MAN * man,
    FIELD_BMODEL_ELBOARD_ID id,
    ARCID msg_arc_id, u16 str_id)
{ 
  GFL_MSGDATA * msgData;

  GF_ASSERT(id < FIELD_BMODEL_ELBOARD_ID_MAX);

  msgData = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msg_arc_id, man->heapID);
  GFL_MSG_GetString(msgData, str_id, man->elb_str[id]);

  GFL_MSG_Delete(msgData);
}
//------------------------------------------------------------------
/**
 * @brief   テクスチャの登録処理
 * @param man 配置モデルマネジャーへのポインタ
 * @param data  アニメデータへのポインタ
 * @param g3DresTex   登録するテクスチャリソース
 *
 * 現在、電光掲示板のためだけに呼び出している。
 * 電光掲示板以外の配置モデルではなにもせずに帰る
 */
//------------------------------------------------------------------
void FIELD_BMANIME_DATA_entryTexData(FIELD_BMODEL_MAN* man, const FIELD_BMANIME_DATA * data,
    const GFL_G3D_RES * g3DresTex)
{ 
  switch ((BMANIME_PROG_TYPE)data->prg_type)
  { 
  case BMANIME_PROG_TYPE_ELBOARD1:
    man->elb_tex[FIELD_BMODEL_ELBOARD_ID1] = ELBOARD_TEX_Add(
        g3DresTex, man->elb_str[FIELD_BMODEL_ELBOARD_ID1], man->heapID);
    break;
  case BMANIME_PROG_TYPE_ELBOARD2:
    man->elb_tex[FIELD_BMODEL_ELBOARD_ID2] = ELBOARD_TEX_Add(
        g3DresTex, man->elb_str[FIELD_BMODEL_ELBOARD_ID1], man->heapID);
    break;
  case BMANIME_PROG_TYPE_NONE:
  default:
    //何もしない
    break;
  }
}

//============================================================================================
/**
 *
 *
 *
 * @brief	３Ｄグローバルオブジェクト読み込み
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	u32 arcID;
	u32	datID;
	u32	inDatNum;
}MAKE_RES_PARAM;

typedef struct {
	MAKE_RES_PARAM	mdl;
	MAKE_RES_PARAM	tex;
	MAKE_RES_PARAM	anm[GLOBAL_OBJ_ANMCOUNT];
}MAKE_OBJ_PARAM;

enum {  MAKE_RES_NONPARAM = (0xffffffff) };

//------------------------------------------------------------------
//------------------------------------------------------------------
static void MAKE_RES_PARAM_init(MAKE_RES_PARAM * resParam)
{
  resParam->arcID = MAKE_RES_NONPARAM;
  resParam->datID = MAKE_RES_NONPARAM;
  resParam->inDatNum = 0;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void MAKE_RES_PARAM_set(MAKE_RES_PARAM * resParam, u32 arcID, u32 datID, u32 inDatNum)
{
  resParam->arcID = arcID;
  resParam->datID = datID;
  resParam->inDatNum = inDatNum;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void MAKE_OBJ_PARAM_init(MAKE_OBJ_PARAM * objParam)
{ 
  int i;
  MAKE_RES_PARAM_init( &objParam->mdl );
  MAKE_RES_PARAM_init( &objParam->tex );

  for( i=0; i<GLOBAL_OBJ_ANMCOUNT; i++ ){
    MAKE_RES_PARAM_init( &objParam->anm[i] );
  }

}




static void createResource( OBJ_RES * objRes, const MAKE_OBJ_PARAM * param);
static void deleteResource( OBJ_RES * objRes );
//------------------------------------------------------------------
//配置モデルマネジャーからの内容で生成
//------------------------------------------------------------------
static void createAllResource(FIELD_BMODEL_MAN * man)
{ 
  u32 entryCount = man->entryCount;

	if( entryCount == 0 ){
    return;
  }

  man->objRes_Count = entryCount;
  man->objRes = GFL_HEAP_AllocClearMemory ( man->heapID, sizeof(OBJ_RES) * entryCount );

  /** 配置モデルリソース登録 */
  {
    u8 entryNo;
    MAKE_OBJ_PARAM objParam;
    MAKE_OBJ_PARAM_init(&objParam);

    for ( entryNo = 0; entryNo < man->objRes_Count; entryNo++ )
    {
      int j, count;
      const u16 * anmIDs;
      const FIELD_BMANIME_DATA * anmData;
      OBJ_RES * objRes = &man->objRes[entryNo];
      BMODEL_ID bm_id = man->entryToBMIDTable[entryNo];

      MAKE_RES_PARAM_set(&objParam.mdl, man->mdl_arc_id, bm_id, 0);
      
      //配置モデルに対応したアニメデータを取得 
      anmData = getLoadedAnimeData(man, bm_id);
      count = BMANIME_getCount(anmData);
      anmIDs = anmData->IDs;

      for( j=0; j<GLOBAL_OBJ_ANMCOUNT; j++ )
      {
        if( j<count ) 
        { 
          MAKE_RES_PARAM_set(&objParam.anm[j], ARCID_BMODEL_ANIME, anmIDs[j], 0);
        } else {
          MAKE_RES_PARAM_init( &objParam.anm[j] );
        }
      }

      //TAMADA_Printf("BM:Create Rsc %d (%d)\n", entryNo, bm_id);
      createResource( objRes, &objParam );
      objRes->anmData = anmData;
      {
        GFL_G3D_RES* resTex = objRes->g3DresTex ? objRes->g3DresTex : objRes->g3DresMdl;
        FIELD_BMANIME_DATA_entryTexData( man, anmData, resTex );
      }
    }
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
  /** 配置モデルG3D_OBJECT登録 */
static void createFullTimeObjHandle(FIELD_BMODEL_MAN * man, GFL_G3D_MAP_GLOBALOBJ * g3dMapObj)
{
  int i;

  u32 entryCount = man->entryCount;

  if( entryCount == 0 ){
    return;
  }

  g3dMapObj->gobjCount = entryCount;
  g3dMapObj->gobj = GFL_HEAP_AllocClearMemory( man->heapID, sizeof(GFL_G3D_MAP_OBJ) * entryCount );

  man->objHdl_Count = entryCount;
  man->objHdl = GFL_HEAP_AllocClearMemory( man->heapID, sizeof(OBJ_HND) * entryCount );

  for ( i = 0; i < man->objHdl_Count; i++ )
  {
    //const FIELD_BMANIME_DATA * anmData;
    OBJ_HND * objHdl = &man->objHdl[i];
    OBJ_RES * objRes = &man->objRes[i];
    //BMODEL_ID bm_id = man->entryToBMIDTable[i];
    OBJHND_initialize( objHdl, objRes );

    //anmData = getLoadedAnimeData(man, bm_id);
    g3dMapObj->gobj[i].g3DobjHQ = objHdl->g3Dobj;
    g3dMapObj->gobj[i].g3DobjLQ = NULL;
  }
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static void deleteFullTimeObjHandle(FIELD_BMODEL_MAN * man, GFL_G3D_MAP_GLOBALOBJ * g3dMapObj)
{
	if( g3dMapObj->gobj != NULL ){
		GFL_HEAP_FreeMemory( g3dMapObj->gobj );
		g3dMapObj->gobj = NULL;
	}
	if( g3dMapObj->gobjIDexchange != NULL ){
		GFL_HEAP_FreeMemory( g3dMapObj->gobjIDexchange );
		g3dMapObj->gobjIDexchange = NULL;
	}

  if( man->objHdl != NULL ){
    int i;
    for( i=0; i<man->objHdl_Count; i++ ){
			OBJHND_finalize( &man->objHdl[i] );
    }
    GFL_HEAP_FreeMemory( man->objHdl );
    man->objHdl_Count = 0;
    man->objHdl = NULL;
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void deleteAllResource(FIELD_BMODEL_MAN * man)
{
	if( man->objRes != NULL ){
		int i;

		for( i=0; i<man->objRes_Count; i++ ){
      deleteResource( &man->objRes[i] );
		}
		GFL_HEAP_FreeMemory( man->objRes );
		man->objRes_Count = 0;
		man->objRes = NULL;
	}
}

//------------------------------------------------------------------
// オブジェクトリソースを作成
//------------------------------------------------------------------
static void createResource( OBJ_RES * objRes, const MAKE_OBJ_PARAM * param)
{
	GFL_G3D_RES* resTex;
	int i;

  //モデルデータ生成
	objRes->g3DresMdl = GFL_G3D_CreateResourceArc( param->mdl.arcID, param->mdl.datID );

  //テクスチャ登録
	if( param->tex.arcID != MAKE_RES_NONPARAM ){
		objRes->g3DresTex = GFL_G3D_CreateResourceArc( param->tex.arcID, param->tex.datID );
		resTex = objRes->g3DresTex;
	} else {
		objRes->g3DresTex = NULL;
		resTex = objRes->g3DresMdl;
	}
  if (GFL_G3D_CheckResourceType( resTex, GFL_G3D_RES_CHKTYPE_TEX ) == TRUE)
  {
	  DEBUG_Field_Grayscale(resTex);
	  GFL_G3D_TransVramTexture( resTex );
  } else {
    const MAKE_RES_PARAM * prm;
    if (param->tex.arcID != MAKE_RES_NONPARAM)
    {
      prm = &param->tex;
    } else {
      prm = &param->mdl;
    }
    OS_Printf("配置モデルにテクスチャが含まれていません！(arcID=%d, mdlID=%d)\n",prm->arcID,prm->datID);
    GF_ASSERT(0);
  }
	for( i=0; i<GLOBAL_OBJ_ANMCOUNT; i++ ){
		if( param->anm[i].arcID != MAKE_RES_NONPARAM ){
			objRes->g3DresAnm[i] = GFL_G3D_CreateResourceArc( param->anm[i].arcID, param->anm[i].datID );
    }
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void deleteResource( OBJ_RES * objRes )
{
  int i;
	GFL_G3D_RES*	resTex;

  for( i=0; i<GLOBAL_OBJ_ANMCOUNT; i++ ){
    if( objRes->g3DresAnm[i] != NULL ){
      GFL_G3D_DeleteResource( objRes->g3DresAnm[i] );
      objRes->g3DresAnm[i] = NULL;
    }
  }
  if( objRes->g3DresTex == NULL ){
    resTex = objRes->g3DresMdl;
  } else {
    resTex = objRes->g3DresTex;
    GFL_G3D_DeleteResource( objRes->g3DresTex );
    objRes->g3DresTex = NULL;
  }
  GFL_G3D_FreeVramTexture( resTex );

  if( objRes->g3DresMdl != NULL ){
    GFL_G3D_DeleteResource( objRes->g3DresMdl );
    objRes->g3DresMdl = NULL;
  }
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static void OBJHND_initialize( OBJ_HND * objHdl, const OBJ_RES* objRes)
{
  GFL_G3D_RND* g3Drnd;
  GFL_G3D_RES* resTex;
  GFL_G3D_ANM* anmTbl[GLOBAL_OBJ_ANMCOUNT];
  int i;

  resTex = objRes->g3DresTex != NULL ? objRes->g3DresTex : objRes->g3DresMdl;

  //レンダー生成
  g3Drnd = GFL_G3D_RENDER_Create( objRes->g3DresMdl, 0, resTex );

  //アニメオブジェクト生成
  for( i=0; i<GLOBAL_OBJ_ANMCOUNT; i++ ){
    objHdl->anmMode[i] = BM_ANMMODE_NOTHING;
    if( objRes->g3DresAnm[i] != NULL ){
      anmTbl[i] = GFL_G3D_ANIME_Create( g3Drnd, objRes->g3DresAnm[i], 0 );  
    } else {
      anmTbl[i] = NULL;
    }
  }
  //GFL_G3D_OBJECT生成
  objHdl->g3Dobj = GFL_G3D_OBJECT_Create( g3Drnd, anmTbl, GLOBAL_OBJ_ANMCOUNT );
  objHdl->res = objRes;

  if (objRes->anmData->anm_type == BMANIME_TYPE_ETERNAL)
  {
    int anmNo;
    for( anmNo=0; anmNo<GLOBAL_OBJ_ANMCOUNT; anmNo++ ){
      GFL_G3D_OBJECT_EnableAnime( objHdl->g3Dobj, anmNo );  //renderとanimeの関連付け
      GFL_G3D_OBJECT_ResetAnimeFrame( objHdl->g3Dobj, anmNo ); 
      objHdl->anmMode[anmNo] = BM_ANMMODE_ETERNAL;
    }
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void OBJHND_finalize( OBJ_HND * objHdl )
{
  GFL_G3D_RND*  g3Drnd;
  GFL_G3D_ANM*  g3Danm[GLOBAL_OBJ_ANMCOUNT] = { NULL, NULL, NULL, NULL };
  u16        g3DanmCount;
  int i;

  if( objHdl->g3Dobj != NULL ){
    //各種ハンドル取得
    g3DanmCount = GFL_G3D_OBJECT_GetAnimeCount( objHdl->g3Dobj );
    for( i=0; i<g3DanmCount; i++ ){
      g3Danm[i] = GFL_G3D_OBJECT_GetG3Danm( objHdl->g3Dobj, i ); 
      if (g3Danm[i] != NULL) {
        GFL_G3D_ANIME_Delete( g3Danm[i] ); 
      }
    }
    g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( objHdl->g3Dobj );
  
    //各種ハンドル＆リソース削除
    GFL_G3D_OBJECT_Delete( objHdl->g3Dobj );
    objHdl->g3Dobj = NULL;
    objHdl->res = NULL;

    GFL_G3D_RENDER_Delete( g3Drnd );
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void OBJHND_animate( OBJ_HND * objHdl )
{
  int anmNo;

  if( objHdl->g3Dobj == NULL ){
    return;
  }

  for( anmNo=0; anmNo<GLOBAL_OBJ_ANMCOUNT; anmNo++ ){
    BOOL result;
    switch (objHdl->anmMode[anmNo])
    {
    case BM_ANMMODE_NOTHING:
    case BM_ANMMODE_STOP:
      break;
    case BM_ANMMODE_TEMPORARY:
      result = GFL_G3D_OBJECT_IncAnimeFrame( objHdl->g3Dobj, anmNo, FX32_ONE );
      if (!result) {
        objHdl->anmMode[anmNo] = BM_ANMMODE_STOP;
      }
      break;
    case BM_ANMMODE_ETERNAL:
      GFL_G3D_OBJECT_LoopAnimeFrame( objHdl->g3Dobj, anmNo, FX32_ONE ); 
      break;
    }
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void disableAllAnime(OBJ_HND * objHdl)
{
  int idx;
  for (idx = 0; idx < GLOBAL_OBJ_ANMCOUNT; idx++)
  {
    if (objHdl->anmMode[idx] != BM_ANMMODE_NOTHING) {
      GFL_G3D_OBJECT_DisableAnime( objHdl->g3Dobj, idx );
      objHdl->anmMode[idx] = BM_ANMMODE_NOTHING;
    }
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void OBJHND_setAnime( OBJ_HND * objHdl, u32 anmNo, BMANM_REQUEST req)
{
  if (anmNo >= GLOBAL_OBJ_ANMCOUNT)
  {
    GF_ASSERT(0);
    anmNo = 0;
  }
  switch ((BMANM_REQUEST)req) {
  case BMANM_REQ_START:
    disableAllAnime( objHdl );
    GFL_G3D_OBJECT_EnableAnime(objHdl->g3Dobj, anmNo );
    GFL_G3D_OBJECT_ResetAnimeFrame(objHdl->g3Dobj, anmNo );
    objHdl->anmMode[anmNo] = BM_ANMMODE_TEMPORARY;
    break;
  case BMANM_REQ_STOP:
    if (objHdl->anmMode[anmNo] != BM_ANMMODE_NOTHING) {
      objHdl->anmMode[anmNo] = BM_ANMMODE_STOP;
    }
    break;
  case BMANM_REQ_END:
    disableAllAnime( objHdl );
    break;
  default:
    GF_ASSERT(0); //未定義リクエスト
    break;
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL OBJHND_getAnimeStatus(OBJ_HND * objHdl, u32 anmNo)
{
  GF_ASSERT(anmNo < GLOBAL_OBJ_ANMCOUNT);
  switch (objHdl->anmMode[anmNo]) {
  case BM_ANMMODE_NOTHING:
    return TRUE;
  case BM_ANMMODE_ETERNAL:
    return FALSE;
  case BM_ANMMODE_STOP:
    return TRUE;
  case BM_ANMMODE_TEMPORARY:
    return FALSE;
  }
  return FALSE;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_ResistAllMapObjects
(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap, const PositionSt* objStatus, u32 objCount)
{
  GFL_G3D_MAP_GLOBALOBJ_ST status;
  int i, j, count;
  BOOL hasSubModel;

  for( i=0, j = 0, count = objCount; i<count ; j++, i++ )
  {
    if (j >= GFL_G3D_MAP_OBJST_MAX)
    {
      OS_Printf("マップブロック内の配置モデル数が%dを超えているため表示できません\n",
          GFL_G3D_MAP_OBJST_MAX);
      GF_ASSERT(0);
    }

    hasSubModel = FIELD_BMODEL_MAN_GetSubModel(man,
          objStatus[i].resourceID, &status.trans, &status.id);

    FIELD_BMODEL_MAN_ResistMapObject(man, g3Dmap, &objStatus[i], j);

    if (hasSubModel == TRUE)
    {
      TAMADA_Printf("Resist Sub Model:index(%d) model id(%d)\n", i, status.id);
      j++;
      status.rotate = objStatus[i].rotate;
      status.trans.x += objStatus[i].xpos;
      status.trans.y += objStatus[i].ypos;
      status.trans.z -= objStatus[i].zpos;
      G3DMAPOBJST_create(man, g3Dmap, &status, j);
      //GFL_G3D_MAP_ResistGlobalObj( g3Dmap, &status, j );
    }

  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_ReleaseAllMapObjects
(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap)
{
  G3DMAPOBJST_deleteByG3Dmap( man, g3Dmap );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_ResistMapObject
(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap, const PositionSt* objStatus, u32 objCount)
{
  GFL_G3D_MAP_GLOBALOBJ_ST status;
  status.id = BMIDtoEntryNo(man, objStatus->resourceID);
  VEC_Set(&status.trans, objStatus->xpos, objStatus->ypos, -objStatus->zpos);
  status.rotate = (u16)(objStatus->rotate);
  G3DMAPOBJST_create(man, g3Dmap, &status, objCount);
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static inline BOOL G3DMAPOBJST_exists(const G3DMAPOBJST * obj)
{
  return (obj->g3Dmap != NULL);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void G3DMAPOBJST_init(FIELD_BMODEL_MAN * man, G3DMAPOBJST * obj)
{
  obj->g3Dmap = NULL;
  obj->entryNoBackup = GFL_G3D_MAP_OBJID_NULL;
  obj->index = 0;
  obj->viewFlag = FALSE;
  obj->objSt = NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static G3DMAPOBJST * G3DMAPOBJST_create(
    FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap, GFL_G3D_MAP_GLOBALOBJ_ST * status, int idx)
{
  int i;
  for (i = 0; i < NELEMS(man->g3DmapObjSt); i++)
  {
    G3DMAPOBJST * obj = &man->g3DmapObjSt[i];
    if ( G3DMAPOBJST_exists(obj) == FALSE )
    {
      GFL_G3D_MAP_ResistGlobalObj( g3Dmap, status, idx);
      obj->g3Dmap = g3Dmap;
      obj->entryNoBackup = GFL_G3D_MAP_OBJID_NULL;
      obj->index = idx;
      obj->viewFlag = TRUE;
      obj->objSt = GFL_G3D_MAP_GetGlobalObj(g3Dmap, idx);

      return obj;
    }
  }
  GF_ASSERT(0);
  return NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void G3DMAPOBJST_deleteByG3Dmap(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap)
{
  int i, max = NELEMS(man->g3DmapObjSt);
  for (i = 0; i < max; i++)
  {
    G3DMAPOBJST * obj = &man->g3DmapObjSt[i];
    if (obj->g3Dmap == g3Dmap)
    {
      G3DMAPOBJST_deleteByObject(man, obj);
    }
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void G3DMAPOBJST_deleteByObject(FIELD_BMODEL_MAN * man, G3DMAPOBJST * obj)
{
  GF_ASSERT( obj->objSt == GFL_G3D_MAP_GetGlobalObj(obj->g3Dmap, obj->index) );
  GFL_G3D_MAP_ReleaseGlobalObj( obj->g3Dmap, obj->index );
  G3DMAPOBJST_init( man, obj );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
G3DMAPOBJST ** FIELD_BMODEL_MAN_CreateObjStatusList
( FIELD_BMODEL_MAN* man, const FLDHIT_RECT * rect, u32 * num )
{
  G3DMAPOBJST ** result;
  int objNo, count = 0;
  result = GFL_HEAP_AllocClearMemory(man->heapID, sizeof(G3DMAPOBJST*) * GFL_G3D_MAP_OBJST_MAX);

  for (objNo = 0; objNo < NELEMS(man->g3DmapObjSt); objNo ++)
  {
    VecFx32 pos;
    G3DMAPOBJST * obj = &man->g3DmapObjSt[objNo];
    if ( G3DMAPOBJST_exists(obj) == FALSE ) continue;
    GFL_G3D_MAP_GetTrans( obj->g3Dmap, &pos );
    VEC_Add( &obj->objSt->trans, &pos, &pos );
    if (rect->top <= pos.z && pos.z <= rect->bottom && rect->left <= pos.x && pos.x <= rect->right)
    {
      result[count] = obj;
      count ++;
      TAMADA_Printf("BLOCK:%x IDX:%02d POS(%d, %d)\n",
          obj->g3Dmap, obj->index, FX_Whole(pos.x), FX_Whole(pos.z) );
      TAMADA_Printf("Search Hit.\n");
    }
  }
  *num = count;
  return result;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_BMODEL_MAN_G3DMAPOBJSTisDoor(const FIELD_BMODEL_MAN * man, const G3DMAPOBJST * obj)
{
  u8 entryNo;
  GF_ASSERT( G3DMAPOBJST_exists(obj) );
  entryNo = obj->objSt->id;
  ENTRYNO_ASSERT( man, entryNo );
  return BMINFO_isDoor(&man->bmInfo[entryNo]);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void G3DMAPOBJST_changeViewFlag(G3DMAPOBJST * obj, BOOL flag)
{
  GF_ASSERT(obj->viewFlag == !flag);
  if (flag)
  {
    obj->objSt->id = obj->entryNoBackup;
    obj->entryNoBackup = GFL_G3D_MAP_OBJID_NULL;
    obj->viewFlag = TRUE;
  }
  else
  {
    obj->entryNoBackup = obj->objSt->id;
    obj->objSt->id = GFL_G3D_MAP_OBJID_NULL;
    obj->viewFlag = FALSE;
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_BMODEL_MAN_IsDoor(const FIELD_BMODEL_MAN * man, const GFL_G3D_MAP_GLOBALOBJ_ST * status)
{
  u8 entryNo = status->id;
  ENTRYNO_ASSERT( man, entryNo );
  return BMINFO_isDoor(&man->bmInfo[entryNo]);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void * FIELD_BMODEL_MAN_GetObjHandle(FIELD_BMODEL_MAN * man, const GFL_G3D_MAP_GLOBALOBJ_ST * status)
{
  u8 entryNo = status->id;
  ENTRYNO_ASSERT( man, entryNo );
  return &man->objHdl[entryNo];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_SetAnime(FIELD_BMODEL_MAN * man, void * handle, u32 idx, BMANM_REQUEST req)
{
  OBJ_HND * objHdl = (OBJ_HND *)handle;
  OBJHND_setAnime( objHdl, idx, req );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_BMODEL_MAN_GetAnimeStatus(FIELD_BMODEL_MAN *man, void * handle, u32 idx)
{
  OBJ_HND * objHdl = (OBJ_HND *)handle;
  return OBJHND_getAnimeStatus( objHdl, idx );
}




//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void * FIELD_BMODEL_GetObjHandle( FIELD_BMODEL * bmodel )
{
  return &bmodel->objHdl;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_Draw( const FIELD_BMODEL * bmodel )
{
  GFL_G3D_DRAW_DrawObject( bmodel->objHdl.g3Dobj, &bmodel->g3dObjStatus );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
FIELD_BMODEL * FIELD_BMODEL_Create(FIELD_BMODEL_MAN * man, const G3DMAPOBJST * obj)
{
  const OBJ_RES * objRes;
  const GFL_G3D_MAP_GLOBALOBJ_ST * status = obj->objSt;

  FIELD_BMODEL * bmodel = GFL_HEAP_AllocMemory( man->heapID, sizeof(FIELD_BMODEL) );
  //ローテーション設定
  {
    fx32 sin = FX_SinIdx(status->rotate);
    fx32 cos = FX_CosIdx(status->rotate);
    MTX_RotY33( &bmodel->g3dObjStatus.rotate, sin, cos );
  }
  //位置設定：g3Dmapに依存せず表示するため、ここで座標加算しておく
  GFL_G3D_MAP_GetTrans( obj->g3Dmap, &bmodel->g3dObjStatus.trans );
  VEC_Add( &bmodel->g3dObjStatus.trans, &status->trans, &bmodel->g3dObjStatus.trans );
  
  //スケール設定
  VEC_Set( &bmodel->g3dObjStatus.scale, FX32_ONE, FX32_ONE, FX32_ONE );

  GF_ASSERT( status->id < man->objRes_Count );
  objRes = &man->objRes[status->id];
  OBJHND_initialize( &bmodel->objHdl, objRes );
  return bmodel;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_Delete(FIELD_BMODEL * bmodel)
{
  OBJHND_finalize( &bmodel->objHdl );
  GFL_HEAP_FreeMemory(bmodel);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_SetAnime(FIELD_BMODEL * bmodel, u32 idx, BMANM_REQUEST req)
{
  OBJHND_setAnime(&bmodel->objHdl, idx, req);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_BMODEL_GetAnimeStatus(FIELD_BMODEL * bmodel, u32 idx)
{
  return OBJHND_getAnimeStatus( &bmodel->objHdl, idx );
}

