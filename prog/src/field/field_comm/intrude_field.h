//==============================================================================
/**
 * @file    intrude_field.h
 * @brief   侵入でフィールド時に行うメインのヘッダ
 * @author  matsuda
 * @date    2009.09.03(木)
 */
//==============================================================================
#pragma once

#include "field/field_comm/intrude_types.h"


//==============================================================================
//  定数定義
//==============================================================================
///パレスマップの範囲(この座標外に出た場合、ワープさせる必要がある)
enum{
  PALACE_MAP_LEFT = 0 << FX32_SHIFT,              ///<パレスマップ左端
  PALACE_MAP_RIGHT = 3072 << FX32_SHIFT,          ///<パレスマップ右端
  PALACE_MAP_LEN = PALACE_MAP_RIGHT - PALACE_MAP_LEFT,  ///<パレスマップ幅

  PALACE_MAP_RANGE_LEFT_X = PALACE_MAP_LEFT + (32 << FX32_SHIFT),
  PALACE_MAP_RANGE_RIGHT_X = PALACE_MAP_RIGHT - (32 << FX32_SHIFT),

  PALACE_MAP_RANGE_LEN = (2056-1016) << FX32_SHIFT,     ///<パレスマップのX長
};

//==============================================================================
//  外部関数宣言
//==============================================================================
extern void IntrudeField_UpdateCommSystem( FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor);
extern BOOL IntrudeField_CheckTalk(INTRUDE_COMM_SYS_PTR intcomm, const FIELD_PLAYER *fld_player, u32 *hit_netid);
extern BOOL IntrudeField_CheckTalkedTo(INTRUDE_COMM_SYS_PTR intcomm, u32 *hit_netid);
extern void IntrudeField_ConnectMap(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, INTRUDE_COMM_SYS_PTR intcomm);


//--------------------------------------------------------------
//  デバッグ用
//--------------------------------------------------------------
extern GMEVENT * DEBUG_IntrudeTreeMapWarp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor, INTRUDE_COMM_SYS_PTR intcomm);
extern GMEVENT * DEBUG_PalaceJamp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor);
