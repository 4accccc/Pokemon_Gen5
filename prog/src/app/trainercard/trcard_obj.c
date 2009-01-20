#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "arc_def.h"

#include "trcard_obj.h"
#include "trainer_case.naix"

#define MAIN_LCD	( GFL_BG_MAIN_DISP )	// 0
#define SUB_LCD		( GFL_BG_SUB_DISP )		// 1

#define TR_SUBSURFACE_Y	(32)

//** CharManager PlttManager用 **//
#define TR_CARD_CHAR_CONT_NUM				(2)
#define TR_CARD_CHAR_VRAMTRANS_MAIN_SIZE	(0)//(2048)
#define TR_CARD_CHAR_VRAMTRANS_SUB_SIZE		(0)
#define TR_CARD_PLTT_CONT_NUM				(2)
#define TR_CARD_PALETTE_NUM					(16)	//パレット9本
#define TR_CARD_BADGE_PALETTE_NUM			(4)	//パレット4本

#define BTN_EFF_POS_X	(12*8)
#define BTN_EFF_POS_Y	(17*8)


typedef struct BADGE_POS_tag{
	int x;
	int y;
}BADGE_POS;

//リソースマネージャー登録数テーブル
static const u8 ResEntryNumTbl[RESOURCE_NUM] = {
	1,			//キャラリソース
	2,			//パレットリソース
	1,			//セルリソース
	1,			//セルアニメリソース
};

#define BADGE_LX0	(8*12)
#define BADGE_LX1	(8*18)
#define BADGE_LX2	(8*24)
#define BADGE_LX3	(8*30)

#define BADGE_LY0	(8*6)
#define BADGE_LY1	(8*11)
#define BADGE_LY2	(8*17)
#define BADGE_LY3	(8*22)

#define BADGE_NC_OFS	(8*5)

static const BADGE_POS BadgePos[] = {
	{BADGE_LX0,BADGE_LY0},
	{BADGE_LX1,BADGE_LY0},
	{BADGE_LX2,BADGE_LY0},
	{BADGE_LX3,BADGE_LY0},
	{BADGE_LX0,BADGE_LY1},
	{BADGE_LX1,BADGE_LY1},
	{BADGE_LX2,BADGE_LY1},
	{BADGE_LX3,BADGE_LY1},

	{BADGE_LX0,BADGE_LY2},
	{BADGE_LX1,BADGE_LY2},
	{BADGE_LX2,BADGE_LY2},
	{BADGE_LX3,BADGE_LY2},
	{BADGE_LX0,BADGE_LY3},
	{BADGE_LX1,BADGE_LY3},
	{BADGE_LX2,BADGE_LY3},
	{BADGE_LX3,BADGE_LY3},
};

static const BADGE_POS StarPos[] = {
	{8*3,8*5},
	{8*10,8*5},
	{8*17,8*5},
	{8*24,8*5},
	{8*3,8*12},
	{8*10,8*12},
	{8*17,8*12},
	{8*24,8*12},
};

static void InitCharPlttManager(void);

//--------------------------------------------------------------------------------------------
/**
 * セルアクター初期化
 *
 * @param	wk	トレーナーカードOBJワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void InitTRCardCellActor( TR_CARD_OBJ_WORK *wk )
{
	int i;
//	initVramTransferManagerHeap( 32, HEAPID_TR_CARD );

	//メインのキャラ境界を128kbyteモードに変更
	GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_128K);	
	GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);	

	InitCharPlttManager();
	// OAMマネージャーの初期化
	NNS_G2dInitOamManagerModule();

	GFL_CLACT_Init( GFL_CLSYSINIT_DEF_DIVSCREEN , wk->heapId );
	// セルアクター初期化
	wk->cellUnit = GFL_CLACT_UNIT_Create( TR_CARD_ACT_MAX, wk->heapId );

	//サブサーフェースの距離を設定
//	CLACT_U_SetSubSurfaceMatrix(&wk->RendData,0,SUB_SURFACE_Y+FX32_CONST(TR_SUBSURFACE_Y));
	
	//リソースマネージャー初期化
//	for(i=0;i<RESOURCE_NUM;i++){		//リソースマネージャー作成
//		wk->ResMan[MAIN_LCD][i] = CLACT_U_ResManagerInit(ResEntryNumTbl[i], i, HEAPID_TR_CARD);
//		wk->ResMan[SUB_LCD][i] = CLACT_U_ResManagerInit(ResEntryNumTbl[i], i, HEAPID_TR_CARD);
//	}

	for(i = 0;i < 2;i++){
		int vram_type[2] = {
			NNS_G2D_VRAM_TYPE_2DMAIN,NNS_G2D_VRAM_TYPE_2DSUB,
		};
		u8 res_name[2][4] = {
		 {	NARC_trainer_case_card_badge_NCGR, 
			NARC_trainer_case_card_badge_NCLR, 
			NARC_trainer_case_card_badge_NCER, 
			NARC_trainer_case_card_badge_NANR },
		 {	NARC_trainer_case_card_button_NCGR, 
			NARC_trainer_case_card_button_NCLR, 
			NARC_trainer_case_card_button_NCER, 
			NARC_trainer_case_card_button_NANR },
		};
		//chara読み込み
		GFL_ARC_UTIL_TransVramCharacterMakeProxy( ARCID_TRAINERCARD , res_name[i][1] 
											, FALSE , MAP_TYPE_1D , 0 , vram_type[i] , 
											, wk->heapId , &charaData[i] );
//		wk->ResObjTbl[MAIN_LCD+i][CLACT_U_CHAR_RES] = GFL_ARC_UTIL_LoadOBJCharacter(
//											wk->ResMan[MAIN_LCD+i][CLACT_U_CHAR_RES],
//											ARCID_TRAINERCARD, res_name[i][CLACT_U_CHAR_RES],
//											FALSE, i, vram_type[i], HEAPID_TR_CARD);

		//pal読み込み
		GFL_ARC_UTIL_TransVramPaletteMakeProxy( ARCID_TRAINERCARD , res_name[i][0]
											, vram_type[i] , 
//		wk->ResObjTbl[MAIN_LCD+i][CLACT_U_PLTT_RES] = CLACT_U_ResManagerResAddArcPltt(
//											wk->ResMan[MAIN_LCD+i][CLACT_U_PLTT_RES],
//											ARCID_TRAINERCARD, res_name[i][CLACT_U_PLTT_RES],
//											FALSE, i, vram_type[i], 
//											TR_CARD_PALETTE_NUM, HEAPID_TR_CARD);

		//cell読み込み
		wk->ResObjTbl[MAIN_LCD+i][2] = GFL_ARC_UTIL_LoadCellBank( ARCID_TRAINERCARD
											, res_name[i][2] , FALSE , &wk->cellData[i] , wk->heapId );
//		wk->ResObjTbl[MAIN_LCD+i][CLACT_U_CELL_RES] = CLACT_U_ResManagerResAddArcKindCell(
//											wk->ResMan[MAIN_LCD+i][CLACT_U_CELL_RES],
//											ARCID_TRAINERCARD, res_name[i][CLACT_U_CELL_RES],
//											FALSE, i, CLACT_U_CELL_RES, HEAPID_TR_CARD);

		//同じ関数でanim読み込み
		wk->ResObjTbl[MAIN_LCD+i][3] = GFL_ARC_UTIL_LoadAnimeBank( ARCID_TRAINERCARD ,
											, res_name[i][3] , FALSE , &wk->anmData[i] , wk->heapId );
//		wk->ResObjTbl[MAIN_LCD+i][CLACT_U_CELLANM_RES] = CLACT_U_ResManagerResAddArcKindCell(
//											wk->ResMan[MAIN_LCD+i][CLACT_U_CELLANM_RES],
//											ARCID_TRAINERCARD, res_name[i][CLACT_U_CELLANM_RES],
//											FALSE, i, CLACT_U_CELLANM_RES, HEAPID_TR_CARD);

		// リソースマネージャーから転送
		//読み込みと同時に転送済み
		// Chara転送
//		CLACT_U_CharManagerSet( wk->ResObjTbl[MAIN_LCD+i][CLACT_U_CHAR_RES] );

		// パレット転送
//		CLACT_U_PlttManagerSet( wk->ResObjTbl[MAIN_LCD+i][CLACT_U_PLTT_RES] );
	}

	{
		u8 i;
		ARCHANDLE* pal_handle;

		const int pal_arc_idx[TR_CARD_BADGE_ACT_MAX] = {
			NARC_trainer_case_card_badge2_0_NCLR,
			NARC_trainer_case_card_badge2_1_NCLR,
			NARC_trainer_case_card_badge2_2_NCLR,
			NARC_trainer_case_card_badge2_3_NCLR,
			NARC_trainer_case_card_badge2_4_NCLR,
			NARC_trainer_case_card_badge2_5_NCLR,
			NARC_trainer_case_card_badge2_6_NCLR,
			NARC_trainer_case_card_badge2_7_NCLR
		};
		//パレットアーカイブハンドルオープン
		pal_handle  = ArchiveDataHandleOpen( ARCID_TRAINERCARD, HEAPID_TR_CARD);
		for (i=0;i<TR_CARD_BADGE_ACT_MAX;i++){
			wk->PalDataBuf[i] = ArchiveDataLoadAllocByHandle( pal_handle, pal_arc_idx[i], HEAPID_TR_CARD );
			if( wk->PalDataBuf[i] != NULL )
			{
				if( NNS_G2dGetUnpackedPaletteData( wk->PalDataBuf[i], &wk->PalData[i] ) == FALSE ){
					sys_FreeMemoryEz( wk->PalData[i] );
					GF_ASSERT(0);
				}
			}else{
				GF_ASSERT(0);
			}
		}
		//ハンドルクローズ
		ArchiveDataHandleClose( pal_handle );
	}
	
	GF_Disp_GX_VisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// MAIN DISP OBJ ON
	GF_Disp_GXS_VisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// SUB DISP OBJ ON
}

#if 0
#pragma mark [>end edit

//--------------------------------------------------------------------------------------------
/**
 * セルアクターをセット
 *
 * @param	wk				トレーナーカードOBJワーク
 * @param	inBadgeDisp		バッジ表示フラグリスト
 * @param	isClear			殿堂入りしているか？
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void SetTrCardActor( TR_CARD_OBJ_WORK *wk, const u8 *inBadgeDisp ,const BOOL isClear)
{
	int i;
	CLACT_HEADER			cl_act_header;
	
	// セルアクターヘッダ作成	
	CLACT_U_MakeHeader(	&cl_act_header,
						MAIN_LCD, MAIN_LCD, MAIN_LCD, MAIN_LCD,
						CLACT_U_HEADER_DATA_NONE, CLACT_U_HEADER_DATA_NONE,
						0, 1,
						wk->ResMan[MAIN_LCD][CLACT_U_CHAR_RES],
						wk->ResMan[MAIN_LCD][CLACT_U_PLTT_RES],
						wk->ResMan[MAIN_LCD][CLACT_U_CELL_RES],
						wk->ResMan[MAIN_LCD][CLACT_U_CELLANM_RES],
						NULL,NULL);

	{
		//登録情報格納
		u8 ofs;
		CLACT_ADD add;
		u16	badge_ofs = 0;

		add.ClActSet	= wk->ClactSet;
		add.ClActHeader	= &cl_act_header;

		add.mat.x		= 0;//FX32_CONST(32) ;
		add.mat.y		= 0;//FX32_CONST(96) ;		//画面は上下連続している
		add.mat.z		= 0;
		add.sca.x		= FX32_ONE;
		add.sca.y		= FX32_ONE;
		add.sca.z		= FX32_ONE;
		add.rot			= 0;
		add.pri			= 2;
		add.DrawArea	= NNS_G2D_VRAM_TYPE_2DMAIN;
		add.heap		= HEAPID_TR_CARD;

		//セルアクター表示開始
		if(!isClear){
			badge_ofs = BADGE_NC_OFS; 
		}

		// 下画面(メイン画面)
		//バッジ
		for(i=0;i<TR_CARD_BADGE_ACT_MAX;i++){
			add.mat.x = FX32_ONE * BadgePos[i].x;
			add.mat.y = FX32_ONE * (BadgePos[i].y+badge_ofs);
			wk->ClActWork[i] = CLACT_Add(&add);
			CLACT_SetAnmFlag(wk->ClActWork[i],TRUE);
			CLACT_AnmChg( wk->ClActWork[i], i );
			if (!inBadgeDisp[i]){
				CLACT_SetDrawFlag(wk->ClActWork[i], 0);	//非表示
			}
		}
	}	
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面セルアクターをセット
 *
 * @param	wk				トレーナーカードOBJワーク
 * @param	inBadgeDisp		バッジ表示フラグリスト
 * @param	isClear			殿堂入りしているか？
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void SetTrCardActorSub( TR_CARD_OBJ_WORK *wk)
{
	int i;
	CLACT_HEADER			cl_act_header;
	static u8 ActPos[][2] = {
	 {192,21*8},{0,21*8},
	};
	static u8 AnmNo[] = {
		ANMS_BACK_OFF,ANMS_EXP,ANMS_EFF	
	};
	// セルアクターヘッダ作成	
	CLACT_U_MakeHeader(	&cl_act_header,
						SUB_LCD, SUB_LCD, SUB_LCD, SUB_LCD,
						CLACT_U_HEADER_DATA_NONE, CLACT_U_HEADER_DATA_NONE,
						0, 1,
						wk->ResMan[SUB_LCD][CLACT_U_CHAR_RES],
						wk->ResMan[SUB_LCD][CLACT_U_PLTT_RES],
						wk->ResMan[SUB_LCD][CLACT_U_CELL_RES],
						wk->ResMan[SUB_LCD][CLACT_U_CELLANM_RES],
						NULL,NULL);

	{
		//登録情報格納
//		u8 ofs;
		CLACT_ADD add;
//		u16	badge_ofs = 0;

		add.ClActSet	= wk->ClactSet;
		add.ClActHeader	= &cl_act_header;

		add.mat.x		= 0;//FX32_CONST(32) ;
		add.mat.y		= 0;//FX32_CONST(96) ;		//画面は上下連続している
		add.mat.z		= 0;
		add.sca.x		= FX32_ONE;
		add.sca.y		= FX32_ONE;
		add.sca.z		= FX32_ONE;
		add.rot			= 0;
		add.pri			= 0;
		add.DrawArea	= NNS_G2D_VRAM_TYPE_2DSUB;
		add.heap		= HEAPID_TR_CARD;

		//セルアクター表示開始
		// 下画面(サブ画面)
		for(i=0;i<TR_CARD_SUB_ACT_MAX;i++){
			add.mat.x = FX32_ONE * ActPos[i][0];
			add.mat.y = FX32_ONE * ActPos[i][1] + (FX32_CONST(TR_SUBSURFACE_Y)+SUB_SURFACE_Y);
			wk->ClActWorkS[i] = CLACT_Add(&add);
			CLACT_SetAnmFlag(wk->ClActWorkS[i],TRUE);
			CLACT_AnmChg( wk->ClActWorkS[i], AnmNo[i] );
			CLACT_DrawPriorityChg( wk->ClActWorkS[i],TR_CARD_SUB_ACT_MAX-i);
			CLACT_SetDrawFlag(wk->ClActWorkS[i], FALSE);	//非表示
		}
	}	
}

//--------------------------------------------------------------------------------------------
/**
 * 2Dセルオブジェクト解放
 *
 * @param	wk	オブジェクトワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void RereaseCellObject(TR_CARD_OBJ_WORK *wk)
{
	u8 i;
	//パレット解放
	for(i=0;i<TR_CARD_BADGE_ACT_MAX;i++){
		sys_FreeMemoryEz(wk->PalDataBuf[i]);
	}

	// セルアクターリソース解放

	// キャラ転送マネージャー破棄
	CLACT_U_CharManagerDelete(wk->ResObjTbl[MAIN_LCD][CLACT_U_CHAR_RES]);
	CLACT_U_CharManagerDelete(wk->ResObjTbl[SUB_LCD][CLACT_U_CHAR_RES]);

	// パレット転送マネージャー破棄
	CLACT_U_PlttManagerDelete(wk->ResObjTbl[MAIN_LCD][CLACT_U_PLTT_RES]);
	CLACT_U_PlttManagerDelete(wk->ResObjTbl[SUB_LCD][CLACT_U_PLTT_RES]);
		
	// キャラ・パレット・セル・セルアニメのリソースマネージャー破棄
	for(i=0;i<RESOURCE_NUM;i++){
		CLACT_U_ResManagerDelete(wk->ResMan[MAIN_LCD][i]);
		CLACT_U_ResManagerDelete(wk->ResMan[SUB_LCD][i]);
	}
	//サブサーフェースの距離を設定
	CLACT_U_SetSubSurfaceMatrix(&wk->RendData,0,SUB_SURFACE_Y);

	// セルアクターセット破棄
	CLACT_DestSet(wk->ClactSet);

	//OAMレンダラー破棄
	REND_OAM_Delete();

	DeleteCharManager();
	DeletePlttManager();
	
	GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);	
}

//--------------------------------------------------------------------------------------------
/**
 * バッジパレット変更
 *
 * @param	wk			OBJワーク
 * @param	inBadgeNo	バッジナンバー
 * @param	inOalNo		パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void SetBadgePalette( TR_CARD_OBJ_WORK *wk, const u8 inBadgeNo, const u8 inPalNo )
{
	NNSG2dImagePaletteProxy *proxy;
	u32 palette_vram;
	u32 pal_ofs;
	u8 * adr;
	
	GF_ASSERT(inBadgeNo<TR_CARD_BADGE_ACT_MAX);
	GF_ASSERT(inPalNo<=3);
	
///	CLACT_PaletteNoChg( wk->ClActWork[inBadgeNo], inPalNo );

	proxy = CLACT_PaletteProxyGet( wk->ClActWork[inBadgeNo] );
	palette_vram = NNS_G2dGetImagePaletteLocation(proxy, NNS_G2D_VRAM_TYPE_2DMAIN);
	palette_vram += inBadgeNo*(16*2);
	adr = wk->PalData[inBadgeNo]->pRawData;
	DC_FlushRange( &adr[inPalNo*16*2], 16*2 );
	GX_LoadOBJPltt( &adr[inPalNo*16*2], palette_vram, 16*2 );	//32byte 1Palette
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面ボタンアクター　表示状態変更
 *
 * @param	wk			OBJワーク
 * @param	inBadgeNo	バッジナンバー
 * @param	inOalNo		パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void SetSActDrawSt( TR_CARD_OBJ_WORK *wk, u8 act_id, u8 anm_pat ,u8 draw_f)
{
//	CLACT_SetAnmFlag(wk->ClActWorkS[act_id],TRUE);
	CLACT_AnmChg( wk->ClActWorkS[act_id],anm_pat);
	CLACT_AnmReStart(wk->ClActWorkS[act_id]);
	CLACT_SetDrawFlag(wk->ClActWorkS[act_id],draw_f);	
}

//--------------------------------------------------------------------------------------------
/**
 * サブ画面ボタンエフェクトアクター　表示状態変更
 *
 * @param	wk			OBJワーク
 * @param	inBadgeNo	バッジナンバー
 * @param	inOalNo		パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void SetEffActDrawSt( TR_CARD_OBJ_WORK *wk, u8 pat ,u8 draw_f)
{
	static const u8 pos_x[2] = {28*8,7*8};
	if(pat == 2){
		CATS_ObjectPosSet(wk->ClActWorkS[ACTS_BTN_EFF],sys.tp_x,sys.tp_y+TR_SUBSURFACE_Y);
	}else{
		CATS_ObjectPosSet(wk->ClActWorkS[ACTS_BTN_EFF],pos_x[pat],22*8+TR_SUBSURFACE_Y);
	}
	CLACT_AnmReStart(wk->ClActWorkS[ACTS_BTN_EFF]);
	CLACT_SetDrawFlag(wk->ClActWorkS[ACTS_BTN_EFF],draw_f);	
}

//--------------------------------------------------------------------------------------------
/**
 * キャラクタマネージャー
 *パレットマネージャーの初期化
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitCharPlttManager(void)
{
	// キャラクタマネージャー初期化
	{
		CHAR_MANAGER_MAKE cm = {
			TR_CARD_CHAR_CONT_NUM,
			TR_CARD_CHAR_VRAMTRANS_MAIN_SIZE,
			TR_CARD_CHAR_VRAMTRANS_SUB_SIZE,
			HEAPID_TR_CARD
		};
		InitCharManager(&cm);
	}
	// パレットマネージャー初期化
	InitPlttManager(TR_CARD_PLTT_CONT_NUM, HEAPID_TR_CARD);

	// 読み込み開始位置を初期化
	CharLoadStartAll();
	PlttLoadStartAll();

	REND_OAM_UtilOamRamClear_Main(HEAPID_TR_CARD);
	REND_OAM_UtilOamRamClear_Sub(HEAPID_TR_CARD);

//	DellVramTransferManager();
}


#pragma mark [>start edit
#endif


