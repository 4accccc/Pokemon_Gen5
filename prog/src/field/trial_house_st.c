//======================================================================
/**
 * @file  trial_house_st.c
 * @brief トライアルハウス　常駐部
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "trial_house.h"
#include "trial_house_def.h"
#include "system/main.h"  //for HEAPID_APP_CONTROL

#include "fld_btl_inst_tool.h"
#include "poke_tool/regulation_def.h"
#include "event_battle.h" //for EVENT_BSubwayTrainerBattle

//------------------------------------------------------------------
/**
 * @brief ポケモン選択ワーク
 */
//------------------------------------------------------------------
typedef struct 
{
	GAMESYS_WORK*      gsys;  // ゲームシステム
	FIELDMAP_WORK* fieldmap;  // フィールドマップ
  PLIST_DATA*      PokeListlData;  // ポケモンリスト
  BOOL*            RetDecide;  // 選択したかのチェック
  PL_LIST_TYPE ListType;
  PL_MODE_TYPE ModeType;
  int Reg;
  POKEPARTY* Party;
  PL_SELECT_POS Select;
  PL_RETURN_TYPE RetType;
  u8 SelAry[6];
  u8 dummy[2];
  TRIAL_HOUSE_WORK_PTR HouseWorkPtr;
}TH_POKESEL_WORK;

static GMEVENT_RESULT PokeSelEvt(GMEVENT * event, int * seq, void * work);

//--------------------------------------------------------------
/**
 * ポケモン選択イベント作成
 * @param gsys          ゲームシステムポインタ
 * @param inListType    バトルタイプ　シングルＯＲダブル
 * @param inPartyType   パーティタイプ　手持ちＯＲバトルボックス
 * @param outRet        リストの結果    中止のときFALSE
 *
 * @retval GMEVENT      イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *TRIAL_HOUSE_CreatePokeSelEvt(  GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr,
                                        const int inBtlType, const int inPartyType, BOOL *outRet )
{
  GMEVENT* event;
	TH_POKESEL_WORK* work;
  GAMEDATA*  gdata = GAMESYSTEM_GetGameData( gsys );

  // イベント生成
  event = GMEVENT_Create(gsys, NULL, PokeSelEvt, sizeof(TH_POKESEL_WORK));
  work = GMEVENT_GetEventWork(event);
	work->gsys = gsys;
	work->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  work->RetDecide = outRet;
  work->ModeType = PL_MODE_BATTLE;

  work->HouseWorkPtr = ptr;
  if (inBtlType)
  {
    work->ListType = PL_TYPE_SINGLE;
    work->Reg = REG_SUBWAY_SINGLE;
  }
  else
  {
    work->ListType = PL_TYPE_DOUBLE;
    work->Reg = REG_SUBWAY_DOUBLE;
  }

  if ( inPartyType ) work->Party = GAMEDATA_GetMyPokemon( gdata );  //手持ち
  else work->Party = GAMEDATA_GetMyPokemon( gdata );                //バトルボックス @todo

  return event;
}

static GMEVENT_RESULT PokeSelEvt(GMEVENT * event, int * seq, void * work)
{
	TH_POKESEL_WORK *evt_wk = work;
	GAMESYS_WORK *gsys = evt_wk->gsys;
  FIELDMAP_WORK *fieldmap = evt_wk->fieldmap;

	switch( *seq ) 
  {
  case 0:
    //ポケモンリストイベントコール
    {
      GMEVENT* call_event;
      call_event = FBI_TOOL_CreatePokeListEvt(
          gsys,
          evt_wk->ListType, evt_wk->Reg, evt_wk->Party, evt_wk->SelAry,
          &evt_wk->Select, &evt_wk->RetType );
      GMEVENT_CallEvent(event, call_event);
    }
    *seq = 1;
		break;
  case 1:
    //結果の格納
    if( evt_wk->RetType != PL_RET_NORMAL ||
      evt_wk->Select == PL_SEL_POS_EXIT ||
      evt_wk->Select == PL_SEL_POS_EXIT2 )
    {
      *(evt_wk->RetDecide) = FALSE;          //キャンセル
    }
    else
    {
      *(evt_wk->RetDecide) = TRUE;           //ポケモンを選択して、けっていした
      //シングル３体、ダブル４体をトライアルハウスポケパーティにセット
      ;
    }
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}


//--------------------------------------------------------------
/**
 * 戦闘イベント作成
 * @param gsys          ゲームシステムポインタ
 * @param inListType    バトルタイプ　シングルＯＲダブル
 * @retval GMEVENT      イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *TRIAL_HOUSE_CreateBtlEvt( GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr )
{
  GMEVENT* event;
	BATTLE_SETUP_PARAM *bp;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  {
    POKEPARTY *party = ptr->Party;
    BSWAY_PLAYMODE mode = ptr->PlayMode;
    BSUBWAY_PARTNER_DATA *tr_data = &ptr->TrData;
    int num = ptr->MemberNum;

    bp = FBI_TOOL_CreateBattleParam( gsys, party, mode, tr_data, num  );
  }

  //バトルサブウェイの戦闘イベントを作成
  event = EVENT_BSubwayTrainerBattle( gsys, fieldmap, bp );

  return event;
}

