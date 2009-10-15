/*-------------------------------------------
 * SPL library サンプル "FreeRes"
 *
 * メモリに読み込んだリソースのうち
 * テクスチャ部分だけ解放するサンプルです。
 * テクスチャはVRAMに読み込まれた後は
 * 使用されないので、このようなことが可能です。
 *
 * (c)2004 Nintendo
 * 
 * 
 * $Log: main.c,v $
 * Revision 1.3  2005/02/18 08:31:28  okane_shinji
 * VRAMﾇﾛﾃﾖﾊﾑｹｹ､ﾋ､ﾈ､筅ﾊ､ｦｽ､ﾀｵ
 *
 * Revision 1.2  2004/12/02 07:59:50  okane_shinji
 * ver110対応
 *
 * Revision 1.1  2004/09/15 01:28:53  okane_shinji
 * 初期登録
 *
 *
 *
 *-------------------------------------------*/



#include<nitro.h>
#include<nnsys/fnd.h>
#include<nnsys/gfd.h>
//#include<nnsys/g3d.h>

#include"ptcl_demo.h"
#include"spl.h"

// パーティクル関係
static SPLManager* spl_manager = NULL ;
static SPLEmitter* spl_emitter = NULL ;
static VecFx32 init_pos = {0,0,0} ;
void*  resPtcl ;

// ヒープ用のメモリ
#define HeapBufElementNum (4096/sizeof(u32))
static u32 sHeapBuf[HeapBufElementNum];

// ヒープ
NNSFndHeapHandle hFrmHeap;

/*---------------------------------------------------
 * フレームヒープにファイルをマッピングする
 *-------------------------------------------------*/
static void* loadfile_to_frameheap( const char *path )
{
    FSFile file;
    void*  memory;

    FS_InitFile( &file );
    if ( FS_OpenFile( &file, path ) )
    {
        u32 fileSize = FS_GetLength( &file );

        memory = NNS_FndAllocFromFrmHeap( hFrmHeap, fileSize );
        if ( memory == NULL )
        {
            OS_Printf( "no enough memory.\n" ) ;
        }
        else
        {
            if ( FS_ReadFile( &file, memory, fileSize ) != fileSize )   // ファイルサイズ分読み込めていない場合
            {
                memory = NULL;
                OS_Printf( "file reading failed.\n" ) ;
            }
        }
        ( void )FS_CloseFile( &file ) ;
    }
    return memory;
}

/*---------------------------------------------------
 * 初期化
 *-------------------------------------------------*/
static void init()
{
    int i ;
    u32 tex_size ;
    u32* p ;

    // 各種初期化
    PtclDemo_InitSystem() ;
    PtclDemo_InitVRAM() ;
    PtclDemo_InitMemory() ;
    PtclDemo_InitScreen() ;
    // フレームヒープを初期化
    hFrmHeap = NNS_FndCreateFrmHeap(  sHeapBuf, sizeof( sHeapBuf )  ) ;

    OS_Printf( "確保直後のフレームヒープ : 0x%x ～ 0x%x ( size : %d ) ( 空き : %d ) \n" , NNS_FndGetHeapStartAddress( hFrmHeap ) , NNS_FndGetHeapEndAddress( hFrmHeap ), (u32)NNS_FndGetHeapEndAddress( hFrmHeap ) - (u32)NNS_FndGetHeapStartAddress( hFrmHeap ), NNS_FndGetAllocatableSizeForFrmHeap( hFrmHeap ) ) ;

    // VRAMマネージャをセット
    NNS_GfdInitFrmTexVramManager(3, TRUE);
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);

    //////////////////////////////////////////////////
    // SPL初期化

    // パーティクルマネージャを初期化
    spl_manager = SPL_Init( OS_AllocFromMain, 1, 100, 0, 1, 30 );

    // パーティクルリソースを読み込み
    resPtcl = loadfile_to_frameheap( "data/output.spa" ) ;
    SPL_Load( spl_manager, resPtcl );
    DC_StoreAll();

    // 使っていない領域を開放
    (void)NNS_FndAdjustFrmHeap( hFrmHeap ) ;
    OS_Printf( "確保直後のフレームヒープ : 0x%x ～ 0x%x ( size : %d ) ( 空き : %d ) \n" , NNS_FndGetHeapStartAddress( hFrmHeap ) , NNS_FndGetHeapEndAddress( hFrmHeap ), (u32)NNS_FndGetHeapEndAddress( hFrmHeap ) - (u32)NNS_FndGetHeapStartAddress( hFrmHeap ), NNS_FndGetAllocatableSizeForFrmHeap( hFrmHeap )  ) ;

    // テクスチャを読み込み
    (void)SPL_LoadTexByVRAMManager( spl_manager ) ;
    (void)SPL_LoadTexPlttByVRAMManager( spl_manager ) ;

    // エミッタを作成
    spl_emitter = SPL_Create( spl_manager, 0, &init_pos );

    //////////////////////////////////////////////////
    // メモリにマッピングされたリソースのうち、
    // テクスチャ領域を消去

    // フレームヒープのうちテクスチャ使用部分を切り取る。
    (void)NNS_FndResizeForMBlockFrmHeap( hFrmHeap, sHeapBuf, SPL_GetTexOffsetOnResource( resPtcl ) + 48 ) ;

    // リソースのテクスチャ領域を書き換える（他の用途に使用する）。
    // pはもともとテクスチャのあった領域であるが、
    // その他の用途に使用できるようなる。
    tex_size = SPL_GetTexSizeOnResource( resPtcl ) ; 
    p        = (u32*)NNS_FndAllocFromFrmHeap( hFrmHeap, tex_size );
    for( i = 0 ; i < tex_size / 4 ; i = i + 1 )
    {
        // その他の用途。。
        p[i] = 0 ;
    }


}

/*---------------------------------------------------
 * 描画
 *-------------------------------------------------*/
static void draw()
{
    MtxFx43 camera ;

    PtclDemo_Lookat( 0, 0, FX32_ONE * 10, 0, 0, 0, 0, FX32_ONE, 0, &camera ) ;

    // 座標軸を書く
    PtclDemo_DrawAxis() ;

    // パーティクル描画
    SPL_Draw( spl_manager, &camera );
}

/*---------------------------------------------------
 * 計算
 *-------------------------------------------------*/
static void calc()
{
    SPL_Calc( spl_manager ) ;
}

/*---------------------------------------------------
 * メイン
 *-------------------------------------------------*/
void NitroMain()
{
    init() ;

    while(1)
    {
        G3X_Reset();

        draw();

        calc() ;

        G3_SwapBuffers(GX_SORTMODE_MANUAL, GX_BUFFERMODE_W);
        SVC_WaitVBlankIntr(); 
    }
}



// end of file