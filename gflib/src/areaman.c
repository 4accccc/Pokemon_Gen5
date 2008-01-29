//==============================================================================
/**
 *
 *@file		areaman.c
 *@brief	領域管理
 *@author	taya
 *@data		2006.10.27
 *
 */
//==============================================================================
#include "gflib.h"
//#include "assert.h"

#include "areaman.h"

//----------------------------------------------
// メモリ破壊チェック用マジックナンバー
//----------------------------------------------
#define  MAGIC_NUMBER	(0x573c765a)

//----------------------------------------------
// マネージャ本体の構造体定義
//----------------------------------------------
struct _GFL_AREAMAN {
	u16		numBlocks;
	u16		areaByteSize;
	u32*	pMagicNumber;

	#ifdef AREAMAN_DEBUG
	BOOL	printDebugFlag;
	#endif
	u8		area[0];
};

//----------------------------------------------
// Bit計算省略用テーブル
//----------------------------------------------
static const u8 EmptyCount[256] = {
	8,7,7,6,7,6,6,5,7,6,6,5,6,5,5,4, 7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3,
	7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3, 6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
	7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3, 6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
	6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2, 5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
	7,6,6,5,6,5,5,4,6,5,5,4,5,4,4,3, 6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2,
	6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2, 5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
	6,5,5,4,5,4,4,3,5,4,4,3,4,3,3,2, 5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1,
	5,4,4,3,4,3,3,2,4,3,3,2,3,2,2,1, 4,3,3,2,3,2,2,1,3,2,2,1,2,1,1,0,
};

static const u8 OpenFwdCount[256] = {
	8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const u8 OpenBackCount[256] = {
	8,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0, 4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
	5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0, 4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
	6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0, 4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
	5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0, 4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
	7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0, 4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
	5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0, 4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
	6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0, 4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
	5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0, 4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
};

static const u8 FwdFillBit[9] = {
	0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

static const u8 BackFillBit[9] = {
	0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
};


//==============================================================
// プロトタイプ宣言
//==============================================================
static inline u32 calc_blocks_to_bytes( u32 blocks );
static inline void CHECK_ASSERT( GFL_AREAMAN* man );
static inline u32 get_open_back_count( GFL_AREAMAN* man, u32 bytePos );
static u32 reserveHi_less8bit( GFL_AREAMAN* man, u32 startPos, u32 numBlockArea, u32 numBlockReserve );
static u32 reserveHi( GFL_AREAMAN* man, u32 startPos, u32 numBlockArea, u32 numBlockReserve );
static u32 reserveLo_less8bit( GFL_AREAMAN* man, u32 startPos, u32 numBlockArea, u32 numBlockReserve );
static u32 reserveLo( GFL_AREAMAN* man, u32 startPos, u32 numBlockArea, u32 numBlockReserve );
static GFL_AREAMAN_POS check_empty_bit( u8 baseBitMap, u32 start_bit, u32 num );
static GFL_AREAMAN_POS check_empty_bit_lo( u8 baseBitMap, u32 start_bit, u32 num );
static void reserve_bit( u8* area, u32 start_pos, u32 num );
static void release_bit( u8* area, u32 pos, u32 num );
static void reserve_area( GFL_AREAMAN* man, int pos, u32 blockNum );


#ifdef AREAMAN_DEBUG
static void print_bit( u8 b );
static void print_bit_all( GFL_AREAMAN* man );
static void print_reserveinfo( GFL_AREAMAN* man, u32 pos, u32 blockNum, int pat );
#endif


#define CHECK_ASSERT(man)	\
{							\
	GF_ASSERT(man!=NULL);	\
	GF_ASSERT((*(man->pMagicNumber))==MAGIC_NUMBER);	\
}


static inline u32 calc_blocks_to_bytes( u32 blocks )
{
	u32 size = (blocks / 8) + ((blocks % 8) != 0);
	while( size % 4 ){ size++; }
	return size;
}

static inline u32 get_open_back_count( GFL_AREAMAN* man, u32 bytePos )
{
	if( bytePos < (u32)(man->areaByteSize) )
	{
		return OpenBackCount[ man->area[bytePos] ];
	}
	return 0;
}


//------------------------------------------------------------------
/**
 * 領域マネージャ作成
 *
 * @param   numBlocks		[in] このマネージャーが管理するブロック数	
 * @param   heapID			[in] 使用ヒープＩＤ
 *
 * @retval  GFL_AREAMAN*	領域マネージャポインタ
 */
//------------------------------------------------------------------
GFL_AREAMAN*
	GFL_AREAMAN_Create
		( u32 numBlocks, u16 heapID )
{
	GFL_AREAMAN* man;
	u16 areaByteSize;

	areaByteSize = calc_blocks_to_bytes( numBlocks );
	man = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_AREAMAN)+areaByteSize+sizeof(u32) );

	man->numBlocks = numBlocks;
	man->areaByteSize = areaByteSize;

	man->pMagicNumber = (u32*)(man->area + areaByteSize);
	
	*(man->pMagicNumber) = MAGIC_NUMBER;

	#ifdef AREAMAN_DEBUG
	man->printDebugFlag = FALSE;
	#endif

	GFL_STD_MemFill( man->area, 0, areaByteSize );

	return man;
}


//------------------------------------------------------------------
/**
 * 領域マネージャ破棄
 *
 * @param   man			[in] 領域マネージャポインタ
 */
//------------------------------------------------------------------
void
	GFL_AREAMAN_Delete
		( GFL_AREAMAN* man )
{
	CHECK_ASSERT( man );

	GFL_HEAP_FreeMemory( man );
}


#ifdef AREAMAN_DEBUG
//------------------------------------------------------------------
/**
 * デバッグ出力
 *
 * @param   man		
 * @param   flag		
 */
//------------------------------------------------------------------
void
	GFL_AREAMAN_SetPrintDebug
		( GFL_AREAMAN* man, BOOL flag )
{
	man->printDebugFlag = flag;

	if( flag )
	{
		OS_TPrintf("AreaMan[%08x] Print Start (MaxBlock=%d)\n", (u32)man, man->numBlocks);
	}
}
#endif


//------------------------------------------------------------------
/**
 * 領域先頭から末尾まで、空いている所を探して確保
 *
 * @param   man				[in] マネージャ
 * @param   blockNum		[in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS		確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
u32
	GFL_AREAMAN_ReserveAuto
		( GFL_AREAMAN* man, u32 blockNum )
{
	CHECK_ASSERT( man );

	return GFL_AREAMAN_ReserveAssignArea( man, 0, man->numBlocks, blockNum );
}

//------------------------------------------------------------------
/**
 * 領域末尾から先頭まで、空いている所を探して確保
 *
 * @param   man				[in] マネージャ
 * @param   blockNum		[in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS		確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
u32
	GFL_AREAMAN_ReserveAutoLo
		( GFL_AREAMAN* man, u32 blockNum )
{
	CHECK_ASSERT( man );

	return GFL_AREAMAN_ReserveAssignAreaLo( man, man->numBlocks-1, man->numBlocks, blockNum );
}



//==============================================================================================
/**
 * 領域の指定範囲内から空いている所を探して確保（前方から探索）
 *
 * @param   man				[in] マネージャ
 * @param   startBlock		[in] 探索開始ブロック
 * @param   numBlockArea	[in] 探索ブロック範囲
 * @param   numBlockReserve	[in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS		確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//==============================================================================================
u32
	GFL_AREAMAN_ReserveAssignArea
		( GFL_AREAMAN* man, u32 startBlock, u32 numBlockArea, u32 numBlockReserve )
{
	CHECK_ASSERT( man );

	#ifdef AREAMAN_DEBUG
	if( man->printDebugFlag )
	{
		OS_TPrintf(	"AREAMAN[%08x] reserve %d blocks , start:%d, area:%d\n", 
					(u32)man, numBlockReserve, startBlock, numBlockArea);
	}
	#endif

	if(	(numBlockArea < numBlockReserve)
	||	((startBlock + numBlockReserve) > man->numBlocks )
	){
		#ifdef AREAMAN_DEBUG
		if( man->printDebugFlag )
		{
			OS_TPrintf("** reserve failed %dblocks (pat A) ** \n", numBlockReserve);
		}
		#endif
		return AREAMAN_POS_NOTFOUND;
	}

	{
		u32 ret;

		if( numBlockReserve <= 8 )
		{
			ret = reserveHi_less8bit( man, startBlock, numBlockArea, numBlockReserve );
		}
		else
		{
			ret = reserveHi( man, startBlock, numBlockArea, numBlockReserve );
		}

		return ret;
	}
}


//------------------------------------------------------------------
/**
 * 前方からの探索実処理（8ブロック以下専用）
 *
 * @param   man				[in] マネージャ
 * @param   startPos		[in] 探索開始ブロック
 * @param   numBlockArea	[in] 探索ブロック範囲
 * @param   numBlockReserve	[in] 確保したいブロック数
 *
 * @retval  u32				確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
static u32 reserveHi_less8bit( GFL_AREAMAN* man, u32 startPos, u32 numBlockArea, u32 numBlockReserve )
{
	int returnPos, endPos;
	int bytePos, bytePosEnd, ofc;

	endPos = startPos + numBlockArea + 1 - numBlockReserve;
	bytePos = startPos / 8;
	bytePosEnd = endPos / 8;
	ofc = startPos & 7;

	#ifdef AREAMAN_DEBUG
	if( man->printDebugFlag )
	{
		OS_TPrintf("rHil8bit... endPos=%d, bytePos=%d, ofc=%d, bytePosEnd=%d\n",
			endPos, bytePos, ofc, bytePosEnd);
	}
	#endif

	do {

		returnPos = AREAMAN_POS_NOTFOUND;

		if( (ofc+numBlockReserve) <= 8 )
		{
			u32 p = check_empty_bit( man->area[bytePos], ofc, numBlockReserve );
			if( p != AREAMAN_POS_NOTFOUND )
			{
				returnPos = bytePos*8 + p;
				break;
			}
		}

		for( bytePos++ ; bytePos<=bytePosEnd; bytePos++ )
		{
			if( EmptyCount[ man->area[bytePos] ] >= numBlockReserve )
			{
				u32 p = check_empty_bit( man->area[bytePos], 0, numBlockReserve );
				returnPos = bytePos*8 + p;
				break;
			}
			else if( OpenBackCount[ man->area[bytePos] ] && (bytePos < bytePosEnd) )
			{
				u32  rem = numBlockReserve - OpenBackCount[ man->area[bytePos] ];

				if( OpenFwdCount[ man->area[bytePos+1] ] >= rem )
				{
					returnPos = bytePos*8 + (8-OpenBackCount[ man->area[bytePos] ]);
					break;
				}
			}
		}

	}while(0);


	if( returnPos != (int)AREAMAN_POS_NOTFOUND )
	{
		if( returnPos <= endPos )
		{
			reserve_area( man, returnPos, numBlockReserve );

			#ifdef AREAMAN_DEBUG
			print_reserveinfo( man, returnPos, numBlockReserve, __LINE__ );
			#endif

			return returnPos;
		}
	}

	return AREAMAN_POS_NOTFOUND;
}

//------------------------------------------------------------------
/**
 * 前方からの探索実処理（8ブロックより大きい領域用）
 *
 * @param   man				[in] マネージャ
 * @param   startPos		[in] 探索開始ブロック
 * @param   numBlockArea	[in] 探索ブロック範囲
 * @param   numBlockReserve	[in] 確保したいブロック数
 *
 * @retval  u32				確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
static u32 reserveHi( GFL_AREAMAN* man, u32 startPos, u32 numBlockArea, u32 numBlockReserve )
{
	u32 returnPos, endPos;
	int bytePos, bytePosEnd, obc;

	endPos = startPos + numBlockArea - numBlockReserve;
	bytePos = startPos / 8;
	bytePosEnd = endPos / 8;

	obc = 8 - (startPos % 8);
	if( OpenBackCount[ man->area[bytePos] ] < obc )
	{
		obc = OpenBackCount[ man->area[bytePos] ];
	}

//	OS_TPrintf("[RSHI] startPos:%d, endPos:%d, bytePos:%d, bytePosEnd:%d, startObc:%d\n", 
//		startPos, endPos, bytePos, bytePosEnd, obc);


	while( bytePos <= bytePosEnd )
	{
		if( obc )
		{
			int remBlocks, remBytes, remBits, p;

			remBlocks = numBlockReserve - obc;
			remBytes = remBlocks / 8;
			remBits = remBlocks % 8;
			p = bytePos + 1;

//			OS_TPrintf("  bytePos:%d, remBlocks:%d, remBits:%d, remBytes:%d\n",
//						bytePos, remBlocks, remBits, remBytes );

			while( remBytes )
			{
				if( man->area[p] ){ break; }
				p++;
				remBytes--;
			}

			if( remBytes == 0 )
			{
				int returnPos = AREAMAN_POS_NOTFOUND;

//				OS_TPrintf("    remBytes is clear! prevByte:%d, ofc:%d\n",
//						man->area[p-1], OpenFwdCount[man->area[p]]);

				if( remBits == 0 )
				{
					returnPos = (bytePos*8) + (8 - obc);
				}
				else
				{
					if( OpenFwdCount[man->area[p]] >= remBits )
					{
						returnPos = (bytePos*8) + (8 - obc);
					}
				}

				if( returnPos != (int)(AREAMAN_POS_NOTFOUND) )
				{
					reserve_area( man, returnPos, numBlockReserve );
					#ifdef AREAMAN_DEBUG
					print_reserveinfo( man, returnPos, numBlockReserve, __LINE__ );
					#endif
					return returnPos;
				}
			}
		}
		if( ++bytePos < bytePosEnd )
		{
			obc = OpenBackCount[ man->area[bytePos] ];
		}
	}

	return AREAMAN_POS_NOTFOUND;

}

//==============================================================================================
/**
 * 領域の指定範囲内から空いている所を探して確保（後方から探索）
 *
 * @param   man				[in] マネージャ
 * @param   startBlock		[in] 探索開始ブロック（100を指定したら、100,99,98 ... と探索する）
 * @param   numBlockArea	[in] 探索ブロック範囲
 * @param   numBlockReserve	[in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS		確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//==============================================================================================
u32
	GFL_AREAMAN_ReserveAssignAreaLo
		( GFL_AREAMAN* man, u32 startBlock, u32 numBlockArea, u32 numBlockReserve )
{
	CHECK_ASSERT( man );

	#ifdef AREAMAN_DEBUG
	if( man->printDebugFlag )
	{
		OS_TPrintf("[AREAMAN] startBlock:%d, numBlockArea:%d, numBlockReserve:%d\n",
				startBlock, numBlockArea, numBlockReserve );
	}
	#endif

	GF_ASSERT_MSG( numBlockArea >= numBlockReserve, "areasize:%d, reserve:%d\n", 
					numBlockArea, numBlockReserve );
	GF_ASSERT( startBlock < man->numBlocks );
	GF_ASSERT( (int)numBlockReserve <= (int)(startBlock+1) );

	#ifdef AREAMAN_DEBUG
	if( man->printDebugFlag )
	{
		OS_TPrintf(	"AREAMAN[%08x] reserve %d blocks , area:%d~%d\n", 
					(u32)man, numBlockReserve, startBlock, startBlock-numBlockArea+1);
	}
	#endif

	if(numBlockArea < numBlockReserve)
	{
		#ifdef AREAMAN_DEBUG
		if( man->printDebugFlag )
		{
			OS_TPrintf("** reserve failed %dblocks (pat A) ** \n", numBlockReserve);
		}
		#endif
		return AREAMAN_POS_NOTFOUND;
	}

	{
		u32 ret;

		if( numBlockReserve <= 8 )
		{
			ret = reserveLo_less8bit( man, startBlock, numBlockArea, numBlockReserve );
		}
		else
		{
			ret = reserveLo( man, startBlock, numBlockArea, numBlockReserve );
		}

		#ifdef AREAMAN_DEBUG
		print_reserveinfo( man, ret, numBlockReserve, __LINE__ );
		#endif

		return ret;
	}
}
//------------------------------------------------------------------
/**
 * 後方探索実処理（8ブロック以下専用）
 *
 * @param   man				[in] マネージャ
 * @param   startPos		[in] 探索開始ブロック（100を指定したら、100,99,98 ... と探索する）
 * @param   numBlockArea	[in] 探索ブロック範囲
 * @param   numBlockReserve	[in] 確保したいブロック数
 *
 * @retval  u32				確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
static u32 reserveLo_less8bit( GFL_AREAMAN* man, u32 startPos, u32 numBlockArea, u32 numBlockReserve )
{
	int returnPos, endPos;
	int bytePos, bytePosEnd, ofc;

	endPos = startPos - numBlockArea + 1;
	bytePos = startPos / 8;
	bytePosEnd = endPos / 8;
	ofc = startPos & 7;

	do {

		returnPos = AREAMAN_POS_NOTFOUND;

		if( ofc )
		{
			u32 p = check_empty_bit_lo( man->area[bytePos], ofc-1, numBlockReserve );
			if( p != AREAMAN_POS_NOTFOUND )
			{
				returnPos = bytePos*8 + p;
				break;
			}
		}

		for( bytePos-- ; bytePos>=bytePosEnd; bytePos-- )
		{
			if( EmptyCount[ man->area[bytePos] ] >= numBlockReserve )
			{
				u32 p = check_empty_bit_lo( man->area[bytePos], 7, numBlockReserve );
				returnPos = bytePos*8 + p;
				break;
			}
			else if( OpenFwdCount[ man->area[bytePos] ] && (bytePos > bytePosEnd) )
			{
				u32  rem = numBlockReserve - OpenFwdCount[ man->area[bytePos] ];

				if( OpenBackCount[ man->area[bytePos-1] ] >= rem )
				{
					returnPos = (bytePos-1)*8 + (8-rem);
					break;
				}
			}
		}

	}while(0);


	if( returnPos != (int)AREAMAN_POS_NOTFOUND )
	{
		if( returnPos >= endPos )
		{
			reserve_area( man, returnPos, numBlockReserve );
			return returnPos;
		}
	}

	return AREAMAN_POS_NOTFOUND;
}

//------------------------------------------------------------------
/**
 * 後方探索実処理（8ブロックより大きい領域用）
 *
 * @param   man				[in] マネージャ
 * @param   startPos		[in] 探索開始ブロック（100を指定したら、100,99,98 ... と探索する）
 * @param   numBlockArea	[in] 探索ブロック範囲
 * @param   numBlockReserve	[in] 確保したいブロック数
 *
 * @retval  u32				確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
static u32 reserveLo( GFL_AREAMAN* man, u32 startPos, u32 numBlockArea, u32 numBlockReserve )
{
	u32 returnPos, endPos;
	int bytePos, bytePosEnd, ofc;

	endPos = startPos - numBlockArea + 1;
	bytePos = startPos / 8;
	bytePosEnd = endPos / 8;
	ofc = (startPos % 8) + 1;

	if( OpenFwdCount[ man->area[bytePos] ] < ofc )
	{
		ofc = OpenFwdCount[ man->area[bytePos] ];
	}

//	OS_TPrintf("startPos:%d, endPos:%d, bytePos:%d, bytePosEnd:%d, startOfc:%d\n", 
//		startPos, endPos, bytePos, bytePosEnd, ofc);

	while( bytePos >= bytePosEnd )
	{
		if( ofc )
		{
			int remBlocks, remBytes, remBits, p;

			remBlocks = numBlockReserve - ofc;
			remBytes = remBlocks / 8;
			remBits = remBlocks % 8;
			p = bytePos - 1;

//			OS_TPrintf("  bytePos:%d, remBlocks:%d, remBits:%d, remBytes:%d\n",
//						bytePos, remBlocks, remBits, remBytes );

			while( remBytes )
			{
				if( p < bytePosEnd ){ break; }
				if( man->area[p] ){ break; }
				p--;
				remBytes--;
			}

			if( remBytes == 0 )
			{
				int returnPos = AREAMAN_POS_NOTFOUND;

//				OS_TPrintf("    remBytes is clear! prevByte:%d, obc:%d\n",
//						man->area[p-1], OpenBackCount[man->area[p-1]]);

				if( remBits == 0 )
				{
					returnPos = (p+1)*8;
				}
				else
				{
					if( p >= bytePosEnd )
					{
						if( OpenBackCount[man->area[p]] >= remBits )
						{
							returnPos = p*8 + (8-remBits);
						}
					}
				}

				if( (returnPos != (int)(AREAMAN_POS_NOTFOUND))
				&&	(returnPos >= endPos )
				){
					reserve_area( man, returnPos, numBlockReserve );
					return returnPos;
				}
			}
		}
		if( --bytePos >= bytePosEnd )
		{
			ofc = OpenFwdCount[ man->area[bytePos] ];
		}
	}

	return AREAMAN_POS_NOTFOUND;

}

//------------------------------------------------------------------
/**
 * 領域の指定位置から確保
 *
 * @param   man				[in] マネージャ
 * @param   blockNum		[in] 確保したいブロック数
 *
 * @retval  BOOL			TRUEで成功
 */
//------------------------------------------------------------------
BOOL
	GFL_AREAMAN_ReserveAssignPos
		( GFL_AREAMAN* man, GFL_AREAMAN_POS pos, u32 blockNum )
{
	CHECK_ASSERT( man );

	#ifdef AREAMAN_DEBUG
	if( man->printDebugFlag )
	{
		OS_TPrintf("AREAMAN[%08x] AP reserve %d blocks , start:%d\n", (u32)man, blockNum, pos);
	}
	#endif

	reserve_area( man, pos, blockNum );

	#ifdef AREAMAN_DEBUG
	print_reserveinfo( man, pos, blockNum, __LINE__ );
	#endif

	return TRUE;
}


//------------------------------------------------------------------
/**
 * 領域を開放
 *
 * @param   man			[in] マネージャ
 * @param   pos			[in] 確保している位置
 * @param   blockNum	[in] 確保しているブロック数
 */
//------------------------------------------------------------------
void 
	GFL_AREAMAN_Release
		( GFL_AREAMAN* man, GFL_AREAMAN_POS pos, u32 blockNum )
{
	CHECK_ASSERT( man );
	GF_ASSERT((pos+blockNum) <= man->numBlocks);

	#ifdef AREAMAN_DEBUG
	if( man->printDebugFlag )
	{
		OS_TPrintf("AREAMAN[%08x] release %d blocks , start:%d\n", (u32)man, blockNum, pos);
	}
	#endif

	{
		int p;
		p = pos % 8;
		pos /= 8;

		if( (p+blockNum) <= 8 )
		{
			release_bit( &man->area[pos], p, blockNum );
		}
		else
		{
			int bits, bytes, rem;

			p = 8-p;
			bits = blockNum - p;
			bytes = bits / 8;
			rem = bits % 8;

			GF_ASSERT( (man->area[pos] & (BackFillBit[p])) == BackFillBit[p] );
			man->area[pos++] &= (~BackFillBit[p]);

			while(bytes--)
			{
				GF_ASSERT(man->area[pos]==0xff);
				man->area[pos++] = 0x00;
			}

			GF_ASSERT( (man->area[pos] & (FwdFillBit[rem])) == FwdFillBit[rem] );
			man->area[pos] &= (~FwdFillBit[rem]);
		}
	}


	#ifdef AREAMAN_DEBUG
	print_bit_all( man );
	#endif

}


//------------------------------------------------------------------
/**
 * 領域データ1byte分の中に、指定数だけ連続した空きブロックが含まれているかチェック（前方から検索）
 *
 * @param   baseBitMap		チェックする領域データ1byte分
 * @param   start_bit		チェック開始bit（7～0：7が最後方）
 * @param   num				
 *
 * @retval  GFL_AREAMAN_POS		
 */
//------------------------------------------------------------------
static GFL_AREAMAN_POS check_empty_bit( u8 baseBitMap, u32 start_bit, u32 num )
{
	if( num <= 8 )
	{
		u32 i, max;
		u8  inbit;

		inbit = FwdFillBit[num] >> start_bit;
		max = 8-num;

		for(i=start_bit; i<=max; i++)
		{
			if( (baseBitMap & inbit) == 0 )
			{
				return i;
			}
			inbit >>= 1;
		}
	}

	return AREAMAN_POS_NOTFOUND;
}
//------------------------------------------------------------------
/**
 * 領域データ1byte分の中に、指定数だけ連続した空きブロックが含まれているかチェック（後方から検索）
 *
 * @param   baseBitMap		チェックする領域データ1byte分
 * @param   start_bit		チェック開始bit（7～0：7が最後方）
 * @param   num				
 *
 * @retval  GFL_AREAMAN_POS		
 */
//------------------------------------------------------------------
static GFL_AREAMAN_POS check_empty_bit_lo( u8 baseBitMap, u32 start_bit, u32 num )
{
	if( num < 8 )
	{
		int i;
		u8  inbit;

		start_bit = (start_bit + 1 - num);

		inbit = FwdFillBit[num] >> start_bit;

		for(i=start_bit; i>=0; i--)
		{
			if( (baseBitMap & inbit) == 0 )
			{
				return i;
			}
			inbit <<= 1;
		}
	}

	return AREAMAN_POS_NOTFOUND;
}




static void reserve_bit( u8* area, u32 start_pos, u32 num )
{
	GF_ASSERT(num <= 8);

	{
		u8 bit = FwdFillBit[num] >> start_pos;
		if( ((*area) & bit) == 0 )
		{
			(*area) |= bit;
		}
		else
		{
			GF_ASSERT(0);
		}
	}
}


static void release_bit( u8* area, u32 pos, u32 num )
{
	GF_ASSERT((pos+num)<=8);

	{
		u8 bit = FwdFillBit[num] >> pos;
		GF_ASSERT( (bit&(*area)) == bit );
		(*area) &= (~bit);
	}
}


static void reserve_area( GFL_AREAMAN* man, int pos, u32 blockNum )
{
	int start_pos, start_bit_count, start_bit, rem, bytes;

	GF_ASSERT_MSG((pos+blockNum)<=man->numBlocks, "pos=%d,blockNum=%d\n", pos, blockNum);

	start_pos = pos % 8;
	start_bit_count = 8 - start_pos;
	pos /= 8;

	if( start_bit_count >= blockNum )
	{
		start_bit = FwdFillBit[blockNum] >> start_pos;
		bytes = 0;
		rem = 0;
	}
	else
	{
		start_bit = BackFillBit[start_bit_count];
		bytes = (blockNum - start_bit_count) / 8;
		rem = (blockNum - start_bit_count) % 8;
	}


	if( (man->area[pos] & start_bit) != 0 )
	{
		#ifdef AREAMAN_DEBUG
		if( man->printDebugFlag )
		{
			OS_TPrintf("pos=%d, start_bit=");
			print_bit( start_bit );
			OS_TPrintf(" area=");
			print_bit( man->area[pos] );
		}
		#endif

		GF_ASSERT(0);
	}


	man->area[pos++] |= start_bit;
	while( bytes-- )
	{
		GF_ASSERT(man->area[pos]==0);
		man->area[pos++] = 0xff;
	}
	if( rem )
	{
		GF_ASSERT((man->area[pos]&FwdFillBit[rem])==0);
		man->area[pos] |= FwdFillBit[rem];
	}
}


#ifdef AREAMAN_DEBUG
static void print_bit( u8 b )
{
	static const char* let[] = { "0", "1" };

	OS_PutString( let[ (b>>7)&1 ] );
	OS_PutString( let[ (b>>6)&1 ] );
	OS_PutString( let[ (b>>5)&1 ] );
	OS_PutString( let[ (b>>4)&1 ] );
	OS_PutString( let[ (b>>3)&1 ] );
	OS_PutString( let[ (b>>2)&1 ] );
	OS_PutString( let[ (b>>1)&1 ] );
	OS_PutString( let[ (b>>0)&1 ] );

#if 0
	OS_TPrintf("%d", (b>>7)&1);
	OS_TPrintf("%d", (b>>6)&1);
	OS_TPrintf("%d", (b>>5)&1);
	OS_TPrintf("%d", (b>>4)&1);
	OS_TPrintf("%d", (b>>3)&1);
	OS_TPrintf("%d", (b>>2)&1);
	OS_TPrintf("%d", (b>>1)&1);
	OS_TPrintf("%d", (b>>0)&1);
#endif
}

static void print_bit_all( GFL_AREAMAN* man )
{
	if( man->printDebugFlag )
	{
		int i;

		for(i=0; i<man->areaByteSize; i++)
		{
			print_bit(man->area[i]);
			OS_TPrintf(" ");
			if((i+1)%8==0)
			{
				OS_TPrintf("\n");
			}
		}
		OS_TPrintf("\n");
	}
}

static void print_reserveinfo( GFL_AREAMAN* man, u32 pos, u32 blockNum, int line )
{
	if( man->printDebugFlag )
	{
		OS_TPrintf("reserve %d to %d (%dblocks) - line:%d\n", pos, pos+blockNum-1, blockNum, line);
		print_bit_all( man );
	}
}

#endif
