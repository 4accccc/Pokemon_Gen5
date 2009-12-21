//======================================================================
/**
 * @file	field_player_code.h
 * @date	2008.9.29
 * @brief	フィールドプレイヤー　スクリプトからも参照する値
 * @note アセンブラ参照が有るためdefine以外は禁止。
 */
//======================================================================
#pragma once

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
/// FIELD_PLAYER_REQBIT
//--------------------------------------------------------------
///自機を２速歩行に変更
#define FIELD_PLAYER_REQBIT_NORMAL (1<<0)
///自機を自転車に変更
#define FIELD_PLAYER_REQBIT_CYCLE  (1<<1)
///自機を波乗り状態に変更
#define FIELD_PLAYER_REQBIT_SWIM   (1<<2)
///動作形態にあわせた表示にする
#define FIELD_PLAYER_REQBIT_MOVE_FORM_TO_DRAW_FORM (1<<3)
///アイテムゲット自機表示にする
#define FIELD_PLAYER_REQBIT_ITEMGET (1<<4)
///レポート自機表示にする
#define FIELD_PLAYER_REQBIT_REPORT (1<<5)
///PC預け自機表示にする
#define FIELD_PLAYER_REQBIT_PC_AZUKE (1<<6)
///カットイン自機表示にする
#define FIELD_PLAYER_REQBIT_CUTIN (1<<7)
///ビット最大
#define FIELD_PLAYER_REQBIT_MAX (8)
