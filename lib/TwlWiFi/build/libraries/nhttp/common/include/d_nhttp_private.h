/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     d_nhttp_private.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef __D_NHTTP_PRIVATE_H__
#define __D_NHTTP_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "d_def.h"
#include "nhttp.h"
#include "NHTTPlib_int.h"

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/

extern NHTTPError NHTTPi_DeleteConnection(NHTTPConnection* connection_p);

#ifdef __cplusplus
}
#endif

#endif // __D_NHTTP_PRIVATE_H__
