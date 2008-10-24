//======================================================================
/**
 *
 * @file	dlplay_child_pt.c	
 * @brief	wbダウンロードプレイ　プラチナ用機種別コード
 * @author	ariizumi
 * @data	08/10/23
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "pt_save.h"

#include "../../pokemon_wb/prog/src/test/ariizumi/ari_debug.h"
#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_func.h"
#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_comm_func.h"

#include "dlplay_data_main.h"
#include "dlplay_child_common.h"


#define PT_SAVE_SIZE (sizeof(SECTOR_SIZE * SECTOR_MAX))
//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================

BOOL	DLPlayData_PT_LoadData( DLPLAY_DATA_DATA *d_data );



//	データの読み込み
BOOL	DLPlayData_PT_LoadData( DLPLAY_DATA_DATA *d_data )
{
	switch( d_data->subSeq_ )
	{
	case 0:	//初期化およびデータ読み込み
		{

		char str[256];
		sprintf(str,"Start load data with PT size:[%d]",PT_SAVE_SIZE);
		DLPlayFunc_PutString(str,d_data->msgSys_ );

		d_data->pData_		= GFL_HEAP_AllocClearMemory(  d_data->heapID_, sizeof( _SAVEDATA_PT ) );
		d_data->pDataMirror_= GFL_HEAP_AllocClearMemory(  d_data->heapID_, sizeof( _SAVEDATA_PT ) );
	
		d_data->lockID_ = OS_GetLockID();
		GF_ASSERT( d_data->lockID_ != OS_LOCK_ID_ERROR );
		//プラチナは4MBフラッシュ
		CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS );
		CARD_LockBackup( (u16)d_data->lockID_ );
		CARD_ReadFlashAsync( 0x0000 , d_data->pData_ , PT_SAVE_SIZE , NULL , NULL );
		d_data->subSeq_++;
		}
		break;
	case 1:	//ミラーリングデータ読み込み
		//読み込みの完了を待つ
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			DLPlayFunc_PutString("Data1 load complete.",d_data->msgSys_ );
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
			CARD_ReadFlashAsync( 0x40000 , d_data->pDataMirror_ ,PT_SAVE_SIZE, NULL , NULL );
			d_data->subSeq_++;
		}

		break;
	case 2://CRCチェックおよび解析
		//読み込みの完了を待つ
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			DLPlayFunc_PutString("Data2 load complete.",d_data->msgSys_ );
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
			CARD_UnlockBackup( (u16)d_data->lockID_ );
			OS_ReleaseLockID( (u16)d_data->lockID_ );
			{
				//ここからCRCチェック
				const u16 crc1 = MATH_CalcCRC16CCITT( &d_data->crcTable_ , d_data->pData_ , PT_SAVE_SIZE );
				const u16 crc2 = MATH_CalcCRC16CCITT( &d_data->crcTable_ , d_data->pDataMirror_ , PT_SAVE_SIZE );

				GF_ASSERT(crc1==crc2);
				{
					char str[128];
					sprintf(str,"Check crc [%d][%d]",crc1,crc2);
					DLPlayFunc_PutString(str,d_data->msgSys_);
				}
			}
			d_data->subSeq_++;
		}
		break;
	}
	return FALSE;
}

