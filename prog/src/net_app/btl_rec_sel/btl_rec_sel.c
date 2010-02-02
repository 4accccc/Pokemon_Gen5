//============================================================================
/**
 *  @file   btl_rec_sel.c
 *  @brief  通信対戦後の録画選択画面
 *  @author Koji Kawada
 *  @data   2010.01.27
 *  @note   
 *
 *  モジュール名：BTL_REC_SEL
 */
//============================================================================
//#define OFFLINE_TEST


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "gamesystem/msgspeed.h"
#include "system/palanm.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menu.h"
#include "savedata/battle_rec.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "poke_tool/poke_tool.h"
#include "pokeicon/pokeicon.h"
#include "ui/ui_easy_clwk.h"

#include "btl_rec_sel_graphic.h"
#include "net_app/btl_rec_sel.h"


// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_b_record.h"
#include "c_gear.naix"
#include "batt_rec_gra.naix"
// サウンド


// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x50000)               ///< ヒープサイズ

// BGフレーム
#define BG_FRAME_M_BACK              (GFL_BG_FRAME2_M)        // プライオリティ2
#define BG_FRAME_M_TIME              (GFL_BG_FRAME3_M)        // プライオリティ1
#define BG_FRAME_M_TEXT              (GFL_BG_FRAME1_M)        // プライオリティ0

#define BG_FRAME_S_BACK              (GFL_BG_FRAME0_S)        // プライオリティ0

// BGパレット
// 本数
enum
{
  BG_PAL_NUM_M_BACKDROP      = 0,
  BG_PAL_NUM_M_TEXT_FONT     = 1,
  BG_PAL_NUM_M_TEXT_FRAME    = 1,
  BG_PAL_NUM_M_YN            = 1,  // 使用せず
  BG_PAL_NUM_M_BACK          = 9,
};
// 位置
enum
{
  BG_PAL_POS_M_BACK          = 0                                                       ,  // 0
  BG_PAL_POS_M_BACKDROP      = BG_PAL_POS_M_BACK        + BG_PAL_NUM_M_BACK            ,  // 9 
  BG_PAL_POS_M_TEXT_FONT     = BG_PAL_POS_M_BACKDROP    + BG_PAL_NUM_M_BACKDROP        ,  // 9 
  BG_PAL_POS_M_TEXT_FRAME    = BG_PAL_POS_M_TEXT_FONT   + BG_PAL_NUM_M_TEXT_FONT       ,  // 10 
  BG_PAL_POS_M_YN            = BG_PAL_POS_M_TEXT_FRAME  + BG_PAL_NUM_M_TEXT_FRAME      ,  // 11  // 使用せず
  BG_PAL_POS_M_MAX           = BG_PAL_POS_M_YN          + BG_PAL_NUM_M_YN              ,  // 12  // ここから空き
};
// 本数
enum
{
  BG_PAL_NUM_S_BACK          = 15,
};
// 位置
enum
{
  BG_PAL_POS_S_BACK          = 0                                                      ,  // 0
  BG_PAL_POS_S_MAX           = BG_PAL_POS_S_BACK        + BG_PAL_NUM_S_BACK           ,  // 1  // ここから空き
};

// OBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_PI           = 3,
};
// 位置
enum
{
  OBJ_PAL_POS_M_PI           = 0,
  OBJ_PAL_POS_M_MAX          = OBJ_PAL_POS_M_PI         + OBJ_PAL_NUM_M_PI           ,  // 3  // ここから空き
};
// 本数
enum
{
  OBJ_PAL_NUM_S_             = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_S_MAX          = 0                                                      ,      // ここから空き
};

// ストリームテキストウィンドウのウィンドウ内の背景色
#define TEXT_WININ_BACK_COLOR        (15)

// フェード
#define FADE_IN_WAIT           (2)         ///< フェードインのスピード
#define FADE_OUT_WAIT          (2)         ///< フェードアウトのスピード

#define INSIDE_FADE_OUT_WAIT   (0)         ///< 内部で表示を切り替える際のフェードアウトのスピード
#define INSIDE_FADE_IN_WAIT    (0)         ///< 内部で表示を切り替える際のフェードインのスピード

// ポケアイコン
enum
{
  PI_POS_R,
  PI_POS_L,
  PI_POS_MAX,
};
#define PI_PARTY_NUM      (6)      // ポケアイコンの個数

// 固定テキスト
enum
{
  FIX_PRE,
  FIX_PRE_TITLE,
  FIX_SEL_TIME,
  FIX_TIME,
  FIX_MAX,
};

// 1秒間のフレーム数
#define FPS (30)

// 文字数
#define STRBUF_FIX_TIME_LENGTH       (  8)  // ??:??
#define STRBUF_LENGTH                (256)  // この文字数で足りるかbuflen.hで要確認

// シーケンス
enum
{
  SEQ_FADE_INIT,
  SEQ_FADE,
  SEQ_QA_INIT,
  SEQ_QA_QUES,
  SEQ_STM_INIT,
  SEQ_QA_ANS_REC,
  SEQ_QA_ANS_NOREC,
  SEQ_QA_ANS_PRE,
  SEQ_PRE_SHOW_ON,
  SEQ_PRE_SHOW_OFF,
  SEQ_SAVE_INIT,
  SEQ_SAVE,
  SEQ_SAVEOK,
  SEQ_SAVENG,
  SEQ_WAIT_INIT,
  SEQ_WAIT,
  SEQ_END,
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
// ポケアイコン
typedef struct
{
  u32                monsno;  // 0のときなし
  u32                formno;
  BOOL               egg;
  UI_EASY_CLWK_RES   res;
  GFL_CLWK*          clwk;
  u8                 x;
  u8                 y;
  u8                 anim;
}
PI_DATA;

//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // グラフィック、フォントなど
  HEAPID                      heap_id;
  BTL_REC_SEL_GRAPHIC_WORK*      graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // シーケンス処理用 
  int                         fade_next_seq;
  int                         fade_mode;
  int                         fade_start_evy;
  int                         fade_end_evy;
  int                         fade_wait;

  int                         qa_next_seq;
  u32                         qa_str_id;
 
  int                         stm_next_seq;
  u32                         stm_str_id;

  int                         next_seq;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // メッセージ
  GFL_MSGDATA*                msgdata_rec;

  // ストリームテキストウィンドウ
  PRINT_STREAM*               text_stream;
  GFL_TCBLSYS*                text_tcblsys;
  GFL_BMPWIN*                 text_winin_bmpwin;
  GFL_BMPWIN*                 text_dummy_bmpwin;      ///< 0番のキャラクターを1x1でつくっておく
  GFL_ARCUTIL_TRANSINFO       text_winfrm_tinfo;
  STRBUF*                     text_strbuf;

  // はい・いいえウィンドウ
  BMPWIN_YESNO_DAT            yn_dat;
  BMPMENU_WORK*               yn_wk;

  // 固定テキスト
  GFL_BMPWIN*                 fix_bmpwin[FIX_MAX];
  u32                         fix_frame;
  u32                         fix_wait_count;
  BOOL                        fix_timeup;  // TRUEのとき時間切れ

  // ポケアイコン
  PI_DATA                     pi_data[PI_POS_MAX][PI_PARTY_NUM];

  // バトルモードから表示するテキストID、ポケアイコンの並べ方を決める
  u32                         battle_mode_str_id;
  BOOL                        battle_mode_arrange_two;

  // BattleRec BATTLE_REC
  BATTLE_REC_SAVEDATA*        battle_rec_savedata;
  BATTLE_MODE                 battle_rec_mode;
  int                         battle_rec_new_mode;
  u16                         battle_rec_new_work0;
  u16                         battle_rec_new_work1;
  SAVE_RESULT                 battle_rec_new_save_result;

  // BG Main
  GFL_ARCUTIL_TRANSINFO       bg_m_tinfo;
  
  // BG Sub
  GFL_ARCUTIL_TRANSINFO       bg_s_tinfo;
}
BTL_REC_SEL_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// シーケンス処理用
static void Btl_Rec_Sel_ChangeSeqFade( int* seq,
                BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, int mode, int start_evy, int end_evy, int wait );
static void Btl_Rec_Sel_ChangeSeqQa( int* seq,
                BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, u32 str_id );
static void Btl_Rec_Sel_NoChangeSeqQa( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, u32 str_id );
static void Btl_Rec_Sel_ChangeSeqStm( int* seq,
                BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, u32 str_id );
static void Btl_Rec_Sel_NoChangeSeqStm( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, u32 str_id );

// VBlank関数
static void Btl_Rec_Sel_VBlankFunc( GFL_TCB* tcb, void* wk );

// ストリームテキストウィンドウ
static void Btl_Rec_Sel_TextInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_TextExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_TextMain( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_TextShowWinFrm( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_TextClearWinIn( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );

static void Btl_Rec_Sel_TextStartStream( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                                     u32 str_id );
static BOOL Btl_Rec_Sel_TextWaitStream( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );

// はい・いいえウィンドウ
static void Btl_Rec_Sel_YnInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_YnExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_YnStartSel(  BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );

// 固定テキスト
static void Btl_Rec_Sel_FixInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_FixExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_FixMain( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_FixShowOnPre( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_FixShowOffPre( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_FixStartTime( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work, u16 sec );
static void Btl_Rec_Sel_FixEndTime( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_FixUpdateTime( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work, u16 sec );

// ポケアイコン
static void Btl_Rec_Sel_PiInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static void Btl_Rec_Sel_PiExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
static GFL_CLWK* CreatePokeicon( GFL_CLUNIT* clunit, HEAPID heap_id, CLSYS_DRAW_TYPE draw_type, u8 pltt_line,
                     u32 monsno, u32 formno, BOOL egg, UI_EASY_CLWK_RES* res, u8 x, u8 y, u8 anim );
static void DeletePokeicon( UI_EASY_CLWK_RES* res, GFL_CLWK* clwk );

// バトルモードから表示するテキストID、ポケアイコンの並べ方を決める
static void Btl_Rec_Sel_DecideFromBattleMode( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );

// BG Main
void Btl_Rec_Sel_BgMInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
void Btl_Rec_Sel_BgMExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
void Btl_Rec_Sel_BgMCreateNon( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
void Btl_Rec_Sel_BgMCreateVs2( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
void Btl_Rec_Sel_BgMCreateVs4( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
  
// BG Sub
void Btl_Rec_Sel_BgSInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );
void Btl_Rec_Sel_BgSExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Btl_Rec_Sel_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Btl_Rec_Sel_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Btl_Rec_Sel_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    BTL_REC_SEL_ProcData =
{
  Btl_Rec_Sel_ProcInit,
  Btl_Rec_Sel_ProcMain,
  Btl_Rec_Sel_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief       PROC パラメータ生成
 *
 *  @param[in]   heap_id       ヒープID
 *  @param[in]   gamedata      GAMEDATA
 *  @param[in]   b_rec         録画セーブ可能のときTRUE
 *
 *  @retval      BTL_REC_SEL_PARAM
 */
//------------------------------------------------------------------
BTL_REC_SEL_PARAM*  BTL_REC_SEL_AllocParam(
                            HEAPID           heap_id,
                            GAMEDATA*        gamedata,
                            BOOL             b_rec )
{
  BTL_REC_SEL_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof( BTL_REC_SEL_PARAM ) );
  BTL_REC_SEL_InitParam( param, gamedata, b_rec );
  return param;
}

//------------------------------------------------------------------
/**
 *  @brief           PROC パラメータ解放
 *
 *  @param[in,out]   param      BTL_REC_SEL_AllocParamで生成したもの
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             BTL_REC_SEL_FreeParam(
                            BTL_REC_SEL_PARAM*  param )
{
  GFL_HEAP_FreeMemory( param );
}

//------------------------------------------------------------------
/**
 *  @brief           PROC パラメータを設定する
 *
 *  @param[in,out]   param      BTL_REC_SEL_PARAM
 *  @param[in]       gamedata   GAMEDATA
 *  @param[in]       b_rec      録画セーブ可能のときTRUE
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             BTL_REC_SEL_InitParam(
                            BTL_REC_SEL_PARAM*  param,
                            GAMEDATA*           gamedata,
                            BOOL                b_rec )
{
  param->gamedata    = gamedata;
  param->b_rec       = b_rec;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Btl_Rec_Sel_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  BTL_REC_SEL_PARAM*    param    = (BTL_REC_SEL_PARAM*)pwk;
  BTL_REC_SEL_WORK*     work;

  // オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));

  // ヒープ
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BTL_REC_SEL, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(BTL_REC_SEL_WORK), HEAPID_BTL_REC_SEL );
    GFL_STD_MemClear( work, sizeof(BTL_REC_SEL_WORK) );
    
    work->heap_id       = HEAPID_BTL_REC_SEL;
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

  // グラフィック、フォントなど
  {
    work->graphic       = BTL_REC_SEL_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // VBlank中TCB
  {
    work->vblank_tcb = GFUser_VIntr_CreateTCB( Btl_Rec_Sel_VBlankFunc, work, 1 );
  }

  // メッセージ
  work->msgdata_rec     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_b_record_dat, work->heap_id );

  // ストリームテキストウィンドウ
  Btl_Rec_Sel_TextInit( param, work );
  Btl_Rec_Sel_TextShowWinFrm( param, work );
  // はい・いいえウィンドウ
  Btl_Rec_Sel_YnInit( param, work );
  // 固定テキスト
  Btl_Rec_Sel_FixInit( param, work );
  // BG Main
  Btl_Rec_Sel_BgMInit( param, work ); 
  Btl_Rec_Sel_BgMCreateNon( param, work ); 
  // BG Sub
  Btl_Rec_Sel_BgSInit( param, work ); 
 
  // プライオリティ、表示、背景色など
  {
    GFL_BG_SetPriority( BG_FRAME_M_BACK              , 2 );
    GFL_BG_SetPriority( BG_FRAME_M_TIME              , 1 );
    GFL_BG_SetPriority( BG_FRAME_M_TEXT              , 0 );  // 最前面

    GFL_BG_SetPriority( BG_FRAME_S_BACK              , 0 );  // 最前面

    GFL_BG_SetVisible( BG_FRAME_M_BACK               , VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_M_TIME               , VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_M_TEXT               , VISIBLE_ON );
  
    GFL_BG_SetVisible( BG_FRAME_S_BACK               , VISIBLE_ON );

    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );
    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x0000 );
  }

  // フェードイン(黒16→見える0)する前に画面を黒にしておく
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, 0 );

  // シーケンス処理用
  if( param->b_rec )
  {
    Btl_Rec_Sel_ChangeSeqFade( seq, param, work, SEQ_QA_INIT,
        GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );
    Btl_Rec_Sel_NoChangeSeqQa( param, work, SEQ_QA_ANS_REC, msg_record_01_01 );
  }
  else
  {
    Btl_Rec_Sel_ChangeSeqFade( seq, param, work, SEQ_WAIT_INIT,
        GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Btl_Rec_Sel_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  BTL_REC_SEL_PARAM*    param    = (BTL_REC_SEL_PARAM*)pwk;
  BTL_REC_SEL_WORK*     work     = (BTL_REC_SEL_WORK*)mywk;

  // BG Sub
  Btl_Rec_Sel_BgSExit( param, work ); 
  // BG Main
  Btl_Rec_Sel_BgMExit( param, work ); 
  // 固定テキスト
  Btl_Rec_Sel_FixExit( param, work );
  // はい・いいえウィンドウ
  Btl_Rec_Sel_YnExit( param, work );
  // ストリームテキストウィンドウ
  Btl_Rec_Sel_TextExit( param, work );

  // メッセージ
  GFL_MSG_Delete( work->msgdata_rec );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    BTL_REC_SEL_GRAPHIC_Exit( work->graphic );
  }

  // ヒープ
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_BTL_REC_SEL );
  }

	// オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Btl_Rec_Sel_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  BTL_REC_SEL_PARAM*    param    = (BTL_REC_SEL_PARAM*)pwk;
  BTL_REC_SEL_WORK*     work     = (BTL_REC_SEL_WORK*)mywk;

  SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( param->gamedata );

  int key_trg = GFL_UI_KEY_GetTrg();

  switch(*seq)
  {
  case SEQ_FADE_INIT:
    {
      // フェードイン(黒16→見える0) / フェードアウト(見える0→黒16)
      GFL_FADE_SetMasterBrightReq( work->fade_mode, work->fade_start_evy, work->fade_end_evy, work->fade_wait );
      (*seq) = SEQ_FADE;
    }
    break;
  case SEQ_FADE:
    { 
      if( !GFL_FADE_CheckFade() )
        (*seq) = work->fade_next_seq;
    }
    break;
  case SEQ_QA_INIT:
    {
      Btl_Rec_Sel_TextStartStream( param, work, work->qa_str_id );
      (*seq) = SEQ_QA_QUES;
    }
    break;
  case SEQ_QA_QUES:
    {
      if( Btl_Rec_Sel_TextWaitStream( param, work ) )
      {
        Btl_Rec_Sel_YnStartSel( param, work );
        Btl_Rec_Sel_FixStartTime( param, work, 30 );
        (*seq) = work->qa_next_seq;
      }
    }
    break;
  case SEQ_STM_INIT:
    {
      Btl_Rec_Sel_TextStartStream( param, work, work->stm_str_id );
      (*seq) = work->stm_next_seq;
    }
    break;
  case SEQ_QA_ANS_REC:
    {
      u32 ret = BmpMenu_YesNoSelectMain( work->yn_wk );
      if( ret != BMPMENU_NULL )
      {
        Btl_Rec_Sel_FixEndTime( param, work );
      }
      else
      {
        if( work->fix_timeup )
        {
          ret = BMPMENU_CANCEL;
          BmpMenu_YesNoMenuExit( work->yn_wk );
        }
      }

      if( ret != BMPMENU_NULL )
      {
        if( ret == 0 )
        {
          LOAD_RESULT result;
#ifdef OFFLINE_TEST
          BOOL ret = TRUE; 
#else
          BOOL ret = BattleRec_DataOccCheck( sv, work->heap_id, &result, LOADDATA_MYREC ); 
          // resultがRECLOAD_RESULT_OKならTRUEが返ってくるようだ
#endif

          if( ret )  // 録画データがある場合
            Btl_Rec_Sel_ChangeSeqFade( seq, param, work, SEQ_PRE_SHOW_ON,
                GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, INSIDE_FADE_OUT_WAIT );
          else
            (*seq) = SEQ_SAVE_INIT;
        }
        else
          Btl_Rec_Sel_ChangeSeqQa( seq, param, work, SEQ_QA_ANS_NOREC, msg_record_02_01 );
      }
    }
    break;
  case SEQ_QA_ANS_NOREC:
    {
      u32 ret = BmpMenu_YesNoSelectMain( work->yn_wk );
      if( ret != BMPMENU_NULL )
      {
        Btl_Rec_Sel_FixEndTime( param, work );
      }
      else
      {
        if( work->fix_timeup )
        {
          ret = BMPMENU_CANCEL;
          BmpMenu_YesNoMenuExit( work->yn_wk );
        }
      }

      if( ret != BMPMENU_NULL )
      {
        if( ret == 0 )
          (*seq) = SEQ_WAIT_INIT;
        else
          Btl_Rec_Sel_ChangeSeqQa( seq, param, work, SEQ_QA_ANS_REC, msg_record_01_01 );
      }
    }
    break;
  case SEQ_QA_ANS_PRE:
    {
      u32 ret = BmpMenu_YesNoSelectMain( work->yn_wk );
      if( ret != BMPMENU_NULL )
      {
        Btl_Rec_Sel_FixEndTime( param, work );
      }
      else
      {
        if( work->fix_timeup )
        {
          ret = BMPMENU_CANCEL;
          BmpMenu_YesNoMenuExit( work->yn_wk );
        }
      }

      if( ret != BMPMENU_NULL )
      {
        if( ret == 0 )
          work->next_seq = SEQ_SAVE_INIT;
        else
          work->next_seq = SEQ_WAIT_INIT;
        Btl_Rec_Sel_ChangeSeqFade( seq, param, work, SEQ_PRE_SHOW_OFF,
           GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, INSIDE_FADE_OUT_WAIT );
      }
    }
    break;
  case SEQ_PRE_SHOW_ON:
    {
#ifdef OFFLINE_TEST
      u8 i, j;
 
      // サンプルデータ作成
      {
        for( j=0; j<PI_POS_MAX; j++ )
        {
          for( i=0; i<PI_PARTY_NUM; i++ )
          {
            work->pi_data[j][i].monsno = PI_PARTY_NUM * j + i;
            work->pi_data[j][i].formno = 0;
          }
        }
        work->battle_rec_mode = BATTLE_MODE_COLOSSEUM_SINGLE_FREE;
        //work->battle_rec_mode = BATTLE_MODE_COLOSSEUM_MULTI_FREE;
      }
      Btl_Rec_Sel_DecideFromBattleMode( param, work );
      Btl_Rec_Sel_FixShowOnPre( param, work );
      Btl_Rec_Sel_PiInit( param, work );
      if( work->battle_mode_arrange_two )
        Btl_Rec_Sel_BgMCreateVs2( param, work );
      else
        Btl_Rec_Sel_BgMCreateVs4( param, work );


      Btl_Rec_Sel_ChangeSeqFade( seq, param, work, SEQ_QA_INIT,
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, INSIDE_FADE_IN_WAIT );
      Btl_Rec_Sel_NoChangeSeqQa( param, work, SEQ_QA_ANS_PRE, msg_record_03_01 );
      
      Btl_Rec_Sel_TextClearWinIn( param, work );
#else
      // ここで録画データを表示
      LOAD_RESULT result;
      work->battle_rec_savedata = BattleRec_LoadAlloc( sv, work->heap_id, &result, LOADDATA_MYREC );
      // 必ずTRUEが返ってくる 

      if( result == RECLOAD_RESULT_OK )
      {
        BATTLE_REC_HEADER_PTR head = BattleRec_HeaderPtrGetWork( work->battle_rec_savedata );
        u64 hp;
        int i;
        for( i=0; i<HEADER_MONSNO_MAX; i++ )
        {
          u32 monsno;
          u32 formno = 0;
          hp = RecHeader_ParamGet( head, RECHEAD_IDX_MONSNO, i );
          monsno = (u32)hp;
          if( monsno != 0 )
          {
            hp = RecHeader_ParamGet( head, RECHEAD_IDX_FORM_NO, i );
            formno = (u32)hp;
          }
          work->pi_data[i/PI_PARTY_NUM][i%PI_PARTY_NUM].monsno = monsno;
          work->pi_data[i/PI_PARTY_NUM][i%PI_PARTY_NUM].formno = formno;
        }
        hp = RecHeader_ParamGet( head, RECHEAD_IDX_MODE, i );
        work->battle_rec_mode = (int)hp;

        Btl_Rec_Sel_DecideFromBattleMode( param, work );
        Btl_Rec_Sel_FixShowOnPre( param, work );
        Btl_Rec_Sel_PiInit( param, work );
        if( work->battle_mode_arrange_two )
          Btl_Rec_Sel_BgMCreateVs2( param, work );
        else
          Btl_Rec_Sel_BgMCreateVs4( param, work );

        Btl_Rec_Sel_ChangeSeqFade( seq, param, work, SEQ_QA_INIT,
            GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, INSIDE_FADE_IN_WAIT );
        Btl_Rec_Sel_NoChangeSeqQa( param, work, SEQ_QA_ANS_PRE, msg_record_03_01 );
      }
      else
      {
        Btl_Rec_Sel_ChangeSeqFade( seq, param, work, SEQ_SAVE_INIT,
            GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, INSIDE_FADE_IN_WAIT );
      }

      BattleRec_ExitWork( work->battle_rec_savedata );
      
      Btl_Rec_Sel_TextClearWinIn( param, work );
#endif
    }
    break;
  case SEQ_PRE_SHOW_OFF:
    {
      // ここで録画データを非表示
      Btl_Rec_Sel_FixShowOffPre( param, work );
      Btl_Rec_Sel_PiExit( param, work );
      Btl_Rec_Sel_BgMCreateNon( param, work );
      
      Btl_Rec_Sel_ChangeSeqFade( seq, param, work, work->next_seq,
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, INSIDE_FADE_IN_WAIT );
      
      Btl_Rec_Sel_TextClearWinIn( param, work );
    }
    break;
  case SEQ_SAVE_INIT:
    {
      // ここでセーブ開始
      Btl_Rec_Sel_TextStartStream( param, work, msg_record_04_01 );

      work->battle_rec_new_mode = 0;
      work->battle_rec_new_work0 = 0;
      work->battle_rec_new_work1 = 0;
      work->battle_rec_new_save_result = SAVE_RESULT_CONTINUE;

      (*seq) = SEQ_SAVE;
    }
    break;
  case SEQ_SAVE:
    {
#ifdef OFFLINE_TEST
      work->battle_rec_new_save_result = SAVE_RESULT_OK;
#else
      if( work->battle_rec_new_save_result != SAVE_RESULT_OK && work->battle_rec_new_save_result != SAVE_RESULT_NG )
      {
        work->battle_rec_new_save_result = BattleRec_Save( sv, work->heap_id,
            work->battle_rec_new_mode, 0, LOADDATA_MYREC,  // fight_countはバトルサブウェイ以外では使わないので0でいい
            &(work->battle_rec_new_work0), &(work->battle_rec_new_work1) );
      }
#endif

      if( Btl_Rec_Sel_TextWaitStream( param, work ) )
      {
        if( work->battle_rec_new_save_result == SAVE_RESULT_OK )  // セーブ正常終了
        {
          Btl_Rec_Sel_ChangeSeqStm( seq, param, work, SEQ_SAVEOK, msg_record_05_01 );
        }
        else if( work->battle_rec_new_save_result == SAVE_RESULT_NG )  // セーブ異常終了
        {
          Btl_Rec_Sel_ChangeSeqStm( seq, param, work, SEQ_SAVENG, msg_record_06_01 );
        }
      }
    }
    break;
  case SEQ_SAVEOK:
    {
      if( Btl_Rec_Sel_TextWaitStream( param, work ) )
        (*seq) = SEQ_WAIT_INIT;
    }
    break;
  case SEQ_SAVENG:
    {
      if( Btl_Rec_Sel_TextWaitStream( param, work ) )
        (*seq) = SEQ_WAIT_INIT;
    }
    break;
  case SEQ_WAIT_INIT:
    {
      Btl_Rec_Sel_TextStartStream( param, work, msg_record_07_01 );
      {
#ifndef OFFLINE_TEST
        GFL_NETHANDLE* nethandle = GFL_NET_HANDLE_GetCurrentHandle();
        GFL_NET_TimingSyncStart( nethandle, BTL_REC_SEL_NET_TIMING_SYNC_NO );
#endif
      }
      (*seq) = SEQ_WAIT;
    }
    break;
  case SEQ_WAIT:
    {
      if( Btl_Rec_Sel_TextWaitStream( param, work ) )
      {
        BOOL ret = TRUE;
#ifndef OFFLINE_TEST
        GFL_NETHANDLE* nethandle = GFL_NET_HANDLE_GetCurrentHandle();
        ret = GFL_NET_IsTimingSync( nethandle, BTL_REC_SEL_NET_TIMING_SYNC_NO );
#endif
        if( ret )  // 相手待ち終了
        {
          Btl_Rec_Sel_ChangeSeqFade( seq, param, work, SEQ_END,
              GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );
        }
      }
    }
    break;
  case SEQ_END:
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  Btl_Rec_Sel_TextMain( param, work );
  Btl_Rec_Sel_FixMain( param, work );

  PRINTSYS_QUE_Main( work->print_que );

  // 2D描画
  BTL_REC_SEL_GRAPHIC_2D_Draw( work->graphic );
  // 3D描画
  BTL_REC_SEL_GRAPHIC_3D_StartDraw( work->graphic );
  // ここに挿入 
  BTL_REC_SEL_GRAPHIC_3D_EndDraw( work->graphic );

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// シーケンス処理用
//=====================================
static void Btl_Rec_Sel_ChangeSeqFade( int* seq,
                BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, int mode, int start_evy, int end_evy, int wait )
{
  (*seq) = SEQ_FADE_INIT;
  work->fade_next_seq    = next_seq;
  work->fade_mode        = mode;
  work->fade_start_evy   = start_evy;
  work->fade_end_evy     = end_evy;
  work->fade_wait        = wait;
}

static void Btl_Rec_Sel_ChangeSeqQa( int* seq,
                BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, u32 str_id )
{
  (*seq) = SEQ_QA_INIT;
  Btl_Rec_Sel_NoChangeSeqQa( param, work, next_seq, str_id );
}
static void Btl_Rec_Sel_NoChangeSeqQa( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, u32 str_id )
{
  work->qa_next_seq    = next_seq;
  work->qa_str_id      = str_id;
}

static void Btl_Rec_Sel_ChangeSeqStm( int* seq,
                BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, u32 str_id )
{
  (*seq) = SEQ_STM_INIT;
  Btl_Rec_Sel_NoChangeSeqStm( param, work, next_seq, str_id );
}
static void Btl_Rec_Sel_NoChangeSeqStm( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                int next_seq, u32 str_id )
{
  work->stm_next_seq     = next_seq;
  work->stm_str_id       = str_id;
}

//-------------------------------------
/// VBlank関数
//=====================================
static void Btl_Rec_Sel_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  BTL_REC_SEL_WORK* work = (BTL_REC_SEL_WORK*)wk;

}

//-------------------------------------
/// TEXT
//=====================================
static void Btl_Rec_Sel_TextInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  // パレット
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
      BG_PAL_POS_M_TEXT_FONT * 0x20, BG_PAL_NUM_M_TEXT_FONT * 0x20, work->heap_id );

  // BGフレームのスクリーンの空いている箇所に何も表示がされないようにしておく
  work->text_dummy_bmpwin = GFL_BMPWIN_Create( BG_FRAME_M_TEXT, 0, 0, 1, 1,
                                BG_PAL_POS_M_TEXT_FONT, GFL_BMP_CHRAREA_GET_F );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_dummy_bmpwin), 0 );
  GFL_BMPWIN_TransVramCharacter(work->text_dummy_bmpwin);

  // ウィンドウ内
  work->text_winin_bmpwin = GFL_BMPWIN_Create( BG_FRAME_M_TEXT, 1, 19, 30, 4,
                                 BG_PAL_POS_M_TEXT_FONT, GFL_BMP_CHRAREA_GET_F );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), TEXT_WININ_BACK_COLOR );
  GFL_BMPWIN_TransVramCharacter(work->text_winin_bmpwin);
    
  // ウィンドウ枠
  work->text_winfrm_tinfo = BmpWinFrame_GraphicSetAreaMan( BG_FRAME_M_TEXT,
                                BG_PAL_POS_M_TEXT_FRAME,
                                MENU_TYPE_SYSTEM, work->heap_id );
  
  // ウィンドウ内、ウィンドウ枠どちらの表示もまだ行わないので、初期化ではここまで。

  // TCBL、フォントカラー、転送など
  work->text_tcblsys = GFL_TCBL_Init( work->heap_id, work->heap_id, 1, 0 );
  GFL_FONTSYS_SetColor( 1, 2, TEXT_WININ_BACK_COLOR );
  GFL_BG_LoadScreenV_Req( BG_FRAME_M_TEXT );

  // NULL初期化
  work->text_stream       = NULL;
  work->text_strbuf       = NULL;
}
static void Btl_Rec_Sel_TextExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  if( work->text_stream ) PRINTSYS_PrintStreamDelete( work->text_stream );
  if( work->text_strbuf ) GFL_STR_DeleteBuffer( work->text_strbuf );
  GFL_TCBL_Exit( work->text_tcblsys );
  GFL_BG_FreeCharacterArea( BG_FRAME_M_TEXT,
      GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
      GFL_ARCUTIL_TRANSINFO_GetSize(work->text_winfrm_tinfo) );
  GFL_BMPWIN_Delete( work->text_winin_bmpwin );
  GFL_BMPWIN_Delete( work->text_dummy_bmpwin );
}
static void Btl_Rec_Sel_TextMain( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  GFL_TCBL_Main( work->text_tcblsys );
}
static void Btl_Rec_Sel_TextShowWinFrm( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  BmpWinFrame_Write( work->text_winin_bmpwin, WINDOW_TRANS_ON_V,
      GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
      BG_PAL_POS_M_TEXT_FRAME );
  GFL_BMPWIN_MakeTransWindow_VBlank( work->text_winin_bmpwin );
}
static void Btl_Rec_Sel_TextClearWinIn( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), TEXT_WININ_BACK_COLOR );
  GFL_BMPWIN_MakeTransWindow_VBlank( work->text_winin_bmpwin );
}
static void Btl_Rec_Sel_TextStartStream( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work,
                                     u32 str_id )
{
  // 一旦消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), TEXT_WININ_BACK_COLOR );
  
  // 前のを消す
  if( work->text_stream ) PRINTSYS_PrintStreamDelete( work->text_stream );  // この関数でタスクも削除してくれるので、同時に動くタスクは1つで済む
  if( work->text_strbuf ) GFL_STR_DeleteBuffer( work->text_strbuf );

  // 文字列作成
  {
    work->text_strbuf  = GFL_MSG_CreateString( work->msgdata_rec, str_id );
  }
  
  // ストリーム開始
  work->text_stream = PRINTSYS_PrintStream(
                          work->text_winin_bmpwin,
                          0, 0,
                          work->text_strbuf,
                          work->font, MSGSPEED_GetWait(),
                          work->text_tcblsys, 2,
                          work->heap_id,
                          TEXT_WININ_BACK_COLOR );
}
static BOOL Btl_Rec_Sel_TextWaitStream( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  BOOL ret = FALSE;

  switch( PRINTSYS_PrintStreamGetState( work->text_stream ) )
  { 
  case PRINTSTREAM_STATE_PAUSE:
    if( ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ) || GFL_UI_TP_GetTrg() )
    { 
      PRINTSYS_PrintStreamReleasePause( work->text_stream );
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
/// はい・いいえウィンドウ
//=====================================
static void Btl_Rec_Sel_YnInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  // この関数はBtl_Rec_Sel_TextInitが済んで
  // BG_PAL_POS_M_TEXT_FONTが設定されている状態で呼ぶこと
  // GFL_FONTSYS_SetColorの設定も必要かもしれない

  u32 size = GFL_BG_1CHRDATASIZ * ( 7*4 +12 );  // system/bmp_menu.cのBmpMenu_YesNoSelectInitを参考にしたサイズ + 少し大きめに
  u32 pos = GFL_BG_AllocCharacterArea( BG_FRAME_M_TEXT, size, GFL_BG_CHRAREA_GET_B );
  GF_ASSERT_MSG( pos != AREAMAN_POS_NOTFOUND, "BTL_REC_SEL : BGキャラ領域が足りませんでした。\n" );
  GFL_BG_FreeCharacterArea( BG_FRAME_M_TEXT, pos, size );

  work->yn_dat.frmnum    = BG_FRAME_M_TEXT;
  work->yn_dat.pos_x     = 24;
  work->yn_dat.pos_y     = 13;
  work->yn_dat.palnum    = BG_PAL_POS_M_TEXT_FONT;//BG_PAL_POS_M_YN;
  work->yn_dat.chrnum    = pos;  // 使われていないようだ
}
static void Btl_Rec_Sel_YnExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  // BmpMenu_YesNoSelectMainが解放してくれるので、何もしなくてよい
}
static void Btl_Rec_Sel_YnStartSel(  BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  // この関数はBtl_Rec_Sel_TextInitが済んで
  // work->text_winfrm_tinfoが設定されている状態で呼ぶこと

  work->yn_wk = BmpMenu_YesNoSelectInit(
                    &work->yn_dat,
                    GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
                    BG_PAL_POS_M_TEXT_FRAME,
                    0,
                    work->heap_id );
}

//-------------------------------------
/// 固定テキスト
//=====================================
static void Btl_Rec_Sel_FixInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  // この関数はBtl_Rec_Sel_TextInitが済んで
  // BG_PAL_POS_M_TEXT_FONTが設定されている状態で呼ぶこと
  // GFL_FONTSYS_SetColorの設定も必要かもしれない

  typedef struct
  {
    u8 frmnum;
    u8 posx;
    u8 posy;
    u8 sizx;
    u8 sizy;
    u8 dir;
  }
  FIX_DATA;
  FIX_DATA fix_data[FIX_MAX] =
  {
    { BG_FRAME_M_TEXT,  1,  0, 14, 2, GFL_BMP_CHRAREA_GET_F },
    { BG_FRAME_M_TEXT,  2,  2, 30, 4, GFL_BMP_CHRAREA_GET_F },
    { BG_FRAME_M_TIME,  1, 15, 11, 3, GFL_BMP_CHRAREA_GET_B },  // ダミーのキャラをつくるのが面倒なので後ろから確保
    { BG_FRAME_M_TIME, 14, 15,  5, 3, GFL_BMP_CHRAREA_GET_B },//{ 14, 15,  2, 3 },
  };
  u8 i;

  for( i=0; i<FIX_MAX; i++ )
  {
    work->fix_bmpwin[i] = GFL_BMPWIN_Create( fix_data[i].frmnum,
                        fix_data[i].posx, fix_data[i].posy, fix_data[i].sizx, fix_data[i].sizy,
                        BG_PAL_POS_M_TEXT_FONT, fix_data[i].dir );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->fix_bmpwin[i]), 0 );
  }
}
static void Btl_Rec_Sel_FixExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  u8 i;
  for( i=0; i<FIX_MAX; i++ )
  {
    GFL_BMPWIN_Delete( work->fix_bmpwin[i] );
  }
}
static void Btl_Rec_Sel_FixMain( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  u8 i;

  if( work->fix_frame >= FPS || work->fix_wait_count > 0 )
  {
    // 残り時間表示を更新する
    {
      u32 sec = work->fix_frame / FPS;
      Btl_Rec_Sel_FixUpdateTime(param, work, sec);
    }

    // 時間を進める
    if( work->fix_frame >= FPS )
    {
      work->fix_frame--;
      if( work->fix_frame == (FPS-1) )
      {
        // 時間切れ
        work->fix_timeup = TRUE;
      }
    }
    else
    {
      work->fix_wait_count--;
      if( work->fix_wait_count == 0 )
      {
        // SELECT TIME ??を非表示にする
        for( i=FIX_SEL_TIME; i<=FIX_TIME; i++ )
        {
          GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->fix_bmpwin[i]), 0 );
          GFL_BMPWIN_MakeTransWindow_VBlank( work->fix_bmpwin[i] );
        }
      }
    }
  }
}
static void Btl_Rec_Sel_FixShowOnPre( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  STRBUF* strbuf;
  GFL_BMP_DATA* bmp_data;
  u8 i;

  // 前回の記録
  {
    strbuf = GFL_MSG_CreateString( work->msgdata_rec, msg_record_10_01 );
    bmp_data = GFL_BMPWIN_GetBmp( work->fix_bmpwin[FIX_PRE] );
    PRINTSYS_PrintQueColor( work->print_que, bmp_data, 0, 2, strbuf, work->font, PRINTSYS_LSB_Make(4,3,0) );
    GFL_STR_DeleteBuffer( strbuf );
  }

  // バトルモード
  // 例：コロシアム　シングル
  // 　　せいげんなし
  {
    strbuf = GFL_MSG_CreateString( work->msgdata_rec, work->battle_mode_str_id );
    bmp_data = GFL_BMPWIN_GetBmp( work->fix_bmpwin[FIX_PRE_TITLE] );
    PRINTSYS_PrintQueColor( work->print_que, bmp_data, 0, 2, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
    GFL_STR_DeleteBuffer( strbuf );
  }

  for( i=FIX_PRE; i<=FIX_PRE_TITLE; i++ )
  {
    GFL_BMPWIN_MakeTransWindow_VBlank( work->fix_bmpwin[i] );
  }
}
static void Btl_Rec_Sel_FixShowOffPre( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  u8 i;
  for( i=FIX_PRE; i<=FIX_PRE_TITLE; i++ )
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->fix_bmpwin[i]), 0 );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->fix_bmpwin[i] );
  }
}
static void Btl_Rec_Sel_FixStartTime( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work, u16 sec )
{
  work->fix_frame = sec * FPS + (FPS-1);
  work->fix_wait_count = FPS /2;
  work->fix_timeup = FALSE;

  Btl_Rec_Sel_FixUpdateTime( param, work, sec );
}
static void Btl_Rec_Sel_FixEndTime( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  u8 i;

  work->fix_frame = 0;
  work->fix_wait_count = 0;

  for( i=FIX_SEL_TIME; i<=FIX_TIME; i++ )
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->fix_bmpwin[i]), 0 );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->fix_bmpwin[i] );
  }
}
static void Btl_Rec_Sel_FixUpdateTime( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work, u16 sec )
{
  STRBUF* strbuf;
  GFL_BMP_DATA* bmp_data;
  PRINTSYS_LSB color;
  u8 i;

  // 一旦消去
  for( i=FIX_SEL_TIME; i<=FIX_TIME; i++ )
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->fix_bmpwin[i]), 0 );
  }

  if( sec > 10 )
  {
    color = PRINTSYS_LSB_Make(15,2,0);
  }
  else
  {
    color = PRINTSYS_LSB_Make(4,3,0);
  }

  // SELECT TIME
  {
    strbuf = GFL_MSG_CreateString( work->msgdata_rec, msg_record_time01 );
    bmp_data = GFL_BMPWIN_GetBmp( work->fix_bmpwin[FIX_SEL_TIME] );
    //PRINTSYS_PrintQueColor( work->print_que, bmp_data, 0, 6, strbuf, work->font, color );
    PRINTSYS_PrintColor( bmp_data, 0, 6, strbuf, work->font, color );
    GFL_STR_DeleteBuffer( strbuf );
  }

  // ??:??
/*
  {
    WORDSET* wordset = WORDSET_Create( work->heap_id );
    STRBUF* src_strbuf = GFL_MSG_CreateString( work->msgdata_rec, msg_record_time02 );
    strbuf = GFL_STR_CreateBuffer( STRBUF_FIX_TIME_LENGTH, work->heap_id );
    
    WORDSET_RegisterNumber( wordset, 0, 0, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wordset, 1, sec, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
   
    bmp_data = GFL_BMPWIN_GetBmp( work->fix_bmpwin[FIX_TIME] );
    PRINTSYS_PrintQueColor( work->print_que, bmp_data, 0, 6, strbuf, work->font, color );
   
    GFL_STR_DeleteBuffer( strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
    WORDSET_Delete( wordset );
  }
*/
  {
    WORDSET* wordset = WORDSET_Create( work->heap_id );
    STRBUF* src_strbuf = GFL_MSG_CreateString( work->msgdata_rec, msg_record_time03 );
    strbuf = GFL_STR_CreateBuffer( STRBUF_FIX_TIME_LENGTH, work->heap_id );
    
    WORDSET_RegisterNumber( wordset, 1, sec, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
   
    bmp_data = GFL_BMPWIN_GetBmp( work->fix_bmpwin[FIX_TIME] );
    //PRINTSYS_PrintQueColor( work->print_que, bmp_data, 0, 6, strbuf, work->font, color );
    PRINTSYS_PrintColor( bmp_data, 0, 6, strbuf, work->font, color );

    GFL_STR_DeleteBuffer( strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
    WORDSET_Delete( wordset );
  }

  for( i=FIX_SEL_TIME; i<=FIX_TIME; i++ )
  {
    GFL_BMPWIN_MakeTransWindow_VBlank( work->fix_bmpwin[i] );
  }
}

//-------------------------------------
/// ポケアイコン
//=====================================
static void Btl_Rec_Sel_PiInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  GFL_CLUNIT* clunit = BTL_REC_SEL_GRAPHIC_GetClunit( work->graphic );
  u8 i, j;
 
  // データ作成
  {
    for( j=0; j<PI_POS_MAX; j++ )
    {
      for( i=0; i<PI_PARTY_NUM; i++ )
      {
        //work->pi_data[j][i].monsno = PI_PARTY_NUM * j + i;
        //work->pi_data[j][i].formno = 0;
        work->pi_data[j][i].egg    = 0;
        work->pi_data[j][i].anim   = 1;
      }
    }
  }

  // 位置
  {
    for( j=0; j<PI_POS_MAX; j++ )
    {
      for( i=0; i<PI_PARTY_NUM; i++ )
      {
        if( work->battle_mode_arrange_two )  // 2人対戦
        {
          if( j == PI_POS_R )
          {
            work->pi_data[j][i].x      = 12*8 + 3*8*i;
            work->pi_data[j][i].y      =  7*8;
          }
          else
          {
            work->pi_data[j][i].x      =  1*8 + 3*8*i;
            work->pi_data[j][i].y      = 12*8;
          }
        }
        else  // 4人対戦
        {
          if( j == PI_POS_R )
          {
            if( i < 3 )
            {
              work->pi_data[j][i].x      = 20*8 + 3*8*i;
              work->pi_data[j][i].y      =  6*8;
            }
            else
            {
              work->pi_data[j][i].x      = 20*8 + 3*8*(i-3);
              work->pi_data[j][i].y      =  9*8;
            }
          }
          else
          {
            if( i< 3 )
            {
              work->pi_data[j][i].x      =  2*8 + 3*8*i;
              work->pi_data[j][i].y      =  9*8;
            }
            else
            {
              work->pi_data[j][i].x      =  2*8 + 3*8*(i-3);
              work->pi_data[j][i].y      = 12*8;
            }
          }
        }
      }
    }
  }

  // 生成
  for( j=0; j<PI_POS_MAX; j++ )
  {
    for( i=0; i<PI_PARTY_NUM; i++ )
    {
      if( work->pi_data[j][i].monsno != 0 )
      {
        work->pi_data[j][i].clwk = CreatePokeicon(
                                       clunit, work->heap_id, CLSYS_DRAW_MAIN, OBJ_PAL_POS_M_PI,
                                       work->pi_data[j][i].monsno, work->pi_data[j][i].formno,
                                       work->pi_data[j][i].egg, &(work->pi_data[j][i].res),
                                       work->pi_data[j][i].x, work->pi_data[j][i].y, work->pi_data[j][i].anim  );
      }
    }
  }
}
static void Btl_Rec_Sel_PiExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  u8 i, j;
  for( j=0; j<PI_POS_MAX; j++ )
  {
    for( i=0; i<PI_PARTY_NUM; i++ )
    {
      if( work->pi_data[j][i].monsno != 0 )
      {
        DeletePokeicon( &(work->pi_data[j][i].res), work->pi_data[j][i].clwk );
      }
    }
  }
}
static GFL_CLWK* CreatePokeicon( GFL_CLUNIT* clunit, HEAPID heap_id, CLSYS_DRAW_TYPE draw_type, u8 pltt_line,
                     u32 monsno, u32 formno, BOOL egg, UI_EASY_CLWK_RES* res, u8 x, u8 y, u8 anim )
{
  GFL_CLWK* clwk;

  UI_EASY_CLWK_RES_PARAM res_param;

  res_param.draw_type       = draw_type;
  res_param.comp_flg        = UI_EASY_CLWK_RES_COMP_NCLR;
  res_param.arc_id          = ARCID_POKEICON;
  res_param.pltt_id         = POKEICON_GetPalArcIndex();
  res_param.ncg_id          = POKEICON_GetCgxArcIndexByMonsNumber( monsno, formno, egg );
  res_param.cell_id         = POKEICON_GetCellArcIndex(); 
  res_param.anm_id          = POKEICON_GetAnmArcIndex();
  res_param.pltt_line       = pltt_line;
  res_param.pltt_src_ofs    = 0;
  res_param.pltt_src_num    = 3;
    
  UI_EASY_CLWK_LoadResource( res, &res_param, clunit, heap_id );

  // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
  clwk = UI_EASY_CLWK_CreateCLWK( res, clunit, x, y, anim, heap_id );

  // 上にアイテムアイコンを描画するので優先度を下げておく
  GFL_CLACT_WK_SetSoftPri( clwk, 1 );
  // オートアニメ ON
  GFL_CLACT_WK_SetAutoAnmFlag( clwk, TRUE );

  {
    u8 pal_num = POKEICON_GetPalNum( monsno, formno, egg );
    GFL_CLACT_WK_SetPlttOffs( clwk, pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  }

  return clwk;
}
static void DeletePokeicon( UI_EASY_CLWK_RES* res, GFL_CLWK* clwk )
{
  GFL_CLACT_WK_Remove( clwk );
  UI_EASY_CLWK_UnLoadResource( res );
}

//-------------------------------------
/// バトルモードから表示するテキストID、ポケアイコンの並べ方を決める
//=====================================
static void Btl_Rec_Sel_DecideFromBattleMode( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  // テキストID
  work->battle_mode_str_id = msg_record_title01 + work->battle_rec_mode;

  // ポケアイコンの並べ方
  if( BATTLE_MODE_COLOSSEUM_SINGLE_FREE <= work->battle_rec_mode && work->battle_rec_mode <= BATTLE_MODE_COLOSSEUM_SINGLE_50_SHOOTER )
  {
    work->battle_mode_arrange_two = TRUE;
  }
  else
  {
    work->battle_mode_arrange_two = FALSE;
  }
}

//-------------------------------------
/// BG Main
//=====================================
void Btl_Rec_Sel_BgMInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_BATT_REC_GRA, work->heap_id );

  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_batt_rec_gra_batt_rec_browse_bg_NCLR, PALTYPE_MAIN_BG,
                                    BG_PAL_POS_M_BACK * 0x20, BG_PAL_NUM_M_BACK * 0x20, work->heap_id );
  work->bg_m_tinfo = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( handle,
                            NARC_batt_rec_gra_batt_rec_data_NCGR,
                            BG_FRAME_M_BACK,
                            0,
                            FALSE, work->heap_id );
  GF_ASSERT_MSG( work->bg_m_tinfo != GFL_ARCUTIL_TRANSINFO_FAIL, "BTL_REC_SEL : BGキャラ領域が足りませんでした。\n" );

  GFL_ARC_CloseDataHandle( handle );
}
void Btl_Rec_Sel_BgMExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  GFL_BG_FreeCharacterArea( BG_FRAME_M_BACK,
                            GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_m_tinfo ),
                            GFL_ARCUTIL_TRANSINFO_GetSize( work->bg_m_tinfo ) );
}
void Btl_Rec_Sel_BgMCreateNon( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  //GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_batt_rec_gra_batt_rec_non_NSCR,
  //                                 BG_FRAME_M_BACK,
  //                                 GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_m_tinfo ),
  //                                 0,//32*32*GFL_BG_1SCRDATASIZ,
  //                                 FALSE, work->heap_id );

  GFL_ARC_UTIL_TransVramScreen( ARCID_BATT_REC_GRA,
                                NARC_batt_rec_gra_batt_rec_non_NSCR,
                                BG_FRAME_M_BACK,
                                GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_m_tinfo ),
                                0,
                                FALSE,
                                work->heap_id );
      
  //GFL_BG_SetScroll( BG_FRAME_M_BACK, GFL_BG_SCROLL_X_SET, 0 );
  //GFL_BG_SetScroll( BG_FRAME_M_BACK, GFL_BG_SCROLL_Y_SET, 0 );

  //GFL_BG_LoadScreenReq( BG_FRAME_M_BACK );
  GFL_BG_LoadScreenV_Req( BG_FRAME_M_BACK );
}
void Btl_Rec_Sel_BgMCreateVs2( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  GFL_ARC_UTIL_TransVramScreen( ARCID_BATT_REC_GRA,
                                NARC_batt_rec_gra_batt_rec_vs2_NSCR,
                                BG_FRAME_M_BACK,
                                GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_m_tinfo ),
                                0,
                                FALSE,
                                work->heap_id );
 
  GFL_BG_LoadScreenV_Req( BG_FRAME_M_BACK );
}
void Btl_Rec_Sel_BgMCreateVs4( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  GFL_ARC_UTIL_TransVramScreen( ARCID_BATT_REC_GRA,
                                NARC_batt_rec_gra_batt_rec_vs4_NSCR,
                                BG_FRAME_M_BACK,
                                GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_m_tinfo ),
                                0,
                                FALSE,
                                work->heap_id );
 
  GFL_BG_LoadScreenV_Req( BG_FRAME_M_BACK );
}
  
//-------------------------------------
/// BG Sub
//=====================================
void Btl_Rec_Sel_BgSInit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  // Cギア入手前の下画面を使う

  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, work->heap_id );

  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_c_gear_c_gear_NCLR, PALTYPE_SUB_BG,
                                    BG_PAL_POS_S_BACK * 0x20, 0/*BG_PAL_NUM_S_BACK * 0x20*/, work->heap_id );
  work->bg_s_tinfo = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( handle,
                            NARC_c_gear_c_gear_NCGR,
                            BG_FRAME_S_BACK,
                            0,//32*24*GFL_BG_1CHRDATASIZ,
                            FALSE, work->heap_id );
  GF_ASSERT_MSG( work->bg_s_tinfo != GFL_ARCUTIL_TRANSINFO_FAIL, "BTL_REC_SEL : BGキャラ領域が足りませんでした。\n" );
  GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_c_gear_c_gear01_n_NSCR,
                                   BG_FRAME_S_BACK,
                                   GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_s_tinfo ),
                                   0,//32*24*GFL_BG_1SCRDATASIZ,
                                   FALSE, work->heap_id );
  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenReq( BG_FRAME_S_BACK );
}
void Btl_Rec_Sel_BgSExit( BTL_REC_SEL_PARAM* param, BTL_REC_SEL_WORK* work )
{
  GFL_BG_FreeCharacterArea( BG_FRAME_S_BACK,
                            GFL_ARCUTIL_TRANSINFO_GetPos( work->bg_s_tinfo ),
                            GFL_ARCUTIL_TRANSINFO_GetSize( work->bg_s_tinfo ) );
}

