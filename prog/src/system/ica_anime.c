/////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  icaアニメーション ストリーミング再生
 * @file   ica_anime.c
 * @author obata
 * @date   2009.09.04
 *
 */
/////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/ica_anime.h"

// デバッグ出力スイッチ
//#define ICA_ANIME_DEBUG


//===================================================================================
/**
 * @brief icaデータ管理ワーク
 */
//===================================================================================
struct _ICA_ANIME
{
  HEAPID     heapID;        // 使用するヒープID
  ARCHANDLE* arcHandle;     // アーカイブハンドル
  ARCDATID   datID;         // アーカイブ内データインデックス
  u32        frameSize;     // アニメーションの全フレーム数
  fx32       nowFrame;      // 現在のフレーム数
  int        bufStartFrame; // バッファ先頭データのフレーム数
  int        bufSize;       // バッファサイズ
  VecFx32*   transBuf;      // 平行移動バッファ
  VecFx32*   rotateBuf;     // 回転バッファ
  VecFx32*   scaleBuf;      // 拡縮バッファ
  BOOL       haveTrans;     // 平行移動データの有無
  BOOL       haveRotate;    // 回転データの有無
  BOOL       haveScale;     // 拡縮データの有無
  BOOL       allocMemory;   // バッファ確保フラグ
};


//===================================================================================
/**
 * @brief プロトタイプ宣言
 */
//===================================================================================
static void InitAnime( ICA_ANIME* anime );
static void LoadAnimeInfo( ICA_ANIME* anime, ARCID arc_id, ARCDATID dat_id, HEAPID heap_id );
static void AllocBuffer( ICA_ANIME* anime, int buf_size );
static BOOL AssignBuffer( ICA_ANIME* anime, int bufsize, void* work, int worksize );
static void UpdateBuf( ICA_ANIME* anime, fx32 start_frame );


//===================================================================================
/**
 * @breif 公開関数の定義
 */
//===================================================================================

//===================================================================================
// 作成・破棄
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを作成する (データ一括読み込み)
 *
 * @param heap_id  使用するヒープID
 * @param arc_id   アニメーションデータのアーカイブファイル指定
 * @param dat_id   アーカイブファイル上のインデックスナンバー
 * @param work     内部バッファとして使用するワーク
 * @param worksize ワークサイズ
 *
 * @return 作成したアニメーション管理オブジェクト
 *         作成に失敗したら NULL
 */
//-----------------------------------------------------------------------------------
ICA_ANIME* ICA_ANIME_Create( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, void* work, int worksize )
{
  ICA_ANIME* anime;

  // アニメーション管理ワークを確保
  anime = (ICA_ANIME*)GFL_HEAP_AllocMemory( heap_id, sizeof(ICA_ANIME) );

  // 初期化
  InitAnime( anime );
  LoadAnimeInfo( anime, arc_id, dat_id, heap_id );
  if( !AssignBuffer( anime, anime->frameSize, work, worksize ) ) return NULL;
  UpdateBuf( anime, 0 );

  return anime;
}

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを作成する (ストリーミング再生)
 *
 * @param heap_id      使用するヒープID
 * @param arc_id       アニメーションデータのアーカイブファイル指定
 * @param dat_id       アーカイブファイル上のインデックスナンバー
 * @param buf_interval データ読み込みの間隔 (指定フレーム数に一度, 読み込みを行う)
 * @param work         内部バッファとして使用するワーク
 * @param worksize     ワークサイズ
 *
 * @return 作成したアニメーション管理オブジェクト
 *         作成に失敗したら NULL
 */
//-----------------------------------------------------------------------------------
ICA_ANIME* ICA_ANIME_CreateStreaming( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, int buf_interval, void* work, int worksize )
{
  ICA_ANIME* anime;

  // アニメーション管理ワークを確保
  anime = (ICA_ANIME*)GFL_HEAP_AllocMemory( heap_id, sizeof(ICA_ANIME) );

  // 初期化
  InitAnime( anime );
  LoadAnimeInfo( anime, arc_id, dat_id, heap_id );
  if( !AssignBuffer( anime, buf_interval, work, worksize ) ) return NULL;
  UpdateBuf( anime, 0 );

  return anime;
}

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを作成する
 *        ( 内部でバッファを確保・データ一括読み込み )
 *
 * @param heap_id 使用するヒープID
 * @param arc_id  アニメーションデータのアーカイブファイル指定
 * @param dat_id  アーカイブファイル上のインデックスナンバー
 *
 * @return 作成したアニメーション管理オブジェクト
 */
//-----------------------------------------------------------------------------------
ICA_ANIME* ICA_ANIME_CreateAlloc( HEAPID heap_id, ARCID arc_id, ARCDATID dat_id )
{
  ICA_ANIME* anime;

  // アニメーション管理ワークを確保
  anime = (ICA_ANIME*)GFL_HEAP_AllocMemory( heap_id, sizeof(ICA_ANIME) );

  // 初期化
  InitAnime( anime );
  LoadAnimeInfo( anime, arc_id, dat_id, heap_id );
  AllocBuffer( anime, anime->frameSize );   // 全フレーム分のバッファが必要
  UpdateBuf( anime, 0 );

  return anime;
}

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを作成する
 *        ( 内部でバッファを確保・ストリーミング再生 )
 *
 * @param heap_id      使用するヒープID
 * @param arc_id       アニメーションデータのアーカイブファイル指定
 * @param dat_id       アーカイブファイル上のインデックスナンバー
 * @param buf_interval データ読み込みの間隔 (指定フレーム数に一度, 読み込みを行う)
 *
 * @return 作成したアニメーション管理オブジェクト
 */
//-----------------------------------------------------------------------------------
ICA_ANIME* ICA_ANIME_CreateStreamingAlloc( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, int buf_interval )
{
  ICA_ANIME* anime;

  // アニメーション管理ワークを確保
  anime = (ICA_ANIME*)GFL_HEAP_AllocMemory( heap_id, sizeof(ICA_ANIME) );

  // 初期化
  InitAnime( anime );
  LoadAnimeInfo( anime, arc_id, dat_id, heap_id );
  AllocBuffer( anime, buf_interval );   // バッファサイズは読み込み間隔
  UpdateBuf( anime, 0 );

  return anime;
}

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを破棄する
 *
 * @param anime 破棄するアニメーション
 */
//-----------------------------------------------------------------------------------
void ICA_ANIME_Delete( ICA_ANIME* anime )
{
  GFL_ARC_CloseDataHandle( anime->arcHandle );
  if( anime->allocMemory )  // 各バッファはクラス内で確保した場合のみ解放する
  {
    if( anime->transBuf )
    {
      GFL_HEAP_FreeMemory( anime->transBuf );
    }
    if( anime->rotateBuf ) 
    {
      GFL_HEAP_FreeMemory( anime->rotateBuf );
    }
    if( anime->scaleBuf )  
    {
      GFL_HEAP_FreeMemory( anime->scaleBuf );
    }
  }
  GFL_HEAP_FreeMemory( anime );
}


//===================================================================================
// アニメーション更新
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを進める
 *
 * @param anime 更新するアニメーション
 * @param frame 進めるフレーム数
 *
 * @return ループしたら TRUE
 */
//-----------------------------------------------------------------------------------
BOOL ICA_ANIME_IncAnimeFrame( ICA_ANIME* anime, fx32 frame )
{
  int now;
  BOOL loop = FALSE;

  // 現在フレームを更新
  anime->nowFrame += frame;
  now = anime->nowFrame >> FX32_SHIFT;

  // 最終フレームを超えたら, ゼロフレーム目に戻る
  if( anime->frameSize <= now )
  {
    anime->nowFrame = 0;
    now = 0;
    loop = TRUE;
  }

  // 現在フレームがバッファリング範囲をはずれたら, バッファを更新する
  if( ( now < anime->bufStartFrame ) ||
      ( (anime->bufStartFrame + anime->bufSize) <= now ) )
  {
    UpdateBuf( anime, anime->nowFrame );
  }

  // ループしたかどうかを返す
  return loop;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 現在フレームを指定する
 *
 * @param anime 設定するアニメーション
 * @param frame フレーム数指定
 */
//-----------------------------------------------------------------------------------
void ICA_ANIME_SetAnimeFrame( ICA_ANIME* anime, fx32 frame )
{
  int now;

  // フレーム数を指定
  anime->nowFrame = frame;
  now = anime->nowFrame >> FX32_SHIFT;

  // 現在フレームがバッファリング範囲をはずれたら, バッファを更新する
  if( ( now < anime->bufStartFrame ) ||
      ( (anime->bufStartFrame + anime->bufSize) <= now ) )
  {
    UpdateBuf( anime, anime->nowFrame );
  }
}


//===================================================================================
// ■データ取得
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションの総フレーム数を取得する
 *
 * @param anime 取得対象アニメーション
 * @return 指定したアニメーション全体のフレーム数
 */
//-----------------------------------------------------------------------------------
u32 ICA_ANIME_GetMaxFrame( ICA_ANIME* anime )
{
  return anime->frameSize;
}

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションの現在のフレーム数を取得する
 *
 * @param anime 取得対象アニメーション
 * @return 指定したアニメーション全体のフレーム数
 */
//-----------------------------------------------------------------------------------
fx32 ICA_ANIME_GetNowFrame( ICA_ANIME* anime )
{
  return anime->nowFrame;
}


//===================================================================================
// アニメーション取得
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief 現在フレームにおける平行移動データを取得する
 *
 * @param anime   取得対象アニメーション
 * @param vec_dst 取得したデータの格納先
 *
 * @return データを取得できた場合 TRUE
 */
//-----------------------------------------------------------------------------------
BOOL ICA_ANIME_GetTranslate( const ICA_ANIME* anime, VecFx32* vec_dst )
{
  // 読み込んだファイルに平行移動データが存在する場合のみ取得可能
  if( anime->haveTrans )
  {
    // バッファ内インデックスを計算
    int buf_index = (anime->nowFrame>>FX32_SHIFT) - anime->bufStartFrame;

    // 現在フレームの値を取得
    *vec_dst = anime->transBuf[ buf_index ];
    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 現在フレームにおける回転データを取得する
 *
 * @param anime   取得対象アニメーション
 * @param vec_dst 取得したデータの格納先
 *
 * @return データを取得できた場合 TRUE
 */
//-----------------------------------------------------------------------------------
BOOL ICA_ANIME_GetRotate( const ICA_ANIME* anime, VecFx32* vec_dst )
{
  // 読み込んだファイルに回転データが存在する場合のみ取得可能
  if( anime->haveRotate )
  {
    // バッファ内インデックスを計算
    int buf_index = (anime->nowFrame>>FX32_SHIFT) - anime->bufStartFrame;

    // 現在フレームの値を取得
    *vec_dst = anime->rotateBuf[ buf_index ];
    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 現在フレームにおける拡縮データを取得する
 *
 * @param anime   取得対象アニメーション
 * @param vec_dst 取得したデータの格納先
 *
 * @return データを取得できた場合 TRUE
 */
//-----------------------------------------------------------------------------------
BOOL ICA_ANIME_GetScale( const ICA_ANIME* anime, VecFx32* vec_dst )
{
  // 読み込んだファイルに拡縮データが存在する場合のみ取得可能
  if( anime->haveScale )
  {
    // バッファ内インデックスを計算
    int buf_index = (anime->nowFrame>>FX32_SHIFT) - anime->bufStartFrame;

    // 現在フレームの値を取得
    *vec_dst = anime->scaleBuf[ buf_index ];
    return TRUE;
  }

  return FALSE;
} 


//===================================================================================
// アニメーション取得( フレーム指定版 )
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける平行移動データを取得する
 *
 * @param anime   取得対象アニメーション
 * @param vec_dst 取得したデータの格納先
 * @param frame   フレーム指定
 *
 * @return データを取得できた場合 TRUE
 */
//-----------------------------------------------------------------------------------
BOOL ICA_ANIME_GetTranslateAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame )
{
  // 指定フレームをループさせる
  frame = frame % anime->frameSize;

  // 全アニメーションデータを保持している場合にのみ, ランダムアクセスが可能とする
  if( anime->bufSize == anime->frameSize )
  {
    vec_dst->x = anime->transBuf[frame].x;
    vec_dst->y = anime->transBuf[frame].y;
    vec_dst->z = anime->transBuf[frame].z;
    return TRUE;
  }

  // ストリーミング再生時はランダムアクセス不可
  return FALSE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける回転データを取得する
 *
 * @param anime   取得対象アニメーション
 * @param vec_dst 取得したデータの格納先
 * @param frame   フレーム指定
 *
 * @return データを取得できた場合 TRUE
 */
//-----------------------------------------------------------------------------------
BOOL ICA_ANIME_GetRotateAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame )
{
  // 指定フレームをループさせる
  frame = frame % anime->frameSize;

  // 全アニメーションデータを保持している場合にのみ, ランダムアクセスが可能とする
  if( anime->bufSize == anime->frameSize )
  {
    vec_dst->x = anime->rotateBuf[frame].x;
    vec_dst->y = anime->rotateBuf[frame].y;
    vec_dst->z = anime->rotateBuf[frame].z;
    return TRUE;
  }

  // ストリーミング再生時はランダムアクセス不可
  return FALSE;
}

//-----------------------------------------------------------------------------------
/**
 * @brief 指定フレームにおける拡縮データを取得する
 *
 * @param anime   取得対象アニメーション
 * @param vec_dst 取得したデータの格納先
 * @param frame   フレーム指定
 *
 * @return データを取得できた場合 TRUE
 */
//-----------------------------------------------------------------------------------
BOOL ICA_ANIME_GetScaleAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame )
{
  // 指定フレームをループさせる
  frame = frame % anime->frameSize;

  // 全アニメーションデータを保持している場合にのみ, ランダムアクセスが可能とする
  if( anime->bufSize == anime->frameSize )
  {
    vec_dst->x = anime->scaleBuf[frame].x;
    vec_dst->y = anime->scaleBuf[frame].y;
    vec_dst->z = anime->scaleBuf[frame].z;
    return TRUE;
  }

  // ストリーミング再生時はランダムアクセス不可
  return FALSE;
}


//===================================================================================
/**
 * @breif 非公開関数の定義
 */
//===================================================================================

//---------------------------------------------------------------------------
/**
 * @brief アニメーション管理バッファを初期化する
 *
 * @param anime 初期化するアニメーション
 */
//---------------------------------------------------------------------------
static void InitAnime( ICA_ANIME* anime )
{
  anime->heapID        = 0;
  anime->arcHandle     = NULL;
  anime->datID         = 0;
  anime->frameSize     = 0;
  anime->nowFrame      = 0;
  anime->bufStartFrame = 0;
  anime->bufSize       = 0;
  anime->transBuf      = NULL;
  anime->rotateBuf     = NULL;
  anime->scaleBuf      = NULL;
  anime->haveTrans     = FALSE;
  anime->haveRotate    = FALSE;
  anime->haveScale     = FALSE;
  anime->allocMemory   = FALSE;
}

//---------------------------------------------------------------------------
/**
 * @brief アニメーション情報を読み込む
 *
 * @param anime   読み込んだ情報の格納先
 * @param arc_id  アニメーションデータのアーカイブファイル指定
 * @param dat_id  アーカイブファイル上のインデックスナンバー
 * @param heap_id 使用するヒープID
 */
//---------------------------------------------------------------------------
static void LoadAnimeInfo( ICA_ANIME* anime, ARCID arc_id, ARCDATID dat_id, HEAPID heap_id )
{
  u8 scale, rotate, trans;

  // 各種ID, ハンドルを記憶
  anime->heapID    = heap_id;
  anime->arcHandle = GFL_ARC_OpenDataHandle( arc_id, heap_id );
  anime->datID     = dat_id;

  // アニメーションフレーム数を取得
  GFL_ARC_LoadDataOfsByHandle( anime->arcHandle, dat_id, 0, sizeof(u32), &anime->frameSize );

  // 各アニメーションデータの有無を取得
  GFL_ARC_LoadDataOfsByHandle( anime->arcHandle, anime->datID, 4, sizeof(u8), &scale );
  GFL_ARC_LoadDataOfsByHandle( anime->arcHandle, anime->datID, 5, sizeof(u8), &rotate );
  GFL_ARC_LoadDataOfsByHandle( anime->arcHandle, anime->datID, 6, sizeof(u8), &trans );
  anime->haveScale  = ( scale  != 0 );
  anime->haveRotate = ( rotate != 0 );
  anime->haveTrans  = ( trans  != 0 ); 

#ifdef ICA_ANIME_DEBUG
  OBATA_Printf( "haveScale  = %d\n", scale );
  OBATA_Printf( "haveRotate = %d\n", rotate );
  OBATA_Printf( "haveTrans  = %d\n", trans );
#endif
}

//---------------------------------------------------------------------------
/**
 * @brief アニメーション情報に応じてバッファを確保する
 *
 * @param anime    バッファを確保するアニメーション
 * @param buf_size バッファサイズ
 */
//---------------------------------------------------------------------------
static void AllocBuffer( ICA_ANIME* anime, int buf_size )
{
  // バッファサイズを記憶
  anime->bufSize = buf_size;

  // 各種バッファを確保
  if( anime->haveTrans )
  {
    anime->transBuf = GFL_HEAP_AllocMemory( anime->heapID, sizeof(VecFx32) * buf_size );
  }
  if( anime->haveRotate )
  {
    anime->rotateBuf = GFL_HEAP_AllocMemory( anime->heapID, sizeof(VecFx32) * buf_size );
  }
  if( anime->haveScale )
  {
    anime->scaleBuf = GFL_HEAP_AllocMemory( anime->heapID, sizeof(VecFx32) * buf_size );
  }

  // バッファを確保したことを記憶する
  anime->allocMemory = TRUE;
}

//---------------------------------------------------------------------------
/**
 * @brief 確保済みのメモリを内部バッファに割り当てる
 *
 * @param anime    割り当て対象アニメーション 
 * @param bufsize  内部バッファの大きさ
 * @param work     割り当てるワーク
 * @param worksize ワークサイズ
 *
 * @return 正しく割り当てられたら, TRUE
 */
//---------------------------------------------------------------------------
static BOOL AssignBuffer( ICA_ANIME* anime, int bufsize, void* work, int worksize )
{
  int minsize = 0;
  int    size = sizeof(VecFx32) * bufsize; // バッファ1本あたりのサイズ
  int   index = 0;

  // 最低限必要なメモリ量を計算
  if( anime->haveTrans )  minsize += size;
  if( anime->haveRotate ) minsize += size;
  if( anime->haveScale )  minsize += size;

  // 指定されたワークが足りない場合
  if( worksize < minsize )
  {
    OBATA_Printf( "------------------------------------------\n" );
    OBATA_Printf( "ICA_ANIME に渡されたワークが小さすぎます。\n" );
    OBATA_Printf( "------------------------------------------\n" );
    return FALSE;
  }

  // ワークを割り当てる
  if( anime->haveTrans )
  {
    anime->transBuf = (VecFx32*)( (int)work + size * index );
    index++;
  }
  if( anime->haveRotate )
  {
    anime->rotateBuf = (VecFx32*)( (int)work + size * index );
    index++;
  }
  if( anime->haveScale )
  {
    anime->scaleBuf  = (VecFx32*)( (int)work + size * index );
    index++;
  }

  // バッファサイズを記憶
  anime->bufSize = bufsize;

  return TRUE;
}

//---------------------------------------------------------------------------
/**
 * @brief バッファを更新する
 * 
 * @param anime       更新対象のアニメーション管理オブジェクト
 * @param start_frame 新しいデータの開始フレーム
 */
//---------------------------------------------------------------------------
static void UpdateBuf( ICA_ANIME* anime, fx32 start_frame )
{
  int   i, j;
  int   frame     = (start_frame >> FX32_SHIFT) % anime->frameSize;
  int   ofs       = 8;  // ヘッダサイズ
  int   data_size = 0;
  void* data      = NULL;

  // 先頭データへのオフセットを計算
  if( anime->haveScale  ) ofs += sizeof(float)*3*frame;
  if( anime->haveRotate ) ofs += sizeof(float)*3*frame;
  if( anime->haveTrans  ) ofs += sizeof(float)*3*frame;

  // データサイズを計算
  if( anime->haveScale  ) data_size += sizeof(float)*3*anime->bufSize;
  if( anime->haveRotate ) data_size += sizeof(float)*3*anime->bufSize;
  if( anime->haveTrans  ) data_size += sizeof(float)*3*anime->bufSize;

  // 指定フレーム位置から 一定フレーム数分のデータを取得し, バッファ更新
  data = GFL_HEAP_AllocMemoryLo( anime->heapID, data_size );
  GFL_ARC_LoadDataOfsByHandle( anime->arcHandle, anime->datID, ofs, data_size, data );
  for( i=0, j=0; i<anime->bufSize; i++ )
  {
    if( anime->haveScale )
    {
      anime->scaleBuf[i].x = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
      anime->scaleBuf[i].y = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
      anime->scaleBuf[i].z = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
    }
    if( anime->haveRotate )
    {
      anime->rotateBuf[i].x = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
      anime->rotateBuf[i].y = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
      anime->rotateBuf[i].z = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
    }
    if( anime->haveTrans )
    {
      anime->transBuf[i].x = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
      anime->transBuf[i].y = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
      anime->transBuf[i].z = FX_F32_TO_FX32( *( (float*)( (int)data + j*sizeof(float) ) ) );   j++;
    }
  }
  GFL_HEAP_FreeMemory( data );

  // バッファ開始フレーム位置を記憶
  anime->bufStartFrame = start_frame >> FX32_SHIFT;

#ifdef ICA_ANIME_DEBUG
  OBATA_Printf( "update ica anime buffer\n" );
  OBATA_Printf( "frame     = %d\n", frame );
  OBATA_Printf( "ofs       = %d\n", ofs );
  OBATA_Printf( "data_size = %d\n", data_size );
  OBATA_Printf( "bufSize   = %d\n", anime->bufSize );
#endif
}
