//==============================================================================
/**
 *
 *@file		vman.c
 *@brief	VRAM領域マネージャ
 *@author	taya
 *@data		2006.11.28
 *
 */
//==============================================================================

#include "heapsys.h"
#include "assert.h"
#include "areaman.h"

#include "vman.h"


#define NELEMS(ary)		(sizeof(ary)/sizeof(ary[0]))


enum {
	RESERVEINFO_INIT_POS = 0xffffffff,
	RESERVEINFO_INIT_SIZE = 0xffffffff,
};


//==============================================================
// Prototype
//==============================================================
static u32 calc_objtype_unit_bytesize( u32 vramSize );
static inline u32 calc_block_size( u32 byteSize, u32 unitSize );



u32 GFI_VRAM_CalcVramSize( u32 vramBank )
{
	static const struct {
		u32  bank;
		u32  size;
	}VramState[] = {
		{ GX_VRAM_A, 128 },
		{ GX_VRAM_B, 128 },
		{ GX_VRAM_C, 128 },
		{ GX_VRAM_D, 128 },
		{ GX_VRAM_E,  64 },
		{ GX_VRAM_F,  16 },
		{ GX_VRAM_G,  16 },
		{ GX_VRAM_H,  32 },
		{ GX_VRAM_I,  16 },
	};

	int i;
	u32 size = 0;

	for(i=0; i<NELEMS(VramState); i++)
	{
		if( vramBank & VramState[i].bank )
		{
			size += VramState[i].size;
		}
	}

	return size * 1024;
}


//==============================================================================================
// マネージャ実体定義
//==============================================================================================
struct _GFL_VMAN{
	AREAMAN*   areaMan;
	u32        unitByteSize;
};

//==============================================================================================
// メイン起動時の初期化
//==============================================================================================
void GFL_VMAN_sysInit( void )
{

}
void GFL_VMAN_sysExit( void )
{
	
}
//==============================================================================================
//==============================================================================================

//==============================================================================================
/**
 * マネージャ作成
 *
 * @param   heapID			[in] ヒープID
 * @param   type			[in] マネージャタイプ（BG or OBJ）
 * @param   vramBank		[in] VRAMバンク指定
 *
 * @retval  GFL_VMAN*	作成したマネージャオブジェクトへのポインタ
 */
////==============================================================================================
GFL_VMAN* GFL_VMAN_Create( u32 heapID, GFL_VMAN_TYPE type, GXVRamOBJ vramBank )
{
	GFL_VMAN*  vm;
	u32 vramSize;

	vm = sys_AllocMemory( heapID, sizeof(GFL_VMAN) );
	vramSize = GFI_VRAM_CalcVramSize( vramBank );

	if( type == GFL_VMAN_TYPE_OBJ )
	{
		vm->unitByteSize = calc_objtype_unit_bytesize( vramSize );
	}
	else
	{
		vm->unitByteSize = 0x20;
	}

	vm->areaMan = AREAMAN_Create( vramSize / vm->unitByteSize, heapID );

	return vm;
}
//==============================================================================================
/**
 * マネージャ破棄
 *
 * @param   man		[in] マネージャオブジェクトへのポインタ
 *
 */
//==============================================================================================
void GFL_VMAN_Delete( GFL_VMAN* man )
{
	AREAMAN_Delete( man->areaMan );
	sys_FreeMemoryEz( man );
}

//==============================================================================================
/**
 * 領域確保情報のデバッガ出力ON/OFF
 *
 * @param   man		[in] マネージャオブジェクトへのポインタ
 *
 */
//==============================================================================================
void GFL_VMAN_SetPrintDebug( GFL_VMAN* man, BOOL flag )
{
	AREAMAN_SetPrintDebug( man->areaMan, flag );
}

//------------------------------------------------------------------
/**
 * VRAMのトータルサイズから、管理する単位バイト数を計算
 *
 * @param   vramSize		VRAMトータルサイズ（バイト）
 *
 * @retval  u32		管理単位バイト数
 */
//------------------------------------------------------------------
static u32 calc_objtype_unit_bytesize( u32 vramSize )
{
	vramSize /= 1024;

	if( vramSize <= 32 )
	{
		return 32;
	}
	return vramSize;
}


//==============================================================================================
//==============================================================================================


//==============================================================================================
/**
 * 予約領域情報構造体の初期化（※初期化＝無効な値が保存されている状態）
 *
 * @param   reserveInfo		[in] 初期化する構造体へのポインタ
 *
 */
//==============================================================================================
void GFL_VMAN_InitReserveInfo( GFL_VMAN_RESERVE_INFO* reserveInfo )
{
	reserveInfo->pos = RESERVEINFO_INIT_POS;
	reserveInfo->size = RESERVEINFO_INIT_SIZE;
}
//==============================================================================================
/**
 * 予約領域情報構造体が初期化されているか調べる
 *
 * @param   reserveInfo		[in] 構造体へのポインタ
 *
 * @retval  BOOL			TRUEなら初期化されている
 */
//==============================================================================================
BOOL GFL_VMAN_IsReserveInfoInitialized( const GFL_VMAN_RESERVE_INFO* reserveInfo )
{
	return (reserveInfo->pos==RESERVEINFO_INIT_POS) && (reserveInfo->size==RESERVEINFO_INIT_SIZE);
}



//==============================================================================================
/**
 * 領域を予約する（先頭から終端まで、空いている所を探す）
 *
 * @param   man			[in]  マネージャ
 * @param   size		[in]  予約したいバイトサイズ
 * @param   reserveInfo	[out] 予約領域情報を保存するための構造体
 *
 * @retval  u32			予約できたらTRUE, できなかったらFALSE
 */
//==============================================================================================
BOOL GFL_VMAN_Reserve( GFL_VMAN* man, u32 byteSize, GFL_VMAN_RESERVE_INFO* reserveInfo )
{
	GF_ASSERT( man != NULL );
	GF_ASSERT( reserveInfo != NULL );
	GF_ASSERT( (byteSize % man->unitByteSize) == 0 );

	{
		AREAMAN_POS pos;
		u32 blocks;

		blocks = byteSize / man->unitByteSize;
		pos = AREAMAN_ReserveAuto( man->areaMan, blocks );

		if( pos != AREAMAN_POS_NOTFOUND )
		{
			reserveInfo->pos = pos;
			reserveInfo->size = blocks;
			return TRUE;
		}

		return FALSE;
	}
}
//==============================================================================================
/**
 * 領域を予約する（指定範囲内で、空いている所を探す）
 *
 * @param   man				[in]  マネージャ
 * @param   reserveSize		[in]  予約したいバイトサイズ
 * @param   startOffset		[in]  探索開始領域バイトオフセット
 * @param   areaSize		[in]  探索範囲バイトサイズ
 * @param   reserveInfo		[out] 予約領域情報を保存するための構造体ポインタ
 *
 * @retval  u32			予約できたらTRUE, できなかったらFALSE
 */
//==============================================================================================
BOOL GFL_VMAN_ReserveAssignArea( GFL_VMAN* man, u32 reserveSize, u32 startOffset, u32 areaSize, GFL_VMAN_RESERVE_INFO* reserveInfo )
{
	GF_ASSERT( man != NULL );
	GF_ASSERT( reserveInfo != NULL );
	GF_ASSERT( (reserveSize % man->unitByteSize) == 0 );
	GF_ASSERT( (startOffset % man->unitByteSize) == 0 );
	GF_ASSERT( (areaSize % man->unitByteSize) == 0 );

	{
		AREAMAN_POS pos;

		startOffset /= man->unitByteSize;
		areaSize /= man->unitByteSize;
		reserveSize /= man->unitByteSize;

		pos = AREAMAN_ReserveAssignArea( man->areaMan, startOffset, areaSize, reserveSize );

		if( pos != AREAMAN_POS_NOTFOUND )
		{
			reserveInfo->pos = pos;
			reserveInfo->size = reserveSize;
			return TRUE;
		}
	}

	return FALSE;
}

//==============================================================================================
/**
 * 
 *
 * @param   man				
 * @param   reserveSize		
 * @param   offset			
 * @param   reserveInfo		
 *
 * @retval  BOOL		
 */
//==============================================================================================
BOOL GFL_VMAN_ReserveFixPos( GFL_VMAN* man, u32 reserveSize, u32 offset, GFL_VMAN_RESERVE_INFO* reserveInfo )
{
	GF_ASSERT( man != NULL );
	GF_ASSERT( reserveInfo != NULL );
	GF_ASSERT( (reserveSize % man->unitByteSize) == 0 );
	GF_ASSERT( (offset % man->unitByteSize) == 0 );

	offset /= man->unitByteSize;
	reserveSize /= man->unitByteSize;

	if( AREAMAN_ReserveAssignPos( man->areaMan, offset, reserveSize ) )
	{
		reserveInfo->pos = offset;
		reserveInfo->size = reserveSize;
		return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------
/**
 * バイト数→管理ブロック数へ変換
 *
 * @param   byteSize		バイト数
 * @param   unitSize		単位バイト数
 *
 * @retval  u32		管理ブロック数
 */
//------------------------------------------------------------------
static inline u32 calc_block_size( u32 byteSize, u32 unitSize )
{
	if( byteSize <= unitSize )
	{
		return 1;
	}
	else
	{
		return byteSize / unitSize;
	}
}

//==============================================================================================
/**
 * 予約した領域の解放
 *
 * @param   man			[in] マネージャ
 * @param   reserveInfo	[in] 予約領域情報
 *
 */
//==============================================================================================
void GFL_VMAN_Release( GFL_VMAN* man, GFL_VMAN_RESERVE_INFO* reserveInfo )
{
	GF_ASSERT(man != NULL);
	GF_ASSERT(reserveInfo!=NULL);
	GF_ASSERT(GFL_VMAN_IsReserveInfoInitialized(reserveInfo)==FALSE);

	AREAMAN_Release( man->areaMan, reserveInfo->pos, reserveInfo->size );
	GFL_VMAN_InitReserveInfo( reserveInfo );
}

//==============================================================================================
/**
 * 予約した領域の、VRAM先頭からのバイトオフセットを求める
 *
 * @param   man				[in] マネージャ
 * @param   reserveInfo		[in] 予約領域情報
 *
 * @retval  u32				VRAM先頭からのバイトオフセット
 */
//==============================================================================================
u32 GFL_VMAN_GetByteOffset( GFL_VMAN* man, const GFL_VMAN_RESERVE_INFO* reserveInfo )
{
	GF_ASSERT(man != NULL);
	GF_ASSERT(reserveInfo!=NULL);
	GF_ASSERT(reserveInfo->pos != RESERVEINFO_INIT_POS);

	return reserveInfo->pos * man->unitByteSize;
}
