//============================================================================================
/**
 * @file	worldtrade_sublcd.c
 * @brief	世界交換サブ画面処理
 * @author	Akito Mori
 * @date	06.04.16
 */
//============================================================================================
#include <gflib.h>
#include <dwc.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include <dpw_tr.h>
#include "print/wordset.h"
#include "message.naix"
#include "system/wipe.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"

#include "savedata/wifilist.h"

#include "net_app/union/union_beacon_tool.h"
#include "net_app/worldtrade.h"
#include "worldtrade_local.h"

#include "record_gra.naix"			// グラフィックアーカイブ定義


//==============================================================================
// 構造体定義
//==============================================================================
typedef struct{
	int seq;
	int heroy;
	int sex;
	WORLDTRADE_WORK *wk;
}HERO_DEMO_WORK;


//==============================================================================
// プロトタイプ宣言
//==============================================================================
static void HeroDemoTask(  GFL_TCB* tcb, void *work  );
static void HeroReturnDemoTask(  GFL_TCB* tcb, void *work  );
static void SetSubLcdObj_Pos( GFL_CLWK* act, int x, int y );
static void HeroAnimeChange( HERO_DEMO_WORK *hdw, int animeno );
static void LoadFieldObjData( WORLDTRADE_WORK *wk );
static void TransFieldObjData( NNSG2dCharacterData *CharaData, NNSG2dPaletteData *PalData, int id, int view, int sex );
static int ObjAppearNoGet( int objno );



//==============================================================================
// 定数定義
//==============================================================================

// 主人公の表示位置
#define HERO_START_POSX	( 128 )
#define HERO_START_POSY	( -40 )

// 落ちてくる時の登場位置
#define HERO_LAND_POSY	( 160 )

// アニメ終了ライン
#define HERO_STOP_POSY	( 130 + 8 ) //SetSubLcdObj_Pos内で-8しているのでその分

// 交換相手の7人の座標定義
#define OBJPOS1_X	( 128 )
#define OBJPOS2_X	(  88+8 )
#define OBJPOS3_X	( 168-8 )
#define OBJPOS4_X	(  56+8 )
#define OBJPOS5_X	( 200-8 )
#define OBJPOS6_X	(  40+8 )
#define OBJPOS7_X	( 216-8 )

#define OBJPOS1_Y	( 40 +16-2 )
#define OBJPOS2_Y	( 44 +16-5 )
#define OBJPOS3_Y	( 56 +16-2 )
#define OBJPOS4_Y	( 88 +16-2 )

// 人間OBJの開始セル番号（アニメする）
#define FIELD_OBJ_NO_START			( 14 )
// 人間OBJの開始セル番号（アニメしない）
#define FIELD_OBJ_STAND_NO_START	( 17 )


// 人物OBJキャラグラフィックファイルのバイト数
#define FIELDOBJ_NCG_SIZE	(256*256/2)

//==============================================================================
// 検索結果ＯＢＪの座標
//==============================================================================
static const u16 obj_postable[][2]={
	{ OBJPOS1_X,  OBJPOS1_Y },
	{ OBJPOS2_X,  OBJPOS2_Y },
	{ OBJPOS3_X,  OBJPOS2_Y },
	{ OBJPOS4_X,  OBJPOS3_Y },
	{ OBJPOS5_X,  OBJPOS3_Y },
	{ OBJPOS6_X,  OBJPOS4_Y },
	{ OBJPOS7_X,  OBJPOS4_Y },
};


//==============================================================================
/**
 * @brief   下画面セルアクター表示登録
 *
 * @param   wk		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_SubLcdActorAdd( WORLDTRADE_WORK *wk, int sex )
{
	int i;
	GFL_CLWK_DATA	add;

	
	// ついでにフィールドＯＢＪグラフィック読み込み
	LoadFieldObjData( wk );

#if 0 //新CLACTではヘッダは要らない
	WorldTrade_MakeCLACT( &add,  wk, &wk->clActHeader_sub, NNS_G2D_VRAM_TYPE_2DSUB );
#endif
	// 主人公アクター登録
	add.pos_x = HERO_START_POSX;
	add.pos_y = ( 130 );// + NAMEIN_SUB_ACTOR_DISTANCE;
	add.anmseq	= 0;
	add.softpri	= 0;
	add.bgpri		= 0;
	wk->SubActWork[0] = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[SUB_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[SUB_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[SUB_LCD][CLACT_U_CELL_RES],	
			&add,CLSYS_DRAW_SUB, HEAPID_WORLDTRADE);
	GFL_CLACT_WK_SetAutoAnmFlag( wk->SubActWork[0],1);
	GFL_CLACT_WK_SetBgPri( wk->SubActWork[0], 2 );
	
	// 上を向いて立つ
	GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[0], 3+sex*7 );
	GFL_CLACT_WK_SetDrawEnable( wk->SubActWork[0], 1 );
	
	// 検索結果ＯＢＪ登録＆隠す
	for(i=0;i<SEARCH_POKE_MAX;i++){
		wk->SubActWork[i+1] = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[SUB_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[SUB_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[SUB_LCD][CLACT_U_CELL_RES],	
			&add,CLSYS_DRAW_SUB, HEAPID_WORLDTRADE);

		GFL_CLACT_WK_SetAutoAnmFlag(wk->SubActWork[i+1],1);
		GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[i+1], FIELD_OBJ_NO_START+i*4 );
		GFL_CLACT_WK_SetDrawEnable( wk->SubActWork[i+1], 0 );
		SetSubLcdObj_Pos( wk->SubActWork[i+1], obj_postable[i][0], obj_postable[i][1] );
		GFL_CLACT_WK_SetBgPri( wk->SubActWork[i+1], 2 );
	}

	// 交換相手を指定するためのカーソル登録＆隠す
	wk->PartnerCursorActWork = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[SUB_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[SUB_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[SUB_LCD][CLACT_U_CELL_RES],	
			&add,CLSYS_DRAW_SUB, HEAPID_WORLDTRADE);
	GFL_CLACT_WK_SetAutoAnmFlag(wk->PartnerCursorActWork,1);
	GFL_CLACT_WK_SetAnmSeq( wk->PartnerCursorActWork, 43 );
	GFL_CLACT_WK_SetDrawEnable( wk->PartnerCursorActWork, 0 );
	SetSubLcdObj_Pos( wk->PartnerCursorActWork, obj_postable[0][0], obj_postable[0][1]+32 );
	GFL_CLACT_WK_SetBgPri( wk->PartnerCursorActWork, 1 );

	// 「DSの下画面をみてね」アイコン
	wk->PromptDsActWork = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[SUB_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[SUB_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[SUB_LCD][CLACT_U_CELL_RES],	
			&add,CLSYS_DRAW_SUB, HEAPID_WORLDTRADE);
	GFL_CLACT_WK_SetAutoAnmFlag(wk->PromptDsActWork,1);
	GFL_CLACT_WK_SetAnmSeq( wk->PromptDsActWork, 42 );
	WorldTrade_CLACT_PosChangeSub( wk->PromptDsActWork, DS_ICON_X, DS_ICON_Y+wk->DrawOffset );
	GFL_CLACT_WK_SetDrawEnable( wk->PromptDsActWork, 0 );

	// 人物OBJのY座標を保存
	{
		GFL_CLACTPOS	pos;
		int i;
		for(i=0;i<SUB_OBJ_NUM;i++){
			GFL_CLACT_WK_GetPos( wk->SubActWork[i], &pos, CLSYS_DRAW_SUB );
			wk->SubActY[i][0] = pos.x;
			wk->SubActY[i][1] = pos.y;
		}
	}
}




//==============================================================================
/**
 * @brief   主人公登場デモ
 *
 * @param   wk		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_HeroDemo( WORLDTRADE_WORK *wk, int sex )
{
	
	// 主人公＆検索結果ＯＢＪ登録
	WorldTrade_SubLcdActorAdd( wk, sex );
	
	// デモタスク登録
	{
		HERO_DEMO_WORK *hdw;

		wk->task_wk	= GFL_HEAP_AllocMemory( HEAPID_WORLDTRADE, sizeof(HERO_DEMO_WORK) );
		wk->demotask	= GFL_TCB_AddTask( wk->tcbsys,  HeroDemoTask, wk->task_wk, 5 );
		hdw          = GFL_TCB_GetWork(wk->demotask);
		hdw->seq     = 0;
		hdw->heroy   = HERO_START_POSY;
		hdw->sex     = sex;
		hdw->wk      = wk;

		// くるくる回る
		HeroAnimeChange( hdw, 0 );
		PMSND_PlaySE( SE_GTC_PLAYER_IN );
	}
}


//------------------------------------------------------------------
/**
 * @brief   主人公の性別に合わせてアニメ変更をかける
 *
 * @param   wk		
 * @param   animeno		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void HeroAnimeChange( HERO_DEMO_WORK *hdw, int animeno )
{
	GFL_CLACT_WK_SetAnmSeq( hdw->wk->SubActWork[0], animeno+hdw->sex*7 );
}

enum{
	HERO_DEMO_FALL=0,
	HERO_DEMO_FALL_STOP,
	HERO_DEMO_WALK,
	HERO_DEMO_WALK_STOP,
};

//------------------------------------------------------------------
/**
 * @brief   主人公の動きタスク
 *          (くるくる降りてくる→ゆっくり止まる→上にむかって歩く）
 *
 * @param   tcb		
 * @param   work		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void HeroDemoTask(  GFL_TCB* tcb, void *work  )
{
	int i;
	HERO_DEMO_WORK *hdw = (HERO_DEMO_WORK *)work;
	WORLDTRADE_WORK *wk = hdw->wk;

	switch(hdw->seq){
	// 主人公落ちてくる
	case HERO_DEMO_FALL:
		if( hdw->heroy > HERO_LAND_POSY ){
			hdw->heroy = HERO_LAND_POSY;
			hdw->seq = HERO_DEMO_FALL_STOP;
			// 回転がゆっくりになって止まる
			HeroAnimeChange( hdw, 1 );
		}
		hdw->heroy+=5;
		SetSubLcdObj_Pos( wk->SubActWork[0], HERO_START_POSX, hdw->heroy );
		break;

	// 主人公回転終了
	case HERO_DEMO_FALL_STOP:
		if( !GFL_CLACT_WK_CheckAnmActive( wk->SubActWork[0] ) ){
			HeroAnimeChange( hdw, 2 );
			hdw->seq = HERO_DEMO_WALK;
		}
		break;

	// 上に向かって歩く
	case HERO_DEMO_WALK:
		if( hdw->heroy <= HERO_STOP_POSY ){
			hdw->heroy = HERO_STOP_POSY;
			hdw->seq = HERO_DEMO_WALK_STOP;
			// 回転がゆっくりになって止まる
			HeroAnimeChange( hdw, 3 );

		}
		else{
    		hdw->heroy-=2;
    	}
		SetSubLcdObj_Pos( wk->SubActWork[0], HERO_START_POSX, hdw->heroy );
		
		break;

	// デモ終了（たぶん機械のスイッチＯＮとかが走る）
	case HERO_DEMO_WALK_STOP:

		PMSND_PlaySE( SE_GTC_ON );
		hdw->wk->demo_end = 1;
		GFL_HEAP_FreeMemory( hdw );
		GFL_TCB_DeleteTask(tcb);

		break;
	}


}

//==============================================================================
/**
 * @brief   主人公さようならデモ
 *
 * @param   wk		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_ReturnHeroDemo( WORLDTRADE_WORK *wk, int sex )
{

	// デモタスク登録
	{
		HERO_DEMO_WORK *hdw;

#if 0
		wk->demotask = PMDS_taskAdd( HeroReturnDemoTask, sizeof(HERO_DEMO_WORK), 5, HEAPID_WORLDTRADE);
#else
		wk->task_wk	= GFL_HEAP_AllocMemory( HEAPID_WORLDTRADE, sizeof(HERO_DEMO_WORK) );
		wk->demotask	= GFL_TCB_AddTask( wk->tcbsys, HeroReturnDemoTask, wk->task_wk, 5 );
#endif
		hdw          = GFL_TCB_GetWork(wk->demotask);
		hdw->seq     = 0;
		hdw->heroy   = HERO_STOP_POSY;
		hdw->sex     = sex;
		hdw->wk      = wk;

		// 下に向かって歩く
		HeroAnimeChange( hdw, 5 );
		PMSND_PlaySE( SE_GTC_OFF );

	}
}


enum{
	R_HERO_DEMO_WALK=0,
	R_HERO_DEMO_WALK_STOP,
	R_HERO_DEMO_RISE,
	R_HERO_DEMO_RISE_END,
};

//------------------------------------------------------------------
/**
 * @brief   主人公の動きタスク
 *          (くるくる降りてくる→ゆっくり止まる→上にむかって歩く）
 *
 * @param   tcb		
 * @param   work		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void HeroReturnDemoTask(  GFL_TCB* tcb, void *work  )
{
	int i;
	HERO_DEMO_WORK *hdw = (HERO_DEMO_WORK *)work;
	WORLDTRADE_WORK *wk = hdw->wk;

	switch(hdw->seq){
	// 主人公下に向かって歩く
	case R_HERO_DEMO_WALK:
		if( hdw->heroy > HERO_LAND_POSY ){
			hdw->heroy = HERO_LAND_POSY;
			hdw->seq = R_HERO_DEMO_WALK_STOP;
			// 回転し始める
			HeroAnimeChange( hdw, 6 );
		}
		hdw->heroy+=2;
		SetSubLcdObj_Pos( wk->SubActWork[0], HERO_START_POSX, hdw->heroy );
		break;

	// 主人公回転し始める
	case R_HERO_DEMO_WALK_STOP:
		if( !GFL_CLACT_WK_CheckAnmActive( wk->SubActWork[0] ) ){
			HeroAnimeChange( hdw, 0 );
			hdw->seq = R_HERO_DEMO_RISE;
			PMSND_PlaySE( SE_GTC_PLAYER_OUT );
		}
		break;

	// 主人公登っていく
	case R_HERO_DEMO_RISE:
		if( hdw->heroy <-20 ){
			hdw->seq = R_HERO_DEMO_RISE_END;
			// もう画面外なのでなんでもいい
			HeroAnimeChange( hdw, 3 );
		}
		hdw->heroy-=5;
		SetSubLcdObj_Pos( wk->SubActWork[0], HERO_START_POSX, hdw->heroy );
		
		break;

	// 画面から消えたら隠して終了
	case R_HERO_DEMO_RISE_END:

		hdw->wk->demo_end = 1;
		GFL_HEAP_FreeMemory( hdw );
		GFL_TCB_DeleteTask(tcb);
		break;
	}


}



//------------------------------------------------------------------
/**
 * @brief   サブ画面アクターの座標を代入する
 *
 * @param   act		アクターのポインタ
 * @param   x		
 * @param   y		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void SetSubLcdObj_Pos( GFL_CLWK* act, int x, int y )
{

#if 0
	mat.x = FX32_CONST( x );
	mat.y = FX32_CONST( y - 8 ) + NAMEIN_SUB_ACTOR_DISTANCE;
	mat.z = 0;
#else
	GFL_CLACTPOS	pos;
	pos.x	= x;
	pos.y	= y - 8;// + NAMEIN_SUB_ACTOR_DISTANCE;
#endif
	GFL_CLACT_WK_SetPos( act, &pos, CLSYS_DRAW_SUB );
}

// 検索結果ＯＢＪをタッチするための座標テーブル
static const GFL_UI_TP_HITTBL obj_touchtbl[]={
	{	OBJPOS1_Y-16+32, OBJPOS1_Y+16+32, OBJPOS1_X-16, OBJPOS1_X+16,},
	{	OBJPOS2_Y-16+32, OBJPOS2_Y+16+32, OBJPOS2_X-16, OBJPOS2_X+16,},
	{	OBJPOS2_Y-16+32, OBJPOS2_Y+16+32, OBJPOS3_X-16, OBJPOS3_X+16,},
	{	OBJPOS3_Y-16+32, OBJPOS3_Y+16+32, OBJPOS4_X-16, OBJPOS4_X+16,},
	{	OBJPOS3_Y-16+32, OBJPOS3_Y+16+32, OBJPOS5_X-16, OBJPOS5_X+16,},
	{	OBJPOS4_Y-16+32, OBJPOS4_Y+16+32, OBJPOS6_X-16, OBJPOS6_X+16,},
	{	OBJPOS4_Y-16+32, OBJPOS4_Y+16+32, OBJPOS7_X-16, OBJPOS7_X+16,},
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
};

//------------------------------------------------------------------
/**
 * @brief   下画面のＯＢＪタッチチェック
 *
 * @param   max		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
int WorldTrade_SubLcdObjHitCheck( int max )
{
	int button=GFL_UI_TP_HitTrg( obj_touchtbl );
	if( button != GFL_UI_TP_HIT_NONE ){
		if(button<max){
			return button;
		}
	}
	

	return -1;
}

//==============================================================================
/**
 * @brief   検索結果の数だけＯＢＪを表示させる
 *
 * @param   wk		
 * @param   num		検索Hitした人数
 * @param   flag	0:登場アニメいらない	1:登場アニメさせる
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_SubLcdMatchObjAppear( WORLDTRADE_WORK *wk, int num, int flag )
{
	int i;

	// 登場SE
	if(num!=0 && flag==1){
		PMSND_PlaySE( SE_GTC_APPEAR );
	}

	// 検索人数分
	for(i=0;i<SEARCH_POKE_MAX;i++){

		if(i<num){
			// HITした方を登場させる
			int view = wk->DownloadPokemonData[i].trainerType;
			int sex  = wk->DownloadPokemonData[i].gender;

			OS_TPrintf("OBJ登場 view=%02x, sex=%d\n", view, sex);
		
			TransFieldObjData( wk->FieldObjCharaData, wk->FieldObjPalData, i, view, sex );

			// アニメさせるかさせないか
			if(flag){
				GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[i+1], FIELD_OBJ_NO_START+i*4 );
			}else{
				GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[i+1], FIELD_OBJ_STAND_NO_START+i*4 );
			}
			GFL_CLACT_WK_SetDrawEnable( wk->SubActWork[i+1], 1 );
		}else{

			// 表示OFF
			GFL_CLACT_WK_SetDrawEnable( wk->SubActWork[i+1], 0 );
		}
	}
	
}


//------------------------------------------------------------------
/**
 * @brief   ニトロキャラクター上でOBJが立っているセルアニメは何番か？
 *
 * @param   objno		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int ObjAppearNoGet( int objno )
{
	return FIELD_OBJ_NO_START + objno*4;
}

//==============================================================================
/**
 * @brief   検索結果ＯＢＪを全てひっこめる
 *
 * @param   none		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_SubLcdMatchObjHide( WORLDTRADE_WORK *wk )
{
	int i;
	for(i=0;i<7;i++){
		if(GFL_CLACT_WK_GetDrawEnable(wk->SubActWork[i+1])){
			GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[i+1], ObjAppearNoGet( i )+1 );
		}
	}
}


//------------------------------------------------------------------
/**
 * @brief   フィールドOBJ画像読み込み（ユニオンと自機）
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void LoadFieldObjData( WORLDTRADE_WORK *wk )
{
	// パレット読み込み
	wk->FieldObjPalBuf = GFL_ARC_UTIL_LoadPalette( ARCID_RECORD_GRA, NARC_record_gra_union_chara_NCLR, &(wk->FieldObjPalData), HEAPID_WORLDTRADE );

	// 画像読み込み
	wk->FieldObjCharaBuf = GFL_ARC_UTIL_LoadOBJCharacter( ARCID_RECORD_GRA, NARC_record_gra_union_chara_NCGR,  0, &(wk->FieldObjCharaData), HEAPID_WORLDTRADE );
	DC_FlushRange( wk->FieldObjCharaData, FIELDOBJ_NCG_SIZE );

}

#define OBJ_TRANS_SIZE	( 4*4 )
static const u16 obj_offset[]={
	( 32*9                    )*0x20,
	( 32*9 + OBJ_TRANS_SIZE*1 )*0x20,
	( 32*9 + OBJ_TRANS_SIZE*2 )*0x20,
	( 32*9 + OBJ_TRANS_SIZE*3 )*0x20,
	( 32*9 + OBJ_TRANS_SIZE*4 )*0x20,
	( 32*9 + OBJ_TRANS_SIZE*5 )*0x20,
	( 32*9 + OBJ_TRANS_SIZE*6 )*0x20,
};


//------------------------------------------------------------------
/**
 * @brief   渡されたユニオン見た目コードからフィールドOBJのキャラを転送する
 *
 * @param   id		
 * @param   view		
 * @param   sex		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void TransFieldObjData( NNSG2dCharacterData *CharaData, NNSG2dPaletteData *PalData, int id, int view, int sex )
{
	int pos;
	u8 *chara, *pal;
	
	// ユニオンキャラを転送

	pos   = UnionView_GetCharaNo( sex, view );

	chara = (u8*)CharaData->pRawData;
	pal   = (u8*)PalData->pRawData;

	GXS_LoadOBJ( &chara[(OBJ_TRANS_SIZE*3)*pos*0x20], obj_offset[id], OBJ_TRANS_SIZE*0x20 );
	GXS_LoadOBJPltt( &pal[pos*32], (id+2)*32, 32 );

	OS_Printf("ID=%d のユニオン見た目は %d アイコン番号は %d\n", id, view, pos);



	
	
}

//------------------------------------------------------------------
/**
 * @brief   フィールドOBJ画像解放
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
void FreeFieldObjData( WORLDTRADE_WORK *wk )
{
	if(wk->demo_end)
	{
		GFL_HEAP_FreeMemory( wk->FieldObjPalBuf  );
		GFL_HEAP_FreeMemory( wk->FieldObjCharaBuf );
	}
	OS_TPrintf( "palbuff	%x\n", wk->FieldObjPalBuf );
	OS_TPrintf( "charbuff	%x\n", wk->FieldObjCharaBuf );
}


//==============================================================================
/**
 * @brief   交換相手指定カーソルの座標変更
 *
 * @param   wk		
 * @param   index		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_SetPartnerCursorPos( WORLDTRADE_WORK *wk, int index, int offset_y )
{
	SetSubLcdObj_Pos( wk->PartnerCursorActWork, obj_postable[index][0], 
												obj_postable[index][1]+32+offset_y );
}

//==================================================================
/**
 * 交換に出す代わりのポケモンを選択している時のトレーナーOBJ座標を設定する
 *
 * @param   wk		
 */
//==================================================================
void WorldTrade_SetPartnerExchangePos( WORLDTRADE_WORK *wk )
{
    int i;
    
    for(i = 0; i < SUB_OBJ_NUM; i++){
        WorldTrade_CLACT_PosChangeSub( wk->SubActWork[i], wk->SubActY[i][0], wk->SubActY[i][1] );
    }
}

//==================================================================
/**
 * 交換に出す代わりのポケモンを選択している時のトレーナーOBJ座標を元に戻す
 *
 * @param   wk		
 */
//==================================================================
void WorldTrade_SetPartnerExchangePosIsReturns( WORLDTRADE_WORK *wk )
{
    int i;
    
	for(i=0;i<SUB_OBJ_NUM;i++){
		WorldTrade_CLACT_PosChangeSub( wk->SubActWork[i], wk->SubActY[i][0], wk->SubActY[i][1]+32 );
    }
}

