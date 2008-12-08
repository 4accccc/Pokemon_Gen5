/*---------------------------------------------------------------------------*
  Project:  NET Initialize demo
  File:	 heap.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: heap.c,v $
  Revision 1.7  2007/06/23 12:21:28  seiki_masashi
  Added REXDEMOCreateMEM1Heap, and so on.

  Revision 1.6  2007/05/28 08:40:57  hirose_kazuki
  Small fix.

  Revision 1.5  2007/05/28 08:36:34  hirose_kazuki
  Added REXDEMODestroyHeapForSocket()

  Revision 1.4  2006/10/30 10:01:55  yasu
  Add comment for heap for socket.

  Revision 1.3  2006/08/29 07:19:20  adachi_hiroaki
  プレフィックス変更、そのほか整理

  Revision 1.2  2006/08/25 02:14:11  adachi_hiroaki
  ヒープの作成に失敗したらその旨表示するようにした

  Revision 1.1  2006/08/10 12:10:04  adachi_hiroaki
  機能追加


  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <revolution/os.h>
#include <revolution/mem.h>
#include "rexdemo/heap.h"

/*---------------------------------------------------------------------------*/

static MEMHeapHandle	heapHandleSocket = NULL;
static MEMHeapHandle	heapHandleMEM1   = NULL;


void* 
REXDEMOAllocForSocket( u32 name, s32 size )
{
#pragma unused( name )
	void* ptr;

	if (0 < size)
	{
		// This is thread-safe, because the MEM_HEAP_OPT_THREAD_SAFE
		// option is specified in MEMCreateExpHeapEx().
		ptr = MEMAllocFromExpHeapEx( heapHandleSocket, (u32) size, 32 );
		return ptr;
	}
	else
	{
		return NULL;
	}
}

void
REXDEMOFreeForSocket( u32 name, void* ptr, s32 size )
{
#pragma unused( name )

	if (ptr && 0 < size)
	{
		// This is thread-safe, because
		// the MEM_HEAP_OPT_THREAD_SAFE option is specified.
		MEMFreeToExpHeap( heapHandleSocket, ptr );
	}
}

BOOL
REXDEMOCreateHeapForSocket( u32 size )
{
	void*			arenaLo;
	void*			arenaHi;
	
	//
	// Initialize the heap for Alloc() and Free().
	//
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!! Heap area for socket library must be in MEM2 !!!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	arenaLo = OSGetMEM2ArenaLo();
	arenaHi = OSGetMEM2ArenaHi();
	
	if ((u32) arenaHi - (u32) arenaLo < size)
	{
		return FALSE;
	}
	heapHandleSocket = MEMCreateExpHeapEx( arenaLo, size, MEM_HEAP_OPT_THREAD_SAFE );
	if (heapHandleSocket == MEM_HEAP_INVALID_HANDLE)
	{
		OSReport("MEMCreateExpHeapEx failed.\n");
		return FALSE;
	}
	
	OSSetMEM2ArenaLo((u8*)arenaLo + size);
	return TRUE;
}

void
REXDEMODestroyHeapForSocket( void )
{
	s32   heapSize;
	void* heapPtr;
	
	if ( heapHandleSocket == NULL )
		return;
	
	heapSize = MEMGetHeapTotalSize(heapHandleSocket);
	heapPtr  = MEMDestroyExpHeap(heapHandleSocket);
	
	if ( heapPtr != NULL )
	{
		if ( (u32)OSGetMEM2ArenaLo() == (u32)heapPtr + heapSize )
		{
			OSSetMEM2ArenaLo(heapPtr);
		}
	}
	heapHandleSocket = NULL;
	
	return;
}


SOLibraryConfig*
REXDEMOGetSOLibConfig( void )
{
	static SOLibraryConfig libConfig = { REXDEMOAllocForSocket, REXDEMOFreeForSocket };
	if (heapHandleSocket)
	{
		return &libConfig;
	}
	else
	{
		return NULL;
	}
}

BOOL REXDEMOCreateMEM1Heap( u32 size )
{
	void*			arenaLo;
	void*			arenaHi;
	
	//
	// Initialize the heap for Alloc() and Free().
	//
	arenaLo = OSGetMEM1ArenaLo();
	arenaHi = OSGetMEM1ArenaHi();
	
	if ((u32) arenaHi - (u32) arenaLo < size)
	{
		return FALSE;
	}
	heapHandleMEM1 = MEMCreateExpHeapEx( arenaLo, size, MEM_HEAP_OPT_THREAD_SAFE );
	if (heapHandleMEM1 == MEM_HEAP_INVALID_HANDLE)
	{
		OSReport("MEMCreateExpHeapEx failed.\n");
		return FALSE;
	}
	
	OSSetMEM1ArenaLo((u8*)arenaLo + size);
	return TRUE;
}

void REXDEMODestroyMEM1Heap( void )
{
	s32   heapSize;
	void* heapPtr;
	
	if ( heapHandleMEM1 == NULL )
		return;
	
	heapSize = MEMGetHeapTotalSize(heapHandleMEM1);
	heapPtr  = MEMDestroyExpHeap(heapHandleMEM1);
	
	if ( heapPtr != NULL )
	{
		if ( (u32)OSGetMEM1ArenaLo() == (u32)heapPtr + heapSize )
		{
			OSSetMEM1ArenaLo(heapPtr);
		}
	}
	heapHandleMEM1 = NULL;
	
	return;
}

void* REXDEMOMEM1Alloc( s32 size )
{
	void* ptr;

	if (0 < size)
	{
		// This is thread-safe, because the MEM_HEAP_OPT_THREAD_SAFE
		// option is specified in MEMCreateExpHeapEx().
		ptr = MEMAllocFromExpHeapEx( heapHandleMEM1, (u32) size, 32 );
		return ptr;
	}
	else
	{
		return NULL;
	}
}

void REXDEMOMEM1Free( void* ptr )
{
	if (ptr)
	{
		// This is thread-safe, because
		// the MEM_HEAP_OPT_THREAD_SAFE option is specified.
		MEMFreeToExpHeap( heapHandleMEM1, ptr );
	}
}



/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
