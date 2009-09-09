//==============================================================================
/**
 * @file    
 * @brief   簡単な説明を書く
 * @author  matsuda
 * @date    2009.09.03(木)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================
#pragma once

#include "field/fieldmap_proc.h"

//==============================================================================
//  構造体定義
//==============================================================================
///GameCommSys_Bootする時に引き渡すparent_work
typedef struct{
  GAMESYS_WORK *gsys;
  GAME_COMM_SYS_PTR game_comm;
  u8 parent_macAddress[6];
  u8 my_invasion;             ///<TRUE:侵入開始するユーザーである
  u8 padding;
}FIELD_INVALID_PARENT_WORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void * IntrudeComm_InitCommSystem( int *seq, void *pwk );
extern BOOL  IntrudeComm_InitCommSystemWait( int *seq, void *pwk, void *pWork );
extern void  IntrudeComm_UpdateSystem( int *seq, void *pwk, void *pWork );
extern BOOL  IntrudeComm_TermCommSystem( int *seq, void *pwk, void *pWork );
extern BOOL  IntrudeComm_TermCommSystemWait( int *seq, void *pwk, void *pWork );
extern void IntrudeComm_FieldCreate(void *pwk, void *app_work, FIELD_MAIN_WORK *fieldWork);
extern void IntrudeComm_FieldDelete(void *pwk, void *app_work, FIELD_MAIN_WORK *fieldWork);

