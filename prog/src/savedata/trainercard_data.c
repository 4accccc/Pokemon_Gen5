//=============================================================================
/**
 * @file	trainercard_data.c
 * @bfief	トレーナーカード用セーブデータアクセス関連
 * @author	Nozomu Saito
 *
 *
 */
//=============================================================================
#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/trainercard_data.h"

#define BADGE_MAX	(8)
#define SIGHN_W	(24)
#define SIGHN_H	(8)

#define DEFAULT_BADGE_SCRUCH	(140)

typedef struct TR_CARD_SV_DATA_tag
{
	u8 SignData[SIGHN_W*SIGHN_H*64/8];	//サイン面データ
}TR_CARD_SV_DATA;

//==============================================================================
/**
 * セーブデータサイズ取得
 *
 * @param	none
 *
 * @return	int		サイズ
 */
//==============================================================================
int TRCSave_GetSaveDataSize(void)
{
	return sizeof(TR_CARD_SV_DATA);
}

//==============================================================================
/**
 * セーブデータ初期化
 *
 * @param	outTrCard	トレーナーカードデータポインタ
 *
 * @return	none
 */
//==============================================================================
void TRCSave_InitSaveData(TR_CARD_SV_PTR outTrCard)
{
	GFL_STD_MemFill( outTrCard, 0, sizeof(TR_CARD_SV_DATA) );

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_TRCARD)
	SVLD_SetCrc(GMDATA_ID_TRCARD);
#endif //CRC_LOADCHECK
}

//==============================================================================
/**
 * セーブデータ先頭ポインタ取得
 *
 * @param	sv		セーブポインタ
 *
 * @return	TR_CARD_SV_PTR		トレーナーカードデータポインタ
 */
//==============================================================================
TR_CARD_SV_PTR TRCSave_GetSaveDataPtr(SAVE_CONTROL_WORK * sv)
{
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_TRCARD)
	SVLD_CheckCrc(GMDATA_ID_TRCARD);
#endif //CRC_LOADCHECK

	return SaveControl_DataPtrGet(sv, GMDATA_ID_TRCARD);
}



//==============================================================================
/**
 * サインデータ先頭ポインタ取得
 *
 * @param	inTrCard		トレーナーカードデータポインタ
 *
 * @return	u8*				サインデータポインタ
 */
//==============================================================================
u8 *TRCSave_GetSignDataPtr(TR_CARD_SV_PTR inTrCard)
{
	return inTrCard->SignData;
}

//==============================================================================
/**
 * @brief   サインが書き込まれているか確認する
 *
 * @param   inTrCard		
 *
 * @retval  int		TRUEなら書き込まれている。FALSEなら全て空白
 */
//==============================================================================
int TRCSave_GetSigned( TR_CARD_SV_PTR inTrCard )
{
	int search=0;
	int i;
	u32 *ptr = 	(u32*)inTrCard->SignData;	//サイン面データ

	
	for(i=0;i<SIGHN_W*SIGHN_H*64/(8*4);i++){
		search += ptr[i];
	}

	if(search){
		return TRUE;
	}
	
	return FALSE;
}
