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
};


#endif //MUSICAL_SAVE_LOCAL_H__