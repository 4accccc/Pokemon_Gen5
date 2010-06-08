//============================================================================
/**
 *  @file   egg_demo.c
 *  @brief  タマゴ孵化デモ
 *  @author Koji Kawada
 *  @data   2010.01.25
 *  @note   
 *  モジュール名：EGG_DEMO
 */
//============================================================================
// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/game_data.h"
#include "savedata/record.h"
#include "gamesystem/msgspeed.h"
#include "system/palanm.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menu.h"
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "poke_tool/poke_tool.h"
#include "sound/pm_sndsys.h"
#include "../../field/field_sound.h"
#include "app/name_input.h"
#include "poke_tool/poke_memo.h"
#include "field/zonedata.h"  //ZONEDATA_GetPlaceNameID

#include "egg_demo_graphic.h"
#include "egg_demo_view.h"
#include "demo/egg_demo.h"


// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "script_message.naix"
#include "msg/script/msg_egg_event.h"
#include "egg_demo_particle.naix"
// サウンド
#include "sound/wb_sound_data.sadl"


// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x50000)               ///< ヒープサイズ
// BGフレーム
#define BG_FRAME_M_POKEMON           (GFL_BG_FRAME0_M)        // プライオリティ2
#define BG_FRAME_M_BELT              (GFL_BG_FRAME2_M)        // プライオリティ1
#define BG_FRAME_M_TEXT              (GFL_BG_FRAME1_M)        // プライオリティ0

#define BG_FRAME_S_BACK              (GFL_BG_FRAME0_S)        // プライオリティ0

// BGフレームのプライオリティ
#define BG_FRAME_PRI_M_POKEMON    (2)
#define BG_FRAME_PRI_M_BELT       (1)
#define BG_FRAME_PRI_M_TEXT       (0)

#define BG_FRAME_PRI_S_BACK       (0)

// BGパレット
// 本数
enum
{
  BG_PAL_NUM_M_BELT          = 1,
  BG_PAL_NUM_M_TEXT_FONT     = 1,
  BG_PAL_NUM_M_TEXT_FRAME    = 1,
};
// 位置
enum
{
  BG_PAL_POS_M_BELT          = 0,
  BG_PAL_POS_M_TEXT_FONT     = 1,
  BG_PAL_POS_M_TEXT_FRAME    = 2,
  BG_PAL_POS_M_MAX           = 3,  // ここから空き
};
// 本数
enum
{
  BG_PAL_NUM_S_              = 0,
};
// 位置
enum
{
  BG_PAL_POS_S_              = 0,
  BG_PAL_POS_S_MAX           = 0,  // ここから空き
};

// OBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_BELT         = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_M_BELT         = 0,
  OBJ_PAL_POS_M_MAX          = 1,       // ここから空き
};
// 本数
enum
{
  OBJ_PAL_NUM_S_             = 0,
};
// 位置
enum
{
  OBJ_PAL_POS_S_             = 0, 
  OBJ_PAL_POS_S_MAX          = 0,      // ここから空き
};

// OBJのBGプライオリティ
#define OBJ_BG_PRI_M_BELT   (BG_FRAME_PRI_M_BELT)

// OBJ
enum
{
  OBJ_CELL_BELT_UP_01,
  OBJ_CELL_BELT_UP_02,
  OBJ_CELL_BELT_UP_03,
  OBJ_CELL_BELT_DOWN_01,
  OBJ_CELL_BELT_DOWN_02,
  OBJ_CELL_BELT_DOWN_03,
  OBJ_CELL_MAX,
};
enum
{
  OBJ_RES_BELT_NCG,
  OBJ_RES_BELT_NCL,
  OBJ_RES_BELT_NCE,
  OBJ_RES_MAX,
};
static const GFL_CLWK_DATA obj_cell_data[OBJ_CELL_MAX] =
{
  { 128, 96, 0, 0, OBJ_BG_PRI_M_BELT },
  { 128, 96, 1, 0, OBJ_BG_PRI_M_BELT },
  { 128, 96, 2, 0, OBJ_BG_PRI_M_BELT },
  { 128, 96, 3, 0, OBJ_BG_PRI_M_BELT },
  { 128, 96, 4, 0, OBJ_BG_PRI_M_BELT },
  { 128, 96, 5, 0, OBJ_BG_PRI_M_BELT },
};


// TEXT
#define TEXT_WININ_BACK_COLOR        (15)

// フェード
#define FADE_IN_WAIT            (-16)       ///< フェードインのスピード  // BGやOBJのBELTが黒くしているのでフェードインは一瞬で行うのでマイナス
#define FADE_OUT_WAIT           (2)         ///< フェードアウトのスピード
#define FROM_WHITE_FADE_IN_WAIT (2)         ///< 白く飛ばしたフェードから、見える状態になるときのスピード
#define NAMEIN_FADE_OUT_WAIT    (0)         ///< 名前入力へ移行するためのフェードアウトのスピード

// はい・いいえウィンドウ
typedef enum
{
  TM_RESULT_NO,
  TM_RESULT_YES,
  TM_RESULT_SEL,
}
TM_RESULT;

// 文字数
#define STRBUF_LENGTH                (256)  // この文字数で足りるかbuflen.hで要確認
#define NAMEIN_STRBUF_LENGTH         ( 32)  // この文字数で足りるかbuflen.hで要確認

// 根幹ステップ
typedef enum
{
  TRUNK_STEP_FADE_IN_START,
  TRUNK_STEP_FADE_IN,
  TRUNK_STEP_BELT_OPEN,
  TRUNK_STEP_SOUND_INTRO,
  TRUNK_STEP_SOUND_HATCH_START_WAIT,
  TRUNK_STEP_DEMO_EGG,
  TRUNK_STEP_DEMO_HATCH,
  TRUNK_STEP_DEMO_READY,
  TRUNK_STEP_DEMO_MON,
  TRUNK_STEP_TEXT_0_0,
  TRUNK_STEP_TEXT_0_1,
  TRUNK_STEP_TEXT_0,
  TRUNK_STEP_TEXT_1,
  TRUNK_STEP_TM,
  TRUNK_STEP_NAMEIN_FADE_OUT,
  TRUNK_STEP_NAMEIN,
  TRUNK_STEP_NAMEIN_BLACK,
  TRUNK_STEP_NAMEIN_END,
  TRUNK_STEP_SOUND_FADE_OUT,
  TRUNK_STEP_FADE_OUT,
  TRUNK_STEP_END,
}
TRUNK_STEP;

// サウンドステップ
typedef enum
{
  SOUND_STEP_WAIT,
  SOUND_STEP_FIELD_FADE_OUT,
  SOUND_STEP_INTRO,
  SOUND_STEP_HATCH_LOAD,
  SOUND_STEP_HATCH,
  SOUND_STEP_CONGRATULATE_LOAD,
  SOUND_STEP_CONGRATULATE,
  SOUND_STEP_HATCH_FADE_OUT,
}
SOUND_STEP;

// TEXTのステップ
typedef enum
{
  TEXT_STEP_WAIT,
}
TEXT_STEP;

// はい・いいえウィンドウのステップ
typedef enum
{
  TM_STEP_WAIT,
  TM_STEP_SELECT,
  TM_STEP_END,
}
TM_STEP;


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
  // グラフィック、フォントなど
  HEAPID                      heap_id;
  EGG_DEMO_GRAPHIC_WORK*      graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // ステップ
  TRUNK_STEP                  trunk_step;
  u8                          trunk_wait;
  SOUND_STEP                  sound_step;
  TEXT_STEP                   text_step;
  TM_STEP                     tm_step;

  u32                         sound_div_seq;  // BGMの分割ロードのシーケンス

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // OBJ
  u32                         obj_res[OBJ_RES_MAX];
  GFL_CLWK*                   obj_clwk[OBJ_CELL_MAX]; 

  // TEXT
  PRINT_STREAM*               text_stream;
  GFL_TCBLSYS*                text_tcblsys;
  GFL_BMPWIN*                 text_winin_bmpwin;
  GFL_BMPWIN*                 text_dummy_bmpwin;      ///< 0番のキャラクターを1x1でつくっておく
  GFL_ARCUTIL_TRANSINFO       text_winfrm_tinfo;
  GFL_MSGDATA*                text_msgdata_egg;
  STRBUF*                     text_strbuf;
  BOOL                        text_show_vblank_req;  // TRUEのときテキストを表示するBG面を表示する

  // はい・いいえウィンドウ
  BMPWIN_YESNO_DAT            tm_dat;
  BMPMENU_WORK*               tm_wk;
  TM_RESULT                   tm_result;

  // タマゴ孵化デモの演出
  EGG_DEMO_VIEW_WORK*         view;

  // 名前入力
  NAMEIN_PARAM*               namein_param;

  // ローカルPROCシステム
  GFL_PROCSYS*  local_procsys;
}
EGG_DEMO_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Egg_Demo_VBlankFunc( GFL_TCB* tcb, void* wk );

// 初期化処理、終了処理(PROC変更前後に行うことができるもの)
static void Egg_Demo_Init( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_Exit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// BG
static void Egg_Demo_BgInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_BgExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// OBJ
static void Egg_Demo_ObjInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_ObjExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_ObjStartAnime( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_ObjIsEndAnime( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// サウンド
static void Egg_Demo_SoundInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_SoundCheckFadeOutField( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPlayIntro( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_SoundCheckPlayIntro( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPlayHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_SoundCheckPlayHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPushHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPopHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundPlayCongratulate( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_SoundCheckPlayCongratulate( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundFadeOutHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static BOOL Egg_Demo_SoundCheckFadeOutHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_SoundFadeInField( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// TEXT
static void Egg_Demo_TextInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TextExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TextMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TextShowWindow( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TextMakeStream( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work,
                                     u32 str_id );
static BOOL Egg_Demo_TextWaitStream( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );

// はい・いいえウィンドウ
static void Egg_Demo_TmInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TmExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TmMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static void Egg_Demo_TmStartSelect(  EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );
static TM_RESULT Egg_Demo_TmGetResult( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Egg_Demo_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Egg_Demo_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Egg_Demo_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    EGG_DEMO_ProcData =
{
  Egg_Demo_ProcInit,
  Egg_Demo_ProcMain,
  Egg_Demo_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================

#if 0

//------------------------------------------------------------------
/**
 *  @brief       タマゴ孵化デモ用パラメータ生成
 *
 *  @param[in]   heap_id       ヒープID
 *  @param[in]   gamedata      GAMEDATA
 *  @param[in]   pp            POKEMON_PARAM
 *
 *  @retval      EGG_DEMO_PARAM
 */
//------------------------------------------------------------------
EGG_DEMO_PARAM*  EGG_DEMO_AllocParam(
                            HEAPID           heap_id,
                            GAMEDATA*        gamedata,
                            POKEMON_PARAM*   pp )
{
  EGG_DEMO_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof( EGG_DEMO_PARAM ) );
  EGG_DEMO_InitParam( param, gamedata, pp );
  return param;
}

//------------------------------------------------------------------
/**
 *  @brief           タマゴ孵化デモ用パラメータ解放
 *
 *  @param[in,out]   param      EGG_DEMO_AllocParamで生成したもの
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             EGG_DEMO_FreeParam(
                            EGG_DEMO_PARAM*  param )
{
  GFL_HEAP_FreeMemory( param );
}

//------------------------------------------------------------------
/**
 *  @brief           タマゴ孵化デモ用パラメータを設定する
 *
 *  @param[in,out]   param      EGG_DEMO_PARAM
 *  @param[in]       gamedata   GAMEDATA
 *  @param[in]       pp         POKEMON_PARAM
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             EGG_DEMO_InitParam(
                            EGG_DEMO_PARAM*  param,
                            GAMEDATA*        gamedata,
                            POKEMON_PARAM*   pp )
{
  param->gamedata    = gamedata;
  param->pp          = pp;
}

#endif


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Egg_Demo_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  EGG_DEMO_PARAM*    param    = (EGG_DEMO_PARAM*)pwk;
  EGG_DEMO_WORK*     work;

  // ヒープ
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_EGG_DEMO, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(EGG_DEMO_WORK), HEAPID_EGG_DEMO );
    GFL_STD_MemClear( work, sizeof(EGG_DEMO_WORK) );
    
    work->heap_id       = HEAPID_EGG_DEMO;
  }

  // ステップ
  {
    work->trunk_step    = TRUNK_STEP_FADE_IN_START;
    work->trunk_wait    = 5;  // 最初画面が乱れるので、しばし真っ暗のまま待つ
    work->sound_step    = SOUND_STEP_WAIT;
    work->text_step     = TEXT_STEP_WAIT;
    work->tm_step       = TM_STEP_WAIT;
  }

  Egg_Demo_Init( param, work );

  // サウンド
  Egg_Demo_SoundInit( param, work );
  
  // フェードイン(黒→黒)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, 0 );

  // ローカルPROCシステムを作成
  work->local_procsys = GFL_PROC_LOCAL_boot( work->heap_id );

  // 通信アイコン
  GFL_NET_ReloadIconTopOrBottom( FALSE, work->heap_id );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Egg_Demo_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  EGG_DEMO_PARAM*    param    = (EGG_DEMO_PARAM*)pwk;
  EGG_DEMO_WORK*     work     = (EGG_DEMO_WORK*)mywk;

  // ローカルPROCシステムを破棄
  GFL_PROC_LOCAL_Exit( work->local_procsys ); 

  // サウンド
  Egg_Demo_SoundExit( param, work );

  if( work->trunk_step != TRUNK_STEP_NAMEIN_END )
    Egg_Demo_Exit( param, work );

  // ヒープ
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_EGG_DEMO );
  }

  // 通信アイコン
  // 終了するときは通信アイコンに対して何もしない

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Egg_Demo_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  EGG_DEMO_PARAM*    param    = (EGG_DEMO_PARAM*)pwk;
  EGG_DEMO_WORK*     work     = (EGG_DEMO_WORK*)mywk;

  int key_trg = GFL_UI_KEY_GetTrg();
  
  // ローカルPROCの更新処理
  GFL_PROC_MAIN_STATUS  local_proc_status   =  GFL_PROC_LOCAL_Main( work->local_procsys );
  if( local_proc_status == GFL_PROC_MAIN_VALID ) return GFL_PROC_RES_CONTINUE;

  switch( work->trunk_step )
  {
  case TRUNK_STEP_FADE_IN_START:
    {
      if( work->trunk_wait == 0 )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_FADE_IN;

        // フェードイン(黒→見える)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );
      }
      else
      {
        work->trunk_wait--;
      }
    }
    break;
  case TRUNK_STEP_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_BELT_OPEN;

        Egg_Demo_ObjStartAnime( param, work );
      }
    }
    break;
  case TRUNK_STEP_BELT_OPEN:
    {
      if(    ( !Egg_Demo_SoundCheckFadeOutField( param, work ) )
          && Egg_Demo_ObjIsEndAnime( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_SOUND_INTRO;

        Egg_Demo_SoundPlayIntro( param, work );
      }
    }
    break;
  case TRUNK_STEP_SOUND_INTRO:
    {
      if( !Egg_Demo_SoundCheckPlayIntro( param, work ) )
      {
#if 0
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_EGG;
        
        // タマゴ孵化デモの演出
        EGG_DEMO_VIEW_Start( work->view );
        
        Egg_Demo_SoundPlayHatch( param, work );
#else
        // 次へ
        work->trunk_step = TRUNK_STEP_SOUND_HATCH_START_WAIT;

        Egg_Demo_SoundPlayHatch( param, work );
#endif
      }
    }
    break;
  case TRUNK_STEP_SOUND_HATCH_START_WAIT:
    {
      if( Egg_Demo_SoundCheckPlayHatch( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_EGG;

        // タマゴ孵化デモの演出
        EGG_DEMO_VIEW_Start( work->view );
      }
    }
    break;
  case TRUNK_STEP_DEMO_EGG:
    {
      // タマゴ孵化デモの演出
      if( EGG_DEMO_VIEW_White( work->view ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_HATCH;

        Egg_Demo_SoundPushHatch( param, work );
      }
    }
    break;
  case TRUNK_STEP_DEMO_HATCH:
    {
      // 次へ
      work->trunk_step = TRUNK_STEP_DEMO_READY;

      // タマゴ孵化
      {
        //PP_Put( param->pp, ID_PARA_tamago_flag, 0 );  // タマゴフラグを落とすだけでは不完全な初期化になってしまうので、PP_Birthを呼ぶ。
        PLAYER_WORK* player_wk = GAMEDATA_GetMyPlayerWork( param->gamedata );
        PP_Birth(  // PP_Birthはトレーナーメモの設定もしてくれるので、下にあるトレーナーメモの設定はコメントアウトした。
            param->pp,
            GAMEDATA_GetMyStatus( param->gamedata ),
            ZONEDATA_GetPlaceNameID( PLAYERWORK_getZoneID( player_wk ) ),
            work->heap_id );
      }

      // 図鑑登録（捕まえた）
      {
        ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( param->gamedata );
        ZUKANSAVE_SetPokeSee( zukan_savedata, param->pp );  // 見た
        ZUKANSAVE_SetPokeGet( zukan_savedata, param->pp );  // 捕まえた
      }

/*
      {
        // トレーナーメモ
        PLAYER_WORK* player_wk = GAMEDATA_GetMyPlayerWork( param->gamedata );
        POKE_MEMO_SetTrainerMemoPP(
            param->pp,
            POKE_MEMO_INCUBATION,
            GAMEDATA_GetMyStatus( param->gamedata ),
            ZONEDATA_GetPlaceNameID( PLAYERWORK_getZoneID( player_wk ) ),
            work->heap_id );
      }
*/

      // タマゴ孵化デモの演出
      EGG_DEMO_VIEW_Hatch( work->view, param->pp );
    }
    break;
  case TRUNK_STEP_DEMO_READY:
    {
      // タマゴ孵化デモの演出
      if( EGG_DEMO_VIEW_IsReady( work->view ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_DEMO_MON;
        
        // タマゴ孵化デモの演出
        EGG_DEMO_VIEW_MonStart( work->view );
      }
    }
    break;
  case TRUNK_STEP_DEMO_MON:
    {
      // タマゴ孵化デモの演出
      if( EGG_DEMO_VIEW_IsEnd( work->view ) )
      {
        // 次へ
        //work->trunk_step = TRUNK_STEP_TEXT_0;
        work->trunk_step = TRUNK_STEP_TEXT_0_0;

        Egg_Demo_TextShowWindow( param, work );
        //Egg_Demo_TextMakeStream( param, work, egg_evemt_02 );
        
        Egg_Demo_SoundPlayCongratulate( param, work );
      }
    }
    break;
  
  case TRUNK_STEP_TEXT_0_0:
    {
      work->text_show_vblank_req = TRUE;
      // 次へ
      work->trunk_step = TRUNK_STEP_TEXT_0_1;
    }
    break;
  case TRUNK_STEP_TEXT_0_1:
    {
      Egg_Demo_TextMakeStream( param, work, egg_evemt_02 );
      // 次へ
      work->trunk_step = TRUNK_STEP_TEXT_0;
    }
    break;
  
  case TRUNK_STEP_TEXT_0:
    {
      if(    Egg_Demo_TextWaitStream( param, work )
          && (!Egg_Demo_SoundCheckPlayCongratulate( param, work )) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_TEXT_1;
        
        Egg_Demo_TextMakeStream( param, work, egg_evemt_03 );
      }
    }
    break;
  case TRUNK_STEP_TEXT_1:
    {
      if( Egg_Demo_TextWaitStream( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_TM;

        Egg_Demo_TmStartSelect( param, work );
      }
    }
    break;
  case TRUNK_STEP_TM:
    {
      TM_RESULT tm_result = Egg_Demo_TmGetResult( param, work );
      if( tm_result == TM_RESULT_NO )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_SOUND_FADE_OUT;

        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );

        Egg_Demo_SoundFadeOutHatch( param, work );

        // タッチorキー
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );  // はい・いいえウィンドウに応えて終わるので、必ずキーで終了
      }
      else if( tm_result == TM_RESULT_YES )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_NAMEIN_FADE_OUT;

        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, NAMEIN_FADE_OUT_WAIT );

        // タッチorキー
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );  // はい・いいえウィンドウに応えて終わるので、必ずキーで終了
      }
    }
    break;
  case TRUNK_STEP_NAMEIN_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        MISC *misc  = SaveData_GetMisc( GAMEDATA_GetSaveControlWork( param->gamedata ) );

        Egg_Demo_Exit( param, work );

        // 次へ
        work->trunk_step = TRUNK_STEP_NAMEIN;

        // 名前入力へ行く
        work->namein_param = NAMEIN_AllocParamPokemonByPP( work->heap_id, param->pp, NAMEIN_POKEMON_LENGTH, NULL, misc );
        // ローカルPROC呼び出し
        GFL_PROC_LOCAL_CallProc( work->local_procsys, FS_OVERLAY_ID(namein), &NameInputProcData, work->namein_param );
      }
    }
    break;
  case TRUNK_STEP_NAMEIN:
    {
      // ローカルPROCが終了するのを待つ  // このMainの最初でGFL_PROC_MAIN_VALIDならreturnしているので、ここでは判定しなくてもよいが念のため
      if( local_proc_status != GFL_PROC_MAIN_VALID )
      {
        // 名前入力から戻る
        if( !NAMEIN_IsCancel( work->namein_param ) )
        {
          STRBUF* strbuf = GFL_STR_CreateBuffer( NAMEIN_STRBUF_LENGTH, work->heap_id );
          NAMEIN_CopyStr( work->namein_param, strbuf );
          PP_Put( param->pp, ID_PARA_nickname, (u32)strbuf );
          GFL_STR_DeleteBuffer( strbuf );

          // ポケモンにニックネームを付けた回数
          {
            RECORD* rec = GAMEDATA_GetRecordPtr(param->gamedata); 
            RECORD_Inc(rec, RECID_NICKNAME);
          }
        }
        NAMEIN_FreeParam( work->namein_param );

        // 次へ
        work->trunk_step = TRUNK_STEP_NAMEIN_BLACK;
     
        Egg_Demo_SoundFadeOutHatch( param, work );
      }
      else
      {
        return GFL_PROC_RES_CONTINUE;
      }
    }
    break;
  case TRUNK_STEP_NAMEIN_BLACK:
    {
      if( !Egg_Demo_SoundCheckFadeOutHatch( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_NAMEIN_END;
 
        Egg_Demo_SoundFadeInField( param, work );
        
        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  case TRUNK_STEP_SOUND_FADE_OUT:
    {
      if( !Egg_Demo_SoundCheckFadeOutHatch( param, work ) )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_FADE_OUT;
        
        Egg_Demo_SoundFadeInField( param, work );
      }
    }
    break;
  case TRUNK_STEP_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->trunk_step = TRUNK_STEP_END;

        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  default:
    break;
  }

  // サウンド
  Egg_Demo_SoundMain( param, work );
  
  if(    work->trunk_step != TRUNK_STEP_NAMEIN
      && work->trunk_step != TRUNK_STEP_NAMEIN_BLACK
      && work->trunk_step != TRUNK_STEP_NAMEIN_END
      && work->trunk_step != TRUNK_STEP_END )
  {
    // TEXT
    Egg_Demo_TextMain( param, work );
    // はい・いいえウィンドウ
    Egg_Demo_TmMain( param, work );

    // タマゴ孵化デモの演出
    EGG_DEMO_VIEW_Main( work->view );

    PRINTSYS_QUE_Main( work->print_que );

    // 2D描画
    EGG_DEMO_GRAPHIC_2D_Draw( work->graphic );
    // 3D描画
    EGG_DEMO_GRAPHIC_3D_StartDraw( work->graphic );
    EGG_DEMO_VIEW_Draw( work->view );  // タマゴ孵化デモの演出
    EGG_DEMO_GRAPHIC_3D_EndDraw( work->graphic );
  }

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
static void Egg_Demo_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  EGG_DEMO_WORK* work = (EGG_DEMO_WORK*)wk;

  // テキストを表示するBG面
  if( work->text_show_vblank_req )
  {
    GFL_BG_SetVisible( BG_FRAME_M_TEXT               , VISIBLE_ON );
    work->text_show_vblank_req = FALSE;
  }
}

//-------------------------------------
/// 初期化処理、終了処理(PROC変更前後に行うことができるもの)
//=====================================
static void Egg_Demo_Init( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
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
    work->graphic       = EGG_DEMO_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // VBlank中TCB
  {
    work->vblank_tcb = GFUser_VIntr_CreateTCB( Egg_Demo_VBlankFunc, work, 1 );
  }

  // BG
  Egg_Demo_BgInit( param, work );
  // OBJ
  Egg_Demo_ObjInit( param, work );
  // TEXT
  Egg_Demo_TextInit( param, work );
  // はい・いいえウィンドウ
  Egg_Demo_TmInit( param, work );

  // プライオリティ、表示、背景色など
  {
    GFL_BG_SetPriority( BG_FRAME_M_POKEMON           , BG_FRAME_PRI_M_POKEMON );
    GFL_BG_SetPriority( BG_FRAME_M_BELT              , BG_FRAME_PRI_M_BELT );
    GFL_BG_SetPriority( BG_FRAME_M_TEXT              , BG_FRAME_PRI_M_TEXT );  // 最前面

    GFL_BG_SetPriority( BG_FRAME_S_BACK              , 0 );  // 最前面

    GFL_BG_SetVisible( BG_FRAME_M_POKEMON            , VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_M_BELT               , VISIBLE_ON );
    //GFL_BG_SetVisible( BG_FRAME_M_TEXT               , VISIBLE_ON );  // テキストを表示するBG面
  
    GFL_BG_SetVisible( BG_FRAME_S_BACK               , VISIBLE_ON );

    // パーティクル対応
    G2_SetBlendAlpha( /*GX_BLEND_PLANEMASK_NONE,*/GX_BLEND_PLANEMASK_BG0,
                      GX_BLEND_PLANEMASK_BD | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG0,
                      0, 0 );

    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x4210 );
    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x0000 );
  }

  // タマゴ孵化デモの演出
  work->view = EGG_DEMO_VIEW_Init( work->heap_id, param->pp );
}
static void Egg_Demo_Exit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // タマゴ孵化デモの演出
  EGG_DEMO_VIEW_Exit( work->view );

  // はい・いいえウィンドウ
  Egg_Demo_TmExit( param, work );
  // TEXT
  Egg_Demo_TextExit( param, work );
  // OBJ
  Egg_Demo_ObjExit( param, work );
  // BG
  Egg_Demo_BgExit( param, work );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Clear( work->print_que );
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    EGG_DEMO_GRAPHIC_Exit( work->graphic );
  }
}

//-------------------------------------
/// BG
//=====================================
static void Egg_Demo_BgInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_EGG_DEMO, work->heap_id );

  // BELT
  GFL_ARCHDL_UTIL_TransVramPalette(
      handle,
      NARC_egg_demo_particle_demo_egg_NCLR,
      PALTYPE_MAIN_BG,
      BG_PAL_POS_M_BELT * 0x20,
      BG_PAL_NUM_M_BELT * 0x20,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
      handle,
      NARC_egg_demo_particle_demo_egg_bg_NCGR,
      BG_FRAME_M_BELT,
			0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      NARC_egg_demo_particle_demo_egg_NSCR,
      BG_FRAME_M_BELT,
      0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenReq( BG_FRAME_M_BELT );
}
static void Egg_Demo_BgExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // 何もしない
}

//-------------------------------------
/// OBJ
//=====================================
static void Egg_Demo_ObjInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  u8 i;

  // リソース読み込み
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_EGG_DEMO, work->heap_id );

  work->obj_res[OBJ_RES_BELT_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
          handle,
          NARC_egg_demo_particle_demo_egg_NCLR,
          CLSYS_DRAW_MAIN,
          OBJ_PAL_POS_M_BELT * 0x20,
          0,
          OBJ_PAL_NUM_M_BELT,
          work->heap_id );
      
  work->obj_res[OBJ_RES_BELT_NCG] = GFL_CLGRP_CGR_Register(
          handle,
          NARC_egg_demo_particle_demo_egg_NCGR,
          FALSE,
          CLSYS_DRAW_MAIN,
          work->heap_id );

  work->obj_res[OBJ_RES_BELT_NCE] = GFL_CLGRP_CELLANIM_Register(
          handle,
          NARC_egg_demo_particle_demo_egg_NCER,
          NARC_egg_demo_particle_demo_egg_NANR,
          work->heap_id );
  
  GFL_ARC_CloseDataHandle( handle );

  // CLWK作成
  for( i=0; i<OBJ_CELL_MAX; i++ )
  {
    work->obj_clwk[i] = GFL_CLACT_WK_Create(
        EGG_DEMO_GRAPHIC_GetClunit( work->graphic ),
        work->obj_res[OBJ_RES_BELT_NCG],
        work->obj_res[OBJ_RES_BELT_NCL],
        work->obj_res[OBJ_RES_BELT_NCE],
        &obj_cell_data[i],
        CLSYS_DEFREND_MAIN,
        work->heap_id );
    GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[i], FALSE );
  }
}
static void Egg_Demo_ObjExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // CLWK破棄
  u8 i;
  for( i=0; i<OBJ_CELL_MAX; i++ )
  {
    GFL_CLACT_WK_Remove( work->obj_clwk[i] );
  }

  // リソース破棄
  GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_RES_BELT_NCE] );
  GFL_CLGRP_CGR_Release( work->obj_res[OBJ_RES_BELT_NCG] );
  GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_RES_BELT_NCL] );
}
static void Egg_Demo_ObjStartAnime( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  u8 i;
  for( i=0; i<OBJ_CELL_MAX; i++ )
  {
    GFL_CLACT_WK_SetAnmIndex( work->obj_clwk[i], 0 );
    GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[i], TRUE );
  }
}
static BOOL Egg_Demo_ObjIsEndAnime( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  BOOL b_end = TRUE;
  u8 i;
  for( i=0; i<OBJ_CELL_MAX; i++ )
  {
    if( GFL_CLACT_WK_CheckAnmActive( work->obj_clwk[i] ) )
    {
      b_end = FALSE;
      break;
    }
  }
  return b_end;
}

//-------------------------------------
/// サウンド
//=====================================
static void Egg_Demo_SoundInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_FadeOutBGM( FSND_FADE_SHORT );
  work->sound_step = SOUND_STEP_FIELD_FADE_OUT;
}
static void Egg_Demo_SoundExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // 何もしない
}
static void Egg_Demo_SoundMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  switch( work->sound_step )
  {
  case SOUND_STEP_WAIT:
    {
    }
    break;
  case SOUND_STEP_FIELD_FADE_OUT:
    {
      if( !PMSND_CheckFadeOnBGM() )
      {
        PMSND_PauseBGM( TRUE );
        PMSND_PushBGM();
        work->sound_step = SOUND_STEP_WAIT;
      }
    }
    break;
  case SOUND_STEP_INTRO:
    {
      if( !PMSND_CheckPlayBGM() )
      {
        work->sound_step = SOUND_STEP_WAIT;
      }
    }
    break;
  case SOUND_STEP_HATCH_LOAD:
    {
      // BGMの分割ロードを利用してみる
      BOOL play_start = PMSND_PlayBGMdiv( SEQ_BGM_KOUKAN, &(work->sound_div_seq), FALSE );
      if( play_start )
      {
        work->sound_step = SOUND_STEP_HATCH;
      }
    }
    break;
  case SOUND_STEP_HATCH:
    {
    }
    break;
  case SOUND_STEP_CONGRATULATE_LOAD:
    {
      // BGMの分割ロードを利用してみる
      BOOL play_start = PMSND_PlayBGMdiv( SEQ_ME_SHINKAOME, &(work->sound_div_seq), FALSE );
      if( play_start )
      {
        work->sound_step = SOUND_STEP_CONGRATULATE;
      }
    }
    break;
  case SOUND_STEP_CONGRATULATE:
    {
      if( !PMSND_CheckPlayBGM() )
      {
        Egg_Demo_SoundPopHatch( param, work );
        work->sound_step = SOUND_STEP_HATCH;
      }
    }
    break;
  case SOUND_STEP_HATCH_FADE_OUT:
    {
      if( !PMSND_CheckFadeOnBGM() )
      {
        PMSND_StopBGM();
        work->sound_step = SOUND_STEP_WAIT;
      }
    }
    break;
  }
}
static BOOL Egg_Demo_SoundCheckFadeOutField( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_FIELD_FADE_OUT ) return TRUE;
  else                                                return FALSE;
}
static void Egg_Demo_SoundPlayIntro( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_PlayBGM(SEQ_BGM_SHINKA);
  work->sound_step = SOUND_STEP_INTRO;
}
static BOOL Egg_Demo_SoundCheckPlayIntro( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_INTRO ) return TRUE;
  else                                       return FALSE;
}
static void Egg_Demo_SoundPlayHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
/*
BGMの分割ロードを利用してみることにしたのでコメントアウト
  PMSND_PlayBGM(SEQ_BGM_KOUKAN);
  work->sound_step = SOUND_STEP_HATCH;
*/
  // BGMの分割ロードを利用してみる
  work->sound_div_seq = 0;
  PMSND_PlayBGMdiv( SEQ_BGM_KOUKAN, &(work->sound_div_seq), TRUE );
  work->sound_step = SOUND_STEP_HATCH_LOAD;
}
static BOOL Egg_Demo_SoundCheckPlayHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  return ( work->sound_step == SOUND_STEP_HATCH );
}
static void Egg_Demo_SoundPushHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // ここはフェードしなくていい
  PMSND_PauseBGM( TRUE );
  PMSND_PushBGM();
}
static void Egg_Demo_SoundPopHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_PopBGM();
  PMSND_PauseBGM( FALSE );
  PMSND_FadeInBGM( FSND_FADE_FAST );
}
static void Egg_Demo_SoundPlayCongratulate( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
/*
  PMSND_PlayBGM(SEQ_ME_SHINKAOME);
  work->sound_step = SOUND_STEP_CONGRATULATE;
*/
  // BGMの分割ロードを利用してみる
  work->sound_div_seq = 0;
  PMSND_PlayBGMdiv( SEQ_ME_SHINKAOME, &(work->sound_div_seq), TRUE );
  work->sound_step = SOUND_STEP_CONGRATULATE_LOAD;
}
static BOOL Egg_Demo_SoundCheckPlayCongratulate( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  return ( work->sound_step == SOUND_STEP_CONGRATULATE_LOAD || work->sound_step == SOUND_STEP_CONGRATULATE );
}
static void Egg_Demo_SoundFadeOutHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  PMSND_FadeOutBGM( FSND_FADE_NORMAL );
  work->sound_step = SOUND_STEP_HATCH_FADE_OUT;
}
static BOOL Egg_Demo_SoundCheckFadeOutHatch( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_HATCH_FADE_OUT ) return TRUE;
  else                                                return FALSE;
}
static void Egg_Demo_SoundFadeInField( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // フィールドBGM
  PMSND_PopBGM();
  PMSND_PauseBGM( FALSE );
  PMSND_FadeInBGM( FSND_FADE_NORMAL );
}

//-------------------------------------
/// TEXT
//=====================================
static void Egg_Demo_TextInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
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

  // メッセージ
  work->text_msgdata_egg     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_egg_event_dat, work->heap_id );

  // TCBL、フォントカラー、転送など
  work->text_tcblsys = GFL_TCBL_Init( work->heap_id, work->heap_id, 1, 0 );
  GFL_FONTSYS_SetColor( 1, 2, TEXT_WININ_BACK_COLOR );
  GFL_BG_LoadScreenV_Req( BG_FRAME_M_TEXT );

  // NULL初期化
  work->text_stream       = NULL;
  work->text_strbuf       = NULL;

  // テキストを表示するBG面
  work->text_show_vblank_req = FALSE;
  GFL_BG_SetVisible( BG_FRAME_M_TEXT               , VISIBLE_OFF );
}
static void Egg_Demo_TextExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  if( work->text_stream ) PRINTSYS_PrintStreamDelete( work->text_stream );
  if( work->text_strbuf ) GFL_STR_DeleteBuffer( work->text_strbuf );
  GFL_TCBL_Exit( work->text_tcblsys );
  GFL_MSG_Delete( work->text_msgdata_egg );
  GFL_BG_FreeCharacterArea( BG_FRAME_M_TEXT,
      GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
      GFL_ARCUTIL_TRANSINFO_GetSize(work->text_winfrm_tinfo) );
  GFL_BMPWIN_Delete( work->text_winin_bmpwin );
  GFL_BMPWIN_Delete( work->text_dummy_bmpwin );
}
static void Egg_Demo_TextMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  switch( work->text_step )
  {
  case TEXT_STEP_WAIT:
    {
    }
    break;
  }

  GFL_TCBL_Main( work->text_tcblsys );
}
static void Egg_Demo_TextShowWindow( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
/*
  BmpWinFrame_Write( work->text_winin_bmpwin, WINDOW_TRANS_ON_V,
      GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
      BG_PAL_POS_M_TEXT_FRAME );
  GFL_BMPWIN_MakeTransWindow_VBlank( work->text_winin_bmpwin );
*/

  BmpWinFrame_Write( work->text_winin_bmpwin, WINDOW_TRANS_ON,
      GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
      BG_PAL_POS_M_TEXT_FRAME );
  GFL_BMPWIN_MakeTransWindow( work->text_winin_bmpwin );
}
static void Egg_Demo_TextMakeStream( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work,
                                     u32 str_id )
{
  // 一旦消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), TEXT_WININ_BACK_COLOR );
  
  // 前のを消す
  if( work->text_stream ) PRINTSYS_PrintStreamDelete( work->text_stream );  // この関数でタスクも削除してくれるので、同時に動くタスクは1つで済む
  if( work->text_strbuf ) GFL_STR_DeleteBuffer( work->text_strbuf );

  // 文字列作成
  {
    STRBUF*   src_strbuf  = GFL_MSG_CreateString( work->text_msgdata_egg, str_id );
    WORDSET*  wordset     = WORDSET_Create( work->heap_id );
    WORDSET_RegisterPokeMonsName( wordset, 0, param->pp );
    work->text_strbuf = GFL_STR_CreateBuffer( STRBUF_LENGTH, work->heap_id );
    WORDSET_ExpandStr( wordset, work->text_strbuf, src_strbuf );
    WORDSET_Delete( wordset );
    GFL_STR_DeleteBuffer( src_strbuf );
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
static BOOL Egg_Demo_TextWaitStream( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  BOOL ret = FALSE;

  switch( PRINTSYS_PrintStreamGetState( work->text_stream ) )
  { 
  case PRINTSTREAM_STATE_RUNNING:
    if( ( GFL_UI_KEY_GetCont() & ( PAD_BUTTON_A | PAD_BUTTON_B ) ) || GFL_UI_TP_GetCont() )
    {
      PRINTSYS_PrintStreamShortWait( work->text_stream, 0 );
    }
    break;
  case PRINTSTREAM_STATE_PAUSE:
    if( ( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) ) || GFL_UI_TP_GetTrg() )
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
static void Egg_Demo_TmInit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // この関数はEgg_Demo_TextInitが済んで
  // BG_PAL_POS_M_TEXT_FONTが設定されている状態で呼ぶこと
  // GFL_FONTSYS_SetColorの設定も必要かもしれない

  u32 size = GFL_BG_1CHRDATASIZ * ( 7*4 +12 );  // system/bmp_menu.cのBmpMenu_YesNoSelectInitを参考にしたサイズ + 少し大きめに
  u32 pos = GFL_BG_AllocCharacterArea( BG_FRAME_M_TEXT, size, GFL_BG_CHRAREA_GET_B );
  GF_ASSERT_MSG( pos != AREAMAN_POS_NOTFOUND, "EGG_DEMO : BGキャラ領域が足りませんでした。\n" );
  GFL_BG_FreeCharacterArea( BG_FRAME_M_TEXT, pos, size );

  work->tm_dat.frmnum    = BG_FRAME_M_TEXT;
  work->tm_dat.pos_x     = 24;
  work->tm_dat.pos_y     = 13;
  work->tm_dat.palnum    = BG_PAL_POS_M_TEXT_FONT;
  work->tm_dat.chrnum    = pos;  // 使われていないようだ

  work->tm_result = TM_RESULT_SEL;
}
static void Egg_Demo_TmExit( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // BmpMenu_YesNoSelectMainが解放してくれるので、何もしなくてよい
}
static void Egg_Demo_TmMain( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  switch( work->tm_step )
  {
  case TM_STEP_WAIT:
    {
    }
    break;
  case TM_STEP_SELECT:
    {
      u32 ret = BmpMenu_YesNoSelectMain( work->tm_wk );
      if( ret != BMPMENU_NULL )
      {
        if( ret == 0 )
          work->tm_result = TM_RESULT_YES;
        else //if( ret == BMPMENU_CANCEL )
          work->tm_result = TM_RESULT_NO;

        work->tm_step = TM_STEP_END;
      }
    }
    break;
  }
}
static void Egg_Demo_TmStartSelect(  EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  // この関数はEgg_Demo_TextInitが済んで
  // work->text_winfrm_tinfoが設定されている状態で呼ぶこと

  work->tm_wk = BmpMenu_YesNoSelectInit(
                    &work->tm_dat,
                    GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
                    BG_PAL_POS_M_TEXT_FRAME,
                    0,
                    work->heap_id );

  work->tm_result = TM_RESULT_SEL;
  work->tm_step = TM_STEP_SELECT;
}
static TM_RESULT Egg_Demo_TmGetResult( EGG_DEMO_PARAM* param, EGG_DEMO_WORK* work )
{
  return work->tm_result;
}

