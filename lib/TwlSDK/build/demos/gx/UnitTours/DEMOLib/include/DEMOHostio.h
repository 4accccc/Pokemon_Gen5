/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOHostio.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-05-08 #$
  $Rev: 5892 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#ifndef DEMO_HOSTIO_H_
#define DEMO_HOSTIO_H_

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         DEMOMountHostIO

  Description:  IS-TWL-DEBUGGERのHostI/Oファイルシステムを
                "hostio:" という名前でFSにマウントする。

  Arguments:    basepath    基準となるデバッグホスト側のルートディレクトリ。
                            FSのパス名に "hostio:/relpath" と指定した場合
                            ホスト側の "basepath/relpath" を意味する。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOMountHostIO(const char *basepath);


#ifdef __cplusplus
}/* extern "C" */
#endif

/* DEMO_HOSTIO_H_ */
#endif
