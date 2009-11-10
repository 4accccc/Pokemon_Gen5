//============================================================================================
/**
 * @file	gimmickwork.h
 * @brief	マップ固有の仕掛け用ワークをあつかうための公開ヘッダ
 * @author	tamada
 * @date	2006.02.01
 *
 * ＜使い方＞
 * - このワークはセーブワークで、ゾーン内ローカルなものです。マップをまたぐと初期化されます。
 * - マップに入ったタイミングで、GIMMICKWORK_Assignを使って認証を行います。
 * - 必要な場合は、GIMMICKWORK_Getでポインタを取得します。
 */
//============================================================================================
#ifndef	__GIMMICKWORK_H__
#define	__GIMMICKWORK_H__

#include "savedata/save_control.h"
#include "gamesystem/gamedata_def.h"  //GAMEDATA

#define GIMMICK_NO_ASSIGN (0xffffffff)

//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	仕掛け用ワークへの不完全型定義
 */
//---------------------------------------------------------------------------
typedef struct _GIMMICKWORK GIMMICKWORK;

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief   ギミックデータへのポインタ取得
 * @param	  gamedata			GAMEDATAへのポインタ
 * @return	ギミックデータへのポインタ
 *
 * 実態はsrc/gamesystem/game_data.cにあるが、情報隠蔽を
 * 考慮してアクセス関数群を扱うヘッダに配置している
 */
//----------------------------------------------------------
extern GIMMICKWORK * GAMEDATA_GetGimmickWork(GAMEDATA * gamedata);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
extern int GIMMICKWORK_GetWorkSize(void);
extern void GIMMICKWORK_Init(GIMMICKWORK * gimmick);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
extern void * GIMMICKWORK_Assign(GIMMICKWORK * gimmick, int gimmick_id);
extern void * GIMMICKWORK_Get(GIMMICKWORK * gimmick, int gimmick_id);
extern int GIMMICKWORK_GetAssignID(const GIMMICKWORK * gimmick);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//extern GIMMICKWORK * SaveData_GetGimmickWork(SAVE_CONTROL_WORK * sv);

extern void SaveData_SaveGimmickWork(const GIMMICKWORK * inGimmick, SAVE_CONTROL_WORK * sv);
extern void SaveData_LoadGimmickWork(SAVE_CONTROL_WORK * sv, GIMMICKWORK * outGimmick);

#endif	/* __GIMMICKWORK_H__ */
