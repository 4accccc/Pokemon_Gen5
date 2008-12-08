/*--------------------------------------------------------------------------
  Project:  NW4R - LayoutLib - lyt
  File:     heap.h

  Copyright (C)2006-2008 Nintendo  All Rights Reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1.2 $
 *-------------------------------------------------------------------------*/

#ifndef NW4R_LYT_DEMOLIB_HEAP_H_
#define NW4R_LYT_DEMOLIB_HEAP_H_

#include <revolution/mem.h>

namespace demo
{

extern MEMHeapHandle    gDemoHeap1;
extern MEMHeapHandle    gDemoHeap2;

MEMHeapHandle       InitHeap();

}   // namespace demo

#endif  // NW4R_LYT_DEMOLIB_HEAP_H_
