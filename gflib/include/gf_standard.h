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
inline void GFL_STD_MemCopy16( const void* src, void* dest, u32 size ) { MI_CpuCopy8(src,dest,size); }

//--------------------------------------------------------------------
/**
 * @brief	メモリーコピー関数 32BIT転送版
 * @param   src   コピー元
 * @param   dest  コピー先
 * @param   size  サイズ
 * @return  none
 */
//--------------------------------------------------------------------
inline void GFL_STD_MemCopy32( const void* src, void* dest, u32 size ) { MI_CpuCopy8(src,dest,size); }

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




#endif	/*	__GF_STANDARD_H__ */
