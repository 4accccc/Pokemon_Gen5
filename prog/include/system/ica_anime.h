/////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  icaアニメーション ストリーミング再生
 * @file   ica_anime.h
 * @author obata
 * @date   2009.09.04
 *
 */
/////////////////////////////////////////////////////////////////////////////////////
#pragma once


//===================================================================================
// ■不完全型の宣言
//===================================================================================
typedef struct _ICA_ANIME ICA_ANIME;


//===================================================================================
// ■関数一覧
//===================================================================================
/*
ICA_ANIME* ICA_ANIME_Create( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, void* work, int worksize ); 
ICA_ANIME* ICA_ANIME_CreateStreaming( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, int buf_interval, 
    void* work, int worksize ); 
ICA_ANIME* ICA_ANIME_CreateAlloc( HEAPID heap_id, ARCID arc_id, ARCDATID dat_id ); 
ICA_ANIME* ICA_ANIME_CreateStreamingAlloc( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, int buf_interval ); 
void ICA_ANIME_Delete( ICA_ANIME* anime );
BOOL ICA_ANIME_IncAnimeFrame( ICA_ANIME* anime, fx32 frame );
void ICA_ANIME_SetAnimeFrame( ICA_ANIME* anime, fx32 frame );
u32 ICA_ANIME_GetMaxFrame( ICA_ANIME* anime );
fx32 ICA_ANIME_GetNowFrame( ICA_ANIME* anime );
BOOL ICA_ANIME_GetTranslate( const ICA_ANIME* anime, VecFx32* vec_dst );
BOOL ICA_ANIME_GetRotate( const ICA_ANIME* anime, VecFx32* vec_dst );
BOOL ICA_ANIME_GetScale( const ICA_ANIME* anime, VecFx32* vec_dst );
BOOL ICA_ANIME_GetTranslateAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame );
BOOL ICA_ANIME_GetRotateAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame );
BOOL ICA_ANIME_GetScaleAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame );
*/

//===================================================================================
// ■作成・破棄
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
extern ICA_ANIME* ICA_ANIME_Create( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, void* work, int worksize );

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
extern ICA_ANIME* ICA_ANIME_CreateStreaming( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, int buf_interval, 
    void* work, int worksize );

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを作成する 
 *        (内部でバッファを確保・データ一括読み込み)
 *
 * @param heap_id 使用するヒープID
 * @param arc_id  アニメーションデータのアーカイブファイル指定
 * @param dat_id  アーカイブファイル上のインデックスナンバー
 *
 * @return 作成したアニメーション管理オブジェクト
 */
//-----------------------------------------------------------------------------------
extern ICA_ANIME* ICA_ANIME_CreateAlloc( HEAPID heap_id, ARCID arc_id, ARCDATID dat_id );

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを作成する
 *        (内部でバッファを確保・ストリーミング再生)
 *
 * @param heap_id      使用するヒープID
 * @param arc_id       アニメーションデータのアーカイブファイル指定
 * @param dat_id       アーカイブファイル上のインデックスナンバー
 * @param buf_interval データ読み込みの間隔 (指定フレーム数に一度, 読み込みを行う)
 *
 * @return 作成したアニメーション管理オブジェクト
 */
//-----------------------------------------------------------------------------------
extern ICA_ANIME* ICA_ANIME_CreateStreamingAlloc( 
    HEAPID heap_id, ARCID arc_id, ARCDATID dat_id, int buf_interval );

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションを破棄する
 *
 * @param anime 破棄するアニメーション
 */
//-----------------------------------------------------------------------------------
extern void ICA_ANIME_Delete( ICA_ANIME* anime );


//===================================================================================
// ■アニメーション更新
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
extern BOOL ICA_ANIME_IncAnimeFrame( ICA_ANIME* anime, fx32 frame );

//-----------------------------------------------------------------------------------
/**
 * @brief 現在フレームを指定する
 *
 * @param anime 設定するアニメーション
 * @param frame フレーム数指定
 */
//-----------------------------------------------------------------------------------
extern void ICA_ANIME_SetAnimeFrame( ICA_ANIME* anime, fx32 frame );


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
extern u32 ICA_ANIME_GetMaxFrame( ICA_ANIME* anime );

//-----------------------------------------------------------------------------------
/**
 * @brief アニメーションの現在のフレーム数を取得する
 *
 * @param anime 取得対象アニメーション
 * @return 指定したアニメーション全体のフレーム数
 */
//-----------------------------------------------------------------------------------
extern fx32 ICA_ANIME_GetNowFrame( ICA_ANIME* anime );


//===================================================================================
// ■アニメーション取得 (現在フレーム版)
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
extern BOOL ICA_ANIME_GetTranslate( const ICA_ANIME* anime, VecFx32* vec_dst );

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
extern BOOL ICA_ANIME_GetRotate( const ICA_ANIME* anime, VecFx32* vec_dst );

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
extern BOOL ICA_ANIME_GetScale( const ICA_ANIME* anime, VecFx32* vec_dst );


//===================================================================================
// ■アニメーション取得 (フレーム指定版)
// ※全フレーム分のデータを一括読み込みした場合のみ取得可能
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
extern BOOL ICA_ANIME_GetTranslateAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame );

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
extern BOOL ICA_ANIME_GetRotateAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame );

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
extern BOOL ICA_ANIME_GetScaleAt( const ICA_ANIME* anime, VecFx32* vec_dst, int frame );
