//======================================================================
/**
 * @file	jpeg_encoder.c
 * @brief	RGB555形式の画像をJpegにエンコード・デコードする
 * @author	ariizumi
 * @data	09/05/22
 *
 * モジュール名：JPG_ENC
 */
//======================================================================
//スレッドを使ってJPGのエンコード・デコードを行います。


#pragma once

//--------------------------------------------------------------
//	
//--------------------------------------------------------------

typedef struct _JPG_ENC_WORK JPG_ENC_WORK;

typedef void (*JPG_ENC_EncodeCallBack)( void *userWork , u8* jpgData , u32 jpgSize);

const u32 JPG_ENC_GetEncodeWorkSize( const u16 sizeX , const u16 sizeY , u32 sampling , u32 option );
void JPG_ENC_StartEncode( JPG_ENC_WORK* work , void *src , u8 *dst , u32 dstSize , void *workBuff , 
                          const u16 sizeX , const u16 sizeY , u32 quality, 
                          u32 sampling , u32 option , JPG_ENC_EncodeCallBack callBack , void* userWork );
const BOOL  JPG_ENC_IsEncode( const JPG_ENC_WORK* work );

JPG_ENC_WORK* JPG_ENC_InitSystem( HEAPID heapId );
void JPG_ENC_ExitSystem( JPG_ENC_WORK* work );
