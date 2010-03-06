/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_NONPORT_H_
#define DWC_NONPORT_H_

#ifdef __cplusplus
extern "C"
{
#endif

// ‹@ŽíˆË‘¶’è‹`
//
#if defined(_NITRO) && SDK_VERSION_MAJOR > 4
#include <nonport/twl/dwc_nonport.h>
#elif defined(_NITRO) && SDK_VERSION_MAJOR < 5
#include <nonport/nitro/dwc_nonport.h>
#elif  defined(HW2)
#include <nonport/gamecube/dwc_nonport.h>
#elif  defined(RVL)
#include <nonport/revolution/dwc_nonport.h>
#elif defined(_WIN32)
#include <nonport/pc/dwc_nonport.h>
#elif defined(_LINUX)
#include <nonport/linux/dwc_nonport.h>
#else
#error "unknown platform"
#endif

// ‹¤’Ê’è‹`
//
#include <nonport/common/dwc_nonport.h>

#ifdef __cplusplus
}
#endif



#endif // DWC_NONPORT_H_
