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

typedef EL_SCOREBOARD_TEX ELBOARD_TEX;

//============================================================================================
//============================================================================================

enum {  
  BMANIME_ID_COUNT_MAX = 4,

};

enum{ 
  FILEID_BMODEL_ANIMEDATA = NARC_buildmodel_info_buildmodel_anime_bin,


};
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
  u16 anm_id[BMANIME_ID_COUNT_MAX];

};

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _FIELD_BMODEL_MAN
{
	HEAPID heapID;

	u8 IDToEntryTable[BMODEL_ID_MAX];

	u16 entryCount;
	BMODEL_ID entryToIDTable[BMODEL_ENTRY_MAX];
  FIELD_BMANIME_DATA animeData[BMODEL_ENTRY_MAX];

	ARCID model_arcid;

  //FLDMAPPERに引き渡すための生成データ保持ワーク
	FLDMAPPER_RESISTDATA_OBJTBL	gobjData_Tbl;
	FLDMAPPER_RESISTOBJDATA resistObjTbl[BMODEL_ENTRY_MAX * 2];
  
  STRBUF * elb_str[FIELD_BMODEL_ELBOARD_ID_MAX];
  ELBOARD_TEX * elb_tex[FIELD_BMODEL_ELBOARD_ID_MAX];
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

static void loadAnimeData(FIELD_BMODEL_MAN * man, u16 file_id);
//------------------------------------------------------------------
//------------------------------------------------------------------
static void FIELD_BMANIME_DATA_init(FIELD_BMANIME_DATA * data);

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

  //必要な配置モデルアニメデータの読み込み
  loadAnimeData(man, model_info_dataid);

  //FLDMAPPERに引き渡すデータを生成
	makeResistObjTable(man);
}

//------------------------------------------------------------------
/**
 * @brief
 * @param man 配置モデルマネジャーへのポインタ
 * @return  FLDMAPPER_RESISTDATA_OBJTBL 生成したデータへのポインタ
 *
 * 生成データを保持するメモリははマネジャー管理下に置かれている。
 * このタイミングで確保しているわけではない
 */
//------------------------------------------------------------------
const FLDMAPPER_RESISTDATA_OBJTBL * FIELD_BMODEL_MAN_GetOBJTBL(const FIELD_BMODEL_MAN * man)
{	
	return &man->gobjData_Tbl;
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

	//読み込み
#if 1 //ランダムマップ
	if( arc_id == ARCID_BMODEL_IDX_OUTDOOR &&
	    file_id == 18 )
	{
    u8 i;
    u8 tempArr[15] = {  0, 2, 3, 4, 6,
                        8, 9,10,13,14,
                       15,17,33,34,41};
    man->entryCount = 15;
    for( i=0;i<15;i++ )
    {
      man->entryToIDTable[i] = tempArr[i];
    }
  }
  else
#endif 
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
  for (i = 0, count = 0; i < BMANIME_ID_COUNT_MAX; i++)
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
  for (i = 0; i < BMANIME_ID_COUNT_MAX; i++)
  {
    //TAMADA_Printf("%04x ", data->anm_id[i]);
  }
  //TAMADA_Printf("\n");
}
#endif

//------------------------------------------------------------------
/**
 * @brief 配置モデル用アニメデータの読み込み
 */
//------------------------------------------------------------------
static void loadAnimeData(FIELD_BMODEL_MAN * man, u16 file_id)
{ 
  ARCHANDLE * info_hdl = GFL_ARC_OpenDataHandle(ARCID_BMODEL_INFO, man->heapID);
  u16 anime_id;
  int i;

  { 
    int count = GFL_ARC_GetDataSizeByHandle(info_hdl, file_id) / sizeof(u16);
    //TAMADA_Printf("BModel <--> anime table (%d)\n",count);
    for (i = 0; i < count ; i++)
    { 
      GFL_ARC_LoadDataOfsByHandle(info_hdl, file_id, i * sizeof(u16), sizeof(u16), &anime_id);
      //TAMADA_Printf("%03d %04x\n", i, anime_id);
    }
  }
  for (i = 0; i < man->entryCount; i++)
  { 
    BMODEL_ID id = man->entryToIDTable[i];
    GFL_ARC_LoadDataOfsByHandle(info_hdl, file_id, id * sizeof(u16), sizeof(u16), &anime_id);
    //TAMADA_Printf("bmodel: entry ID(%d) --> anime_id(%d)\n", id, anime_id);
    if (anime_id != 0xffff)
    { 
      GFL_ARC_LoadDataOfsByHandle(info_hdl, FILEID_BMODEL_ANIMEDATA,
          anime_id * sizeof(FIELD_BMANIME_DATA), sizeof(FIELD_BMANIME_DATA),
          &man->animeData[i]);
      FIELD_BMANIME_DATA_dump(&man->animeData[i]);
    }
  }
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


