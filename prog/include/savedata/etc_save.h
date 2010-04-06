//==============================================================================
/**
 * @file    etc_save.h
 * @brief   第2のMISCヘッダ
 * @author  matsuda
 * @date    2010.04.06(火)
 */
//==============================================================================
#pragma once


//==============================================================================
//  定数定義
//==============================================================================
///その他セーブ構造体の不完全型
typedef struct _ETC_SAVE_WORK ETC_SAVE_WORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 EtcSave_GetWorkSize( void );
extern void EtcSave_WorkInit(void *work);
extern ETC_SAVE_WORK * SaveData_GetEtc( SAVE_CONTROL_WORK * p_sv );

extern void EtcSave_SetAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id);
extern BOOL EtcSave_CheckAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id);
