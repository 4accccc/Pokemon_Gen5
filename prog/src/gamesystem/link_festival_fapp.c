//==============================================================================
/**
 * @file    link_festival_fapp.c
 * @brief   リンクフェスティバル 詳細画面アプリでしか呼び出さない関数をFMISSION_APPオーバーレイに追い出し
 * @author  iwasawa
 * @date    2011.09.25
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/rtc_tool.h"
#include "field/gpower_id.h"
#include "gamesystem/g_power.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_comm.h"
#include "waza_tool/wazano_def.h"
#include "arc_def.h"
#include "power.naix"
#include "poke_tool/status_rcv.h"

#include "gamesystem/link_festival.h"
#include "gamesystem/link_festival_fapp.h"
#include "gamesystem/link_festival_local.h"
#include "savedata/fmission_save.h"
#include "savedata/medalbox.h"
#include "app/mdlid_def.h"  // for MDLID_
#include "item/itemsym.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"
#include "field/script_def.h"

typedef struct BONUS_CALC_COEFFICIENT{
  int   num; //閾値
  fx32  coe;  //係数
}BONUS_CALC_COEFFICIENT;

static const BONUS_CALC_COEFFICIENT DATA_BonusCalcCoefficient[] = {
  { 100, FX32_CONST(0.05) },   //101以上は0.05
  { 30,  FX32_CONST(0.1) },   //31-100は0.1
  { 20,  FX32_CONST(0.2) },   //21-30は0.2
  { 10,  FX32_CONST(0.5) },   //11-20は0.5
  { 0,   FX32_CONST(1.0) },  //0-10は1.0
};

static void linkfes_SetMissionResultMedal( LINK_FESTIVAL* wk, LINKFEST_MISSION_RESULT* pl, MEDALBOX* medalbox, FMISSION_SAVE* f_save );


//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッション結果を取得＆レコードをセーブに反映
 *
 * @param fest_sys  LINK_FESTIVAL*
 * @param pl  LINKFEST_MISSION_RESULT構造体型へのポインタ
 *
 *  ミッション達成回数などをインクリメントするので複数回呼び出さないこと
 */
//---------------------------------------------------------
void LINK_FESTIVAL_GetMissionResult( LINK_FESTIVAL* wk, LINKFEST_MISSION_RESULT* pl )
{
  BOOL ret;
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( wk->gdata );
  FMISSION_SAVE* f_save = SAVEDATA_GetFMissionSave( save );
  EVENTWORK* ev = GAMEDATA_GetEventWork( wk->gdata );
  MEDALBOX * medalbox = SaveData_GetMedalBox( save );

  MI_CpuClear8( pl, sizeof(LINKFEST_MISSION_RESULT));

  //ミッションプレイ状態解除(これ以上スコアの更新は掛からない)
  LINK_FESTIVAL_MissionLiveStatusSet( wk, FMISSION_LIVE_STATUS_RESULT );

  //結果確認
  pl->clear_f = LINK_FESTIVAL_IsMissionClear( wk, &pl->score );

  if( wk->m_beacon->distribution_f == FALSE ){  //配信ミッションでは行わない処理

    if( wk->m_beacon->m_id == FMISSION_ID_ITEM_TUTORIAL_01 ){
      if( !EVENTWORK_CheckEventFlag( ev, SYS_FLAG_FES_TUTORIAL_CLEAR )){
        EVENTWORK_SetEventFlag( ev, SYS_FLAG_FES_TUTORIAL_CLEAR );
        //イベント起動用ワークを設定
        *( EVENTWORK_GetEventWorkAdrs( ev, WK_SYS_FES_TUTORIAL ) ) = SCR_FES_TUTORIAL_MISSION_FAILED+pl->clear_f;
      }
    }

    //時間依存型の場合、失敗でもハイスコアを更新する
    pl->new_record_score_f = FMISSION_SAVE_UpdateMissionStatusHighScore( f_save, wk->m_beacon->m_id, pl->score );
  
    //自分のハイスコアを記録する
    pl->new_record_my_score_f = FMISSION_SAVE_UpdateMissionStatusHighScoreMine( f_save, wk->m_beacon->m_id, wk->mission.my_score );
  }

  { //参加者数記録
    int join_num = LINK_FESTIVAL_GetMissionJoinNum( wk );
  
    //最高参加者数を記録する
    FMISSION_SAVE_UpdateMissionMaxJoinNum( f_save, LINK_FESTIVAL_GetMissionJoinNum( wk ) );

    //参加者数が規定値を越えた際にオープンするメダルチェック
    if( join_num >= 10 ){
      MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_JOIN_NUM_LV1 );
      if( join_num >= 30 ){
        MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_JOIN_NUM_LV2 );
      }
    }
  
    //トップスコアを取った回数を記録する(参加者が一人の時は更新しない)
    if( join_num > 1 && LINK_FESTIVAL_TopScoreIsMine( wk ) ){
      FMISSION_SAVE_IncMissionTopScoreNum( f_save );
    }
  }
 
  //残り時間
  pl->life = FEST_MISSION_GetLifeAtTimeOfFinish( &wk->mission, pl->score );

  if( !pl->clear_f ){
    return; //ミッション失敗
  }
  //以下ミッション成功時のみの処理

  FMISSION_SAVE_IncMissionClearNum( f_save ); //ミッション達成回数インクリメント


  if( wk->m_beacon->distribution_f == FALSE ){  //配信ミッションでは行わない処理
    //ミッションレベルアップ
    FMISSION_SAVE_SetMissionStatusClearLevel( f_save, wk->m_beacon->m_id, wk->m_beacon->m_level+FMISSION_CLEAR_LEVEL_1 );

    //クリアフラグが設定されていたら、設定されたフラグをOnにする
    if( wk->m_arc->clear_flag_id != 0 ){
      EVENTWORK_SetEventFlag( ev, wk->m_arc->clear_flag_id );
    }
  }
  //ご褒美
  {
    BOOL sp_bonus = FALSE;
    u16 my_score = wk->mission.my_score;
    u8 join_num = LINK_FESTIVAL_GetMissionJoinNum( wk );

    //基本のご褒美
    pl->item = wk->m_beacon->item;
    pl->item_num = wk->m_beacon->pay;

    switch( wk->m_beacon->pay_bonus ){
    case FMISSION_PAY_BONUS_TIME: //タイムボーナス(残り時間に依存するボーナス)
      {  //MyScore + { (残り秒/15)*MissionLevel/3 }
        fx32 life = FX_Div( FX32_CONST(pl->life), FX32_CONST(15));
        fx32 val = FX_Mul( life, FX32_CONST(wk->m_beacon->m_level+1) );  //残り時間にミッションレベルを掛ける
        pl->bonus_num = FX_Whole( FX_Div( val, FX32_CONST(3)) ); //固定値3で割って小数点を切捨て
        pl->bonus_num += my_score; //最後にマイスコア分足す
      }
      break;
    case FMISSION_PAY_BONUS_SCORE: //スコアボーナス(トータルスコアに依存するボーナス)
      { //【 MyScore + { (TotalScore-MyScore)の係数特殊計算} 】 * MissionLevel / 3 
        int i, other;
        fx32 total = FX32_CONST(my_score);
        
        other = pl->score-my_score; //自分以外の人が稼いだスコア
        
        for( i = 0; i < NELEMS(DATA_BonusCalcCoefficient);i++ ){
          int diff;
          const BONUS_CALC_COEFFICIENT* p_coe = &DATA_BonusCalcCoefficient[i];

          if( other <= p_coe->num ){
            continue;
          }
          diff = other - p_coe->num; 
          total += FX_Mul( FX32_CONST(diff), p_coe->coe );
          other -= diff;  //消費した分のスコアを減算
        }
        total = FX_Mul( total, FX32_CONST( wk->m_beacon->m_level+1)); //ミッションレベルを掛ける
        pl->bonus_num = FX_Whole( FX_Div( total, FX32_CONST(3) ) );  //最後に固定値3で割って小数点切捨て
      }

      //スペシャル報酬 level3ミッション以上&&スコアが目標値の倍以上なら
      sp_bonus = ( (wk->m_beacon->m_level >= FMISSION_LEVEL_3) && (pl->score >= (wk->m_beacon->goal*2)));
      break;
    }
    if( pl->bonus_num > FMISSION_PAY_NUM_BONUS_MAX ){
      pl->bonus_num = FMISSION_PAY_NUM_BONUS_MAX;  //ボーナスには上限がある
    }
    if( wk->m_beacon->sp_item != ITEM_DUMMY_DATA &&
        wk->m_beacon->sp_pay > 0 &&
        sp_bonus ){
      pl->sp_item = wk->m_beacon->sp_item;
      pl->sp_item_num = wk->m_beacon->sp_pay;
    }
  }

  //ハイリンクの経験値
  {
    u16 up_level,now_level;
    int step, is_up;
    OCCUPY_INFO* occupy_sv = GAMEDATA_GetMyOccupyInfo( wk->gdata );
    u32 exp = FMISSION_SAVE_GetHilinkLevelExp( f_save, wk->m_beacon->version_idx );

    exp += pl->score;
    if( exp > FMISSION_SV_HILINK_LEVEL_EXP_MAX ){
      exp = FMISSION_SV_HILINK_LEVEL_EXP_MAX; //上限超えていたら、上限で切り捨て
    }
    
    if( wk->m_beacon->version_idx == FMISSION_VERSION_WHITE ){
      now_level = OccupyInfo_GetWhiteLevel( occupy_sv );
    }else{
      now_level = OccupyInfo_GetBlackLevel( occupy_sv );
    }
    
    IWASAWA_Printf("FMResult exp = %d, n_level = %d\n", exp, now_level );
    up_level = 0;
    do{
      //閾値は現Lv*5(上限50) ミッション未クリア時のレベルは0～なので+1
      step = (now_level+1)*5;
      if( step > 50 ){
        step = 50;
      }
      is_up = exp/step;
      
      if( is_up > 0 ){
        up_level += 1;
        now_level += 1;
        exp -= step;
      }
    }while( is_up );
   
    if( up_level >= FMISSION_SV_HILINK_LEVEL_UP_ONCE_MAX ){
      //一度にあがるパレスレベルは10まで
      up_level = FMISSION_SV_HILINK_LEVEL_UP_ONCE_MAX;
      exp = 0;  //繰り越し分は0リセット
    }
    //パレスレベルを上げる
    pl->hilink_version = wk->m_beacon->version_idx;
    if( wk->m_beacon->version_idx == FMISSION_VERSION_WHITE ){
      pl->hilink_level_up = OccupyInfo_LevelUpWhite( occupy_sv, up_level );
    }else{
      pl->hilink_level_up = OccupyInfo_LevelUpBlack( occupy_sv, up_level );
    }
    IWASAWA_Printf(" -> up = %d, exp = %d\n", pl->hilink_level_up, exp );
    
    //余りの経験値をセーブに書き戻し
    FMISSION_SAVE_SetHilinkLevelExp( f_save, wk->m_beacon->version_idx, exp );
  }

  //メダルチェック
  linkfes_SetMissionResultMedal( wk, pl, medalbox, f_save );
}

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッション結果からメダルの反映
 *
 * @param fest_sys  LINK_FESTIVAL*
 * @param pl  LINKFEST_MISSION_RESULT構造体型へのポインタ
 *
 *  ミッション達成回数などをインクリメントするので複数回呼び出さないこと
 */
//---------------------------------------------------------
static void linkfes_SetMissionResultMedal( LINK_FESTIVAL* wk, LINKFEST_MISSION_RESULT* pl, MEDALBOX* medalbox, FMISSION_SAVE* f_save )
{
  { //フェスのトータルスコアが規定値を越えた際にオープンするメダルチェック
    if( pl->score >= 100 ){
      MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_HIGHSCORE_LV1 );
      if( pl->score >= 1000 ){
        MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_HIGHSCORE_LV2 );
      }
    }
  }
  if( !pl->clear_f ){
    return;
  }
  //クリア時に確認
  {
    u32 clear_num = FMISSION_SAVE_GetMissionClearNum( f_save );
   
    if( clear_num >= 30 ){
      MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_CLEAR_LV1 );
      if( clear_num >= 100 ){
        MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_CLEAR_LV2 );
      }
    }
  }
  //自分のバージョンで開催可能な全ミッションをクリアしたかどうかのチェック
  {
    int i;
    FMISSION_HOST_TYPE egnore;

    if( GET_VERSION() == VERSION_B2 ){
      egnore = FMISSION_HOST_WHITE;
    }else{
      egnore = FMISSION_HOST_BLACK;
    }
    for( i = 0;i < FMISSION_ID_MAX; i++ ){
      if( wk->p_arc_data[i].host >= FMISSION_HOST_COMM ||
          wk->p_arc_data[i].host == egnore ){
        continue; //自分では開催できないミッションは除く
      }
      if( FMISSION_SAVE_GetMissionStatusClearLevel( f_save, i ) == FMISSION_CLEAR_LEVEL_0 ){
        break;  //クリアしてない
      }
    }
    //ループ完了していたら、開催可能な全ミッションクリアしてる
    if( i == FMISSION_ID_MAX ){
      MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_CLEAR_ALL );
    }
  }
}


