//==============================================================================
/**
 * @file	  d_gpower_list.c
 * @brief	  GPOWER配信メニュー
 * @author	ohno
 * @date	  2009.2.12
 */
//==============================================================================
#include <gflib.h>
#if PM_DEBUG
#include "system/main.h"
#include "system/gfl_use.h"
//#include "savedata/contest_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"
#include "sound/pm_sndsys.h"
#include "sound/pm_voice.h"

#include "net\network_define.h"
#include "../../field/event_gtsnego.h"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "debug_message.naix"
#include "test_graphic\d_taya.naix"
#include "msg\debug\msg_d_ohno.h"
#include "test/performance.h"
#include "font/font.naix"

#include "pokeicon/pokeicon.h"
#include "item/item.h"
#include "poke_tool/pokeparty.h"
#include "net_app/wificlub/wifi_p2pmatch.h"
#include "net_app/pokemontrade.h"
#include "poke_tool/monsno_def.h"
#include "debug_ohno.h"


#include "debug/debugwin_sys.h"
#include "test/debug_pause.h"

#include "../matsuda/d_haisin_power.h"
#include "../matsuda/d_haisin_comm.h"


//==============================================================================
//	定数定義
//==============================================================================
///STRBUFポインタの数(メニューの配列より多く持っておく事)
#define D_STRBUF_NUM			(30)


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
  GFL_BMPWIN		*win;
  GFL_BMP_DATA	*bmp;
  //	PRINT_UTIL		printUtil[1];
  BOOL			message_req;		///<TRUE:メッセージリクエストがあった
}DM_MSG_DRAW_WIN;

typedef struct {

  u16		seq;
  HEAPID	heapID;
  int debug_mode;
  int timer;

  HAISIN_POWER_GROUP aGPower;

  int timerGPower;
  int startListGPowerCnt;
  int GPowerOdds;
  BOOL bGPowerSeq;
  BOOL isInit;

  GFL_FONT		*fontHandle;
  PRINT_STREAM	*printStream;
  GFL_MSGDATA		*mm;
  STRBUF			*strbuf[D_STRBUF_NUM];
  STRBUF			*strbufEx;
  GFL_TCBLSYS		*tcbl;
  DM_MSG_DRAW_WIN drawwin;
  WORDSET *pWordSet;
  //アクター
  GFL_CLUNIT *clunit;
  GFL_CLWK	*clwk_icon;		//アイテムアイコンアクター
  NNSG2dImagePaletteProxy	icon_pal_proxy;
  NNSG2dCellDataBank	*icon_cell_data;
  NNSG2dAnimBankData	*icon_anm_data;
  void	*icon_cell_res;
  void	*icon_anm_res;

  int cursor_y;

  void *parent_work;

  //バトル検定
  int trialNo;
  int trialType;
  /// ポケモン番号
  int pokeNo;
  int form;

} D_GPOWER_WORK;

///メニューリスト
typedef struct{
  u32 str_id;
  const GFL_PROC_DATA *next_proc;
  void *(*parent_work_func)(D_GPOWER_WORK *);
  u32 ov_id;
}D_MENULIST;

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL DebugOhno_ItemDebug(D_GPOWER_WORK *wk);
static void * _PokeTradeDemoWorkCreate(D_GPOWER_WORK *wk);
static void * _PokeTradeDemo2WorkCreate(D_GPOWER_WORK *wk);
static void * _PokeTradeWorkCreate(D_GPOWER_WORK *wk);
static void * _PokeIrcTradeWorkCreate(D_GPOWER_WORK *wk);
static void * _PokeTradeGtsNegoCreate(D_GPOWER_WORK *wk);
static void * _initTrial(D_GPOWER_WORK *wk);


//==============================================================================
//	外部データ
//==============================================================================
FS_EXTERN_OVERLAY(wificlub);
FS_EXTERN_OVERLAY(ohno_debugapp);
FS_EXTERN_OVERLAY(ircbattlematch);
FS_EXTERN_OVERLAY(pokemon_trade);
FS_EXTERN_OVERLAY(gamesync);
extern const GFL_PROC_DATA MidiTest_ProcData;
extern const GFL_PROC_DATA WifiClubProcData;
extern const GFL_PROC_DATA DebugOhnoMainProcData;
extern const GFL_PROC_DATA DebugLayoutMainProcData;
extern const GFL_PROC_DATA G_SYNC_ProcData;
extern const GFL_PROC_DATA VTRProcData;
extern const GFL_PROC_DATA PokemonTradeWiFiProcData;
extern const GFL_PROC_DATA PokemonTradeProcData;
extern const GFL_PROC_DATA PokemonTradeIrcProcData;
extern const GFL_PROC_DATA DebugSaveAddrProcData;
extern const GFL_PROC_DATA SaveFriendProcData;



extern const GFL_PROC_DATA NetFourParentProcData;
extern const GFL_PROC_DATA NetFourChildProcData;



//==============================================================================
//	データ
//==============================================================================
//メニューデータ
static const D_MENULIST DebugMenuList[] = {
  {//
    DEBUG_OHNO_MSG0031,
    NULL,
    NULL,
    FS_OVERLAY_ID(pokemon_trade)
    },
  {//
    DEBUG_OHNO_MSG0032,
    NULL,
    NULL,
    FS_OVERLAY_ID(pokemon_trade)
    },
  {//                                HAISIN_POWER_MAXにあわせて数を増やす必要がある
    DEBUG_OHNO_MSG0033,
    NULL,
    NULL,
    FS_OVERLAY_ID(pokemon_trade)
    },
  {//
    DEBUG_OHNO_MSG0033,
    NULL,
    NULL,
    FS_OVERLAY_ID(pokemon_trade)
    },
  {//
    DEBUG_OHNO_MSG0033,
    NULL,
    NULL,
    FS_OVERLAY_ID(pokemon_trade)
    },
  {//
    DEBUG_OHNO_MSG0033,
    NULL,
    NULL,
    FS_OVERLAY_ID(ohno_debugapp)
    },
  {//
    DEBUG_OHNO_MSG0033,
    NULL,
    NULL,
    FS_OVERLAY_ID(ohno_debugapp)
    },
  {//
    DEBUG_OHNO_MSG0033,
    NULL,
    NULL,
    FS_OVERLAY_ID(ohno_debugapp)
    },
  {//
    DEBUG_OHNO_MSG0033,
    NULL,
    NULL,
    FS_OVERLAY_ID(ohno_debugapp)
    },
  {//
    DEBUG_OHNO_MSG0033,
    NULL,
    NULL,
    FS_OVERLAY_ID(ohno_debugapp)
    },
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
static GFL_PROC_RESULT DebugOhnoMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
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

  D_GPOWER_WORK* wk;

  DEBUG_PerformanceSetActive(FALSE);
  //デバッグ
  SaveControl_Load(SaveControl_GetPointer());

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PROC, 0x70000 );
  wk = GFL_PROC_AllocWork( proc, sizeof(D_GPOWER_WORK), HEAPID_PROC );
  MI_CpuClear8(wk, sizeof(D_GPOWER_WORK));
  wk->heapID = HEAPID_PROC;

  if(pwk){
    GFL_STD_MemCopy( pwk, &wk->aGPower, sizeof(HAISIN_POWER_GROUP));
  }

  
  GFL_DISP_SetBank( &vramBank );

  //バックグラウンドの色を入れておく
  GFL_STD_MemFill16((void*)HW_BG_PLTT, 0x5ad6, 2);

  // 各種効果レジスタ初期化
  G2_BlendNone();

  // BGsystem初期化
  GFL_BG_Init( wk->heapID );
  GFL_BMPWIN_Init( wk->heapID );
  GFL_FONTSYS_Init();

  PMSND_PlayBGM(  SEQ_BGM_POKECEN );

  //ＢＧモード設定
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }

  // 個別フレーム設定
  {
    static const GFL_BG_BGCNT_HEADER bgcntText = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      };

    GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

    //		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
    GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );

    //		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
    GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

    GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
    GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
  }

  // 上下画面設定
  GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

  GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

  {//メッセージ描画の為の準備
    int i;

    wk->pWordSet    = WORDSET_Create( wk->heapID );
    wk->drawwin.win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 2, 0, 28, 23, 0, GFL_BMP_CHRAREA_GET_F );
    wk->drawwin.bmp = GFL_BMPWIN_GetBmp(wk->drawwin.win);
    GFL_BMP_Clear( wk->drawwin.bmp, 0xff );
    GFL_BMPWIN_MakeScreen( wk->drawwin.win );
    GFL_BMPWIN_TransVramCharacter( wk->drawwin.win );
    //		PRINT_UTIL_Setup( wk->drawwin.printUtil, wk->drawwin.win );

    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

    wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
                                      GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

    //		PRINTSYS_Init( wk->heapID );
    //		wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

    wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_ohno_dat, wk->heapID );
    for(i = 0; i < D_STRBUF_NUM; i++){
      wk->strbuf[i] = GFL_STR_CreateBuffer(64, wk->heapID);
    }
    wk->strbufEx = GFL_STR_CreateBuffer(64, wk->heapID);
    
    wk->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 4, 32 );

    GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI
  }

  //フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0, 0x20, HEAPID_PROC);




  {
    int x;

    wk->bGPowerSeq = wk->aGPower.movemode;

    if(wk->bGPowerSeq) {  //順送りモードの場合 -1から
      wk->startListGPowerCnt = -1;
    }

    for(x = 0; x < wk->aGPower.data_num ; x++){
      wk->GPowerOdds += wk->aGPower.hp[x].odds;
    }
    wk->timerGPower = -1;
    wk->isInit = TRUE;
  }

  // 時間が同じ場合 24時間配布
  if(wk->aGPower.start_time_hour == wk->aGPower.end_time_hour){
    if(wk->aGPower.start_time_min == wk->aGPower.end_time_min){
      wk->aGPower.start_time_hour = 0;
      wk->aGPower.start_time_min = 0;
      wk->aGPower.end_time_hour = 100;
      wk->aGPower.end_time_min = 0;
    }
  }
  //時間がひっくり返っていた場合
  if(wk->aGPower.start_time_hour > wk->aGPower.end_time_hour){
  }
  else if(wk->aGPower.start_time_hour == wk->aGPower.end_time_hour){
    if(wk->aGPower.start_time_min > wk->aGPower.end_time_min){
      
      //時計を１２時間進める
      //エンドタイムを24すすめる
      wk->aGPower.end_time_hour+=24;
      
    }
  }
  
  return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  D_GPOWER_WORK* wk = mywk;
  BOOL ret = 0;
  int i;
  BOOL que_ret=TRUE;

  GFL_TCBL_Main( wk->tcbl );
  //	que_ret = PRINTSYS_QUE_Main( wk->printQue );
  //	if( PRINT_UTIL_Trans( wk->drawwin.printUtil, wk->printQue ) == FALSE){
  //return GFL_PROC_RES_CONTINUE;
  //	}
  //	else{
  if(que_ret == TRUE && wk->drawwin.message_req == TRUE){
    //			GFL_BMP_Clear( wk->bmp, 0xff );
    GFL_BMPWIN_TransVramCharacter( wk->drawwin.win );
    wk->drawwin.message_req = FALSE;
  }
  //	}

  switch(wk->debug_mode){
  case 0:
    ret = DebugOhno_ItemDebug(wk);
    if(ret == TRUE){
      wk->seq = 0;
      wk->debug_mode = 1;
    }
    break;
  case 1:
  default:
    ret = TRUE;
  }

  if(ret == TRUE){
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  D_GPOWER_WORK* wk = mywk;
  int i;

  if(FALSE == HaisinBeacon_ExitWait()){  //終了待ち
    return GFL_PROC_RES_CONTINUE;
  }

  
  GFL_BMPWIN_Delete(wk->drawwin.win);
  for(i = 0; i < D_STRBUF_NUM; i++){
    GFL_STR_DeleteBuffer(wk->strbuf[i]);
  }
  GFL_STR_DeleteBuffer(wk->strbufEx);

  //	PRINTSYS_QUE_Delete(wk->printQue);
  GFL_MSG_Delete(wk->mm);
  WORDSET_Delete(wk->pWordSet);


  GFL_FONT_Delete(wk->fontHandle);
  GFL_TCBL_Exit(wk->tcbl);

  GFL_BG_Exit();
  GFL_BMPWIN_Exit();

  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap(HEAPID_PROC);

  return GFL_PROC_RES_FINISH;
}


static void _printList(D_GPOWER_WORK *wk, int initmsgNo)
{
  const int yini = 12;
  BOOL ret, irc_ret = 0;
  int msg_id;
  int i, x;

  GFL_BMP_Clear( wk->drawwin.bmp, 0xff );
  for(i = 0; i < NELEMS(DebugMenuList); i++){
    
    if(wk->cursor_y == i){
      GFL_MSG_GetString(wk->mm, DEBUG_OHNO_MSG0005, wk->strbuf[i]);
      PRINTSYS_Print(wk->drawwin.bmp, 0, yini+i*2*8, wk->strbuf[i], wk->fontHandle);
    }

    x = 2;
    
    if(i > 1 && (wk->aGPower.data_num!=0) && (wk->aGPower.data_num + 2) > i){
      GFL_MSG_GetString(wk->mm, DebugMenuList[i].str_id, wk->strbufEx);
      
      WORDSET_RegisterGPowerName(wk->pWordSet, 0, wk->aGPower.hp[i-2].g_power_id);
      
      WORDSET_RegisterNumber(wk->pWordSet, 1, wk->aGPower.hp[i-2].time , 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
      WORDSET_RegisterNumber(wk->pWordSet, 2, wk->aGPower.hp[i-2].odds , 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT);
      WORDSET_ExpandStr( wk->pWordSet, wk->strbuf[i], wk->strbufEx  );
    }
    else{
      if(i==0){
        GFL_MSG_GetString(wk->mm, initmsgNo, wk->strbuf[i]);
      }
      else{
        GFL_MSG_GetString(wk->mm, DebugMenuList[i].str_id, wk->strbuf[i]);
      }
    }
    
    PRINTSYS_Print( wk->drawwin.bmp, 10, yini+i*2*8, wk->strbuf[i], wk->fontHandle);
  }
}

//==============================================================================
//
//==============================================================================

typedef enum{
  _START_LISTWRITE,
  _TIMER_WAIT,
  _MESSAGE_REWRITE,
  _NET_START,
  _NET_WAIT,
  _MESSAGE_END,
  _SEQ_END,
  _SEQ_MAX,
  
} _GPOWERSEQ_ENUM;


//--------------------------------------------------------------
/**
 * @brief   ワイヤレス通信テスト
 *
 * @param   wk
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugOhno_ItemDebug(D_GPOWER_WORK *wk)
{
  const int yini = 12;
  BOOL ret, irc_ret = 0;
  int msg_id;
  int i, x;

  GF_ASSERT(wk);

  switch(wk->seq){
  case _START_LISTWRITE:		//リスト描画
    wk->drawwin.message_req = TRUE;

    _printList(wk, DEBUG_OHNO_MSG0034);

    wk->drawwin.message_req = TRUE;
    wk->seq++;
    break;
  case _TIMER_WAIT:
    if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_START){
      wk->seq = _MESSAGE_END;
      break;
    }
    {  //時間がきたら _NET_STARTに
      RTCTime time;
      GFL_RTC_GetTime( &time );

      OS_TPrintf(" %d %d  \n",time.hour, time.minute);
      
      if(wk->aGPower.start_time_hour <= time.hour && wk->aGPower.start_time_min <= time.minute){
        wk->seq = _NET_START;
    //    wk->timerGPower = -1;
        break;
      }
    }
    break;
  case _MESSAGE_REWRITE:
    _printList(wk, DEBUG_OHNO_MSG0031);
    wk->drawwin.message_req = TRUE;
    wk->seq++;
    break;
  case _NET_START:
    {
      if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_START){
        HaisinBeacon_Exit();  //終了
        wk->seq = _MESSAGE_END;
        break;
      }
    }
    {  //時間が終了を過ぎていたら 3に
      RTCTime time;
      GFL_RTC_GetTime( &time );
      if(wk->aGPower.end_time_hour >= 24){
        time.hour += 24;
      }
      if(wk->aGPower.end_time_hour <= time.hour && wk->aGPower.end_time_min <= time.minute){
        HaisinBeacon_Exit();  //終了
        wk->seq = _MESSAGE_END;
        break;
      }
    }
    if(wk->timerGPower == 0){
      if(HaisinBeacon_ExitWait()){  //終了待ち
        wk->timerGPower--;
      }
    }
    else if(wk->timerGPower == 1){
      HaisinBeacon_Exit();  //終了
      wk->timerGPower--;
    }
    else if( wk->isInit == FALSE){
      wk->isInit = HaisinBeacon_InitWait();
    }
    else if(wk->timerGPower > 0){
      wk->timerGPower--;
    }
    else if(wk->timerGPower < 0){
      int x;

      if(wk->bGPowerSeq){  //順送りモードの場合
        wk->startListGPowerCnt++;
        if(wk->startListGPowerCnt >= wk->aGPower.data_num ){
          wk->startListGPowerCnt = 0;
        }
        HaisinBeacon_Init(wk->aGPower.hp[ wk->startListGPowerCnt ].g_power_id, wk->aGPower.beacon_space_time);
        wk->timerGPower = wk->aGPower.hp[ wk->startListGPowerCnt ].time * 60 * 60;
      }
      else{  //乱数の場合
        int rand = GFUser_GetPublicRand(wk->GPowerOdds - 1);
        for(x = 0; x < wk->aGPower.data_num ; x++){
          if(rand < wk->aGPower.hp[x].odds){
            HaisinBeacon_Init(wk->aGPower.hp[x].g_power_id, wk->aGPower.beacon_space_time);
            wk->timerGPower = wk->aGPower.hp[x].time * 60 * 60;
            wk->startListGPowerCnt = x;
            break;
          }
          rand -= wk->aGPower.hp[x].odds;
        }
      }
      wk->seq = _MESSAGE_REWRITE;
      wk->isInit = FALSE;
      wk->cursor_y = wk->startListGPowerCnt + 2;
    }
    if(wk->isInit && wk->timerGPower > 1 ){
      HaisinBeacon_Update();
    }
    break;
  case _NET_WAIT:
    break;
  case _MESSAGE_END:
    wk->drawwin.message_req = TRUE;
    _printList(wk, DEBUG_OHNO_MSG0035);
    wk->seq = _NET_WAIT;
    break;
  default:
    return TRUE;	//ワイヤレス通信処理へ
  }

  return FALSE;
}


FS_EXTERN_OVERLAY(ui_common);
//FS_EXTERN_OVERLAY(app_mail);
//FS_EXTERN_OVERLAY(dpw_common);


static void * _PokeTradeDemoWorkCreate(D_GPOWER_WORK *wk)
{
  POKEMONTRADE_DEMO_PARAM *pWork;
  MYSTATUS* pFriend;


  pWork = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(POKEMONTRADE_DEMO_PARAM));

  if(wk->pokeNo!=0){
    pWork->pMyPoke = PP_Create(wk->pokeNo, 100, 123456, GFL_HEAPID_APP);
    pWork->pNPCPoke = PP_Create(wk->pokeNo, 100, 123456, GFL_HEAPID_APP);
    PP_Put(pWork->pMyPoke, ID_PARA_form_no, wk->form);
    PP_Put(pWork->pNPCPoke, ID_PARA_form_no, wk->form);
  }
  else{
    pWork->pMyPoke = PP_Create(MONSNO_162, 100, 123456, GFL_HEAPID_APP);
    pWork->pNPCPoke = PP_Create(MONSNO_310, 100, 123456, GFL_HEAPID_APP);
  }

  
  pWork->gamedata = GAMEDATA_Create(GFL_HEAPID_APP);
  pFriend = GAMEDATA_GetMyStatusPlayer(pWork->gamedata, 1);
  GFL_STD_MemCopy(MyStatus_AllocWork(GFL_HEAPID_APP),pFriend,MyStatus_GetWorkSize() );

  pWork->pMy = GAMEDATA_GetMyStatusPlayer(pWork->gamedata, 0);
  pWork->pNPC = pFriend;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  //  GFL_OVERLAY_Load( FS_OVERLAY_ID(app_mail));
  //  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));


  return pWork;
}

static void * _PokeTradeDemo2WorkCreate(D_GPOWER_WORK *wk)
{
  POKEMONTRADE_DEMO_PARAM *pWork;
  MYSTATUS* pFriend;


  pWork = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(POKEMONTRADE_DEMO_PARAM));

  pWork->pMyPoke = PP_Create(MONSNO_162, 100, 123456, GFL_HEAPID_APP);
  pWork->pNPCPoke = PP_Create(MONSNO_510, 100, 123456, GFL_HEAPID_APP);


  pWork->gamedata = GAMEDATA_Create(GFL_HEAPID_APP);
  pFriend = GAMEDATA_GetMyStatusPlayer(pWork->gamedata, 1);
  GFL_STD_MemCopy(MyStatus_AllocWork(GFL_HEAPID_APP),pFriend,MyStatus_GetWorkSize() );

  pWork->pMy = GAMEDATA_GetMyStatusPlayer(pWork->gamedata, 0);
  pWork->pNPC = pFriend;

  //オーバーレイ読み込み
  // GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  //  GFL_OVERLAY_Load( FS_OVERLAY_ID(app_mail));
  //  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));


  return pWork;
}


//----------------------------------------------------------------------------
/**
 *	@brief	フレンドダミーデータ作成
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void _DebugDataCreate(EVENT_GTSNEGO_WORK *pWork)
{
  int i;

  //  for(i = 0; i < WIFI_NEGOTIATION_DATAMAX;i++){
  for(i = 0; i < 30;i++){
    MYSTATUS* pMyStatus = MyStatus_AllocWork(GFL_HEAPID_APP);
    MyStatus_SetProfileID(pMyStatus,1+i);
    MyStatus_SetID(pMyStatus,1+i);
    MyStatus_SetTrainerView(pMyStatus,i%16);
    MyStatus_SetMyNationArea(pMyStatus,1+i,1);
    WIFI_NEGOTIATION_SV_SetFriend(GAMEDATA_GetWifiNegotiation(pWork->gamedata),pMyStatus);
    GFL_HEAP_FreeMemory(pMyStatus);
  }
}



static void * _PokeTradeGtsNegoCreate(D_GPOWER_WORK *wk)
{
  EVENT_GTSNEGO_WORK *pWork;
  MYSTATUS* pFriend;


  pWork = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(EVENT_GTSNEGO_WORK));

  pWork->aUser[0].selectType=0;
  pWork->aUser[1].selectType=0;
  pWork->aUser[0].selectLV=0;
  pWork->aUser[1].selectLV=0;

  pWork->gamedata = GAMEDATA_Create(GFL_HEAPID_APP);
  //  pWork->pStatus[0] = MyStatus_AllocWork(GFL_HEAPID_APP);
  //  pWork->pStatus[1] = MyStatus_AllocWork(GFL_HEAPID_APP);

  pFriend = GAMEDATA_GetMyStatusPlayer(pWork->gamedata, 1);
  GFL_STD_MemCopy(MyStatus_AllocWork(GFL_HEAPID_APP),pFriend,MyStatus_GetWorkSize() );

  _DebugDataCreate(pWork);

  DEBUGWIN_InitProc( GFL_BG_FRAME3_M , NULL );
  DEBUG_PAUSE_SetEnable( TRUE );
  
  return pWork;
}


static void * _PokeTradeWorkCreate(D_GPOWER_WORK *wk)
{
  POKEMONTRADE_PARAM *pWork2;
  EVENT_GTSNEGO_WORK *pWork;
  MYSTATUS* pFriend;

  pWork2 = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(POKEMONTRADE_PARAM));

  pWork2->pNego = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(EVENT_GTSNEGO_WORK));
  pWork = pWork2->pNego;

  pWork->aUser[0].selectType=0;
  pWork->aUser[1].selectType=0;
  pWork->aUser[0].selectLV=0;
  pWork->aUser[1].selectLV=0;

  pWork2->gamedata = GAMEDATA_Create(GFL_HEAPID_APP);
  pWork2->bDebug=TRUE;
  pWork->gamedata = pWork2->gamedata ;
  //  pWork->pStatus[0] = MyStatus_AllocWork(GFL_HEAPID_APP);
  //  pWork->pStatus[1] = MyStatus_AllocWork(GFL_HEAPID_APP);
  pFriend = GAMEDATA_GetMyStatusPlayer(pWork2->gamedata, 1);
  GFL_STD_MemCopy(MyStatus_AllocWork(GFL_HEAPID_APP),pFriend,MyStatus_GetWorkSize() );


  return pWork2;
}

static void * _PokeIrcTradeWorkCreate(D_GPOWER_WORK *wk)
{
  POKEMONTRADE_PARAM *pWork;
  MYSTATUS* pFriend;


  pWork = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(POKEMONTRADE_PARAM));
  pWork->gamedata = GAMEDATA_Create(GFL_HEAPID_APP);

  pFriend = GAMEDATA_GetMyStatusPlayer(pWork->gamedata, 1);
  GFL_STD_MemCopy(MyStatus_AllocWork(GFL_HEAPID_APP),pFriend,MyStatus_GetWorkSize() );


  return pWork;
}

static void * _initTrial(D_GPOWER_WORK *wk)
{
  DEBUG_TRIAL_PARAM* pWork = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(DEBUG_TRIAL_PARAM));
  pWork->trialType = wk->trialType;
  pWork->trialNo = wk->trialNo;
  return pWork;
}

//==============================================================================
//
//==============================================================================
//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA DebugGPowerFuncProcData = {
  DebugOhnoMainProcInit,
  DebugOhnoMainProcMain,
  DebugOhnoMainProcEnd,
};




#endif //PM_DEBUG
