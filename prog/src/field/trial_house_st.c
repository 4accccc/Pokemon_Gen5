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
#include "event_battle.h" //for EVENT_TrialHouseTrainerBattle

#include "script_def.h"

#include "savedata/battle_box_save.h"   //for BATTLE_BOX_SAVE

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
  u16*            RetDecide;  // 選択したかのチェック
  PL_LIST_TYPE ListType;
  int Reg;
  POKEPARTY* Party;         //指定ポケパーティポインタ（アロックせず、ポインタとして使用）
  POKEPARTY* BoxParty;      //バトルボックス用ポケパーティ（アロックする）
  PL_SELECT_POS SelectPos;
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
 * @param inRegType    バトルレギュレーションタイプ　シングルＯＲダブル
 * @param inPartyType   パーティタイプ　手持ちＯＲバトルボックス
 * @param outRet        リストの結果    中止のときFALSE
 *
 * @retval GMEVENT      イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *TRIAL_HOUSE_CreatePokeSelEvt(  GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr,
                                        const int inRegType, const int inPartyType, u16 *outRet )
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
  work->HouseWorkPtr = ptr;

  if ( (inRegType == REG_SUBWAY_SINGLE)||(inRegType == REG_SUBWAY_DOUBLE) )
  {
    if (inRegType == REG_SUBWAY_SINGLE)
    {
      work->ListType = PL_TYPE_SINGLE;
      work->Reg = REG_SUBWAY_SINGLE;
    }
    else  //REG_SUBWAY_DOUBLE
    {
      work->ListType = PL_TYPE_DOUBLE;
      work->Reg = REG_SUBWAY_DOUBLE;
    }
  }
  else
  {
    GF_ASSERT_MSG(0,"reg error %d",inRegType);
    work->ListType = PL_TYPE_SINGLE;
    work->Reg = REG_SUBWAY_SINGLE;
  }

  if ( inPartyType == SCR_BTL_PARTY_SELECT_TEMOTI)
  {
    work->BoxParty = NULL;
    work->Party = GAMEDATA_GetMyPokemon( gdata );  //手持ち
  }
  else
  {
    SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork( gdata );
    BATTLE_BOX_SAVE* box = BATTLE_BOX_SAVE_GetBattleBoxSave( sv );
    work->BoxParty = BATTLE_BOX_SAVE_MakePokeParty( box, HEAPID_APP_CONTROL );
    work->Party = work->BoxParty; //バトルボックス
  }
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
          evt_wk->ListType, PL_MODE_BATTLE_EXAMINATION, evt_wk->Reg, evt_wk->Party, evt_wk->SelAry,
          &evt_wk->SelectPos, &evt_wk->RetType, evt_wk->HouseWorkPtr->Party );
      GMEVENT_CallEvent(event, call_event);
    }
    *seq = 1;
		break;
  case 1:
    //バトルボックスパーティを解放
    if (evt_wk->BoxParty != NULL)
      GFL_HEAP_FreeMemory(evt_wk->BoxParty);
    
    //結果の分岐
    if( evt_wk->RetType != PL_RET_NORMAL ||
      evt_wk->SelectPos == PL_SEL_POS_EXIT ||
      evt_wk->SelectPos == PL_SEL_POS_EXIT2 )
    {
      *(evt_wk->RetDecide) = FALSE;          //キャンセル
    }
    else *(evt_wk->RetDecide) = TRUE;           //ポケモンを選択して、けっていした
    
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

  //トライアルハウス戦闘イベントを作成
  event = EVENT_TrialHouseTrainerBattle( gsys, fieldmap, bp );

  return event;
}

//--------------------------------------------------------------
/**
 * @brief	採点に必要な情報を集める  フィールド復帰前にコールされえるのでこのファイルに置く
 * @param	ptr       ワークポインタ
 * @param param     バトルセットアップパラメータポインタ
 * @retval	none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_AddBtlPoint( TRIAL_HOUSE_WORK_PTR ptr, BATTLE_SETUP_PARAM *prm )
{
  ptr->PointWork.TurnNum += prm->TurnNum;              //かかったターン数
  ptr->PointWork.PokeChgNum += prm->PokeChgNum;        //交代回数
  ptr->PointWork.VoidAtcNum += prm->VoidAtcNum;        //効果がない技を出した回数
  ptr->PointWork.WeakAtcNum += prm->WeakAtcNum;        //ばつぐんの技を出した回数
  ptr->PointWork.ResistAtcNum += prm->ResistAtcNum;    //いまひとつの技を出した回数
  ptr->PointWork.VoidNum += prm->VoidNum;              //効果がない技を受けた回数
  ptr->PointWork.ResistNum += prm->ResistNum;          //いまひとつの技を受けた回数
  ptr->PointWork.WinTrainerNum += prm->WinTrainerNum;  //倒したトレーナー数
  ptr->PointWork.WinPokeNum += prm->WinPokeNum;        //倒したポケモン数
  ptr->PointWork.LosePokeNum += prm->LosePokeNum;      //倒されたポケモン数
  ptr->PointWork.UseWazaNum += prm->UseWazaNum;        //使用した技の数
  //ＨＰ割合計算
  {
    int i;
    int poke_num;
    int hp, hp_max;
    int per;
    POKEPARTY* party;
    party = prm->party[BTL_CLIENT_PLAYER];
    //ポケモン数を取得
    poke_num = PokeParty_GetPokeCount(party);
    //ＨＰ格納バッファ初期化
    hp = 0;
    hp_max = 0;
    for (i=0;i<poke_num;i++)
    {
      int pokehp, pokehp_max;
      POKEMON_PARAM *pp;
      pp = PokeParty_GetMemberPointer(party, i); 
      //残りＨＰ取得
      pokehp = PP_Get( pp, ID_PARA_hp, NULL);
      //最大ＨＰ取得
      pokehp_max = PP_Get( pp, ID_PARA_hpmax, NULL);
      //足しこみ
      hp += pokehp;
      hp_max += pokehp_max;
    }
    per = (hp * 100) / hp_max; //残りＨＰパーセント
    ptr->PointWork.RestHpPer += per;      //５戦あるので足しこんでいくと最大で500％になる
    NOZOMU_Printf( "per = %d, total_per = %d\n",per, ptr->PointWork.RestHpPer );
  }
}

