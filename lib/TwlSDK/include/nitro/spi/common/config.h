/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - include
  File:     config.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_SPI_COMMON_CONFIG_H_
#define NITRO_SPI_COMMON_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/spec.h>

// userInfo定義
//#if ( SDK_NVRAM_FORMAT >= 300 )
#include <nitro/spi/common/userInfo_ts_300.h>
//#else
//#include <nitro/spi/common/userInfo_ts_200.h>
//#endif

// NVRAM内のuserInfo格納位置
//#ifdef  SDK_TS
#undef  NVRAM_CONFIG_CONST_ADDRESS     // ユーザー情報の位置はヘッダのオフセットから取得する
//#else
//#define NVRAM_CONFIG_CONST_ADDRESS     // ユーザー情報の位置を固定アドレスに
//#endif

//#ifdef  NVRAM_CONFIG_CONST_ADDRESS
//#define NVRAM_CONFIG_DATA_ADDRESS_DUMMY     0x3fe00
//#else
#define NVRAM_CONFIG_DATA_OFFSET_ADDRESS    0x0020
#define NVRAM_CONFIG_DATA_OFFSET_SIZE       2
#define NVRAM_CONFIG_DATA_OFFSET_SHIFT      3
//#endif

#define NVRAM_CONFIG_SAVE_COUNT_MAX         0x0080      // NitroConfigData.saveCountの最大値
#define NVRAM_CONFIG_SAVE_COUNT_MASK        0x007f      // NitroConfigData.saveCountの値の範囲をマスクする。(0x00-0x7f）

// NVRAM内のMACアドレス格納位置
#define NVRAM_CONFIG_MACADDRESS_ADDRESS     0x00036
#define NVRAM_CONFIG_ENABLECHANNEL_ADDRESS  0x0003c

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_SPI_COMMON_CONFIG_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
