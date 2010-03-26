/*
 *  @file   event_freeword_input.c
 *  @brief  すれ違い関連フリーワード入力イベント
 *  @author Miyuki Iwasawa
 *  @date   10.03.25
 *
 *  NameInアプリを呼び出し、結果をワークに反映するまでを行う
 */

#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "msgdata.h"
#include "print/wordset.h"

#include "fieldmap.h"
#include "event_fieldmap_control.h"
#include "sound/pm_sndsys.h"

#include "gamesystem/game_beacon.h"
#include "gamesystem/beacon_status.h"
#include "savedata/misc.h"

#include "event_freeword_input.h"

typedef struct _EVWORK_FWORD_INPUT{
  GAMESYS_WORK* gsys;
  GAMEDATA*     gdata;
  MISC*         misc;
  MYSTATUS*     my;
  FIELDMAP_WORK* fieldmap;
  BEACON_STATUS* b_status;

  u16*          ret_wk;   ///<入力を確定したかどうかを返すワーク
  NAMEIN_PARAM  param;
 
  HEAPID  heapID;
  int mode;
  
}EVWORK_FWORD_INPUT;

static const u8 DATA_WordLength[] = {
 NAMEIN_GREETING_WORD_LENGTH,
 NAMEIN_THANKS_WORD_LENGTH,
 NAMEIN_FREE_WORD_LENGTH,
};

//==========================================================
///プロトタイプ
//==========================================================
static GMEVENT_RESULT FreeWordInputEvent(GMEVENT * event, int * seq, void *work);
static void sub_ResultGet( EVWORK_FWORD_INPUT* wk, NAMEIN_PARAM* param );

//-------------------------------------------------------------------
/*
 *  @brief  フリーワード入力汎用イベント生成
 */
//-------------------------------------------------------------------
GMEVENT* EVENT_FreeWordInput( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, GMEVENT* parent, int mode, u16* ret_wk )
{
 	GMEVENT* event = GMEVENT_Create(gsys, parent, FreeWordInputEvent, sizeof(EVWORK_FWORD_INPUT));
	EVWORK_FWORD_INPUT *  wk = GMEVENT_GetEventWork(event);

  wk->gsys = gsys;
  wk->gdata = GAMESYSTEM_GetGameData( gsys );
  wk->fieldmap = fieldmap;
  wk->ret_wk = ret_wk;

  wk->mode = mode;
  wk->heapID = HEAPID_PROC;

  wk->misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork( wk->gdata ) );
  wk->my = GAMEDATA_GetMyStatus( wk->gdata );
  wk->b_status = GAMEDATA_GetBeaconStatus( wk->gdata );

  //名前入力パラメータ初期化
  wk->param.mode = mode;
  wk->param.hero_sex = MyStatus_GetMySex( wk->my );
  wk->param.p_misc = wk->misc;
  
  GF_ASSERT( mode >= NAMEIN_GREETING_WORD && mode <= NAMEIN_FREE_WORD );

  wk->param.wordmax = DATA_WordLength[ mode - NAMEIN_GREETING_WORD ];
  wk->param.strbuf = GFL_STR_CreateBuffer( wk->param.wordmax+EOM_SIZE, wk->heapID );

  switch( mode ){
  case NAMEIN_GREETING_WORD: // すれちがい挨拶言葉 param1自分の性別
    GFL_STR_SetStringCode( wk->param.strbuf,  MISC_CrossComm_GetSelfIntroduction( wk->misc ));
    break;
  case NAMEIN_THANKS_WORD:   // すれちがいお礼言葉 param1自分の性別
    GFL_STR_SetStringCode( wk->param.strbuf, MISC_CrossComm_GetThankyouMessage( wk->misc ) );
    break;
  case NAMEIN_FREE_WORD:  //すれ違いフリーワード
    GFL_STR_CopyBuffer( wk->param.strbuf, BEACON_STATUS_GetFreeWordBuffer( wk->b_status ));
    break;
  default:
    GF_ASSERT(0);
  }
  return event;
}

static GMEVENT_RESULT FreeWordInputEvent(GMEVENT * event, int * seq, void *work)
{
	EVWORK_FWORD_INPUT * wk = work;
	
  switch (*seq) {
  case 0:
    {
      GMEVENT* child = 
        EVENT_FieldSubProc( wk->gsys, wk->fieldmap,
          FS_OVERLAY_ID(namein), &NameInputProcData, &wk->param );
      GMEVENT_CallEvent( event, child );
    }
    (*seq)++;
    break;
  case 1:
    //結果反映
    sub_ResultGet( wk, &wk->param );

    //後始末
    GFL_STR_DeleteBuffer( wk->param.strbuf );
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//-----------------------------------------------------------
/*
 *  @brief  結果反映
 */
//-----------------------------------------------------------
static void sub_ResultGet( EVWORK_FWORD_INPUT* wk, NAMEIN_PARAM* param )
{
  if( wk->mode == NAMEIN_FREE_WORD ){
    //すれ違いフリーワードは、元の文字列と変わっていなくてもキャンセル扱いにしない
    int strlen =  GFL_STR_GetBufferLength( param->strbuf );
    if( strlen > 0 ){
      GFL_STR_CopyBuffer( BEACON_STATUS_GetFreeWordBuffer( wk->b_status ), wk->param.strbuf );
      GAMEBEACON_Set_FreeWord( wk->gdata, param->strbuf );
    }
    if(wk->ret_wk != NULL){
      *wk->ret_wk = (strlen > 0);
    }
    return;
  }

  if( wk->ret_wk != NULL ){
    *wk->ret_wk = ( param->cancel == FALSE );
  }
  if( param->cancel == TRUE ){  //キャンセルされた
    return;
  }
  switch( wk->mode ){
  case NAMEIN_GREETING_WORD: // すれちがい挨拶言葉
    MISC_CrossComm_SetSelfIntroduction( wk->misc, param->strbuf );
    GAMEBEACON_SendDataUpdate_SelfIntroduction();
    break;
  case NAMEIN_THANKS_WORD:   // すれちがいお礼言葉
    MISC_CrossComm_SetThankyouMessage( wk->misc, param->strbuf );
    break;
  default:
    GF_ASSERT(0);
  }
}

