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

#include "battle/battle.h"
#include "fldmmdl.h"

//--------------------------------------------------------------
///  視線ヒット格納
//--------------------------------------------------------------
typedef struct
{
	int range;				//視線距離
	int dir;					//移動方向
	int scr_id;				//スクリプトID
	int tr_id;				//トレーナーID
	BtlRule rule_type;			//戦闘タイプ
  int move_type;    //視線動作タイプ
  MMDL *mmdl;
}TRAINER_HITDATA;

//--------------------------------------------------------------
/**
 * トレーナー視線データ構造体
 */
//--------------------------------------------------------------
typedef struct {
  TRAINER_HITDATA hitdata;

  GMEVENT *ev_eye_move;
}SCR_TRAINER_HITDATA;

