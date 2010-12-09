//======================================================================
/**
 * @file  gym_elec_sv.h
 * @brief  電気ジムセーブデータ
 * @author  Saito
 */
//======================================================================
#pragma once

typedef enum
{
  GYM_ELEC_EVENT_SEQ_START,
  GYM_ELEC_EVENT_SEQ_TR01_BTL_END,
  GYM_ELEC_EVENT_SEQ_TR02_BTL_END,
  GYM_ELEC_EVENT_SEQ_TR03_BTL_END,
  GYM_ELEC_EVENT_SEQ_BOSS_BTL_END,
} GYM_ELEC_EVENT_SEQ;

//電気ジムセーブワーク
typedef struct GYM_ELEC_SV_WORK_tag
{
  GYM_ELEC_EVENT_SEQ  event_seq;
}GYM_ELEC_SV_WORK;

