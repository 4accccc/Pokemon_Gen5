/*-------------------------------------------
 * SPL library サンプル "Simple"
 *
 * G3DモデルとSPLによるパーティクルを
 * 共存して表示するサンプルです。
 *
 * (c)2004 Nintendo
 * 
 * 
 * $Log: main.c,v $
 * Revision 1.6  2005/02/18 08:31:28  okane_shinji
 * VRAMﾇﾛﾃﾖﾊﾑｹｹ､ﾋ､ﾈ､筅ﾊ､ｦｽ､ﾀｵ
 *
 * Revision 1.5  2005/02/08 07:04:54  okane_shinji
 * (none)
 *
 * Revision 1.4  2004/12/27 04:32:20  okane_shinji
 * (none)
 *
 * Revision 1.3  2004/12/21 02:01:25  okane_shinji
 * (none)
 *
 * Revision 1.2  2004/12/02 08:00:07  okane_shinji
 * (none)
 *
 * Revision 1.1  2004/09/15 01:26:27  okane_shinji
 * 初期登録
 *
 *
 *
 *-------------------------------------------*/



#include<nitro.h>
#include<nnsys/fnd.h>
#include<nnsys/gfd.h>
#include<nnsys/g3d.h>

#include"ptcl_demo.h"
#include"spl.h"


// モデル関係
static NNSG3dRenderObj      object         ;
static NNSG3dResMdl*        model   = NULL ;
static NNSG3dResFileHeader* resFile = NULL ;

// パーティクル関係
static SPLManager* spl_manager = NULL ;
static SPLEmitter* spl_emitter = NULL ;
static VecFx32 init_pos = {0,0,0} ;
void*  resPtcl ;

// テクスチャ読み込みを自前でおこなう場合のコールバック関数定義例
#if 0
/*---------------------------------------------------
 * テクスチャVRAMアドレスを返すためのコールバック関数
 *-------------------------------------------------*/
static u32 sAllocTex( u32 size, BOOL is4x4comp )
{
    NNSGfdTexKey key = NNS_GfdAllocTexVram( size, is4x4comp, 0 );
    return NNS_GfdGetTexKeyAddr( key ) ;
}

/*---------------------------------------------------
 * テクスチャパレットVRAMアドレスを返すためのコールバック関数
 *-------------------------------------------------*/
static u32 sAllocTexPalette(  u32 size, BOOL is4x4comp )
{
    NNSGfdPlttKey key = NNS_GfdAllocPlttVram( size, is4x4comp, 0 );
    return NNS_GfdGetPlttKeyAddr( key ) ;
}
#endif



/*---------------------------------------------------
 * 初期化
 *-------------------------------------------------*/
static void init()
{
    // 各種初期化
    PtclDemo_InitSystem() ;
    PtclDemo_InitVRAM() ;
    PtclDemo_InitMemory() ;
    PtclDemo_InitScreen() ;

    //////////////////////////////////////////////////
    // G3D初期化とモデル読み込み

    // G3D初期化
    NNS_G3dInit();

    // VRAMマネージャをセット
    NNS_GfdInitFrmTexVramManager(3, TRUE);
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);
    
    // モデル読み込み
    resFile = PtclDemo_LoadFile( "data/mario.nsbmd" ) ;
    SDK_ASSERTMSG( resFile, "loading resource is failed." ); 

    // リソースファイルを初期化
    if( !NNS_G3dResDefaultSetup(resFile) )
    {
        OS_Panic( "NNS_G3dResDefaultSetup failed" ) ;
    }
        
    // モデルの取得
    model = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(resFile), 0);
    SDK_ASSERTMSG( model, "getting model is failed." ); 

    // モデルを関連付ける
    NNS_G3dRenderObjInit(&object, model);

    // 透視投影とカメラをセット
    {
        VecFx32 pos = { 0, FX32_ONE * 10, FX32_ONE * 50 } ;
        VecFx32 up  = { 0, FX32_ONE     , 0             } ;
        VecFx32 at  = { 0, 0            , 0             } ;
        
        NNS_G3dGlbPerspective( FX32_SIN30, FX32_COS30, FX32_ONE * 4 / 3, FX32_ONE, FX32_ONE * 5000 );
        NNS_G3dGlbLookAt( &pos, &up, &at );
    }


    //////////////////////////////////////////////////
    // SPL初期化

    // パーティクルマネージャを初期化
    spl_manager = SPL_Init( OS_AllocFromMain, 1, 100, 0, 1, 30 );

    // パーティクルリソースを読み込み
    resPtcl = PtclDemo_LoadFile( "data/output.spa" ) ;
    SPL_Load( spl_manager, resPtcl );

    // テクスチャを読み込み
    (void)SPL_LoadTexByVRAMManager( spl_manager ) ;
    (void)SPL_LoadTexPlttByVRAMManager( spl_manager ) ;

    #if 0 
    // コールバック関数を用いてテクスチャ読み込みを自前でおこなう場合
    SPL_LoadTexByCallbackFunction( spl_manager, allocTex ) ;
    SPL_LoadTexPlttByCallbackFunction( spl_manager, sAllocTexPalette );
    #endif


    // エミッタを作成
    spl_emitter = SPL_Create( spl_manager, 0, &init_pos );


}

/*---------------------------------------------------
 * 描画
 *-------------------------------------------------*/
static void draw()
{

        // グローバルステートを適用
    NNS_G3dGlbFlush();
    
    // 座標軸を書く
    PtclDemo_DrawAxis() ;


    // パーティクル描画
    SPL_Draw( spl_manager, NNS_G3dGlbGetSrtCameraMtx() );

    // ふたたびグローバルステートを適用してからモデルを描画
    NNS_G3dGlbFlush();
    NNS_G3dDraw(&object);
    
    NNS_G3dGlbFlush();
    
    
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