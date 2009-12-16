////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ギミック処理(ポケモンリーグフロント下)
 * @file   field_gimmick_league_front02.c
 * @author obata
 * @date   2009.12.01
 */
//////////////////////////////////////////////////////////////////////////////////////////// 
#include "fieldmap.h"
#include "field_gimmick_league_front02.h"
#include "gamesystem/game_event.h"
#include "gamesystem/gamesystem.h"
#include "field_gimmick_def.h"  // for FLD_GIMMICK_C09P02
#include "arc/league_front.naix"  // for NARC_xxxx
#include "savedata/gimmickwork.h"  // for GIMMICKWORK
#include "system/ica_anime.h"  // for ICA_ANIME
#include "fld_exp_obj.h"  // for FLD_EXP_OBJ
#include "arc/league_front.naix"  // for NARC_xxxx
#include "field/field_const.h"  // for FIELD_CONST_GRID_SIZE
#include "event_fieldmap_control.h"  // for EVENT_FieldBrightIn


//==========================================================================================
// ■定数
//==========================================================================================
#define ARCID (ARCID_LEAGUE_FRONT_GIMMICK)  // ギミックデータのアーカイブID

// リフト座標
#define HALF_GRID ((FIELD_CONST_GRID_SIZE / 2) << FX32_SHIFT)
#define LIFT_POS_X_GRID (30)
#define LIFT_POS_Z_GRID (81)
#define LIFT_POS_X ((LIFT_POS_X_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define LIFT_POS_Y (2 << FX32_SHIFT)
#define LIFT_POS_Z ((LIFT_POS_Z_GRID * FIELD_CONST_GRID_SIZE) << FX32_SHIFT)
#define PLAYER_OFS_X (0)
#define PLAYER_OFS_Y (0)
#define PLAYER_OFS_Z ((FIELD_CONST_GRID_SIZE * 2) << FX32_SHIFT)

// ギミックワークのデータインデックス
typedef enum{
  GIMMICKWORK_DATA_WORK_ADRS,  // LF02ギミック管理ワークのアドレス
  GIMMICKWORK_DATA_NUM,
  GIMMICKWORK_DATA_MAX = GIMMICKWORK_DATA_NUM - 1
} GIMMICKWORK_DATA_INDEX;


//==========================================================================================
// ■3Dリソース
//==========================================================================================
//----------
// リソース
//----------
typedef enum{
  RES_LIFT_NSBMD,  // リフトのモデル
  RES_LIFT_NSBTA,  // リフトのテクスチャアニメーション
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  {ARCID, NARC_league_front_pl_ele_01_nsbmd, GFL_G3D_UTIL_RESARC},
  {ARCID, NARC_league_front_pl_ele_01_nsbta, GFL_G3D_UTIL_RESARC},
}; 
//------------------------
// アニメーション(リフト)
//------------------------
typedef enum{
  LIFT_ANM_TA,  // テクスチャアニメーション
  LIFT_ANM_NUM
} LIFT_ANM_INDEX;
static const GFL_G3D_UTIL_ANM anm_table_lift[LIFT_ANM_NUM] = 
{
  {RES_LIFT_NSBTA, 0},
}; 
//-------------
// オブジェクト
//-------------
typedef enum{
  OBJ_LIFT,  // リフト
  OBJ_NUM
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
{
  {RES_LIFT_NSBMD, 0, RES_LIFT_NSBMD, anm_table_lift, LIFT_ANM_NUM},  // リフト
}; 
//----------
// ユニット
//----------
typedef enum{
  UNIT_GIMMICK,
  UNIT_NUM
} UNIT_INDEX;
static const GFL_G3D_UTIL_SETUP unit[UNIT_NUM] = 
{ 
  {res_table, RES_NUM, obj_table, OBJ_NUM},
};


//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct
{ 
  HEAPID heapID;  // 使用するヒープID

} LF02WORK;


//==========================================================================================
// ■非公開関数のプロトタイプ宣言
//==========================================================================================
static void Save( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx, u32 data );
static u32 Load( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx );
static void InitGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap );
static void LoadGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap );
static void SaveGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap );


//==========================================================================================
// ■ギミック登録関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  LF02WORK* work;
  HEAPID heap_id;

  // DEBUG:
  OBATA_Printf( "GIMMICK-LF02: setup\n" ); 

  // ギミック管理ワーク作成
  heap_id      = FIELDMAP_GetHeapID( fieldmap );
  work         = (LF02WORK*)GFL_HEAP_AllocMemory( heap_id, sizeof(LF02WORK) );
  work->heapID = heap_id; 

  // ギミック初期化
  InitGimmick( work, fieldmap );
  LoadGimmick( work, fieldmap ); 

  // LF02ギミック管理ワークのアドレスを保存
  Save( fieldmap, GIMMICKWORK_DATA_WORK_ADRS, (u32)work ); 
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  LF02WORK* work;

  // セーブ
  work = (LF02WORK*)Load( fieldmap, GIMMICKWORK_DATA_WORK_ADRS );
  SaveGimmick( work, fieldmap );

  // ギミック管理ワーク破棄
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OBATA_Printf( "GIMMICK-LF02: end\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void LEAGUE_FRONT_02_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  // 拡張オブジェクトのアニメ再生
  {
    FLD_EXP_OBJ_CNT_PTR exobj_cnt;
    exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
    FLD_EXP_OBJ_PlayAnime( exobj_cnt );
  }
}


//==========================================================================================
// ■ 非公開関数
//==========================================================================================

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックワークへのセーブ処理
 *
 * @param fieldmap フィールドマップ
 * @param idx      格納先インデックス
 * @param data     格納する実データ
 */
//------------------------------------------------------------------------------------------
static void Save( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx, u32 data )
{
  GAMESYS_WORK*   gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*      gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK* gmkwork = GAMEDATA_GetGimmickWork( gdata );
  u32*       save_work = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_C09P02 );

  // ギミックワークに保存
  save_work[idx] = data;

  // DEBUG:
  OBATA_Printf( "GIMMICK-FL02: save[%d] = %d\n", idx, data );
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックワークからのロード処理
 *
 * @param fieldmap フィールドマップ
 * @param idx      取得データのインデックス
 *
 * @return 指定したインデックスに格納した値
 */
//------------------------------------------------------------------------------------------
static u32 Load( FIELDMAP_WORK* fieldmap, GIMMICKWORK_DATA_INDEX idx )
{
  GAMESYS_WORK*   gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*      gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK* gmkwork = GAMEDATA_GetGimmickWork( gdata );
  u32*       save_work = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_C09P02 );
  u32              val = 0;

  // ギミックワークから取得
  val = save_work[idx];

  // DEBUG:
  OBATA_Printf( "GIMMICK-FL02: load[%d] = %d\n", idx, val );

  return val;
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックを初期化する
 *
 * @param work     初期化対象ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void InitGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt;

  // 拡張オブジェクトのユニットを追加
  exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &unit[UNIT_GIMMICK], UNIT_GIMMICK );

  // リフトのアニメーション初期化
  FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT, LIFT_ANM_TA, TRUE );

  // リフトの座標をアニメ最終フレームの値で初期化
  {
    ICA_ANIME* anime; 
    VecFx32 trans;
    u32 max_frame;
    anime = ICA_ANIME_CreateAlloc( work->heapID,
                                   ARCID, NARC_league_front_pl_ele_01_ica_bin );
    max_frame = ICA_ANIME_GetMaxFrame( anime );
    ICA_ANIME_GetTranslateAt( anime, &trans, max_frame - 1 );
    {
      GFL_G3D_OBJSTATUS* objstatus;
      FLD_EXP_OBJ_CNT_PTR exobj_cnt;
      exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
      objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT );
      VEC_Set( &objstatus->trans, trans.x, trans.y, trans.z );
    }
    ICA_ANIME_Delete( anime );
  }
  // @todo icaにx,zの情報が入ったら以下の処理は削除する
  {
    GFL_G3D_OBJSTATUS* objstatus;
    objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT );
    VEC_Set( &objstatus->trans, LIFT_POS_X, LIFT_POS_Y, LIFT_POS_Z );
  }

  // DEBUG:
  OBATA_Printf( "GIMMICK-LF02: init gimmick\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックの状態を復帰する
 *
 * @param work     復帰対象ギミックの管理ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void LoadGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap )
{
} 

//------------------------------------------------------------------------------------------
/**
 * @brief ギミックの状態を保存する
 *
 * @param work     保存対象ギミックの管理ワーク
 * @param fieldmap 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void SaveGimmick( LF02WORK* work, FIELDMAP_WORK* fieldmap )
{
}


//==========================================================================================
// ■ イベント
//==========================================================================================
typedef struct
{
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;
  ICA_ANIME* liftAnime;  // リフトの移動アニメーション

} LIFTDOWN_EVENTWORK;

//------------------------------------------------------------------------------------------
/**
 * @brief  リフトの座標を更新する
 * @return 移動が終了したら TRUE, そうでなければ　FALSE
 */
//------------------------------------------------------------------------------------------
static BOOL MoveLift( LIFTDOWN_EVENTWORK* work )
{
  BOOL anime_end;

  // アニメーションを進める
  anime_end = ICA_ANIME_IncAnimeFrame( work->liftAnime, FX32_ONE );

  // リフトの座標を更新
  if( !anime_end )
  {
    VecFx32 trans;
    GFL_G3D_OBJSTATUS* objstatus;
    FLD_EXP_OBJ_CNT_PTR exobj_cnt;
    exobj_cnt = FIELDMAP_GetExpObjCntPtr( work->fieldmap );
    objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, UNIT_GIMMICK, OBJ_LIFT );
    ICA_ANIME_GetTranslate( work->liftAnime, &trans );
    VEC_Set( &objstatus->trans, trans.x, trans.y, trans.z );

    // @todo icaにx,zの情報が入ったら以下の処理は削除する
    objstatus->trans.x = LIFT_POS_X;
    objstatus->trans.z = LIFT_POS_Z;
  }
  return anime_end;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 自機の座標をリフトの動きに合わせる
 */
//------------------------------------------------------------------------------------------
static void SetPlayerOnLift( LIFTDOWN_EVENTWORK* work )
{
  FIELD_PLAYER* player;
  VecFx32 pos, trans;
  player = FIELDMAP_GetFieldPlayer( work->fieldmap );
  ICA_ANIME_GetTranslate( work->liftAnime, &trans );
  VEC_Set( &pos, 
      trans.x + PLAYER_OFS_X, 
      trans.y + PLAYER_OFS_Y, 
      trans.z + PLAYER_OFS_Z );
  // @todo icaにx,zの情報が入ったら以下の処理は削除する
  {
    pos.x = LIFT_POS_X + PLAYER_OFS_X;
    pos.z = LIFT_POS_Z + PLAYER_OFS_Z;
  }
  // x, z座標がグリッドの中心になるように調整
  pos.x = GRID_TO_FX32( FX32_TO_GRID(pos.x) ) + HALF_GRID;
  pos.z = GRID_TO_FX32( FX32_TO_GRID(pos.z) ) + HALF_GRID;
  FIELD_PLAYER_SetPos( player, &pos );
  // DEBUG:
  OBATA_Printf( "pos = %d, %d, %d\n", FX_Whole(pos.x), FX_Whole(pos.y), FX_Whole(pos.z) );
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベント処理
 */
//------------------------------------------------------------------------------------------
static GMEVENT_RESULT LiftDownEvent( GMEVENT* event, int* seq, void* wk )
{
  LIFTDOWN_EVENTWORK* work = (LIFTDOWN_EVENTWORK*)wk;

  switch( *seq )
  {
  // アニメ開始
  case 0:
    { // アニメデータ読み込み
      HEAPID heap_id;
      heap_id = FIELDMAP_GetHeapID( work->fieldmap );
      work->liftAnime = ICA_ANIME_CreateAlloc( heap_id, ARCID, 
                                               NARC_league_front_pl_ele_01_ica_bin );
    }
    // リフトと自機の座標を初期化
    MoveLift( work );
    SetPlayerOnLift( work );
    ++(*seq);
    OBATA_Printf( "GIMMICK-LF02 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    break;
  // フェードイン
  case 1:
    {
      GMEVENT* new_event;
      new_event = EVENT_FieldFadeIn_Black( work->gsys, work->fieldmap, FIELD_FADE_WAIT );
      GMEVENT_CallEvent( event, new_event );
    }
    ++(*seq);
    OBATA_Printf( "GIMMICK-LF02 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    break;
  // アニメ終了待ち
  case 2:
    {
      BOOL anime_end;

      // リフトを動かす
      anime_end = MoveLift( work );

      // 自機をリフトに合わせる
      if( !anime_end ){ SetPlayerOnLift( work ); }

      // アニメ終了判定
      if( anime_end )
      { 
        ICA_ANIME_Delete( work->liftAnime );
        ++(*seq); 
        OBATA_Printf( "GIMMICK-LF02 LIFT DOWN EVENT: seq ==> %d\n", *seq );
      } 
    }
    break;
  // 終了処理
  case 3:
    // 自機を着地させる
    {
      VecFx32 pos;
      fx32 height;
      FIELD_PLAYER* player;
      MMDL* mmdl;
      player = FIELDMAP_GetFieldPlayer( work->fieldmap );
      mmdl   = FIELD_PLAYER_GetMMdl( player );
      FIELD_PLAYER_GetPos( player, &pos );
      MMDL_GetMapPosHeight( mmdl, &pos, &height );
      pos.y = height;
      FIELD_PLAYER_SetPos( player, &pos );
    }
    ++(*seq); 
    OBATA_Printf( "GIMMICK-LF02 LIFT DOWN EVENT: seq ==> %d\n", *seq );
    break;
  // 終了
  case 4:
    OBATA_Printf( "GIMMICK-LF02 LIFT DOWN EVENT: seq ==> finish\n" );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief リフト降下イベントを作成する
 * 
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 *
 * @return リフト降下イベント
 */
//------------------------------------------------------------------------------------------
GMEVENT* LEAGUE_FRONT_02_GIMMICK_GetLiftDownEvent( GAMESYS_WORK* gsys, 
                                                   FIELDMAP_WORK* fieldmap )
{
  GMEVENT* event;
  LIFTDOWN_EVENTWORK* evwork;
  LF02WORK* gmkwork = (LF02WORK*)Load( fieldmap, GIMMICKWORK_DATA_WORK_ADRS );

  // 生成
  event = GMEVENT_Create( gsys, NULL, LiftDownEvent, sizeof(LIFTDOWN_EVENTWORK) );
  // 初期化
  evwork = GMEVENT_GetEventWork( event );
  evwork->gsys      = gsys;
  evwork->fieldmap  = fieldmap;
  evwork->liftAnime = NULL;
  return event;
}
