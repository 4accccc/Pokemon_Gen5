//======================================================================
/**
 * @file	debug_str_conv.h
 * @brief	SJIS‚Æunicode‚Ì‘ŠŒİ•ÏŠ·
 * @author	ariizumi
 * @data	09/12/01
 *
 * ƒ‚ƒWƒ…[ƒ‹–¼FDEB_STR_CONV
 */
//======================================================================
#pragma once
#if 0
//--------------------------------------------------------------
//	char‚Ì•¶š—ñ‚ğSTRCODE(u16)‚É•ÏŠ·
//
//  @param str1 charŒ^•¶š—ñ
//  @param str2 STRCODEŒ^•¶š—ñ
//  @param str1‚Ì•ÏŠ·‚ğs‚¤’·‚³(byte’PˆÊ
//--------------------------------------------------------------
extern void DEB_STR_CONV_SjisToStrcode( const char* str1 , STRCODE *str2 , const u32 len );

//--------------------------------------------------------------
//	STRCODE(u16)‚Ì•¶š—ñ‚ğchar‚É•ÏŠ·
//
//  @param str1 STRCODEŒ^•¶š—ñ
//  @param str2 charŒ^•¶š—ñ
//  @param str1‚Ì•ÏŠ·‚ğs‚¤’·‚³(•¶š”
//--------------------------------------------------------------
extern void DEB_STR_CONV_StrcodeToSjis( const STRCODE *str1 , char *str2 , const u32 len );
extern void DEB_STR_CONV_StrBufferToSjis( const STRBUF *str1 , char *str2 , const u32 len );
#endif

//GFLib‚ÉˆÚA‚³‚ê‚Ü‚µ‚½B

//--------------------------------------------------------------
//	char‚Ì•¶š—ñ‚ğSTRCODE(u16)‚É•ÏŠ·
//
//  @param str1 charŒ^•¶š—ñ
//  @param str2 STRCODEŒ^•¶š—ñ
//  @param str1‚Ì•ÏŠ·‚ğs‚¤’·‚³(byte’PˆÊ
//--------------------------------------------------------------
inline void DEB_STR_CONV_SjisToStrcode( const char* str1 , STRCODE *str2 , const u32 len )
{
  GFL_STR_CONV_SjisToStrcode( str1,str2,len );
}

//--------------------------------------------------------------
//	STRCODE(u16)‚Ì•¶š—ñ‚ğchar‚É•ÏŠ·
//
//  @param str1 STRCODEŒ^•¶š—ñ
//  @param str2 charŒ^•¶š—ñ
//  @param str1‚Ì•ÏŠ·‚ğs‚¤’·‚³(•¶š”
//--------------------------------------------------------------
inline void DEB_STR_CONV_StrcodeToSjis( const STRCODE *str1 , char *str2 , const u32 len )
{
  GFL_STR_CONV_StrcodeToSjis( str1,str2,len );
}

inline void DEB_STR_CONV_StrBufferToSjis( const STRBUF *str1 , char *str2 , const u32 len )
{
  GFL_STR_CONV_StrBufferToSjis( str1,str2,len );
}
