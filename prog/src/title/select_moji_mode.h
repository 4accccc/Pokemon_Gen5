//======================================================================
/**
 * @file	select_mode.c
 * @brief	ゲーム開始時の漢字・ひらがなの選択
 * @author	ariizumi
 * @data	09/05/11
 */
//======================================================================

#ifndef SELECT_MODE_H__
#define SELECT_MODE_H__

#include "savedata/config.h"
#include "savedata/mystatus.h"
#include "savedata/misc.h"

extern const GFL_PROC_DATA SelectModeProcData;

typedef enum
{
  SMT_START_GAME,     //最初から(漢字→通信
  SMT_CONTINUE_GAME,  //続きから(通信のみ
}SELECT_MODE_TYPE;

typedef struct
{
  SELECT_MODE_TYPE type;
  CONFIG *configSave;
  MYSTATUS *mystatus;
	MISC * miscSave;
}SELECT_MODE_INIT_WORK;

#endif //SELECT_MOJI_MODE_H__
