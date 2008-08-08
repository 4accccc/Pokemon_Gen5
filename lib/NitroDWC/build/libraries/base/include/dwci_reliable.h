/*---------------------------------------------------------------------------*
  Project:  NitroDWC
  File:     dwci_reliable.h

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/
#ifndef DWCi_RELIABLE_H_
#define DWCi_RELIABLE_H_

#include <base/dwc_base_gamespy.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* -----------------------------------------------------------------
      å^íËã`
      -------------------------------------------------------------------*/
    typedef GT2Result (*DWCiGT2SendFunc)( GT2Connection connection, const GT2Byte * message, int len, GT2Bool reliable );

    /* -----------------------------------------------------------------
      ì‡ïîä÷êî
      -------------------------------------------------------------------*/

    void DWCi_InitReliableQueue      ();
    void DWCi_CleanupReliableQueue   ();
    BOOL DWCi_IsReliableQueueFree    ();
    BOOL DWCi_ProcessReliableQueue   ();
    BOOL DWCi_ProcessReliableQueueEx ( DWCiGT2SendFunc func );
    BOOL DWCi_InsertReliableQueue    ( GT2Connection connection, const u8* buffer, int size );

#ifdef __cplusplus
}
#endif


#endif
