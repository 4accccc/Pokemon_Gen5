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
#include "savedata/misc.h" 
#include "savedata/questionnaire_save.h" 
#include "savedata/questionnaire_index.h" 
#include "gamesystem/game_beacon.h"


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

static BOOL CheckNormCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 RID );
static BOOL CheckMaxCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 RID );
static BOOL CheckNormTime( SAVE_CONTROL_WORK* save );
static BOOL CheckResearchOne( SAVE_CONTROL_WORK* save );

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


//=============================================================================
/**
 * @brief 調査中の依頼の達成をチェックする(受けた依頼の達成チェックに使用)
 *
 * @param save
 * @return 調査状況 RESEARCH_REQ_STATE_ACHIEVE他
 */
//=============================================================================
u16 RESEARCH_TEAM_CheckCurrentResearchFinish( SAVE_CONTROL_WORK* save )
{
  QUESTIONNAIRE_SAVE_WORK* QSave = SaveData_GetQuestionnaire( save );
  const MISC*              misc  = SaveData_GetMiscConst( save );

  u8 RID;
  u8 RType;

  RID = MISC_GetResearchRequestID( misc ); // 調査依頼IDを取得
  RType = RESEARCH_TEAM_GetResearchType( RID ); // 依頼タイプを取得

  // 人数調査の場合
  if( RType == RESEARCH_REQ_TYPE_COUNT ) {
    // ノルマ達成
    if( CheckNormCount( QSave, RID ) ) {
      return RESEARCH_REQ_STATE_ACHIEVE;
    }
    // ノルマ未達成
    else {
      return RESEARCH_REQ_STATE_FALSE_NORM_COUNT;
    }
  }
  // 時間調査の場合
  else if( RType == RESEARCH_REQ_TYPE_TIME ) {
    // ノルマ時間が経過
    if( CheckNormTime( save ) ) {
      // 1人以上調査した
      if( CheckResearchOne( save ) ) {
        return RESEARCH_REQ_STATE_ACHIEVE;
      }
      // 1人も調査していない
      else {
        return RESEARCH_REQ_STATE_FALSE_ZERO_COUNT;
      }
    }
    // ノルマ時間が経過していない
    else {
      return RESEARCH_REQ_STATE_FALSE_NORM_TIME;
    }
  }
  else {
    GF_ASSERT(0);
  }
  return RESEARCH_REQ_STATE_FALSE_NORM_COUNT;
}

//=============================================================================
/**
 * @brief 指定した依頼をすでに達成しているかどうかをチェックする(依頼を受けようとした際の事前チェックに使用)
 *
 * @param save
 * @param RID    チェックする依頼のID
 * @return 調査状況 RESEARCH_REQ_STATE_ACHIEVE他
 */
//=============================================================================
u16 RESEARCH_TEAM_CheckRequestFinishInAdvance( SAVE_CONTROL_WORK* save, u8 RID )
{
  QUESTIONNAIRE_SAVE_WORK* QSave = SaveData_GetQuestionnaire( save );
  const MISC*              misc  = SaveData_GetMiscConst( save );

  u8 RType;

  RType = RESEARCH_TEAM_GetResearchType( RID ); // 依頼タイプを取得

  // 人数調査の場合
  if( RType == RESEARCH_REQ_TYPE_COUNT ) {
    // ノルマ達成
    if( CheckNormCount( QSave, RID ) ) {
      return RESEARCH_REQ_STATE_ACHIEVE;
    }
    // ノルマ未達成
    else {
      return RESEARCH_REQ_STATE_FALSE_NORM_COUNT;
    }
  }
  // 時間調査の場合
  else if( RType == RESEARCH_REQ_TYPE_TIME ) {
    // MAXカウントまで調査している
    if( CheckMaxCount( QSave, RID ) ) {
      return RESEARCH_REQ_STATE_ACHIEVE;
    }
    // ノルマ時間が経過していない
    else {
      return RESEARCH_REQ_STATE_FALSE_NORM_TIME;
    }
  }
  else {
    GF_ASSERT(0);
  }
  return RESEARCH_REQ_STATE_FALSE_NORM_COUNT;
}

//=============================================================================
/**
 * @brief 調査中の依頼の経過時間を取得する
 */
//=============================================================================
u16 RESEARCH_TEAM_GetPassedTime( SAVE_CONTROL_WORK* save )
{
  MISC* misc = SaveData_GetMisc( save );

  s64 startSec;
  u16 passedHour;

  // 調査中でない
  if( MISC_GetResearchRequestID( misc ) == RESEARCH_REQ_ID_NONE ) {
    GF_ASSERT(0);
    return 0;
  }
  
  // 調査の開始時刻を取得
  startSec = MISC_GetResearchStartTimeBySecond( misc );

  { 
    s64 nowSec, passedSec;
    RTCDate passedDate;
    RTCTime passedTime;

    // 現在時刻を取得
    nowSec = GFL_RTC_GetDateTimeBySecond();

    // 経過時間[h]を算出
    passedSec = nowSec - startSec;
    RTC_ConvertSecondToDateTime( &passedDate, &passedTime, passedSec );

    if( (0 < passedDate.year) ||  // 1年以上が経過
        (1 < passedDate.month) || // 1月以上が経過
        (1 < passedDate.day) )    // 1日以上が経過
    { 
      passedHour = 24; // 最大24時間とする ( それ以上は必要ないので )
    }
    else
    {
      passedHour = passedTime.hour;
    }
  }

  return passedHour;
}
//=============================================================================
/**
 * @brief 現在の調査人数を取得する
 *
 * @param QID 質問ID ( QUESTION_ID_xxxx )
 *
 * @return 指定した質問の, 現時点での調査人数 ( いままで + 今日 )
 */
//=============================================================================
u32 RESEARCH_TEAM_GetCurrentCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 QID )
{
  u32 today = QuestionnaireWork_GetTodayCount( QSave, QID );
  u32 total = QuestionnaireWork_GetTotalCount( QSave, QID );
  u32 current = today + total;
  if( QUESTIONNAIRE_TOTAL_COUNT_MAX < current ) { 
    current = QUESTIONNAIRE_TOTAL_COUNT_MAX; 
  }
  return current;
}

//=============================================================================
/**
 * @brief ノルマ人数の達成をチェックする
 *
 * @param QSave
 * @param RID   チェックする調査依頼のID ( RESEARCH_REQ_ID_xxxx )
 *
 * @return 指定した依頼を達成している場合 TRUE
 *         そうでなければ FALSE
 */
//=============================================================================
static BOOL CheckNormCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 RID )
{
  int i;
  int norm;
  u8 QID[ MAX_QNUM_PER_RESEARCH_REQ ];
  u8 QNum;

  RESEARCH_TEAM_GetQuestionID( RID, QID );
  QNum = RESEARCH_TEAM_GetQuestionNum( RID );
  norm = RESEARCH_TEAM_GetNormCount( RID );

  for( i=0; i<QNum; i++ )
  {
    u32 count = RESEARCH_TEAM_GetCurrentCount( QSave, QID[i] );

#ifdef DEBUG_PRINT_ON
    OS_TFPrintf( 3, 
        "CheckNormCount: QID=%d, norm=%d, count=%d\n", QID[i], norm, count );
#endif

    // ノルマを達成していない
    if( count < norm ) {
      return FALSE;
    }
  } 
  return TRUE;
}


//=============================================================================
/**
 * @brief 調査中の依頼について, ノルマ時間の達成をチェックする
 *
 * @param save
 *
 * @return 調査中の依頼がノルマ時間を達成している場合 TRUE
 *         そうでなければ FALSE
 */
//=============================================================================
static BOOL CheckNormTime( SAVE_CONTROL_WORK* save )
{
  QUESTIONNAIRE_SAVE_WORK* QSave = SaveData_GetQuestionnaire( save );
  const MISC*              misc  = SaveData_GetMiscConst( save );

  int RID;
  int pass;
  int norm;

  RID  = MISC_GetResearchRequestID( misc );
  pass = RESEARCH_TEAM_GetPassedTime( save ); // 経過時間を取得
  norm = RESEARCH_TEAM_GetNormTime( RID ); // ノルマ時間を取得

  GF_ASSERT( RESEARCH_TEAM_GetResearchType(RID) == RESEARCH_REQ_TYPE_TIME );

  // 経過している
  if( norm <= pass ) {
    return TRUE;
  }
  // 経過していない
  else {
    return FALSE;
  }
}

//=============================================================================
/**
 * @brief 調査中の依頼について, 調査の開始から１人でも調査したかどうかをチェックする
 *
 * @param save
 *
 * @return 調査中の依頼を開始してから, １人以上の調査を行っている場合 TRUE
 *         そうでなければ FALSE
 */
//=============================================================================
static BOOL CheckResearchOne( SAVE_CONTROL_WORK* save )
{
  int i;
  u8 RID;
  u8 QNum;
  const MISC* misc;

  // 調査中の質問数を取得
  misc = SaveData_GetMiscConst( save );
  RID  = MISC_GetResearchRequestID( misc );
  QNum = RESEARCH_TEAM_GetQuestionNum( RID );

  // 調査開始からの, 各質問の調査人数をチェック
  for( i=0; i<QNum; i++ )
  {
    // １人も調査していない質問を発見
    if( MISC_GetResearchCount( misc, i ) == 0 ) {
      return FALSE;
    }
  }
  return TRUE;
}

//=============================================================================
/**
 * @brief MAXカウントの達成をチェックする
 *
 * @param QSave
 * @param RID  チェックする依頼のID
 *
 * @return 指定した依頼に該当する全質問をMAXカウントまで調査している場合 TRUE
 *         そうでなければ FALSE
 */
//=============================================================================
static BOOL CheckMaxCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 RID )
{
  int i;
  u8 QID[ MAX_QNUM_PER_RESEARCH_REQ ];
  u8 QNum;
  int norm;

  RESEARCH_TEAM_GetQuestionID( RID, QID );
  QNum = RESEARCH_TEAM_GetQuestionNum( RID );
  norm = RESEARCH_TEAM_GetNormCount( RID );

  for( i=0; i<QNum; i++ )
  {
    u32 count = RESEARCH_TEAM_GetCurrentCount( QSave, QID[i] );

    // MAXカウントに達していない
    if( count < QUESTIONNAIRE_TOTAL_COUNT_MAX ) {
      return FALSE;
    }
  } 
  return TRUE;
}

