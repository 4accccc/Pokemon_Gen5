//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		snd_strm.h
 *	@brief	サウンド	ストリーミング再生
 *	@author Toru=Nagihashi
 *	@data		20101104
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __SND_STRM_H__
#define __SND_STRM_H__

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					外部宣言
*/
//-----------------------------------------------------------------------------
// 初期化・破棄・メイン
//  ストリームは使用箇所が限定されているため、
//  この処理がmainで回っていません。
//  各アプリごとに初期化・破棄・メイン処理を呼んでください
extern void SND_STRM_Init( HEAPID heapID );
extern void SND_STRM_Exit( void );
extern void SND_STRM_Main( void );

// 事前読み込み
//  事前読み込みをしなくてもPlayすることができますが、
//  読み込みが発生するため、数フレーム遅れます。
//  ぴったりのタイミングで鳴らし始めたい場合、
//  事前にLoadしてからPlayして下さい。
extern void SND_STRM_LoadEx( s32 strmNo, u32 offset );
#define SND_STRM_Load( strmNo ) SND_STRM_LoadEx( strmNo, 0 )

// 再生、停止処理
extern void SND_STRM_Play( s32 strmNo );
extern void SND_STRM_Stop( void );
extern BOOL SND_STRM_CheckPlay( void );

// ボリューム操作
extern void SND_STRM_Volume( int volume );
extern void SND_STRM_FadeOut( int fade_frame );

//パンの設定やポーズ、スタートも可能ですが、
//使うかわからないので現在はありません。

//長さ
//  現在の曲の長さを取得します。Loadしてから使ってください
extern u32 SND_STRM_GetMicroSeconds( void );

#endif //__SND_STRM_H__
