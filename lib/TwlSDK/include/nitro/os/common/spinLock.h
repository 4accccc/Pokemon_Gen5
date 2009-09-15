/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     spinLock.h

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

#ifndef NITRO_OS_SPINLOCK_H_
#define NITRO_OS_SPINLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

#ifndef SDK_TWL
#ifdef	SDK_ARM9
#include	<nitro/hw/ARM9/mmap_global.h>
#else  //SDK_ARM7
#include	<nitro/hw/ARM7/mmap_global.h>
#endif
#else // SDK_TWL
#include    <twl/hw/common/mmap_shared.h>
#ifdef	SDK_ARM9
#include	<twl/hw/ARM9/mmap_global.h>
#else  //SDK_ARM7
#include	<twl/hw/ARM7/mmap_global.h>
#endif
#endif //SDK_TWL

//======================================================================
//                      ロック変数
//
//・カード／カートリッジ／ＣＰＵ間共有内部ワークＲＡＭ／ＶＲＡＭ－Ｃ／Ｄ
//  などのＣＰＵ間での共有リスースや、ＣＰＵ間送信ＦＩＦＯ 等の専有リソース
//  の排他制御ためにこの関数をご使用下さい。
//======================================================================

// ロックＩＤ

#define OS_UNLOCK_ID            0      // ロック変数がロックされていない場合のＩＤ
#define OS_MAINP_LOCKED_FLAG    0x40   // メインプロセッサによるロック確認フラグ
#define OS_MAINP_LOCK_ID_START  0x40   // メインプロセッサ用ロックＩＤの割り当て開始番号
#define OS_MAINP_LOCK_ID_END    0x6f   //                               割り当て終了番号
#define OS_MAINP_DBG_LOCK_ID    0x70   //                               デバッガ予約番号
#define OS_MAINP_SYSTEM_LOCK_ID 0x7f   //                               システム予約番号
#define OS_SUBP_LOCKED_FLAG     0x80   //   サブプロセッサによるロック確認フラグ
#define OS_SUBP_LOCK_ID_START   0x80   //   サブプロセッサ用ロックＩＤの割り当て開始番号
#define OS_SUBP_LOCK_ID_END     0xaf   //                               割り当て終了番号
#define OS_SUBP_DBG_LOCK_ID     0xb0   //                               デバッガ予約番号
#define OS_SUBP_SYSTEM_LOCK_ID  0xbf   //                               システム予約番号

#define OS_LOCK_SUCCESS         0      // ロック成功
#define OS_LOCK_ERROR           (-1)   // ロックエラー

#define OS_UNLOCK_SUCCESS       0      // ロック解除成功
#define OS_UNLOCK_ERROR         (-2)   // ロック解除エラー

#define OS_LOCK_FREE            0      // ロック解除中

#define OS_LOCK_ID_ERROR        (-3)   // ロックＩＤエラー


//---- structure of lock variable 
typedef volatile struct OSLockWord
{
    u32     lockFlag;
    u16     ownerID;
    u16     extension;
}
OSLockWord;

/*---------------------------------------------------------------------------*
  Name:         OS_InitLock

  Description:  システムのロック変数や共有リソースへのアクセス権
                の設定を初期化します。

                カートリッジの排他制御のための領域はクリアされます。
                (その領域をデバッガが使用するため)

  Arguments:    なし

  Returns:      なし
 *---------------------------------------------------------------------------*/
void    OS_InitLock(void);

/*---------------------------------------------------------------------------*
  Name:         OS_LockByWord
                OS_LockCartridge
                OS_LockCard

  Description:  スピンロック関数

                プロセッサ間やモジュール間で共有リソースを排他制御するための
                ロック変数のスピンロックを行います。
                ロックが成功するまで試行し続けます。
                プロセッサ間の共有リソースは必ずロック後に使用して下さい。
                プロセッサ専有のリソースはタイミング的に調整できるのであれば
                ロックしなくても構いません。
                専有リソースはデバッグ時に限定してロックすることもできます。

  Arguments:    lockID              ロックID
                lockp               ロック変数のポインタ

  Returns:      >0                  ロックされている値
                OS_LOCK_SUCCESS     (=0) ロック成功
 *---------------------------------------------------------------------------*/
s32     OS_LockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void));
s32     OS_LockCartridge(u16 lockID);
s32     OS_LockCard(u16 lockID);

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockByWord
                OS_UnlockCartridge
                OS_UnlockCard

  Description:  ロックの解除
                ロックを解除し、共有リソースのアクセス権をサブプロセッサに渡します。
                ロックしていないモジュールが実行した場合には解除されず、
                 OS_UNLOCK_ERROR が返されます。

  Arguments:    lockID              ロックID
                lockp               ロック変数のポインタ

  Returns:      OS_UNLOCK_SUCCESS   ロック解除成功
                OS_UNLOCK_ERROR     ロック解除エラー
 *---------------------------------------------------------------------------*/
s32     OS_UnlockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void));
s32     OS_UnlockCartridge(u16 lockID);
s32     OS_UnlockCard(u16 lockID);

//---- 互換性のために以前の名称のものも残しています。('UnLock' <-> 'Unlock')
//     ISDライブラリがこれらの関数を呼んでいます。また、inline ではありません。
s32     OS_UnLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void));
s32     OS_UnLockCartridge(u16 lockID);
s32     OS_UnLockCard(u16 lockID);

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockByWord
                OS_TryLockCartridge
                OS_TryLockCard

  Description:  ロックの試行

                １回だけスピンロックを試行します。
                プロセッサ間の共有リソースは必ずロック後に使用して下さい。
                プロセッサ専有のリソースはタイミング的に調整できるのであれば
                ロックしなくても構いません。
                専有リソースはデバッグ時に限定してロックすることもできます。

  Arguments:    lockID              ロックID
                lockp               ロック変数のポインタ
                CtrlFuncp           リソース制御関数のポインタ

  Returns:      >0                  ロック中（直前に格納されていたID）
                OS_LOCK_SUCCESS     ロック成功
 *---------------------------------------------------------------------------*/
s32     OS_TryLockByWord(u16 lockID, OSLockWord *lockp, void (*crtlFuncp) (void));
s32     OS_TryLockCartridge(u16 lockID);
s32     OS_TryLockCard(u16 lockID);

/*---------------------------------------------------------------------------*
  Name:         OS_ReadOwnerOfLockWord
                OS_ReadOwnerOfLockCartridge
                OS_ReadOwnerOfLockCard

  Description:  ロック変数の所有モジュールIDの読み込み

                ロック変数の所有モジュールID を読み込みます。
                モジュールID が非0 の場合はその時点でどちらのプロセッサ側が
                所有権を持っているのかを確認できます。
                共有リソースの場合は「メインプロセッサ側が所有権を持っている状態」
                のみを割り込みを禁止することによって維持することができます。
                その他の状態はサブプロセッサが変化させてしまう可能性があります。
                所有モジュールID が 0 であってもロック変数が解除されているとは限りません。

                ※但し、メインメモリに対してはキャッシュ経由でなければ
                バイトアクセスはできないことに注意して下さい。
                ですので、メインメモリ上では基本的に OS_ReadOwnerOfLockWord() を使用して下さい。

  Arguments:    lockp       ロック変数のポインタ

  Returns:      所有モジュールID
 *---------------------------------------------------------------------------*/
u16     OS_ReadOwnerOfLockWord(OSLockWord *lockp);
#define OS_ReadOwnerOfLockCartridge()  OS_ReadOwnerOfLockWord( (OSLockWord *)HW_CTRDG_LOCK_BUF )
#define OS_ReadOwnerOfLockCard()       OS_ReadOwnerOfLockWord( (OSLockWord *)HW_CARD_LOCK_BUF  )

/*---------------------------------------------------------------------------*
  Name:         OS_GetLockID

  Description:  lockID を取得します。

  Arguments:    なし

  Returns:      OS_LOCK_ID_ERROR  ... ID取得に失敗
                (ARM9の場合)
                0x40～0x6f        ... lockID
                (ARM7の場合)
                0x80～0xaf        ... lockID

                IDは48種類までしか割り当てることができません。
                モジュール内にて複数のロック変数を制御する場合は
                できるだけ1つのIDを使用するようにして下さい。
 *---------------------------------------------------------------------------*/
s32     OS_GetLockID(void);


/*---------------------------------------------------------------------------*
  Name:         OS_ReleaseLockID

  Description:  ロックIDを解放します。

  Arguments:    lockID : 解放しようとするロックID

  Returns:      なし
 *---------------------------------------------------------------------------*/
void    OS_ReleaseLockID(u16 lockID);


#ifdef SDK_TWL
//================================================================
//   private function.
//   以下は 内部関数なのでアプリケーションからは使用しないでください。
#define OSi_SYNCTYPE_SENDER 0
#define OSi_SYNCTYPE_RECVER 1

#define OSi_SYNCVAL_NOT_READY 0
#define OSi_SYNCVAL_READY     1

void OSi_SyncWithOtherProc( int type, void* syncBuf );

static inline void OSi_SetSyncValue( u8 n )
{
	*(vu8*)(HW_INIT_LOCK_BUF+4) = n;
}
static inline u8 OSi_GetSyncValue( void )
{
	return *(vu8*)(HW_INIT_LOCK_BUF+4);
}

#endif


//--------------------------------------------------------------------------------
#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_SPINLOCK_H_ */
#endif
