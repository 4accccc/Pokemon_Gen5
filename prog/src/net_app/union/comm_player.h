//==============================================================================
/**
 * @file    comm_player.h
 * @brief   通信プレイヤー制御のヘッダ
 * @author  matsuda
 * @date    2009.07.17(金)
 */
//==============================================================================
#pragma once

#include "gamesystem/gamesystem.h"
#include "field/fldmmdl.h"

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
extern COMM_PLAYER_SYS_PTR CommPlayer_Init(int max, GAMESYS_WORK *gsys, HEAPID heap_id);
extern void CommPlayer_Exit(COMM_PLAYER_SYS_PTR cps);
extern void CommPlayer_Update(COMM_PLAYER_SYS_PTR cps);
extern void CommPlayer_Add(COMM_PLAYER_SYS_PTR cps, int index, u16 obj_code, const COMM_PLAYER_PACKAGE *pack);
extern void CommPlayer_Del(COMM_PLAYER_SYS_PTR cps, int index);
extern void CommPlayer_Push(COMM_PLAYER_SYS_PTR cps);
extern void CommPlayer_Pop(COMM_PLAYER_SYS_PTR cps);
extern void CommPlayer_FlagSet_UpdateStop(COMM_PLAYER_SYS_PTR cps, BOOL stop_flag);
extern BOOL CommPlayer_CheckOcc(COMM_PLAYER_SYS_PTR cps, int index);
extern u16 CommPlayer_GetObjCode(COMM_PLAYER_SYS_PTR cps, int index);
extern void CommPlayer_SetParam(COMM_PLAYER_SYS_PTR cps, int index, const COMM_PLAYER_PACKAGE *pack);
extern BOOL CommPlayer_Mine_DataUpdate(COMM_PLAYER_SYS_PTR cps, COMM_PLAYER_PACKAGE *pack);
extern BOOL CommPlayer_SearchGridPos(COMM_PLAYER_SYS_PTR cps, s16 gx, s16 gz, u32 *out_index);
extern BOOL CommPlayer_SetGyoeTask(COMM_PLAYER_SYS_PTR cps, int index);
extern MMDL * CommPlayer_GetMmdl(COMM_PLAYER_SYS_PTR cps, int index);
