//======================================================================
/**
 * @file  gym_elec_sv.h
 * @brief  電気ジムセーブデータ
 * @author  Saito
 */
//======================================================================
#pragma once

#define CAPSULE_NUM_MAX (4)
#define LEVER_NUM_MAX (CAPSULE_NUM_MAX)
#define PLATFORM_NO_STOP (-1)

typedef struct CAPSULE_DAT_tag
{
///  int RaleIdx;      //現在走行中のレールインデックス
  fx32 AnmFrame;    //レールの現在アニメフレーム
}CAPSULE_DAT;

//電気ジムセーブワーク
typedef struct GYM_ELEC_SV_WORK_tag
{
  CAPSULE_DAT CapDat[CAPSULE_NUM_MAX];
  u8 LeverSw[LEVER_NUM_MAX];    //0or1
/**  
  u8 RaleChgReq[CAPSULE_NUM_MAX];
  u8 NowRaleIdx[CAPSULE_NUM_MAX];     //現在走行しているレールのインデックス(スイッチ非依存)
  s8 StopPlatformIdx[CAPSULE_NUM_MAX];  //PLATFORM_NO_STOP (-1)　を使用するのでマイナス値を使用できるように。
*/
  u16 EvtFlg[2];    //イベント進行フラグ  0：イベントトレーナー1人目　1：イベントトレーナー2人目
}GYM_ELEC_SV_WORK;

