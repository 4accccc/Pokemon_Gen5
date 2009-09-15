/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_Oam_Bmp1D
  File:     data.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*
 ******************************************************************************
 *
 *  Project    :
 *               nitroSDKサンプルプログラム
 *  Title      :
 *               １次元マッピング・ビットマップＯＢＪの表示
 *  Explanation:
 *　　　　　　　 １次元マッピングのビットマップデータを持つＯＢＪの表示を行う。
 *
 ******************************************************************************
 */
#ifndef OBJ_DATA_H
#define OBJ_DATA_H

extern const unsigned short bitmapOBJ_directcolor_Texel[128 * 128 * 2];

#define IMAGE_DATA	(bitmapOBJ_directcolor_Texel)
#define IMAGE_DATA_SIZE	(sizeof(bitmapOBJ_directcolor_Texel))

#endif
