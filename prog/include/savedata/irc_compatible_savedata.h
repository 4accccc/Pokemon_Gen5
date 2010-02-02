//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_compatible_savedata.h
 *	@brief	赤外線相性チェック用セーブデータ
 *	@author	Toru=Nagihashi
 *	@date		2009.07.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include <strbuf.h>
#include "savedata/save_control.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define IRC_COMPATIBLE_SV_NEW_NONE		(127)		//新規ランクがいない
#define IRC_COMPATIBLE_SV_ADD_NONE		(0xFF)	//ランキングに入らなかった
#define IRC_COMPATIBLE_SV_RANKING_MAX	(30)		//ランキング最大数

#define IRC_COMPATIBLE_SV_DATA_SCORE_MAX		(100)		//スコア最大
#define IRC_COMPATIBLE_SV_DATA_PLAYCNT_MAX	(999)		//遊んだ回数最大

#define IRC_COMPATIBLE_SV_DATA_NAME_LEN			(PERSON_NAME_SIZE + EOM_SIZE)	//名前の長さ

#define IRC_COMPATIBLE_SEX_MALE     (1)
#define IRC_COMPATIBLE_SEX_FEMALE   (0)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	相性チェック用セーブデータ
//=====================================
typedef struct _IRC_COMPATIBLE_SAVEDATA IRC_COMPATIBLE_SAVEDATA;
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	セーブデータ取得取得
//=====================================
extern IRC_COMPATIBLE_SAVEDATA * IRC_COMPATIBLE_SV_GetSavedata( SAVE_CONTROL_WORK* p_sv_ctrl );

//-------------------------------------
///	データ取得関数
//=====================================
extern u32 IRC_COMPATIBLE_SV_GetRankNum( const IRC_COMPATIBLE_SAVEDATA *cp_sv );
extern const STRCODE* IRC_COMPATIBLE_SV_GetPlayerName( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank );
extern u8	IRC_COMPATIBLE_SV_GetScore( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank );
extern u16 IRC_COMPATIBLE_SV_GetPlayCount( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank );
extern u32 IRC_COMPATIBLE_SV_GetSex( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank );
extern u32 IRC_COMPATIBLE_SV_GetBirthMonth( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank );
extern u32 IRC_COMPATIBLE_SV_GetBirthDay( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank );
extern u8	IRC_COMPATIBLE_SV_GetNewRank( const IRC_COMPATIBLE_SAVEDATA *cp_sv );
extern u8 IRC_COMPATIBLE_SV_GetBioRhythm( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u32 rank );
extern u32 IRC_COMPATIBLE_SV_GetBestBioRhythm( const IRC_COMPATIBLE_SAVEDATA *cp_sv, u8 player_birth_month, u8 player_birth_day );

inline BOOL IRC_COMPATIBLE_SV_IsEmpty( const IRC_COMPATIBLE_SAVEDATA *cp_sv )
{ 
  return IRC_COMPATIBLE_SV_GetRankNum( cp_sv ) == 0;
}

//-------------------------------------
///	irc_compatible.cでのみ使うprivate関数
//=====================================
extern u8 Irc_Compatible_SV_CalcBioRhythm( u8 month, u8 day );

//-------------------------------------
///	データ設定関数
//=====================================
extern u8 IRC_COMPATIBLE_SV_AddRanking( IRC_COMPATIBLE_SAVEDATA *p_sv, const STRCODE *cp_name, u8 score, u8 sex, u8 birth_month, u8 birth_day, u32 trainerID );

//-------------------------------------
///	セーブデータシステムで使用する関数
//=====================================
extern int IRC_COMPATIBLE_SV_GetWorkSize( void );
extern void IRC_COMPATIBLE_SV_Init( IRC_COMPATIBLE_SAVEDATA *p_sv );

