/*---------------------------------------------------------------------------*
  Project:  TwlSDK - IRC - include
  File:     backup_ex.h

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
#ifndef NITRO_CARD_BACKUP_EX_H_
#define NITRO_CARD_BACKUP_EX_H_


#include <nitro/card/types.h>
#include <nitro/mi/dma.h>


#ifdef __cplusplus
extern  "C" {
#endif


/*---------------------------------------------------------------------------*/
/* constants */

// IRC の動的切り替え用
#define CARD_BACKUP_TYPE_VENDER_IRC		(0xFF)

// IRC バックアップデバイスタイプ
#define CARD_BACKUP_TYPE_EEPROM_4KBITS_IRC   (CARDBackupType)CARD_BACKUP_TYPE_DEFINE(EEPROM,  9, CARD_BACKUP_TYPE_VENDER_IRC)
#define CARD_BACKUP_TYPE_EEPROM_64KBITS_IRC  (CARDBackupType)CARD_BACKUP_TYPE_DEFINE(EEPROM, 13, CARD_BACKUP_TYPE_VENDER_IRC)
#define CARD_BACKUP_TYPE_EEPROM_512KBITS_IRC (CARDBackupType)CARD_BACKUP_TYPE_DEFINE(EEPROM, 16, CARD_BACKUP_TYPE_VENDER_IRC)
#define CARD_BACKUP_TYPE_FLASH_2MBITS_IRC    (CARDBackupType)CARD_BACKUP_TYPE_DEFINE(FLASH,  18, CARD_BACKUP_TYPE_VENDER_IRC)
#define CARD_BACKUP_TYPE_FLASH_4MBITS_IRC    (CARDBackupType)CARD_BACKUP_TYPE_DEFINE(FLASH,  19, CARD_BACKUP_TYPE_VENDER_IRC)
#define CARD_BACKUP_TYPE_FLASH_8MBITS_IRC    (CARDBackupType)CARD_BACKUP_TYPE_DEFINE(FLASH,  20, CARD_BACKUP_TYPE_VENDER_IRC)


#ifdef __cplusplus
}
#endif // extern "C"


#endif // NITRO_CARD_BACKUP_EX_H_
