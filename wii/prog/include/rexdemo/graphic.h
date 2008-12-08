/*---------------------------------------------------------------------------*
  Project:  NET demo
  File:	 sample.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: graphic.h,v $
  Revision 1.3  2006/09/05 10:55:54  yosizaki
  added REXDEMOUsage.

  Revision 1.2  2006/08/31 14:04:59  yosizaki
  描画設定関数を追加.

  Revision 1.1  2006/08/29 07:18:24  adachi_hiroaki
  共通のグラフィック処理を追加


  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef __DEMOS_SHARED_H__
#define __DEMOS_SHARED_H__

#include <revolution/types.h>
#include <revolution/gx.h>

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================*/

#ifdef WPADEMU
#define KPAD_BUTTON_PLUS	KPAD_BUTTON_START
#define KPAD_BUTTON_MINUS   KPAD_BUTTON_SELECT
#define KPAD_BUTTON_1	   KPAD_BUTTON_SMALL_A
#define KPAD_BUTTON_2	   KPAD_BUTTON_SMALL_B
#define KPAD_BUTTON_Z	   KPAD_BUTTON_Z1
#define KPAD_BUTTON_C	   KPAD_BUTTON_Z2
#endif

#define REXDEMO_COLOR_BLACK	  ( (const GXColor){ 0x00, 0x00, 0x00, 0xff } )
#define REXDEMO_COLOR_WHITE	  ( (const GXColor){ 0xff, 0xff, 0xff, 0xff } )
#define REXDEMO_COLOR_GRAY	   ( (const GXColor){ 0x7f, 0x7f, 0x7f, 0xff } )
#define REXDEMO_COLOR_RED		( (const GXColor){ 0xff, 0x00, 0x00, 0xff } )
#define REXDEMO_COLOR_GREEN	  ( (const GXColor){ 0x00, 0xff, 0x00, 0xff } )
#define REXDEMO_COLOR_BLUE	   ( (const GXColor){ 0x00, 0x00, 0xff, 0xff } )
#define REXDEMO_COLOR_YELLOW	 ( (const GXColor){ 0xff, 0xff, 0x00, 0xff } )
#define REXDEMO_COLOR_SKY		( (const GXColor){ 0x00, 0xff, 0xff, 0xff } )
#define REXDEMO_COLOR_PINK	   ( (const GXColor){ 0xff, 0x00, 0xff, 0xff } )
#define REXDEMO_COLOR_ORANGE	 ( (const GXColor){ 0xff, 0x7f, 0x00, 0xff } )
#define REXDEMO_COLOR_PURPLE	 ( (const GXColor){ 0x7f, 0x00, 0xff, 0xff } )

#define REXDEMOReport( ... )	 REXDEMOReportEx( REXDEMO_COLOR_BLACK, __VA_ARGS__ )
#define REXDEMOWarning( ... )	REXDEMOReportEx( REXDEMO_COLOR_ORANGE, __VA_ARGS__ )
#define REXDEMOError( ... )	  REXDEMOReportEx( REXDEMO_COLOR_RED, __VA_ARGS__ )
#define REXDEMOUsage( ... )	  REXDEMOReportEx( REXDEMO_COLOR_BLUE, __VA_ARGS__ )

/*---------------------------------------------------------------------------*/
void	REXDEMOInitScreen( BOOL autoRender );
void	REXDEMOWaitRetrace( void );
void	REXDEMOReportEx( GXColor col, const char* msg, ... );

void	REXDEMOSetTextColor( const GXColor col );
void	REXDEMOSetGroundColor( const GXColor col );
void	REXDEMOSetFontSize( int width, int height );
void	REXDEMOBeginRender( void );
void	REXDEMOPrintf( int x, int y, int z, const char *fmt, ... );


/*===========================================================================*/
#ifdef __cplusplus
}
#endif
#endif  /* __DEMOS_SHARED_H__ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
