//=============================================================================================
/**
 * @file	gfl_use.h
 * @author	GAME FREAK inc.
 * @date	2006.12.06
 * @brief	GFライブラリ呼び出し部分
 */
//=============================================================================================

#pragma once

//gsまでのポケモンで使っていたgf_randはu16で返していたので、
//移植用に定義nagihashi
#define GFUSER_RAND_PAST_MAX (0xFFFF)

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//GFL関連初期化処理
extern void GFLUser_Init(void);

//GFL関連Main呼び出し
extern void GFLUser_Main(void);

//GFL関連描画処理呼び出し
extern void GFLUser_Display(void);

//GFL関連終了処理
extern void GFLUser_Exit(void);

//GFL関連HBlank割り込み処理
extern void GFLUser_HIntr(void);

//HBlank中TCBの登録処理を追加
extern GFL_TCB * GFUser_HIntr_CreateTCB(GFL_TCB_FUNC * func, void * work, u32 pri);

//GFL関連VBlank割り込み処理
extern void GFLUser_VIntr(void);

//GFL関連VBlankタイミング処理
extern void GFLUser_VTiming(void);

//VBlankタイミング中TCBの登録処理を追加
extern GFL_TCB * GFUser_VIntr_CreateTCB(GFL_TCB_FUNC * func, void * work, u32 pri);

//VBlankタイミング中TCBSYSの取得
extern	GFL_TCBSYS * GFUser_VIntr_GetTCBSYS( void );

//VBlankカウンター取得とリセット
extern void GFUser_VIntr_ResetVblankCounter( void );
extern int GFUser_VIntr_GetVblankCounter( void );

//汎用乱数初期化関数で初期化する
extern void GFUser_SetStartRandContext(GFL_STD_RandContext *context);

// 汎用乱数を得る
extern u32 GFUser_GetPublicRand(u32 range);
extern u32 GFUser_GetPublicRand0(u32 range);


//	アーカイブIDでファイル名を取得
extern const char * GFUser_GetFileNameByArcID(ARCID arcID);

//アサートの終了処理関数(DEBUG_SD_PRINTで使うため定義を出す
extern void GFUser_AssertFinish( void );

typedef void (*GFL_USE_VINTR_FUNC)( void );
//VBlank割り込み関数の登録
extern BOOL GFUser_SetVIntrFunc(GFL_USE_VINTR_FUNC func);
//VBlank割り込み関数の解除
extern void GFUser_ResetVIntrFunc(void);

