//===============================================================================
/*
 *  @file   fes_gimmick_item.h
 *  @brief  フェスミッション アイテムミッションギミック
 *  @author Miyuki Iwasawa
 *  @date   11.08.12
 */
//===============================================================================

#pragma once

#include "field/eventdata_local.h"
#include "field/eventdata_type.h"

#define FES_GIMMICK_ITEM_DATA_MAX (20) ///<1ゾーンでmax20箇所候補にできる
#define FES_GIMMICK_ITEM_POINT_MAX (8) ///<1ゾーンでmax8箇所抽選される

#define FES_GIMMICK_ITEM_FLAG_WORK_NUM  (6)   ///<過去6ゾーン分確保
#define FES_GIMMICK_ITEM_FLAG_WORK_END  (FES_GIMMICK_ITEM_FLAG_WORK_NUM-1)   ///<過去6ゾーン分確保

//フェスミッションアイテムギミックフラグ
typedef struct _FES_GIMMICK_ITEM_FLAG{
  u16 zone_id;
  u8  flag[FES_GIMMICK_ITEM_POINT_MAX];
}FES_GIMMICK_ITEM_FLAG;

//フェスミッションアイテムギミックフラグ領域
typedef struct _FES_GIMMICK_ITEM_FLAG_WORK{
  int tbl_num;
  FES_GIMMICK_ITEM_FLAG tbl[FES_GIMMICK_ITEM_FLAG_WORK_NUM];
}FES_GIMMICK_ITEM_FLAG_WORK;

//フェスアイテムミッションギミックワーク
typedef struct _FES_GIMMICK_ITEM_WORK{
  u16 zone_id; ///<現在のゾーンID

  u8  data_max;  ///<ゾーンに登録されたデータmax
  u8  point_max; ///<抽選ポイント数
  BG_TALK_DATA  data[FES_GIMMICK_ITEM_DATA_MAX];  ///<ポジション候補データテーブル
  u8  idx[FES_GIMMICK_ITEM_POINT_MAX];  ///<抽選されたデータの参照indexテーブル
  u16 item[FES_GIMMICK_ITEM_POINT_MAX]; ///<抽選されたアイテムテーブル

  //取得済みログデータバッファへのポインタ
  FES_GIMMICK_ITEM_FLAG_WORK flag_wk;

}FES_GIMMICK_ITEM_WORK;

//NOTWIFI
extern void FES_GIMMICK_ITEM_ZoneChangeSetup( FES_GIMMICK_ITEM_WORK* wk, GAMESYS_WORK* gsys, LINK_FESTIVAL* fest_sys, FEST_MISSION* fmwk, u16 zone_id );

//FIELDMAP
extern void FES_GIMMICK_ITEM_FieldRecoverSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk );
extern BOOL FES_GIMMICK_ITEM_Delete( FES_GIMMICK* sys_wk, FES_GIMMICK_ITEM_WORK* wk );

extern GMEVENT* FES_GIMMICK_ITEM_CheckEventForGrid( FES_GIMMICK* sys_wk, const VecFx32* cp_pos );
extern GMEVENT* FES_GIMMICK_ITEM_CheckEventForRail( FES_GIMMICK* sys_wk,  const RAIL_LOCATION* cp_location );
extern BOOL FES_GIMMICK_ITEM_CheckHitGridPos( FES_GIMMICK* sys_wk, u16 gx, u16 gz, fx32 height );

