//==============================================================================
/**
 *
 *@file		heapsys.c
 *@brief	ヒープ領域管理
 *
 */
//==============================================================================
#include "gflib.h"

#ifdef HEAP_DEBUG
//#include  "system\heapdefine.h"
//#define  ALLOCINFO_PRINT_HEAPID   HEAPID_BASE_APP	// このヒープＩＤに関してのみ詳細な情報を出力
#endif

//==============================================================================
/**
 *
 *
 *
 *
 *	メインアリーナ管理
 *
 *
 *
 *
 */
//==============================================================================
//----------------------------------------------------------------
/**
 *	プロトタイプ宣言
 */
//----------------------------------------------------------------
#ifdef HEAPSYS_DEBUG
static void HeaderDebugParamSet( void* memory, const char* filename, u16 lineNum );
static void PrintShortHeap( HEAPID heapID, u32 size, const char* filename, u32 linenum );
static void PrintAllocInfo( void* memory, u32 size );
static void PrintFreeInfo( void* memory );
#endif

//----------------------------------------------------------------
/**
 *	ユーザーメモリブロックヘッダ定義（サイズ:MEMHEADER_USERINFO_SIZE = 26 ）
 */
//----------------------------------------------------------------
#define FILENAME_LEN	(24/2)
typedef struct {
	char	filename[ FILENAME_LEN ];	///< 呼び出し先ソース名
	u16		lineNum;
}DEBUG_MEMHEADER;


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
		( HEAPID parentHeapID, HEAPID childHeapID, u32 size )
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
		( HEAPID childHeapID )
{
	BOOL result = GFI_HEAP_DeleteHeap( childHeapID );
#ifdef HEAPSYS_DEBUG
	GFL_HEAP_DEBUG_PrintUnreleasedMemoryCheck( childHeapID );
#endif

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
		( HEAPID heapID, u32 size )
#else
void*
	GFL_HEAP_AllocMemoryblock
		( HEAPID heapID, u32 size, const char* filename, u16 linenum )
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
				OS_Printf( "heapID is not exist.\n" );
				break;
			case HEAP_CANNOT_ALLOC_NOHEAP:
				OS_Printf( "heap is not exist.\n" );
				break;
			case HEAP_CANNOT_ALLOC_MEM:
				OS_Printf( "not enough memory. heapID = %d remains = %x\n",
							heapID,	GFI_HEAP_GetHeapFreeSize( heapID ));
				break;
		}
		#ifdef HEAPSYS_DEBUG
		PrintShortHeap( heapID, size, filename, linenum );
		#endif
		OS_Panic( "....................................\n" );
	} else {
		#ifdef HEAPSYS_DEBUG
		HeaderDebugParamSet( memory, filename, linenum );
		#endif
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
		( NNSFndAllocator* pAllocator, HEAPID heapID, s32 alignment )
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
		( HEAPID heapID )
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
		( HEAPID heapID )
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




#ifdef HEAPSYS_DEBUG
//----------------------------------------------------------------
/**
 *	デバッグ用関数（ローカル）
 */
//----------------------------------------------------------------
//------------------------------------------------------------------
/**
 * ブロックヘッダにデバッグ情報を書き込む
 *
 * @param   header		ヘッダアドレス
 * @param   filename	ファイル名
 * @param   line_no		行番号
 */
//------------------------------------------------------------------
static void HeaderDebugParamSet( void* memory, const char* filename, u16 lineNum )
{
	DEBUG_MEMHEADER* header = (DEBUG_MEMHEADER*)GFI_HEAP_GetMemheaderUserinfo( memory );

	int i;
	for(i = 0; i < FILENAME_LEN; i++)
	{
		header->filename[i] = filename[i];
		if( filename[i] == '\0' ){ break; }
	}
	header->lineNum = lineNum;
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
static void GetFileName( char* dst, char* src )
{
	int i;

	// 終端コードナシで領域めいっぱいファイル名に使ってるため、こういう処理が必要
	for(i = 0; i < FILENAME_LEN; i++)
	{
		if( src[i] == '\0' ){ break; }
		dst[i] = src[i];
	}
	dst[i] = '\0';
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
static void PrintShortHeap( HEAPID heapID, u32 size, const char* filename, u32 linenum )
{
	NNSFndHeapHandle handle = GFI_HEAP_GetHandle(heapID);
	u32 freeAreaSize = NNS_FndGetTotalFreeSizeForExpHeap( handle );
	u32 allocatableMaxSize = NNS_FndGetAllocatableSizeForExpHeapEx( handle, 4 );

	OS_Printf("Can't alloc %ldbytes memory from Heap(%d)\n", size, heapID);
	OS_Printf("This Heap have %ldbytes Free Area\n", freeAreaSize );
	OS_Printf("and %ldbytes Allocatable Area\n", allocatableMaxSize );
	OS_Printf("%s(%d)\n", filename, linenum);

	OS_Printf("these Memoryblocks haven't released\n");
	GFL_HEAP_DEBUG_PrintExistMemoryBlocks( heapID );
}

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
static void PrintAllocInfo( void* memory, u32 size )
{
	DEBUG_MEMHEADER* header = (DEBUG_MEMHEADER*)GFI_HEAP_GetMemheaderUserinfo( memory );
	HEAPID	heapID = GFI_HEAP_GetMemheaderHeapID( memory );
	u16		allocCount = GFI_HEAP_GetHeapCount( heapID );
	char	filename[FILENAME_LEN+1];

	GetFileName( filename, header->filename );

	OS_Printf("[HEAP] ALLOC count=%3d rest=0x%08x adrs:0x%08x size:0x%05x %s(%d)\n",
				allocCount, GFI_HEAP_GetHeapFreeSize( heapID ), (u32)memory - MEMHEADER_SIZE,
				size + MEMHEADER_SIZE, filename, header->lineNum );
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
static void PrintFreeInfo( void* memory )
{
	DEBUG_MEMHEADER* header = (DEBUG_MEMHEADER*)GFI_HEAP_GetMemheaderUserinfo( memory );
	HEAPID	heapID = GFI_HEAP_GetMemheaderHeapID( memory );
	u16		allocCount = GFI_HEAP_GetHeapCount( heapID );
	char	filename[FILENAME_LEN+1];
	u32		blockSize, restSize;

	GetFileName( filename, header->filename );
				
	blockSize = NNS_FndGetSizeForMBlockExpHeap((void*)((u32)memory - MEMHEADER_SIZE))
					+ sizeof(NNSiFndExpHeapMBlockHead);
	// 残りサイズは現サイズ＋これから解放しようとするメモリブロックのサイズになるはず
	restSize =  GFI_HEAP_GetHeapFreeSize( heapID ) + blockSize;
				
	OS_Printf("[HEAP] FREE  count=%3d rest=0x%08x adrs:0x%08x size:0x%05x %s(%d)\n",
				allocCount, restSize, (u32)memory - MEMHEADER_SIZE, blockSize,
				filename, header->lineNum );
}

//----------------------------------------------------------------
/**
 *	デバッグ用関数（グローバル）
 */
//----------------------------------------------------------------
//------------------------------------------------------------------
/**
 * 未解放メモリの情報をプリント
 *
 * @param   heapID		
 * 未解放領域があればＣＰＵ停止
 */
//------------------------------------------------------------------
void GFL_HEAP_DEBUG_PrintUnreleasedMemoryCheck( HEAPID heapID )
{
	u16	restheap_count = GFI_HEAP_GetHeapCount( heapID );

	if( restheap_count ){
		OS_Printf( "these Memoryblocks haven't released\n" );
		OS_Printf( "HeapID = %d  restcnt = %d .....\n", heapID, restheap_count );
		OS_Printf( "freesize = 0x%x bytes \n", GFI_HEAP_GetHeapFreeSize(heapID) );

		GFL_HEAP_DEBUG_PrintExistMemoryBlocks( heapID );
		OS_Panic( "....................................\n" );
	}
}

//------------------------------------------------------------------
/**
 * 特定ヒープの全メモリブロック情報を表示
 *
 * @param   heapID				ヒープID
 */
//------------------------------------------------------------------
static void HeapConflictVisitorFunc(void* memBlock, NNSFndHeapHandle heapHandle, u32 param)
{
	void*	memory = (u8*)memBlock + MEMHEADER_SIZE;
	DEBUG_MEMHEADER* header = (DEBUG_MEMHEADER*)GFI_HEAP_GetMemheaderUserinfo( memory );
	char	filename[FILENAME_LEN+1];

	GetFileName( filename, header->filename );

	OS_Printf(filename);
	OS_Printf(" (%d)  adr:%08x  siz:%05x\n", 
			header->lineNum, (u32)memory, NNS_FndGetSizeForMBlockExpHeap(memBlock));
}

void GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID heapID )
{
	//拡張ヒープから確保したメモリブロック全て（Allocされたもの）に対し、指定した関数を呼ばせる
	NNS_FndVisitAllocatedForExpHeap( GFI_HEAP_GetHandle(heapID), HeapConflictVisitorFunc, 0 );
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
u32 GFL_HEAP_DEBUG_GetMemoryBlockSize( const void* memory )
{
	((u8*)memory) -= MEMHEADER_SIZE;
	return NNS_FndGetSizeForMBlockExpHeap( memory );
}

#endif



#if 0

//==============================================================================================
// デバッグ用ヒープ状態スタック
//==============================================================================================

struct _HEAP_STATE_STACK {
	u16		sp;
	u16		stackNum;
	HEAPID  heapID;
	u64		stack[0];
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
HEAP_STATE_STACK*  HSS_Create( HEAPID heapID, u32 stackNum )
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

#endif










//==============================================================================
/**
 *
 *
 *
 *
 *	ＤＴＣＭ管理
 *
 *
 *
 *
 */
//==============================================================================
#include "heapDTCM.h"

//------------------------------------------------------------------------------
/**
 * システム初期化
 *
 * @param	size		使用サイズ			
 */
//------------------------------------------------------------------------------
void
	GFL_HEAP_DTCM_sysInit
		( u32 size )
{
	if( GFI_HEAP_DTCM_sysInit( size ) == FALSE ){
		OS_Panic("cannot create heap from DTCM size = %x\n", size );
	}
}


//------------------------------------------------------------------------------
/**
 * システム終了
 */
//------------------------------------------------------------------------------
void
	GFL_HEAP_DTCM_sysExit
		( void )
{
	if( GFI_HEAP_DTCM_sysExit() == FALSE ){
		OS_Panic("cannot delete heap for DTCM\n" );
	}
}


//------------------------------------------------------------------
/**
 * ヒープからメモリを確保する
 *
 * @param   size		確保サイズ
 *
 * @retval  void*		確保した領域アドレス（失敗ならNULL）
 */
//------------------------------------------------------------------
void*
	GFL_HEAP_DTCM_AllocMemory
		( u32 size )
{
	void* mem = GFI_HEAP_DTCM_AllocMemory( size );

	if( mem == NULL )
	{
		OS_Printf( "Alloc DTCM_Memory FAILED. allocsize = %x\n", size );
	} else {
//		OS_Printf( "Alloc DTCM_Memory. size = %x\n", size );
	}
	return mem;
}


//------------------------------------------------------------------
/**
 * ヒープからメモリを解放する
 *
 * @param   memory		確保したメモリアドレス
 */
//------------------------------------------------------------------
void
	GFL_HEAP_DTCM_FreeMemory
		( void* memory )
{
	BOOL result = GFI_HEAP_DTCM_FreeMemory( memory );

	if( result == NULL )
	{
		OS_Printf( "Free DTCM_Memory FAILED. memory = %08x\n", memory );
	} else {
//		OS_Printf( "Free DTCM_Memory. memory = %08x\n", memory );
	}
}







