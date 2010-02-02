//============================================================================================
/**
 * @file	un_savedata.c
 * @brief	国連関連セーブデータ
 * @author	saito
 * @date	2010.02.02
 *
 */
//============================================================================================
#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/un_savedata_local.h"
#include "savedata/un_savedata.h"

//---------------------------------------------------------------------------
/**
 * @brief	仕掛け用ワークのサイズ取得
 * @return	int		サイズ
 */
//---------------------------------------------------------------------------
int UNSV_GetWorkSize(void)
{
	return sizeof(UNSV_WORK);
}

//---------------------------------------------------------------------------
/**
 * @brief	仕掛け用ワークの初期化
 * @param	gimmick		仕掛けワークへのポインタ
 */
//---------------------------------------------------------------------------
void UNSV_Init(UNSV_WORK * work)
{
	MI_CpuClear8(work, sizeof(UNSV_WORK));
}
#if 0
//----------------------------------------------------------
/**
 * @brief	入室する部屋の国コードセット
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	none
 */
//----------------------------------------------------------
void UNSV_SetCountryCode(const UNSV_WORK * inWork, const u8 inCountryCode)
{
  //@todo カントリーコード上限チェック

  //コードセット
  inWork->TargetCountryCode = inCountryCode;
}
#endif
//----------------------------------------------------------
/**
 * @brief	セーブワークへデータをコピー
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	none
 */
//----------------------------------------------------------
void SaveData_SaveUnsvWork(const UNSV_WORK * inWork, SAVE_CONTROL_WORK * sv)
{
  UNSV_WORK *work;
  work = SaveControl_DataPtrGet(sv, GMDATA_ID_UNSV);
	MI_CpuCopy32(inWork, work, sizeof(UNSV_WORK));
}

//----------------------------------------------------------
/**
 * @brief	ワークへロード
 * @param	sv			セーブデータ保持ワークへのポインタ
 * @return	none
 */
//----------------------------------------------------------
void SaveData_LoadUnsvWork(SAVE_CONTROL_WORK * sv, UNSV_WORK * outWork)
{
  UNSV_WORK *work;
  work = SaveControl_DataPtrGet(sv, GMDATA_ID_UNSV);
	MI_CpuCopy32(work, outWork, sizeof(UNSV_WORK));
}

