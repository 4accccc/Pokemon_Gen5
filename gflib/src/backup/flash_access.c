//============================================================================================
/**
 * @file	flash_access.c
 * @brief
 * @author	tamada GAME FREAK inc.
 * @date	2007.06.12
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>

#include "assert.h"

#include "flash_access.h"

typedef	u16	CARD_ID;
//============================================================================================
//
//
//			ライブラリI/O部分
//
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	バックアップフラッシュの動作確認
 * @retval	TRUE	バックアップフラッシュが正常に動作する
 * @retval	FALSE	正常動作しない（存在しない、違う種類のバックアップデバイス等）
 */
//---------------------------------------------------------------------------
BOOL GFL_FLASH_Init(void)
{
	s32 lock_id;
	BOOL result;
	//u32 read_buf;

	lock_id = OS_GetLockID();
	GF_ASSERT(lock_id != OS_LOCK_ID_ERROR);
	CARD_LockBackup((CARD_ID)lock_id);

	if (CARD_IdentifyBackup(CARD_BACKUP_TYPE_FLASH_4MBITS)) {
		result = CARD_BACKUP_TYPE_FLASH_4MBITS;
	} else
	if (CARD_IdentifyBackup(CARD_BACKUP_TYPE_FLASH_2MBITS)) {
		result = CARD_BACKUP_TYPE_FLASH_2MBITS;
	} else {
		result = CARD_BACKUP_TYPE_NOT_USE;
	}

	CARD_UnlockBackup((CARD_ID)lock_id);
	OS_ReleaseLockID((CARD_ID)lock_id);

	if (result == CARD_BACKUP_TYPE_FLASH_4MBITS) {
		OS_TPrintf("Identified 4M FLASH\t");
	}else if (result == CARD_BACKUP_TYPE_FLASH_2MBITS) {
		OS_TPrintf("Identified 2M FLASH\t");
	}else{
		OS_TPrintf("No FLASH Identified\t");
	}

	OS_TPrintf("CardResult %d\n",CARD_GetResultCode());
	//4Mbit FLASH SectorSize == 0x10000 PageSize == 0x100 TotalSize == 0x80000
	//OS_TPrintf("SECTOR SIZE %x\n",CARD_GetBackupSectorSize());
	//OS_TPrintf("PAGE SIZE %x\n",CARD_GetBackupPageSize());
	//OS_TPrintf("TOTAL SIZE %x\n", CARD_GetBackupTotalSize());

	return (result != CARD_BACKUP_TYPE_NOT_USE);
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブ
 * @param	src		フラッシュのアドレス（０～）※セクタ指定ではない
 * @param	dst		書き込むデータのアドレス
 * @param	len		書き込むデータの長さ
 * @return	BOOL	TRUEで成功、FALSEで失敗
 */
//---------------------------------------------------------------------------
BOOL GFL_FLASH_Save(u32 src, void * dst, u32 len)
{
	u16 lock_id;
	BOOL result;
	lock_id = GFL_FLASH_SAVEASYNC_Init(src, dst, len);
	while (GFL_FLASH_SAVEASYNC_Main(lock_id, &result) == FALSE) {
		/* Do Nothing */
	}
	return result;
#if 0
	s32 lock_id;
	BOOL result;

	lock_id = OS_GetLockID();
	GF_ASSERT(lock_id != OS_LOCK_ID_ERROR);
	CARD_LockBackup((CARD_ID)lock_id);

//	result = CARD_WriteAndVerifyFlash(src, dst, len);
	CARD_WriteAndVerifyFlashAsync(src, dst, len, NULL, NULL);
	result = CARD_WaitBackupAsync();

	CARD_UnlockBackup((CARD_ID)lock_id);
	OS_ReleaseLockID((CARD_ID)lock_id);

	return result;
#endif
}
//---------------------------------------------------------------------------
/**
 * @brief	ロード
 * @param	src		フラッシュのアドレス（０～）※セクタ指定ではない
 * @param	dst		読み込み先アドレス
 * @param	len		読み込むデータの長さ
 * @return	BOOL	TRUEで成功、FALSEで失敗
 */
//---------------------------------------------------------------------------
BOOL GFL_FLASH_Load(u32 src, void * dst, u32 len)
{
	s32 lock_id;
	BOOL result;

	lock_id = OS_GetLockID();
	GF_ASSERT(lock_id != OS_LOCK_ID_ERROR);
	CARD_LockBackup((CARD_ID)lock_id);

	//result = CARD_ReadFlash(src, dst, len);
	CARD_ReadFlashAsync(src, dst, len, NULL, NULL);
	result = CARD_WaitBackupAsync();

	CARD_UnlockBackup((CARD_ID)lock_id);
	OS_ReleaseLockID((CARD_ID)lock_id);

#ifndef	DISABLE_FLASH_CHECK		//バックアップフラッシュなしでも動作
	if (!result) {
		//GFL_HEAP_FreeMemory(SvPointer);
		//GF_ASSERT(0);
		//BackupErrorWarningCall(HEAPID_BASE_SAVE);
	}
#endif

	return result;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static BOOL SaveEndFlag;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void SaveCallBack(void * src)
{
	SaveEndFlag = TRUE;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
u16 GFL_FLASH_SAVEASYNC_Init(u32 src, void * dst, u32 len)
{
	s32 lock_id;
	u32 buf;
	BOOL result;

	lock_id = OS_GetLockID();
	GF_ASSERT(lock_id != OS_LOCK_ID_ERROR);
	CARD_LockBackup((CARD_ID)lock_id);

#ifndef	DISABLE_FLASH_CHECK
	result = CARD_ReadFlash(0, &buf, sizeof(buf));
	if (!result) {
		/* セーブデータが読めない＝接触不良 */
		//PMSVLD_SaveError(lock_id, SAVEERROR_MSG_DISABLE_READ);
	}
#endif
	SaveEndFlag = FALSE;
	CARD_WriteAndVerifyFlashAsync(src, dst, len, SaveCallBack, NULL);
	return (CARD_ID)lock_id;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static BOOL GFL_FLASH_SAVEASYNC_Main(u16 lock_id, BOOL * result)
{
	if (SaveEndFlag == TRUE) {
		CARD_UnlockBackup(lock_id);
		OS_ReleaseLockID(lock_id);

#ifdef	DISABLE_FLASH_CHECK
		*result = TRUE;
#else
		switch (CARD_GetResultCode()) {
		case CARD_RESULT_SUCCESS:
			*result = TRUE;
			break;
		default:
			GF_ASSERT(0);
		case CARD_RESULT_TIMEOUT:
			*result = FALSE;
			/* セーブデータが書き込めない＝フラッシュ寿命か故障 */
			//PMSVLD_SaveError(lock_id, SAVEERROR_MSG_DISABLE_WRITE);
		case CARD_RESULT_NO_RESPONSE:
			*result = FALSE;
			/* CARD_RESULT_NO_RESPONSE＝＝接触不良 */
			//PMSVLD_SaveError(lock_id, SAVEERROR_MSG_DISABLE_READ);
			break;
		}
#endif
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
/**
 * @brief
 * @param	lock_id
 *
 */
//---------------------------------------------------------------------------
void GFL_FLASH_SAVEASYNC_Cancel(u16 lock_id)
{
    if(!CARD_TryWaitBackupAsync()){
        CARD_CancelBackupAsync();		//非同期処理キャンセルのリクエスト

        CARD_UnlockBackup(lock_id);
        OS_ReleaseLockID(lock_id);
    }
}


//---------------------------------------------------------------------------
/**
 * @brief	セーブ失敗によるカード異常
 * @param	lock_id		バスロックでシステムが返したID
 */
//---------------------------------------------------------------------------
static void PMSVLD_SaveError(u16 lock_id, int error_msg_id)
{
	//ロックしているバスを開放する（ロムが読めなくなるので）
	CARD_UnlockBackup(lock_id);
	OS_ReleaseLockID(lock_id);

	//セーブ失敗画面でセーブヒープを使用できるように開放する
	//GFL_HEAP_FreeMemory(SvPointer);

	//セーブ失敗画面呼び出し
	GF_ASSERT(0);
	//SaveErrorWarningCall(HEAPID_BASE_SAVE, error_msg_id);
}

