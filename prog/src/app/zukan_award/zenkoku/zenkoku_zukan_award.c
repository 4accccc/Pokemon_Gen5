//============================================================================
/**
 *  @file   zenkoku_zukan_award.c
 *  @brief  地方図鑑表彰状
 *  @author Koji Kawada
 *  @data   2010.03.08
 *  @note   
 *  モジュール名：ZENKOKU_ZUKAN_AWARD
 */
//============================================================================
//#define DEBUG_TEXT_MOVE  // これが定義されているとき、テキストを動かせる
//#define USE_OBJ_GF  // これが定義されているとき、OBJ_GFを使う


#define HEAPID_ZENKOKU_ZUKAN_AWARD (HEAPID_AWARD)


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "savedata/mystatus.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "app/app_nogear_subscreen.h"

#include "sound/wb_sound_data.sadl"
#include "sound/pm_sndsys.h"

#include "zenkoku_zukan_award_graphic.h"
#include "app/zenkoku_zukan_award.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "script_message.naix"
#include "msg/script/msg_award.h"
#include "shoujou.naix"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x30000)               ///< ヒープサイズ

// メインBGフレーム
#define BG_FRAME_M_FRONT       (GFL_BG_FRAME1_M)
#define BG_FRAME_M_TEXT        (GFL_BG_FRAME2_M)

// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_FRONT   (1)
#define BG_FRAME_PRI_M_TEXT    (0)

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_GRA_FRONT     = 15,
  BG_PAL_NUM_M_TEXT          = 1,
};
// 位置
enum
{
  BG_PAL_POS_M_GRA_FRONT    =  0,
  BG_PAL_POS_M_TEXT         = 15,
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_BALL        = 2,
  OBJ_PAL_NUM_M_GF          = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_M_BALL        = 0,
  OBJ_PAL_POS_M_GF          = 2,
};

// ProcMainのシーケンス
enum  // この並び順を利用しているところがあるので、並び順は変更しないこと。
{
  SEQ_START          = 0,
  SEQ_FADE_IN,
  SEQ_SCROLL,      // work->param->b_fixがFALSEのときしか来ない
  SEQ_SOUND_WAIT,  // work->param->b_fixがFALSEのときしか来ない
  SEQ_MAIN,
  SEQ_FADE_OUT,
  SEQ_END,
};

// テキスト
enum
{
  TEXT_DUMMY,
  TEXT_BLACK,
  TEXT_NAME,
  TEXT_MAIN,
  TEXT_STAFF,
  TEXT_MAX,
};

// BG_PAL_POS_M_TEXTの割り当て
#define TEXT_PAL_POS      (BG_PAL_POS_M_TEXT)
#define TEXT_COLOR_L      (1)  // 文字主色
#define TEXT_COLOR_S      (2)  // 文字影色
#define TEXT_COLOR_B      (0)  // 文字背景色(透明)
#define TEXT_COLOR_BLACK  (3)  // スクロールする際の空き領域を黒にしておく

// 文字の色
#define TEXT_COLOR_VALUE_L   (0x1D29)
#define TEXT_COLOR_VALUE_S   (0x3210)

static const u8 bmpwin_setup[TEXT_MAX][9] =  // MOJIMODE_HIRAGANA
{
  // frmnum           posx  posy  sizx  sizy  palnum                dir                    x  y (x,yは無視してセンタリングすることもある)
  {  BG_FRAME_M_TEXT,    0,    0,    1,    1, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_M_TEXT,    0,   24,    1,    1, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_M_TEXT,    7,    4,   20,    3, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },  // センタリングなら 6,    4,   20,    3,
  {  BG_FRAME_M_TEXT,    7,    8,   20,    9, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },  // センタリングなら 6,    8,   20,    9,
  {  BG_FRAME_M_TEXT,    6,   17,   20,    5, TEXT_PAL_POS,         GFL_BMP_CHRAREA_GET_F, 0, 0 },
};

static const u8 bmpwin_setup_kanji[TEXT_MAX][4] =  // MOJIMODE_KANJI
{
  // posx  posy   x  y (x,yは無視してセンタリングすることもある)
  {     0,    0,  0, 0 },
  {     0,   24,  0, 0 },
  {     7,    4,  0, 0 },  // センタリングなら{     6,    4,  0, 0 },
  {     6,    8,  0, 0 },
  {     6,   17,  0, 0 },
};

// フェード
#define FADE_IN_WAIT  (0)
#define FADE_OUT_WAIT (0)

// 文字数
#define TEXT_NAME_LEN_MAX    (32)  // EOMを含めた文字数

// スクロール
#define SCROLL_START_POS_Y  (64)
#define SCROLL_WAIT         ( 0)  // 0以上の値。0で毎フレーム移動。1で待ち、移動、待ち、移動。2で待ち、待ち、移動。
#define SCROLL_VALUE        ( 2)  // 移動するときの移動量。


// OBJ
enum
{
  OBJ_BALL_RES_NCG,
  OBJ_BALL_RES_NCL,
  OBJ_BALL_RES_NCE,
#ifdef USE_OBJ_GF
  OBJ_GF_RES_NCG,
  OBJ_GF_RES_NCL,
  OBJ_GF_RES_NCE,
#endif
  OBJ_RES_MAX,
};
// CELL
enum
{
  OBJ_BALL_CELL,
#ifdef USE_OBJ_GF
  OBJ_GF_CELL,
#endif
  OBJ_CELL_MAX,
};
#define OBJ_BALL_CELL_ANMSEQ  (0)
#ifdef USE_OBJ_GF
#define OBJ_GF_CELL_ANMSEQ    (0)
#endif
static const GFL_CLWK_DATA obj_cell_data[OBJ_CELL_MAX] =
{
  // pos_x, pos_y, anmseq,               softpri, bgpri
  {   18,    17,   OBJ_BALL_CELL_ANMSEQ, 0,       BG_FRAME_PRI_M_FRONT },
#ifdef USE_OBJ_GF
  {  175,   136,   OBJ_GF_CELL_ANMSEQ,   0,       BG_FRAME_PRI_M_FRONT },
#endif
};


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
  // ヒープ、パラメータなど
  HEAPID                      heap_id;
  ZENKOKU_ZUKAN_AWARD_PARAM*   param;
  
  // グラフィック、フォントなど
  ZENKOKU_ZUKAN_AWARD_GRAPHIC_WORK*      graphic;
  GFL_FONT*                             font;
  PRINT_QUE*                            print_que;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // テキスト
  GFL_MSGDATA*                msgdata;
  GFL_BMPWIN*                 text_bmpwin[TEXT_MAX];
  BOOL                        text_finish[TEXT_MAX];  // bmpwinの転送が済んでいればTRUE

  // スクロール
  u8                          scroll_wait_count;

  // OBJ
  u32                         obj_res[OBJ_RES_MAX];
  GFL_CLWK*                   obj_clwk[OBJ_CELL_MAX];
}
ZENKOKU_ZUKAN_AWARD_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zenkoku_Zukan_Award_VBlankFunc( GFL_TCB* tcb, void* wk );

// BG
static void Zenkoku_Zukan_Award_BgInit( ZENKOKU_ZUKAN_AWARD_WORK* work );
static void Zenkoku_Zukan_Award_BgExit( ZENKOKU_ZUKAN_AWARD_WORK* work );

// テキスト
static void Zenkoku_Zukan_Award_TextInit( ZENKOKU_ZUKAN_AWARD_WORK* work );
static void Zenkoku_Zukan_Award_TextExit( ZENKOKU_ZUKAN_AWARD_WORK* work );
static void Zenkoku_Zukan_Award_TextMain( ZENKOKU_ZUKAN_AWARD_WORK* work );

// 何もないところは黒にしておく
static void Zenkoku_Zukan_Award_BlackInit( ZENKOKU_ZUKAN_AWARD_WORK* work );

// OBJ
static void Zenkoku_Zukan_Award_ObjInit( ZENKOKU_ZUKAN_AWARD_WORK* work );
static void Zenkoku_Zukan_Award_ObjExit( ZENKOKU_ZUKAN_AWARD_WORK* work );

// スクロール
static void Zenkoku_Zukan_Award_ScrollInit( ZENKOKU_ZUKAN_AWARD_WORK* work );
static void Zenkoku_Zukan_Award_ScrollMain( ZENKOKU_ZUKAN_AWARD_WORK* work );
static BOOL Zenkoku_Zukan_Award_ScrollIsEnd( ZENKOKU_ZUKAN_AWARD_WORK* work );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Zenkoku_Zukan_Award_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Zenkoku_Zukan_Award_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Zenkoku_Zukan_Award_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    ZENKOKU_ZUKAN_AWARD_ProcData =
{
  Zenkoku_Zukan_Award_ProcInit,
  Zenkoku_Zukan_Award_ProcMain,
  Zenkoku_Zukan_Award_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           パラメータ生成
 *
 *  @param[in]       heap_id       ヒープID
 *  @param[in]       gamedata      自分状態データMYSTATUSを得たりするために必要(名前と性別を使用)
 *  @param[in]       b_fix         飾ってあるのを見るときTRUE
 *
 *  @retval          ZENKOKU_ZUKAN_AWARD_PARAM
 */
//------------------------------------------------------------------
ZENKOKU_ZUKAN_AWARD_PARAM*  ZENKOKU_ZUKAN_AWARD_AllocParam(
                                HEAPID               heap_id,
                                const GAMEDATA*      gamedata,
                                BOOL                 b_fix
                           )
{
  ZENKOKU_ZUKAN_AWARD_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof( ZENKOKU_ZUKAN_AWARD_PARAM ) );
  ZENKOKU_ZUKAN_AWARD_InitParam(
      param,
      gamedata,
      b_fix );
  return param;
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータ解放
 *
 *  @param[in,out]   param      パラメータ生成で生成したもの
 *
 *  @retval          
 */
//------------------------------------------------------------------
void            ZENKOKU_ZUKAN_AWARD_FreeParam(
                            ZENKOKU_ZUKAN_AWARD_PARAM*  param )
{
  GFL_HEAP_FreeMemory( param );
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータを設定する
 *
 *  @param[in,out]   param         ZENKOKU_ZUKAN_AWARD_PARAM
 *  @param[in]       gamedata      自分状態データMYSTATUSを得たりするために必要(名前と性別を使用)
 *  @param[in]       b_fix         飾ってあるのを見るときTRUE
 *
 *  @retval          
 */
//------------------------------------------------------------------
void  ZENKOKU_ZUKAN_AWARD_InitParam(
                  ZENKOKU_ZUKAN_AWARD_PARAM*      param,
                  const GAMEDATA*                 gamedata,
                  BOOL                            b_fix
                         )
{
  param->gamedata          = gamedata;
  param->b_fix             = b_fix;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Zenkoku_Zukan_Award_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZENKOKU_ZUKAN_AWARD_WORK*     work;

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZENKOKU_ZUKAN_AWARD, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(ZENKOKU_ZUKAN_AWARD_WORK), HEAPID_ZENKOKU_ZUKAN_AWARD );
    GFL_STD_MemClear( work, sizeof(ZENKOKU_ZUKAN_AWARD_WORK) );
    
    work->heap_id       = HEAPID_ZENKOKU_ZUKAN_AWARD;
    work->param         = (ZENKOKU_ZUKAN_AWARD_PARAM*)pwk;
  }

  // グラフィック、フォントなど
  {
    work->graphic       = ZENKOKU_ZUKAN_AWARD_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // メインBG
  GFL_BG_SetPriority( BG_FRAME_M_FRONT,  BG_FRAME_PRI_M_FRONT );
  GFL_BG_SetPriority( BG_FRAME_M_TEXT,   BG_FRAME_PRI_M_TEXT  );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zenkoku_Zukan_Award_VBlankFunc, work, 1 );

  // 生成
  Zenkoku_Zukan_Award_BgInit( work );
  Zenkoku_Zukan_Award_TextInit( work );
  Zenkoku_Zukan_Award_BlackInit( work );  // 何もないところは黒にしておく
  Zenkoku_Zukan_Award_ObjInit( work );

  // サブBG
  {
    const MYSTATUS*  mystatus  = GAMEDATA_GetMyStatus( (GAMEDATA*)(work->param->gamedata) );  // ちょっとイヤだがconstはずし
    APP_NOGEAR_SUBSCREEN_Init();
    APP_NOGEAR_SUBSCREEN_Trans( work->heap_id, mystatus->sex );  // PM_MALE or PM_FEMALE  // include/pm_version.h
  }

  // スクロール
  Zenkoku_Zukan_Award_ScrollInit( work );
 
  // バックグラウンドカラー
  //GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );
  //GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x0000 );
  // バックグラウンドカラー(透過色)まで色として使っているので、変更してはダメ。

  // フェードイン(ただちに真っ黒)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, 0 );

  // 通信アイコン
  GFL_NET_ReloadIconTopOrBottom( FALSE, work->heap_id );

  // サウンド
  if( !(work->param->b_fix) )
  {
    PMSND_FadeOutBGM( PMSND_FADE_FAST );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Zenkoku_Zukan_Award_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZENKOKU_ZUKAN_AWARD_WORK*     work     = (ZENKOKU_ZUKAN_AWARD_WORK*)mywk;

  // 通信アイコン
  // 終了するときは通信アイコンに対して何もしない
  
  // サブBG
  APP_NOGEAR_SUBSCREEN_Exit();

  // 破棄
  Zenkoku_Zukan_Award_ObjExit( work );
  Zenkoku_Zukan_Award_TextExit( work );
  Zenkoku_Zukan_Award_BgExit( work );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Clear( work->print_que );
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    ZENKOKU_ZUKAN_AWARD_GRAPHIC_Exit( work->graphic );
  }

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_ZENKOKU_ZUKAN_AWARD );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Zenkoku_Zukan_Award_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  ZENKOKU_ZUKAN_AWARD_WORK*     work     = (ZENKOKU_ZUKAN_AWARD_WORK*)mywk;

  switch(*seq)
  {
  case SEQ_START:
    {
      BOOL  b_next  = TRUE;
      // サウンド
      if( !(work->param->b_fix) )
      {
        if( PMSND_CheckFadeOnBGM() )
        {
          b_next = FALSE;
        }
        else
        {
          PMSND_PauseBGM( TRUE );
          PMSND_PushBGM();
          PMSND_PlayBGM( SEQ_ME_HYOUKA6 );
        }
      }
      if( b_next )
      {
        // フェードイン(黒→見える)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );
        *seq = SEQ_FADE_IN;
      }
    }
    break;
  case SEQ_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        if( work->param->b_fix ) *seq = SEQ_MAIN;
        else                     *seq = SEQ_SCROLL;
      }
    }
    break;
  case SEQ_SCROLL:  // work->param->b_fixがFALSEのときしか来ない
    {
      if( Zenkoku_Zukan_Award_ScrollIsEnd( work ) )
      {
        *seq = SEQ_SOUND_WAIT;
      }
    }
    break;
  case SEQ_SOUND_WAIT:  // work->param->b_fixがFALSEのときしか来ない
    {
      // サウンド
      if( !PMSND_CheckPlayBGM() )
      {
        PMSND_PopBGM();
        PMSND_PauseBGM( FALSE );
        PMSND_FadeInBGM( PMSND_FADE_FAST );
        
        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      BOOL b_end = FALSE;
      u32 x, y;
      if( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
        b_end = TRUE;
      }
      else if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
        b_end = TRUE;
      }

      if( b_end )
      {
        *seq = SEQ_FADE_OUT;
        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        *seq = SEQ_END;
      }
    }
    break;
  case SEQ_END:
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }


#ifdef DEBUG_TEXT_MOVE
  {
    static int target = 0;
#ifdef USE_OBJ_GF
    static const int target_max = 5;
#else
    static const int target_max = 4;
#endif

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L )
    {
      target--;
      if( target < 0 ) target = target_max -1;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
    {
      target++;
      if( target >= target_max ) target = 0;
    }
    else
    {
      if( target >= 3 )
      {
        int obj_cell_idx = target - 3;
        GFL_CLACTPOS pos;
        GFL_CLACT_WK_GetPos( work->obj_clwk[obj_cell_idx], &pos, CLSYS_DEFREND_MAIN );

        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
        {
          if( obj_cell_idx == OBJ_BALL_CELL )    OS_Printf( "Monster Ball (%d, %d)\n", pos.x, pos.y );
#ifdef USE_OBJ_GF
          else if( obj_cell_idx == OBJ_GF_CELL ) OS_Printf( "Logo (%d, %d)\n", pos.x, pos.y );
#endif
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
        {
          pos.y--;
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
        {
          pos.y++;
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
        {
          pos.x--;
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
        {
          pos.x++;
        }
        
        GFL_CLACT_WK_SetPos( work->obj_clwk[obj_cell_idx], &pos, CLSYS_DEFREND_MAIN );
      }
      else
      {
        GFL_BMPWIN* target_bmpwin = work->text_bmpwin[target +2];
        u8 x = GFL_BMPWIN_GetPosX( target_bmpwin );
        u8 y = GFL_BMPWIN_GetPosY( target_bmpwin );
        BOOL b_change = FALSE;
      
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
        {
          switch(target)
          {
          case 0:
            OS_Printf( "Player Name (%d, %d)\n", x, y );
            break;
          case 1:
            OS_Printf( "Main Text (%d, %d)\n", x, y );
            break;
          case 2:
            OS_Printf( "Game Freak (%d, %d)\n", x, y );
            break;
          }
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
        {
          y--;
          b_change = TRUE;
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
        {
          y++;
          b_change = TRUE;
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
        {
          x--;
          b_change = TRUE;
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
        {
          x++;
          b_change = TRUE;
        }

        if( b_change )
        {
          u8 i;

          //GFL_BG_ClearScreen( BG_FRAME_M_TEXT );  // このBGフレームの空いているところを黒にしているので、全クリアはまずい
          GFL_BG_FillScreen( BG_FRAME_M_TEXT, 0, 0, 0, 32, 24, GFL_BG_SCRWRT_PALNL );
       
          GFL_BMPWIN_SetPosX( target_bmpwin, x );
          GFL_BMPWIN_SetPosY( target_bmpwin, y );
        
          for( i=0; i<TEXT_MAX; i++ )
          {
            GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
          }
        }
      }
    }
  }
#endif


  PRINTSYS_QUE_Main( work->print_que );

  // メイン
  Zenkoku_Zukan_Award_TextMain( work );

  if( SEQ_FADE_IN<=(*seq) && (*seq)<=SEQ_FADE_OUT )
  {
    Zenkoku_Zukan_Award_ScrollMain( work );
  }

  // 2D描画
  ZENKOKU_ZUKAN_AWARD_GRAPHIC_2D_Draw( work->graphic );

  // 3D描画
  //ZENKOKU_ZUKAN_AWARD_GRAPHIC_3D_StartDraw( work->graphic );
  //ZENKOKU_ZUKAN_AWARD_GRAPHIC_3D_EndDraw( work->graphic );

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
static void Zenkoku_Zukan_Award_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZENKOKU_ZUKAN_AWARD_WORK* work = (ZENKOKU_ZUKAN_AWARD_WORK*)wk;
}

//-------------------------------------
/// BG
//=====================================
static void Zenkoku_Zukan_Award_BgInit( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_SHOUJOU, work->heap_id );

  // FRONT
  GFL_ARCHDL_UTIL_TransVramPalette(
      handle,
      NARC_shoujou_wb_syoujyou_z_NCLR,
      PALTYPE_MAIN_BG,
      BG_PAL_POS_M_GRA_FRONT * 0x20,
      BG_PAL_NUM_M_GRA_FRONT * 0x20,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
      handle,
      NARC_shoujou_wb_syoujyou_z_NCGR,
      BG_FRAME_M_FRONT,
			0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      NARC_shoujou_wb_syoujyou_z_NSCR,
      BG_FRAME_M_FRONT,
      0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenReq( BG_FRAME_M_FRONT );
}
static void Zenkoku_Zukan_Award_BgExit( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  // 何もしない
}

//-------------------------------------
/// テキスト
//=====================================
static void Zenkoku_Zukan_Award_TextInit( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  u8 i;
  const MYSTATUS*     mystatus  = GAMEDATA_GetMyStatus( (GAMEDATA*)(work->param->gamedata) );  // ちょっとイヤだがconstはずし
  SAVE_CONTROL_WORK*  sv        = GAMEDATA_GetSaveControlWork( (GAMEDATA*)(work->param->gamedata) );  // ちょっとイヤだがconstはずし
  CONFIG*             cfg       = SaveData_GetConfig( sv );
  MOJIMODE            mojimode  = CONFIG_GetMojiMode( cfg );

  // NULL、ゼロ初期化
  work->msgdata = NULL;
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_bmpwin[i] = NULL;
    work->text_finish[i] = FALSE;
  }
  work->text_finish[TEXT_DUMMY] = TRUE;  // ダミーは済んでいることにしておく
  work->text_finish[TEXT_BLACK] = TRUE;  // ダミーは済んでいることにしておく

  // パレット
  GFL_ARC_UTIL_TransVramPaletteEx(
      ARCID_FONT,
      NARC_font_default_nclr,
      PALTYPE_MAIN_BG,
      0,
      BG_PAL_POS_M_TEXT * 0x20,
      BG_PAL_NUM_M_TEXT * 0x20,
      work->heap_id );

/*
  {
    // パレットの作成＆転送
    u16 num = 3;
    u16 siz = sizeof(u16) * num;
    u16 ofs = BG_PAL_POS_M_TEXT * 0x20 + 0x20 - 2 * num;
    u16* pal = GFL_HEAP_AllocClearMemory( work->heap_id, siz );
    pal[0x00] = 0x294b;  // 13  // 黒
    pal[0x01] = 0x5694;  // 14  // 灰
    pal[0x02] = 0x7fff;  // 15  // 白 
    GFL_BG_LoadPalette( BG_PAL_POS_M_TEXT, pal, siz, ofs );
    GFL_HEAP_FreeMemory( pal );
  }
*/

  // メッセージ
  work->msgdata = GFL_MSG_Create(
                      GFL_MSG_LOAD_NORMAL,
                      ARCID_SCRIPT_MESSAGE,
                      NARC_script_message_award_dat,
                      work->heap_id );

  // ビットマップウィンドウ
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_bmpwin[i] = GFL_BMPWIN_Create(
                                     bmpwin_setup[i][0],
                                     bmpwin_setup[i][1], bmpwin_setup[i][2], bmpwin_setup[i][3], bmpwin_setup[i][4],
                                     bmpwin_setup[i][5], bmpwin_setup[i][6] );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[i]), 0 );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );
  }

  // MOJIMODE_KANJI
  if( mojimode == MOJIMODE_KANJI )
  {
    for( i=0; i<TEXT_MAX; i++ )
    {
      GFL_BMPWIN_SetPosX( work->text_bmpwin[i], bmpwin_setup_kanji[i][0] );
      GFL_BMPWIN_SetPosY( work->text_bmpwin[i], bmpwin_setup_kanji[i][1] );
    }
  }

  // テキスト
  // プレイヤー名前さま
  {
    WORDSET* wordset       = WORDSET_Create( work->heap_id );
    STRBUF*  src_strbuf    = GFL_MSG_CreateString( work->msgdata, msg_award_05 );
    STRBUF*  strbuf        = GFL_STR_CreateBuffer( TEXT_NAME_LEN_MAX, work->heap_id );

    u16 str_width;
    u16 bmp_width;
    s16 str_x     = ( mojimode == MOJIMODE_KANJI )?(bmpwin_setup_kanji[TEXT_NAME][2]):(bmpwin_setup[TEXT_NAME][7]);
    s16 str_y     = ( mojimode == MOJIMODE_KANJI )?(bmpwin_setup_kanji[TEXT_NAME][3]):(bmpwin_setup[TEXT_NAME][8]);

    WORDSET_RegisterPlayerName( wordset, 0, mystatus );
    WORDSET_ExpandStr( wordset, strbuf, src_strbuf );

    str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
    bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_NAME]) );
    if( bmp_width >= str_width )
    {
      //str_x = (s16)( ( bmp_width - str_width ) / 2 );  // この一塊をセンタリング  // 左寄せなのでコメントアウト
    }

    PRINTSYS_PrintQueColor(
        work->print_que,
        GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_NAME]),
        str_x, str_y,
        strbuf,
        work->font,
        PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );
    GFL_STR_DeleteBuffer( strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
    WORDSET_Delete( wordset ); 
  }

  // ここに証明します
  {
#ifdef DEBUG_TEXT_MOVE
    STRBUF*  strbuf       = GFL_MSG_CreateString( work->msgdata, ( GFL_UI_KEY_GetCont() & ( PAD_BUTTON_Y ) ) ? ( msg_award_03 ) : ( msg_award_07 ) );
#else
    STRBUF*  strbuf       = GFL_MSG_CreateString( work->msgdata, msg_award_07 );
#endif

    u16 str_width;
    u16 bmp_width;
    s16 str_x     = ( mojimode == MOJIMODE_KANJI )?(bmpwin_setup_kanji[TEXT_MAIN][2]):(bmpwin_setup[TEXT_MAIN][7]);
    s16 str_y     = ( mojimode == MOJIMODE_KANJI )?(bmpwin_setup_kanji[TEXT_MAIN][3]):(bmpwin_setup[TEXT_MAIN][8]);

    str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
    bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_MAIN]) );
    if( bmp_width >= str_width )
    {
      if( mojimode == MOJIMODE_KANJI )
      {
        str_x = (s16)( ( bmp_width - str_width ) / 2 );  // この一塊をセンタリング
      }
      else
      {
        //str_x = (s16)( ( bmp_width - str_width ) / 2 );  // この一塊をセンタリング  // 左寄せなのでコメントアウト
      }
    }
    
    PRINTSYS_PrintQueColor(
        work->print_que,
        GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_MAIN]),
        str_x, str_y,
        strbuf,
        work->font,
        PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );
    GFL_STR_DeleteBuffer( strbuf );
  }

  // ゲームフリークスタッフ一同
  {
    STRBUF*  strbuf       = GFL_MSG_CreateString( work->msgdata, msg_award_06 );

    u16 str_width;
    u16 bmp_width;
    s16 str_x     = ( mojimode == MOJIMODE_KANJI )?(bmpwin_setup_kanji[TEXT_STAFF][2]):(bmpwin_setup[TEXT_STAFF][7]);
    s16 str_y     = ( mojimode == MOJIMODE_KANJI )?(bmpwin_setup_kanji[TEXT_STAFF][3]):(bmpwin_setup[TEXT_STAFF][8]);

    str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
    bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_STAFF]) );
    if( bmp_width >= str_width )
    {
      //str_x = (s16)( bmp_width - str_width );  // この一塊を右寄せ  // gmmで右寄せ指定しているのでコメントアウト
    }

    PRINTSYS_PrintQueColor(
        work->print_que,
        GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_STAFF]),
        str_x, str_y,
        strbuf,
        work->font,
        PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );
    GFL_STR_DeleteBuffer( strbuf );
  }

  // 済んでいないかもしれないが、1度呼んでおく
  for( i=TEXT_DUMMY +1; i<TEXT_MAX; i++ )
  {
    GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
  }

  // 既に済んでいるかもしれないので、Mainを1度呼んでおく
  Zenkoku_Zukan_Award_TextMain( work );
}
static void Zenkoku_Zukan_Award_TextExit( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  u8 i;

  for( i=0; i<TEXT_MAX; i++ )
  {
    if( work->text_bmpwin[i] ) GFL_BMPWIN_Delete( work->text_bmpwin[i] );
  }
  
  if( work->msgdata ) GFL_MSG_Delete( work->msgdata );
}
static void Zenkoku_Zukan_Award_TextMain( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  u8 i;

  for( i=0; i<TEXT_MAX; i++ )
  {
    if( !(work->text_finish[i]) )
    {
      if( !PRINTSYS_QUE_IsExistTarget( work->print_que, GFL_BMPWIN_GetBmp(work->text_bmpwin[i]) ) )
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
        work->text_finish[i] = TRUE;
      }
    }
  }
}

//-------------------------------------
/// 何もないところは黒にしておく
//=====================================
static void Zenkoku_Zukan_Award_BlackInit( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  // 必ずZenkoku_Zukan_Award_TextInitが済んでから呼ぶこと

  // パレットの作成＆転送
  {
    u16* pal = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(u16) * 0x10 );
    pal[0x00]                = 0x0000;              // 透明
    pal[TEXT_COLOR_L]        = TEXT_COLOR_VALUE_L;  // 文字主色
    pal[TEXT_COLOR_S]        = TEXT_COLOR_VALUE_S;  // 文字影色
    pal[TEXT_COLOR_BLACK]    = 0x0000;              // 黒
    GFL_BG_LoadPalette( BG_FRAME_M_TEXT, pal, 0x20, TEXT_PAL_POS*0x20 );
    GFL_HEAP_FreeMemory( pal );
  }

  // キャラの塗り潰し＆転送
  {
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_BLACK]), TEXT_COLOR_BLACK );  // 黒
	  GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_BLACK] );
  }

  // スクリーンの作成＆転送
  {
    u16* scr = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(u16) * 32*24 );  // 1画面分もスクロールしないので、これだけとっておけば十分のはず
    u8 i, j;
    u16 h = 0;
    u16 chr_num = GFL_BMPWIN_GetChrNum( work->text_bmpwin[TEXT_BLACK] );
    for(j=0; j<24; j++)       // TWL_ProgramingManual.pdf TWLプログラミングマニュアル
    {                         // 6.2.3.2.2 スクリーンデータのアドレスマッピング
      for(i=0; i<32; i++)     // スクリーンサイズが256×512 ドットのとき
      {                       // を参考にした。
        u16 chara_name = chr_num;
        u16 flip_h     = 0;
        u16 flip_v     = 0;
        u16 pal        = TEXT_PAL_POS;
        scr[h] = ( pal << 12 ) | ( flip_v << 11 ) | ( flip_h << 10 ) | ( chara_name << 0 );
        h++;
      }
    }
    GFL_BG_WriteScreen( BG_FRAME_M_TEXT, scr, 0, 24, 32, 24 );  // 横幅が256ドットなので、折り返しはないと思う
    GFL_BG_LoadScreenReq( BG_FRAME_M_TEXT );
    GFL_HEAP_FreeMemory( scr );
  }
}

//-------------------------------------
/// OBJ
//=====================================
static void Zenkoku_Zukan_Award_ObjInit( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  // リソースの読み込み
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_SHOUJOU, work->heap_id );

  // BALL
  work->obj_res[OBJ_BALL_RES_NCL] = GFL_CLGRP_PLTT_RegisterEx(
      handle,
      NARC_shoujou_shoujou_obj_NCLR,
      CLSYS_DRAW_MAIN,
      OBJ_PAL_POS_M_BALL * 0x20,
      0,
      OBJ_PAL_NUM_M_BALL,
      work->heap_id );
  work->obj_res[OBJ_BALL_RES_NCG] = GFL_CLGRP_CGR_Register(
      handle,
      NARC_shoujou_shoujou_obj_NCGR,
      FALSE,
      CLSYS_DRAW_MAIN,
      work->heap_id );
  work->obj_res[OBJ_BALL_RES_NCE] = GFL_CLGRP_CELLANIM_Register(
      handle,
      NARC_shoujou_shoujou_obj_NCER,
      NARC_shoujou_shoujou_obj_NANR,
      work->heap_id );

#ifdef USE_OBJ_GF
  // GF
  work->obj_res[OBJ_GF_RES_NCL] = GFL_CLGRP_PLTT_RegisterEx(
      handle,
      NARC_shoujou_shoujou_logo_NCLR,
      CLSYS_DRAW_MAIN,
      OBJ_PAL_POS_M_GF * 0x20,
      0,
      OBJ_PAL_NUM_M_GF,
      work->heap_id );
  work->obj_res[OBJ_GF_RES_NCG] = GFL_CLGRP_CGR_Register(
      handle,
      NARC_shoujou_shoujou_logo_NCGR,
      FALSE,
      CLSYS_DRAW_MAIN,
      work->heap_id );
  work->obj_res[OBJ_GF_RES_NCE] = GFL_CLGRP_CELLANIM_Register(
      handle,
      NARC_shoujou_shoujou_logo_NCER,
      NARC_shoujou_shoujou_logo_NANR,
      work->heap_id );
#endif

  GFL_ARC_CloseDataHandle( handle );

  // CLWK作成
  // BALL
  work->obj_clwk[OBJ_BALL_CELL] = GFL_CLACT_WK_Create(
      ZENKOKU_ZUKAN_AWARD_GRAPHIC_GetClunit( work->graphic ),
      work->obj_res[OBJ_BALL_RES_NCG],
      work->obj_res[OBJ_BALL_RES_NCL],
      work->obj_res[OBJ_BALL_RES_NCE],
      &obj_cell_data[OBJ_BALL_CELL],
      CLSYS_DEFREND_MAIN,
      work->heap_id );
  GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_BALL_CELL], TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[OBJ_BALL_CELL], TRUE );

#ifdef USE_OBJ_GF
  // GF
  work->obj_clwk[OBJ_GF_CELL] = GFL_CLACT_WK_Create(
      ZENKOKU_ZUKAN_AWARD_GRAPHIC_GetClunit( work->graphic ),
      work->obj_res[OBJ_GF_RES_NCG],
      work->obj_res[OBJ_GF_RES_NCL],
      work->obj_res[OBJ_GF_RES_NCE],
      &obj_cell_data[OBJ_GF_CELL],
      CLSYS_DEFREND_MAIN,
      work->heap_id );
  GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_GF_CELL], FALSE );
  GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[OBJ_GF_CELL], TRUE );
#endif
}
static void Zenkoku_Zukan_Award_ObjExit( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  // CLWK破棄
  u8 i;
  for( i=0; i<OBJ_CELL_MAX; i++ )
  {
    GFL_CLACT_WK_Remove( work->obj_clwk[i] );
  }

  // リソース破棄
#ifdef USE_OBJ_GF
  // GF
  GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_GF_RES_NCE] );
  GFL_CLGRP_CGR_Release( work->obj_res[OBJ_GF_RES_NCG] );
  GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_GF_RES_NCL] );
#endif
  // BALL
  GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_BALL_RES_NCE] );
  GFL_CLGRP_CGR_Release( work->obj_res[OBJ_BALL_RES_NCG] );
  GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_BALL_RES_NCL] );
}

//-------------------------------------
/// スクロール
//=====================================
static void Zenkoku_Zukan_Award_ScrollInit( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  if( work->param->b_fix ) return;

  GFL_BG_SetScroll( BG_FRAME_M_FRONT, GFL_BG_SCROLL_Y_SET, SCROLL_START_POS_Y );
  GFL_BG_SetScroll( BG_FRAME_M_TEXT, GFL_BG_SCROLL_Y_SET, SCROLL_START_POS_Y );
 
  {
    u8 i;
    for( i=0; i<OBJ_CELL_MAX; i++ )
    {
      GFL_CLACTPOS pos;
      pos.x = obj_cell_data[i].pos_x;
      pos.y = obj_cell_data[i].pos_y - SCROLL_START_POS_Y;
      GFL_CLACT_WK_SetPos( work->obj_clwk[i], &pos, CLSYS_DEFREND_MAIN );
    }
  }

  work->scroll_wait_count = SCROLL_WAIT;
}
static void Zenkoku_Zukan_Award_ScrollMain( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  int curr_value;

  if( work->param->b_fix ) return;

  curr_value = GFL_BG_GetScrollY( BG_FRAME_M_FRONT );
  if( curr_value != 0 )
  {
    if( work->scroll_wait_count == 0 )
    {
      int value;
      if( curr_value < SCROLL_VALUE ) value = curr_value;
      else                            value = SCROLL_VALUE;
      GFL_BG_SetScrollReq( BG_FRAME_M_FRONT, GFL_BG_SCROLL_Y_DEC, value );
      GFL_BG_SetScrollReq( BG_FRAME_M_TEXT, GFL_BG_SCROLL_Y_DEC, value );

      {
        u8 i;
        for( i=0; i<OBJ_CELL_MAX; i++ )
        {
          GFL_CLACTPOS pos;
          GFL_CLACT_WK_GetPos( work->obj_clwk[i], &pos, CLSYS_DEFREND_MAIN );
          pos.y += (s16)value;
          GFL_CLACT_WK_SetPos( work->obj_clwk[i], &pos, CLSYS_DEFREND_MAIN );
        }
      }

      work->scroll_wait_count = SCROLL_WAIT;
    }
    else
    {
      work->scroll_wait_count--;
    }
  }
}
static BOOL Zenkoku_Zukan_Award_ScrollIsEnd( ZENKOKU_ZUKAN_AWARD_WORK* work )
{
  BOOL ret = FALSE;

  if( work->param->b_fix ) return TRUE;

  {
    BOOL ret_bg = ( GFL_BG_GetScrollY( BG_FRAME_M_FRONT ) == 0 );
    BOOL ret_obj = TRUE;
    //GFL_CLACTPOS pos;  // BGのスクロールのみで判断することにしたので、コメントアウト。
    //GFL_CLACT_WK_GetPos( work->obj_clwk[OBJ_BALL_CELL], &pos, CLSYS_DEFREND_MAIN );
    //if( pos.y == 0 ) ret_obj = TRUE;

    ret = ( ret_bg && ret_obj );
  }
  return ret;
}

