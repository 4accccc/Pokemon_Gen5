//======================================================================
/**
 * @file	debug_sd_print.c
 * @brief	SDカードにデバッグ出力を行う
 * @author	ariizumi
 * @data	09/12/24
 *
 * モジュール名：DEB_SD_PRINT
 */
//======================================================================
#include <gflib.h>
#include <assert_default.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "debug/debug_sd_print.h"

#if DEB_SD_PRINT_ENABLE

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define DEB_SD_PRINT_FILESIZE (1024*512)
#define DEB_SD_PRINT_MSG_BUFFER (0x40000)  //ログを溜めておくバッファ

//OFFの場合、sd_printではアサートが出ない。
#define DEB_SD_PRINT_ENABLE_ASSERT (0)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
	BOOL   isEnable;
	BOOL   isCallAssert;
	FSFile writeFile;

  u32    fileSize;
  u32    buffPos;
  
  char   *msgBuffer;
}DEB_SD_PRINT_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static DEB_SD_PRINT_WORK *sdPrintWork = NULL;
static char commonBuffer[256];

static void DEB_SD_PRINT_OpenFile(void);
static const BOOL DEB_SD_PRINT_IsEnable(void);
static void DEB_SD_PRINT_PrintCore( const char *str );

static void DEB_SD_PRINT_AssertDispMain( const char* str );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
void DEB_SD_PRINT_InitSystem( const HEAPID heapId )
{
  
  if( OS_IsRunOnTwl() )
  {
  	sdPrintWork = GFL_HEAP_AllocMemory( heapId , sizeof(DEB_SD_PRINT_WORK));
  	sdPrintWork->msgBuffer = GFL_NET_Align32Alloc( GFL_HEAPID_TWL , DEB_SD_PRINT_MSG_BUFFER);
    sdPrintWork->isEnable = FALSE;
    
    FS_InitFatDriver();
    {
      sdPrintWork->buffPos = 0;
      sdPrintWork->isEnable = TRUE;
      sdPrintWork->isCallAssert = FALSE;
      DEB_SD_PRINT_OpenFile();
    }
    
    //ASSERTを上書き
    if( OS_GetConsoleType() & (OS_CONSOLE_NITRO|OS_CONSOLE_TWL) )
    {
      // ROM実行用（可能なら情報をLCD表示）
      GFL_ASSERT_SetDisplayFunc( ASSERT_DEFAULT_dispInit , DEB_SD_PRINT_AssertDispMain , ASSERT_DEFAULT_dispFinish );
    }
    else
    {
      // NITROデバッガ用（コールスタック表示＆可能なら処理続行のため）
      GFL_ASSERT_SetDisplayFunc( NULL, DEB_SD_PRINT_AssertDispMain, GFUser_AssertFinish );
    }
  }
}
void DEB_SD_PRINT_UpdateSystem(void)
{
  if( DEB_SD_PRINT_IsEnable() == TRUE )
  {
    if( sdPrintWork->buffPos > 0 )
    {
      int ret;

      sdPrintWork->fileSize += sdPrintWork->buffPos;
      ret = FS_SetFileLength( &sdPrintWork->writeFile , sdPrintWork->fileSize );
#if DEB_SD_PRINT_ENABLE_ASSERT
      GF_ASSERT_MSG( ret == 0  , "SDCard Error Set size[%d]\n",FS_GetResultCode(&sdPrintWork->writeFile));
#else
      if( ret != 0 ){ sdPrintWork->isEnable = FALSE; return; }
#endif
      FS_WriteFile( &sdPrintWork->writeFile , sdPrintWork->msgBuffer , sdPrintWork->buffPos );
      sdPrintWork->buffPos = 0;
      
      if( sdPrintWork->fileSize > 1024*32 )
      {
        const BOOL ret = FS_CloseFile(&sdPrintWork->writeFile);
#if DEB_SD_PRINT_ENABLE_ASSERT
        GF_ASSERT_MSG( ret , "SDCard Error Close[%d]\n",FS_GetResultCode(&sdPrintWork->writeFile));
#else
        if( ret == FALSE ){ sdPrintWork->isEnable = FALSE; return; }
#endif
        DEB_SD_PRINT_OpenFile();
      }
    }
  }
}

static void DEB_SD_PRINT_OpenFile(void)
{
  RTCDate date;
  RTCTime time;
  FSPathInfo info;
  int ret = FS_GetPathInfo("sdmc:/",&info );
  if( ret == TRUE )
  {
    int size;
    char fileName[64];
    sdPrintWork->fileSize = 0;
    GFL_RTC_GetDateTime(&date,&time);
    STD_TSPrintf( fileName , "sdmc:/%02d%02d%02d_%02d%02d%02d.txt",
                                date.year ,
                                date.month ,
                                date.day ,
                                time.hour ,
                                time.minute ,
                                time.second );
    ret = FS_CreateFile(fileName, FS_PERMIT_R|FS_PERMIT_W );
#if DEB_SD_PRINT_ENABLE_ASSERT
    GF_ASSERT_MSG( ret , "SDCard Error Create[%d]\n",FS_GetArchiveResultCode("sdmc:/"));
#else
    if( ret != 0 ){ sdPrintWork->isEnable = FALSE; return; }
#endif
    ret = FS_OpenFileEx( &sdPrintWork->writeFile , fileName , FS_FILEMODE_RW );
#if DEB_SD_PRINT_ENABLE_ASSERT
    GF_ASSERT_MSG( ret , "SDCard Error Open[%d]\n",FS_GetResultCode(&sdPrintWork->writeFile));
#else
    if( ret != 0 ){ sdPrintWork->isEnable = FALSE; return; }
#endif
  }
  else
  {
    sdPrintWork->isEnable = FALSE;
  }
  
}

static const BOOL DEB_SD_PRINT_IsEnable(void)
{
  if( sdPrintWork != NULL &&
      sdPrintWork->isEnable == TRUE )
  {
    return TRUE;
  }
  return FALSE;
}

static void DEB_SD_PRINT_PrintCore( const char *str )
{
  if( DEB_SD_PRINT_IsEnable() == TRUE )
  {
    int ret;
    const int len = STD_StrLen( str );
    if( sdPrintWork->buffPos + len >= DEB_SD_PRINT_MSG_BUFFER )
    {
#if DEB_SD_PRINT_ENABLE_ASSERT
      GF_ASSERT_MSG( 0 , "SDCard Error Buffer is over![%d]\n",sdPrintWork->buffPos + len);
#else
      sdPrintWork->isEnable = FALSE;
      return;
#endif
      
    }
    GFL_STD_MemCopy( str , &sdPrintWork->msgBuffer[sdPrintWork->buffPos] , len );
    sdPrintWork->buffPos += len;
  }
}

static void DEB_SD_PRINT_AssertDispMain( const char* str )
{
  //内容の表示と書き出し
  OS_TPrintf(str);
  //DEB_SD_PRINT_PrintCore( str );
  if( sdPrintWork->isCallAssert == FALSE )
  {
    sdPrintWork->isCallAssert = TRUE;
    DEB_SD_PRINT_UpdateSystem();
  }
  
  if( OS_GetConsoleType() & OS_CONSOLE_NITRO )
  {
    // ROM実行用（可能なら情報をLCD表示）
    ASSERT_DEFAULT_dispMain(str);
  }
}

void OS_VFPrintf(int console, const char *fmt, va_list vlist)
{
  (void)vsnprintf(commonBuffer, sizeof(commonBuffer), fmt, vlist);
  OS_FPutString(console, commonBuffer);
  
  //独自処理
  DEB_SD_PRINT_PrintCore( commonBuffer );
}

void OS_TVFPrintf(int console, const char *fmt, va_list vlist)
{
  (void)OS_VSNPrintf(commonBuffer, sizeof(commonBuffer), fmt, vlist);
  OS_FPutString(console, commonBuffer);
  
  //独自処理
  DEB_SD_PRINT_PrintCore( commonBuffer );
}

#ifndef SDK_FINALROM
void OSi_Panic(const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    (void)OS_DisableInterrupts();
    OS_FPrintf(OS_PRINT_OUTPUT_ERROR, "%s:%d Panic:", file, line);
    OS_VFPrintf(OS_PRINT_OUTPUT_ERROR, fmt, vlist);
    OS_FPrintf(OS_PRINT_OUTPUT_ERROR, "\n");

    //内容の書き出し
    if( sdPrintWork->isCallAssert == FALSE )
    {
      sdPrintWork->isCallAssert = TRUE;
      DEB_SD_PRINT_UpdateSystem();
    }

    OS_Terminate();               // Never Returns
}

void OSi_TPanic(const char *file, int line, const char *fmt, ...)
{
    va_list vlist;

    va_start(vlist, fmt);
    (void)OS_DisableInterrupts();
    OS_TPrintf("%s:%d Panic:", file, line);
    OS_TVPrintf(fmt, vlist);
    OS_TPrintf("\n");

    //内容の書き出し
    if( sdPrintWork->isCallAssert == FALSE )
    {
      sdPrintWork->isCallAssert = TRUE;
      DEB_SD_PRINT_UpdateSystem();
    }

    OS_Terminate();               // Never Returns
}
#endif //SDK_FINALROM

#else
void DEB_SD_PRINT_InitSystem( const HEAPID heapId )
{
}
void DEB_SD_PRINT_UpdateSystem(void)
{
}


#endif //DEB_SD_PRINT_ENABLE
