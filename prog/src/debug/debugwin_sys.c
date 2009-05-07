//======================================================================
/**
 * @file	debugwin_sys.c
 * @brief	汎用的デバッグシステム
 * @author	ariizumi
 * @data	09/05/01
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "print/printsys.h"

#include "debug/debugwin_sys.h"

#if USE_DEBUGWIN_SYSTEM

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define DEBWIN_HEAPSIZE (0x2000)
#define DEBWIN_WIDTH  (20)
#define DEBWIN_HEIGHT (24)


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//フラグ用ビット
enum
{
  DWF_IS_ACTIVE = 1,
  DWF_IS_INIT   = 2,
  DWF_MASK      = 0xFFFF,
}DEBUGWIN_FLAG;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  u16 flg;
  u32 frmnum;
  GFL_FONT *fontHandle;

	u8  *charTempArea;			///<キャラクタVRAM退避先
	u16 *scrnTempArea;			///<スクリーンVRAM退避先
	u16 *plttTempArea;			///<パレットVRAM退避先
	GFL_BMP_DATA *bmp;

}DEBWIN_SYSTEM_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void DEBUGWIN_OpenDebugWindow( void );
static void DEBUGWIN_CloseDebugWindow( void );


static DEBWIN_SYSTEM_WORK *debWork = NULL;

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
void DEBUGWIN_InitSystem( u8* charArea , u16* scrnArea , u16* plttArea )
{
  
  if( debWork != NULL )
  {
    OS_TPrintf("Debug system is initialized yet!!\n");
    return;
  }
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP+HEAPDIR_MASK, HEAPID_DEBUGWIN, DEBWIN_HEAPSIZE );

  debWork = GFL_HEAP_AllocMemory( HEAPID_DEBUGWIN , sizeof(DEBWIN_SYSTEM_WORK) );
  debWork->flg = 0;
  debWork->charTempArea = charArea;
  debWork->scrnTempArea = scrnArea;
  debWork->plttTempArea = plttArea;
  DEBUGWIN_ExitProc();
  
}

void DEBUGWIN_ExitSystem(void)
{
  if( debWork == NULL )
  {
    OS_TPrintf("Debug system isn't initialized!!\n");
    return;
  }
  
  GFL_HEAP_FreeMemory( debWork );
  debWork = NULL;
  GFL_HEAP_DeleteHeap( HEAPID_DEBUGWIN );
  
}

void DEBUGWIN_UpdateSystem(void)
{
  if( (debWork->flg & DWF_IS_INIT) )
  {
    if( GFL_UI_KEY_GetTrg() & DEBUGWIN_TRG_KEY )
    {
      debWork->flg = debWork->flg ^ DWF_IS_ACTIVE;
      
      if( DEBUGWIN_IsActive() == TRUE )
      {
        DEBUGWIN_OpenDebugWindow();
      }
      else
      {
        DEBUGWIN_CloseDebugWindow();
      }
    }
  }
}

BOOL DEBUGWIN_IsActive(void)
{
  return (debWork->flg & DWF_IS_ACTIVE);
}

void DEBUGWIN_InitProc( const u32 frmnum , GFL_FONT *fontHandle )
{
  debWork->frmnum = frmnum;
  debWork->fontHandle = fontHandle;
  debWork->flg = debWork->flg | DWF_IS_INIT;
}

void DEBUGWIN_ExitProc( void )
{
  debWork->frmnum = 0;
  debWork->fontHandle = NULL;
  debWork->flg = debWork->flg & (DWF_MASK-DWF_IS_INIT);
}

static void DEBUGWIN_OpenDebugWindow( void )
{
	//VRAMのデータを退避
	GFL_STD_MemCopy16(GFL_DISPUT_GetCgxPtr(debWork->frmnum), debWork->charTempArea, DEBWIN_CHAR_TEMP_AREA);
	GFL_STD_MemCopy16(GFL_DISPUT_GetScrPtr(debWork->frmnum), debWork->scrnTempArea, DEBWIN_SCRN_TEMP_AREA);
	GFL_STD_MemCopy16(GFL_DISPUT_GetPltPtr(debWork->frmnum), debWork->plttTempArea, DEBWIN_PLTT_TEMP_AREA);

  //スクリーンの作成
  {
    u8 x,y;
    for( x=0;x<DEBWIN_WIDTH;x++ )
    {
      for( y=0;y<DEBWIN_HEIGHT;y++ )
      {
        u16 buf = x+y*DEBWIN_WIDTH;
        GFL_BG_WriteScreen( debWork->frmnum , &buf , x,y,1,1 );
      }
    }
    GFL_BG_LoadScreenV_Req(debWork->frmnum);
  }
	
	debWork->bmp = GFL_BMP_CreateInVRAM( GFL_DISPUT_GetCgxPtr(debWork->frmnum) , DEBWIN_WIDTH , DEBWIN_HEIGHT , GFL_BMP_16_COLOR , HEAPID_DEBUGWIN );
  GFL_BMP_Clear( debWork->bmp , 0 );
  {
    STRCODE code[4] ={L'て',L'す',L'と',0xFFFF};
    STRBUF *str = GFL_STR_CreateBuffer( 8 , HEAPID_DEBUGWIN);
    GFL_STR_SetStringCode( str , code );
    PRINTSYS_Print( debWork->bmp , 0,0, str, debWork->fontHandle );
    PRINTSYS_Print( debWork->bmp , 16,0, str, debWork->fontHandle );
    PRINTSYS_Print( debWork->bmp , 16,16, str, debWork->fontHandle );
    PRINTSYS_Print( debWork->bmp , 32,16, str, debWork->fontHandle );
    PRINTSYS_Print( debWork->bmp , 32,32, str, debWork->fontHandle );
    GFL_STR_DeleteBuffer( str );
  }

}

static void DEBUGWIN_CloseDebugWindow( void )
{
  GFL_BMP_Delete( debWork->bmp );
  
	GFL_STD_MemCopy16(debWork->charTempArea, GFL_DISPUT_GetCgxPtr(debWork->frmnum), DEBWIN_CHAR_TEMP_AREA);
	GFL_STD_MemCopy16(debWork->scrnTempArea, GFL_DISPUT_GetScrPtr(debWork->frmnum), DEBWIN_SCRN_TEMP_AREA);
	GFL_STD_MemCopy16(debWork->plttTempArea, GFL_DISPUT_GetPltPtr(debWork->frmnum), DEBWIN_PLTT_TEMP_AREA);
}

#else //USE_DEBUGWIN_SYSTEM
#pragma mark [>dummy
void DEBUGWIN_InitSystem( u8* charArea , u16* scrnArea , u16* plttArea ){}
void DEBUGWIN_UpdateSystem(void){}
void DEBUGWIN_ExitSystem(void){}
BOOL DEBUGWIN_IsActive(void){return FALSE;}


#endif //USE_DEBUGWIN_SYSTEM
