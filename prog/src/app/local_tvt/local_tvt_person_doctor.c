//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_person_doctor.c
 *	@brief  各人物処理：博士
 *	@author	Toru=Nagihashi
 *	@data		2011.01.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "poke_tool/shinka_check.h"
#include "poke_tool/monsno_def.h"
#include "item/itemsym.h"
#include "waza_tool/wazano_def.h"
#include "savedata/zukan_savedata.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_tvt_shinka.h"
#include "script_message.naix"
#include "msg/script/msg_hyouka_scr.h"

//自分のモジュール
#include "local_tvt_local_def.h"
#include "local_tvt_graphic.h"
#include "local_tvt_util.h"
#include "local_tvt_debug.h"

//外部公開
#include "local_tvt_person_doctor.h"

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
///	進化データ
//=====================================
typedef struct 
{
  u16 param;
  u16 msgID;
}SHINKA_DATA;

//-------------------------------------
///	評価データ
//=====================================
typedef struct 
{
  u16 range;
  u16 msgID;
} ZUKAN_HYOUKA_DATA;


//-------------------------------------
///	博士用ワーク
//=====================================
typedef struct 
{
  s32 seq;
  LOCAL_TVT_POKELIST *p_pokelist;
  GAMEDATA            *p_gamedata;
  GFL_MSGDATA         *p_msgdata_evo;
  GFL_MSGDATA         *p_msgdata_hyouka;
  ARCHANDLE           *p_evo_handle;
  HEAPID              heapID;
} LOCAL_TVT_DOCTOR;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static LOCAL_TVT_DOCTOR *LOCAL_TVT_DOCTOR_Init( GAMEDATA *p_gamedata, HEAPID heapID );
static void LOCAL_TVT_DOCTOR_Exit( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static BOOL LOCAL_TVT_DOCTOR_Greeting( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

static BOOL LOCAL_TVT_DOCTOR_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_DOCTOR_CheckEvolution( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_DOCTOR_CheckZukan( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );
static BOOL LOCAL_TVT_DOCTOR_Farewell( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs );

//進化チェック
static u16 Shinka_GetMsgID( ARCHANDLE *p_handle, u16 monsNO );
static BOOL ShinkaData_Search( const SHINKA_DATA *cp_data_tbl, u32 len, u16 param, u16 *p_msgID );

//図鑑チェック
static u16 ZukanCheck_GetMsgID( const ZUKAN_SAVEDATA *p_zukan_sv, HEAPID heapID );
static u16 ZukanCheck_GetMe( const ZUKAN_SAVEDATA *p_zukan_sv, HEAPID heapID );
static BOOL ZukanData_Search( const ZUKAN_HYOUKA_DATA *cp_data_tbl, u32 len, u16 param, u16 *p_msgID );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	通信アイテム進化データ
//=====================================
static const SHINKA_DATA sc_tuushin_item_data[]=
{
  {
    ITEM_APPUGUREEDO,
    SHINKA_STR_030,
  },
  {
    ITEM_OUZYANOSIRUSI,
    SHINKA_STR_031,
  },
  {
    ITEM_SINKAINOKIBA,
    SHINKA_STR_032,
  },
  {
    ITEM_SINKAINOUROKO,
    SHINKA_STR_033,
  },
  {
    ITEM_METARUKOOTO,
    SHINKA_STR_034,
  },
  {
    ITEM_RYUUNOUROKO,
    SHINKA_STR_035,
  },
  {
    ITEM_PUROTEKUTAA,
    SHINKA_STR_036,
  },
  {
    ITEM_EREKIBUUSUTAA,
    SHINKA_STR_037,
  },
  {
    ITEM_MAGUMABUUSUTAA,
    SHINKA_STR_038,
  },
  {
    ITEM_AYASIIPATTI,
    SHINKA_STR_039,
  },
  {
    ITEM_REIKAINONUNO,
    SHINKA_STR_040,
  },
};
//-------------------------------------
///	アイテム使用進化データ
//=====================================
static const SHINKA_DATA sc_item_use_data[]=
{
  {
    ITEM_KAMINARINOISI,
    SHINKA_STR_043,
  },
  {
    ITEM_TAIYOUNOISI,
    SHINKA_STR_044,
  },
  {
    ITEM_TUKINOISI,
    SHINKA_STR_045,
  },
  {
    ITEM_HONOONOISI,
    SHINKA_STR_046,
  },
  {
    ITEM_MIZUNOISI,
    SHINKA_STR_047,
  },
  {
    ITEM_RIIHUNOISI,
    SHINKA_STR_048,
  },
  {
    ITEM_HIKARINOISI,
    SHINKA_STR_049,
  },
};

//-------------------------------------
///	ポケモン分岐データアイテム使用進化データ
//=====================================
static const SHINKA_DATA sc_soubi_yoru_data[]=
{
  {
    ITEM_SURUDOIKIBA,
    SHINKA_STR_041,
  },
  {
    ITEM_SURUDOITUME,
    SHINKA_STR_042,
  },
};

//-------------------------------------
///	技進化データ
//=====================================
static const SHINKA_DATA sc_waza_data[]=
{
  {
    WAZANO_MONOMANE,
    SHINKA_STR_050,
  },
  {
    WAZANO_DABURUATAKKU,
    SHINKA_STR_051,
  },
  {
    WAZANO_KOROGARU,
    SHINKA_STR_052,
  },
  {
    WAZANO_GENSINOTIKARA,
    SHINKA_STR_053,
  },
};

//-------------------------------------
///	ポケモン個別進化データ
//=====================================
static const SHINKA_DATA sc_pokemon_data[]=
{
  {
    MONSNO_KUSAIHANA,
    SHINKA_STR_054,
  },
  {
    MONSNO_NYOROZO,
    SHINKA_STR_055,
  },
  {
    MONSNO_YADON,
    SHINKA_STR_056,
  },
  {
    MONSNO_IIBUI,
    SHINKA_STR_057,
  },
  {
    MONSNO_BARUKII,
    SHINKA_STR_058,
  },
  {
    MONSNO_KEMUSSO,
    SHINKA_STR_059,
  },
  {
    MONSNO_KIRURIA,
    SHINKA_STR_060,
  },
  {
    MONSNO_TUTININ,
    SHINKA_STR_061,
  },
  {
    MONSNO_YUKIWARASI,
    SHINKA_STR_062,
  },
  {
    MONSNO_PAARURU,
    SHINKA_STR_063,
  },
  {
    MONSNO_MINOMUTTI,
    SHINKA_STR_064,
  },
};
//-------------------------------------
///	図鑑評価メッセージデータ
//=====================================
static const ZUKAN_HYOUKA_DATA sc_zukan_hyouka_data[] = 
{
  {  39, msg_hyouka_z01 },
  {  79, msg_hyouka_z02 },
  { 119, msg_hyouka_z03 },
  { 159, msg_hyouka_z04 },
  { 199, msg_hyouka_z05 },
  { 249, msg_hyouka_z06 },
  { 299, msg_hyouka_z07 },
  { 349, msg_hyouka_z08 },
  { 399, msg_hyouka_z09 },
  { 449, msg_hyouka_z10 },
  { 499, msg_hyouka_z11 },
  { 549, msg_hyouka_z12 },
  { 599, msg_hyouka_z13 },
  { 614, msg_hyouka_z14 },
  { 629, msg_hyouka_z15 },
  { 633, msg_hyouka_z16 },
};

//-------------------------------------
///	図鑑評価MEデータ
//=====================================
static const ZUKAN_HYOUKA_DATA sc_zukan_hyouka_me_data[] = 
{
  { 159, SEQ_ME_HYOUKA1 },
  { 349, SEQ_ME_HYOUKA2 },
  { 499, SEQ_ME_HYOUKA3 },
  { 549, SEQ_ME_HYOUKA4 },
  { 633, SEQ_ME_HYOUKA5 },
};

//----------------------------------------------------------------------------
/**
 *	@brief  パーソンワークを設定
 *
 *	@param	LOCAL_TVT_PERSON *p_person  パーソンワーク
 *	@param  GAMEDATA  ゲームデータ
 *	@param	heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_DOCTOR_SetPerson( LOCAL_TVT_PERSON *p_person, const GFL_MSGDATA *cp_msg, GAMEDATA *p_gamedata, HEAPID heapID )
{
  GFL_STD_MemClear( p_person, sizeof(LOCAL_TVT_PERSON));
  p_person->p_wk_adrs = LOCAL_TVT_DOCTOR_Init( p_gamedata, heapID );
  p_person->ExitFunction  = LOCAL_TVT_DOCTOR_Exit;
  p_person->greetingFunction  = LOCAL_TVT_DOCTOR_Greeting;
  p_person->selectFunction[0] = LOCAL_TVT_DOCTOR_Talk;
  p_person->list_msgID[0]     = CALL_DOCTOR_01;
  p_person->selectFunction[1] = LOCAL_TVT_DOCTOR_CheckEvolution;
  p_person->list_msgID[1]     = CALL_DOCTOR_02;

#if 0
  {
    const ZUKAN_SAVEDATA *cp_zukan  = GAMEDATA_GetZukanSave( p_gamedata );
    if( ZUKANSAVE_GetZenkokuZukanFlag(cp_zukan)
        || LOCAL_TVT_DEBUG_IsFlagOn(LOCAL_TVT_DEBUG_FLAG_CHECK_ZUKAN_ON) )
    {
      //TODO
      //図鑑チェックはクリア後
      p_person->selectFunction[2] = LOCAL_TVT_DOCTOR_CheckZukan;
      p_person->list_msgID[2]     = CALL_DOCTOR_03;
      p_person->selectFunction[3] = LOCAL_TVT_DOCTOR_Farewell;
      p_person->list_msgID[3]     = CALL_DOCTOR_04;
      p_person->list_num          = 4;
    }
    else
    { 
      //クリア前
      p_person->selectFunction[2] = LOCAL_TVT_DOCTOR_Farewell;
      p_person->list_msgID[2]     = CALL_DOCTOR_04;
      p_person->list_num          = 3;
    }
  }
#else
  p_person->selectFunction[2] = LOCAL_TVT_DOCTOR_CheckZukan;
  p_person->list_msgID[2]     = CALL_DOCTOR_03;
  p_person->selectFunction[3] = LOCAL_TVT_DOCTOR_Farewell;
  p_person->list_msgID[3]     = CALL_DOCTOR_04;
  p_person->list_num          = 4;
#endif
  p_person->list_start_msgID  = CALL_DOCTOR_SELECT_01;
  p_person->cp_list_msg_data  = cp_msg;
}

//=============================================================================
/**
 *					プライベート
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  博士用ワークを作成
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static LOCAL_TVT_DOCTOR *LOCAL_TVT_DOCTOR_Init( GAMEDATA *p_gamedata, HEAPID heapID )
{
  LOCAL_TVT_DOCTOR *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_DOCTOR));
  p_wk->heapID      = heapID;
  p_wk->p_gamedata  = p_gamedata;
  p_wk->p_msgdata_evo = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE , NARC_message_tvt_shinka_dat, heapID );
  p_wk->p_msgdata_hyouka = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE , NARC_script_message_hyouka_scr_dat, heapID );
  p_wk->p_evo_handle  = SHINKA_GetArcHandle( heapID );
  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  博士用ワークを破棄
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param  const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 */ 
//-----------------------------------------------------------------------------
static void LOCAL_TVT_DOCTOR_Exit( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  LOCAL_TVT_DOCTOR *p_wk       = p_wk_adrs;
  GFL_ARC_CloseDataHandle( p_wk->p_evo_handle );
  GFL_MSG_Delete( p_wk->p_msgdata_hyouka );
  GFL_MSG_Delete( p_wk->p_msgdata_evo );
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  挨拶
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param  const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_DOCTOR_Greeting( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_DOCTOR *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_doctor = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetGreetingMsgID( cp_param->p_data, LOCAL_TVT_PERSON_DOCTOR );
      LOCAL_TVT_CHARA_StartTalk( p_doctor, cp_msg, msgID );
    }
    p_wk->seq++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_doctor, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_END:
    p_wk->seq = 0;
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  はなす
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_DOCTOR_Talk( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_DOCTOR *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_doctor = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetTalkMsgID( cp_param->p_data, LOCAL_TVT_PERSON_DOCTOR );
      LOCAL_TVT_CHARA_StartTalk( p_doctor, cp_msg, msgID );
    }
    p_wk->seq++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_doctor, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_END:
    p_wk->seq = 0;
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  しんかをおしえて
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_DOCTOR_CheckEvolution( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_MSG_INIT,
    SEQ_MSG_WAIT,
    SEQ_POKE_INIT,
    SEQ_POKE_MAIN,
    SEQ_POKE_END,
    SEQ_RESULT_INIT,
    SEQ_RESULT_WAIT,
  };
  LOCAL_TVT_DOCTOR *p_wk = p_wk_adrs;
  LOCAL_TVT_CHARA* p_doctor = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_MSG_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      LOCAL_TVT_CHARA_StartTalk( p_doctor, cp_msg, CALL_DOCTOR_SHINKA_01 );
    }
    p_wk->seq++;
    break;
 case SEQ_MSG_WAIT:
    if( LOCAL_TVT_CHARA_WaitTalk( p_doctor, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_POKE_INIT:
    if( p_wk->p_pokelist != NULL )
    {
      //作成してるなら再スタート
      LOCAL_TVT_POKELIST_SetActive( p_wk->p_pokelist, TRUE );
      LOCAL_TVT_POKELIST_ReStart( p_wk->p_pokelist );
    }
    else 
    {
      //作成してないなら作成
      LOCAL_TVT_POKELIST_PARAM  param;
      GFL_STD_MemClear( &param, sizeof(LOCAL_TVT_POKELIST_PARAM));
      param.p_message = cp_param->p_message;
      param.p_clunit  = cp_param->p_clunit;
      param.p_party   = GAMEDATA_GetMyPokemon( p_wk->p_gamedata );

      p_wk->p_pokelist  = LOCAL_TVT_POKELIST_Init( &param, p_wk->heapID );
    }
    p_wk->seq++;
    break;
  case SEQ_POKE_MAIN:
    LOCAL_TVT_POKELIST_Main( p_wk->p_pokelist );
    switch( LOCAL_TVT_POKELIST_GetResult( p_wk->p_pokelist ) )
    {
    case LOCAL_TVT_POKELIST_RESULT_DECIDE:
      LOCAL_TVT_POKELIST_SetActive( p_wk->p_pokelist, FALSE );
      p_wk->seq = SEQ_RESULT_INIT;
      break;
    case LOCAL_TVT_POKELIST_RESULT_CANCEL:
      p_wk->seq = SEQ_POKE_END;
    }
    break;
  case SEQ_POKE_END:
    LOCAL_TVT_POKELIST_Exit( p_wk->p_pokelist );
    p_wk->p_pokelist  = NULL;
    p_wk->seq = 0;
    return TRUE;
  case SEQ_RESULT_INIT:
    {
      const POKEMON_PARAM *cp_pp = LOCAL_TVT_POKELIST_GetSelect( p_wk->p_pokelist );
      int monsNO  = PP_Get( cp_pp, ID_PARA_monsno, NULL );
      u16 msgID   = Shinka_GetMsgID( p_wk->p_evo_handle, monsNO );

      LOCAL_TVT_MESSAGE_RegisterPlayerName( cp_param->p_message, p_wk->p_gamedata );
      LOCAL_TVT_MESSAGE_RegisterPokeNickName( cp_param->p_message, cp_pp, 1 );
      LOCAL_TVT_CHARA_StartTalk( p_doctor, p_wk->p_msgdata_evo, msgID );
      p_wk->seq++;
    }
    break;
  case SEQ_RESULT_WAIT:
    if( LOCAL_TVT_CHARA_WaitTalk( p_doctor, cp_param->p_chara_sys ) )
    {
      p_wk->seq = SEQ_MSG_INIT;
    }
    break;
  default:
    GF_ASSERT( 0 );
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  図鑑チェック
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_DOCTOR_CheckZukan( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_NUM_INIT,
    SEQ_NUM_MAIN,
    SEQ_ME_INIT,
    SEQ_ME_WAIT,
    SEQ_RESULT_INIT,
    SEQ_RESULT_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_DOCTOR *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_doctor = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_NUM_INIT:
    {
      const ZUKAN_SAVEDATA *cp_zukan_sv  = GAMEDATA_GetZukanSave( p_wk->p_gamedata );
      u16 num = ZUKANSAVE_GetZukanPokeGetCount( cp_zukan_sv, p_wk->heapID );
      LOCAL_TVT_MESSAGE_RegisterNumber( cp_param->p_message, num, 3, 0 );
      LOCAL_TVT_CHARA_StartTalk( p_doctor, p_wk->p_msgdata_hyouka, msg_hyouka_hakase_02 );
    }
    p_wk->seq++;
    break;
  case SEQ_NUM_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_doctor, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_ME_INIT:
    {
      const ZUKAN_SAVEDATA *p_zukan_sv  = GAMEDATA_GetZukanSave( p_wk->p_gamedata );
      u16 me = ZukanCheck_GetMe( p_zukan_sv, p_wk->heapID );
      PMSND_PushBGM();
      PMSND_PlayBGM( me );
    }
    p_wk->seq++;
    break;
  case SEQ_ME_WAIT:
    if( !PMSND_CheckPlayBGM() )
    {
      PMSND_PopBGM();
      p_wk->seq++;
    }
    break;
  case SEQ_RESULT_INIT:
    {
      const ZUKAN_SAVEDATA *p_zukan_sv  = GAMEDATA_GetZukanSave( p_wk->p_gamedata );
      u16 msgID = ZukanCheck_GetMsgID( p_zukan_sv, p_wk->heapID );
      LOCAL_TVT_CHARA_StartTalk( p_doctor, p_wk->p_msgdata_hyouka, msgID );
    }
    p_wk->seq++;
    break;
  case SEQ_RESULT_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_doctor, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_END:
    p_wk->seq = 0;
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  さよなら
 *
 *	@param	LOCAL_TVT_PERSON_SYS *p_person  人物システム
 *	@param const LOCAL_TVT_PERSON_SYS_PARAM *cp_param 引数
 *	@param	*p_wk_adrs  ワーク
 *
 *	@return TRUEで終了  FALSEで継続
 */ 
//-----------------------------------------------------------------------------
static BOOL LOCAL_TVT_DOCTOR_Farewell( LOCAL_TVT_PERSON_SYS *p_person, const LOCAL_TVT_PERSON_SYS_PARAM *cp_param, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_END,
  };
  LOCAL_TVT_DOCTOR *p_wk       = p_wk_adrs;
  LOCAL_TVT_CHARA* p_doctor = LOCAL_TVT_CHARA_SYS_GetWork( cp_param->p_chara_sys, 0 );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    {
      const GFL_MSGDATA *cp_msg = LOCAL_TVT_MESSAGE_GetCallMsgData( cp_param->p_message );
      u16 msgID = LOCAL_TVT_CALLDATA_GetFarewellMsgID( cp_param->p_data, LOCAL_TVT_PERSON_DOCTOR );
      LOCAL_TVT_CHARA_StartTalk( p_doctor, cp_msg, msgID );
    }
    p_wk->seq++;
    break;

  case SEQ_MAIN:
    if( LOCAL_TVT_CHARA_WaitTalk( p_doctor, cp_param->p_chara_sys ) )
    {
      p_wk->seq++;
    }
    break;

  case SEQ_END:
    p_wk->seq = 0;
    LOCAL_TVT_PERSON_End( p_person );
    return TRUE;

  default:
    GF_ASSERT(0);
  }

  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  進化に対応したMSGIDを取り出す
 *
 *  @param  進化検索用ハンドル
 *	@param	u16 monsNO  モンスター番号
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static u16 Shinka_GetMsgID( ARCHANDLE *p_handle, u16 monsNO )
{
  u16 msgID = SHINKA_STR_001;

  //まずはポケモン個別検索
  if( ShinkaData_Search( sc_pokemon_data, NELEMS(sc_pokemon_data), monsNO, &msgID )  )
  {
    return msgID;
  }
  else
  {
    //進化条件検索
    SHINKA_COND shinka_cond = SHINKA_GetParamByHandle( p_handle, monsNO, 0, SHINKA_PARAM_COND );
    u16 param  = SHINKA_GetParamByHandle( p_handle, monsNO, 0, SHINKA_PARAM_DATA );

    //進化条件により、再度検索
    switch( shinka_cond )
    {
    case SHINKA_COND_TUUSHIN_ITEM:  //通信アイテム
      ShinkaData_Search( sc_tuushin_item_data, NELEMS(sc_tuushin_item_data), param, &msgID );
      break;
    case SHINKA_COND_ITEM:  //アイテム
      ShinkaData_Search( sc_item_use_data, NELEMS(sc_item_use_data), param, &msgID );
      break;
    case SHINKA_COND_SOUBI_NIGHT: //夜の装備アイテム
      ShinkaData_Search( sc_soubi_yoru_data, NELEMS(sc_soubi_yoru_data), param, &msgID );
      break;
    case SHINKA_COND_WAZA:  //技
      ShinkaData_Search( sc_waza_data, NELEMS(sc_waza_data), param, &msgID );
      break;
    default:
      msgID = SHINKA_STR_001 + shinka_cond;
    }
  }

  return msgID;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データを検索する
 *
 *	@param	const SHINKA_DATA *cp_data  データ
 *	@param	len       データ長さ
 *	@param  param     検索キー
 *	@param	*p_msgID  メッセージID
 *
 *	@return TRUEで検索完了
 */
//-----------------------------------------------------------------------------
static BOOL ShinkaData_Search( const SHINKA_DATA *cp_data_tbl, u32 len, u16 param, u16 *p_msgID )
{
  int i;
  for( i = 0; i < len; i++ )
  {
    const SHINKA_DATA *cp_data  = &cp_data_tbl[i];
    if( cp_data->param == param )
    {
      *p_msgID  = cp_data->msgID;
      return TRUE;
    }
  }

  *p_msgID  = SHINKA_STR_001;
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  図鑑チェックメッセージを取得
 *
 *	@param	const ZUKAN_SAVEDATA *p_zukan_sv  図鑑セーブ
 *	@param	heapID  ヒープID
 *
 *	@return メッセージ
 */
//-----------------------------------------------------------------------------
static u16 ZukanCheck_GetMsgID( const ZUKAN_SAVEDATA *p_zukan_sv, HEAPID heapID )
{
  u16 count = ZUKANSAVE_GetZukanPokeGetCount( p_zukan_sv, heapID );
  u16 msgID = msg_hyouka_z01;

  if ( ZUKANSAVE_CheckZenkokuComp( p_zukan_sv ) )
  {
    msgID = msg_hyouka_z17;
  }
  else
  {
    ZukanData_Search( sc_zukan_hyouka_data, NELEMS(sc_zukan_hyouka_data), count, &msgID );
  }

  return msgID;
}
//----------------------------------------------------------------------------
/**
 *	@brief  図鑑チェックMEを取得
 *
 *	@param	const ZUKAN_SAVEDATA *p_zukan_sv  図鑑セーブ
 *	@param	heapID  ヒープID
 *
 *	@return ME
 */
//-----------------------------------------------------------------------------
static u16 ZukanCheck_GetMe( const ZUKAN_SAVEDATA *p_zukan_sv, HEAPID heapID )
{
  u16 count = ZUKANSAVE_GetZukanPokeGetCount( p_zukan_sv, heapID );
  u16 me = SEQ_ME_HYOUKA1;

  if ( ZUKANSAVE_CheckZenkokuComp( p_zukan_sv ) )
  {
    me = SEQ_ME_HYOUKA6;
  }
  else
  {
    ZukanData_Search( sc_zukan_hyouka_me_data, NELEMS(sc_zukan_hyouka_me_data), count, &me );
  }

  return me;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データを検索する
 *
 *	@param	const ZUKAN_HYOUKA_DATA *cp_data  データ
 *	@param	len       データ長さ
 *	@param  param     検索キー
 *	@param	*p_msgID  メッセージID
 *
 *	@return TRUEで検索完了
 */
//-----------------------------------------------------------------------------
static BOOL ZukanData_Search( const ZUKAN_HYOUKA_DATA *cp_data_tbl, u32 len, u16 param, u16 *p_msgID )
{
  int i;

  *p_msgID  = cp_data_tbl[0].msgID;
  for( i = 0; i < len; i ++ )
  {
    if( param <= cp_data_tbl[i].range )
    {
      *p_msgID  = cp_data_tbl[i].msgID;
      return TRUE;
    }
  }
  return FALSE; 
}
