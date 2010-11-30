//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		snd_strm.h
 *	@brief		サウンド	ストリーミング再生
 *	@author		tomoya takahashi  -> Toru=Nagihashi
 *	@data		2008.12.02  -> 20101104
 *	@note   神王蟲プロジェクトで使用していたソースを移植
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __SND_STRM_H__
#define __SND_STRM_H__

#include "snd_strm.naix"

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
 *					外部宣言
 *
 *					・使い方
 *
 *					初期読込み  SND_STRM_Setup
 *					読込み破棄  SND_STRM_Release
 *
 *					あとはPlayやStopを使って鳴らしてください。
*/
//-----------------------------------------------------------------------------
// システムで使用するもの
extern void SND_STRM_Init( HEAPID heapID );
extern void SND_STRM_Exit( void );
extern void SND_STRM_Main( void );

// 音楽情報の読み込み・破棄
extern void SND_STRM_SetUp( ARCID arcid, ARCDATID dataid, HEAPID heapID );
//Setupは基本、Exを使わないでください。Exでは本来の曲と違うヘルツやタイプを指定してしまう可能性があるためです。
extern void SND_STRM_SetUpEx( ARCID arcid, ARCDATID dataid, SND_STRM_TYPE type, SND_STRM_HZ hz, HEAPID heapID );
extern void SND_STRM_Release( void );
extern BOOL SND_STRM_CheckSetUp( void );

// 再生、停止処理
extern void SND_STRM_Play( void );
extern void SND_STRM_Stop( void );
extern BOOL SND_STRM_CheckPlay( void );

// ボリューム操作
extern void SND_STRM_Volume( int volume );

//デバッグ情報
#ifdef PM_DEBUG
extern STRBUF * SND_STRM_DEBUG_CreateString( ARCID arcid, ARCDATID dataid, HEAPID heapID );
extern SND_STRM_HZ SND_STRM_DEBUG_GetHertz( ARCID arcid, ARCDATID dataid );
extern SND_STRM_TYPE SND_STRM_DEBUG_GetType( ARCID arcid, ARCDATID dataid );
extern u32  SND_STRM_DEBUG_GetSndStrmMax( void );
#endif //PM_DEBUG

#endif		// __SND_STRM_H__

