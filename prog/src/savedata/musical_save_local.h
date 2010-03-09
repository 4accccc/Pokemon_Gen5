//============================================================================================
/**
 * @file	musical_save_local.h
 * @brief	ミュージカル用セーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 */
//============================================================================================

#ifndef MUSICAL_SAVE_LOCAL_H__
#define MUSICAL_SAVE_LOCAL_H__

#include "savedata/musical_save.h"

struct _MUSICAL_SAVE {
  //ミュージカルショット
  MUSICAL_SHOT_DATA   musicalShotData;
	//前回装備
	MUSICAL_EQUIP_SAVE	befEquip;
	
  MUSICAL_FAN_STATE fanState[MUS_SAVE_FAN_NUM];

  //参加回数
  u16 entryNum; 
  //トップ回数
  u16 topNum;
  //累計得点
  u16 sumPoint;

	//アイテム所持bit
	u8 itemBit[MUS_SAVE_ITEM_BIT_MAX];
  //Newアイテムbit
	u8 itemNewBit[MUS_SAVE_ITEM_BIT_MAX];
  
  //前回コンディション
  u8 befCondition[MCT_MAX];
  //前回評価点
  u8 befPoint;
  
  //選択している演目番号
  u8 programNumber;
  
  //配信データのチェック
  u8 enableDistData;
  
  u8 padding[2];
};

#endif //MUSICAL_SAVE_LOCAL_H__