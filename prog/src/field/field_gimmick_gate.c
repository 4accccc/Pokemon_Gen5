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
#include "field_gimmick_gate.h" 
#include "arc/arc_def.h" 
#include "arc/gate.naix"
#include "arc/message.naix"

#include "field/gimmick_obj_elboard.h"
#include "savedata/gimmickwork.h"
#include "gimmick_obj_elboard.h"
#include "elboard_zone_data.h"
#include "elboard_spnews_data.h"
#include "arc/gate.naix"
#include "field_gimmick_def.h"
#include "../../../resource/fldmapdata/zonetable/zone_id.h"
#include "msg/msg_gate.h"
#include "field/zonedata.h"
#include "field/enc_pokeset.h"  // for ENCPOKE_GetGenerateZone

#include "field_task_manager.h"
#include "field_task.h"
#include "field_task_camera_zoom.h"
#include "field_task_camera_rot.h"
#include "field_task_target_offset.h"


//==========================================================================================
// ■電光掲示板データ登録テーブル
//==========================================================================================
typedef struct
{
  u16 zone_id;
  int gimmick_id;
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

//------------------------------------------------------------------------------------------
/**
 * @brief 指定したギミックIDが登録されているかどうか判定する
 *
 * @param gmk_id 判定対象ギミックID
 *
 * @return 登録されている場合 TRUE
 */
//------------------------------------------------------------------------------------------
static BOOL IsGimmickIDEntried( int gmk_id )
{
  int i;
  for( i=0; i<NELEMS(entry_table); i++ )
  {
    if( entry_table[i].gimmick_id == gmk_id )
    {
      return TRUE;  // 発見
    }
  }
  return FALSE; // 未発見
}


//==========================================================================================
// ■定数
//==========================================================================================
// ニュース番号
typedef enum
{
  NEWS_DATE,         // 日時
  NEWS_WEATHER,      // 天気
  NEWS_PROPAGATION,  // 大量発生
  NEWS_INFO_A,       // 情報A
  NEWS_INFO_B,       // 情報B
  NEWS_INFO_C,       // 情報C
  NEWS_CM,           // 一言CM
  NEWS_NUM,
  NEWS_MAX = NEWS_NUM -1
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
  RES_ELBOARD_NSBTA_1,  // ニュース・スクロール・アニメーション1
  RES_ELBOARD_NSBTA_2,  // ニュース・スクロール・アニメーション2
  RES_ELBOARD_NSBTA_3,  // ニュース・スクロール・アニメーション3
  RES_ELBOARD_NSBTA_4,  // ニュース・スクロール・アニメーション4
  RES_ELBOARD_NSBTA_5,  // ニュース・スクロール・アニメーション5
  RES_ELBOARD_NSBTA_6,  // ニュース・スクロール・アニメーション6
  RES_ELBOARD_NSBTA_7,  // ニュース・スクロール・アニメーション7
  RES_MONITOR_NSBTP_1,  // モニター・テクスチャパターン・アニメーション1
  RES_MONITOR_NSBTP_2,  // モニター・テクスチャパターン・アニメーション2
  RES_MONITOR_NSBTP_3,  // モニター・テクスチャパターン・アニメーション3
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_nsbtx, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_1_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_2_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_3_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_4_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_5_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_6_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_7_nsbta, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_tv01_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_tv02_nsbtp, GFL_G3D_UTIL_RESARC },
  { ARCID_GATE_GIMMICK, NARC_gate_gelboard01_tv03_nsbtp, GFL_G3D_UTIL_RESARC },
};

// アニメインデックス
typedef enum
{
  ANM_ELBOARD_DATE,         // 掲示板ニュース・スクロール・日付
  ANM_ELBOARD_WEATHER,      // 掲示板ニュース・スクロール・天気
  ANM_ELBOARD_PROPAGATION,  // 掲示板ニュース・スクロール・大量発生
  ANM_ELBOARD_INFO_A,       // 掲示板ニュース・スクロール・情報A
  ANM_ELBOARD_INFO_B,       // 掲示板ニュース・スクロール・情報B
  ANM_ELBOARD_INFO_C,       // 掲示板ニュース・スクロール・情報C
  ANM_ELBOARD_CM,           // 掲示板ニュース・スクロール・一言CM
  ANM_MONITOR_1,            // モニター・テクスチャ・アニメーション1
  ANM_MONITOR_2,            // モニター・テクスチャ・アニメーション2
  ANM_MONITOR_3,            // モニター・テクスチャ・アニメーション3
  ANM_NUM
} ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table[ANM_NUM] = 
{
  // アニメリソースID, アニメデータID(リソース内部INDEX)
  { RES_ELBOARD_NSBTA_1, 0 },
  { RES_ELBOARD_NSBTA_2, 0 },
  { RES_ELBOARD_NSBTA_3, 0 },
  { RES_ELBOARD_NSBTA_4, 0 },
  { RES_ELBOARD_NSBTA_5, 0 },
  { RES_ELBOARD_NSBTA_6, 0 },
  { RES_ELBOARD_NSBTA_7, 0 },
  { RES_MONITOR_NSBTP_1, 0 },
  { RES_MONITOR_NSBTP_2, 0 },
  { RES_MONITOR_NSBTP_3, 0 },
};

// オブジェクトインデックス
typedef enum
{
  OBJ_ELBOARD,  // 電光掲示板
  OBJ_NUM
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
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
  UNIT_ELBOARD, // 電光掲示板 + モニター
  UNIT_NUM
} UNIT_INDEX;
static const GFL_G3D_UTIL_SETUP unit[UNIT_NUM] =
{
  { res_table, RES_NUM, obj_table, OBJ_NUM },
};


//==========================================================================================
// ■ニュースパラメータ
//==========================================================================================
// スクロール・アニメーション・インデックス
static u16 news_anm_index[NEWS_NUM] = 
{
  ANM_ELBOARD_DATE,        // 日付
  ANM_ELBOARD_WEATHER,     // 天気
  ANM_ELBOARD_PROPAGATION, // 大量発生
  ANM_ELBOARD_INFO_A,      // 情報A
  ANM_ELBOARD_INFO_B,      // 情報B
  ANM_ELBOARD_INFO_C,      // 情報C
  ANM_ELBOARD_CM,          // 一言CM
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
  "gelboard_7",
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
  "gelboard_7_pl",
};
// 天気に使用するメッセージ
u32 str_id_weather[WEATHER_NO_NUM] = 
{
  msg_gate_sunny,     // 晴れ
  msg_gate_snow,      // 雪
  msg_gate_rain,      // 雨
  msg_gate_storm,     // 砂嵐
  msg_gate_spark,     // 雷雨
  msg_gate_snowstorm, // 吹雪
  msg_gate_arare,     // 霰
  msg_gate_mirage,    // 蜃気楼
};


//==========================================================================================
// ■モニターパラメータ
//==========================================================================================
// アニメーション番号
typedef enum {
  MONITOR_ANIME_1,
  MONITOR_ANIME_2,
  MONITOR_ANIME_3,
  MONITOR_ANIME_NUM,
  MONITOR_ANIME_MAX = MONITOR_ANIME_NUM - 1
} MONITOR_ANIME_INDEX;

// 実アニメーション番号
static u16 monitor_anime[MONITOR_ANIME_NUM] = 
{ 
  ANM_MONITOR_1,
  ANM_MONITOR_2,
  ANM_MONITOR_3,
};


//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct
{ 
  HEAPID                   heapID;  // 使用するヒープID
  FIELDMAP_WORK*         fieldmap;  // フィールドマップ
  GOBJ_ELBOARD*           elboard;  // 電光掲示板管理オブジェクト
  u32               recoveryFrame;  // 復帰フレーム
  u8                spNewsDataNum;  // 臨時ニュースデータ数
  ELBOARD_ZONE_DATA*     gateData;  // ゲートデータ
  ELBOARD_SPNEWS_DATA* spNewsData;  // 臨時ニュースデータ
} GATEWORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void GimmickSave( FIELDMAP_WORK* fieldmap );
static void GimmickLoad( GATEWORK* work, FIELDMAP_WORK* fieldmap );
static GATEWORK* CreateGateWork( FIELDMAP_WORK* fieldmap );
static void DeleteGateWork( GATEWORK* work );
static BOOL LoadGateData( GATEWORK* work, FIELDMAP_WORK* fieldmap );
static void DeleteGateData( GATEWORK* work );
static void LoadSpNewsData( GATEWORK* work );
static void DeleteSpNewsData( GATEWORK* work );
static void SetElboardPos( GFL_G3D_OBJSTATUS* status, ELBOARD_ZONE_DATA* data );
static void SetupElboardNews( GATEWORK* work );
static void SetupElboardSpecialNews( GATEWORK* work );
static BOOL CheckSpecialNews( GATEWORK* work );
static void AddNews_DATE( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data );
static void AddNews_PROPAGATION( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data, 
                                 const GAMEDATA* gdata );
static void AddNews_WEATHER( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data );
static void AddNews_INFO( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data );
static void AddNews_CM( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data );
static void AddNews_DIRECT( GOBJ_ELBOARD* elboard, const ELBOARD_SPNEWS_DATA* data );
static void AddNews_CHAMP( GOBJ_ELBOARD* elboard, const ELBOARD_SPNEWS_DATA* data );
static void AddSpNews( GOBJ_ELBOARD* elboard, 
                       const ELBOARD_SPNEWS_DATA* data_array, u8 data_num );


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
  HEAPID                heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  GAMESYS_WORK*            gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*               gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*          gmkwork = GAMEDATA_GetGimmickWork(gdata);
  GATEWORK*                work = NULL;  // GATEギミック管理ワーク

  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &unit[UNIT_ELBOARD], EXPOBJ_UNIT_ELBOARD );

  // ギミック管理ワークを作成
  work = CreateGateWork( fieldmap );

  // ギミックのセーブデータを読み込む
  GimmickLoad( work, fieldmap ); 
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  int            gmk_id = GIMMICKWORK_GetAssignID( gmkwork );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, gmk_id );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // ギミック状態をセーブ
  GimmickSave( fieldmap );
  
  // 電光掲示板管理ワークを破棄
  if( work->elboard )
  {
    GOBJ_ELBOARD_Delete( work->elboard );
  }

  // ギミック管理ワークを破棄
  DeleteGateWork( work );
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
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  int            gmk_id = GIMMICKWORK_GetAssignID( gmkwork );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, gmk_id );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 電光掲示板メイン処理
  GOBJ_ELBOARD_Main( work->elboard, FX32_ONE ); 

  // モニターアニメーション再生
  {
    FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
    FLD_EXP_OBJ_PlayAnime( exobj_cnt );
  }

#if 0
  // TEST:
  {
    int key = GFL_UI_KEY_GetCont();
    int trg = GFL_UI_KEY_GetTrg();
    if( key & PAD_BUTTON_L )
    {
      if( trg & PAD_BUTTON_A )
      { 
        GATE_GIMMICK_Camera_LookElboard( fieldmap, 30 );
      }
      if( trg & PAD_BUTTON_B )
      { 
        GATE_GIMMICK_Camera_Reset( fieldmap, 30 );
      }
    }
  }
#endif
}


//==========================================================================================
// ■ 掲示板
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief 掲示板に平常通りのニュースを設定する
 *
 * @param fieldmap フィールドマップ
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_Elboard_SetupNormalNews( FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork( gdata );
  int            gmk_id = GIMMICKWORK_GetAssignID( gmkwork );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, gmk_id );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 電光掲示板が登録されていない場所で呼ばれたら何もしない
  if( IsGimmickIDEntried( gmk_id ) != TRUE ){ return; }

  // すでに他のニュースが登録されている場合
  {
    int num = GOBJ_ELBOARD_GetNewsNum( work->elboard );
    if( num != 0 )
    {
      OBATA_Printf( "======================================\n" );
      OBATA_Printf( "すでに他のニュースが設定されています。\n" );
      OBATA_Printf( "======================================\n" );
      return;
    }
  } 
  // データが取得できなかったら, 以降の処理を中断
  if( !work->gateData ){ return; }

  // ニュースを追加
  AddNews_DATE( work->elboard, work->gateData );                // 日付
  AddNews_WEATHER( work->elboard, work->gateData );             // 天気
  AddNews_PROPAGATION( work->elboard, work->gateData, gdata );  // 大量発生
  AddNews_INFO( work->elboard, work->gateData );                // 地域情報
  AddNews_CM( work->elboard, work->gateData );                  // 一言CM
}

//------------------------------------------------------------------------------------------
/**
 * @brief 掲示板に指定したニュースを追加する
 *
 * @param fieldmap フィールドマップ
 * @param str_id   追加するニュースのメッセージ番号
 * @param wordset  指定メッセージに展開するワードセット
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_Elboard_AddSpecialNews( FIELDMAP_WORK* fieldmap, 
                                          u32 str_id, WORDSET* wordset )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork( gdata );
  int            gmk_id = GIMMICKWORK_GetAssignID( gmkwork );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, gmk_id );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 電光掲示板が登録されていない場所で呼ばれたら何もしない
  if( IsGimmickIDEntried( gmk_id ) != TRUE ){ return; }

  // ニュースを追加
  {
    int index;
    NEWS_PARAM news;

    index = GOBJ_ELBOARD_GetNewsNum( work->elboard );
    if( MAX_NEWS_NUM <= index )
    {
      OBATA_Printf( "==========================================\n" );
      OBATA_Printf( "すでに最大数のニュースが設定されています。\n" );
      OBATA_Printf( "==========================================\n" );
      return;
    }
    news.animeIndex = news_anm_index[index];
    news.texName    = news_tex_name[index];
    news.pltName    = news_plt_name[index];
    news.msgArcID   = ARCID_MESSAGE;
    news.msgDatID   = NARC_message_gate_dat;
    news.msgStrID   = str_id;
    news.wordset    = wordset;
    GOBJ_ELBOARD_AddNews( work->elboard, &news );
  }

}

//------------------------------------------------------------------------------------------
/**
 * @brief 掲示板の状態を復帰させる(復帰ポイントは自動的に記憶)
 *
 * @param fieldmap フィールドマップ
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_Elboard_Recovery( FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork( gdata );
  int            gmk_id = GIMMICKWORK_GetAssignID( gmkwork );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, gmk_id );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 電光掲示板が登録されていない場所で呼ばれたら何もしない
  if( IsGimmickIDEntried( gmk_id ) != TRUE )
  {
    return;
  } 
  // 掲示板復帰処理
  GOBJ_ELBOARD_SetFrame( work->elboard, work->recoveryFrame << FX32_SHIFT );
}


//==========================================================================================
// ■カメラ
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief カメラを電光掲示板に向ける
 *
 * @param fieldmap フィールドマップ
 * @param frame    カメラの動作に要するフレーム数
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_Camera_LookElboard( FIELDMAP_WORK* fieldmap, u16 frame )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork( gdata );
  int            gmk_id = GIMMICKWORK_GetAssignID( gmkwork );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, gmk_id );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス
  fx32 val_len;
  u16 val_pitch, val_yaw;
  VecFx32 val_target;

  // 電光掲示板データを取得
  if( !work->gateData )
  { // 取得失敗
    OBATA_Printf( "==========================================\n" );
    OBATA_Printf( "GIMMICK-GATE: 電光掲示板データの取得に失敗\n" );
    OBATA_Printf( "==========================================\n" );
    return;
  }
  // 電光掲示板の向きでカメラの回転を決定
  switch( work->gateData->dir )
  {
  case DIR_DOWN:
    val_pitch = 0x0ee5;
    val_yaw   = 0;
    val_len   = 0x0086 << FX32_SHIFT;
    VEC_Set( &val_target, 0, 0x001b<<FX32_SHIFT, 0xfff94000 );
    break;
  case DIR_RIGHT: 
    val_pitch = 0x1ad3;
    val_yaw   = 0x3f5d;
    val_len   = 0x0058 << FX32_SHIFT;
    VEC_Set( &val_target, 0xfff5d000, 0x001a<<FX32_SHIFT, 0xfffff000 );
    break;
  case DIR_UP:
  case DIR_LEFT:
  default:
    OBATA_Printf( "==================================\n" );
    OBATA_Printf( "GIMMICK-GATE: 掲示板の向きが未対応\n" );
    OBATA_Printf( "==================================\n" );
    return;
  } 
  // タスク登録
  {
    FIELD_TASK_MAN* man;
    FIELD_TASK* zoom;
    FIELD_TASK* pitch;
    FIELD_TASK* yaw;
    FIELD_TASK* target;
    zoom   = FIELD_TASK_CameraLinearZoom( fieldmap, frame, val_len );
    pitch  = FIELD_TASK_CameraRot_Pitch( fieldmap, frame, val_pitch );
    yaw    = FIELD_TASK_CameraRot_Yaw( fieldmap, frame, val_yaw );
    target = FIELD_TASK_CameraTargetOffset( fieldmap, frame, &val_target );
    man = FIELDMAP_GetTaskManager( fieldmap );
    FIELD_TASK_MAN_AddTask( man, zoom, NULL );
    FIELD_TASK_MAN_AddTask( man, pitch, NULL );
    FIELD_TASK_MAN_AddTask( man, yaw, NULL );
    FIELD_TASK_MAN_AddTask( man, target, NULL );
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief カメラを元に戻す
 *
 * @param fieldmap フィールドマップ
 * @param frame    カメラの動作に要するフレーム数
 */
//------------------------------------------------------------------------------------------
void GATE_GIMMICK_Camera_Reset( FIELDMAP_WORK* fieldmap, u16 frame )
{
  fx32 val_len;
  u16 val_pitch, val_yaw;
  VecFx32 val_target = {0, 0, 0};
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( fieldmap );
  FLD_CAMERA_PARAM def_param;
  // デフォルトパラメータ取得
  FIELD_CAMERA_GetInitialParameter( camera, &def_param );
  val_len   = def_param.Distance << FX32_SHIFT;
  val_pitch = def_param.Angle.x;
  val_yaw   = def_param.Angle.y;
  // タスク登録
  {
    FIELD_TASK_MAN* man;
    FIELD_TASK* zoom;
    FIELD_TASK* pitch;
    FIELD_TASK* yaw;
    FIELD_TASK* target;
    zoom   = FIELD_TASK_CameraLinearZoom( fieldmap, frame, val_len );
    pitch  = FIELD_TASK_CameraRot_Pitch( fieldmap, frame, val_pitch );
    yaw    = FIELD_TASK_CameraRot_Yaw( fieldmap, frame, val_yaw );
    target = FIELD_TASK_CameraTargetOffset( fieldmap, frame, &val_target );
    man = FIELDMAP_GetTaskManager( fieldmap );
    FIELD_TASK_MAN_AddTask( man, zoom, NULL );
    FIELD_TASK_MAN_AddTask( man, pitch, NULL );
    FIELD_TASK_MAN_AddTask( man, yaw, NULL );
    FIELD_TASK_MAN_AddTask( man, target, NULL );
  }
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
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  int            gmk_id = GIMMICKWORK_GetAssignID( gmkwork );
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, gmk_id );
  GATEWORK*        work = (GATEWORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス

  // 掲示板の動作フレーム数を記憶
  gmk_save[1] = GOBJ_ELBOARD_GetFrame( work->elboard ) >> FX32_SHIFT; // 掲示板の復帰ポイント
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックのセーブデータを読み込む
 *
 * @param work     ギミック管理ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void GimmickLoad( GATEWORK* work, FIELDMAP_WORK* fieldmap )
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  int            gmk_id = GIMMICKWORK_GetAssignID( gmkwork );
  u32*         gmk_save = NULL;

  // セーブデータ取得
  gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, gmk_id );
  work->recoveryFrame = gmk_save[1];  // 掲示板の復帰ポイント

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
  GATEWORK*                work = NULL;
  HEAPID                heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );

  // ギミック管理ワークを作成
  work = (GATEWORK*)GFL_HEAP_AllocMemory( heap_id, sizeof(GATEWORK) );
  GFL_STD_MemClear( work, sizeof(GATEWORK) );

  // 初期化
  work->heapID   = heap_id;
  work->fieldmap = fieldmap;

  // 電光掲示板管理ワークを作成
  {
    ELBOARD_PARAM param;
    param.heapID       = heap_id;
    param.maxNewsNum   = NEWS_NUM;
    param.dispSize     = DISPLAY_SIZE;
    param.newsInterval = NEWS_INTERVAL;
    param.g3dObj       = FLD_EXP_OBJ_GetUnitObj( exobj_cnt, EXPOBJ_UNIT_ELBOARD, OBJ_ELBOARD );
    work->elboard      = GOBJ_ELBOARD_Create( &param );
  } 

  // 電光掲示板データを取得
  if( !LoadGateData( work, fieldmap ) )
  {
    // データが取得できなかったら, 以降の初期化を中断
    return work;
  }
  // 臨時ニュースデータを取得
  LoadSpNewsData( work );

  // 電光掲示板を配置
  { 
    GFL_G3D_OBJSTATUS* status = 
      FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_ELBOARD, OBJ_ELBOARD );
    SetElboardPos( status, work->gateData );
  } 

  // モニター・アニメーション開始
  FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, EXPOBJ_UNIT_ELBOARD, OBJ_ELBOARD, 
                           monitor_anime[work->gateData->monitorAnimeIndex], TRUE ); 

  return work;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミック管理ワークを破棄する
 *
 * @param work 破棄するワーク
 */
//------------------------------------------------------------------------------------------
static void DeleteGateWork( GATEWORK* work )
{
  DeleteGateData( work );      // ゲートデータ
  DeleteSpNewsData( work );    // 臨時ニュースデータ
  GFL_HEAP_FreeMemory( work ); // 本体
}

//------------------------------------------------------------------------------------------
/**
 * @brief ゾーンに応じた電光掲示板データを取得する
 *
 * @param work     読み込んだデータを保持するワーク
 * @param fieldmap フィールドマップ
 *
 * @return データが正しく読み込めたら TRUE
 */
//------------------------------------------------------------------------------------------
static BOOL LoadGateData( GATEWORK* work, FIELDMAP_WORK* fieldmap )
{
  int i;
  u16 zone_id;

  // すでに読み込まれている
  if( work->gateData ){ return FALSE; }

  // ゾーンIDを取得
  zone_id = FIELDMAP_GetZoneID( fieldmap );

  // 電光掲示板データテーブルから, 該当するデータを検索
  for( i=0; i<NELEMS(entry_table); i++ )
  {
    if( entry_table[i].zone_id == zone_id )
    {
      // 電光掲示板データを読み込む
      work->gateData = GFL_HEAP_AllocMemory( work->heapID, sizeof(ELBOARD_ZONE_DATA) );
      ELBOARD_ZONE_DATA_Load( work->gateData, ARCID_GATE_GIMMICK, entry_table[i].dat_id );
      return TRUE;
    }
  } 

  // 現在のゾーンに対応する電光掲示板データが登録されていない場合
  OBATA_Printf( "=============================================\n" );
  OBATA_Printf( "error: 電光掲示板データが登録されていません。\n" );
  OBATA_Printf( "=============================================\n" );
  return FALSE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ゲートデータを破棄する
 *
 * @param work 破棄するゲートデータを保持するワーク
 */
//------------------------------------------------------------------------------------------
static void DeleteGateData( GATEWORK* work )
{
  if( work->gateData )
  {
    GFL_HEAP_FreeMemory( work->gateData );
    work->gateData = NULL;
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief 臨時ニュースデータを読み込む
 *
 * @param work 読み込んだデータを保持するワーク
 */
//------------------------------------------------------------------------------------------
static void LoadSpNewsData( GATEWORK* work )
{

  // すでに読み込まれている
  if( work->spNewsData ){ return; }

  // 全データを読み込み
  {
    int data_idx;
    int data_num;
    // データ数取得
    data_num = GFL_ARC_GetDataFileCnt( ARCID_ELBOARD_SPNEWS );
    // 各データを取得
    work->spNewsData = GFL_HEAP_AllocMemory( work->heapID, 
                                             sizeof(ELBOARD_SPNEWS_DATA) * data_num );
    for( data_idx=0; data_idx<data_num; data_idx++ )
    {
      ELBOARD_SPNEWS_DATA_Load( &work->spNewsData[data_idx], ARCID_ELBOARD_SPNEWS, data_idx );
    }
    work->spNewsDataNum = data_num;
  }
}

//------------------------------------------------------------------------------------------
/**
 * @brief 臨時ニュースデータを破棄する
 *
 * @param 破棄するデータを保持するワーク
 */
//------------------------------------------------------------------------------------------
static void DeleteSpNewsData( GATEWORK* work )
{
  if( work->spNewsData )
  {
    GFL_HEAP_FreeMemory( work->spNewsData );
    work->spNewsData = NULL;
    work->spNewsDataNum = 0;
  }
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
  rot *= 182;  // 65536÷360 = 182.044...
  GFL_CALC3D_MTX_CreateRot( 0, rot, 0, &status->rotate );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 電光掲示板 ニュースセットアップ
 *
 * @param work ギミック管理ワーク
 */
//------------------------------------------------------------------------------------------
static void SetupElboardNews( GATEWORK* work )
{
}

//------------------------------------------------------------------------------------------
/**
 * @brief 掲示板に臨時ニュースを設定する
 *
 * @param work ギミック管理ワーク
 */
//------------------------------------------------------------------------------------------
static void SetupElboardSpecialNews( GATEWORK* work )
{ 
  GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( work->fieldmap );
  GAMEDATA*    gdata = GAMESYSTEM_GetGameData( gsys );

  // すでに他のニュースが登録されている場合
  {
    int num = GOBJ_ELBOARD_GetNewsNum( work->elboard );
    if( num != 0 )
    {
      OBATA_Printf( "======================================\n" );
      OBATA_Printf( "すでに他のニュースが設定されています。\n" );
      OBATA_Printf( "======================================\n" );
      return;
    }
  } 
  // データを持っていない
  if( !work->spNewsData ){ return; }

  // ニュースを追加
  AddNews_DATE( work->elboard, work->gateData );                // 日付
  AddNews_WEATHER( work->elboard, work->gateData );             // 天気
  AddNews_PROPAGATION( work->elboard, work->gateData, gdata );  // 大量発生
  //AddNews_SPECIAL( work->elboard, work->gateData );             // 臨時ニュース
  AddNews_CM( work->elboard, work->gateData );                  // 一言CM
}

//------------------------------------------------------------------------------------------
/**
 * @brief 臨時ニュースの有無を調べる
 *
 * @param work ギミック管理ワーク
 *
 * @return 表示すべき臨時ニュースがある場合 TRUE, そうでなければ FALSE
 */
//------------------------------------------------------------------------------------------
static BOOL CheckSpecialNews( GATEWORK* work )
{
  int i;
  u32        zone_id = FIELDMAP_GetZoneID( work->fieldmap );
  GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( work->fieldmap );
  GAMEDATA*    gdata = GAMESYSTEM_GetGameData( gsys );
  EVENTWORK*  evwork = GAMEDATA_GetEventWork( gdata );

  // 臨時ニュースデータを検索
  for( i=0; i<work->spNewsDataNum; i++ )
  { 
    BOOL flag_hit = EVENTWORK_CheckEventFlag( evwork, work->spNewsData[i].flag );
    BOOL zone_hit = ELBOARD_SPNEWS_DATA_CheckZoneHit( &work->spNewsData[i], zone_id );
    if( flag_hit && zone_hit )  // if(フラグON && ゾーン一致)
    {
      return TRUE;
    }
  }
  return FALSE;
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
  HEAPID heap_id; 
  NEWS_PARAM news;
  WORDSET* wordset;
  RTCDate date;

  // ワードセット作成
  heap_id = GOBJ_ELBOARD_GetHeapID( elboard );
  wordset = WORDSET_Create( heap_id );
  RTC_GetDate( &date );
  WORDSET_RegisterNumber( wordset, 0, date.month, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterNumber( wordset, 1, date.day,   2, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );

  // ニュースパラメータを作成
  news.animeIndex = news_anm_index[NEWS_DATE];
  news.texName    = news_tex_name[NEWS_DATE];
  news.pltName    = news_plt_name[NEWS_DATE];
  news.msgArcID   = ARCID_MESSAGE;
  news.msgDatID   = NARC_message_gate_dat;
  news.msgStrID   = data->msgID_date;
  news.wordset    = wordset;

  // ニュースを追加
  GOBJ_ELBOARD_AddNews( elboard, &news );

  // ワードセット破棄
  WORDSET_Delete( wordset );
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
  int i;
  HEAPID heap_id;
  NEWS_PARAM news;
  WORDSET* wordset;
  u32 zone_id[WEATHER_ZONE_NUM];

  // 表示するゾーンリストを作成
  zone_id[0] = data->zoneID_weather_1;
  zone_id[1] = data->zoneID_weather_2;
  zone_id[2] = data->zoneID_weather_3;
  zone_id[3] = data->zoneID_weather_4;

  for( i=0; i<WEATHER_ZONE_NUM; i++ )
  {
    OBATA_Printf( "zone_id[%d] = %d\n", i, zone_id[i] );
  }

  // ワードセット作成
  heap_id = GOBJ_ELBOARD_GetHeapID( elboard );
  wordset = WORDSET_CreateEx( WEATHER_ZONE_NUM, 256, heap_id );

  // ワードセット登録処理
  {
    GFL_MSGDATA* msg_place_name;
    GFL_MSGDATA* msg_gate;

    // メッセージデータ ハンドルオープン
    msg_place_name = GFL_MSG_Create( 
        GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_place_name_dat, heap_id ); 
    msg_gate = GFL_MSG_Create( 
        GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_gate_dat, heap_id ); 

    // 各ゾーンの地名＋天気を登録
    for( i=0; i<WEATHER_ZONE_NUM; i++ )
    {
      STRBUF* strbuf_zone;    // 地名
      STRBUF* strbuf_weather; // 天気
      STRBUF* strbuf_set;     // 地名＋天気

      // ゾーンが無効値なら表示しない
      if( zone_id[i] == ZONE_ID_MAX ) 
      {
        continue;
      }
      // 地名を取得
      {
        int str_id = ZONEDATA_GetPlaceNameID( zone_id[i] );
        strbuf_zone = GFL_MSG_CreateString( msg_place_name, str_id );
        OBATA_Printf( "str_id = %d\n", str_id );
      }
      // 天気を取得
      {
        int weather = ZONEDATA_GetWeatherID( zone_id[i] );
        strbuf_weather = GFL_MSG_CreateString( msg_gate, str_id_weather[weather] );
        OBATA_Printf( "weather = %d\n", weather );
      }
      // 地名＋天気のセットを作成
      {
        WORDSET* wordset_expand = WORDSET_CreateEx( 2, 256, heap_id );
        STRBUF* strbuf_expand = GFL_MSG_CreateString( msg_gate, msg_gate_weather );
        strbuf_set = GFL_STR_CreateBuffer( 64, heap_id );
        WORDSET_RegisterWord( wordset_expand, 0, strbuf_zone, 0, TRUE, 0 );
        WORDSET_RegisterWord( wordset_expand, 1, strbuf_weather, 0, TRUE, 0 );
        WORDSET_ExpandStr( wordset_expand, strbuf_set, strbuf_expand );
        GFL_STR_DeleteBuffer( strbuf_expand );
        WORDSET_Delete( wordset_expand );
      }
      // 作成した文字列をワードセットに登録
      WORDSET_RegisterWord( wordset, i, strbuf_set, 0, TRUE, 0 );
      // 後始末
      GFL_STR_DeleteBuffer( strbuf_zone );
      GFL_STR_DeleteBuffer( strbuf_weather );
      GFL_STR_DeleteBuffer( strbuf_set );
    }

    // メッセージデータ ハンドルクローズ
    GFL_MSG_Delete( msg_gate );
    GFL_MSG_Delete( msg_place_name );
  }

  // ニュースパラメータを作成
  news.animeIndex = news_anm_index[NEWS_WEATHER];
  news.texName    = news_tex_name[NEWS_WEATHER];
  news.pltName    = news_plt_name[NEWS_WEATHER];
  news.msgArcID   = ARCID_MESSAGE;
  news.msgDatID   = NARC_message_gate_dat;
  news.msgStrID   = data->msgID_weather;
  news.wordset    = wordset;

  OBATA_Printf( "msgStrID = %d\n", data->msgID_weather );

  // ニュースを追加
  GOBJ_ELBOARD_AddNews( elboard, &news );

  // ワードセット破棄
  WORDSET_Delete( wordset );
}

//------------------------------------------------------------------------------------------
/**
 * @breif 掲示板のニュースを追加する(大量発生)
 *
 * @param elboard 追加する掲示板
 * @param data    電光掲示板データ
 * @param gdata   ゲームデータ
 */
//------------------------------------------------------------------------------------------
static void AddNews_PROPAGATION( GOBJ_ELBOARD* elboard, const ELBOARD_ZONE_DATA* data, 
                                 const GAMEDATA* gdata )
{
  HEAPID heap_id; 
  NEWS_PARAM news;
  WORDSET* wordset;
  u16 zone_id;

  // 大量発生が起きているゾーンを取得
  zone_id = ENCPOKE_GetGenerateZone( gdata ); 
  // 大量発生が起きていない
  if( zone_id == 0xFFFF ){ return; }

  // ワードセット作成
  heap_id = GOBJ_ELBOARD_GetHeapID( elboard );
  wordset = WORDSET_Create( heap_id );

  // ワードセットに地名をセット
  {
    GFL_MSGDATA* msg_place_name;
    int str_id;
    STRBUF* zone_name;

    msg_place_name = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
                                     ARCID_MESSAGE, NARC_message_place_name_dat, heap_id ); 
    str_id         = ZONEDATA_GetPlaceNameID( zone_id );
    zone_name      = GFL_MSG_CreateString( msg_place_name, str_id );
    WORDSET_RegisterWord( wordset, 0, zone_name, 0, TRUE, 0 );
    GFL_STR_DeleteBuffer( zone_name );
    GFL_MSG_Delete( msg_place_name );
  }

  // ニュースパラメータを作成
  news.animeIndex = news_anm_index[NEWS_PROPAGATION];
  news.texName    = news_tex_name[NEWS_PROPAGATION];
  news.pltName    = news_plt_name[NEWS_PROPAGATION];
  news.msgArcID   = ARCID_MESSAGE;
  news.msgDatID   = NARC_message_gate_dat;
  news.msgStrID   = data->msgID_propagation;
  news.wordset    = wordset;

  // ニュースを追加
  GOBJ_ELBOARD_AddNews( elboard, &news );

  // ワードセット破棄
  WORDSET_Delete( wordset );
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
  news[0].msgDatID   = NARC_message_gate_dat;
  news[0].wordset    = NULL;

  news[1].animeIndex = news_anm_index[NEWS_INFO_B];
  news[1].texName    = news_tex_name[NEWS_INFO_B];
  news[1].pltName    = news_plt_name[NEWS_INFO_B];
  news[1].msgArcID   = ARCID_MESSAGE;
  news[1].msgDatID   = NARC_message_gate_dat;
  news[1].wordset    = NULL;

  news[2].animeIndex = news_anm_index[NEWS_INFO_C];
  news[2].texName    = news_tex_name[NEWS_INFO_C];
  news[2].pltName    = news_plt_name[NEWS_INFO_C];
  news[2].msgArcID   = ARCID_MESSAGE;
  news[2].msgDatID   = NARC_message_gate_dat;
  news[2].wordset    = NULL;

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
  news.msgDatID   = NARC_message_gate_dat;
  news.wordset    = NULL;
  
  // ニュースを追加
  GOBJ_ELBOARD_AddNews( elboard, &news );
} 
