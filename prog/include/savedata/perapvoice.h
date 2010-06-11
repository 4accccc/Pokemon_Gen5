//============================================================================================
/**
 * @file  perapvoice.h
 * @brief ぺラップ声データ用ヘッダ
 * @date  2006.04.06
 */
//============================================================================================

#ifndef __PERAPVOICE_H__
#define __PERAPVOICE_H__

#include "savedata/save_control.h"
#include "poke_tool/pokeparty.h"

//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief ぺラップ声データへの不完全型定義
 */
//---------------------------------------------------------------------------
typedef struct PERAPVOICE PERAPVOICE;

//---------------------------------------------------------------------------
/**
 * @brief   記録する声データの大きさ
 */
//---------------------------------------------------------------------------
#define PERAPVOICE_LENGTH   (1000)

#define PERAPVOICE_DATA_LENGTH  (PERAPVOICE_LENGTH+4)

//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
extern PERAPVOICE * PERAPVOICE_AllocWork(HEAPID heapID);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

extern PERAPVOICE * SaveData_GetPerapVoice(SAVE_CONTROL_WORK * sv);
extern int PERAPVOICE_GetWorkSize(void);
extern BOOL PERAPVOICE_GetExistFlag(const PERAPVOICE * pv);
extern void PERAPVOICE_ClearExistFlag( PERAPVOICE * pv );
extern void PERAPVOICE_SetExistFlag( PERAPVOICE * pv );  //交換時の巻き戻し専用
extern void PERAPVOICE_SetVoiceData(PERAPVOICE * pv, const s8 * src);
extern void PERAPVOICE_CopyData(PERAPVOICE * des, const PERAPVOICE * src);
extern const void * PERAPVOICE_GetVoiceData(const PERAPVOICE * pv);
extern void PERAPVOICE_ExpandVoiceData( s8 *des, const s8 *src );
extern void PERAPVOICE_CheckPerapInPokeParty( PERAPVOICE *pv, POKEPARTY *party );
extern int PERAPVOICE_GetWazaParam( PERAPVOICE* pv );

#endif  __PERAPVOICE_H__
