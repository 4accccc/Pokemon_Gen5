//============================================================================================
/**
 * @file   oekaki.h
 * @brief  お絵かきボード画面処理
 * @author Akito Mori
 * @date   06.02.13
 */
//============================================================================================
#ifndef _OEKAKI_H_
#define _OEKAKI_H_

//============================================================================================
//  定義
//============================================================================================

// プロセス定義データ
extern GFL_PROC_RESULT OekakiProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT OekakiProc_Main( PGFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT OekakiProc_End(  GFL_PROC * proc, int *seq, void *pwk, void *mywk );

// ポインタ参照だけできるお絵かきボードワーク構造体
typedef struct OEKAKI_WORK OEKAKI_WORK; 

typedef struct{
  //COMM_UNIONROOM_VIEW *view;
  u32 *view;
}OEKAKI_PARAM;

#endif
