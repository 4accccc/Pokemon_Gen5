//==============================================================================
/**
 * @file    save_outside.h
 * @brief   セーブ管理外領域のセーブシステムのヘッダ
 * @author  matsuda
 * @date    2010.04.11(日)
 */
//==============================================================================
#pragma once

#include "savedata/mystery_data.h"
#include "savedata/mystery_data_local.h"


//==============================================================================
//  定数定義
//==============================================================================
///管理外不思議データで所持できるカード数
#define OUTSIDE_MYSTERY_MAX     (3)


//==============================================================================
//  構造体定義
//==============================================================================
///管理外セーブ：不思議な贈り物
typedef struct{
  u8 recv_flag[MYSTERY_DATA_MAX_EVENT / 8];		//256 * 8 = 2048 bit
  GIFT_PACK_DATA card[OUTSIDE_MYSTERY_MAX];		// カード情報
  RECORD_CRC crc;   //CRC & 暗号化キー   4 byte
}OUTSIDE_MYSTERY;

///管理外セーブシステムの不定形型
typedef struct _OUTSIDE_SAVE_CONTROL OUTSIDE_SAVE_CONTROL;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern OUTSIDE_SAVE_CONTROL * OutsideSave_SystemLoad(HEAPID heap_id);
extern void OutsideSave_SystemUnload(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern void OutsideSave_SaveAsyncInit(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern BOOL OutsideSave_SaveAsyncMain(OUTSIDE_SAVE_CONTROL *outsv_ctrl);

//--------------------------------------------------------------
//  ツール
//--------------------------------------------------------------
extern BOOL OutsideSave_GetExistFlag(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern BOOL OutsideSave_GetBreakFlag(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern OUTSIDE_MYSTERY * OutsideSave_GetMysterPtr(OUTSIDE_SAVE_CONTROL *outsv_ctrl);
extern void OutsideSave_MysteryData_Outside_to_Normal(OUTSIDE_SAVE_CONTROL *outsv_ctrl, SAVE_CONTROL_WORK *normal_ctrl);
