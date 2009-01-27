//============================================================================================
/**
 * @file	perapvoice_local.h
 * @brief	ぺラップ声データ用ヘッダ
 * @date	2006.04.06
 */
//============================================================================================

#ifndef	__PERAPVOICE_LOCAL_H__
#define	__PERAPVOICE_LOCAL_H__

#include "savedata/perapvoice.h"
#include "savedata/save_control.h"

//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
extern PERAPVOICE * SaveData_GetPerapVoice(SAVE_CONTROL_WORK * sv);
extern void PERAPVOICE_Init(PERAPVOICE * pv);
extern int PERAPVOICE_GetWorkSize(void);


#endif	__PERAPVOICE_LOCAL_H__
