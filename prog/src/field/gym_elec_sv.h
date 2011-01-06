//======================================================================
/**
 * @file  gym_elec_sv.h
 * @brief  電気ジムセーブデータ
 * @author  Toru=Nagihashi
 * @note    evで読み込んでいます。
 */
//======================================================================
#pragma once

//=============================================================================
/**
 *    スクリプト、ソース共通定義
 */
//=============================================================================
//-------------------------------------
///	_GYM_ELEC_INITにいれる値及びプログラム中に使う値
//=====================================
#define GYM_ELEC_EVENT_SEQ_START        (0) //初期状態              影手前　曲０段階
#define GYM_ELEC_EVENT_SEQ_TR01_BTL_END (1) //一人目に勝った        影中間  曲１段階
#define GYM_ELEC_EVENT_SEQ_TR02_BTL_END (2) //二人目に勝った        影奥    曲２段階
#define GYM_ELEC_EVENT_SEQ_TR03_BTL_END (3) //三人目に勝った        影なし  曲完成
#define GYM_ELEC_EVENT_SEQ_BOSS_BTL_END (4) //ジムリーダーに勝った  影なし  曲完成
#define GYM_ELEC_EVENT_SEQ_MAX          (5) //スクリプトでは使用禁止

//-------------------------------------
///_GYM_ELEC_SET_SPOTLIGHTにいれる値及びプログラム中に使う値
//=====================================
#define GYM_ELEC_SPOTLIGHT_NORMAL   (0) //通常ライト
#define GYM_ELEC_SPOTLIGHT_LEFT     (1) //左から当たるライト
#define GYM_ELEC_SPOTLIGHT_RIGHT    (2) //右から当たるライト
#define GYM_ELEC_SPOTLIGHT_CENTER   (3) //上から当たるライト
#define GYM_ELEC_SPOTLIGHT_MAX      (4) //スクリプトでは使用禁止

//-------------------------------------
///_GYM_ELEC_SET_EFFECTにいれる値及びプログラム中に使う値
//=====================================
#define GYM_ELEC_EFFECT_FAN_NORMAL  (0) //観客が通常アニメをする
#define GYM_ELEC_EFFECT_FAN_TR_END  (1) //観客が盛り上がりアニメする＋カメラフラッシュをする
#define GYM_ELEC_EFFECT_FAN_BOSS_END (2) //観客が大きく盛り上がりアニメする＋カメラフラッシュを多くする
#define GYM_ELEC_EFFECT_DISPLAY_BOSS  (3) //ディスプレイをカミツレにする
#define GYM_ELEC_EFFECT_DISPLAY_HERO  (4) //ディスプレイを主人公にする
#define GYM_ELEC_EFFECT_LIGHTUP_BOSS_START  (5) //カミツレ勝利後のライトアップを付ける(アルファ有り版)
#define GYM_ELEC_EFFECT_LIGHTUP_BOSS_RECOVER  (6) //カミツレ勝利後のライトアップを付ける（復帰版）
#define GYM_ELEC_EFFECT_FLASH_WHITE   (7) //画面を輝度を白にする
#define GYM_ELEC_EFFECT_FLASH_NORMAL  (8) //画面の輝度をもとに戻す

//=============================================================================
/**
 *    ソース用、定義
 */
//=============================================================================
#ifndef __ASM_NO_DEF_
//-------------------------------------
///	電気ジムセーブワーク
//=====================================
typedef struct GYM_ELEC_SV_WORK_tag
{
   u16 event_seq; //GYM_ELEC_EVENT_SEQ_～の値
   u16 dummy;
}GYM_ELEC_SV_WORK;

#endif //__ASM_NO_DEF_

