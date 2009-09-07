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
  u32 frameStep; // 間引き具合
  u32 dataSize; // 有効データ数
  u32 dataHead; // 先頭データインデックス
}
ANIME_ATTR;

//===================================================================================
/**
 * @brief アニメーションデータ
 */
//===================================================================================
struct _ICA_DATA
{
  u32 frameSize;      // 総フレーム数
  u32 scaleDataSize;  // スケールデータ数
  u32 rotateDataSize; // 回転データ数
  u32 transDataSize;  // 平行移動データ数

  // 値タイプごとのアニメーション属性
  ANIME_ATTR anime_attr[ VALUE_TYPE_NUM ];

  // アーカイブデータハンドル
  ARCHANDLE* arcHandle;
  ARCDATID datID;
};


//===================================================================================
/**
 * @brief プロトタイプ宣言
 */
//===================================================================================
static void LoadIcaData( ICA_DATA* p_data, HEAPID heap_id, ARCID arc_id, ARCDATID dat_id );
static float GetValue( ICA_DATA* p_data, VALUE_TYPE anime_type, fx32 now_frame );
static float CalcLinearValue( ICA_DATA* ica_data, VALUE_TYPE val_type, const float now_frame );
static float GetDataByHandle( ICA_DATA* ica_data, VALUE_TYPE val_type, int val_index );


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

  // アーカイブデータハンドルオープン
  data->arcHandle = GFL_ARC_OpenDataHandle( arc_id, heap_id );
  data->datID = dat_id;

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
  GFL_ARC_CloseDataHandle( p_data->arcHandle );
  GFL_HEAP_FreeMemory( p_data );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける平行移動データを取得する
 *
 * @param p_data   アニメーションデータ
 * @param p_vec     取得した座標の格納先
 * @param now_frame 現在のフレーム数
 */
//-----------------------------------------------------------------------------------
void ICA_DATA_GetTranslate( ICA_DATA* p_data, VecFx32* p_vec, fx32 now_frame )
{ 
  float x = GetValue( p_data, VALUE_TYPE_TRANSLATE_X, now_frame );
  float y = GetValue( p_data, VALUE_TYPE_TRANSLATE_Y, now_frame );
  float z = GetValue( p_data, VALUE_TYPE_TRANSLATE_Z, now_frame );

  VEC_Set( p_vec, FX_F32_TO_FX32(x), FX_F32_TO_FX32(y), FX_F32_TO_FX32(z) );
}

//---------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける回転データを取得する
 *
 * @param p_data     アニメーションデータ
 * @param p_vec      取得した回転の格納先
 * @param now_frame  現在のフレーム数
 */
//---------------------------------------------------------------------------
void ICA_DATA_GetRotate( ICA_DATA* p_data, VecFx32* p_vec, fx32 now_frame )
{ 
  float x = GetValue( p_data, VALUE_TYPE_ROTATE_X, now_frame );
  float y = GetValue( p_data, VALUE_TYPE_ROTATE_Y, now_frame );
  float z = GetValue( p_data, VALUE_TYPE_ROTATE_Z, now_frame );

  VEC_Set( p_vec, FX_F32_TO_FX32(x), FX_F32_TO_FX32(y), FX_F32_TO_FX32(z) );
}

//---------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける向きデータを取得する
 *
 * @param p_data        アニメーションデータ
 * @param p_vec_forward  取得した前方ベクトルの格納先
 * @param p_vec_upward   取得した上方ベクトルの格納先
 * @param now_frame      現在のフレーム数
 */
//---------------------------------------------------------------------------
void ICA_DATA_GetDir( 
    ICA_DATA* p_data, VecFx32* p_vec_forward, VecFx32* p_vec_upward, fx32 now_frame )
{
  float x, y, z;
  u16 rx, ry, rz;
  MtxFx33 matrix;
  VecFx32 def_forward = { 0, 0, -FX32_ONE };
  VecFx32 def_upward  = { 0, FX32_ONE, 0 };

  // 指定フレームの値を取得
  x = GetValue( p_data, VALUE_TYPE_ROTATE_X, now_frame );
  y = GetValue( p_data, VALUE_TYPE_ROTATE_Y, now_frame );
  z = GetValue( p_data, VALUE_TYPE_ROTATE_Z, now_frame );

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
  MTX_MultVec33( &def_forward, &matrix, p_vec_forward );
  MTX_MultVec33( &def_upward, &matrix, p_vec_upward );
}

//---------------------------------------------------------------------------
/**
 * @brief カメラ座標・ターゲット座標を指定フレームの状態に設定する
 *
 * @param p_data   アニメーションデータ
 * @param p_camera  設定対象のカメラ
 * @param now_frame 現在のフレーム数
 */
//---------------------------------------------------------------------------
void ICA_DATA_SetCameraStatus( 
    ICA_DATA* p_data, GFL_G3D_CAMERA* p_camera, fx32 now_frame )
{
  VecFx32 pos, target, forward, upward;

  // 座標・向きベクトルを取得
  ICA_DATA_GetTranslate( p_data, &pos, now_frame );
  ICA_DATA_GetDir( p_data, &forward, &upward, now_frame );
  VEC_Add( &pos, &forward, &target );

  // カメラの設定
  GFL_G3D_CAMERA_SetPos( p_camera, &pos );
  GFL_G3D_CAMERA_SetTarget( p_camera, &target );
  GFL_G3D_CAMERA_SetCamUp( p_camera, &upward );
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
 * @param p_data 読み込んだデータの格納先変数
 * @param heap_id 使用するヒープID
 * @param arc_id  読み込むアーカイブファイル
 * @param dat_id  アーカイブファイル上のインデックスナンバー
 */
//----------------------------------------------------------------------------------- 
static void LoadIcaData( ICA_DATA* p_data, HEAPID heap_id, ARCID arc_id, ARCDATID dat_id )
{
  int i;
  void* data = NULL;
  int size = 0;
  int ofs = 0;

  // データを取得
  size = GFL_ARC_GetDataSize( arc_id, dat_id );
  data = GFL_HEAP_AllocMemoryLo( heap_id, size ); 
  GFL_ARC_LoadData( data, arc_id, dat_id );


  // フレームサイズを取得
  p_data->frameSize = *( (u32*)( (int)data + ofs ) );
  ofs += sizeof( u32 );

  // スケールデータ数を取得
  p_data->scaleDataSize = *( (u32*)( (int)data + ofs ) );
  ofs += sizeof( u32 );
  OBATA_Printf( "scaleDataSize = %d\n", p_data->scaleDataSize );
  ofs += p_data->scaleDataSize * sizeof( float );

  // 回転データ数を取得
  p_data->rotateDataSize = *( (u32*)( (int)data + ofs ) );
  ofs += sizeof( u32 );
  OBATA_Printf( "rotateDataSize = %d\n", p_data->rotateDataSize );
  ofs += p_data->rotateDataSize * sizeof( float );

  // 平行移動データ数を取得
  p_data->transDataSize = *( (u32*)( (int)data + ofs ) );
  ofs += sizeof( u32 );
  OBATA_Printf( "transDataSize = %d\n", p_data->transDataSize );
  ofs += p_data->transDataSize * sizeof( float );

  // <node_anm>要素の属性値を取得
  for( i=0; i<VALUE_TYPE_NUM; i++ )
  {
    p_data->anime_attr[i].frameStep = *( (u32*)( (int)data + ofs ) );
    ofs += sizeof( u32 );
    p_data->anime_attr[i].dataSize = *( (u32*)( (int)data + ofs ) );
    ofs += sizeof( u32 );
    p_data->anime_attr[i].dataHead = *( (u32*)( (int)data + ofs ) );
    ofs += sizeof( u32 );
    OBATA_Printf( "%d %d %d\n", 
        p_data->anime_attr[i].frameStep,
        p_data->anime_attr[i].dataSize,
        p_data->anime_attr[i].dataHead );
  }

  // 後始末
  GFL_HEAP_FreeMemory( data );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける値を計算する
 *
 * @param p_data    アニメーションデータ
 * @param anime_type 取得する値のタイプ
 * @param now_frame  現在のフレーム数
 *
 * @return 指定フレーム時の値
 */
//-----------------------------------------------------------------------------------
static float GetValue( ICA_DATA* p_data, VALUE_TYPE anime_type, fx32 now_frame )
{
  // 値を求める
  return CalcLinearValue( p_data, anime_type, FX_FX32_TO_F32(now_frame) );
}

//-----------------------------------------------------------------------------------
/**
 * @brief 線形補完で指定フレームの値を求める
 *
 * @param ica_data   アニメーション・データ
 * @param val_type   取得値のタイプ
 * @param now_frame  フレーム指定
 *
 * @return 指定フレームにおける値
 */
//-----------------------------------------------------------------------------------
static float CalcLinearValue( ICA_DATA* ica_data, VALUE_TYPE val_type, const float now_frame )
{
  const int frame_size = ica_data->frameSize;
  const int frame_step = ica_data->anime_attr[ val_type ].frameStep;
  const int data_size  = ica_data->anime_attr[ val_type ].dataSize;
  const int data_head  = ica_data->anime_attr[ val_type ].dataHead;
  int frame;
  int shift, ival, iframe_last_interp;
  int weight0, weight1;
  float val0, val1;

  // ループさせる
  frame = (int)now_frame % frame_size;

  if( data_size == 1 ) // データが１つの時
  {
    ival = data_head;
    return GetDataByHandle( ica_data, val_type, ival );
  }

  if( frame_step == 1 ) // frame_stepが１の時
  {
    ival = data_head + frame;
    return GetDataByHandle( ica_data, val_type, ival );
  }

  shift = ( frame_step==2 ) ? 1 : 2;  // シフト値
  ival  = frame >> shift;             // 現在のフレームに対応する実データ位置
  ival += data_head;

  // 間引き部分と 全フレーム部分のしきいとなるフレーム
  iframe_last_interp = ((frame_size - 1) >> shift) << shift;

  // しきいとなるフレーム以降の場合、実データ配列から直接返す
  if( frame >= iframe_last_interp )
  {
    ival = ival + ( frame - iframe_last_interp );
    return GetDataByHandle( ica_data, val_type, ival );
  }

  // しきいとなるフレームより前の場合は補間計算で求める
  val0    = GetDataByHandle( ica_data, val_type, ival );
  val1    = GetDataByHandle( ica_data, val_type, ival+1 );
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

//-----------------------------------------------------------------------------------
/**
 * @brief アーカイブデータハンドルから, 指定データを取得する
 *
 * @param ica_data  icaデータ
 * @param val_type  取得値のタイプ
 * @param val_index 取得値のインデックス
 *
 * @return 指定データの指定フレームにおける値
 */
//-----------------------------------------------------------------------------------
static float GetDataByHandle( ICA_DATA* ica_data, VALUE_TYPE val_type, int val_index )
{
  int ofs;
  float val;

  // 取得する値のデータ内オフセットを算出
  switch( val_type )
  {
  case VALUE_TYPE_SCALE_X:
  case VALUE_TYPE_SCALE_Y:
  case VALUE_TYPE_SCALE_Z:
    ofs = sizeof(u32) 
        + sizeof(u32) + ( val_index * sizeof(float) );
    break;
  case VALUE_TYPE_ROTATE_X:
  case VALUE_TYPE_ROTATE_Y:
  case VALUE_TYPE_ROTATE_Z: 
    ofs = sizeof(u32) 
        + sizeof(u32) + ( ica_data->scaleDataSize * sizeof(float) )
        + sizeof(u32) + ( val_index * sizeof(float) );
    break;
  case VALUE_TYPE_TRANSLATE_X:
  case VALUE_TYPE_TRANSLATE_Y:
  case VALUE_TYPE_TRANSLATE_Z:
    ofs = sizeof(u32) 
        + sizeof(u32) + ( ica_data->scaleDataSize * sizeof(float) )
        + sizeof(u32) + ( ica_data->rotateDataSize * sizeof(float) )
        + sizeof(u32) + ( val_index * sizeof(float) );
    break;
  }

  // 値を取得
  GFL_ARC_LoadDataOfsByHandle( ica_data->arcHandle, ica_data->datID, ofs, sizeof(float), &val );
  return val;
}
