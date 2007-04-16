//============================================================================================
/**
 *
 * @file	double3Ddisplay.c
 * @brief	両面３Ｄ
 *
 */
//============================================================================================
#include "gflib.h"
#include "double3Ddisp.h"

/*
============================================================================================


		現在試用段階です。
		ディスプレイは外側で両面ＯＮにしてください。あたりまえだけど。

		基本はNitroSDKのサンプルのまま
		サブ画面(非３Ｄ画面)のキャプチャーデータ切り替え用に
		ＢＧ，ＯＢＪリソースをフルに使っているのがネックで
		このままＢＧやＯＢＪと共存できるのかは疑問

		サブVRAMはキャプチャーのために
		ＢＧ，ＯＢＪともに256ビットマップとして128kを必要とする。
		なので現状それを満たすには、VRAM_C,VRAM_Dが固定で使用されることになる。
		VRAM_Dを使用するので、通信との両立は不可能


============================================================================================
*/
typedef struct {
	HEAPID		heapID;

	BOOL		flip_flag;	// flip切り替えフラグ
	BOOL		swap;		// SwapBuffers実行フラグ

	GXOamAttr	sOamBak[128];
//	TCB*		vIntrTask;

}GFL_DOUBLE3D;

GFL_DOUBLE3D* dbl3D = NULL;

static void setupSubOAM(void);
//------------------------------------------------------------------
/**
 * @brief	初期化
 *
 * キャプチャーデータ表示ＯＢＪを設定
 *
 * @param	heapID	ヒープＩＤ
 */
//------------------------------------------------------------------
void 
	GFL_G3D_DOUBLE3D_Init
		( HEAPID heapID )
{
	GF_ASSERT( dbl3D == NULL );

	dbl3D = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_DOUBLE3D) );
	dbl3D->heapID = heapID;
	dbl3D->flip_flag = TRUE;	// flip切り替えフラグ
	dbl3D->swap = FALSE;		// SwapBuffers実行フラグ
	setupSubOAM();
//
//	//VBLANKタスク設定
//	dbl3D->vIntrTask = GFUser_VIntr_CreateTCB( GFL_G3D_DOUBLE3D_VblankIntr, NULL, 0 );
}

//------------------------------------------------------------------
//ＯＡＭ使用設定
static void setupSubOAM(void)
{
    int     i;
    int     x, y;
    int     idx = 0;

    GXS_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_2D_W256);

    for (i = 0; i < 128; ++i){
        dbl3D->sOamBak[i].attr01 = 0;
        dbl3D->sOamBak[i].attr23 = 0;
    }
    for (y = 0; y < 192; y += 64){
        for ( x = 0; x < 256; x += 64, idx++ ){
            G2_SetOBJAttr(	&dbl3D->sOamBak[idx], x, y, 0, 
							GX_OAM_MODE_BITMAPOBJ, FALSE, GX_OAM_EFFECT_NONE,
							GX_OAM_SHAPE_64x64, GX_OAM_COLOR_16, (y / 8) * 32 + (x / 8), 15, 0);
        }
    }
    DC_FlushRange( &dbl3D->sOamBak[0], sizeof(dbl3D->sOamBak) );
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GXS_LoadOAM( &dbl3D->sOamBak[0], 0, sizeof(dbl3D->sOamBak) );

}

//------------------------------------------------------------------
/**
 * @brief	終了
 */
//------------------------------------------------------------------
void 
	GFL_G3D_DOUBLE3D_Exit
		( void )
{
//	GFL_TCB_DeleteTask( dbl3D->vIntrTask );
	GFL_HEAP_FreeMemory( dbl3D );
	dbl3D = NULL;
}


/*
============================================================================================


		描画の際、以下の関数を用いて。各種切り替え設定を行ってください。
		
		GFL_G3D_DOUBLE3D_GetFlip
			現在のflipを取得することにより、上下に相当する描画を切り替えます。
			（カメラ等）

		GFL_G3D_DOUBLE3D_SetSwapFlag
			SwapBuffer終了後に設定してください

		（例）
		static void g3d_draw( void )
		{
			if( flip_flag ){
				GFL_G3D_CAMERA_Switching( g3DcameraLower );
			} else {
				GFL_G3D_CAMERA_Switching( g3DcameraUpper );
			}
			GFL_G3D_SCENE_Draw( g3Dscene );  
			swap = TRUE;
		}


============================================================================================
*/
//------------------------------------------------------------------
/**
 * @brief	flip面フラグ取得
 *
 * @return	TRUE = 下が３Ｄ, FALSE = 上が３Ｄ
 */
//------------------------------------------------------------------
BOOL
	GFL_G3D_DOUBLE3D_GetFlip
		( void )
{
	return	dbl3D->flip_flag;
}

//------------------------------------------------------------------
/**
 * @brief	swap終了フラグ設定
 */
//------------------------------------------------------------------
void 
	GFL_G3D_DOUBLE3D_SetSwapFlag
		( void )
{
	dbl3D->swap = TRUE;
}


/*
============================================================================================


		画面の切り替えはVBLANK中に実行します。
		
		GFL_G3D_DOUBLE3D_VblankIntr( 直接呼出し用 )
		GFL_G3D_DOUBLE3D_VblankIntrTask( TCB用 )

		のいずれかをセットアップしてください。


============================================================================================
*/
static void setupFrame2N( void );
static void setupFrame2N_1( void );
//------------------------------------------------------------------
/**
 * @brief	VBLANK割り込み関数
 */
//------------------------------------------------------------------
#define SPIN_WAIT                      // OS_SpinWait関数を用いる方法と用いない方法を切り替えます。

void
	GFL_G3D_DOUBLE3D_VblankIntr
		( void )
{
    // 画像の描画もバッファのスワップ処理も完了していることを確認します。
#ifdef SPIN_WAIT
    if (GX_GetVCount() <= 193) {
        OS_SpinWait(784);
    }
    if (!G3X_IsGeometryBusy() && dbl3D->swap)
#else
    if ((G3X_GetVtxListRamCount() == 0) && dbl3D->swap)
#endif
        // 画像の描画もスワップ処理も完了している場合、上下画面の切り替えを行います。
    {
        if ( dbl3D->flip_flag ){                 // flipの切り替え(上下画面の切り替え処理)
            setupFrame2N_1();
        } else {
            setupFrame2N();
        }
        dbl3D->swap = FALSE;
        dbl3D->flip_flag = !dbl3D->flip_flag;
    }
}

//------------------------------------------------------------------
//TCB用
#define SPIN_WAIT                      // OS_SpinWait関数を用いる方法と用いない方法を切り替えます。

void	
	GFL_G3D_DOUBLE3D_VblankIntrTCB
		( TCB* tcb, void* work )
{
	GFL_G3D_DOUBLE3D_VblankIntr();
}

//------------------------------------------------------------------
//上画面反映設定
static void setupFrame2N( void )
{

    GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

    (void)GX_ResetBankForSubOBJ();
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
    GX_SetBankForLCDC(GX_VRAM_LCDC_D);
    GX_SetCapture(GX_CAPTURE_SIZE_256x192,
                  GX_CAPTURE_MODE_A,
                  GX_CAPTURE_SRCA_2D3D, (GXCaptureSrcB)0, GX_CAPTURE_DEST_VRAM_D_0x00000, 16, 0);

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);
    //GX_SetVisiblePlane(GX_GetVisiblePlane() | GX_PLANEMASK_BG0);
    //G2_SetBG0Priority(0);

    GXS_SetGraphicsMode(GX_BGMODE_5);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG2);
    G2S_SetBG2ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256,
                           GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
    //G2S_SetBG2Priority(0);
    G2S_BG2Mosaic(FALSE);
}

//------------------------------------------------------------------
//下画面反映設定
static void setupFrame2N_1( void )
{
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
    (void)GX_ResetBankForSubBG();
    GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D);
    GX_SetBankForLCDC(GX_VRAM_LCDC_C);
    GX_SetCapture(GX_CAPTURE_SIZE_256x192,
                  GX_CAPTURE_MODE_A,
                  GX_CAPTURE_SRCA_2D3D, (GXCaptureSrcB)0, GX_CAPTURE_DEST_VRAM_C_0x00000, 16, 0);

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);
    //GX_SetVisiblePlane(GX_GetVisiblePlane() | GX_PLANEMASK_BG0);
    //G2_SetBG0Priority(0);

    GXS_SetGraphicsMode(GX_BGMODE_5);
    GXS_SetVisiblePlane(GX_PLANEMASK_OBJ);
}

/*
---------------------------------------------------------------------------------------------------

▼SwapBuffersコマンドとジオメトリエンジンの動作の説明

・SwapBuffersコマンドはG3_SwapBuffers関数を呼んだときに、ジオメトリコマンドFIFOに格納され
  SwapBuffersコマンドはどのタイミングでジオメトリコマンドFIFOから呼ばれたとしても、
  次のVブランクの開始時に、バッファのスワップ処理が実行されます。
  (つまり、Vブランクの開始時にしか実行できません)
・そのため、もし描画処理などがVブランク期間内までずれ込んで、
  SwapBuffersコマンドの実行がVブランクの開始時までにできなかった場合、
  次のフレームのVブランク開始時までジオメトリエンジンはビジーのまま約1フレーム待ち続ける。
  この間は画像の描画やスワップなどはできないでそのフレームは前と同じ画像を表示する。

s : SwapBuffersコマンド発行
S : バッファのスワップ処理実行
w : SwapBuffersコマンド発行からバッファのスワップ処理実行開始までのウェイト
G : ジオメトリエンジンが描画などの処理中

                          |
-VBlank(end)------------------------------------------
               +-----+    |     +-----+
               |     |    |     |     |
               |     |    |     |     |
               |  G  |    |     |  G  |
               |     |    |     |     |
               |     |    |     |     |
               |     |    |     |     |
               |     |    |     |     |
               |     |    |     |     |
               +-----+    |     |     |
               |  s  |    |     |     |
               +-----+    |     |     |
               |  w  |    |     |     |
-VBlank(start)-+-----+----+-----+     +-----------
  ※784(cycle) |  S  |    |     |     |  ※このスワップ処理にCPUサイクルで784サイクルかかります
               +-----+    |     +-----+           
    ※check  →           |     |  s  |  ※このcheckの部分でG3X_IsGeometryBusy関数を用いて
                          |     +-----+    スワップ処理が終わったかどうかを判定しています。　　
-VBlank(end)--------------+-----+     +---------
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |  w  |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
                          |     |     |
-VBlank(start)------------+-----+-----+----------
                          |     |  S  |
                          |     +-----+
                          |
                          |
-VBlank(end)--------------+-----------------------

★問題が発生する条件
    ・Vブランク内でバッファのスワップ処理が完了していないのに、上下画面の切り替えを行った場合
      上下で同じ画面が表示されてしまいます。

▲問題点を防ぐためVブランク内でやるべきこと
    ・Vブランクに入る前までに、次の画像の描画が終了していることを確認
      描画が終了しているなら、次のVブランク開始時にバッファのスワップ処理が実行されます。
    ・Vブランク内で、バッファのスワップ処理が終了したことを確認
      スワップ処理が終了しているなら、上下画面の切り替えを行います。

    そのために
    ・Vブランクに入る前に、次の画像の描画が終了していること
		(次のVブランクでスワップ処理が確実に始まる)
    ・Vブランク内で、バッファのスワップ処理が終了したこと
    ↑この２つを確認する必要があります。

■具体的にVブランク内で行っている処理

        ・ if(GX_GetVCount() <= 193)        
					スワップ処理するには1ラインもかからないので
                    この時点で193ラインに達していれば、
                    スワップ処理は終わっていると判断でき、
                    OS_SpinWait関数で784サイクル待つ必要がありません。
                                                   
        ・ OS_SpinWait(784)      
					784サイクル待ちます。
        
        ・ if (!G3X_IsGeometryBusy() && swap)
					Vブランク前に SwapBuffers コマンドが発行済みで、
                    ジオメトリエンジンがビジーでない場合
                    (画像の描画処理やバッファのスワップ処理実行中でない)
                                                
●処理の意味
    バッファのスワップ処理(レンダリングエンジンが参照するデータを切り替える)には、
    CPUサイクルで784(392×2)サイクル必要なため、
    もし、Vブランクの開始時にバッファのスワップ処理が開始されたのだとしたら、784サイクル待てば
    バッファのスワップ処理は終わっていると判断できます。

    Vブランク開始から784サイクル待ってジオメトリエンジンがビジーな場合
        Vブランク前の画像の描画処理が遅れたため、バッファのスワップ処理がまだ終了していない。
        →このときはスワップ処理も上下画面切り替えもするべきではありません。
    Vブランク開始から784サイクル待ってジオメトリエンジンがビジーでない場合
        バッファのスワップ処理も画像の描画処理も終了している。
        →画像の描画もスワップ処理も終了しているので、上下画面の切り替えをするべきです。

もし、Vブランク中でのこの784サイクルがどうしてももったいないと思う場合は
この OS_SpinWait(784) の前に何か、784サイクル以上の処理を入れてください。

もしくは、この方法はポリゴンを1つも描画していないときは使えない方法なのですが、

void VBlankIntr(void)
{
    if ( (G3X_GetVtxListRamCount() == 0) && swap )
    {
        if (flag)  // flipの切り替え
        {
            setupFrame2N_1();
        }
        else
        {
            setupFrame2N();
        }

上のように修正しても問題を防ぐことができます。
この方法はスワップバッファ処理がVBlank開始時に実行されたとき
頂点RAMに格納されている頂点数が 0 になることを
G3X_GetVtxListRamCount関数(頂点RAMに格納されている頂点の数を返す関数) で確認することによって
スワップバッファ処理が実行されたという判定をしています。

注意：もし、非常に長い割り込み処理をVブランクの割り込み前に発生せて、
      Vブランクの開始が遅れ、Vブランクの時間が非常に短く(約3ライン以下)なってしまった場合
      表示中に上下画面の切り替えが行われたり、そのフレームだけ前の画像が表示されることがあります。

---------------------------------------------------------------------------------------------------
*/


