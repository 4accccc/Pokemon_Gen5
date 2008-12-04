#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================================
/**
 * @file	gf_standard.h
 * @brief	Ｃ標準関数定義用ヘッダ
 *          inlineにできないものは gf_standard.cに記述します
 * @author	GAME FREAK inc.
 * @date	2006.11.29
 */
//=============================================================================================

#ifndef	__GF_STANDARD_H__
#define	__GF_STANDARD_H__

#include "heap.h"

//----------------------------------------------------------------------------
/**
 *	@brief	標準ライブラリの初期化関数
 *	@retval	なし
 */
//-----------------------------------------------------------------------------
extern void GFL_STD_Init(HEAPID heapID);

//----------------------------------------------------------------------------
/**
 *	@brief	標準ライブラリの終了関数
 *	@retval	なし
 */
//-----------------------------------------------------------------------------
extern void GFL_STD_Exit(void);

//--------------------------------------------------------------------
/**
 * @brief	メモリーコピー関数
 * @param   src   コピー元
 * @param   dest  コピー先
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemCopy( const void* src, void* dest, u32 size ) { MI_CpuCopy8(src,dest,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリーコピー関数 16BIT転送版
 * @param   src   コピー元
 * @param   dest  コピー先
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemCopy16( const void* src, void* dest, u32 size ) { MI_CpuCopy16(src,dest,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリーコピー関数 32BIT転送版
 * @param   src   コピー元
 * @param   dest  コピー先
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemCopy32( const void* src, void* dest, u32 size ) { MI_CpuCopy32(src,dest,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリを指定のデータで埋めます
 * @param   dest   転送先アドレス
 * @param   data  転送データ
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemFill( void* dest, u8 data, u32 size ) { MI_CpuFill8(dest,data,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリを指定のデータで埋めます(16BIT版)
 * @param   dest   転送先アドレス
 * @param   data  転送データ
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemFill16( void* dest, u16 data, u32 size ) { MI_CpuFill16(dest,data,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリを指定のデータで埋めます(32BIT版)
 * @param   dest   転送先アドレス
 * @param   data  転送データ
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemFill32( void* dest, u32 data, u32 size ) { MI_CpuFill32(dest,data,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリを０でクリアします
 * @param   dest   転送先アドレス
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemClear( void* dest, u32 size ) { MI_CpuClear8(dest,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリを０でクリアします(16BIT版)
 * @param   dest   転送先アドレス
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemClear16( void* dest, u32 size ) { MI_CpuClear16(dest,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリを０でクリアします(32BIT版)
 * @param   dest   転送先アドレス
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemClear32( void* dest, u32 size ) { MI_CpuClear32(dest,size); }

//----------------------------------------------------------------------------
/**
 *	@brief	メモリ領域を比較する
 *	@param	s1	    比較メモリー1
 *	@param	s2	    比較メモリー2
 *	@param	size	比較サイズ
 *	@retval	０      同じ
 *	@retval	！０    異なる
 */
//-----------------------------------------------------------------------------
extern int GFL_STD_MemComp( const void *s1, const void *s2, const u32 size );

//----------------------------------------------------------------------------
/**
 *	@brief	０が現れるまでのバイト数を計る
            文字の扱いにはSTRBUFを使うこと  これは通信のSSIDなど ASCII文字列に使用する用途にもちいる事
 *	@param	s	    データ列
 *	@return	サイズ＝長さ
 */
//-----------------------------------------------------------------------------
extern int GFL_STD_StrLen( const void *s );

//----------------------------------------------------------------------------
/**
 *	@brief   メルセンヌツイスター初期化関数 RTCとvsyncnumで初期化してください
 *	@param   s  シード値(この値から初期化値が線形合同法で生成されます)
 *  @return  none
 */
//----------------------------------------------------------------------------
extern void GFL_STD_MtRandInit(u32 s);

//----------------------------------------------------------------------------
/**
 *  @brief   メルセンヌツイスターで符号なし３２ビット長の乱数を取得
 *  @param   max    取得数値の範囲を指定 0 ～ max-1 の範囲の値が取得できます。
 *                  0を指定した場合にはすべての範囲の32bit値となります。
 *                  % や / を使用せずにここのmax値を変更してください
 *	@return  生成された乱数
 */
//----------------------------------------------------------------------------
extern u32 __GFL_STD_MtRand(void); ///< 使用禁止  下のinline関数を使ってください
static inline u32 GFL_STD_MtRand(u32 max)
{
    u64 x = (u64)__GFL_STD_MtRand();

    // 引数maxが定数ならばコンパイラにより最適化される。
    if (max == 0) {
        return (u32)x;
    }
    else {
        return (u32)((x * max) >> 32);
    }
}

#define GFL_STD_MTRAND_MAX	(0xffffffff)

/**
 *  @brief   線形合同法乱数の構造体
 */
typedef struct
{
    u64     x;                         ///< 乱数値
    u64     mul;                       ///< 乗数
    u64     add;                       ///< 加算する数
}
GFL_STD_RandContext;

//----------------------------------------------------------------------------
/**
 *  @brief  線形合同法による32bit乱数コンテキストを初期化します。
 *  @param  context 乱数構造体のポインタ
 *  @param  seed    初期値として設定する乱数の種  メルセンヌツイスターを2回呼んで (a1<<32)+a2で生成してください
 *  @return none
 */
//----------------------------------------------------------------------------
static inline void GFL_STD_RandInit(GFL_STD_RandContext *context, u64 seed)
{
    context->x = seed;
    context->mul = (1566083941LL << 32) + 1812433253LL;
    context->add = 2531011;
}

//----------------------------------------------------------------------------
/**
 *  @brief  線形合同法による32bit乱数取得関数
 *  @param  context 乱数構造体のポインタ
 *  @param  max     取得数値の範囲を指定 0 ～ max-1 の範囲の値が取得できます。
 *                  GFL_STD_MTRAND_MAXを指定した場合にはすべての範囲の32bit値となります。
 *                  % や / を使用せずにここのmax値を変更してください
 *  @return 32bitのランダム値
 */
//----------------------------------------------------------------------------
static inline u32 GFL_STD_Rand(GFL_STD_RandContext *context, u32 max)
{
    context->x = context->mul * context->x + context->add;

    // 引数maxが定数ならばコンパイラにより最適化される。
    if (max == GFL_STD_MTRAND_MAX)
    {
        return (u32)(context->x >> 32);
    }
    else
    {
        return (u32)(((context->x >> 32) * max) >> 32);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	CRCを計算する
 *	@param	pData	    計算する領域
 *	@param	dataLength  データの長さ
 *	@return	CRC値
 */
//-----------------------------------------------------------------------------

extern u16 GFL_STD_CrcCalc( const void* pData, u32 dataLength );

//----------------------------------------------------------------------------
/**
 *  @brief  線形合同法による32bit乱数コンテキストを初期化します。
            初期化シードはRTCにそった乱数です
 *  @param  context 乱数構造体のポインタ
 *  @return none
 */
//----------------------------------------------------------------------------
extern void GFL_STD_RandGeneralInit(GFL_STD_RandContext *context);

#endif	/*	__GF_STANDARD_H__ */
#ifdef __cplusplus
} /* extern "C" */
#endif
