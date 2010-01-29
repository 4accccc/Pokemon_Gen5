//============================================================================================
/**
 * @file    itemuse_proc.c
 * @brief   フィールドに関連したアイテムの使用処理
 * @author  k.ohno
 * @date    09.08.04
 */
//============================================================================================
#include "gflib.h"
#include "itemuse_event.h"
#include "field/fieldmap.h"
#include "event_mapchange.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fishing.h"
#include "fskill_amaikaori.h"
#include "field/zonedata.h"
#include "app/itemuse.h"

#define ITEMCHECK_NONE  (0xFF)

typedef struct _FLD_ITEM_FUNCTION{
  ItemUseEventFunc useFunc;
  u8  checkID;  //ITEMCHECK_ENABLE型、必要ない時はITEMCHECK_NONE
}FLD_ITEM_FUNCTION;

//////////////////////////////////////////////////
///プロトタイプ
//////////////////////////////////////////////////
static BOOL itemcheck_Cycle( GAMEDATA* gdata, FIELDMAP_WORK* field_wk, PLAYER_WORK* playerWork );
static BOOL itemcheck_Ananukenohimo( GAMEDATA* gdata, FIELDMAP_WORK* field_wk, PLAYER_WORK* playerWork );
static BOOL itemcheck_Turizao( GAMEDATA* gdata, FIELDMAP_WORK* field_wk, PLAYER_WORK* playerWork );

//=============================================================================
/**
 *  共通
 */
//=============================================================================
//-------------------------------------
/// フィールドアイテム使用共通呼び出し用テーブル
//=====================================
static FLD_ITEM_FUNCTION const DATA_FldItemFunction[ EVENT_ITEMUSE_CALL_MAX ] =
{ 
  { //自転車
    &EVENT_CycleUse,
    ITEMCHECK_CYCLE,
  },
  { //パレスへGo
    &EVENT_PalaceJumpUse,
    ITEMCHECK_NONE,
  },
  { //穴抜けの紐
    &EVENT_ChangeMapByAnanukenohimo,
    ITEMCHECK_ANANUKENOHIMO,
  },
  { //甘い蜜
    &EVENT_FieldAmaimitu,
    ITEMCHECK_NONE,
  },
  { //釣竿
    &EVENT_FieldFishing,
    ITEMCHECK_TURIZAO,
  }
};

//-------------------------------------
/// フィールドアイテム使用チェック関数テーブル
//=====================================
static ItemUseCheckFunc const DATA_ItemUseCheckFunc[ITEMCHECK_MAX] = {
  itemcheck_Cycle, // 自転車(乗っていない時はTRUEで乗れる、乗っている時はTRUEで降りれる)
  NULL, // タウンマップ
  NULL, // ともだち手帳
  itemcheck_Ananukenohimo, // あなぬけのヒモ
  NULL, // あまいミツ
  itemcheck_Turizao, // つりざお
  NULL, // バトルレコーダー
  NULL, // メール
  NULL, // スプレー
};


//----------------------------------------------------------------------------
/**
 *  @brief  バッグ画面に引き渡すアイテム使用チェック
 *
 *  @param  GMEVENT *event  イベント
 *  @param  *seq            シーケンス
 *  @param  *wk_adrs        ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
void ITEMUSE_InitCheckWork( ITEMCHECK_WORK* icwk, GAMESYS_WORK* gsys, FIELDMAP_WORK* field_wk )
{
  int i;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  PLAYER_WORK *playerWork = GAMEDATA_GetMyPlayerWork( gdata );

  MI_CpuClear8( icwk, sizeof(ITEMCHECK_WORK) );

  for( i = 0;i < ITEMCHECK_MAX; i++){
    ITEMUSE_SetItemUseCheck( icwk, i, ITEMUSE_UseCheckCall( i, gsys, field_wk ));
  }
  //自機フォルムチェック
  icwk->PlayerForm = PLAYERWORK_GetMoveForm( playerWork ); 
  // ( PlayerForm == PLAYER_MOVE_FORM_CYCLE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  共通アイテム使用チェック関数呼び出し
 *
 *  @param  EVENT_ITEMUSE_CALL_TYPE type  呼び出すチェックタイプ
 *  @param  *gsys       ゲームシステム
 *  @param  *field_wk   フィールド
 *
 *  @return イベント
 */
//-----------------------------------------------------------------------------
BOOL ITEMUSE_UseCheckCall( ITEMCHECK_ENABLE type, GAMESYS_WORK *gsys, FIELDMAP_WORK *field_wk )
{ 
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  PLAYER_WORK *playerWork = GAMEDATA_GetMyPlayerWork( gdata );

  GF_ASSERT( type < ITEMCHECK_MAX );
  
  if( DATA_ItemUseCheckFunc[type] == NULL ){
    return TRUE;  //無条件で使用していい
  }
  return (DATA_ItemUseCheckFunc[ type ])( gdata, field_wk, playerWork );
}

//----------------------------------------------------------------------------
/**
 *  @brief  共通アイテム使用イベント呼び出し
 *
 *  @param  EVENT_ITEMUSE_CALL_TYPE type  呼び出すアイテム
 *  @param  *gsys       ゲームシステム
 *  @param  *field_wk   フィールド
 *
 *  @return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_FieldItemUse( EVENT_ITEMUSE_CALL_TYPE type, GAMESYS_WORK *gsys, FIELDMAP_WORK *field_wk )
{ 
  GF_ASSERT( type < EVENT_ITEMUSE_CALL_MAX );
  OS_Printf("event type = %d\n", type);
  return (DATA_FldItemFunction[ type ].useFunc)( field_wk, gsys );
}

//----------------------------------------------------------------------------
/**
 *  @brief  共通アイテム使用チェック呼び出し
 *
 *  @param  EVENT_ITEMUSE_CALL_TYPE type  呼び出すアイテム
 *  @param  *gsys       ゲームシステム
 *  @param  *field_wk   フィールド
 *
 *  @retval TRUE：使用可
 *  @retval FALSE：使用不可
 */
//-----------------------------------------------------------------------------
BOOL FieldItemUseCheck( EVENT_ITEMUSE_CALL_TYPE type, GAMESYS_WORK *gsys, FIELDMAP_WORK *field_wk )
{ 
  GF_ASSERT( type < EVENT_ITEMUSE_CALL_MAX );

  if( DATA_FldItemFunction[ type ].checkID == ITEMCHECK_NONE ){
    return TRUE;  //使用制限無し
  }
  return ITEMUSE_UseCheckCall( DATA_FldItemFunction[type].checkID, gsys, field_wk );
}

//-----------------------------------------------------------------------------
//=============================================================================
///個別使用チェック関数
//=============================================================================
//-----------------------------------------------------------------------------

/*
 *  @brief  自転車使用チェック
 */
static BOOL itemcheck_Cycle( GAMEDATA* gdata, FIELDMAP_WORK* field_wk, PLAYER_WORK* playerWork )
{
//  PLAYER_WORK *playerWork = GAMEDATA_GetMyPlayerWork( gdata );
  u16 zone_id = PLAYERWORK_getZoneID( playerWork );
  u8 form = PLAYERWORK_GetMoveForm( playerWork ); 

  if( form == PLAYER_MOVE_FORM_CYCLE ){
    return TRUE;  //降りれる。今の所サイクリングロードがないので降りられない場所はない
  }
  //ゾーンチェック
  if( ZONEDATA_BicycleEnable( zone_id ) == FALSE ){
    return FALSE;
  }

  //フォルムチェック
  if( form != PLAYER_MOVE_FORM_NORMAL ){
    return FALSE;
  }
  return TRUE;  //乗れる
}

/*
 *  @brief  穴抜けの紐チェック
 */
static BOOL itemcheck_Ananukenohimo( GAMEDATA* gdata, FIELDMAP_WORK* field_wk, PLAYER_WORK* playerWork )
{
//  PLAYER_WORK *playerWork = GAMEDATA_GetMyPlayerWork( gdata );
  u16 zone_id = PLAYERWORK_getZoneID( playerWork );

  if(!ZONEDATA_EscapeEnable( zone_id )){
    return FALSE;
  }
  return TRUE;
}

/*
 *  @brief  釣竿使用チェック
 */
static BOOL itemcheck_Turizao( GAMEDATA* gdata, FIELDMAP_WORK* field_wk, PLAYER_WORK* playerWork )
{
  //釣竿チェック
  return FieldFishingCheckPos( gdata, field_wk, NULL );
}



//=============================================================================
/**
 *  自転車呼び出し
 */
//=============================================================================
typedef struct{
  GAMESYS_WORK *gameSys;
} CYCLEUSE_STRUCT;

//------------------------------------------------------------------------------
/**
 * @brief   自転車を使う
 * @retval  none
 */
//------------------------------------------------------------------------------

static GMEVENT_RESULT CycleEvent(GMEVENT * event, int * seq, void *work)
{
  CYCLEUSE_STRUCT* pCy = work;
  FIELDMAP_SetPlayerItemCycle( GAMESYSTEM_GetFieldMapWork( pCy->gameSys ) );
  return GMEVENT_RES_FINISH;

}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_CycleUse(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys)
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, CycleEvent, sizeof(CYCLEUSE_STRUCT));
  CYCLEUSE_STRUCT * pCy = GMEVENT_GetEventWork(event);
  pCy->gameSys = gsys;
  return event;
}


//------------------------------------------------------------------------------
/**
 * @brief   パレスにジャンプする
 * @retval  none
 */
//------------------------------------------------------------------------------


GMEVENT * EVENT_PalaceJumpUse(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys)
{
  VecFx32 pos;
  ZONEID jump_zone;
  
  if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
    pos.x = 184 << FX32_SHIFT;
    pos.y = 0;
    pos.z = 184 << FX32_SHIFT;
    jump_zone = ZONE_ID_UNION;   //こちらに飛ぶのはデバッグです
  }
  else{
    pos.x = 760 << FX32_SHIFT;
    pos.y = 0;
    pos.z = 234 << FX32_SHIFT;
    jump_zone = ZONE_ID_PALACE01;
  }
  return EVENT_ChangeMapPos(gsys, fieldWork, jump_zone, &pos, 0, FALSE);
}


