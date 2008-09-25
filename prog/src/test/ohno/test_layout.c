//============================================================================================
/**
 * @file	test_layout.c
 * @brief	デバッグ用関数
 * @author	layout
 * @date	2008.08.27
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include <textprint.h>

#include "system/main.h"
#include "arc_def.h"

#include "../goto/comm_error.h"
#include "layout.naix"



typedef struct{
    int nscr;
    int ncgr;
    int nclr;
    int frame;
    int priority;
    int paletteBit;
} LAYOUTEDIT_BODY;



#include "layoutediter.c"



typedef struct
{
	int				heapID;	
	GFL_BMPWIN*		win;
	GFL_BMP_DATA*	bmp;
    int layoutno;

} LAYOUT_WORK;


//----------------------------------------------------------------------------
/**
 *	@brief	アサートから抜ける
 *
 *	L+R+X+Y
 */
//-----------------------------------------------------------------------------
static void GFUser_AssertDispFinishFunc( void )
{
	int key_cont;

	
	while(1){

		GFL_UI_Main();
		key_cont = GFL_UI_KEY_GetCont();
		// キーが押されるまでループ
		if( (key_cont & PAD_BUTTON_L) && (key_cont & PAD_BUTTON_R) &&
			(key_cont & PAD_BUTTON_X) && (key_cont & PAD_BUTTON_Y) ){
			return ;
		}

		// VBlank待ち
		// (これを有効にすると、デバッカで停止させたときにコールスタックが表示されない)
//		OS_WaitIrq(TRUE,OS_IE_V_BLANK);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	レイアウトデータに従い表示
 */
//-----------------------------------------------------------------------------

static void GFL_Layout_Disp(int no, LAYOUTEDIT_BODY* pBody, int* pHead,int heapID)
{
    int start = pHead[no];
    int end = pHead[no+1];
    int i,p,pal,startpal,endpal;
    int sizeOffset,addSize;
    int mainSizeOffset = 0,subSizeOffset = 0;

    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );

    for(i=start;i < end;i++){
        startpal = endpal = -1;
        if(pBody[i].frame < GFL_BG_FRAME0_S){
            sizeOffset = mainSizeOffset;
            pal = PALTYPE_MAIN_BG;
        }
        else{
            sizeOffset = subSizeOffset;
            pal = PALTYPE_SUB_BG;
        }
        addSize = GFL_ARC_UTIL_TransVramBgCharacter( ARCID_LAYOUT, pBody[i].ncgr , pBody[i].frame, 0, 0, 0, heapID );

        for(p = 0 ; p < 16 ; p++){
            if( pBody[i].paletteBit & (0x1<<p) ){
                OS_TPrintf("pal %d %d %d\n",i,startpal,endpal);
                if(startpal==-1){
                    startpal = p * 32;
                }
                if(endpal < p * 32){
                    endpal =  p * 32;
                }
            }
        }
        GFL_ARC_UTIL_TransVramPaletteEx( ARCID_LAYOUT, pBody[i].nclr , pal, startpal, startpal, endpal - startpal + 32, heapID );
        GFL_ARC_UTIL_TransVramScreen( ARCID_LAYOUT, pBody[i].nscr, pBody[i].frame, sizeOffset, 0, 0, heapID );

        if(pBody[i].frame < GFL_BG_FRAME0_S){
            mainSizeOffset += addSize;
        }
        else{
            subSizeOffset += addSize;
        }
        GFL_BG_SetPriority(pBody[i].frame,  pBody[i].priority);
		GFL_BG_SetVisible( pBody[i].frame,   VISIBLE_ON );
    }
}

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugLayoutMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	LAYOUT_WORK* wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_LAYOUT, 0x10000 );
	wk = GFL_PROC_AllocWork( proc, sizeof( LAYOUT_WORK ), HEAPID_LAYOUT );
	wk->heapID = HEAPID_LAYOUT;
    wk->layoutno = 0;
		
	{
		static const GFL_BG_DISPVRAM dispvramBank = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_0123_E			// テクスチャパレットスロット			
		};		
		GFL_DISP_SetBank( &dispvramBank );
	}	
	
	G2_BlendNone();
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );
		
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}
		
	{
		static const GFL_BG_BGCNT_HEADER bg0cntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		static const GFL_BG_BGCNT_HEADER bg1cntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bg0cntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M,   &bg1cntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bg0cntText,   GFL_BG_MODE_TEXT );

		///< main

		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0x00, 1, 0 );


        //	GFL_ARC_UTIL_TransVramBgCharacter( ARCID_LAYOUT, NARC_C__home_layout_layout_kaseki_board_NCGR, GFL_BG_FRAME0_M, 0, 0, 0, wk->heapID );
///		GFL_ARC_UTIL_TransVramPalette( ARCID_LAYOUT, NARC_C__home_layout_layout_kaseki_board_NCLR, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
///		GFL_ARC_UTIL_TransVramScreen( ARCID_LAYOUT, NARC_C__home_layout_layout_kaseki_board1_NSCR, GFL_BG_FRAME0_M, 0, 0, 0, wk->heapID );
//		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
//		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		///< sub
		
		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
//		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
//		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
        GFL_Layout_Disp(wk->layoutno,LayoutEditorTable,LayoutEditorTableNum, wk->heapID);

    }		
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );

	CommErrorSys_Setup();

    // アサート回避関数の設定
    GFL_ASSERT_SetDisplayFunc( NULL, NULL, GFUser_AssertDispFinishFunc );


	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugLayoutMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	LAYOUT_WORK* wk = mywk;
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
	{	
		return GFL_PROC_RES_FINISH;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
	{
        
        
        GFL_Layout_Disp(wk->layoutno,LayoutEditorTable,LayoutEditorTableNum,wk->heapID);
        {
            int num = sizeof(LayoutEditorTableNum)/sizeof(int)-1;
            wk->layoutno++;
            if(wk->layoutno >= num){
                wk->layoutno = 0;
            }
        }
        ///< main
//		CommErrorSys_Init( GFL_BG_FRAME0_M );
//		CommErrorSys_Call();
	}
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{
		///< sub 
		CommErrorSys_Init( GFL_BG_FRAME0_S );
		CommErrorSys_Call();
	}
	
	if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_INC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_DEC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_X_INC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_X_DEC, 1 );
	}
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{
	//	CommErrorSys_Init( GFL_BG_FRAME0_M );	///< 未初期化テスト
		CommErrorSys_Call();
	}

	return GFL_PROC_RES_CONTINUE;	
}
//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugLayoutMainProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	LAYOUT_WORK* wk = mywk;

	GFL_PROC_FreeWork( proc );
	
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();
	
	GFL_HEAP_DeleteHeap( HEAPID_LAYOUT );
	
	OS_TPrintf( "proc Exit\n" );

	return GFL_PROC_RES_FINISH;
}




//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA		DebugLayoutMainProcData = {
	DebugLayoutMainProcInit,
	DebugLayoutMainProcMain,
	DebugLayoutMainProcExit,
};

 






