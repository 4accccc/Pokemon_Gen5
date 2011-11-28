//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_secret_data.c
 *	@brief  隠しキャラのデータ
 *	@author	Toru=Nagihashi
 *	@data		2011.09.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include "local_tvt_secret_data.h"

#define CALL_SE_CNT_DEFAULT  (2)
#define CALL_SE_CNT_SP  (4)
//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	隠しキャラ状態表
//=====================================
static const struct
{
  LTVT_SECREAT_PHASE  min;
  LTVT_SECREAT_PHASE  max;
  LTVT_SECREAT_TALK   talk;
  LTVT_SECREAT_FACE   face;
  PERSON_STATE_SECRET_DATE   date; 
  BOOL  is_player_call; //プレイヤーがかけられる
  u32  call_se_cnt;     //呼び出し回数
} sc_secreat_data[] =
{
  { LOCAL_TVT_SECRET_PHASE_1_START, //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_1_END,   //フェーズ範囲終了
    LTVT_SECREAT_TALK_QANSWER,   //会話プログラム処理
    LTVT_SECREAT_FACE_SOUND,        //顔
    PERSON_STATE_SECRET_DATE_NONE,         //デートにさそうか
    FALSE,                          //プレイヤーがかけられるか
    CALL_SE_CNT_DEFAULT,            //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_1_LAST,  //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_1_LAST,  //フェーズ範囲終了
    LTVT_SECREAT_TALK_FORCE_TALK,   //会話プログラム処理
    LTVT_SECREAT_FACE_SOUND,        //顔
    PERSON_STATE_SECRET_DATE_GIVE,         //デートにさそうか
    FALSE,                          //プレイヤーがかけられるか
    CALL_SE_CNT_DEFAULT,            //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_2_START, //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_3_END,   //フェーズ範囲終了
    LTVT_SECREAT_TALK_NONE,         //会話プログラム処理
    LTVT_SECREAT_FACE_MASK,         //顔
    PERSON_STATE_SECRET_DATE_NONE,         //デートにさそうか
    TRUE,                           //プレイヤーがかけられるか
    CALL_SE_CNT_DEFAULT,            //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_3_LAST,  //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_3_LAST,  //フェーズ範囲終了
    LTVT_SECREAT_TALK_NONE,         //会話プログラム処理
    LTVT_SECREAT_FACE_MASK2,         //顔
    PERSON_STATE_SECRET_DATE_FIRST,        //デートにさそうか
    TRUE,                           //プレイヤーがかけられるか
    CALL_SE_CNT_DEFAULT,            //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_4_START_1,  //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_4_END_1,  //フェーズ範囲終了
    LTVT_SECREAT_TALK_DAILY_RND,    //会話プログラム処理
    LTVT_SECREAT_FACE_MASK2,         //顔
    PERSON_STATE_SECRET_DATE_REGULAR,       //デートにさそうか
    TRUE,                           //プレイヤーがかけられるか
    CALL_SE_CNT_DEFAULT,            //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_4_SOUND,  //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_4_SOUND,  //フェーズ範囲終了
    LTVT_SECREAT_TALK_NONE,         //会話プログラム処理
    LTVT_SECREAT_FACE_SOUND,         //顔
    PERSON_STATE_SECRET_DATE_NONE,      //デートにさそうか
    TRUE,                           //プレイヤーがかけられるか
    CALL_SE_CNT_SP,               //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_4_START_2,  //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_4_END_2,  //フェーズ範囲終了
    LTVT_SECREAT_TALK_DAILY_RND,         //会話プログラム処理
    LTVT_SECREAT_FACE_MASK2,         //顔
    PERSON_STATE_SECRET_DATE_REGULAR,       //デートにさそうか
    TRUE,                           //プレイヤーがかけられるか
    CALL_SE_CNT_DEFAULT,            //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_4_TRUE,  //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_4_TRUE,  //フェーズ範囲終了
    LTVT_SECREAT_TALK_RING_OFF,         //会話プログラム処理
    LTVT_SECREAT_FACE_TRUE,         //顔
    PERSON_STATE_SECRET_DATE_NONE,       //デートにさそうか
    TRUE,                           //プレイヤーがかけられるか
    CALL_SE_CNT_SP,            //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_4_APOLOGY,  //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_4_APOLOGY,  //フェーズ範囲終了
    LTVT_SECREAT_TALK_NONE,         //会話プログラム処理
    LTVT_SECREAT_FACE_MASK2,         //顔
    PERSON_STATE_SECRET_DATE_NONE,       //デートにさそうか
    TRUE,                           //プレイヤーがかけられるか
    CALL_SE_CNT_DEFAULT,            //呼び出し回数
  },
  { LOCAL_TVT_SECRET_PHASE_4_START_3,  //フェース範囲開始
    LOCAL_TVT_SECRET_PHASE_4_END_3,  //フェーズ範囲終了
    LTVT_SECREAT_TALK_DAILY_RND,         //会話プログラム処理
    LTVT_SECREAT_FACE_MASK2,         //顔
    PERSON_STATE_SECRET_DATE_REGULAR,       //デートにさそうか
    TRUE,                           //プレイヤーがかけられるか
    CALL_SE_CNT_DEFAULT,            //呼び出し回数
  },
};

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  友好度からインデックスを取得
 *
 *	@param	LTVT_SECREAT_PHASE phase フェーズ
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
u32 LTVT_SECREAT_DATA_SearchIndex( LTVT_SECREAT_PHASE phase )
{
  int i;

  if( phase >= LOCAL_TVT_SECRET_PHASE_RANGE_MAX )
  {
    return NELEMS(sc_secreat_data) - 1;
  }

  for( i = 0; i < NELEMS(sc_secreat_data); i++ )
  {
    if( sc_secreat_data[i].min <= phase && phase <= sc_secreat_data[i].max )
    {
      return i;
    }
  }

  GF_ASSERT_MSG( 0,"%d\n",phase);
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  会話プログラム処理IDを取得
 *
 *	@param	u32 index インデックス
 *
 *	@return 会話プログラム処理ID
 */
//-----------------------------------------------------------------------------
LTVT_SECREAT_TALK LTVT_SECREAT_DATA_GetTalkProg( u32 index )
{
  return sc_secreat_data[index].talk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  顔状態を取得
 *
 *	@param	u32 index インデックス
 *
 *	@return 顔状態
 */
//-----------------------------------------------------------------------------
LTVT_SECREAT_FACE LTVT_SECREAT_DATA_GetFace( u32 index )
{
  return sc_secreat_data[index].face;
}
//----------------------------------------------------------------------------
/**
 *	@brief  デート誘い状態を取得
 *
 *	@param	u32 index   インデックス
 *
 *	@return デート誘い状態
 */
//-----------------------------------------------------------------------------
PERSON_STATE_SECRET_DATE LTVT_SECREAT_DATA_GetDate( u32 index )
{
  return sc_secreat_data[index].date;
}
//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤーからかけることが可能かどうか取得
 *
 *	@param	u32 index   インデックス
 *
 *	@return プレイヤーからかけることが可能かどうか  TRUE可能
 */
//-----------------------------------------------------------------------------
BOOL LTVT_SECREAT_DATA_IsPlayerCall( u32 index )
{
  return sc_secreat_data[index].is_player_call;
}
//----------------------------------------------------------------------------
/**
 *	@brief  相手からかかってくるかどうか取得
 *
 *	@param	u32 index インデックス 
 *
 *	@return 相手からかかってくるかどうか  TRUEかかってくる
 */
//-----------------------------------------------------------------------------
u32 LTVT_SECREAT_DATA_GetCallSECnt( u32 index )
{
  return sc_secreat_data[index].call_se_cnt;
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分からかけられるようになった初回かチェック
 *
 *	@param	u32 index   インデックス
 *
 *	@return 初回フェーズ
 */
//-----------------------------------------------------------------------------
LTVT_SECREAT_PHASE LTVT_SECREAT_DATA_GetCallFirst( u32 index )
{
  return LOCAL_TVT_SECRET_PHASE_2_START;
}
