//======================================================================
/**
 * @file   battle_examination.h
 * @brief  バトル検定セーブデータ
 * @author k.ohno
 * @date	 10.01.08
 */
//======================================================================


#include "savedata/bsubway_savedata.h"
#include "battle/bsubway_battle_data.h"


//typedef _BSUBWAY_PARTNER_DATA BSUBWAY_PARTNER_DATA;

#define BATTLE_EXAMINATION_MAX (5)   ///< バトル検定の人数

typedef struct{
  BSUBWAY_PARTNER_DATA trainer[BATTLE_EXAMINATION_MAX];
  u16 crc;
} BATTLE_EXAMINATION_SAVEDATA;


extern int BATTLE_EXAMINATION_SAVE_GetWorkSize(void);
extern BATTLE_EXAMINATION_SAVEDATA* BATTLE_EXAMINATION_SAVE_AllocWork(HEAPID heapID);
extern void BATTLE_EXAMINATION_SAVE_Init(BATTLE_EXAMINATION_SAVEDATA* pSV);

