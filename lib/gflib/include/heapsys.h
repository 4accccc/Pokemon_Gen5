//==============================================================================
/**
 *
 *@file   heapsys.h
 *@brief  ヒープ領域管理
 *
 */
//==============================================================================
#ifndef __HEAPSYS_H__
#define __HEAPSYS_H__

#include <gf_standard.h>
#include <heap.h> //←この中の関数を直接呼び出すのは禁止

#ifdef __cplusplus
extern "C" {
#endif

#define HEAPSYS_DEBUG

//==============================================================================
/**
 *
 *
 *
 *
 *  メインアリーナ管理
 *
 *
 *
 *
 */
//==============================================================================
//----------------------------------------------------------------
/**
 *  定数
 */
//----------------------------------------------------------------
#define GFL_HEAPID_SYSTEM (0) // 基本ヒープＩＤ（INDEX:0 は必ずシステム定義にすること）

//----------------------------------------------------------------
/**
 *  インライン関数
 */
//----------------------------------------------------------------
inline  HEAPID GetHeapLowID( HEAPID heapID )
{
  return (( heapID & HEAPID_MASK )|( HEAPDIR_MASK ));
}

static inline HEAPID GFL_HEAP_LOWID( HEAPID heapID )
{
  return (( heapID & HEAPID_MASK )|( HEAPDIR_MASK ));
}


//------------------------------------------------------------------------------
/**
 * システム初期化（プログラム起動時に１度だけ呼ばれる）
 *
 * @param   header      親ヒープ初期化構造体へのポインタ
 * @param   baseHeapMax   親ヒープ総数
 * @param   heapMax     親ヒープ・子ヒープ合計数
 * @param   startOffset   ヒープ領域開始オフセット（要４バイトアライン）
 */
//------------------------------------------------------------------------------
extern void
  GFL_HEAP_Init
    (const HEAP_INIT_HEADER* header, u32 parentHeapMax, u32 totalHeapMax, u32 startOffset);

//------------------------------------------------------------------------------
/**
 * システム終了
 */
//------------------------------------------------------------------------------
extern void
  GFL_HEAP_Exit
    ( void );

//------------------------------------------------------------------
/**
 * ヒープ作成
 *
 * @param   parentHeapID    メモリ領域確保用ヒープＩＤ（既に有効である必要がある）
 * @param   childHeapID     新規に作成するヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   size        ヒープサイズ
 */
//------------------------------------------------------------------
extern void
  GFL_HEAP_CreateHeap
    ( HEAPID parentHeapID, HEAPID childHeapID, u32 size );

inline void
  GFL_HEAP_CreateHeapLo
    ( HEAPID parentHeapID, HEAPID childHeapID, u32 size )
{
  GFL_HEAP_CreateHeap( parentHeapID, GetHeapLowID(childHeapID), size );
}


extern void
  GFL_HEAP_CreateHeapInBuffer
    ( void* buffer, u32 bufferSize, HEAPID childHeapID );

//------------------------------------------------------------------
/**
 * ヒープ破棄
 *
 * @param   childHeapID     破棄するヒープＩＤ（最上位ビットは取得方向フラグ）
 */
//------------------------------------------------------------------
extern void
  GFL_HEAP_DeleteHeap
    ( HEAPID childHeapID );

//------------------------------------------------------------------
/**
 * ヒープからメモリを確保する
 *
 * @param   heapID    ヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   size    確保サイズ
 *
 * @retval  void*   確保した領域アドレス
 *
 *  テンポラリ領域など、すぐに解放する領域は
 *  後方取得マクロ（ヒープＩＤを加工）で確保すれば領域の断片化が起こりづらくなる。
 *
 *　※デバッグビルド時にはマクロでラップして呼び出しソース情報を渡している
 */
//------------------------------------------------------------------
#ifndef HEAPSYS_DEBUG

extern void*
  GFL_HEAP_AllocMemoryblock //この関数を直接呼び出すのは禁止
    ( HEAPID heapID, u32 size, BOOL clearFlag );

#define GFL_HEAP_AllocMemory( heapID, size ) \
      GFL_HEAP_AllocMemoryblock( heapID, size, FALSE )

#define GFL_HEAP_AllocMemoryLo( heapID, size )  \
      GFL_HEAP_AllocMemoryblock( GFL_HEAP_LOWID(heapID), size, FALSE )

#define GFL_HEAP_AllocClearMemory( heapID, size ) \
    GFL_HEAP_AllocMemoryblock( heapID, size, TRUE )

#define GFL_HEAP_AllocClearMemoryLo( heapID, size ) \
    GFL_HEAP_AllocMemoryblock( GFL_HEAP_LOWID(heapID), size )

#else

extern void*
  GFL_HEAP_AllocMemoryblock //この関数を直接呼び出すのは禁止
    ( HEAPID heapID, u32 size, BOOL clearFlag, const char* filename, u16 linenum );

#define GFL_HEAP_AllocMemory( heapID, size )  \
      GFL_HEAP_AllocMemoryblock( heapID, size, FALSE, __FILE__, __LINE__)

#define GFL_HEAP_AllocMemoryLo( heapID, size )  \
      GFL_HEAP_AllocMemoryblock( GFL_HEAP_LOWID(heapID), size, FALSE, __FILE__, __LINE__)

#define GFL_HEAP_AllocClearMemory( heapID, size ) \
    GFL_HEAP_AllocMemoryblock( heapID, size, TRUE, __FILE__, __LINE__)

#define GFL_HEAP_AllocClearMemoryLo( heapID, size ) \
    GFL_HEAP_AllocMemoryblock( GFL_HEAP_LOWID(heapID), size, TRUE, __FILE__, __LINE__)

#endif  // #ifndef HEAPSYS_DEBUG


//------------------------------------------------------------------
/**
 * ヒープからメモリを解放する
 *
 * @param   memory    確保したメモリアドレス
 */
//------------------------------------------------------------------
extern void
  GFL_HEAP_FreeMemoryblock  //この関数を直接呼び出すのは禁止
    ( void* memory );

#define GFL_HEAP_FreeMemory( mem )  GFL_HEAP_FreeMemoryblock( mem )

//------------------------------------------------------------------
/**
 * NitroSystem ライブラリ系関数が要求するアロケータを作成する
 *
 * @param   pAllocator    NNSFndAllocator構造体のアドレス
 * @param   heapID      ヒープＩＤ（最上位ビットは取得方向フラグ）
 * @param   align     確保するメモリブロックに適用するアライメント（負の値は正の値に変換）
 */
//------------------------------------------------------------------
extern void
  GFL_HEAP_InitAllocator
    ( NNSFndAllocator* pAllocator, HEAPID heapID, s32 alignment );

//------------------------------------------------------------------
/**
 * 確保したメモリブロックのサイズを変更する。
 *
 * @param   memory    メモリブロックポインタ
 * @param   newSize   変更後のサイズ（バイト単位）
 */
//------------------------------------------------------------------
extern void
  GFL_HEAP_ResizeMemory
    ( void* memory, u32 newSize );

//------------------------------------------------------------------
/**
 * ヒープの空き領域サイズを返す
 *
 * @param   heapID  ヒープＩＤ（最上位ビットは取得方向フラグ）
 *
 * @retval  u32   空き領域サイズ（バイト単位）エラー時は0
 *          errorCode：失敗原因
 */
//------------------------------------------------------------------
extern u32
  GFL_HEAP_GetHeapFreeSize
    ( HEAPID heapID );

//------------------------------------------------------------------
/**
 * ヒープ領域が破壊されていないかチェック
 *
 * @param   heapID  ヒープID（最上位ビットは取得方向フラグ）
 */
//------------------------------------------------------------------
extern BOOL
  GFL_HEAP_CheckHeapSafe
    ( HEAPID heapID );


//------------------------------------------------------------------
//------------------------------------------------------------------
/*
 *  ヒープ情報取得（デバッグ時のみ有効）
 */
//------------------------------------------------------------------
//------------------------------------------------------------------

#ifdef HEAPSYS_DEBUG
extern void GFL_HEAP_DEBUG_PrintUnreleasedMemoryCheck ( HEAPID heapID );
extern void GFL_HEAP_DEBUG_PrintExistMemoryBlocks ( HEAPID heapID );
extern u32  GFL_HEAP_DEBUG_GetMemoryBlockSize ( const void* memory );
extern void GFL_HEAP_DEBUG_StartPrint( HEAPID heapID );
extern void GFL_HEAP_DEBUG_EndPrint( void );
extern void GFL_HEAP_DEBUG_PrintSystemInfo( void );
#else
#define GFL_HEAP_DEBUG_PrintUnreleasedMemoryCheck ( heapID ) /* */
#define GFL_HEAP_DEBUG_PrintExistMemoryBlocks ( heapID ) /* */
#define GFL_HEAP_DEBUG_GetMemoryBlockSize ( memory )  /* */
#define GFL_HEAP_DEBUG_StartPrint(heapID)  /* */
#define GFL_HEAP_DEBUG_EndPrint()  /* */
#define GFL_HEAP_PrintDebugInfo(()  /* */

#endif




//==============================================================================
/**
 *
 *
 *
 *
 *  ＤＴＣＭ管理
 *        DTCMエリアの使用設定をします。
 *        領域幅が少ないので（16k=0x4000）、
 *        メインアリーナを使用するheap.cとは異なり、
 *        単一ヒープのみの簡易的な作りにしてあります。
 *        初期化時に使用サイズの設定を行いますが
 *        基本的にスタックおよび関数内メモリに割り当てられるエリアなので
 *        大きなサイズは控えるようにしてください。
 *        現状は0x3000-0x80を最大サイズとしています。
 */
//==============================================================================
//------------------------------------------------------------------------------
/**
 * システム初期化
 *
 * @param size    使用サイズ
 */
//------------------------------------------------------------------------------
extern void
  GFL_HEAP_DTCM_Init
    ( u32 size );

//------------------------------------------------------------------------------
/**
 * システム終了
 */
//------------------------------------------------------------------------------
extern void
  GFL_HEAP_DTCM_Exit
    ( void );

//------------------------------------------------------------------
/**
 * ヒープからメモリを確保する
 *
 * @param   size    確保サイズ
 *
 * @retval  void*   確保した領域アドレス（失敗ならNULL）
 */
//------------------------------------------------------------------
extern void*
  GFL_HEAP_DTCM_AllocMemory
    ( u32 size );

//------------------------------------------------------------------
/**
 * ヒープからメモリを解放する
 *
 * @param   memory    確保したメモリアドレス
 */
//------------------------------------------------------------------
extern void
  GFL_HEAP_DTCM_FreeMemory
    ( void* memory );










#if 0
typedef struct _HEAP_STATE_STACK  HEAP_STATE_STACK;

#ifdef HEAP_DEBUG
extern HEAP_STATE_STACK*  HSS_Create( HEAPID heapID, u32 stackNum );
extern void HSS_Push( HEAP_STATE_STACK* hss );
extern void HSS_Pop( HEAP_STATE_STACK* hss );
extern void HSS_Delete( HEAP_STATE_STACK* hss );
#else
#define HSS_Create(a,b,c) ((void)0);
#define HSS_Push(p)     ((void)0);
#define HSS_Pop(p)      ((void)0);
#define HSS_Delete(p)   ((void)0);
#endif

//------------------------------------------------------------------
/*
 *  簡単メモリリークチェック（現在は無効）
 */
//------------------------------------------------------------------
#define HeapStatePush()   /* */
#define HeapStatePop()    /* */
#define HeapStateCheck(h) /* */
#endif

#ifdef __cplusplus
}/* extern "C" */
#endif


#endif  // __HEAPSYS_H__
