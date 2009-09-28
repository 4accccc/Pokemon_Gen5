//==============================================================================
/**
 * @file	compati_check.c
 * @brief	相性チェック
 * @author	matsuda
 * @date	2009.02.09(月)
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
#include "compati_irc.h"
#include "msg\msg_compati_check.h"


//--------------------------------------------------------------
//	
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(dev_irc);

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
	CCBMPWIN_TITLE,		///<ゲーム名
	CCBMPWIN_POINT,		///<得点
	CCBMPWIN_TALK,		///<会話文
	
	CCBMPWIN_MAX,
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

///赤外線接続中のバックグランド背景色
#define BG_COLOR_IRC		(0x0c34)
///赤外線接続中のバックグランド背景色のパレット位置
#define BG_COLOR_IRC_COLOR_POS	((16*0 + 0xc) * 2)

///タイムアップまでの時間(秒数指定)
#define CC_TIMEUP			(30)

//--------------------------------------------------------------
//	アクター
//--------------------------------------------------------------
///アクター最大数
#define CC_ACT_MAX			(64)

//--------------------------------------------------------------
//	リソースID
//--------------------------------------------------------------
enum{
	//キャラID
	CHARID_CIRCLE = 0,
	CHARID_END,		//キャラID終端
	
	//セルID
	CELLID_CIRCLE = 0,
	CELLID_END,		//セルID終端

	//パレットID
	PLTTID_CIRCLE = 0,
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
	PRINT_STREAM	*ps_entry[CCBMPWIN_MAX];
	GFL_MSGDATA		*mm;
	STRBUF			*strbuf_win[CCBMPWIN_MAX];
	STRBUF			*strbuf_point;
	GFL_TCBLSYS		*tcbl;
	MSG_DRAW_WIN drawwin[CCBMPWIN_MAX];	//+1 = メインメッセージウィンドウ
	WORDSET			*wordset;
	
	//アクター
	GFL_CLUNIT *clunit;
	GFL_CLWK *circle_cap[CC_ACT_MAX];

	GFL_TCB *vintr_tcb;
	
	u16 light_pal[16];			///<円を押した時の明るい色のパレットデータ
	u16 normal_pal[16];			///<円を押していない時のパレットデータ
	u16 bg_color;				///<バックグラウンド色の元の色
	
	u32 cgr_id[CHARID_END];
	u32 cell_id[CELLID_END];
	u32 pltt_id[PLTTID_END];
	
	CCT_TOUCH_SYS ccts;			///<円とタッチペンの当たり判定管理ワーク
	COMPATI_IRC_SYS ircsys;		///<ゲーム中の赤外線接続管理ワーク
	COMPATI_CONNECT_SYS commsys;
	
	int before_irc_ret;
	s32 irc_time_count;
	s32 irc_time_count_max;
}COMPATI_SYS;



//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT CompatiCheck_ProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT CompatiCheck_ProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT CompatiCheck_ProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void CCLocal_VblankFunc(GFL_TCB *tcb, void *work);
static void CCLocal_VramSetting(COMPATI_SYS *cs);
static void CCLocal_BGFrameSetting(COMPATI_SYS *cs);
static void CCLocal_BGGraphicLoad(ARCHANDLE *hdl);
static void CCLocal_MessageSetting(COMPATI_SYS *cs);
static void CCLocal_MessageExit(COMPATI_SYS *cs);
static void CCLocal_ActorSetting(COMPATI_SYS *cs);
static void CCLocal_CircleActor_Create(COMPATI_SYS *cs, ARCHANDLE *hdl);
static void CCLocal_CircleActor_Delete(COMPATI_SYS *cs);
static void CCLocal_CircleActor_PosSet(COMPATI_SYS *cs, const CC_CIRCLE_PACKAGE *circle_package);
static void CCLocal_CirclePalCopy(COMPATI_SYS *cs, ARCHANDLE *hdl);
static void CCLocal_MessagePut(COMPATI_SYS *cs, int win_index, STRBUF *strbuf, int x, int y);
static void CCLocal_PointMessagePut(COMPATI_SYS *cs, int point);
static int CCSeq_Init(COMPATI_SYS *cs);
static int CCSeq_Countdown(COMPATI_SYS *cs);
static int CCSeq_Main(COMPATI_SYS *cs);
static int CCSeq_End(COMPATI_SYS *cs);



//==============================================================================
//	データ
//==============================================================================
///呼び出し用のPROCデータ
const GFL_PROC_DATA CompatiCheckProcData = {
	CompatiCheck_ProcInit,
	CompatiCheck_ProcMain,
	CompatiCheck_ProcEnd,
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
static int (* const CCMainSeqTbl[])(COMPATI_SYS *cs) = {
	CCSeq_Init,
	CCSeq_Countdown,
	CCSeq_Main,
	CCSeq_End,
};
///メインシーケンス番号　※CCMainSeqTblと並びを同じにしておくこと！
enum{
	SEQ_INIT,
	SEQ_COUNTDOWN,
	SEQ_MAIN,
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
static GFL_PROC_RESULT CompatiCheck_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	COMPATI_SYS* cs;
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
	cs = GFL_PROC_AllocWork( proc, sizeof(COMPATI_SYS), HEAPID_COMPATI );
	GFL_STD_MemClear(cs, sizeof(COMPATI_SYS));
	cs->cppw = cppw;
	
	//ファイルオープン
	hdl = GFL_ARC_OpenDataHandle(ARCID_COMPATI_CHECK, HEAPID_COMPATI);
	{
		//VRAMバンク設定
		CCLocal_VramSetting(cs);
		CCLocal_BGFrameSetting(cs);
		CCLocal_BGGraphicLoad(hdl);
		GFL_STD_MemCopy16((void*)(HW_BG_PLTT + 0xc*2), &cs->bg_color, 2);
		
		//各種ライブラリ設定
		GFL_BMPWIN_Init(HEAPID_COMPATI);
		GFL_FONTSYS_Init();
		cs->tcbl = GFL_TCBL_Init(HEAPID_COMPATI, HEAPID_COMPATI, 4, 32);
		cs->wordset = WORDSET_Create(HEAPID_COMPATI);

		//メッセージ描画の為の準備
		CCLocal_MessageSetting(cs);

		//アクター設定
		CCLocal_ActorSetting(cs);
		CCLocal_CircleActor_Create(cs, hdl);
		CCLocal_CircleActor_PosSet(cs, &cppw->circle_package);
		CCLocal_CirclePalCopy(cs, hdl);
	}
	//ファイルクローズ
	GFL_ARC_CloseDataHandle(hdl);
	
	//VブランクTCB登録
	cs->vintr_tcb = GFUser_VIntr_CreateTCB(CCLocal_VblankFunc, cs, 5);

	//最初のメッセージ描画
	CCLocal_PointMessagePut(cs, 0);
	GFL_MSG_GetString(cs->mm, COMPATI_STR_010, cs->strbuf_win[CCBMPWIN_TITLE]);
	CCLocal_MessagePut(cs, CCBMPWIN_TITLE, cs->strbuf_win[CCBMPWIN_TITLE], 0, 0);
	GFL_MSG_GetString(cs->mm, COMPATI_STR_011, cs->strbuf_win[CCBMPWIN_TALK]);
	CCLocal_MessagePut(cs, CCBMPWIN_TALK, cs->strbuf_win[CCBMPWIN_TALK], 0, 0);
	
	//ゲーム中の赤外線通信用にここでIRCオーバーレイ読み込み　※check 暫定処理
//	GFL_OVERLAY_Load(FS_OVERLAY_ID(dev_irc));
	CompatiIrc_Init(&cs->ircsys, &cs->commsys);
	
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
static GFL_PROC_RESULT CompatiCheck_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	COMPATI_SYS* cs = mywk;
	int ret, i;
	BOOL que_ret;
	
	GFL_TCBL_Main( cs->tcbl );
	que_ret = PRINTSYS_QUE_Main( cs->printQue );
	for(i = 0; i < CCBMPWIN_MAX; i++){
		if( PRINT_UTIL_Trans( cs->drawwin[i].printUtil, cs->printQue ) == FALSE){
			//return GFL_PROC_RES_CONTINUE;
		}
		else{
			if(que_ret == TRUE && cs->drawwin[i].message_req == TRUE){
	//			GFL_BMP_Clear( cs->bmp, 0xff );
				GFL_BMPWIN_TransVramCharacter( cs->drawwin[i].win );
				cs->drawwin[i].message_req = FALSE;
			}
		}
	}
	
	ret = CCMainSeqTbl[*seq](cs);
	switch(ret){
	case SEQ_CONTINUE:
		break;
	case SEQ_FINISH:
		return GFL_PROC_RES_FINISH;
	case SEQ_NEXT:
		cs->local_seq = 0;
		cs->local_timer = 0;
		(*seq)++;
		break;
	default:
		cs->local_seq = 0;
		cs->local_timer = 0;
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
static GFL_PROC_RESULT CompatiCheck_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	COMPATI_SYS* cs = mywk;
	COMPATI_PARENTWORK *cppw = pwk;

	//IRCオーバーレイ解放　※check
//	GFL_OVERLAY_Unload(FS_OVERLAY_ID(dev_irc));
	
	//得点結果をパレントワークに格納
	cppw->point = cs->ccts.total_len / CC_POINT_DOT;
	cppw->irc_time_count_max = cs->irc_time_count_max;
	
	GFL_TCB_DeleteTask(cs->vintr_tcb);
	
	CCLocal_CircleActor_Delete(cs);
	CCLocal_MessageExit(cs);

	GFL_CLACT_UNIT_Delete(cs->clunit);
	GFL_CLACT_SYS_Delete();

	GFL_TCBL_Exit(cs->tcbl);
	
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
static void CCLocal_VblankFunc(GFL_TCB *tcb, void *work)
{
	GFL_CLACT_SYS_VBlankFunc();
	GFL_BG_VBlankFunc();
}

//--------------------------------------------------------------
/**
 * @brief   VRAMバンク＆モード設定
 * @param   cs		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void CCLocal_VramSetting(COMPATI_SYS *cs)
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
 * @param   cs		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void CCLocal_BGFrameSetting(COMPATI_SYS *cs)
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
	GFL_BG_SetVisible(FRAME_COUNT_M, VISIBLE_ON);
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
static void CCLocal_BGGraphicLoad(ARCHANDLE *hdl)
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
 * @param   cs		
 */
//--------------------------------------------------------------
static void CCLocal_MessageSetting(COMPATI_SYS *cs)
{
	int i;

	cs->drawwin[CCBMPWIN_TITLE].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 8, 2, 16, 2, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	cs->drawwin[CCBMPWIN_POINT].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 14, 8, 6, 2, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	cs->drawwin[CCBMPWIN_TALK].win = GFL_BMPWIN_Create(
		FRAME_MSG_S, 1, 18, 30, 4, CC_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
	for(i = 0; i < CCBMPWIN_MAX; i++){
		GF_ASSERT(cs->drawwin[i].win != NULL);
		cs->drawwin[i].bmp = GFL_BMPWIN_GetBmp(cs->drawwin[i].win);
		GFL_BMP_Clear( cs->drawwin[i].bmp, 0xff );
		GFL_BMPWIN_MakeScreen( cs->drawwin[i].win );
		GFL_BMPWIN_TransVramCharacter( cs->drawwin[i].win );
		PRINT_UTIL_Setup( cs->drawwin[i].printUtil, cs->drawwin[i].win );
	}
	GFL_BG_LoadScreenReq(FRAME_MSG_S);	//MakeScreenしたのを反映

	cs->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_COMPATI );

//		PRINTSYS_Init( HEAPID_COMPATI );
	cs->printQue = PRINTSYS_QUE_Create( HEAPID_COMPATI );

	cs->mm = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_compati_check_dat, HEAPID_COMPATI);
	for(i = 0; i < CCBMPWIN_MAX; i++){
		cs->strbuf_win[i] = GFL_STR_CreateBuffer( 64, HEAPID_COMPATI );
	}
	cs->strbuf_point = GFL_MSG_CreateString(cs->mm, COMPATI_STR_009);
}

//--------------------------------------------------------------
/**
 * @brief   メッセージ関連破棄
 *
 * @param   cs		
 */
//--------------------------------------------------------------
static void CCLocal_MessageExit(COMPATI_SYS *cs)
{
	int i;
	
	GFL_MSG_Delete(cs->mm);
	WORDSET_Delete(cs->wordset);
	PRINTSYS_QUE_Delete(cs->printQue);
	GFL_FONT_Delete(cs->fontHandle);
	for(i = 0; i < CCBMPWIN_MAX; i++){
		GFL_BMPWIN_Delete(cs->drawwin[i].win);
		GFL_STR_DeleteBuffer(cs->strbuf_win[i]);
	}
	GFL_STR_DeleteBuffer(cs->strbuf_point);
}

//--------------------------------------------------------------
/**
 * @brief   アクター設定
 *
 * @param   cs		
 */
//--------------------------------------------------------------
static void CCLocal_ActorSetting(COMPATI_SYS *cs)
{
	GFL_CLSYS_INIT clsys_init = GFL_CLSYSINIT_DEF_DIVSCREEN;
	
	clsys_init.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
	clsys_init.oamnum_main = 128-GFL_CLSYS_OAMMAN_INTERVAL;
	clsys_init.tr_cell = CC_ACT_MAX;
	GFL_CLACT_SYS_Create(&clsys_init, &CompatiVramBank, HEAPID_COMPATI);
	
	cs->clunit = GFL_CLACT_UNIT_Create(CC_ACT_MAX, 0, HEAPID_COMPATI);
	GFL_CLACT_UNIT_SetDefaultRend(cs->clunit);

	//OBJ表示ON
	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * @brief   円アクター作成
 *
 * @param   cs		
 * @param   hdl		アーカイブハンドル
 */
//--------------------------------------------------------------
static void CCLocal_CircleActor_Create(COMPATI_SYS *cs, ARCHANDLE *hdl)
{
	int i;
	
	//リソース登録
	cs->cgr_id[CHARID_CIRCLE] = GFL_CLGRP_CGR_Register(
		hdl, NARC_compati_check_cc_circle_NCGR, FALSE, CLSYS_DRAW_MAIN, HEAPID_COMPATI);
	cs->cell_id[CELLID_CIRCLE] = GFL_CLGRP_CELLANIM_Register(
		hdl, NARC_compati_check_cc_circle_NCER, NARC_compati_check_cc_circle_NANR, HEAPID_COMPATI);
	//パレットリソース登録(押した円だけ色を変えるので円の数分登録)
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		cs->pltt_id[PLTTID_CIRCLE + i] = GFL_CLGRP_PLTT_RegisterEx(hdl, 
			NARC_compati_check_cc_circle_NCLR,
			CLSYS_DRAW_MAIN, 0x20 * i, 0, 1, HEAPID_COMPATI);
	}
	
	//アクター登録
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		cs->circle_cap[i] = GFL_CLACT_WK_Create(cs->clunit, cs->cgr_id[CHARID_CIRCLE], 
			cs->pltt_id[PLTTID_CIRCLE + i], cs->cell_id[CELLID_CIRCLE], 
			&CircleObjParam, CLSYS_DEFREND_MAIN, HEAPID_COMPATI);
		GFL_CLACT_WK_AddAnmFrame(cs->circle_cap[i], FX32_ONE);
		GFL_CLACT_WK_SetDrawEnable(cs->circle_cap[i], FALSE);
	}
}

//--------------------------------------------------------------
/**
 * @brief   円アクター破棄
 *
 * @param   cs		
 */
//--------------------------------------------------------------
static void CCLocal_CircleActor_Delete(COMPATI_SYS *cs)
{
	int i;
	
	//アクター削除
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		GFL_CLACT_WK_Remove(cs->circle_cap[i]);
	}
	
	//リソース解放
	GFL_CLGRP_CGR_Release(cs->cgr_id[CHARID_CIRCLE]);
	GFL_CLGRP_CELLANIM_Release(cs->cell_id[CELLID_CIRCLE]);
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		GFL_CLGRP_PLTT_Release(cs->pltt_id[PLTTID_CIRCLE + i]);
	}
}

//--------------------------------------------------------------
/**
 * @brief   円アクターの座標、アニメを設定
 *
 * @param   cs					
 * @param   circle_package		サークルデータへのポインタ
 */
//--------------------------------------------------------------
static void CCLocal_CircleActor_PosSet(COMPATI_SYS *cs, const CC_CIRCLE_PACKAGE *circle_package)
{
	int i;
	GFL_CLACTPOS pos;
	
	for(i = 0; i < CC_CIRCLE_MAX; i++){
		if(circle_package->data[i].type == CC_CIRCLE_TYPE_NULL){
			return;
		}
		pos.x = circle_package->data[i].x;
		pos.y = circle_package->data[i].y;
		GFL_CLACT_WK_SetPos(cs->circle_cap[i], &pos, CLSYS_DEFREND_MAIN);
		GFL_CLACT_WK_SetAnmSeq(cs->circle_cap[i], circle_package->data[i].type);
		GFL_CLACT_WK_SetDrawEnable(cs->circle_cap[i], TRUE);
	}
}

//--------------------------------------------------------------
/**
 * @brief   円アクターのパレットのノーマル色と明るい色をワークにコピーしておく
 *
 * @param   cs		
 * @param   hdl		アーカイブハンドル
 */
//--------------------------------------------------------------
static void CCLocal_CirclePalCopy(COMPATI_SYS *cs, ARCHANDLE *hdl)
{
	void *arc_data;
	NNSG2dPaletteData *paldata;

	arc_data = GFL_ARCHDL_UTIL_LoadPalette(hdl, NARC_compati_check_cc_circle_light_NCLR, 
		&paldata, HEAPID_COMPATI);
	GFL_STD_MemCopy16(paldata->pRawData, cs->light_pal, 0x20);
	GFL_HEAP_FreeMemory(arc_data);

	arc_data = GFL_ARCHDL_UTIL_LoadPalette(hdl, NARC_compati_check_cc_circle_NCLR, 
		&paldata, HEAPID_COMPATI);
	GFL_STD_MemCopy16(paldata->pRawData, cs->normal_pal, 0x20);
	GFL_HEAP_FreeMemory(arc_data);
}

//--------------------------------------------------------------
/**
 * @brief   メッセージを表示する
 *
 * @param   cs			
 * @param   strbuf		表示するメッセージデータ
 * @param   x			X座標
 * @param   y			Y座標
 */
//--------------------------------------------------------------
static void CCLocal_MessagePut(COMPATI_SYS *cs, int win_index, STRBUF *strbuf, int x, int y)
{
	GFL_BMP_Clear( cs->drawwin[win_index].bmp, 0xff );
	PRINTSYS_PrintQue(cs->printQue, cs->drawwin[win_index].bmp, x, y, strbuf, cs->fontHandle);
	cs->drawwin[win_index].message_req = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   得点をメッセージ表示する
 *
 * @param   cs			
 * @param   point		得点
 */
//--------------------------------------------------------------
static void CCLocal_PointMessagePut(COMPATI_SYS *cs, int point)
{
	WORDSET_RegisterNumber(cs->wordset, 0, point, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
	WORDSET_ExpandStr(cs->wordset, cs->strbuf_win[CCBMPWIN_POINT], cs->strbuf_point);
	CCLocal_MessagePut(cs, CCBMPWIN_POINT, cs->strbuf_win[CCBMPWIN_POINT], 0, 0);
}


//==============================================================================
//	シーケンス
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   メインシーケンス：初期設定
 * @param   cs		
 * @retval  シーケンス動作
 */
//--------------------------------------------------------------
static int CCSeq_Init(COMPATI_SYS *cs)
{
	switch(cs->local_seq){
	case 0:
		WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, 
			WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_COMPATI);
		cs->local_seq++;
		break;
	case 1:
		if(WIPE_SYS_EndCheck() == TRUE){
			return SEQ_NEXT;
		}
		break;
	}
	
	return SEQ_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   メインシーケンス：カウントダウン
 * @param   cs		
 * @retval  シーケンス動作
 */
//--------------------------------------------------------------
static int CCSeq_Countdown(COMPATI_SYS *cs)
{
	cs->local_timer++;
	if(cs->local_timer < 60){
		return SEQ_CONTINUE;
	}
	cs->local_timer = 0;

	switch(cs->local_seq){
	case 0:
		GFL_BG_SetScroll(FRAME_COUNT_M, GFL_BG_SCROLL_X_SET, 256);
		GFL_BG_SetScroll(FRAME_COUNT_M, GFL_BG_SCROLL_Y_SET, 0);
		cs->local_seq++;
		break;
	case 1:
		GFL_BG_SetScroll(FRAME_COUNT_M, GFL_BG_SCROLL_X_SET, 0);
		GFL_BG_SetScroll(FRAME_COUNT_M, GFL_BG_SCROLL_Y_SET, 256);
		cs->local_seq++;
		break;
	case 2:
		GFL_BG_SetScroll(FRAME_COUNT_M, GFL_BG_SCROLL_X_SET, 256);
		GFL_BG_SetScroll(FRAME_COUNT_M, GFL_BG_SCROLL_Y_SET, 256);
		cs->local_seq++;
		break;
	case 3:
		GFL_BG_SetVisible(FRAME_COUNT_M, VISIBLE_OFF);
		return SEQ_NEXT;
	}
	return SEQ_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   メインシーケンス：メイン
 * @param   cs		
 * @retval  シーケンス動作
 */
//--------------------------------------------------------------
static int CCSeq_Main(COMPATI_SYS *cs)
{
	int pal_addr, i;
	int before_point, after_point;
	int irc_ret;
	
	irc_ret = CompatiIrc_Main(&cs->ircsys, &cs->commsys);
	if(cs->before_irc_ret == TRUE && irc_ret == COMPATIIRC_RESULT_CONNECT){
		cs->irc_time_count++;
		if(cs->irc_time_count > cs->irc_time_count_max){
			cs->irc_time_count_max++;
		}
	}
	else if(irc_ret == COMPATIIRC_RESULT_FALSE){
		cs->irc_time_count = 0;
	}
	cs->before_irc_ret = irc_ret;
	
	cs->local_timer++;
	if(cs->local_timer > CC_TIMEUP * 60){
		if(irc_ret == COMPATIIRC_RESULT_EXIT){
			return SEQ_NEXT;
		}
		else{
			CompatiIrc_Shoutdown(&cs->ircsys, &cs->commsys);
		}
	}
	
	before_point = cs->ccts.total_len / CC_POINT_DOT;

	CCTOOL_CircleTouchCheck(&cs->cppw->circle_package, &cs->ccts);
	if(cs->ccts.touch_hold == CIRCLE_TOUCH_HOLD){
		pal_addr = GFL_CLGRP_PLTT_GetAddr(
			cs->pltt_id[PLTTID_CIRCLE + cs->ccts.hold_circle_no], CLSYS_DRAW_MAIN);
		GFL_STD_MemCopy16(cs->light_pal, (void*)(HW_OBJ_PLTT + pal_addr), 0x20);
	}
	else{
		for(i = 0; i < CC_CIRCLE_MAX; i++){
			pal_addr = GFL_CLGRP_PLTT_GetAddr(cs->pltt_id[PLTTID_CIRCLE + i], CLSYS_DRAW_MAIN);
			GFL_STD_MemCopy16(cs->normal_pal, (void*)(HW_OBJ_PLTT + pal_addr), 0x20);
		}
	}

	after_point = cs->ccts.total_len / CC_POINT_DOT;
	if(before_point != after_point){
		CCLocal_PointMessagePut(cs, after_point);
		OS_TPrintf("after_point = %d\n", after_point);
	}
	
	//赤外線で繋がっていれば背景色変更
	if(irc_ret == COMPATIIRC_RESULT_CONNECT){
		u16 set_color = BG_COLOR_IRC;
		GFL_STD_MemCopy16(&set_color, (void*)(HW_BG_PLTT + BG_COLOR_IRC_COLOR_POS), 2);
		GFL_STD_MemCopy16(&set_color, (void*)(HW_DB_BG_PLTT + BG_COLOR_IRC_COLOR_POS), 2);
	}
	else{
		GFL_STD_MemCopy16(&cs->bg_color, (void*)(HW_BG_PLTT + BG_COLOR_IRC_COLOR_POS), 2);
		GFL_STD_MemCopy16(&cs->bg_color, (void*)(HW_DB_BG_PLTT + BG_COLOR_IRC_COLOR_POS), 2);
	}
	
	return SEQ_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   メインシーケンス：終了処理
 * @param   cs		
 * @retval  シーケンス動作
 */
//--------------------------------------------------------------
static int CCSeq_End(COMPATI_SYS *cs)
{
	switch(cs->local_seq){
	case 0:
		WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, 
			WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_COMPATI);
		cs->local_seq++;
		break;
	case 1:
		if(WIPE_SYS_EndCheck() == TRUE){
			return SEQ_FINISH;
		}
		break;
	}
	return SEQ_CONTINUE;
}
