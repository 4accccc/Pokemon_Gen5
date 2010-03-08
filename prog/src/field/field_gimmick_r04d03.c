//======================================================================
/**
 * @file  field_gimmick_r04d03.c
 * @brief  4番道路+リゾートデザート
 * @author  Saito
 */
//======================================================================

#include <gflib.h>
#include "fieldmap.h" 
#include "field_gimmick_r04d03.h"
#include "field_gimmick_def.h"
//#include "gamesystem/iss_3ds_sys.h"
#include "savedata/gimmickwork.h"
//#include "sound/pm_sndsys.h"
#include "system/gfl_use.h"
//#include "sound_obj.h" 

#include "arc/arc_def.h"
#include "arc/h01.naix"

#define EXPOBJ_UNIT_IDX (0)
#define ARCID (ARCID_H01_GIMMICK) // ギミックデータのアーカイブID
//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct
{ 
  HEAPID            heapID; // 使用するヒープID
} GMK_WORK;

//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソース
typedef enum {
  RES_TRAILER_HEAD_NSBMD,  // トレーラー(前)のモデル
  RES_TRAILER_TAIL_NSBMD,  // トレーラー(後)のモデル
  RES_NUM
} RES_INDEX;

static const GFL_G3D_UTIL_RES res_table[RES_NUM] = 
{
  { ARCID, NARC_h01_trailer_01a_nsbmd, GFL_G3D_UTIL_RESARC },  // トレーラー(前)のモデル
  { ARCID, NARC_h01_trailer_01b_nsbmd, GFL_G3D_UTIL_RESARC },  // トレーラー(後)のモデル
};

// オブジェクト
typedef enum {
  OBJ_TRAILER_1_HEAD,  // トレーラー1(前)
  OBJ_TRAILER_1_TAIL,  // トレーラー1(後)
  OBJ_TRAILER_2_HEAD,  // トレーラー2(前)
  OBJ_TRAILER_2_TAIL,  // トレーラー2(後)
  OBJ_NUM
} OBJ_INDEX;

static const GFL_G3D_UTIL_OBJ obj_table[OBJ_NUM] = 
{
  { RES_TRAILER_HEAD_NSBMD, 0, RES_TRAILER_HEAD_NSBMD, NULL, 0 },  // トレーラー1(前)
  { RES_TRAILER_TAIL_NSBMD, 0, RES_TRAILER_TAIL_NSBMD, NULL, 0 },  // トレーラー1(後)
  { RES_TRAILER_HEAD_NSBMD, 0, RES_TRAILER_HEAD_NSBMD, NULL, 0 },  // トレーラー2(前)
  { RES_TRAILER_TAIL_NSBMD, 0, RES_TRAILER_TAIL_NSBMD, NULL, 0 },  // トレーラー2(後)
};

// セットアップ情報
static GFL_G3D_UTIL_SETUP setup = { res_table, RES_NUM, obj_table, OBJ_NUM };



//------------------------------------------------------------------------------------------
/**
 * @brief 初期化関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void R04D03_GIMMICK_Setup( FIELDMAP_WORK* fieldmap )
{
  u32* gmk_save;  // ギミックの実セーブデータ
  GMK_WORK* work;  // ギミック管理ワーク
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldmap );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
  GAMESYS_WORK*            gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*               gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*          gmkwork = GAMEDATA_GetGimmickWork(gdata);

  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &setup, EXPOBJ_UNIT_IDX );
/**
  // ギミック管理ワークを作成
  work = (H01WORK*)GFL_HEAP_AllocMemory( heapID, sizeof(H01WORK) );

  // ギミック管理ワークを初期化 
  InitWork( work, fieldmap );

  // ロード
  LoadGimmick( work, fieldmap );

  // ギミック管理ワークのアドレスを保存
  gmk_save    = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  gmk_save[0] = (int)work;
*/
  // DEBUG:
  NOZOMU_Printf( "GIMMICK: set up\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 終了関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void R04D03_GIMMICK_End( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldmap );
/**
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  GMK_WORK*         work = (GMK_WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス
*/

/**
  // 風を止める
  PMSND_ChangeBGMVolume( work->wind_data.trackBit, 0 );

  // セーブ
  SaveGimmick( work, fieldmap );

  // 音源オブジェクトを破棄
  for( i=0; i<SOBJ_NUM; i++ )
  {
    SOUNDOBJ_Delete( work->sobj[i] );
  }
  // ギミック管理ワークを破棄
  GFL_HEAP_FreeMemory( work );
*/
  //ユニット破棄
  FLD_EXP_OBJ_DelUnit( exobj_cnt, EXPOBJ_UNIT_IDX );
  // DEBUG:
  NOZOMU_Printf( "GIMMICK: end\n" );
}

//------------------------------------------------------------------------------------------
/**
 * @brief 動作関数
 *
 * @param fieldmap ギミック動作フィールドマップ
 */
//------------------------------------------------------------------------------------------
void R04D03_GIMMICK_Move( FIELDMAP_WORK* fieldmap )
{
  int i;
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldmap );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
/**  
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  GMK_WORK*         work = (H01WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス
*/
/**
  // 観測者の位置を設定
  {
    FIELD_CAMERA* fieldCamera;
    VecFx32 cameraPos, targetPos;

    fieldCamera = FIELDMAP_GetFieldCamera( fieldmap );
    FIELD_CAMERA_GetCameraPos( fieldCamera, &cameraPos );
    FIELD_CAMERA_GetTargetPos( fieldCamera, &targetPos );
    ISS_3DS_SYS_SetObserverPos( work->iss3dsSys, &cameraPos, &targetPos );
  }

  // すべての音源オブジェクトを動かす
  for( i=0; i<SOBJ_NUM; i++ )
  { 
    // 待機状態 ==> 発車カウントダウン
    if( 0 < work->wait[i] )
    {
      // カウントダウン終了 ==> 発車
      if( --work->wait[i] <= 0 )
      {
        MoveStart( work, i );
        // 後部分を前部分に追従させる
        switch(i)
        {
        case SOBJ_TRAILER_1_HEAD:
          work->wait[SOBJ_TRAILER_1_TAIL] = TAIL_INTERVAL;
          break;
        case SOBJ_TRAILER_2_HEAD:
          work->wait[SOBJ_TRAILER_2_TAIL] = TAIL_INTERVAL;
          break;
        }
      }
    }
    // 動作中 ==> アニメーションを更新
    else
    {
      // アニメーション再生が終了 ==> 待機状態へ
      if( SOUNDOBJ_IncAnimeFrame( work->sobj[i], FX32_ONE ) )
      {
        SetStandBy( work, i );
      }
    }
  }

  // 風を更新
  UpdateWindVolume( fieldmap, work );
*/  
}

