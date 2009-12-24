//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_fk_sound_anime.h
 *	@brief  四天王　歩きデモ用　サウンドアニメーションシステム
 *	@author	tomoya takahashi
 *	@date		2009.12.24
 *
 *	モジュール名：FIELD_FK_SOUND_ANIME
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------


//-------------------------------------
///	管理SE最大数
//=====================================
enum
{
  FIELD_FK_SOUND_ANIME_SE_IDX_TRANS_X,    // TRANS Xに反応するSE
  FIELD_FK_SOUND_ANIME_SE_IDX_TRANS_Y,    // TRANS Yに反応するSE
  FIELD_FK_SOUND_ANIME_SE_IDX_TRANS_Z,    // TRANS Zに反応するSE
  FIELD_FK_SOUND_ANIME_SE_IDX_SCALE_X,    // SCALE Xに反応するSE
  FIELD_FK_SOUND_ANIME_SE_IDX_SCALE_Y,    // SCALE Yに反応するSE
  FIELD_FK_SOUND_ANIME_SE_IDX_SCALE_Z,    // SCALE Zに反応するSE
  FIELD_FK_SOUND_ANIME_SE_IDX_ROTATE_X,   // ROTATE Xに反応するSE
  FIELD_FK_SOUND_ANIME_SE_IDX_ROTATE_Y,   // ROTATE Yに反応するSE
  FIELD_FK_SOUND_ANIME_SE_IDX_ROTATE_Z,   // ROTATE Zに反応するSE

  FIELD_FK_SOUND_ANIME_SE_MAX,
};


//-------------------------------------
///	SE　NONE
//=====================================
#define FIELD_FK_SOUND_ANIME_SE_NONE  (0xffff)  // テーブルでSEが無いときに設定する値


// アニメーションスピード
#define FIELD_FK_SOUND_ANIME_SPEED (FX32_ONE)


//-------------------------------------
///	SEアニメーション　ストリーミング読み込みデフォルト値
//=====================================
#define FIELD_FK_SOUND_ANIME_SE_BUF_INTERVAL_SIZE ( 10 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	フィールド四天王　サウンドアニメーション
//=====================================
typedef struct _FIELD_FK_SOUND_ANIME FIELD_FK_SOUND_ANIME;


//-------------------------------------
///	セットアップデータ
//=====================================
typedef struct 
{
  u16 se_tbl[ FIELD_FK_SOUND_ANIME_SE_MAX ];   
} FIELD_FK_SOUND_ANIME_DATA;



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

extern FIELD_FK_SOUND_ANIME* FIELD_FK_SOUND_ANIME_Create( const FIELD_FK_SOUND_ANIME_DATA* cp_param, u32 arcID, u32 dataID, int buf_interval, HEAPID heapID );
extern void FIELD_FK_SOUND_ANIME_Delete( FIELD_FK_SOUND_ANIME* p_wk );
extern BOOL FIELD_FK_SOUND_ANIME_Update( FIELD_FK_SOUND_ANIME* p_wk );

extern BOOL FIELD_FK_SOUND_ANIME_IsAnime( const FIELD_FK_SOUND_ANIME* cp_wk );


#ifdef _cplusplus
}	// extern "C"{
#endif



