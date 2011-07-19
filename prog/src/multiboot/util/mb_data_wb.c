//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mb_data_wb.c
 *	@brief  WB用
 *	@author	Toru=Nagihashi
 *	@data		2011.07.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include <string.h>
#include <backup_system.h>
#include <nitro/card/backup_ex.h>
#include "system/gfl_use.h"
#include "system/save_error_warning.h"
#include "arc_def.h"

#include "poke_tool/poke_tool.h"
#include "multiboot/mb_data_main.h"
#include "mb_data_wb.h"
#include "wb_save.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
#ifdef PM_DEBUG
static u32 debug_seq = 0;
#endif

BOOL MB_DATA_WB_LoadData( MB_DATA_WORK *dataWork )
{
  enum
  {
    SEQ_INIT,
    SEQ_LOAD_MAIN,
    SEQ_LOAD_MIRROR,
    SEQ_CHECK_ILLIGAL,
    SEQ_CHECK_COUNTRY,
    SEQ_END,
  };

  const u32 saveSize = MB_DATA_GetSavedataSize( dataWork->cardType );
  switch( dataWork->subSeq )
  {
  case SEQ_INIT: //初期化およびデータ読み込み
    WB_SAVE_Init();
    {
#if DLPLAY_FUNC_USE_PRINT
    char str[256];
    sprintf(str,"Start load data. size:[%d]", saveSize );
    //DLPlayFunc_PutString(str,dataWork->msgSys );
#endif
    {
      s32 lockID = OS_GetLockID();
      GF_ASSERT_HEAVY( lockID != OS_LOCK_ID_ERROR );
      CARD_LockRom( (u16)lockID );
      CARD_CheckPulledOut();  //抜き検出
      CARD_UnlockRom( (u16)lockID );
      OS_ReleaseLockID( (u16)lockID );
    }
    dataWork->pData    = GFL_HEAP_AllocClearMemory(  dataWork->heapId, saveSize );
    dataWork->pDataMirror= GFL_HEAP_AllocClearMemory(  dataWork->heapId, saveSize );
  
    dataWork->lockID_ = OS_GetLockID();
    GF_ASSERT_HEAVY( dataWork->lockID_ != OS_LOCK_ID_ERROR );
    //WBは4Mbitフラッシュ
    CARD_LockBackup( (u16)dataWork->lockID_ );
#if PM_DEBUG
    if( dataWork->isDummyCard == TRUE )
    {
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
      {
        CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS );
      }
      else
      {
        CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS_IRC );
      }
    }
    else
#endif
    {
#if PM_DEBUG
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
      {
        CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS );
      }
      else
#endif
      {
        CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS_IRC );
      }
    }
    CARD_ReadFlashAsync( 0x0000 , dataWork->pData , saveSize , NULL , NULL );
    dataWork->subSeq++;
    }
    break;
  case SEQ_LOAD_MAIN: //ミラーリングデータ読み込み
    //読み込みの完了を待つ
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      if( CARD_GetResultCode() != CARD_RESULT_SUCCESS )
      {
        SaveErrorCall_Load();
      }
      //GF_ASSERT_HEAVY( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
      CARD_ReadFlashAsync( WB_MIRROR_SAVE_ADDRESS , dataWork->pDataMirror , saveSize , NULL , NULL );
      dataWork->subSeq++;
      
      //DLPlayFunc_PutString("Data1 load complete.",dataWork->msgSys );
    }

    break;
  case SEQ_LOAD_MIRROR: //読み込みの完了を待つ
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      if( CARD_GetResultCode() != CARD_RESULT_SUCCESS )
      {
        SaveErrorCall_Load();
      }
      //GF_ASSERT_HEAVY( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
      CARD_UnlockBackup( (u16)dataWork->lockID_ );
      OS_ReleaseLockID( (u16)dataWork->lockID_ );
      dataWork->subSeq++;

#ifdef PM_DEBUG
      {
        WB_PLAYER_DATA *player = MB_DATA_WB_GetSaveAddress( dataWork, WB_GMDATA_ID_PLAYER_DATA );
        WB_MYSTATUS *myStatus = &player->mystatus;
        MB_DATA_TPrintf("Name[%x %x %x]\n",myStatus->name[0],myStatus->name[1],myStatus->name[2]);
        MB_DATA_TPrintf("TrainerID[%d]\n",myStatus->id);
        MB_DATA_TPrintf("RegionCode[%d]\n",myStatus->region_code);
        MB_DATA_TPrintf("RomCode[%d]\n",myStatus->rom_code);

        debug_seq = 0;
      }
#endif
      
      //DLPlayFunc_PutString("Data2 load complete.",dataWork->msgSys );
    }
    break;
  case SEQ_CHECK_ILLIGAL: //各ブロックの正当性チェック
    {

#ifdef PM_DEBUG
#if 0
      if( !DEBUG_WB_SAVE_Dump( dataWork->pData, &debug_seq ) )
      {
        break;
      }
#endif
#endif

      {
        const BOOL isCorrectData = WB_SAVE_CheckIllegal( dataWork->pData );
        const BOOL isCorrectDataMirror = WB_SAVE_CheckIllegal( dataWork->pDataMirror );
        if( isCorrectData || isCorrectDataMirror )
        {
          if( isCorrectData )
          {
            dataWork->mainLoadPos = DDS_FIRST;
          }
          else
          {
            dataWork->mainLoadPos = DDS_SECOND;
          }

          //メインとミラーのCRCを保存
          {
            const WB_SAVE_FOOTER *cp_footer_main = WB_SAVE_GetBlockAddress( dataWork->pData, WB_GMDATA_ID_FOOTER );
            dataWork->cardCrcTable[WB_MAIN_CRC] = cp_footer_main->save_block_crc_tbl_crc;
          }
          {
            const WB_SAVE_FOOTER *cp_footer_sub = WB_SAVE_GetBlockAddress( dataWork->pDataMirror, WB_GMDATA_ID_FOOTER );
            dataWork->cardCrcTable[WB_SUB_CRC] = cp_footer_sub->save_block_crc_tbl_crc;
          }
        }
        else
        {
          dataWork->errorState = DES_MISS_LOAD_BACKUP;
        }
      }
    }
    dataWork->subSeq++;
    break;
  
  case SEQ_CHECK_COUNTRY: 
    //国コードチェック
    if( dataWork->errorState == DES_NONE )
    {
      WB_PLAYER_DATA *player = MB_DATA_WB_GetSaveAddress( dataWork, WB_GMDATA_ID_PLAYER_DATA );
      WB_MYSTATUS *myStatus = &player->mystatus;

      MB_DATA_TPrintf("RegionCode[%d]\n",myStatus->region_code);
      if( myStatus->region_code != PM_LANG )
      {
        dataWork->errorState = DES_ANOTHER_COUNTRY;
      }
    }
    dataWork->subSeq++;
    break;
  case 5:
    dataWork->subSeq = 0;
    return TRUE;
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  CRCを読み込み
 *
 *	@param	MB_DATA_WORK *dataWork ワーク
 *
 *	@return TRUE：CRCをチェック完了  FALSE：チェック中
 */
//-----------------------------------------------------------------------------
BOOL MB_DATA_WB_LoadRomCRC( MB_DATA_WORK *dataWork )
{
  switch( dataWork->subSeq )
  {
  case 0: //初期化およびデータ読み込み
    WB_SAVE_Init();
    {
      const u32 footerAdr = MB_DATA_GetStartAddress( WB_GMDATA_ID_FOOTER, dataWork->cardType );
      const u32 footerSize = WB_SAVE_GetBlockSize( WB_GMDATA_ID_FOOTER );
      dataWork->pDataCrcCheck = GFL_HEAP_AllocClearMemory( dataWork->heapId , footerSize );

      dataWork->lockID_ = OS_GetLockID();
      CARD_LockBackup( (u16)dataWork->lockID_ );

      CARD_ReadFlashAsync( 0x0000+footerAdr , dataWork->pDataCrcCheck , footerSize , NULL , NULL );
      dataWork->subSeq++;
    }
    break;
  case 1:
    //読み込みの完了を待つ
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      const u32 footerAdr = MB_DATA_GetStartAddress( WB_GMDATA_ID_FOOTER, dataWork->cardType );
      const u32 footerSize = WB_SAVE_GetBlockSize( WB_GMDATA_ID_FOOTER );
      const WB_SAVE_FOOTER *cp_footer = dataWork->pDataCrcCheck;
      dataWork->loadCrcTable[WB_MAIN_CRC] = cp_footer->save_block_crc_tbl_crc;

      CARD_ReadFlashAsync( WB_MIRROR_SAVE_ADDRESS+footerAdr , dataWork->pDataCrcCheck , footerSize, NULL , NULL );
      dataWork->subSeq++;
    }
    break;
  case 2: 
    //読み込みの完了を待つ
    if( CARD_TryWaitBackupAsync() == TRUE )
    {
      const WB_SAVE_FOOTER *cp_footer = dataWork->pDataCrcCheck;
      dataWork->loadCrcTable[WB_SUB_CRC] = cp_footer->save_block_crc_tbl_crc;

      if( CARD_GetResultCode() != CARD_RESULT_SUCCESS )
      {
        SaveErrorCall_Load();
      }
      //GF_ASSERT_HEAVY( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
      CARD_UnlockBackup( (u16)dataWork->lockID_ );
      OS_ReleaseLockID( (u16)dataWork->lockID_ );

      GFL_HEAP_FreeMemory( dataWork->pDataCrcCheck );
      dataWork->pDataCrcCheck = NULL;

      MB_DATA_TPrintf("LOAD CRC[%5d][%5d][%5d][%5d]\n",dataWork->loadCrcTable[0],dataWork->loadCrcTable[1],dataWork->loadCrcTable[2],dataWork->loadCrcTable[3]);
      MB_DATA_TPrintf("CARD CRC[%5d][%5d][%5d][%5d]\n",dataWork->cardCrcTable[0],dataWork->cardCrcTable[1],dataWork->cardCrcTable[2],dataWork->cardCrcTable[3]);
      dataWork->subSeq++;
    }
    break;
  case 3:
    dataWork->subSeq = 0;
    return TRUE;
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief　WBのセーブアドレスを取得
 *
 *	@param	const u16 id WB_GMDATA_ID
 *
 *	@return 開始アドレス
 */
//-----------------------------------------------------------------------------
u32 MB_DATA_WB_GetStartAddress( const u16 id )
{
  return WB_SAVE_GetBlockOffset( id );
}

//----------------------------------------------------------------------------
/**
 *	@brief  WBのアドレスを取得
 *
 *	@param	MB_DATA_WORK *dataWork　ワーク
 *	@param	u16 id  ID
 *
 *	@return アドレス
 */
//-----------------------------------------------------------------------------
void * MB_DATA_WB_GetSaveAddress( MB_DATA_WORK *dataWork, const u16 id )
{
  switch( dataWork->mainLoadPos )
  {
  case DDS_FIRST: 
    GF_ASSERT( dataWork->pData );
    return WB_SAVE_GetBlockAddress( dataWork->pData, id );
  case DDS_SECOND: 
    GF_ASSERT( dataWork->pDataMirror );
    return WB_SAVE_GetBlockAddress( dataWork->pDataMirror, id );
  default: 
    GF_ASSERT_HEAVY(0); 
    return NULL;
  }
}
