/*---------------------------------------------
  Utility program codes for particle demo .
  system.
 
 $Log: ptcl_demo_system.c,v $
 Revision 1.2  2004/12/27 04:32:20  okane_shinji
 (none)

 Revision 1.1  2004/09/15 01:23:09  okane_shinji
 初期登録

 
 *---------------------------------------------*/

#include<nnsys.h>
#include"ptcl_demo.h"

#define DEFAULT_DMA_NUMBER      MI_DMA_MAX_NUM

// 確保するヒープサイズ
#define   MAIN_HEAP_SIZE    0x64000

/*-----------------------------------------------------------------
  Name       : PtclDemo_VBlankIntr( void )
  Description: Vブランク
  Returns    : なし
 *-----------------------------------------------------------------*/
static void PtclDemo_VBlankIntr( void )
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_InitSystem( void )
  Description: システムを初期化します
  Returns    : なし
 *-----------------------------------------------------------------*/
void PtclDemo_InitSystem( void )
{
    OS_Init();
    FX_Init();
    GX_SetPower(GX_POWER_ALL);
    GX_Init();
    OS_InitTick();
    GX_DispOff();
    GXS_DispOff();

    OS_SetIrqFunction(OS_IE_V_BLANK, PtclDemo_VBlankIntr);

    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    FS_Init( FS_DMA_NOT_USE ); // Irq関係の関数の後ろに持ってこないといけない模様
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);

}

/*-----------------------------------------------------------------
  Name       : PtclDemo_InitVRAM( void )
  Description: VRAMを初期化します
  Returns    : なし
 *-----------------------------------------------------------------*/
void PtclDemo_InitVRAM( void )
{
    GX_SetBankForLCDC( GX_VRAM_LCDC_ALL );

    MI_CpuClearFast( (void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE );
    
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast ( (void*)HW_OAM    , 192, HW_OAM_SIZE    );
    MI_CpuFillFast ( (void*)HW_DB_OAM , 192, HW_DB_OAM_SIZE );

    MI_CpuClearFast( (void*)HW_PLTT   , HW_PLTT_SIZE    );
    MI_CpuClearFast( (void*)HW_DB_PLTT, HW_DB_PLTT_SIZE );
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_InitMemory( void )
  Description: メモリを初期化します
  Returns    : なし
 *-----------------------------------------------------------------*/

void PtclDemo_InitMemory( void )
{
    void*          heapStart;
    void*          nstart;
    OSHeapHandle   handle;

    //---- MainRAM アリーナに対して メモリ割り当てシステム初期化
    nstart = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 2 );
    OS_SetMainArenaLo( nstart );

    //---- アリーナからヒープ用領域確保
    heapStart = OS_AllocFromMainArenaLo( MAIN_HEAP_SIZE, 32 );

    //---- ヒープ作成
    handle = OS_CreateHeap( OS_ARENA_MAIN, heapStart, (void*)( (u32)heapStart + MAIN_HEAP_SIZE ) );

    //---- カレントヒープ設定
    (void)OS_SetCurrentHeap( OS_ARENA_MAIN, handle );

}

/*-----------------------------------------------------------------
  Name       : PtclDemo_LoadFile( const char *path )
  Description: ファイルを読み込んでメモリにマッピングします。
  Returns    : マッピングされたメモリのアドレス。
 *-----------------------------------------------------------------*/
void* PtclDemo_LoadFile( const char *path )
{

    FSFile file;
    void*  memory;

    FS_InitFile( &file );
    if ( FS_OpenFile( &file, path ) )
    {
        u32 fileSize = FS_GetLength( &file );

        memory = OS_Alloc( fileSize );
        if ( memory == NULL )
        {
            OS_Printf( "no enough memory.\n" ) ;
        }
        else
        {
            if ( FS_ReadFile( &file, memory, fileSize ) != fileSize )   // ファイルサイズ分読み込めていない場合
            {
                OS_Free( memory );
                memory = NULL;
                OS_Printf( "file reading failed.\n" ) ;
            }
        }
        ( void )FS_CloseFile( &file ) ;
    }
    return memory;

}








// end of file