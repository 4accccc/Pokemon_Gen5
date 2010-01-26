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

#define DREAM_WORLD_DATA_MAX_ITEMBOX (20)          ///< 受け取るアイテムボックスの数
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


/// 家具構造体 26byte
typedef struct {
  u16 id;
  u16 furnitureName[DREAM_WORLD_DATA_FURNITURE_NAME_NUM];
} DREAM_WORLD_FURNITUREDATA;



extern int DREAMWORLD_SV_GetWorkSize(void);
extern DREAMWORLD_SAVEDATA* DREAMWORLD_SV_AllocWork(HEAPID heapID);
extern void DREAMWORLD_SV_Init(DREAMWORLD_SAVEDATA* pSV);
extern POKEMON_PARAM* DREAMWORLD_SV_GetSleepPokemon(DREAMWORLD_SAVEDATA* pSV);
extern void DREAMWORLD_SV_SetSleepPokemon(DREAMWORLD_SAVEDATA* pSV,POKEMON_PARAM* pp);

extern u16 DREAMWORLD_SV_GetItem(DREAMWORLD_SAVEDATA* pSV,int index);
extern u8 DREAMWORLD_SV_GetItemNum(DREAMWORLD_SAVEDATA* pSV, int index);
extern void DREAMWORLD_SV_SetItem(DREAMWORLD_SAVEDATA* pSV,int index ,int itemNo, int num);
extern void DREAMWORLD_SV_DeleteItem(DREAMWORLD_SAVEDATA* pSV,int index);
extern void DREAMWORLD_SV_ClearAllItem(DREAMWORLD_SAVEDATA* pSV);

extern DREAM_WORLD_FURNITUREDATA* DREAMWORLD_SV_GetFurnitureData(DREAMWORLD_SAVEDATA* pSV,int index);
extern void DREAMWORLD_SV_SetFurnitureData(DREAMWORLD_SAVEDATA* pSV,int index,DREAM_WORLD_FURNITUREDATA* pF);

extern DREAM_WORLD_SLEEP_TYPE DREAMWORLD_SV_GetPokemonStatus(DREAMWORLD_SAVEDATA* pSV);
extern void DREAMWORLD_SV_SetPokemonStatus(DREAMWORLD_SAVEDATA* pSV,DREAM_WORLD_SLEEP_TYPE status);

extern void DREAMWORLD_SV_SetSleepPokemonFlg(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg);
extern BOOL DREAMWORLD_SV_GetSleepPokemonFlg(DREAMWORLD_SAVEDATA* pSV);

extern void DREAMWORLD_SV_SetSignin(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg);  //PDW登録済み
extern BOOL DREAMWORLD_SV_GetSignin(DREAMWORLD_SAVEDATA* pSV);

extern void DREAMWORLD_SV_SetAccount(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg);  //アカウント登録済み
extern BOOL DREAMWORLD_SV_GetAccount(DREAMWORLD_SAVEDATA* pSV);

extern void DREAMWORLD_SV_SetRatemode(DREAMWORLD_SAVEDATA* pSV,BOOL bFlg);  //レーティングバトル開放
extern BOOL DREAMWORLD_SV_GetRatemode(DREAMWORLD_SAVEDATA* pSV);


extern DREAMWORLD_SAVEDATA* DREAMWORLD_SV_GetDreamWorldSaveData(SAVE_CONTROL_WORK* pSave);


