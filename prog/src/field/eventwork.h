//======================================================================
/**
 * @file	eventflag.h
 * @brief	セーブフラグ、ワーク操作
 * @author	Tamada
 * @author	Satoshi Nohara
 * @date	05.10.22
 */
//======================================================================
#ifndef	__EVENTFLAG_H__
#define	__EVENTFLAG_H__

#include <gflib.h>
//#include "system/savedata_def.h"


//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
/// SYS_FLAG_MODE
//--------------------------------------------------------------
typedef enum
{
  SYS_FLAG_MODE_SET = 0, ///<フラグセット
  SYS_FLAG_MODE_RESET, ///<フラグリセット
  SYS_FLAG_MODE_CHECK, ///<フラグチェック
}SYS_FLAG_MODE;

//======================================================================
//  struct
//======================================================================
//----------------------------------------------------------
/**
 * @brief	イベントワーク型定義
 */
//----------------------------------------------------------
typedef struct _EVENTWORK EVENTWORK;

//======================================================================
//  extern
//======================================================================
//----------------------------------------------------------
//	セーブデータシステムが依存する関数
//----------------------------------------------------------
extern int EVENTWORK_GetWorkSize(void);
extern void EVENTWORK_InitWork( u8 *work );
#if 0
extern void EVENTWORK_LoadEventWork(
    EVENTWORK *save, const EVENTWORK *load );
extern EVENTWORK * EVENTWORK_AllocWork( HEAPID heapID );
extern void EVENTWORK_FreeWork( EVENTWORK *evwk );
extern void EVENTWORK_Init(EVENTWORK * evwk);
#endif

//----------------------------------------------------------
//	EVENTWORK操作のための関数
//----------------------------------------------------------
extern BOOL EVENTWORK_CheckEventFlag( EVENTWORK * ev, u16 flag_no);
extern void EVENTWORK_SetEventFlag( EVENTWORK * ev, u16 flag_no);
extern void EVENTWORK_ResetEventFlag( EVENTWORK * ev, u16 flag_no);
extern void EVENTWORK_ClearEventFlags( EVENTWORK * ev, u16 start_no, u16 end_no );

extern u16 * EVENTWORK_GetEventWorkAdrs( EVENTWORK * ev, u16 work_no );
extern void EVENTWORK_ClearEventWorks( EVENTWORK * ev, u16 start_no, u16 end_no );

//----------------------------------------------------------
//    時間フラグのクリア
//----------------------------------------------------------
extern void EVENTWORK_ClearTimeFlags( EVENTWORK * ev );

//----------------------------------------------------------
//    時間更新処理リクエストの取り出し
//    ※現状、隠しアイテム復活用
//----------------------------------------------------------
extern BOOL EVENTWORK_PopTimeRequest( EVENTWORK * ev );

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
//extern EVENTWORK * SaveData_GetEventWork(SAVEDATA * sv);

#endif	//__EVENTFLAG_H__
