//==============================================================================
/**
 *
 *@file		heap.c
 *@brief	ヒープ領域管理
 *
 * Description:  システムヒープとアプリケーションヒープを作成します。両ヒープ
 * 				にはNITRO-Systemの拡張ヒープを使用しています。
 *
 * 				システムヒープ用のメモリとして、SYSTEM_HEAP_SIZE分をメインア
 * 				リーナから確保し、メインアリーナの残りを全てアプリケーション
 * 				ヒープ用のメモリに確保しています。
 *
 * 				システムヒープは、ゲームシステム等のシステムプログラムで使用
 * 				することを想定しています。アプリケーションヒープには、ゲーム
 * 				で使用するデータをロードする為に使用します。
 *
 */
//==============================================================================
#include <nitro.h>
#include <nnsys.h>
#include "heap.h"

//----------------------------------------------------------------
/**
 *	定数
 */
//----------------------------------------------------------------
#define DEFAULT_ALIGN_MASK			(3)			// メモリ確保時のアライメント用マスク
#define HEAP_MAX					(24)		// 一度に作成可能な子ヒープの数
#define INVALID_HANDLE_IDX			(255)		// ヒープハンドル無効値
//#define MAGICNUM					(6478)		// メモリ管理ヘッダマジックナンバー

//----------------------------------------------------------------
/**
 *	ワーク定義
 */
//----------------------------------------------------------------
typedef struct {
	NNSFndHeapHandle	handle;
	NNSFndHeapHandle	parentHandle;
	void*				heapMemBlock;
	u16					count;

}HEAP_HANDLE_DATA;

typedef struct {
	HEAP_HANDLE_DATA*	hhd;
	u8*					handleIdxTbl;

	u16					heapMax;
	u16					parentHeapMax;
	u16					usableHeapMax;

	u16					errorCode;
}HEAP_SYS;

//----------------------------------------------------------------
/**
 *	メモリブロックヘッダ定義
 *
 *	４バイト境界を意識する。ＤＳはライブラリでケアされているので
 *	マジックナンバーは使わない。（専用関数を使ってメモリ破壊チェックを行う）
 */
//----------------------------------------------------------------
typedef struct {
	u16		heapID;						///< u32
//	u16		magicnum;					///< マジックナンバー
	u8		userInfo[MEMHEADER_USERINFO_SIZE];	///< 外部使用領域
}MEMHEADER;

//----------------------------------------------------------------
/**
 *	変数定義
 */
//----------------------------------------------------------------
static HEAP_SYS  HeapSys = { 0 };

//----------------------------------------------------------------
/**
 *	ハンドル取得マクロ
 */
//----------------------------------------------------------------
#define HeapHandleIdxTbl( idx )			(HeapSys.handleIdxTbl[ idx ])

#define HeapHandle( num )				(HeapSys.hhd[ num ].handle)
#define ParentHeapHandle( num )			(HeapSys.hhd[ num ].parentHandle)
#define HeapMemBlock( num )				(HeapSys.hhd[ num ].heapMemBlock)
#define HeapCount( num )				(HeapSys.hhd[ num ].count)

#define GetHeapHandle( idx )			(HeapHandle( HeapHandleIdxTbl( idx ) ))
#define GetParentHeapHandle( idx )		(ParentHeapHandle( HeapHandleIdxTbl( idx ) ))
#define GetHeapMemBlock( idx )			(HeapMemBlock( HeapHandleIdxTbl( idx ) ))
#define GetHeapCount( idx )				(HeapCount( HeapHandleIdxTbl( idx ) ))

#define SetHeapHandle( idx, val )		(HeapHandle( HeapHandleIdxTbl( idx ) ) = ( val ))
#define SetParentHeapHandle( idx, val )	(ParentHeapHandle( HeapHandleIdxTbl( idx ) ) = ( val ))
#define SetHeapMemBlock( idx, val )		(HeapMemBlock( HeapHandleIdxTbl( idx ) ) = ( val ))
#define SetHeapCount( idx, val )		(HeapCount( HeapHandleIdxTbl( idx ) ) = ( val ))
		
//==============================================================
// プロトタイプ宣言
//==============================================================

//------------------------------------------------------------------------------
/**
 * システム初期化
 *
 * @param   header			初期化情報
 * @param   parentHeapMax	親（基本）ヒープエリア数
 * @param   totalHeapMax	全ヒープエリア数
 * @param   startOffset		与えられたバイト数だけ全体の開始位置をずらす
 *
 * @retval  BOOL			TRUEで成功／FALSEで失敗
 *							errorCode：確保できなかった親ヒープＩＤ
 */
//------------------------------------------------------------------------------
BOOL
	GFI_HEAP_sysInit
		(const HEAP_INIT_HEADER* header, u16 parentHeapMax, u16 totalHeapMax, u32 startOffset)
{
	void *mem;
	u32  usableHeapMax, i;

	//同時使用ヒープ量の設定
	usableHeapMax = parentHeapMax + HEAP_MAX;
	//使用ヒープが少なくても一定量の確保は行う
	if( totalHeapMax < usableHeapMax )
	{
		totalHeapMax = usableHeapMax;
	}

	// 開始オフセットをアラインメント(4byte)にあわせつつずらす(メモリ解析しづらくさせるため)
	if( startOffset )
	{
		while( startOffset & DEFAULT_ALIGN_MASK )
		{
			startOffset++;
		}
		OS_AllocFromMainArenaLo( startOffset, DEFAULT_ALIGN );
	}

	// ヒープハンドルデータ領域確保
	HeapSys.hhd = OS_AllocFromMainArenaLo( sizeof(HEAP_HANDLE_DATA)*(usableHeapMax), DEFAULT_ALIGN);

	for(i = 0; i < usableHeapMax; i++ )	// ハンドルデータの初期化(無効値でクリア)
	{
		HeapHandle(i)		= NNS_FND_HEAP_INVALID_HANDLE;
		ParentHeapHandle(i)	= NNS_FND_HEAP_INVALID_HANDLE;
		HeapMemBlock(i)		= 0;
		HeapCount(i)		= 0;
	}

	// ヒープINDEX領域確保
	HeapSys.handleIdxTbl = OS_AllocFromMainArenaLo( totalHeapMax, DEFAULT_ALIGN);
	for(i = 0; i < totalHeapMax; i++ )	// INDEXの初期化(無効値でクリア)
	{
		HeapHandleIdxTbl(i)	= INVALID_HANDLE_IDX;	
	}

	HeapSys.heapMax			= totalHeapMax;
	HeapSys.parentHeapMax	= parentHeapMax;
	HeapSys.usableHeapMax	= usableHeapMax;
	HeapSys.errorCode		= 0;

	// 基本ヒープ＆インデックス作成、(iはヒープＩＤと同義)
	for(i = 0; i < parentHeapMax; i++)
	{
		mem = OS_AllocFromMainArenaLo( header[i].size, DEFAULT_ALIGN );

		if( mem != NULL )
		{
			HeapHandle(i) = NNS_FndCreateExpHeap( mem, header[i].size );//ヒープ確保＆ハンドル取得
			HeapHandleIdxTbl(i)	= i;//INDEXテーブルの設定
		} else {
			HeapSys.errorCode	= i;
			return FALSE;
		}
	}
	return TRUE;
}


//------------------------------------------------------------------------------
/**
 * システム終了
 *
 * @retval  BOOL			TRUEで成功／FALSEで失敗
 */
//------------------------------------------------------------------------------
BOOL
	GFI_HEAP_sysExit
		( void )
{
	return TRUE;
}


//------------------------------------------------------------------
/**
 * ヒープ作成
 *
 * @param   parentHeapID	メモリ領域確保用ヒープＩＤ（既に有効である必要がある）
 * @param   childHeapID		新規に作成するヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   size			ヒープサイズ
 *
 * @retval  BOOL			TRUEで作成成功／FALSEで失敗
 *							errorCode：失敗原因
 */
//------------------------------------------------------------------
BOOL 
	GFI_HEAP_CreateHeap
		( u16 parentHeapID, u16 childHeapID, u32 size )
{
	s32 align;

	if( childHeapID & HEAPDIR_MASK )	//確保方法の取得
	{
		align = -DEFAULT_ALIGN;	//後方から確保
	} else {
		align = DEFAULT_ALIGN;	//前方から確保
	}

	childHeapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( HeapHandleIdxTbl( childHeapID ) != INVALID_HANDLE_IDX )
	{	//二重登録
		HeapSys.errorCode = HEAP_CANNOT_CREATE_DOUBLE;
	} else {
		NNSFndHeapHandle  parentHeap = GetHeapHandle( parentHeapID );//親ヒープハンドル取得

		if( parentHeap == NNS_FND_HEAP_INVALID_HANDLE )
		{	//親ヒープ無効
			HeapSys.errorCode = HEAP_CANNOT_CREATE_NOPARENT;
		} else {
			void* mem = NNS_FndAllocFromExpHeapEx( parentHeap, size, align );	//メモリ確保
			if( mem == NULL )
			{	//メモリ不足
				HeapSys.errorCode = HEAP_CANNOT_CREATE_HEAP;
			} else {
				int i;
				// 基本ヒープは解放されない前提で、以降をサーチ
				for( i = HeapSys.parentHeapMax; i < HeapSys.usableHeapMax; i++ )
				{
					if( HeapHandle(i) == NNS_FND_HEAP_INVALID_HANDLE )
					{
						HeapHandle(i) = NNS_FndCreateExpHeap( mem, size );	//ハンドル作成
						if( HeapHandle(i) == NNS_FND_HEAP_INVALID_HANDLE )
						{	//ヒープハンドル作成失敗
							HeapSys.errorCode = HEAP_CANNOT_CREATE_HEAPHANDLE;
						} else {
							ParentHeapHandle(i) = parentHeap;	//親ハンドルの保存
							HeapMemBlock(i) = mem;				//領域ポインタ保存
							HeapHandleIdxTbl(childHeapID) = i;	//INDEXテーブルの設定
	
							HeapSys.errorCode = 0;
							return TRUE;
						}
					}
				}
				HeapSys.errorCode = HEAP_CANNOT_CREATE_HEAPTABLE;	//ヒープ作成量オーバー
			}
		}
	}
	return FALSE;
}


//------------------------------------------------------------------
/**
 * ヒープ破棄
 *
 * @param   childHeapID		破棄するヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  BOOL			TRUEで破棄成功／FALSEで失敗
 *							errorCode：失敗原因
 */
//------------------------------------------------------------------
BOOL
	GFI_HEAP_DeleteHeap
		( u16 childHeapID )
{
	NNSFndHeapHandle  handle;

	childHeapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	handle = GetHeapHandle( childHeapID );	//破棄ヒープハンドル取得

	if( handle == NNS_FND_HEAP_INVALID_HANDLE )
	{	//多重破棄
		HeapSys.errorCode = HEAP_CANNOT_DELETE_DOUBLE;
	} else {
		NNSFndHeapHandle	parentHandle = GetParentHeapHandle( childHeapID );
		void*				heapMemBlock = GetHeapMemBlock( childHeapID );

		NNS_FndDestroyExpHeap( handle );	//ヒープの破棄

		if(( parentHandle == NNS_FND_HEAP_INVALID_HANDLE )||( heapMemBlock == NULL ))
		{	//親ヒープ無効
			HeapSys.errorCode = HEAP_CANNOT_DELETE_NOPARENT;
		} else {
			NNS_FndFreeToExpHeap( parentHandle, heapMemBlock );	//メモリブロックの解放

			// ハンドルデータの初期化(無効値でクリア)
			SetHeapHandle( childHeapID, NNS_FND_HEAP_INVALID_HANDLE );
			SetParentHeapHandle( childHeapID, NNS_FND_HEAP_INVALID_HANDLE );
			SetHeapMemBlock( childHeapID, NULL );
			SetHeapCount( childHeapID, 0 );

			// INDEXの初期化(無効値でクリア)
			HeapHandleIdxTbl( childHeapID ) = INVALID_HANDLE_IDX;
			return TRUE;
		}
	}
	return FALSE;
}


//------------------------------------------------------------------
/**
 * ヒープからメモリを確保する
 *
 * @param   heapID		ヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   size		確保サイズ
 *
 * @retval  void*		確保した領域アドレス（失敗ならNULL）
 *						errorCode：失敗原因
 */
//------------------------------------------------------------------
void*
	GFI_HEAP_AllocMemory
		( u16 heapID, u32 size )
{
	s32 align;

	if( heapID & HEAPDIR_MASK )	//確保方法の取得
	{
		align = -DEFAULT_ALIGN;	//後方から確保
	} else {
		align = DEFAULT_ALIGN;	//前方から確保
	}

	heapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( heapID >= HeapSys.heapMax )
	{	//指定ＩＤが無効
		HeapSys.errorCode = HEAP_CANNOT_ALLOC_NOID;
	} else {
		NNSFndHeapHandle  handle = GetHeapHandle( heapID );	//ヒープハンドル取得
		if( handle == NNS_FND_HEAP_INVALID_HANDLE )
		{	//指定ＩＤヒープが登録されていない
			HeapSys.errorCode = HEAP_CANNOT_ALLOC_NOHEAP;
		} else {
			u32			memsiz	= size + sizeof(MEMHEADER);	//メモリ管理ヘッダ追加
			OSIntrMode	irqold	= OS_DisableInterrupts();	//割り込みを禁止
			void*		memory	= NNS_FndAllocFromExpHeapEx( handle, memsiz, align );//メモリ確保

			if( memory == NULL )
			{	//メモリ不足
				HeapSys.errorCode = HEAP_CANNOT_ALLOC_MEM;
			} else {
				MEMHEADER* memheader = (MEMHEADER*)memory;

//				memheader->magicnum = MAGICNUM;				//ヘッダにマジックナンバーを設定
				memheader->heapID = heapID;					//ヘッダにヒープＩＤを保存
				(u8*)memory += sizeof(MEMHEADER);			//実メモリ領域へポインタ移動
				GetHeapCount(heapID)++;						//メモリ確保カウンタのインクリメント
				HeapSys.errorCode = 0;
			} 
			OS_RestoreInterrupts( irqold );	//割り込みを復帰

			return memory;
		}
	}
	return NULL;
}


//------------------------------------------------------------------
/**
 * ヒープからメモリを解放する
 *
 * @param   memory		確保したメモリアドレス
 *
 * @retval  BOOL		TRUEで解放成功／FALSEで失敗
 *						errorCode：失敗原因
 */
//------------------------------------------------------------------
BOOL
	GFI_HEAP_FreeMemory
		( void* memory )
{
	MEMHEADER* memheader = (MEMHEADER*)((u8*)memory - sizeof(MEMHEADER));
//	u16	magicnum	= memheader->magicnum;	//メモリ管理ヘッダマジックナンバーの取得
	u16	heapID		= memheader->heapID;	//対象ヒープＩＤの取得
	
//	if( magicnum != MAGICNUM )
//	{	//指定ポインタに間違いがある
//		HeapSys.errorCode = HEAP_CANNOT_FREE_NOBLOCK;
//	} else {
		if( heapID >= HeapSys.heapMax )
		{	//保存ＩＤヒープが異常
			HeapSys.errorCode = HEAP_CANNOT_FREE_NOID;
		} else {
			NNSFndHeapHandle  handle = GetHeapHandle( heapID );	//ヒープハンドル取得
			if( handle == NNS_FND_HEAP_INVALID_HANDLE)
			{	//保存ＩＤヒープが取得したハンドルが未登録状態
				HeapSys.errorCode = HEAP_CANNOT_FREE_NOHEAP;
			} else {
				if( GetHeapCount(heapID) == 0 )
				{	//メモリ確保カウンタに異常（存在していない）
					HeapSys.errorCode = HEAP_CANNOT_FREE_NOMEM;
				} else {
					OSIntrMode	irqold	= OS_DisableInterrupts();	//割り込みを禁止
	
					GetHeapCount(heapID)--;	//メモリ確保カウンタのデクリメント
					NNS_FndFreeToExpHeap( handle, memheader );	//メモリブロックの解放
					OS_RestoreInterrupts( irqold );	//割り込みを復帰

					HeapSys.errorCode = 0;
					return TRUE;
				}
			}
		}
//	}
	return FALSE;
}


//------------------------------------------------------------------
/**
 * NitroSystem ライブラリ系関数が要求するアロケータを作成する
 *
 * @param   pAllocator		NNSFndAllocator構造体のアドレス
 * @param   heapID			ヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   align			確保するメモリブロックに適用するアライメント（負の値は正の値に変換）
 *
 * @retval  BOOL			TRUEで成功／FALSEで失敗
 *							errorCode：失敗原因
 */
//------------------------------------------------------------------
BOOL
	GFI_HEAP_InitAllocator
		( NNSFndAllocator* pAllocator, u16 heapID, s32 alignment )
{
	s32 align;

	if( alignment < 0 )
	{
		alignment = -alignment;	//正の値に強制変換
	}
	if( heapID & HEAPDIR_MASK )	//確保方法の取得
	{
		align = -alignment;	//後方から確保
	} else {
		align = alignment;	//前方から確保
	}

	heapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( heapID >= HeapSys.heapMax )
	{	//指定ＩＤが無効
		HeapSys.errorCode = HEAP_CANNOT_INITALLOCATER_NOID;
	} else {
		NNS_FndInitAllocatorForExpHeap( pAllocator, GetHeapHandle( heapID ), align );

		HeapSys.errorCode = 0;
		return TRUE;
	}
	return FALSE;
}


//------------------------------------------------------------------
/**
 * 確保したメモリブロックのサイズを変更する。
 *
 * @param   memory		メモリブロックポインタ
 * @param   newSize		変更後のサイズ（バイト単位）
 *
 * @retval  BOOL		TRUEで成功／FALSEで失敗
 *						errorCode：失敗原因
 */
//------------------------------------------------------------------
BOOL
	GFI_HEAP_MemoryResize
		( void* memory, u32 newSize )
{
	MEMHEADER* memheader = (MEMHEADER*)((u8*)memory - sizeof(MEMHEADER));
//	u16	magicnum	= memheader->magicnum;	//メモリ管理ヘッダマジックナンバーの取得
	u16	heapID		= memheader->heapID;	//対象ヒープＩＤの取得

	newSize += sizeof(MEMHEADER);	// ヘッダ領域追加

//	if( magicnum != MAGICNUM )
//	{	//指定ポインタに間違いがある
//		HeapSys.errorCode = HEAP_CANNOT_MEMRESIZE_NOBLOCK;
//	} else {
		if( heapID >= HeapSys.heapMax )
		{	//保存ＩＤヒープが異常
			HeapSys.errorCode = HEAP_CANNOT_MEMRESIZE_NOID;	
		} else {
			NNSFndHeapHandle  handle = GetHeapHandle( heapID );	//ヒープハンドル取得
			if( handle == NNS_FND_HEAP_INVALID_HANDLE)
			{	//保存ＩＤヒープが取得したハンドルが未登録状態
				HeapSys.errorCode = HEAP_CANNOT_MEMRESIZE_NOHEAP;
			} else {
				u32		oldSize		= NNS_FndGetSizeForMBlockExpHeap( memheader );
				u32		resultsize	= NNS_FndResizeForMBlockExpHeap( handle, memheader, newSize );
				BOOL	result		= TRUE;
				HeapSys.errorCode = 0;

				if( oldSize < newSize )
				{
					if( resultsize == 0 )
					{	//メモリ不足で拡大出来ない
						HeapSys.errorCode = HEAP_CANNOT_MEMRESIZE_LARGE;
						result = FALSE;
					}
				} else {
					if( resultsize == oldSize )
					{	//差分が少なすぎて縮小出来ない
						HeapSys.errorCode = HEAP_CANNOT_MEMRESIZE_SMALL;
						result = FALSE;
					}
				}
				return result;
			}
		}
//	}
	return FALSE;
}


//------------------------------------------------------------------
/**
 * ヒープの空き領域サイズを返す
 *
 * @param   heapID	ヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  u32		空き領域サイズ（バイト単位）、取得失敗時は0
 *					errorCode：失敗原因
 */
//------------------------------------------------------------------
u32
	GFI_HEAP_GetHeapFreeSize
		( u16 heapID )
{
	heapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( heapID >= HeapSys.heapMax )
	{	//指定ＩＤが無効
		HeapSys.errorCode = HEAP_CANNOT_GETSIZE_NOID;
	} else {
		HeapSys.errorCode = 0;
		return NNS_FndGetTotalFreeSizeForExpHeap( GetHeapHandle( heapID ) );
	}
	return 0;
}


//------------------------------------------------------------------
/**
 * ハンドル情報の取得
 *
 * @param   heapID				ヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  NNSFndHeapHandle*	該当するハンドル、取得失敗時はNULL
 *								errorCode：失敗原因
 */
//------------------------------------------------------------------
NNSFndHeapHandle
	GFI_HEAP_GetHandle
		( u16 heapID )
{
	heapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( heapID >= HeapSys.heapMax )
	{	//指定ＩＤが無効
		HeapSys.errorCode = HEAP_CANNOT_GETHANDLE_NOID;
	} else {
		HeapSys.errorCode = 0;
		return GetHeapHandle( heapID );
	}
	return NULL;
}


//------------------------------------------------------------------
/**
 * 親ハンドル情報の取得
 *
 * @param   heapID				ヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  NNSFndHeapHandle*	該当するヒープを所持している親ハンドル、取得失敗時はNULL
 *								errorCode：失敗原因
 */
//------------------------------------------------------------------
NNSFndHeapHandle
	GFI_HEAP_GetParentHandle
		( u16 heapID )
{
	heapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( heapID >= HeapSys.heapMax )
	{	//指定ＩＤが無効
		HeapSys.errorCode = HEAP_CANNOT_GETPARENTHANDLE_NOID;
	} else {
		HeapSys.errorCode = 0;
		return GetParentHeapHandle( heapID );
	}
	return NULL;
}


//------------------------------------------------------------------
/**
 * メモリブロック情報の取得
 *
 * @param   heapID	ヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  void*	該当するヒープのポインタ、取得失敗時はNULL
 *					errorCode：失敗原因
 */
//------------------------------------------------------------------
void*
	GFI_HEAP_GetHeapMemBlock
		( u16 heapID )
{
	heapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( heapID >= HeapSys.heapMax )
	{	//指定ＩＤが無効
		HeapSys.errorCode = HEAP_CANNOT_GETHEAPMEMBLOCK_NOID;
	} else {
		HeapSys.errorCode = 0;
		return GetHeapMemBlock( heapID );
	}
	return NULL;
}


//------------------------------------------------------------------
/**
 * アロケート情報の取得
 *
 * @param   heapID	ヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  u16		該当するヒープでアロケートされている数、取得失敗時は0xff
 *					errorCode：失敗原因
 */
//------------------------------------------------------------------
u16
	GFI_HEAP_GetHeapCount
		( u16 heapID )
{
	heapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( heapID >= HeapSys.heapMax )
	{	//指定ＩＤが無効
		HeapSys.errorCode = HEAP_CANNOT_GETHEAPCOUNT_NOID;
	} else {
		HeapSys.errorCode = 0;
		return GetHeapCount( heapID );
	}
	return 0xffff;
}


//------------------------------------------------------------------
/**
 * メモリヘッダ上ヒープＩＤの取得
 *
 * @param   memory	確保したメモリアドレス
 *
 * @retval  u32		ヒープＩＤ、取得失敗時は0xff
 *					errorCode：失敗原因
 */
//------------------------------------------------------------------
u16
	GFI_HEAP_GetMemheaderHeapID
		( void* memory )
{
	MEMHEADER* memheader = (MEMHEADER*)((u8*)memory - sizeof(MEMHEADER));
//	u16	magicnum	= memheader->magicnum;	//メモリ管理ヘッダマジックナンバーの取得
	
//	if( magicnum != MAGICNUM )
//	{	//指定ポインタに間違いがある
//		HeapSys.errorCode = HEAP_CANNOT_GETMEMHEADERHEAPID_NOBLOCK;
//	} else {
		return memheader->heapID;
//	}
	return 0xffff;
}


//------------------------------------------------------------------
/**
 * メモリヘッダ上ユーザー情報ポインタの取得
 *
 * @param   memory	確保したメモリアドレス
 *
 * @retval  u32		ユーザー情報ポインタ、取得失敗時はNULL
 *					errorCode：失敗原因
 */
//------------------------------------------------------------------
void*
	GFI_HEAP_GetMemheaderUserinfo
		( void* memory )
{
	MEMHEADER* memheader = (MEMHEADER*)((u8*)memory - sizeof(MEMHEADER));
//	u16	magicnum	= memheader->magicnum;	//メモリ管理ヘッダマジックナンバーの取得
	
//	if( magicnum != MAGICNUM )
//	{	//指定ポインタに間違いがある
//		HeapSys.errorCode = HEAP_CANNOT_GETMEMHEADERUSERINFO_NOBLOCK;
//	} else {
		return memheader->userInfo;
//	}
	return NULL;
}


//------------------------------------------------------------------
/**
 * ヒープ領域が破壊されていないかチェック
 *
 * @param   heapID		ヒープID
 *
 * @retval  BOOL		破壊されていなければTRUE
 */
//------------------------------------------------------------------
BOOL
	GFI_HEAP_CheckHeapSafe
		( u16 heapID )
{
	heapID &= HEAPID_MASK;	//実ヒープＩＤの取得

	if( heapID >= HeapSys.heapMax )
	{	//指定ＩＤが無効
		HeapSys.errorCode = HEAP_CANNOT_CHECKHEAPSAFE_NOID;
	} else {
		NNSFndHeapHandle handle = GetHeapHandle( heapID );
		if( handle == NNS_FND_HEAP_INVALID_HANDLE )
		{	//取得したハンドルが未登録状態
			HeapSys.errorCode = HEAP_CANNOT_CHECKHEAPSAFE_NOHEAP;
		} else {
			BOOL result = NNS_FndCheckExpHeap( handle, NNS_FND_HEAP_ERROR_PRINT );
			if( result == FALSE )
			{ 	//ヒープ領域が破壊されている
				HeapSys.errorCode = HEAP_CANNOT_CHECKHEAPSAFE_DESTROY;
			} else {
				HeapSys.errorCode = 0;
				return TRUE;
			}
		}
	}
	return FALSE;
}


//------------------------------------------------------------------
/**
 * エラーコードの取得
 *
 * @retval  int		エラーコード
 */
//------------------------------------------------------------------
u32
	GFI_HEAP_ErrorCodeGet
		( void )
{
	return (u32)HeapSys.errorCode;
}




