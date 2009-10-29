//////////////////////////////////////////////////////////////////////////////////////////// 
/** 
 *
 * @brief  ギミック登録関数(ゲート) 
 * @file   field_gimmick_gate.c 
 * @author obata 
 * @date   2009.10.21 
 *
 * [電光掲示板の追加方法]
 * ①配置したいゾーンでギミックが動作するように, fld_gimmick_assign.xls に登録
 * ②登録したゾーンでのギミック動作関数を, field_gimmick.c に登録
 * ③配置するゾーンの電光掲示板データを作成し, 電光掲示板データ登録テーブルに追加
 *
 */ 
//////////////////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h> 
#include "fieldmap.h" 
#include "field_gimmick_gate.h" 
#include "arc/arc_def.h" 
#include "arc/gate.naix"
#include "arc/message.naix"
#include "msg/msg_place_name.h" // TEMP:

#include "field/gimmick_obj_elboard.h"
#include "savedata/gimmickwork.h"
#include "field_gimmick_def.h"
#include "gimmick_obj_elboard.h"
#include "elboard_zone_data.h"
#include "arc/gate.naix"
#include "../../../resource/fldmapdata/zonetable/zone_id.h"


//==========================================================================================
// ■電光掲示板データ登録テーブル
//==========================================================================================
typedef struct
{
  u16 zone_id;
  u8  gimmick_id;
  u32 dat_id; 
} ENTRY_DATA;

static const ENTRY_DATA entry_table[] = 
{
  { ZONE_ID_C04R0601, FLD_GIMMICK_C04R0601, NARC_gate_elboard_zone_data_c04r0601_bin },
  { ZONE_ID_C08R0601, FLD_GIMMICK_C08R0601, NARC_gate_elboard_zone_data_c08r0601_bin },
  { ZONE_ID_R13R0201, FLD_GIMMICK_R13R0201, NARC_gate_elboard_zone_data_r13r0201_bin },
  { ZONE_ID_R02R0101, FLD_GIMMICK_R02R0101, NARC_gate_elboard_zone_data_r02r0101_bin },
  { ZONE_ID_C04R0701, FLD_GIMMICK_C04R0701, NARC_gate_elboard_zone_data_c04r0701_bin },
  { ZONE_ID_C04R0801, FLD_GIMMICK_C04R0801, NARC_gate_elboard_zone_data_c04r0801_bin },
  { ZONE_ID_C02R0701, FLD_GIMMICK_C02R0701, NARC_gate_elboard_zone_data_c02r0701_bin },
  { ZONE_ID_R14R0101, FLD_GIMMICK_R14R0101, NARC_gate_elboard_zone_data_r14r0101_bin },
  { ZONE_ID_C08R0501, FLD_GIMMICK_C08R0501, NARC_gate_elboard_zone_data_c08r0501_bin },
  { ZONE_ID_C08R0701, FLD_GIMMICK_C08R0701, NARC_gate_elboard_zone_data_c08r0701_bin },
  { ZONE_ID_H01R0101, FLD_GIMMICK_H01R0101, NARC_gate_elboard_zone_data_h01r0101_bin },
  { ZONE_ID_H01R0201, FLD_GIMMICK_H01R0201, NARC_gate_elboard_zone_data_h01r0201_bin },
  { ZONE_ID_C03R0601, FLD_GIMMICK_C03R0601, NARC_gate_elboard_zone_data_c03r0601_bin },
};
// フィールドマップに対応するギミックIDを取得する
static u8 GetGimmickID( FIELDMAP_WORK* fieldmap )
{
  int i;
  u16 zone_id = FIELDMAP_GetZoneID( fieldmap );
  for( i=0; i<NELEMS(entry_table); i++ )
  {
    if( entry_table[i].zone_id == zone_id )
    {
      return entry_table[i].gimmick_id;
    }
  }
  return 0;
}


//==========================================================================================
// ■定数
//==========================================================================================
// ニュース番号
typedef enum
{
  NEWS_DATE,      // 日時
  NEWS_WEATHER,   // 天気
  NEWS_INFO_A,    // 情報A
  NEWS_INFO_B,    // 情報B
  NEWS_INFO_C,    // 情報C
  NEWS_CM,        // 一言CM
  NEWS_NUM,
  NEWS_MAX = NEWS_NUM-1
} NEWS_INDEX; 

// 一度に表示する地域情報の数
#define LOCAL_INFO_NUM (3)

// 拡張オブジェクト・ユニット番号
#define EXPOBJ_UNIT_ELBOARD (1)

// 表示可能な一画面に表示可能な文字数
#define DISPLAY_SIZE (8)

// ニュース間の間隔(文字数)
#define NEWS_INTERVAL (3)



//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソースインデックス
typedef enum
{
  RES_ELBOARD_NSBMD,    // 掲示板のモデル
  RES_ELBOARD_NSBTX,    // 掲示板のテクスチャ
  RES_ELBOARD_NSBTA_1,  // スクロールアニメーション1
  RES_ELBOARD_NSBTA_2,  // スクロールアニメーション2
  RES_ELBOARD_NSBTA_3,  // スクロールアニメーション3
  RES_ELBOARD_NSBTA_4,  // スクロールアニメーション4
  RES_ELBOARD_NSBTA_5,  // スクロールアニメーション5
  RES_ELBOARD_NSBTA_6,  // スクロールアニメーション6
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[] = 
{
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_nsbtx, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_1_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_2_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_3_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_4_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_5_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_6_nsbta, GFL_G3D_UTIL_RESARC },
};

// アニメインデックス
typedef enum
{
  ANM_ELBOARD_DATE,    // 日付
  ANM_ELBOARD_WEATHER, // 天気
  ANM_ELBOARD_INFO_A,  // 情報A
  ANM_ELBOARD_INFO_B,  // 情報B
  ANM_ELBOARD_INFO_C,  // 情報C
  ANM_ELBOARD_CM,      // 一言CM
  ANM_NUM
} ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table[] = 
{
  // アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ELBOARD_NSBTA_1, 0 },
  { RES_ELBOARD_NSBTA_2, 0 },
  { RES_ELBOARD_NSBTA_3, 0 },
  { RES_ELBOARD_NSBTA_4, 0 },
  { RES_ELBOARD_NSBTA_5, 0 },
  { RES_ELBOARD_NSBTA_6, 0 },
};

// オブジェクトインデックス
typedef enum
{
  OBJ_ELBOARD,  // 電光掲示板
  OBJ_NUM
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[] = 
{
  // モデルリソースID, 
  // モデルデータID(リソース内部INDEX), 
  // テクスチャリソースID,
  // アニメテーブル, 
  // アニメリソース数
  { RES_ELBOARD_NSBMD, 0, RES_ELBOARD_NSBTX, anm_table, ANM_NUM },
}; 

// ユニットインデックス
typedef enum
{
  UNIT_ELBOARD, // 電光掲示板
  UNIT_NUM
} UNIT_INDEX;
static const GFL_G3D_UTIL_SETUP unit[UNIT_NUM] =
{
  { res_table, RES_NUM, obj_table, OBJ_NUM },
};


//==========================================================================================
// ■ニュースパラメータ
//==========================================================================================
// アニメーション・インデックス
static u16 news_anm_index[NEWS_NUM] = 
{
  ANM_ELBOARD_DATE,    // 日付
  ANM_ELBOARD_WEATHER, // 天気
  ANM_ELBOARD_INFO_A,  // 情報A
  ANM_ELBOARD_INFO_B,  // 情報B
  ANM_ELBOARD_INFO_C,  // 情報C
  ANM_ELBOARD_CM,      // 一言CM
};
// テクスチャ名
static char* news_tex_name[NEWS_NUM] =
{
  "gelboard_1",
  "gelboard_2",
  "gelboard_3",
  "gelboard_4",
  "gelboard_5",
  "gelboard_6",
};
// パレット名
static char* news_plt_name[NEWS_NUM] =
{
  "gelboard_1_pl",
  "gelboard_2_pl",
  "gelboard_3_pl",
  "gelboard_4_pl",
  "gelboard_5_pl",
  "gelboard_6_pl",
};


//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct
{ 
  HEAPID        heapID;   // 使用するヒープID
  GOBJ_ELBOARD* elboard;  // 電光掲示板管理オブジェクト
} GATEWORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void GimmickSave( FIELDMAP_WORK* fieldmap );
static void GimmickLoad( GATEWORK* work, FIELDMAP_WORK* fieldmap );
static GATEWORK* CreateGateWork( FIELDMAP_WORK* fieldmap );
static BOOL LoadGateData( ELBOARD_ZONE_DATA* buf, FIELDMAP_WORK* fieldmap );
static void SetElboardPos( GFL_G3D_OBJSTATUS* status, ELBOARD_ZONE_DATA* data );
static void AddNews_DATE( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data );
static void AddNews_WEATHER( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data );
static void AddNews_INFO( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data );
static void AddNews_CM( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data );


//==========================================================================================
// ■ギミック登録関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  u32* gmk_save;  // ギミックの実セーブデータ
  GATEWORK* work;  // GATEギミック管理ワーク
  HEAPID                heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  GAMESYS_WORK*            gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*               gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*         sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*          gmkwork = SaveData_GetGimmickWork( sv );

  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &unit[UNIT_ELBOARD], EXPOBJ_UNIT_ELBOARD );

  // ギミック管理ワークを作成
  work = CreateGateWork( fieldmap );

  // ギミック状態の復帰
  GimmickLoad( work, fieldmap );

  OBATA_Printf( "GATE_GIMMICK_Setup\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*  gmkwork = SaveData_GetGimmickWork( sv );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, GetGimmickID(fieldmap) );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // ギミック状態をセーブ
  GimmickSave( fieldmap );
  
  // 電光掲示板管理ワークを破棄
  if( work->elboard )
  {
    GOBJ_ELBOARD_Delete( work->elboard );
  }

  // ギミック管理ワークを破棄
  GFL_HEAP_FreeMemory( work );

  OBATA_Printf( "GATE_GIMMICK_End\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*  gmkwork = SaveData_GetGimmickWork( sv );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, GetGimmickID(fieldmap) );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 電光掲示板メイン処理
  GOBJ_ELBOARD_Main( work->elboard, FX32_ONE ); 

  OBATA_Printf( "GATE_GIMMICK_Move\n" );
}


//==========================================================================================
// ■ 非公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ギミック状態を保存する
 *
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void GimmickSave( FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*  gmkwork = SaveData_GetGimmickWork( sv );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, GetGimmickID(fieldmap) );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 掲示板の動作フレーム数を記憶
  gmk_save[1] = GOBJ_ELBOARD_GetFrame( work->elboard ) >> FX32_SHIFT;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミック状態を復帰する
 *
 * @param work     ギミック管理ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void GimmickLoad( GATEWORK* work, FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*  gmkwork = SaveData_GetGimmickWork( sv );
  u32*         gmk_save = NULL;

  // 有効なセーブデータがある場合
  if( GIMMICKWORK_GetAssignID( gmkwork ) == GetGimmickID(fieldmap) )
  {
    u32 frame;

    // セーブデータ取得
    gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, GetGimmickID(fieldmap) );
    frame    = gmk_save[1];
    GOBJ_ELBOARD_SetFrame( work->elboard, frame << FX32_SHIFT );
  }
  // セーブデータが無い場合
  else
  {
    // 新規セーブデータを作成
    gmk_save = (u32*)GIMMICKWORK_Assign( gmkwork, GetGimmickID(fieldmap) );
  }

  // ギミック管理ワークのアドレスを記憶
  gmk_save[0] = (int)work; 
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミック管理ワークを作成する
 *
 * @param fieldmap 依存するフィールドマップ
 *
 * @return 作成したギミック管理ワーク
 */
//------------------------------------------------------------------------------------------
static GATEWORK* CreateGateWork( FIELDMAP_WORK* fieldmap )
{
  int i;
  GATEWORK*                work = NULL;
  HEAPID                heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  ELBOARD_ZONE_DATA elboard_data;

  // ギミック管理ワークを作成
  work = (GATEWORK*)GFL_HEAP_AllocMemory( heap_id, sizeof(GATEWORK) );
  GFL_STD_MemClear( work, sizeof(GATEWORK) );

  // ヒープIDを記憶
  work->heapID = heap_id;

  // 電光掲示板データを取得
  if( !LoadGateData( &elboard_data, fieldmap ) )
  {
    // データが取得できなかったら, 以降の初期化を中断
    return work;
  }

  // 電光掲示板を配置
  { 
    GFL_G3D_OBJSTATUS* status = 
      FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_ELBOARD, OBJ_ELBOARD );
    SetElboardPos( status, &elboard_data );
  } 
  // 電光掲示板管理ワークを作成
  {
    ELBOARD_PARAM param;
    param.heapID       = work->heapID;
    param.maxNewsNum   = NEWS_NUM;
    param.dispSize     = DISPLAY_SIZE;
    param.newsInterval = NEWS_INTERVAL;
    param.g3dObj       = FLD_EXP_OBJ_GetUnitObj( exobj_cnt, EXPOBJ_UNIT_ELBOARD, OBJ_ELBOARD );
    work->elboard = GOBJ_ELBOARD_Create( &param );
  }
  // ニュースを追加
  AddNews_DATE( work->elboard, &elboard_data );
  AddNews_WEATHER( work->elboard, &elboard_data );
  AddNews_INFO( work->elboard, &elboard_data );
  AddNews_CM( work->elboard, &elboard_data );

  return work;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ゾーンに応じた電光掲示板データを取得する
 *
 * @param buf      読み込んだデータの格納先
 * @param fieldmap フィールドマップ
 *
 * @return データが正しく読み込めたら TRUE
 */
//------------------------------------------------------------------------------------------
static BOOL LoadGateData( ELBOARD_ZONE_DATA* buf, FIELDMAP_WORK* fieldmap )
{
  int i;
  u16 zone_id;

  // ゾーンIDを取得
  zone_id = FIELDMAP_GetZoneID( fieldmap );

  // 電光掲示板データテーブルから, 該当するデータを検索
  for( i=0; i<NELEMS(entry_table); i++ )
  {
    if( entry_table[i].zone_id == zone_id )
    {
      // 電光掲示板データを読み込む
      ELBOARD_ZONE_DATA_Load( buf, ARCID_GATE_GIMMICK, entry_table[i].dat_id ); 
      return TRUE;
    }
  } 

  // 現在のゾーンに対応する電光掲示板データが登録されていない場合
  OBATA_Printf( "---------------------------------------------\n" );
  OBATA_Printf( "error: 電光掲示板データが登録されていません。\n" );
  OBATA_Printf( "---------------------------------------------\n" );
  return FALSE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 電光掲示板を配置する
 *
 * @param status 電光掲示板のオブジェステータス
 * @param data   電光掲示板データ
 */
//------------------------------------------------------------------------------------------
static void SetElboardPos( GFL_G3D_OBJSTATUS* status, ELBOARD_ZONE_DATA* data )
{
  u16 rot;

  // 座標を設定
  status->trans.x = data->x << FX32_SHIFT;
  status->trans.y = data->y << FX32_SHIFT;
  status->trans.z = data->z << FX32_SHIFT;

  // 向きを設定
  switch( data->dir )
  {
  case DIR_DOWN:  rot = 0;    break;
  case DIR_RIGHT: rot = 90;   break;
  case DIR_UP:    rot = 180;  break;
  case DIR_LEFT:  rot = 270;  break;
  default:        rot = 0;    break;
  }
  rot *= 182;  // 65536/360 = 182.044...
  GFL_CALC3D_MTX_CreateRot( 0, rot, 0, &status->rotate );
}

//------------------------------------------------------------------------------------------
/**
 * @breif 掲示板のニュースを追加する(日付)
 *
 * @param elboard 追加する掲示板
 * @param data    電光掲示板データ
 */
//------------------------------------------------------------------------------------------
static void AddNews_DATE( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data )
{
  NEWS_PARAM news;

  // ニュースパラメータを作成
  news.animeIndex = news_anm_index[NEWS_DATE];
  news.texName    = news_tex_name[NEWS_DATE];
  news.pltName    = news_plt_name[NEWS_DATE];
  news.msgArcID   = ARCID_MESSAGE;
  news.msgDatID   = data->arcDatID;
  news.msgStrID   = data->msgID_date;

  // ニュースを追加
  GOBJ_ELBOARD_AddNews( elboard, &news );
}

//------------------------------------------------------------------------------------------
/**
 * @breif 掲示板のニュースを追加する(天気)
 *
 * @param elboard 追加する掲示板
 * @param data    電光掲示板データ
 */
//------------------------------------------------------------------------------------------
static void AddNews_WEATHER( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data )
{
  NEWS_PARAM news;

  // ニュースパラメータを作成
  news.animeIndex = news_anm_index[NEWS_WEATHER];
  news.texName    = news_tex_name[NEWS_WEATHER];
  news.pltName    = news_plt_name[NEWS_WEATHER];
  news.msgArcID   = ARCID_MESSAGE;
  news.msgDatID   = data->arcDatID;
  news.msgStrID   = data->msgID_weather;

  // ニュースを追加
  GOBJ_ELBOARD_AddNews( elboard, &news );
}

//------------------------------------------------------------------------------------------
/**
 * @breif 掲示板のニュースを追加する(地域情報)
 *
 * @param elboard 追加する掲示板
 * @param data    電光掲示板データ
 */
//------------------------------------------------------------------------------------------
static void AddNews_INFO( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data )
{
  int i;
  NEWS_PARAM news[LOCAL_INFO_NUM];
  RTCDate date;

  // 日時データを取得
  RTC_GetDate( &date ); 

  // 曜日に応じた情報を選択
  switch( date.week )
  {
  case RTC_WEEK_SUNDAY:    
    news[0].msgStrID = data->msgID_infoG;  
    news[1].msgStrID = data->msgID_infoH;  
    news[2].msgStrID = data->msgID_infoI;  
    break;
  case RTC_WEEK_MONDAY:    
    news[0].msgStrID = data->msgID_infoD;  
    news[1].msgStrID = data->msgID_infoE;  
    news[2].msgStrID = data->msgID_infoF;  
    break;
  case RTC_WEEK_TUESDAY:   
    news[0].msgStrID = data->msgID_infoA;  
    news[1].msgStrID = data->msgID_infoB;  
    news[2].msgStrID = data->msgID_infoC;  
    break;
  case RTC_WEEK_WEDNESDAY: 
    news[0].msgStrID = data->msgID_infoE;  
    news[1].msgStrID = data->msgID_infoF;  
    news[2].msgStrID = data->msgID_infoG;  
    break;
  case RTC_WEEK_THURSDAY:  
    news[0].msgStrID = data->msgID_infoB;  
    news[1].msgStrID = data->msgID_infoC;  
    news[2].msgStrID = data->msgID_infoD;  
    break;
  case RTC_WEEK_FRIDAY:    
    news[0].msgStrID = data->msgID_infoF;  
    news[1].msgStrID = data->msgID_infoG;  
    news[2].msgStrID = data->msgID_infoH;  
    break;
  case RTC_WEEK_SATURDAY:  
    news[0].msgStrID = data->msgID_infoC;  
    news[1].msgStrID = data->msgID_infoD;  
    news[2].msgStrID = data->msgID_infoE;  
    break;
  }

  news[0].animeIndex = news_anm_index[NEWS_INFO_A];
  news[0].texName    = news_tex_name[NEWS_INFO_A];
  news[0].pltName    = news_plt_name[NEWS_INFO_A];
  news[0].msgArcID   = ARCID_MESSAGE;
  news[0].msgDatID   = data->arcDatID;

  news[1].animeIndex = news_anm_index[NEWS_INFO_B];
  news[1].texName    = news_tex_name[NEWS_INFO_B];
  news[1].pltName    = news_plt_name[NEWS_INFO_B];
  news[1].msgArcID   = ARCID_MESSAGE;
  news[1].msgDatID   = data->arcDatID;

  news[2].animeIndex = news_anm_index[NEWS_INFO_C];
  news[2].texName    = news_tex_name[NEWS_INFO_C];
  news[2].pltName    = news_plt_name[NEWS_INFO_C];
  news[2].msgArcID   = ARCID_MESSAGE;
  news[2].msgDatID   = data->arcDatID;

  // ニュースを追加
  for( i=0; i<LOCAL_INFO_NUM; i++ )
  {
    GOBJ_ELBOARD_AddNews( elboard, &news[i] );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @breif 掲示板のニュースを追加する(一言CM)
 *
 * @param elboard 追加する掲示板
 */
//------------------------------------------------------------------------------------------
static void AddNews_CM( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data )
{
  NEWS_PARAM news;
  RTCDate date;

  // 日時データを取得
  RTC_GetDate( &date );

  // 曜日に応じたCMを選択
  switch( date.week )
  {
  case RTC_WEEK_SUNDAY:    news.msgStrID = data->msgID_cmSun;  break;
  case RTC_WEEK_MONDAY:    news.msgStrID = data->msgID_cmMon;  break;
  case RTC_WEEK_TUESDAY:   news.msgStrID = data->msgID_cmTue;  break;
  case RTC_WEEK_WEDNESDAY: news.msgStrID = data->msgID_cmWed;  break;
  case RTC_WEEK_THURSDAY:  news.msgStrID = data->msgID_cmThu;  break;
  case RTC_WEEK_FRIDAY:    news.msgStrID = data->msgID_cmFri;  break;
  case RTC_WEEK_SATURDAY:  news.msgStrID = data->msgID_cmSat;  break;
  }
  news.animeIndex = news_anm_index[NEWS_CM];
  news.texName    = news_tex_name[NEWS_CM];
  news.pltName    = news_plt_name[NEWS_CM];
  news.msgArcID   = ARCID_MESSAGE;
  news.msgDatID   = data->arcDatID;
  
  // ニュースを追加
  GOBJ_ELBOARD_AddNews( elboard, &news );
}
