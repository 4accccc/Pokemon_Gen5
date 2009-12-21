//==============================================================================
/**
 * @file    game_beacon.h
 * @brief   
 * @author  matsuda
 * @date    2009.12.13(日)
 */
//==============================================================================
#pragma once

#include "buflen.h"
#include "gamesystem/gamedata_def.h"
#include "print/wordset.h"


//==============================================================================
//  定数定義
//==============================================================================
///ログ件数
#define GAMEBEACON_LOG_MAX       (20)


//==============================================================================
//  構造体定義
//==============================================================================
///送受信されるビーコンパラメータ
typedef struct{
  u16 action_no;                            ///<行動No
  u16 send_counter;                         ///<送信No(同じデータの無視判定に使用)
  u32 trainer_id;                           ///<トレーナーID
  STRCODE name[BUFLEN_PERSON_NAME];         ///<トレーナー名
  
  GXRgb favorite_color;                     ///<本体情報の色
  u16 padding;
  
  //action_noによってセットされるものが変わる
  union{
    STRCODE nickname[BUFLEN_POKEMON_NAME];  ///<ポケモン名(ニックネーム)
  };
}GAMEBEACON_INFO;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void GAMEBEACON_Init(HEAPID heap_id);
extern void GAMEBEACON_Exit(void);
extern void GAMEBEACON_Update(void);
extern void GAMEBEACON_Setting(GAMEDATA *gamedata);
extern void GAMEBEACON_SendDataCopy(GAMEBEACON_INFO *info);

extern BOOL GAMEBEACON_SetRecvBeacon(const GAMEBEACON_INFO *info);
extern const GAMEBEACON_INFO * GAMEBEACON_Get_BeaconLog(int log_no);
extern u32 GAMEBEACON_Get_LogCount(void);
extern BOOL GAMEBEACON_Get_FavoriteColor(GXRgb *dest_buf, int log_no);
extern void GAMEBEACON_Wordset(const GAMEBEACON_INFO *info, WORDSET *wordset, HEAPID temp_heap_id);
extern u32 GAMEBEACON_GetMsgID(const GAMEBEACON_INFO *info);

//--------------------------------------------------------------
//  ビーコンセット
//--------------------------------------------------------------
extern void GAMEBEACON_Set_Congratulations(void);
extern void GAMEBEACON_Set_PokemonEvolution(const STRBUF *nickname);
extern void GAMEBEACON_Set_PokemonLevelUp(const STRBUF *nickname);
extern void GAMEBEACON_Set_PokemonGet(const STRBUF *nickname);
extern void GAMEBEACON_Set_UnionIn(void);
extern void GAMEBEACON_Set_UnionOut(void);
extern void GAMEBEACON_Set_EncountDown(void);
