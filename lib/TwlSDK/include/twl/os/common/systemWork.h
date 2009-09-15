/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     systemWork.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_COMMON_SYSTEMWORK_H_
#define TWL_OS_COMMON_SYSTEMWORK_H_


/* if include from Other Environment for exsample VC or BCB, */
/* please define SDK_FROM_TOOL                               */
#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))
//
//--------------------------------------------------------------------
#ifndef SDK_ASM

#include        <twl/types.h>
#include        <twl/hw/common/mmap_shared.h>
#ifdef SDK_TWL
#ifdef SDK_ARM9
#include        <twl/hw/ARM9/mmap_global.h>
#else //SDK_ARM7
#include        <twl/hw/ARM7/mmap_global.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_TWL

// マウントデバイス
typedef enum OSMountDevice {
    OS_MOUNT_DEVICE_SD   = 0,
    OS_MOUNT_DEVICE_NAND = 1,
    OS_MOUNT_DEVICE_MAX  = 2
}OSMountDevice;


// マウント対象
typedef enum OSMountTarget {
    OS_MOUNT_TGT_ROOT = 0,
    OS_MOUNT_TGT_FILE = 1,
    OS_MOUNT_TGT_DIR  = 2,
    OS_MOUNT_TGT_MAX  = 3
}OSMountTarget;


// パーミッション
typedef enum OSMountPermission {
    OS_MOUNT_USR_X = 0x01,
    OS_MOUNT_USR_W = 0x02,
    OS_MOUNT_USR_R = 0x04
}OSMountPermission;


// リソースの配置先
typedef enum OSMountResource {
    OS_MOUNT_RSC_MMEM = 0,
    OS_MOUNT_RSC_WRAM = 1
}OSMountResource;


#define OS_MOUNT_PARTITION_MAX_NUM      3           // マウント可能なパーティションのMax.No.
#define OS_MOUNT_DRIVE_START            'A'         // ドライブ名の先頭（大文字'A'-'Z'のみ指定可能）
#define OS_MOUNT_DRIVE_END              'Z'         // ドライブ名の最後
#define OS_MOUNT_ARCHIVE_NAME_LEN       16          // アーカイブ名のMax.length
#define OS_MOUNT_PATH_LEN               64          // パスのMax.length
#define OS_MOUNT_INFO_MAX               (size_t)((HW_TWL_FS_BOOT_SRL_PATH_BUF - HW_TWL_FS_MOUNT_INFO_BUF) / sizeof(OSMountInfo))

#define OS_TITLEIDLIST_MAX              118         // タイトルIDリストの最大保持数


// アーカイブマウント情報構造体
typedef struct OSMountInfo {
    u8      drive[ 1 ];
    u8      device : 3;
    u8      target : 2;
    u8      partitionIndex : 2;
    u8      resource : 1;
    u8      userPermission : 3;                         // ユーザーがRW可能かどうかを指定する
    u8      rsv_A : 5;
    u8      rsv_B;
    char    archiveName[ OS_MOUNT_ARCHIVE_NAME_LEN ];   // \0終端込みのサイズ
    char    path[ OS_MOUNT_PATH_LEN ];                  // \0終端込みのサイズ
}OSMountInfo;   // 84bytes


// タイトルIDリスト構造体
typedef struct OSTitleIDList {
	u8		num;
	u8		rsv[ 15 ];
	u8		publicFlag [ 16 ];	// publicセーブデータがあるかどうかのフラグ
	u8		privateFlag[ 16 ];	// privateセーブデータがあるかどうかのフラグ
	u8		appJumpFlag[ 16 ];	// アプリジャンプ可能かどうかのフラグ
	u8		sameMakerFlag[ 16 ];	// 同じメーカーかどうかのフラグ
	u64		TitleID[ OS_TITLEIDLIST_MAX ];
}OSTitleIDList; // 1024bytes

typedef struct OSHotBootStatus {
	u8		isDisable :1;
	u8		rsv :7;
}OSHotBootStatus;

/*---------------------------------------------------------------------------*
  Name:         OS_GetMountInfo

  Description:  マウント情報を取得します。

  Arguments:    なし

  Returns:      マウント情報リストの先頭ポインタを返します。
 *---------------------------------------------------------------------------*/
static inline const OSMountInfo *OS_GetMountInfo( void )
{
    return (const OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetBootSRLPath

  Description:  自分自身のSRLパス情報を取得します。

  Arguments:    なし

  Returns:      自分自身のSRLパスへのポインタを返します。
 *---------------------------------------------------------------------------*/
static inline const char *OS_GetBootSRLPath( void )
{
    return (const char *)HW_TWL_FS_BOOT_SRL_PATH_BUF;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetTitleId

  Description:  get application's title ID

  Arguments:    None

  Returns:      title ID as u64 value
 *---------------------------------------------------------------------------*/
static inline u64 OS_GetTitleId( void )
{
    return *(u64 *)(HW_TWL_ROM_HEADER_BUF + 0x230);
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetMakerCode

  Description:  get application's MakerCode

  Arguments:    None

  Returns:      maker code as u16 value
 *---------------------------------------------------------------------------*/
static inline u16 OS_GetMakerCode( void )
{
    return *(u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetSystemMenuVersionInfoContentID

  Description:  システムメニューバージョンのcontentID情報を取得します。

  Arguments:    なし

  Returns:      contentIDの先頭ポインタを返します。(NULL終端あり）
 *---------------------------------------------------------------------------*/
static inline const u8 *OSi_GetSystemMenuVersionInfoContentID( void )
{
    return (const u8 *)HW_SYSM_VER_INFO_CONTENT_ID;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetSystemMenuVersionLastGameCode

  Description:  システムメニューバージョンのゲームコードの最終byteを取得します。

  Arguments:    なし

  Returns:      ゲームコードの最終byteへのポインタを返します。
 *---------------------------------------------------------------------------*/
static inline u8 OSi_GetSystemMenuVersionInfoLastGameCode( void )
{
    return *(u8 *)HW_SYSM_VER_INFO_CONTENT_LAST_INITIAL_CODE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_IsEnableHotBoot

  Description:  HotBootフラグセット可能？

  Arguments:    なし

  Returns:      TRUE : HotBootセット抑制中、　FALSE：HotBootセットOK
 *---------------------------------------------------------------------------*/
static inline BOOL OSi_IsEnableHotBoot( void )
{
	return ( (OSHotBootStatus *)HW_SYSM_DISABLE_SET_HOTBOOT )->isDisable ? 0 : 1;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SetEnableHotBoot

  Description:  HotBootフラグセット可否制御

  Arguments:    isEnable -> TRUE : HotBootセットOK、　FALSE：HotBootセット抑制

  Returns:      なし
 *---------------------------------------------------------------------------*/
static inline void OSi_SetEnableHotBoot( BOOL isEnable )
{
	( (OSHotBootStatus *)HW_SYSM_DISABLE_SET_HOTBOOT )->isDisable = isEnable ? 0 : 1;
}


#endif // SDK_TWL

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif // SDK_ASM

#endif // SDK_FROM_TOOL

/* TWL_OS_COMMON_SYSTEMWORK_H_ */
#endif
