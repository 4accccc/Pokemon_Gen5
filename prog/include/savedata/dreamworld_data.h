//============================================================================================
/**
 * @file	  dreamworld_data.h
 * @brief	  ポケモンドリームワールドに必要なデータ
 * @author	k.ohno
 * @date	  2009.11.08
 */
//============================================================================================
#pragma once

#include "poke_tool/poke_tool.h"

#define DREAM_WORLD_DATA_MAX_EVENT (100)          ///< 受け取るイベントの種類
#define DREAM_WORLD_DATA_FURNITURE_NAME_NUM (12)  ///< 家具名文字列NUM
#define DREAM_WORLD_DATA_MAX_FURNITURE (5)        ///< 家具のMAX ５つ


typedef struct _DREAMWORLD_SAVEDATA DREAMWORLD_SAVEDATA;


/// 寝てるポケモンの状態
typedef enum{
  DREAM_WORLD_SLEEP_TYPE_NONE,    ///< 預けていない。寝かせていない。
  DREAM_WORLD_SLEEP_TYPE_SLEEP,   ///< 寝かせてサーバに送った
  DREAM_WORLD_SLEEP_TYPE_PLAYED,  ///< ドリームワールドで遊んだ
  DREAM_WORLD_SLEEP_TYPE_MAX,
} DREAM_WORLD_SLEEP_TYPE;



/// PDWでもらうタイプ
typedef enum{
  DREAM_WORLD_RESULT_TYPE_NONE,    ///< 空っぽ
  DREAM_WORLD_RESULT_TYPE_EVENT,   ///< イベント
  DREAM_WORLD_RESULT_TYPE_ITEM,    ///< アイテム
  DREAM_WORLD_RESULT_TYPE_ENCOUNT, ///< エンカウントタイプ
  DREAM_WORLD_RESULT_TYPE_MAX,
} DREAM_WORLD_RESULT_TYPE;


typedef struct {
  u16 eventNo;   // イベント番号
} DREAM_WORLD_EVENTDATA;

typedef struct {
	u16 itemID;   //	アイテムID						2byte			受信したアイテムの種類
} DREAM_WORLD_ITEMDATA;

typedef struct {
	u16 encountType; //エンカウントタイプ
} DREAM_WORLD_ENCOUNTDATA;

/// 家具構造体 26byte
typedef struct {
  u16 id;
  u16 furnitureName[DREAM_WORLD_DATA_FURNITURE_NAME_NUM];
} DREAM_WORLD_FURNITUREDATA;

/// ご褒美データ共用体
typedef union {
  DREAM_WORLD_EVENTDATA event;
  DREAM_WORLD_ITEMDATA item;
  DREAM_WORLD_ENCOUNTDATA enc;
} DREAM_WORLD_TREAT_DATA;





extern int DREAMWORLD_SV_GetWorkSize(void);
extern DREAMWORLD_SAVEDATA* DREAMWORLD_SV_AllocWork(HEAPID heapID);
extern void DREAMWORLD_SV_Init(DREAMWORLD_SAVEDATA* pSV);
extern POKEMON_PARAM* DREAMWORLD_SV_GetSleepPokemon(DREAMWORLD_SAVEDATA* pSV);
extern void DREAMWORLD_SV_SetSleepPokemon(DREAMWORLD_SAVEDATA* pSV,POKEMON_PARAM* pp);
extern DREAM_WORLD_RESULT_TYPE DREAMWORLD_SV_GetCategoryType(DREAMWORLD_SAVEDATA* pSV);
extern void DREAMWORLD_SV_SetCategoryType(DREAMWORLD_SAVEDATA* pSV,DREAM_WORLD_RESULT_TYPE type);
extern DREAM_WORLD_TREAT_DATA* DREAMWORLD_SV_GetTreatData(DREAMWORLD_SAVEDATA* pSV);
extern void DREAMWORLD_SV_SetTreatData(DREAMWORLD_SAVEDATA* pSV,DREAM_WORLD_TREAT_DATA* pTreat);
extern DREAM_WORLD_FURNITUREDATA* DREAMWORLD_SV_GetFurnitureData(DREAMWORLD_SAVEDATA* pSV,int index);
extern void DREAMWORLD_SV_SetFurnitureData(DREAMWORLD_SAVEDATA* pSV,int index,DREAM_WORLD_FURNITUREDATA* pF);
extern DREAM_WORLD_RESULT_TYPE DREAMWORLD_SV_GetPokemonStatus(DREAMWORLD_SAVEDATA* pSV);
extern void DREAMWORLD_SV_SetPokemonStatus(DREAMWORLD_SAVEDATA* pSV,DREAM_WORLD_RESULT_TYPE status);
extern BOOL DREAMWORLD_SV_IsEventRecvFlag(DREAMWORLD_SAVEDATA* pSV,int num);
extern void DREAMWORLD_SV_SetEventRecvFlag(DREAMWORLD_SAVEDATA* pSV, int num);

extern void DREAMWORLD_SV_SetSleepPokemonFlg(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg);
extern BOOL DREAMWORLD_SV_GetSleepPokemonFlg(DREAMWORLD_SAVEDATA* pSV);

extern DREAMWORLD_SAVEDATA* DREAMWORLD_SV_GetDreamWorldSaveData(SAVE_CONTROL_WORK* pSave);


