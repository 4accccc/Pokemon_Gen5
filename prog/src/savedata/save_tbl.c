//==============================================================================
/**
 * @file	save.c
 * @brief	WBセーブ
 * @author	matsuda
 * @date	2008.08.27(水)
 */
//==============================================================================
#include <gflib.h>
#include <backup_system.h>
#include "savedata/save_tbl.h"
#include "savedata/contest_savedata.h"


//==============================================================================
//	定数定義
//==============================================================================
#define MAGIC_NUMBER	(0x31053527)
#define	SECTOR_SIZE		(SAVE_SECTOR_SIZE)
#define SECTOR_MAX		(SAVE_PAGE_MAX)

///ノーマル領域で使用するセーブサイズ
#define SAVESIZE_NORMAL		(SECTOR_SIZE * SECTOR_MAX)
///EXTRA1領域で使用するセーブサイズ
#define SAVESIZE_EXTRA1		(SECTOR_SIZE)
///EXTRA2領域で使用するセーブサイズ
#define SAVESIZE_EXTRA2		(SECTOR_SIZE)


static const GFL_SAVEDATA_TABLE SaveDataTbl_Normal[] = {
	{	//コンテスト
		GMDATA_ID_CONTEST,
//		SVBLK_ID_NORMAL,
		(FUNC_GET_SIZE)CONDATA_GetWorkSize,
		(FUNC_INIT_WORK)CONDATA_Init,
	},
};


//--------------------------------------------------------------
/**
 * @brief   セーブパラメータテーブル
 */
//--------------------------------------------------------------
const GFL_SVLD_PARAM SaveParam_Normal = {
	SaveDataTbl_Normal,
	NELEMS(SaveDataTbl_Normal),
	0,								//バックアップ領域先頭アドレス
	SAVESIZE_NORMAL,				//使用するバックアップ領域の大きさ
	MAGIC_NUMBER,
	GFL_BACKUP_NORMAL_FLASH,
};

