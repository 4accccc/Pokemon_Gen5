//============================================================================================
/**
 * @file	gf_standard.c
 * @brief	Ｃ標準関数 + 乱数関数
 * @author	GAME FREAK inc.
 * @date	2006.11.29
 */
//============================================================================================
#include "gflib.h"
#include "gf_standard.h"



/*-----------------------------------------------------------------------------
 *					定数宣言
 ----------------------------------------------------------------------------*/
#define N 624						///< 乱数配列の要素数
#define M 397						///< 自然数
#define MATRIX_A 0x9908b0dfUL		///< 求めた値の１ビット目が１の時にXORする値 
#define UPPER_MASK 0x80000000UL		///< ビットのw-rビットを取得するマクロ	今はw = 32 r = 31です。
#define LOWER_MASK 0x7fffffffUL		///< ビットのrビットを取得するマクロ		こうすると３２ビット符号なしの乱数を取得できます。

/*-----------------------------------------------------------------------------
 *					ワーク
 ----------------------------------------------------------------------------*/

struct _GF_STD_WORK{
    u32 mt[N];						///< MTの式の結果の入る配列
    int mti;						///< 配列要素番号(N+1の時は初期化されていない)
    u32 mag01[2];	///< 式の結果１ビット目を添え字番号として入れてXorをするテーブル
    MATHCRC16Table crcTable;   ///< CRCテーブル
};

typedef struct _GF_STD_WORK GF_STD_WORK;


static GF_STD_WORK* _pSTDSys = NULL; // スタンダードライブラリのメモリーを管理するstatic変数

//----------------------------------------------------------------------------
/**
 *	@brief	標準ライブラリの初期化関数
 *	@retval	なし
 */
//-----------------------------------------------------------------------------

void GFL_STD_Init(HEAPID heapID)
{
    GF_ASSERT(_pSTDSys==NULL);
    _pSTDSys = GFL_HEAP_AllocMemory(heapID, sizeof(GF_STD_WORK));
    _pSTDSys->mti = N + 1;			///< 配列要素番号(N+1の時は初期化されていない)
    _pSTDSys->mag01[0] = 0x0UL;
    _pSTDSys->mag01[1] = MATRIX_A;	///< 式の結果１ビット目を添え字番号として入れてXorをするテーブル
    MATH_CRC16CCITTInitTable(&_pSTDSys->crcTable);
}

//----------------------------------------------------------------------------
/**
 *	@brief	標準ライブラリの終了関数
 *	@retval	なし
 */
//-----------------------------------------------------------------------------

void GFL_STD_Exit(void)
{
    GFL_HEAP_FreeMemory(_pSTDSys);
    _pSTDSys=NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メモリ領域を比較する
 *	@param	s1	比較メモリー1
 *	@param	s2	比較メモリー2
 *	@param	size	比較サイズ
 *	@retval	０   同じ
 *	@retval	！０ 異なる
 */
//-----------------------------------------------------------------------------
int GFL_STD_MemComp(const void *s1, const void *s2, const u32 size)
{
    const unsigned char  *p1 = (const unsigned char *)s1;
    const unsigned char  *p2 = (const unsigned char *)s2;
    u32 n = size;

    while (n-- > 0) {
        if (*p1 != *p2)
            return (*p1 - *p2);
        p1++;
        p2++;
    }
    return (0);
}

//----------------------------------------------------------------------------
/**
 *	@brief	０が現れるまでのバイト数を計る
            文字の扱いにはSTRBUFを使うこと  これは通信のSSIDなど ASCII文字列に使用する用途にもちいる事
 *	@param	s	    データ列
 *	@return	サイズ＝長さ
 */
//-----------------------------------------------------------------------------
int GFL_STD_StrLen( const void *s )
{
    const unsigned char  *p1 = (const unsigned char *)s;
    int n = 0;

    while( 1 ){
        if(*p1 == 0){
            return n;
        }
        p1++;
        n++;
    }
    return 0;
}


/*-----------------------------------------------------------------------------
 *
 *	H16/11/10		Tomoya Takahashi
 *	
 *				MT方での乱数発生関数
 *
 ----------------------------------------------------------------------------*/


/**-----------------------------------------------------------------------------
 *
 *	@brief   メルセンヌツイスター初期化関数 RTCとvsyncnumで初期化してください
 *	@param   s  シード値(この値から初期化値が線形合同法で生成されます)
 *  @return  none
 * 
 ----------------------------------------------------------------------------*/
void GFL_STD_MtRandInit(u32 s)
{
//    mt[0]= s & 0xffffffff;			// シード値を要素０に代入
    _pSTDSys->mt[0]= s ;			// シード値を要素０に代入

	//
	// 線形合同法で乱数を生成
	//
    for (_pSTDSys->mti=1; _pSTDSys->mti<N; _pSTDSys->mti++) {
        _pSTDSys->mt[_pSTDSys->mti] = 
	    (1812433253UL * (_pSTDSys->mt[_pSTDSys->mti-1] ^ (_pSTDSys->mt[_pSTDSys->mti-1] >> 30)) + _pSTDSys->mti); 
//        mt[mti] &= 0xffffffff;
    }
}

/**-----------------------------------------------------------------------------
 *  @brief   メルセンヌツイスターで符号なし３２ビット長の乱数を取得
 *	@return  生成された乱数
------------------------------------------------------------------------------*/
u32 __GFL_STD_MtRand(void)
{
    u32 y;
    
	//
	// 乱数の生成
	//
	// 式から乱数を生成するかのチェック
    if (_pSTDSys->mti >= N) {		// 無限にループさせる
		int kk;			// 式の計算ループ用

		// mtiがN+1のときは初期化
        if (_pSTDSys->mti == N+1)   
		{
			GFL_STD_MtRandInit(5489UL);		// 適当な値で初期化
		}

		//
		// mtiがNになったので
		// 式から乱数を生成（僕もわかりません）
		// 
        for (kk=0;kk<N-M;kk++) {
            y = (_pSTDSys->mt[kk]&UPPER_MASK)|(_pSTDSys->mt[kk+1]&LOWER_MASK);
            _pSTDSys->mt[kk] = _pSTDSys->mt[kk+M] ^ (y >> 1) ^ _pSTDSys->mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (_pSTDSys->mt[kk]&UPPER_MASK)|(_pSTDSys->mt[kk+1]&LOWER_MASK);
            _pSTDSys->mt[kk] = _pSTDSys->mt[kk+(M-N)] ^ (y >> 1) ^ _pSTDSys->mag01[y & 0x1UL];
        }
        y = (_pSTDSys->mt[N-1]&UPPER_MASK)|(_pSTDSys->mt[0]&LOWER_MASK);
        _pSTDSys->mt[N-1] = _pSTDSys->mt[M-1] ^ (y >> 1) ^ _pSTDSys->mag01[y & 0x1UL];

        _pSTDSys->mti = 0;		// 先頭に戻す
    }
  
	//
	// 今の乱数を取得
	// 
    y = _pSTDSys->mt[_pSTDSys->mti++];

	// この下の処理でもっと値を分布させています。
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

//----------------------------------------------------------------------------
/**
 *	@brief	CRCを計算する
 *	@param	pData	    計算する領域
 *	@param	dataLength  データの長さ
 *	@return	CRC値
 */
//-----------------------------------------------------------------------------

u16 GFL_STD_CrcCalc( const void* pData, u32 dataLength )
{
    if(dataLength < sizeof(u16)){
        u8 dummyData[2];
        u8* pD = (u8*)pData;
        dummyData[0]=*pD;
        dummyData[1]=dummyData[0];
        return MATH_CalcCRC16CCITT(&_pSTDSys->crcTable, &dummyData, sizeof(u16));
    }
    return MATH_CalcCRC16CCITT(&_pSTDSys->crcTable, pData, dataLength);
}


//----------------------------------------------------------------------------
/**
 *  @brief  線形合同法による32bit乱数コンテキストを初期化します。
            初期化シードはRTCにそった乱数です
 *  @param  context 乱数構造体のポインタ
 *  @return none
 */
//----------------------------------------------------------------------------

void GFL_STD_RandGeneralInit(GFL_STD_RandContext *context)
{
    RTCDate date;
    RTCTime time;
    u32 seed;
    u64 seed64;
    RTC_GetDateTime(&date, &time);
    seed = date.year + date.month * 0x100 * date.day * 0x10000
        + time.hour * 0x10000 + (time.minute + time.second) * 0x1000000;

    GFL_STD_MtRandInit(seed);
    seed64 = GFL_STD_MtRand( 0 );
    seed64 = (seed64 << 32) + GFL_STD_MtRand( 0 );
    GFL_STD_RandInit( context, seed64 );
}
