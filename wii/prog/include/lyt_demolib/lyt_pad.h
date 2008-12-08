/*--------------------------------------------------------------------------
  Project:  NW4R - LayoutLib - lyt
  File:     pad.h

  Copyright (C)2006-2008 Nintendo  All Rights Reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1.2 $
 *-------------------------------------------------------------------------*/

#ifndef NW4R_LYT_DEMOLIB_PAD_H_
#define NW4R_LYT_DEMOLIB_PAD_H_

#include <revolution/mem.h>
#include <revolution/kpad.h>
#include <nw4r/math.h>

namespace demo
{

void                InitPad();

void                ReadPad();

u32                 PadGetButtonHold(u32 i = 0);

u32                 PadGetButtonRelease(u32 i = 0);

u32                 PadGetButtonTrigger(u32 i = 0);

const nw4r::math::VEC2  PadGetPostion(u32 i = 0);

}   // namespace demo


#endif  // NW4R_LYT_DEMOLIB_PAD_H_
