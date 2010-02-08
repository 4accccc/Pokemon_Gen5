//============================================================================================
/**
 * @file	str_tool.h
 * @brief	文字列操作関数群
 * @author	taya
 * @date	2008.11.17
 *
 * 生文字配列を扱う関数群／使用は原則、許可制とします。
 */
//============================================================================================
#ifndef __STR_TOOL_H__
#define __STR_TOOL_H__

//------------------------------------------------------------------
/**
 * 数値を文字列化する時の整形タイプ
 */
//------------------------------------------------------------------
typedef enum {

	STR_NUM_DISP_LEFT,		///< 左詰め
	STR_NUM_DISP_SPACE,		///< 右詰め（スペース埋め）
	STR_NUM_DISP_ZERO,		///< 右詰め（ゼロ埋め）

}StrNumberDispType;



//------------------------------------------------------------------
/**
 * 数値を文字列化する時の文字コードタイプ
 */
//------------------------------------------------------------------
typedef enum {

	STR_NUM_CODE_ZENKAKU,		///< 全角
	STR_NUM_CODE_HANKAKU,		///< 半角

	#if PM_LANG == LANG_JAPAN
	STR_NUM_CODE_DEFAULT = STR_NUM_CODE_ZENKAKU,
	#else
	STR_NUM_CODE_DEFAULT = STR_NUM_CODE_HANKAKU,
	#endif

}StrNumberCodeType;

extern void STRTOOL_SetNumber( STRBUF* dst, int number, u32 keta, StrNumberDispType dispType, StrNumberCodeType codeType );
extern void STRTOOL_SetUnsignedNumber( STRBUF* dst, u32 number, u32 keta, StrNumberDispType dispType, StrNumberCodeType codeType );
extern void STRTOOL_SetHexNumber( STRBUF* dst, u32 number, u32 keta, StrNumberDispType dispType, StrNumberCodeType codeType );
extern void STRTOOL_Copy( const STRCODE* src, STRCODE* dst, u32 dstElems );
extern BOOL STRTOOL_Comp( const STRCODE* src, const STRCODE* dst );


#endif
