#include <gflib.h>
#include "fieldmap.h"
#include "field_gimmick_h01.h"
#include "arc/fieldmap/buildmodel_outdoor.naix"
#include "savedata/gimmickwork.h"
#include "field_gimmick_def.h"
#include "gamesystem/iss_3ds_unit.h"
#include "gamesystem/iss_3ds_sys.h"
#include "sound/pm_sndsys.h"


//======================================================================
/**
 * @breif 定数
 */
//======================================================================
//-----
// BGM
//-----
// トラック番号
#define TRACK_WIND (7)
#define TRACK_SHIP (8)
#define TRACK_TR1  (9)
#define TRACK_TR2  (10)
// トラックマスク
#define TRACK_MASK_WIND (1 << (TRACK_WIND - 1))
#define TRACK_MASK_SHIP (1 << (TRACK_SHIP - 1))
#define TRACK_MASK_TR1  (1 << (TRACK_TR1  - 1))
#define TRACK_MASK_TR2  (1 << (TRACK_TR2  - 1))

//------------
// トレーラー
//------------ 
// 移動範囲
#define TR_Z_MIN (1200 << FX32_SHIFT)
#define TR_Z_MAX (4230 << FX32_SHIFT) 
#define TR_X_01 (1756 << FX32_SHIFT)
#define TR_X_02 (1895 << FX32_SHIFT)
#define TR_Y    (490 << FX32_SHIFT) 
// 移動時間
#define TR_FRAME (300)
// 音が届く距離
#define TR_SOUND_RANGE (1000 << FX32_SHIFT)

//-----
// 船
//-----
// 移動範囲
#define SHIP_X_MIN (0 << FX32_SHIFT) 
#define SHIP_X_MAX (4000 << FX32_SHIFT) 
#define SHIP_Y     (-35 << FX32_SHIFT)
#define SHIP_Z     (2386 << FX32_SHIFT)  
// 移動時間
#define SHIP_FRAME (1000)
// 音が届く距離
#define SHIP_SOUND_RANGE (3000 << FX32_SHIFT)

//------
// 風
//------
#define WIND_Y_MIN (800 << FX32_SHIFT)
#define WIND_Y_MAX (1200 << FX32_SHIFT)

//-------------
// FLD_EXP_OBJ
//-------------
#define EXPOBJ_UNIT_IDX (0)  // 拡張オブジェクトのユニット番号

// 3Dリソース
static const GFL_G3D_UTIL_RES res_table[] = 
{
  { ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_trailer_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_trailer_01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_BMODEL_OUTDOOR, NARC_output_buildmodel_outdoor_h01_ship_nsbmd, GFL_G3D_UTIL_RESARC },
};

// 3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ obj_table[] = 
{
  { 0, 0, 0, NULL, 0 },
  { 1, 0, 1, NULL, 0 },
  { 2, 0, 2, NULL, 0 },
};

// UTILセットアップ情報
static GFL_G3D_UTIL_SETUP setup = { res_table, NELEMS(res_table), obj_table, NELEMS(obj_table) };


//----------------------
// 音源オブジェクト番号
//----------------------
typedef enum
{
  SOUNDOBJ_TR1,
  SOUNDOBJ_TR2,
  SOUNDOBJ_SHIP,
  SOUNDOBJ_NUM
} SOUNDOBJ_INDEX;


//============================================================================================
/**
 * @breif 音源オブジェクト(トレーラ・船など)
 */
//============================================================================================
typedef struct
{
  SOUNDOBJ_INDEX     index; // 自身のID
  u8         iss3dsUnitIdx; // 3Dサウンドユニットの管理インデックス
  ISS_3DS_UNIT* iss3dsUnit; // 3Dサウンドユニット
  u16                frame; // 現在フレーム
  u16             endFrame; // 最終フレーム
  VecFx32         startPos; // 開始位置
  VecFx32           endPos; // 最終位置 
}
SOUNDOBJ;


//============================================================================================
/**
 * @breif ギミック管理ワーク
 */
//============================================================================================
typedef struct
{
  ISS_3DS_SYS*          iss3DSSys;  // 3Dサウンドシステム
  SOUNDOBJ soundObj[SOUNDOBJ_NUM];  // 音源オブジェクト
}
H01WORK;


//============================================================================================
/**
 * @brief プロトタイプ宣言
 */
//============================================================================================
static void MoveSoundObj( FIELDMAP_WORK* fieldmap, SOUNDOBJ* sobj );
static void UpdateWindVolume( FIELDMAP_WORK* fieldmap );


//======================================================================
/**
 * @brief ギミック登録関数
 */
//======================================================================

//--------------------------------------------------------------------
/**
 * @brief セットアップ関数
 */
//--------------------------------------------------------------------
void H01_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldmap ); 
  GAMESYS_WORK*      gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*   sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*    gmkwork = SaveData_GetGimmickWork( sv );
  u16 frame[SOUNDOBJ_NUM] = {0, 0, 0};
  SOUNDOBJ* sobj;
  GFL_G3D_OBJSTATUS* status;
  H01WORK* h01work;
  int work_adrs;

  // ギミック管理ワークを作成
  h01work = GFL_HEAP_AllocMemory( heap_id, sizeof( H01WORK ) );

  // セーブデータをチェック
  if( GIMMICKWORK_GetAssignID( gmkwork ) == FLD_GIMMICK_H01 )
  {
    work_adrs = (int)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
    frame[0] = *( (u16*)( work_adrs + 4 ) );
    frame[1] = *( (u16*)( work_adrs + 6 ) );
    frame[2] = *( (u16*)( work_adrs + 8 ) );
  }
  else  // 初回セットアップ時
  {
    work_adrs = (int)GIMMICKWORK_Assign( gmkwork, FLD_GIMMICK_H01 );
  }

  // ギミックワークに管理ワークのアドレスを保存
  *( (H01WORK**)work_adrs ) = h01work;

  // 拡張オブジェクトのユニットを作成
  FLD_EXP_OBJ_AddUnit( ptr, &setup, EXPOBJ_UNIT_IDX );

  // ギミック管理ワーク初期化
  {
    // 3Dサウンドシステム作成
    FIELD_CAMERA* fc = FIELDMAP_GetFieldCamera( fieldmap );
    const GFL_G3D_CAMERA* camera = FIELD_CAMERA_GetCameraPtr( fc );
    h01work->iss3DSSys = ISS_3DS_SYS_Create( heap_id, SOUNDOBJ_NUM, camera );
  }

  // 各オブジェクトの初期ステータスを設定
  sobj = &h01work->soundObj[SOUNDOBJ_TR1];
  sobj->index = SOUNDOBJ_TR1;
  sobj->frame = frame[SOUNDOBJ_TR1];
  sobj->endFrame = TR_FRAME;
  VEC_Set( &sobj->startPos, TR_X_01, TR_Y, TR_Z_MIN );
  VEC_Set( &sobj->endPos,   TR_X_01, TR_Y, TR_Z_MAX );
  status = FLD_EXP_OBJ_GetUnitObjStatus( ptr, EXPOBJ_UNIT_IDX, SOUNDOBJ_TR1 );
  VEC_Set( &status->trans, 0, 0, 0 );
  VEC_Set( &status->scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_RotY33( &status->rotate, FX_SinIdx( 32768 ), FX_CosIdx( 32768 ) );	  // y軸180度回転
  sobj->iss3dsUnitIdx = ISS_3DS_SYS_AddUnit( h01work->iss3DSSys );
  sobj->iss3dsUnit = ISS_3DS_SYS_GetUnit( h01work->iss3DSSys, sobj->iss3dsUnitIdx );
  ISS_3DS_UNIT_SetRange( sobj->iss3dsUnit, TR_SOUND_RANGE );
  ISS_3DS_UNIT_SetMaxVolume( sobj->iss3dsUnit, 127 );
  ISS_3DS_UNIT_SetTrackBit( sobj->iss3dsUnit, TRACK_MASK_TR1 );

  sobj = &h01work->soundObj[SOUNDOBJ_TR2];
  sobj->index = SOUNDOBJ_TR2;
  sobj->frame = frame[SOUNDOBJ_TR2];
  sobj->endFrame = TR_FRAME;
  VEC_Set( &sobj->startPos, TR_X_02, TR_Y, TR_Z_MAX );
  VEC_Set( &sobj->endPos,   TR_X_02, TR_Y, TR_Z_MIN );
  status = FLD_EXP_OBJ_GetUnitObjStatus( ptr, EXPOBJ_UNIT_IDX, SOUNDOBJ_TR2 );
  VEC_Set( &status->trans, 0, 0, 0 );
  VEC_Set( &status->scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status->rotate );
  sobj->iss3dsUnitIdx = ISS_3DS_SYS_AddUnit( h01work->iss3DSSys );
  sobj->iss3dsUnit = ISS_3DS_SYS_GetUnit( h01work->iss3DSSys, sobj->iss3dsUnitIdx );
  ISS_3DS_UNIT_SetRange( sobj->iss3dsUnit, TR_SOUND_RANGE ); ISS_3DS_UNIT_SetMaxVolume( sobj->iss3dsUnit, 127 );
  ISS_3DS_UNIT_SetTrackBit( sobj->iss3dsUnit, TRACK_MASK_TR2 );

  sobj = &h01work->soundObj[SOUNDOBJ_SHIP];
  sobj->index = SOUNDOBJ_SHIP;
  sobj->frame = frame[SOUNDOBJ_SHIP];
  sobj->endFrame = SHIP_FRAME;
  VEC_Set( &sobj->startPos, SHIP_X_MIN, SHIP_Y, SHIP_Z );
  VEC_Set( &sobj->endPos,   SHIP_X_MAX, SHIP_Y, SHIP_Z );
  status = FLD_EXP_OBJ_GetUnitObjStatus( ptr, EXPOBJ_UNIT_IDX, SOUNDOBJ_SHIP );
  VEC_Set( &status->trans, 0, 0, 0 );
  VEC_Set( &status->scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &status->rotate );
  sobj->iss3dsUnitIdx = ISS_3DS_SYS_AddUnit( h01work->iss3DSSys );
  sobj->iss3dsUnit = ISS_3DS_SYS_GetUnit( h01work->iss3DSSys, sobj->iss3dsUnitIdx );
  ISS_3DS_UNIT_SetRange( sobj->iss3dsUnit, SHIP_SOUND_RANGE );
  ISS_3DS_UNIT_SetMaxVolume( sobj->iss3dsUnit, 127 );
  ISS_3DS_UNIT_SetTrackBit( sobj->iss3dsUnit, TRACK_MASK_SHIP );
}

//--------------------------------------------------------------------
/**
 * @brief 終了関数
 */
//--------------------------------------------------------------------
void H01_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{

  FLD_EXP_OBJ_CNT_PTR ptr = FIELDMAP_GetExpObjCntPtr( fieldmap ); 
  GAMESYS_WORK*      gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*   sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*    gmkwork = SaveData_GetGimmickWork( sv );
  int                 ofs = 0;
  int           work_adrs = (int)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  H01WORK*        h01work = (H01WORK*)( *( (H01WORK**)work_adrs ) );

  // 仕掛け用ワークを初期化
  work_adrs = (int)GIMMICKWORK_Assign( gmkwork, FLD_GIMMICK_H01 );

  // データを保存
  ofs = 4;
  *( (u16*)(work_adrs + ofs) ) = h01work->soundObj[SOUNDOBJ_TR1].frame;
  ofs += sizeof( u16 );
  *( (u16*)(work_adrs + ofs) ) = h01work->soundObj[SOUNDOBJ_TR2].frame;
  ofs += sizeof( u16 );
  *( (u16*)(work_adrs + ofs) ) = h01work->soundObj[SOUNDOBJ_SHIP].frame;
  ofs += sizeof( u16 );

  // 3Dサウンドシステム破棄
  ISS_3DS_SYS_Delete( h01work->iss3DSSys );

  // ギミック管理ワーク破棄
  GFL_HEAP_FreeMemory( h01work );
} 

//--------------------------------------------------------------------
/**
 * @brief 動作関数
 */
//--------------------------------------------------------------------
void H01_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*      gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK*   sv = GAMEDATA_GetSaveControlWork( gdata );
  GIMMICKWORK*    gmkwork = SaveData_GetGimmickWork( sv );
  int           work_adrs = (int)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  H01WORK*        h01work = (H01WORK*)( *( (H01WORK**)work_adrs ) );

  // すべての音源オブジェクトを動かす
  for( i=0; i<SOUNDOBJ_NUM; i++ )
  {
    MoveSoundObj( fieldmap, &h01work->soundObj[i] );
  }

  // 3Dサウンドシステム動作
  ISS_3DS_SYS_Main( h01work->iss3DSSys );

  // 風の音量を調整
  UpdateWindVolume( fieldmap );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief 音源オブジェを動かす
 *
 * @param fieldmap フィールドマップ
 * @param sobj     動かすオブジェクト
 */
//--------------------------------------------------------------------------------------------
static void MoveSoundObj( FIELDMAP_WORK* fieldmap, SOUNDOBJ* sobj )
{
  fx32 w0, w1;
  VecFx32 v0, v1;
  FLD_EXP_OBJ_CNT_PTR   ptr = FIELDMAP_GetExpObjCntPtr( fieldmap ); 
  GFL_G3D_OBJSTATUS* status = FLD_EXP_OBJ_GetUnitObjStatus( ptr, EXPOBJ_UNIT_IDX, sobj->index );

  // フレーム進める
  sobj->frame = (sobj->frame + 1) % sobj->endFrame;

  // 位置ベクトルを線形補間
  w1 = FX_Div( sobj->frame, sobj->endFrame );
  w0 = FX32_ONE - w1;
  GFL_CALC3D_VEC_MulScalar( &sobj->startPos, w0, &v0 );
  GFL_CALC3D_VEC_MulScalar( &sobj->endPos,   w1, &v1 );
  VEC_Add( &v0, &v1, &status->trans );

  // 音源位置を合わせる
  ISS_3DS_UNIT_SetPos( sobj->iss3dsUnit, &status->trans );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief 風の音量を調整する
 *
 * @param fieldmap フィールドマップ
 */
//--------------------------------------------------------------------------------------------
static void UpdateWindVolume( FIELDMAP_WORK* fieldmap )
{
  int volume;
  VecFx32 pos;
  FIELD_CAMERA* camera = FIELDMAP_GetFieldCamera( fieldmap );

  // カメラ座標を取得
  FIELD_CAMERA_GetCameraPos( camera, &pos );

  // 風の音量を算出
  if( pos.y <= WIND_Y_MIN ) 
  {
    volume = 0;
  }
  else if( WIND_Y_MAX <= pos.y )
  {
    volume = 127;
  }
  else
  {
    fx32    max = WIND_Y_MAX - WIND_Y_MIN;
    fx32 height = pos.y - WIND_Y_MIN;
    fx32 rate   = FX_Div( height, max );
    volume = 127 * FX_FX32_TO_F32( rate );
  }

  // 音量を調整
  PMSND_ChangeBGMVolume( TRACK_MASK_WIND, volume );
}
