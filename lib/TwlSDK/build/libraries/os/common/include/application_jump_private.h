/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     application_jump_private.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef _APPLICATION_JUMP_PRIVATE_H_
#define _APPLICATION_JUMP_PRIVATE_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data-------------------------------------------

// Hot/Coldブート判定
#define OSi_GetMCUFreeRegisterValue()			( *(vu32*)HW_RESET_PARAMETER_BUF )

#define OS_MCU_RESET_VALUE_BUF_ENABLE_MASK		0x80000000
#define OS_MCU_RESET_VALUE_LEN					1

// プリロード時にromヘッダを置く場所
#define OS_TWL_HEADER_PRELOAD_MMEM			0x23e0000
#define OS_TWL_HEADER_PRELOAD_MMEM_END		(0x23e0000 + 0x4000)

// ランチャーパラメータにセットするブートタイプ
typedef enum LauncherBootType {
    LAUNCHER_BOOTTYPE_ILLEGAL = 0,          // 不正な状態
    LAUNCHER_BOOTTYPE_ROM,                  // ROM から起動
    LAUNCHER_BOOTTYPE_TEMP,                 // NANDのtmpフォルダ内のアプリを起動
    LAUNCHER_BOOTTYPE_NAND,                 // NAND 内のアプリを起動
    LAUNCHER_BOOTTYPE_MEMORY,               // メモリ上のアプリを起動

	LAUNCHER_BOOTTYPE_MAX
}LauncherBootType;

// ランチャーパラメータ　フラグ
typedef struct LauncherBootFlags {
	u16			isValid : 1;				// TRUE:valid, FALSE:invalid
	u16			bootType : 3;				// LauncherBootTypeの値を使用
	u16			isLogoSkip : 1;				// ロゴデモスキップ要求
	u16			isInitialShortcutSkip : 1;	// 初回起動シーケンススキップ要求
	u16			isAppLoadCompleted : 1;		// アプリロード済みを示す
	u16			isAppRelocate : 1;			// アプリ再配置要求
	u16			rsv : 8;
}LauncherBootFlags;


// ランチャーパラメータ　ヘッダ
typedef struct LauncherParamHeader {
	u32			magicCode;					// SYSM_LAUNCHER_PARAM_MAGIC_CODEが入る
	u8			version;					// タイプによってBodyを判別する。
	u8			bodyLength;					// bodyの長さ
	u16			crc16;						// bodyのCRC16
}LauncherParamHeader;


// ランチャーパラメータ　ボディ
typedef union LauncherParamBody {
	struct {								// ※とりあえず最初はTitlePropertyとフォーマットを合わせておく
		OSTitleId			prevTitleID;	// リセット前のタイトルID
		OSTitleId			bootTitleID;	// リセット後にダイレクト起動するタイトルID
		LauncherBootFlags	flags;			// リセット時のランチャー動作フラグ
		u8					rsv[ 6 ];		// 予約
	}v1;
}LauncherParamBody;


// ランチャーパラメータ
typedef struct LauncherParam {
	LauncherParamHeader	header;
	LauncherParamBody	body;
}LauncherParam;

// function's prototype------------------------------------

#ifdef SDK_ARM9
void OSi_InitPrevTitleId( void );
void OS_SetLauncherParamAndResetHardware( OSTitleId id, LauncherBootFlags *flag );


BOOL OSi_IsJumpFromSameMaker(void);
u32 OSi_GetInitialCode(void);
u32 OSi_GetPrevInitialCode(void);
BOOL OSi_CanArbitraryJumpTo(u32 initialCode);
BOOL OSi_JumpToArbitraryApplication(u32 initialCode);
OSTitleId OSi_GetPrevTitleId(void);

#else
BOOL OS_ReadLauncherParameter( LauncherParam *buf, BOOL *isHotstart );
#endif

#ifdef __cplusplus
}       // extern "C"
#endif

#endif  // _APPLICATION_JUMP_PRIVATE_H_
