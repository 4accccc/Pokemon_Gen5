//============================================================================================
/**
 * @file	status_cont.c
 * @brief	ステータスコントロール
 */
//============================================================================================
#include "gflib.h"
#include "textprint.h"

#include "setup.h"
#include "team_cont.h"
#include "camera_cont.h"
#include "status_cont.h"

//============================================================================================
//
//
//	ステータス
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
typedef struct {
	GFL_CLWK*	clact;
	u32			resCgx;
}CLACT_GRP;

typedef struct {
	CLACT_GRP		castle;
	CLACT_GRP		summon[TEAM_SUMMON_COUNT_MAX];
	CLACT_GRP		player[TEAM_PLAYER_COUNT_MAX];
}STATUS_CLACT_P;

struct _STATUS_CONTROL {
	HEAPID				heapID;
	TEAM_CONTROL**		p_tc;
	STATUS_CLACT_P**	p_tact;
	int					teamCount;
	CAMERA_CONTROL*		cc;
	PLAYER_CONTROL*		myPc;

	BOOL				reloadFlag;

	GFL_BMP_DATA*		bmpTmp;
};

enum {
	STATUS_DRAW_NAME	= 0x0001,
	STATUS_DRAW_BER		= 0x0002,
	STATUS_DRAW_FULL	= 0x0003,
};

#define STATUS_BMP_X	(8)	//キャラ単位
#define STATUS_BMP_Y	(2)	//キャラ単位

static const char	castleName[] = {"Castle"};
static const char	summonName[] = {"Summon"};
// ｘｙ座標、アニメーションシーケンス、ソフト優先順位(0>0xff)、BG優先順位
static const GFL_CLWK_DATA clwkData = { 0, 0, 0, 0, 1 }; 

static BOOL setScreenPos( GFL_CLWK* clact, VecFx32* trans, CAMERA_CONTROL* cc );
static void resetStatus( STATUS_CLACT_P* tact );
static void DrawStatusBer( GFL_BMP_DATA* bmp, int hpNow, int hpMax );
static void StatusDraw
		( CLACT_GRP* clactGrp, const char* name, int hp, int hpMax, int mode, GFL_BMP_DATA* bmp );
//------------------------------------------------------------------
/**
 * @brief	ステータスコントロール起動
 */
//------------------------------------------------------------------
typedef struct {
	STATUS_CLACT_P*	tact;
	GFL_BMP_DATA*	bmpTmp;
}ADDSTATUS_CALLBACK_WORK;

static void	AddStatusSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work );
static void	AddStatusPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work );
//------------------------------------------------------------------
STATUS_CONTROL* AddStatusControl( STATUS_SETUP* setup )
{
	int i, j;
	STATUS_CONTROL*	stc = GFL_HEAP_AllocClearMemory( setup->heapID, sizeof(STATUS_CONTROL) );
	TEAM_CONTROL** p_tc = GFL_HEAP_AllocClearMemory
							( setup->heapID, setup->teamCount * sizeof(TEAM_CONTROL*) );
	STATUS_CLACT_P** p_tact = GFL_HEAP_AllocClearMemory
							( setup->heapID, setup->teamCount * sizeof(STATUS_CLACT_P*) );
	GFL_BMP_DATA* bmp = GFL_BMP_Create
							( STATUS_BMP_X, STATUS_BMP_Y, GFL_BMP_16_COLOR, setup->heapID );
	GFL_CLUNIT* clactUnit = Get_GS_ClactUnit( setup->gs, CLACTUNIT_STATUS );
	u32 resCgxID = Get_GS_ClactResIdx( setup->gs, CLACTRES_STATUS_CGX );
	u32 resCelID = Get_GS_ClactResIdx( setup->gs, CLACTRES_STATUS_CEL );
	u32 resPltID = Get_GS_ClactResIdx( setup->gs, CLACTRES_STATUS_PLT );
	ADDSTATUS_CALLBACK_WORK* ascw = GFL_HEAP_AllocClearMemoryLo
									( setup->heapID, sizeof(ADDSTATUS_CALLBACK_WORK) );

	stc->heapID		= setup->heapID;
	stc->p_tc		= p_tc;
	stc->p_tact		= p_tact;
	stc->teamCount	= setup->teamCount;
	stc->cc			= setup->cc;
	stc->myPc		= setup->myPc;
	stc->bmpTmp		= bmp;

	for( i=0; i<stc->teamCount; i++ ){
		STATUS_CLACT_P*	tact;
		tact = GFL_HEAP_AllocClearMemory( stc->heapID, sizeof(STATUS_CLACT_P) );

		p_tc[i] = setup->p_tc[i];
		p_tact[i] = tact;

		ascw->tact = tact;
		ascw->bmpTmp = stc->bmpTmp;
		for( j=0; j<TEAM_SUMMON_COUNT_MAX; j++ ){
			tact->summon[j].clact = GFL_OBJGRP_CreateClActVT( clactUnit,	
														resCgxID, resPltID, resCelID,
														&clwkData, CLSYS_DRAW_MAIN, stc->heapID );
			tact->summon[j].resCgx = resCgxID;
			resCgxID++;
			GF_ASSERT( resCgxID < STATUS_SETUP_NUM );
		}
		ProcessingAllTeamSummonObject( p_tc[i], AddStatusSummonCallBack, (void*)ascw );

		for( j=0; j<TEAM_PLAYER_COUNT_MAX; j++ ){
			tact->player[j].clact = GFL_OBJGRP_CreateClActVT( clactUnit,	
														resCgxID, resPltID, resCelID,
														&clwkData, CLSYS_DRAW_MAIN, stc->heapID );
			tact->player[j].resCgx = resCgxID;
			resCgxID++;
			GF_ASSERT( resCgxID < STATUS_SETUP_NUM );
		}
		ProcessingAllTeamPlayer( p_tc[i], AddStatusPlayerCallBack, (void*)ascw );

		resetStatus( tact );
	}
	stc->reloadFlag = TRUE;

	GFL_HEAP_FreeMemory( ascw ); 

	return stc;
}

//------------------------------------------------------------------
static void	AddStatusSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work )
{
	ADDSTATUS_CALLBACK_WORK* ascw = (ADDSTATUS_CALLBACK_WORK*)work;
	int hp, hpMax;

	GetSummonObjectHP( tc, summonID, &hp, &hpMax );
	StatusDraw( &ascw->tact->summon[num], summonName, hp, hpMax, 
				STATUS_DRAW_FULL, ascw->bmpTmp );
}

static void	AddStatusPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work )
{
	ADDSTATUS_CALLBACK_WORK* ascw = (ADDSTATUS_CALLBACK_WORK*)work;
	PLAYER_STATUS* ps = GetPlayerStatusPointer( pc );

	StatusDraw( &ascw->tact->player[num], ps->name, ps->hp, ps->hpMax, 
				STATUS_DRAW_FULL, ascw->bmpTmp );
}

//------------------------------------------------------------------
/**
 * @brief	ステータスコントロール終了
 */
//------------------------------------------------------------------
void RemoveStatusControl( STATUS_CONTROL* stc )
{
	int i, j;

	for( i=0; i<stc->teamCount; i++ ){
		STATUS_CLACT_P*	tact = stc->p_tact[i];

		for( j=0; j<TEAM_PLAYER_COUNT_MAX; j++ ){
			GFL_CLACT_WK_Remove( tact->player[j].clact );
		}
		for( j=0; j<TEAM_SUMMON_COUNT_MAX; j++ ){
			GFL_CLACT_WK_Remove( tact->summon[j].clact );
		}
		GFL_HEAP_FreeMemory( tact ); 
	}
	GFL_BMP_Delete( stc->bmpTmp );
	GFL_HEAP_FreeMemory( stc->p_tact ); 
	GFL_HEAP_FreeMemory( stc->p_tc ); 
	GFL_HEAP_FreeMemory( stc ); 
}

//------------------------------------------------------------------
/**
 * @brief	ステータスメインコントロール
 */
//------------------------------------------------------------------
typedef struct {
	STATUS_CLACT_P*	tact;
	CAMERA_CONTROL*	cc;
	PLAYER_CONTROL*	myPc;
	GFL_BMP_DATA*	bmpTmp;
}DRAWSTATUS_CALLBACK_WORK;

static void	DrawStatusSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work );
static void	DrawStatusPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work );

//------------------------------------------------------------------
void MainStatusControl( STATUS_CONTROL* stc )
{
	int i, j;
	DRAWSTATUS_CALLBACK_WORK* dscw = GFL_HEAP_AllocClearMemory
									( stc->heapID, sizeof(DRAWSTATUS_CALLBACK_WORK) );
	VecFx32 trans;

	dscw->cc		= stc->cc;
	dscw->myPc		= stc->myPc;
	dscw->bmpTmp	= stc->bmpTmp;

	for( i=0; i<stc->teamCount; i++ ){
		dscw->tact = stc->p_tact[i];

		resetStatus( stc->p_tact[i] );
		ProcessingAllTeamSummonObject( stc->p_tc[i], DrawStatusSummonCallBack, (void*)dscw );
		ProcessingAllTeamPlayer( stc->p_tc[i], DrawStatusPlayerCallBack, (void*)dscw );
	}
	GFL_HEAP_FreeMemory( dscw ); 

	if( stc->reloadFlag == TRUE ){
		stc->reloadFlag = FALSE;
	}
}

//------------------------------------------------------------------
static void	DrawStatusSummonCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work )
{
	DRAWSTATUS_CALLBACK_WORK* dscw = (DRAWSTATUS_CALLBACK_WORK*)work;
	VecFx32 trans;
	int hp, hpMax;
	BOOL	drawFlag;

	GetSummonObjectDrawSW( tc, summonID, &drawFlag );
	if( drawFlag == FALSE ){
		return;
	}
	GetSummonObjectTrans( tc, summonID, &trans );
	trans.y += FX32_ONE*22;

	drawFlag = setScreenPos( dscw->tact->summon[num].clact, &trans, dscw->cc );
	GetSummonObjectHP( tc, summonID, &hp, &hpMax );
	if( hp <= 0 ){
		drawFlag = FALSE;
	}
	if( drawFlag == TRUE ){
		StatusDraw( &dscw->tact->summon[num], summonName, 
					hp, hpMax, STATUS_DRAW_FULL, dscw->bmpTmp );
		GFL_CLACT_WK_SetDrawFlag( dscw->tact->summon[num].clact, TRUE );
	}
}

static void	DrawStatusPlayerCallBack( PLAYER_CONTROL* pc, int num, void* work )
{
	DRAWSTATUS_CALLBACK_WORK* dscw = (DRAWSTATUS_CALLBACK_WORK*)work;
	VecFx32 trans;
	PLAYER_STATUS* ps = GetPlayerStatusPointer( pc );
	BOOL	drawFlag;

	if( pc == dscw->myPc ){
		GFL_CLACTPOS pos;
		pos.x = 48;
		pos.y = 192-16;
//		pos.x = 0x80;
//		pos.y = 0x40;
		GFL_CLACT_WK_SetPos( dscw->tact->player[num].clact, &pos, CLSYS_DRAW_MAIN );
		StatusDraw( &dscw->tact->player[num], ps->name, 
					ps->hp, ps->hpMax, STATUS_DRAW_FULL, dscw->bmpTmp );
		GFL_CLACT_WK_SetDrawFlag( dscw->tact->player[num].clact, TRUE );
	} else {
		GetPlayerControlTrans( pc, &trans );
		trans.y += FX32_ONE*22;

		drawFlag = setScreenPos( dscw->tact->player[num].clact, &trans, dscw->cc );
		if( drawFlag == TRUE ){
			StatusDraw( &dscw->tact->player[num], ps->name, 
						ps->hp, ps->hpMax, STATUS_DRAW_FULL, dscw->bmpTmp );
			GFL_CLACT_WK_SetDrawFlag( dscw->tact->player[num].clact, TRUE );
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	ステータス描画リセット
 */
//------------------------------------------------------------------
static void resetStatus( STATUS_CLACT_P* tact )
{
	int i;

	for( i=0; i<TEAM_SUMMON_COUNT_MAX; i++ ){
		GFL_CLACT_WK_SetDrawFlag( tact->summon[i].clact, FALSE );
	}
	for( i=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){
		GFL_CLACT_WK_SetDrawFlag( tact->player[i].clact, FALSE );
	}
}

//------------------------------------------------------------------
/**
 * @brief	ステータススクリーン座標セット
 */
//------------------------------------------------------------------
static BOOL setScreenPos( GFL_CLWK* clact, VecFx32* trans, CAMERA_CONTROL* cc )
{

	if( CullingCameraBitween( cc, trans, 0x2000 ) == TRUE ){
		int	scalar;

		GetCameraControlDotProduct( cc, trans, &scalar );
		if(( scalar > 0 )&&( scalar < 0x3000 )){
			int				scrx, scry;
			GFL_CLACTPOS	pos;

			//３Ｄ→２Ｄスクリーン座標変換
			NNS_G3dWorldPosToScrPos( trans, &scrx, &scry );
			pos.x = scrx;
			pos.y = scry;
			if(( scry >= 0 )&&( scry <= 192 )){
				GFL_CLACT_WK_SetPos( clact, &pos, CLSYS_DRAW_MAIN );
			}
			return TRUE;
		}
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 * @brief	
 */
//------------------------------------------------------------------





//------------------------------------------------------------------
/**
 * @brief	ステータス転送(OBJsize = 8*2 chr)
 */
//------------------------------------------------------------------
#define SPRIT_DATSIZ	(4*32)
static void GetStatusCgx( u32 resID, GFL_BMP_DATA* bmp )
{
	u8* dst = GFL_BMP_GetCharacterAdrs( bmp );
	u8* src = GFL_OBJGRP_GetVramTransCGRPointer( resID );

	GFL_STD_MemCopy32( src + 0*SPRIT_DATSIZ,  dst + 0*SPRIT_DATSIZ, SPRIT_DATSIZ );
	GFL_STD_MemCopy32( src + 2*SPRIT_DATSIZ,  dst + 1*SPRIT_DATSIZ, SPRIT_DATSIZ );
	GFL_STD_MemCopy32( src + 1*SPRIT_DATSIZ,  dst + 2*SPRIT_DATSIZ, SPRIT_DATSIZ );
	GFL_STD_MemCopy32( src + 3*SPRIT_DATSIZ,  dst + 3*SPRIT_DATSIZ, SPRIT_DATSIZ );
}

static void SetStatusCgx( u32 resID, GFL_BMP_DATA* bmp )
{
	u8* src = GFL_BMP_GetCharacterAdrs( bmp );
	u8* dst = GFL_OBJGRP_GetVramTransCGRPointer( resID );

	GFL_STD_MemCopy32( src + 0*SPRIT_DATSIZ, dst + 0*SPRIT_DATSIZ, SPRIT_DATSIZ );
	GFL_STD_MemCopy32( src + 2*SPRIT_DATSIZ, dst + 1*SPRIT_DATSIZ, SPRIT_DATSIZ );
	GFL_STD_MemCopy32( src + 1*SPRIT_DATSIZ, dst + 2*SPRIT_DATSIZ, SPRIT_DATSIZ );
	GFL_STD_MemCopy32( src + 3*SPRIT_DATSIZ, dst + 3*SPRIT_DATSIZ, SPRIT_DATSIZ );
}

//------------------------------------------------------------------
/**
 * @brief	ステータスバー描画
 */
//------------------------------------------------------------------
#define BER_LENGTH		(1+(6*8)+1)
#define BER_POSX		(7)
#define BER_POSY		(12)
#define BER_HEIGHT		(2)
#define BER_COLOR_LIFE	(4)
#define BER_COLOR_LOST	(3)
static void DrawStatusBer( GFL_BMP_DATA* bmp, int hpNow, int hpMax )
{
	int width = hpNow * BER_LENGTH / hpMax;

	GF_ASSERT( width <= BER_LENGTH );

	GFL_BMP_Fill( bmp, BER_POSX, BER_POSY, width, BER_HEIGHT, BER_COLOR_LIFE );
	if( width < BER_LENGTH ){
		GFL_BMP_Fill(	bmp, BER_POSX + width, BER_POSY, 
						BER_LENGTH - width, BER_HEIGHT, BER_COLOR_LOST );
	}
}

//------------------------------------------------------------------
/**
 * @brief	ステータス描画
 */
//------------------------------------------------------------------
static const GFL_TEXT_PRINTPARAM initPrintParam = { NULL, 1, 1, 1, 1, 15, 0, GFL_TEXT_WRITE_16 };

//------------------------------------------------------------------
static void StatusDraw
		( CLACT_GRP* clactGrp, const char* name, int hp, int hpMax, int mode, GFL_BMP_DATA* bmp )
{
	GetStatusCgx( clactGrp->resCgx, bmp );

	if( mode & STATUS_DRAW_NAME ){
		GFL_TEXT_PRINTPARAM printParam  = initPrintParam;

		printParam.bmp = bmp;
		printParam.writex = 1;
		printParam.writey = 1;
		GFL_TEXT_PrintSjisCode( name, &printParam );
	}
	if( mode & STATUS_DRAW_BER ){
		DrawStatusBer( bmp, hp, hpMax );
	}
	SetStatusCgx( clactGrp->resCgx, bmp );

	GFL_CLACT_WK_SetAnmSeq( clactGrp->clact, 1 );	//ループアニメ
}



//------------------------------------------------------------------
/**
 * @brief	デバッグ用
 */
//------------------------------------------------------------------
void ChangeStatusControl( STATUS_CONTROL* stc , PLAYER_CONTROL* pc )
{
	stc->myPc = pc;
}


