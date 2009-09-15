/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SSP - demos - jpegDecoder
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-02-23#$
  $Rev: 10061 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

static u8* OutputBuffer;

static u8 WriteBuffer[923648] ATTRIBUTE_ALIGN(4);
static u32 WriteBufferSize;

#include <twl/ssp/ARM9/jpegdec.h>
#include <twl/ssp/ARM9/exifdec.h>

#include "DEMO.h"

/*---------------------------------------------------------------------------*
  Name:         MyAlloc

  Description:  OS_Alloc のラッパー関数
 *---------------------------------------------------------------------------*/
static void* MyAlloc(u32 size)
{
    void* ptr;
    
    ptr = OS_Alloc(size);
    
    return ptr;
}

/*---------------------------------------------------------------------------*
  Name:         MyFree

  Description:  OS_Free のラッパー関数
 *---------------------------------------------------------------------------*/
static void MyFree(void* ptr)
{
    OS_Free(ptr);
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void *tempLo;
    OSHeapHandle hh;

    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlankIntr

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain / NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    /* OSの初期化 */
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);

    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOStartDisplay();

    // SD初期化
    FS_Init( FS_DMA_NOT_USE );

    InitializeAllocateSystem();

    OS_TPrintf("\n");
    OS_TPrintf("===================================\n");
    OS_TPrintf("    SSP JpegDecoder Test\n");
    OS_TPrintf("===================================\n");

    {
        s16 decode_file_width, decode_file_height;
        
        u8 *data;
        u32 length;
        FSFile file[1];
        
        
        FS_InitFile(file);
        if( !FS_OpenFileEx(file, "rom:/decode.jpg", FS_FILEMODE_R) )
        {
            OS_Panic("failed FS_OpenFileEx");
        }
        length = FS_GetFileLength(file);
        data = MyAlloc(length);
        if(data == 0)
        {
            OS_Panic("Cannot allocate memory ... data!");
        }
        if( FS_ReadFile(file, data, (s32)length) == -1)
        {
            OS_Panic("failed FS_ReadFile");
        }
        
        {
            OSTick jpegDecode_begin,jpegDecode_end;
            BOOL result;
            
            OS_TPrintf("Start Decode ... length = %d\n", length);
            
            jpegDecode_begin = OS_GetTick();

            decode_file_width = 256;
            decode_file_height = 192;
            OutputBuffer = MyAlloc(decode_file_width * decode_file_height * sizeof(u16));
            
            result = SSP_StartJpegDecoder(data, length, OutputBuffer, &decode_file_width, &decode_file_height, SSP_JPEG_RGB555);
            
            jpegDecode_end = OS_GetTick();
            
            if(result == 0)
                OS_Panic("Failed Cpu Decode\n");
            
            OS_TPrintf("[TIMER] MAIN SSP_StartJpegDecoder time[usec] = %d\n", OS_TicksToMicroSeconds(jpegDecode_end - jpegDecode_begin));
            OS_TPrintf("[DEBUG] MAIN decode_file_width = %d : decode_file_height = %d\n", decode_file_width, decode_file_height);
            
            {
                int i,j;
                float i_step,j_step;
                int i_count,j_count;
                s16 t_width = decode_file_width;
                
                j_step = (float)decode_file_width / 256;
                i_step = (float)decode_file_height / 192;
                
                i_count = j_count = 0;
                for(i = 0; i < 192; i++)
                {
                    for(j = 0; j < 256; j++)
                    {
                        *((u16*)((int)G2_GetBG3ScrPtr() + i*256*2 + j*2)) = *((u16*)((u32)OutputBuffer + (u32)(i_count * decode_file_width*2) + (u32)(j_count*2)));
                        j_count = (int)((double)j*j_step);
                    }
                    i_count=(int)((double)i*i_step);
                }
            }
            
            // 日時を表示してみる
            {
                u8 dateTime[20];
                
                (void)SSP_GetJpegDecoderDateTime(dateTime);
                OS_TPrintf("DateTimeOriginal = %s\n", dateTime);
            }
            {
                char software[30];
                u32 initialCode;
                
                (void)SSP_GetJpegDecoderSoftware(software);
                // EXIFのsoftwareタグにはイニシャルコードがリトルエンディアンで入っている
                MI_CpuCopy8(software, &initialCode, 4);
                OS_TPrintf("InitialCode = %c%c%c%c\n", initialCode>>24, initialCode>>16, initialCode>>8, initialCode);
            }
            // メーカーノートのアドレス、サイズを取得
            {
                u8 *makerNoteAddr = SSP_GetJpegDecoderMakerNoteAddrEx(SSP_MAKERNOTE_PHOTO);
                u16 makerNoteSize = SSP_GetJpegDecoderMakerNoteSizeEx(SSP_MAKERNOTE_PHOTO);
                u8 *makerNoteUserAddr = SSP_GetJpegDecoderMakerNoteAddrEx(SSP_MAKERNOTE_USER);
                u16 makerNoteUserSize = SSP_GetJpegDecoderMakerNoteSizeEx(SSP_MAKERNOTE_USER);
                
                OS_TPrintf("MakerNoteAddr = 0x%X\n", makerNoteAddr);
                OS_TPrintf("MakerNoteSize = 0x%X\n", makerNoteSize);
                OS_TPrintf("MakerNoteUserAddr = 0x%X\n", makerNoteUserAddr);
                OS_TPrintf("MakerNoteUserSize = 0x%X\n", makerNoteUserSize);
            }
            MyFree(OutputBuffer);
            
            jpegDecode_begin = OS_GetTick();
            
            decode_file_width = SSP_JPEG_THUMBNAIL_WIDTH;
            decode_file_height = SSP_JPEG_THUMBNAIL_HEIGHT;
            OutputBuffer = MyAlloc(decode_file_width * decode_file_height * sizeof(u16));
            
            result = SSP_StartJpegDecoder(data, length, OutputBuffer, &decode_file_width, &decode_file_height, SSP_JPEG_THUMBNAIL | SSP_JPEG_RGB555);
            
            jpegDecode_end = OS_GetTick();
            
            if(result == 0)
                OS_Panic("Failed Cpu Decode\n");
            
            OS_TPrintf("[TIMER] Thumbnail SSP_StartJpegDecoder time[usec] = %d\n", OS_TicksToMicroSeconds(jpegDecode_end - jpegDecode_begin));
            OS_TPrintf("[DEBUG] Thumbnail decode_file_width = %d : decode_file_height = %d\n", decode_file_width, decode_file_height);
            
            
            // 日時を表示してみる
            {
                u8 dateTime[20];
                
                (void)SSP_GetJpegDecoderDateTime(dateTime);
                OS_TPrintf("DateTimeOriginal = %s\n", dateTime);
            }
            {
                char software[30];
                u32 initialCode;
                
                (void)SSP_GetJpegDecoderSoftware(software);
                // EXIFのsoftwareタグにはイニシャルコードがリトルエンディアンで入っている
                MI_CpuCopy8(software, &initialCode, 4);
                OS_TPrintf("InitialCode = %c%c%c%c\n", initialCode>>24, initialCode>>16, initialCode>>8, initialCode);
            }
            // メーカーノートのアドレス、サイズを取得
            {
                u8 *makerNoteAddr = SSP_GetJpegDecoderMakerNoteAddrEx(SSP_MAKERNOTE_PHOTO);
                u16 makerNoteSize = SSP_GetJpegDecoderMakerNoteSizeEx(SSP_MAKERNOTE_PHOTO);
                u8 *makerNoteUserAddr = SSP_GetJpegDecoderMakerNoteAddrEx(SSP_MAKERNOTE_USER);
                u16 makerNoteUserSize = SSP_GetJpegDecoderMakerNoteSizeEx(SSP_MAKERNOTE_USER);
                
                OS_TPrintf("MakerNoteAddr = 0x%X\n", makerNoteAddr);
                OS_TPrintf("MakerNoteSize = 0x%X\n", makerNoteSize);
                OS_TPrintf("MakerNoteUserAddr = 0x%X\n", makerNoteUserAddr);
                OS_TPrintf("MakerNoteUserSize = 0x%X\n", makerNoteUserSize);
            }
            MyFree(OutputBuffer);
        }
        MyFree(data);
    }
    OS_Terminate();
}

