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

#include "field_buildmodel.h"
#include "field_bmanime.h"

#include "field_g3d_mapper.h"		//下記ヘッダに必要
#include "fieldmap_resist.h"		//FLDMAPPER_RESISTDATA_OBJTBLなど

#include "system/el_scoreboard.h"

#include "arc/fieldmap/area_id.h"
#include "arc/fieldmap/buildmodel_info.naix"

#include "savedata/save_control.h"    //テストでランダムマップの都市の種類を取得
#include "savedata/randommap_save.h"  //テストでランダムマップの都市の種類を取得

typedef EL_SCOREBOARD_TEX ELBOARD_TEX;

#include "../resource/fldmapdata/build_model/buildmodel_outdoor.naix"
#include "../resource/fldmapdata/build_model/buildmodel_indoor.naix"

#include "map/dp3format.h"
//============================================================================================
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
enum { GLOBAL_OBJ_ANMCOUNT	= 4 };

enum {  

  BMODEL_USE_MAX = 4, 
};

enum{ 
  FILEID_BMODEL_ANIMEDATA = NARC_buildmodel_info_buildmodel_anime_bin,
};

#define GLOBAL_OBJ_COUNT	(64)

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
	BMODEL_ID_MAX = 400,	//とりあえず
	BMODEL_ENTRY_MAX = 128,	//とりあえず

	BMODEL_ID_NG	= BMODEL_ID_MAX,
	BMODEL_ENTRY_NG = BMODEL_ENTRY_MAX,

  BMANIME_NULL_ID  = 0xffff,

  //電光掲示板で表示する文字列の長さ
  BMODEL_ELBOARD_STR_LEN = 64 * 2,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _FIELD_BMANIME_DATA
{ 
  u8 anm_type;  //BMANIME_TYPE  アニメの種類指定
  u8 prg_type;  //動作プログラムの種類指定
  u8 anm_count; //アニメカウント
  u8 set_count; //

  ///アニメアーカイブ指定ID
  u16 anm_id[GLOBAL_OBJ_ANMCOUNT];

};
//------------------------------------------------------------------
//------------------------------------------------------------------
struct _FIELD_BMODEL {
  GFL_G3D_OBJ * gfl_obj;
  BOOL suicide_flag;
  GFL_G3D_OBJSTATUS status;
};


//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
  u16 anime_id;
  u16 prog_id;
  u16 submodel_id;
}BM_INFO;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GFL_G3D_RES*	g3DresMdl;						//モデルリソース(High Q)
	GFL_G3D_RES*	g3DresTex;						//テクスチャリソース
	GFL_G3D_RES*	g3DresAnm[GLOBAL_OBJ_ANMCOUNT];	//アニメリソース
}GLOBAL_RESOURCE;

typedef struct {
	GFL_G3D_OBJ*	g3Dobj;							//オブジェクトハンドル
  const GLOBAL_RESOURCE * res;
}GLOBAL_OBJ;
//------------------------------------------------------------------
//------------------------------------------------------------------
struct _FIELD_BMODEL_MAN
{
	HEAPID heapID;

	u8 IDToEntryTable[BMODEL_ID_MAX];

	u16 entryCount;
	BMODEL_ID entryToIDTable[BMODEL_ENTRY_MAX];
  FIELD_BMANIME_DATA animeData[BMODEL_ENTRY_MAX];
  u16 prog_id[BMODEL_ENTRY_MAX];
  BMODEL_ID subModels[BMODEL_ENTRY_MAX];

	ARCID model_arcid;

  //FLDMAPPERに引き渡すための生成データ保持ワーク
	FLDMAPPER_RESISTDATA_OBJTBL	gobjData_Tbl;
	FLDMAPPER_RESISTOBJDATA resistObjTbl[BMODEL_ENTRY_MAX * 2];
  
  STRBUF * elb_str[FIELD_BMODEL_ELBOARD_ID_MAX];
  ELBOARD_TEX * elb_tex[FIELD_BMODEL_ELBOARD_ID_MAX];

  FIELD_BMODEL * entryObj[BMODEL_USE_MAX];


	GFL_G3D_MAP_GLOBALOBJ	globalObj;						//共通オブジェクト

	u32						gResCount;				//共通オブジェクト数
	GLOBAL_RESOURCE*			gResource;					//共通オブジェクト
  u32 gObjCount;
  GLOBAL_OBJ * gObject;
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void makeIDToEntryTable(FIELD_BMODEL_MAN * man);
static void makeResistObjTable(FIELD_BMODEL_MAN * man);
static u16 calcArcIndex(u16 area_id);

static void loadBModelIDList(FIELD_BMODEL_MAN * man, u16 arc_id, u16 file_id);

static void loadBmInfo(FIELD_BMODEL_MAN * man, u16 file_id);
//------------------------------------------------------------------
//------------------------------------------------------------------
static void FIELD_BMANIME_DATA_init(FIELD_BMANIME_DATA * data);

static void CreateGlobalObj_forBModel(FIELD_BMODEL_MAN * man);
static void DeleteGlobalObject(FIELD_BMODEL_MAN * man, GFL_G3D_MAP_GLOBALOBJ * globalObj);

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

	man->globalObj.gobj = NULL;
	man->globalObj.gobjCount = 0;
	man->globalObj.gobjIDexchange = NULL;

	man->globalObj.gddobj = NULL;
	man->globalObj.gddobjCount = 0;
	man->globalObj.gddobjIDexchange = NULL;
	
  man->gResCount = 0;
  man->gResource = NULL;

  man->gObjCount = 0;
  man->gObject = NULL;

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
  DeleteGlobalObject(man, &man->globalObj);

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
      FIELD_BMODEL_RunAnime(man->entryObj[i]);
    }
  }

	//アニメーションコントロール（暫定でフレームループさせているだけ）
	{
    GLOBAL_OBJ * gObj;
		int j;

		for( i=0; i<man->gObjCount; i++ ){
      gObj = &man->gObject[i];

			if( gObj->g3Dobj != NULL ){
				for( j=0; j<GLOBAL_OBJ_ANMCOUNT; j++ ){
					GFL_G3D_OBJECT_LoopAnimeFrame( gObj->g3Dobj, j, FX32_ONE ); 
				}
			}
		}
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

	GFL_STD_MemClear(man->IDToEntryTable, sizeof(man->IDToEntryTable));
	GFL_STD_MemFill16(man->entryToIDTable, BMODEL_ENTRY_NG, sizeof(man->entryToIDTable));
  for (i = 0; i < BMODEL_ENTRY_MAX; i++)
  { 
    FIELD_BMANIME_DATA_init(&man->animeData[i]);
    man->subModels[i] = 0xffff;
    man->prog_id[i] = 0;
  }

  if (AREADATA_GetInnerOuterSwitch(areadata) != 0) 
  {	
    man->model_arcid = ARCID_BMODEL_OUTDOOR;
    model_info_dataid = NARC_buildmodel_info_buildmodel_outdoor_bin;
    bmlist_arc_id = ARCID_BMODEL_IDX_OUTDOOR;
  }
  else
  {	
    man->model_arcid = ARCID_BMODEL_INDOOR;
    model_info_dataid = NARC_buildmodel_info_buildmodel_indoor_bin;
    bmlist_arc_id = ARCID_BMODEL_IDX_INDOOR;
  }

  //エリア別配置モデルIDリストの読み込み
  loadBModelIDList(man, bmlist_arc_id, bmlist_index);

	//ID→登録順序の変換データテーブル生成
	makeIDToEntryTable(man);

  //必要な配置モデルデータの読み込み
  loadBmInfo(man, model_info_dataid);

  //FLDMAPPERに引き渡すデータを生成
	makeResistObjTable(man);

  CreateGlobalObj_forBModel(man);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
#if 0
u16 FIELD_BMODEL_MAN_GetNarcIndex(const FIELD_BMODEL_MAN * man, BMODEL_ID id)
{
	int index;
	GF_ASSERT(id < BMODEL_ID_MAX);
	index = man->IDToEntryTable[id];
	GF_ASSERT(index < BMODEL_ENTRY_MAX);
	return man->entryToIDTable[index];
}
#endif

//------------------------------------------------------------------
/**
 * @brief 配置モデルIDを登録済み配置モデルのインデックスに変換する
 * @param man 配置モデルマネジャーへのポインタ
 */
//------------------------------------------------------------------
u16 FIELD_BMODEL_MAN_GetEntryIndex(const FIELD_BMODEL_MAN * man, BMODEL_ID id)
{	
	u16 entry;
	GF_ASSERT(id < BMODEL_ID_MAX);
	entry = man->IDToEntryTable[id];
	//TAMADA_Printf("bmodel: id(%d) -->entry_index(%d)\n",id, entry);
	return entry;
}

//------------------------------------------------------------------
/**
 * @brief 配置モデルIDから対応するアニメデータを取得する
 * @param bm_id   配置モデルID
 */
//------------------------------------------------------------------
const FIELD_BMANIME_DATA * FIELD_BMODEL_MAN_GetAnimeData(FIELD_BMODEL_MAN * man, u16 bm_id)
{ 
  u16 entry_id = man->IDToEntryTable[bm_id];
  GF_ASSERT(bm_id < BMODEL_ID_MAX);
  GF_ASSERT(entry_id < man->entryCount);
  return &man->animeData[entry_id];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_BMODEL_MAN_GetSubModel(const FIELD_BMODEL_MAN * man, 
    u16 bm_id, VecFx32 * ofs, u32 * entry_idx)
{
  u16 submodel_id;
#if 1
  //とりあえず、ポケセン以外の扉は出さない
  if (bm_id != NARC_buildmodel_outdoor_pc_01_nsbmd
    || man->model_arcid != ARCID_BMODEL_OUTDOOR )
  {
    return FALSE;
  }
#endif
  
  if (man->model_arcid != ARCID_BMODEL_OUTDOOR)
  {
    return FALSE;
  }
  submodel_id = man->subModels[FIELD_BMODEL_MAN_GetEntryIndex(man, bm_id)];
  if (submodel_id == 0xffff)
  {
    return FALSE;
  }
  *entry_idx = FIELD_BMODEL_MAN_GetEntryIndex(man, submodel_id);
  VEC_Set(ofs, 0, 0, 0);
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
  return &man->globalObj;
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
 * entryCount/entryToIDTableを書き換える
 */
//------------------------------------------------------------------
static void loadBModelIDList(FIELD_BMODEL_MAN * man, u16 arc_id, u16 file_id)
{ 
  ARCHANDLE * hdl = GFL_ARC_OpenDataHandle(arc_id, man->heapID);
  u16 data_count = GFL_ARC_GetDataFileCntByHandle(hdl);

  //TAMADA_Printf("bmodel list id = %d\n",file_id);
  if (data_count < file_id)
  {	
    GF_ASSERT_MSG(0, "配置モデルリストデータがありません(%d<%d)\n", data_count, file_id);
    file_id = 0;		//とりあえずハングアップ回避
  }

	{	
		u16 size = GFL_ARC_GetDataSizeByHandle(hdl, file_id);
		man->entryCount = size / sizeof(BMODEL_ID);
		if(size > sizeof(man->entryToIDTable))
		{	
			GF_ASSERT_MSG(0, "配置モデルリストデータがオーバー（size=%d ARCINDEX=%d)\n", size, file_id);
			man->entryCount = BMODEL_ENTRY_MAX;	//とりあえずハングアップ回避
			size = sizeof(man->entryToIDTable);
		}
		//TAMADA_Printf("entryCount=%d\n", man->entryCount);
		GFL_ARC_LoadDataOfsByHandle(hdl, file_id, 0, size, man->entryToIDTable);
	}
	GFL_ARC_CloseDataHandle(hdl);
	
}

//------------------------------------------------------------------
//BMODEL_IDで引くIDToEntryTableにはentryToIDTableへのオフセットを格納
//------------------------------------------------------------------
static void makeIDToEntryTable(FIELD_BMODEL_MAN * man)
{
	int i;
	for (i = 0; i < man->entryCount; i++)
	{	
		int pos = man->entryToIDTable[i];
		man->IDToEntryTable[pos] = i;
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


//------------------------------------------------------------------
/**
 * @brief FLDMAPPERでの登録に使用できるデータを生成
 */
//------------------------------------------------------------------
static void makeResistObjTable(FIELD_BMODEL_MAN * man)
{
	static const FLDMAPPER_RESISTDATA_OBJTBL init = {	
		0, NULL, 0, 0, NULL, 0
	};
	int i;
	FLDMAPPER_RESISTDATA_OBJTBL * resist = &man->gobjData_Tbl;
	FLDMAPPER_RESISTOBJDATA * mapperObj = man->resistObjTbl;
	GFL_STD_MemFill16(mapperObj, NON_LOWQ, sizeof(man->resistObjTbl));
	for (i = 0; i < man->entryCount; i++)
	{	
		mapperObj[i].highQ_ID = man->entryToIDTable[i];
	}
	*resist = init;
	resist->objArcID = man->model_arcid;
	resist->objData = mapperObj;
	resist->objCount = man->entryCount;
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
 * @brief 配置モデルアニメデータアーカイブ指定IDの取得
 *
 * 本当はmanを受け取る必要はないが、将来的なこととインターフェイスを
 * あわせる意味で引数に指定している。
 */
//------------------------------------------------------------------
ARCID FIELD_BMODEL_MAN_GetAnimeArcID(const FIELD_BMODEL_MAN * man)
{ 
  return ARCID_BMODEL_ANIME;
}
//------------------------------------------------------------------
/**
 * @brief 保持しているアニメIDへのポインタを返す
 * @param data  アニメデータへのポインタ
 * @return  u16 * アニメID配列へのポインタ
 *
 * 配列長自体はFIELD_BMANIME_DATA_getAnimeCountで取得する
 */
//------------------------------------------------------------------
const u16 * FIELD_BMANIME_DATA_getAnimeFileID(const FIELD_BMANIME_DATA * data)
{ 
  return data->anm_id;
}

//------------------------------------------------------------------
/**
 * @brief 保持しているアニメIDの数を返す
 * @param data  アニメデータへのポインタ
 * @param u32 保持しているアニメIDの数
 */
//------------------------------------------------------------------
u32 FIELD_BMANIME_DATA_getAnimeCount(const FIELD_BMANIME_DATA * data)
{ 
  u32 i,count;
  for (i = 0, count = 0; i < GLOBAL_OBJ_ANMCOUNT; i++)
  { 
    if (data->anm_id[i] != BMANIME_NULL_ID)
    { 
      count ++;
    }
  }
  return count;
}
//------------------------------------------------------------------
/**
 * @brief アニメ適用タイプを返す
 * @param data  アニメデータへのポインタ
 * @return  BMANIME_TYPE
 */
//------------------------------------------------------------------
BMANIME_TYPE FIELD_BMANIME_DATA_getAnimeType(const FIELD_BMANIME_DATA * data)
{ 
  return data->anm_type;
}

//------------------------------------------------------------------
/**
 * @brief プログラムタイプを返す
 * @param data  アニメデータへのポインタ
 * @return  BMANIME_PROG_TYPE
 */
//------------------------------------------------------------------
BMANIME_PROG_TYPE FIELD_BMANIME_DATA_getProgType(const FIELD_BMANIME_DATA * data)
{ 
  return data->prg_type;
}

//------------------------------------------------------------------
/**
 * @brief FIELD_BMANIME_DATAの初期化
 */
//------------------------------------------------------------------
static void FIELD_BMANIME_DATA_init(FIELD_BMANIME_DATA * data)
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
static void FIELD_BMANIME_DATA_dump(const FIELD_BMANIME_DATA * data)
{ 
  static const char * animetype[] ={  
    "BMANIME_TYPE_NONE","BMANIME_TYPE_ETERNAL","BMANIME_TYPE_EVENT",
    "BMANIME_TYPE_ERROR",
  };
  int i;
  int type = data->anm_type;
  if (type >= BMANIME_TYPE_MAX) type = 3;
  //TAMADA_Printf("FIELD_BMANIME_DATA:");
  //TAMADA_Printf("%s, %d\n", animetype[data->anm_type], type);
  //TAMADA_Printf("%d %d %d\n",data->prg_type, data->anm_count, data->set_count);
  for (i = 0; i < GLOBAL_OBJ_ANMCOUNT; i++)
  {
    //TAMADA_Printf("%04x ", data->anm_id[i]);
  }
  //TAMADA_Printf("\n");
}
#endif

//------------------------------------------------------------------
/**
 * @brief 配置モデル用データの読み込み
 */
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
static void loadBmInfo(FIELD_BMODEL_MAN * man, u16 file_id)
{
  ARCHANDLE * info_hdl = GFL_ARC_OpenDataHandle(ARCID_BMODEL_INFO, man->heapID);
  u16 anime_id;
  BM_INFO * bminfo = GFL_HEAP_AllocMemory(man->heapID, sizeof(BM_INFO));
  int i;
  enum { DATASIZE = sizeof(BM_INFO) };

  for (i = 0; i < man->entryCount; i++)
  { 
    BMODEL_ID id = man->entryToIDTable[i];
    GFL_ARC_LoadDataOfsByHandle(info_hdl, file_id, id * DATASIZE, DATASIZE, bminfo);
    anime_id = bminfo->anime_id;
    TAMADA_Printf("bmodel: entry ID(%d) --> anime_id(%d)\n", id, anime_id);
    if (anime_id != 0xffff)
    { 
      GFL_ARC_LoadDataOfsByHandle(info_hdl, FILEID_BMODEL_ANIMEDATA,
          anime_id * sizeof(FIELD_BMANIME_DATA), sizeof(FIELD_BMANIME_DATA),
          &man->animeData[i]);
      FIELD_BMANIME_DATA_dump(&man->animeData[i]);
    }
    man->prog_id[i] = bminfo->prog_id;
    man->subModels[i] = bminfo->submodel_id;
  }
  GFL_HEAP_FreeMemory(bminfo);
  GFL_ARC_CloseDataHandle(info_hdl);
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
  STRBUF * str;

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
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_Draw( const FIELD_BMODEL * bmodel )
{
  GFL_G3D_DRAW_DrawObject( bmodel->gfl_obj, &bmodel->status );
#if 0
  fx32 sin, cos;
  NNSG3dRenderObj *NNSrnd;
	MtxFx33				mtxRot;
  static const VecFx32 scale = { FX32_ONE, FX32_ONE, FX32_ONE };

  NNSrnd = GFL_G3D_RENDER_GetRenderObj( GFL_G3D_OBJECT_GetG3Drnd( bmodel->gfl_obj ) );
  	
  fx32 sin = FX_SinIdx(bmodel->rotate);
  fx32 cos = FX_CosIdx(bmodel->rotate);
  MTX_RotY33(&mtxRot, sin, cos);
  NNS_G3dGlbSetBaseTrans( &bmodel->pos );		// 位置設定
  NNS_G3dGlbSetBaseRot( &mtxRot );		// 角度設定
  NNS_G3dGlbSetBaseScale( &scale );	// スケール設定
  NNS_G3dGlbFlush();							//グローバルステート反映

  NNS_G3dDraw( NNSrnd );
#endif
}

//------------------------------------------------------------------
//------------------------------------------------------------------
FIELD_BMODEL * FIELD_BMODEL_Create(FIELD_BMODEL_MAN * man,
    const FLDMAPPER * g3Dmapper, const GFL_G3D_MAP_GLOBALOBJ_ST * status)
{
  const GLOBAL_RESOURCE * objRes;
  const GLOBAL_OBJ * gObj;

  FIELD_BMODEL * bmodel = GFL_HEAP_AllocMemory( man->heapID, sizeof(FIELD_BMODEL) );
  fx32 sin = FX_SinIdx(status->rotate);
  fx32 cos = FX_CosIdx(status->rotate);

  MTX_RotY33( &bmodel->status.rotate, sin, cos );
  bmodel->status.trans = status->trans;
  VEC_Set( &bmodel->status.scale, FX32_ONE, FX32_ONE, FX32_ONE );

  GF_ASSERT( status->id < man->gResCount );
  objRes = &man->gResource[status->id];
  gObj = &man->gObject[status->id];
  {
    int i, count;
	  GFL_G3D_ANM* anmTbl[GLOBAL_OBJ_ANMCOUNT];
    GFL_G3D_RES* resTex = objRes->g3DresTex ? objRes->g3DresTex : objRes->g3DresMdl;
	  GFL_G3D_RND* g3Drnd = GFL_G3D_RENDER_Create( objRes->g3DresMdl, 0, resTex );
    count = GFL_G3D_OBJECT_GetAnimeCount( gObj->g3Dobj );
    for (i = 0; i < GLOBAL_OBJ_ANMCOUNT; i++)
    {
      if (i < count && objRes->g3DresAnm[i] != NULL) 
      {
        anmTbl[i] = GFL_G3D_ANIME_Create( g3Drnd, objRes->g3DresAnm[i], 0 );
      } else {
        anmTbl[i] = NULL;
      }
    }
    bmodel->gfl_obj = GFL_G3D_OBJECT_Create( g3Drnd, anmTbl, GLOBAL_OBJ_ANMCOUNT );
  }
  bmodel->suicide_flag = FALSE;
  return bmodel;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_Delete(FIELD_BMODEL * bmodel)
{
  int i;
	GFL_G3D_RND*	g3Drnd;
  g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( bmodel->gfl_obj );
  for (i = 0; i < GLOBAL_OBJ_ANMCOUNT; i++)
  {
    GFL_G3D_ANM * g3Danm = GFL_G3D_OBJECT_GetG3Danm( bmodel->gfl_obj, i );
    if (g3Danm != NULL)
    {
      GFL_G3D_ANIME_Delete( g3Danm );
    }
  }
  GFL_G3D_OBJECT_Delete( bmodel->gfl_obj );
  GFL_G3D_RENDER_Delete( g3Drnd );
  //テクスチャ、モデリング、リソースは借り物なので解放しない

  GFL_HEAP_FreeMemory(bmodel);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_SetAnime(FIELD_BMODEL * bmodel, u32 idx)
{
  GFL_G3D_OBJECT_EnableAnime(bmodel->gfl_obj, idx );
  GFL_G3D_OBJECT_ResetAnimeFrame(bmodel->gfl_obj, idx );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_RunAnime(FIELD_BMODEL * bmodel)
{
  int i;
  BOOL result;
  for (i = 0; i < GLOBAL_OBJ_ANMCOUNT; i++)
  {
    GFL_G3D_ANM * g3Danm = GFL_G3D_OBJECT_GetG3Danm( bmodel->gfl_obj, i );
    if (g3Danm == NULL) continue;
    result = GFL_G3D_OBJECT_LoopAnimeFrame( bmodel->gfl_obj, i, FX32_ONE);
    if (!result)
    {
      bmodel->suicide_flag = TRUE;
      GFL_G3D_OBJECT_DisableAnime( bmodel->gfl_obj, i );
    }
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_BMODEL_GetAnimeStatus(FIELD_BMODEL * bmodel)
{
  return bmodel->suicide_flag;
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

enum {  MAKE_RES_NONPARAM = (0xffffffff) };

typedef struct {
	MAKE_RES_PARAM	mdl;
	MAKE_RES_PARAM	tex;
	MAKE_RES_PARAM	anm[GLOBAL_OBJ_ANMCOUNT];
}MAKE_OBJ_PARAM;

static void createGlobalObj( GLOBAL_OBJ * gObj, const GLOBAL_RESOURCE* objRes);
static void createGlobalResource( GLOBAL_RESOURCE * objRes, const MAKE_OBJ_PARAM * param);
static void deleteGlobalObj( GLOBAL_OBJ * gObj );
static void deleteGlobalResource( GLOBAL_RESOURCE * objRes );
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
//------------------------------------------------------------------
//配置モデルマネジャーからの内容で生成
//------------------------------------------------------------------
static void CreateGlobalObj_forBModel(FIELD_BMODEL_MAN * man)
{ 
	int i;
  MAKE_OBJ_PARAM objParam;
  ARCID anmArcID;
  GFL_G3D_MAP_GLOBALOBJ * globalObj = &man->globalObj;
  const FLDMAPPER_RESISTDATA_OBJTBL * gobjTbl = &man->gobjData_Tbl;

	if( gobjTbl->objCount == 0 ){
    return;
  }

  man->gResCount = gobjTbl->objCount;
  man->gResource = GFL_HEAP_AllocClearMemory
          ( man->heapID, sizeof(GLOBAL_RESOURCE) * gobjTbl->objCount );
  man->gObjCount = gobjTbl->objCount;
  man->gObject = GFL_HEAP_AllocClearMemory(
      man->heapID, sizeof(GLOBAL_OBJ) * gobjTbl->objCount );

  globalObj->gobjCount = gobjTbl->objCount;
  globalObj->gobj = GFL_HEAP_AllocClearMemory
          ( man->heapID, sizeof(GFL_G3D_MAP_OBJ) * gobjTbl->objCount );

  MAKE_OBJ_PARAM_init(&objParam);
  anmArcID = FIELD_BMODEL_MAN_GetAnimeArcID(man);

  for ( i = 0; i < man->gResCount; i++ )
  {
    int j, count;
    const u16 * anmIDs;
    const FIELD_BMANIME_DATA * anmData;
    GLOBAL_RESOURCE * objRes = &man->gResource[i];

    MAKE_RES_PARAM_set(&objParam.mdl, gobjTbl->objArcID, gobjTbl->objData[i].highQ_ID, 0);
    
    //配置モデルに対応したアニメデータを取得 
    anmData = FIELD_BMODEL_MAN_GetAnimeData(man, gobjTbl->objData[i].highQ_ID);
    count = FIELD_BMANIME_DATA_getAnimeCount(anmData);
    anmIDs = FIELD_BMANIME_DATA_getAnimeFileID(anmData);

    for( j=0; j<GLOBAL_OBJ_ANMCOUNT; j++ )
    {
      if( j<count ) 
      { 
        MAKE_RES_PARAM_set(&objParam.anm[j], anmArcID, anmIDs[j], 0);
      } else {
        MAKE_RES_PARAM_init( &objParam.anm[j] );
      }
    }

    createGlobalResource( objRes, &objParam );
    FIELD_BMANIME_DATA_entryTexData( man, anmData, objRes->g3DresTex );
  }

  for ( i = 0; i < man->gObjCount; i++ )
  {
    const FIELD_BMANIME_DATA * anmData;
    GLOBAL_OBJ * gObj = &man->gObject[i];
    GLOBAL_RESOURCE * objRes = &man->gResource[i];
    createGlobalObj( gObj, objRes );

    anmData = FIELD_BMODEL_MAN_GetAnimeData(man, gobjTbl->objData[i].highQ_ID);
    if (FIELD_BMANIME_DATA_getAnimeType(anmData) == BMANIME_TYPE_ETERNAL)
    {
      int j;
      for( j=0; j<GLOBAL_OBJ_ANMCOUNT; j++ ){
        GFL_G3D_OBJECT_EnableAnime( gObj->g3Dobj, j );  //renderとanimeの関連付け
        GFL_G3D_OBJECT_ResetAnimeFrame( gObj->g3Dobj, j ); 
      }
    }
    globalObj->gobj[i].g3DobjHQ = gObj->g3Dobj;
    globalObj->gobj[i].g3DobjLQ = NULL;
  }

}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void DeleteGlobalObject(FIELD_BMODEL_MAN * man, GFL_G3D_MAP_GLOBALOBJ * globalObj)
{
	if( globalObj->gobj != NULL ){
		GFL_HEAP_FreeMemory( globalObj->gobj );
		globalObj->gobj = NULL;
	}
	if( globalObj->gobjIDexchange != NULL ){
		GFL_HEAP_FreeMemory( globalObj->gobjIDexchange );
		globalObj->gobjIDexchange = NULL;
	}

  if( man->gObject != NULL ){
    int i;
    for( i=0; i<man->gObjCount; i++ ){
			deleteGlobalObj( &man->gObject[i] );
    }
    GFL_HEAP_FreeMemory( man->gObject );
    man->gObjCount = 0;
    man->gObject = NULL;
  }

	if( man->gResource != NULL ){
		int i;

		for( i=0; i<man->gResCount; i++ ){
      deleteGlobalResource( &man->gResource[i] );
		}
		GFL_HEAP_FreeMemory( man->gResource );
		man->gResCount = 0;
		man->gResource = NULL;
	}
}

//------------------------------------------------------------------
// オブジェクトリソースを作成
//------------------------------------------------------------------
static void createGlobalResource( GLOBAL_RESOURCE * objRes, const MAKE_OBJ_PARAM * param)
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
	DEBUG_Field_Grayscale(resTex);
	GFL_G3D_TransVramTexture( resTex );
	for( i=0; i<GLOBAL_OBJ_ANMCOUNT; i++ ){
		if( param->anm[i].arcID != MAKE_RES_NONPARAM ){
			objRes->g3DresAnm[i] = GFL_G3D_CreateResourceArc( param->anm[i].arcID, param->anm[i].datID );
    }
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void createGlobalObj( GLOBAL_OBJ * gObj, const GLOBAL_RESOURCE* objRes)
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
		if( objRes->g3DresAnm[i] != NULL ){
			anmTbl[i] = GFL_G3D_ANIME_Create( g3Drnd, objRes->g3DresAnm[i], 0 );  
		} else {
			anmTbl[i] = NULL;
		}
	}
  //GFL_G3D_OBJECT生成
	gObj->g3Dobj = GFL_G3D_OBJECT_Create( g3Drnd, anmTbl, GLOBAL_OBJ_ANMCOUNT );
  gObj->res = objRes;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void deleteGlobalResource( GLOBAL_RESOURCE * objRes )
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
static void deleteGlobalObj( GLOBAL_OBJ * gObj )
{
	GFL_G3D_RND*	g3Drnd;
	GFL_G3D_RES*	resTex;
	GFL_G3D_ANM*	g3Danm[GLOBAL_OBJ_ANMCOUNT] = { NULL, NULL, NULL, NULL };
	u16				g3DanmCount;
	int i;

	if( gObj->g3Dobj != NULL ){
		//各種ハンドル取得
		g3DanmCount = GFL_G3D_OBJECT_GetAnimeCount( gObj->g3Dobj );
		for( i=0; i<g3DanmCount; i++ ){
			g3Danm[i] = GFL_G3D_OBJECT_GetG3Danm( gObj->g3Dobj, i ); 
      if (g3Danm[i] != NULL) {
        GFL_G3D_ANIME_Delete( g3Danm[i] ); 
      }
		}
		g3Drnd = GFL_G3D_OBJECT_GetG3Drnd( gObj->g3Dobj );
	
		//各種ハンドル＆リソース削除
		GFL_G3D_OBJECT_Delete( gObj->g3Dobj );
		gObj->g3Dobj = NULL;
    gObj->res = NULL;

		GFL_G3D_RENDER_Delete( g3Drnd );
	}
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

  for( i=0, j = 0, count = objCount; i<count && j < 32; j++, i++ ){
#if 0
    status.id = FIELD_BMODEL_MAN_GetEntryIndex(man, objStatus[i].resourceID);
    VEC_Set( &status.trans, 
        objStatus[i].xpos, objStatus[i].ypos, -objStatus[i].zpos );
    status.rotate = (u16)(objStatus[i].rotate);
    //OS_Printf("bm id = %d, rotate = %04x\n",i, status.rotate);
    GFL_G3D_MAP_ResistGlobalObj( g3Dmap, &status, j );
#else
    FIELD_BMODEL_MAN_ResistMapObject(man, g3Dmap, &objStatus[i], j);
#endif
    if (FIELD_BMODEL_MAN_GetSubModel(man,
          objStatus[i].resourceID, &status.trans, &status.id) == TRUE) 
    {
      TAMADA_Printf("Resist Sub Model:index(%d) model id(%d)\n", i, status.id);
      j++;
      status.trans.x += objStatus[i].xpos;
      status.trans.y += objStatus[i].ypos;
      status.trans.z -= objStatus[i].zpos;
      GFL_G3D_MAP_ResistGlobalObj( g3Dmap, &status, j );
    }

  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_ReleaseAllMapObjects
(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap)
{
  /* 今は実装無し */
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_ResistMapObject
(FIELD_BMODEL_MAN * man, GFL_G3D_MAP * g3Dmap, const PositionSt* objStatus, u32 objCount)
{
  GFL_G3D_MAP_GLOBALOBJ_ST status;
  status.id = FIELD_BMODEL_MAN_GetEntryIndex(man, objStatus->resourceID);
  VEC_Set(&status.trans, objStatus->xpos, objStatus->ypos, -objStatus->zpos);
  status.rotate = (u16)(objStatus->rotate);
  GFL_G3D_MAP_ResistGlobalObj( g3Dmap, &status, objCount );
}



