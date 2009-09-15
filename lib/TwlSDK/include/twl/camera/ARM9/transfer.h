/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera - include
  File:     transfer.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_CAMERA_TRANSFER_H_
#define TWL_CAMERA_TRANSFER_H_

#include <twl/types.h>
#include <twl/mi/common/dma.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         CAMERA_DmaRecv

  Description:  receiving a frame data from CAMERA buffer.
                Sync version.

  Arguments:    dmaNo   : NDMA channel No. (0 - 3)
                dest    : destination address
                unit    : transfer length at once (byte)(width * lines at once)
                length  : transfer length (byte) (frame size)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CAMERA_DmaRecv(u32 dmaNo, void *dest, u32 unit, u32 length)
{
    MI_Camera_NDmaRecv( dmaNo, dest, unit/4, length, FALSE );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_DmaRecvAsync

  Description:  receiving a frame data from CAMERA buffer.
                Async version.

  Arguments:    dmaNo   : NDMA channel No. (0 - 3)
                dest    : destination address
                unit    : transfer length at once (byte)(width * lines at once)
                length  : transfer length (byte) (frame size)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CAMERA_DmaRecvAsync(u32 dmaNo, void *dest, u32 unit, u32 length, MINDmaCallback callback, void* arg)
{
    MI_Camera_NDmaRecvAsync( dmaNo, dest, unit/4, length, FALSE, callback, arg );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_DmaPipeInfinity

  Description:  receiving data from CAMERA buffer.
                Once starting DMA, it will transfer every unit data to same
                destination address.
                You should call MIi_StopExDma(dmaNo) to stop

  Arguments:    dmaNo   : NDMA channel No. (0 - 3)
                dest    : destination address
                unit    : transfer length at once (byte)(width * lines at once)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CAMERA_DmaPipeInfinity(u32 dmaNo, void *dest, u32 unit, MINDmaCallback callback, void* arg)
{
    // continuousモードなので、sizeにあたる値は無視される
    MI_Camera_NDmaRecvAsync( dmaNo, dest, unit/4, unit, TRUE, callback, arg );
}

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_TRANSFER_H_ */
