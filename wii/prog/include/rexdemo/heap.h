/*---------------------------------------------------------------------------*
  Project:  NET Initialize demo
  File:	 heap.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: heap.h,v $
  Revision 1.1  2007/06/23 12:21:28  seiki_masashi
  Added REXDEMOCreateMEM1Heap, and so on.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef __DEMOS_SHARED_HEAP_H__
#define __DEMOS_SHARED_HEAP_H__

#include <revolution/types.h>
#include <revolution/ncd.h>
#include <revolution/so.h>

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================*/

#define REXDEMO_SOCKET_HEAPSIZE_DEFAULT	 (1024*128)

void*   REXDEMOAllocForSocket( u32 name, s32 size );
void	REXDEMOFreeForSocket( u32 name, void* ptr, s32 size );
BOOL	REXDEMOCreateHeapForSocket(u32 size);
void	REXDEMODestroyHeapForSocket( void );
SOLibraryConfig* REXDEMOGetSOLibConfig( void );

BOOL REXDEMOCreateMEM1Heap( u32 size );
void REXDEMODestroyMEM1Heap( void );
void* REXDEMOMEM1Alloc( s32 size );
void REXDEMOMEM1Free( void* ptr );

/*===========================================================================*/
#ifdef __cplusplus
}
#endif
#endif  /* __DEMOS_SHARED_HEAP_H__ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
