//==============================================================================
/**
 * @file    battle_rec.h
 * @brief   戦闘録画セーブデータのヘッダ
 * @author  matsuda
 * @date    2009.09.01(火)
 */
//==============================================================================
#pragma once

#include "savedata/gds_profile.h"	//GDS_PROFILE_PTR定義

//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 BattleRec_GetWorkSize( void );
extern void BattleRec_WorkInit(void *rec);
extern void * BattleRec_RecWorkAdrsGet( void );
extern GDS_PROFILE_PTR BattleRec_GDSProfilePtrGet(void);

//==============================================================================
//	battl_rec_ov.c
//			※battle_recorderオーバーレイ領域に配置されているので注意!
//==============================================================================
//extern void BattleRecTool_ErrorStrCheck(BATTLE_REC_SAVEDATA *src, BATTLE_PARAM *bp, int heap_id);
extern SAVE_RESULT BattleRec_GDS_RecvData_Save(SAVE_CONTROL_WORK *sv, int num, u8 secure, u16 *work0, u16 *work1);
extern void BattleRec_GDS_SendData_Conv(SAVE_CONTROL_WORK *sv);
extern SAVE_RESULT BattleRec_GDS_MySendData_DataNumberSetSave(SAVE_CONTROL_WORK *sv, u64 data_number, u16 *work0, u16 *work1);
extern SAVE_RESULT BattleRec_SecureSetSave(SAVE_CONTROL_WORK *sv, int num);

#ifdef PM_DEBUG
extern void DEBUG_BattleRec_SecureFlagSet(SAVE_CONTROL_WORK *sv);
#endif

