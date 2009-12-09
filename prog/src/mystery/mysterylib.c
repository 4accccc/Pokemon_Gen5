//============================================================================================
/**
 * @file	mysterylib.c	
 * @bfief	ふしぎな　おくりもので使えるライブラリもどき
 * @author	Satoshi Mitsuhara
 * @date	06.06.12
 *
 * $Id: mysterylib.c,v 1.41 2006/08/10 09:56:15 mitsuhara Exp $
 */
//============================================================================================
#include <gflib.h>
#include "arc_def.h"

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"
#include "system/wipe.h"

#include "poke_tool/monsno_def.h"
#include "item/item.h"
#include "item/itemsym.h"

#include "savedata/mystery_data.h"

#include "mysterylib.h"
#include "mystery.naix"
//#include "../application/tradelist/tradelist.naix"

#include "nitrocrypto/crypto.h"


#define STACK_SIZE		1024

typedef struct {
	GFL_BG_INI *bgl;
	BOOL init_flag;

	int heapid;
	int next_seq;
	int fade_type;

	int save_seq;
	SAVE_CONTROL_WORK *sv;
	void *time_icon;
	int icon_base;
	
	BOOL init_clact_flag[4];
	// 演出用セルアクター
	MYSTERY_CLACT clact;
	GFL_CLWK* giftact;
	int giftact_wait;
	fx32 sub_add;
	
	u8 PokeGra[0x20*10*10];
//	SOFT_SPRITE_ARC PokeGraSsa;

	void (*vfunc)(void *);
	GFL_TCB*	vblankFuncTcb;

	//パレットのVBlank読み込みで、読み込みは普通で、転送をblankタイミングで
	void	*pltRes;	
	NNSG2dPaletteData *pltData;
	
	BOOL	isInitClact;
	
} MYSTERYLIB_WORK;


#define	MYSTERYLIB_BLACK	(GF_PRINTCOLOR_MAKE( 1, 2, 15))
#define	MYSTERYLIB_WHITE	(GF_PRINTCOLOR_MAKE(15, 2, 0))
#define	MYSTERYLIB_BLUE		(GF_PRINTCOLOR_MAKE(7, 8, 15))
#define MYSTERYLIB_CLRCODE	15

#define MYSTERYLIB_NOMSG	-1

#define SUBSURFACEX		(  0*FX32_ONE)
#define SUBSURFACEY		(256*FX32_ONE)

static MYSTERYLIB_WORK MysteryLibWork;

#define GetMysteryLibWorkPtr()	&MysteryLibWork

static void MysteryLib_VBlankFunc(GFL_TCB *tcb,void *work);

//------------------------------------------------------------------
/**
 * @brief	ふしぎな　おくりものライブラリ初期化
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_Init(int heapid)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	//  if(wk->init_flag == TRUE)	return;

	GFL_STD_MemFill(wk, 0, sizeof(MYSTERYLIB_WORK));
	wk->init_flag = TRUE;
	wk->heapid = heapid;
	wk->isInitClact = FALSE;
	wk->vblankFuncTcb = NULL;
}


#if 0
#pragma mark [>end edit

//------------------------------------------------------------------
/**
 * @brief	PROC初期化をまとめた関数
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void* MysteryLib_InitProc(PROC *proc, int heapid, int size, int heapsize)
{
	void *wk;

	sys_CreateHeap(HEAPID_BASE_APP, heapid, heapsize);
	sys_PrintHeapFreeSize(heapid);
	wk = PROC_AllocWork(proc, size, heapid);
	// 初期化不良が怖いのでワークはゼロクリア
	memset(wk, 0, size);
	/* 画面の初期設定 */
	WIPE_SetBrightness(WIPE_DISP_MAIN, WIPE_FADE_BLACK);
	WIPE_SetBrightness(WIPE_DISP_SUB,  WIPE_FADE_BLACK);
	return wk;
}


//------------------------------------------------------------------
/**
 * @brief	ＢＧ初期化関数(１フレームのみ)
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_BgInitFrame(GF_BGL_INI *ini, int frame, u8 size, u32 scrbase, u32 scrchar)
{
	GF_BGL_BGCNT_HEADER MBg_Data = {
		0, 0, 0x800, 0,
		GF_BGL_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x00000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};
	MBg_Data.screenSize = size;
	switch(size){
	case GF_BGL_SCRSIZ_256x256: MBg_Data.scrbufferSiz = 0x0800; break;
	case GF_BGL_SCRSIZ_256x512: MBg_Data.scrbufferSiz = 0x1000; break;
	case GF_BGL_SCRSIZ_512x256: MBg_Data.scrbufferSiz = 0x1000; break;
	case GF_BGL_SCRSIZ_512x512: MBg_Data.scrbufferSiz = 0x2000; break;
	}
	MBg_Data.screenBase = scrbase / 0x0800;
	MBg_Data.charBase = scrchar / 0x4000;
	GF_BGL_BGControlSet(ini, frame, &MBg_Data, GF_BGL_MODE_TEXT );
	GF_BGL_ScrClear(ini, frame);
}


#endif
#pragma mark [>start edit

//------------------------------------------------------------------
/**
 * @brief	フェードのタイプ変更
 * @param	0: BLACK  1: WHITE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_ChangeFadeType(int type)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	wk->fade_type = type;
}

//------------------------------------------------------------------
/**
 * @brief	フェード関数
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_RequestFade(int type, int next_seq, int *seq, int next)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	if(wk->fade_type == 0)
		WIPE_SYS_Start(WIPE_PATTERN_WMS, type, type, WIPE_FADE_BLACK,WIPE_DEF_DIV,WIPE_DEF_SYNC, wk->heapid);
	else
		WIPE_SYS_Start(WIPE_PATTERN_WMS, type, type, WIPE_FADE_WHITE,WIPE_DEF_DIV,WIPE_DEF_SYNC, wk->heapid);
	if(seq)
		*seq = next;
	wk->next_seq = next_seq;
}


//------------------------------------------------------------------
/**
 * @brief	フェード終了関数
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_FadeEndCheck(int *seq)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	if(WIPE_SYS_EndCheck()){
		*seq = wk->next_seq;
	}
}


#if 0
#pragma mark [>end edit

//------------------------------------------------------------------
/**
 * @brief	MYSTERY_WIN構造体の初期化
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_WinInit1(MYSTERY_WIN *mw, GF_BGL_BMPWIN *win, int palno, int arc, int fchr, int fcol)
{
	// 一度設定したらあまり変更のないものを最初に設定
	memset(mw, 0, sizeof(MYSTERY_WIN));
	mw->drawflag = TRUE;
	mw->redrawflag = TRUE;
	mw->msgid = MYSTERYLIB_NOMSG;
	mw->win = win;
	mw->palno = palno;
	mw->arcno = arc;
	mw->fchr = fchr;
	mw->fcol = fcol;
	mw->screen = GF_BGL_FRAME0_M;
	mw->font = FONT_SYSTEM;
	mw->color = MYSTERYLIB_BLACK;
	mw->clrcolor = MYSTERYLIB_CLRCODE;
	mw->winmode = MYSTERYLIB_WINTYPE_NORMAL;
	mw->dx = mw->dy = 0;
	mw->word = NULL;
	mw->msgspd = MSG_NO_PUT;
}
void MysteryLib_WinInit2(MYSTERY_WIN *mw, int w, int h,/* int msgid,*/ int base)
{
	// ウィンドウを生成するたびに変更がありそうなもの
	mw->width = w;
	mw->height = h;
	mw->base = base;
}
void MysteryLib_WinInit3(MYSTERY_WIN *mw, int type, int font)
{
	mw->winmode = type;
	mw->font = font;
}
void MysteryLib_WinSetColor(MYSTERY_WIN *mw, int color)
{
	if(color != MYSTERYLIB_COLOR_NULL){
		switch(color){
		case MYSTERYLIB_COLOR_WHITE: mw->color = MYSTERYLIB_WHITE; break;
		case MYSTERYLIB_COLOR_BLACK: mw->color = MYSTERYLIB_BLACK; break;
		case MYSTERYLIB_COLOR_BLUE:  mw->color = MYSTERYLIB_BLUE;  break;
		}
	}
}
void MysteryLib_WinSetPos(MYSTERY_WIN *mw, int dx, int dy)
{
	mw->dx = dx;
	mw->dy = dy;
}
	
//------------------------------------------------------------------
/**
 * @brief	メッセージウィンドウを登録
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
static int MysteryLib_RedrawWin(MYSTERY_WIN *mw, int msgid)
{
	int midx;
	STRBUF * msg;
	WORDSET *word;
	MSGDATA_MANAGER *msgman;

	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	// ウィンドウをクリア
	if(msgid != MYSTERYLIB_NOMSG && mw->msgid != msgid){
		mw->msgid = msgid;
		if(mw->redrawflag == TRUE)
			GF_BGL_BmpWinDataFill(mw->win, mw->clrcolor);
		if(mw->msgid != MYSTERYLIB_NOMSG){
			// アーカイブから展開して表示
			msgman = MSGMAN_Create(MSGMAN_TYPE_DIRECT, ARC_MSG, mw->arcno, wk->heapid);
			if(mw->word == NULL)	word = WORDSET_Create(wk->heapid);
			else			word = mw->word;
			msg = MSGDAT_UTIL_AllocExpandString(word, msgman, mw->msgid, wk->heapid);
			if(mw->rightflag == FALSE){
	midx = GF_STR_PrintColor(mw->win, mw->font, msg, mw->dx, mw->dy, mw->msgspd, mw->color, NULL);
			} else {
	// 右寄せして表示
	int width, wsize;
	width = FontProc_GetPrintStrWidth(mw->font, msg, FontHeaderGet(mw->font, FONT_HEADER_SPACE_X));
	wsize = GF_BGL_BmpWinGet_SizeX(mw->win) * 8 - width;
	midx = GF_STR_PrintColor(mw->win, mw->font, msg, wsize, mw->dy, mw->msgspd, mw->color, NULL);
	mw->rightflag = FALSE;
			}
			STRBUF_Delete(msg);
			if(mw->word == NULL)	WORDSET_Delete(word);
			MSGMAN_Delete(msgman);
		}
	}
	mw->msgspd = MSG_NO_PUT;
	return midx;
}
int MysteryLib_CreateWin(GF_BGL_INI *bgl, MYSTERY_WIN *mw, int sx, int sy, int msgid)
{
	int midx;
	if(mw->win->ini == NULL){
		GF_BGL_BmpWinAdd(bgl, mw->win, mw->screen, sx, sy, mw->width, mw->height, mw->palno, mw->base);
		midx = MysteryLib_RedrawWin(mw, msgid);
	} else {
		// 表示座標を変更する
		if(sx != -1)	GF_BGL_BmpWinSet_PosX(mw->win, sx);
		if(sy != -1)	GF_BGL_BmpWinSet_PosY(mw->win, sy);
		midx = MysteryLib_RedrawWin(mw, msgid);
	}
	// 転送フラグがTRUEならばＶＲＡＭへ転送する
	if(mw->drawflag == TRUE){
		if(mw->winmode == MYSTERYLIB_WINTYPE_NORMAL)
			BmpMenuWinWrite(mw->win, WINDOW_TRANS_ON, mw->fchr, mw->fcol);
		else
			BmpTalkWinWrite(mw->win, WINDOW_TRANS_ON, mw->fchr, mw->fcol);
	}
	return midx;
}

//------------------------------------------------------------------
/**
 * @brief	ウィンドウが消費するキャラクタ総数を返す
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
int MysteryLib_GetWindowChr(MYSTERY_WIN *mw)
{
	return mw->width * mw->height;
}

//------------------------------------------------------------------

#endif
#pragma mark [>start edit

//------------------------------------------------------------------
/**
 * @brief	キャラクタ、パレットマネージャー初期化
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_InitCPManager(void)
{
}


//------------------------------------------------------------------
/**
 * @brief	セルアクターシステムが初期化されてるか返す
 * @param	NONE
 * @return	TRUE: 初期化済み　FALSE:初期化してない
 */
//------------------------------------------------------------------
BOOL MysteryLib_isInitClact(void)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	return wk->isInitClact;
}

//------------------------------------------------------------------
/**
 * @brief	セルアクターシステムの初期化
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_InitClactSystem(void)
{
	const GFL_DISP_VRAM vramSetTable = {
		GX_VRAM_BG_128_C,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_32_H,			/* サブ2DエンジンのBG */
		GX_VRAM_SUB_BGEXTPLTT_NONE,		/* サブ2DエンジンのBG拡張パレット */
		GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_32K,		// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
	};
	const u8 CELL_MAX = 16;
	int i;
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();

	
	GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
	cellSysInitData.oamst_main = 4; //通信アイコンの分(4の倍数！
	cellSysInitData.oamnum_main = 64-4;
	cellSysInitData.oamst_sub = 16; 
	cellSysInitData.oamnum_sub = 128-16;
	
	GFL_CLACT_SYS_Create( &cellSysInitData , &vramSetTable, wk->heapid );
	wk->clact.cellUnit  = GFL_CLACT_UNIT_Create( CELL_MAX , 0,wk->heapid );
	GFL_CLACT_UNIT_SetDefaultRend( wk->clact.cellUnit );
	
	//リソースに空インデックス設定
	for( i=0;i<2;i++ )
	{
		wk->clact.resCell[i].pltIdx = GFL_CLGRP_REGISTER_FAILED;
		wk->clact.resCell[i].ncgIdx = GFL_CLGRP_REGISTER_FAILED;
		wk->clact.resCell[i].anmIdx = GFL_CLGRP_REGISTER_FAILED;
	}

	wk->isInitClact = TRUE;
}


//--------------------------------------------------------------
/**
 * @brief	
 *
 * @param	none	
 *
 * @retval	MYSTERY_CLACT*	
 *
 */
//--------------------------------------------------------------
MYSTERY_CLACT* MysteryLib_GetClactSystem( void )
{
	MYSTERYLIB_WORK* wk = GetMysteryLibWorkPtr();
	
	GF_ASSERT( wk != NULL );
	
	GF_ASSERT( wk->isInitClact == TRUE );
	
	return &wk->clact;
}


//------------------------------------------------------------------
/**
 * @brief	セルアクター初期化
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_InitClact(int arcfile, int arcchar, int arcpal, int arccell, int arcanim, int screen)
{
	
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	int hwscreen = (screen == GFL_BG_MAIN_DISP ? CLSYS_DRAW_MAIN : CLSYS_DRAW_SUB);
	int comp;
	ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( arcfile , wk->heapid );

	//  OS_TPrintf("%d, %d, %d, %d, %d, %d\n", arcfile, arcchar, arcpal, arccell, arcanim, screen);

	comp = 0;
	// ごめんなさい、ITEMICONだけは特別扱い
	if(arcfile == ARCID_ITEMICON)	comp = 0;
	
	//chara読み込み
	if(arcchar != -1)
	{
		wk->clact.resCell[screen].ncgIdx = GFL_CLGRP_CGR_Register( p_handle , arcchar , FALSE , hwscreen , wk->heapid );
	}
	//pal読み込み
	if(arcpal != -1)
	{
		wk->clact.resCell[screen].pltIdx = GFL_CLGRP_PLTT_Register( p_handle , arcpal , hwscreen , 0 , wk->heapid );
	}
	//cell読み込み
	if(arccell != -1 && arcanim != -1 )
	{
		wk->clact.resCell[screen].anmIdx = GFL_CLGRP_CELLANIM_Register( p_handle , arccell , arcanim, wk->heapid );
	}

//	//まだ何も転送していないから
	if(screen == GFL_BG_MAIN_DISP)
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );	//メイン画面OBJ面ＯＮ
	else
		GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON );	//サブ画面OBJ面ＯＮ

//	sys_VBlankFuncChange(MysteryLib_VBlankFunc, NULL);
	wk->vblankFuncTcb = GFUser_VIntr_CreateTCB( MysteryLib_VBlankFunc , (void*)wk , 0 );

	GFL_ARC_CloseDataHandle( p_handle );
	
}

#if 0
#pragma mark [>end edit


//--------------------------------------------------------------
/**
 * @brief	VBlank
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void MysteryLib_SetVBlank( void )
{
	sys_VBlankFuncChange( MysteryLib_VBlankFunc, NULL );
}

#endif
#pragma mark [>start edit

//------------------------------------------------------------------
/**
 * @brief	サブ画面のサーフェイス変更
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_SetSubSurfaceMatrix(fx32 x, fx32 y)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();

//	CLACT_U_SetSubSurfaceMatrix(&wk->clact.renddata, x, y);
	wk->sub_add = y;
}


//------------------------------------------------------------------
/**
 * @brief	毎フレーム呼び出して欲しい関数
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_DoClact( void )
{
	MysteryLib_DoClact_Ex( 0 );
}

void MysteryLib_DoClact_Ex( int state )
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();

	if( wk->giftact )
	{
		VecFx32 *vec;
		if( wk->giftact_wait == 0 )
		{
			if( GFL_CLACT_WK_GetDrawEnable( wk->giftact ) == FALSE && state == 0 ){
				GFL_CLACT_WK_SetDrawEnable( wk->giftact, 1 );
			}
			//FIXME 処理の意味は？
//			vec = ( VecFx32 * )CLACT_GetMatrix( wk->giftact );
//			
//			if( vec->y < 384*FX32_ONE ){
//				vec->y += FX32_ONE * 3;
//			}
		}
		else
		{
			wk->giftact_wait--;
		}
	}
	if( wk->clact.cellUnit != NULL )
	{
		//OBJの更新
		GFL_CLACT_SYS_Main();
	}
}

//------------------------------------------------------------------
/**
 * @brief	CLACT_ADDの雛形を作成
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
GFL_CLWK* MysteryLib_MakeCLACT(int screen, GFL_CLWK *anim, int sx, int sy, int anum)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	GFL_CLWK *cellWork;

	if( anim == NULL )
	{
		GFL_CLWK_DATA	cellInitData;
		cellInitData.pos_x = sx;
		cellInitData.pos_y = sy;
		cellInitData.anmseq = anum;
		cellInitData.softpri = 0;
		cellInitData.bgpri = 0;
		cellWork = GFL_CLACT_WK_Create( wk->clact.cellUnit , wk->clact.resCell[screen].ncgIdx , 
										wk->clact.resCell[screen].pltIdx , wk->clact.resCell[screen].anmIdx , 
										&cellInitData , (screen==GFL_BG_MAIN_DISP?CLSYS_DEFREND_MAIN:CLSYS_DEFREND_SUB) ,
										wk->heapid );
	}
	else
	{
		cellWork = anim;
	}

	GFL_CLACT_WK_SetAutoAnmFlag(cellWork, 1);
	GFL_CLACT_WK_SetAnmSeq(cellWork, anum);
	GFL_CLACT_WK_SetDrawEnable(cellWork, 1);

	return cellWork;
}

//------------------------------------------------------------------
/**
 * @brief	セルアクターの削除
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_RemoveClact(void)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	MYSTERY_CLACT *act = &wk->clact;
	int i;

	// これだけ特別なactなのでここで消去
	if(wk->giftact){
		GFL_CLACT_WK_Remove(wk->giftact);
		wk->giftact = NULL;
	}
	for( i=0;i<2;i++ )
	{
		if( act->resCell[i].pltIdx != GFL_CLGRP_REGISTER_FAILED )
		{
			GFL_CLGRP_PLTT_Release( act->resCell[i].pltIdx );
			act->resCell[i].pltIdx = GFL_CLGRP_REGISTER_FAILED;
		}
		if( act->resCell[i].ncgIdx != GFL_CLGRP_REGISTER_FAILED )
		{
			GFL_CLGRP_CGR_Release( act->resCell[i].ncgIdx );
			act->resCell[i].ncgIdx = GFL_CLGRP_REGISTER_FAILED;
		}
		if( act->resCell[i].anmIdx != GFL_CLGRP_REGISTER_FAILED )
		{
			GFL_CLGRP_CELLANIM_Release( act->resCell[i].anmIdx );
			act->resCell[i].anmIdx = GFL_CLGRP_REGISTER_FAILED;
		}
	}

	GFL_CLACT_UNIT_Delete( wk->clact.cellUnit );
	GFL_CLACT_SYS_Delete();
	
	if( wk->vblankFuncTcb != NULL )
	{
		GFL_TCB_DeleteTask( wk->vblankFuncTcb );
	}
	
	

/* old
	// セルアクターセット破棄
	CLACT_DestSet(act->clactSet);
	act->clactSet = NULL;
	//OAMレンダラー破棄
	REND_OAM_Delete();
	// リソース解放
	DeleteCharManager();
	DeletePlttManager();

	sys_VBlankFuncChange( NULL, NULL );
	
	GFL_CLACT_UNIT_Delete( cellUnit );
	
*/}

#if 0
#pragma mark [>end edit

//------------------------------------------------------------------
/**
 * @brief	ゲームカセットに対応したビットを返す
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
u32 MysteryLib_GetCasetteCode(int cas)
{
	return (u32)1 << (cas - VERSION_DIAMOND);
}

//------------------------------------------------------------------

#pragma mark [start Edit
#endif 



//------------------------------------------------------------------
/**
 * @brief	VBLank Function
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
static void MysteryLib_VBlankFunc(GFL_TCB *tcb,void *work)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();

	if(wk->vfunc){
		wk->vfunc(wk);
		wk->vfunc = NULL;
	}
	
	//多分いらない
	/*
	// レンダラ共有OAMマネージャVram転送
	REND_OAMTrans();
		if(wk->bgl)
				GF_BGL_VBlankFunc( wk->bgl );
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
	*/
	GFL_CLACT_SYS_VBlankFunc();
#if 0
	// 必要があればThreadをたたき起こす
	if(OS_IsThreadTerminated(&wk->thread) == FALSE)
		OS_WakeupThreadDirect(&wk->thread);
#endif
}

//------------------------------------------------------------------
/**
 * @brief	おくりものに対応したパレット番号を返す
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
static int MysteryLib_GetGiftPalno(int gift)
{
	int i;
	struct {
		u8 type, palno;
	} GiftPalno[] = {
		{ MYSTERYGIFT_TYPE_NONE,		0 },
		{ MYSTERYGIFT_TYPE_POKEMON,		1 },	// ポケモン
		{ MYSTERYGIFT_TYPE_POKEEGG,		1 },	// タマゴ
		{ MYSTERYGIFT_TYPE_ITEM,		2 },	// どうぐ
		{ MYSTERYGIFT_TYPE_RULE,		3 },	// ルール
		{ MYSTERYGIFT_TYPE_GOODS,		2 },	// グッズ
		{ MYSTERYGIFT_TYPE_ACCESSORY,	2 },	// アクセサリ
		{ MYSTERYGIFT_TYPE_RANGEREGG,	0 },	// ナマフィーのタマゴ
		{ MYSTERYGIFT_TYPE_MEMBERSCARD,	5 },	// メンバーズカード
		{ MYSTERYGIFT_TYPE_LETTER,		5 },	// オーキドのてがみ
		{ MYSTERYGIFT_TYPE_WHISTLE, 	5 },	// てんかいのふえ
		{ MYSTERYGIFT_TYPE_POKETCH, 	4 },	// ポケッチ
		{ MYSTERYGIFT_TYPE_SECRET_KEY, 	5 },	// 秘密の鍵
		{ MYSTERYGIFT_TYPE_MOVIE,	 	5 },	// 映画配布
		{ -1 },
	};
	
	for(i = 0; GiftPalno[i].type != -1; i++)
		if(GiftPalno[i].type == gift)
			return GiftPalno[i].palno;
	return 0;
}

#pragma mark [end Edit
#if 0 


//------------------------------------------------------------------
/**
 * @brief	指定のポケモングラフィックスをロード
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
#define POKEGRA_VRAM_OFFSET  ( 0 )
#define POKEGRA_VRAM_SIZE     ( 0x20*10*10 )
static void MysteryLib_LoadPokeGra(CLACT_WORK_PTR clwk, POKEMON_PARAM *pp, int mons_no, int form_no, u8 *char_work, SOFT_SPRITE_ARC *ssa)
{
	int sex, col;
	u32 rnd;
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
#if 0//def DEBUG_ONLY_FOR_mituhara
	OS_TPrintf("フォルム番号は %d です\n", form_no);
#endif
	
	// ポケモンの画像を読み込む（ただしソフトウェアスプライト用のテクスチャ状態）
	sex = PokeSexGet(pp);
	col = PokeRareGet(pp);
	PokeGraArcDataGet(ssa, mons_no, sex, PARA_FRONT, col, form_no, 0);

	// テクスチャをOAM用に変換する(パッチールのぶちも考慮)
	rnd = PokeParaGet( pp, ID_PARA_personal_rnd, NULL );
	Ex_ChangesInto_OAM_from_PokeTex(ssa->arc_no, ssa->index_chr, wk->heapid, 0, 0, 10, 10,
					char_work, rnd, 0, PARA_FRONT, mons_no);

	// OAM用VRAMに転送
	DC_FlushRange(char_work, 0x20*10*10);
	{
	NNSG2dImageProxy* plo = CLACT_ImageProxyGet( clwk );
	u32 addrs = NNS_G2dGetImageLocation( plo, NNS_G2D_VRAM_TYPE_2DSUB );

	GXS_LoadOBJ(char_work, addrs + 1*POKEGRA_VRAM_SIZE, POKEGRA_VRAM_SIZE);
//	GXS_LoadOBJ(char_work, POKEGRA_VRAM_OFFSET + 1*POKEGRA_VRAM_SIZE, POKEGRA_VRAM_SIZE);
	}

	// パレット転送
	{
	NNSG2dImagePaletteProxy* plo = CLACT_PaletteProxyGet( clwk );
	u32 addrs = NNS_G2dGetImagePaletteLocation( plo, NNS_G2D_VRAM_TYPE_2DSUB );
	
		ArcUtil_PalSet(ssa->arc_no, ssa->index_pal, PALTYPE_SUB_OBJ, 0x20*(2+1)+addrs, 32, wk->heapid);
//  ArcUtil_PalSet(ssa->arc_no, ssa->index_pal, PALTYPE_SUB_OBJ, 0x20*(2+1), 32, wk->heapid);
	}
}


#pragma mark [start Edit
#endif 

//------------------------------------------------------------------
/**
 * @brief	プレゼントボックス
 */
//------------------------------------------------------------------
static void MysteryLib_SetGraPresent(MYSTERYLIB_WORK *wk, int type, GIFT_DELIVERY *deli)
{
	MysteryLib_InitClact(ARCID_MYSTERY,
					 NARC_mystery_fushigi_box_NCGR,
					 NARC_mystery_fushigi_box_NCLR,
					 NARC_mystery_fushigi_box_NCER,
					 NARC_mystery_fushigi_box_NANR,
					 GFL_BG_SUB_DISP);
	MysteryLib_SetSubSurfaceMatrix(SUBSURFACEX, SUBSURFACEY);
	wk->giftact = MysteryLib_MakeCLACT(GFL_BG_SUB_DISP, wk->giftact, HW_LCD_WIDTH/2, 0, 0);
}

#pragma mark [end Edit
#if 0 


//------------------------------------------------------------------
/**
 * @brief	ポケモン
 */
//------------------------------------------------------------------
static void MysteryLib_SetGraPoke(MYSTERYLIB_WORK *wk, int type, GIFT_DELIVERY *deli)
{
	POKEMON_PARAM *pp;

	OS_TPrintf("parentType = %d\n", deli->data.pokemon.parentType);
	
	MysteryLib_InitClact(ARC_TRADELIST_GRA,
					 NARC_TradeList_obj_s_lz_ncgr,
					 NARC_TradeList_DsTradeList_nclr,
					 NARC_TradeList_obj_s_lz_ncer,
					 NARC_TradeList_obj_s_lz_nanr,
					 GF_BGL_SUB_DISP);
	MysteryLib_SetSubSurfaceMatrix(SUBSURFACEX, SUBSURFACEY);
	wk->giftact = MysteryLib_MakeCLACT(GF_BGL_SUB_DISP, wk->giftact, HW_LCD_WIDTH/2, 0, 1);
	pp = (POKEMON_PARAM *)&deli->data.pokemon.data;
	switch(type){
	case MYSTERYGIFT_TYPE_POKEMON:	// ポケモン
	case MYSTERYGIFT_TYPE_MOVIE:		// 映画配布
		MysteryLib_LoadPokeGra(wk->giftact, pp, PokeParaGet(pp,ID_PARA_monsno,0), PokeParaGet(pp,ID_PARA_form_no,0), wk->PokeGra, &wk->PokeGraSsa);
		break;
	case MYSTERYGIFT_TYPE_POKEEGG:	// タマゴ
		MysteryLib_LoadPokeGra(wk->giftact, pp, MONSNO_TAMAGO, 0, wk->PokeGra, &wk->PokeGraSsa);
		break;
	case MYSTERYGIFT_TYPE_RANGEREGG:	// ナマフィーのタマゴ
		MysteryLib_LoadPokeGra(wk->giftact, pp, MONSNO_TAMAGO, 1, wk->PokeGra, &wk->PokeGraSsa);
		break;
	}
}

//------------------------------------------------------------------
/**
 * @brief	アイテム
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
static void MysteryLib_SetGraItem(MYSTERYLIB_WORK *wk, int type, GIFT_DELIVERY *deli)
{
	int item;

	switch(type){
	case MYSTERYGIFT_TYPE_ITEM:		// どうぐ
		item = deli->data.item.itemNo;
		break;
	case MYSTERYGIFT_TYPE_MEMBERSCARD:	// メンバーズカード
		item = ITEM_MENBAAZUKAADO;
		break;
	case MYSTERYGIFT_TYPE_LETTER:		// オーキドのてがみ
		item = ITEM_OOKIDONOTEGAMI;
		break;
	case MYSTERYGIFT_TYPE_WHISTLE:	// てんかいのふえ
		item = ITEM_TENKAINOHUE;
		break;
	case MYSTERYGIFT_TYPE_SECRET_KEY:	//秘密の鍵
		item = ITEM_HIMITUNOKAGI;
		break;
	}
	MysteryLib_InitClact(ARC_ITEMICON,
					 GetItemIndex(item,ITEM_GET_ICON_CGX),
					 GetItemIndex(item,ITEM_GET_ICON_PAL),
					 ItemIconCellGet(),
					 ItemIconCAnmGet(),
					 GF_BGL_SUB_DISP);
	MysteryLib_SetSubSurfaceMatrix(SUBSURFACEX, SUBSURFACEY);
	wk->giftact = MysteryLib_MakeCLACT(GF_BGL_SUB_DISP, wk->giftact, HW_LCD_WIDTH/2, 0, 0);
}

#pragma mark [start Edit
#endif 


//------------------------------------------------------------------
/**
 * @brief	サブ画面のBGパレットを転送
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
static void MysteryLib_InitGiftPal( void *p)
{
	MYSTERYLIB_WORK *wk = (MYSTERYLIB_WORK *)p;
#if 0
	// サブ画面ＢＧパレット転送
	GFL_ARC_UTIL_TransVramPalette( ARCID_MYSTERY , NARC_mystery_fushigi_back_NCLR , PALTYPE_SUB_BG ,16*2*8,16*2*6,wk->heapid );
	//ArcUtil_PalSet(ARC_MYSTERY_GRA, NARC_mystery_fushigi_back_nclr, PALTYPE_SUB_BG, 16*2*8, 16*2*6, wk->heapid);
#endif
	//VBlankタイミングでパレットの読み込みができない(IRQ割り込み)
	//なので転送だけ行う
	GFL_BG_LoadPalette( GFL_BG_FRAME1_S , wk->pltData->pRawData , 16*2*6 , 16*2*8 );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
	
	GFL_HEAP_FreeMemory( wk->pltRes );
	wk->pltRes = NULL;
	wk->pltData = NULL;

}

//------------------------------------------------------------------
/**
 * @brief	下画面にプレゼント受け取り演出を表示
 *
 * SUB_BG0	格子模様(上書きされるから見えなくなってる)
 * SUB_BG1	背景ＢＧ
 * SUB_BG2,3	未使用
 * SUB_OAM	プレゼント表示
 * ※サブ画面のＢＧパレットを８～１３まで使用
*/
//------------------------------------------------------------------
void MysteryLib_InitGift(GFL_BG_INI *ini, GIFT_DELIVERY *deli)
{
	//FIXME パレット番号が無駄にかぶっているので、必要分だけ転送など、
	//		パレットがかぶらないようにしてチラツキを抑えること！
	
	
	int palno, type;
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();

	type = deli->gift_type;


#if 0
	//  type = MYSTERYGIFT_TYPE_POKEMON;
	type = MYSTERYGIFT_TYPE_ITEM;
	//  deli->data.pokemon.monsno = 151;
	//  deli->data.item.itemNo = ITEM_KODAWARIMEGANE;
#endif  

	palno = MysteryLib_GetGiftPalno(type);
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );


#if 0
	//VBlankタイミングでパレットの読み込みができない(IRQ割り込み)
	//なので転送だけ行う
	// サブ画面ＢＧパレット転送
	ArcUtil_PalSet(ARC_MYSTERY_GRA, NARC_mystery_fushigi_back_nclr, PALTYPE_SUB_BG, 16*2*8, 16*2*6, wk->heapid);
#endif
	wk->pltRes = GFL_ARC_UTIL_LoadPalette( ARCID_MYSTERY, NARC_mystery_fushigi_back_NCLR, &wk->pltData , wk->heapid );
	
	// サブ画面BG1キャラ転送
	GFL_ARC_UTIL_TransVramBgCharacter( ARCID_MYSTERY , NARC_mystery_fushigi_back_NCGR , 
				GFL_BG_FRAME1_S, 0, 10*16*0x20 , 0 ,wk->heapid );
//	ArcUtil_BgCharSet(ARC_MYSTERY_GRA, NARC_mystery_fushigi_back_lz_cngr, ini,
//				GF_BGL_FRAME1_S, 0, 10*16*0x20, 1, wk->heapid);
	// サブ画面スクリーン１
	//TODO とりあえず普通にやってみる
#if 1
	GFL_ARC_UTIL_TransVramScreen( ARCID_MYSTERY , NARC_mystery_fushigi_back_NSCR , GFL_BG_FRAME1_S , 0 , 32*24*2 , 0 , wk->heapid);
//	ArcUtil_ScrnSet(ARC_MYSTERY_GRA, NARC_mystery_fushigi_back_lz_cscr, ini, GF_BGL_FRAME1_S, 0, 32*24*2, 1, wk->heapid);
#else
	{
	 // ArcUtil_ScrnSetだと非垂直期間中にVRAM転送されてしまうので、修正
	 NNSG2dScreenData* scrnData;
	 void *arcData = ArcUtil_Load(ARC_MYSTERY_GRA, NARC_mystery_fushigi_back_lz_cscr, 1, wk->heapid, ALLOC_BOTTOM);
	 NNS_G2dGetUnpackedScreenData( arcData, &scrnData );
	 GF_BGL_ScreenBufSet(ini, GF_BGL_FRAME1_S, scrnData->rawData, 32*24*2 );
	 sys_FreeMemoryEz( arcData );
	}
#endif
	GFL_BG_ChangeScreenPalette( GFL_BG_FRAME1_S , 0,0,32,24,8 + palno );
//	GF_BGL_ScrPalChange(ini, GF_BGL_FRAME1_S, 0, 0, 32, 24, 8 + palno);
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S);
//	GF_BGL_LoadScreenV_Req(ini, GF_BGL_FRAME1_S);
	wk->vfunc = MysteryLib_InitGiftPal;
	wk->bgl=ini;

	
	// 上から降ってくるアクターの設定
	//FIXME とりあえず保留
	MysteryLib_SetGraPresent(wk, MYSTERYGIFT_TYPE_RULE, deli);
	switch(type){
	case MYSTERYGIFT_TYPE_RULE:		// ルール
	case MYSTERYGIFT_TYPE_GOODS:		// グッズ
	case MYSTERYGIFT_TYPE_ACCESSORY:	// アクセサリ
	case MYSTERYGIFT_TYPE_POKETCH:	// ポケッチ
//		MysteryLib_SetGraPresent(wk, type, deli);
		break;
	case MYSTERYGIFT_TYPE_RANGEREGG:	// ナマフィーのタマゴ
//		wk->giftact_wait = 120;
	case MYSTERYGIFT_TYPE_POKEMON:	// ポケモン
	case MYSTERYGIFT_TYPE_POKEEGG:	// タマゴ
	case MYSTERYGIFT_TYPE_MOVIE:		// 映画配布
//		MysteryLib_SetGraPoke(wk, type, deli);
		break;
	case MYSTERYGIFT_TYPE_ITEM:		// どうぐ
	case MYSTERYGIFT_TYPE_MEMBERSCARD:	// メンバーズカード
	case MYSTERYGIFT_TYPE_LETTER:		// オーキドのてがみ
	case MYSTERYGIFT_TYPE_WHISTLE:	// てんかいのふえ
	case MYSTERYGIFT_TYPE_SECRET_KEY:	// 秘密の鍵
//		MysteryLib_SetGraItem(wk, type, deli);
		break;
	}
	// 最初は非表示
//	GFL_CLACT_WK_SetDrawFlag(wk->giftact, 0);
}


//------------------------------------------------------------------
/**
 * @brief	指定のバッファを暗号化
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
#define SECRET_KEY	0xD679
void MysteryLib_CreateCryptoData(GIFT_COMM_PACK *gift_data, GIFT_DATA *data, int heapid)
{
	MATHCRC16Table *CrcTbl;
	CRYPTORC4Context *rc4context;
	u16 key[4];
	u16 crc, mask;
	int i;

#if 0//def DEBUG_ONLY_FOR_mituhara
	{
		u8 *p;
#if 0
		p = (u8 *)&gift_data->data;
		for(i = 0; i < 256; i++){
			if((i & 15) == 0)	OS_TPrintf("\n");
			OS_TPrintf("%02X ", p[i]);
		}
#else
		p = (u8 *)&gift_data->beacon;
		for(i = 0; i < 128; i++){
			if((i & 15) == 0)	OS_TPrintf("\n");
			OS_TPrintf("%02X ", p[i]);
		}
#endif
		OS_TPrintf("\n");
	}
#endif

	// ビーコン情報からCRC値を生成
	CrcTbl = GFL_HEAP_AllocMemory(heapid, sizeof(MATHCRC16Table));
	MATH_CRC16InitTable(CrcTbl);
	crc = MATH_CalcCRC16(CrcTbl, &gift_data->beacon, sizeof(GIFT_BEACON));
	GFL_HEAP_FreeMemory(CrcTbl);
	
	OS_GetMacAddress((u8*)key);
	key[3] = key[1];
	key[1] = crc;
	mask = SECRET_KEY;
	for(i = 0; i < 4; i++){
		key[i] = key[i] ^ mask;
		mask = key[i];
	}	

#if 0//def DEBUG_ONLY_FOR_mituhara
	OS_TPrintf("KEY: %04X %04X %04X %04X\n", key[0], key[1], key[2], key[3]);
#endif

	// MACアドレスをキーとして暗号化
	rc4context = GFL_HEAP_AllocMemory(heapid, sizeof(CRYPTORC4Context));
	CRYPTO_RC4Init(rc4context, key, 8 );
	CRYPTO_RC4Encrypt(rc4context, &gift_data->data, sizeof(GIFT_DATA), data);
	GFL_HEAP_FreeMemory(rc4context);
#if 0//def DEBUG_ONLY_FOR_mituhara
	{
		u8 *p;
		p = (u8 *)data;
		for(i = 0; i < 256; i++){
			if((i & 15) == 0)	OS_TPrintf("\n");
			OS_TPrintf("%02X ", p[i]);
		}
		OS_TPrintf("\n");
	}
#endif
}


//------------------------------------------------------------------
/**
 * @brief	暗号化されたバッファをDecode
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_DecodeCryptoData(GIFT_COMM_PACK *gift_data, GIFT_DATA *data, int heapid)
{
	MATHCRC16Table *CrcTbl;
	CRYPTORC4Context *rc4context;
	const WMBssDesc *bssDesc;
	u16 key[4]={0,0,0,0};
	u16 crc, mask;
	int i;

#if 0//def DEBUG_ONLY_FOR_mituhara
	{
		u8 *p;

		// 複合化する前のバイナリデータをDTCMに保存(やり方が無茶苦茶…)
		memcpy((void *)0x027E0100, gift_data, sizeof(GIFT_COMM_PACK));
		OS_TPrintf("複合化前のデータ: %08X - %08X\n", 0x027E0100, 0x027E0100 + sizeof(GIFT_COMM_PACK));

		p = (u8 *)&gift_data->beacon;
		for(i = 0; i < 128; i++){
			if((i & 15) == 0)	OS_TPrintf("\n");
			OS_TPrintf("%02X ", p[i]);
		}
		OS_TPrintf("\n\n\n");


		
		p = (u8 *)&gift_data->data;
		for(i = 0; i < 256; i++){
			if((i & 15) == 0)	OS_TPrintf("\n");
			OS_TPrintf("%02X ", p[i]);
		}
		OS_TPrintf("\n");
	}
#endif
	// ビーコン情報からCRC値を生成
	CrcTbl = GFL_HEAP_AllocMemory(heapid, sizeof(MATHCRC16Table));
	MATH_CRC16InitTable(CrcTbl);
	crc = MATH_CalcCRC16(CrcTbl, &gift_data->beacon, sizeof(GIFT_BEACON));
	GFL_HEAP_FreeMemory(CrcTbl);
	
	// 親のMACアドレスを得る
	//FIXME 親のMACアドレスを接続時保持しておくこと
	//bssDesc = CommMPGetWMBssDesc(0);
	//memcpy(key, bssDesc->bssid, WM_SIZE_BSSID);
	key[3] = key[1];
	key[1] = crc;
	mask = SECRET_KEY;
	for(i = 0; i < 4; i++){
		key[i] = key[i] ^ mask;
		mask = key[i];
	}	

#if 0//def DEBUG_ONLY_FOR_mituhara
	OS_TPrintf("KEY: %04X %04X %04X %04X\n", key[0], key[1], key[2], key[3]);
#endif

	// MACアドレスをキーとして複合化
	rc4context = GFL_HEAP_AllocMemory(heapid, sizeof(CRYPTORC4Context));
	CRYPTO_RC4Init(rc4context, key, 8 );
	CRYPTO_RC4Encrypt(rc4context, &gift_data->data, sizeof(GIFT_DATA), data);
	GFL_HEAP_FreeMemory(rc4context);

#if 0//def DEBUG_ONLY_FOR_mituhara
	{
		u8 *p;
		p = (u8 *)data;
		for(i = 0; i < 256; i++){
			if((i & 15) == 0)	OS_TPrintf("\n");
			OS_TPrintf("%02X ", p[i]);
		}
		OS_TPrintf("\n");
	}
#endif
}


//------------------------------------------------------------------
/**
 * @brief	メニューでライン移動があった際に呼ばれるコールバック
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_MenuKeyMove(BMPMENULIST_WORK * wk, u32 param, u8 mode)
{
	switch(mode){
	case 0:		// 通常動作
#if MYSTERY_SND_ON
		Snd_SePlay(SEQ_SE_DP_SELECT);
#endif
		break;
	case 1:		// メニュー起動時
		break;
	}
}

#pragma mark [end Edit
#if 0 

//------------------------------------------------------------------
/**
 * @brief	AGBカートリッジ抜き検出割り込み設定
 * @param	NONE
 * @return	NONE
 * ※ IrqTableを破壊的に操作します…
*/
//------------------------------------------------------------------
static void CartridgeIrqFunc(void)
{
	u32 val;
	if(PAD_DetectFold() == FALSE){
		// ふたが閉まっていなければ処理を停止させる
		// ふたが閉じてる際の処理はmain.cのsleep復帰後処理に任せる
		// ただしRTCの電池が無い場合にはカートリッジが刺さっていても割り込みが入るので、
		// 抜け検出をして"物理的に"抜かれていた場合のみ処理を停止させる
		if(CTRDG_IsAgbCartridge() == FALSE)
			CTRDG_TerminateForPulledOut();
	}
}
void MysteryLib_SetAgbCartridgeIntr(BOOL flag)
{
	if(flag == TRUE){
		OS_DisableIrq();
		OS_SetIrqFunction(OS_IE_CARTRIDGE, CartridgeIrqFunc);
		OS_EnableIrqMask(OS_IE_CARTRIDGE);
		OS_EnableIrq();
	} else {
		OS_DisableIrqMask(OS_IE_CARTRIDGE);
	}
}
static void CartridgeIrqFunc2(void){}
void MysteryLib_SetAgbCartridgeIntr2(void)
{
	OS_DisableIrq();
	OS_SetIrqFunction(OS_IE_CARTRIDGE, CartridgeIrqFunc2);
	OS_EnableIrqMask(OS_IE_CARTRIDGE);
	OS_EnableIrq();
}


#pragma mark [start Edit
#endif 

//------------------------------------------------------------------
/**
 * @brief	ＤＳセーブシーケンス
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
void MysteryLib_InitSaveDSCard(SAVE_CONTROL_WORK *sv)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	wk->sv = sv;
	wk->save_seq = MYSTERYLIB_SEQ_SAVE_INIT;
}

int MysteryLib_SaveDSCard(void)
{
	SAVE_RESULT result;
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();

	switch(wk->save_seq){
	case MYSTERYLIB_SEQ_SAVE_INIT:
		//FIXME ソフリ
		//sys_SoftResetNG(SOFTRESET_TYPE_SAVELOAD);
		
		//SaveData_DivSave_Init(wk->sv, SVBLK_ID_MAX);
		SaveControl_SaveAsyncInit( wk->sv );
		wk->save_seq++;
		break;

	case MYSTERYLIB_SEQ_SAVE_MAIN:
//    result = GAMEDATA_SaveAsyncMain(pWork->gamedata);  //@todo

    if(result == SAVE_RESULT_NG){
			wk->save_seq = MYSTERYLIB_SEQ_SAVE_NG;
		} else if(result == SAVE_RESULT_OK){
			wk->save_seq = MYSTERYLIB_SEQ_SAVE_OK;
		} else if(result == SAVE_RESULT_LAST){
			wk->save_seq = MYSTERYLIB_SEQ_SAVE_LAST;
		}
		if(result == SAVE_RESULT_OK || result == SAVE_RESULT_NG)
		//FIXME ソフリ
			//sys_SoftResetOK(SOFTRESET_TYPE_SAVELOAD);
		return result;
		break;

	case MYSTERYLIB_SEQ_SAVE_LAST:
		// 最後のセーブ待ち
		// ↓にあるMysteryLib_DoLastSaveを実行すると最後のセーブが完了
#if 0//def DEBUG_ONLY_FOR_mituhara
		OS_TPrintf("LAST-SAVE\n");
#endif
		break;
	case MYSTERYLIB_SEQ_SAVE_OK:
		break;
	case MYSTERYLIB_SEQ_SAVE_NG:
		break;
	}
	return SAVE_RESULT_CONTINUE;
}

void MysteryLib_DoLastSave(void)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	if(wk->save_seq == MYSTERYLIB_SEQ_SAVE_LAST)
		wk->save_seq = MYSTERYLIB_SEQ_SAVE_MAIN;
}

void MysteryLib_CancelSaveDSCard(void)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	//FIXME
//	SaveData_DivSave_Cancel(wk->sv);
	wk->save_seq = MYSTERYLIB_SEQ_SAVE_NG;
//	sys_SoftResetOK(SOFTRESET_TYPE_SAVELOAD);
}

int MysteryLib_GetSaveStatus(void)
{
	MYSTERYLIB_WORK *wk = GetMysteryLibWorkPtr();
	return wk->save_seq;
}

/*  */
