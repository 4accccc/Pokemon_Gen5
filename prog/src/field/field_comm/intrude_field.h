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
//  外部関数宣言
//==============================================================================
extern void IntrudeField_UpdateCommSystem( FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor);


//--------------------------------------------------------------
//  デバッグ用
//--------------------------------------------------------------
extern GMEVENT * DEBUG_IntrudeTreeMapWarp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor, INTRUDE_COMM_SYS_PTR intcomm);
extern GMEVENT * DEBUG_PalaceJamp(FIELDMAP_WORK *fieldWork, GAMESYS_WORK *gameSys, FIELD_PLAYER *pcActor);
