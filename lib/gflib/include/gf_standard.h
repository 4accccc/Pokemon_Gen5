#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================================
/**
 * @file  gf_standard.h
 * @brief Ｃ標準関数定義用ヘッダ
 *          inlineにできないものは gf_standard.cに記述します
 * @author  GAME FREAK inc.
 * @date  2006.11.29
 */
//=============================================================================================

#pragma once

#include "heap.h"

//要素数カウント C++だとテンプレートを使うらしい
#define elementof(ar) (sizeof(ar) / sizeof((ar)[0]))


///< ２次元をあらわす汎用構造体
typedef struct {
  int x;
  int y;
} GFL_POINT;

///< ３次元をあらわす汎用構造体
typedef struct {
  int x;
  int y;
  int z;
} GFL_POINT3D;

///< 始点と範囲をあらわす汎用構造体
typedef struct {
  int left;
  int top;
  int right;
  int bottom;
} GFL_RECT;


#define GFL_STD_RAND_MAX  (0)  ///< MtRand等で渡す最大乱数を引き出す引数

//----------------------------------------------------------------------------
/**
 *  @brief  標準ライブラリの初期化関数
 *  @retval なし
 */
//-----------------------------------------------------------------------------
extern void GFL_STD_Init(HEAPID heapID);

//----------------------------------------------------------------------------
/**
 *  @brief  標準ライブラリの終了関数
 *  @retval なし
 */
//-----------------------------------------------------------------------------
extern void GFL_STD_Exit(void);

//--------------------------------------------------------------------
/**
 * @brief メモリーコピー関数
 * @param   src   コピー元
 * @param   dest  コピー先
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemCopy( const void* src, void* dest, u32 size ) { MI_CpuCopy8(src,dest,size); }

//--------------------------------------------------------------------
/**
 * @brief メモリーコピー関数 16BIT転送版
 * @param   src   コピー元
 * @param   dest  コピー先
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemCopy16( const void* src, void* dest, u32 size ) { MI_CpuCopy16(src,dest,size); }

//--------------------------------------------------------------------
/**
 * @brief メモリーコピー関数 32BIT転送版
 * @param   src   コピー元
 * @param   dest  コピー先
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemCopy32( const void* src, void* dest, u32 size ) { MI_CpuCopy32(src,dest,size); }

//--------------------------------------------------------------------
/**
 * @brief メモリを指定のデータで埋めます
 * @param   dest   転送先アドレス
 * @param   data  転送データ
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemFill( void* dest, u8 data, u32 size ) { MI_CpuFill8(dest,data,size); }

//--------------------------------------------------------------------
/**
 * @brief メモリを指定のデータで埋めます(16BIT版)
 * @param   dest   転送先アドレス
 * @param   data  転送データ
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemFill16( void* dest, u16 data, u32 size ) { MI_CpuFill16(dest,data,size); }

//--------------------------------------------------------------------
/**
 * @brief メモリを指定のデータで埋めます(32BIT版)
 * @param   dest   転送先アドレス
 * @param   data  転送データ
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemFill32( void* dest, u32 data, u32 size ) { MI_CpuFill32(dest,data,size); }

//--------------------------------------------------------------------
/**
 * @brief メモリを０でクリアします
 * @param   dest   転送先アドレス
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemClear( void* dest, u32 size ) { MI_CpuClear8(dest,size); }

//--------------------------------------------------------------------
/**
 * @brief メモリを０でクリアします(16BIT版)
 * @param   dest   転送先アドレス
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemClear16( void* dest, u32 size ) { MI_CpuClear16(dest,size); }

//--------------------------------------------------------------------
/**
 * @brief メモリを０でクリアします(32BIT版)
 * @param   dest   転送先アドレス
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemClear32( void* dest, u32 size ) { MI_CpuClear32(dest,size); }

//----------------------------------------------------------------------------
/**
 *  @brief  メモリ領域を比較する
 *  @param  s1      比較メモリー1
 *  @param  s2      比較メモリー2
 *  @param  size  比較サイズ
 *  @retval ０      同じ
 *  @retval ！０    異なる
 */
//-----------------------------------------------------------------------------
extern int GFL_STD_MemComp( const void *s1, const void *s2, const u32 size );

//----------------------------------------------------------------------------
/**
 *  @brief  絶対値を得る 注意  式を書くと二回呼ばれることがあります
 *  @param  a       絶対値がほしい値
 *  @retval         絶対値
 */
//-----------------------------------------------------------------------------
inline int GFL_STD_Abs(int a){ return MATH_ABS(a); }

//----------------------------------------------------------------------------
/**
 *  @brief  ０が現れるまでのバイト数を計る
            文字の扱いにはSTRBUFを使うこと  これは通信のSSIDなど ASCII文字列に使用する用途にもちいる事
 *  @param  s     データ列
 *  @return サイズ＝長さ
 */
//-----------------------------------------------------------------------------
extern int GFL_STD_StrLen( const void *s );
//----------------------------------------------------------------------------
/**
 *  @brief  文字列連結
            文字の扱いにはSTRBUFを使うこと  これは通信のSSIDなど ASCII文字列に使用する用途にもちいる事
 *  @param  src   連結される方の文字列
 *  @param  dist  連結する方の文字列
 *  @param  s     サイズ
 *  @return none
 */
//-----------------------------------------------------------------------------
extern void GFL_STD_StrCat( void *src, const void *dist, int size );

//----------------------------------------------------------------------------
/**
 *  @brief  文字列比較  (C標準 strncmp と等価です）
 *          文字の扱いにはSTRBUFを使うこと  これは通信のSSIDなど ASCII文字列に使用する用途にもちいる事
 *  @param  str1  比較する文字列１
 *  @param  str2  比較する文字列２
 *  @return 一致していれば0、その他の場合には0以外が返る
 */
//-----------------------------------------------------------------------------
extern int GFL_STD_StrCmp( const char* str1, const char* str2 );

//----------------------------------------------------------------------------
/**
 *  @brief  文字列比較  (C標準 strncmp と等価です）
 *          文字の扱いにはSTRBUFを使うこと  これは通信のSSIDなど ASCII文字列に使用する用途にもちいる事
 *  @param  str1  比較する文字列１
 *  @param  str2  比較する文字列２
 *  @param  len   比較する文字列長
 *  @return len まで一致していれば0、その他の場合には0以外が返る
 */
//-----------------------------------------------------------------------------
extern int GFL_STD_StrnCmp( const char *str1, const char *str2, int len );



//----------------------------------------------------------------------------
/**
 *  @brief   メルセンヌツイスター初期化関数 RTCとvsyncnumで初期化してください
 *  @param   s  シード値(この値から初期化値が線形合同法で生成されます)
 *  @return  none
 */
//----------------------------------------------------------------------------
extern void GFL_STD_MtRandInit(u32 s);

//----------------------------------------------------------------------------
/**
 *  @brief   メルセンヌツイスターで符号なし３２ビット長の乱数を取得
 *  @param   range  取得数値の範囲を指定 0 ～ range-1 の範囲の値が取得できます。
 *                  GFL_STD_RAND_MAX=0を指定した場合にはすべての範囲の32bit値となります。
 *                  % や / を使用せずにここのrange値を変更してください
 *                  GFL_STD_RAND_MAXを使わない場合は下の関数
 *  @return  生成された乱数
 */
//----------------------------------------------------------------------------
extern u32 __GFL_STD_MtRand(void); ///< 使用禁止  下のinline関数を使ってください
static inline u32 GFL_STD_MtRand(u32 range)
{
  u64 x = (u64)__GFL_STD_MtRand();

  // 引数maxが定数ならばコンパイラにより最適化される。
  if (range == GFL_STD_RAND_MAX) {
    return (u32)x;
  }
  else {
    return (u32)((x * range) >> 32);
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief   メルセンヌツイスターで符号なし３２ビット長の乱数を取得
 *  @param   range    取得数値の範囲を指定 0 ～ range-1 の範囲の値が取得できます。
 *                    % や / を使用せずにここのrange値を変更してください
 *                  全部とる場合上の関数
 *  @return  生成された乱数
 */
//----------------------------------------------------------------------------
static inline u32 GFL_STD_MtRand0(u32 range)
{
  u64 x = (u64)__GFL_STD_MtRand();
  return (u32)((x * range) >> 32);
}

/**
 *  @brief   線形合同法乱数の構造体
 */
typedef struct
{
  u64     x;                         ///< 乱数値
  u64     mul;                       ///< 乗数
  u64     add;                       ///< 加算する数
} GFL_STD_RandContext;

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
 *  @param  range   取得数値の範囲を指定 0 ～ range-1 の範囲の値が取得できます。
 *                  GFL_STD_RAND_MAX = 0 を指定した場合にはすべての範囲の32bit値となります。
 *                  % や / を使用せずにここのrange値を変更してください
 *                  GFL_STD_RAND_MAXを使わない場合は下の関数
 *  @return 32bitのランダム値
 */
//----------------------------------------------------------------------------
static inline u32 GFL_STD_Rand(GFL_STD_RandContext *context, u32 range)
{
  context->x = context->mul * context->x + context->add;

  // 引数maxが定数ならばコンパイラにより最適化される。
  if (range == GFL_STD_RAND_MAX)
    {
      return (u32)(context->x >> 32);
    }
  else
    {
      return (u32)(((context->x >> 32) * range) >> 32);
    }
}

//----------------------------------------------------------------------------
/**
 *  @brief  線形合同法による32bit乱数取得関数
 *  @param  context 乱数構造体のポインタ
 *  @param  range   取得数値の範囲を指定 0 ～ range-1 の範囲の値が取得できます。
 *                  % や / を使用せずにここのrange値を変更してください
 *                  全部とる場合上の関数
 *  @return 32bitのランダム値
 */
//----------------------------------------------------------------------------
static inline u32 GFL_STD_Rand0(GFL_STD_RandContext *context, u32 range)
{
  context->x = context->mul * context->x + context->add;
  return (u32)(((context->x >> 32) * range) >> 32);
}

//----------------------------------------------------------------------------
/**
 *  @brief  CRCを計算する
 *  @param  pData     計算する領域
 *  @param  dataLength  データの長さ
 *  @return CRC値
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

//----------------------------------------------------------------------------------------------
//  暗号化系移植
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
/**
 * チェックサムの算出ルーチン
 *
 * @param adrs  開始アドレス
 * @param size  データサイズ
 *
 * @retval  "算出した値"
 */
//----------------------------------------------------------------------------------------------
extern u32 GFL_STD_CODED_CheckSum(const void * adrs, u32 size);

extern void GFL_STD_CODED_Coded(void *data,u32 size,u32 code);
extern void GFL_STD_CODED_Decoded(void *data,u32 size,u32 code);


#ifdef __cplusplus
} /* extern "C" */
#endif
