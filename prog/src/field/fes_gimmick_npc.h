//===============================================================================
/*
 *  @file   fes_gimmick_npc.h
 *  @brief  フェスミッション NPCミッションギミック
 *  @author Miyuki Iwasawa
 *  @date   11.09.09
 */
//===============================================================================

#pragma once

#include "field/eventdata_local.h"
#include "field/eventdata_type.h"

#define FES_GIMMICK_NPC_DATA_MAX (10) ///<1ゾーンでmax20箇所候補にできる

#define FES_GIMMICK_NPC_POINT_LOW_MIN (0)     ///<ポイント抽選数下限 少なめ(街)
#define FES_GIMMICK_NPC_POINT_LOW_MAX (3)     ///<ポイント抽選数上限 少なめ(街)
#define FES_GIMMICK_NPC_POINT_MIDDLE_MIN (1)  ///<ポイント抽選数下限 中(道路)
#define FES_GIMMICK_NPC_POINT_MIDDLE_MAX (3)  ///<ポイント抽選数上限 中(道路)
#define FES_GIMMICK_NPC_POINT_HIGH_MIN (3)  ///<ポイント抽選数下限 中(ダンジョン)
#define FES_GIMMICK_NPC_POINT_HIGH_MAX (3)  ///<ポイント抽選数上限 中(ダンジョン)

#define FES_GIMMICK_NPC_POINT_MAX (FES_GIMMICK_NPC_POINT_HIGH_MAX) ///<1ゾーンでmax8箇所抽選される

#define FES_GIMMICK_NPC_FLAG_WORK_NUM  (3)   ///<過去3ゾーン分確保

//フェスミッションアイテムギミックフラグ
typedef struct _FES_GIMMICK_NPC_FLAG{
  u16 zone_id;
  u8  flag[FES_GIMMICK_NPC_POINT_MAX];
}FES_GIMMICK_NPC_FLAG;

//フェスミッションアイテムギミックフラグ領域
typedef struct _FES_GIMMICK_NPC_FLAG_WORK{
  int tbl_num;
  FES_GIMMICK_NPC_FLAG tbl[FES_GIMMICK_NPC_FLAG_WORK_NUM];
}FES_GIMMICK_NPC_FLAG_WORK;

//フェスアイテムミッションギミックワーク
typedef struct _FES_GIMMICK_NPC_WORK{
  u16 zone_id; ///<現在のゾーンID

  u8  data_max;  ///<ゾーンに登録されたデータmax
  u8  point_max; ///<抽選ポイント数
  BG_TALK_DATA  data[FES_GIMMICK_NPC_DATA_MAX];  ///<ポジション候補データテーブル
  u8  idx[FES_GIMMICK_NPC_POINT_MAX];  ///<抽選されたデータの参照indexテーブル
  u16 item[FES_GIMMICK_NPC_POINT_MAX]; ///<抽選されたアイテムテーブル
  MMDL* mmdl_tbl[FES_GIMMICK_NPC_POINT_MAX];

  //取得済みログデータバッファへのポインタ
  FES_GIMMICK_NPC_FLAG_WORK flag_wk;

}FES_GIMMICK_NPC_WORK;

//NOTWIFI
extern void FES_GIMMICK_NPC_ZoneChangeSetup( FES_GIMMICK_NPC_WORK* wk, GAMESYS_WORK* gsys, LINK_FESTIVAL* fest_sys, FEST_MISSION* fmwk, u16 zone_id );
extern u16 FES_GIMMICK_NPC_SUB_GetObjCode( FES_GIMMICK_NPC_WORK* wk, const FEST_MISSION* fmwk );
extern FES_GIMMICK_NPC_FLAG* FES_GIMMICK_NPC_SUB_FlagTableGet( FES_GIMMICK_NPC_WORK* wk, u16 zone_id );

//FIELDMAP
extern void FES_GIMMICK_NPC_MMdlSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );
extern void FES_GIMMICK_NPC_FieldRecoverSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );
extern void FES_GIMMICK_NPC_FlagSet( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, u16 zone_id, u16 idx );
extern void FES_GIMMICK_NPC_Delete( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );

extern GMEVENT* FES_GIMMICK_NPC_CheckEventForGrid( FES_GIMMICK* sys_wk, const VecFx32* cp_pos );
extern GMEVENT* FES_GIMMICK_NPC_CheckEventForRail( FES_GIMMICK* sys_wk,  const RAIL_LOCATION* cp_location );
extern BOOL FES_GIMMICK_NPC_CheckHitGridPos( FES_GIMMICK* sys_wk, u16 gx, u16 gz, fx32 height );

