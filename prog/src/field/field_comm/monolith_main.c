//==============================================================================
/**
 * @file    monolith_main.c
 * @brief   モノリス：メイン処理
 * @author  matsuda
 * @date    2009.11.20(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "arc_def.h"
#include "field/monolith_main.h"
#include "monolith_common.h"
#include "monolith.naix"
#include "system/gfl_use.h"
#include "message.naix"
#include "font/font.naix" //NARC_font_large_gftr
#include "system/wipe.h"
#include "gamesystem/g_power.h"

#include "mission.naix"
#include "app_menu_common.naix"
#include "app/app_menu_common.h"
#include "system/net_err.h"
#include "intrude_comm_command.h"
#include "intrude_work.h"



//==============================================================================
//  定数定義
//==============================================================================
///モノリス共通素材アクターパレット本数
#define MONOLITH_COMMON_PLTT_NUM    (3)

///PFDで管理するメインBGパレットサイズ
#define MONOLITH_PLTT_SIZE_BG_MAIN      (16 - 2)  //-2=「YES/NO」用
///PFDで管理するサブBGパレットサイズ
#define MONOLITH_PLTT_SIZE_BG_SUB       (16 - 2)  //-2=「YES/NO」用
///PFDで管理するメインOBJパレットサイズ
#define MONOLITH_PLTT_SIZE_OBJ_MAIN     (16 - 2)  //-2=ローカライズ+通信アイコン
///PFDで管理するサブOBJパレットサイズ
#define MONOLITH_PLTT_SIZE_OBJ_SUB      (16 - 2)  //-1=ローカライズ+通信アイコン


//==============================================================================
//  構造体定義
//==============================================================================
///モノリス制御システム
typedef struct{
  MONOLITH_SETUP setup;             ///<モノリス全画面共通設定データ
  MONOLITH_COMMON_WORK common;      ///<モノリス全画面共用ワーク
  MONOLITH_APP_PARENT app_parent;   ///<APP Proc用のParentWork
	GFL_TCB *vintr_tcb;               ///<VBlankTCBへのポインタ
  GFL_PROCSYS *procsys_up;          ///<上画面PROCシステム
  GFL_PROCSYS *procsys_down;        ///<下画面PROCシステム
  u8 menu_index;                    ///<MONOLITH_MENU_???
  u8 proc_up_occ;                   ///<TRUE:上画面PROC有効
  u8 proc_down_occ;                 ///<TRUE:下画面PROC有効
  u8 padding;
  u32 alloc_menubar_pos;            ///<メニューバー
}MONOLITH_SYSTEM;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithProc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithProc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _Setup_VramSetting(void);
static void _Setup_VramExit(void);
static void _Setup_LibSetting(void);
static void _Setup_LibExit(void);
static void _Setup_PaletteSetting(MONOLITH_SETUP *setup);
static void _Setup_PaletteExit(MONOLITH_SETUP *setup);
static void _Setup_BGFrameSetting(void);
static void _Setup_BGFrameExit(void);
static void _Setup_BGGraphicLoad(MONOLITH_SYSTEM *monosys, MONOLITH_SETUP *setup);
static void _Setup_BGGraphicUnload(MONOLITH_SYSTEM *monosys);
static void _Setup_MessageSetting(MONOLITH_SETUP *setup);
static void _Setup_MessageExit(MONOLITH_SETUP *setup);
static void _Setup_ActorSetting(MONOLITH_SETUP *setup);
static void _Setup_ActorExit(MONOLITH_SETUP *setup);
static void _Setup_OBJGraphicLoad(MONOLITH_SETUP *setup);
static void _Setup_OBJGraphicUnload(MONOLITH_SETUP *setup);
static void _VblankFunc(GFL_TCB *tcb, void *work);
static void _Setup_MissionListData(MONOLITH_SYSTEM *monosys, MONOLITH_PARENT_WORK *parent);


//==============================================================================
//  データ
//==============================================================================
///各画面で使用するPROCテーブル   ※MONOLITH_MENUと並びを同じにする事！
static const struct{
  const GFL_PROC_DATA *proc_up;    ///<上画面PROC
  const GFL_PROC_DATA *proc_down;  ///<下画面PROC
}MonolithProcTbl[] = {
  {//MONOLITH_MENU_TITLE
    &MonolithAppProc_Up_PalaceMap,
    &MonolithAppProc_Down_Title,
  },
  {//MONOLITH_MENU_MISSION
    &MonolithAppProc_Up_MissionExplain,
    &MonolithAppProc_Down_MissionSelect,
  },
  {//MONOLITH_MENU_POWER
    &MonolithAppProc_Up_PowerExplain,
    &MonolithAppProc_Down_PowerSelect,
  },
  {//MONOLITH_MENU_STATUS
    &MonolithAppProc_Up_PalaceMap,
    &MonolithAppProc_Down_Status,
  },
};

///モノリスメインPROCデータ
const GFL_PROC_DATA MonolithProcData = {
  MonolithProc_Init,
  MonolithProc_Main,
  MonolithProc_End,
};

//--------------------------------------------------------------
//	BG
//--------------------------------------------------------------
///VRAMバンク設定
static const GFL_DISP_VRAM MonolithVramBank = {
	GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_128K,		// サブOBJマッピングモード
};

///BGモード設定
static const GFL_BG_SYS_HEADER MonolithBgSysHeader = {
	GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
};


//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_SYSTEM *monosys;
  MONOLITH_PARENT_WORK *parent = pwk;
  
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
//	G2_BlendNone();
//	G2S_BlendNone();
	G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 15, 15);
	G2S_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 15, 15);

	//上下画面設定
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MONOLITH, 0x70000 );
  
  monosys = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_SYSTEM), HEAPID_MONOLITH);
  GFL_STD_MemClear(monosys, sizeof(MONOLITH_SYSTEM));
  monosys->common.power_select_no = GPOWER_ID_NULL;
  
  //ミッションリストに従ってミッションデータの配列を作成
  _Setup_MissionListData(monosys, parent);
  
	monosys->setup.hdl = GFL_ARC_OpenDataHandle(ARCID_MONOLITH, HEAPID_MONOLITH);
  {
    _Setup_VramSetting();
    _Setup_LibSetting();
    _Setup_PaletteSetting(&monosys->setup);
    _Setup_BGFrameSetting();
    _Setup_BGGraphicLoad(monosys, &monosys->setup);
    _Setup_MessageSetting(&monosys->setup);
    _Setup_ActorSetting(&monosys->setup);
    _Setup_OBJGraphicLoad(&monosys->setup);
    monosys->setup.powerdata = GPOWER_PowerData_LoadAlloc(HEAPID_MONOLITH);
  }

  //ローカルPROC作成
  monosys->procsys_up = GFL_PROC_LOCAL_boot(HEAPID_MONOLITH);
  monosys->procsys_down = GFL_PROC_LOCAL_boot(HEAPID_MONOLITH);
  
  APP_PRINTSYS_COMMON_PrintStreamInit(&monosys->setup.app_printsys, APP_PRINTSYS_COMMON_TYPE_BOTH);
  
	//VブランクTCB登録
	monosys->vintr_tcb = GFUser_VIntr_CreateTCB(_VblankFunc, monosys, MONOLITH_VINTR_TCB_PRI_MAIN);
  
  //通信アイコンリロード
  GFL_NET_ReloadIconTopOrBottom(FALSE, HEAPID_MONOLITH);

  monosys->app_parent.parent = parent;
  monosys->app_parent.setup = &monosys->setup;
  monosys->app_parent.common = &monosys->common;
  return GFL_PROC_RES_FINISH;
}


//--------------------------------------------------------------
/**
 * @brief   プロセス関数：メイン
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_SYSTEM *monosys = mywk;
  MONOLITH_PARENT_WORK *parent = pwk;
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(parent->gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  enum{
    SEQ_INIT,
    SEQ_MONOLITH_IN,
    SEQ_INIT_WAIT,
    SEQ_PROC_MAIN,
    SEQ_END,
    SEQ_MONOLITH_OUT,
    SEQ_END_WAIT,
    SEQ_FINISH,
  };
  
  switch(*seq){
  case SEQ_INIT:
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, 
      WIPE_DEF_DIV, WIPE_DEF_SYNC, GFL_HEAP_LOWID(HEAPID_MONOLITH));
    (*seq)++;
    break;
  case SEQ_MONOLITH_IN:
    if(intcomm == NULL || NetErr_App_CheckError()){
      (*seq)++;
    }
    else{
      if(IntrudeSend_OtherMonolithIn() == TRUE){
        (*seq)++;
      }
    }
    break;
  case SEQ_INIT_WAIT:
    if(WIPE_SYS_EndCheck() == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_PROC_MAIN:
    //侵入が起動していた状態で切断状態になったらモノリス画面を終了させる
    if(NetErr_App_CheckError() || (intcomm == NULL && GameCommSys_GetLastStatus(game_comm) != GAME_COMM_LAST_STATUS_NULL) || (intcomm == NULL && GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION)){
      monosys->app_parent.force_finish = TRUE;
    }
    
    if(monosys->app_parent.force_finish == FALSE){
      //上下、各PROCが無い場合は次のPROCを生成
      if(monosys->proc_up_occ == FALSE){
        GFL_PROC_LOCAL_CallProc(monosys->procsys_up, NO_OVERLAY_ID, 
          MonolithProcTbl[monosys->menu_index].proc_up, &monosys->app_parent);
        monosys->proc_up_occ = TRUE;
      }
      if(monosys->proc_down_occ == FALSE){
        GFL_FONTSYS_SetColor(   //一応文字色を標準に戻す処理を入れておく
          MONOLITH_FONT_DEFCOLOR_LETTER, 
          MONOLITH_FONT_DEFCOLOR_SHADOW, MONOLITH_FONT_DEFCOLOR_BACK );
        GFL_PROC_LOCAL_CallProc(monosys->procsys_down, NO_OVERLAY_ID, 
          MonolithProcTbl[monosys->menu_index].proc_down, &monosys->app_parent);
        monosys->proc_down_occ = TRUE;
      }
    }

    {
      GFL_PROC_MAIN_STATUS up_status, down_status;
      
      //下画面が主導権を握るので下から処理
      down_status = GFL_PROC_LOCAL_Main(monosys->procsys_down);
      if(monosys->app_parent.next_menu_index == MONOLITH_MENU_END
          || (down_status != GFL_PROC_MAIN_VALID && monosys->app_parent.force_finish == TRUE)){
        monosys->app_parent.up_proc_finish = TRUE;
      }
      else if(monosys->menu_index != monosys->app_parent.next_menu_index
          && MonolithProcTbl[monosys->menu_index].proc_up 
          != MonolithProcTbl[monosys->app_parent.next_menu_index].proc_up){
        //下画面PROCが次のPROCへ行こうとしていて上画面のPROCが今と一致していないのであれば
        //上画面PROCに対してFINISHを知らせる
        monosys->app_parent.up_proc_finish = TRUE;
      }
      
      //上画面PROC実行
      up_status = GFL_PROC_LOCAL_Main(monosys->procsys_up);
      
      if(up_status != GFL_PROC_MAIN_VALID || down_status != GFL_PROC_MAIN_VALID){
        if(monosys->app_parent.force_finish == TRUE 
            || monosys->app_parent.next_menu_index == MONOLITH_MENU_END){
          OS_TPrintf("モノリス画面終了待ち up=%d, down=%d\n", up_status, down_status);
          if(up_status == GFL_PROC_MAIN_NULL && down_status == GFL_PROC_MAIN_NULL){
            (*seq)++;
          }
        }
        else{
          monosys->menu_index = monosys->app_parent.next_menu_index;
          if(up_status != GFL_PROC_MAIN_VALID){
            monosys->proc_up_occ = FALSE;
            monosys->app_parent.up_proc_finish = FALSE;
          }
          if(down_status != GFL_PROC_MAIN_VALID){
            monosys->proc_down_occ = FALSE;
          }
        }
      }
    }
    break;
  case SEQ_END:
    WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, 
      WIPE_DEF_DIV, WIPE_DEF_SYNC, GFL_HEAP_LOWID(HEAPID_MONOLITH));
    (*seq)++;
    break;
  case SEQ_MONOLITH_OUT:
    if(intcomm == NULL || NetErr_App_CheckError()){
      (*seq)++;
    }
    else{
      if(IntrudeSend_OtherMonolithOut() == TRUE){
        (*seq)++;
      }
    }
    break;
  case SEQ_END_WAIT:
    if(WIPE_SYS_EndCheck() == TRUE){
      (*seq)++;
    }
    break;
  case SEQ_FINISH:
    return GFL_PROC_RES_FINISH;
  }
  
	GFL_TCBL_Main( monosys->setup.tcbl_sys );
	PRINTSYS_QUE_Main( monosys->setup.printQue );
	GFL_CLACT_SYS_Main();

  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：終了
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT MonolithProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_SYSTEM *monosys = mywk;
  
  GFL_PROC_LOCAL_Exit(monosys->procsys_up);
  GFL_PROC_LOCAL_Exit(monosys->procsys_down);
  
	GFL_TCB_DeleteTask(monosys->vintr_tcb);

  GPOWER_PowerData_Unload(monosys->setup.powerdata);
  _Setup_OBJGraphicUnload(&monosys->setup);
  _Setup_ActorExit(&monosys->setup);
  _Setup_MessageExit(&monosys->setup);
  _Setup_BGGraphicUnload(monosys);
  _Setup_BGFrameExit();
  _Setup_PaletteExit(&monosys->setup);
  _Setup_LibExit();
  _Setup_VramExit();

	GFL_ARC_CloseDataHandle(monosys->setup.hdl);

	G2_BlendNone();
	G2S_BlendNone();

  GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_MONOLITH);
  return GFL_PROC_RES_FINISH;
}


//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   VRAMバンク＆モード設定
 */
//--------------------------------------------------------------
static void _Setup_VramSetting(void)
{
	GFL_DISP_SetBank( &MonolithVramBank );
	GFL_BG_Init( HEAPID_MONOLITH );
	GFL_BG_SetBGMode( &MonolithBgSysHeader );

	//VRAMクリア
	GFL_STD_MemClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
}

//--------------------------------------------------------------
/**
 * VRAMバンク＆モード設定解放
 */
//--------------------------------------------------------------
static void _Setup_VramExit(void)
{
  GFL_BG_Exit();
}

//--------------------------------------------------------------
/**
 * GFLIB関連の初期設定
 */
//--------------------------------------------------------------
static void _Setup_LibSetting(void)
{
  GFL_BMPWIN_Init( HEAPID_MONOLITH );
}

//--------------------------------------------------------------
/**
 * GFLIB関連の破棄処理
 */
//--------------------------------------------------------------
static void _Setup_LibExit(void)
{
  GFL_BMPWIN_Exit();
}

//--------------------------------------------------------------
/**
 * パレットシステム作成
 *
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_PaletteSetting(MONOLITH_SETUP *setup)
{
  setup->pfd = PaletteFadeInit(HEAPID_MONOLITH);
  PaletteFadeWorkAllocSet(setup->pfd, FADE_MAIN_OBJ, 
    MONOLITH_PLTT_SIZE_OBJ_MAIN*32, HEAPID_MONOLITH);
  PaletteFadeWorkAllocSet(setup->pfd, FADE_SUB_OBJ, 
    MONOLITH_PLTT_SIZE_OBJ_SUB*32, HEAPID_MONOLITH);
  PaletteFadeWorkAllocSet(setup->pfd, FADE_MAIN_BG, 
    MONOLITH_PLTT_SIZE_BG_MAIN*32, HEAPID_MONOLITH);
  PaletteFadeWorkAllocSet(setup->pfd, FADE_SUB_BG, 
    MONOLITH_PLTT_SIZE_BG_SUB*32, HEAPID_MONOLITH);
  PaletteTrans_AutoSet(setup->pfd, TRUE);
}

//--------------------------------------------------------------
/**
 * パレットシステム破棄
 *
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_PaletteExit(MONOLITH_SETUP *setup)
{
  PaletteFadeWorkAllocFree(setup->pfd, FADE_MAIN_OBJ);
  PaletteFadeWorkAllocFree(setup->pfd, FADE_SUB_OBJ);
  PaletteFadeWorkAllocFree(setup->pfd, FADE_MAIN_BG);
  PaletteFadeWorkAllocFree(setup->pfd, FADE_SUB_BG);
  PaletteFadeFree(setup->pfd);
}

//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 */
//--------------------------------------------------------------
static void _Setup_BGFrameSetting(void)
{
  int i;
  
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {	//メイン画面BGフレーム
		{//GFL_BG_FRAME3_M
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			MONO_BG_COMMON_SCRBASE, MONO_BG_COMMON_CHARBASE, MONO_BG_COMMON_CHAR_SIZE,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
	};

	static const GFL_BG_BGCNT_HEADER sub_bgcnt_frame[] = {	//サブ画面BGフレーム
		{//GFL_BG_FRAME3_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			MONO_BG_COMMON_SCRBASE, MONO_BG_COMMON_CHARBASE, MONO_BG_COMMON_CHAR_SIZE,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME3_M,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( GFL_BG_FRAME3_S,   &sub_bgcnt_frame[0],   GFL_BG_MODE_TEXT );

//	GFL_BG_FillCharacter( FRAME_BACK_M, 0x00, 1, 0 );

	GFL_BG_FillScreen( GFL_BG_FRAME3_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( GFL_BG_FRAME3_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

  for(i = GFL_BG_FRAME0_M; i <= GFL_BG_FRAME3_S; i++){
  	GFL_BG_SetVisible(i, VISIBLE_OFF);
  }
	GFL_BG_SetVisible(GFL_BG_FRAME3_M, VISIBLE_ON);
	GFL_BG_SetVisible(GFL_BG_FRAME3_S, VISIBLE_ON);
}

//--------------------------------------------------------------
/**
 * BGフレーム解放
 */
//--------------------------------------------------------------
static void _Setup_BGFrameExit(void)
{
	GFL_BG_SetVisible(GFL_BG_FRAME3_M, VISIBLE_OFF);
	GFL_BG_SetVisible(GFL_BG_FRAME3_S, VISIBLE_OFF);
  GFL_BG_FreeBGControl(GFL_BG_FRAME3_M);
  GFL_BG_FreeBGControl(GFL_BG_FRAME3_S);
}

//--------------------------------------------------------------
/**
 * @brief   共通素材BGグラフィックをVRAMへ転送
 */
//--------------------------------------------------------------
static void _Setup_BGGraphicLoad(MONOLITH_SYSTEM *monosys, MONOLITH_SETUP *setup)
{
	//共通パレット
  PaletteWorkSetEx_ArcHandle(setup->pfd, setup->hdl, 
    NARC_monolith_mono_bgu_NCLR, HEAPID_MONOLITH, FADE_MAIN_BG, 
    MONOLITH_PLTT_SIZE_BG_MAIN*0x20, 0, 0);
  PaletteWorkSetEx_ArcHandle(setup->pfd, setup->hdl, 
    NARC_monolith_mono_bgd_NCLR, HEAPID_MONOLITH, FADE_SUB_BG, 
    MONOLITH_PLTT_SIZE_BG_SUB*0x20, 0, 0);

	//メイン画面：共通素材
	GFL_ARCHDL_UTIL_TransVramBgCharacter(setup->hdl, NARC_monolith_mono_bgu_lz_NCGR, 
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_MONOLITH);
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgu_base_lz_NSCR, 
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_MONOLITH);

	//サブ画面：共通素材
	GFL_ARCHDL_UTIL_TransVramBgCharacter(setup->hdl, NARC_monolith_mono_bgd_lz_NCGR, 
		GFL_BG_FRAME3_S, 0, 0, TRUE, HEAPID_MONOLITH);
	GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgd_base_lz_NSCR, 
		GFL_BG_FRAME3_S, 0, 0, TRUE, HEAPID_MONOLITH);

	//フォントパレット転送
  PaletteWorkSetEx_Arc(setup->pfd, ARCID_FONT, NARC_font_default_nclr, HEAPID_MONOLITH,
      FADE_MAIN_BG, 0, MONOLITH_BG_UP_FONT_PALNO * 16, 0);
  PaletteWorkSetEx_Arc(setup->pfd, ARCID_FONT, NARC_font_default_nclr, HEAPID_MONOLITH,
      FADE_SUB_BG, 0, MONOLITH_BG_DOWN_FONT_PALNO * 16, 0);

	//メニューバー
  {
    ARCHANDLE *app_handle = GFL_ARC_OpenDataHandle(ARCID_APP_MENU_COMMON, HEAPID_MONOLITH);
  
    monosys->alloc_menubar_pos = GFL_BG_AllocCharacterArea(
      GFL_BG_FRAME3_S, MENUBAR_CGX_SIZE, GFL_BG_CHRAREA_GET_B );
    GFL_ARCHDL_UTIL_TransVramBgCharacter(
      app_handle, NARC_app_menu_common_menu_bar_NCGR, GFL_BG_FRAME3_S, monosys->alloc_menubar_pos, 
      MENUBAR_CGX_SIZE, FALSE, HEAPID_MONOLITH);
    
    APP_COMMON_MenuBarScrn_Fusion(app_handle, GFL_BG_FRAME3_S, HEAPID_MONOLITH, 
      monosys->alloc_menubar_pos, MONOLITH_MENUBAR_PALNO);
    
    PaletteWorkSet_ArcHandle(setup->pfd, app_handle, NARC_app_menu_common_menu_bar_NCLR,
      HEAPID_MONOLITH, FADE_SUB_BG, 0x20, MONOLITH_MENUBAR_PALNO*16);
  
  	GFL_ARC_CloseDataHandle(app_handle);
  	GFL_BG_LoadScreenReq( GFL_BG_FRAME3_S );
  }

	GFL_BG_LoadScreenReq( GFL_BG_FRAME3_M );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME3_S );
}

//--------------------------------------------------------------
/**
 * @brief   共通素材BGグラフィックを解放
 */
//--------------------------------------------------------------
static void _Setup_BGGraphicUnload(MONOLITH_SYSTEM *monosys)
{
  GFL_BG_FreeCharacterArea( GFL_BG_FRAME3_S, monosys->alloc_menubar_pos, MENUBAR_CGX_SIZE );
}

//--------------------------------------------------------------
/**
 * @brief   メッセージ描画関連の設定
 *
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_MessageSetting(MONOLITH_SETUP *setup)
{
//	GFL_FONTSYS_Init();
  GFL_FONTSYS_SetColor( 
    MONOLITH_FONT_DEFCOLOR_LETTER, MONOLITH_FONT_DEFCOLOR_SHADOW, MONOLITH_FONT_DEFCOLOR_BACK );

	setup->tcbl_sys = GFL_TCBL_Init(HEAPID_MONOLITH, HEAPID_MONOLITH, 4, 32);
	setup->font_handle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_MONOLITH );
	setup->printQue = PRINTSYS_QUE_Create( HEAPID_MONOLITH );
	setup->wordset = WORDSET_Create(HEAPID_MONOLITH);

	setup->mm_mission_mono = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_mission_monolith_dat, HEAPID_MONOLITH);
	setup->mm_power = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_power_dat, HEAPID_MONOLITH);
	setup->mm_power_explain = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_power_explain_dat, HEAPID_MONOLITH);
	setup->mm_monolith = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monolith_dat, HEAPID_MONOLITH);
}

//--------------------------------------------------------------
/**
 * @brief   メッセージ関連破棄
 *
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_MessageExit(MONOLITH_SETUP *setup)
{
	GFL_MSG_Delete(setup->mm_mission_mono);
	GFL_MSG_Delete(setup->mm_power);
	GFL_MSG_Delete(setup->mm_power_explain);
	GFL_MSG_Delete(setup->mm_monolith);
	WORDSET_Delete(setup->wordset);
	PRINTSYS_QUE_Clear(setup->printQue);
	PRINTSYS_QUE_Delete(setup->printQue);
	GFL_FONT_Delete(setup->font_handle);
	GFL_TCBL_Exit(setup->tcbl_sys);
}

//--------------------------------------------------------------
/**
 * @brief   アクター設定
 *
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_ActorSetting(MONOLITH_SETUP *setup)
{
	GFL_CLSYS_INIT clsys_init = GFL_CLSYSINIT_DEF_DIVSCREEN;
	
	clsys_init.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
	clsys_init.oamnum_main = 128-GFL_CLSYS_OAMMAN_INTERVAL;
	clsys_init.oamst_sub = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
	clsys_init.oamnum_sub = 128-GFL_CLSYS_OAMMAN_INTERVAL;
	clsys_init.CGR_RegisterMax = 96;  //BMPOAMで大量にリソース登録する為
	clsys_init.CELL_RegisterMax = 64; //BMPOAMで大量にリソース登録する為
	GFL_CLACT_SYS_Create(&clsys_init, &MonolithVramBank, HEAPID_MONOLITH);
	
	setup->clunit = GFL_CLACT_UNIT_Create(96, 0, HEAPID_MONOLITH);
	GFL_CLACT_UNIT_SetDefaultRend(setup->clunit);

	//OBJ表示ON
//	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
//	GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);

  //PLTT SLOT
  setup->plttslot = PLTTSLOT_Init(HEAPID_MONOLITH, 16, 16);

  //BMP OAM
  setup->bmpoam_sys = BmpOam_Init(HEAPID_MONOLITH, setup->clunit);
}

//--------------------------------------------------------------
/**
 * アクター破棄
 *
 * @param   setup		
 */
//--------------------------------------------------------------
static void _Setup_ActorExit(MONOLITH_SETUP *setup)
{
	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_OFF);
	GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_OFF);
	
	BmpOam_Exit(setup->bmpoam_sys);
  GFL_CLACT_UNIT_Delete(setup->clunit);
  GFL_CLACT_SYS_Delete();
  
  PLTTSLOT_Exit(setup->plttslot);
}

//--------------------------------------------------------------
/**
 * @brief   共通素材OBJグラフィックをVRAMへ転送
 */
//--------------------------------------------------------------
static void _Setup_OBJGraphicLoad(MONOLITH_SETUP *setup)
{
  int i;
  ARCHANDLE *font_hdl;
  CLSYS_DRAW_TYPE draw_type;
  
  font_hdl = GFL_ARC_OpenDataHandle(ARCID_FONT, HEAPID_MONOLITH);
  
  for(i = 0; i < COMMON_RESOURCE_INDEX_MAX; i++){
    draw_type = (i == COMMON_RESOURCE_INDEX_UP) ? CLSYS_DRAW_MAIN : CLSYS_DRAW_SUB;
    
  	//共通パレット
    setup->common_res[i].pltt_index = PLTTSLOT_ResourceSet_PalAnm(setup->pfd, setup->plttslot, 
      setup->hdl, NARC_monolith_mono_obj_NCLR, draw_type, 
      MONOLITH_COMMON_PLTT_NUM, HEAPID_MONOLITH);

  	//共通素材CGX
    setup->common_res[i].char_index = GFL_CLGRP_CGR_Register(
      setup->hdl, NARC_monolith_mono_obj_lz_NCGR, TRUE, draw_type, HEAPID_MONOLITH );

    //共通素材CELL
    setup->common_res[i].cell_index = GFL_CLGRP_CELLANIM_Register(
      setup->hdl, NARC_monolith_mono_obj_NCER, NARC_monolith_mono_obj_NANR, HEAPID_MONOLITH);

  	//フォントパレット転送(BMP FONT用)
    setup->common_res[i].pltt_bmpfont_index = PLTTSLOT_ResourceCompSet_PalAnm(setup->pfd, 
      setup->plttslot, font_hdl, NARC_font_default_nclr, draw_type, 1, HEAPID_MONOLITH);
  }

  GFL_ARC_CloseDataHandle(font_hdl);
}

//--------------------------------------------------------------
/**
 * @brief   共通素材OBJグラフィック解放処理
 */
//--------------------------------------------------------------
static void _Setup_OBJGraphicUnload(MONOLITH_SETUP *setup)
{
  int i;
  CLSYS_DRAW_TYPE draw_type;
  
  for(i = 0; i < COMMON_RESOURCE_INDEX_MAX; i++){
    draw_type = (i == COMMON_RESOURCE_INDEX_UP) ? CLSYS_DRAW_MAIN : CLSYS_DRAW_SUB;

    PLTTSLOT_ResourceFree(setup->plttslot, setup->common_res[i].pltt_index, draw_type);
    GFL_CLGRP_CGR_Release(setup->common_res[i].char_index);
    GFL_CLGRP_CELLANIM_Release(setup->common_res[i].cell_index);
    PLTTSLOT_ResourceFree(setup->plttslot, setup->common_res[i].pltt_bmpfont_index, draw_type);
  }
}

//--------------------------------------------------------------
/**
 * @brief   VブランクTCB
 *
 * @param   tcb			
 * @param   work		
 */
//--------------------------------------------------------------
static void _VblankFunc(GFL_TCB *tcb, void *work)
{
  MONOLITH_SYSTEM *monosys = work;
  
	GFL_CLACT_SYS_VBlankFunc();
  PaletteFadeTrans( monosys->setup.pfd );
}

//--------------------------------------------------------------
/**
 * ミッションリストに従ってミッションデータの配列を作成
 *
 * @param   monosys		
 * @param   parent		
 */
//--------------------------------------------------------------
static void _Setup_MissionListData(MONOLITH_SYSTEM *monosys, MONOLITH_PARENT_WORK *parent)
{
  MISSION_TYPE mission_type;
  MISSION_CONV_DATA *mcd;
  u32 mcd_max;
  int mission_no;
  
  if(parent->list_occ == FALSE){
    return;
  }
  
  mcd = GFL_ARC_LoadDataAlloc(ARCID_MISSION, NARC_mission_mission_data_bin, HEAPID_MONOLITH);
  mcd_max = GFL_ARC_GetDataSize(ARCID_MISSION, NARC_mission_mission_data_bin);
  
  for(mission_type = 0; mission_type < MISSION_TYPE_MAX; mission_type++){
    mission_no = parent->list.occupy.mlst.mission_no[mission_type];
    if(mission_no >= mcd_max){
      GF_ASSERT_MSG(0, "mission_no=%d, mcd_max=%d", mission_no, mcd_max);
      mission_no = 0;
    }
    monosys->setup.mission_cdata_array[mission_type] = mcd[mission_no];
  }
  
  GFL_HEAP_FreeMemory(mcd);
}
