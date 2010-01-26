//==============================================================================
/**
 * @file    union_subdisp.c
 * @brief   ユニオン下画面
 * @author  matsuda
 * @date    2009.07.27(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "net_app/union/union_main.h"
#include "net_app/union/union_subdisp.h"
#include "unionroom.naix"
#include "system\pms_data.h"
#include "print\printsys.h"
#include "font/font.naix"
#include "print\str_tool.h"
#include "union_local.h"
#include "union_chat.h"
#include "infowin/infowin.h"
#include "app_menu_common.naix"
#include "field/fieldmap.h"
#include "system/pms_draw.h"


//==============================================================================
//  定数定義
//==============================================================================
enum{
  UNISUB_ACTOR_APPEAL_CHAT,
  UNISUB_ACTOR_APPEAL_BATTLE,
  UNISUB_ACTOR_APPEAL_TRADE,
  UNISUB_ACTOR_APPEAL_PICTURE,
  UNISUB_ACTOR_APPEAL_GURUGURU,
  UNISUB_ACTOR_APPEAL_SCROLL,
  
  UNISUB_ACTOR_MAX,
  
  UNISUB_DECOME_NUM = 6,
};

///アクターソフトプライオリティ
enum{
  SOFTPRI_APPEAL_ICON = 10,
};

///アクターBGプライオリティ
enum{
  BGPRI_APPEAL_ICON = 0,
};

///サブ画面BGフレーム定義
enum{
  UNION_FRAME_S_NULL = GFL_BG_FRAME0_S,           ///<未使用面
  UNION_FRAME_S_MESSAGE = GFL_BG_FRAME1_S,        ///<文字面
  UNION_FRAME_S_PLATE = GFL_BG_FRAME2_S,          ///<プレート面
  UNION_FRAME_S_BACKGROUND = GFL_BG_FRAME3_S,     ///<背景面
};

///メニューバーへのスクリーンオフセット(2byte * 32char * )
#define MENUBAR_SCRN_START      (2*32*(192/8-3))
///メニューバーへのスクリーンサイズ
#define MENUBAR_SCRN_SIZE       (3*32*2)

///チャットログ用のBMPWINサイズ
enum{
  UNION_BMPWIN_START_X = 8,     //BMPWIN X位置(dot単位)
  UNION_BMPWIN_START_Y = 0*8,   //BMPWIN Y位置(dot単位)
  UNION_BMPWIN_SIZE_X = 29*8,   //BMPWIN Xサイズ(dot単位)
  UNION_BMPWIN_SIZE_Y = 7*8,    //BMPWIN Yサイズ(dot単位)
};

///プレートのサイズ
enum{
  UNION_PLATE_START_X = 0,      ///<プレートのスクリーン開始位置X(キャラ単位)
  UNION_PLATE_START_Y = 0,      ///<プレートのスクリーン開始位置Y(キャラ単位)
  UNION_PLATE_SIZE_X = 32,      ///<プレートのスクリーンサイズX(キャラ単位)
  UNION_PLATE_SIZE_Y = 7,       ///<プレートのスクリーンサイズY(キャラ単位)
  UNION_PLATE_SCREEN_SIZE = UNION_PLATE_SIZE_X * UNION_PLATE_SIZE_Y * 2,  ///プレート一人分のスクリーンサイズ
};

///プレートのタッチ範囲
enum{
  PLATE_LEFT = 0,
  PLATE_RIGHT = 0x1d*8,
  PLATE_TOP = 0*8,
  PLATE_BOTTOM = PLATE_TOP + UNION_PLATE_SIZE_Y*8,
  
  PLATE_HEIGHT = PLATE_BOTTOM - PLATE_TOP,
};

///タッチ後、プレートの色が変わっている時間
#define UNION_PLATE_TOUCH_LIFE      (90)

typedef enum{
  PLATE_COLOR_NORMAL,    ///<ノーマル色(タッチしていない状態)
  PLATE_COLOR_TOUCH,     ///<タッチ色
}PLATE_COLOR;

///サブBG画面のパレット構成
enum{
  UNION_SUBBG_PAL_MALE,             ///<男プレート
  UNION_SUBBG_PAL_MALE_TOUCH,       ///<男プレート(タッチ中)
  UNION_SUBBG_PAL_FEMALE,           ///<女プレート
  UNION_SUBBG_PAL_FEMALE_TOUCH,     ///<女プレート(タッチ中)
  UNION_SUBBG_PAL_BACKGROUND,       ///<背景
  
  UNION_SUBBG_PAL_MENU_BAR = 0xd,
//  UNION_SUBBG_PAL_INFOWIN = 0xe,
  UNION_SUBBG_PAL_FONT = 0xf,       ///<フォント
};

///OBJのパレット構成
enum{
  UNION_SUBOBJ_PAL_COMMON = 0,
  
  UNION_SUBOBJ_PAL_PMSWORD_START = 9,
  UNION_SUBOBJ_PAL_PMSWORD_END = 13,
};

///一度に画面に表示できるログ件数
#define UNION_CHAT_VIEW_LOG_NUM   (3)

///wb_union_icon.nceのアニメーションシーケンス
enum{
  UNION_ACT_ANMSEQ_CHAT,
  UNION_ACT_ANMSEQ_BATTLE,
  UNION_ACT_ANMSEQ_TRADE,
  UNION_ACT_ANMSEQ_PICTURE,
  UNION_ACT_ANMSEQ_GURUGURU,
  UNION_ACT_ANMSEQ_SCROLL,
};

///スクロールバーの座標
enum{
  ACT_SCROLL_BAR_X = 256-12,
  ACT_SCROLL_BAR_Y_TOP = 4*8-4,
  ACT_SCROLL_BAR_Y_BOTTOM = 192-5*8+4,
  ACT_SCROLL_BAR_Y_OFFSET = ACT_SCROLL_BAR_Y_BOTTOM - ACT_SCROLL_BAR_Y_TOP,
  ACT_SCROLL_BAR_WIDTH = 22,
  ACT_SCROLL_BAR_HEIGHT = 22,

  ///スクロールバーエリアの範囲
  SCROLL_AREA_LEFT = ACT_SCROLL_BAR_X - ACT_SCROLL_BAR_WIDTH/2,
  SCROLL_AREA_RIGHT = ACT_SCROLL_BAR_X + ACT_SCROLL_BAR_WIDTH/2,
  SCROLL_AREA_TOP = ACT_SCROLL_BAR_Y_TOP,
  SCROLL_AREA_BOTTOM = ACT_SCROLL_BAR_Y_BOTTOM,
};

///メニューバーのCGXサイズ
#define MENUBAR_CGX_SIZE    (4*0x20)

//==============================================================================
//  構造体定義
//==============================================================================
///プレートタッチ管理ワーク
typedef struct{
  u16 life;      ///<プレートのタッチ後の色変更時間
  u8 view_no;
  u8 padding;
}PLATE_TOUCH_WORK;

///ユニオン下画面制御ワーク
typedef struct _UNION_SUBDISP{
  GAMESYS_WORK *gsys;
  
  u32 alloc_menubar_pos;
  
  u32 index_cgr;
  u32 index_pltt;
  u32 index_cell;

	GFL_CLUNIT *clunit;
  GFL_CLWK *act[UNISUB_ACTOR_MAX];
  
  u8 appeal_no;    ///<選択中のアピール番号
  u8 scrollbar_touch;   ///<TRUE:スクロールバータッチ中
  u8 padding[2];
  
  PLATE_TOUCH_WORK plate_touch[UNION_CHAT_VIEW_LOG_NUM];
  
  PRINT_QUE *printQue;
  GFL_FONT *font_handle;
  GFL_BMPWIN *bmpwin_chat[UNION_CHAT_VIEW_LOG_NUM];
  PMS_DRAW_WORK *pmsdraw;   ///<簡易会話表示システム
}UNION_SUBDISP;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _UniSub_SystemSetup(UNION_SUBDISP_PTR unisub);
static void _UniSub_SystemExit(UNION_SUBDISP_PTR unisub);
static void _UniSub_BGLoad(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle);
static void _UniSub_BGUnload(UNION_SUBDISP_PTR unisub);
static void _UniSub_ActorResouceLoad(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle);
static void _UniSub_ActorResourceUnload(UNION_SUBDISP_PTR unisub);
static void _UniSub_ActorCreate(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle);
static void _UniSub_ActorDelete(UNION_SUBDISP_PTR unisub);
static void _UniSub_TouchUpdate(UNION_SUBDISP_PTR unisub);
static void _UniSub_IconPalChange(UNION_SUBDISP_PTR unisub, int act_index);
static void _UniSub_MenuBarLoad(UNION_SUBDISP_PTR unisub);
static void _UniSub_MenuBarFree(UNION_SUBDISP_PTR unisub);
static void _UniSub_BmpWinCreate(UNION_SUBDISP_PTR unisub);
static void _UniSub_BmpWinDelete(UNION_SUBDISP_PTR unisub);
static void _UniSub_TouchUpdate(UNION_SUBDISP_PTR unisub);
static void _UniSub_IconPalChange(UNION_SUBDISP_PTR unisub, int act_index);
static void _UniSub_PrintChatUpdate(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log);
static void _UniSub_Chat_DispWrite(UNION_SUBDISP_PTR unisub, UNION_CHAT_DATA *chat, u8 write_pos);
static void _UniSub_Chat_DispAllWrite(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log);
static void _UniSub_Chat_DispCopy(UNION_SUBDISP_PTR unisub, u8 src_pos, u8 dest_pos);
static void _UniSub_Chat_DispScroll(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log, s32 offset);
static BOOL _UniSub_ScrollBar_TouchCheck(UNION_SUBDISP_PTR unisub);
static void _UniSub_ScrollBar_Update(UNION_SYSTEM_PTR unisys, UNION_SUBDISP_PTR unisub);
static void _UniSub_ScrollBar_PosUpdate(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log);
static void _UniSub_ScrollBar_ViewPosUpdate(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log);
static u32 _UniSub_ScrollBar_GetPageMax(UNION_CHAT_LOG *log);
static u32 _UniSub_ScrollBar_GetPageY(UNION_CHAT_LOG *log, u32 page_max);
static BOOL _UniSub_ScrollArea_TouchCheck(UNION_SUBDISP_PTR unisub);
static BOOL _UniSub_ChatPlate_TouchCheck(UNION_SYSTEM_PTR unisys, UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log);
static PLATE_TOUCH_WORK * _UniSub_GetPlateTouchPtr(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log, int view_no);
static BOOL _UniSub_EntryPlateTouchWork(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log, int view_no);

static void _UniSub_ChatPlate_Update(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log);
static BOOL _UniSub_ChatPlate_ChangeColor(UNION_SUBDISP_PTR unisub, int plate_no, PLATE_COLOR color);


//==============================================================================
//  データ
//==============================================================================
///ボタンと対応したplay_category
ALIGN4 static const u8 AppealNo_CategoryTbl[] = {
  UNION_APPEAL_NULL,        ///<チャット
  UNION_APPEAL_BATTLE,      ///<戦闘
  UNION_APPEAL_TRADE,       ///<交換
  UNION_APPEAL_PICTURE,     ///<お絵かき
  UNION_APPEAL_GURUGURU,    ///<ぐるぐる交換
};


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ユニオン下画面：初期設定
 *
 * @param   gsys		
 *
 * @retval  UNION_SUBDISP_PTR		
 */
//==================================================================
UNION_SUBDISP_PTR UNION_SUBDISP_Init(GAMESYS_WORK *gsys)
{
  UNION_SUBDISP_PTR unisub;
  ARCHANDLE *handle;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);

  unisub = GFL_HEAP_AllocClearMemory(HEAPID_FIELDMAP, sizeof(UNION_SUBDISP));
  unisub->gsys = gsys;
  
  handle = GFL_ARC_OpenDataHandle(ARCID_UNION, HEAPID_FIELDMAP);
  
  _UniSub_SystemSetup(unisub);
  _UniSub_BGLoad(unisub, handle);
  _UniSub_BmpWinCreate(unisub);
  _UniSub_ActorResouceLoad(unisub, handle);
  _UniSub_ActorCreate(unisub, handle);
  _UniSub_MenuBarLoad(unisub);
  GFL_ARC_CloseDataHandle(handle);

  unisub->pmsdraw = PMS_DRAW_Init(unisub->clunit, CLSYS_DRAW_SUB, 
    unisub->printQue, unisub->font_handle, UNION_SUBOBJ_PAL_PMSWORD_START, 
    UNION_CHAT_VIEW_LOG_NUM, HEAPID_FIELDMAP);
  
  {//メニューから画面復帰の場合用に全体描画
    UNION_SYSTEM_PTR unisys = GameCommSys_GetAppWork(game_comm);
    if(unisys != NULL){
      _UniSub_Chat_DispAllWrite(unisub, &unisys->chat_log);
    }
  }
  
  //OBJWINDOW(通信アイコン) の中だけBlendで輝度が落ちないようにする
  GFL_NET_WirelessIconOBJWinON();
  G2S_SetWndOBJInsidePlane(GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 |
      GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, FALSE);
  G2S_SetWndOutsidePlane(GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 |
      GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, TRUE);
	GXS_SetVisibleWnd(GX_WNDMASK_OW);
  
  return unisub;
}

//==================================================================
/**
 * ユニオン下画面：破棄設定
 *
 * @param   unisub		
 */
//==================================================================
void UNION_SUBDISP_Exit(UNION_SUBDISP_PTR unisub)
{
  PMS_DRAW_Exit(unisub->pmsdraw);
  _UniSub_MenuBarFree(unisub);
  _UniSub_ActorDelete(unisub);
  _UniSub_ActorResourceUnload(unisub);
  _UniSub_BmpWinDelete(unisub);
  _UniSub_BGUnload(unisub);
  _UniSub_SystemExit(unisub);

  GFL_HEAP_FreeMemory(unisub);
  G2S_BlendNone();
  
  //OBJWINDOW(通信アイコン) の中だけBlendで輝度が落ちないようにする
  GFL_NET_WirelessIconOBJWinOFF();
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
}

//==================================================================
/**
 * ユニオン下画面：更新
 *
 * @param   unisub		
 * @param   bActive   TRUE:下画面アクティブ状態
 *                    FALSE:非アクティブ(他のイベント中：操作を受け付けてはいけない)
 */
//==================================================================
void UNION_SUBDISP_Update(UNION_SUBDISP_PTR unisub, BOOL bActive)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(unisub->gsys);
  UNION_SYSTEM_PTR unisys = GameCommSys_GetAppWork(game_comm);
  int i;
  
  GFL_NET_WirelessIconOBJWinON();
	if(bActive == TRUE){
  	G2S_BlendNone();
  }
  else{
    G2S_SetBlendBrightness(GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 |
      GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ |
      GX_BLEND_PLANEMASK_BD, -FIELD_NONE_ACTIVE_EVY);
  }

	PRINTSYS_QUE_Main(unisub->printQue);
  PMS_DRAW_Main(unisub->pmsdraw);
  
  //チャット処理
  if(unisys != NULL){
    UNION_MY_SITUATION *situ = &unisys->my_situation;
    if(situ->chat_upload == TRUE){
      UnionChat_AddChat(unisys, NULL, &situ->chat_pmsdata);
      situ->chat_upload = FALSE;
    }
  #if PM_DEBUG
    else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_L){
//      PMSDAT_SetDebugRandom(&situ->chat_pmsdata);
      PMSDAT_SetDebugRandomDeco( &situ->chat_pmsdata, HEAPID_FIELDMAP );
      UnionChat_AddChat(unisys, NULL, &situ->chat_pmsdata);
    }
  #endif
    _UniSub_PrintChatUpdate(unisub, &unisys->chat_log);
  }
  
  if(bActive == TRUE){
    _UniSub_TouchUpdate(unisub);

    unisub->scrollbar_touch = _UniSub_ScrollBar_TouchCheck(unisub);
    if(unisub->scrollbar_touch == FALSE){
      unisub->scrollbar_touch = _UniSub_ScrollArea_TouchCheck(unisub);
    }
  }
  
  if(unisys != NULL){
    _UniSub_ScrollBar_Update(unisys, unisub);
    if(bActive == TRUE){
      _UniSub_ChatPlate_TouchCheck(unisys, unisub, &unisys->chat_log);
    }
    _UniSub_ChatPlate_Update(unisub, &unisys->chat_log);
  }
  
  if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_UNION){
    if(unisys != NULL){
      UnionMain_SetAppealNo(unisys, unisub->appeal_no);
    }
  }
}

//==================================================================
/**
 * ユニオン下画面：描画
 *
 * @param   unisub		
 * @param   bActive   TRUE:下画面アクティブ状態
 *                    FALSE:非アクティブ(他のイベント中：操作を受け付けてはいけない)
 */
//==================================================================
void UNION_SUBDISP_Draw(UNION_SUBDISP_PTR unisub, BOOL bActive)
{
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_SystemSetup(UNION_SUBDISP_PTR unisub)
{
  unisub->clunit = GFL_CLACT_UNIT_Create(UNISUB_ACTOR_MAX + UNISUB_DECOME_NUM, 5, HEAPID_FIELDMAP);
  unisub->printQue = PRINTSYS_QUE_Create(HEAPID_FIELDMAP);
  unisub->font_handle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
    GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_FIELDMAP );
}

//--------------------------------------------------------------
/**
 * システム関連の破棄
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_SystemExit(UNION_SUBDISP_PTR unisub)
{
  GFL_FONT_Delete(unisub->font_handle);
  PRINTSYS_QUE_Delete(unisub->printQue);
  GFL_CLACT_UNIT_Delete(unisub->clunit);
}

//--------------------------------------------------------------
/**
 * BGロード
 *
 * @param   unisub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _UniSub_BGLoad(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle)
{
	static const GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
  	{//UNION_FRAME_S_MESSAGE
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x6800,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
  	{//UNION_FRAME_S_PLATE
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x6800,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
  	{//UNION_FRAME_S_BACKGROUND
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x6800,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
	};
	GFL_BG_SetBGControl( UNION_FRAME_S_MESSAGE, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( UNION_FRAME_S_PLATE, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( UNION_FRAME_S_BACKGROUND, &TextBgCntDat[2], GFL_BG_MODE_TEXT );

	//BG VRAMクリア
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);

	//レジスタOFF
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
	G2S_BlendNone();
	
  //キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    handle, NARC_unionroom_wb_union_plate_NCGR, UNION_FRAME_S_BACKGROUND, 0, 
    0, FALSE, HEAPID_FIELDMAP);
  
  //スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_unionroom_wb_unionbg_NSCR, UNION_FRAME_S_BACKGROUND, 0, 
    0x800, FALSE, HEAPID_FIELDMAP);
  //プレートのスクリーン(VRAMには展開せずスクリーンバッファにのみ1つ分のプレートだけを転送
  {
    u16 *scrn_buf, *load_buf;
    NNSG2dScreenData *scrnData;
    
    scrn_buf = GFL_BG_GetScreenBufferAdrs(UNION_FRAME_S_PLATE);
    load_buf = GFL_ARCHDL_UTIL_LoadScreen(
      handle, NARC_unionroom_wb_unionbg_plate_NSCR, FALSE, &scrnData, HEAPID_FIELDMAP);
    GFL_STD_MemCopy(scrnData->rawData, scrn_buf, 
      UNION_PLATE_SIZE_X * UNION_PLATE_SIZE_Y * 2);
    GFL_HEAP_FreeMemory(load_buf);
  }
  
  //パレット転送
  GFL_ARCHDL_UTIL_TransVramPalette(handle, NARC_unionroom_wb_unionbg_NCLR, PALTYPE_SUB_BG, 0, 
    0x20*5, HEAPID_FIELDMAP);
  //フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 
    UNION_SUBBG_PAL_FONT * 0x20, 0x20, HEAPID_FIELDMAP);

	GFL_BG_SetVisible( UNION_FRAME_S_NULL, VISIBLE_OFF );
	GFL_BG_SetVisible( UNION_FRAME_S_BACKGROUND, VISIBLE_ON );
	GFL_BG_SetVisible( UNION_FRAME_S_PLATE, VISIBLE_ON );
	GFL_BG_SetVisible( UNION_FRAME_S_MESSAGE, VISIBLE_ON );
}

//--------------------------------------------------------------
/**
 * BGアンロード
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_BGUnload(UNION_SUBDISP_PTR unisub)
{
	GFL_BG_SetVisible( UNION_FRAME_S_MESSAGE, VISIBLE_OFF );
	GFL_BG_SetVisible( UNION_FRAME_S_PLATE, VISIBLE_OFF );
	GFL_BG_SetVisible( UNION_FRAME_S_BACKGROUND, VISIBLE_OFF );
	GFL_BG_FreeBGControl(UNION_FRAME_S_MESSAGE);
	GFL_BG_FreeBGControl(UNION_FRAME_S_PLATE);
	GFL_BG_FreeBGControl(UNION_FRAME_S_BACKGROUND);
}

//--------------------------------------------------------------
/**
 * メニューバーのロード
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_MenuBarLoad(UNION_SUBDISP_PTR unisub)
{
  ARCHANDLE *app_handle;
  u32 menu_charno;

	app_handle = GFL_ARC_OpenDataHandle(ARCID_APP_MENU_COMMON, HEAPID_FIELDMAP);

  menu_charno = GFL_BG_AllocCharacterArea(
    UNION_FRAME_S_MESSAGE, MENUBAR_CGX_SIZE, GFL_BG_CHRAREA_GET_B );
  unisub->alloc_menubar_pos = menu_charno;

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    app_handle, NARC_app_menu_common_menu_bar_NCGR, UNION_FRAME_S_MESSAGE, menu_charno, 
    MENUBAR_CGX_SIZE, FALSE, HEAPID_FIELDMAP);

  //メニューバーのスクリーン
  {
    u16 *scrn_buf, *load_buf, *read_buf;
    NNSG2dScreenData *scrnData;
    int i;
    
    scrn_buf = GFL_BG_GetScreenBufferAdrs(UNION_FRAME_S_MESSAGE);
    load_buf = GFL_ARCHDL_UTIL_LoadScreen(
      app_handle, NARC_app_menu_common_menu_bar_NSCR, FALSE, &scrnData, HEAPID_FIELDMAP);
    read_buf = (u16*)scrnData->rawData;
    GFL_STD_MemCopy(&read_buf[MENUBAR_SCRN_START/2], 
      &scrn_buf[MENUBAR_SCRN_START/2], MENUBAR_SCRN_SIZE);
    GFL_HEAP_FreeMemory(load_buf);
    for(i = 0; i < MENUBAR_SCRN_SIZE/2; i++){
      scrn_buf[MENUBAR_SCRN_START/2 + i] = (scrn_buf[MENUBAR_SCRN_START/2 + i] & 0x0fff) + menu_charno + (UNION_SUBBG_PAL_MENU_BAR << 12);
    }
  }

  GFL_ARCHDL_UTIL_TransVramPalette(app_handle, NARC_app_menu_common_menu_bar_NCLR, 
    PALTYPE_SUB_BG, UNION_SUBBG_PAL_MENU_BAR*0x20, 0x20*1, HEAPID_FIELDMAP);

	GFL_ARC_CloseDataHandle(app_handle);
}

//--------------------------------------------------------------
/**
 * メニューバーの解放
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_MenuBarFree(UNION_SUBDISP_PTR unisub)
{
  GFL_BG_FreeCharacterArea(UNION_FRAME_S_MESSAGE, unisub->alloc_menubar_pos, MENUBAR_CGX_SIZE);
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_BmpWinCreate(UNION_SUBDISP_PTR unisub)
{
  int i;
  
  for(i = 0; i < UNION_CHAT_VIEW_LOG_NUM; i++){
    unisub->bmpwin_chat[i] = GFL_BMPWIN_Create(UNION_FRAME_S_MESSAGE, 
      UNION_BMPWIN_START_X / 8, (UNION_BMPWIN_START_Y + UNION_BMPWIN_SIZE_Y * i) / 8, 
      UNION_BMPWIN_SIZE_X / 8, UNION_BMPWIN_SIZE_Y / 8, 
      UNION_SUBBG_PAL_FONT, GFL_BMP_CHRAREA_GET_B);
  }

}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_BmpWinDelete(UNION_SUBDISP_PTR unisub)
{
  int i;
  
  for(i = 0; i < UNION_CHAT_VIEW_LOG_NUM; i++){
    GFL_BMPWIN_Delete(unisub->bmpwin_chat[i]);
  }
}

//--------------------------------------------------------------
/**
 * アクターで使用するリソースのロード
 *
 * @param   unisub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _UniSub_ActorResouceLoad(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle)
{
  //OBJ共通パレット
  unisub->index_pltt = GFL_CLGRP_PLTT_RegisterEx(
    handle, NARC_unionroom_wb_unionobj_NCLR, CLSYS_DRAW_SUB, 0, 0, 3, HEAPID_FIELDMAP );

  //アピールアイコン
  unisub->index_cgr = GFL_CLGRP_CGR_Register(
    handle, NARC_unionroom_wb_union_icon_NCGR, FALSE, CLSYS_DRAW_SUB, HEAPID_FIELDMAP);
  unisub->index_cell = GFL_CLGRP_CELLANIM_Register(
    handle, NARC_unionroom_wb_union_icon_NCER, 
    NARC_unionroom_wb_union_icon_NANR, HEAPID_FIELDMAP);
}

//--------------------------------------------------------------
/**
 * アクターで使用するアクターのアンロード
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_ActorResourceUnload(UNION_SUBDISP_PTR unisub)
{
  GFL_CLGRP_PLTT_Release(unisub->index_pltt);
  GFL_CLGRP_CGR_Release(unisub->index_cgr);
  GFL_CLGRP_CELLANIM_Release(unisub->index_cell);
}

//--------------------------------------------------------------
/**
 * アクター作成
 *
 * @param   unisub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _UniSub_ActorCreate(UNION_SUBDISP_PTR unisub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 192 - 12,                   //X, Y座標
  	0,
  	SOFTPRI_APPEAL_ICON,    //ソフトプライオリティ
  	BGPRI_APPEAL_ICON,      //BGプライオリティ
  };
  
  //アピールアイコン
  for(i = UNISUB_ACTOR_APPEAL_CHAT; i <= UNISUB_ACTOR_APPEAL_GURUGURU; i++){
    head.pos_x = 32 + i * 48;
    head.anmseq = i;
    unisub->act[i] = GFL_CLACT_WK_Create(unisub->clunit, 
      unisub->index_cgr, unisub->index_pltt, unisub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
  }
  
  //スクロールバー
  head.anmseq = UNION_ACT_ANMSEQ_SCROLL;
  head.pos_x = ACT_SCROLL_BAR_X;
  head.pos_y = ACT_SCROLL_BAR_Y_BOTTOM;
  unisub->act[UNISUB_ACTOR_APPEAL_SCROLL] = GFL_CLACT_WK_Create(unisub->clunit, 
      unisub->index_cgr, unisub->index_pltt, unisub->index_cell, 
      &head, CLSYS_DEFREND_SUB, HEAPID_FIELDMAP);
}

//--------------------------------------------------------------
/**
 * アクター削除
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_ActorDelete(UNION_SUBDISP_PTR unisub)
{
  int i;
  
  for(i = 0; i < UNISUB_ACTOR_MAX; i++){
    GFL_CLACT_WK_Remove(unisub->act[i]);
  }
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * タッチ判定
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_TouchUpdate(UNION_SUBDISP_PTR unisub)
{
  u32 x, y;
  int i;
  GFL_CLACTPOS clpos;
  struct{
    s16 top;
    s16 bottom;
    s16 left;
    s16 right;
  }crect;
  
  if(GFL_UI_TP_GetPointTrg(&x, &y) == FALSE){
    return;
  }
  
  for(i = UNISUB_ACTOR_APPEAL_CHAT; i <= UNISUB_ACTOR_APPEAL_GURUGURU; i++){
    GFL_CLACT_WK_GetPos(unisub->act[i], &clpos, CLSYS_DEFREND_SUB);
    crect.top = clpos.y - 10;
    crect.bottom = clpos.y + 10;
    crect.left = clpos.x - 10;
    crect.right = clpos.x + 10;
    if(crect.left < x && crect.right > x && crect.top < y && crect.bottom > y){
      if(i == UNISUB_ACTOR_APPEAL_CHAT){
        FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(unisub->gsys);
        FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
        
        FIELD_SUBSCREEN_SetAction(subscreen, FIELD_SUBSCREEN_ACTION_UNION_CHAT);
      }
      else{
        _UniSub_IconPalChange(unisub, i);
      }
      return;
    }
  }
}

//--------------------------------------------------------------
/**
 * 対象のアイコンの色を変える
 *
 * @param   unisub		
 * @param   act_index		
 */
//--------------------------------------------------------------
static void _UniSub_IconPalChange(UNION_SUBDISP_PTR unisub, int act_index)
{
  int i;
  UNION_APPEAL select_appeal;
  
  select_appeal = AppealNo_CategoryTbl[act_index - UNISUB_ACTOR_APPEAL_CHAT];
  if(select_appeal == unisub->appeal_no){
    unisub->appeal_no = UNION_APPEAL_NULL;
  }
  else{
    unisub->appeal_no = select_appeal;
  }
  
  for(i = UNISUB_ACTOR_APPEAL_CHAT; i <= UNISUB_ACTOR_APPEAL_GURUGURU; i++){
    if(i == act_index && unisub->appeal_no != UNION_APPEAL_NULL){
      GFL_CLACT_WK_SetPlttOffs(unisub->act[i], 1, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    }
    else{
      GFL_CLACT_WK_SetPlttOffs(unisub->act[i], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    }
  }
}

//==================================================================
/**
 * チャットログ画面：更新処理
 *
 * @param   unisub		
 * @param   log		    チャットログ管理データへのポインタ
 */
//==================================================================
static void _UniSub_PrintChatUpdate(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log)
{
  s32 offset;
  int i;
  
  if(log->old_chat_view_no == log->chat_view_no){
    return;
  }
  
  //PrintQueにまだ残っているのがあるならば更新はしない
  if(PMS_DRAW_IsPrintEnd(unisub->pmsdraw) == FALSE){
    return;
  }
  
  if(log->chat_log_count < UNION_CHAT_VIEW_LOG_NUM){ //まだスクロールが発生しない状態
    UNION_CHAT_DATA *chat;
    for(i = log->old_chat_view_no; i <= log->chat_view_no; i++){
      if(i > -1){
        chat = UnionChat_GetReadBuffer(log, i);
        GF_ASSERT(chat != NULL);
        _UniSub_Chat_DispWrite(unisub, chat, i);
      }
    }
  }
  else{
    offset = log->chat_view_no - log->old_chat_view_no;
    if(MATH_ABS(offset) >= UNION_CHAT_VIEW_LOG_NUM){
      _UniSub_Chat_DispAllWrite(unisub, log); //全体再描画
    }
    else{
      _UniSub_Chat_DispScroll(unisub, log, offset);
    }
  }
  
  log->old_chat_view_no = log->chat_view_no;
}

//--------------------------------------------------------------
/**
 * チャット描画
 *
 * @param   unisub		  
 * @param   chat		    チャットデータへのポインタ
 * @param   write_pos		出力位置
 */
//--------------------------------------------------------------
void _UniSub_Chat_DispWrite(UNION_SUBDISP_PTR unisub, UNION_CHAT_DATA *chat, u8 write_pos)
{
  u16 *src_plate_screen, *dest_plate_screen;
  int palno;
  
  src_plate_screen = GFL_BG_GetScreenBufferAdrs(UNION_FRAME_S_PLATE);
  dest_plate_screen = src_plate_screen;
  dest_plate_screen += UNION_PLATE_SCREEN_SIZE/2 * write_pos;
  
  palno = (chat->sex == PM_MALE) ? UNION_SUBBG_PAL_MALE : UNION_SUBBG_PAL_FEMALE;
  
  //プレートのスクリーン描画(既にVRAMに展開されている先頭のプレートスクリーンをコピー)
  GFL_STD_MemCopy(src_plate_screen, dest_plate_screen, UNION_PLATE_SCREEN_SIZE);
  GFL_BG_ChangeScreenPalette(UNION_FRAME_S_PLATE, 
    UNION_PLATE_START_X, UNION_PLATE_START_Y + UNION_PLATE_SIZE_Y * write_pos, 
    UNION_PLATE_SIZE_X, UNION_PLATE_SIZE_Y, palno);
  GFL_BG_LoadScreenV_Req(UNION_FRAME_S_PLATE);
  
  //文字面描画
  {
    STRBUF *buf_name;
    GFL_POINT point = {0, 16+4};
    
    if(PMS_DRAW_IsPrinting(unisub->pmsdraw, write_pos) == TRUE){
      PMS_DRAW_Clear(unisub->pmsdraw, write_pos, TRUE);
    }
    
    //名前
    buf_name = GFL_STR_CreateBuffer(PERSON_NAME_SIZE + EOM_SIZE, HEAPID_FIELDMAP);
    GFL_STR_SetStringCode(buf_name, chat->name);
    PRINTSYS_PrintQue( unisub->printQue, GFL_BMPWIN_GetBmp(unisub->bmpwin_chat[write_pos]), 
      0, 0, buf_name, unisub->font_handle);
    GFL_STR_DeleteBuffer(buf_name);
    
    //友達手帳の名前  ※check　まだ未作成 2009.08.24(月)
    
    //簡易会話
    PMS_DRAW_PrintOffset(unisub->pmsdraw, unisub->bmpwin_chat[write_pos], 
      &chat->pmsdata, write_pos, &point);
  }
}

//--------------------------------------------------------------
/**
 * 一画面中の全てのチャットを描画
 *
 * @param   unisub		
 * @param   log		    
 */
//--------------------------------------------------------------
void _UniSub_Chat_DispAllWrite(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log)
{
  int start, end, write_pos, i;
  UNION_CHAT_DATA *chat;
  
  start = log->chat_view_no;
  end = start - UNION_CHAT_VIEW_LOG_NUM;
  if(end < 0){
    end = -1;
  }
  write_pos = start - end - 1;

  for(i = start; i > end; i--){
    chat = UnionChat_GetReadBuffer(log, i);
    GF_ASSERT(chat != NULL);
    GF_ASSERT(write_pos > -1);
    _UniSub_Chat_DispWrite(unisub, chat, write_pos);
    write_pos--;
  }
}

//--------------------------------------------------------------
/**
 * 描画されているチャットBMPを別のBMPにコピー
 *
 * @param   unisub		
 * @param   src_pos		  コピー元描画位置
 * @param   dest_pos		コピー先描画位置
 */
//--------------------------------------------------------------
void _UniSub_Chat_DispCopy(UNION_SUBDISP_PTR unisub, u8 src_pos, u8 dest_pos)
{
  u16 *src_plate_screen, *dest_plate_screen;

  //プレートのコピー
  src_plate_screen = GFL_BG_GetScreenBufferAdrs(UNION_FRAME_S_PLATE);
  dest_plate_screen = src_plate_screen;
  dest_plate_screen += UNION_PLATE_SCREEN_SIZE/2 * dest_pos;
  src_plate_screen += UNION_PLATE_SCREEN_SIZE/2 * src_pos;
  GFL_STD_MemCopy(src_plate_screen, dest_plate_screen, UNION_PLATE_SCREEN_SIZE);
  GFL_BG_LoadScreenV_Req(UNION_FRAME_S_PLATE);
  
  //BMPのコピー
  PMS_DRAW_Copy(unisub->pmsdraw, src_pos, dest_pos);
}

//--------------------------------------------------------------
/**
 * チャットをスクロール
 *
 * @param   unisub		
 * @param   log		
 * @param   offset		
 *
 * コピーで済むものはコピーで済ませるのが(_UniSub_Chat_DispAllWriteとの違い)
 */
//--------------------------------------------------------------
void _UniSub_Chat_DispScroll(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log, s32 offset)
{
  int src_pos, dest_pos;
  int i, write_pos;
  UNION_CHAT_DATA *chat;
  
  if(offset > 0){ //今のログを上に移動していく
    dest_pos = 0;
    src_pos = dest_pos + offset;
    while(src_pos < UNION_CHAT_VIEW_LOG_NUM){
      _UniSub_Chat_DispCopy(unisub, src_pos, dest_pos);
      dest_pos++;
      src_pos++;
    }
    
    for(write_pos = UNION_CHAT_VIEW_LOG_NUM - offset; write_pos < UNION_CHAT_VIEW_LOG_NUM; write_pos++)
    {
      chat = UnionChat_GetReadBuffer(log, log->chat_view_no - (UNION_CHAT_VIEW_LOG_NUM-1) + write_pos);
      GF_ASSERT(chat != NULL);
      _UniSub_Chat_DispWrite(unisub, chat, write_pos);
    }
  }
  else{           //ログを下に移動していく
    dest_pos = UNION_CHAT_VIEW_LOG_NUM - 1;
    src_pos = dest_pos + offset;
    while(src_pos > -1){
      _UniSub_Chat_DispCopy(unisub, src_pos, dest_pos);
      dest_pos--;
      src_pos--;
    }

    for(write_pos = (-offset)-1; write_pos > -1; write_pos--)
    {
      chat = UnionChat_GetReadBuffer(log, log->chat_view_no - (UNION_CHAT_VIEW_LOG_NUM - 1) + write_pos);
      GF_ASSERT_MSG(chat != NULL, "view_no=%d, write_pos=%d, offset=%d\n", log->chat_view_no, write_pos, offset);
      _UniSub_Chat_DispWrite(unisub, chat, write_pos);
    }
  }
}

//--------------------------------------------------------------
/**
 * スクロールバータッチ判定
 *
 * @param   unisub		
 *
 * @retval  BOOL		TRUE：タッチしている　FALSE：していない
 */
//--------------------------------------------------------------
static BOOL _UniSub_ScrollBar_TouchCheck(UNION_SUBDISP_PTR unisub)
{
  GFL_CLACTPOS bar;
  u32 tp_x, tp_y;
  
  if(GFL_UI_TP_GetPointCont(&tp_x, &tp_y) == FALSE){
    return FALSE;
  }
  
  GFL_CLACT_WK_GetPos(unisub->act[UNISUB_ACTOR_APPEAL_SCROLL], &bar, CLSYS_DEFREND_SUB);

  if(unisub->scrollbar_touch == FALSE){
    if(GFL_UI_TP_GetTrg() == FALSE){
      return FALSE;
    }
    if(bar.x - ACT_SCROLL_BAR_WIDTH/2 <= tp_x && bar.x + ACT_SCROLL_BAR_WIDTH/2 >= tp_x
        && bar.y - ACT_SCROLL_BAR_HEIGHT/2 <= tp_y && bar.y + ACT_SCROLL_BAR_HEIGHT/2 >= tp_y){
      return TRUE;
    }
  }
  else{
    if(tp_y < ACT_SCROLL_BAR_Y_TOP){
      tp_y = ACT_SCROLL_BAR_Y_TOP;
    }
    else if(tp_y > ACT_SCROLL_BAR_Y_BOTTOM){
      tp_y = ACT_SCROLL_BAR_Y_BOTTOM;
    }
    bar.y = tp_y;
    GFL_CLACT_WK_SetPos(unisub->act[UNISUB_ACTOR_APPEAL_SCROLL], &bar, CLSYS_DEFREND_SUB);
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * スクロールバー更新処理
 *
 * @param   unisys		
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_ScrollBar_Update(UNION_SYSTEM_PTR unisys, UNION_SUBDISP_PTR unisub)
{
  
  if(unisub->scrollbar_touch == TRUE){
    _UniSub_ScrollBar_PosUpdate(unisub, &unisys->chat_log);
  }
  else{
    _UniSub_ScrollBar_ViewPosUpdate(unisub, &unisys->chat_log);
  }
}

//--------------------------------------------------------------
/**
 * スクロールバーの座標位置に従ってチャットログ画面のViewポジションを変更する
 *
 * @param   unisub		
 * @param   log		
 */
//--------------------------------------------------------------
static void _UniSub_ScrollBar_PosUpdate(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log)
{
  s32 page_y, page_max, half_page_y, view_page;
  GFL_CLACTPOS bar;
  s32 check_y, view_no;
  int i;
  
  GFL_CLACT_WK_GetPos(unisub->act[UNISUB_ACTOR_APPEAL_SCROLL], &bar, CLSYS_DEFREND_SUB);
  check_y = (bar.y - ACT_SCROLL_BAR_Y_TOP) << 8;
  
  page_max = _UniSub_ScrollBar_GetPageMax(log);
  page_y = _UniSub_ScrollBar_GetPageY(log, page_max);
  
  view_page = 0;
  half_page_y = page_y >> 1;
  check_y += half_page_y;
  for(i = 0; i < page_max; i++){
//    if(check_y >= page_y * (view_page+1) - half_page_y
//        && check_y <= page_y * (view_page+1) + half_page_y){
    if(check_y <= page_y * (view_page + 1)){
      break;
    }
    view_page++;
  }
  if(view_page >= page_max){
    //小数で端数が出ていると最後までfor文を回しても最後尾だけ判定から漏れる為フォロー
    view_page = page_max;
  }
  
  log->chat_view_no = log->chat_log_count - page_max + view_page;
  if(log->chat_view_no < UNION_CHAT_VIEW_LOG_NUM){
    if(log->chat_log_count < UNION_CHAT_VIEW_LOG_NUM){
      log->chat_view_no = log->chat_log_count;
    }
    else{
      log->chat_view_no = UNION_CHAT_VIEW_LOG_NUM - 1;
    }
  }
  
//  OS_TPrintf("ccc page_y = %d, bar.y = %d, page_y8 = %d, log_count = %d, view_no = %d, view_page = %d, page_max = %d, check_y = %d\n", page_y, bar.y, page_y >> 8, log->chat_log_count, log->chat_view_no, view_page, page_max, check_y);
  if(log->chat_view_no < 0 || log->chat_view_no > log->chat_log_count){
    OS_TPrintf("OVER!!! chat_view_no = %d, log_count = %d, view_page = %d\n", 
      log->chat_view_no, log->chat_log_count, view_page);
    log->chat_view_no = log->chat_log_count;
    GF_ASSERT(0);
  }

#if 0
  bar.x = ACT_SCROLL_BAR_X;
  bar.y = page_y;
  GFL_CLACT_WK_SetPos(unisub->act[UNISUB_ACTOR_APPEAL_SCROLL], &bar, CLSYS_DEFREND_SUB);
#endif
}

//--------------------------------------------------------------
/**
 * チャットログ画面のViewポジションに合わせてスクロールバーの座標を変更する
 *
 * @param   unisub		
 * @param   log		
 */
//--------------------------------------------------------------
static void _UniSub_ScrollBar_ViewPosUpdate(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log)
{
  GFL_CLACTPOS bar;
  int log_offset;
  u32 page_y;
  
  page_y = _UniSub_ScrollBar_GetPageY(log, _UniSub_ScrollBar_GetPageMax(log));

  log_offset = log->chat_log_count - log->chat_view_no;
  bar.y = ACT_SCROLL_BAR_Y_BOTTOM - ((page_y * log_offset) >> 8);
  
  bar.x = ACT_SCROLL_BAR_X;
  GFL_CLACT_WK_SetPos(unisub->act[UNISUB_ACTOR_APPEAL_SCROLL], &bar, CLSYS_DEFREND_SUB);
  
//  OS_TPrintf("aaa page_y = %d, bar.y = %d, log_offset = %d, page_y8 = %d, log_count = %d, view_no = %d\n", page_y, bar.y, log_offset, page_y >> 8, log->chat_log_count, log->chat_view_no);
}

//--------------------------------------------------------------
/**
 * チャットログ画面の表示できるページ数取得(最大値はUNION_CHAT_LOG_MAXまで)
 *
 * @param   log		
 *
 * @retval  u32		表示可能ページ数
 */
//--------------------------------------------------------------
static u32 _UniSub_ScrollBar_GetPageMax(UNION_CHAT_LOG *log)
{
  s32 page_max;
  
  if(log->chat_log_count < UNION_CHAT_LOG_MAX){
    page_max = log->chat_log_count;
  }
  else{
    page_max = UNION_CHAT_LOG_MAX - 1;
  }
  
  page_max -= UNION_CHAT_VIEW_LOG_NUM - 1;
  if(page_max < 0){
    page_max = 0;
  }
  return page_max;
}

//--------------------------------------------------------------
/**
 * チャットログの1ページが持つスクロールバーのドット数(Y)を取得する
 *
 * @param   log		
 *
 * @retval  u32		ドット数(固定小数：下位8ビット小数)
 */
//--------------------------------------------------------------
static u32 _UniSub_ScrollBar_GetPageY(UNION_CHAT_LOG *log, u32 page_max)
{
  u32 page_y;

  page_y = ACT_SCROLL_BAR_Y_OFFSET << 8;
  page_y /= page_max;
  return page_y;
}

//--------------------------------------------------------------
/**
 * スクロールエリアタッチ判定
 *
 * @param   unisub		
 *
 * @retval  BOOL		TRUE：タッチした　FALSE：していない
 */
//--------------------------------------------------------------
static BOOL _UniSub_ScrollArea_TouchCheck(UNION_SUBDISP_PTR unisub)
{
  GFL_CLACTPOS bar;
  u32 tp_x, tp_y;
  
  if(GFL_UI_TP_GetPointTrg(&tp_x, &tp_y) == FALSE){
    return FALSE;
  }
  if(tp_x >= SCROLL_AREA_LEFT && tp_x <= SCROLL_AREA_RIGHT
      && tp_y >= SCROLL_AREA_TOP && tp_y <= SCROLL_AREA_BOTTOM){
    bar.x = ACT_SCROLL_BAR_X;
    bar.y = tp_y;
    GFL_CLACT_WK_SetPos(unisub->act[UNISUB_ACTOR_APPEAL_SCROLL], &bar, CLSYS_DEFREND_SUB);
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * プレートのタッチチェック
 *
 * @param   unisub		
 *
 * @retval  BOOL		TRUE:タッチ発生
 */
//--------------------------------------------------------------
static BOOL _UniSub_ChatPlate_TouchCheck(UNION_SYSTEM_PTR unisys, UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log)
{
  u32 tp_x, tp_y;
  int y;
  
  if(GFL_UI_TP_GetPointTrg(&tp_x, &tp_y) == FALSE){
    return FALSE;
  }
  
  if(tp_x >= PLATE_LEFT && tp_x <= PLATE_RIGHT){
    for(y = 0; y < UNION_CHAT_VIEW_LOG_NUM; y++){
      if(tp_y >= (PLATE_TOP + PLATE_HEIGHT * y) && tp_y <= (PLATE_BOTTOM + PLATE_HEIGHT * y)){
        break;
      }
    }
  }
  
  if(y >= UNION_CHAT_VIEW_LOG_NUM){
    return FALSE;
  }
  
  if(_UniSub_ChatPlate_ChangeColor(unisub, y, PLATE_COLOR_TOUCH) == TRUE){
    //int view_no = log->chat_view_no - (UNION_CHAT_VIEW_LOG_NUM-1) + y;
    int view_no;
    if(log->chat_view_no < UNION_CHAT_VIEW_LOG_NUM){
      view_no = y;
    }
    else{
      view_no = log->chat_view_no - (UNION_CHAT_VIEW_LOG_NUM-1) + y;
    }
    _UniSub_EntryPlateTouchWork(unisub, log, view_no);
    {//タッチした相手をフォーカス対象にセットする
      UNION_CHAT_DATA *chat = UnionChat_GetReadBuffer(log, view_no);
      if(chat != NULL){
        GFL_STD_MemCopy(chat->mac_address, unisys->my_situation.focus_mac_address, 6);
      }
    }
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * プレートタッチワークから空きを探す
 *
 * @param   unisub		
 * @param   view_no		
 *
 * @retval  PLATE_TOUCH_WORK *		
 */
//--------------------------------------------------------------
static PLATE_TOUCH_WORK * _UniSub_GetPlateTouchPtr(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log, int view_no)
{
  int i;
  
  //既にエントリー済みなら同ワークの値を更新
  for(i = 0; i < UNION_CHAT_VIEW_LOG_NUM; i++){
    if(unisub->plate_touch[i].life > 0 && view_no == unisub->plate_touch[i].view_no){
      return &unisub->plate_touch[i];
    }
  }

  //空きを探す
  for(i = 0; i < UNION_CHAT_VIEW_LOG_NUM; i++){
    if(unisub->plate_touch[i].life == 0){
      return &unisub->plate_touch[i];
    }
  }
  
  //画面外に出ているview_noの領域を潰して使用する
  for(i = 0; i < UNION_CHAT_VIEW_LOG_NUM; i++){
    if(unisub->plate_touch[i].view_no > log->chat_view_no
        || unisub->plate_touch[i].view_no <= log->chat_view_no - UNION_CHAT_VIEW_LOG_NUM){
      return &unisub->plate_touch[i];
    }
  }
  
  GF_ASSERT(0);
  return NULL;
}

//--------------------------------------------------------------
/**
 * プレートタッチワークにエントリーする
 *
 * @param   unisub		
 * @param   view_no		
 * 
 * @retval  TRUE:エントリー成功　FALSE:エントリー失敗
 */
//--------------------------------------------------------------
static BOOL _UniSub_EntryPlateTouchWork(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log, int view_no)
{
  PLATE_TOUCH_WORK *pt;
  
  pt = _UniSub_GetPlateTouchPtr(unisub, log, view_no);
  if(pt == NULL){
    return FALSE; //通常ありえない　内部でASSERTを仕込んでいる
  }
  pt->life = UNION_PLATE_TOUCH_LIFE;
  pt->view_no = view_no;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * プレートの色更新処理
 *
 * @param   unisub		
 */
//--------------------------------------------------------------
static void _UniSub_ChatPlate_Update(UNION_SUBDISP_PTR unisub, UNION_CHAT_LOG *log)
{
  int y, write_pos;
  
  for(y = 0; y < UNION_CHAT_VIEW_LOG_NUM; y++){
    if(unisub->plate_touch[y].life > 0){
      unisub->plate_touch[y].life--;
      if(unisub->plate_touch[y].life == 0){
        if(log->chat_view_no < UNION_CHAT_VIEW_LOG_NUM){
          write_pos = unisub->plate_touch[y].view_no;
        }
        else{
          write_pos = (UNION_CHAT_VIEW_LOG_NUM-1) 
            + unisub->plate_touch[y].view_no - log->chat_view_no;
        }
        if(write_pos >= 0 && write_pos < UNION_CHAT_VIEW_LOG_NUM){
          _UniSub_ChatPlate_ChangeColor(unisub, write_pos, PLATE_COLOR_NORMAL);
        }
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * プレートの色をタッチ色か、ノーマル色か変更する
 *
 * @param   unisub		
 * @param   plate_no		プレート番号
 * @param   color		    変更後の色指定
 * 
 * @retval  TRUE:色変え成功
 */
//--------------------------------------------------------------
static BOOL _UniSub_ChatPlate_ChangeColor(UNION_SUBDISP_PTR unisub, int plate_no, PLATE_COLOR color)
{
  u16 *scrn_buf;
  u16 change_palno, now_palno;
  
  scrn_buf = GFL_BG_GetScreenBufferAdrs(UNION_FRAME_S_PLATE);
  scrn_buf += ((UNION_PLATE_START_X + UNION_PLATE_SIZE_X * UNION_PLATE_SIZE_Y) 
    + (UNION_PLATE_SIZE_X * UNION_PLATE_SIZE_Y) * plate_no) / 2;
  now_palno = (*scrn_buf) >> 12;
  switch(now_palno){
  case UNION_SUBBG_PAL_MALE:
  case UNION_SUBBG_PAL_MALE_TOUCH:
    if(color == PLATE_COLOR_NORMAL){
      change_palno = UNION_SUBBG_PAL_MALE;
    }
    else{
      change_palno = UNION_SUBBG_PAL_MALE_TOUCH;
    }
    break;
  case UNION_SUBBG_PAL_FEMALE:
  case UNION_SUBBG_PAL_FEMALE_TOUCH:
    if(color == PLATE_COLOR_NORMAL){
      change_palno = UNION_SUBBG_PAL_FEMALE;
    }
    else{
      change_palno = UNION_SUBBG_PAL_FEMALE_TOUCH;
    }
    break;
  default:
    return FALSE;
  }
  
  GFL_BG_ChangeScreenPalette(UNION_FRAME_S_PLATE, 
    UNION_PLATE_START_X, UNION_PLATE_START_Y + UNION_PLATE_SIZE_Y * plate_no, 
    UNION_PLATE_SIZE_X, UNION_PLATE_SIZE_Y, change_palno);
  GFL_BG_LoadScreenV_Req(UNION_FRAME_S_PLATE);
  return TRUE;
}
