//===================================================================================
/**
 *  @file   event_move_hilink.h
 *  @brief  ハイリンク移動イベントコア(Cギアワイヤレス、デルパワー使用画面からイベント接続)
 *  @author Miyuki Iwasawa
 *  @date   11.07.12
 *
 *  ワイヤレス、デルパワー使用画面から「ハイリンク」を選んだ後
 *  侵入ステートによって起こる分岐を処理して、各ステートに応じたイベントを内部コールする
 *
 *  侵入ステートがINTRUDE_CONNECT_MISSION_TARGETの場合、このイベント起動前の
 *  各画面からの復帰時ブラックアウト中に、下画面をFIELD_SUBSCREEN_INTRUDEに変更しておく
 *  必要があるので注意！
 */
//===================================================================================

#pragma once

#include "field_comm/intrude_work.h"

FS_EXTERN_OVERLAY(event_move_hilink);

//=============================================================
/*
 *  @brief  ハイリンクへのワープイベント起動
 *
 *  @param  gsys
 *  @param  state INTRUDE_CONNECT アプリから受け取ったIntrude_GetIntrudeStatus(pComm)の戻り値
 */ 
//=============================================================
extern GMEVENT*  EVENT_MoveHilink( GAMESYS_WORK* gsys, INTRUDE_CONNECT state );




