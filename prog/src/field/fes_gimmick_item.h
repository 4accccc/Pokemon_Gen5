//===============================================================================
/*
 *  @file   fes_gimmick_item.h
 *  @brief  フェスミッション アイテムミッションギミック
 *  @author Miyuki Iwasawa
 *  @date   11.08.12
 */
//===============================================================================

#include "field/eventdata_local.h"
#include "field/eventdata_type.h"

#define FES_GIMMICK_ITEM_DATA_MAX (20) ///<1ゾーンでmax20箇所候補にできる

#define FES_GIMMICK_ITEM_POINT_TOWN_MIN (0)   ///<ポイント抽選数下限 街
#define FES_GIMMICK_ITEM_POINT_TOWN_MAX (4)   ///<ポイント抽選数上限 街
#define FES_GIMMICK_ITEM_POINT_FIELD_MIN (2)  ///<ポイント抽選数下限 ダンジョン・道路
#define FES_GIMMICK_ITEM_POINT_FIELD_MAX (8)  ///<ポイント抽選数上限 ダンジョン・道路

#define FES_GIMMICK_ITEM_POINT_MAX (FES_GIMMICK_ITEM_POINT_FIELD_MAX) ///<1ゾーンでmax8箇所抽選される

#define FES_GIMMCIK_ITEM_FLAG_WORK_NUM  (3)   ///<過去3ゾーン分確保

//フェスミッションアイテムギミックフラグ
typedef struct _FES_GIMMICK_ITEM_FLAG{
  u16 zone_id;
  u8  flag[FES_GIMMICK_ITEM_POINT_MAX];
}FES_GIMMICK_ITEM_FLAG;

//フェスミッションアイテムギミックフラグ領域
typedef struct _FES_GIMMICK_ITEM_FLAG_WORK{
  int tbl_num;
  FES_GIMMICK_ITEM_FLAG tbl[FES_GIMMCIK_ITEM_FLAG_WORK_NUM];
}FES_GIMMICK_ITEM_FLAG_WORK;

//フェスアイテムミッションギミックワーク
typedef struct _FES_GIMMICK_ITEM_WORK{
  u16 zone_id; ///<現在のゾーンID

  u8  data_max;  ///<ゾーンに登録されたデータmax
  u8  point_max; ///<抽選ポイント数
  POS_EVENT_DATA  data[FES_GIMMICK_ITEM_DATA_MAX];  ///<ポジション候補データテーブル
  u8  idx[FES_GIMMICK_ITEM_POINT_MAX];  ///<抽選されたデータの参照indexテーブル
  u16 item[FES_GIMMICK_ITEM_POINT_MAX]; ///<抽選されたアイテムテーブル

  //取得済みログデータバッファへのポインタ
  FES_GIMMICK_ITEM_FLAG_WORK* flag_wk;

}FES_GIMMICK_ITEM_WORK;

extern void FES_GIMMICK_ITEM_ZoneChangeSetup( FES_GIMMICK* sys_wk, u16 zone_id );
extern void FES_GIMMICK_ITEM_FieldRecoverSetup( FES_GIMMICK* sys_wk );
extern void FES_GIMMICK_ITEM_Delete( FES_GIMMICK* sys_wk );

extern GMEVENT* FES_GIMMICK_ITEM_CheckEventForGrid( FES_GIMMICK* sys_wk, const VecFx32* cp_pos );
extern GMEVENT* FES_GIMMICK_ITEM_CheckEventForRail( FES_GIMMICK* sys_wk,  const RAIL_LOCATION* cp_location );

