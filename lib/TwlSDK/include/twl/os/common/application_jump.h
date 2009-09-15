/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     application_jump.h

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

#ifndef _APPLICATION_JUMP_H_
#define _APPLICATION_JUMP_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data-------------------------------------------

#define OS_MCU_RESET_VALUE_BUF_HOTBT_MASK		0x00000001
#define OS_MCU_RESET_VALUE_OFS					0

typedef enum OSAppJumpType{
    OS_APP_JUMP_NORMAL = 0,
    OS_APP_JUMP_TMP = 1
}OSAppJumpType;

// TMPアプリのパス
#define OS_TMP_APP_PATH			"nand:/<tmpjump>"


// function's prototype------------------------------------
#ifdef SDK_ARM9
/*---------------------------------------------------------------------------*
  Name:         OS_JumpToSystemMenu

  Description:  ハードウェアリセットを行い、システムメニューにジャンプします。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToSystemMenu( void );

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToInternetSetting

  Description:  ハードウェアリセットを行い、TWL 本体設定のインターネット-接続設定
                にジャンプします。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToInternetSetting(void);

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToEULAViewer

  Description:  ハードウェアリセットを行い、TWL 本体設定のインターネット-利用規約
                にジャンプします。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToEULAViewer(void);

/*---------------------------------------------------------------------------*
  Name:         OS_JumpToWirelessSetting

  Description:  ハードウェアリセットを行い、TWL 本体設定の 無線通信 にジャンプします。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OS_JumpToWirelessSetting(void);

/*---------------------------------------------------------------------------*
  Name:         OS_RebootSystem

  Description:  ハードウェアリセットを行い、自分自身を起動します。
  
  Arguments:    None
 
  Returns:      FALSE … NITRO 上で実行、もしくは再起動処理に失敗した
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OS_RebootSystem( void );

/*---------------------------------------------------------------------------*
  Name:         OS_IsBootFromSystemMenu

  Description:  システムメニューから起動されたかを判定。
  
  Arguments:    なし
 
  Returns:      TRUE システムメニューから起動
                FALSE それ以外のアプリケーションから起動
 *---------------------------------------------------------------------------*/
BOOL OS_IsBootFromSystemMenu(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsTemporaryApplication

  Description:  自分自身が TMP アプリであるかをチェックします。
  
  Arguments:    None
 
  Returns:      TRUE  … TMP アプリ
                FALSE … TMP アプリ以外
 *---------------------------------------------------------------------------*/
BOOL OS_IsTemporaryApplication(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsRebooted

  Description:  OS_RebootSystem による再起動が行われたかをチェックします
  
  Arguments:    None
 
  Returns:      TRUE  … 1回以上再起動が行われた
                FALSE … 初回起動
 *---------------------------------------------------------------------------*/
BOOL OS_IsRebooted( void );

// ---- These are internal functions, so don't use for application.
BOOL OS_ReturnToPrevApplication( void );
BOOL OSi_CanApplicationJumpTo( OSTitleId titleID );
OSTitleId OSi_GetPrevTitleId( void );
BOOL OS_DoApplicationJump( OSTitleId id, OSAppJumpType jumpType );
#endif

#ifdef __cplusplus
}       // extern "C"
#endif

#endif  // _APPLICATION_JUMP_H_
