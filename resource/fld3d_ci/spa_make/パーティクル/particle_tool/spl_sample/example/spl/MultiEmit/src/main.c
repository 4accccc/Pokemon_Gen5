/*-------------------------------------------
 * SPL library サンプル "MultiEmit"
 *
 * 一つのエミッタで同時に複数地点からパーティクルを
 * 放出するサンプルです。 
 *
 * エミッタを生成した後、自動的な放出を停止します。
 * 次にSPL_Calcの前でSPL_EmitもしくはSPL_EmitAtを呼び出します。
 *
 * (c)2004 Nintendo
 * 
 * 
 * $Log: main.c,v $
 * Revision 1.6  2005/02/18 08:31:21  okane_shinji
 * VRAMﾇﾛﾃﾖﾊﾑｹｹ､ﾋ､ﾈ､筅ﾊ､ｦｽ､ﾀｵ
 *
 * Revision 1.3  2004/12/02 08:01:22  okane_shinji
 * ver110対応
 *
 * Revision 1.1  2004/09/15 01:29:53  okane_shinji
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

// パーティクル関係
static SPLManager* spl_manager = NULL ;
static SPLEmitter* spl_emitter = NULL ;
static VecFx32 init_pos = {0,0,0} ;
void*  resPtcl ;

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

    // VRAMマネージャをセット
    NNS_GfdInitFrmTexVramManager(3, TRUE);
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);

    //////////////////////////////////////////////////
    // SPL初期化

    // パーティクルマネージャを初期化
    spl_manager = SPL_Init( OS_AllocFromMain, 2, 100, 0, 1, 30 );

    // パーティクルリソースを読み込み
    resPtcl = PtclDemo_LoadFile( "data/output.spa" ) ;
    SPL_Load( spl_manager, resPtcl );

    // テクスチャを読み込み
    (void)SPL_LoadTexByVRAMManager( spl_manager ) ;
    (void)SPL_LoadTexPlttByVRAMManager( spl_manager ) ;

    // エミッタを作成
    spl_emitter = SPL_Create( spl_manager, 0, &init_pos );

    // 自動的なパーティクル放出を停止する
    SPL_StopEmission( spl_emitter ); 

}

/*---------------------------------------------------
 * 描画
 *-------------------------------------------------*/
static void draw()
{
    MtxFx43 camera ;

    PtclDemo_Lookat( 0, 0, FX32_ONE * 20, 0, 0, 0, 0, FX32_ONE, 0, &camera ) ;

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

    const VecFx32 pos1 = { -FX32_ONE * 10, 0, 0 } ;
    const VecFx32 pos2 = {  FX32_ONE * 10, 0, 0 } ;

    // 指定した位置からパーティクルを放出
    SPL_EmitAt( spl_manager, spl_emitter, &pos1 ) ; 
    SPL_EmitAt( spl_manager, spl_emitter, &pos2 ) ;     

    SPL_Calc( spl_manager ) ;
    
//    OS_Printf("エミッタ数%d パーティクル数%d\n", SPL_GetEmitterNum(spl_manager), SPL_GetWholeParticleNum( spl_manager ) ) ;

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