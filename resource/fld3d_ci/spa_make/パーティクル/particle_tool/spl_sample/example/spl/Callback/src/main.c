/*-------------------------------------------
 * SPL library サンプル "Callback"
 *
 * コールバック関数を用いたサンプルです。
 * 
 * コールバック関数の中でパーティクルが
 * 一定の位置に到達したら、そのパーティクルを終了
 * させます。また、Aボタンを押すことで
 * コールバック関数を呼び出すかどうか切り替えることができます。
 *
 * (c)2004 Nintendo
 * 
 * 
 * $Log: main.c,v $
 * Revision 1.4  2005/02/18 08:31:28  okane_shinji
 * VRAMﾇﾛﾃﾖﾊﾑｹｹ､ﾋ､ﾈ､筅ﾊ､ｦｽ､ﾀｵ
 *
 * Revision 1.3  2005/02/09 00:15:54  okane_shinji
 * コールバック関数の仕様変更に対応
 *
 * Revision 1.2  2004/12/02 07:59:54  okane_shinji
 * ver110対応
 *
 * Revision 1.1  2004/09/15 01:29:43  okane_shinji
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

BOOL callback_enable = FALSE ;

/*---------------------------------------------------
 * コールバック関数
 * この関数はエミッタemiのCalc中から呼び出されます。
 *-------------------------------------------------*/
static void sCallbackFunc( SPLEmitter* emi, unsigned int uFlag )
{
    // 先頭のエミッタを取得
    SPLParticle* ptcl = SPL_GetHeadParticle( emi ) ;

    // コールバックの呼び出し位置を確認
    if( uFlag != SPL_EMITTER_CALLBACK_BACK )
    {
        return ;
    }

    // エミッタに属するパーティクルを調べていって、範囲外にあれば寿命を終了
    while(1)
    {
        VecFx32 ptcl_pos ;
        
        // パーティクルの位置を取得する
        SPL_GetParticlePos( ptcl, &ptcl_pos ) ;
        
        // パーティクルのy位置がある値(FX32_ONE)以上であればパーティクルを終了する
        if( ptcl_pos.y > FX32_ONE )
        {
            SPL_TerminateParticleLife( ptcl ) ;
        }

        // 次のパーティクルを調べる。
        ptcl = SPL_GetNextParticle( ptcl ) ;
    
        if( ptcl == NULL )
        {
            break ;
        }
    }
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
    spl_emitter = SPL_Create( spl_manager, 0, &init_pos );

    // エミッタにコールバック関数を割り当てる
    SPL_SetCallbackFunc( spl_emitter, sCallbackFunc ) ;
    callback_enable = TRUE ;

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

    // Aボタンが押されたらコールバックの有効 / 無効を切り替える
    if( PtclDemo_KeyOneshot( PAD_BUTTON_A ) )
    {
        if( callback_enable )
        {
            SPL_SetCallbackFunc( spl_emitter, NULL ) ;
        } else {
            SPL_SetCallbackFunc( spl_emitter, sCallbackFunc ) ;
        }
        callback_enable = !callback_enable ;  
    }
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

        PtclDemo_UpdateInput() ;

        G3_SwapBuffers(GX_SORTMODE_MANUAL, GX_BUFFERMODE_W);
        SVC_WaitVBlankIntr(); 
    }
}



// end of file