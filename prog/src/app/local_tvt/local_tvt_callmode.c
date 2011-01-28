//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_callmode.c
 *	@brief  ローカルライブキャスター呼び出しモード
 *	@author	Toru=Nagihashi
 *	@data		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/time_icon.h"
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_tvt_event_system.h"

//自分のモジュール
#include "app/local_tvt_sys.h"
#include "local_tvt_snd.h"
#include "local_tvt_chara.h"
#include "local_tvt_graphic.h"
#include "local_tvt_util.h"
#include "local_tvt_callselect.h"
#include "local_tvt_calldata.h"
#include "local_tvt_person.h"
#include "local_tvt_person_support.h"
#include "local_tvt_person_mother.h"
#include "local_tvt_person_doctor.h"
#include "local_tvt_person_chelen.h"
#include "local_tvt_person_rival.h"
#include "local_tvt_person_secret.h"

//外部参照
#include "local_tvt_callmode.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	呼び出し中メッセージ
//=====================================
typedef struct 
{
  GFL_BMPWIN        *p_msgwin;
  TIMEICON_WORK     *p_timeicon;
  LOCAL_TVT_MESSAGE *p_message;
  u16               call_cnt;
  u16               is_end;
} CALLMSG;


//-------------------------------------
///	メインワーク
//=====================================
struct _LOCAL_TVT_CALL
{
  HEAPID                heapID;
  LOCAL_TVT_CALL_PARAM  param;
  LOCAL_TVT_CALLSELECT  *p_callselect;
  LOCAL_TVT_SEQ         *p_seq;
  LOCAL_TVT_PERSON_SYS  *p_person;
  LOCAL_TVT_CALLDATA    *p_data;
  CALLMSG               callmsg;
  u16                   call_list[LOCAL_TVT_CALLSELECT_SELECT_MAX];
  s32                   call_no;
};
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void Local_Tvt_Call_SetMode( LOCAL_TVT_CALL *p_wk, LOCAL_TVT_CALL_MODE mode );
static void Local_Tvt_SetCallSelect( LOCAL_TVT_CALL *p_wk, LOCAL_TVT_CALLSELECT_PARAM *p_param );
static void Local_Tvt_SetPerson( LOCAL_TVT_CALL *p_wk, LOCAL_TVT_PERSON *p_person );

//シーケンス関数
static void LOCAL_TVT_SEQ_FadeInSendMode( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );
static void LOCAL_TVT_SEQ_FadeInReceiveMode( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );
static void LOCAL_TVT_SEQ_CallSelect( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );
static void LOCAL_TVT_SEQ_Calling( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );
static void LOCAL_TVT_SEQ_Talk( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );
static void LOCAL_TVT_SEQ_FadeOut( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs );

//呼び出し中画面
static void CALLMSG_Init( CALLMSG *p_wk, LOCAL_TVT_MESSAGE *p_message, HEAPID heapID );
static void CALLMSG_Exit( CALLMSG *p_wk );
static void CALLMSG_Main( CALLMSG *p_wk );
static BOOL CALLMSG_IsEnd( const CALLMSG *cp_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しモード作成
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	*cp_param   引数
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_CALL * LOCAL_TVT_CALL_Init( const LOCAL_TVT_CALL_PARAM *cp_param, HEAPID heapID )
{
  LOCAL_TVT_CALL* p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_CALL) );
  p_wk->param   = *cp_param;
  p_wk->heapID  = heapID;

  //呼び出しデータ作成
  p_wk->p_data  = LOCAL_TVT_CALLDATA_Init( p_wk->param.p_gamedata, heapID );

  //シーケンス管理
  p_wk->p_seq = LOCAL_TVT_SEQ_Init( p_wk, NULL, heapID );

  //モード別設定
  Local_Tvt_Call_SetMode( p_wk, p_wk->param.mode );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しモード破棄
 *
 *	@param	LOCAL_TVT_CALL *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CALL_Exit( LOCAL_TVT_CALL *p_wk )
{
  //シーケンス破棄
  LOCAL_TVT_SEQ_Exit( p_wk->p_seq );

  //データ破棄
  LOCAL_TVT_CALLDATA_Exit( p_wk->p_data );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しモードメイン処理
 *
 *	@param	LOCAL_TVT_CALL *p_wk  ワーク
 *
 *	@return TRUE終了  FALSE継続
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_CALL_Main( LOCAL_TVT_CALL *p_wk )
{
  GF_ASSERT( GFL_MSG_GetStrCount(LOCAL_TVT_MESSAGE_GetCallMsgData(p_wk->param.p_message)) != 0 );

  //シーケンス処理
  LOCAL_TVT_SEQ_Main( p_wk->p_seq );

  if( LOCAL_TVT_SEQ_IsEnd( p_wk->p_seq ) )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  モード設定
 *
 *	@param	LOCAL_TVT_CALL *p_wk  ワーク
 *	@param	mode                  モード
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Call_SetMode( LOCAL_TVT_CALL *p_wk, LOCAL_TVT_CALL_MODE mode )
{
  switch( mode )
  {
  default:
    GF_ASSERT(0);
  case LOCAL_TVT_CALL_MODE_SEND:
    LOCAL_TVT_SEQ_SetNext( p_wk->p_seq, LOCAL_TVT_SEQ_FadeInSendMode );
    break;

  case LOCAL_TVT_CALL_MODE_RECEIVE:
    //誰から掛かってきたのか設定
    p_wk->call_list[0]  = LOCAL_TVT_PERSON_SECRET;
    p_wk->call_no = 0;
    LOCAL_TVT_SEQ_SetNext( p_wk->p_seq, LOCAL_TVT_SEQ_FadeInReceiveMode );
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し設定
 *
 *	@param	LOCAL_TVT_CALL *p_wk  メインワーク
 *	@param	*p_param  設定構造体
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_SetCallSelect( LOCAL_TVT_CALL *p_wk, LOCAL_TVT_CALLSELECT_PARAM *p_param )
{
  GFL_STD_MemClear( p_param, sizeof(LOCAL_TVT_CALLSELECT_PARAM));
  p_param->p_message = p_wk->param.p_message;
  p_param->p_clunit  = p_wk->param.p_clunit;
  p_param->p_msgdata = LOCAL_TVT_MESSAGE_GetSystemMsgData(p_wk->param.p_message);

  //それぞれの呼び出し設定
  {
    static const struct
    {
      u16 person;
      u16 msgid;
    } sc_call_data[] =
    {
      {
        LOCAL_TVT_PERSON_DOCTOR,
        LOCAL_TVT_CHARA_04,
      },
      {
        LOCAL_TVT_PERSON_RIVAL,
        LOCAL_TVT_CHARA_01,
      },
      {
        LOCAL_TVT_PERSON_CHELEN,
        LOCAL_TVT_CHARA_02,
      },
      {
        LOCAL_TVT_PERSON_BELL,
        LOCAL_TVT_CHARA_03,
      },
      {
        LOCAL_TVT_PERSON_MOTHER,
        LOCAL_TVT_CHARA_05,
      },
      { //隠しキャラのみ分岐する
        LOCAL_TVT_PERSON_SECRET,
        LOCAL_TVT_CHARA_06,
      },
    };

    int i;
    int cnt = 0;
    u32 sex = LOCAL_TVT_CALLDATA_GetSecretSex( p_wk->p_data );
    u16 person  = 0;
    u16 msgid   = 0;
    for( i = 0; i < NELEMS(sc_call_data) && cnt < LOCAL_TVT_CALLSELECT_SELECT_MAX; i++ )
    {
      if( LOCAL_TVT_CALLDATA_IsCall( p_wk->p_data, sc_call_data[i].person ) )
      {
        if( i == LOCAL_TVT_PERSON_SECRET )
        {
          person  = LOCAL_TVT_PERSON_SECRET;
          if( sex == PTL_SEX_MALE )
          {
            msgid = LOCAL_TVT_CHARA_06;
          }
          else
          {
            msgid = LOCAL_TVT_CHARA_07;
          }
        }
        else
        {
          person  = sc_call_data[i].person;
          msgid   = sc_call_data[i].msgid;
        }
        p_wk->call_list[cnt]    = person;
        p_param->msgID[cnt]     = msgid;
        p_param->is_select[cnt] = LOCAL_TVT_CALLDATA_IsSelect( p_wk->p_data, person );
        cnt++;
      }
    }
    p_param->max = cnt;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物設定
 *
 *	@param	LOCAL_TVT_CALL  *p_wk             メインワーク
 *	@param	LOCAL_TVT_PERSON *p_person        設定構造体
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_SetPerson( LOCAL_TVT_CALL *p_wk, LOCAL_TVT_PERSON *p_person )
{
  LOCAL_TVT_CHARA_PARAM param[LTM_4_MEMBER];
  GFL_STD_MemClear( param, sizeof(LOCAL_TVT_CHARA_PARAM)*LTM_4_MEMBER );

  //自分の絵の設定
  GFL_STD_MemClear( p_person, sizeof(LOCAL_TVT_PERSON) );
  {

    PLAYER_WORK *p_player = GAMEDATA_GetMyPlayerWork( p_wk->param.p_gamedata );
    u16 zoneID    = PLAYERWORK_getZoneID(p_player);
    param[1].bgType       = LOCAL_TVT_CALLDATA_GetBackByZoneID( p_wk->p_data, zoneID );
    param[1].weatherType  = LOCAL_TVT_CALLDATA_GetWeatherByZoneID( p_wk->p_data, zoneID );
  }
  param[1].charaType    = LTCT_PLAYER_M;
  param[2].charaType    = LTCT_NONE;
  param[3].charaType    = LTCT_NONE;

  //相手の設定
  switch( p_wk->call_list[p_wk->call_no] )
  {
  case LOCAL_TVT_PERSON_DOCTOR:
    param[0].charaType    = LTCT_DOCTOR_D;
    param[0].bgType       = LOCAL_TVT_CALLDATA_GetBack( p_wk->p_data, LOCAL_TVT_PERSON_DOCTOR );
    param[0].weatherType  = LOCAL_TVT_CALLDATA_GetWeather( p_wk->p_data, LOCAL_TVT_PERSON_DOCTOR );
    LOCAL_TVT_DOCTOR_SetPerson( p_person, LOCAL_TVT_MESSAGE_GetCallMsgData( p_wk->param.p_message), p_wk->param.p_gamedata, p_wk->heapID );
    break;
  case LOCAL_TVT_PERSON_RIVAL:
    param[0].charaType    = LTCT_RIVAL;
    param[0].bgType       = LOCAL_TVT_CALLDATA_GetBack( p_wk->p_data, LOCAL_TVT_PERSON_RIVAL );
    param[0].weatherType  = LOCAL_TVT_CALLDATA_GetWeather( p_wk->p_data, LOCAL_TVT_PERSON_RIVAL );
    LOCAL_TVT_RIVAL_SetPerson( p_person, LOCAL_TVT_MESSAGE_GetCallMsgData( p_wk->param.p_message), p_wk->param.p_gamedata, p_wk->heapID );
    break;
  case LOCAL_TVT_PERSON_CHELEN:
    param[0].charaType    = LTCT_CHELEN;
    param[0].bgType       = LOCAL_TVT_CALLDATA_GetBack( p_wk->p_data, LOCAL_TVT_PERSON_CHELEN );
    param[0].weatherType  = LOCAL_TVT_CALLDATA_GetWeather( p_wk->p_data, LOCAL_TVT_PERSON_CHELEN );
    LOCAL_TVT_CHELEN_SetPerson( p_person, LOCAL_TVT_MESSAGE_GetCallMsgData( p_wk->param.p_message), p_wk->param.p_gamedata, p_wk->heapID );
    break;
  case LOCAL_TVT_PERSON_BELL:
    param[0].charaType    = LTCT_SUPPORT;
    param[0].bgType       = LOCAL_TVT_CALLDATA_GetBack( p_wk->p_data, LOCAL_TVT_PERSON_BELL );
    param[0].weatherType  = LOCAL_TVT_CALLDATA_GetWeather( p_wk->p_data, LOCAL_TVT_PERSON_BELL );
    LOCAL_TVT_SUPPORT_SetPerson( p_person, LOCAL_TVT_MESSAGE_GetCallMsgData( p_wk->param.p_message), p_wk->param.p_gamedata, p_wk->heapID );
    break;
  case LOCAL_TVT_PERSON_MOTHER:
    param[0].charaType    = LTCT_MOTHER;
    param[0].bgType       = LOCAL_TVT_CALLDATA_GetBack( p_wk->p_data, LOCAL_TVT_PERSON_MOTHER );
    param[0].weatherType  = LOCAL_TVT_CALLDATA_GetWeather( p_wk->p_data, LOCAL_TVT_PERSON_MOTHER );
    LOCAL_TVT_MOTHER_SetPerson( p_person, p_wk->param.p_message, LOCAL_TVT_MESSAGE_GetCallMsgData( p_wk->param.p_message), p_wk->param.p_gamedata, p_wk->heapID );
    break;
  case LOCAL_TVT_PERSON_SECRET:
    switch( LOCAL_TVT_SECRET_GetFace( p_wk->p_data ) )
    {
    default:
      GF_ASSERT(0);
    case LOCAL_TVT_SECRET_FACE_INVISIBLE:
      param[0].charaType    = LTCT_SOUNDONLY;
      break;
    case LOCAL_TVT_SECRET_FACE_HIDE_M:
      param[0].charaType    = LTCT_SECRET_M;
      break;
    case LOCAL_TVT_SECRET_FACE_HIDE_F:
      param[0].charaType    = LTCT_SECRET_F;
      break;
    case LOCAL_TVT_SECRET_FACE_VISIBLE_M:
      param[0].charaType    = LTCT_SECRET_M;
      break;
    case LOCAL_TVT_SECRET_FACE_VISIBLE_F:
      param[0].charaType    = LTCT_SECRET_F;
      break;
    }
    param[0].bgType       = LOCAL_TVT_CALLDATA_GetBack( p_wk->p_data, LOCAL_TVT_PERSON_SECRET );
    param[0].weatherType  = LOCAL_TVT_CALLDATA_GetWeather( p_wk->p_data, LOCAL_TVT_PERSON_SECRET );
    {
      LOCAL_TVT_SECRET_MODE mode  = LOCAL_TVT_SECRET_MODE_RECIVE;
      if( p_wk->param.mode == LOCAL_TVT_MODE_APP_SECRET_CALL )
      {
        mode = LOCAL_TVT_SECRET_MODE_SEND;
      }
      LOCAL_TVT_SECRET_SetPerson( p_person, LOCAL_TVT_MESSAGE_GetCallMsgData( p_wk->param.p_message), p_wk->param.p_gamedata, p_wk->p_data, mode, p_wk->heapID );
    }
    break;
  default:
    LOCAL_TVT_SUPPORT_SetPerson( p_person, LOCAL_TVT_MESSAGE_GetCallMsgData( p_wk->param.p_message), p_wk->param.p_gamedata, p_wk->heapID );
    break;
  }

  //絵の読み込み
  LOCAL_TVT_CHARA_SYS_LoadChara( p_wk->param.p_chara_sys, param, LTM_2_MEMBER );
}

//=============================================================================
/**
 *  シーケンス関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン  送信モード
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq      関数内シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_FadeInSendMode( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
  };
  LOCAL_TVT_CALL  *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_START:
    {
      LOCAL_TVT_CALLSELECT_PARAM  param;
      Local_Tvt_SetCallSelect( p_wk, &param );
      p_wk->p_callselect  = LOCAL_TVT_CALLSELECT_Init( &param, p_wk->heapID );
      GFL_NET_ReloadIconTopOrBottom(FALSE , p_wk->heapID );
    }

		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    (*p_seq)++;
    break;

  case SEQ_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_CallSelect );
    break;

  default:
    GF_ASSERT(0);
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン  送信モード
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq      関数内シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_FadeInReceiveMode( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_LOAD_INIT,
    SEQ_LOAD_WAIT,
    SEQ_INIT,
    SEQ_WAIT,
    SEQ_END,
  };
  LOCAL_TVT_CALL  *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_LOAD_INIT:
    {
      LOCAL_TVT_PERSON person;
      LOCAL_TVT_PERSON_SYS_PARAM  param;
      Local_Tvt_SetPerson( p_wk, &person );

      GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_PERSON_SYS_PARAM));
      param.p_message   = p_wk->param.p_message;
      param.p_chara_sys = p_wk->param.p_chara_sys;
      param.p_clunit    = p_wk->param.p_clunit;
      param.p_data      = p_wk->p_data;
      p_wk->p_person  = LOCAL_TVT_PERSON_SYS_Init( &person, &param, p_wk->heapID );
    }
    (*p_seq)++;
    break;

  case SEQ_LOAD_WAIT:
    if( LOCAL_TVT_CHARA_SYS_IsLoadEnd( p_wk->param.p_chara_sys ) )
    {
      LOCAL_TVT_CHARA_SYS_SetVisible( p_wk->param.p_chara_sys, TRUE );
      (*p_seq)++;
    }
    break;

  case SEQ_INIT:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    (*p_seq)++;
    break;

  case SEQ_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_Talk );
    break;

  default:
    GF_ASSERT(0);
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し選択
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq      関数内シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_CallSelect( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_DECIDE_EXIT,
    SEQ_CALCEL_EXIT,
  };

  LOCAL_TVT_CALL  *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_INIT:
    if( p_wk->p_callselect == NULL )
    {
      LOCAL_TVT_CALLSELECT_PARAM  param;
      Local_Tvt_SetCallSelect( p_wk, &param );
      p_wk->p_callselect  = LOCAL_TVT_CALLSELECT_Init( &param, p_wk->heapID );
      GFL_NET_ReloadIconTopOrBottom(FALSE , p_wk->heapID );
    }
    (*p_seq)++;
    break;

  case SEQ_MAIN:

    LOCAL_TVT_CALLSELECT_Main( p_wk->p_callselect );

    if( LOCAL_TVT_CALLSELECT_GetResult( p_wk->p_callselect ) == LOCAL_TVT_CALLSELECT_RESULT_DECIDE )
    {
      (*p_seq)  = SEQ_DECIDE_EXIT;
    }
    else if( LOCAL_TVT_CALLSELECT_GetResult( p_wk->p_callselect ) == LOCAL_TVT_CALLSELECT_RESULT_CANCEL )
    {
      (*p_seq)  = SEQ_CALCEL_EXIT;
    }
    break;

  case SEQ_DECIDE_EXIT:
    p_wk->call_no = LOCAL_TVT_CALLSELECT_GetSelect( p_wk->p_callselect );
    LOCAL_TVT_CALLSELECT_Exit( p_wk->p_callselect );
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_Calling );
    break;

  case SEQ_CALCEL_EXIT:
    LOCAL_TVT_CALLSELECT_Exit( p_wk->p_callselect );
    p_wk->p_callselect = NULL;
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_FadeOut );
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し中
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq      関数内シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_Calling( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  LOCAL_TVT_CALL  *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_INIT:
    CALLMSG_Init( &p_wk->callmsg, p_wk->param.p_message, p_wk->heapID );
    (*p_seq)++;
    break;

  case SEQ_MAIN:
    CALLMSG_Main( &p_wk->callmsg );
    if( CALLMSG_IsEnd( &p_wk->callmsg ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_EXIT:
    CALLMSG_Exit( &p_wk->callmsg );
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_Talk );
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  キャラと会話 
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq      関数内シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_Talk( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_LOAD_WAIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  LOCAL_TVT_CALL  *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_INIT:
    if( p_wk->p_person == NULL )
    {
      LOCAL_TVT_PERSON person;
      LOCAL_TVT_PERSON_SYS_PARAM  param;
      Local_Tvt_SetPerson( p_wk, &person );

      GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_PERSON_SYS_PARAM));
      param.p_message   = p_wk->param.p_message;
      param.p_chara_sys = p_wk->param.p_chara_sys;
      param.p_clunit    = p_wk->param.p_clunit;
      param.p_data      = p_wk->p_data;
      p_wk->p_person  = LOCAL_TVT_PERSON_SYS_Init( &person, &param, p_wk->heapID );
      (*p_seq)++;
    }
    else
    {
      (*p_seq)  = SEQ_MAIN;
    }
    break;
    
  case SEQ_LOAD_WAIT:
    if( LOCAL_TVT_CHARA_SYS_IsLoadEnd( p_wk->param.p_chara_sys ) )
    {
      LOCAL_TVT_CHARA_SYS_SetVisible( p_wk->param.p_chara_sys, TRUE );
      PMSND_PlaySE( LOCAL_TVT_SE_CALL_ANSWER );
      (*p_seq)++;
    }
    break;

  case SEQ_MAIN:
    LOCAL_TVT_PERSON_SYS_Main( p_wk->p_person );
    if( LOCAL_TVT_PERSON_SYS_IsEnd( p_wk->p_person ) )
    {
      (*p_seq)++;
    }
    break;

  case SEQ_EXIT:
    LOCAL_TVT_PERSON_SYS_Exit( p_wk->p_person );
    p_wk->p_person = NULL;
    LOCAL_TVT_SEQ_SetNext( p_seqwk, LOCAL_TVT_SEQ_FadeOut );
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	LOCAL_TVT_SEQ *p_seqwk  シーケンスワーク
 *	@param	*p_seq      関数内シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_SEQ_FadeOut( LOCAL_TVT_SEQ *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
  };

  switch( *p_seq )
  {
  case SEQ_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    (*p_seq)++;
    break;

  case SEQ_WAIT:
		if( !GFL_FADE_CheckFade() )
		{
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    LOCAL_TVT_SEQ_End( p_seqwk );
    break;

  default:
    GF_ASSERT(0);
  }
}

//=============================================================================
/**
 *    呼び出しメッセージ
 */
//=============================================================================
#define CALLMSG_CALL_CNT  (240)
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しメッセージ初期化
 *
 *	@param	CALLMSG *p_wk ワーク
 *	@param	*p_message    メッセージ
 *	@param	heapID        ヒープID
 */
//-----------------------------------------------------------------------------
static void CALLMSG_Init( CALLMSG *p_wk, LOCAL_TVT_MESSAGE *p_message, HEAPID heapID )
{

  GF_ASSERT( GFL_MSG_GetStrCount(LOCAL_TVT_MESSAGE_GetCallMsgData(p_message)) != 0 );
  GFL_STD_MemClear( p_wk, sizeof(CALLMSG));
  GF_ASSERT( GFL_MSG_GetStrCount(LOCAL_TVT_MESSAGE_GetCallMsgData(p_message)) != 0 );
  p_wk->p_message = p_message;
  p_wk->p_msgwin = GFL_BMPWIN_Create( LTVT_FRAME_SUB_MSG , 
                                        2 , 9 , 28 , 2 ,
                                        LTVT_PLT_BG_SUB_FONT,
                                        GFL_BMP_CHRAREA_GET_B );
  GF_ASSERT( GFL_MSG_GetStrCount(LOCAL_TVT_MESSAGE_GetCallMsgData(p_wk->p_message)) != 0 );
  GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_msgwin );

  GF_ASSERT( GFL_MSG_GetStrCount(LOCAL_TVT_MESSAGE_GetCallMsgData(p_wk->p_message)) != 0 );
  //表示開始
  LOCAL_TVT_MESSAGE_SetPrintColor( p_wk->p_message, LOCAL_TVT_MSG_COLOR_BLACK );
  GF_ASSERT( GFL_MSG_GetStrCount(LOCAL_TVT_MESSAGE_GetCallMsgData(p_wk->p_message)) != 0 );
  LOCAL_TVT_MESSAGE_PrintMsgCall( p_wk->p_message, p_wk->p_msgwin, CALL_STR_03 );
  LOCAL_TVT_MESSAGE_WriteFrame( p_wk->p_message, p_wk->p_msgwin );

  p_wk->p_timeicon  = TIMEICON_Create( GFUser_VIntr_GetTCBSYS(),
      p_wk->p_msgwin, 0x0F, TIMEICON_DEFAULT_WAIT, heapID );

  GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MSG );

  PMSND_PlaySE_byPlayerID( LOCAL_TVT_SE_TEL_CALL, SEPLAYER_SE2 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しメッセージ破棄
 *
 *	@param	CALLMSG *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CALLMSG_Exit( CALLMSG *p_wk )
{
  TIMEICON_Exit( p_wk->p_timeicon );

  //スクリーンクリア
  LOCAL_TVT_MESSAGE_ClearFrame( p_wk->p_message, p_wk->p_msgwin );
  GFL_BMPWIN_ClearTransWindow( p_wk->p_msgwin );
  GFL_BMPWIN_Delete( p_wk->p_msgwin );

  GFL_STD_MemClear( p_wk, sizeof(CALLMSG));
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しメッセージメイン処理
 *
 *	@param	CALLMSG *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void CALLMSG_Main( CALLMSG *p_wk )
{
  if( p_wk->is_end )
  {
    return;
  }
      
  if( p_wk->call_cnt == CALLMSG_CALL_CNT/2 )
  {
    PMSND_PlaySE_byPlayerID( LOCAL_TVT_SE_TEL_CALL, SEPLAYER_SE2 );
  }

  if( p_wk->call_cnt++ >= CALLMSG_CALL_CNT )
  {
    PMSND_StopSE();
    p_wk->is_end  = TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しメッセージ終了検知
 *
 *	@param	const CALLMSG *cp_wk  ワーク
 *
 *	@return TRUEで終了  FALSEでまだ
 */
//-----------------------------------------------------------------------------
static BOOL CALLMSG_IsEnd( const CALLMSG *cp_wk )
{
  return cp_wk->is_end;
}
