//==============================================================================
/**
 * @file    save_outside.h
 * @brief   セーブ管理外領域のセーブシステムのヘッダ
 * @author  matsuda
 * @date    2010.04.11(日)
 */
//==============================================================================
#pragma once


//==============================================================================
//  定数定義
//==============================================================================


//==============================================================================
//  構造体定義
//==============================================================================
///管理外セーブ：不思議な贈り物
typedef struct _OUTSIDE_MYSTERY OUTSIDE_MYSTERY;

///管理外セーブシステムの不定形型
typedef struct _OUTSIDE_SAVE_CONTROL OUTSIDE_SAVE_CONTROL;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern OUTSIDE_SAVE_CONTROL * OutsideSave_SystemLoad(HEAPID heap_id);
extern void OutsideSave_SystemUnload(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern void OutsideSave_SaveAsyncInit(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern BOOL OutsideSave_SaveAsyncMain(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern void OutsideSave_FlashAllErase(HEAPID heap_id);

//--------------------------------------------------------------
//  ツール
//--------------------------------------------------------------
extern BOOL OutsideSave_GetExistFlag(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern BOOL OutsideSave_GetBreakFlag(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern OUTSIDE_MYSTERY * OutsideSave_GetMysterPtr(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern void OutsideSave_MysteryData_Outside_to_Normal(OUTSIDE_SAVE_CONTROL *outsv_ctrl, SAVE_CONTROL_WORK *normal_ctrl);

//--------------------------------------------------------------
//  デバッグ用
//--------------------------------------------------------------
#ifdef PM_DEBUG
extern BOOL DEBUG_OutsideSave_GetBreak(OUTSIDE_SAVE_CONTROL *outsv_ctrl, BOOL *ret_a, BOOL *ret_b);
#endif  //PM_DEBUG
