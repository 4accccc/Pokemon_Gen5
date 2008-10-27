//======================================================================
/**
 *
 * @file	dlplay_data_main.c
 * @brief	セーブデータを読み込むところ
 * @author	ariizumi
 * @data	
 */
//======================================================================
#include <gflib.h>
#include <backup_system.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "savedata_def.h"

#include "../../pokemon_wb/prog/src/test/ariizumi/ari_debug.h"
#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_func.h"
#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_comm_func.h"

#include "dlplay_child_common.h"
#include "dlplay_data_main.h"
#include "dlplay_child_pt.h"

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

DLPLAY_DATA_DATA* DLPlayData_InitSystem( int heapID , DLPLAY_MSG_SYS *msgSys );
void			  DLPlayData_TermSystem( DLPLAY_DATA_DATA *d_data );

BOOL	DLPlayData_LoadDataFirst( DLPLAY_DATA_DATA *d_data );
BOOL	DLPlayData_SaveData( DLPLAY_DATA_DATA *d_data );

//初期化
DLPLAY_DATA_DATA* DLPlayData_InitSystem( int heapID , DLPLAY_MSG_SYS *msgSys )
{
	DLPLAY_DATA_DATA *d_data;

	d_data = GFL_HEAP_AllocClearMemory(  heapID, sizeof( DLPLAY_DATA_DATA ) );

	d_data->heapID_ = heapID;

	d_data->mainSeq_ = 0;
	d_data->subSeq_	 = 0;
	d_data->msgSys_	= msgSys;
	d_data->pBoxData_ = NULL;
	MATH_CRC16CCITTInitTable( &d_data->crcTable_ );	//CRC初期化

	{
		//ROMの識別を行う
		CARDRomHeader *headerData;
		s32 lockID = OS_GetLockID();

		CARD_Enable( TRUE );
		GF_ASSERT( lockID != OS_LOCK_ID_ERROR );
		CARD_LockRom( (u16)lockID );
		headerData = (CARDRomHeader*)CARD_GetRomHeader();
		CARD_UnlockRom( (u16)lockID );
		OS_ReleaseLockID( (u16)lockID );
		{
			char str[64];
			sprintf(str,"name:[%12s] code:[%08x]",headerData->game_name ,headerData->game_code);
			DLPlayFunc_PutString(str,d_data->msgSys_);
		}
		//FIX ME:正しい判別処理を入れる
		d_data->cardType_ = CARD_TYPE_PT;


		//一応カードの種類が確定してから初期化
		GFL_BACKUP_Init(GFL_HEAPID_APP, GFL_HEAPID_APP);
	}
	
	return d_data;
}

//開放
void	DLPlayData_TermSystem( DLPLAY_DATA_DATA *d_data )
{

}

//セーブデータの読み込み
BOOL	DLPlayData_LoadDataFirst( DLPLAY_DATA_DATA *d_data )
{
	//自前で読み込む・・・
	switch( d_data->cardType_ )
	{
	case CARD_TYPE_DP:
		break;

	case CARD_TYPE_PT:
		return DLPlayData_PT_LoadData( d_data );
		break;

	case CARD_TYPE_GS:
		break;
	}
	return FALSE;
}

BOOL	DLPlayData_SaveData( DLPLAY_DATA_DATA *d_data )
{
		//自前で書き込む・・・
	switch( d_data->cardType_ )
	{
	case CARD_TYPE_DP:
		break;

	case CARD_TYPE_PT:
		return DLPlayData_PT_SaveData( d_data );
		break;

	case CARD_TYPE_GS:
		break;
	}
	return FALSE;
}





