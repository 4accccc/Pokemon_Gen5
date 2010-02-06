//==============================================================================
/**
 * @file    debug_beacon.h
 * @brief   デバッグ用のビーコン情報作成
 * @author  matsuda
 * @date    2010.02.06(土)
 */
//==============================================================================
#pragma once
#ifdef PM_DEBUG

#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"


///デバッグ用行動データ
typedef struct{
  u16 action_no;            ///<行動No
  u16 param[12];     ///<パラメータ(行動No毎に異なる)
}DEBUG_ACTION_DATA;

//==================================================================
/**
 * @brief   デバッグ用の仮想人物データ構造体
 */
//==================================================================
typedef struct{
  u8 nation;                               ///<国コード
  u8 area;                                 ///<地域コード
  
  u32 suretigai_count;                  ///<すれ違い人数
  u8 g_power_id;                        ///<発動しているGパワーID(GPOWER_ID_xxx)
  u8 pm_version;                        ///<PM_VERSION

  u16 zone_id;                           ///<現在地
  u8 sex;                               ///<性別
  u8 language;                          ///<PM_LANG

  u8 townmap_tbl_index;                 ///<タウンマップでの座標テーブルIndex
  u8 trainer_view;                      ///<ユニオンルームでの見た目(０～１５）
  u8 research_team_rank;                ///<調査隊ランク(RESEARCH_TEAM_RANK)
  u8 favorite_color_index;              ///<本体情報の色(Index) ０～１５
  u32 thanks_recv_count;                 ///<お礼を受けた回数
  
  u16 trainer_id;                          ///<トレーナーID
  
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];  ///<トレーナー名

  STRCODE self_introduction[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN + EOM_SIZE]; ///<自己紹介メッセージ
  
  PMS_DATA pmsdata;                      ///<自己紹介簡易会話

  DEBUG_ACTION_DATA action_data;     ///<デフォルトの行動データ
}DEBUG_GAMEBEACON_INFO;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void DebugSendBuf_BeaconSet(const DEBUG_GAMEBEACON_INFO *debuginfo);
extern void DebugRecvBuf_BeaconSet(const DEBUG_GAMEBEACON_INFO *debuginfo, const DEBUG_ACTION_DATA *debugaction);

#endif  //PM_DEBUG
