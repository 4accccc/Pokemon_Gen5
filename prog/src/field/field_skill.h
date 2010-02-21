//======================================================================
/**
 * @file  field_skill.h
 * @brief  フィールド技処理（秘伝技など）
 * @author  Hiroyuki Nakamura
 * @date  2005.12.01
 */
//======================================================================
#pragma once

#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/playerwork.h"

#include "field/fieldmap_proc.h"
#include "fldmmdl.h"
#include "map_attr.h"

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
/// FLDSKILL_IDX 技インデックス
//--------------------------------------------------------------
typedef enum
{
  FLDSKILL_IDX_IAIGIRI = 0,    // いあいぎり
  FLDSKILL_IDX_NAMINORI,    // なみのり
  FLDSKILL_IDX_TAKINOBORI,    // たきのぼり
  FLDSKILL_IDX_KAIRIKI,    // かいりき
  FLDSKILL_IDX_SORAWOTOBU,    // そらをとぶ
  FLDSKILL_IDX_KIRIBARAI,    // きりばらい
  FLDSKILL_IDX_IWAKUDAKI,    // いわくだき
  FLDSKILL_IDX_ROCKCLIMB,    // ロッククライム
  FLDSKILL_IDX_FLASH,    // フラッシュ
  FLDSKILL_IDX_TELEPORT,    // テレポート
  FLDSKILL_IDX_ANAWOHORU, // あなをほる
  FLDSKILL_IDX_AMAIKAORI,    // あまいかおり
  FLDSKILL_IDX_OSYABERI,    // おしゃべり
  FLDSKILL_IDX_DIVING,      //ダイビング
  FLDSKILL_IDX_MAX, //最大
}FLDSKILL_IDX;

//--------------------------------------------------------------
/// FLDSKILL_PRM 技使用関数取得パラメータ
//--------------------------------------------------------------
typedef enum
{
  FLDSKILL_PRM_USEFUNC = 0,  //使用関数
  FLDSKILL_PRM_CHECKFUNC  //チェック関数
}FIEDLSKILL_PRM;

//--------------------------------------------------------------
/// FLDSKILL_RET 使用チェックの戻り値
//--------------------------------------------------------------
typedef enum
{
  FLDSKILL_RET_USE_OK = 0,  // 使用可能
  FLDSKILL_RET_USE_NG,    // 使用不可（ここでは使えません）
  FLDSKILL_RET_NO_BADGE,    // 使用不可・バッジなし
  FLDSKILL_RET_COMPANION,    // 使用不可・連れ歩き
  FLDSKILL_RET_PLAYER_SWIM,    // 使用不可・なみのり中
}FLDSKILL_RET;

//======================================================================
//  struct
//======================================================================
///技使用ワークへの不完全型ポインタ
typedef struct _TAG_FLDSKILL_USE_HEADER FLDSKILL_USE_HEADER;

///技使用チェックワークへの不完全型ポインタ
typedef struct _TAG_FLDSKILL_CHECK_WORK FLDSKILL_CHECK_WORK;

///技使用チェック関数
typedef FLDSKILL_RET (*FLDSKILL_CHECK_FUNC)(const FLDSKILL_CHECK_WORK*);

///技使用関数
typedef GMEVENT* (*FLDSKILL_USE_FUNC)(const FLDSKILL_USE_HEADER*,const FLDSKILL_CHECK_WORK*);

//--------------------------------------------------------------
/// FLDSKILL_USE_WORK 技使用ヘッダー
//--------------------------------------------------------------
struct _TAG_FLDSKILL_USE_HEADER
{
  u16 poke_pos; //技を使用する手持ちポケモン位置番号
  u16 use_wazano; //使用する技番号
  u32 zoneID;
  u32 GridX;
  u32 GridY;
  u32 GridZ;
};

//--------------------------------------------------------------
/// FLDSKILL_CHECK_WORK 技使用チェックワーク
//--------------------------------------------------------------
struct _TAG_FLDSKILL_CHECK_WORK
{
  u16 zone_id;
  u16 enable_skill;
  PLAYER_MOVE_FORM moveform;
  GAMESYS_WORK *gsys;
  MMDL *front_mmdl;
  FIELDMAP_WORK *fieldmap;
};

//======================================================================
//  proto
//======================================================================
//extern FLDSKILL_CHECK_FUNC FLDSKILL_GetCheckFunc( FLDSKILL_IDX );
//extern FLDSKILL_USE_FUNC FLDSKILL_GetUseFunc( FLDSKILL_IDX idx );
extern void FLDSKILL_InitCheckWork(
    FIELDMAP_WORK *fieldmap, FLDSKILL_CHECK_WORK *scwk );
extern FLDSKILL_RET FLDSKILL_CheckUseSkill(
    FLDSKILL_IDX idx, FLDSKILL_CHECK_WORK *scwk );
extern void FLDSKILL_InitUseHeader( FLDSKILL_USE_HEADER *head,
    u16 poke_pos, u16 use_wazano,
    u32 zoneID, u32 inGridX, u32 inGridY, u32 inGridZ);
extern GMEVENT * FLDSKILL_UseSkill( FLDSKILL_IDX idx,
    const FLDSKILL_USE_HEADER *head, const FLDSKILL_CHECK_WORK *scwk );
