//============================================================================================
/**
 * @file	talkmsgwin.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"

#include "font/font.naix"
#include "print/printsys.h"
#include "print/str_tool.h"

#include "system/talkmsgwin.h"

#include "talkwin_test.naix"
//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	
 *
 *
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	定数
 */
//------------------------------------------------------------------
#define COL_SIZ				(2)
#define PLTT_SIZ			(16*COL_SIZ)

#define BACKGROUND_COLOR (15)

typedef enum {
	TAIL_SETPAT_NONE = 0,
	TAIL_SETPAT_U,
	TAIL_SETPAT_D,
	TAIL_SETPAT_L,
	TAIL_SETPAT_R,
}TAIL_SETPAT;

//============================================================================================
/**
 *
 * @brief	構造体定義
 *
 */
//============================================================================================
typedef struct {
	VecFx32				trans;
	fx32					scale;
	VecFx16				vtxTail0;
	VecFx16				vtxTail1;
	VecFx16				vtxTail2;
	VecFx16				vtxWin0;
	VecFx16				vtxWin1;
	VecFx16				vtxWin2;
	VecFx16				vtxWin3;
	VecFx16				vecN;
	TAIL_SETPAT		pattern;
}TAIL_DATA;

typedef struct {
	int							seq;

	PRINT_STREAM*		printStream;
	GFL_BMPWIN*			bmpwin;

	VecFx32*				pTarget;
	STRBUF*					msg;
	GXRgb						color;
	u16							refTarget;

	TAIL_DATA				tailData;

	u8							writex;
	u8							writey;

	u16							timer;
}TMSGWIN;

struct _TALKMSGWIN_SYS{
	TALKMSGWIN_SYS_SETUP	setup;
	TMSGWIN								tmsgwin[TALKMSGWIN_NUM];
  GFL_TCBLSYS*					tcbl;
  u16										chrNum;
};

static u32 setupWindowBG( TALKMSGWIN_SYS_SETUP* setup );

static void initWindow( TMSGWIN* tmsgwin );
static BOOL checkEmptyWindow( TMSGWIN* tmsgwin );
static void setupWindow(	TALKMSGWIN_SYS*		tmsgwinSys,
													TMSGWIN*					tmsgwin,
													VecFx32*					pTarget,
													STRBUF*						msg,
													TALKMSGWIN_SETUP* setup );
static void deleteWindow( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin );
static void openWindow( TMSGWIN* tmsgwin );
static void closeWindow( TMSGWIN* tmsgwin );
static void mainfuncWindow( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin );
static void draw3Dwindow( TMSGWIN* tmsgwin);

static BOOL calcTail( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin );
static void	drawTail(TAIL_DATA* tailData, GXRgb color, u16 scaleWait, BOOL tailOnly);

static void writeWindow( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin );
static void clearWindow( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin );

#define TALKMSGWIN_OPENWAIT	(16)
//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	システム関数
 *
 *
 *
 *
 *
 */
//============================================================================================
TALKMSGWIN_SYS* TALKMSGWIN_SystemCreate( TALKMSGWIN_SYS_SETUP* setup )
{
	int i;

	TALKMSGWIN_SYS* tmsgwinSys = GFL_HEAP_AllocClearMemory(setup->heapID, sizeof(TALKMSGWIN_SYS));
	tmsgwinSys->setup = *setup;
  tmsgwinSys->tcbl = GFL_TCBL_Init(setup->heapID, setup->heapID, 32, 32);

	for( i=0; i<TALKMSGWIN_NUM; i++ ){ initWindow(&tmsgwinSys->tmsgwin[i]); }

	{
		u32 siz = setupWindowBG(&tmsgwinSys->setup);
		tmsgwinSys->chrNum = siz/0x20;
	}

	return tmsgwinSys;
}

//------------------------------------------------------------------
void TALKMSGWIN_SystemMain( TALKMSGWIN_SYS* tmsgwinSys )
{
	int i;

  GFL_TCBL_Main(tmsgwinSys->tcbl);

	for( i=0; i<TALKMSGWIN_NUM; i++ ){ 
		mainfuncWindow(tmsgwinSys, &tmsgwinSys->tmsgwin[i]); 
	}
}

//------------------------------------------------------------------
void TALKMSGWIN_SystemDraw2D( TALKMSGWIN_SYS* tmsgwinSys )
{
}

//------------------------------------------------------------------
void TALKMSGWIN_SystemDraw3D( TALKMSGWIN_SYS* tmsgwinSys )
{
	int i;

//	GFL_G3D_DRAW_Start();			//描画開始

	for( i=0; i<TALKMSGWIN_NUM; i++ ){ draw3Dwindow(&tmsgwinSys->tmsgwin[i]); }

//	GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）
}

//------------------------------------------------------------------
void TALKMSGWIN_SystemDelete( TALKMSGWIN_SYS* tmsgwinSys )
{
	int i;

	for( i=0; i<TALKMSGWIN_NUM; i++ ){ 
		deleteWindow(tmsgwinSys, &tmsgwinSys->tmsgwin[i]); 
	}
  GFL_TCBL_Exit(tmsgwinSys->tcbl);
	GFL_HEAP_FreeMemory(tmsgwinSys);
}

//------------------------------------------------------------------
u32 TALKMSGWIN_SystemGetUsingChrNumber( TALKMSGWIN_SYS* tmsgwinSys )
{
	return tmsgwinSys->chrNum;
}

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	ウインドウ関数
 *
 *
 *
 *
 *
 */
//============================================================================================
#if 0
int TALKMSGWIN_CreateWindow(	TALKMSGWIN_SYS*		tmsgwinSys, 
															VecFx32*					pTarget,
															STRBUF*						msg,
															TALKMSGWIN_SETUP* tmsgwinSetup )
{
	int i;

	for( i=0; i<TALKMSGWIN_NUM; i++ ){ 
		if(checkEmptyWindow(&tmsgwinSys->tmsgwin[i]) == TRUE){
			setupWindow(tmsgwinSys, &tmsgwinSys->tmsgwin[i], pTarget, msg, tmsgwinSetup);
			return i;
		}
	}
	return -1;
}
#endif
//------------------------------------------------------------------
void TALKMSGWIN_CreateWindowIdx(	TALKMSGWIN_SYS*		tmsgwinSys, 
																	int								tmsgwinIdx,
																	VecFx32*					pTarget,
																	STRBUF*						msg,
																	TALKMSGWIN_SETUP* tmsgwinSetup )
{
	GF_ASSERT( (tmsgwinIdx>=0)&&(tmsgwinIdx<TALKMSGWIN_NUM) );

	setupWindow(tmsgwinSys, &tmsgwinSys->tmsgwin[tmsgwinIdx], pTarget, msg, tmsgwinSetup);
}

//------------------------------------------------------------------
void TALKMSGWIN_CreateWindowIdxConnect(	TALKMSGWIN_SYS*		tmsgwinSys, 
																				int								tmsgwinIdx,
																				int								prev_tmsgwinIdx,
																				STRBUF*						msg,
																				TALKMSGWIN_SETUP* tmsgwinSetup )
{
	GF_ASSERT( (tmsgwinIdx>=0)&&(tmsgwinIdx<TALKMSGWIN_NUM) );
	GF_ASSERT( (prev_tmsgwinIdx>=0)&&(prev_tmsgwinIdx<TALKMSGWIN_NUM) );

	setupWindow(tmsgwinSys, &tmsgwinSys->tmsgwin[tmsgwinIdx], 
							tmsgwinSys->tmsgwin[prev_tmsgwinIdx].pTarget, msg, tmsgwinSetup);

	tmsgwinSys->tmsgwin[prev_tmsgwinIdx].pTarget = NULL;
	tmsgwinSys->tmsgwin[prev_tmsgwinIdx].refTarget = tmsgwinIdx;
}

//------------------------------------------------------------------
void TALKMSGWIN_DeleteWindow( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx )
{
	GF_ASSERT( (tmsgwinIdx>=0)&&(tmsgwinIdx<TALKMSGWIN_NUM) );

	deleteWindow(tmsgwinSys, &tmsgwinSys->tmsgwin[tmsgwinIdx]);
}

//------------------------------------------------------------------
void TALKMSGWIN_OpenWindow( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx )
{
	GF_ASSERT( (tmsgwinIdx>=0)&&(tmsgwinIdx<TALKMSGWIN_NUM) );

	openWindow(&tmsgwinSys->tmsgwin[tmsgwinIdx]);
}

//------------------------------------------------------------------
void TALKMSGWIN_CloseWindow( TALKMSGWIN_SYS* tmsgwinSys, int tmsgwinIdx )
{
	GF_ASSERT( (tmsgwinIdx>=0)&&(tmsgwinIdx<TALKMSGWIN_NUM) );

	closeWindow(&tmsgwinSys->tmsgwin[tmsgwinIdx]);
}





//============================================================================================
/**
 * @brief	ウインドウ制御関数
 */
//============================================================================================
enum {
	WINSEQ_EMPTY = 0,
	WINSEQ_IDLING,
	WINSEQ_OPEN,
	WINSEQ_HOLD,
	WINSEQ_CLOSE,
};

//------------------------------------------------------------------
static void initWindow( TMSGWIN* tmsgwin )
{
	tmsgwin->seq = WINSEQ_EMPTY;
	tmsgwin->printStream = NULL;
}

//------------------------------------------------------------------
static BOOL checkEmptyWindow( TMSGWIN* tmsgwin )
{
	if( tmsgwin->seq != WINSEQ_EMPTY ){ return FALSE; }

	return TRUE;
}

//------------------------------------------------------------------
static void setupWindow(	TALKMSGWIN_SYS*		tmsgwinSys,
													TMSGWIN*					tmsgwin,
													VecFx32*					pTarget,
													STRBUF*						msg,
													TALKMSGWIN_SETUP* setup )
{
	GF_ASSERT( (setup->winpx<32)&&(setup->winpy<24)&&(setup->winsx<=32)&&(setup->winsy<=24) );

	deleteWindow(tmsgwinSys, tmsgwin);

	tmsgwin->pTarget = pTarget;
	tmsgwin->msg = msg;
	tmsgwin->color = setup->color;
	tmsgwin->refTarget = 0;

	//描画用ビットマップ作成
	{
		u8 px = ( setup->winpx + setup->winsx <= 32 )? setup->winpx : 32 - setup->winsx;
		u8 py = ( setup->winpy + setup->winsy <= 24 )? setup->winpy : 24 - setup->winsy;

		tmsgwin->bmpwin = GFL_BMPWIN_Create(tmsgwinSys->setup.ini.frameID,
																				px, py, setup->winsx, setup->winsy,
																				tmsgwinSys->setup.ini.fontPltID,
																				GFL_BG_CHRAREA_GET_B );
		//ウインドウ生成
#if 0
		GFL_BMP_Clear(GFL_BMPWIN_GetBmp(tmsgwin->bmpwin), 2);
		GFL_BMP_Fill(GFL_BMPWIN_GetBmp(tmsgwin->bmpwin), 2, 2, setup->winsx*8-4, setup->winsy*8-4, 15);
#else
		GFL_BMP_Clear(GFL_BMPWIN_GetBmp(tmsgwin->bmpwin), BACKGROUND_COLOR);
#endif
		GFL_BMPWIN_TransVramCharacter(tmsgwin->bmpwin);
	}
	//吹き出しエフェクトパラメータ計算
	calcTail(tmsgwinSys, tmsgwin);

	//描画位置算出（センタリング）
	{
		u32 width = PRINTSYS_GetStrWidth(msg, tmsgwinSys->setup.fontHandle, 0);
		u32 height = PRINTSYS_GetStrHeight(msg, tmsgwinSys->setup.fontHandle);

		//heightが正しくないのでとりあえず
		height = setup->winsy*8;

		if(width > (setup->winsx * 8)){
			tmsgwin->writex = 0;
			GF_ASSERT(0);
		} else {
			tmsgwin->writex = (setup->winsx*8 - width)/2;
		}
		if(height > (setup->winsy * 8)){
			tmsgwin->writey = 0;
			GF_ASSERT(0);
		} else {
			tmsgwin->writey = (setup->winsy*8 - height)/2;
		}
	}

	tmsgwin->seq = WINSEQ_IDLING;
}

//------------------------------------------------------------------
static void deleteWindow( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin )
{
	if(checkEmptyWindow(tmsgwin) == FALSE){
		if(tmsgwin->printStream != NULL){
			PRINTSYS_PrintStreamDelete(tmsgwin->printStream);
			tmsgwin->printStream = NULL;
		}
		clearWindow(tmsgwinSys, tmsgwin);

		GFL_BMPWIN_Delete(tmsgwin->bmpwin);
	
		tmsgwin->seq = WINSEQ_EMPTY;
	}
}

//------------------------------------------------------------------
static void openWindow( TMSGWIN* tmsgwin )
{
	tmsgwin->seq = WINSEQ_OPEN;
	tmsgwin->timer = 0;
}

//------------------------------------------------------------------
static void closeWindow( TMSGWIN* tmsgwin )
{
	tmsgwin->seq = WINSEQ_CLOSE;
}

//------------------------------------------------------------------
static void mainfuncWindow( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin )
{
	switch(tmsgwin->seq){
	case WINSEQ_EMPTY:
	case WINSEQ_IDLING:
		break;
	case WINSEQ_OPEN:
		if(tmsgwin->timer < TALKMSGWIN_OPENWAIT){
			tmsgwin->timer++;
		}else{
			tmsgwin->seq = WINSEQ_HOLD;
			writeWindow(tmsgwinSys, tmsgwin);

			tmsgwin->printStream = PRINTSYS_PrintStream(	tmsgwin->bmpwin,							// GFL_BMPWIN
																										tmsgwin->writex,							// u16
																										tmsgwin->writey,							// u16
																										tmsgwin->msg,									// STRBUF*
																										tmsgwinSys->setup.fontHandle,	// GFL_FONT*
																										2,														// int wait
																										tmsgwinSys->tcbl,							// GFL_TCBLSYS*
																										0,														// u32 tcbpri
																										tmsgwinSys->setup.heapID,			// HEAPID
																										BACKGROUND_COLOR );						// u16 clrCol
		}
		break;
	case WINSEQ_HOLD:
		if(calcTail(tmsgwinSys, tmsgwin) == TRUE){ writeWindow(tmsgwinSys, tmsgwin); }
		break;
	case WINSEQ_CLOSE:
		deleteWindow(tmsgwinSys, tmsgwin);
		break;
	}
}

//------------------------------------------------------------------
static void draw3Dwindow( TMSGWIN* tmsgwin )
{
	switch(tmsgwin->seq){
	case WINSEQ_EMPTY:
	case WINSEQ_IDLING:
		break;
	case WINSEQ_OPEN:
	case WINSEQ_CLOSE:
		drawTail(&tmsgwin->tailData, tmsgwin->color, tmsgwin->timer, FALSE);
		break;
	case WINSEQ_HOLD:
		drawTail(&tmsgwin->tailData, tmsgwin->color, tmsgwin->timer, TRUE);
		break;
	}
}

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	３Ｄ描画
 *
 *
 *
 *
 *
 */
//============================================================================================
static void	drawTail( TAIL_DATA* tailData, GXRgb color, u16 scaleWait, BOOL tailOnly )
{
	fx32 scale;

	G3_PushMtx();
	//平行移動パラメータ設定
	G3_Translate(tailData->trans.x, tailData->trans.y, tailData->trans.z);
	//グローバルスケール設定
	scale = tailData->scale * scaleWait / TALKMSGWIN_OPENWAIT;
	G3_Scale(scale, scale, scale);

	G3_TexImageParam(GX_TEXFMT_NONE, GX_TEXGEN_NONE, 0, 0, 0, 0, GX_TEXPLTTCOLOR0_USE, 0);

	//マテリアル設定
	G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), GX_RGB(16, 16, 16), TRUE);
	G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), GX_RGB(0, 0, 0), FALSE);
	G3_PolygonAttr(	GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 63, 31, 0);
	
	if(tailData->pattern != TAIL_SETPAT_NONE){
		G3_Begin(GX_BEGIN_TRIANGLES);

		G3_Color(color);

		G3_Vtx(tailData->vtxTail2.x, tailData->vtxTail2.y, tailData->vtxTail2.z); 
		G3_Vtx(tailData->vtxTail0.x, tailData->vtxTail0.y, tailData->vtxTail0.z); 
		G3_Vtx(tailData->vtxTail1.x, tailData->vtxTail1.y, tailData->vtxTail1.z); 

		G3_End();
	}
	if( tailOnly == FALSE ){
		G3_Begin( GX_BEGIN_QUADS );

		G3_Color(color);

		G3_Vtx(tailData->vtxWin2.x, tailData->vtxWin2.y, tailData->vtxWin2.z); 
		G3_Vtx(tailData->vtxWin0.x, tailData->vtxWin0.y, tailData->vtxWin0.z); 
		G3_Vtx(tailData->vtxWin1.x, tailData->vtxWin1.y, tailData->vtxWin1.z); 
		G3_Vtx(tailData->vtxWin3.x, tailData->vtxWin3.y, tailData->vtxWin3.z); 

		G3_End();
	}

	G3_PopMtx(1);
}


//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	３Ｄ座標計算
 *
 *
 *
 *
 *
 */
//============================================================================================
static BOOL calcTail( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin )
{
	VecFx32		vecTail0, vecTail1, vecTail2;
	VecFx32		vecWin0, vecWin1, vecWin2, vecWin3;
	fx32			near_backup, scale;
	u8				pattern;
	BOOL			result = FALSE;

	//変換精度を上げるためnearの距離をとる
	GFL_G3D_CAMERA_GetNear(tmsgwinSys->setup.g3Dcamera, &near_backup);
	{
		fx32 near = 64* FX32_ONE;

		GFL_G3D_CAMERA_SetNear(tmsgwinSys->setup.g3Dcamera, &near);
		GFL_G3D_CAMERA_Switching(tmsgwinSys->setup.g3Dcamera);
	}

	//tailをnearクリップ平面状の座標に変換
	{
		int	targetx, targety; 
		int	tail_length;
		int	px = (GFL_BMPWIN_GetPosX(tmsgwin->bmpwin)-1) * 8;
		int	py = (GFL_BMPWIN_GetPosY(tmsgwin->bmpwin)-1) * 8;
		int	sx = (GFL_BMPWIN_GetScreenSizeX(tmsgwin->bmpwin)+2) * 8;
		int	sy = (GFL_BMPWIN_GetScreenSizeY(tmsgwin->bmpwin)+2) * 8;

		//tail頂点算出
		if(tmsgwin->pTarget != NULL){
			//taget座標よりtail頂点算出
			NNS_G3dWorldPosToScrPos(tmsgwin->pTarget, &targetx, &targety);
		} else {
			//指定参照ウインドウ座標よりtail頂点算出
			GFL_BMPWIN* ref_bmpwin = tmsgwinSys->tmsgwin[tmsgwin->refTarget].bmpwin;
			int	ref_px = (GFL_BMPWIN_GetPosX(ref_bmpwin)-1) * 8;
			int	ref_py = (GFL_BMPWIN_GetPosY(ref_bmpwin)-1) * 8;
			int	ref_sx = (GFL_BMPWIN_GetScreenSizeX(ref_bmpwin)+2) * 8;
			int	ref_sy = (GFL_BMPWIN_GetScreenSizeY(ref_bmpwin)+2) * 8;

			if((px + sx) < ref_px){
				//参照winの左への配置設定
				targetx = ref_px;
			} else if(px > (ref_px + ref_sx)){
				//参照winの右への配置設定
				targetx = ref_px + ref_sx;
			} else {
				targetx = ref_px + ref_sx/2;
			}
			if((py + sy) < ref_py){
				//参照winの上への配置設定
				targety = ref_py;
			} else if(py > (ref_py + ref_sy)){
				//参照winの下への配置設定
				targety = ref_py + ref_sy;
			} else {
				targety = ref_py + ref_sy/2;
			}
		}
		NNS_G3dScrPosToWorldLine(targetx, targety, &vecTail0, NULL );
#if 0
		tail_length = ((sx>=32)&&(sy>=32))? 32 : sy;
		if( (targety > py)&&(targety < (py + sy)) ){
			NNS_G3dScrPosToWorldLine((px + sx/2),	(py + sy/2 - tail_length/2), &vecTail1, NULL );
			NNS_G3dScrPosToWorldLine((px + sx/2),	(py + sy/2 + tail_length/2), &vecTail2, NULL );
		} else {
			NNS_G3dScrPosToWorldLine((px + sx/2 - tail_length/2),	(py + sy/2), &vecTail1, NULL );
			NNS_G3dScrPosToWorldLine((px + sx/2 + tail_length/2),	(py + sy/2), &vecTail2, NULL );
		}
#else
		//tail根っこ算出
		{
			u16 ex1, ey1, ex2, ey2;

			tail_length = ((sx>=16)&&(sy>=16))? 16 : sy;
			if(targety < py){
				ey1 = py + 1;
				ey2 = py + 1;
				ex1 = px + sx/2 - tail_length/2;
				ex2 = px + sx/2 + tail_length/2;
				pattern = TAIL_SETPAT_U;
			} else if(targety > (py + sy)){
				ey1 = py + sy - 1;
				ey2 = py + sy - 1;
				ex1 = px + sx/2 - tail_length/2;
				ex2 = px + sx/2 + tail_length/2;
				pattern = TAIL_SETPAT_D;
			} else {
				ey1 = py + sy/2 - tail_length/2;
				ey2 = py + sy/2 + tail_length/2;
				if(targetx < px){
					ex1 = px + 1;
					ex2 = px + 1;
					pattern = TAIL_SETPAT_L;
				} else if(targetx > (px + sx)){
					ex1 = px + sx - 1;
					ex2 = px + sx - 1;
					pattern = TAIL_SETPAT_R;
				} else {
					ex1 = px + sx/2;
					ex2 = px + sx/2;
					pattern = TAIL_SETPAT_NONE;
				}
			}
			NNS_G3dScrPosToWorldLine(ex1, ey1, &vecTail1, NULL );
			NNS_G3dScrPosToWorldLine(ex2, ey2, &vecTail2, NULL );
		}
#endif
	}
	//winをnearクリップ平面状の座標に変換
	{
		int	px = GFL_BMPWIN_GetPosX(tmsgwin->bmpwin) * 8;
		int	py = GFL_BMPWIN_GetPosY(tmsgwin->bmpwin) * 8;
		int	sx = GFL_BMPWIN_GetScreenSizeX(tmsgwin->bmpwin) * 8;
		int	sy = GFL_BMPWIN_GetScreenSizeY(tmsgwin->bmpwin) * 8;

		NNS_G3dScrPosToWorldLine((px + 0),	(py + 0),		&vecWin0, NULL );
		NNS_G3dScrPosToWorldLine((px + sx),	(py + 0),		&vecWin1, NULL );
		NNS_G3dScrPosToWorldLine((px + 0),	(py + sy),	&vecWin2, NULL );
		NNS_G3dScrPosToWorldLine((px + sx), (py + sy),	&vecWin3, NULL );
	}

	//vecTail0基準にベクトル変換
	VEC_Subtract(&vecTail1, &vecTail0, &vecTail1);
	VEC_Subtract(&vecTail2, &vecTail0, &vecTail2);
	VEC_Subtract(&vecWin0, &vecTail0, &vecWin0);
	VEC_Subtract(&vecWin1, &vecTail0, &vecWin1);
	VEC_Subtract(&vecWin2, &vecTail0, &vecWin2);
	VEC_Subtract(&vecWin3, &vecTail0, &vecWin3);

	//スケールの決定
	{
		fx32 valTail1, valTail2, valWin0, valWin1, valWin2, valWin3;
		valTail1 = VEC_Mag(&vecTail1);
		valTail2 = VEC_Mag(&vecTail2);
		valWin0 = VEC_Mag(&vecWin0);
		valWin1 = VEC_Mag(&vecWin1);
		valWin2 = VEC_Mag(&vecWin2);
		valWin3 = VEC_Mag(&vecWin3);

		//一番長いベクトルの長さをスケールに設定（頂点設定値の制限のため）
		scale = (valTail1 >= valTail2)? valTail1 : valTail2;
		if( scale < valWin0 ){ scale = valWin0; }
		if( scale < valWin1 ){ scale = valWin1; }
		if( scale < valWin2 ){ scale = valWin2; }
		if( scale < valWin3 ){ scale = valWin3; }
	}

	//パラメータ設定
	VEC_Set(&tmsgwin->tailData.trans, vecTail0.x, vecTail0.y, vecTail0.z);
	tmsgwin->tailData.scale = scale;

	VEC_Fx16Set(&tmsgwin->tailData.vtxTail0, 0, 0, 0);
	VEC_Fx16Set(&tmsgwin->tailData.vtxTail1, 
							FX_Div(vecTail1.x,scale), FX_Div(vecTail1.y,scale), FX_Div(vecTail1.z,scale));
	VEC_Fx16Set(&tmsgwin->tailData.vtxTail2,
							FX_Div(vecTail2.x,scale), FX_Div(vecTail2.y,scale), FX_Div(vecTail2.z,scale));

	VEC_Fx16Set(&tmsgwin->tailData.vtxWin0, 
							FX_Div(vecWin0.x,scale), FX_Div(vecWin0.y,scale), FX_Div(vecWin0.z,scale));
	VEC_Fx16Set(&tmsgwin->tailData.vtxWin1,
							FX_Div(vecWin1.x,scale), FX_Div(vecWin1.y,scale), FX_Div(vecWin1.z,scale));
	VEC_Fx16Set(&tmsgwin->tailData.vtxWin2,
							FX_Div(vecWin2.x,scale), FX_Div(vecWin2.y,scale), FX_Div(vecWin2.z,scale));
	VEC_Fx16Set(&tmsgwin->tailData.vtxWin3,
							FX_Div(vecWin3.x,scale), FX_Div(vecWin3.y,scale), FX_Div(vecWin3.z,scale));
	{
		//法線ベクトル取得
		VecFx32 camPos, camTarget, vecN;

		GFL_G3D_CAMERA_GetPos(tmsgwinSys->setup.g3Dcamera, &camPos );
		GFL_G3D_CAMERA_GetTarget(tmsgwinSys->setup.g3Dcamera, &camTarget );
		VEC_Subtract(&camPos, &camTarget, &vecN);
		VEC_Normalize(&vecN, &vecN);
		VEC_Fx16Set(&tmsgwin->tailData.vecN, vecN.x, vecN.y, vecN.z);
	}
	if(tmsgwin->tailData.pattern != pattern){
		tmsgwin->tailData.pattern = pattern;
		result = TRUE;
	}
	//near復帰
	GFL_G3D_CAMERA_SetNear(tmsgwinSys->setup.g3Dcamera, &near_backup);
	GFL_G3D_CAMERA_Switching(tmsgwinSys->setup.g3Dcamera);

	return result;
}

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	ＢＧウインドウキャラクター
 *
 *
 *
 *
 *
 */
//============================================================================================
static u32 setupWindowBG( TALKMSGWIN_SYS_SETUP* setup )
{
	u32 chrSiz;
	//GFL_BG_FillCharacter(setup->ini.frameID, 0, 1, 0);	// 先頭にクリアキャラ配置
	GFL_BG_ClearScreen(setup->ini.frameID);

	//パレット転送
	{
		PALTYPE paltype = PALTYPE_MAIN_BG;

		switch(setup->ini.frameID){
		case GFL_BG_FRAME0_M:
		case GFL_BG_FRAME1_M:
		case GFL_BG_FRAME2_M:
		case GFL_BG_FRAME3_M:
			break;
		case GFL_BG_FRAME0_S:
		case GFL_BG_FRAME1_S:
		case GFL_BG_FRAME2_S:
		case GFL_BG_FRAME3_S:
			paltype = PALTYPE_SUB_BG;
			break;
		}
		GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, 
																	NARC_font_default_nclr,
																	paltype,
																	setup->ini.fontPltID * PLTT_SIZ,
																	PLTT_SIZ,
																	setup->heapID);
		GFL_ARC_UTIL_TransVramPalette(ARCID_TALKWIN_TEST, 
																	NARC_talkwin_test_talkwin_NCLR,
																	paltype,
																	setup->ini.winPltID * PLTT_SIZ,
																	PLTT_SIZ,
																	setup->heapID);
	}
	//キャラクター転送
	chrSiz = GFL_ARC_UTIL_TransVramBgCharacter(	ARCID_TALKWIN_TEST, 
																							NARC_talkwin_test_talkwin_NCGR,
																							setup->ini.frameID,
																							setup->chrNumOffs,
																							0,
																							FALSE,
																							setup->heapID);
	return chrSiz;
}

//------------------------------------------------------------------
#define FLIP_H (0x0400)
#define FLIP_V (0x0800)
#define UL_CHR (1)
#define UR_CHR (1 + FLIP_H)
#define DL_CHR (1 + FLIP_V)
#define DR_CHR (1 + FLIP_H + FLIP_V)
#define U_CHR (3)
#define D_CHR (3 + FLIP_V)
#define L_CHR (4)
#define R_CHR (4 + FLIP_H)
#define SPC_CHR (5)

static void writeWindow( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin )
{
	u16	px = GFL_BMPWIN_GetPosX(tmsgwin->bmpwin);
	u16	py = GFL_BMPWIN_GetPosY(tmsgwin->bmpwin);
	u16	sx = GFL_BMPWIN_GetScreenSizeX(tmsgwin->bmpwin);
	u16	sy = GFL_BMPWIN_GetScreenSizeY(tmsgwin->bmpwin);
	BOOL overU = FALSE;
	BOOL overD = FALSE;
	BOOL overL = FALSE;
	BOOL overR = FALSE;
	u8	frameID = tmsgwinSys->setup.ini.frameID;
	u16	wplt = (tmsgwinSys->setup.ini.winPltID) << 12;
	u16	chrOffs = tmsgwinSys->setup.chrNumOffs;

	if(px == 0){ overL = TRUE; }
	if((px + sx) == 32-1){ overR = TRUE; }
	if(py == 0){ overU = TRUE; }
	if((py + sy) == 24-1){ overD = TRUE; }

	//文字描画領域
	GFL_BMPWIN_MakeScreen(tmsgwin->bmpwin);

	//枠領域
	if(overU == FALSE){
		if(overL == FALSE){
			GFL_BG_FillScreen
				(frameID, (wplt | (UL_CHR+chrOffs)), px - 1, py - 1, 1, 1, GFL_BG_SCRWRT_PALIN );
		}
		GFL_BG_FillScreen
			(frameID, (wplt | (U_CHR+chrOffs)), px, py - 1, sx, 1, GFL_BG_SCRWRT_PALIN );
		if(overR == FALSE){
			GFL_BG_FillScreen
				(frameID, (wplt | (UR_CHR+chrOffs)), px + sx, py - 1, 1, 1, GFL_BG_SCRWRT_PALIN );
		}
	}
	if(overL == FALSE){
		GFL_BG_FillScreen
			(frameID, (wplt | (L_CHR+chrOffs)), px - 1, py, 1, sy, GFL_BG_SCRWRT_PALIN );
	}
	if(overR == FALSE){
		GFL_BG_FillScreen
			(frameID, (wplt | (R_CHR+chrOffs)), px + sx, py, 1, sy, GFL_BG_SCRWRT_PALIN );
	}
	if(overD == FALSE){
		if(overL == FALSE){
			GFL_BG_FillScreen
				(frameID, (wplt | (DL_CHR+chrOffs)), px - 1, py + sy, 1, 1, GFL_BG_SCRWRT_PALIN );
		}
		GFL_BG_FillScreen
			(frameID, (wplt | (D_CHR+chrOffs)), px, py + sy, sx, 1, GFL_BG_SCRWRT_PALIN );
		if(overR == FALSE){
			GFL_BG_FillScreen
				(frameID, (wplt | (DR_CHR+chrOffs)), px + sx, py + sy, 1, 1, GFL_BG_SCRWRT_PALIN );
		}
	}
	//tail接続領域
	if(tmsgwin->tailData.pattern == TAIL_SETPAT_U){
		if(overU == FALSE){
			GFL_BG_FillScreen
				(frameID, (wplt | (SPC_CHR+chrOffs)), px + sx/2 - 1, py - 1, 2, 1, GFL_BG_SCRWRT_PALIN );
		}
	} else if(tmsgwin->tailData.pattern == TAIL_SETPAT_D){
		if(overD == FALSE){
			GFL_BG_FillScreen
				(frameID, (wplt | (SPC_CHR+chrOffs)), px + sx/2 - 1, py + sy, 2, 1, GFL_BG_SCRWRT_PALIN );
		}
	} else if(tmsgwin->tailData.pattern == TAIL_SETPAT_L){
		if(overL == FALSE){
			GFL_BG_FillScreen
				(frameID, (wplt | (SPC_CHR+chrOffs)), px - 1, py + sy/2 - 1, 1, 2, GFL_BG_SCRWRT_PALIN );
		}
	} else if(tmsgwin->tailData.pattern == TAIL_SETPAT_R){
		if(overR == FALSE){
			GFL_BG_FillScreen
				(frameID, (wplt | (SPC_CHR+chrOffs)), px + sx, py + sy/2 - 1, 1, 2, GFL_BG_SCRWRT_PALIN );
		}
	}

	GFL_BG_LoadScreenReq(frameID);
}

//------------------------------------------------------------------
static void clearWindow( TALKMSGWIN_SYS* tmsgwinSys, TMSGWIN* tmsgwin )
{
	u16	px = GFL_BMPWIN_GetPosX(tmsgwin->bmpwin);
	u16	py = GFL_BMPWIN_GetPosY(tmsgwin->bmpwin);
	u16	sx = GFL_BMPWIN_GetScreenSizeX(tmsgwin->bmpwin);
	u16	sy = GFL_BMPWIN_GetScreenSizeY(tmsgwin->bmpwin);
	BOOL overU = FALSE;
	BOOL overD = FALSE;
	BOOL overL = FALSE;
	BOOL overR = FALSE;
	u8	frameID = tmsgwinSys->setup.ini.frameID;

	GFL_BMPWIN_ClearScreen(tmsgwin->bmpwin);

	if(px == 0){ overL = TRUE; }
	if((px + sx) == 32-1){ overR = TRUE; }
	if(py == 0){ overU = TRUE; }
	if((py + sy) == 24-1){ overD = TRUE; }

	if(overU == FALSE){
		if(overL == FALSE){
			GFL_BG_FillScreen(frameID, 0, px - 1, py - 1, 1, 1, GFL_BG_SCRWRT_PALIN );
		}
		GFL_BG_FillScreen(frameID, 0, px, py - 1, sx, 1, GFL_BG_SCRWRT_PALIN );
		if(overR == FALSE){
			GFL_BG_FillScreen(frameID, 0, px + sx, py - 1, 1, 1, GFL_BG_SCRWRT_PALIN );
		}
	}
	if(overL == FALSE){
		GFL_BG_FillScreen(frameID, 0, px - 1, py, 1, sy, GFL_BG_SCRWRT_PALIN );
	}
	if(overR == FALSE){
		GFL_BG_FillScreen(frameID, 0, px + sx, py, 1, sy, GFL_BG_SCRWRT_PALIN );
	}
	if(overD == FALSE){
		if(overL == FALSE){
			GFL_BG_FillScreen(frameID, 0, px - 1, py + sy, 1, 1, GFL_BG_SCRWRT_PALIN );
		}
		GFL_BG_FillScreen(frameID, 0, px, py + sy, sx, 1, GFL_BG_SCRWRT_PALIN );
		if(overR == FALSE){
			GFL_BG_FillScreen(frameID, 0, px + sx, py + sy, 1, 1, GFL_BG_SCRWRT_PALIN );
		}
	}
	GFL_BG_LoadScreenReq(frameID);
}







