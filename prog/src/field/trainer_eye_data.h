//======================================================================
/**
 * @file  trainer_eye_data.h
 * @date  2009.09.09
 * @author  GAMEFREAK inc.
 *
 * 2009.09.09 script.cから分離した
 */
//======================================================================
#pragma once
//--------------------------------------------------------------
/**
 * トレーナー視線データ構造体
 */
//--------------------------------------------------------------
typedef struct {
	int range;				//視線距離
	int dir;					//移動方向
	int scr_id;				//スクリプトID
	int tr_id;				//トレーナーID
	int tr_type;			//トレーナータイプ
	MMDL *mmdl;
  GMEVENT *ev_eye_move;
}EV_TRAINER_EYE_HITDATA;

//--------------------------------------------------------------
//--------------------------------------------------------------
static inline void TRAINER_EYE_HITDATA_Set(
    EV_TRAINER_EYE_HITDATA * eye,
    MMDL *mmdl,
    s16 range, u16 dir, u16 scr_id, u16 tr_id, int tr_type, int tr_no )
{
  eye->range = range;
	eye->dir = dir;
	eye->scr_id = scr_id;
	eye->tr_id = tr_id;
	eye->tr_type = tr_type;
	eye->mmdl = mmdl;
  eye->ev_eye_move = NULL;
}

