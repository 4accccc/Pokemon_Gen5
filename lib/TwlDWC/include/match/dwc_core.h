/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_CORE_H_
#define DWC_CORE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <match/dwc_common.h>
#ifndef DWC_NOGS
#include <match/dwc_login.h>
#include <match/dwc_datastorage.h>
#include <match/dwc_friend.h>
#include <match/dwc_match.h>
#include <match/dwc_transport.h>
#include <match/dwc_main.h>
#endif


#ifdef __cplusplus
}
#endif


#endif // DWC_CORE_H_
