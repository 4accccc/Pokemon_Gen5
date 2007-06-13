//============================================================================================
/**
 * @file	flash_access.h
 * @brief	フラッシュアクセス用関数
 * @author	tamada GAME FREAK inc.
 * @date	2007.06.12
 *
 *
 */
//============================================================================================

#pragma	once

//---------------------------------------------------------------------------
//	フラッシュアクセス用関数
//	※デバッグ用途以外ではSaveData_～を呼ぶはずなので、通常は直接使わない。
//---------------------------------------------------------------------------
extern BOOL GFL_FLASH_Init(void);
extern BOOL GFL_FLASH_Save(u32 src, void * dst, u32 len);
extern BOOL GFL_FLASH_Load(u32 src, void * dst, u32 len);
extern u16 GFL_FLASH_SAVEASYNC_Init(u32 src, void * dst, u32 len);
extern BOOL GFL_FLASH_SAVEASYNC_Main(u16 lock_id, BOOL * result);

