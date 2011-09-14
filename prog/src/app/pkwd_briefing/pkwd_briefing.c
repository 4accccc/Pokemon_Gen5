//==============================================================================
/**
 * @file  pkwd_briefing.c
 * @brief ポケウッドあらすじ画面
 * @author  mori
 * @date  2011.08.01 (title.cから作成）
 *
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/ica_anime.h"
#include "system/ica_camera.h"
#include "sound/pm_voice.h"
#include "print/printsys.h"
#include "print/gf_font.h"
#include "font/font.naix"
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_title.h"
#include "font/font.naix"
#include "title/startmenu.h"
#include "test/performance.h"
#include "app/pkwd_briefing.h"
#include "infowin/infowin.h"
#include "sound/pm_sndsys.h"
#include "poke_tool/monsno_def.h"

#include "demo3d.naix"
#include "title/title.h"


#include "battle/battgra_wb.naix"   // 背景モデリングデータ
#include "pkwd_announce_gra.naix"   // カメラデータ

#include "msg/msg_pw_briefing001.h"    // あらすじメッセージデータ（001～040までアリ）

//==============================================================================
//  定数定義
//==============================================================================
/// あらすじ文字列の1回表示毎のウェイト
#define BRIEFING_MSG_WAIT      (180)

// １画面に表示する文字数( (18文字＋改行)x8 )x2
#define BRIEFING_MSG_LENGTH   ( 19*8*2 )

/// 入力トリガマスク
#define NEXT_PROC_MASK    ( PAD_BUTTON_B )

///フォントが使用するパレット番号
#define U_FONT_PALNO  (0x0)

//情報ステータスバーパレット
#define INFOWIN_PAL_NO  ( 0x0f ) 


///タイトルロゴBGのBGプライオリティ
enum{
  BGPRI_MSG = 1,        ///<
  BGPRI_TITLE_LOGO = 0, ///<
  BGPRI_TITLE_BACK = 3, ///<
  BGPRI_BRIEF1 = 0,       ///<
  BGPRI_BRIEF2 = 1,       ///<
  BGPRI_BKGR = 2,       ///<
  BGPRI_3D = 3,         ///<
};

enum{
  FRAME_BRIEF1  = GFL_BG_FRAME1_M,   ///< メイン画面メッセージ１
  FRAME_BRIEF2  = GFL_BG_FRAME2_M,   ///< メイン画面メッセージ２
  FRAME_BKGR    = GFL_BG_FRAME3_M,   ///< メイン画面背景「Develped by GameFreak」
  FRAME_SEATS   = GFL_BG_FRAME1_S,   ///< 観客席BG
  FRAME_INFOWIN = GFL_BG_FRAME2_S,   ///< 情報ウインドウ面
};



/// シーケンスID定義
enum{
  SEQ_INIT = 0,   ///<
  SEQ_SETUP,      ///<
  SEQ_FADEIN,     ///<
  SEQ_MAIN,       ///<
  SEQ_VOICE_PLAY, ///<
  SEQ_NEXT,       ///<
  SEQ_FADEOUT,    ///<
  SEQ_END,        ///<
  SEQ_SKIP,       ///<
};

enum{
  OBJ_RES_PAL,
  OBJ_RES_CHR,
  OBJ_RES_CEL,
  OBJ_RES_MAX,
};

// ポケウッド観客
enum{
  BTLV_INPUT_AUDIENCE_MAX = 22,
};


#ifdef PM_DEBUG
#endif

//==============================================================================
//  構造体定義
//==============================================================================
typedef struct{
  GFL_TCBLSYS*  tcbl;
  GFL_TCB       *vintr_tcb;
}SYS_CONTROL;

typedef struct{
  GFL_BMPWIN*     bmpwin[2];
  GFL_BMP_DATA*   bmp;
  PRINT_UTIL      printUtil[2];
  BOOL            message_req;    ///<TRUE:メッセージリクエストがあった
  GFL_FONT*       fontHandle;
  PRINT_QUE*      printQue;
  GFL_MSGDATA*    mm;
  STRBUF*         expandbuf[2];
  STRBUF*         endkeyword;    //「なし」を読み込む
  int             scrol_work;

  int             blend_flag;
  int             blend_work;
  int             now_page;
  int             wait;
}G2D_CONTROL;

typedef struct{
  GFL_G3D_UTIL*     g3Dutil;
  u16               g3DutilUnitIdx;
  GFL_G3D_LIGHTSET* g3Dlightset;
  ICA_ANIME* icaAnime;
  GFL_G3D_CAMERA* camera;

}G3D_CONTROL;

typedef struct{
  GFL_CLUNIT*               clunit;
  GFL_CLWK*                 clwk;    //アイテムアイコンアクター
  NNSG2dImagePaletteProxy   icon_pal_proxy;
  NNSG2dCellDataBank*       icon_cell_data;
  NNSG2dAnimBankData*       icon_anm_data;
  void*                     icon_cell_res;
  void*                     icon_anm_res;
  u32                       clres[OBJ_RES_MAX];
}OAM_CONTROL;

//ポケウッド観客
typedef struct{
  u32                   audience_charaID;		// objcharIDを流用
  u32                   audience_plttID;
  u32                   audience_cellID;
  GFL_CLUNIT*           audience_clunit;
  GFL_CLWK*             audience_wk[ BTLV_INPUT_AUDIENCE_MAX ];
  ARCHANDLE                *aHandle;
}AUDIENCE_CONTROL;

typedef struct {
  PKWD_BRIEFING_PARAM *param;
  u16           seq;
  HEAPID        heapID;
  
  SYS_CONTROL   CSys;
  G2D_CONTROL   CG2d;
  G3D_CONTROL   CG3d;
  OAM_CONTROL   COam;
  AUDIENCE_CONTROL AOam;
  
  u32           totalWait;
  int           voiceIndex;
  int           scene_flag;

  u32           bef_vcnt;

  int           blend_seq;
  int           sex;
}PKWD_BRIEFING_WORK;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
GFL_PROC_RESULT PkwdBriefingProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT PkwdBriefingProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT PkwdBriefingProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================
//  外部関数宣言
//==============================================================================

//==============================================================================
//  データ
//==============================================================================
//--------------------------------------------------------------
//  PROC
//--------------------------------------------------------------
const GFL_PROC_DATA PkwdBriefingProcData = {
  PkwdBriefingProcInit,
  PkwdBriefingProcMain,
  PkwdBriefingProcEnd,
};

//==============================================================================
//
//  
//
//==============================================================================
static void initIO(void);
static void initVRAM(void);

static void setupSystem(SYS_CONTROL* CSys, HEAPID heapID);
static void mainSystem(SYS_CONTROL* CSys, HEAPID heapID);
static void releaseSystem(SYS_CONTROL* CSys);

static void setupG2Dcontrol(G2D_CONTROL* CG2d, int scenario_no, HEAPID heapID);
static void mainG2Dcontrol(G2D_CONTROL* CG2d, HEAPID heapID);
static void releaseG2Dcontrol(G2D_CONTROL* CG2d);

static void setupG3Dcontrol(G3D_CONTROL* CG3d, HEAPID heapID, int no);
static void mainG3DUpdate(G3D_CONTROL* CG3d, HEAPID heapID);
static void mainG3Dcontrol(G3D_CONTROL* CG3d, HEAPID heapID, int no);
static void releaseG3Dcontrol(G3D_CONTROL* CG3d);

static void setupOAMcontrol(OAM_CONTROL* COam, HEAPID heapID);
static void mainOAMcontrol(OAM_CONTROL* COam, HEAPID heapID);
static void releaseOAMcontrol(OAM_CONTROL* COam);

static void MainFunc(PKWD_BRIEFING_WORK* tw);
static void VintrFunc(GFL_TCB *tcb, void *work);
static void BriefingFunc( PKWD_BRIEFING_WORK *tw );

static void initLcdWindow( void );
static void _briefing_print( G2D_CONTROL *CG2d, int page, int sex );
static BOOL _next_page_check( PKWD_BRIEFING_WORK *tw );

static void btlv_init_audience_disp( PKWD_BRIEFING_WORK *pbw, HEAPID heapID );
static  void  BTLV_INPUT_DeleteAudienceObj( AUDIENCE_CONTROL *AOam );


FS_EXTERN_OVERLAY( battle_view );

//--------------------------------------------------------------
/**
 * @brief   タイトル画面初期化
 *
 * @param   proc  
 * @param   seq   
 * @param   pwk   
 * @param   mywk  タイトル画面ワーク（ここで確保）
 *
 * @retval  
 */
//--------------------------------------------------------------
GFL_PROC_RESULT PkwdBriefingProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PKWD_BRIEFING_WORK *tw;
  PKWD_BRIEFING_PARAM *param = (PKWD_BRIEFING_PARAM *)pwk;
  
  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PKWD_BRIEFING, 0x120000 );
  tw = GFL_PROC_AllocWork( proc, sizeof(PKWD_BRIEFING_WORK), HEAPID_PKWD_BRIEFING );
  GFL_STD_MemClear(tw, sizeof(PKWD_BRIEFING_WORK));
  tw->heapID = HEAPID_PKWD_BRIEFING;
  tw->param  = param;

  tw->sex    = MyStatus_GetMySex(tw->param->mystatus);

  GFL_OVERLAY_Load( FS_OVERLAY_ID(battle_view));

  return GFL_PROC_RES_FINISH;
}


//----------------------------------------------------------------------------------
/**
 * @brief キー入力
 *
 * @param   tw    PKWD_BRIEFING_WORK
 */
//----------------------------------------------------------------------------------
static void _key_check( PKWD_BRIEFING_WORK *tw )
{
  // ゲーム開始
  if(  GFL_UI_KEY_GetTrg() & NEXT_PROC_MASK){
    tw->seq  = SEQ_VOICE_PLAY;
    GFL_BG_SetVisible(FRAME_INFOWIN, VISIBLE_ON);
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF );
//    PMSND_FadeOutBGM( 4 );

    return;
  }

}

#ifdef PM_DEBUG

static u32 oldVCount;
static u32 delayVCount;
static u32 delayVCountTotal;

static const u16 pal_tbl[2]={
  0x0000, 0xffff
};

static void _vcount_func(void)
{
  { //処理落ちを検出
    u32 nowVCount = OS_GetVBlankCount();
    u32 subVCount = nowVCount - oldVCount;
  
    delayVCount = 0;
    if( oldVCount > 0 && subVCount > 1 )
    {
      // 処理落ちした
      delayVCount = (subVCount-1);
      GXS_LoadBGPltt( &pal_tbl[0], 0, 2);
    }else{
      // してない
      GXS_LoadBGPltt( &pal_tbl[1], 0, 2);
    }
    oldVCount = nowVCount;
    delayVCountTotal += delayVCount;
  }

}
#endif

#ifdef PM_DEBUG
static int d_flag=0;
static int d_blend1=0;
static int d_blend2=0;

static void _debug_blend(void)
{
  if(GFL_UI_KEY_GetRepeat()&PAD_KEY_UP){
    if(d_blend1<31) d_blend1++;
    OS_Printf("flg=%d, blend1=%d, blend2=%d\n", d_flag, d_blend1, d_blend2);
  }else if(GFL_UI_KEY_GetRepeat()&PAD_KEY_DOWN){
    if(d_blend1>0) d_blend1--;
    OS_Printf("flg=%d, blend1=%d, blend2=%d\n", d_flag, d_blend1, d_blend2);
  }else if(GFL_UI_KEY_GetRepeat()&PAD_KEY_LEFT){
    if(d_blend2>0) d_blend2--;
    OS_Printf("flg=%d, blend1=%d, blend2=%d\n", d_flag, d_blend1, d_blend2);
  }else if(GFL_UI_KEY_GetRepeat()&PAD_KEY_RIGHT){
    if(d_blend2<31) d_blend2++;
    OS_Printf("flg=%d, blend1=%d, blend2=%d\n", d_flag, d_blend1, d_blend2);
  }else if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_X){
    d_flag++;
    if(d_flag>2) d_flag=0;
    OS_Printf("flg=%d, blend1=%d, blend2=%d\n", d_flag, d_blend1, d_blend2);
  }

  if(d_flag==0){
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,GX_BLEND_PLANEMASK_BG0,  d_blend1, d_blend2 );
  }else if(d_flag==1){
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2,GX_BLEND_PLANEMASK_BG0,  d_blend1, d_blend2 );
  }else if(d_flag==2){
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG0, d_blend1, d_blend2 );
  }
}
#endif


//=============================================================================================
/**
 * @brief タイトル画面メイン
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk  タイトル画面ワーク
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT PkwdBriefingProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PKWD_BRIEFING_WORK* tw = mywk;
  u8 test_buff;

  switch(tw->seq){
  case SEQ_INIT:
    // 初期化
    initIO();
    initVRAM();
    initLcdWindow();

    tw->seq = SEQ_SETUP;
    break;

  case SEQ_SETUP:
    // セットアップ
    setupSystem(&tw->CSys, tw->heapID);
    setupG2Dcontrol(&tw->CG2d, tw->param->scenario_no, tw->heapID);
    setupG3Dcontrol(&tw->CG3d, tw->heapID, tw->param->scenario_no);
    setupOAMcontrol(&tw->COam, tw->heapID);
    btlv_init_audience_disp( tw, tw->heapID );
    // 1ページ目の文字列を描画
    _briefing_print( &tw->CG2d, 0, tw->sex );
    //情報ステータスバー初期化
    INFOWIN_Init( FRAME_INFOWIN, INFOWIN_PAL_NO, tw->param->gameData , tw->heapID );
    GFL_NET_ReloadIconTopOrBottom( FALSE, tw->heapID );

    // FADEIN設定
    GFL_FADE_SetMasterBrightReq
      (GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 2);
    tw->totalWait = 0;
    tw->seq = SEQ_FADEIN;
//    PMSND_PlayBGM_WideChannel( SEQ_BGM_POKEMON_THEME );
    //SND_STRM_Play( STRM_TITLE );

    tw->bef_vcnt  = OS_GetVBlankCount();
    break;

  case SEQ_FADEIN:
    if(GFL_FADE_CheckFade() == FALSE){ tw->seq = SEQ_MAIN; }
    MainFunc(tw);
    break;

  case SEQ_MAIN:

    MainFunc(tw);
    // キーチェック
    _key_check(tw);

    // あらすじ表示管理
    BriefingFunc( tw );

#ifdef PM_DEBUG
//    _debug_blend();
#endif

    PRINTSYS_QUE_Main( tw->CG2d.printQue );
    PRINT_UTIL_Trans( &tw->CG2d.printUtil[0], tw->CG2d.printQue );
    PRINT_UTIL_Trans( &tw->CG2d.printUtil[1], tw->CG2d.printQue );
    break;
  case SEQ_VOICE_PLAY:
    if(PMVOICE_CheckPlay(tw->voiceIndex)==FALSE){
      tw->seq  = SEQ_NEXT;
    }
    MainFunc(tw);
    break;
  case SEQ_NEXT:
    // FADEOUT設定
    GFL_FADE_SetMasterBrightReq
      (GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 2);
    tw->seq = SEQ_FADEOUT;
    MainFunc(tw);
//    PMSND_FadeOutBGM( 16 );
    break;

  case SEQ_FADEOUT:
    if( GFL_FADE_CheckFade() == FALSE && 
        PMSND_CheckFadeOnBGM() == FALSE ){
//      PMSND_StopBGM();
      tw->seq = SEQ_END;
    }
    MainFunc(tw);
    break;

  case SEQ_END:
    // 解放
    INFOWIN_Exit();
    BTLV_INPUT_DeleteAudienceObj( &tw->AOam );
    releaseOAMcontrol(&tw->COam);
    releaseG3Dcontrol(&tw->CG3d);
    releaseG2Dcontrol(&tw->CG2d);
    releaseSystem(&tw->CSys);

    return GFL_PROC_RES_FINISH;

  case SEQ_SKIP:
    return GFL_PROC_RES_FINISH;
  }

  
  return GFL_PROC_RES_CONTINUE;

}


//=============================================================================================
/**
 * @brief タイトル画面終了
 *
 * @param   proc  
 * @param   seq   
 * @param   pwk   
 * @param   mywk  タイトル画面ワーク
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT PkwdBriefingProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PKWD_BRIEFING_WORK * tw = mywk;

  GFL_PROC_FreeWork(proc);
  GFL_HEAP_DeleteHeap(HEAPID_PKWD_BRIEFING);

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(battle_view));


  return GFL_PROC_RES_FINISH;
}

//==============================================================================
//  
//  Init
//
//==============================================================================
//----------------------------------------------------------------------------------
/**
 * @brief 画面初期化
 *
 * @param   none    
 */
//----------------------------------------------------------------------------------
static void initIO(void)
{
  // 各種効果レジスタ初期化
  GX_SetMasterBrightness(-16);
  GXS_SetMasterBrightness(-16);
  GFL_DISP_GX_SetVisibleControlDirect(0);   //全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);
  G2_BlendNone();
  G2S_BlendNone();
  GX_SetVisibleWnd(GX_WNDMASK_NONE);
  GXS_SetVisibleWnd(GX_WNDMASK_NONE);
}

//==============================================================================
//  VRAM
//==============================================================================
static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_128_D,               // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,         // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,           // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,     // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_NONE,               // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,        // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_16_I,           // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,    // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_01_AB,              // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_0123_E,         // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_64K,     // メインOBJマッピングモード
  GX_OBJVRAMMODE_CHAR_1D_32K,     // サブOBJマッピングモード
};

//----------------------------------------------------------------------------------
/**
 * @brief VRAM 設定
 *
 * @param   none    
 */
//----------------------------------------------------------------------------------
static void initVRAM(void)
{
  static const GFL_BG_SYS_HEADER sysHeader = {
    GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
  };
  //VRAM設定
  GFL_DISP_SetBank( &vramBank );
  GFL_BG_SetBGMode( &sysHeader );
}

//==============================================================================
//  
//  Function
//
//==============================================================================
//----------------------------------------------------------------------------------
/**
 * @brief 常駐処理
 *
 * @param   tw    
 */
//----------------------------------------------------------------------------------
static void MainFunc(PKWD_BRIEFING_WORK* tw)
{
  //処理落ち対策
  const u32 now_vcnt = OS_GetVBlankCount();
  u32 delay_vcnt  = now_vcnt - tw->bef_vcnt;
  tw->bef_vcnt  = now_vcnt;

  mainSystem(&tw->CSys, tw->heapID);

  //処理落ちした場合は１シンク進める
  do
  {
    mainG3DUpdate(&tw->CG3d, tw->heapID);
  }
  while( --delay_vcnt > 0 );

  mainG3Dcontrol(&tw->CG3d, tw->heapID, tw->param->scenario_no);
  mainOAMcontrol(&tw->COam, tw->heapID);
  INFOWIN_Update();
}


//----------------------------------------------------------------------------------
/**
 * @brief VBLANK処理
 *
 * @param   tcb   
 * @param   work    
 */
//----------------------------------------------------------------------------------
static void VintrFunc(GFL_TCB *tcb, void *work)
{
  SYS_CONTROL* CSys = work;
  
  GFL_CLACT_SYS_VBlankFunc();

  // BG転送  
  GFL_BG_VBlankFunc();

}

//==============================================================================
//  
//  Setup & Release
//
//==============================================================================
//==============================================================================
//  System
//==============================================================================
#define DTCM_SIZE   (0x1000)

//------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 * @brief   基本システム初期化
 *
 * @param   CSys    SYS_CONTROL
 * @param   heapID  
 */
//----------------------------------------------------------------------------------
static void setupSystem(SYS_CONTROL* CSys, HEAPID heapID)
{
  GFL_BG_Init( heapID );
  GFL_BMPWIN_Init( heapID );
  GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K,
                GFL_G3D_VMANLNK, GFL_G3D_PLT32K,
                DTCM_SIZE, heapID, NULL );
  GFL_FONTSYS_Init();   // 初期化のみリリースなし

  CSys->tcbl = GFL_TCBL_Init( heapID, heapID, 4, 32 );
  CSys->vintr_tcb = GFUser_VIntr_CreateTCB(VintrFunc, CSys, 5);

  // 上下画面設定
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
  GFL_DISP_SetDispOn();

}

//------------------------------------------------------------------------------
static void mainSystem(SYS_CONTROL* CSys, HEAPID heapID)
{
  GFL_TCBL_Main( CSys->tcbl );
}

//------------------------------------------------------------------------------
static void releaseSystem(SYS_CONTROL* CSys)
{
  // 上下画面設定
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
  GFL_DISP_SetDispOn();

  GFL_TCB_DeleteTask(CSys->vintr_tcb);
  GFL_TCBL_Exit(CSys->tcbl);

  GFL_G3D_Exit();
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}


// あらすじBMPWIN座標・幅定義
#define BRIEF_BMPWIN_X    (  2 )
#define BRIEF_BMPWIN_Y    (  4 )
#define BRIEF_BMPWIN_W    ( 28 )
#define BRIEF_BMPWIN_H    ( 16 )

// BMPWINクリアカラーコード
#define BRIEF_CLEAR_COLOR ( 0xee )

//==============================================================================
//  G2DControl
//==============================================================================

//--------------------------------------------------------------
static void setupG2Dcontrol(G2D_CONTROL* CG2d, int scenario_no, HEAPID heapID)
{
  //BGフレーム設定
  {
    static const GFL_BG_BGCNT_HEADER main_brief1_frame = {  //メイン画面BGフレーム1
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x04000,
      GX_BG_EXTPLTT_01, BGPRI_BRIEF1, 0, 0, FALSE
    };
    static const GFL_BG_BGCNT_HEADER main_brief2_frame = {  //メイン画面BGフレーム2
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, 0x04000,
      GX_BG_EXTPLTT_01, BGPRI_BRIEF2, 0, 0, FALSE
    };
    static const GFL_BG_BGCNT_HEADER main_bkgr_frame = {  //メイン画面BGフレーム3
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, 0x04000,
      GX_BG_EXTPLTT_01, BGPRI_BKGR, 0, 0, FALSE
    };
    
    
    static const GFL_BG_BGCNT_HEADER sub_msg_frame = {  //サブ画面BGフレーム1
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x04000,
      GX_BG_EXTPLTT_01, BGPRI_MSG, 0, 0, FALSE
    };
    static const GFL_BG_BGCNT_HEADER sub_logo_frame = { //サブ画面BGフレーム2
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x04000, 0x4000,
      GX_BG_EXTPLTT_01, BGPRI_TITLE_LOGO, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( FRAME_BRIEF1,  &main_brief1_frame, GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( FRAME_BRIEF2,  &main_brief2_frame, GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( FRAME_BKGR,    &main_bkgr_frame,   GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( FRAME_SEATS,     &sub_msg_frame,     GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( FRAME_INFOWIN, &sub_logo_frame,    GFL_BG_MODE_TEXT );
  }
  //メイン画面背景設定
  {
    //GFL_BG_SetBackGroundColor(GFL_BG_FRAME0_M, GX_RGB(25, 31, 31));
    GFL_ARC_UTIL_TransVramBgCharacter(
      ARCID_PKWD_ANNOUNCE, NARC_pkwd_announce_gra_pkwd_brief_NCGR, FRAME_BRIEF1, 0, 0, FALSE, heapID);
    GFL_ARC_UTIL_TransVramScreen(
      ARCID_PKWD_ANNOUNCE, NARC_pkwd_announce_gra_pkwd_brief_NSCR, FRAME_BRIEF1, 0, 0, FALSE, heapID);
    GFL_ARC_UTIL_TransVramBgCharacter(
      ARCID_PKWD_ANNOUNCE, NARC_pkwd_announce_gra_pkwd_brief_NCGR, FRAME_BRIEF2, 0, 0, FALSE, heapID);
    GFL_ARC_UTIL_TransVramScreen(
      ARCID_PKWD_ANNOUNCE, NARC_pkwd_announce_gra_pkwd_brief_NSCR, FRAME_BRIEF2, 0, 0, FALSE, heapID);

    GFL_ARC_UTIL_TransVramPalette(
      ARCID_PKWD_ANNOUNCE, NARC_pkwd_announce_gra_pkwd_brief_NCLR, PALTYPE_MAIN_BG, 0, 0, heapID);
      

    GFL_BG_LoadScreenReq( FRAME_BKGR );
    GFL_BG_SetVisible( FRAME_BKGR, VISIBLE_OFF );
    GFL_BG_SetVisible( FRAME_BRIEF1, VISIBLE_OFF );
    GFL_BG_SetVisible( FRAME_BRIEF2, VISIBLE_OFF );
  }
  {
    int i;
//    GFL_BG_FillCharacter( FRAME_SEATS, 0x00, 1, 0 );
//    GFL_BG_FillScreen( FRAME_SEATS, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

    //BMPWIN作成
    CG2d->bmpwin[0] = GFL_BMPWIN_Create( FRAME_BRIEF1, BRIEF_BMPWIN_X, BRIEF_BMPWIN_Y, 
                                                       BRIEF_BMPWIN_W, BRIEF_BMPWIN_H, 
                                                       U_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
    CG2d->bmpwin[1] = GFL_BMPWIN_Create( FRAME_BRIEF2, BRIEF_BMPWIN_X, BRIEF_BMPWIN_Y, 
                                                       BRIEF_BMPWIN_W, BRIEF_BMPWIN_H, 
                                                       U_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
    
    for(i=0;i<2;i++){
      PRINT_UTIL_Setup( &CG2d->printUtil[i], CG2d->bmpwin[i] );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( CG2d->bmpwin[i] ), BRIEF_CLEAR_COLOR );
      GFL_BMPWIN_MakeScreen( CG2d->bmpwin[i] );
      GFL_BMPWIN_TransVramCharacter( CG2d->bmpwin[i] );
    }
    GFL_BG_LoadScreenReq( FRAME_BRIEF1 );
    GFL_BG_LoadScreenReq( FRAME_BRIEF2 );

    //フォントパレット転送(緑字)
    // メッセージフォント用パレット転送
//    GFL_ARC_UTIL_TransVramPalette( ARCID_PKWD_ANNOUNCE, NARC_pkwd_announce_gra_pkwd_brief_nclr, 
//                                   PALTYPE_MAIN_BG, 0, 0x20, heapID );

    //フォント&プリントキューシステム作成
    CG2d->fontHandle = GFL_FONT_Create
      ( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
    CG2d->printQue = PRINTSYS_QUE_Create( heapID );

    //メッセージ取得
    CG2d->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                               NARC_message_pw_briefing001_dat+scenario_no, heapID );
    CG2d->expandbuf[0] = GFL_STR_CreateBuffer( BRIEFING_MSG_LENGTH, heapID );
    CG2d->expandbuf[1] = GFL_STR_CreateBuffer( BRIEFING_MSG_LENGTH, heapID );
    CG2d->endkeyword   = GFL_STR_CreateBuffer( BRIEFING_MSG_LENGTH, heapID );
    GFL_MSG_GetString(CG2d->mm, pw_briefing_end_keyword, CG2d->endkeyword);

    CG2d->message_req = TRUE;
  }
}


static void releaseG2Dcontrol(G2D_CONTROL* CG2d)
{
  GFL_STR_DeleteBuffer(CG2d->endkeyword);
  GFL_STR_DeleteBuffer(CG2d->expandbuf[1]);
  GFL_STR_DeleteBuffer(CG2d->expandbuf[0]);
  GFL_MSG_Delete(CG2d->mm);
  PRINTSYS_QUE_Delete(CG2d->printQue);
  GFL_FONT_Delete(CG2d->fontHandle);
  GFL_BMPWIN_Delete(CG2d->bmpwin[1]);
  GFL_BMPWIN_Delete(CG2d->bmpwin[0]);
}

//==============================================================================
//  G3DControl
//==============================================================================
//ライト初期設定データ
static const GFL_G3D_LIGHT_DATA light0Tbl[] = {
  { 0, {{ (FX16_ONE-1), -(FX16_ONE-1)/2, -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 1, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 2, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
  { 3, {{ -(FX16_ONE-1), -(FX16_ONE-1), -(FX16_ONE-1) }, GX_RGB(31,31,31) } },
};
static const GFL_G3D_LIGHTSET_SETUP light0Setup = { light0Tbl, NELEMS(light0Tbl) };

static const GFL_G3D_OBJSTATUS drawStatus = {
  { 0, 0, 0 },                                        //座標
  { FX32_ONE, FX32_ONE, FX32_ONE },                   //スケール
  { FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE }, //回転
};


enum{
  G3DRES_BCTA_BMD = 0,
  G3DRES_BCTA_BCA,
  G3DRES_BCTA_BTA,
  G3DRES_BCTA_BMA,

};

// ポケウッド０１～ポケウッド１０を読み込むための実データテーブル
#include "pkwd_briefing_model.cdat"


static const VecFx32 sc_CAMERA_PER_POS    = { 0,0,FX32_CONST( 0 ) };  //位置
static const VecFx32 sc_CAMERA_PER_UP     = { 0,FX32_ONE,0 };         //上方向
static const VecFx32 sc_CAMERA_PER_TARGET = { 0,0,FX32_CONST( 0 ) };  //ターゲット

static const VecFx32 cam_pos	= { FX_F32_TO_FX32( 6.7f ), FX_F32_TO_FX32( 6.7f ), FX_F32_TO_FX32( 12.3f ) };
static const VecFx32 cam_target = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 2.6f ), FX_F32_TO_FX32( 0.0f ) };
static const VecFx32 cam_up		= { 0,						FX32_ONE,				0 };

//--------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 * @brief 3D初期化
 *
 * @param   CG3d    
 * @param   heapID    
 */
//----------------------------------------------------------------------------------
static void setupG3Dcontrol(G3D_CONTROL* CG3d, HEAPID heapID, int no)
{
  u16 objIdx;
  
  CG3d->g3Dutil = GFL_G3D_UTIL_Create( resource_count_tbl[no][0], resource_count_tbl[no][1], heapID );
  CG3d->g3DutilUnitIdx = GFL_G3D_UTIL_AddUnit( CG3d->g3Dutil, &g3Dutil_setup[no] );

  //アニメーションを有効にする
  objIdx = GFL_G3D_UTIL_GetUnitObjIdx( CG3d->g3Dutil, CG3d->g3DutilUnitIdx );
  GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle(CG3d->g3Dutil, objIdx + 0), 0); // 0:ica
//  GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle(CG3d->g3Dutil, objIdx + 0), 1); // 0:itp
//  GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle(CG3d->g3Dutil, objIdx + 1), 0); // 1:ica
//  GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle(CG3d->g3Dutil, objIdx + 2), 0); // 1:ica


  // カメラ作成
  {
	CG3d->camera = GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS, 
										 FX32_SIN13,
										 FX32_COS13,
										 FX_F32_TO_FX32( 1.35f ),
										 NULL,
										 FX32_ONE,
										 //FX32_ONE * 112,
										 FX32_ONE * 512,
										 NULL,
										 &cam_pos,
										 &cam_up,
										 &cam_target,
										 heapID );
  
  }
  // カメラicaデータをロード
  CG3d->icaAnime = ICA_ANIME_CreateAlloc(
      heapID, ARCID_PKWD_ANNOUNCE, NARC_pkwd_announce_gra_pkwd_field_camera_bin );


  //ライトセット
  CG3d->g3Dlightset = GFL_G3D_LIGHT_Create( &light0Setup, heapID );
  GFL_G3D_LIGHT_Switching(CG3d->g3Dlightset);

  GFL_BG_SetBGControl3D(BGPRI_3D);

  G3X_AntiAlias( TRUE );      //  アンチエイリアス
  G3X_AlphaTest( FALSE, 16 ); //  アルファテスト　　オフ
  G3X_AlphaBlend( TRUE );     //  アルファブレンド　オン


}

//----------------------------------------------------------------------------------
/**
 * @brief 3D更新メイン
 *
 * @param   CG3d    
 * @param   heapID    
 */
//----------------------------------------------------------------------------------
static void mainG3DUpdate(G3D_CONTROL* CG3d, HEAPID heapID)
{
  // ICAカメラ更新
  ICA_ANIME_IncAnimeFrameNoLoop( CG3d->icaAnime, FX32_ONE );
}

//----------------------------------------------------------------------------------
/**
 * @brief 3D描画メイン
 *
 * @param   CG3d    
 * @param   heapID    
 */
//----------------------------------------------------------------------------------
static void mainG3Dcontrol(G3D_CONTROL* CG3d, HEAPID heapID, int no)
{
  GFL_G3D_OBJ*  g3Dobj;
  u16           objIdx;

  objIdx = GFL_G3D_UTIL_GetUnitObjIdx( CG3d->g3Dutil, CG3d->g3DutilUnitIdx );

  // ICAカメラ座標を設定
  ICA_CAMERA_SetCameraStatus( CG3d->camera, CG3d->icaAnime );

  // カメラ切り替え
  GFL_G3D_CAMERA_Switching( CG3d->camera );


  // 描画
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();
  {
    int i;
    
    // モデリング描画
    for(i=0; i<resource_count_tbl[no][1]; i++){
      g3Dobj = GFL_G3D_UTIL_GetObjHandle( CG3d->g3Dutil, objIdx + i );
      GFL_G3D_DRAW_DrawObject( g3Dobj, &drawStatus);  // 全部デフォ位置でOK
    }
  }
  GFL_G3D_DRAW_End();

  // アニメフレーム進める
  { // ica1
    int i;
    for(i=0;i<resource_count_tbl[no][1]; i++){
      g3Dobj = GFL_G3D_UTIL_GetObjHandle( CG3d->g3Dutil, objIdx + i );
      GFL_G3D_OBJECT_LoopAnimeFrame( g3Dobj, 0, FX32_ONE );
    }
  }
}



//----------------------------------------------------------------------------------
/**
 * @brief 3D関連解放
 *
 * @param   CG3d    
 */
//----------------------------------------------------------------------------------
static void releaseG3Dcontrol(G3D_CONTROL* CG3d)
{
  // カメラICA解放
  ICA_ANIME_Delete( CG3d->icaAnime );

  // カメラ破棄
  GFL_G3D_CAMERA_Delete( CG3d->camera );
  GFL_G3D_LIGHT_Delete( CG3d->g3Dlightset );
  GFL_G3D_UTIL_DelUnit( CG3d->g3Dutil, CG3d->g3DutilUnitIdx );
  GFL_G3D_UTIL_Delete( CG3d->g3Dutil );
}




//==============================================================================
//  OAMControl
//==============================================================================
///アクター最大数
#define ACT_MAX     (64)

//--------------------------------------------------------------
static void setupOAMcontrol(OAM_CONTROL* COam, HEAPID heapID)
{
  GFL_CLACT_SYS_Create(&GFL_CLSYSINIT_DEF_DIVSCREEN, &vramBank, heapID);
  
  COam->clunit = GFL_CLACT_UNIT_Create(ACT_MAX, 0, heapID);
  GFL_CLACT_UNIT_SetDefaultRend(COam->clunit);

  //OBJ表示ON
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_OFF );

}

static void mainOAMcontrol(OAM_CONTROL* COam, HEAPID heapID)
{
  GFL_CLACT_SYS_Main();
}

static void releaseOAMcontrol(OAM_CONTROL* COam)
{

 GFL_CLACT_UNIT_Delete(COam->clunit);
 GFL_CLACT_SYS_Delete();
}



enum{
  BLEND_SEQ_START=0,
  BLEND_SEQ_BLACK_BACK_WAIT,
  BLEND_SEQ_BLACK_BACK_IN,
  BLEND_SEQ_FADEIN_START_WAIT,
  BLEND_SEQ_FADEIN_START,
  BLEND_SEQ_FADEIN,
  BLEND_SEQ_WAIT,
  BLEND_SEQ_FADEOUT,
  BLEND_SEQ_END,
};


#define SCREEN_MOVE_CALC ((16-tw->CG2d.blend_work)/2)
#define SCREEN_MOVE_CALC2 (-(16-tw->CG2d.blend_work)/2)

//----------------------------------------------------------------------------------
/**
 * @brief	あらすじ表示半透明処理
 *
 * @param   tw		
 */
//----------------------------------------------------------------------------------
static void BriefingFunc( PKWD_BRIEFING_WORK *tw )
{
  switch(tw->blend_seq){
  // 開始
  case BLEND_SEQ_START:
    if((tw->CG2d.now_page&1)==0){
      GFL_BG_SetVisible( FRAME_BRIEF1, VISIBLE_ON );
      GFL_BG_SetVisible( FRAME_BRIEF2, VISIBLE_OFF );
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,GX_BLEND_PLANEMASK_BG0,  0, 16 );
    }else{
      GFL_BG_SetVisible( FRAME_BRIEF1, VISIBLE_OFF );
      GFL_BG_SetVisible( FRAME_BRIEF2, VISIBLE_ON );
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2,GX_BLEND_PLANEMASK_BG0,  0, 16 );
    }
    tw->blend_seq = BLEND_SEQ_BLACK_BACK_WAIT;
    break;
  // 背景を暗くするまえにウェイト
  case BLEND_SEQ_BLACK_BACK_WAIT:
    tw->CG2d.wait++;
    if(tw->CG2d.wait==60){
      tw->blend_seq = BLEND_SEQ_BLACK_BACK_IN;
    }
    break;
  // 背景を少し暗くする
  case BLEND_SEQ_BLACK_BACK_IN:
    tw->CG2d.blend_work++;
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,GX_BLEND_PLANEMASK_BG0,  0, 16-tw->CG2d.blend_work );
    if(tw->CG2d.blend_work==8){
      tw->blend_seq = BLEND_SEQ_FADEIN_START_WAIT;
      tw->CG2d.blend_work=0;
      tw->CG2d.wait = 0;
    }
    break;
  // 文字列表示まで少々待ち
  case BLEND_SEQ_FADEIN_START_WAIT:
    if(tw->CG2d.wait++>30){
      tw->blend_seq = BLEND_SEQ_FADEIN_START;
    }
    break;
  // 文字列フェード開始
  case BLEND_SEQ_FADEIN_START:
    if((tw->CG2d.now_page&1)==0){
      GFL_BG_SetVisible( FRAME_BRIEF1, VISIBLE_ON );
      GFL_BG_SetVisible( FRAME_BRIEF2, VISIBLE_OFF );
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,GX_BLEND_PLANEMASK_BG0,  0, 8 );
    }else{
      GFL_BG_SetVisible( FRAME_BRIEF1, VISIBLE_OFF );
      GFL_BG_SetVisible( FRAME_BRIEF2, VISIBLE_ON );
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2,GX_BLEND_PLANEMASK_BG0,  0, 8 );
    }
    tw->blend_seq = BLEND_SEQ_FADEIN;
    break;
  // 文字列フェード
  case BLEND_SEQ_FADEIN:
    tw->CG2d.blend_work++;
    if((tw->CG2d.now_page&1)==0){
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,GX_BLEND_PLANEMASK_BG0,  tw->CG2d.blend_work, 8 );
      G2_SetBG1Offset( SCREEN_MOVE_CALC, 0 );
    }else{
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2,GX_BLEND_PLANEMASK_BG0,  tw->CG2d.blend_work, 8 );
      G2_SetBG2Offset( SCREEN_MOVE_CALC, 0 );
    }
    if(tw->CG2d.blend_work==16){
      tw->blend_seq = BLEND_SEQ_WAIT;
    }
    break;
  // 文字列表示したまま２秒程度待ち
  case BLEND_SEQ_WAIT:
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_A || ++tw->CG2d.wait>BRIEFING_MSG_WAIT){
      tw->blend_seq = BLEND_SEQ_FADEOUT;
      tw->CG2d.wait=0;
    }
    break;
  // 文字列フェードアウト
  case BLEND_SEQ_FADEOUT:
    tw->CG2d.blend_work--;
    if((tw->CG2d.now_page&1)==0){
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,GX_BLEND_PLANEMASK_BG0,  tw->CG2d.blend_work, 8 );
      G2_SetBG1Offset( SCREEN_MOVE_CALC2, 0 );
    }else{
      G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2,GX_BLEND_PLANEMASK_BG0,  tw->CG2d.blend_work, 8 );
      G2_SetBG2Offset( SCREEN_MOVE_CALC2, 0 );
    }
    if(tw->CG2d.blend_work==0){
      tw->blend_seq = BLEND_SEQ_END;
    }
    break;
  // 次の文字列を準備するか終了するか判定
  case BLEND_SEQ_END:
    tw->CG2d.now_page++;
    if(_next_page_check( tw )){
      tw->seq  = SEQ_VOICE_PLAY;                // 終了
    }else{
      tw->blend_seq = BLEND_SEQ_FADEIN_START;   // 次のページ表示へ
      _briefing_print( &tw->CG2d, tw->CG2d.now_page, tw->sex );
    }
    break;
  }
}


  // ポケウッド用
enum{
  POKEWOOD_WIN0_TOP_Y       =   16,
  POKEWOOD_WIN0_BOTTOM_Y    =  176,
};
//----------------------------------------------------------------------------------
/**
 * @brief   ハードウェアウインドウ設定
 *
 * @param   none		
 */
//----------------------------------------------------------------------------------
static void initLcdWindow( void )
{
    // 上画面の上下をマスクする
    G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 |
                           GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 |
                           GX_WND_PLANEMASK_OBJ, TRUE );
    G2_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE , FALSE );
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
    G2_SetWnd0Position( 0, POKEWOOD_WIN0_TOP_Y, 255, POKEWOOD_WIN0_BOTTOM_Y);
}


//----------------------------------------------------------------------------------
/**
 * @brief	ページの文字列を読み込み指定のBG面にあらすじ文字列を描画する
 *
 * @param   CG2d		
 * @param   page		表示するページ
 */
//----------------------------------------------------------------------------------
static void _briefing_print( G2D_CONTROL* CG2d, int page, int sex )
{
  int linage;
  int index = page%2;

  if(sex==PM_MALE){
    GFL_MSG_GetString(CG2d->mm, pw_briefing_001+page, CG2d->expandbuf[index]);
  }else{
    GFL_MSG_GetString(CG2d->mm, pw_briefing_f_001+page, CG2d->expandbuf[index]);
  }
  linage =   PRINTSYS_GetLineCount( CG2d->expandbuf[index] );

  //メッセージ表示
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( CG2d->bmpwin[index] ), BRIEF_CLEAR_COLOR );
  PRINT_UTIL_PrintColor( &CG2d->printUtil[index], CG2d->printQue, 0, 
                        (BRIEF_BMPWIN_H*8-linage*16)/2, CG2d->expandbuf[index], CG2d->fontHandle,
                        PRINTSYS_MACRO_LSB(1,2,14) );

}


//----------------------------------------------------------------------------------
/**
 * @brief	次のページの文字列を読み込む
 *
 * @param   tw		
 */
//----------------------------------------------------------------------------------
static BOOL _next_page_check( PKWD_BRIEFING_WORK *tw )
{
  int next  = tw->CG2d.now_page;
  int index = next%2;

  if(tw->sex==PM_MALE){
    GFL_MSG_GetString(tw->CG2d.mm, pw_briefing_001+next, tw->CG2d.expandbuf[index]);
  }else{
    GFL_MSG_GetString(tw->CG2d.mm, pw_briefing_f_001+next, tw->CG2d.expandbuf[index]);
  }
  return GFL_STR_CompareBuffer(tw->CG2d.expandbuf[index], tw->CG2d.endkeyword);
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// 下画面観客
//-------------------------------------------------------------------------------------------------

static void SetAudienceOnOff( AUDIENCE_CONTROL *AOam, int no, BOOL flag );
static void SetAudienceCondition( AUDIENCE_CONTROL *AOam, const s32 no, const s32 mode );

//--------------------------------------------------------------
/**
 * @brief   ポケウッド画面観客OBJ表示
 */
//--------------------------------------------------------------
// 観客の情報
// (x,y,pal,0)
static const u8 audience_info[][4]={
  { 32,  104, 3, 0, },
  { 80,  104, 3, 0, },
  { 128, 104, 3, 0, },
  { 176, 104, 3, 0, },
  { 224, 104, 3, 0, },
  {   8, 136, 2, 0, },
  {  56, 136, 2, 0, },
  { 104, 136, 2, 0, },
  { 152, 136, 2, 0, },
  { 200, 136, 2, 0, }, // 10
  { 248, 136, 2, 0, },
  { 32,  168, 1, 0, },
  { 80,  168, 1, 0, },
  { 128, 168, 1, 0, },
  { 176, 168, 1, 0, },
  { 224, 168, 1, 0, },
  {   8, 200, 0, 0, },
  {  56, 200, 0, 0, },
  { 104, 200, 0, 0, },
  { 152, 200, 0, 0, }, // 20
  { 200, 200, 0, 0, },
  { 248, 200, 0, 0, },
};

// 基本情報
static const GFL_CLWK_DATA audience_param[] =
{
  {
    0, 0,     //x, y
    0, 2, 0,  //アニメ番号、優先順位、BGプライオリティ
  },
};

//--------------------------------------------------------------
/**
 * @brief   ポケウッド画面観客OBJ初期化
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_CreateAudienceObj( AUDIENCE_CONTROL *AOam, HEAPID heapID )
{
  u8  col;
  int i;
  
  AOam->aHandle = GFL_ARC_OpenDataHandle( ARCID_BATTGRA, heapID );

  AOam->audience_clunit  = GFL_CLACT_UNIT_Create( BTLV_INPUT_AUDIENCE_MAX, 1, heapID );

  // リソース転送
  AOam->audience_charaID = GFL_CLGRP_CGR_Register( AOam->aHandle, NARC_battgra_wb_pkwd_obj_NCGR, 0, CLSYS_DRAW_SUB, heapID );
  AOam->audience_cellID  = GFL_CLGRP_CELLANIM_Register( AOam->aHandle, NARC_battgra_wb_pkwd_obj_NCER, NARC_battgra_wb_pkwd_obj_NANR, heapID );
  AOam->audience_plttID  = GFL_CLGRP_PLTT_RegisterEx( AOam->aHandle, NARC_battgra_wb_pkwd_obj_NCLR, CLSYS_DRAW_SUB, 0, 0, 4, heapID );
  
  //
  // clwkのセット
  for( i = 0 ; i < BTLV_INPUT_AUDIENCE_MAX ; i++ )
  {
    // 左上側から順に埋まっていく
    GFL_CLWK_DATA param = audience_param[0];
    param.pos_x = audience_info[i][0];
    param.pos_y = audience_info[i][1];
    param.softpri = 0x80-i;    // 変更
    AOam->audience_wk[ i ] = GFL_CLACT_WK_Create( AOam->audience_clunit, AOam->audience_charaID,
                                                 AOam->audience_plttID, AOam->audience_cellID,
                                                 &param, CLSYS_DEFREND_SUB, heapID );
    //
    GFL_CLACT_WK_SetAutoAnmFlag( AOam->audience_wk[ i ], TRUE );
    GFL_CLACT_WK_SetDrawEnable(  AOam->audience_wk[ i ], TRUE );
    
    // カラーをセット
    col = audience_info[i][2];
    GFL_CLACT_WK_SetPlttOffs( AOam->audience_wk[ i ], col, CLWK_PLTTOFFS_MODE_PLTT_TOP );
  }
}

//--------------------------------------------------------------
/**
 * @brief   ポケウッド画面観客OBJ破棄
 */
//--------------------------------------------------------------
static  void  BTLV_INPUT_DeleteAudienceObj( AUDIENCE_CONTROL *AOam )
{
  int i;
  
  for( i = 0 ; i < BTLV_INPUT_AUDIENCE_MAX ; i++ )
  {
    if(AOam->audience_wk[i]){
      GFL_CLACT_WK_Remove( AOam->audience_wk[ i ] );
      AOam->audience_wk[i] = 0;
    }
  }
  // CGRは他で破棄される
  GFL_CLGRP_CELLANIM_Release( AOam->audience_cellID );
  GFL_CLGRP_PLTT_Release( AOam->audience_plttID );

  if(AOam->audience_clunit!=NULL){
    GFL_CLACT_UNIT_Delete( AOam->audience_clunit );
  }

  GFL_ARC_CloseDataHandle( AOam->aHandle );
}


static int _get_audienceBit( u32 work, int index )
{
  return (work>>index)&1;
}

static void _set_audienceBit( u32 *work, int index )
{
  *work |= (1<<index);
}

//----------------------------------------------------------------------------------
/**
 * @brief	観客人数を設定する
 *
 * @param   wk		
 */
//----------------------------------------------------------------------------------
static u32 set_audience_appear( AUDIENCE_CONTROL *AOam, int audienceNum )
{
  int count=0,no,i;
  u32 audienceBit = 0;
  
  while(count!=audienceNum){
    no = GFUser_GetPublicRand( BTLV_INPUT_AUDIENCE_MAX );
    if(_get_audienceBit(audienceBit, no)==0){
      SetAudienceOnOff( AOam, no, TRUE );
      SetAudienceCondition( AOam, no, 0 );
      _set_audienceBit( &audienceBit, no );
      count++;
      OS_Printf("audience %d=on\n", no );
    }
  }

  for(i=0;i<BTLV_INPUT_AUDIENCE_MAX;i++){
    if(_get_audienceBit(audienceBit, i)==0){
      SetAudienceOnOff( AOam, i, FALSE );
      SetAudienceCondition( AOam, i, 0 );
      OS_Printf("audience %d=off\n", i);
    }
  }
  return audienceBit;
}

//--------------------------------------------------------------
/**
 * @brief   観客の処理(初期化まとめ)
 */
//--------------------------------------------------------------
static void btlv_init_audience_disp( PKWD_BRIEFING_WORK *pbw, HEAPID heapID )
{
  // OBJリソース転送・登録
  BTLV_INPUT_CreateAudienceObj( &pbw->AOam, heapID );
  
  // 観客表示・非表示設定
  pbw->param->audienceBit = set_audience_appear( &pbw->AOam, pbw->param->audienceNum );
  
  // BG0非表示：操作パネル => 今回は表示(仮対応)
  // BG1表示：上書きして使用
  // BG2非表示：メッセージ、情報パネル
  // BG3非表示：背景(アフィン)
  GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
  GFL_BG_SetVisible( FRAME_SEATS, VISIBLE_ON );
  GFL_BG_SetVisible( FRAME_INFOWIN, VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME3_S, VISIBLE_OFF );
  // 念のため
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
  //
  // (BG)リソース転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter( pbw->AOam.aHandle, NARC_battgra_wb_pkwd_movie_c_NCGR,
                                          FRAME_SEATS, 0, 32*32, FALSE, heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( pbw->AOam.aHandle, NARC_battgra_wb_pkwd_movie02_c_NSCR,	// test
                                          FRAME_SEATS, 0, 0, FALSE, heapID );
  GFL_ARCHDL_UTIL_TransVramPalette( pbw->AOam.aHandle, NARC_battgra_wb_pkwd_movie_c_NCLR,
                                          PALTYPE_SUB_BG, 0, 7*0x20, heapID );
  // 念のため
//  SetupSetScroll( pbw->AOam, BG_NO_FRAME, 0, 0, BG_VISIBLE_OFF, BG_VISIBLE_ON, BG_VISIBLE_ON, BG_VISIBLE_OFF );
}

//----------------------------------------------------------------------------------
/**
 * @brief	表示・非表示の設定
 *
 * @param   AOam		
 * @param   no		
 * @param   flag		
 */
//----------------------------------------------------------------------------------
static void SetAudienceOnOff( AUDIENCE_CONTROL *AOam, int no, BOOL flag )
{
  //--------------
  // 表示/非表示
  //--------------
  GFL_CLACT_WK_SetDrawEnable( AOam->audience_wk[ no ], flag );
  
}

//--------------------------------------------------------------
/**
 * @brief   観客の切り替え
 *
 * @param	PKWD_BRIEFING_WORK	*biw
 * @param	const BOOL		(TRUE:表示、FALSE:非表示)
 * @param	const s32		番号(左上から(0 - BTLV_INPUT_AUDIENCE_MAX-1))
 * @param	const s32		種類(AUDIENCE_SEQ_*)
 */
//--------------------------------------------------------------
static void SetAudienceCondition( AUDIENCE_CONTROL *AOam, const s32 no, const s32 mode )
{
  u8   col;
  s32  seq;
  
  //----------------------
  // シーケンスの切り替え
  //----------------------
  seq = mode;
  //
  GFL_CLACT_WK_SetAnmSeq(      AOam->audience_wk[ no ], seq );
  GFL_CLACT_WK_SetAutoAnmFlag( AOam->audience_wk[ no ], TRUE );
  // パレット変更
  col = audience_info[no][2];
  GFL_CLACT_WK_SetPlttOffs( AOam->audience_wk[ no ], col, CLWK_PLTTOFFS_MODE_PLTT_TOP );
}


//=============================================================================================
//  観客の表示テスト
//=============================================================================================
static void debug_btlv_audience( AUDIENCE_CONTROL *AOam )
{
  int  i;
  BOOL bflg=FALSE;
  
  // 左
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
    for(i=0;i<BTLV_INPUT_AUDIENCE_MAX;i++){
      bflg = TRUE;
      // セット
      SetAudienceOnOff(AOam, i, bflg);
      SetAudienceCondition( AOam, i, i%9 );
    }
  }
  // 右
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
    for(i=0;i<BTLV_INPUT_AUDIENCE_MAX;i++){
      bflg = TRUE;
      if(i%2){
        bflg = FALSE;
      }
      // セット
      SetAudienceOnOff(AOam, i, bflg);
      SetAudienceCondition( AOam, i, i%9 );
    }
  }
}

