//==============================================================================
/**
 *
 *@file		heapsys.c
 *@brief	ヒープ領域管理
 *
 */
//==============================================================================
#include "gflib.h"
#include "heap_inter.h"

#ifdef HEAP_DEBUG
//#include  "system\heapdefine.h"
//#define  ALLOCINFO_PRINT_HEAPID   HEAPID_BASE_APP	// このヒープＩＤに関してのみ詳細な情報を出力
#endif

//==============================================================
// Prototype
//==============================================================
#if 0
static int SearchEmptyHandleIndex( void );
static BOOL CreateHeapCore( u32 parentHeapID, u32 childHeapID, u32 size, s32 align );
static void* AllocMemoryCore( NNSFndHeapHandle heapHandle, u32 size, s32 alignment, u32 heapID );
static void PrintAllocInfo( const MEMORY_BLOCK_HEADER* header, NNSFndHeapHandle handle, u32 size );
static void PrintFreeInfo( const MEMORY_BLOCK_HEADER* header, NNSFndHeapHandle handle);
static void HeaderDebugParamSet( MEMORY_BLOCK_HEADER* header, const char* filename, u32 line_no );
static void PrintShortHeap( u32 heapID, u32 size, const char* filename, u32 line_num );
static void PrintExistMemoryBlocks( u32 heapID );
static void HeapConflictVisitorFunc(void* memBlock, NNSFndHeapHandle heapHandle, u32 param);
static void CopyFileName( char* dst, const MEMORY_BLOCK_HEADER* header );
static void PrintUnreleasedMemoryInfo( u32 heapID );





#ifdef HEAP_DEBUG
static void HeaderDebugParamSet( MEMORY_BLOCK_HEADER* header, const char* filename, u32 line_no );
static void PrintExistMemoryBlocks( u32 heapID );
static void HeapConflictVisitorFunc(void* memBlock, NNSFndHeapHandle heapHandle, u32 param);
static void CopyFileName( char* dst, const MEMORY_BLOCK_HEADER* header );
static void PrintUnreleasedMemoryInfo( u32 heapID );
#endif
#endif


//------------------------------------------------------------------------------
/**
 * システム初期化
 *
 * @param   header			親ヒープ初期化構造体へのポインタ
 * @param   baseHeapMax		親ヒープ総数
 * @param   heapMax			親ヒープ・子ヒープ合計数
 * @param   startOffset		ヒープ領域開始オフセット（要４バイトアライン）
 */
//------------------------------------------------------------------------------
void
	GFL_HEAP_sysInit
		(const HEAP_INIT_HEADER* header, u32 parentHeapMax, u32 totalHeapMax, u32 startOffset)
{
	BOOL result = GFI_HEAP_sysInit( header, parentHeapMax, totalHeapMax, startOffset );
	u32	 remainsize = (u32)(OS_GetMainArenaHi())-(u32)(OS_GetMainArenaLo());

	OS_Printf( "remains of MainRAM = 0x%08x bytes.\n", remainsize ); 
	if( result == FALSE )
	{
		u32 errorCode = GFI_HEAP_ErrorCodeGet();
		OS_Panic( "Create ParentHeap FAILED. ID = %d size = %x\n" , 
					errorCode, header[errorCode].size );
	}
}


//------------------------------------------------------------------------------
/**
 * システム終了
 */
//------------------------------------------------------------------------------
void
	GFL_HEAP_sysExit
		( void )
{
	BOOL result = GFI_HEAP_sysExit();

	if( result == FALSE )
	{
		OS_Panic( "Delete ParentHeap FAILED.\n" );
	}
}


//------------------------------------------------------------------
/**
 * ヒープ作成
 *
 * @param   parentHeapID		メモリ領域確保用ヒープＩＤ（既に有効である必要がある）
 * @param   childHeapID			新規に作成するヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   size				ヒープサイズ
 */
//------------------------------------------------------------------
void 
	GFL_HEAP_CreateHeap
		( u32 parentHeapID, u32 childHeapID, u32 size )
{
	BOOL result = GFI_HEAP_CreateHeap( parentHeapID, childHeapID, size );

	if( result == FALSE )
	{
		u32 errorCode = GFI_HEAP_ErrorCodeGet();

		OS_Printf( "Create ChildHeap FAILED. ID = %d size = %x\n", childHeapID, size );

		switch( errorCode )
		{
			case HEAP_CANNOT_CREATE_DOUBLE:
				OS_Panic( "already exist.\n" );
				break;
			case HEAP_CANNOT_CREATE_NOPARENT:
				OS_Panic( "ParentHeap not exist.\n" );
				break;
			case HEAP_CANNOT_CREATE_HEAP:
				OS_Panic( "not enough memory. ParentHeap ID = %d remains = %x\n",
							parentHeapID, GFI_HEAP_GetHeapFreeSize( parentHeapID ) );
				break;
			case HEAP_CANNOT_CREATE_HEAPTABLE:
				OS_Panic( "no more create heap.\n" );
				break;
			case HEAP_CANNOT_CREATE_HEAPHANDLE:
				OS_Panic( "make handle FAILED.\n" );
				break;
		}
	} else {
		OS_Printf( "Create ChildHeap. ID = %d size = %x\n", childHeapID, size );
	}
}


//------------------------------------------------------------------
/**
 * ヒープ破棄
 *
 * @param   childHeapID			破棄するヒープＩＤ（最上位ビットは取得方向フラグ）
 */
//------------------------------------------------------------------
void
	GFL_HEAP_DeleteHeap
		( u32 childHeapID )
{
#ifdef HEAPSYS_DEBUG
	{
		u16	restheap_count = GFI_HEAP_GetHeapCount( childHeapID );

		if( restheap_count ){
			OS_Printf( "these Memoryblocks haven't released\n" );
			OS_Printf( "HeapID = %d  restcnt = %d .....\n", childHeapID, restheap_count );

			OS_Panic( "\n" );
		}
	}
#endif
	{
		BOOL result = GFI_HEAP_DeleteHeap( childHeapID );

		if( result == FALSE )
		{
			u32 errorCode = GFI_HEAP_ErrorCodeGet();
	
			OS_Printf( "Delete ChildHeap FAILED. ID = %d\n", childHeapID );
	
			switch( errorCode )
			{
				case HEAP_CANNOT_DELETE_DOUBLE:
					OS_Panic( "not exist.\n" );
					break;
				case HEAP_CANNOT_DELETE_NOPARENT:
					OS_Panic( "ParentHeap cannot search.\n" );
					break;
			}
		} else {
			OS_Printf( "Delete ChildHeap. ID = %d\n", childHeapID );
		}
	}
}


//------------------------------------------------------------------
/**
 * ヒープからメモリを確保する
 *
 * @param   heapID		ヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   size		確保サイズ
 *
 * @retval  void*		確保した領域アドレス
 */
//------------------------------------------------------------------
#ifndef HEAPSYS_DEBUG
void*
	GFL_HEAP_AllocMemoryblock
		( u32 heapID, u32 size )
#else
void*
	GFL_HEAP_AllocMemoryblock
		( u32 heapID, u32 size, const char* filename, u32 linenum )
#endif
{
	void* memory = GFI_HEAP_AllocMemory( heapID, size );

	if( memory == NULL )
	{
		u32 errorCode = GFI_HEAP_ErrorCodeGet();

		OS_Printf( "Alloc Memory FAILED. heapID = %d. allocsize = %x\n", heapID, size );

		switch( errorCode )
		{
			case HEAP_CANNOT_ALLOC_NOID:
				OS_Panic( "heapID is not exist.\n" );
				break;
			case HEAP_CANNOT_ALLOC_NOHEAP:
				OS_Panic( "heap is not exist.\n" );
				break;
			case HEAP_CANNOT_ALLOC_MEM:
				OS_Panic( "not enough memory. heapID = %d remains = %x\n",
							heapID,	GFI_HEAP_GetHeapFreeSize( heapID ));
				break;
		}
	}
	//↓必要に応じて情報の表示をする（呼び出される回数が多いのでDefaultでは表示しない）
	//OS_Printf( "Alloc Memory. Heap ID = %d. allocsize = %x\n", heapID, size );
	return memory;
}


//------------------------------------------------------------------
/**
 * ヒープからメモリを解放する
 *
 * @param   memory		確保したメモリアドレス
 */
//------------------------------------------------------------------
void
	GFL_HEAP_FreeMemoryblock
		( void* memory )
{
	BOOL result = GFI_HEAP_FreeMemory( memory );

	if( result == FALSE )
	{
		u32 errorCode = GFI_HEAP_ErrorCodeGet();

		OS_Printf( "Free Memory FAILED. memory = %08x\n", memory );

		switch( errorCode )
		{
			case HEAP_CANNOT_FREE_NOBLOCK:
				OS_Panic( "this pointer is not memory_block.\n" );
				break;
			case HEAP_CANNOT_FREE_NOID:
				OS_Panic( "system_backup_heapID is not exist.\n" );
				break;
			case HEAP_CANNOT_FREE_NOHEAP:
				OS_Panic( "system_backup_heap is not exist.\n" );
				break;
			case HEAP_CANNOT_FREE_NOMEM:
				OS_Printf( "this memory_block is not exist\n" );
				GFL_HEAP_CheckHeapSafe( GFI_HEAP_GetMemheaderHeapID( memory ));
				break;
		}
	}
	//↓必要に応じて情報の表示をする（呼び出される回数が多いのでDefaultでは表示しない）
	//OS_Printf( "Free Memory.\n", heapID, size );
}


//------------------------------------------------------------------
/**
 * NitroSystem ライブラリ系関数が要求するアロケータを作成する
 *
 * @param   pAllocator		NNSFndAllocator構造体のアドレス
 * @param   heapID			ヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   align			確保するメモリブロックに適用するアライメント（負の値は正の値に変換）
 */
//------------------------------------------------------------------
void
	GFL_HEAP_InitAllocator
		( NNSFndAllocator* pAllocator, u32 heapID, s32 alignment )
{
	BOOL result = GFI_HEAP_InitAllocator( pAllocator, heapID, alignment );

	if( result == FALSE )
	{
		OS_Printf( "InitAllocator FAILED.\n" );
		OS_Panic( "heapID is not exist. ID = %d\n", heapID );
	}
}


//------------------------------------------------------------------
/**
 * 確保したメモリブロックのサイズを変更する。
 *
 * @param   memory		メモリブロックポインタ
 * @param   newSize		変更後のサイズ（バイト単位）
 */
//------------------------------------------------------------------
void
	GFL_HEAP_MemoryResize
		( void* memory, u32 newSize )
{
	BOOL result = GFI_HEAP_MemoryResize( memory, newSize );

	if( result == FALSE )
	{
		u32 errorCode = GFI_HEAP_ErrorCodeGet();

		OS_Printf( "MemoryResize FAILED.\n" );

		switch( errorCode )
		{
			case HEAP_CANNOT_MEMRESIZE_NOBLOCK:
				OS_Panic( "this pointer is not memory_block.\n" );
				break;
			case HEAP_CANNOT_MEMRESIZE_NOID:
				OS_Panic( "system_backup_heapID is crushed.\n" );
				break;
			case HEAP_CANNOT_MEMRESIZE_NOHEAP:
				OS_Panic( "system_backup_heap is not exist.\n" );
				break;
			case HEAP_CANNOT_MEMRESIZE_LARGE:
				OS_Printf( "cannot large\n" );
				break;
			case HEAP_CANNOT_MEMRESIZE_SMALL:
				OS_Printf( "cannot small\n" );
				break;
		}
	}
}


//------------------------------------------------------------------
/**
 * ヒープの空き領域サイズを返す
 *
 * @param   heapID	ヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  u32		空き領域サイズ（バイト単位）
 */
//------------------------------------------------------------------
u32
	GFL_HEAP_GetHeapFreeSize
		( u32 heapID )
{
	u32 result =  GFI_HEAP_GetHeapFreeSize( heapID );

	if( result == 0 )
	{
		OS_Printf( "GetHeapFreeSize FAILED.\n" );
		OS_Panic( "heapID is not exist. ID = %d\n", heapID );
	}
	return result;
}


//------------------------------------------------------------------
/**
 * ヒープ領域が破壊されていないかチェック
 *
 * @param   heapID	ヒープID（最上位ビットは取得方向フラグ）
 *
 * @retval  BOOL	破壊されていなければTRUE
 */
//------------------------------------------------------------------
void
	GFL_HEAP_CheckHeapSafe
		( u32 heapID )
{
	BOOL result = GFI_HEAP_CheckHeapSafe( heapID );

	if( result == FALSE )
	{
		u32 errorCode = GFI_HEAP_ErrorCodeGet();

		OS_Printf( "Memory Check FAILED.\n" );

		switch( errorCode )
		{
			case HEAP_CANNOT_CHECKHEAPSAFE_NOID:
				OS_Printf( "heapID is not exist.\n" );
				break;
			case HEAP_CANNOT_CHECKHEAPSAFE_NOHEAP:
				OS_Printf( "heap is not exist.\n" );
				break;
			case HEAP_CANNOT_CHECKHEAPSAFE_DESTROY:
				OS_Panic( "memory is Crushed\n" );
				break;
		}
	}
}





#if 0
#ifndef HEAP_DEBUG
#else	// #ifndef HEAP_DEBUG

//------------------------------------------------------------------
/**
 * メモリ確保時の詳細情報表示
 *
 * @param   header		メモリブロックヘッダ
 * @param   handle		メモリブロックが含まれるヒープのハンドル
 * @param   size		確保サイズ（リクエストされたサイズ）
 *
 */
//------------------------------------------------------------------
static void PrintAllocInfo( const MEMORY_BLOCK_HEADER* header, NNSFndHeapHandle handle, u32 size )
{
	char filename[MEMBLOCK_FILENAME_AREASIZE+1];

	CopyFileName( filename, header );

	OS_TPrintf("[HEAP] ALLOC count=%3d rest=0x%08x adrs:0x%08x size:0x%05x %s(%d)\n",
				GetHeapCount(header->heapID), NNS_FndGetTotalFreeSizeForExpHeap(handle), 
				(u32)header, size+sizeof(MEMORY_BLOCK_HEADER), filename, header->lineNum );
}

//------------------------------------------------------------------
/**
 * メモリ解放時の詳細情報表示
 *
 * @param   heapID			ヒープID
 * @param   size			確保したサイズ
 * @param   handle			ヒープハンドル
 * @param   filename		呼び出し元ソースファイル名
 * @param   line			呼び出し元ソースファイル行番号
 *
 */
//------------------------------------------------------------------
static void PrintFreeInfo( const MEMORY_BLOCK_HEADER* header, NNSFndHeapHandle handle)
{
	char filename[MEMBLOCK_FILENAME_AREASIZE+1];
	u32  blockSize, restSize;

	CopyFileName( filename, header );

	blockSize = NNS_FndGetSizeForMBlockExpHeap(header) + sizeof(NNSiFndExpHeapMBlockHead);

	// 残りサイズは現サイズ＋これから解放しようとするメモリブロックのサイズになるはず
	restSize = NNS_FndGetTotalFreeSizeForExpHeap(handle) + blockSize;

	OS_TPrintf("[HEAP] FREE  count=%3d rest=0x%08x adrs:0x%08x size:0x%05x %s(%d)\n",
				GetHeapCount(header->heapID), restSize, (u32)header, blockSize,
				filename, header->lineNum );
}



//------------------------------------------------------------------
/**
 * sys_AllocMemoryのデバッグ用ラッパー関数
 *
 * @param   heapID			ヒープID
 * @param   size			確保サイズ
 * @param   filename		呼び出しソースのファイル名
 * @param   line_num		呼び出しソースの行番号
 *
 * @retval  void*			確保した領域アドレス（失敗ならNULL）
 */
//------------------------------------------------------------------
void* sys_AllocMemoryDebug( u32 heapID, u32 size, const char* filename, u32 line_num )
{
	GF_ASSERT_MSG((OS_GetProcMode() != OS_PROCMODE_IRQ), "Alloc in IRQ\n %s(%d)", filename, line_num);

	if( heapID < HeapSys.heapMax )
	{
		NNSFndHeapHandle  h = GetHeapHandle( heapID );
		void* mem = AllocMemoryCore( h, size, DEFAULT_ALIGN, heapID );

		if( mem != NULL )
		{
			HeaderDebugParamSet((MEMORY_BLOCK_HEADER*)( (u8*)mem - sizeof(MEMORY_BLOCK_HEADER) ),
									filename, line_num);
			GetHeapCount(heapID)++;

			#ifdef ALLOCINFO_PRINT_HEAPID
			if( ALLOCINFO_PRINT_HEAPID == heapID )
			{
				const MEMORY_BLOCK_HEADER* mh = 
					(const MEMORY_BLOCK_HEADER*)((u8*)mem - sizeof(MEMORY_BLOCK_HEADER));
				PrintAllocInfo( mh, h, size );
				GF_ASSERT( sys_CheckHeapSafe( heapID ) );
			}
			#endif
		}
		else
		{
			PrintShortHeap( heapID, size, filename, line_num );
			GF_ASSERT(0);
		}

		return mem;
	}
	else
	{
		GF_ASSERT_MSG(0, "heapID = %d\n", heapID);
		return NULL;
	}
}
//------------------------------------------------------------------
/**
 * sys_AllocMemoryLoのデバッグ用ラッパー関数
 *
 * @param   heapID			ヒープID
 * @param   size			確保サイズ
 * @param   filename		呼び出しソースのファイル名
 * @param   line_num		呼び出しソースの行番号
 *
 * @retval  void*			確保した領域アドレス（失敗ならNULL）
 */
//------------------------------------------------------------------
void* sys_AllocMemoryLoDebug( u32 heapID, u32 size, const char* filename, u32 line_num )
{
	GF_ASSERT_MSG((OS_GetProcMode() != OS_PROCMODE_IRQ), "Alloc in IRQ\n %s(%d)", filename, line_num);

	if( heapID < HeapSys.heapMax )
	{
		NNSFndHeapHandle  h = GetHeapHandle( heapID );
		void* mem = AllocMemoryCore( h, size, -DEFAULT_ALIGN, heapID );

		if( mem != NULL )
		{
			HeaderDebugParamSet((MEMORY_BLOCK_HEADER*)( (u8*)mem - sizeof(MEMORY_BLOCK_HEADER) ),
									filename, line_num);
			GetHeapCount(heapID)++;

			#ifdef ALLOCINFO_PRINT_HEAPID
			if( ALLOCINFO_PRINT_HEAPID == heapID )
			{
				const MEMORY_BLOCK_HEADER* mh = 
					(const MEMORY_BLOCK_HEADER*)((u8*)mem - sizeof(MEMORY_BLOCK_HEADER));
				PrintAllocInfo( mh, h, size );
				GF_ASSERT( sys_CheckHeapSafe( heapID ) );
			}
			#endif
		}
		else
		{
			PrintShortHeap( heapID, size, filename, line_num );
			GF_ASSERT(0);
		}
		return mem;
	}
	else
	{
		GF_ASSERT(0);
		return NULL;
	}
}
//------------------------------------------------------------------
/**
 * ブロックヘッダにデバッグ情報を書き込む
 *
 * @param   header		ヘッダアドレス
 * @param   filename	ファイル名
 * @param   line_no		行番号
 *
 */
//------------------------------------------------------------------
static void HeaderDebugParamSet( MEMORY_BLOCK_HEADER* header, const char* filename, u32 line_no )
{
	int i;
	for(i = 0; i < MEMBLOCK_FILENAME_AREASIZE; i++)
	{
		header->filename[i] = filename[i];
		if( filename[i] == '\0' ){ break; }
	}
	header->lineNum = line_no;
}

//------------------------------------------------------------------
/**
 * 残りメモリが不足して確保できないメッセージ取得
 *
 * @param   heapID		ヒープＩＤ
 * @param   size		確保しようとしたサイズ
 *
 */
//------------------------------------------------------------------
static void PrintShortHeap( u32 heapID, u32 size, const char* filename, u32 line_num )
{
	NNSFndHeapHandle h;
	u32 freeAreaSize, allocatableMaxSize;

	h = GetHeapHandle(heapID);
	freeAreaSize = NNS_FndGetTotalFreeSizeForExpHeap( h );
	allocatableMaxSize = NNS_FndGetAllocatableSizeForExpHeapEx( h, DEFAULT_ALIGN );

	GF_ASSERT_Printf("Can't alloc %ldbytes memory from Heap(%d)\n", size, heapID);
	GF_ASSERT_Printf("This Heap have %ldbytes Free Area\n", freeAreaSize );
	GF_ASSERT_Printf("and %ldbytes Allocatable Area\n", allocatableMaxSize );
	GF_ASSERT_Printf("%s(%d)\n", filename, line_num);

	GF_ASSERT_Printf("these Memoryblocks haven't released\n");
	PrintExistMemoryBlocks( heapID );
}
#endif	// #ifndef HEAP_DEBUG #else


//------------------------------------------------------------------
/**
 * ヒープから確保したメモリを解放する
 *
 * @param   memory		確保したメモリアドレス
 *
 */
//------------------------------------------------------------------
void sys_FreeMemoryEz( void* memory )
{
	#ifdef HEAP_DEBUG
	if( OS_GetProcMode() == OS_PROCMODE_IRQ )
	{
		char filename[MEMBLOCK_FILENAME_AREASIZE+1];
		MEMORY_BLOCK_HEADER* header;

		header = (MEMORY_BLOCK_HEADER*)((u8*)memory - sizeof(MEMORY_BLOCK_HEADER));
		CopyFileName( filename, header );
		GF_ASSERT_MSG(0, "Free in IRQ\n %s(%d) siz:%x", filename, header->lineNum, 
						NNS_FndGetSizeForMBlockExpHeap(header) );
	}
	#endif

	{
		u32 heapID;

		(u8*)memory -= sizeof(MEMORY_BLOCK_HEADER);
		heapID = ((MEMORY_BLOCK_HEADER*)memory)->heapID;

		if( heapID < HeapSys.heapMax )
		{
			NNSFndHeapHandle  h = GetHeapHandle(heapID);

			GF_ASSERT(h != NNS_FND_HEAP_INVALID_HANDLE);
			if( GetHeapCount(heapID) == 0 )
			{
				sys_CheckHeapSafe(heapID);
			}
			GF_ASSERT_MSG( GetHeapCount(heapID), "heapID = %d", heapID );

			GetHeapCount(heapID)--;

			#ifdef ALLOCINFO_PRINT_HEAPID
			if( ALLOCINFO_PRINT_HEAPID == heapID )
			{
				GF_ASSERT( sys_CheckHeapSafe(heapID) );
				PrintFreeInfo( (const MEMORY_BLOCK_HEADER*)memory, h );
			}
			#endif

			{
				OSIntrMode old;
				old = OS_DisableInterrupts();
				NNS_FndFreeToExpHeap( h, memory );
				OS_RestoreInterrupts( old );
			}

		}
		else
		{
			GF_ASSERT_MSG(0, "heapID = %d\n", heapID);
		}
	}
}
//------------------------------------------------------------------
/**
 * ヒープから確保したメモリを解放する（ヒープID指定版）
 *
 * ※もしかしたらヒープIDを指定する合理的な理由があるかもしれないので
 *   残しておく。普通は FreeMemoryEz を使えば問題ないはず。
 *
 * @param   heapID		ヒープID
 * @param   memory		確保したメモリポインタ
 *
 */
//------------------------------------------------------------------
void sys_FreeMemory( u32 heapID, void* memory )
{
	ASSERT_IRQ_ENABLED();

	if( heapID < HeapSys.heapMax )
	{
		NNSFndHeapHandle h = GetHeapHandle(heapID);

		GF_ASSERT(h != NNS_FND_HEAP_INVALID_HANDLE);

		(u8*)memory -= sizeof(MEMORY_BLOCK_HEADER);
		if( ((MEMORY_BLOCK_HEADER*)memory)->heapID != heapID )
		{
			GF_ASSERT_MSG(0, "確保時と違うヒープIDで解放されようとしている\n");
		}
		NNS_FndFreeToExpHeap( h, memory );

		GF_ASSERT( GetHeapCount(heapID) );
		GetHeapCount(heapID)--;

		#ifdef ALLOCINFO_PRINT_HEAPID
		if( ALLOCINFO_PRINT_HEAPID == heapID )
		{
			GF_ASSERT( sys_CheckHeapSafe( heapID ) );
			PrintFreeInfo( (const MEMORY_BLOCK_HEADER*)memory, h );
		}
		#endif

	}
	else
	{
		GF_ASSERT(0);
	}
}
//------------------------------------------------------------------
/**
 * ヒープの空き領域サイズを返す
 *
 * @param   heapID	ヒープID
 *
 * @retval  u32		空き領域サイズ（バイト単位）
 */
//------------------------------------------------------------------
u32 sys_GetHeapFreeSize( u32 heapID )
{
	if( heapID < HeapSys.heapMax )
	{
		NNSFndHeapHandle h = GetHeapHandle(heapID);
		return NNS_FndGetTotalFreeSizeForExpHeap( h );
	}
	GF_ASSERT(0);
	return 0;
}
//------------------------------------------------------------------
/**
 * NitroSystem ライブラリ系関数が要求するアロケータを作成する
 *
 * @param   pAllocator		NNSFndAllocator構造体のアドレス
 * @param   heapID			ヒープID
 * @param   alignment		確保するメモリブロックに適用するアライメント
 *
 */
//------------------------------------------------------------------
void sys_InitAllocator( NNSFndAllocator* pAllocator, u32 heapID, int alignment)
{
	if( heapID < HeapSys.heapMax )
	{
		NNSFndHeapHandle h = GetHeapHandle(heapID);
		NNS_FndInitAllocatorForExpHeap( pAllocator, h, alignment );
	}
	else
	{
		GF_ASSERT(0);
	}
}

//------------------------------------------------------------------
/**
 * 確保したメモリブロックのサイズを縮小する。
 *
 * @param   memBlock		メモリブロックポインタ
 * @param   newSize			縮小後のサイズ（バイト単位）
 *
 *
 * 縮小は、メモリブロックの後ろ方向からメモリを解放することで行う。
 * 解放された分はシステムに返還され、新たなアロケート領域として使用できる。
 *
 * 例えば【ヘッダ＋実体】のような形式のグラフィックバイナリをＲＡＭに読み込み、
 * 実体部をVRAMに転送した後、ヘッダのみを残したいというケースなどで使用することを
 * 想定している。使用は慎重に。
 *
 */
//------------------------------------------------------------------
void sys_CutMemoryBlockSize( void* memBlock, u32 newSize )
{
	ASSERT_IRQ_ENABLED();

	{
		u32 oldSize;

		(u8*)memBlock -= sizeof(MEMORY_BLOCK_HEADER);
		oldSize = NNS_FndGetSizeForMBlockExpHeap( memBlock );
		newSize += sizeof(MEMORY_BLOCK_HEADER);	// 呼び出し側はヘッダを意識していないので

		if( oldSize >= newSize )
		{
			u32 heapID;
			NNSFndHeapHandle handle;

			heapID = ((MEMORY_BLOCK_HEADER*)memBlock)->heapID;
			handle = GetHeapHandle(heapID);

			NNS_FndResizeForMBlockExpHeap( handle, memBlock, newSize );
		}
		else
		{
			GF_ASSERT(0);
		}
	}
}



//------------------------------------------------------------------
/**
 * ヒープ領域が破壊されていないかチェック（デバッグ用）
 *
 * @param   heapID		ヒープID
 *
 * @retval  BOOL		破壊されていなければTRUEが返る
 */
//------------------------------------------------------------------
BOOL sys_CheckHeapSafe( u32 heapID )
{
	#ifdef HEAP_DEBUG
	if( heapID < HeapSys.heapMax )
	{
		NNSFndHeapHandle h = GetHeapHandle(heapID);
		if( h != NNS_FND_HEAP_INVALID_HANDLE )
		{
			return NNS_FndCheckExpHeap( h, NNS_FND_HEAP_ERROR_PRINT );
		}
	}
	return TRUE;
	#else

	// 本来製品版には存在しないはずの関数だが、
	// GF_ASSERTを製品版でも有効にしたため、ASSERTチェック中に呼ばれることのある
	// この関数を削除できなくなった。そのため、常にASSERTをスルーできるように
	// TRUEを返す関数として残しておく。
	return TRUE;
	#endif
}

#ifdef HEAP_DEBUG
//------------------------------------------------------------------
/**
 * 全メモリブロックを解放してあるかチェック（デバッグ用）
 *（※この関数が呼ばれた時にまだ使用メモリが残っているとASSERTで止まる）
 *
 * @param   heapID		ヒープＩＤ
 *
 */
//------------------------------------------------------------------
void sys_CheckHeapFullReleased( u32 heapID )
{
	if( GetHeapCount(heapID) )
	{
		PrintUnreleasedMemoryInfo( heapID );
		GF_ASSERT(0);
	}
}
//------------------------------------------------------------------
/**
 * ヒープから確保したメモリブロックの実サイズ取得（デバッグ用）
 *
 * @param   memBlock		
 *
 * @retval  u32		メモリブロックサイズ
 */
//------------------------------------------------------------------
u32 sys_GetMemoryBlockSize( const void* memBlock )
{
	((u8*)memBlock) -= sizeof(MEMORY_BLOCK_HEADER);
	return NNS_FndGetSizeForMBlockExpHeap( memBlock );
}

//------------------------------------------------------------------
/**
 * 指定ヒープのメモリアロケート回数＆空き領域サイズを64bitにパックして返す
 *（ヒープ破棄時とは別にリークチェックを行うため）
 *
 * @param   heapID		ヒープID
 *
 * @retval  u64		
 */
//------------------------------------------------------------------
u64 sys_GetHeapState( u32 heapID )
{
	if( heapID < HeapSys.heapMax )
	{
		u64  ret;
		NNSFndHeapHandle h;

		h = GetHeapHandle(heapID);
		ret = (GetHeapCount(heapID) << 32) | NNS_FndGetTotalFreeSizeForExpHeap(h);
		return ret;
	}
	GF_ASSERT(0);
	return 0;
}

/*---------------------------------------------------------------------------*
 * @brief	デバッグ用メモリ状況表示
*---------------------------------------------------------------------------*/

//------------------------------------------------------------------
/**
 * 特定ヒープのメモリ空き容量合計を表示
 *
 * @param   heapID		
 *
 */
//------------------------------------------------------------------
void sys_PrintHeapFreeSize( u32 heapID )
{
	if( heapID < HeapSys.heapMax )
	{
		NNSFndHeapHandle h = GetHeapHandle(heapID);
		OS_TPrintf("HEAP(ID :%d) FreeArea = %ld bytes\n", heapID, NNS_FndGetTotalFreeSizeForExpHeap(h) );
	}
}
//------------------------------------------------------------------
/**
 * 特定ヒープの使用中メモリブロック情報を表示
 *
 * @param   heapID		
 *
 */
//------------------------------------------------------------------
void sys_PrintHeapExistMemoryInfo( u32 heapID )
{
	PrintExistMemoryBlocks( heapID );
}
//------------------------------------------------------------------
/**
 * 特定ヒープの全メモリブロック情報を表示
 *
 * @param   heapID				ヒープID
 *
 */
//------------------------------------------------------------------
static void PrintExistMemoryBlocks( u32 heapID )
{
	NNSFndHeapHandle h = GetHeapHandle(heapID);
	NNS_FndVisitAllocatedForExpHeap( h, HeapConflictVisitorFunc, 0 );
}
//------------------------------------------------------------------
/**
 * sys_PrintHeapConflictのチェック結果がエラーなら、
 * 全メモリブロック情報を引数にしてこの関数が呼ばれる
 *
 * @param   memBlock		
 * @param   heapHandle		
 * @param   param		
 *
 */
//------------------------------------------------------------------
static void HeapConflictVisitorFunc(void* memBlock, NNSFndHeapHandle heapHandle, u32 param)
{
	char  filename[MEMBLOCK_FILENAME_AREASIZE + 1];
	int i;

	MEMORY_BLOCK_HEADER* head = (MEMORY_BLOCK_HEADER*)memBlock;

	CopyFileName( filename, head );

	GF_ASSERT_Printf(filename);
	GF_ASSERT_Printf(" (%d)", head->lineNum );
	GF_ASSERT_Printf(" adr:%08x", (u8*)memBlock + sizeof(MEMORY_BLOCK_HEADER));
	GF_ASSERT_Printf(" siz:%05x\n", NNS_FndGetSizeForMBlockExpHeap(head));
}

//------------------------------------------------------------------
/**
 * メモリブロックヘッダに保存されているファイル名をバッファにコピー
 *
 * @param   dst			コピー先バッファ
 * @param   header		メモリブロックヘッダ
 *
 */
//------------------------------------------------------------------
static void CopyFileName( char* dst, const MEMORY_BLOCK_HEADER* header )
{
	int i;

	// 終端コードナシで領域めいっぱいファイル名に使ってるため
	// こういう処理が必要...
	for(i = 0; i < MEMBLOCK_FILENAME_AREASIZE; i++)
	{
		if( header->filename[i] == '\0' ){ break; }
		dst[i] = header->filename[i];
	}
	dst[i] = '\0';

}

//------------------------------------------------------------------
/**
 * 未解放メモリの情報をプリント
 *
 * @param   heapID		
 *
 */
//------------------------------------------------------------------
static void PrintUnreleasedMemoryInfo( u32 heapID )
{
	GF_ASSERT_Printf("these Memoryblocks haven't released\n");
	GF_ASSERT_Printf("HeapID:%d  restcnt = %d .....\n", heapID, GetHeapCount(heapID));
	#ifdef HEAP_DEBUG
	{
		NNSFndHeapHandle  handle = GetHeapHandle(heapID);
		u32  siz = NNS_FndGetTotalFreeSizeForExpHeap( handle );
		GF_ASSERT_Printf(" ID %d  freesize = 0x%x bytes \n", heapID, siz );
	}
	#endif
	PrintExistMemoryBlocks( heapID );
}

//==============================================================================================
// デバッグ用ヒープ状態スタック
//==============================================================================================

struct _HEAP_STATE_STACK {
	u16   sp;
	u16   stackNum;
	u32   heapID;
	u64   stack[0];
};

//------------------------------------------------------------------
/**
 * ヒープ状態スタックを新規作成
 *
 * @param   heapID			チェックするヒープID
 * @param   stackNum		スタック要素数
 *
 * @retval  HEAP_STATE_STACK*		作成したスタックへのポインタ
 */
//------------------------------------------------------------------
HEAP_STATE_STACK*  HSS_Create( u32 heapID, u32 stackNum )
{
	HEAP_STATE_STACK* hss;
	u32 size;

	size = sizeof(HEAP_STATE_STACK) + sizeof(u64)*stackNum;
	hss = sys_AllocMemoryLo( HEAPID_BASE_DEBUG, size );
	hss->sp = 0;
	hss->heapID = heapID;
	return hss;
}

//------------------------------------------------------------------
/**
 * ヒープ状態 Push
 *
 * @param   hss		スタックへのポインタ
 *
 */
//------------------------------------------------------------------
void HSS_Push( HEAP_STATE_STACK* hss )
{
	GF_ASSERT( hss->sp < hss->stackNum );
	hss->stack[hss->sp++] = sys_GetHeapState( hss->heapID );
}

//------------------------------------------------------------------
/**
 * ヒープ状態 Pop
 *
 * @param   hss		スタックへのポインタ
 *
 */
//------------------------------------------------------------------
void HSS_Pop( HEAP_STATE_STACK* hss )
{
	GF_ASSERT( hss->sp );
	hss->sp--;
	GF_ASSERT( hss->stack[hss->sp] == sys_GetHeapState(hss->heapID) );
}


//------------------------------------------------------------------
/**
 * ヒープ状態スタックを破棄する
 *
 * @param   hss		スタックへのポインタ
 *
 */
//------------------------------------------------------------------
void HSS_Delete( HEAP_STATE_STACK* hss )
{
	sys_FreeMemoryEz( hss );
}


#endif	// HEAP_DEBUG

#endif

