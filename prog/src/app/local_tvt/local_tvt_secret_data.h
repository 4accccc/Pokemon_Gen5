//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_secret_data.h
 *	@brief  隠しキャラのデータ
 *	@author	Toru=Nagihashi
 *	@date		2011.09.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "savedata/person_state.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define ZERO_ORIGIN(x)  (x-1)
//-------------------------------------
///	段階
//=====================================
typedef enum
{
  LOCAL_TVT_SECRET_PHASE_1_START  = ZERO_ORIGIN(1),    //個人情報をきく
  LOCAL_TVT_SECRET_PHASE_1_END    = ZERO_ORIGIN(9),
  LOCAL_TVT_SECRET_PHASE_1_LAST   = ZERO_ORIGIN(10),    //受け渡し
  LOCAL_TVT_SECRET_PHASE_2_START  = ZERO_ORIGIN(11),   //あちらの情報をおしえてくれる
  LOCAL_TVT_SECRET_PHASE_2_END    = ZERO_ORIGIN(20), 
  LOCAL_TVT_SECRET_PHASE_3_START  = ZERO_ORIGIN(21),   //仕事の悩み相談  
  LOCAL_TVT_SECRET_PHASE_3_END    = ZERO_ORIGIN(29), 
  LOCAL_TVT_SECRET_PHASE_3_LAST   = ZERO_ORIGIN(30),   //初回デートに誘う
  LOCAL_TVT_SECRET_PHASE_4_START_1  = ZERO_ORIGIN(31),   //日常会話（ランダム）
  LOCAL_TVT_SECRET_PHASE_4_END_1    = ZERO_ORIGIN(39),   
  LOCAL_TVT_SECRET_PHASE_4_SOUND    = ZERO_ORIGIN(40),    //音声のみ仕事の合間
  LOCAL_TVT_SECRET_PHASE_4_START_2  = ZERO_ORIGIN(41),   //日常会話（ランダム）
  LOCAL_TVT_SECRET_PHASE_4_END_2    = ZERO_ORIGIN(49),   
  LOCAL_TVT_SECRET_PHASE_4_TRUE     = ZERO_ORIGIN(50),   //素顔
  LOCAL_TVT_SECRET_PHASE_4_APOLOGY  = ZERO_ORIGIN(51),   //素顔のフォロー
  LOCAL_TVT_SECRET_PHASE_4_START_3  = ZERO_ORIGIN(52),   //日常会話（ランダム）
  LOCAL_TVT_SECRET_PHASE_4_END_3    = LOCAL_TVT_SECRET_PHASE_4_START_3,   

  LOCAL_TVT_SECRET_PHASE_RANGE_MAX = LOCAL_TVT_SECRET_PHASE_4_END_3,
} LTVT_SECREAT_PHASE;

//-------------------------------------
///	会話プログラム処理
//=====================================
typedef enum
{
  LTVT_SECREAT_TALK_NONE,
  LTVT_SECREAT_TALK_QANSWER,
  LTVT_SECREAT_TALK_RING_OFF,       //すぐ切る
  LTVT_SECREAT_TALK_DAILY_RND,      //日常会話ランダム
  LTVT_SECREAT_TALK_FORCE_TALK,       //すぐに話すへ行く
} LTVT_SECREAT_TALK;

//-------------------------------------
///	顔
//=====================================
typedef enum
{
  LTVT_SECREAT_FACE_SOUND,
  LTVT_SECREAT_FACE_MASK,
  LTVT_SECREAT_FACE_TRUE,
  LTVT_SECREAT_FACE_MASK2,
} LTVT_SECREAT_FACE;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern u32 LTVT_SECREAT_DATA_SearchIndex( LTVT_SECREAT_PHASE phase );

extern LTVT_SECREAT_TALK LTVT_SECREAT_DATA_GetTalkProg( u32 index );
extern LTVT_SECREAT_FACE LTVT_SECREAT_DATA_GetFace( u32 index );
extern PERSON_STATE_SECRET_DATE LTVT_SECREAT_DATA_GetDate( u32 index );
extern BOOL LTVT_SECREAT_DATA_IsPlayerCall( u32 index );
extern u32 LTVT_SECREAT_DATA_GetCallSECnt( u32 index );
extern LTVT_SECREAT_PHASE LTVT_SECREAT_DATA_GetCallFirst( u32 index );
