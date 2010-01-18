//============================================================================
/**
 *  @file   shinka_demo.c
 *  @brief  進化デモ
 *  @author Koji Kawada
 *  @data   2010.01.13
 *  @note   
 *
 *  モジュール名：SHINKADEMO
 */
//============================================================================
// インクルード
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>

#include "demo/shinka_demo.h"

#include "gamesystem/game_beacon.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "arc_def.h"
#include "msg/msg_shinka_demo.h"
#include "msg/msg_waza_oboe.h"
#include "msg/msg_yesnomenu.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "font/font.naix"
#include "battle/app/b_plist.h"
#include "message.naix"

// オーバーレイ
FS_EXTERN_OVERLAY(battle_b_app);
FS_EXTERN_OVERLAY(battle_plist);


#include "gamesystem/msgspeed.h"
#include "system/bmp_winframe.h"

#include "ui/yesno_menu.h"

#include "shinka_demo_graphic.h"

// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x80000)               ///< ヒープサイズ

// BGフレーム
#define BG_FRAME_M_POKEMON           (GFL_BG_FRAME0_M)        // プライオリティ1
#define BG_FRAME_M_STM_TEXT          (GFL_BG_FRAME1_M)        // プライオリティ0

#define BG_FRAME_S_BACK              (GFL_BG_FRAME0_S)        // プライオリティ2
#define BG_FRAME_S_BUTTON            (GFL_BG_FRAME1_S)        // プライオリティ1
#define BG_FRAME_S_BTN_TEXT          (GFL_BG_FRAME2_S)        // プライオリティ0

// BGパレット
// 本数
enum
{
  BG_PAL_NUM_M_STM_TEXT_FONT     = 1,
  BG_PAL_NUM_M_STM_TEXT_FRAME    = 1,
};
// 位置
enum
{
  BG_PAL_POS_M_STM_TEXT_FONT     = 0,
  BG_PAL_POS_M_STM_TEXT_FRAME    = BG_PAL_POS_M_STM_TEXT_FONT   + BG_PAL_NUM_M_STM_TEXT_FONT       , // 1
  BG_PAL_POS_M_MAX               = BG_PAL_POS_M_STM_TEXT_FRAME  + BG_PAL_NUM_M_STM_TEXT_FRAME      , // 2  // ここから空き
};
// 本数
enum
{
  BG_PAL_NUM_S_BUTTON            = 2,
  BG_PAL_NUM_S_BTN_TEXT          = 1,
};
// 位置
enum
{
  BG_PAL_POS_S_BUTTON            = 0,
  BG_PAL_POS_S_BTN_TEXT          = BG_PAL_POS_S_BUTTON          + BG_PAL_NUM_S_BUTTON             ,  // 2 
  BG_PAL_POS_S_MAX               = BG_PAL_POS_S_BTN_TEXT        + BG_PAL_NUM_S_BTN_TEXT           ,  // 3  // ここから空き
};

// OBJパレット
// 本数
// 位置
enum
{
  OBJ_PAL_POS_M_MAX              = 0                                                            ,         // ここから空き
};
// 本数
enum
{
  OBJ_PAL_NUM_S_CURSOR           = 2,
};
// 位置
enum
{
  OBJ_PAL_POS_S_CURSOR           = 0,
  OBJ_PAL_POS_S_MAX              = OBJ_PAL_POS_S_CURSOR        + OBJ_PAL_NUM_S_CURSOR            ,  // 2  // ここから空き
};

// BTN_TEXT
#define BTM_TEXT_WININ_BACK_COLOR        (15)

// フェード
#define FADE_IN_WAIT           (2)         ///< フェードインのスピード
#define FADE_OUT_WAIT          (2)         ///< フェードアウトのスピード

#define STATUS_FADE_OUT_WAIT   (0)         ///< 技選択へ移行するためのフェードアウトのスピード
#define STATUS_FADE_IN_WAIT    (0)         ///< 技選択から戻ってくるためのフェードインのスピード

// 
#define TCBSYS_TASK_MAX        (8)

// ステップ
typedef enum
{
  STEP_FADE_IN,                     // フェードイン
  STEP_EVO_BEFORE,                  // おや！？　ようすが……！
  STEP_EVO_DEMO,                    // デモ
  STEP_EVO_CANCEL,                  // あれ……？　へんかが　とまった！
  STEP_EVO_AFTER,                   // おめでとう！　しんかした！
  STEP_WAZA_OBOE,                   // 技覚えチェック
  STEP_WAZA_VACANT_MASTER,          // あたらしく　おぼえた！(技が３つ以下だった場合)
  STEP_WAZA_FULL_PREPARE,           // 次のQの準備
  STEP_WAZA_FULL_Q,                 // ほかの　わざを　わすれさせますか？
  STEP_WAZA_FULL_YN,                // 選択
  STEP_WAZA_STATUS_FADE_OUT,        // 技選択に移行するためのフェードアウト
  STEP_WAZA_STATUS,                 // 技選択のフェードイン→技選択
  STEP_WAZA_STATUS_OUT,             // 技選択のフェードアウト
  STEP_WAZA_STATUS_FADE_IN,         // 技選択から戻ってくるためのフェードイン
  STEP_WAZA_CONFIRM_PREPARE,        // 次のQの準備
  STEP_WAZA_CONFIRM_Q,              // わすれさせて　よろしいですね？
  STEP_WAZA_CONFIRM_YN,             // 選択
  STEP_WAZA_FORGET,                 // つかいかたを　きれいにわすれた！
  STEP_WAZA_FORGET_MASTER,          // あたらしく　おぼえた！
  STEP_WAZA_ABANDON_PREPARE,        // 次のQの準備
  STEP_WAZA_ABANDON_Q,              // おぼえるのを　あきらめますか？
  STEP_WAZA_ABANDON_YN,             // 選択
  STEP_WAZA_NOT_MASTER,             // おぼえずに　おわった！
  STEP_FADE_OUT_BEFORE,             // フェードアウト前に少し待つ際の間
  STEP_FADE_OUT,                    // フェードアウト
  STEP_END,
}
STEP;

// 文字列中のタグに登録するもの
typedef enum
{
  TAG_REGIST_TYPE_WORD,               // 文字列(STRBUFを渡す)
  TAG_REGIST_TYPE_POKE_MONS_NAME,     // ポケモンの種族名(POKEMON_PARAMを渡す)
  TAG_REGIST_TYPE_POKE_NICK_NAME,     // ポケモンのニックネーム(POKEMON_PARAMを渡す)
  TAG_REGIST_TYPE_WAZA,               // ワザ名(ワザIDを渡す)
  TAG_REGIST_TYPE_NONE,               // なし(NULLを渡す)
}
TAG_REGIST_TYPE;

#define STRBUF_LENGTH       (256)  // この文字数で足りるかbuflen.hで要確認

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// これから変更していく↓
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
enum{
  BACK_COL = 0,
  LETTER_COL,
  SHADOW_COL,

  BUFLEN_SHINKA_DEMO_MSG = 192,
  MSG_WAIT = 80,
  STR_X_CENTERING = 0x1000,
  SHINKADEMO_TCB_MAX = 8,
};

enum{ 
  BMPWIN_MAIN = 0,
  BMPWIN_SUB,

  BMPWIN_MAX,
  BMPWIN_NO_CLEAR = 0x80000000,
};

#define G2D_BACKGROUND_COL  (0x0000)
#define G2D_FONT_COL        (0x7fff)
#define G2D_SHADOW_COL      (0x3def)
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// これから変更していく↑
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  GFL_TCBSYS          *tcb_sys;
  void                *tcb_work;
  GFL_TCB             *v_tcb;
//  PALETTE_FADE_PTR    pfd;  // パレットフェードデータ
  HEAPID              heapID;
  GFL_TCBLSYS*        tcbl_sys;
  GFL_BMPWIN*         bmpwin[ BMPWIN_MAX ];
  GFL_MSGDATA*        shinka_msg;
  GFL_MSGDATA*        wazaoboe_msg;
  GFL_MSGDATA*        yesno_msg;
//  GFL_FONT*           font;
  WORDSET*            wordset;
  STRBUF*             expbuf;
  PRINT_STREAM*       ps;
//  POKEMON_PARAM*      pp;
  SHINKA_DEMO_PARAM*  param;
  BPLIST_DATA         plistData;
  int                 wait;
  int                 work;
//  WazaID              wazaoboe_no;
//  int                 wazaoboe_index;
//  u8                  cursor_flag;


  // 
  HEAPID                      heap_id;
  SHINKADEMO_GRAPHIC_WORK*    graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // 
  POKEMON_PARAM*              pp;
  BOOL                        evo_cancel;

  // ステップ
  STEP                        step;

  // STM_TEXT
  PRINT_STREAM*               stm_text_stream;
  GFL_TCBLSYS*                stm_text_tcblsys;
  GFL_BMPWIN*                 stm_text_winin_bmpwin;
  GFL_BMPWIN*                 stm_text_dummy_bmpwin;      ///< 0番のキャラクターを1x1でつくっておく
  GFL_ARCUTIL_TRANSINFO       stm_text_winfrm_tinfo;
  GFL_MSGDATA*                stm_text_msgdata_shinka;
  GFL_MSGDATA*                stm_text_msgdata_wazaoboe;
  STRBUF*                     stm_text_strbuf;

  // (古い)ニックネーム、忘れる技
  STRBUF*                     poke_nick_name_strbuf;  // ポケモンの種族名がニックネームの場合、進化後にニックネームも変更されるので。
  WazaID                      wazawasure_no;
  u8                          wazawasure_pos;  // 忘れる技の位置

  // wazaoboe
  WazaID                      wazaoboe_no;
  int                         wazaoboe_index;

  // YESNO_MENU
  GFL_MSGDATA*                yesno_menu_msgdata;
  STRBUF*                     yesno_menu_strbuf_yes;
  STRBUF*                     yesno_menu_strbuf_no;
  YESNO_MENU_WORK*            yesno_menu_work;

  // ステータス
  GFL_TCBSYS*                 tcbsys;
  void*                       tcbsys_work;
  PALETTE_FADE_PTR            pfd;  // パレットフェード
  u8                          cursor_flag;
  BPLIST_DATA                 bplist_data;
  GFL_TCB*                    vblank_tcb;

  // 制作中だけ
  u32 TMP_count;

}SHINKA_DEMO_WORK;

//-------------------------------------
/// PROC パラメータ
//=====================================
struct _SHINKA_DEMO_PARAM
{
  const POKEPARTY*  ppt;
  u16               after_mons_no;        //進化後のポケモンナンバー
  u8                shinka_pos;           //進化するポケモンのPOKEPARTY内の位置
  u8                shinka_cond;          //進化条件（ヌケニンチェックに使用）
};


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
static void ShinkaDemo_MakeStmTextStream( SHINKA_DEMO_WORK* work,
                                          GFL_MSGDATA* msgdata, u32 str_id,
                                          TAG_REGIST_TYPE type0, const void* data0,
                                          TAG_REGIST_TYPE type1, const void* data1 );
static BOOL ShinkaDemo_WaitStmTextStream( SHINKA_DEMO_WORK* work );
static void ShinkaDemo_MakeYesNoMenuStrbuf( SHINKA_DEMO_WORK* work,
                                            GFL_MSGDATA* yes_msgdata, u32 yes_str_id,
                                            TAG_REGIST_TYPE yes_type0, const void* yes_data0,
                                            TAG_REGIST_TYPE yes_type1, const void* yes_data1,
                                            GFL_MSGDATA* no_msgdata, u32 no_str_id,
                                            TAG_REGIST_TYPE no_type0, const void* no_data0,
                                            TAG_REGIST_TYPE no_type1, const void* no_data1 );

static STRBUF* MakeStr( HEAPID heap_id,
                        GFL_MSGDATA* msgdata, u32 str_id,
                        TAG_REGIST_TYPE type0, const void* data0,
                        TAG_REGIST_TYPE type1, const void* data1 );

//=============================================================================
/**
*  PROC
*/
//=============================================================================
static void ShinkaDemo_VBlankFunc( GFL_TCB* tcb, void* wk );
static GFL_PROC_RESULT ShinkaDemoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ShinkaDemoProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ShinkaDemoProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

const GFL_PROC_DATA   ShinkaDemoProcData = {
  ShinkaDemoProcInit,
  ShinkaDemoProcMain,
  ShinkaDemoProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief       進化デモ用パラメータ構造体生成
 *
 *  @param[in]   heapID        ヒープID
 *  @param[in]   ppt           POKEPARTY構造体
 *  @param[in]   after_mons_no 進化後のポケモンナンバー
 *  @param[in]   pos           進化するポケモンのPOKEPARTY内のインデックス
 *  @param[in]   cond          進化条件
 *
 *  @retval      SHINKA_DEMO_PARAM
 */
//------------------------------------------------------------------
SHINKA_DEMO_PARAM*  SHINKADEMO_AllocParam( HEAPID heapID, const POKEPARTY* ppt, u16 after_mons_no, u8 pos, u8 cond )
{ 
  SHINKA_DEMO_PARAM*  sdp = GFL_HEAP_AllocMemory( heapID, sizeof( SHINKA_DEMO_PARAM ) );

  sdp->ppt = ppt;
  sdp->after_mons_no = after_mons_no;
  sdp->shinka_pos = pos;
  sdp->shinka_cond = cond;

  return sdp;
}

//------------------------------------------------------------------
/**
 *  @brief           進化デモ用パラメータワーク解放
 *
 *  @param[in,out]   sdp           パラメータワーク構造体
 *
 *  @retval          SHINKA_DEMO_PARAM
 */
//------------------------------------------------------------------
void  SHINKADEMO_FreeParam( SHINKA_DEMO_PARAM* sdp )
{ 
  GFL_HEAP_FreeMemory( sdp );
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT ShinkaDemoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SHINKA_DEMO_PARAM*    param    = (SHINKA_DEMO_PARAM*)pwk;
  SHINKA_DEMO_WORK*     work;

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID( ui_common ) );

  // ヒープ
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SHINKA_DEMO, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(SHINKA_DEMO_WORK), HEAPID_SHINKA_DEMO );
    GFL_STD_MemClear( work, sizeof(SHINKA_DEMO_WORK) );
  }

  // 何よりも先に行う初期化
  {
    // 非表示
    u8 i;
    for(i=GFL_BG_FRAME0_M; i<=GFL_BG_FRAME3_S; i++)
    {
      GFL_BG_SetVisible( i, VISIBLE_OFF );
    }
  } 

  // SHINKA_DEMO_WORK準備
  {
    // 
    work->heap_id       = HEAPID_SHINKA_DEMO;
    work->graphic       = SHINKADEMO_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    SHINKADEMO_GRAPHIC_InitBGSub( work->graphic );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );

    //
    work->pp            = PokeParty_GetMemberPointer( param->ppt, param->shinka_pos );

    // ステップ
    work->step      = STEP_FADE_IN;
  }

  // STM_TEXT
  {
    // パレット
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
      BG_PAL_POS_M_STM_TEXT_FONT * 0x20, BG_PAL_NUM_M_STM_TEXT_FONT * 0x20, work->heap_id );

    // BGフレームのスクリーンの空いている箇所に何も表示がされないようにしておく
    work->stm_text_dummy_bmpwin = GFL_BMPWIN_Create( BG_FRAME_M_STM_TEXT, 0, 0, 1, 1,
                                    BG_PAL_POS_M_STM_TEXT_FONT, GFL_BMP_CHRAREA_GET_F );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->stm_text_dummy_bmpwin), 0 );
    GFL_BMPWIN_TransVramCharacter(work->stm_text_dummy_bmpwin);

    // ウィンドウ内
    work->stm_text_winin_bmpwin = GFL_BMPWIN_Create( BG_FRAME_M_STM_TEXT, 1, 19, 30, 4,
                                     BG_PAL_POS_M_STM_TEXT_FONT, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->stm_text_winin_bmpwin), BTM_TEXT_WININ_BACK_COLOR );
    GFL_BMPWIN_TransVramCharacter(work->stm_text_winin_bmpwin);
    
    // ウィンドウ枠
    work->stm_text_winfrm_tinfo = BmpWinFrame_GraphicSetAreaMan( BG_FRAME_M_STM_TEXT,
                                    BG_PAL_POS_M_STM_TEXT_FRAME,
                                    MENU_TYPE_SYSTEM, work->heap_id );
    BmpWinFrame_Write( work->stm_text_winin_bmpwin, WINDOW_TRANS_ON_V,
                       GFL_ARCUTIL_TRANSINFO_GetPos(work->stm_text_winfrm_tinfo),
                       BG_PAL_POS_M_STM_TEXT_FRAME );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->stm_text_winin_bmpwin );

    // メッセージ
    work->stm_text_msgdata_shinka     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_shinka_demo_dat, work->heap_id );
    work->stm_text_msgdata_wazaoboe   = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_waza_oboe_dat, work->heap_id );

    // TCBL、フォントカラー、転送など
    work->stm_text_tcblsys = GFL_TCBL_Init( work->heap_id, work->heap_id, 1, 0 );
    GFL_FONTSYS_SetColor( 1, 2, BTM_TEXT_WININ_BACK_COLOR );
    GFL_BG_LoadScreenV_Req( BG_FRAME_M_STM_TEXT );

    // NULL初期化
    work->stm_text_strbuf       = NULL;
    work->stm_text_stream       = NULL;
  }

  // (古い)ニックネーム
  {
    work->poke_nick_name_strbuf = GFL_STR_CreateBuffer( STRBUF_LENGTH, work->heapID );
  }

  // YESNO_MENU
  {
    // メッセージ
    work->yesno_menu_msgdata     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_yesnomenu_dat, work->heap_id );

    // ワーク
    work->yesno_menu_work        = YESNO_MENU_CreateRes( work->heap_id,
                                      BG_FRAME_S_BUTTON, 0, BG_PAL_POS_S_BUTTON,
                                      OBJ_PAL_POS_S_CURSOR,
                                      SHINKADEMO_GRAPHIC_GetClunit(work->graphic),
                                      work->font,
                                      work->print_que,
                                      FALSE );

    // NULL初期化
    work->yesno_menu_strbuf_yes = NULL;
    work->yesno_menu_strbuf_no  = NULL;
  }

  // プライオリティ、表示、背景色など
  {
    GFL_BG_SetPriority( BG_FRAME_M_POKEMON               , 1 );
    GFL_BG_SetPriority( BG_FRAME_M_STM_TEXT              , 0 );  // 最前面

    GFL_BG_SetPriority( BG_FRAME_S_BACK                  , 2 );
    //GFL_BG_SetPriority( BG_FRAME_S_BUTTON                , 1 );
    //GFL_BG_SetPriority( BG_FRAME_S_BTN_TEXT              , 0 );  // 最前面

    GFL_BG_SetVisible( BG_FRAME_M_POKEMON               , VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_M_STM_TEXT              , VISIBLE_ON );
  
    GFL_BG_SetVisible( BG_FRAME_S_BACK                  , VISIBLE_ON );
    //GFL_BG_SetVisible( BG_FRAME_S_BUTTON                , VISIBLE_ON );
    //GFL_BG_SetVisible( BG_FRAME_S_BTN_TEXT              , VISIBLE_ON );
  
    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );
    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x0000 );
  }

  // ステータス
  {
    // 初期化
    work->tcbsys           = NULL;
    work->tcbsys_work      = NULL;
    work->pfd              = NULL;
    work->vblank_tcb       = NULL;

    work->cursor_flag      = 0;

    // ステータス用タスク
    //work->tcbsys_work = GFL_HEAP_AllocMemory( work->heap_id, GFL_TCB_CalcSystemWorkSize(TCBSYS_TASK_MAX) );
    //GFL_STD_MemClear( work->tcbsys_work, GFL_TCB_CalcSystemWorkSize(TCBSYS_TASK_MAX) );
    //work->tcbsys = GFL_TCB_Init( TCBSYS_TASK_MAX, work->tcbsys_work );

    // パレットフェード
    //work->pfd = PaletteFadeInit( work->heap_id );
    //PaletteTrans_AutoSet( work->pfd, TRUE );
    //PaletteFadeWorkAllocSet( work->pfd, FADE_MAIN_BG, 0x200, work->heapID );
    //PaletteFadeWorkAllocSet( work->pfd, FADE_SUB_BG, 0x1e0, work->heapID );
    //PaletteFadeWorkAllocSet( work->pfd, FADE_MAIN_OBJ, 0x200, work->heapID );
    //PaletteFadeWorkAllocSet( work->pfd, FADE_SUB_OBJ, 0x1e0, work->heapID );

    //work->vblank_tcb = GFUser_VIntr_CreateTCB( ShinkaDemo_VBlankFunc, work, 1 );
  }

  // フェードイン(黒→見える)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT ShinkaDemoProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SHINKA_DEMO_PARAM*    param    = (SHINKA_DEMO_PARAM*)pwk;
  SHINKA_DEMO_WORK*     work     = (SHINKA_DEMO_WORK*)mywk;

  // ステータス
  {
    //GFL_TCB_DeleteTask( work->vblank_tcb );

    // パレットフェード
    //PaletteFadeWorkAllocFree( work->pfd, FADE_MAIN_BG );
    //PaletteFadeWorkAllocFree( work->pfd, FADE_SUB_BG );
    //PaletteFadeWorkAllocFree( work->pfd, FADE_MAIN_OBJ );
    //PaletteFadeWorkAllocFree( work->pfd, FADE_SUB_OBJ );
    //PaletteFadeFree( work->pfd );

    // ステータス用タスク
    //GFL_HEAP_FreeMemory( work->tcbsys_work );
    //GFL_TCB_Exit( work->tcb_sys );
  }

  // YESNO_MENU
  {
    YESNO_MENU_DeleteRes( work->yesno_menu_work );
    if( work->yesno_menu_strbuf_yes ) GFL_STR_DeleteBuffer( work->yesno_menu_strbuf_yes );
    if( work->yesno_menu_strbuf_no ) GFL_STR_DeleteBuffer( work->yesno_menu_strbuf_no );
    GFL_MSG_Delete( work->yesno_menu_msgdata );
  }

  // (古い)ニックネーム
  {
    GFL_STR_DeleteBuffer( work->poke_nick_name_strbuf );
  }

  // STM_TEXT
  {
    if( work->stm_text_stream ) PRINTSYS_PrintStreamDelete( work->stm_text_stream );
    GFL_TCBL_Exit( work->stm_text_tcblsys );
    if( work->stm_text_strbuf ) GFL_STR_DeleteBuffer( work->stm_text_strbuf );
    GFL_MSG_Delete( work->stm_text_msgdata_shinka );
    GFL_MSG_Delete( work->stm_text_msgdata_wazaoboe );
    GFL_BG_FreeCharacterArea( BG_FRAME_M_STM_TEXT,
      GFL_ARCUTIL_TRANSINFO_GetPos(work->stm_text_winfrm_tinfo),
      GFL_ARCUTIL_TRANSINFO_GetSize(work->stm_text_winfrm_tinfo) );
    GFL_BMPWIN_Delete( work->stm_text_winin_bmpwin );
    GFL_BMPWIN_Delete( work->stm_text_dummy_bmpwin );
  }

  // SHINKA_DEMO_WORK後片付け
  {
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    SHINKADEMO_GRAPHIC_ExitBGSub( work->graphic );
    SHINKADEMO_GRAPHIC_Exit( work->graphic );
  }
  
  // ヒープ
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_SHINKA_DEMO );
  }

  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( ui_common ) );
 
  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT ShinkaDemoProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SHINKA_DEMO_PARAM*    param    = (SHINKA_DEMO_PARAM*)pwk;
  SHINKA_DEMO_WORK*     work     = (SHINKA_DEMO_WORK*)mywk;

  int key_trg = GFL_UI_KEY_GetTrg();

  switch( work->step )
  {
  case STEP_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->step = STEP_EVO_BEFORE;
        
        ShinkaDemo_MakeStmTextStream(
          work,
          work->stm_text_msgdata_shinka, SHINKADEMO_ShinkaBeforeMsg,
          TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
          TAG_REGIST_TYPE_NONE, NULL );
      }
    }
    break;
  case STEP_EVO_BEFORE:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // 次へ
        work->step = STEP_EVO_DEMO;
        
        work->evo_cancel = FALSE;
        work->TMP_count = 0;
      }
    }
    break;
  case STEP_EVO_DEMO:
    {
      work->TMP_count++;

      if( work->TMP_count < 180 )
      {
        if( key_trg & PAD_BUTTON_B )
        {
          work->evo_cancel = TRUE;
        }
      }

      //if( work->TMP_count >= 180 )
      if( work->TMP_count >= 180 || work->evo_cancel )
      {
        if( work->evo_cancel )
        {
          // 次へ
          work->step = STEP_EVO_CANCEL;

          ShinkaDemo_MakeStmTextStream(
            work,
            work->stm_text_msgdata_shinka, SHINKADEMO_ShinkaCancelMsg,
            TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
            TAG_REGIST_TYPE_NONE, NULL );
        }
        else
        {
          // (古い)ニックネーム
          PP_Get( work->pp, ID_PARA_nickname, work->poke_nick_name_strbuf );

          // 進化
          PP_ChangeMonsNo( work->pp, param->after_mons_no );

          // 次へ
          work->step = STEP_EVO_AFTER;

          ShinkaDemo_MakeStmTextStream(
            work,
            work->stm_text_msgdata_shinka, SHINKADEMO_ShinkaMsg,
            TAG_REGIST_TYPE_WORD, work->poke_nick_name_strbuf,
            TAG_REGIST_TYPE_POKE_MONS_NAME, work->pp );
        }
      }
    }
    break;
  case STEP_EVO_CANCEL:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // 次へ
        work->step = STEP_FADE_OUT_BEFORE;
      }
    }
    break;
  case STEP_EVO_AFTER:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // すれ違い用データをセット
        { 
          STRBUF* strbuf = GFL_STR_CreateBuffer( STRBUF_LENGTH, work->heapID );
          PP_Get( work->pp, ID_PARA_nickname, strbuf );  // ここは新しいニックネームでいいのだろうか？
          GAMEBEACON_Set_PokemonEvolution( strbuf );
          GFL_STR_DeleteBuffer( strbuf );
        }

        // 次へ
        work->step = STEP_WAZA_OBOE;

        work->wazaoboe_index = 0;
      }
    }
    break;
  case STEP_WAZA_OBOE:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // 技覚えチェック
        work->wazaoboe_no = PP_CheckWazaOboe( work->pp, &work->wazaoboe_index, work->heap_id );
        if( work->wazaoboe_no == PTL_WAZAOBOE_NONE )
        {
          // 次へ
          work->step = STEP_FADE_OUT_BEFORE;
        }
        else if( work->wazaoboe_no == PTL_WAZASET_SAME )
        {
          // 次のインデックスで技覚えチェック
          break;
        }
        else if( work->wazaoboe_no & PTL_WAZAOBOE_FULL )
        {
          work->wazaoboe_no &= ( PTL_WAZAOBOE_FULL ^ 0xffff );

          // 次へ
          work->step = STEP_WAZA_FULL_PREPARE;
        }
        else
        {
          // 次へ
          work->step = STEP_WAZA_VACANT_MASTER;
          
          // 技覚えた
          ShinkaDemo_MakeStmTextStream(
            work,
            work->stm_text_msgdata_wazaoboe, msg_waza_oboe_04,
            TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
            TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
        }
      }
    }
    break;
  case STEP_WAZA_VACANT_MASTER:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        if( key_trg & PAD_BUTTON_A )
        {
          // 次へ
          work->step = STEP_WAZA_OBOE;
        }
      }
    }
    break;
  case STEP_WAZA_FULL_PREPARE:
    {
      // 次へ
      work->step = STEP_WAZA_FULL_Q;
          
      // 技の手持ちがいっぱい
      ShinkaDemo_MakeStmTextStream(
        work,
        work->stm_text_msgdata_wazaoboe, msg_waza_oboe_05,
        TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
        TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
    }
    break;
  case STEP_WAZA_FULL_Q:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // 次へ
        work->step = STEP_WAZA_FULL_YN;

        // 技忘れ確認処理
        ShinkaDemo_MakeYesNoMenuStrbuf( work,
                                        work->yesno_menu_msgdata, msgid_yesno_wazawasureru,
                                        TAG_REGIST_TYPE_NONE, NULL,
                                        TAG_REGIST_TYPE_NONE, NULL,
                                        work->yesno_menu_msgdata, msgid_yesno_wazawasurenai,
                                        TAG_REGIST_TYPE_NONE, NULL,
                                        TAG_REGIST_TYPE_NONE, NULL );
        YESNO_MENU_OpenMenu( work->yesno_menu_work, work->yesno_menu_strbuf_yes, work->yesno_menu_strbuf_no );
      }
    }
    break;
  case STEP_WAZA_FULL_YN:
    {
      YESNO_MENU_SEL sel = YESNO_MENU_GetSelect( work->yesno_menu_work );
      if( sel != YESNO_MENU_SEL_SEL )
      {
        if( sel == YESNO_MENU_SEL_YES )
        {
          // 次へ
          work->step = STEP_WAZA_STATUS_FADE_OUT;

          // フェードアウト(見える→黒)
          GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, STATUS_FADE_OUT_WAIT );
        }
        else if( sel == YESNO_MENU_SEL_NO )
        {
          // 次へ
          work->step = STEP_WAZA_ABANDON_PREPARE;
        }
        YESNO_MENU_CloseMenu( work->yesno_menu_work );
      }
    }
    break;
  case STEP_WAZA_STATUS_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->step = STEP_WAZA_STATUS;

        // 下画面の破棄
        YESNO_MENU_DeleteRes( work->yesno_menu_work );
        SHINKADEMO_GRAPHIC_ExitBGSub( work->graphic );

        // ステータス用タスク
        work->tcbsys_work = GFL_HEAP_AllocMemory( work->heap_id, GFL_TCB_CalcSystemWorkSize(TCBSYS_TASK_MAX) );
        GFL_STD_MemClear( work->tcbsys_work, GFL_TCB_CalcSystemWorkSize(TCBSYS_TASK_MAX) );
        work->tcbsys = GFL_TCB_Init( TCBSYS_TASK_MAX, work->tcbsys_work );

        work->vblank_tcb = GFUser_VIntr_CreateTCB( ShinkaDemo_VBlankFunc, work, 1 );
 
        // パレットフェード
        work->pfd = PaletteFadeInit( work->heap_id );
        PaletteTrans_AutoSet( work->pfd, TRUE );
        PaletteFadeWorkAllocSet( work->pfd, FADE_SUB_BG, 0x1e0, work->heapID );
        PaletteFadeWorkAllocSet( work->pfd, FADE_SUB_OBJ, 0x1e0, work->heapID );

        // ステータス
        work->bplist_data.pp          = (POKEPARTY*)param->ppt;
        work->bplist_data.font        = work->font;
        work->bplist_data.heap        = work->heap_id;
        work->bplist_data.mode        = BPL_MODE_WAZASET;
        work->bplist_data.end_flg     = FALSE;
        work->bplist_data.sel_poke    = param->shinka_pos;
        work->bplist_data.chg_waza    = work->wazaoboe_no;
        work->bplist_data.rule        = 0;
        work->bplist_data.cursor_flg  = &work->cursor_flag;
        work->bplist_data.tcb_sys     = work->tcbsys;
        work->bplist_data.pfd         = work->pfd;

        GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_b_app ) );
        GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_plist ) );
        
        BattlePokeList_TaskAdd( &work->bplist_data );

        // フェードイン(黒→見える)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, STATUS_FADE_IN_WAIT );
      }
    }
    break;
  case STEP_WAZA_STATUS:
    {
      // ステータス
      GFL_TCB_Main( work->tcbsys );

      if( work->bplist_data.end_flg )
      {
        // 次へ
        work->step = STEP_WAZA_STATUS_OUT;

        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, STATUS_FADE_OUT_WAIT );
      }
    }
    break;
  case STEP_WAZA_STATUS_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->step = STEP_WAZA_STATUS_FADE_IN;

        // ステータス
        GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_b_app ) );
        GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_plist ) );

        // パレットフェード
        PaletteFadeWorkAllocFree( work->pfd, FADE_SUB_BG );
        PaletteFadeWorkAllocFree( work->pfd, FADE_SUB_OBJ );
        PaletteFadeFree( work->pfd );

        // ステータス用タスク
        GFL_TCB_DeleteTask( work->vblank_tcb );

        GFL_HEAP_FreeMemory( work->tcbsys_work );
        GFL_TCB_Exit( work->tcb_sys );

        // 初期化
        work->tcbsys           = NULL;
        work->tcbsys_work      = NULL;
        work->pfd              = NULL;
        work->vblank_tcb       = NULL;

        // 下画面の生成
        SHINKADEMO_GRAPHIC_InitBGSub( work->graphic );
        work->yesno_menu_work = YESNO_MENU_CreateRes( work->heap_id,
                                  BG_FRAME_S_BUTTON, 0, BG_PAL_POS_S_BUTTON,
                                  OBJ_PAL_POS_S_CURSOR,
                                  SHINKADEMO_GRAPHIC_GetClunit(work->graphic),
                                  work->font,
                                  work->print_que,
                                  FALSE );

        // フェードイン(黒→見える)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, STATUS_FADE_IN_WAIT );
      }
    }
    break;
  case STEP_WAZA_STATUS_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // ステータス
        work->wazawasure_pos = work->bplist_data.sel_wp;
        if( work->wazawasure_pos == BPL_SEL_WP_CANCEL )
        {
          // 次へ
          work->step = STEP_WAZA_ABANDON_PREPARE;
        }
        else
        {
          // 忘れる技
          work->wazawasure_no = PP_Get( work->pp, ID_PARA_waza1 + work->wazawasure_pos, NULL );

          // 次へ
          work->step = STEP_WAZA_CONFIRM_PREPARE;
        }
      }
    }
    break;
  case STEP_WAZA_CONFIRM_PREPARE:
    {
      // 次へ
      work->step = STEP_WAZA_CONFIRM_Q;

      ShinkaDemo_MakeStmTextStream(
        work,
        work->stm_text_msgdata_wazaoboe, msg_waza_oboe_11,
        TAG_REGIST_TYPE_WAZA, &work->wazawasure_no,
        TAG_REGIST_TYPE_NONE, NULL );
    }
    break;
  case STEP_WAZA_CONFIRM_Q:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // 次へ
        work->step = STEP_WAZA_CONFIRM_YN;

        ShinkaDemo_MakeYesNoMenuStrbuf( work,
                                        work->yesno_menu_msgdata, msgid_yesno_wazawasureru,
                                        TAG_REGIST_TYPE_NONE, NULL,
                                        TAG_REGIST_TYPE_NONE, NULL,
                                        work->yesno_menu_msgdata, msgid_yesno_wazawasurenai,
                                        TAG_REGIST_TYPE_NONE, NULL,
                                        TAG_REGIST_TYPE_NONE, NULL );
        YESNO_MENU_OpenMenu( work->yesno_menu_work, work->yesno_menu_strbuf_yes, work->yesno_menu_strbuf_no );
      }
    }
    break;
  case STEP_WAZA_CONFIRM_YN:
    {
      YESNO_MENU_SEL sel = YESNO_MENU_GetSelect( work->yesno_menu_work );
      if( sel != YESNO_MENU_SEL_SEL )
      {
        if( sel == YESNO_MENU_SEL_YES )
        {
          // 次へ
          work->step = STEP_WAZA_FORGET;

          ShinkaDemo_MakeStmTextStream(
            work,
            work->stm_text_msgdata_wazaoboe, msg_waza_oboe_06,
            TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
            TAG_REGIST_TYPE_WAZA, &work->wazawasure_no );
        }
        else if( sel == YESNO_MENU_SEL_NO )
        {
          // 次へ
          work->step = STEP_WAZA_FULL_PREPARE;
        }
        YESNO_MENU_CloseMenu( work->yesno_menu_work );
      }
    }
    break;
  case STEP_WAZA_FORGET:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // 技覚えた
        PP_SetWazaPos( work->pp, work->wazaoboe_no, work->wazawasure_pos );
        
        // 次へ
        work->step = STEP_WAZA_FORGET_MASTER;

        ShinkaDemo_MakeStmTextStream(
          work,
          work->stm_text_msgdata_wazaoboe, msg_waza_oboe_07,
          TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
          TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
      }
    }
    break;
  case STEP_WAZA_FORGET_MASTER:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
         // 次へ
        work->step = STEP_WAZA_OBOE;
      }
    }
    break;
  case STEP_WAZA_ABANDON_PREPARE:
    {
      // 次へ
      work->step = STEP_WAZA_ABANDON_Q;

      ShinkaDemo_MakeStmTextStream(
        work,
        work->stm_text_msgdata_wazaoboe, msg_waza_oboe_08,
        TAG_REGIST_TYPE_NONE, NULL,
        TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
    }
    break;
  case STEP_WAZA_ABANDON_Q:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // 次へ
        work->step = STEP_WAZA_ABANDON_YN;

        // 技あきらめ確認処理
        ShinkaDemo_MakeYesNoMenuStrbuf( work,
                                        work->yesno_menu_msgdata, msgid_yesno_wazaakirameru,
                                        TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no,
                                        TAG_REGIST_TYPE_NONE, NULL,
                                        work->yesno_menu_msgdata, msgid_yesno_wazaakiramenai,
                                        TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no,
                                        TAG_REGIST_TYPE_NONE, NULL );
        YESNO_MENU_OpenMenu( work->yesno_menu_work, work->yesno_menu_strbuf_yes, work->yesno_menu_strbuf_no );
      }
    }
    break;
  case STEP_WAZA_ABANDON_YN:
    {
      YESNO_MENU_SEL sel = YESNO_MENU_GetSelect( work->yesno_menu_work );
      if( sel != YESNO_MENU_SEL_SEL )
      {
        if( sel == YESNO_MENU_SEL_YES )
        {
          // 次へ
          work->step = STEP_WAZA_NOT_MASTER;

          ShinkaDemo_MakeStmTextStream(
            work,
            work->stm_text_msgdata_wazaoboe, msg_waza_oboe_09,
            TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
            TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
        }
        else if( sel == YESNO_MENU_SEL_NO )
        {
          // 次へ
          work->step = STEP_WAZA_FULL_PREPARE;
        }
        YESNO_MENU_CloseMenu( work->yesno_menu_work );
      }
    }
    break;
  case STEP_WAZA_NOT_MASTER:
    {
      if( ShinkaDemo_WaitStmTextStream( work ) )
      {
        // 次へ
        work->step = STEP_WAZA_OBOE;
      }
    }
    break;
  case STEP_FADE_OUT_BEFORE:
    {
      if(1)
      {
        // 次へ
        work->step = STEP_FADE_OUT;

        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );
      }
    }
    break;
  case STEP_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->step = STEP_END;

        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  default:
    break;
  }

  //GFL_TCB_Main( work->tcbsys );

  YESNO_MENU_Main( work->yesno_menu_work );

  GFL_TCBL_Main( work->stm_text_tcblsys );

  PRINTSYS_QUE_Main( work->print_que );

  // 2D描画
  SHINKADEMO_GRAPHIC_2D_Draw( work->graphic );
  // 3D描画
  SHINKADEMO_GRAPHIC_3D_StartDraw( work->graphic );
  SHINKADEMO_GRAPHIC_3D_EndDraw( work->graphic );

  return GFL_PROC_RES_CONTINUE;
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void ShinkaDemo_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  SHINKA_DEMO_WORK* work = (SHINKA_DEMO_WORK*)wk;

  // ステータス
  {
    // パレットフェード
    if( work->pfd ) PaletteFadeTrans( work->pfd );
  }
}

//-------------------------------------
/// STM_TEXTのストリーム作成
//=====================================
static void ShinkaDemo_MakeStmTextStream( SHINKA_DEMO_WORK* work,
                                          GFL_MSGDATA* msgdata, u32 str_id,
                                          TAG_REGIST_TYPE type0, const void* data0,
                                          TAG_REGIST_TYPE type1, const void* data1 )
{
  // 一旦消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->stm_text_winin_bmpwin), BTM_TEXT_WININ_BACK_COLOR );
  
  // 前のを消す
  if( work->stm_text_stream ) PRINTSYS_PrintStreamDelete( work->stm_text_stream );  // この関数でタスクも削除してくれるので、同時に動くタスクは1つで済む
  if( work->stm_text_strbuf ) GFL_STR_DeleteBuffer( work->stm_text_strbuf );

  // 文字列作成
  work->stm_text_strbuf = MakeStr( work->heap_id,
                                   msgdata, str_id,
                                   type0, data0,
                                   type1, data1 );

  // ストリーム開始
  work->stm_text_stream = PRINTSYS_PrintStream( work->stm_text_winin_bmpwin,
                                                0, 0,
                                                work->stm_text_strbuf,
                                                work->font, MSGSPEED_GetWait(),
                                                work->stm_text_tcblsys, 2,
                                                work->heap_id,
                                                BTM_TEXT_WININ_BACK_COLOR );
}

//-------------------------------------
/// STM_TEXTのストリーム待ち
//=====================================
static BOOL ShinkaDemo_WaitStmTextStream( SHINKA_DEMO_WORK* work )
{
  BOOL ret = FALSE;

  switch( PRINTSYS_PrintStreamGetState( work->stm_text_stream ) )
  { 
  case PRINTSTREAM_STATE_PAUSE:
    if( ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ) || GFL_UI_TP_GetTrg() )
    { 
      PRINTSYS_PrintStreamReleasePause( work->stm_text_stream );
    }
    break;
  case PRINTSTREAM_STATE_DONE:
    {
      ret = TRUE;
    }
    break;
  }
  
  return ret;
}

//-------------------------------------
/// YESNO_MENUの文字列作成
//=====================================
static void ShinkaDemo_MakeYesNoMenuStrbuf( SHINKA_DEMO_WORK* work,
                                            GFL_MSGDATA* yes_msgdata, u32 yes_str_id,
                                            TAG_REGIST_TYPE yes_type0, const void* yes_data0,
                                            TAG_REGIST_TYPE yes_type1, const void* yes_data1,
                                            GFL_MSGDATA* no_msgdata, u32 no_str_id,
                                            TAG_REGIST_TYPE no_type0, const void* no_data0,
                                            TAG_REGIST_TYPE no_type1, const void* no_data1 )
{
  // 前のを消す
  if( work->yesno_menu_strbuf_yes ) GFL_STR_DeleteBuffer( work->yesno_menu_strbuf_yes );
  if( work->yesno_menu_strbuf_no  ) GFL_STR_DeleteBuffer( work->yesno_menu_strbuf_no  );

  // 文字列作成
  work->yesno_menu_strbuf_yes = MakeStr(
                                  work->heap_id,
                                  yes_msgdata, yes_str_id,
                                  yes_type0, yes_data0,
                                  yes_type1, yes_data1 );
  work->yesno_menu_strbuf_no  = MakeStr(
                                  work->heap_id,
                                  no_msgdata, no_str_id,
                                  no_type0, no_data0,
                                  no_type1, no_data1 );
}

//-------------------------------------
/// 文字列作成
//=====================================
static STRBUF* MakeStr( HEAPID heap_id,
                        GFL_MSGDATA* msgdata, u32 str_id,
                        TAG_REGIST_TYPE type0, const void* data0,
                        TAG_REGIST_TYPE type1, const void* data1 )
{
  STRBUF* strbuf;

  if(    type0 == TAG_REGIST_TYPE_NONE
      && type1 == TAG_REGIST_TYPE_NONE )
  {
    strbuf = GFL_MSG_CreateString( msgdata, str_id );
  }
  else
  {
    typedef struct
    {
      TAG_REGIST_TYPE type;
      const void*     data;
    }
    TYPE_DATA_SET;
    TYPE_DATA_SET type_data_set[2];

    STRBUF* src_strbuf = GFL_MSG_CreateString( msgdata, str_id );
    WORDSET* wordset = WORDSET_Create( heap_id );

    u32 i;

    type_data_set[0].type = type0;
    type_data_set[0].data = data0;
    type_data_set[1].type = type1;
    type_data_set[1].data = data1;

    for(i=0; i<2; i++)
    {
      switch( type_data_set[i].type )
      {
      case TAG_REGIST_TYPE_WORD:
        {
          WORDSET_RegisterWord( wordset, i, type_data_set[i].data, 0, TRUE, 0 );  // ポケモンのニックネームしか渡す予定がないので、パラメータは固定値にしています
        }
        break;
      case TAG_REGIST_TYPE_POKE_MONS_NAME:
        {
          WORDSET_RegisterPokeMonsName( wordset, i, type_data_set[i].data );
        }
        break;
      case TAG_REGIST_TYPE_POKE_NICK_NAME:
        {
          WORDSET_RegisterPokeNickName( wordset, i, type_data_set[i].data );
        }
        break;
      case TAG_REGIST_TYPE_WAZA:
        {
          WORDSET_RegisterWazaName( wordset, i, *((WazaID*)(type_data_set[i].data)) );
        }
        break;
      default:
        break;
      }
    }
    
    strbuf = GFL_STR_CreateBuffer( STRBUF_LENGTH, heap_id );
    WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
    
    GFL_STR_DeleteBuffer( src_strbuf );
    WORDSET_Delete( wordset );
  }

  return strbuf;
}

