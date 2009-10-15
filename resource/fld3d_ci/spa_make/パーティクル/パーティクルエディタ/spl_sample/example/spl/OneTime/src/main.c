/*-------------------------------------------
 * SPL library サンプル "OneTime"
 *
 * 自滅エミッタの方向を放出前に変更するサンプルです
 *
 * エミッタの放出方向を変更するコールバック関数を作成し、
 * SPL_CreateWithInitializeで作成します。
 *
 * (c)2004 Nintendo
 * 
 * 
 * $Log: main.c,v $
 * Revision 1.3  2005/02/18 08:31:28  okane_shinji
 * VRAMﾇﾛﾃﾖﾊﾑｹｹ､ﾋ､ﾈ､筅ﾊ､ｦｽ､ﾀｵ
 *
 * Revision 1.2  2004/12/28 02:22:56  okane_shinji
 * (none)
 *
 * Revision 1.1  2004/12/28 02:00:21  okane_shinji
 * (none)
 *
 * Revision 1.2  2004/12/02 07:30:12  okane_shinji
 * 初登録
 *
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
 * 作成された直後のエミッタが引数として渡される
 *-------------------------------------------------*/
 
static void CallbackFunc( SPLEmitter* pEmitter )
{
    VecFx16 vec = {FX16_ONE,0,0} ;
    
    // 方向を変更する
    SPL_SetEmitterAxis( pEmitter, &vec ); 
}


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
    spl_manager = SPL_Init( OS_AllocFromMain, 1, 100, 0, 1, 30 );

    // パーティクルリソースを読み込み
    resPtcl = PtclDemo_LoadFile( "data/output.spa" ) ;
    SPL_Load( spl_manager, resPtcl );

    // テクスチャを読み込み
    (void)SPL_LoadTexByVRAMManager( spl_manager ) ;
    (void)SPL_LoadTexPlttByVRAMManager( spl_manager ) ;

    // エミッタを作成
    (void)SPL_CreateWithInitialize( spl_manager, 0, CallbackFunc ) ;

}

/*---------------------------------------------------
 * 描画
 *-------------------------------------------------*/
static void draw()
{
    MtxFx43 camera ;

    PtclDemo_Lookat( 0, 0, FX32_ONE * 50, 0, 0, 0, 0, FX32_ONE, 0, &camera ) ;

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