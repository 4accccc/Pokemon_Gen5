//============================================================================================
/**
 * @file	loader.h
 * @brief	簡易画像ローダ
 * @author	taya
 * @date	2006.10.13
 */
//============================================================================================
#ifndef __LOADER_H__
#define __LOADER_H__

//------------------------------------------------------------------
/**
 * 初期化。画面暗転中に１回だけ呼ぶ。
 */
//------------------------------------------------------------------
extern void LDR_Init( void );


//------------------------------------------------------------------
/**
 * 転送
 *
 * @param   cgx			CGXデータ
 * @param   cgx_size	CGXデータサイズ（バイト）
 * @param   scrn		SCRデータ
 * @param   scrn_size	SCRデータサイズ（バイト）
 * @param   pltt		PALデータ
 * @param   pltt_size	PALデータサイズ（バイト）
 * @param   main_sub_flag	TRUE:MAIN-LCD,  FALSE:SUB-LCD
 *
 */
//------------------------------------------------------------------
extern void LDR_Trans( const u8* cgx, u32 cgx_size,
	const u16* scrn, u32 scrn_size,
	const u16* pltt, u32 pltt_size,
	BOOL  main_sub_flag );


#endif
