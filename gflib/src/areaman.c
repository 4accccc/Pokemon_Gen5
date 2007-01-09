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
	u16		maxBlock;
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
static GFL_AREAMAN_POS check_empty_bit( u8 baseBitMap, u32 start_bit, u32 num );
static void reserve_bit( u8* area, u32 start_pos, u32 num );
static void release_bit( u8* area, u32 pos, u32 num );
static void reserve_area( GFL_AREAMAN* man, int pos, u32 blockNum );

#ifdef AREAMAN_DEBUG
static void print_bit( u8 b );
static void print_bit_all( GFL_AREAMAN* man );
static void print_reserveinfo( GFL_AREAMAN* man, u32 pos, u32 blockNum, int pat );
#endif

static inline u32 calc_blocks_to_bytes( u32 blocks )
{
	return blocks / 8;
}

static inline void CHECK_ASSERT( GFL_AREAMAN* man )
{
	GF_ASSERT(man!=NULL);
	GF_ASSERT((*(man->pMagicNumber))==MAGIC_NUMBER);
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
 * @param   maxBlock		[in] このマネージャーが管理するブロック数	
 * @param   heapID			[in] 使用ヒープＩＤ
 *
 * @retval  GFL_AREAMAN*	領域マネージャポインタ
 */
//------------------------------------------------------------------
GFL_AREAMAN*
	GFL_AREAMAN_Create
		( u32 maxBlock, u16 heapID )
{
	GFL_AREAMAN* man;
	u16 areaByteSize;

	areaByteSize = calc_blocks_to_bytes( maxBlock );
	man = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_AREAMAN)+areaByteSize+sizeof(u32) );

	man->maxBlock = maxBlock;
	man->areaByteSize = areaByteSize;

	man->pMagicNumber = (u32*)(man->area + areaByteSize);
	*(man->pMagicNumber) = MAGIC_NUMBER;

	#ifdef AREAMAN_DEBUG
	man->printDebugFlag = FALSE;
	#endif

	GFL_STD_MemFill( man->area, 0, areaByteSize);

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
		OS_TPrintf("AreaMan[%08x] Print Start (MaxBlock=%d)\n", (u32)man, man->maxBlock);
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

	return GFL_AREAMAN_ReserveAssignArea( man, 0, man->maxBlock, blockNum );
}


//------------------------------------------------------------------
/**
 * 領域の指定範囲内から空いている所を探して確保
 *
 * @param   man				[in] マネージャ
 * @param   startBlock		[in] 探索開始ブロック
 * @param   numBlockArea	[in] 探索ブロック範囲
 * @param   numBlockReserve	[in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS		確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
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
	||	((startBlock + numBlockReserve) > man->maxBlock )
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
		u32 p, rp, obc;
		u32 startBlockLimit;

		startBlockLimit = startBlock + (numBlockArea - numBlockReserve);

		p = startBlock >> 3;
		rp = startBlock & 7;

		if( rp )
		{
			u32 bitpos = check_empty_bit( man->area[p], rp, numBlockReserve );
			if( bitpos != AREAMAN_POS_NOTFOUND )
			{
				reserve_bit( &(man->area[p]), bitpos, numBlockReserve );
				#ifdef AREAMAN_DEBUG
				if( man->printDebugFlag )
				{
					print_reserveinfo( man, p*8+bitpos, numBlockReserve, __LINE__ );
				}
				#endif
				return p*8 + bitpos;
			}
			if( (OpenBackCount[man->area[p]] + rp) >= 8 )
			{
				obc = 8 - rp;
			}
			else
			{
				p++;
				obc = OpenBackCount[ man->area[p] ];
			}
		}
		else
		{
			obc = OpenBackCount[ man->area[p] ];
		}

		while( p<man->areaByteSize )
		{
			if( EmptyCount[man->area[p]] >= numBlockReserve )
			{
				u32 bitpos, retpos;

				bitpos = check_empty_bit( man->area[p], 0, numBlockReserve );
				retpos = p*8+bitpos;
				if( retpos < startBlockLimit )
				{
					reserve_bit( &(man->area[p]), bitpos, numBlockReserve );
					#ifdef AREAMAN_DEBUG
					if( man->printDebugFlag )
					{
						print_reserveinfo( man, retpos, numBlockReserve, __LINE__ );
					}
					#endif
					return retpos;
				}
				else
				{
					#ifdef AREAMAN_DEBUG
					if( man->printDebugFlag )
					{
						OS_TPrintf("** reserve failed %dblocks (pat B) ** \n", numBlockReserve);
					}
					#endif
					return AREAMAN_POS_NOTFOUND;
				}
			}
			if( obc )
			{
				u32 num, rem, pp;

				if( obc > numBlockReserve )
				{
					num = 0;
					rem = 0;
				}
				else
				{
					num = (numBlockReserve - obc) >> 3;
					rem = (numBlockReserve - obc) & 7;
				}

				pp = p + 1;

				while( num )
				{
					if( pp >= man->areaByteSize )
					{
						#ifdef AREAMAN_DEBUG
						if( man->printDebugFlag )
						{
							OS_TPrintf("** reserve failed %dblocks (pat C) ** \n", numBlockReserve);
						}
						#endif
						return AREAMAN_POS_NOTFOUND;
					}
					if( man->area[pp] == 0 )
					{
						num--;
						pp++;
					}
					else
					{
						break;
					}
				}

				if(	(num == 0)
				&&	( (rem == 0)||(OpenFwdCount[man->area[pp]] >= rem) )
				){
					u32 retpos = p*8 + (8-obc);
					if( retpos < startBlockLimit )
					{
						reserve_area( man, retpos, numBlockReserve );
						#ifdef AREAMAN_DEBUG
						if( man->printDebugFlag )
						{
							OS_TPrintf("p=%d, obc=%d, retpos=%d\n", p, obc, retpos);
							print_reserveinfo( man, retpos, numBlockReserve, __LINE__ );
						}
						#endif
						return retpos;
					}
					else
					{
						#ifdef AREAMAN_DEBUG
						if( man->printDebugFlag )
						{
							OS_TPrintf("** reserve failed %dblocks (pat D) ** \n", numBlockReserve);
						}
						#endif
						return AREAMAN_POS_NOTFOUND;
					}
				}
				else
				{
					p = pp;
				}
			}
			else	/* if( obc ) */
			{
				p++;
			}
			obc = get_open_back_count( man, p );
		}
	}

	#ifdef AREAMAN_DEBUG
	if( man->printDebugFlag )
	{
		OS_TPrintf("** reserve failed %dblocks (pat E) ** \n", numBlockReserve);
	}
	#endif

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
	if( man->printDebugFlag )
	{
		print_reserveinfo( man, pos, blockNum, __LINE__ );
	}
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
	GF_ASSERT((pos+blockNum) <= man->maxBlock);

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

			GF_ASSERT( (man->area[pos] & (~BackFillBit[p])) == BackFillBit[p] );
			man->area[pos++] &= (~BackFillBit[p]);

			while(bytes--)
			{
				GF_ASSERT(man->area[pos]==0xff);
				man->area[pos++] = 0x00;
			}

			GF_ASSERT( (man->area[pos] & (~FwdFillBit[rem])) == FwdFillBit[rem] );
			man->area[pos] &= (~FwdFillBit[rem]);
		}
	}
}


//------------------------------------------------------------------
/**
 * 
 * 
 */
//------------------------------------------------------------------
static GFL_AREAMAN_POS check_empty_bit( u8 baseBitMap, u32 start_bit, u32 num )
{
	if( num < 8 )
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
		GF_ASSERT( ((~bit)&(*area)) == bit );
		(*area) &= (~bit);
	}
}


static void reserve_area( GFL_AREAMAN* man, int pos, u32 blockNum )
{
	int start_pos, start_bit_count, start_bit, rem, bytes;

	GF_ASSERT((pos+blockNum)<=man->maxBlock);

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

	GF_ASSERT((man->area[pos] & start_bit) == 0);

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
	OS_TPrintf("%d", (b>>7)&1);
	OS_TPrintf("%d", (b>>6)&1);
	OS_TPrintf("%d", (b>>5)&1);
	OS_TPrintf("%d", (b>>4)&1);
	OS_TPrintf("%d", (b>>3)&1);
	OS_TPrintf("%d", (b>>2)&1);
	OS_TPrintf("%d", (b>>1)&1);
	OS_TPrintf("%d", (b>>0)&1);
}

static void print_bit_all( GFL_AREAMAN* man )
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

static void print_reserveinfo( GFL_AREAMAN* man, u32 pos, u32 blockNum, int line )
{
	OS_TPrintf("reserve %d to %d (%dblocks) - line:%d\n", pos, pos+blockNum-1, blockNum, line);
	print_bit_all( man );
}

#endif
