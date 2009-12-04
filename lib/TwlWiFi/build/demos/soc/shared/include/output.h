/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SOC - shared
  File:     output.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#if !defined(NITROWIFI_SOC_SHARED_OUTPUT_H_)
#define NITROWIFI_SOC_SHARED_OUTPUT_H_

#include <nitro.h>

#ifdef __cplusplus

extern "C"
{
#endif

/*****************************************************************************/

/* function */

/*---------------------------------------------------------------------------*
  Name:         InitDebugOutputHook

  Description:  デバッグ出力をサブ画面にも出力するように初期化.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    InitDebugOutputHook(void);

/*---------------------------------------------------------------------------*
  Name:         DebugOutput

  Description:  デバッグ出力をフックしてログに追加.

  Arguments:    s                出力文字列.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DebugOutput(const char* s);

/*---------------------------------------------------------------------------*
  Name:         CurrentOutput

  Description:  カレント内容をこっそりセット.

  Arguments:    s                出力文字列.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CurrentOutput(const char* s);

/*---------------------------------------------------------------------------*
  Name:         FlushDebugOutput

  Description:  フックしたデバッグ出力ログをサブ画面に反映.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FlushDebugOutput(void);

#ifdef __cplusplus

} /* extern "C" */
#endif

#endif /* NITROWIFI_SOC_SHARED_OUTPUT_H_ */
