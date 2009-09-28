//==============================================================================
/**
 * @file	compati_control.c
 * @brief	相性チェックゲームの入り口と結果画面
 * @author	matsuda
 * @date	2009.02.10(火)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include "net\network_define.h"
#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "message.naix"
#include "test_graphic\d_taya.naix"
#include "font/font.naix"
#include "system\gfl_use.h"
#include "compati_check.naix"
#include "system/wipe.h"
#include "print/wordset.h"

#include "compati_check.h"
#include "compati_types.h"
#include "compati_tool.h"
#include "compati_comm.h"
#include "msg\msg_compati_check.h"


//==============================================================================
//	定数定義
//==============================================================================
///フレーム番号
enum{
	//メイン画面
	FRAME_BACK_M = GFL_BG_FRAME3_M,		///<背景
	FRAME_COUNT_M = GFL_BG_FRAME2_M,		///<カウント
	
	//サブ画面
	FRAME_BACK_S = GFL_BG_FRAME3_S,		///<背景
	FRAME_MSG_S = GFL_BG_FRAME1_S,		///<メッセージフレーム
};

///BGプライオリティ
enum{
	//メイン画面
	BGPRI_BACK_M = 3,
	BGPRI_COUNT_M = 0,
	
	//サブ画面
	BGPRI_BACK_S = 3,
	BGPRI_MSG_S = 1,
};

///フォントのパレット番号
#define CC_FONT_PALNO		(15)

///BMPウィンドウindex
enum{
	CRBMPWIN_POINT_MINE,		///<得点
	CRBMPWIN_POINT_PARTNER,		///<得点
	CRBMPWIN_POINT_TOTAL,		///<得点
	CRBMPWIN_RESULT,
	CRBMPWIN_TALK,
	
	CRBMPWIN_MAX,
};

///赤外線送信コマンド
enum{
	//子から送信
	CMD_CHILD_PROFILE = 0,		///<子のプロフィール
	
	//親から送信
	CMD_PARENT_PROFILE = 2,		///<親のプロフィール
	CMD_PARENT_SUCCESS = 4,		///<全ての受信完了
};

enum{
	MY_CHILD,		///<子である
	MY_PARENT,		///<親である
};

//--------------------------------------------------------------
//	アクター
//--------------------------------------------------------------
///アクター最大数
#define CR_ACT_MAX			(64)

//--------------------------------------------------------------
//	リソースID
//--------------------------------------------------------------
enum{
	//キャラID
	CHARID_CIRCLE,
	CHARID_END,		//キャラID終端
	
	//セルID
	CELLID_CIRCLE,
	CELLID_END,		//セルID終端

	//パレットID
	PLTTID_CIRCLE,
	PLTTID_CIRCLE_END = PLTTID_CIRCLE + CC_CIRCLE_MAX - 1,
	PLTTID_END,		//パレットID終端
};


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	GFL_BMPWIN		*win;
	GFL_BMP_DATA	*bmp;
	PRINT_UTIL		printUtil[1];
	BOOL			message_req;		///<TRUE:メッセージリクエストがあった
}MSG_DRAW_WIN;

///相性チェックシステムワーク構造体
typedef struct {
	COMPATI_PARENTWORK *cppw;

	s16 local_seq;
	s16 local_timer;
	
	GFL_FONT		*fontHandle;
	PRINT_QUE		*printQue;
	GFL_MSGDATA		*mm;
	STRBUF			*strbuf_win[CRBMPWIN_MAX];
	GFL_TCBLSYS		*tcbl;
	MSG_DRAW_WIN drawwin[CRBMPWIN_MAX];	//+1 = メインメッセージウィンドウ
	WORDSET			*wordset;
	
	//アクター
	GFL_CLUNIT *clunit;
	GFL_CLWK *circle_cap[CR_ACT_MAX];

	GFL_TCB *vintr_tcb;
	
	u16 light_pal[16];			///<円を押した時の明るい色のパレットデータ
	
	u32 cgr_id[CHARID_END];
	u32 cell_id[CELLID_END];
	u32 pltt_id[PLTTID_END];
	
	COMPATI_CONNECT_SYS commsys;
	union{
		CCNET_FIRST_PARAM first_param[2];	///< [0]:自分のデータ [1]:受信した相手のデータ
		CCNET_RESULT_PARAM result_param[2];	///< [0]:自分のデータ [1]:受信した相手のデータ
	};
}COMPATI_RESULT_SYS;



//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT CompatiResult_ProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT CompatiResult_ProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT CompatiResult_ProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void CRSLocal_VblankFunc(GFL_TCB *tcb, void *work);
static void CRSLocal_VramSetting(COMPATI_RESULT_SYS *crs);
static void CRSLocal_BGFrameSetting(COMPATI_RESULT_SYS *crs);
static void CRSLocal_BGGraphicLoad(ARCHANDLE *hdl);
static void CRSLocal_MessageSetting(COMPATI_RESULT_SYS *crs);
static void CRSLocal_MessageExit(COMPATI_RESULT_SYS *crs);
static void CRSLocal_ActorSetting(COMPATI_RESULT_SYS *crs);
static void CRSLocal_MessagePut(COMPATI_RESULT_SYS *crs, int win_index, STRBUF *strbuf, int x, int y);
static void CRSLocal_PointMessagePut(COMPATI_RESULT_SYS *crs, int point, u32 msg_id, int win_index, int wordset_no);

static int CRSeq_Init(COMPATI_RESULT_SYS *crs);
static int CRSeq_StartMain(COMPATI_RESULT_SYS *crs);
static int CRSeq_ResultMain(COMPATI_RESULT_SYS *crs);
static int CRSeq_End(COMPATI_RESULT_SYS *crs);



//==============================================================================
//	データ
//==============================================================================
///呼び出し用のPROCデータ
const GFL_PROC_DATA CompatiResultProcData = {
	CompatiResult_ProcInit,
	CompatiResult_ProcMain,
	CompatiResult_ProcEnd,
};

//--------------------------------------------------------------
//	BG
//--------------------------------------------------------------
///VRAMバンク設定
static const GFL_DISP_VRAM CompatiVramBank = {
	GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

///BGモード設定
static const GFL_BG_SYS_HEADER CompatiBgSysHeader = {
	GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
};

//--------------------------------------------------------------
//	メインシーケンス
//--------------------------------------------------------------
///メインシーケンスの関数テーブル
static int (* const CCResultSeqTbl[])(COMPATI_RESULT_SYS *crs) = {
	CRSeq_Init,
	CRSeq_StartMain,
	CRSeq_ResultMain,
	CRSeq_End,
};
///メインシーケンス番号　※CCResultSeqTblと並びを同じにしておくこと！
enum{
	SEQ_INIT,
	SEQ_START_MAIN,
	SEQ_RESULT_MAIN,
	SEQ_END,
	
	SEQ_CONTINUE,		///<シーケンスそのまま
	SEQ_NEXT,			///<次のシーケンスへ
	SEQ_FINISH,			///<終了
};

//--------------------------------------------------------------
//	アクターヘッダ
//--------------------------------------------------------------
///円アクターヘッダ(メイン画面用)
static const GFL_CLWK_DATA CircleObjParam = {
	0, 0,		//pos_x, pos_y
	0, 			//anmseq
	10, 1,	//softpri, bgpri
};


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   PROC 初期化
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT CompatiResult_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	COMPATI_RESULT_SYS* crs;
	ARCHANDLE *hdl;
	COMPATI_PARENTWORK *cppw = pwk;
	
	//画面を真っ暗に
	GX_SetMasterBrightness(-16);
	GXS_SetMasterBrightness(-16);

	//各種効果レジスタ初期化
	GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	GX_SetVisiblePlane(0);
	GXS_SetVisiblePlane(0);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
	G2_BlendNone();
	G2S_BlendNone();
	
	//上下画面設定
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_SUB);

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_COMPATI, 0x70000 );
	crs = GFL_PROC_AllocWork( proc, sizeof(COMPATI_RESULT_SYS), HEAPID_COMPATI );
	GFL_STD_MemClear(crs, sizeof(COMPATI_RESULT_SYS));
	crs->cppw = cppw;
	crs->commsys.send_first_param = &crs->first_param[0];
	crs->commsys.receive_first_param = &crs->first_param[1];
	crs->commsys.send_result_param = &crs->result_param[0];
	crs->commsys.receive_result_param = &crs->result_param[1];
	
	//ファイルオープン
	hdl = GFL_ARC_OpenDataHandle(ARCID_COMPATI_CHECK, HEAPID_COMPATI);
	{
		//VRAMバンク設定
		CRSLocal_VramSetting(crs);
		CRSLocal_BGFrameSetting(crs);
		CRSLocal_BGGraphicLoad(hdl);
		
		//各種ライブラリ設定
		GFL_BMPWIN_Init(HEAPID_COMPATI);
		GFL_FONTSYS_Init();
		crs->tcbl = GFL_TCBL_Init(HEAPID_COMPATI, HEAPID_COMPATI, 4, 32);
		crs->wordset = WORDSET_Create(HEAPID_COMPATI);

		//メッセージ描画の為の準備
		CRSLocal_MessageSetting(crs);

		//アクター設定
		CRSLocal_ActorSetting(crs);
	}
	//ファイルクローズ
	GFL_ARC_CloseDataHandle(hdl);
	
	//VブランクTCB登録
	crs->vintr_tcb = GFUser_VIntr_CreateTCB(CRSLocal_VblankFunc, crs, 5);

	//最初のメッセージ描画
	GFL_MSG_GetString(crs->mm, COMPATI_STR_000, crs->strbuf_win[CRBMPWIN_TALK]);
	CRSLocal_MessagePut(crs, CRBMPWIN_TALK, crs->strbuf_win[CRBMPWIN_TALK], 0, 0);
	
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   PROC メイン
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT CompatiResult_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	COMPATI_RESULT_SYS* crs = mywk;
	int ret, i;
	BOOL que_ret;
	
	GFL_TCBL_Main( crs->tcbl );
	que_ret = PRINTSYS_QUE_Main( crs->printQue );
	for(i = 0; i < CRBMPWIN_MAX; i++){
		if( PRINT_UTIL_Trans( crs->drawwin[i].printUtil, crs->printQue ) == FALSE){
			//return GFL_PROC_RES_CONTINUE;
		}
		else{
			if(que_ret == TRUE && crs->drawwin[i].message_req == TRUE){
	//			GFL_BMP_Clear( crs->bmp, 0xff );
				GFL_BMPWIN_TransVramCharacter( crs->drawwin[i].win );
				crs->drawwin[i].message_req = FALSE;
			}
		}
	}
	
	ret = CCResultSeqTbl[*seq](crs);
	switch(ret){
	case SEQ_CONTINUE:
		break;
	case SEQ_FINISH:
		return GFL_PROC_RES_FINISH;
	case SEQ_NEXT:
		crs->local_seq = 0;
		crs->local_timer = 0;
		(*seq)++;
		break;
	default:
		crs->local_seq = 0;
		crs->local_timer = 0;
		*seq = ret;
		break;
	}

	GFL_CLACT_SYS_Main();
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   PROC 終了
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT CompatiResult_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	COMPATI_RESULT_SYS* crs = mywk;

	GFL_TCB_DeleteTask(crs->vintr_tcb);
	
	CRSLocal_MessageExit(crs);

	GFL_CLACT_UNIT_Delete(crs->clunit);
	GFL_CLACT_SYS_Delete();

	GFL_TCBL_Exit(crs->tcbl);
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_COMPATI);
	
	return GFL_PROC_RES_FINISH;
}

//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   VブランクTCB
 *
 * @param   tcb			
 * @param   work		
 */
//--------------------------------------------------------------
static void CRSLocal_VblankFunc(GFL_TCB *tcb, void *work)
{
	GFL_CLACT_SYS_VBlankFunc();
	GFL_BG_VBlankFunc();
}

//--------------------------------------------------------------
/**
 * @brief   VRAMバンク＆モード設定
 * @param   crs		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void CRSLocal_VramSetting(COMPATI_RESULT_SYS *crs)
{
	GFL_DISP_SetBank( &CompatiVramBank );
	GFL_BG_Init( HEAPID_COMPATI );
	GFL_BG_SetBGMode( &CompatiBgSysHeader );

	//VRAMクリア
	GFL_STD_MemClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
}

//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 * @param   crs		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void CRSLocal_BGFrameSetting(COMPATI_RESULT_SYS *crs)
{
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {	//メイン画面BGフレーム
		{//FRAME_BACK_M
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x4000,
			GX_BG_EXTPLTT_01, BGPRI_BACK_M, 0, 0, FALSE
		},
		{//FRAME_COUNT_M
			0, 0, 0x2000, 0,
			GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, 0x8000,
			GX_BG_EXTPLTT_01, BGPRI_COUNT_M, 0, 0, FALSE
		},
	};

	static const GFL_BG_BGCNT_HEADER sub_bgcnt_frame[] = {	//サブ画面BGフレーム
		{//FRAME_BACK_S
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x4000,
			GX_BG_EXTPLTT_01, BGPRI_BACK_S, 0, 0, FALSE
		},
		{//FRAME_MSG_S
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, 0x08000,
			GX_BG_EXTPLTT_01, BGPRI_MSG_S, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( FRAME_BACK_M,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_COUNT_M,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_BACK_S,   &sub_bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_MSG_S,   &sub_bgcnt_frame[1],   GFL_BG_MODE_TEXT );

	GFL_BG_FillCharacter( FRAME_BACK_M, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_COUNT_M, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_MSG_S, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_BACK_S, 0x00, 1, 0 );

	GFL_BG_FillScreen( FRAME_BACK_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_COUNT_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_MSG_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_BACK_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

	GFL_BG_SetVisible(FRAME_BACK_M, VISIBLE_ON);
	GFL_BG_SetVisible(FRAME_COUNT_M, VISIBLE_OFF);
	GFL_BG_SetVisible(FRAME_BACK_S, VISIBLE_ON);
	GFL_BG_SetVisible(FRAME_MSG_S, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * @brief   BGグラフィックをVRAMへ転送
 *
 * @param   hdl		アーカイブハンドル
 */
//--------------------------------------------------------------
static void CRSLocal_BGGraphicLoad(ARCHANDLE *hdl)
{
	//パレット
	GFL_ARCHDL_UTIL_TransVramPalette(hdl, NARC_compati_check_cc_bg_m_NCLR, 
		PALTYPE_MAIN_BG, 0, 0, HEAPID_COMPATI);
	GFL_ARCHDL_UTIL_TransVramPalette(hdl, NARC_compati_check_cc_bg_s_NCLR, 
		PALTYPE_SUB_BG, 0, 0, HEAPID_COMPATI);

	//メイン画面
	GFL_ARCHDL_UTIL_TransVramBgCharacter(hdl, NARC_compati_check_cc_main_NCGR, 
		FRAME_BACK_M, 0, 0x4000, 0, HEAPID_COMPATI);
	GFL_ARCHDL_UTIL_TransVramScreen(hdl, NARC_compati_check_cc_back_m_NSCR, 
		FRAME_BACK_M, 0, 0, 0, HEAPID_COMPATI);

	GFL_ARCHDL_UTIL_TransVramBgCharacter(hdl, NARC_compati_check_cc_main_NCGR, 
		FRAME_COUNT_M, 0, 0x4000, 0, HEAPID_COMPATI);
	GFL_ARCHDL_UTIL_TransVramScreen(hdl, NARC_compati_check_cc_count_m_NSCR, 
		FRAME_COUNT_M, 0, 0, 0, HEAPID_COMPATI);

	//サブ画面
	GFL_ARCHDL_UTIL_TransVramBgCharacter(hdl, NARC_compati_check_cc_sub_NCGR, 
		FRAME_BACK_S, 0, 0x4000, 0, HEAPID_COMPATI);
	GFL_ARCHDL_UTIL_TransVramScreen(hdl, NARC_compati_check_cc_back_s_NSCR, 
		FRAME_BACK_S, 0, 0, 0, HEAPID_COMPATI);

	//フォントパレット転送
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, 
		PALTYPE_MAIN_BG, CC_FONT_PALNO * 0x20, 0x20, HEAPID_COMPATI);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, 
		PALTYPE_SUB_BG, CC_FONT_PALNO * 0x20, 0x20, HEAPID_COMPATI);

	GFL_BG_LoadScreenReq( FRAME_BACK_M );
	GFL_BG_LoadScreenReq( FRAME_COUNT_M );
	GFL_BG_LoadScreenReq( FRAME_MSG_S );
	GFL_BG_LoadScreenReq( FRAME_BACK_S );
}

//--------------------------------------------------------------
/**
 * @brief   メッセージ描画関連の設定
 *
 * @param   crs		
 */
//--------------------------------------------------------------
static void CRSLocal_MessageSetting(COMPATI_RESULT_SYS *crs)
{
	int i;

	crs->drawwin[CRBMPWIN_POINT_MINE].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 11, 0, 8, 4, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	crs->drawwin[CRBMPWIN_POINT_PARTNER].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 11, 4, 8, 4, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	crs->drawwin[CRBMPWIN_POINT_TOTAL].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 9, 8, 12, 4, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	crs->drawwin[CRBMPWIN_RESULT].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 7, 13, 18, 4, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	crs->drawwin[CRBMPWIN_TALK].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 1, 18, 30, 4, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	for(i = 0; i < CRBMPWIN_MAX; i++){
		GF_ASSERT(crs->drawwin[i].win != NULL);
		crs->drawwin[i].bmp = GFL_BMPWIN_GetBmp(crs->drawwin[i].win);
		GFL_BMP_Clear( crs->drawwin[i].bmp, 0xff );
		GFL_BMPWIN_MakeScreen( crs->drawwin[i].win );
		GFL_BMPWIN_TransVramCharacter( crs->drawwin[i].win );
		PRINT_UTIL_Setup( crs->drawwin[i].printUtil, crs->drawwin[i].win );
	}
	GFL_BG_LoadScreenReq(FRAME_MSG_S);	//MakeScreenしたのを反映

	crs->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_COMPATI );

//		PRINTSYS_Init( HEAPID_COMPATI );
	crs->printQue = PRINTSYS_QUE_Create( HEAPID_COMPATI );

	crs->mm = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_compati_check_dat, HEAPID_COMPATI);
	for(i = 0; i < CRBMPWIN_MAX; i++){
		crs->strbuf_win[i] = GFL_STR_CreateBuffer( 64, HEAPID_COMPATI );
	}
}

//--------------------------------------------------------------
/**
 * @brief   メッセージ関連破棄
 *
 * @param   crs		
 */
//--------------------------------------------------------------
static void CRSLocal_MessageExit(COMPATI_RESULT_SYS *crs)
{
	int i;
	
	GFL_MSG_Delete(crs->mm);
	WORDSET_Delete(crs->wordset);
	PRINTSYS_QUE_Delete(crs->printQue);
	GFL_FONT_Delete(crs->fontHandle);
	for(i = 0; i < CRBMPWIN_MAX; i++){
		GFL_BMPWIN_Delete(crs->drawwin[i].win);
		GFL_STR_DeleteBuffer(crs->strbuf_win[i]);
	}
}

//--------------------------------------------------------------
/**
 * @brief   アクター設定
 *
 * @param   crs		
 */
//--------------------------------------------------------------
static void CRSLocal_ActorSetting(COMPATI_RESULT_SYS *crs)
{
	GFL_CLSYS_INIT clsys_init = GFL_CLSYSINIT_DEF_DIVSCREEN;
	
	clsys_init.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
	clsys_init.oamnum_main = 128-GFL_CLSYS_OAMMAN_INTERVAL;
	clsys_init.tr_cell = CR_ACT_MAX;
	GFL_CLACT_SYS_Create(&clsys_init, &CompatiVramBank, HEAPID_COMPATI);
	
	crs->clunit = GFL_CLACT_UNIT_Create(CR_ACT_MAX, 0, HEAPID_COMPATI);
	GFL_CLACT_UNIT_SetDefaultRend(crs->clunit);

	//OBJ表示ON
	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * @brief   メッセージを表示する
 *
 * @param   crs			
 * @param   strbuf		表示するメッセージデータ
 * @param   x			X座標
 * @param   y			Y座標
 */
//--------------------------------------------------------------
static void CRSLocal_MessagePut(COMPATI_RESULT_SYS *crs, int win_index, STRBUF *strbuf, int x, int y)
{
	GFL_BMP_Clear( crs->drawwin[win_index].bmp, 0xff );
	PRINTSYS_PrintQue(crs->printQue, crs->drawwin[win_index].bmp, x, y, strbuf, crs->fontHandle);
	crs->drawwin[win_index].message_req = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   得点をメッセージ表示する
 *
 * @param   crs			
 * @param   point		得点
 */
//--------------------------------------------------------------
static void CRSLocal_PointMessagePut(COMPATI_RESULT_SYS *crs, int point, u32 msg_id, int win_index, int wordset_no)
{
	STRBUF *temp_str;

	temp_str = GFL_MSG_CreateString(crs->mm, msg_id);
	WORDSET_RegisterNumber(crs->wordset, wordset_no, point, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
	WORDSET_ExpandStr(crs->wordset, crs->strbuf_win[win_index], temp_str);
	CRSLocal_MessagePut(crs, win_index, crs->strbuf_win[win_index], 0, 0);
	GFL_STR_DeleteBuffer(temp_str);
}


//==============================================================================
//	シーケンス
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   メインシーケンス：初期設定
 * @param   crs		
 * @retval  シーケンス動作
 */
//--------------------------------------------------------------
static int CRSeq_Init(COMPATI_RESULT_SYS *crs)
{
	switch(crs->local_seq){
	case 0:
		WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, 
			WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_COMPATI);
		crs->local_seq++;
		break;
	case 1:
		if(WIPE_SYS_EndCheck() == TRUE){
			if(crs->cppw->game_mode == COMPATI_GAMEMODE_START){
				return SEQ_START_MAIN;
			}
			else{
				return SEQ_RESULT_MAIN;
			}
		}
		break;
	}
	
	return SEQ_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   メインシーケンス：相手募集画面メイン
 * @param   crs		
 * @retval  シーケンス動作
 */
//--------------------------------------------------------------
static int CRSeq_StartMain(COMPATI_RESULT_SYS *crs)
{
	switch(crs->local_seq){
	case 0:
		//送信バッファにデータをセット
		GFL_STD_MemCopy(&crs->cppw->circle_package, 
			&crs->commsys.send_first_param->circle_package, sizeof(CC_CIRCLE_PACKAGE));
		OS_GetMacAddress(crs->commsys.send_first_param->macAddress);
		
		if(CompatiComm_Init(&crs->commsys, IRC_TIMEOUT_STANDARD) == TRUE){
			crs->local_seq++;
		}
		break;
	case 1:
		if(CompatiComm_Connect(&crs->commsys) == TRUE){
			crs->local_seq++;
		}
		break;
	case 2:
		if(CompatiComm_FirstSend(&crs->commsys) == TRUE){
			crs->local_seq++;
			OS_TPrintf("FirstParam送受信成功\n");
			//受信したデータをセット
			if(GFL_NET_SystemGetCurrentID() != 0){	//※check 通信で親側が整理して送信しなおすべき
				GFL_STD_MemCopy(&crs->commsys.receive_first_param->circle_package, 
					&crs->cppw->circle_package, sizeof(CC_CIRCLE_PACKAGE));
			}
			GFL_STD_MemCopy(crs->commsys.receive_first_param->macAddress, 
				crs->cppw->partner_macAddress, 6);
		}
		break;
	case 3:
		if(CompatiComm_Shoutdown(&crs->commsys) == TRUE){
			crs->local_seq++;
		}
		break;
	case 4:
		if(CompatiComm_Exit(&crs->commsys) == TRUE){
			GFL_MSG_GetString(crs->mm, COMPATI_STR_001, crs->strbuf_win[CRBMPWIN_TALK]);
			CRSLocal_MessagePut(crs, CRBMPWIN_TALK, crs->strbuf_win[CRBMPWIN_TALK], 0, 0);
			crs->local_seq++;
		}
		break;
	case 5:
		crs->local_timer++;
		if(crs->local_timer > 60){
			return SEQ_END;
		}
		break;
	}
	
	return SEQ_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   メインシーケンス：結果画面メイン
 * @param   crs		
 * @retval  シーケンス動作
 */
//--------------------------------------------------------------
static int CRSeq_ResultMain(COMPATI_RESULT_SYS *crs)
{
	switch(crs->local_seq){
	case 0:
		//送信バッファにデータをセット
		crs->commsys.send_result_param->point = crs->cppw->point;
		crs->commsys.send_result_param->irc_time_count_max = crs->cppw->irc_time_count_max;
		
		if(CompatiComm_Init(&crs->commsys, IRC_TIMEOUT_STANDARD) == TRUE){
			crs->local_seq++;
		}
		break;
	case 1:
		if(CompatiComm_Connect(&crs->commsys) == TRUE){
			crs->local_seq++;
		}
		break;
	case 2:
		if(CompatiComm_ResultSend(&crs->commsys) == TRUE){
			crs->local_seq++;
			OS_TPrintf("ResultParam送受信成功\n");
			
			//受信バッファに結果をセット
			crs->cppw->partner_point = crs->commsys.receive_result_param->point;
			if(crs->cppw->irc_time_count_max 
					< crs->commsys.receive_result_param->irc_time_count_max){
				//大きいほうを優先して結果とする
				crs->cppw->irc_time_count_max = crs->commsys.receive_result_param->irc_time_count_max;
			}
		}
		break;
	case 3:
		if(CompatiComm_Shoutdown(&crs->commsys) == TRUE){
			crs->local_seq++;
		}
		break;
	case 4:
		if(CompatiComm_Exit(&crs->commsys) == TRUE){
			GFL_MSG_GetString(crs->mm, COMPATI_STR_001, crs->strbuf_win[CRBMPWIN_TALK]);
			CRSLocal_MessagePut(crs, CRBMPWIN_TALK, crs->strbuf_win[CRBMPWIN_TALK], 0, 0);
			crs->local_seq++;
		}
		break;
	case 5:
		crs->local_timer++;
		if(crs->local_timer > 60){
			crs->local_timer = 0;
			crs->local_seq++;
		}
		break;
	case 6:
		{
			int my_point, partner_point, total_point;
			int msg_id;
			
			my_point = crs->result_param[0].point;
			partner_point = crs->result_param[1].point;
			total_point = my_point + partner_point + crs->cppw->irc_time_count_max / 60;
			WORDSET_RegisterNumber(crs->wordset, 3, 
				crs->cppw->irc_time_count_max / 60, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
			CRSLocal_PointMessagePut(crs, my_point, COMPATI_STR_101, CRBMPWIN_POINT_MINE, 1);
			CRSLocal_PointMessagePut(
				crs, partner_point, COMPATI_STR_100, CRBMPWIN_POINT_PARTNER, 0);
			CRSLocal_PointMessagePut(crs, total_point, COMPATI_STR_102, CRBMPWIN_POINT_TOTAL, 2);
			if(total_point >= 200){
				msg_id = COMPATI_STR_006;
			}
			else if(total_point >= 150){
				msg_id = COMPATI_STR_005;
			}
			else if(total_point >= 100){
				msg_id = COMPATI_STR_004;
			}
			else if(total_point >= 50){
				msg_id = COMPATI_STR_003;
			}
			else{
				msg_id = COMPATI_STR_002;
			}
			GFL_MSG_GetString(crs->mm, msg_id, crs->strbuf_win[CRBMPWIN_RESULT]);
			CRSLocal_MessagePut(crs, CRBMPWIN_RESULT, crs->strbuf_win[CRBMPWIN_RESULT], 0, 0);
		}
		crs->local_seq++;
		break;
	case 7:
		if(GFL_UI_TP_GetTrg() == TRUE || (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B))){
			return SEQ_END;
		}
		break;
	}
	
	return SEQ_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   メインシーケンス：終了処理
 * @param   crs		
 * @retval  シーケンス動作
 */
//--------------------------------------------------------------
static int CRSeq_End(COMPATI_RESULT_SYS *crs)
{
	switch(crs->local_seq){
	case 0:
		WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, 
			WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_COMPATI);
		crs->local_seq++;
		break;
	case 1:
		if(WIPE_SYS_EndCheck() == TRUE){
			return SEQ_FINISH;
		}
		break;
	}
	return SEQ_CONTINUE;
}
