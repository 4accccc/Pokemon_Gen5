/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     msJump.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-06-25#$
  $Rev: 6757 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_MACHINE_SETTING_JUMP_H_
#define TWL_OS_MACHINE_SETTING_JUMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_TWL
#ifdef SDK_ARM9
//----------------------------------------------------------------------------

#include <nitro.h>

/*---------------------------------------------------------------------------*
    定数
 *---------------------------------------------------------------------------*/
typedef enum OSTWLMSJumpDestination
{
    OS_TWL_MACHINE_SETTING_PAGE_1             =  0,
    OS_TWL_MACHINE_SETTING_PAGE_2             =  2,
    OS_TWL_MACHINE_SETTING_PAGE_3             =  3,
    OS_TWL_MACHINE_SETTING_PAGE_4             =  4,
    OS_TWL_MACHINE_SETTING_APP_MANAGER        =  5,
    OS_TWL_MACHINE_SETTING_WIRELESS_SW        =  6,
    OS_TWL_MACHINE_SETTING_BRIGHTNESS         =  7,
    OS_TWL_MACHINE_SETTING_USER_INFO          =  8,
    OS_TWL_MACHINE_SETTING_DATE               =  9,
    OS_TWL_MACHINE_SETTING_TIME               = 10,
    OS_TWL_MACHINE_SETTING_ALARM              = 11,
    OS_TWL_MACHINE_SETTING_TP_CALIBRATION     = 12,
    OS_TWL_MACHINE_SETTING_LANGUAGE           = 13,
    OS_TWL_MACHINE_SETTING_PARENTAL_CONTROL   = 14,
    OS_TWL_MACHINE_SETTING_NETWORK_SETTING    = 15,
    OS_TWL_MACHINE_SETTING_NETWORK_EULA       = 16,
    OS_TWL_MACHINE_SETTING_NETWORK_OPTION     = 17,
    OS_TWL_MACHINE_SETTING_COUNTRY            = 18,
    OS_TWL_MACHINE_SETTING_SYSTEM_UPDATE      = 19,
    OS_TWL_MACHINE_SETTING_SYSTEM_INITIALIZE  = 20,
    OS_TWL_MACHINE_SETTING_MAX
} OSTWLMSJumpDestination;

/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToMachineSetting

  Description:  TWL の本体設定の指定した項目にアプリジャンプする
  
  Arguments:    dest : 設定項目
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToMachineSetting(u8 dest);


/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToEulaDirect

  Description:  TWL の本体設定にアプリケーションジャンプして、
                "インターネット - 利用規約" を立ち上げます。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToEulaDirect( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToApplicationManagerDirect

  Description:  TWL の本体設定にアプリケーションジャンプして、
                "ソフト管理" を立ち上げます。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToApplicationManagerDirect( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToNetworkSettngDirect

  Description:  TWL の本体設定にアプリケーションジャンプして、
                "インターネット - 接続設定" を立ち上げます。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToNetworkSettngDirect( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToCountrySettingDirect

  Description:  TWL の本体設定にアプリケーションジャンプして、
                "Country" を立ち上げます。
                ※ 日本向けの TWL では同設定が存在しないため、本体設定の先頭にジャンプします。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToCountrySettingDirect( void );

/*---------------------------------------------------------------------------*
  Name:         OSi_JumpToSystemUpdateDirect

  Description:  TWL の本体設定にアプリケーションジャンプして、
                "本体の更新" を立ち上げます。
  
  Arguments:    なし
 
  Returns:      FALSE … 何らかの理由でアプリケーションジャンプに失敗
                ※ 処理が成功した場合、この関数中でリセット処理が発生するため
                   TRUE を返すことはありません。
 *---------------------------------------------------------------------------*/
BOOL OSi_JumpToSystemUpdateDirect( void );

#endif /* SDK_ARM9 */
#endif /* SDK_TWL */


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_SYSTEM_H_ */
#endif
