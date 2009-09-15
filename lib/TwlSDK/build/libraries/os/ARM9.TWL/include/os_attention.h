/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_attention.h

  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_ATTENTION_H_
#define TWL_OS_ATTENTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>


/*---------------------------------------------------------------------------*
 * 定数定義
 *---------------------------------------------------------------------------*/
typedef enum
{
    SPEC_DEST_NONE,
    SPEC_DEST_KOREA,
//    SPEC_DEST_CHINA,

    SPEC_DEST_NUM
}SPEC_DEST;

typedef enum
{
    IMAGE_OBJ_01_CHR,       // 上画面キャラクタデータ
    IMAGE_OBJ_01_SCR,       // 上画面スクリーンデータ
    IMAGE_OBJ_02_CHR,       // 下画面キャラクタデータ
    IMAGE_OBJ_02_SCR,       // 下画面スクリーンデータ
    IMAGE_OBJ_PAL,           // パレットデータ

    IMAGE_OBJ_NUM
}IMAGE_OBJ_INDEX;


/*---------------------------------------------------------------------------*
  Name:         OS_ShowAttentionOfLimitedRom

  Description:  show string of attention to run limited mode on NITRO.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_ShowAttentionOfLimitedRom(void);
void OS_ShowAttentionOfLimitedRomChina(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsLimitedRomRunningOnTwl

  Description:  check running platform (only for Nitro-TWL limited mode)

  Arguments:    None

  Returns:      TRUE  : running on TWL
                FALSE : running on NITRO
 *---------------------------------------------------------------------------*/
BOOL OS_IsLimitedRomRunningOnTwl(void);


#ifdef __cplusplus
}
#endif

#endif // #ifndef TWL_OS_ATTENTION_H_

