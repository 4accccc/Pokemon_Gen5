//======================================================================
/**
 * @file	dup_local_def.h
 * @brief	ミュージカル　ドレスアップ用音定義
 * @author	ariizumi
 * @data	09/09/30
 */
//======================================================================

#pragma once

#include "sound/pm_sndsys.h"

//決定音
#define DUP_SE_DECIDE (SEQ_SE_MSCL_01)
//キャンセル音
#define DUP_SE_CANCEL (SEQ_SE_CANCEL1)
//ドレスアップ終了時キラキラ音
#define DUP_SE_KIRAKIRA (SEQ_SE_DECIDE3)

//最初のカーテン開く音
#define DUP_SE_FIRST_LIST_OPEN (SEQ_SE_DECIDE3)
//リスト回転音
#define DUP_SE_LIST_ROTATE (SEQ_SE_MSCL_03)
//↑を鳴らすための速度と連続防止カウント数
#define DUP_LIST_ROTATE_SE_SPEED (2500)
#define DUP_LIST_ROTATE_SE_WAIT_CNT (30)
//グッズタッチ音
#define DUP_SE_LIST_TOUCH_GOODS (SEQ_SE_MSCL_02)
//リストからグッズを取り出す音
#define DUP_SE_LIST_TAKE_GOODS (SEQ_SE_MSCL_04)
//リストにグッズを戻す音
#define DUP_SE_LIST_RETURN_GOODS (SEQ_SE_MSCL_05)
//グッズに触れない音
#define DUP_SE_LIST_NO_GOODS (SEQ_SE_BEEP)

//グッズをポケモンに装着する音
#define DUP_SE_POKE_SET_GOODS (SEQ_SE_MSCL_06)
//ポケモンタッチ音（グッズ非装着）
#define DUP_SE_POKE_TOUCH (SEQ_SE_MSCL_07)
//グッズに傾斜をつける音
#define DUP_SE_POKE_ADJUST (SEQ_SE_MSCL_08)
//↑を鳴らすための値
#define DUP_POKE_ADJUST_SE_VALUE (10*0x10000/360)

//  PMSND_PlaySE(  );
