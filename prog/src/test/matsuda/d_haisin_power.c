//==============================================================================
/**
 * @file    d_haisin_power.c
 * @brief   Gパワー配信プログラム
 * @author  matsuda
 * @date    2010.08.18(水)
 */
//==============================================================================
#include <gflib.h>
#ifdef PM_DEBUG
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include <backup_system.h>
//#include "savedata/contest_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"

#include "net\network_define.h"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "message.naix"
#include "debug_message.naix"
#include "test_graphic\d_taya.naix"
#include "msg\debug\msg_d_matsu.h"
#include "msg\msg_power.h"
#include "test/performance.h"
#include "font/font.naix"

#include "system/bmp_menu.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_menulist.h"
#include "print/wordset.h"

#include "d_haisin_power.h"


//==============================================================================
//	定数定義
//==============================================================================
#define D_POWER_CLEAR	(1)		///CRC + FOOTER + clear

#define MOVEMODE_RANDOM (0)
#define MOVEMODE_NO (1)


#define _TOOL_GPOWER_ID_MAX (GPOWER_ID_MAX-2)



enum{
	WIN_NAME,
	WIN_SELF,
	WIN_CHARA,
	WIN_BEACON,
	
	WIN_POWER_0,
	WIN_POWER_1,
	WIN_POWER_2,
	WIN_POWER_3,
	WIN_POWER_4,
	WIN_POWER_5,
	WIN_POWER_6,
	WIN_POWER_7,
	WIN_POWER_MAX,
	
	WIN_TIME_0,
	WIN_TIME_1,
	WIN_TIME_2,
	WIN_TIME_3,
	WIN_TIME_4,
	WIN_TIME_5,
	WIN_TIME_6,
	WIN_TIME_7,
	WIN_TIME_MAX,
	
	WIN_PAR_0,
	WIN_PAR_1,
	WIN_PAR_2,
	WIN_PAR_3,
	WIN_PAR_4,
	WIN_PAR_5,
	WIN_PAR_6,
	WIN_PAR_7,
	WIN_PAR_MAX,

  WIN_START_TIME,
  WIN_END_TIME,
  WIN_MODE,
	WIN_SAVE,
	
	WIN_MENU,
	WIN_INFO,
	
	WIN_MAX,
};

//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	GFL_BMPWIN		*win;
	GFL_BMP_DATA	*bmp;
	PRINT_UTIL		printUtil[1];
	BOOL			message_req;		///<TRUE:メッセージリクエストがあった
}DM_MSG_DRAW_WIN;

typedef struct {
	GFL_FONT		*fontHandle;
	PRINT_QUE		*printQue;
	PRINT_STREAM	*printStream;
	GFL_MSGDATA		*mm;
	GFL_MSGDATA		*mm_power;
	GFL_TCBLSYS		*tcbl;
	DM_MSG_DRAW_WIN drawwin[WIN_MAX];
	
	WORDSET			*wordset;
	BMPMENULIST_HEADER	bmphead;
	BMPMENULIST_WORK *bmpmenu_list;
	BMP_MENULIST_DATA *list_data;
	
	HAISIN_CONTROL_WORK *control;
	int active_win;
}D_MATSU_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void DebugMenuHeadCreate(D_MATSU_WORK *wk, BMPMENULIST_HEADER *bmphead);
static void DebugMenuListCreate(D_MATSU_WORK *wk);
static void DebugMenuListDelete(D_MATSU_WORK *wk);
static void Local_MessagePut(D_MATSU_WORK *wk, int win_index, STRBUF *strbuf, int x, int y, int color_type);

static void _PrintAllMessage(D_MATSU_WORK *wk);
static void _PrintMachineName(D_MATSU_WORK *wk);
static void _PrintSelfMessage(D_MATSU_WORK *wk);
static void _PrintUnionChara(D_MATSU_WORK *wk);
static void _PrintBeaconSpaceTime(D_MATSU_WORK *wk);
static void _PrintSave(D_MATSU_WORK *wk);
static void _PrintMode(D_MATSU_WORK *wk);
static void _PrintStartTime(D_MATSU_WORK *wk);
static void _PrintEndTime(D_MATSU_WORK *wk);
static void _PrintPowerName(D_MATSU_WORK *wk, int win_no);
static void _PrintPowerTime(D_MATSU_WORK *wk, int win_no);
static void _PrintPowerOdds(D_MATSU_WORK *wk, int win_no);
static void _PrintWinTarget(D_MATSU_WORK *wk, int bmpwin_no);
static void _CursorMove(D_MATSU_WORK *wk);


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void HaisinData_Save(HAISIN_CONTROL_WORK *control);
extern BOOL HaisinData_Load(HAISIN_CONTROL_WORK *control);

//==============================================================================
//	外部データ
//==============================================================================
extern const GFL_PROC_DATA HaisinMenuProcData;


//==============================================================================
//	データ
//==============================================================================
///BMPメニューヘッダ
static const BMPMENULIST_HEADER DebugSaveBmpMenuHead = {
	NULL,						//const BMP_MENULIST_DATA *list;		//表示文字データポインタ
	NULL,						//void	*call_back;	//カーソル移動ごとのコールバック関数
	NULL,						//void	*icon;		//一列表示ごとのコールバック関数
	NULL,						//GFL_BMPWIN *win;
	1,							//u16		count;		//リスト項目数
	10,							//u16		line;		//表示最大項目数
	0,							//u8		rabel_x;	//ラベル表示Ｘ座標
	11, 						//u8		data_x;		//項目表示Ｘ座標
	0,							//u8		cursor_x;	//カーソル表示Ｘ座標
	0,							//u8		line_y:4;	//表示Ｙ座標
	1,							//u8		f_col:4;	//表示文字色
	15, 						//u8		b_col:4;	//表示背景色
	2,							//u8		s_col:4;	//表示文字影色
	0, 							//u16		msg_spc:3;	//文字間隔Ｘ
	0, 							//u16		line_spc:4;	//文字間隔Ｙ
	BMPMENULIST_LRKEY_SKIP, 	//u16		page_skip:2;//ページスキップタイプ
	0,							//u16		font:6;		//文字指定
	0,							//u16		c_disp_f:1;	//ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)
	NULL,						//void * work;
	12, 						//u16 font_size_x;		//文字サイズX(ドット
	16, 						//u16 font_size_y;		//文字サイズY(ドット
	NULL,						//GFL_MSGDATA *msgdata;	//表示に使用するメッセージバッファ
	NULL,						//PRINT_UTIL *print_util; //表示に使用するプリントユーティリティ
	NULL,						//PRINT_QUE *print_que;	//表示に使用するプリントキュー
	NULL,						//GFL_FONT *font_handle;	//表示に使用するフォントハンドル
};


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT HaisinMenuProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	static const GFL_DISP_VRAM vramBank = {
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
		GX_OBJVRAMMODE_CHAR_1D_32K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
	};

	D_MATSU_WORK* wk;
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MATSUDA_DEBUG, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(D_MATSU_WORK), HEAPID_MATSUDA_DEBUG );
	MI_CpuClear8(wk, sizeof(D_MATSU_WORK));
	wk->control = pwk;

	GFL_DISP_SetBank( &vramBank );

	//バックグラウンドの色を入れておく
	GFL_STD_MemFill16((void*)HW_BG_PLTT, 0x5ad6, 2);
	
	// 各種効果レジスタ初期化
	G2_BlendNone();

	// BGsystem初期化
	GFL_BG_Init( HEAPID_MATSUDA_DEBUG );
	GFL_BMPWIN_Init( HEAPID_MATSUDA_DEBUG );
	GFL_FONTSYS_Init();

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}

	// 個別フレーム設定
	{
		static const GFL_BG_BGCNT_HEADER bgcntText[] = {
			{
  			0, 0, 0x800, 0,
  			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
  			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  		},
			{
  			0, 0, 0x800, 0,
  			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  			GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x18000, 0x8000,
  			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  		},
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText[0],   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M,   &bgcntText[1],   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText[0],   GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, HEAPID_MATSUDA_DEBUG );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0, 1, 0 );
		GFL_BG_FillCharacter( GFL_BG_FRAME1_M, 0xff, 1, 0 );

//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_FillScreen( GFL_BG_FRAME1_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	// 上下画面設定
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );

  //BMPWIN作成
  {
    int i;
    
		wk->drawwin[WIN_MENU].win 
		  = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 10, 1, 19, 20, 0, GFL_BMP_CHRAREA_GET_F );
		
		wk->drawwin[WIN_INFO].win 
		  = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 19, 24-2, 10, 2, 0, GFL_BMP_CHRAREA_GET_F );

    wk->drawwin[WIN_NAME].win 
      = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 1, 0, 14, 2, 0, GFL_BMP_CHRAREA_GET_F );

    wk->drawwin[WIN_SELF].win 
      = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 15, 0, 14, 2, 0, GFL_BMP_CHRAREA_GET_F );

    wk->drawwin[WIN_CHARA].win 
      = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 1, 2, 14, 2, 0, GFL_BMP_CHRAREA_GET_F );
    wk->drawwin[WIN_BEACON].win 
      = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 17, 2, 16, 2, 0, GFL_BMP_CHRAREA_GET_F );

    wk->drawwin[WIN_START_TIME].win 
      = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 1, 24-4, 9, 2, 0, GFL_BMP_CHRAREA_GET_F );
    wk->drawwin[WIN_END_TIME].win 
      = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 11, 24-4, 9, 2, 0, GFL_BMP_CHRAREA_GET_F );
    wk->drawwin[WIN_MODE].win 
      = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 22, 24-4, 8, 2, 0, GFL_BMP_CHRAREA_GET_F );

    wk->drawwin[WIN_SAVE].win 
      = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 4, 24-2, 12, 2, 0, GFL_BMP_CHRAREA_GET_F );
    
    for(i = 0; i < WIN_POWER_MAX - WIN_POWER_0; i++){
      wk->drawwin[WIN_POWER_0 + i].win 
        = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 1, 4 + i*2, 17, 2, 0, GFL_BMP_CHRAREA_GET_F );
      wk->drawwin[WIN_TIME_0 + i].win 
        = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 1+18+1, 4 + i*2, 5, 2, 0, GFL_BMP_CHRAREA_GET_F );
      wk->drawwin[WIN_PAR_0 + i].win 
        = GFL_BMPWIN_Create( GFL_BG_FRAME1_M, 1+18+7, 4 + i*2, 5, 2, 0, GFL_BMP_CHRAREA_GET_F );
    }
    
    for(i = 0; i < WIN_MAX; i++){
      if(wk->drawwin[i].win != NULL){
        wk->drawwin[i].bmp = GFL_BMPWIN_GetBmp(wk->drawwin[i].win);
        GFL_BMP_Clear(wk->drawwin[i].bmp, 0xff);
  			GFL_BMPWIN_MakeScreen( wk->drawwin[i].win );
  			GFL_BMPWIN_TransVramCharacter( wk->drawwin[i].win );
  			PRINT_UTIL_Setup( wk->drawwin[i].printUtil, wk->drawwin[i].win );
  		}
  	}

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
  }

	wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_MATSUDA_DEBUG );

//		PRINTSYS_Init( HEAPID_MATSUDA_DEBUG );
	wk->printQue = PRINTSYS_QUE_Create( HEAPID_MATSUDA_DEBUG );

	wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_matsu_dat, HEAPID_MATSUDA_DEBUG );
	wk->mm_power = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_power_dat, HEAPID_MATSUDA_DEBUG );

	wk->wordset = WORDSET_Create(HEAPID_MATSUDA_DEBUG);

	wk->tcbl = GFL_TCBL_Init( HEAPID_MATSUDA_DEBUG, HEAPID_MATSUDA_DEBUG, 32, 32 );

	GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI

	//フォントパレット転送
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		0, 0x20, HEAPID_MATSUDA_DEBUG);

  //現在の情報を全体描画
  _PrintAllMessage(wk);

  //スリープしない設計にする
  GFL_UI_SleepDisable(GFL_UI_SLEEP_MIC);

  
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT HaisinMenuProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	BOOL ret = 0;
	int i;
	BOOL que_ret;
	u32 bmp_ret;
	u32 write_data = 0x94a1;
	int save_a, save_b;
	u32 trg = GFL_UI_KEY_GetTrg();
	
	GFL_TCBL_Main( wk->tcbl );
	
	que_ret = PRINTSYS_QUE_Main( wk->printQue );
	for(i = 0; i < WIN_MAX; i++){
    if(wk->drawwin[i].win != NULL){
  		if( PRINT_UTIL_Trans( wk->drawwin[i].printUtil, wk->printQue ) == FALSE){
  			//return GFL_PROC_RES_CONTINUE;
  		}
  		else{
  			if(1){//que_ret == TRUE && wk->drawwin[i].message_req == TRUE){
  	//			GFL_BMP_Clear( wk->bmp, 0xff );
  				GFL_BMPWIN_TransVramCharacter( wk->drawwin[i].win );
  				wk->drawwin[i].message_req = FALSE;
  			}
  		}
  	}
	}

  if(wk->bmpmenu_list == NULL){
    _CursorMove(wk);
    
    if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_A){
      if(wk->active_win == WIN_NAME){
        wk->control->return_mode = HAISIN_RETURN_MODE_NAMEIN;
        return GFL_PROC_RES_FINISH;
      }
      else if(wk->active_win == WIN_SELF){
        wk->control->return_mode = HAISIN_RETURN_MODE_SELFMSG;
        return GFL_PROC_RES_FINISH;
      }
      else if(wk->active_win >= WIN_POWER_0 && wk->active_win < WIN_POWER_MAX){
      	//BMPメニュー作成
      	DebugMenuListCreate(wk);
      	DebugMenuHeadCreate(wk, &wk->bmphead);
      	wk->bmpmenu_list = BmpMenuList_Set(&wk->bmphead, 0, 0, HEAPID_MATSUDA_DEBUG);
      	BmpMenuList_SetCursorBmp(wk->bmpmenu_list, HEAPID_MATSUDA_DEBUG);
    		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
      }
      else if(wk->active_win == WIN_SAVE){
        HaisinData_Save(wk->control);
      }
    }
    if(wk->active_win == WIN_MODE){
      if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A){
        wk->control->powergroup.movemode = 1 - wk->control->powergroup.movemode;
        _PrintMode(wk);
        {
          int z;
          for(z = 0; z < WIN_POWER_MAX - WIN_POWER_0; z++){
            _PrintPowerOdds(wk, z);
          }
        }
      }
    }
    if(wk->active_win == WIN_CHARA){
      if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A){
        wk->control->union_index++;
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_B){
        wk->control->union_index--;
      }

      if(wk->control->union_index >= MYSTATUS_UNIONVIEW_MAX*2){
        wk->control->union_index = 0;
      }
      else if(wk->control->union_index < 0){
        wk->control->union_index = MYSTATUS_UNIONVIEW_MAX*2 - 1;
      }
      
      if(GFL_UI_KEY_GetRepeat() & (PAD_BUTTON_A | PAD_BUTTON_B)){
        _PrintWinTarget(wk, wk->active_win);
      }
    }
    if(wk->active_win == WIN_BEACON){
      if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A){
        wk->control->powergroup.beacon_space_time++;
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R){
        wk->control->powergroup.beacon_space_time += 10;
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_B){
        wk->control->powergroup.beacon_space_time--;
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L){
        wk->control->powergroup.beacon_space_time -= 10;
      }
      if(wk->control->powergroup.beacon_space_time <= 0){
        wk->control->powergroup.beacon_space_time = 1;
      }
      if(wk->control->powergroup.beacon_space_time > 999){
        wk->control->powergroup.beacon_space_time = 999;
      }
      if(GFL_UI_KEY_GetRepeat() & (PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_L | PAD_BUTTON_R)){
        _PrintWinTarget(wk, wk->active_win);
      }
    }
    if(wk->active_win == WIN_START_TIME || wk->active_win == WIN_END_TIME){
      s32 hour, min;
      if(wk->active_win == WIN_START_TIME){
        hour = wk->control->powergroup.start_time_hour;
        min = wk->control->powergroup.start_time_min;
      }
      else{
        hour = wk->control->powergroup.end_time_hour;
        min = wk->control->powergroup.end_time_min;
      }
      if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A){
        min += 30;
        if(min >= 60){
          min = 0;
          hour++;
        }
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_B){
        min += 30;
        if(min == 30){
          hour--;
        }
        else if(min >= 60){
          min = 0;
        }
      }
      if(hour > 23){
        hour = 0;
      }
      else if(hour < 0){
        hour = 23;
      }
      if(wk->active_win == WIN_START_TIME){
        wk->control->powergroup.start_time_hour = hour;
        wk->control->powergroup.start_time_min = min;
      }
      else{
        wk->control->powergroup.end_time_hour = hour;
        wk->control->powergroup.end_time_min = min;
      }
      if(GFL_UI_KEY_GetRepeat() & (PAD_BUTTON_A | PAD_BUTTON_B)){
        _PrintWinTarget(wk, wk->active_win);
      }
    }
    if(wk->active_win >= WIN_TIME_0 && wk->active_win < WIN_TIME_MAX){
      if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A){
        wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time++;
        if(wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time > 999){
          wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time = 999;
        }
        _PrintWinTarget(wk, wk->active_win);
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R){
        wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time += 10;
        if(wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time > 999){
          wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time = 999;
        }
        _PrintWinTarget(wk, wk->active_win);
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_B){
        wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time--;
        if(wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time < 0){
          wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time = 0;
        }
        _PrintWinTarget(wk, wk->active_win);
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L){
        wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time -= 10;
        if(wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time < 0){
          wk->control->powergroup.hp[wk->active_win - WIN_TIME_0].time = 0;
        }
        _PrintWinTarget(wk, wk->active_win);
      }
    }
    if(wk->active_win >= WIN_PAR_0 && wk->active_win < WIN_PAR_MAX){
      if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A){
        wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds++;
        if(wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds > 100){
          wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds = 100;
        }
        _PrintWinTarget(wk, wk->active_win);
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R){
        wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds += 10;
        if(wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds > 100){
          wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds = 100;
        }
        _PrintWinTarget(wk, wk->active_win);
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_B){
        wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds--;
        if(wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds < 0){
          wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds = 0;
        }
        _PrintWinTarget(wk, wk->active_win);
      }
      else if(GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L){
        wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds -= 10;
        if(wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds < 0){
          wk->control->powergroup.hp[wk->active_win - WIN_PAR_0].odds = 0;
        }
        _PrintWinTarget(wk, wk->active_win);
      }
    }
    if((GFL_UI_KEY_GetTrg() & PAD_BUTTON_START) ){
      int i, power_count = 0;
      
      for(i = 0; i < HAISIN_POWER_MAX; i++){
        if(wk->control->powergroup.hp[i].g_power_id < _TOOL_GPOWER_ID_MAX){
          power_count++;
        }
      }
      if(power_count != 0){
        wk->control->powergroup.data_num = power_count;
        wk->control->return_mode = HAISIN_RETURN_MODE_START;
        return GFL_PROC_RES_FINISH;
      }
    }
  }
  else{
  	bmp_ret = BmpMenuList_Main(wk->bmpmenu_list);
  	switch(bmp_ret){
  	case BMPMENU_NULL:
  		break;
  	case BMPMENU_CANCEL:
    	BmpMenuList_Exit(wk->bmpmenu_list, NULL, NULL);
    	wk->bmpmenu_list = NULL;
      DebugMenuListDelete(wk);
    	//GFL_BG_ClearScreenCode(GFL_BG_FRAME0_M, 0);
  		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
  		break;
  	default:
  		if(_TOOL_GPOWER_ID_MAX > bmp_ret){
        wk->control->powergroup.hp[wk->active_win - WIN_POWER_0].g_power_id = bmp_ret;
      }
      else{
        wk->control->powergroup.hp[wk->active_win - WIN_POWER_0].g_power_id = GPOWER_ID_NULL;
      }
    	BmpMenuList_Exit(wk->bmpmenu_list, NULL, NULL);
    	wk->bmpmenu_list = NULL;
      DebugMenuListDelete(wk);
    	//GFL_BG_ClearScreenCode(GFL_BG_FRAME0_M, 0);
  		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
      _PrintWinTarget(wk, wk->active_win);
  		break;
  	}
  }
	
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT HaisinMenuProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	int i;
	
	for(i = 0; i < WIN_MAX; i++){
    if(wk->drawwin[i].win != NULL){
  		GFL_BMPWIN_Delete(wk->drawwin[i].win);
  	}
	}
	
	PRINTSYS_QUE_Delete(wk->printQue);
	GFL_MSG_Delete(wk->mm);
	GFL_MSG_Delete(wk->mm_power);
	WORDSET_Delete(wk->wordset);
	
	GFL_FONT_Delete(wk->fontHandle);
	GFL_TCBL_Exit(wk->tcbl);
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_MATSUDA_DEBUG);
	
	OS_TPrintf("リスト終了\n");
	return GFL_PROC_RES_FINISH;
}



//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   BMPメニューヘッダを作成
 *
 * @param   wk			
 * @param   bmphead		
 *
 * 先にDebugMenuListCreateでメニューリストを作成しておく必要があります
 */
//--------------------------------------------------------------
static void DebugMenuHeadCreate(D_MATSU_WORK *wk, BMPMENULIST_HEADER *bmphead)
{
	GF_ASSERT(wk->list_data != NULL);
	
	*bmphead = DebugSaveBmpMenuHead;
	bmphead->list = wk->list_data;
	bmphead->win = wk->drawwin[WIN_MENU].win;
	bmphead->count = _TOOL_GPOWER_ID_MAX + D_POWER_CLEAR;
	bmphead->msgdata = wk->mm;
	bmphead->print_util = wk->drawwin[WIN_MENU].printUtil;
	bmphead->print_que = wk->printQue;
	bmphead->font_handle = wk->fontHandle;
}

//--------------------------------------------------------------
/**
 * @brief   メニューリストを作成する
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void DebugMenuListCreate(D_MATSU_WORK *wk)
{
	BMP_MENULIST_DATA *list_data;
	int save_tbl_max, i;
	STRBUF *tempbuf, *status, *destsrc;
	BOOL ret_a, ret_b;
	
	save_tbl_max = _TOOL_GPOWER_ID_MAX + D_POWER_CLEAR;
	list_data = BmpMenuWork_ListCreate(save_tbl_max, HEAPID_MATSUDA_DEBUG);
	
	for(i = 0; i < _TOOL_GPOWER_ID_MAX; i++){
		list_data[i].param = i;
		destsrc = GFL_MSG_CreateString(wk->mm_power, i);
		list_data[i].str = destsrc;
	}

	//「設定無し」
	destsrc = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_001);
	list_data[_TOOL_GPOWER_ID_MAX + 0].str = destsrc;
	list_data[_TOOL_GPOWER_ID_MAX + 0].param = _TOOL_GPOWER_ID_MAX + 0;

	wk->list_data = list_data;
}

//--------------------------------------------------------------
/**
 * @brief   メニューリストを削除する
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void DebugMenuListDelete(D_MATSU_WORK *wk)
{
	int save_tbl_max, i;
	
	save_tbl_max = _TOOL_GPOWER_ID_MAX + D_POWER_CLEAR;
	for(i = 0; i < save_tbl_max; i++){
//		GFL_STR_DeleteBuffer((STRBUF*)(wk->list_data[i].str));
	}
	BmpMenuWork_ListDelete(wk->list_data);
}

//--------------------------------------------------------------
/**
 * @brief   メッセージを表示する
 *
 * @param   wk			
 * @param   strbuf		表示するメッセージデータ
 * @param   x			X座標
 * @param   y			Y座標
 */
//--------------------------------------------------------------
static void Local_MessagePut(D_MATSU_WORK *wk, int win_index, STRBUF *strbuf, int x, int y, int color_type)
{
	GFL_BMP_Clear( wk->drawwin[win_index].bmp, 0xff );
	if(color_type == 0){
  	PRINTSYS_PrintQue(wk->printQue, wk->drawwin[win_index].bmp, x, y, strbuf, wk->fontHandle);
  }
  else{
  	PRINTSYS_PrintQueColor(wk->printQue, wk->drawwin[win_index].bmp, x, y, strbuf, wk->fontHandle,
  	  PRINTSYS_MACRO_LSB(4, 2, 15));
  }
	wk->drawwin[win_index].message_req = TRUE;
}

//--------------------------------------------------------------
/**
 * 全画面を再描画
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintAllMessage(D_MATSU_WORK *wk)
{
  STRBUF *tempbuf;
  int i;
  
  //名前
  _PrintMachineName(wk);
  _PrintSelfMessage(wk);
  _PrintUnionChara(wk);
  _PrintBeaconSpaceTime(wk);
  _PrintMode(wk);
  _PrintSave(wk);
  _PrintStartTime(wk);
  _PrintEndTime(wk);
  
  //「STARTで開始」
	tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_002);
  Local_MessagePut(wk, WIN_INFO, tempbuf, 0,0,0);
  GFL_STR_DeleteBuffer(tempbuf);
  
  //Gパワー
  for(i = 0; i < WIN_POWER_MAX - WIN_POWER_0; i++){
    _PrintPowerName(wk, i);
    _PrintPowerTime(wk, i);
    _PrintPowerOdds(wk, i);
  }
}

//--------------------------------------------------------------
/**
 * 配信マシンの名前を描画
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintMachineName(D_MATSU_WORK *wk)
{
  Local_MessagePut(wk, WIN_NAME, wk->control->name, 0,0, wk->active_win==WIN_NAME);
}

//--------------------------------------------------------------
/**
 * 一言メッセージを描画
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintSelfMessage(D_MATSU_WORK *wk)
{
  Local_MessagePut(wk, WIN_SELF, wk->control->selfmsg, 0,0, wk->active_win==WIN_SELF);
}

//--------------------------------------------------------------
/**
 * ユニオンの見た目表示
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintUnionChara(D_MATSU_WORK *wk)
{
  STRBUF *tempbuf;

  tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_006 + wk->control->union_index);
  Local_MessagePut(wk, WIN_CHARA, tempbuf, 0,0, wk->active_win==WIN_CHARA);
  GFL_STR_DeleteBuffer(tempbuf);
}

//--------------------------------------------------------------
/**
 * ビーコン配信間隔
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintBeaconSpaceTime(D_MATSU_WORK *wk)
{
  STRBUF *tempbuf, *srcbuf;

  WORDSET_RegisterNumber(wk->wordset, 0, wk->control->powergroup.beacon_space_time, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_025);
  srcbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_025);

	WORDSET_ExpandStr(wk->wordset, tempbuf, srcbuf);
  Local_MessagePut(wk, WIN_BEACON, tempbuf, 0,0, wk->active_win==WIN_BEACON);

  GFL_STR_DeleteBuffer(srcbuf);
  GFL_STR_DeleteBuffer(tempbuf);
}

//--------------------------------------------------------------
/**
 * 「設定をセーブする」
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintSave(D_MATSU_WORK *wk)
{
  STRBUF *tempbuf;

  tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_022);
  Local_MessagePut(wk, WIN_SAVE, tempbuf, 0,0, wk->active_win==WIN_SAVE);
  GFL_STR_DeleteBuffer(tempbuf);
}

//--------------------------------------------------------------
/**
 * 「モードの切り替え」
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintMode(D_MATSU_WORK *wk)
{
  STRBUF *tempbuf;

  tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_026 + wk->control->powergroup.movemode);
  Local_MessagePut(wk, WIN_MODE, tempbuf, 0,0, wk->active_win==WIN_MODE);
  GFL_STR_DeleteBuffer(tempbuf);
}

//--------------------------------------------------------------
/**
 * 開始時間
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintStartTime(D_MATSU_WORK *wk)
{
  STRBUF *tempbuf, *srcbuf;

  WORDSET_RegisterNumber(wk->wordset, 0, wk->control->powergroup.start_time_hour, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber(wk->wordset, 1, wk->control->powergroup.start_time_min, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_023);
  srcbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_023);

	WORDSET_ExpandStr(wk->wordset, tempbuf, srcbuf);
  Local_MessagePut(wk, WIN_START_TIME, tempbuf, 0,0, wk->active_win==WIN_START_TIME);

  GFL_STR_DeleteBuffer(srcbuf);
  GFL_STR_DeleteBuffer(tempbuf);
}

//--------------------------------------------------------------
/**
 * 開始時間
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _PrintEndTime(D_MATSU_WORK *wk)
{
  STRBUF *tempbuf, *srcbuf;

  WORDSET_RegisterNumber(wk->wordset, 0, wk->control->powergroup.end_time_hour, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WORDSET_RegisterNumber(wk->wordset, 1, wk->control->powergroup.end_time_min, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_024);
  srcbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_024);

	WORDSET_ExpandStr(wk->wordset, tempbuf, srcbuf);
  Local_MessagePut(wk, WIN_END_TIME, tempbuf, 0,0, wk->active_win==WIN_END_TIME);

  GFL_STR_DeleteBuffer(srcbuf);
  GFL_STR_DeleteBuffer(tempbuf);
}

//--------------------------------------------------------------
/**
 * 配信パワー名を描画
 *
 * @param   wk		
 * @param   win_no		
 */
//--------------------------------------------------------------
static void _PrintPowerName(D_MATSU_WORK *wk, int win_no)
{
  STRBUF *tempbuf;
  
  if(wk->control->powergroup.hp[win_no].g_power_id < _TOOL_GPOWER_ID_MAX){
    tempbuf = GFL_MSG_CreateString(wk->mm_power, wk->control->powergroup.hp[win_no].g_power_id);
    Local_MessagePut(wk, WIN_POWER_0 + win_no, tempbuf, 0,0, wk->active_win==WIN_POWER_0+win_no);
    GFL_STR_DeleteBuffer(tempbuf);
  }
  else{
	  tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_001);
    Local_MessagePut(wk, WIN_POWER_0 + win_no, tempbuf, 0,0, wk->active_win==WIN_POWER_0+win_no);
    GFL_STR_DeleteBuffer(tempbuf);
  }
}

//--------------------------------------------------------------
/**
 * 配信パワーの時間を描画
 *
 * @param   wk		
 * @param   win_no		
 */
//--------------------------------------------------------------
static void _PrintPowerTime(D_MATSU_WORK *wk, int win_no)
{
  u32 time = wk->control->powergroup.hp[win_no].time;
  STRBUF *tempbuf, *srcbuf;
  
  WORDSET_RegisterNumber(wk->wordset, 0, time, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
	srcbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_003);
  tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_003);

	WORDSET_ExpandStr(wk->wordset, tempbuf, srcbuf);
  Local_MessagePut(wk, WIN_TIME_0 + win_no, tempbuf, 0,0, wk->active_win==WIN_TIME_0+win_no);
  
	GFL_STR_DeleteBuffer(srcbuf);
	GFL_STR_DeleteBuffer(tempbuf);
}

//--------------------------------------------------------------
/**
 * 配信パワーの確率を描画
 *
 * @param   wk		
 * @param   win_no		
 */
//--------------------------------------------------------------
static void _PrintPowerOdds(D_MATSU_WORK *wk, int win_no)
{
  u32 time = wk->control->powergroup.hp[win_no].odds;
  STRBUF *tempbuf, *srcbuf;

  
  if( wk->control->powergroup.movemode == MOVEMODE_RANDOM){
  
    WORDSET_RegisterNumber(wk->wordset, 0, time, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    srcbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_004);
    tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_004);
    
    WORDSET_ExpandStr(wk->wordset, tempbuf, srcbuf);
    Local_MessagePut(wk, WIN_PAR_0 + win_no, tempbuf, 0,0, wk->active_win==WIN_PAR_0+win_no);
    
    GFL_STR_DeleteBuffer(srcbuf);
    GFL_STR_DeleteBuffer(tempbuf);

  }
  else{

    WORDSET_RegisterNumber(wk->wordset, 0, win_no, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    srcbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_028);
    tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_HAISIN_028);
    
    WORDSET_ExpandStr(wk->wordset, tempbuf, srcbuf);
    Local_MessagePut(wk, WIN_PAR_0 + win_no, tempbuf, 0,0, wk->active_win==WIN_PAR_0+win_no);
    
    GFL_STR_DeleteBuffer(srcbuf);
    GFL_STR_DeleteBuffer(tempbuf);

  }

}

//--------------------------------------------------------------
/**
 * BMPWIN番号から描画する項目を引き出し、描画を実行する
 *
 * @param   wk		
 * @param   bmpwin_no		
 */
//--------------------------------------------------------------
static void _PrintWinTarget(D_MATSU_WORK *wk, int bmpwin_no)
{
  if(bmpwin_no == WIN_NAME){
    _PrintMachineName(wk);
  }
  else if(bmpwin_no == WIN_SELF){
    _PrintSelfMessage(wk);
  }
  else if(bmpwin_no == WIN_CHARA){
    _PrintUnionChara(wk);
  }
  else if(bmpwin_no == WIN_BEACON){
    _PrintBeaconSpaceTime(wk);
  }
  else if(bmpwin_no >= WIN_POWER_0 && bmpwin_no < WIN_POWER_MAX){
    _PrintPowerName(wk, bmpwin_no - WIN_POWER_0);
  }
  else if(bmpwin_no >= WIN_TIME_0 && bmpwin_no < WIN_TIME_MAX){
    _PrintPowerTime(wk, bmpwin_no - WIN_TIME_0);
  }
  else if(bmpwin_no >= WIN_PAR_0 && bmpwin_no < WIN_PAR_MAX){
    _PrintPowerOdds(wk, bmpwin_no - WIN_PAR_0);
  }
  else if(bmpwin_no == WIN_SAVE){
    _PrintSave(wk);
  }
  else if(bmpwin_no == WIN_MODE){
    _PrintMode(wk);
  }
  else if(bmpwin_no == WIN_START_TIME){
    _PrintStartTime(wk);
  }
  else if(bmpwin_no == WIN_END_TIME){
    _PrintEndTime(wk);
  }
}

//--------------------------------------------------------------
/**
 * カーソル移動
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _CursorMove(D_MATSU_WORK *wk)
{
  u32 trg = GFL_UI_KEY_GetTrg();
  int backup_win;
  
  backup_win = wk->active_win;
  
  if(trg & PAD_KEY_DOWN){
    if(wk->active_win == WIN_NAME || wk->active_win == WIN_SELF){
      wk->active_win = WIN_CHARA;
    }
    else if(wk->active_win == WIN_CHARA || wk->active_win == WIN_BEACON){
      wk->active_win = WIN_POWER_0;
    }
    else if((wk->active_win >= WIN_POWER_0 && wk->active_win < WIN_POWER_MAX-1)
        || (wk->active_win >= WIN_TIME_0 && wk->active_win < WIN_TIME_MAX-1)
        || (wk->active_win >= WIN_PAR_0 && wk->active_win < WIN_PAR_MAX-1)){
      wk->active_win++;
    }
    else if(wk->active_win == WIN_POWER_MAX-1 || wk->active_win == WIN_TIME_MAX-1 
        || wk->active_win == WIN_PAR_MAX-1){
      wk->active_win = WIN_START_TIME;
    }
    else if(wk->active_win == WIN_START_TIME || wk->active_win == WIN_END_TIME){
      wk->active_win = WIN_SAVE;
    }
  }
  else if(trg & PAD_KEY_UP){
    if(wk->active_win == WIN_POWER_0 || wk->active_win == WIN_TIME_0 || wk->active_win==WIN_PAR_0){
      wk->active_win = WIN_CHARA;
    }
    else if(wk->active_win == WIN_CHARA || wk->active_win == WIN_BEACON){
      wk->active_win = WIN_NAME;
    }
    else if((wk->active_win >= WIN_POWER_1 && wk->active_win < WIN_POWER_MAX)
        || (wk->active_win >= WIN_TIME_1 && wk->active_win < WIN_TIME_MAX)
        || (wk->active_win >= WIN_PAR_1 && wk->active_win < WIN_PAR_MAX)){
      wk->active_win--;
    }
    else if(wk->active_win == WIN_START_TIME || wk->active_win == WIN_END_TIME){
      wk->active_win = WIN_POWER_MAX-1;
    }
    else if(wk->active_win == WIN_SAVE){
      wk->active_win = WIN_START_TIME;
    }
  }
  else if(trg & PAD_KEY_RIGHT){
    if(wk->active_win == WIN_CHARA){
      wk->active_win = WIN_BEACON;
    }
    else if((wk->active_win >= WIN_POWER_0 && wk->active_win < WIN_POWER_MAX)
        || (wk->active_win >= WIN_TIME_0 && wk->active_win < WIN_TIME_MAX)){
      wk->active_win += (WIN_POWER_MAX - WIN_POWER_0) + 1;
    }
    else if(wk->active_win == WIN_NAME){
      wk->active_win = WIN_SELF;
    }
    else if(wk->active_win == WIN_START_TIME){
      wk->active_win = WIN_END_TIME;
    }
    else if(wk->active_win == WIN_END_TIME){
      wk->active_win = WIN_MODE;
    }
  }
  else if(trg & PAD_KEY_LEFT){
    if(wk->active_win == WIN_BEACON){
      wk->active_win = WIN_CHARA;
    }
    else if((wk->active_win >= WIN_TIME_0 && wk->active_win < WIN_TIME_MAX)
        || (wk->active_win >= WIN_PAR_0 && wk->active_win < WIN_PAR_MAX)){
      wk->active_win -= (WIN_POWER_MAX - WIN_POWER_0) + 1;
    }
    else if(wk->active_win == WIN_SELF){
      wk->active_win = WIN_NAME;
    }
    else if(wk->active_win == WIN_END_TIME){
      wk->active_win = WIN_START_TIME;
    }
    else if(wk->active_win == WIN_MODE){
      wk->active_win = WIN_END_TIME;
    }
  }
  
  if(wk->active_win != backup_win){
    _PrintWinTarget(wk, backup_win);
    _PrintWinTarget(wk, wk->active_win);
  }
}


//==============================================================================
//	
//==============================================================================
//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA HaisinMenuProcData = {
	HaisinMenuProcInit,
	HaisinMenuProcMain,
	HaisinMenuProcEnd,
};

#endif //PM_DEBUG

