//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		snd_strm.h
 *	@brief		サウンド	ストリーミング再生
 *	@author		tomoya takahashi
 *	@data		2008.12.02
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
//-------------------------------------
///	サウンドデータタイプ
//=====================================
typedef enum {
	SND_STRM_PCM8,
	SND_STRM_PCM16,

	SND_STRM_TYPE_MAX,	// システム内で使用
} SND_STRM_TYPE;

//-------------------------------------
///	周波数タイプ
//=====================================
typedef enum {
	SND_STRM_8KHZ,		// 8.000KHz
	SND_STRM_11KHZ,		// 11.025KHz
  SND_STRM_16KHZ,   // 16.000KHz
	SND_STRM_22KHZ,		// 22.050KHz
	SND_STRM_32KHZ,		// 32.000KHz

	SND_STRM_HZMAX,		// システム内で使用
} SND_STRM_HZ;

//-------------------------------------
///	ボリューム
//=====================================
#define SND_STRM_VOLUME_MIN	(-1023)
#define SND_STRM_VOLUME_MAX	(0)


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// システムの初期化
extern void SND_STRM_Init( u32 heapID );
extern void SND_STRM_Exit( void );
extern void SND_STRM_Main( void );

// 音楽情報の読み込み・破棄
extern void SND_STRM_SetUp( u32 arcid, u32 dataid, SND_STRM_TYPE type, SND_STRM_HZ hz, u32 heapID );
extern void SND_STRM_SetUpStraightData( SND_STRM_TYPE type, SND_STRM_HZ hz, u32 heapID, u8* data,u32 size);
extern void SND_STRM_Release( void );
extern BOOL SND_STRM_CheckSetUp( void );

// 再生、停止処理
extern void SND_STRM_Play( void );
extern void SND_STRM_Stop( void );
extern BOOL SND_STRM_CheckPlay( void );
extern const BOOL SND_STRM_CheckFinish( void );

// ループ
extern void SND_STRM_SetLoopFlg( const BOOL isLoop );
extern const BOOL SND_STRM_GetLoopFlg( void );

// ボリューム操作
extern void SND_STRM_Volume( int volume );

#endif		// __SND_STRM_H__

