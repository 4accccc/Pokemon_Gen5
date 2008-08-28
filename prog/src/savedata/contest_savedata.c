//==============================================================================
/**
 * @file	contest_savedata.c
 * @brief	コンテストセーブデータ操作
 * @author	matsuda
 * @date	2006.06.03(土)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/contest_savedata.h"



//==============================================================================
//	定数定義
//==============================================================================
///対戦成績記録最大数
#define CON_RECORD_COUNT_MAX		(9999)

#define CONTYPE_MAX					(5)
#define BREEDER_MAX					(4)

//==============================================================================
//	構造体定義
//==============================================================================
struct _CONTEST_DATA{
	u16	TuushinRecord[CONTYPE_MAX][BREEDER_MAX];	///<コンテスト通信対戦成績(CONTYPE, 順位)
};



//--------------------------------------------------------------
/**
 * @brief	カスタムボールのサイズを取得
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
u32	CONDATA_GetWorkSize(void)
{
	return sizeof(CONTEST_DATA);
}

//---------------------------------------------------------------------------
/**
 * @brief	コンテストセーブデータの初期化処理
 * @param	condata		コンテストセーブデータへのポインタ
 */
//---------------------------------------------------------------------------
void CONDATA_Init(void *work)
{
	CONTEST_DATA *condata = work;
	int ranking, type;
	
	for(type = 0; type < CONTYPE_MAX; type++){
		for(ranking = 0; ranking < BREEDER_MAX; ranking++){
			condata->TuushinRecord[type][ranking] = 0;
		}
	}
#if MATSU_MAKE_DEL
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_CONTEST)
	SVLD_SetCrc(GMDATA_ID_CONTEST);
#endif //CRC_LOADCHECK
#endif
}

//--------------------------------------------------------------
/**
 * @brief   対戦成績をセットする
 *
 * @param   condata		コンテストデータへのポインタ
 * @param   type		CONTYPE_???
 * @param   ranking		順位(0origin)
 */
//--------------------------------------------------------------
void CONDATA_RecordAdd(CONTEST_DATA *condata, int type, int ranking)
{
	if(condata->TuushinRecord[type][ranking] < CON_RECORD_COUNT_MAX){
		condata->TuushinRecord[type][ranking]++;
	}
#if MATSU_MAKE_DEL
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_CONTEST)
	SVLD_SetCrc(GMDATA_ID_CONTEST);
#endif //CRC_LOADCHECK
#endif
}

//--------------------------------------------------------------
/**
 * @brief   対戦成績を取得する
 *
 * @param   condata		コンテストデータへのポインタ
 * @param   type		CONTYPE_???
 * @param   ranking		取得する順位(0origin)
 *
 * @retval  対戦成績
 */
//--------------------------------------------------------------
u16 CONDATA_GetValue(CONTEST_DATA *condata, int type, int ranking)
{
#if MATSU_MAKE_DEL
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_CONTEST)
	SVLD_CheckCrc(GMDATA_ID_CONTEST);
#endif //CRC_LOADCHECK
#endif
	return condata->TuushinRecord[type][ranking];
}

