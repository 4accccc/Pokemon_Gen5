//============================================================================================
/**
 * @file	text_cont.c
 * @brief	テキスト生成
 */
//============================================================================================
#include "gflib.h"
#include "textprint.h"

#include "setup.h"
#include "team_cont.h"
#include "text_cont.h"

static char*	SetNumSpc( char* dst, u16 num, BOOL* f );
static char*	Copy2Dtext( const char* src, char* dst );
static char*	Make2Dtext_SetNumber_u16( char* dst, u16 value );
//============================================================================================
//
//
//	ステータスウインドウコントロール
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _STATUSWIN_CONTROL {
	HEAPID			heapID;
	GFL_BMPWIN*		bmpwin;
	TEAM_CONTROL**	p_tc;
	int				teamCount;
	BOOL			reloadFlag;
};

//------------------------------------------------------------------
/**
 * @brief	ステータスウインドウコントロール起動
 */
//------------------------------------------------------------------
STATUSWIN_CONTROL* AddStatusWinControl( STATUSWIN_SETUP* setup )
{
	STATUSWIN_CONTROL* swc = GFL_HEAP_AllocClearMemory( setup->heapID, sizeof(STATUSWIN_CONTROL) );
	TEAM_CONTROL** p_tc = GFL_HEAP_AllocClearMemory
							( setup->heapID, setup->teamCount * sizeof(TEAM_CONTROL*) );
	int i;

	for( i=0; i<setup->teamCount; i++ ){
		p_tc[i] = setup->p_tc[i];
	}

	swc->heapID	= setup->heapID;
	swc->bmpwin	= setup->targetBmpwin;
	swc->p_tc = p_tc;
	swc->teamCount = setup->teamCount;
	swc->reloadFlag = TRUE;

	return swc;
}

//------------------------------------------------------------------
/**
 * @brief	ステータスウインドウコントロール終了
 */
//------------------------------------------------------------------
void RemoveStatusWinControl( STATUSWIN_CONTROL* swc )
{
	GFL_HEAP_FreeMemory( swc->p_tc ); 
	GFL_HEAP_FreeMemory( swc ); 
}

//------------------------------------------------------------------
/**
 * @brief	ステータスウインドウメインコントロール
 */
//------------------------------------------------------------------
typedef struct {
	u8 px;
	u8 py;
	u8 sx;
	u8 sy;
}WINPOS_DATA;

static const WINPOS_DATA winpostable[] = {
	{ 0*8, 0*8, 12*8, 5*8 },
	{ 0*8, 6*8, 12*8, 5*8 },
	{ 0*8, 12*8, 12*8, 5*8 },
};

static void TeamStatusDraw( GFL_BMP_DATA* bmp, TEAM_CONTROL* tc, WINPOS_DATA* wd, HEAPID heapID );

void MainStatusWinControl( STATUSWIN_CONTROL* swc )
{
	if( swc->reloadFlag == TRUE ){
		int i;
		GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( swc->bmpwin );
		//背景塗りつぶし
		GFL_BMP_Clear( bmp, 0 );

		GF_ASSERT( swc->teamCount <= 3 );	//とりあえず領域が３つ分しかないので

		for( i=0; i<swc->teamCount; i++ ){
			GFL_BMP_Fill(	bmp, 
							winpostable[i].px, winpostable[i].py,
							winpostable[i].sx, winpostable[i].sy,
							PLT_2D_COL_WHITE );
			TeamStatusDraw( bmp, swc->p_tc[i], (WINPOS_DATA*)&winpostable[i], swc->heapID );
		}

		//ビットマップキャラクターをアップデート
		GFL_BMPWIN_TransVramCharacter( swc->bmpwin );
		swc->reloadFlag = FALSE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	リロードセット
 */
//------------------------------------------------------------------
void SetStatusWinReload( STATUSWIN_CONTROL* swc )
{
	swc->reloadFlag = TRUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static const char	hpMsg[]	= {"HP"};
static const char	castleName[] = {"Castle"};

//------------------------------------------------------------------
/**
 * @brief	プレーヤーステータス描画
 */
//------------------------------------------------------------------
static void TeamStatusDraw( GFL_BMP_DATA* bmp, TEAM_CONTROL* tc, WINPOS_DATA* wd, HEAPID heapID )
{
	int hp, hpMax;

	char* text = GFL_HEAP_AllocClearMemory( heapID, 32 );
	char* p_text;
	GFL_TEXT_PRINTPARAM printParam = { NULL, 1, 1, 1, 1, 
										PLT_2D_COL_BLACK, PLT_2D_COL_WHITE, GFL_TEXT_WRITE_16 };

	printParam.bmp = bmp;

	//テキストをビットマップに表示
	if( GetCastleHP( tc, &hp, &hpMax ) == TRUE ){
		p_text = Copy2Dtext( hpMsg, text );
		p_text = Make2Dtext_SetNumber_u16( p_text, hp );

		printParam.writex = wd->px + 1;
		printParam.writey = wd->py + 1;
		GFL_TEXT_PrintSjisCode( castleName, &printParam );

		printParam.writex = wd->px + 1 + (8*6);
		printParam.writey = wd->py + 1;
		GFL_TEXT_PrintSjisCode( text, &printParam );
	}
	GFL_HEAP_FreeMemory( text );
}





//============================================================================================
//
//
//	メッセージウインドウコントロール
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
struct _MSGWIN_CONTROL {
	MSGWIN_SETUP	setup;
	BOOL			reloadFlag;
};

//------------------------------------------------------------------
/**
 * @brief	メッセージウインドウコントロール起動
 */
//------------------------------------------------------------------
MSGWIN_CONTROL* AddMessageWinControl( MSGWIN_SETUP* setup )
{
	MSGWIN_CONTROL* mwc = GFL_HEAP_AllocClearMemory( setup->heapID, sizeof(MSGWIN_CONTROL) );
	mwc->setup = *setup;

	PutMessageWin( mwc, GMSG_NULL );
	return mwc;
}

//------------------------------------------------------------------
/**
 * @brief	メッセージウインドウコントロール終了
 */
//------------------------------------------------------------------
void RemoveMessageWinControl( MSGWIN_CONTROL* mwc )
{
	GFL_HEAP_FreeMemory( mwc ); 
}

//------------------------------------------------------------------
/**
 * @brief	メッセージウインドウメインコントロール
 */
//------------------------------------------------------------------
void MainMessageWinControl( MSGWIN_CONTROL* mwc )
{
	if( mwc->reloadFlag == TRUE ){
		//ビットマップキャラクターをアップデート
		GFL_BMPWIN_TransVramCharacter( mwc->setup.targetBmpwin );

		mwc->reloadFlag = FALSE;
	}
}

//------------------------------------------------------------------
/**
 * @brief	メッセージセット
 */
//------------------------------------------------------------------
static const char*	gameMessage[] = {
	{""},
	{"Xボタンで　きょてんを　せっちしましょう"},
	{"スタート"},
	{"ひきわけ　です"},
	{"かち　です"},
	{"まけ　です"},
	{"ここは　べつのチームのエリアとかんしょうしています"},
	{"ここは　じぶんのチームのエリアではありません"},
	{"これいじょう　しょうかんできません"},
};

void PutMessageWin( MSGWIN_CONTROL* mwc, MSGID msgID )
{
	GFL_TEXT_PRINTPARAM printParam = { NULL, 1, 1, 1, 1, 
										PLT_2D_COL_BLACK, PLT_2D_COL_WHITE, GFL_TEXT_WRITE_16 };
	GFL_BMP_DATA* bmp = GFL_BMPWIN_GetBmp( mwc->setup.targetBmpwin );

	GFL_BMP_Clear( bmp, PLT_2D_COL_WHITE );

	printParam.bmp = bmp;
	printParam.writex = 1;
	printParam.writey = 1;// + (9*i);

	GF_ASSERT( msgID < NELEMS(gameMessage) );
	GFL_TEXT_PrintSjisCode( gameMessage[msgID], &printParam );

	mwc->reloadFlag = TRUE;
}





//============================================================================================
//
//
//	マップウインドウコントロール
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	構造体定義
 */
//------------------------------------------------------------------
#define PLAYER_SETUP_MAX	(8)
#define TOWER_COUNT_MAX		(15)

typedef struct {
	GFL_CLWK*		castle;
	GFL_CLWK*		summon[TEAM_SUMMON_COUNT_MAX];
	GFL_CLWK*		player[TEAM_PLAYER_COUNT_MAX];
}MAPWIN_CLACT_P;

struct _MAPWIN_CONTROL {
	HEAPID				heapID;
	GFL_BMPWIN*			bmpwin;
	TEAM_CONTROL**		p_tc;
	MAPWIN_CLACT_P**	p_tact;
	int					teamCount;
	PLAYER_CONTROL*		myPc;
	TEAM_CONTROL*		myTc;

	GFL_CLUNIT*			mapIconUnit;
	u32					mapIconResCgx;
	u32					mapIconResCel;
	u32					mapIconResPlt1;
	u32					mapIconResPlt2;

	BOOL				reloadFlag;
};

// ｘｙ座標、アニメーションシーケンス、ソフト優先順位(0>0xff)、BG優先順位
static const GFL_CLWK_DATA clwkData = { 0, 0, 0, 0, 1 }; 

static void resetMapIcon( MAPWIN_CLACT_P* tact );
static void setMapPos( GFL_CLWK* clact, VecFx32* trans );
//------------------------------------------------------------------
/**
 * @brief	マップウインドウコントロール起動
 */
//------------------------------------------------------------------
MAPWIN_CONTROL* AddMapWinControl( MAPWIN_SETUP* setup )
{
	int i, j;
	MAPWIN_CONTROL*		mpwc = GFL_HEAP_AllocClearMemory( setup->heapID, sizeof(MAPWIN_CONTROL) );
	TEAM_CONTROL**		p_tc = GFL_HEAP_AllocClearMemory
							( setup->heapID, setup->teamCount * sizeof(TEAM_CONTROL*) );
	MAPWIN_CLACT_P**	p_tact = GFL_HEAP_AllocClearMemory
							( setup->heapID, setup->teamCount * sizeof(MAPWIN_CLACT_P*) );

	mpwc->heapID	= setup->heapID;
	mpwc->bmpwin	= setup->targetBmpwin;
	mpwc->p_tc		= p_tc;
	mpwc->p_tact	= p_tact;
	mpwc->teamCount = setup->teamCount;
	mpwc->myPc		= setup->myPc;
	mpwc->myTc		= setup->myTc;

	mpwc->mapIconUnit = Get_GS_ClactUnit( setup->gs, CLACTUNIT_MAPOBJ );
	mpwc->mapIconResCgx = Get_GS_ClactResIdx( setup->gs, CLACTRES_MAPOBJ_CGX );
	mpwc->mapIconResCel = Get_GS_ClactResIdx( setup->gs, CLACTRES_MAPOBJ_CEL );
	mpwc->mapIconResPlt1 = Get_GS_ClactResIdx( setup->gs, CLACTRES_MAPOBJ_PLT_R );
	mpwc->mapIconResPlt2 = Get_GS_ClactResIdx( setup->gs, CLACTRES_MAPOBJ_PLT_B );

	for( i=0; i<mpwc->teamCount; i++ ){
		MAPWIN_CLACT_P*	tact;
		u32 plt;

		tact = GFL_HEAP_AllocClearMemory( mpwc->heapID, sizeof(MAPWIN_CLACT_P) );

		p_tc[i] = setup->p_tc[i];
		p_tact[i] = tact;

		if( i&1 ){	//とりあえず２色しか用意していないので２つに分類
			plt = mpwc->mapIconResPlt2;
		} else {
			plt = mpwc->mapIconResPlt1;
		}
		{
			tact->castle = GFL_OBJGRP_CreateClAct( mpwc->mapIconUnit, 
											mpwc->mapIconResCgx, plt, mpwc->mapIconResCel,
											&clwkData, CLSYS_DRAW_SUB, mpwc->heapID );
			GFL_CLACT_WK_SetAnmSeq( tact->castle, 2 );
			GFL_CLACT_WK_SetBgPri( tact->castle, 0 );
		}
		for( j=0; j<TEAM_SUMMON_COUNT_MAX; j++ ){
			tact->summon[j] = GFL_OBJGRP_CreateClAct( mpwc->mapIconUnit, 
											mpwc->mapIconResCgx, plt, mpwc->mapIconResCel,
											&clwkData, CLSYS_DRAW_SUB, mpwc->heapID );
			GFL_CLACT_WK_SetAnmSeq( tact->summon[j], 1 );
		}
		for( j=0; j<TEAM_PLAYER_COUNT_MAX; j++ ){
			tact->player[j] = GFL_OBJGRP_CreateClAct( mpwc->mapIconUnit, 
											mpwc->mapIconResCgx, plt, mpwc->mapIconResCel,
											&clwkData, CLSYS_DRAW_SUB, mpwc->heapID );
			GFL_CLACT_WK_SetAnmSeq( tact->player[j], 0 );
		}
		resetMapIcon( tact );
	}

	mpwc->reloadFlag = TRUE;

	return mpwc;
}

//------------------------------------------------------------------
/**
 * @brief	マップウインドウコントロール終了
 */
//------------------------------------------------------------------
void RemoveMapWinControl( MAPWIN_CONTROL* mpwc )
{
	int i, j;

	for( i=0; i<mpwc->teamCount; i++ ){
		MAPWIN_CLACT_P*	tact = mpwc->p_tact[i];

		for( j=0; j<TEAM_PLAYER_COUNT_MAX; j++ ){
			GFL_CLACT_WK_Remove( tact->player[j] );
		}
		for( j=0; j<TEAM_SUMMON_COUNT_MAX; j++ ){
			GFL_CLACT_WK_Remove( tact->summon[j] );
		}
		{
			GFL_CLACT_WK_Remove( tact->castle );
		}
		GFL_HEAP_FreeMemory( tact ); 
	}
	GFL_HEAP_FreeMemory( mpwc->p_tact ); 
	GFL_HEAP_FreeMemory( mpwc->p_tc ); 
	GFL_HEAP_FreeMemory( mpwc ); 
}

//------------------------------------------------------------------
/**
 * @brief	マップウインドウメインコントロール
 */
//------------------------------------------------------------------
typedef struct {
	MAPWIN_CLACT_P*	tact;
	PLAYER_CONTROL*	myPc;
	TEAM_CONTROL*	myTc;
}MAPWIN_CALLBACK_WORK;

static void	MapWinSummonDrawCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work );
static void	MapWinPlayerDrawCallBack( PLAYER_CONTROL* pc, int num, void* work );

void MainMapWinControl( MAPWIN_CONTROL* mpwc )
{
	int i, j;
	MAPWIN_CALLBACK_WORK* mpcw = GFL_HEAP_AllocClearMemory
									( mpwc->heapID, sizeof(MAPWIN_CALLBACK_WORK) );
	VecFx32 trans;

	mpcw->myPc = mpwc->myPc;
	mpcw->myTc = mpwc->myTc;

	for( i=0; i<mpwc->teamCount; i++ ){
		mpcw->tact = mpwc->p_tact[i];

		resetMapIcon( mpcw->tact );
		{
			if( GetCastleTrans( mpwc->p_tc[i], &trans ) == TRUE ){
				setMapPos( mpcw->tact->castle, &trans );
				GFL_CLACT_WK_SetDrawFlag( mpcw->tact->castle, TRUE );
			}
		}
		ProcessingAllTeamSummonObject( mpwc->p_tc[i], MapWinSummonDrawCallBack, (void*)mpcw );
		ProcessingAllTeamPlayer( mpwc->p_tc[i], MapWinPlayerDrawCallBack, (void*)mpcw );
	}
	GFL_HEAP_FreeMemory( mpcw ); 

	if( mpwc->reloadFlag == TRUE ){
		//ビットマップキャラクターをアップデート
		GFL_BMPWIN_TransVramCharacter( mpwc->bmpwin );

		mpwc->reloadFlag = FALSE;
	}
}

//------------------------------------------------------------------
static void	MapWinSummonDrawCallBack( TEAM_CONTROL* tc, int summonID, int num, void* work )
{
	MAPWIN_CALLBACK_WORK* mpcw = (MAPWIN_CALLBACK_WORK*)work;
	VecFx32 trans;

	GetSummonObjectTrans( tc, summonID, &trans );
	setMapPos( mpcw->tact->summon[num], &trans );
	GFL_CLACT_WK_SetDrawFlag( mpcw->tact->summon[num], TRUE );
}

static void	MapWinPlayerDrawCallBack( PLAYER_CONTROL* pc, int num, void* work )
{
	MAPWIN_CALLBACK_WORK* mpcw = (MAPWIN_CALLBACK_WORK*)work;
	VecFx32 trans;

	if( mpcw->myPc == pc ){
		GFL_CLACT_WK_SetBgPri( mpcw->tact->player[num], 0 );
	} else {
		GFL_CLACT_WK_SetBgPri( mpcw->tact->player[num], 1 );
	}
	GetPlayerControlTrans( pc, &trans );
	setMapPos( mpcw->tact->player[num], &trans );
	GFL_CLACT_WK_SetDrawFlag( mpcw->tact->player[num], TRUE );
}

//------------------------------------------------------------------
/**
 * @brief	マップウインドウアイコン描画初期化
 */
//------------------------------------------------------------------
static void resetMapIcon( MAPWIN_CLACT_P* tact )
{
	int i;

	GFL_CLACT_WK_SetDrawFlag( tact->castle, FALSE );

	for( i=0; i<TEAM_SUMMON_COUNT_MAX; i++ ){
		GFL_CLACT_WK_SetDrawFlag( tact->summon[i], FALSE );
	}
	for( i=0; i<TEAM_PLAYER_COUNT_MAX; i++ ){
		GFL_CLACT_WK_SetDrawFlag( tact->player[i], FALSE );
	}
}

//------------------------------------------------------------------
/**
 * @brief	マップウインドウ座標セット
 */
//------------------------------------------------------------------
#define MAPCLACT_OFFSX (8)
#define MAPCLACT_OFFSY (8)
static void setMapPos( GFL_CLWK* clact, VecFx32* trans )
{
	GFL_CLACTPOS	pos;
	u16 posx, posy;

	GetMapAreaMaskPos( trans, &posx, &posy );
	pos.x = posx + MAPCLACT_OFFSX;
	pos.y = posy + MAPCLACT_OFFSY;
	GFL_CLACT_WK_SetPos( clact, &pos, CLSYS_DRAW_SUB );
}

//------------------------------------------------------------------
/**
 * @brief	デバッグ用
 */
//------------------------------------------------------------------
void ChangeMapWinControl( MAPWIN_CONTROL* mpwc, PLAYER_CONTROL* pc, TEAM_CONTROL* tc )
{
	mpwc->myPc = pc;
	mpwc->myTc = tc;
}

//------------------------------------------------------------------
/**
 * @brief	
 */
//------------------------------------------------------------------





//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief		文字列作成
 */
//------------------------------------------------------------------
static char*	SetNumSpc( char* dst, u16 num, BOOL* f )
{
	if(( num == 0 )&&( *f == FALSE )){
		*dst = ' ';
		dst++;
	} else {
		*dst = '0' + num;
		*f = TRUE;
		dst++;
	}
	return dst;
}

static char*	Copy2Dtext( const char* src, char* dst )
{
	while( *src != 0 ){
		*dst = *src;
		src++;
		dst++;
	}
	*dst = 0;
	return dst;
}

static char*	Make2Dtext_SetNumber_u16( char* dst, u16 value )
{
	u16		num;
	BOOL	f = FALSE;

	num = value / 10000;
	dst = SetNumSpc( dst, num, &f );
	value %= 10000;

	num = value / 1000;
	dst = SetNumSpc( dst, num, &f );
	value %= 1000;

	num = value / 100;
	dst = SetNumSpc( dst, num, &f );
	value %= 100;

	num = value / 10;
	dst = SetNumSpc( dst, num, &f );
	value %= 10;

	num = value;
	f = TRUE;
	dst = SetNumSpc( dst, num, &f );
	*dst = 0;

	return dst;
}

