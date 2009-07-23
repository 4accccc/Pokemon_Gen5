//==============================================================================
/**
 * @file    comm_player.h
 * @brief   通信プレイヤー制御のヘッダ
 * @author  matsuda
 * @date    2009.07.17(金)
 */
//==============================================================================
#pragma once

#include "field\fieldmap_proc.h"

//==============================================================================
//  型定義
//==============================================================================
///COMM_PLAYER_SYS構造体の不定形ポインタ型
typedef struct _COMM_PLAYER_SYS * COMM_PLAYER_SYS_PTR;


//==============================================================================
//  構造体定義
//==============================================================================
///通信プレイヤーの座標データパッケージ
typedef struct{
  BOOL vanish;
  VecFx32 pos;
  u16 dir;
  u8 padding[2];
}COMM_PLAYER_PACKAGE;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern COMM_PLAYER_SYS_PTR CommPlayer_Init(int max, FIELD_MAIN_WORK *fieldWork, HEAPID heap_id);
extern void CommPlayer_Exit(COMM_PLAYER_SYS_PTR cps);
void CommPlayer_Add(COMM_PLAYER_SYS_PTR cps, int index, int sex, const COMM_PLAYER_PACKAGE *pack);
extern void CommPlayer_Del(COMM_PLAYER_SYS_PTR cps, int index);
extern void CommPlayer_Push(COMM_PLAYER_SYS_PTR cps);
extern void CommPlayer_Pop(COMM_PLAYER_SYS_PTR cps);
extern void CommPlayer_FlagSet_UpdateStop(COMM_PLAYER_SYS_PTR cps, BOOL stop_flag);
extern BOOL CommPlayer_CheckOcc(COMM_PLAYER_SYS_PTR cps, int index);
extern void CommPlayer_SetParam(COMM_PLAYER_SYS_PTR cps, int index, const COMM_PLAYER_PACKAGE *pack);
extern BOOL CommPlayer_Mine_DataUpdate(COMM_PLAYER_SYS_PTR cps, COMM_PLAYER_PACKAGE *pack);
