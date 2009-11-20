/////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief 3Dサウンドシステム
 * @file iss_3d_sys.h
 * @author obata
 * @date 2009.09.08
 * 
 */
/////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "iss_3ds_sys.h"
#include "sound/pm_sndsys.h"

// デバッグ出力スイッチ
//#define ISS_3DS_SYS_DEBUG


//===============================================================================
/**
 * @brief システムワーク
 */
//===============================================================================
struct _ISS_3DS_SYS
{
  HEAPID                heapID;  // ヒープID
  u8                maxUnitNum;  // ユニット最大数
  u8                   unitNum;  // ユニット保持数
  ISS_3DS_UNIT**          unit;  // ユニット配列
  const GFL_G3D_CAMERA* camera;  // カメラ
};


//===============================================================================
// プロトタイプ宣言
//===============================================================================
static void InitSys( ISS_3DS_SYS* system, 
    HEAPID heap_id, u8 max_unit_num, const GFL_G3D_CAMERA* camera );
static void UpdateVolume( ISS_3DS_SYS* system, u8 index );
static void UpdatePan( ISS_3DS_SYS* system, u8 index );


//===============================================================================
// 作成・破棄・動作
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief システムを作成する
 *
 * @param heap_id      使用するヒープID
 * @param max_unit_num ユニット最大数
 * @param camera       使用するカメラ
 *
 * @return 作成したシステム
 */
//-------------------------------------------------------------------------------
ISS_3DS_SYS* ISS_3DS_SYS_Create( 
    HEAPID heap_id, u8 max_unit_num, const GFL_G3D_CAMERA* camera )
{
  ISS_3DS_SYS* system;

  // システムワーク作成
  system = GFL_HEAP_AllocMemory( heap_id, sizeof(ISS_3DS_SYS) );

  // システムワーク初期化
  InitSys( system, heap_id, max_unit_num, camera );

  return system;
}

//-------------------------------------------------------------------------------
/**
 * @brief システムを破棄する
 *
 * @param system 破棄するシステム
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_Delete( ISS_3DS_SYS* system )
{
  int i;

  // 全ユニット破棄
  for( i=0; i<system->maxUnitNum; i++ )
  {
    ISS_3DS_SYS_DeleteUnit( system, i );
  } 
  GFL_HEAP_FreeMemory( system->unit );

  // システム破棄
  GFL_HEAP_FreeMemory( system );
}

//-------------------------------------------------------------------------------
/**
 * @brief システムの動作処理
 *
 * @param system 動かすシステム
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_Main( ISS_3DS_SYS* system )
{
  int i;

  // 各ユニットを適用
  for( i=0; i<system->maxUnitNum; i++ )
  {
    UpdateVolume( system, i );
    UpdatePan( system, i );
  }
}


//===============================================================================
// ユニットの追加・削除・取り出し
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief ユニットを追加する
 * 
 * @param system     追加先システム
 *
 * @return 追加したユニットの管理インデックス
 */
//-------------------------------------------------------------------------------
u8 ISS_3DS_SYS_AddUnit( ISS_3DS_SYS* system )
{
  u8 index;

  // 空き要素を検索
  for( index=0; index<system->maxUnitNum; index++ )
  {
    if( system->unit[index] == NULL ) break;
  }

  // 空きがない場合
  if( system->maxUnitNum <= index )
  {
    // DEBUG:
    OBATA_Printf( "-----------------------------------\n" );
    OBATA_Printf( "ERROR : ISS_3DS_SYS unit over flow\n" );
    OBATA_Printf( "-----------------------------------\n" );
    return 0;
  }

  // ユニットを作成
  system->unit[index] = ISS_3DS_UNIT_Create( system->heapID );
  system->unitNum++;
  // DEBUG:
  OBATA_Printf( "ISS-3DS-SYS: add unit (unit index = %d)\n", index );

  // 作成したユニットのインデックスを返す
  return index;
}

//-------------------------------------------------------------------------------
/**
 * @brief ユニットを破棄する
 *
 * @param system ユニットが登録されているシステム
 * @param index  削除対象ユニットの管理インデックス
 */
//-------------------------------------------------------------------------------
void ISS_3DS_SYS_DeleteUnit( ISS_3DS_SYS* system, u8 index )
{
  // 指定インデックスにユニットが登録されていたら削除する
  if( system->unit[index] != NULL )
  {
    u16 track = ISS_3DS_UNIT_GetTrackBit( system->unit[index] );

    // 操作していたBGMステータスを元に戻す
    PMSND_SetStatusBGM_EX( track, 256, 0, 0 );

    // 削除
    ISS_3DS_UNIT_Delete( system->unit[index] );
    system->unit[index] = NULL;
    system->unitNum--;
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 指定ユニットを取り出す
 *
 * @param system ユニットが登録されているシステム
 * @param index  取り出し対象ユニットの管理インデックス
 *
 * @return 指定された管理番号のユニット
 */
//-------------------------------------------------------------------------------
ISS_3DS_UNIT* ISS_3DS_SYS_GetUnit( ISS_3DS_SYS* system, u8 index )
{
  return system->unit[index];
}


//===============================================================================
// 非公開関数
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief システムワークを初期化する
 *
 * @param system       初期化するシステムワーク
 * @param heap_id      使用するヒープID
 * @param max_unit_num 最大ユニット数
 * @param camera       使用するカメラ
 */
//-------------------------------------------------------------------------------
static void InitSys( ISS_3DS_SYS* system, 
    HEAPID heap_id, u8 max_unit_num, const GFL_G3D_CAMERA* camera )
{
  int i;

  system->heapID     = heap_id;
  system->maxUnitNum = max_unit_num;
  system->unitNum    = 0;
  system->unit       = GFL_HEAP_AllocMemory( heap_id, sizeof(ISS_3DS_UNIT*) * max_unit_num ); 
  system->camera     = camera;

  // ユニット配列初期化
  for( i=0; i<max_unit_num; i++ )
  {
    system->unit[i] = NULL;
  }
} 

//-------------------------------------------------------------------------------
/**
 * @brief ユニット適用処理( 音量 )
 *
 * @param system システム
 * @param index  適用するユニットの管理番号
 */
//-------------------------------------------------------------------------------
static void UpdateVolume( ISS_3DS_SYS* system, u8 index )
{
  u16 track;
  ISS_3DS_UNIT*  unit = system->unit[index];
  const GFL_G3D_CAMERA* cam = system->camera;

  // 指定番号にユニットが登録されてない場合
  if( unit == NULL ) return;

  // トラックマスクを取得
  track = ISS_3DS_UNIT_GetTrackBit( unit );

  // ボリュームを計算
  {
    VecFx32 cam_pos, unit_pos, vec_to_unit;
    fx32 dist, range;
    int volume;

    // 距離を計算
    GFL_G3D_CAMERA_GetPos( cam, &cam_pos ); 
    ISS_3DS_UNIT_GetPos( unit, &unit_pos );
    VEC_Subtract( &unit_pos, &cam_pos, &vec_to_unit );
    dist  = VEC_Mag( &vec_to_unit );
    range = ISS_3DS_UNIT_GetRange( unit );

    // 音量決定
    if( range < dist )  // 音が届かない距離ならゼロ
    {
      volume = 0;
    }
    else                // 音が届く距離なら線形補間
    {
      int   max   = ISS_3DS_UNIT_GetMaxVolume( unit );
      float rate  = FX_FX32_TO_F32( FX_Div( range - dist, range ) ); 
      volume      = max * rate;
    }

    // 音量設定
    PMSND_ChangeBGMVolume( track, volume );
    // DEBUG:
#ifdef ISS_3DS_SYS_DEBUG
    OBATA_Printf( "ISS-3DS-SYS: update volume => %d (unit index = %d)\n", 
        volume, index );
#endif
  } 
}

//-------------------------------------------------------------------------------
/**
 * @brief ユニット適用処理( パン )
 *
 * @param system システム
 * @param index  適用するユニットの管理番号
 */
//-------------------------------------------------------------------------------
static void UpdatePan( ISS_3DS_SYS* system, u8 index )
{
  u16 track;
  ISS_3DS_UNIT*  unit = system->unit[index];
  const GFL_G3D_CAMERA* cam = system->camera;

  // 指定番号にユニットが登録されてない場合
  if( unit == NULL ) return;

  // トラックマスクを取得
  track = ISS_3DS_UNIT_GetTrackBit( unit );

  // パンを計算
  {
    VecFx32 v0, v1, v2, v3, vup, vp;
    VecFx32 cx, cy, cz;
    VecFx32 cam_pos, target, unit_pos;
    int pan;
    fx32 dot;

    // カメラ・注視点・音源それぞれの位置
    GFL_G3D_CAMERA_GetPos( cam, &cam_pos );
    GFL_G3D_CAMERA_GetTarget( cam, &target );
    ISS_3DS_UNIT_GetPos( unit, &unit_pos );

    // カメラの向きベクトルv0
    VEC_Subtract( &target, &cam_pos, &v0 );

    // カメラ→音源ベクトルv1
    VEC_Subtract( &unit_pos, &cam_pos, &v1 );

    // カメラ上方ベクトルvup
    GFL_G3D_CAMERA_GetCamUp( cam, &vup );

    // カメラ座標軸cx, cy, cz
    VEC_Normalize( &v0, &cz );
    VEC_CrossProduct( &cz, &vup, &cx );   VEC_Normalize( &cx, &cx );
    VEC_CrossProduct( &cx, &cz, &cy );    VEC_Normalize( &cy, &cy );

    // v1をカメラ座標系のxz平面へ射影したベクトルv2
    VEC_Normalize( &cy, &cy );
    dot = VEC_DotProduct( &v1, &cy );
    VEC_Set( &vp, FX_Mul(cy.x, dot), FX_Mul(cy.y, dot), FX_Mul(cy.z, dot) );
    VEC_Subtract( &v1, &vp, &v2 );  // v1のcy成分を除去
    VEC_Normalize( &v2, &v2 );

    // パン = cx・v2
    dot = VEC_DotProduct( &cx, &v2 );
    pan = 128 * FX_FX32_TO_F32(dot);

    // パンの値を補正
    if( pan < -128 ) pan = -128;
    if( 127 < pan  ) pan =  127;

    // パンを設定
    PMSND_SetStatusBGM_EX( track, 256, 0, pan );

#ifdef ISS_3DS_SYS_DEBUG
    OBATA_Printf( "pan = %d\n", pan );
#endif
  }
}
