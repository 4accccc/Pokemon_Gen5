//==============================================================================
/**
 *
 *@file		heapsys.h
 *@brief	ヒープ領域管理
 *
 */
//==============================================================================
#ifndef __HEAPSYS_H__
#define __HEAPSYS_H__

#include "heap.h"	//←この中の関数を直接呼び出すのは禁止

#define HEAPSYS_DEBUG

#if 0	/* ↓heap_inter.h 内で外部でも使用する宣言 */
//----------------------------------------------------------------
/**
 *	基本ヒープ作成パラメータ構造体
 */
//----------------------------------------------------------------
typedef struct {
	u32        size;		///< ヒープサイズ
	OSArenaId  arenaID;		///< 作成先アリーナID
}HEAP_INIT_HEADER;

//----------------------------------------------------------------
/**
 *	インライン関数
 */
//----------------------------------------------------------------
static inline  u32 HeapGetLow( u32 heapID )	
{
	return (( heapID & HEAPID_MASK )|( HEAPDIR_MASK ));
}
#endif

//------------------------------------------------------------------------------
/**
 * システム初期化（プログラム起動時に１度だけ呼ばれる）
 *
 * @param   header			親ヒープ初期化構造体へのポインタ
 * @param   baseHeapMax		親ヒープ総数
 * @param   heapMax			親ヒープ・子ヒープ合計数
 * @param   startOffset		ヒープ領域開始オフセット（要４バイトアライン）
 */
//------------------------------------------------------------------------------
extern void
	GFL_HEAP_sysInit
		(const HEAP_INIT_HEADER* header, u32 parentHeapMax, u32 totalHeapMax, u32 startOffset);

//------------------------------------------------------------------------------
/**
 * システム終了
 */
//------------------------------------------------------------------------------
extern void
	GFL_HEAP_sysExit
		( void );

//------------------------------------------------------------------
/**
 * ヒープ作成
 *
 * @param   parentHeapID		メモリ領域確保用ヒープＩＤ（既に有効である必要がある）
 * @param   childHeapID			新規に作成するヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   size				ヒープサイズ
 */
//------------------------------------------------------------------
extern void 
	GFL_HEAP_CreateHeap
		( u32 parentHeapID, u32 childHeapID, u32 size );

extern BOOL sys_CreateHeapLo( u32 parentHeapID, u32 childHeapID, u32 size );

//------------------------------------------------------------------
/**
 * ヒープ破棄
 *
 * @param   childHeapID			破棄するヒープＩＤ（最上位ビットは取得方向フラグ）
 */
//------------------------------------------------------------------
extern void
	GFL_HEAP_DeleteHeap
		( u32 childHeapID );

//------------------------------------------------------------------
/**
 * ヒープからメモリを確保する
 *
 * @param   heapID		ヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   size		確保サイズ
 *
 * @retval  void*		確保した領域アドレス
 *
 *	テンポラリ領域など、すぐに解放する領域は
 *	後方取得マクロ（ヒープＩＤを加工）で確保すれば領域の断片化が起こりづらくなる。
 *
 *　※デバッグビルド時にはマクロでラップして呼び出しソース情報を渡している
 */
//------------------------------------------------------------------
#ifndef HEAPSYS_DEBUG

extern void*
	GFL_HEAP_AllocMemoryblock	//この関数を直接呼び出すのは禁止
		( u32 heapID, u32 size );

#define GFL_HEAP_AllocMemory( ID, siz )		\
			GFL_HEAP_AllocMemoryblock( ID, siz )

#define GFL_HEAP_AllocMemoryLow( ID, siz )	\
			GFL_HEAP_AllocMemoryblock( HeapGetLow(ID), siz )

#else

extern void*
	GFL_HEAP_AllocMemoryblock	//この関数を直接呼び出すのは禁止
		( u32 heapID, u32 size, const char* filename, u32 linenum );

#define GFL_HEAP_AllocMemory( ID, siz )		\
			GFL_HEAP_AllocMemoryblock( ID, siz, __FILE__, __LINE__)

#define GFL_HEAP_AllocMemoryLow( ID, siz )	\
			GFL_HEAP_AllocMemoryblock( HeapGetLow(ID), siz, __FILE__, __LINE__)

#endif

//------------------------------------------------------------------
/**
 * ヒープからメモリを解放する
 *
 * @param   memory		確保したメモリアドレス
 */
//------------------------------------------------------------------
extern void
	GFL_HEAP_FreeMemoryblock	//この関数を直接呼び出すのは禁止
		( void* memory );

#define GFL_HEAP_FreeMemory( mem )	GFL_HEAP_FreeMemoryblock( mem )

//------------------------------------------------------------------
/**
 * NitroSystem ライブラリ系関数が要求するアロケータを作成する
 *
 * @param   pAllocator		NNSFndAllocator構造体のアドレス
 * @param   heapID			ヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   align			確保するメモリブロックに適用するアライメント（負の値は正の値に変換）
 */
//------------------------------------------------------------------
extern void
	GFL_HEAP_InitAllocator
		( NNSFndAllocator* pAllocator, u32 heapID, s32 alignment );

//------------------------------------------------------------------
/**
 * 確保したメモリブロックのサイズを変更する。
 *
 * @param   memory		メモリブロックポインタ
 * @param   newSize		変更後のサイズ（バイト単位）
 */
//------------------------------------------------------------------
extern void
	GFL_HEAP_MemoryResize
		( void* memory, u32 newSize );

//------------------------------------------------------------------
/**
 * ヒープの空き領域サイズを返す
 *
 * @param   heapID	ヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  u32		空き領域サイズ（バイト単位）エラー時は0
 *					errorCode：失敗原因
 */
//------------------------------------------------------------------
extern u32
	GFL_HEAP_GetHeapFreeSize
		( u32 heapID );

//------------------------------------------------------------------
/**
 * ヒープ領域が破壊されていないかチェック
 *
 * @param   heapID	ヒープID（最上位ビットは取得方向フラグ）
 */
//------------------------------------------------------------------
extern void
	GFL_HEAP_CheckHeapSafe
		( u32 heapID );





#if 0
#ifdef HEAP_DEBUG
extern void sys_PrintHeapFreeSize( u32 heapID );
extern void sys_PrintHeapExistMemoryInfo( u32 heapID );
extern u64 sys_GetHeapState( u32 heapID );
extern void sys_PrintHeapConflict( u32 heap, u32 assertionMemSize ); 
extern u32 sys_GetMemoryBlockSize( const void* memBlock );
extern void sys_CheckHeapFullReleased( u32 heapID );
#else
#define sys_PrintHeapFreeSize(h)		/* */
#define sys_PrintHeapExistMemoryInfo(h)	/* */
#define sys_GetHeapState(h)				/* */
#define sys_PrintHeapConflict(h, s)		/* */
#define sys_GetMemoryBlockSize(b)		/* */
#define sys_CheckHeapFullReleased(h)	/* */
#endif


//------------------------------------------------------------------
/*
 * 	ヒープ情報取得（デバッグ時のみ有効）
 */
//------------------------------------------------------------------
typedef struct _HEAP_STATE_STACK	HEAP_STATE_STACK;

#ifdef HEAP_DEBUG
extern HEAP_STATE_STACK*  HSS_Create( u32 heapID, u32 stackNum );
extern void HSS_Push( HEAP_STATE_STACK* hss );
extern void HSS_Pop( HEAP_STATE_STACK* hss );
extern void HSS_Delete( HEAP_STATE_STACK* hss );
#else
#define HSS_Create(a,b,c)	((void)0);
#define HSS_Push(p)			((void)0);
#define HSS_Pop(p)			((void)0);
#define HSS_Delete(p)		((void)0);
#endif

//------------------------------------------------------------------
/*
 * 	簡単メモリリークチェック（現在は無効）
 */
//------------------------------------------------------------------
#define HeapStatePush()		/* */
#define HeapStatePop()		/* */
#define HeapStateCheck(h)	/* */
#endif


#endif	// __HEAPSYS_H__
