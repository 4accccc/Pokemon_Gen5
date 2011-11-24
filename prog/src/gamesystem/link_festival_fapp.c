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

#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"
#include "field/script_def.h"

static void linkfes_SetMissionResultMedal( LINK_FESTIVAL* wk, LINKFEST_MISSION_RESULT* pl, SAVE_CONTROL_WORK* save, FMISSION_SAVE* f_save );


//---------------------------------------------------------
/*
 * @brief リンクフェスティバル　ミッション結果を取得＆レコードをセーブに反映
 *
 * @param fest_sys  LINK_FESTIVAL*
 * @param pl  LINKFEST_MISSION_RESULT構造体型へのポインタ
 * @param mission_time_sec  秒単位のミッション制限時間(FMISSION_ARC_PARAM->m_timeを渡してください)
 *
 *  ミッション達成回数などをインクリメントするので複数回呼び出さないこと
 */
//---------------------------------------------------------
void LINK_FESTIVAL_GetMissionResult( LINK_FESTIVAL* wk, LINKFEST_MISSION_RESULT* pl, u16 mission_time_sec )
{
  BOOL ret;
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( wk->gdata );
  FMISSION_SAVE* f_save = SAVEDATA_GetFMissionSave( save );

  MI_CpuClear8( pl, sizeof(LINKFEST_MISSION_RESULT));

  pl->clear_f = LINK_FESTIVAL_IsMissionClear( wk, &pl->score );

  if( wk->m_beacon->m_id == FMISSION_ID_ITEM_TUTORIAL_01 ){
    EVENTWORK* ev = GAMEDATA_GetEventWork( wk->gdata );
    if( !EVENTWORK_CheckEventFlag( ev, SYS_FLAG_FES_TUTORIAL_CLEAR )){
      EVENTWORK_SetEventFlag( ev, SYS_FLAG_FES_TUTORIAL_CLEAR );
      //イベント起動用ワークを設定
      *( EVENTWORK_GetEventWorkAdrs( ev, WK_SYS_FES_TUTORIAL ) ) = SCR_FES_TUTORIAL_MISSION_FAILED+pl->clear_f;
    }
  }

  //時間依存型の場合、失敗でもハイスコアを更新する
  pl->new_record_score_f = FMISSION_SAVE_UpdateMissionStatusHighScore( f_save, wk->m_beacon->m_id, pl->score );
  
  //自分のハイスコアを記録する
  FMISSION_SAVE_UpdateMissionStatusHighScoreMine( f_save, wk->m_beacon->m_id, wk->mission.my_score );
  //最高参加者数を記録する
  FMISSION_SAVE_UpdateMissionMaxJoinNum( f_save, LINK_FESTIVAL_GetMissionJoinNum( wk ) );
  
  if( !pl->clear_f ){
    return; //ミッション失敗
  }
  //以下ミッション成功時のみの処理

  FMISSION_SAVE_IncMissionClearNum( f_save ); //ミッション達成回数インクリメント

  //トップスコアを取った回数を記録する
  if( LINK_FESTIVAL_TopScoreIsMine( wk ) ){
    FMISSION_SAVE_IncMissionTopScoreNum( f_save );
  }

  //ミッションレベルアップ
  FMISSION_SAVE_SetMissionStatusClearLevel( f_save, wk->m_beacon->m_id, wk->m_beacon->m_level+FMISSION_CLEAR_LEVEL_1 );

  //ご褒美
  {
    BOOL sp_bonus = FALSE;
    pl->item = wk->m_beacon->item;

    pl->item_num = wk->m_beacon->pay;
    switch( wk->m_beacon->pay_bonus ){
     case FMISSION_PAY_BONUS_TIME: //タイムボーナス(残り時間に依存するボーナス)
       { //(残り秒/10)*ミッションレベル
         u32 value = wk->mission.end_life/10;
         pl->bonus_num += value*(wk->m_beacon->m_level+1);
       }
       //スペシャル報酬 level3ミッション以上&&残り時間が制限時間より長ければ
       sp_bonus = ( (wk->m_beacon->m_level >= FMISSION_LEVEL_3) && (wk->mission.end_life >= mission_time_sec) );
      break;
     case FMISSION_PAY_BONUS_SCORE: //スコアボーナス(トータルスコアに依存するボーナス)
       { //(トータルスコア-目標値)/人数*ミッションレベル
         u32 score = (pl->score-wk->m_beacon->goal)/(wk->log_num+1);
         pl->bonus_num += score*(wk->m_beacon->m_level+1);
       }
       //スペシャル報酬 level3ミッション以上&&スコアが目標値の倍以上なら
       sp_bonus = ( (wk->m_beacon->m_level >= FMISSION_LEVEL_3) && (pl->score >= (wk->m_beacon->goal*2)));
       break;
    }
    if( pl->bonus_num > FMISSION_PAY_NUM_BONUS_MAX ){
      pl->bonus_num = FMISSION_PAY_NUM_BONUS_MAX;  //ボーナスには上限がある
    }
    if( sp_bonus ){
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
      //閾値は現Lv*10(上限100) ミッション未クリア時のレベルは0～なので+1
      step = (now_level+1)*10;
      if( step > 100 ){
        step = 100;
      }
      is_up = exp/step;
      
      if( is_up > 0 ){
        up_level += 1;
        now_level += 1;
        exp -= step;
      }
    }while( is_up );
    
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
  linkfes_SetMissionResultMedal( wk, pl, save, f_save );
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
static void linkfes_SetMissionResultMedal( LINK_FESTIVAL* wk, LINKFEST_MISSION_RESULT* pl, SAVE_CONTROL_WORK* save, FMISSION_SAVE* f_save )
{
  MEDALBOX * medalbox = SaveData_GetMedalBox( save );
 
  { //参加者数が規定値を越えた際にオープンするメダルチェック
    int join_num = LINK_FESTIVAL_GetMissionJoinNum( wk );

    if( join_num >= 10 ){
      MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_JOIN_NUM_LV1 );
      if( join_num >= 30 ){
        MEDALBOX_SetTitleMedalAchive( medalbox, MDLID_FES_JOIN_NUM_LV2 );
      }
    }
  }
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


