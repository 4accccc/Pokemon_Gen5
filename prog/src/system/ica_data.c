/////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  .icaファイルのアニメーションデータを扱う関数群
 * @file   ica_data.c
 * @author obata
 * @date   2009.09.01
 *
 */
/////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "ica_data.h"


//===================================================================================
/**
 * @brief 定数
 */
//===================================================================================
// 値の種類
typedef enum 
{
  VALUE_TYPE_SCALE_X,     // 拡縮
  VALUE_TYPE_SCALE_Y,     // 
  VALUE_TYPE_SCALE_Z,     //
  VALUE_TYPE_ROTATE_X,    // 回転
  VALUE_TYPE_ROTATE_Y,    // 
  VALUE_TYPE_ROTATE_Z,    // 
  VALUE_TYPE_TRANSLATE_X, // 平行移動
  VALUE_TYPE_TRANSLATE_Y, // 
  VALUE_TYPE_TRANSLATE_Z, // 
  VALUE_TYPE_NUM
}
VALUE_TYPE;

//===================================================================================
/**
 * @brief アニメーション属性
 *          値の配列をどのように参照するのかを決める属性
 */
//===================================================================================
typedef struct 
{
  u8 frameStep; // 間引き具合
  u16 dataSize; // 有効データ数
  u16 dataHead; // 先頭データインデックス
}
ANIME_ATTR;

//===================================================================================
/**
 * @brief アニメーションデータ
 */
//===================================================================================
struct _ICA_DATA
{
  u16 frameSize;      // 総フレーム数
  
  u16 scaleDataSize;  // スケールデータ数
  float*  scaleData;  // スケールデータ配列

  u16 rotateDataSize; // 回転データ数
  float*  rotateData; // 回転データ配列

  u16 transDataSize;  // 平行移動データ数
  float*  transData;  // 平行移動データ配列

  // アニメーション属性
  ANIME_ATTR anime_attr[ VALUE_TYPE_NUM ];
};


//===================================================================================
/**
 * @brief プロトタイプ宣言
 */
//===================================================================================
static void LoadIcaData( ICA_DATA* p_anime, HEAPID heap_id, ARCID arc_id, ARCDATID dat_id );
static float GetValue( ICA_DATA* p_anime, VALUE_TYPE anime_type, fx32 now_frame );
static float CalcLinearValue( 
    const int frame_size, const float now_frame, 
    const int frame_step, const int data_size, const int data_head, const float* value );


//===================================================================================
/**
 * @brief 公開関数の定義
 */
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションデータを作成する
 *
 * @param heap_id 使用するヒープID
 * @param arc_id  読み込むアーカイブファイル
 * @param dat_id  アーカイブファイル上のインデックスナンバー
 *
 * @return 作成したアニメーションデータ
 */
//-----------------------------------------------------------------------------------
ICA_DATA* ICA_DATA_Create( HEAPID heap_id, ARCID arc_id, ARCDATID dat_id )
{
  ICA_DATA* data;

  // インスタンスを生成
  data = (ICA_DATA*)GFL_HEAP_AllocMemory( heap_id, sizeof(ICA_DATA) );

  // データを読み込む
  LoadIcaData( data, heap_id, arc_id, dat_id );

  // 作成したインスタンスを返す
  return data;
}

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションデータを破棄する
 *
 * @param p_data 破棄するデータ
 */
//-----------------------------------------------------------------------------------
void ICA_DATA_Delete( ICA_DATA* p_data )
{
  GFL_HEAP_FreeMemory( p_data->scaleData );
  GFL_HEAP_FreeMemory( p_data->rotateData );
  GFL_HEAP_FreeMemory( p_data->transData );
  GFL_HEAP_FreeMemory( p_data );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける位置データを取得する
 *
 * @param p_anime   アニメーションデータ
 * @param p_vec     取得した座標の格納先
 * @param now_frame 現在のフレーム数
 */
//-----------------------------------------------------------------------------------
void ICA_DATA_GetPos( ICA_DATA* p_anime, VecFx32* p_vec, fx32 now_frame )
{ 
  float x = GetValue( p_anime, VALUE_TYPE_TRANSLATE_X, now_frame );
  float y = GetValue( p_anime, VALUE_TYPE_TRANSLATE_Y, now_frame );
  float z = GetValue( p_anime, VALUE_TYPE_TRANSLATE_Z, now_frame );

  VEC_Set( p_vec, FX_F32_TO_FX32(x), FX_F32_TO_FX32(y), FX_F32_TO_FX32(z) );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける向きデータを取得する
 *
 * @param p_anime   アニメーションデータ
 * @param p_vec     取得したベクトルの格納先
 * @param now_frame 現在のフレーム数
 */
//-----------------------------------------------------------------------------------
void ICA_DATA_GetDir( ICA_DATA* p_anime, VecFx32* p_vec, fx32 now_frame )
{
  float x, y, z;
  u16 rx, ry, rz;
  MtxFx33 matrix;
  VecFx32 vec_norm;

  // 指定フレームの値を取得
  x = GetValue( p_anime, VALUE_TYPE_ROTATE_X, now_frame );
  y = GetValue( p_anime, VALUE_TYPE_ROTATE_Y, now_frame );
  z = GetValue( p_anime, VALUE_TYPE_ROTATE_Z, now_frame );

  // 正の値に補正
  while( x < 0 ) x += 360.0f;
  while( y < 0 ) y += 360.0f;
  while( z < 0 ) z += 360.0f;

  // u16に変換
  rx = x / 360.0f * 0xffff;
  ry = y / 360.0f * 0xffff;
  rz = z / 360.0f * 0xffff; 

  // 向きを表すベクトルを作成
  GFL_CALC3D_MTX_CreateRot( rx, ry, rz, &matrix );
  VEC_Set( &vec_norm, 0, 0, -FX32_ONE );
  MTX_MultVec33( &vec_norm, &matrix, p_vec );
}

//---------------------------------------------------------------------------
/**
 * @brief カメラ座標・ターゲット座標を指定フレームの状態に設定する
 *
 * @param p_anime   アニメーションデータ
 * @param p_camera  設定対象のカメラ
 * @param now_frame 現在のフレーム数
 */
//---------------------------------------------------------------------------
void ICA_DATA_SetCameraStatus( 
    ICA_DATA* p_anime, GFL_G3D_CAMERA* p_camera, fx32 now_frame )
{
  VecFx32 pos, target;

  // 座標・向きベクトルを取得
  ICA_DATA_GetPos( p_anime, &pos, now_frame );
  ICA_DATA_GetDir( p_anime, &target, now_frame );
  VEC_Add( &pos, &target, &target );

  // カメラの設定
  GFL_G3D_CAMERA_SetPos( p_camera, &pos );
  GFL_G3D_CAMERA_SetTarget( p_camera, &target );
}


//===================================================================================
/**
 * @brief 非公開関数の定義
 */
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief オリジナルフォーマットのバイナリデータをロードする
 *
 * @param p_anime 読み込んだデータの格納先変数
 * @param heap_id 使用するヒープID
 * @param arc_id  読み込むアーカイブファイル
 * @param dat_id  アーカイブファイル上のインデックスナンバー
 */
//-----------------------------------------------------------------------------------
static void LoadIcaData( ICA_DATA* p_anime, HEAPID heap_id, ARCID arc_id, ARCDATID dat_id )
{
  int i;
  int ofs = 0;  // 先頭からのオフセット
  
  // フレームサイズを取得
  GFL_ARC_LoadDataOfs( &p_anime->frameSize, arc_id, dat_id, ofs, sizeof( u16 ) );      
  ofs += sizeof( u16 );

  // スケールデータ数を取得
  GFL_ARC_LoadDataOfs( &p_anime->scaleDataSize, arc_id, dat_id, ofs, sizeof( u16 ) );  
  ofs += sizeof( u16 );

  // スケールデータを取得
  p_anime->scaleData = GFL_HEAP_AllocMemory( heap_id, sizeof( float ) * p_anime->scaleDataSize );
  for( i=0; i<p_anime->scaleDataSize; i++ )
  {
    GFL_ARC_LoadDataOfs( &p_anime->scaleData[i], arc_id, dat_id, ofs, sizeof(float) );  
    ofs += sizeof( float );
  }

  // 回転データ数を取得
  GFL_ARC_LoadDataOfs( &p_anime->rotateDataSize, arc_id, dat_id, ofs, sizeof( u16 ) );  
  ofs += sizeof( u16 );

  // 回転データを取得
  p_anime->rotateData = GFL_HEAP_AllocMemory( heap_id, sizeof( float ) * p_anime->rotateDataSize );
  for( i=0; i<p_anime->rotateDataSize; i++ )
  {
    GFL_ARC_LoadDataOfs( &p_anime->rotateData[i], arc_id, dat_id, ofs, sizeof( float ) );  
    ofs += sizeof( float );
  }

  // 平行移動データ数を取得
  GFL_ARC_LoadDataOfs( &p_anime->transDataSize, arc_id, dat_id, ofs, sizeof( u16 ) ); 
  ofs += sizeof( u16 );

  // 平行移動データを取得
  p_anime->transData = GFL_HEAP_AllocMemory( heap_id, sizeof( float ) * p_anime->transDataSize );
  for( i=0; i<p_anime->transDataSize; i++ )
  {
    GFL_ARC_LoadDataOfs( &p_anime->transData[i], arc_id, dat_id, ofs, sizeof( float ) );  
    ofs += sizeof( float );
  }

  // <node_anm>要素の属性値を取得
  for( i=0; i<VALUE_TYPE_NUM; i++ )
  {
    GFL_ARC_LoadDataOfs( &p_anime->anime_attr[i].frameStep, arc_id, dat_id, ofs, sizeof( u8 ) );  
    ofs += sizeof( u8 );
    GFL_ARC_LoadDataOfs( &p_anime->anime_attr[i].dataSize, arc_id, dat_id, ofs, sizeof( u16 ) );  
    ofs += sizeof( u16 );
    GFL_ARC_LoadDataOfs( &p_anime->anime_attr[i].dataHead, arc_id, dat_id, ofs, sizeof( u16 ) );  
    ofs += sizeof( u16 );
  }
}

//-----------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける値を計算する
 *
 * @param p_anime    アニメーションデータ
 * @param anime_type 取得する値のタイプ
 * @param now_frame  現在のフレーム数
 *
 * @return 指定フレーム時の値
 */
//-----------------------------------------------------------------------------------
static float GetValue( ICA_DATA* p_anime, VALUE_TYPE anime_type, fx32 now_frame )
{
  float*            p_value = NULL;
  ANIME_ATTR* p_attr = &p_anime->anime_attr[ anime_type ];

  // 取得したい値の配列を決定
  switch( anime_type )
  {
  case VALUE_TYPE_SCALE_X:
  case VALUE_TYPE_SCALE_Y:
  case VALUE_TYPE_SCALE_Z:
    p_value = p_anime->scaleData;
    break;
  case VALUE_TYPE_ROTATE_X:
  case VALUE_TYPE_ROTATE_Y:
  case VALUE_TYPE_ROTATE_Z:
    p_value = p_anime->rotateData;
    break;
  case VALUE_TYPE_TRANSLATE_X:
  case VALUE_TYPE_TRANSLATE_Y:
  case VALUE_TYPE_TRANSLATE_Z:
    p_value = p_anime->transData;
    break;
  default:
    return 0;
  }

  // 値を求める
  return CalcLinearValue(
      p_anime->frameSize, FX_FX32_TO_F32(now_frame),
      p_attr->frameStep, p_attr->dataSize, p_attr->dataHead, p_value );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 線形補完で指定フレームの値を求める
 *
 * @param frame_size 総フレーム数
 * @param now_frame  現在のフレーム
 * @param frame_step 間引き具合
 * @param data_size  データ数
 * @param data_head  先頭データインデックス
 * @param value      実データ配列
 *
 * @return 指定フレーム時の値
 */
//-----------------------------------------------------------------------------------
static float CalcLinearValue( 
    const int frame_size, const float now_frame, 
    const int frame_step, const int data_size, const int data_head, const float* value )
{
  int frame;
  int shift, ival, iframe_last_interp;
  int weight0, weight1;
  float val0, val1;

  // ループさせる
  frame = (int)now_frame % frame_size;

  if( data_size == 1 ) // データが１つの時
  {
    return value[ data_head ];
  }

  if(frame_step == 1) // frame_stepが１の時
  {
    return value[ data_head + frame ];
  }

  shift = ( frame_step==2 ) ? 1 : 2;  // シフト値
  ival  = frame >> shift;             // 現在のフレームに対応する実データ位置
  ival += data_head;

  // 間引き部分と 全フレーム部分のしきいとなるフレーム
  iframe_last_interp = ((frame_size - 1) >> shift) << shift;

  // しきいとなるフレーム以降の場合、実データ配列から直接返す
  if( frame >= iframe_last_interp )
  {
    return value[ival + (frame - iframe_last_interp)];
  }

  // しきいとなるフレームより前の場合は補間計算で求める
  val0    = value[ival ];
  val1    = value[ival + 1];
  weight1 = frame - ( ival << shift );
  weight0 = frame_step - weight1;

  // ちょうどフレーム上にある時
  if( weight1 == 0 )
  {
    return val0;
  }
  // 通常の補間処理
  else 
  {
    return ( (val0 * weight0 + val1 * weight1) / ( weight0 + weight1 ) );
  }
} 
