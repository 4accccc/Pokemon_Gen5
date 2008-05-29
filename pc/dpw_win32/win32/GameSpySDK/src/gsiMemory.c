// THIS FILE ONLY INCLUDED WHEN SDKS USE GAMESPY MEMORY MANAGER
#ifdef GSI_MEM_MANAGED


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "gsiMemory.h"
#include "gsiDebug.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define MEMFILL_POOL    0xFC   // free memory
#define MEMFILL_ALLOC	0xFA   // uninitialized
#define MEMFILL_FREED   0xFD   // deleted memory

#define MIN_BLOCK_SIZE  16     // minimum block size


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Block List management
void gsiMemBlockSetPrev(struct GSIMemoryBlock* pThis, struct GSIMemoryBlock* theBlock) { pThis->mPrev = theBlock; }
void gsiMemBlockSetNext(struct GSIMemoryBlock* pThis, struct GSIMemoryBlock* theBlock) { pThis->mNext = theBlock; }

struct GSIMemoryBlock* gsiMemBlockGetPrev(struct GSIMemoryBlock* pThis) { return pThis->mPrev; }
struct GSIMemoryBlock* gsiMemBlockGetNext(struct GSIMemoryBlock* pThis) { return pThis->mNext; }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Block Data management
gsi_i32 gsiMemBlockGetObjectSize(struct GSIMemoryBlock* pThis)                  { return pThis->mObjectSize;    }
void    gsiMemBlockSetObjectSize(struct GSIMemoryBlock* pThis, gsi_i32 theSize) { pThis->mObjectSize = theSize; }
	
// Return a ptr to the object this block contains
void* gsiMemBlockGetObjectPtr(struct GSIMemoryBlock* pThis)  { return (void*)((char*)pThis+sizeof(struct GSIMemoryBlock)); }

// Return total memory needed for this block (sizeof data + sizeof block)
gsi_i32 gsiMemBlockGetTotalSize(struct GSIMemoryBlock* pThis) { return pThis->mObjectSize + sizeof(struct GSIMemoryBlock); }

// Get the next block in memory (NOT the next block in the list)
struct GSIMemoryBlock* gsiMemBlockGetLinearNext(struct GSIMemoryBlock* pThis) 
{ 
	if (gsiMemBlockIsFlagged(pThis, BlockFlag_Final))
		return NULL;
	else
		return (struct GSIMemoryBlock*)((char*)pThis+gsiMemBlockGetTotalSize(pThis)); 
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Block flag management
int  gsiMemBlockIsFlagged(struct GSIMemoryBlock* pThis, gsi_u32 theFlag) { return ((pThis->mFlags&theFlag) == theFlag ? 1:0); }
void gsiMemBlockSetFlag  (struct GSIMemoryBlock* pThis, gsi_u32 theFlag) { pThis->mFlags |= theFlag;  }
void gsiMemBlockClearFlag(struct GSIMemoryBlock* pThis, gsi_u32 theFlag) { pThis->mFlags &= ~theFlag; }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Partitions memory block for optimal usage (?)
// Allocates GSIMemory object
GSIMemoryMgrPtr gsMemMgrCreate(void* thePoolStart, size_t thePoolSize)
{
	struct GSIMemoryBlock* aFirstBlock;
	struct GSIMemoryBlock* aSecondBlock;

	assert(gMemoryMgr == NULL);
	assert(thePoolSize > sizeof(struct GSIMemoryBlock) + 
		sizeof(struct GSIMemoryMgr));

	// Initialize thread protection
	gsiInitializeCriticalSection(&gMemCrit);
	gsiEnterCriticalSection(&gMemCrit);

	// Fill memory
	memset(thePoolStart, MEMFILL_POOL, thePoolSize);
	
	// Create the first block with GSIMemory as the only object
	aFirstBlock = (struct GSIMemoryBlock*)thePoolStart;
	memset(aFirstBlock, 0, sizeof(struct GSIMemoryBlock));
	//aFirstBlock->GSIMemoryBlock::GSIMemoryBlock();
	gsiMemBlockSetObjectSize(aFirstBlock, sizeof(struct GSIMemoryMgr));
	gsiMemBlockSetFlag(aFirstBlock, BlockFlag_Used);

	// Set the next block to the rest of the memory pool
	aSecondBlock = (struct GSIMemoryBlock*)gsiMemBlockGetLinearNext(aFirstBlock);
	memset(aSecondBlock, 0, sizeof(struct GSIMemoryBlock));
	//aSecondBlock->GSIMemoryBlock::GSIMemoryBlock();
	gsiMemBlockSetObjectSize(aSecondBlock, thePoolSize - gsiMemBlockGetTotalSize(aFirstBlock) - sizeof(struct GSIMemoryBlock));
	gsiMemBlockSetFlag(aSecondBlock, BlockFlag_Final);

	// Create a memory manager in the area reserved by the first block
	gMemoryMgr = (struct GSIMemoryMgr*)gsiMemBlockGetObjectPtr(aFirstBlock);
	memset(gMemoryMgr, 0, sizeof(struct GSIMemoryMgr));
	//gMemoryMgr->GSIMemory::GSIMemory();

	// Create the list of used and free blocks
	gMemoryMgr->mFirstUsedBlock = aFirstBlock;
	gMemoryMgr->mLastUsedBlock  = NULL;
	gMemoryMgr->mFirstFreeBlock = aSecondBlock;
	gMemoryMgr->mLastFreeBlock  = NULL;

	gMemoryMgr->mPoolStart = thePoolStart;
	gMemoryMgr->mPoolSize = thePoolSize;

	// Set initial memory usage  (2 block headers, 1 used for a GSIMemoryMgr)
	gMemoryMgr->mMemUsed = (2*sizeof(struct GSIMemoryBlock)+
		                            sizeof(struct GSIMemoryMgr));
	if (gMemoryMgr->mMemUsed > gMemoryMgr->mPeakMemoryUsage)
		gMemoryMgr->mPeakMemoryUsage = gMemoryMgr->mMemUsed;

	gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"GSIMemory created: %d byte allocation pool\r\n", gMemoryMgr->mPoolSize);

	gsiLeaveCriticalSection(&gMemCrit);
	return (GSIMemoryMgrPtr)gMemoryMgr;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsMemMgrDestroy()
{
	assert(gMemoryMgr != NULL);

	// Reset memmgr
	gsiDeleteCriticalSection(&gMemCrit);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsiMemBlockRemoveFromList(struct GSIMemoryBlock* theBlock,
						 struct GSIMemoryBlock** theListHead,
						 struct GSIMemoryBlock** theListTail)
{
	assert(theBlock != NULL);
	//assert(theListHead != NULL);
	//assert(theListTail != NULL);

	if (*theListHead == theBlock)
		*theListHead = gsiMemBlockGetNext(theBlock);

	if (*theListTail == theBlock)
		*theListTail = gsiMemBlockGetPrev(theBlock);

	if (gsiMemBlockGetNext(theBlock) != NULL)
		gsiMemBlockSetPrev(gsiMemBlockGetNext(theBlock), gsiMemBlockGetPrev(theBlock));
	if (gsiMemBlockGetPrev(theBlock) != NULL)
		gsiMemBlockSetNext(gsiMemBlockGetPrev(theBlock), gsiMemBlockGetNext(theBlock));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsiMemBlockAddToList(struct GSIMemoryBlock* theBlock,
					struct GSIMemoryBlock** theListHead,
					struct GSIMemoryBlock** theListTail)
{
	assert(theBlock != NULL);

	// Check empty list
	if (*theListHead == NULL)
	{
		*theListHead = theBlock;
		gsiMemBlockSetNext(theBlock, NULL);
		gsiMemBlockSetPrev(theBlock, NULL);
	}
	else
	{
		// Check single item list
		if (*theListTail == NULL)
			gsiMemBlockSetPrev(theBlock, *theListHead);
		else
			gsiMemBlockSetPrev(theBlock, *theListTail);

		gsiMemBlockSetNext(theBlock, NULL);
		gsiMemBlockSetNext(gsiMemBlockGetPrev(theBlock), theBlock);
		*theListTail = theBlock;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsiMemBlockAssertValid(struct GSIMemoryBlock* pThis)
{
	int inUse = gsiMemBlockIsFlagged(pThis, BlockFlag_Used);
	int aOS   = gsiMemBlockGetObjectSize(pThis);
	gsi_u8* aByte = (gsi_u8*)gsiMemBlockGetObjectPtr(pThis);
	int i=0;

	// Nothing to verify if block is in use
	if (inUse)
		return;

	// Verify the fill character
	for (; i<aOS; i++)
	{
		assert(aByte[i] == MEMFILL_FREED || aByte[i] == MEMFILL_POOL);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Find/Create a block for request size
//    - may return NULL if no match is found
//    - may return larger block than necessary
struct GSIMemoryBlock* gsiMemGetFreeBlock(struct GSIMemoryMgr* pThis, gsi_i32 theRequestedSize)
{
	struct GSIMemoryBlock* aBlock = NULL;

	// Restrict block sizes to a multiple of the MIN_BLOCK_SIZE
	gsi_i32 anAdjustedSize = theRequestedSize;
	if ((theRequestedSize % MIN_BLOCK_SIZE) != 0)                                
		anAdjustedSize += MIN_BLOCK_SIZE - (theRequestedSize % MIN_BLOCK_SIZE);  

	// Search through the list of blocks
	aBlock = pThis->mFirstFreeBlock;
	while(aBlock != NULL)
	{
		assert(!gsiMemBlockIsFlagged(aBlock, BlockFlag_Used));  // shouldn't be in the free list if it's being used

		// If block is large enough, try to use it
		if (gsiMemBlockGetObjectSize(aBlock) >= anAdjustedSize)
		{
			// Is the block large enough to partition?
			if (gsiMemBlockGetObjectSize(aBlock) >= 
				        (gsi_i32)(anAdjustedSize + sizeof(struct GSIMemoryBlock) + MIN_BLOCK_SIZE))
			{
				// Shrink the current block to the requested size
				gsi_i32 anOldSize = gsiMemBlockGetObjectSize(aBlock);
				int     wasFinal  = gsiMemBlockIsFlagged(aBlock, BlockFlag_Final);
				struct GSIMemoryBlock* aNewBlock = NULL;

				gsiMemBlockSetObjectSize(aBlock, anAdjustedSize);
				gsiMemBlockSetFlag      (aBlock, BlockFlag_Used);
				gsiMemBlockClearFlag    (aBlock, BlockFlag_Final);

				// Remove From Free list
				gsiMemBlockRemoveFromList(aBlock, &pThis->mFirstFreeBlock, &pThis->mLastFreeBlock);

				// Add to used list
				gsiMemBlockAddToList(aBlock, &pThis->mFirstUsedBlock, &pThis->mLastUsedBlock);

				// Create a new block in the "empty" space
				aNewBlock = gsiMemBlockGetLinearNext(aBlock);
				//aNewBlock->GSIMemoryBlock::GSIMemoryBlock();
				memset(aNewBlock, 0, sizeof(struct GSIMemoryBlock));
				gsiMemBlockSetObjectSize(aNewBlock, anOldSize - gsiMemBlockGetTotalSize(aBlock));

				// Add to the free list
				gsiMemBlockAddToList(aNewBlock, &pThis->mFirstFreeBlock, &pThis->mLastFreeBlock);

				// Update memory usage for the newly created block
				gMemoryMgr->mMemUsed += sizeof(struct GSIMemoryBlock);
				if (gMemoryMgr->mMemUsed > gMemoryMgr->mPeakMemoryUsage)
					gMemoryMgr->mPeakMemoryUsage = gMemoryMgr->mMemUsed;

				// Did we create a new "last block"
				if (wasFinal)
					gsiMemBlockSetFlag(aNewBlock, BlockFlag_Final);
			}
			else
			{
				// Mark as used
				gsiMemBlockSetFlag(aBlock, BlockFlag_Used);

				// Remove from free block list
				gsiMemBlockRemoveFromList(aBlock, &pThis->mFirstFreeBlock, &pThis->mLastFreeBlock);

				// Add to used list
				gsiMemBlockAddToList(aBlock, &pThis->mFirstUsedBlock, &pThis->mLastUsedBlock);
			}

			return aBlock;
		}

		// Not suitable, try next block
		aBlock = gsiMemBlockGetNext(aBlock);
	}
	return NULL;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void* gsiManagedMalloc(size_t size)
{
	struct GSIMemoryBlock* aBlock;
	void* anObject = NULL;

	assert(gMemoryMgr != NULL);
	
	gsiEnterCriticalSection(&gMemCrit);

	// Find a suitable block
	aBlock = gsiMemGetFreeBlock(gMemoryMgr, size);
	if (aBlock == NULL)
	{
		gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Memory, GSIDebugLevel_Warning,
			"GSIMemory::Malloc: %d bytes (out of memory)\r\n", size);
		gsiLeaveCriticalSection(&gMemCrit);
		return NULL; // out of memory
	}

	// Verify that the block hasn't been corrupted
	gsiMemBlockAssertValid(aBlock);

	// Update memory usage
	gMemoryMgr->mMemUsed += gsiMemBlockGetObjectSize(aBlock);
	assert(gMemoryMgr->mMemUsed <= gMemoryMgr->mPoolSize);
	if (gMemoryMgr->mMemUsed > gMemoryMgr->mPeakMemoryUsage)
		gMemoryMgr->mPeakMemoryUsage = gMemoryMgr->mMemUsed;

	gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"GSIMemory::Malloc: %d->%d bytes (%d remaining)\r\n", 
		size, gsiMemBlockGetObjectSize(aBlock), gMemoryMgr->mPoolSize - gMemoryMgr->mMemUsed);

	// Perform memory fill
	memset(gsiMemBlockGetObjectPtr(aBlock), MEMFILL_ALLOC, gsiMemBlockGetObjectSize(aBlock));

	anObject = (void*)gsiMemBlockGetObjectPtr(aBlock);
	gsiLeaveCriticalSection(&gMemCrit);
	return anObject;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void* gsiManagedMemalign(size_t boundary, size_t size)
{
	assert(gMemoryMgr != NULL);

	// Everything is align on the sizeof GSIMemoryBlock -- currently 32bytes
	// larger sizes are not supported
	assert(boundary <= sizeof(GSIMemoryBlock));
	if (boundary > sizeof(GSIMemoryBlock))
		return NULL;

	return gsiManagedMalloc(size);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void* gsiManagedRealloc(void* ptr, size_t size)
{
	struct GSIMemoryBlock* aBlock;
	struct GSIMemoryBlock* aNextBlock;
	int aBytesNeeded;

	assert(gMemoryMgr != NULL);
	
	// Compatability with ANSI realloc()
	if (ptr == NULL)
		return gsiManagedMalloc(size);
	if (size == 0)
		return NULL;

	gsiEnterCriticalSection(&gMemCrit);

	// Get the block info
	aBlock = (struct GSIMemoryBlock*)((char*)ptr-sizeof(struct GSIMemoryBlock));

	// Calculate the number of bytes needed (enforce multiple of MIN_BLOCK_SIZE)
	aBytesNeeded = size - gsiMemBlockGetObjectSize(aBlock);
	if ((aBytesNeeded % MIN_BLOCK_SIZE) != 0)
		aBytesNeeded += MIN_BLOCK_SIZE - (aBytesNeeded % MIN_BLOCK_SIZE);

	gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"GSIMemory::Realloc %d to %d (%d)\r\n", 
		gsiMemBlockGetObjectSize(aBlock), 
		gsiMemBlockGetObjectSize(aBlock)+aBytesNeeded,
		aBytesNeeded);

	// Shrinking is not supported.
	if (aBytesNeeded <= 0)
	{
		gsiLeaveCriticalSection(&gMemCrit);
		return ptr;
	}

	// If the next block is unused and large enough, try to expand into it
	aNextBlock = gsiMemBlockGetLinearNext(aBlock);
	if (aNextBlock && !gsiMemBlockIsFlagged(aNextBlock,BlockFlag_Used) &&
		gsiMemBlockGetObjectSize(aNextBlock) >= (gsi_i32)aBytesNeeded)
	{
		// How many bytes should we steal? (may be more than needed)
		int aBytesToSteal    = 0;
		int bStealWholeBlock = 0;

		// Remove the next block from the free list
		gsiMemBlockRemoveFromList(aNextBlock, &gMemoryMgr->mFirstFreeBlock, &gMemoryMgr->mLastFreeBlock);
		
		// Partition the next block if there are at least 32 bytes extra 
		if (gsiMemBlockGetObjectSize(aNextBlock) >= (gsi_i32)(aBytesNeeded+MIN_BLOCK_SIZE))
		{
			aBytesToSteal    = aBytesNeeded;
			bStealWholeBlock = 0;
		}
		else // otherwise, takeover the whole thing
		{
			aBytesToSteal    = gsiMemBlockGetTotalSize(aNextBlock);
			bStealWholeBlock = 1;
		}

		// expand current block into the new space
		gsiMemBlockSetObjectSize(aBlock, gsiMemBlockGetObjectSize(aBlock) + aBytesToSteal);

		// Construct a new block with the leftover space, if any
		if (!bStealWholeBlock)
		{
			struct GSIMemoryBlock* aNewBlock = gsiMemBlockGetLinearNext(aBlock);
			//aNewBlock->GSIMemoryBlock::GSIMemoryBlock();
			memset(aNewBlock, 0, sizeof(struct GSIMemoryBlock));
			gsiMemBlockSetObjectSize(aNewBlock, gsiMemBlockGetObjectSize(aNextBlock) - aBytesToSteal);

			// Transfer the final flag
			if (gsiMemBlockIsFlagged(aNextBlock, BlockFlag_Final))
				gsiMemBlockSetFlag(aNewBlock, BlockFlag_Final);

			// add the new block to the free list
			gsiMemBlockAddToList(aNewBlock, &gMemoryMgr->mFirstFreeBlock, &gMemoryMgr->mLastFreeBlock);
		}
		else
		{
			// transfer final flag since we're taking the whole block
			if (gsiMemBlockIsFlagged(aNextBlock, BlockFlag_Final))
				gsiMemBlockSetFlag(aBlock, BlockFlag_Final);

			// Decrease used memory because we destroyed a block
			gMemoryMgr->mMemUsed -= sizeof(struct GSIMemoryBlock);
		}

		// memory fill the memory added to the previous block
		memset(aNextBlock, MEMFILL_ALLOC, aBytesToSteal);

		// increment memory usage
		gMemoryMgr->mMemUsed += aBytesToSteal;
		if (gMemoryMgr->mMemUsed > gMemoryMgr->mPeakMemoryUsage)
			gMemoryMgr->mPeakMemoryUsage = gMemoryMgr->mMemUsed;

		// return current ptr, since it was resized
		gsiLeaveCriticalSection(&gMemCrit);
		return ptr;
	}
	else
	{
		// Alloc an entirely new block
		struct GSIMemoryBlock* aNewBlock = gsiMemGetFreeBlock(gMemoryMgr, size);
		void* anObject = NULL;
		if (aNewBlock == NULL)
		{
			gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Memory, GSIDebugLevel_Warning,
				"GSIMemory::Realloc: %d bytes (out of memory)\r\n", size);

			return NULL;
		}

		// Verify that it's valid
		gsiMemBlockAssertValid(aBlock);

		// Copy the data into the new block
		memcpy(gsiMemBlockGetObjectPtr(aNewBlock), gsiMemBlockGetObjectPtr(aBlock), gsiMemBlockGetObjectSize(aBlock));

		// Free the old block (will decrement memory usage)
		gsifree(gsiMemBlockGetObjectPtr(aBlock));

		// Increment memory usage for the new block
		gMemoryMgr->mMemUsed += gsiMemBlockGetObjectSize(aNewBlock);
		if (gMemoryMgr->mMemUsed > gMemoryMgr->mPeakMemoryUsage)
			gMemoryMgr->mPeakMemoryUsage = gMemoryMgr->mMemUsed;

		// return the new ptr
		anObject = gsiMemBlockGetObjectPtr(aNewBlock);
		gsiLeaveCriticalSection(&gMemCrit);
		return anObject;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsiManagedFree(void* buf)
{
	struct GSIMemoryBlock* aBlock;
	struct GSIMemoryBlock* aNextBlock;

	assert(gMemoryMgr != NULL);
	
	// Freeing NULL is legal
	if (buf == NULL)
		return;

	gsiEnterCriticalSection(&gMemCrit);

	// Get the block info
	aBlock = (struct GSIMemoryBlock*)((char*)buf-sizeof(struct GSIMemoryBlock));

	gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"GSIMemory::Free %d\r\n", gsiMemBlockGetObjectSize(aBlock));

	// Decrease memory usage because we've freed the memory
	gMemoryMgr->mMemUsed -= gsiMemBlockGetObjectSize(aBlock);

	// Clear the used flag
	gsiMemBlockClearFlag(aBlock, BlockFlag_Used);

	// Remove it from the used list
	gsiMemBlockRemoveFromList(aBlock, &gMemoryMgr->mFirstUsedBlock, &gMemoryMgr->mLastUsedBlock);

	// Add it to the free list
	gsiMemBlockAddToList(aBlock, &gMemoryMgr->mFirstFreeBlock, &gMemoryMgr->mLastFreeBlock);

	// Is the next block a free block?  If so, combine them
	aNextBlock = gsiMemBlockGetLinearNext(aBlock);
	if (aNextBlock && !gsiMemBlockIsFlagged(aNextBlock,BlockFlag_Used))
	{
		// Remove it from the free list
		gsiMemBlockRemoveFromList(aNextBlock, &gMemoryMgr->mFirstFreeBlock, &gMemoryMgr->mLastFreeBlock);

		// Transfer the flags
		if (gsiMemBlockIsFlagged(aNextBlock, BlockFlag_Final))
			gsiMemBlockSetFlag(aBlock, BlockFlag_Final);

		// Incorporate the free data into the previous block
		gsiMemBlockSetObjectSize(aBlock, gsiMemBlockGetObjectSize(aBlock) + gsiMemBlockGetTotalSize(aNextBlock));

		// Decrease memory usage because we destroyed a block
		gMemoryMgr->mMemUsed -= sizeof(struct GSIMemoryBlock);
	}

	// Perform memory fill
	memset(gsiMemBlockGetObjectPtr(aBlock), MEMFILL_FREED, gsiMemBlockGetObjectSize(aBlock));

	gsiLeaveCriticalSection(&gMemCrit);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Show allocated, free, total memory, num blocks
void gsMemMgrDumpStats()
{
	int numUsed = 0;
	int numFree = 0;
	
	struct GSIMemoryBlock* aTempPtr = NULL;

	gsiEnterCriticalSection(&gMemCrit);

	// Display the number of free blocks
	//   TODO: dump size statistics
	aTempPtr = gMemoryMgr->mFirstFreeBlock;
	while(aTempPtr != NULL)
	{
		numFree++;
		aTempPtr = aTempPtr->mNext;
	}

	// Display the number of used blocks
	//   TODO: dump size statistics
	aTempPtr = gMemoryMgr->mFirstUsedBlock;
	while(aTempPtr != NULL)
	{
		numUsed++;
		aTempPtr = aTempPtr->mNext;
	}

	// Display info - App type b/c it was requested by the app
	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"BytesUsed: %d, BlocksUsed: %d, BlocksFree: %d\r\n", 
		gMemoryMgr->mMemUsed, numUsed, numFree);

	gsiLeaveCriticalSection(&gMemCrit);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void gsMemMgrDumpAllocations()
{
	struct GSIMemoryBlock* aBlockPtr = NULL;
	gsi_time aStartTime = 0;
	gsi_i32 aNumAllocations = 0;
	gsi_i32 aNumBytesAllocated = 0;

	gsiEnterCriticalSection(&gMemCrit);

	aStartTime = current_time();
	aBlockPtr = (GSIMemoryBlock*)gMemoryMgr->mPoolStart;

	// Announce start
	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"Dumping allocations from pool - [0x%08x] %d bytes.\r\n", 
		gMemoryMgr->mPoolStart, gMemoryMgr->mPoolSize);

	// Dump information about each allocated block
	//    -  Do this in linear order, not list order
	while(aBlockPtr != NULL)
	{
		// If it's in use, verify contents and dump info
		if (gsiMemBlockIsFlagged(aBlockPtr, BlockFlag_Used))
		{
			int anObjectSize = gsiMemBlockGetObjectSize(aBlockPtr);
			aNumAllocations++;
			aNumBytesAllocated += anObjectSize;

			if (aBlockPtr == gMemoryMgr->mPoolStart)
			{
				gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
					"\t[0x%08x] Size: %d (memmgr instance)\r\n", (gsi_u32)aBlockPtr, anObjectSize);
			}
			else
			{
				gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
					"\t[0x%08x] Size: %d\r\n", (gsi_u32)(gsiMemBlockGetObjectPtr(aBlockPtr)), anObjectSize);
			}
		}
		else
		{
			// Verify that the block has the correct memory fill
		}
		// Get linear next (not list next!)
		aBlockPtr = gsiMemBlockGetLinearNext(aBlockPtr);
	}

	// Announce finish
	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"\t--%d allocations, %d bytes allocated.\r\n", aNumAllocations, aNumBytesAllocated);
	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"\t--%d peak memory usage\r\n", gMemoryMgr->mPeakMemoryUsage);

	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"Memory dump complete. (%d ms)\r\n", current_time() - aStartTime);

	gsiLeaveCriticalSection(&gMemCrit);

	GSI_UNUSED(aStartTime); // may be unused if common debug is not defined
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Check each block to make sure it's valid
void gsMemMgrValidateMemoryPool()
{
	gsi_time aStartTime = 0;
	struct GSIMemoryBlock* aBlockPtr = NULL;
	
	gsiEnterCriticalSection(&gMemCrit);
	
	aStartTime = current_time();
	aBlockPtr = (GSIMemoryBlock*)gMemoryMgr->mPoolStart;

	// Announce Start
	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"Checking memory pool...[0x%08x] %d bytes\r\n", gMemoryMgr->mPoolStart, gMemoryMgr->mPoolSize);

	// Check each block
	while(aBlockPtr != NULL)
	{
		gsiMemBlockAssertValid(aBlockPtr);
		aBlockPtr = gsiMemBlockGetLinearNext(aBlockPtr);
	}

	// Announce finish
	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Memory, GSIDebugLevel_Comment,
		"\tMemory check complete. (%d ms)\r\n", current_time() - aStartTime);

	gsiLeaveCriticalSection(&gMemCrit);

	GSI_UNUSED(aStartTime); // may be unused if common debug is not defined
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // GSI_MEM_MANAGED
