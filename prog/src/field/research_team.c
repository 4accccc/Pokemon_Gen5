/////////////////////////////////////////////////////////////////////
/**
 * @brief  すれ違い調査隊
 * @file   research_team.h
 * @author obata
 * @date   2010.03.31
 */
/////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "field/research_team.h" 
#include "field/research_team_def.h"
#include "../app/research_radar/question_id.h"


typedef struct {

  u8 type;      // 調査タイプ
  u8 QNum;      // 調査対象の質問数
  u8 QID1;      // 質問ID1
  u8 QID2;      // 質問ID2
  u8 QID3;      // 質問ID3
  u8 normCount; // ノルマ人数
  u8 normTime;  // ノルマ時間

} RESEARCH_REQUEST_DATA;


static const RESEARCH_REQUEST_DATA RequestData[ RESEARCH_REQ_ID_NUM ] = 
{
  { RESEARCH_REQ_TYPE_TIME,  0, QUESTION_ID_DUMMY,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      0,  0 }, // ダミー
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_WB,               QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  2 }, //「ブラックと　ホワイト　おおいのは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_FIRST_POKEMON,    QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  2 }, //「えらんだ　さいしょのポケモンは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_PLAY_TIME,        QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  4 }, //「いちばんおおい　プレイじかんは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_SEX,              QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  4 }, //「おとこのこと　おんなのこ　おおいのは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_JOB,              QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  4 }, //「いちばんおおい　しごとは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_HOBBY,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  4 }, //「いちばんおおい　しゅみは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_FUN,              QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  8 }, //「たいせんと　こうかん　にんきがあるのは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_FAVORITE_POKEMON, QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  8 }, //「すきな　ポケモンは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_POKEMON_TYPE,     QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  8 }, //「すきな　ポケモンのタイプは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_PLACE,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1,  8 }, //「とかいと　いなか　すみたいのは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_WANT,             QUESTION_ID_PRECIOUS,   QUESTION_ID_DUMMY,      1, 12 }, //「だいじにしているものに　ついて」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  3, QUESTION_ID_PARTNER,          QUESTION_REQUIREMENT,   QUESTION_ID_CHARACTER,  1, 12 }, //「ひとのみりょくに　ついて」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  2, QUESTION_ID_SEASON,           QUESTION_ID_FOOD,       QUESTION_ID_DUMMY,      1, 12 }, //「きせつに　ついて」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  2, QUESTION_ID_COLOR,            QUESTION_ID_MUSIC,      QUESTION_ID_DUMMY,      1, 12 }, //「げいじゅつに　ついて」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_MUSIC,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1, 16 }, //「いちばんすきな　おんがくは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  2, QUESTION_ID_TV,               QUESTION_ID_MOVIE,      QUESTION_ID_DUMMY,      1, 16 }, //「えいぞうに　ついて」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_FAVORITE_CLASS,   QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1, 16 }, //「いちばんすきな　がっこうのじかんは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  2, QUESTION_ID_STRONG_CLASS,     QUESTION_ID_WEAK_CLASS, QUESTION_ID_DUMMY,      1, 16 }, //「べんきょうに　ついて」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_SPORTS,           QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1, 20 }, //「いちばんにんきのある　スポーツは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  2, QUESTION_ID_DAY_OFF,          QUESTION_ID_PLAY_SPOT,  QUESTION_ID_DUMMY,      1, 20 }, //「きゅうじつに　ついて」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_DREAM,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1, 20 }, //「いちばんなりたいものは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_GYM_LEADER,       QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1, 24 }, //「いちばんにんきのある　ジムリーダーは？」( 時間 )
  { RESEARCH_REQ_TYPE_TIME,  1, QUESTION_ID_POKEMON,          QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      1, 24 }, //「ポケモンのおもしろいところは？」( 時間 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_WB,               QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      5,  0 }, //「ブラックと　ホワイト　おおいのは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_FIRST_POKEMON,    QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,      5,  0 }, //「えらんだ　さいしょのポケモンは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_PLAY_TIME,        QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     10,  0 }, //「いちばんおおい　プレイじかんは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_SEX,              QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     10,  0 }, //「おとこのこと　おんなのこ　おおいのは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_JOB,              QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     10,  0 }, //「いちばんおおい　しごとは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_HOBBY,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     10,  0 }, //「いちばんおおい　しゅみは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_FUN,              QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     20,  0 }, //「たいせんと　こうかん　にんきがあるのは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_FAVORITE_POKEMON, QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     20,  0 }, //「すきな　ポケモンは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_POKEMON_TYPE,     QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     20,  0 }, //「すきな　ポケモンのタイプは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_PLACE,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     20,  0 }, //「とかいと　いなか　すみたいのは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 2, QUESTION_ID_WANT,             QUESTION_ID_PRECIOUS,   QUESTION_ID_DUMMY,     30,  0 }, //「だいじにしているものに　ついて」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 3, QUESTION_ID_PARTNER,          QUESTION_REQUIREMENT,   QUESTION_ID_CHARACTER, 30,  0 }, //「ひとのみりょくに　ついて」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 2, QUESTION_ID_SEASON,           QUESTION_ID_FOOD,       QUESTION_ID_DUMMY,     30,  0 }, //「きせつに　ついて」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 2, QUESTION_ID_COLOR,            QUESTION_ID_MUSIC,      QUESTION_ID_DUMMY,     30,  0 }, //「げいじゅつに　ついて」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_MUSIC,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     40,  0 }, //「いちばんすきな　おんがくは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 2, QUESTION_ID_TV,               QUESTION_ID_MOVIE,      QUESTION_ID_DUMMY,     40,  0 }, //「えいぞうに　ついて」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_FAVORITE_CLASS,   QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     40,  0 }, //「いちばんすきな　がっこうのじかんは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 2, QUESTION_ID_STRONG_CLASS,     QUESTION_ID_WEAK_CLASS, QUESTION_ID_DUMMY,     40,  0 }, //「べんきょうに　ついて」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_SPORTS,           QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     50,  0 }, //「いちばんにんきのある　スポーツは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 2, QUESTION_ID_DAY_OFF,          QUESTION_ID_PLAY_SPOT,  QUESTION_ID_DUMMY,     50,  0 }, //「きゅうじつに　ついて」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_DREAM,            QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,     50,  0 }, //「いちばんなりたいものは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_GYM_LEADER,       QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,    100,  0 }, //「いちばんにんきのある　ジムリーダーは？」( 人数 )
  { RESEARCH_REQ_TYPE_COUNT, 1, QUESTION_ID_POKEMON,          QUESTION_ID_DUMMY,      QUESTION_ID_DUMMY,    100,  0 }, //「ポケモンのおもしろいところは？」( 人数 )
};


//--------------------------------------------------------------------
/**
 * @brief 調査タイプを取得する
 *
 * @param requestID 調査依頼ID ( RESEARCH_REQ_ID_xxxx )
 *
 * @return 指定した依頼IDの調査タイプ ( RESEARCH_REQ_TYPE_xxxx )
 */
//--------------------------------------------------------------------
int RESEARCH_TEAM_GetResearchType( u8 requestID )
{
  GF_ASSERT( RESEARCH_REQ_ID_NONE < requestID );
  GF_ASSERT( requestID < RESEARCH_REQ_ID_NUM );

  return RequestData[ requestID ].type;
}

//--------------------------------------------------------------------
/**
 * @brief 質問数を取得する
 *
 * @param requestID [in] 調査依頼ID ( RESEARCH_REQ_ID_xxxx )
 *
 * @return 指定した依頼に該当する質問の数
 */
//--------------------------------------------------------------------
int RESEARCH_TEAM_GetQuestionNum( u8 requestID )
{
  GF_ASSERT( RESEARCH_REQ_ID_NONE < requestID );
  GF_ASSERT( requestID < RESEARCH_REQ_ID_NUM );

  return RequestData[ requestID ].QNum;
}

//--------------------------------------------------------------------
/**
 * @brief 質問IDを取得する
 *
 * @param requestID [in] 調査依頼ID ( RESEARCH_REQ_ID_xxxx )
 * @param destQID   [out]指定した調査依頼に該当する質問ID
 */
//--------------------------------------------------------------------
void RESEARCH_TEAM_GetQuestionID( u8 requestID, u8* destQID )
{
  GF_ASSERT( RESEARCH_REQ_ID_NONE < requestID );
  GF_ASSERT( requestID < RESEARCH_REQ_ID_NUM );

  destQID[0] = RequestData[ requestID ].QID1;
  destQID[1] = RequestData[ requestID ].QID2;
  destQID[2] = RequestData[ requestID ].QID3;
}

//--------------------------------------------------------------------
/**
 * @brief ノルマ人数を取得する
 */
//--------------------------------------------------------------------
int RESEARCH_TEAM_GetNormCount( u8 requestID )
{
  GF_ASSERT( RESEARCH_REQ_ID_NONE < requestID );
  GF_ASSERT( requestID < RESEARCH_REQ_ID_NUM );

  return RequestData[ requestID ].normCount;
}

//--------------------------------------------------------------------
/**
 * @brief ノルマ時間を取得する
 */
//--------------------------------------------------------------------
int RESEARCH_TEAM_GetNormTime( u8 requestID )
{
  GF_ASSERT( RESEARCH_REQ_ID_NONE < requestID );
  GF_ASSERT( requestID < RESEARCH_REQ_ID_NUM );

  return RequestData[ requestID ].normTime;
} 
