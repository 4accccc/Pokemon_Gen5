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

//#define NOT_USE_STATUS_BATTLE  // これが定義されているとき、バトルステータスを使わない


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
#include "gamesystem/game_data.h"
#include "savedata/record.h"
#include "savedata/mail.h"
#include "item/itemsym.h"
#include "poke_tool/shinka_check.h"
#include "print/global_msg.h"
#include "app/app_keycursor.h"

#include "ui/yesno_menu.h"

#include "savedata/save_control.h"
#include "app/p_status.h"

#include "sound/wb_sound_data.sadl"
#include "sound/sound_manager.h"
#include "sound/pm_sndsys.h"
#include "../../field/field_sound.h"

#include "shinka_demo_graphic.h"
#include "shinka_demo_view.h"
#include "shinka_demo_effect.h"


// アーカイブ
#include "shinka_demo_particle.naix"


// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(poke_status);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SYS_SIZE          (0x10000)               ///< 他PROCへ移行している際も解放しないシステムのヒープサイズ
#define HEAP_SIZE              (0x70000)               ///< ヒープサイズ

// BGフレーム
#define BG_FRAME_M_POKEMON           (GFL_BG_FRAME0_M)        // プライオリティ2
#define BG_FRAME_M_BELT              (GFL_BG_FRAME2_M)        // プライオリティ1
#define BG_FRAME_M_TEXT              (GFL_BG_FRAME1_M)        // プライオリティ0

#define BG_FRAME_S_BACK              (GFL_BG_FRAME0_S)        // プライオリティ2
#define BG_FRAME_S_BUTTON            (GFL_BG_FRAME1_S)        // プライオリティ1
#define BG_FRAME_S_TEXT              (GFL_BG_FRAME2_S)        // プライオリティ0

// BGフレームのプライオリティ
#define BG_FRAME_PRI_M_POKEMON    (2)
#define BG_FRAME_PRI_M_BELT       (1)
#define BG_FRAME_PRI_M_TEXT       (0)

#define BG_FRAME_PRI_S_BACK       (2)
#define BG_FRAME_PRI_S_BUTTON     (1)
#define BG_FRAME_PRI_S_TEXT       (0)

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
  BG_PAL_NUM_S_BACK          = 1,
  BG_PAL_NUM_S_BUTTON        = 2,
  BG_PAL_NUM_S_TEXT          = 1,
};
// 位置
enum
{
  BG_PAL_POS_S_BACK          = 0,
  BG_PAL_POS_S_BUTTON        = 1, 
  BG_PAL_POS_S_TEXT          = 3, 
  BG_PAL_POS_S_MAX           = 4,  // ここから空き
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
  OBJ_PAL_POS_M_MAX          = 1,  // ここから空き
};
// 本数
enum
{
  OBJ_PAL_NUM_S_CURSOR        = 2,
};
// 位置
enum
{
  OBJ_PAL_POS_S_CURSOR        = 0,
  OBJ_PAL_POS_S_MAX           = 2,  // ここから空き
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

#define TEXT_WININ_BACK_COLOR        (15)
#define TEXT_WININ_ZERO_COLOR        (0)

// フェード
#define FADE_IN_WAIT           (-16)       ///< フェードインのスピード  // BGやOBJのBELTが黒くしているのでフェードインは一瞬で行うのでマイナス
#define FADE_OUT_WAIT          (2)         ///< フェードアウトのスピード

#define STATUS_FADE_OUT_WAIT   (0)         ///< 技選択へ移行するためのフェードアウトのスピード
#define STATUS_FADE_IN_WAIT    (0)         ///< 技選択から戻ってくるためのフェードインのスピード

// バトルステータス用タスク
#define TCBSYS_TASK_MAX        (8)

// ステップ
typedef enum
{
  STEP_FADE_IN_BEFORE,              // フェードイン開始待ち
  STEP_FADE_IN,                     // フェードイン中
  STEP_BELT_OPEN,                   // ベルト開き中
  STEP_TEXT_SIGN_0,
  STEP_TEXT_SIGN_1,
  STEP_TEXT_SIGN,                   // 「おや！？　ようすが……！」表示中
  STEP_DEMO_CRY,                    // デモ鳴き中
  STEP_BGM_INTRO,                   // イントロBGM再生中
  STEP_DEMO_CHANGE,                 // デモ変化中  // 進化BGM再生開始  // 進化BGM再生中断

  // 進化した場合
  STEP_BGM_CONGRATULATE,            // おめでとうBGM再生開始
  STEP_TEXT_CONGRATULATE_0,
  STEP_TEXT_CONGRATULATE_1,
  STEP_TEXT_CONGRATULATE,           // 「おめでとう！　しんかした！」表示中

  // 進化キャンセルした場合
  STEP_BGM_CANCEL_SHINKA_POP,       // 進化BGMをPOPして再生開始
  STEP_TEXT_CANCEL_0,
  STEP_TEXT_CANCEL_1,
  STEP_TEXT_CANCEL,                 // 「あれ……？　へんかが　とまった！」表示中

  // 進化条件を処理する
  STEP_SHINKA_COND_CHECK,           // 進化条件を処理する

  // 技覚え
  STEP_WAZA_OBOE,                   // 技覚えチェック
  STEP_WAZA_VACANT_MASTER,          // 「あたらしく　おぼえた！」表示中(技が３つ以下だった場合)
  STEP_WAZA_FULL_PREPARE,           // 次のQの準備
  STEP_WAZA_FULL_Q,                 // 「ほかの　わざを　わすれさせますか？」表示中
  STEP_WAZA_FULL_YN,                // 選択

  // フィールド技選択
  STEP_WAZA_STATUS_FIELD_FADE_OUT,  // 技選択に移行するためのフェードアウト(フィールド)
  STEP_WAZA_STATUS_FIELD,           // 技選択PROC(フィールド)
  STEP_WAZA_STATUS_FIELD_OUT,       // 技選択PROCを抜けた後の処理(フィールド)
  STEP_WAZA_STATUS_FIELD_FADE_IN,   // 技選択から戻ってくるためのフェードイン(フィールド)

  // バトル技選択
  STEP_WAZA_STATUS_BATTLE_FADE_OUT, // 技選択に移行するためのフェードアウト(バトル)
  STEP_WAZA_STATUS_BATTLE,          // 技選択のフェードイン→技選択(バトル)
  STEP_WAZA_STATUS_BATTLE_OUT,      // 技選択のフェードアウト(バトル)
  STEP_WAZA_STATUS_BATTLE_FADE_IN,  // 技選択から戻ってくるためのフェードイン(バトル)

  // 技忘れ
  STEP_WAZA_CONFIRM_PREPARE,        // 次のQの準備
  STEP_WAZA_FORGET_0,
  STEP_WAZA_FORGET_1,
  STEP_WAZA_FORGET,                 // 「つかいかたを　きれいにわすれた！」表示中
  STEP_WAZA_FORGET_MASTER,          // 「あたらしく　おぼえた！」表示中
  STEP_WAZA_ABANDON_PREPARE,        // 次のQの準備
  STEP_WAZA_ABANDON_Q_0,
  STEP_WAZA_ABANDON_Q_1,
  STEP_WAZA_ABANDON_Q,              // 「おぼえるのを　あきらめますか？」表示中
  STEP_WAZA_ABANDON_YN,             // 選択
  STEP_WAZA_NOT_MASTER,             // 「おぼえずに　おわった！」表示中

  STEP_FADE_OUT_START,              // フェードアウト開始
  STEP_FADE_OUT_WAIT,               // フェードアウト中でBGMのフェードアウト完了待ち
  STEP_FADE_OUT_END,                // フェードアウト中で画面のフェードアウト完了待ち
  STEP_END,
}
STEP;

// サウンドステップ
typedef enum
{
  SOUND_STEP_WAIT,
  SOUND_STEP_FIELD_FADE_OUT,
  SOUND_STEP_INTRO,
  SOUND_STEP_SHINKA_LOAD,
  SOUND_STEP_SHINKA,
  SOUND_STEP_SHINKA_PUSH,
  SOUND_STEP_SHINKA_FADE_OUT,
  SOUND_STEP_CONGRATULATE_LOAD,
  SOUND_STEP_CONGRATULATE,
  SOUND_STEP_WAZAOBOE,
}
SOUND_STEP;

/*
サウンドヒープが足りないので無理
// サウンドデータプリセットテーブル
static const u32 preset_sound_table[] =
{
//  SEQ_BGM_SHINKA,
//  SEQ_BGM_KOUKAN,
  SEQ_ME_SHINKAOME,
};
*/

// VBlank中にテキストを表示するBG面のONとOFFを切り替える
typedef enum
{
  TEXT_SHOW_VBLANK_REQ_NONE,
  TEXT_SHOW_VBLANK_REQ_ON,
  TEXT_SHOW_VBLANK_REQ_OFF,
}
TEXT_SHOW_VBLANK_REQ;


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
  HEAPID                      heap_sys_id;  // 他PROCへ移行している際も解放しないシステムのヒープ
  HEAPID                      heap_id;
  SHINKADEMO_GRAPHIC_WORK*    graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // ポケモン
  POKEMON_PARAM*              pp;
  // (古い)ニックネーム
  STRBUF*                     poke_nick_name_strbuf;  // ポケモンの種族名がニックネームの場合、進化後にニックネームも変更されるので。
  // 忘れる技
  WazaID                      wazawasure_no;
  u8                          wazawasure_pos;  // 忘れる技の位置
  // 覚える技
  WazaID                      wazaoboe_no;
  int                         wazaoboe_index;

  // 進化キャンセル
  BOOL                        evo_cancel;  // 進化キャンセルしたときはTRUE

  // ステップ
  STEP                        step;
  u32                         wait_count;
  // サウンドステップ
  SOUND_STEP                  sound_step;
/*
サウンドヒープが足りないので無理 
  SOUNDMAN_PRESET_HANDLE*     sound_preset_handle;  // サウンドプリセット用ハンドル
*/
  u32                         sound_div_seq;  // BGMの分割ロードのシーケンス

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // OBJ
  u32                         obj_res[OBJ_RES_MAX];
  GFL_CLWK*                   obj_clwk[OBJ_CELL_MAX];
  BOOL                        obj_exist;  // TRUEのときobj_res, obj_clwk全てあり、FALSEのとき全てなし

  // TEXT
  PRINT_STREAM*               text_stream;
  GFL_TCBLSYS*                text_tcblsys;
  GFL_BMPWIN*                 text_winin_bmpwin;
  GFL_BMPWIN*                 text_dummy_bmpwin;      ///< 0番のキャラクターを1x1でつくっておく
  GFL_ARCUTIL_TRANSINFO       text_winfrm_tinfo;
  GFL_MSGDATA*                text_msgdata_shinka;
  GFL_MSGDATA*                text_msgdata_wazaoboe;
  STRBUF*                     text_strbuf;
  APP_KEYCURSOR_WORK*         text_keycursor_wk;  // メッセージ送りキーカーソルアイコン
  TEXT_SHOW_VBLANK_REQ        text_show_vblank_req;  // VBlank中にテキストを表示するBG面のONとOFFを切り替える
  BOOL                        text_bg_frame_show_state;  // BG_FRAME_M_TEXTが表示されているときTRUE

  // YESNO_MENU
  GFL_MSGDATA*                yesno_menu_msgdata;
  STRBUF*                     yesno_menu_strbuf_yes;
  STRBUF*                     yesno_menu_strbuf_no;
  YESNO_MENU_WORK*            yesno_menu_work;

  // バトルステータス
  GFL_TCBSYS*                 tcbsys;
  void*                       tcbsys_work;
  PALETTE_FADE_PTR            pfd;  // パレットフェード
  u8                          cursor_flag;
  BPLIST_DATA                 bplist_data;

  // フィールドステータス
  PSTATUS_DATA*               psData;

  // 進化デモの演出
  SHINKADEMO_VIEW_WORK*       view;
  // 進化デモのパーティクルと背景
  SHINKADEMO_EFFECT_WORK*     efwk;

  // SE
  BOOL                        se_play;         // デモのメイン部分を再生中にSEを鳴らしていいかどうかのフラグ
  BOOL                        se_to_white;     // 白く飛ばすときのSEを2度鳴らさないようにするためのフラグ
  BOOL                        se_from_white;   // 白から戻るときのSEを2度鳴らさないようにするためのフラグ
  BOOL                        rotate_se_play;  // 回転SEを鳴らしていなかったらFALSE、鳴らしていたらTRUE(今鳴っている最中という訳ではない)

  // ローカルPROCシステム
  GFL_PROCSYS*  local_procsys;
}
SHINKA_DEMO_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// 他PROCへ遷移するので、いつでもInitとExitができるようにしておく
static void ShinkaDemo_Init( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_Exit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );

// サウンド
static void ShinkaDemo_SoundInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundMain( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );

static BOOL ShinkaDemo_SoundCheckFadeOutField( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundFadeInField( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundPlayIntro( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static BOOL ShinkaDemo_SoundCheckPlayIntro( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundPlayShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundPushShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundPopShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static BOOL ShinkaDemo_SoundCheckPlayShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundFadeOutShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static BOOL ShinkaDemo_SoundCheckFadeOutShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundPlayCongratulate( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static BOOL ShinkaDemo_SoundCheckPlayCongratulate( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_SoundPlayWazaoboe( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );

// BG
static void ShinkaDemo_BgInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_BgExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );

// OBJ
static void ShinkaDemo_ObjInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_ObjInitAfterEvolution( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_ObjExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_ObjStartAnime( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static BOOL ShinkaDemo_ObjIsEndAnime( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );

// VBlank関数
static void ShinkaDemo_VBlankFunc( GFL_TCB* tcb, void* wk );

// テキスト
static void ShinkaDemo_TextInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_TextExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_TextMain( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static BOOL ShinkaDemo_TextStreamForgetCallBack( u32 arg );
static void ShinkaDemo_MakeTextStream( SHINKA_DEMO_WORK* work,
                                       GFL_MSGDATA* msgdata, u32 str_id, pPrintCallBack callback,
                                       TAG_REGIST_TYPE type0, const void* data0,
                                       TAG_REGIST_TYPE type1, const void* data1 );
static BOOL ShinkaDemo_WaitTextStream( SHINKA_DEMO_WORK* work );
static BOOL ShinkaDemo_TextWaitInput( SHINKA_DEMO_WORK* work );
static void ShinkaDemo_TextWininClear( SHINKA_DEMO_WORK* work, u8 col_code );
static void ShinkaDemo_TextWinfrmShow( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work, BOOL is_on );

// YESNO_MENU
static void ShinkaDemo_YesNoMenuInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_YesNoMenuExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_MakeYesNoMenuStrbuf( SHINKA_DEMO_WORK* work,
                                            GFL_MSGDATA* yes_msgdata, u32 yes_str_id,
                                            TAG_REGIST_TYPE yes_type0, const void* yes_data0,
                                            TAG_REGIST_TYPE yes_type1, const void* yes_data1,
                                            GFL_MSGDATA* no_msgdata, u32 no_str_id,
                                            TAG_REGIST_TYPE no_type0, const void* no_data0,
                                            TAG_REGIST_TYPE no_type1, const void* no_data1 );

// 文字列作成
static STRBUF* MakeStr( HEAPID heap_id,
                        GFL_MSGDATA* msgdata, u32 str_id,
                        TAG_REGIST_TYPE type0, const void* data0,
                        TAG_REGIST_TYPE type1, const void* data1 );

// 進化条件を処理する
static void ShinkaDemo_ShinkaCondCheckSpecialLevelup( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );
static void ShinkaDemo_ShinkaCondCheckSoubiUse( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
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


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT ShinkaDemoProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SHINKA_DEMO_PARAM*    param    = (SHINKA_DEMO_PARAM*)pwk;
  SHINKA_DEMO_WORK*     work;

  // ヒープ
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SHINKA_DEMO_SYS, HEAP_SYS_SIZE );
    
    work = GFL_PROC_AllocWork( proc, sizeof(SHINKA_DEMO_WORK), HEAPID_SHINKA_DEMO_SYS );
    GFL_STD_MemClear( work, sizeof(SHINKA_DEMO_WORK) );
    
    work->heap_sys_id   = HEAPID_SHINKA_DEMO_SYS;
  }

  // ステップ
  {
    // ステップ
    work->step          = STEP_FADE_IN_BEFORE;
    work->wait_count    = 5;  // 最初画面が乱れるので、しばし真っ暗のまま待つ
    // サウンドステップ
    work->sound_step    = SOUND_STEP_WAIT;
  }

  // ポケモン
  {
    // ポケモン
    work->pp            = PokeParty_GetMemberPointer( param->ppt, param->shinka_pos );
    // (古い)ニックネーム
    work->poke_nick_name_strbuf = GFL_STR_CreateBuffer( STRBUF_LENGTH, work->heap_sys_id );
    PP_Get( work->pp, ID_PARA_nickname, work->poke_nick_name_strbuf );
  }

  // 進化キャンセル
  {
    work->evo_cancel = FALSE;
  }

  // バトルステータス
  {
    work->tcbsys           = NULL;
    work->tcbsys_work      = NULL;
    work->pfd              = NULL;
    work->cursor_flag      = 0;
  }

  // フィールドステータス
  {
    work->psData      = GFL_HEAP_AllocClearMemory( work->heap_sys_id, sizeof(PSTATUS_DATA) );
  }

  // フェードイン(黒→黒)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, FADE_IN_WAIT );

  // サウンド
  ShinkaDemo_SoundInit( param, work );

  // VBlank中TCB
  {
    work->vblank_tcb = GFUser_VIntr_CreateTCB( ShinkaDemo_VBlankFunc, work, 1 );
  }

  // SE
  work->se_play        = FALSE;
  work->se_to_white    = FALSE;
  work->se_from_white  = FALSE;
  work->rotate_se_play = FALSE;

  // ローカルPROCシステムを作成
  work->local_procsys = GFL_PROC_LOCAL_boot( work->heap_sys_id );

  // 初期化処理
  ShinkaDemo_Init( param, work );
  // 通信アイコン
  GFL_NET_ReloadIconTopOrBottom( FALSE, work->heap_sys_id );


#ifdef NOT_USE_STATUS_BATTLE
  {
    // バトルステータスを使わないようにする
    param->b_field  = TRUE;
  }
#endif


  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT ShinkaDemoProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SHINKA_DEMO_PARAM*    param    = (SHINKA_DEMO_PARAM*)pwk;
  SHINKA_DEMO_WORK*     work     = (SHINKA_DEMO_WORK*)mywk;

  // 終了処理
  ShinkaDemo_Exit( param, work );
  // 通信アイコン
  // 終了するときは通信アイコンに対して何もしない

  // ローカルPROCシステムを破棄
  GFL_PROC_LOCAL_Exit( work->local_procsys ); 

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // サウンド
  ShinkaDemo_SoundExit( param, work );

  // フィールドステータス
  {
    GFL_HEAP_FreeMemory( work->psData );
  }   

  // バトルステータス
  {
    // ProcMain内で済ませているので、ここでは特にやることなし
  }

  // ポケモン
  {
    // (古い)ニックネーム
    GFL_STR_DeleteBuffer( work->poke_nick_name_strbuf );
  }

  // ヒープ
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_SHINKA_DEMO_SYS );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT ShinkaDemoProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SHINKA_DEMO_PARAM*    param    = (SHINKA_DEMO_PARAM*)pwk;
  SHINKA_DEMO_WORK*     work     = (SHINKA_DEMO_WORK*)mywk;

  // ローカルPROCの更新処理
  GFL_PROC_MAIN_STATUS  local_proc_status   =  GFL_PROC_LOCAL_Main( work->local_procsys );
  if( local_proc_status == GFL_PROC_MAIN_VALID ) return GFL_PROC_RES_CONTINUE;

  switch( work->step )
  {
  //---------------------------
  case STEP_FADE_IN_BEFORE:
    {
      if( work->wait_count == 0 )
      {
        // 次へ
        work->step = STEP_FADE_IN;

        // フェードイン(黒→見える)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, FADE_IN_WAIT );
      }
      else
      {
        work->wait_count--;
      }
    }
    break;
  case STEP_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->step = STEP_BELT_OPEN;

        ShinkaDemo_ObjStartAnime( param, work );
      }
    }
    break;
  case STEP_BELT_OPEN:
    {
      if(    ( !ShinkaDemo_SoundCheckFadeOutField( param, work ) )
          && ShinkaDemo_ObjIsEndAnime( param, work ) )
      {
        if( work->text_bg_frame_show_state )
        {
          // 次へ
          work->step = STEP_TEXT_SIGN_1;
        }
        else
        {
          // 次へ
          //work->step = STEP_TEXT_SIGN;
          work->step = STEP_TEXT_SIGN_0;

          ShinkaDemo_TextWinfrmShow( param, work, TRUE );
          //ShinkaDemo_MakeTextStream(
          //    work,
          //    work->text_msgdata_shinka, SHINKADEMO_ShinkaBeforeMsg, NULL,
          //    TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
          //    TAG_REGIST_TYPE_NONE, NULL );
        }
      }
    }
    break;
  case STEP_TEXT_SIGN_0:
    {
      work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_ON;
      // 次へ
      work->step = STEP_TEXT_SIGN_1;
    }
    break;
  case STEP_TEXT_SIGN_1:
    {
        ShinkaDemo_MakeTextStream(
            work,
            work->text_msgdata_shinka, SHINKADEMO_ShinkaBeforeMsg, NULL,
            TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
            TAG_REGIST_TYPE_NONE, NULL );
        // 次へ
        work->step = STEP_TEXT_SIGN;
    }
    break;
  case STEP_TEXT_SIGN:
    {
      if( ShinkaDemo_WaitTextStream( work ) )
      {
        if( ShinkaDemo_TextWaitInput( work ) )
        {
          //ShinkaDemo_TextWininClear( work, TEXT_WININ_ZERO_COLOR );
          ShinkaDemo_TextWinfrmShow( param, work, FALSE );

          // 次へ
          work->step = STEP_DEMO_CRY;
        
          // 進化デモ鳴き開始
          SHINKADEMO_VIEW_CryStart( work->view );
        }
      }
    }
    break;
  case STEP_DEMO_CRY:
    {
      // 進化デモ鳴きが終了したか
      if( SHINKADEMO_VIEW_CryIsEnd( work->view ) )
      {
        // 次へ
        work->step = STEP_BGM_INTRO;

        ShinkaDemo_SoundPlayIntro( param, work );
      }
    }
    break;
  case STEP_BGM_INTRO:
    {
      if( !ShinkaDemo_SoundCheckPlayIntro( param, work ) )
      {
        // 次へ
        work->step = STEP_DEMO_CHANGE;
        
        // 進化デモ変化開始
        SHINKADEMO_VIEW_ChangeStart( work->view );
        // 進化デモのパーティクルと背景
        SHINKADEMO_EFFECT_Start( work->efwk );

        PMSND_PlaySE( SEQ_SE_SHDEMO_01 );
        work->se_play = TRUE;
        work->wait_count = 0;
      }
    }
    break;
  case STEP_DEMO_CHANGE:
    {
      int key_trg = GFL_UI_KEY_GetTrg();
  
      // 進化デモ変化が終了したか
      if( SHINKADEMO_VIEW_ChangeIsEnd( work->view ) )
      {
        if( work->evo_cancel )
        {
          // 進化キャンセルした場合 
          work->step = STEP_BGM_CANCEL_SHINKA_POP;
        }
        else
        {
          // 進化した場合
          work->step = STEP_BGM_CONGRATULATE;
        }
      }
      else
      {
        // 進化BGM再生開始するか
        if( SHINKADEMO_VIEW_ChangeIsBgmShinkaStart( work->view ) )
        {
          ShinkaDemo_SoundPlayShinka( param, work ); 
        }
        // 進化BGM再生中断するか
        if( SHINKADEMO_VIEW_ChangeIsBgmShinkaPush( work->view ) )
        {
          ShinkaDemo_SoundPushShinka( param, work );
        }
        // 画面を白く飛ばすか
        if( SHINKADEMO_VIEW_ChangeIsToWhite( work->view ) )
        {
          SHINKADEMO_EFFECT_StartWhite( work->efwk );

          if( !(work->se_to_white) )
          {
            PMSND_PlaySE( SEQ_SE_SHDEMO_04 );
            work->se_play = FALSE;
            work->se_to_white = TRUE;
          }
        }
        // INDEPENDENTをMCSSに入れ替えスタート
        if( SHINKADEMO_EFFECT_IsWhite( work->efwk ) )
        {
          SHINKADEMO_VIEW_ChangeReplaceStart( work->view );
        }
        // ポケモンを白から戻すのをスタート
        if( SHINKADEMO_EFFECT_IsFromWhite( work->efwk) )
        {
          SHINKADEMO_VIEW_ChangeFromWhiteStart( work->view );
          
          if( !(work->se_from_white) )
          {
            // 進化した場合
            if( !(work->evo_cancel) )
            {
              work->se_play = TRUE;
            }
            work->se_from_white = TRUE;
          }
        }

        // SE
        if( work->se_play )
        {
          if(    ( !(work->rotate_se_play) )
              && ( work->wait_count >= 90 ) )
          {
            if( ShinkaDemo_SoundCheckPlayShinka( param, work ) )
            {
              PMSND_PlaySE( SEQ_SE_SHDEMO_02 );
              work->rotate_se_play = TRUE;
            }
          }
          else if(    work->wait_count == 530
                   || work->wait_count == 585
                   || work->wait_count == 640
                   || work->wait_count == 695
          )
          {
            PMSND_PlaySE( SEQ_SE_SHDEMO_03 );
          }
          // この間少しwait_countは止まっています
          else if( work->wait_count == 740 )
          {
            PMSND_PlaySE( SEQ_SE_SHDEMO_05 );
          }
          work->wait_count++;
        }

        // 進化キャンセルしたか
        if(    ( param->b_enable_cancel )
            && ShinkaDemo_SoundCheckPlayShinka( param, work )
            && ( !(work->evo_cancel) ) )
        {
          if( key_trg & PAD_BUTTON_B )
          {
            BOOL success_cancel = SHINKADEMO_VIEW_ChangeCancel( work->view );
            // 進化キャンセル成功
            if( success_cancel )
            {
              work->evo_cancel = TRUE;

              // タッチorキー
              GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );

              // 進化デモのパーティクルと背景
              SHINKADEMO_EFFECT_Cancel( work->efwk );
            }
            // 進化キャンセル失敗
            else
            {
              // 何もしない
            }
          }
        }
      }
    }
    break;

  //---------------------------
  // 進化した場合
  case STEP_BGM_CONGRATULATE:
    {
      // POKEMON_PARAMを進化させる
      {
        // 進化
        PP_ChangeMonsNo( work->pp, param->after_mons_no );

        // 図鑑登録（捕まえた）
        {
          if( param->gamedata )
          {
            ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( param->gamedata );
            ZUKANSAVE_SetPokeSee( zukan_savedata, work->pp );  // 見た  // 図鑑フラグをセットする
            ZUKANSAVE_SetPokeGet( zukan_savedata, work->pp );  // 捕まえた  // 図鑑フラグをセットする
          }
        }
      }

      ShinkaDemo_SoundPlayCongratulate( param, work );

      if( work->text_bg_frame_show_state )
      {
        // 次へ
        work->step = STEP_TEXT_CONGRATULATE_1;
      }
      else
      {
        ShinkaDemo_TextWinfrmShow( param, work, TRUE );
        //ShinkaDemo_MakeTextStream(
        //    work,
        //    work->text_msgdata_shinka, SHINKADEMO_ShinkaMsg, NULL,
        //    TAG_REGIST_TYPE_WORD, work->poke_nick_name_strbuf,
        //    TAG_REGIST_TYPE_POKE_MONS_NAME, work->pp );

        // 次へ
        //work->step = STEP_TEXT_CONGRATULATE;
        work->step = STEP_TEXT_CONGRATULATE_0;
      }
    }
    break;
  case STEP_TEXT_CONGRATULATE_0:
    {
      work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_ON;
      // 次へ
      work->step = STEP_TEXT_CONGRATULATE_1;
    }
    break;
  case STEP_TEXT_CONGRATULATE_1:
    {
      ShinkaDemo_MakeTextStream(
          work,
          work->text_msgdata_shinka, SHINKADEMO_ShinkaMsg, NULL,
          TAG_REGIST_TYPE_WORD, work->poke_nick_name_strbuf,
          TAG_REGIST_TYPE_POKE_MONS_NAME, work->pp );
      // 次へ
      work->step = STEP_TEXT_CONGRATULATE;
    }
    break;
  case STEP_TEXT_CONGRATULATE:
    {
      if(    ShinkaDemo_WaitTextStream( work )  // テキスト送りを行うためにサウンド待ちより前にチェックすることにした。
          && ( !ShinkaDemo_SoundCheckPlayCongratulate( param, work ) ) )
      //if( !ShinkaDemo_SoundCheckPlayCongratulate( param, work ) )
      { 
        //if( ShinkaDemo_WaitTextStream( work ) )
        {
          if( ShinkaDemo_TextWaitInput( work ) )
          {
            {
              // すれ違い用データをセット
              { 
                GAMEBEACON_Set_PokemonEvolution( PP_Get( work->pp, ID_PARA_monsno, NULL ), work->poke_nick_name_strbuf );
                // (古い)ニックネームは(新しい)種族名に進化した
              }

              // 進化させたポケモンの数
              // 1日にポケモン進化させた回数
              {
                RECORD* rec = GAMEDATA_GetRecordPtr(param->gamedata); 
                RECORD_Inc(rec, RECID_POKE_EVOLUTION);    // 進化させたポケモンの数
                RECORD_Inc(rec, RECID_DAYCNT_EVOLUTION);  // 1日にポケモン進化させた回数
              }
            }

            //// 次へ
            //work->step = STEP_WAZA_OBOE;
            //
            //work->wazaoboe_index = 0;
            //STEP_SHINKA_COND_CHECKを通ってからSTEP_WAZA_OBOEへ移行する

            // 次へ
            work->step = STEP_SHINKA_COND_CHECK;
          }
        }
      }
    }
    break;

  //---------------------------
  // 進化キャンセルした場合 
  case STEP_BGM_CANCEL_SHINKA_POP:
    {
      ShinkaDemo_SoundPopShinka( param, work );
    
      if( work->text_bg_frame_show_state )
      {
        // 次へ
        work->step = STEP_TEXT_CANCEL_1;
      }
      else
      {
        ShinkaDemo_TextWinfrmShow( param, work, TRUE );
        //ShinkaDemo_MakeTextStream(
        //    work,
        //    work->text_msgdata_shinka, SHINKADEMO_ShinkaCancelMsg, NULL,
        //    TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
        //    TAG_REGIST_TYPE_NONE, NULL );

        // 次へ
        //work->step = STEP_TEXT_CANCEL;
        work->step = STEP_TEXT_CANCEL_0;
      }
    }
    break;
  case STEP_TEXT_CANCEL_0:
    {
      work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_ON;
      // 次へ
      work->step = STEP_TEXT_CANCEL_1;
    }
    break;
  case STEP_TEXT_CANCEL_1:
    {
      ShinkaDemo_MakeTextStream(
          work,
          work->text_msgdata_shinka, SHINKADEMO_ShinkaCancelMsg, NULL,
          TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
          TAG_REGIST_TYPE_NONE, NULL );
      // 次へ
      work->step = STEP_TEXT_CANCEL;
    }
    break;
  case STEP_TEXT_CANCEL:
    {
      if( ShinkaDemo_WaitTextStream( work ) )
      {
        if( ShinkaDemo_TextWaitInput( work ) )
        {
          // 次へ
          work->step = STEP_FADE_OUT_START;
        }
      }
    }
    break;

  //---------------------------
  // 進化条件を処理する
  case STEP_SHINKA_COND_CHECK:
    {
      switch( param->shinka_cond )
      {
      // ヌケニンを誕生させる
      case SHINKA_COND_SPECIAL_LEVELUP:  // SHINKA_COND_SPECIAL_NUKENINではなくSHINKA_COND_SPECIAL_LEVELUP(SHINKA_COND_SPECIAL_LEVELUPはツチニンでしか得られない値)
        {
          ShinkaDemo_ShinkaCondCheckSpecialLevelup( param, work );
        }
        break;
      // 装備アイテムを消す
      case SHINKA_COND_TUUSHIN_ITEM:
      case SHINKA_COND_SOUBI_NOON:
      case SHINKA_COND_SOUBI_NIGHT:
        {
          ShinkaDemo_ShinkaCondCheckSoubiUse( param, work );
        }
        break;
      }

      // 次へ
      work->step = STEP_WAZA_OBOE;
            
      work->wazaoboe_index = 0;
    }
    break;

  //---------------------------
  // 技覚え 
  case STEP_WAZA_OBOE:
    {
      {
        // 技覚えチェック
        work->wazaoboe_no = PP_CheckWazaOboe( work->pp, &work->wazaoboe_index, work->heap_sys_id );
        if( work->wazaoboe_no == PTL_WAZAOBOE_NONE )
        {
          // 次へ
          work->step = STEP_FADE_OUT_START;
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
          ShinkaDemo_MakeTextStream(
            work,
            work->text_msgdata_wazaoboe, msg_waza_oboe_04, NULL,
            TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
            TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
          
          ShinkaDemo_SoundPlayWazaoboe( param, work );
        }
      }
    }
    break;
  case STEP_WAZA_VACANT_MASTER:
    {
      if( ShinkaDemo_WaitTextStream( work ) )
      {
        if( ShinkaDemo_TextWaitInput( work ) )
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
      ShinkaDemo_MakeTextStream(
        work,
        work->text_msgdata_wazaoboe, msg_waza_oboe_05, NULL,
        TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
        TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
    }
    break;
  case STEP_WAZA_FULL_Q:
    {
      if( ShinkaDemo_WaitTextStream( work ) )
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
          if( param->b_field )
          {
            // 次へ
            work->step = STEP_WAZA_STATUS_FIELD_FADE_OUT;

            // フェードアウト(見える→黒)
            GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, STATUS_FADE_OUT_WAIT );
          }
          else
          {
            // 次へ
            work->step = STEP_WAZA_STATUS_BATTLE_FADE_OUT;

            // フェードアウト(見える→黒)
            GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, STATUS_FADE_OUT_WAIT );
          }
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

  //---------------------------
  // フィールド技選択 
  case STEP_WAZA_STATUS_FIELD_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 終了処理
        ShinkaDemo_Exit( param, work );
        // 通信アイコン
        // 終了するときは通信アイコンに対して何もしない

        // 次へ
        work->step = STEP_WAZA_STATUS_FIELD;

        {
          // フィールドステータス
          SAVE_CONTROL_WORK* svWork = GAMEDATA_GetSaveControlWork( param->gamedata );
          
          work->psData->ppt               = PST_PP_TYPE_POKEPARTY;
          work->psData->game_data         = param->gamedata;
          work->psData->isExitRequest     = FALSE;

          work->psData->ppd               = (void*)(param->ppt);
          work->psData->cfg               = SaveData_GetConfig( svWork );
          work->psData->max               = PokeParty_GetPokeCount( param->ppt );
          work->psData->pos               = param->shinka_pos;

          work->psData->waza              = work->wazaoboe_no;
          work->psData->mode              = PST_MODE_WAZAADD;
          work->psData->page              = PPT_SKILL;
          work->psData->canExitButton     = FALSE;

          GFL_OVERLAY_Load( FS_OVERLAY_ID(poke_status) );
          // ローカルPROC呼び出し
          GFL_PROC_LOCAL_CallProc( work->local_procsys, NO_OVERLAY_ID, &PokeStatus_ProcData, work->psData );
        }
      }
    }
    break;
  case STEP_WAZA_STATUS_FIELD:
    {
      // ローカルPROCが終了するのを待つ  // このMainの最初でGFL_PROC_MAIN_VALIDならreturnしているので、ここでは判定しなくてもよいが念のため
      if( local_proc_status != GFL_PROC_MAIN_VALID )
      {
        // 次へ
        work->step = STEP_WAZA_STATUS_FIELD_OUT;
      }
      else
      {
        return GFL_PROC_RES_CONTINUE;
      }
    }
    break;
  case STEP_WAZA_STATUS_FIELD_OUT:
    {
      {
        // フィールドステータス
        GFL_OVERLAY_Unload( FS_OVERLAY_ID(poke_status) );
      }

      // 次へ
      work->step = STEP_WAZA_STATUS_FIELD_FADE_IN;
 
      // 初期化処理
      ShinkaDemo_Init( param, work );
      // 通信アイコン
      GFL_NET_ReloadIconTopOrBottom( FALSE, work->heap_sys_id );

      // フェードイン(黒→見える)
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, STATUS_FADE_IN_WAIT );
    }
    break;
  case STEP_WAZA_STATUS_FIELD_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        switch( work->psData->ret_mode )
        {
        case PST_RET_DECIDE:
          {
            work->wazawasure_pos  = work->psData->ret_sel;

            // 忘れる技
            work->wazawasure_no = PP_Get( work->pp, ID_PARA_waza1 + work->wazawasure_pos, NULL );

            // 次へ
            work->step = STEP_WAZA_CONFIRM_PREPARE;
          }
          break;
        case PST_RET_CANCEL:
        case PST_RET_EXIT:
        default:
          {
            // 次へ
            work->step = STEP_WAZA_ABANDON_PREPARE;
          }
          break;
        }
      }
    }
    break;

  //---------------------------
  // バトル技選択 
  case STEP_WAZA_STATUS_BATTLE_FADE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->step = STEP_WAZA_STATUS_BATTLE;

        // 下画面の破棄
        YESNO_MENU_DeleteRes( work->yesno_menu_work );
        work->yesno_menu_work = NULL;
        SHINKADEMO_GRAPHIC_ExitBGSub( work->graphic );

        // キーorタッチ
        {
          work->cursor_flag = ( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY );
        } 

        // バトルステータス用タスク
        work->tcbsys_work = GFL_HEAP_AllocMemory( work->heap_sys_id, GFL_TCB_CalcSystemWorkSize(TCBSYS_TASK_MAX) );
        GFL_STD_MemClear( work->tcbsys_work, GFL_TCB_CalcSystemWorkSize(TCBSYS_TASK_MAX) );
        work->tcbsys = GFL_TCB_Init( TCBSYS_TASK_MAX, work->tcbsys_work );
 
        // パレットフェード
        work->pfd = PaletteFadeInit( work->heap_sys_id );
        PaletteTrans_AutoSet( work->pfd, TRUE );
        PaletteFadeWorkAllocSet( work->pfd, FADE_SUB_BG, 0x1e0, work->heap_sys_id );
        PaletteFadeWorkAllocSet( work->pfd, FADE_SUB_OBJ, 0x1e0, work->heap_sys_id );

        // バトルステータス
        work->bplist_data.gamedata    = param->gamedata;
        work->bplist_data.pp          = (POKEPARTY*)param->ppt;
        work->bplist_data.font        = work->font;
        work->bplist_data.heap        = work->heap_sys_id;
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
  case STEP_WAZA_STATUS_BATTLE:
    {
      // バトルステータス
      GFL_TCB_Main( work->tcbsys );

      if( work->bplist_data.end_flg )
      {
        // 次へ
        work->step = STEP_WAZA_STATUS_BATTLE_OUT;

        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, STATUS_FADE_OUT_WAIT );
      }
    }
    break;
  case STEP_WAZA_STATUS_BATTLE_OUT:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->step = STEP_WAZA_STATUS_BATTLE_FADE_IN;

        // バトルステータス
        GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_b_app ) );
        GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_plist ) );

        // パレットフェード
        PaletteFadeWorkAllocFree( work->pfd, FADE_SUB_BG );
        PaletteFadeWorkAllocFree( work->pfd, FADE_SUB_OBJ );
        PaletteFadeFree( work->pfd );

        // バトルステータス用タスク
        GFL_TCB_Exit( work->tcbsys );
        GFL_HEAP_FreeMemory( work->tcbsys_work );

        // 初期化
        work->tcbsys           = NULL;
        work->tcbsys_work      = NULL;
        work->pfd              = NULL;

        // 下画面の生成
        SHINKADEMO_GRAPHIC_InitBGSub( work->graphic );
        work->yesno_menu_work = YESNO_MENU_CreateRes( work->heap_sys_id,
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
  case STEP_WAZA_STATUS_BATTLE_FADE_IN:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // バトルステータス
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

        // キーorタッチ
        {
          GFL_UI_SetTouchOrKey( (work->cursor_flag)?(GFL_APP_END_KEY):(GFL_APP_END_TOUCH) );
        } 
      }
    }
    break;

  //---------------------------
  // 技忘れ 
  case STEP_WAZA_CONFIRM_PREPARE:
    {
      if( work->text_bg_frame_show_state )
      {
        // 次へ
        work->step = STEP_WAZA_FORGET_1;
      }
      else
      {
        // 次へ
        //work->step = STEP_WAZA_FORGET;
        work->step = STEP_WAZA_FORGET_0;
      
        ShinkaDemo_TextWinfrmShow( param, work, TRUE );  // フィールド技選択の後は消えているので
        //ShinkaDemo_MakeTextStream(
        //  work,
        //  work->text_msgdata_wazaoboe, msg_waza_oboe_06, ShinkaDemo_TextStreamForgetCallBack,
        //  TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
        //  TAG_REGIST_TYPE_WAZA, &work->wazawasure_no );
      }
    }
    break;
  case STEP_WAZA_FORGET_0:
    {
      work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_ON;
      // 次へ
      work->step = STEP_WAZA_FORGET_1;
    }
    break;
  case STEP_WAZA_FORGET_1:
    {
      ShinkaDemo_MakeTextStream(
        work,
        work->text_msgdata_wazaoboe, msg_waza_oboe_06, ShinkaDemo_TextStreamForgetCallBack,
        TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
        TAG_REGIST_TYPE_WAZA, &work->wazawasure_no );
      // 次へ
      work->step = STEP_WAZA_FORGET;
    }
    break;
  case STEP_WAZA_FORGET:
    {
      if( ShinkaDemo_WaitTextStream( work ) )
      {
        // 技覚えた
        PP_SetWazaPos( work->pp, work->wazaoboe_no, work->wazawasure_pos );
        
        // 次へ
        work->step = STEP_WAZA_FORGET_MASTER;

        ShinkaDemo_MakeTextStream(
          work,
          work->text_msgdata_wazaoboe, msg_waza_oboe_07, NULL,
          TAG_REGIST_TYPE_POKE_NICK_NAME, work->pp,
          TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );

        ShinkaDemo_SoundPlayWazaoboe( param, work );
      }
    }
    break;
  case STEP_WAZA_FORGET_MASTER:
    {
      if( ShinkaDemo_WaitTextStream( work ) )
      {
         // 次へ
        work->step = STEP_WAZA_OBOE;
      }
    }
    break;
  case STEP_WAZA_ABANDON_PREPARE:
    {
      if( work->text_bg_frame_show_state )
      {
        // 次へ
        work->step = STEP_WAZA_ABANDON_Q_1;
      }
      else
      { 
        // 次へ
        //work->step = STEP_WAZA_ABANDON_Q;
        work->step = STEP_WAZA_ABANDON_Q_0;

        ShinkaDemo_TextWinfrmShow( param, work, TRUE );  // フィールド技選択の後は消えているので
        //ShinkaDemo_MakeTextStream(
        //  work,
        //  work->text_msgdata_wazaoboe, msg_waza_oboe_08, NULL,
        //  TAG_REGIST_TYPE_NONE, NULL,
        //  TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
      }
    }
    break;
  case STEP_WAZA_ABANDON_Q_0:
    {
      work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_ON;
      // 次へ
      work->step = STEP_WAZA_ABANDON_Q_1;
    }
    break;
  case STEP_WAZA_ABANDON_Q_1:
    {
      ShinkaDemo_MakeTextStream(
        work,
        work->text_msgdata_wazaoboe, msg_waza_oboe_08, NULL,
        TAG_REGIST_TYPE_NONE, NULL,
        TAG_REGIST_TYPE_WAZA, &work->wazaoboe_no );
      // 次へ
      work->step = STEP_WAZA_ABANDON_Q;
    }
    break;
  case STEP_WAZA_ABANDON_Q:
    {
      if( ShinkaDemo_WaitTextStream( work ) )
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

          ShinkaDemo_MakeTextStream(
            work,
            work->text_msgdata_wazaoboe, msg_waza_oboe_09, NULL,
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
      if( ShinkaDemo_WaitTextStream( work ) )
      {
        // 次へ
        work->step = STEP_WAZA_OBOE;
      }
    }
    break;

  //---------------------------
  case STEP_FADE_OUT_START:
    {
      if( ShinkaDemo_SoundCheckPlayShinka( param, work ) )
      {
        ShinkaDemo_SoundFadeOutShinka( param, work );
   
        // フェードアウト(見える→黒)
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, FADE_OUT_WAIT );

        // 次へ
        work->step = STEP_FADE_OUT_WAIT;
      }
    }
    break;
  case STEP_FADE_OUT_WAIT:
    {
      if( !ShinkaDemo_SoundCheckFadeOutShinka( param, work ) )
      {
        // 次へ
        work->step = STEP_FADE_OUT_END;
        
        ShinkaDemo_SoundFadeInField( param, work );
      }
    }
    break;
  case STEP_FADE_OUT_END:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // 次へ
        work->step = STEP_END;

        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  case STEP_END:
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  ShinkaDemo_SoundMain( param, work );

  if(    work->step != STEP_WAZA_STATUS_FIELD
      && work->step != STEP_WAZA_STATUS_FIELD_OUT )
  {
    if( work->yesno_menu_work ) YESNO_MENU_Main( work->yesno_menu_work );

    ShinkaDemo_TextMain( param, work );

    PRINTSYS_QUE_Main( work->print_que );

    SHINKADEMO_VIEW_Main( work->view );
    // 進化デモのパーティクルと背景
    SHINKADEMO_EFFECT_Main( work->efwk );

    // 2D描画
    SHINKADEMO_GRAPHIC_2D_Draw( work->graphic );
    // 3D描画
    SHINKADEMO_GRAPHIC_3D_StartDraw( work->graphic );
#if 0 
    // 半透明ポケモンより手前にエフェクトと背景がある場合
    SHINKADEMO_VIEW_Draw( work->view );
    // 進化デモのパーティクルと背景
    SHINKADEMO_EFFECT_Draw( work->efwk );
#else
    // エフェクトと背景より手前に半透明ポケモンがある場合
    // 進化デモのパーティクルと背景
    SHINKADEMO_EFFECT_Draw( work->efwk );
    SHINKADEMO_VIEW_Draw( work->view );
#endif
    SHINKADEMO_GRAPHIC_3D_EndDraw( work->graphic );
  }

  return GFL_PROC_RES_CONTINUE;
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// 初期化処理
//=====================================
static void ShinkaDemo_Init( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  // ヒープ
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SHINKA_DEMO, HEAP_SIZE );
    work->heap_id       = HEAPID_SHINKA_DEMO;
  }

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID( ui_common ) );

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
    // グラフィック、フォントなど
    work->graphic       = SHINKADEMO_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    SHINKADEMO_GRAPHIC_InitBGSub( work->graphic );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // テキストの初期化処理
  ShinkaDemo_TextInit( param, work );
  // YESNO_MENUの初期化処理
  work->yesno_menu_work = NULL;
  ShinkaDemo_YesNoMenuInit( param, work );
  // BG
  ShinkaDemo_BgInit( param, work );
  // OBJ
  {
    work->obj_exist = FALSE;
    if( work->step == STEP_FADE_IN_BEFORE )
    {
      ShinkaDemo_ObjInit( param, work );
    }
    else
    {
      // OBJ幕が開いたところから始まるので、OBJは生成しないことにする(OBJは開けばBGと同じところに来るので、なくてもいい)
      //ShinkaDemo_ObjInitAfterEvolution( param, work );
    }
  }

  // プライオリティ、表示、背景色など
  {
    GFL_BG_SetPriority( BG_FRAME_M_POKEMON               , BG_FRAME_PRI_M_POKEMON );
    GFL_BG_SetPriority( BG_FRAME_M_BELT                  , BG_FRAME_PRI_M_BELT    );
    GFL_BG_SetPriority( BG_FRAME_M_TEXT                  , BG_FRAME_PRI_M_TEXT    );  // 最前面

    GFL_BG_SetPriority( BG_FRAME_S_BACK                  , BG_FRAME_PRI_S_BACK );
    //GFL_BG_SetPriority( BG_FRAME_S_BUTTON                , BG_FRAME_PRI_S_BUTTON );
    //GFL_BG_SetPriority( BG_FRAME_S_TEXT                  , BG_FRAME_PRI_S_TEXT );  // 最前面

    GFL_BG_SetVisible( BG_FRAME_M_POKEMON                , VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_M_BELT                   , VISIBLE_ON );
    //GFL_BG_SetVisible( BG_FRAME_M_TEXT                   , VISIBLE_ON );
  
    GFL_BG_SetVisible( BG_FRAME_S_BACK                  , VISIBLE_ON );
    //GFL_BG_SetVisible( BG_FRAME_S_BUTTON                , VISIBLE_ON );
    //GFL_BG_SetVisible( BG_FRAME_S_TEXT                  , VISIBLE_ON );
  
    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );
    GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x0000 );
  }

  // param修正
  {
    if( work->step == STEP_FADE_IN_BEFORE )
    {
      // 進化条件
      if(    param->shinka_cond == SHINKA_COND_ITEM           // 「たいようのいし」などを使って進化した場合は、進化キャンセルさせない
          || param->shinka_cond == SHINKA_COND_ITEM_MALE
          || param->shinka_cond == SHINKA_COND_ITEM_FEMALE )
      {
        param->b_enable_cancel = FALSE;
      }
    }
    else
    {
      // 何もしない
    }
  }

  // 進化デモの演出
  {
    if( work->step == STEP_FADE_IN_BEFORE )
    {
      work->view = SHINKADEMO_VIEW_Init(
                     work->heap_id,
                     SHINKADEMO_VIEW_LAUNCH_EVO,
                     work->pp,
                     param->after_mons_no
                   );
    }
    else
    {
      work->view = SHINKADEMO_VIEW_Init(
                     work->heap_id,
                     SHINKADEMO_VIEW_LAUNCH_AFTER,
                     work->pp,
                     param->after_mons_no
                   );
    }
  }

  // 進化デモのパーティクルと背景
  {
    if( work->step == STEP_FADE_IN_BEFORE )
    {
      work->efwk = SHINKADEMO_EFFECT_Init( work->heap_id, SHINKADEMO_EFFECT_LAUNCH_EVO );
    }
    else
    {
      work->efwk = SHINKADEMO_EFFECT_Init( work->heap_id, SHINKADEMO_EFFECT_LAUNCH_AFTER );
    }
  }

  // パーティクル対応
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0,
                    GX_BLEND_PLANEMASK_BD | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1,
                    0, 0 );
    // パーティクルのアルファをきれいに出すため
    // ev1とev2は使われない  // TWLプログラミングマニュアル「2D面とのαブレンディング」参考
}

//-------------------------------------
/// 終了処理
//=====================================
static void ShinkaDemo_Exit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  // 進化デモのパーティクルと背景
  {
    SHINKADEMO_EFFECT_Exit( work->efwk );
  }

  // 進化デモの演出
  {
    SHINKADEMO_VIEW_Exit( work->view );
  }

  // OBJ
  {
    ShinkaDemo_ObjExit( param, work );
  }
  // BG
  ShinkaDemo_BgExit( param, work );
  // YESNO_MENUの終了処理
  ShinkaDemo_YesNoMenuExit( param, work );
  // テキストの終了処理
  ShinkaDemo_TextExit( param, work );

  // SHINKA_DEMO_WORK後片付け
  {
    // グラフィック、フォントなど
    PRINTSYS_QUE_Clear( work->print_que );
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    SHINKADEMO_GRAPHIC_ExitBGSub( work->graphic );
    SHINKADEMO_GRAPHIC_Exit( work->graphic );
  }
  
  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( ui_common ) );

  // ヒープ
  {
    GFL_HEAP_DeleteHeap( HEAPID_SHINKA_DEMO );
  }
}

//-------------------------------------
/// サウンド初期化処理
//=====================================
static void ShinkaDemo_SoundInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  PMSND_FadeOutBGM( FSND_FADE_SHORT );
  work->sound_step = SOUND_STEP_FIELD_FADE_OUT;
}
//-------------------------------------
/// サウンド終了処理
//=====================================
static void ShinkaDemo_SoundExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
/*
サウンドヒープが足りないので無理
  // サウンドデータプリセット
  SOUNDMAN_ReleasePresetData( work->sound_preset_handle );
*/
}
//-------------------------------------
/// サウンド主処理
//=====================================
static void ShinkaDemo_SoundMain( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
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

/*
サウンドヒープが足りないので無理
        // サウンドデータプリセット
        work->sound_preset_handle = SOUNDMAN_PresetSoundTbl( preset_sound_table, NELEMS(preset_sound_table) );
*/
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
  case SOUND_STEP_SHINKA_LOAD:
    {
      // BGMの分割ロードを利用してみる
      BOOL play_start = PMSND_PlayBGMdiv( SEQ_BGM_KOUKAN, &(work->sound_div_seq), FALSE );
      if( play_start )
      {
        work->sound_step = SOUND_STEP_SHINKA;
      }
    }
    break;
  case SOUND_STEP_SHINKA:
    {
    }
    break;
  case SOUND_STEP_SHINKA_PUSH:  // SHINKA曲をpushしており何も鳴っていない
    {
    }
    break;
  case SOUND_STEP_SHINKA_FADE_OUT:  // SHINKA曲を停止する
    {
      if( !PMSND_CheckFadeOnBGM() )
      {
        PMSND_StopBGM();
        work->sound_step = SOUND_STEP_WAIT;
      }
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
  case SOUND_STEP_CONGRATULATE:  // SOUND_STEP_SHINKA_PUSHを経ている
    {
      if( !PMSND_CheckPlayBGM() )
      {
        ShinkaDemo_SoundPopShinka( param, work );
      }
    }
    break;
  case SOUND_STEP_WAZAOBOE:  // SOUND_STEP_SHINKA_PUSHを経ている
    {
      if( !PMSND_CheckPlayBGM() )
      {
        ShinkaDemo_SoundPopShinka( param, work );
      }
    }
    break;
  }
}
//-------------------------------------
/// サウンド
//=====================================
static BOOL ShinkaDemo_SoundCheckFadeOutField( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  return ( work->sound_step == SOUND_STEP_FIELD_FADE_OUT );
}
static void ShinkaDemo_SoundFadeInField( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_WAIT )
  {
    // フィールドBGM
    PMSND_PopBGM();
    PMSND_PauseBGM( FALSE );
    PMSND_FadeInBGM( FSND_FADE_NORMAL );
  }
}
static void ShinkaDemo_SoundPlayIntro( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_WAIT )
  {
    PMSND_PlayBGM(SEQ_BGM_SHINKA);  // Introの曲名がSHINKA
    work->sound_step = SOUND_STEP_INTRO;
  }
}
static BOOL ShinkaDemo_SoundCheckPlayIntro( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  return ( work->sound_step == SOUND_STEP_INTRO );
}
static void ShinkaDemo_SoundPlayShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
/*
BGMの分割ロードを利用してみることにしたのでコメントアウト
  if( work->sound_step == SOUND_STEP_WAIT )
  {
    PMSND_PlayBGM(SEQ_BGM_KOUKAN);  // Shinkaの曲名がKOUKAN
    work->sound_step = SOUND_STEP_SHINKA;
  }
*/
  if( work->sound_step == SOUND_STEP_WAIT )
  {
    // BGMの分割ロードを利用してみる
    work->sound_div_seq = 0;
    PMSND_PlayBGMdiv( SEQ_BGM_KOUKAN, &(work->sound_div_seq), TRUE );  // Shinkaの曲名がKOUKAN
    work->sound_step = SOUND_STEP_SHINKA_LOAD;
  }
}
static void ShinkaDemo_SoundPushShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_SHINKA )
  {
    // ここはフェードしなくていい
    PMSND_PauseBGM( TRUE );
    PMSND_PushBGM();
    work->sound_step = SOUND_STEP_SHINKA_PUSH;
  }
}
static void ShinkaDemo_SoundPopShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if(    work->sound_step == SOUND_STEP_SHINKA_PUSH
      || work->sound_step == SOUND_STEP_CONGRATULATE
      || work->sound_step == SOUND_STEP_WAZAOBOE )
  {
    PMSND_PopBGM();
    PMSND_PauseBGM( FALSE );
    PMSND_FadeInBGM( FSND_FADE_FAST );
    work->sound_step = SOUND_STEP_SHINKA;
  }
}
static BOOL ShinkaDemo_SoundCheckPlayShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  return ( work->sound_step == SOUND_STEP_SHINKA );
}
static void ShinkaDemo_SoundFadeOutShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_SHINKA )
  {
    PMSND_FadeOutBGM( FSND_FADE_NORMAL );
    work->sound_step = SOUND_STEP_SHINKA_FADE_OUT;
  }
}
static BOOL ShinkaDemo_SoundCheckFadeOutShinka( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  return ( work->sound_step == SOUND_STEP_SHINKA_FADE_OUT );
}
static void ShinkaDemo_SoundPlayCongratulate( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_SHINKA_PUSH )
  {
/*
BGMの分割ロードを利用してみることにしたのでコメントアウト
    PMSND_PlayBGM(SEQ_ME_SHINKAOME);
    work->sound_step = SOUND_STEP_CONGRATULATE;
*/
    // BGMの分割ロードを利用してみる
    work->sound_div_seq = 0;
    PMSND_PlayBGMdiv( SEQ_ME_SHINKAOME, &(work->sound_div_seq), TRUE );
    work->sound_step = SOUND_STEP_CONGRATULATE_LOAD;
  }
}
static BOOL ShinkaDemo_SoundCheckPlayCongratulate( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  return ( work->sound_step == SOUND_STEP_CONGRATULATE_LOAD || work->sound_step == SOUND_STEP_CONGRATULATE );
}
static void ShinkaDemo_SoundPlayWazaoboe( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->sound_step == SOUND_STEP_SHINKA )
  {
    ShinkaDemo_SoundPushShinka( param, work );
    PMSND_PlayBGM( SEQ_ME_LVUP );  // btl_client.cを参考にした  // MEの場合は、自動的にpush, popしてくれるらしいが、音の終わりを知りたいので手動でpush, popすることにした。
    work->sound_step = SOUND_STEP_WAZAOBOE;
  }
}

//-------------------------------------
/// BG
//=====================================
static void ShinkaDemo_BgInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_SHINKA_DEMO, work->heap_id );

  // BELT
  GFL_ARCHDL_UTIL_TransVramPalette(
      handle,
      NARC_shinka_demo_particle_demo_egg_NCLR,
      PALTYPE_MAIN_BG,
      BG_PAL_POS_M_BELT * 0x20,
      BG_PAL_NUM_M_BELT * 0x20,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
      handle,
      NARC_shinka_demo_particle_demo_egg_bg_NCGR,
      BG_FRAME_M_BELT,
			0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      NARC_shinka_demo_particle_demo_egg_NSCR,
      BG_FRAME_M_BELT,
      0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenReq( BG_FRAME_M_BELT );
}
static void ShinkaDemo_BgExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  // 何もしない
}

//-------------------------------------
/// OBJ
//=====================================
static void ShinkaDemo_ObjInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  u8 i;

  // リソース読み込み
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_SHINKA_DEMO, work->heap_id );

  work->obj_res[OBJ_RES_BELT_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
          handle,
          NARC_shinka_demo_particle_demo_egg_NCLR,
          CLSYS_DRAW_MAIN,
          OBJ_PAL_POS_M_BELT * 0x20,
          0,
          OBJ_PAL_NUM_M_BELT,
          work->heap_id );
      
  work->obj_res[OBJ_RES_BELT_NCG] = GFL_CLGRP_CGR_Register(
          handle,
          NARC_shinka_demo_particle_demo_egg_NCGR,
          FALSE,
          CLSYS_DRAW_MAIN,
          work->heap_id );

  work->obj_res[OBJ_RES_BELT_NCE] = GFL_CLGRP_CELLANIM_Register(
          handle,
          NARC_shinka_demo_particle_demo_egg_NCER,
          NARC_shinka_demo_particle_demo_egg_NANR,
          work->heap_id );
  
  GFL_ARC_CloseDataHandle( handle );

  // CLWK作成
  for( i=0; i<OBJ_CELL_MAX; i++ )
  {
    work->obj_clwk[i] = GFL_CLACT_WK_Create(
        SHINKADEMO_GRAPHIC_GetClunit( work->graphic ),
        work->obj_res[OBJ_RES_BELT_NCG],
        work->obj_res[OBJ_RES_BELT_NCL],
        work->obj_res[OBJ_RES_BELT_NCE],
        &obj_cell_data[i],
        CLSYS_DEFREND_MAIN,
        work->heap_id );
    GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[i], FALSE );
  }

  work->obj_exist = TRUE;
}
static void ShinkaDemo_ObjInitAfterEvolution( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  ShinkaDemo_ObjInit( param, work );

  {
    u8 i;
    for( i=0; i<OBJ_CELL_MAX; i++ )
    {
      GFL_CLACT_WK_AddAnmFrame( work->obj_clwk[i], FX32_CONST(10000) );  // 1回でアニメが終わるように、十分大きな数字を与える
      GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[i], TRUE );
    }
  }
}
static void ShinkaDemo_ObjExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->obj_exist )
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

    work->obj_exist = FALSE;
  }
}
static void ShinkaDemo_ObjStartAnime( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->obj_exist )
  {
    u8 i;
    for( i=0; i<OBJ_CELL_MAX; i++ )
    {
      GFL_CLACT_WK_SetAnmIndex( work->obj_clwk[i], 0 );
      GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[i], TRUE );
    }
  }
}
static BOOL ShinkaDemo_ObjIsEndAnime( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  if( work->obj_exist )
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
    if( b_end )  // OBJ幕が開き切ったら、もうなくてもいいのでなしにする
    {
      ShinkaDemo_ObjExit( param, work );
    }
    return b_end;
  }
  else
  {
    return TRUE;
  }
}

//-------------------------------------
/// VBlank関数
//=====================================
static void ShinkaDemo_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  SHINKA_DEMO_WORK* work = (SHINKA_DEMO_WORK*)wk;

  // VBlank中にテキストを表示するBG面のONとOFFを切り替える
  switch( work->text_show_vblank_req )
  {
  case TEXT_SHOW_VBLANK_REQ_ON:
    {
      GFL_BG_SetVisible( BG_FRAME_M_TEXT                   , VISIBLE_ON );
      work->text_bg_frame_show_state = TRUE;
      work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_NONE;
    }
    break;
  case TEXT_SHOW_VBLANK_REQ_OFF:
    {
      GFL_BG_SetVisible( BG_FRAME_M_TEXT                   , VISIBLE_OFF );
      work->text_bg_frame_show_state = FALSE;
      work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_NONE;
    }
    break;
  }

  // バトルステータス
  {
    // パレットフェード
    if( work->pfd ) PaletteFadeTrans( work->pfd );
  }
}

//-------------------------------------
/// テキスト初期化処理
//=====================================
static void ShinkaDemo_TextInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  // TEXT
  {
    // パレット
    GFL_ARC_UTIL_TransVramPalette(
        ARCID_FONT,
        NARC_font_default_nclr,
        PALTYPE_MAIN_BG,
        BG_PAL_POS_M_TEXT_FONT * 0x20,
        BG_PAL_NUM_M_TEXT_FONT * 0x20,
        work->heap_id );

    // BGフレームのスクリーンの空いている箇所に何も表示がされないようにしておく
    work->text_dummy_bmpwin = GFL_BMPWIN_Create(
        BG_FRAME_M_TEXT,
        0, 0, 1, 1,
        BG_PAL_POS_M_TEXT_FONT,
        GFL_BMP_CHRAREA_GET_F );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_dummy_bmpwin), 0 );
    GFL_BMPWIN_TransVramCharacter(work->text_dummy_bmpwin);

    // ウィンドウ内
    work->text_winin_bmpwin = GFL_BMPWIN_Create(
        BG_FRAME_M_TEXT,
        1, 19, 30, 4,
        BG_PAL_POS_M_TEXT_FONT,
        GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), TEXT_WININ_ZERO_COLOR );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->text_winin_bmpwin );
    
    // ウィンドウ枠
    work->text_winfrm_tinfo = BmpWinFrame_GraphicSetAreaMan(
        BG_FRAME_M_TEXT,
        BG_PAL_POS_M_TEXT_FRAME,
        MENU_TYPE_SYSTEM,
        work->heap_id );

    // メッセージ
    work->text_msgdata_shinka     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_shinka_demo_dat, work->heap_id );
    work->text_msgdata_wazaoboe   = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_waza_oboe_dat, work->heap_id );

    // TCBL、フォントカラー、転送など
    work->text_tcblsys = GFL_TCBL_Init( work->heap_id, work->heap_id, 1, 0 );
    GFL_FONTSYS_SetColor( 1, 2, TEXT_WININ_BACK_COLOR );

    // NULL初期化
    work->text_strbuf       = NULL;
    work->text_stream       = NULL;
  }

  // メッセージ送りキーカーソルアイコン
  work->text_keycursor_wk = APP_KEYCURSOR_Create( TEXT_WININ_BACK_COLOR, TRUE, TRUE, work->heap_id );

  // VBlank中にテキストを表示するBG面のONとOFFを切り替える
  work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_NONE;
  GFL_BG_SetVisible( BG_FRAME_M_TEXT                   , VISIBLE_OFF );
  work->text_bg_frame_show_state = FALSE;
}

//-------------------------------------
/// テキスト終了処理
//=====================================
static void ShinkaDemo_TextExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  // メッセージ送りキーカーソルアイコン
  APP_KEYCURSOR_Delete( work->text_keycursor_wk );

  // TEXT
  {
    if( work->text_stream ) PRINTSYS_PrintStreamDelete( work->text_stream );
    GFL_TCBL_Exit( work->text_tcblsys );
    if( work->text_strbuf ) GFL_STR_DeleteBuffer( work->text_strbuf );
    GFL_MSG_Delete( work->text_msgdata_shinka );
    GFL_MSG_Delete( work->text_msgdata_wazaoboe );
    GFL_BG_FreeCharacterArea(
        BG_FRAME_M_TEXT,
        GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
        GFL_ARCUTIL_TRANSINFO_GetSize(work->text_winfrm_tinfo) );
    GFL_BMPWIN_Delete( work->text_winin_bmpwin );
    GFL_BMPWIN_Delete( work->text_dummy_bmpwin );
  }
}

//-------------------------------------
/// テキスト主処理
//=====================================
static void ShinkaDemo_TextMain( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  GFL_TCBL_Main( work->text_tcblsys );
  
  if( work->text_stream ) 
  {
    APP_KEYCURSOR_Main( work->text_keycursor_wk, work->text_stream, work->text_winin_bmpwin );
  }
}

//-------------------------------------
/// 「つかいかたを　きれいにわすれた！」メッセージのコールバック関数
//=====================================
static BOOL ShinkaDemo_TextStreamForgetCallBack( u32 arg )
{
  switch( arg )
  {
  case 3:  // "ポカン"
    {
      PMSND_PlaySE( SEQ_SE_KON );
    }
    break;
  case 5:  // "ポカン"のSE終了待ち
    {
      return PMSND_CheckPlaySE();
    }
    break;
  }

  return FALSE;
}

//-------------------------------------
/// TEXTのストリーム作成
//=====================================
static void ShinkaDemo_MakeTextStream( SHINKA_DEMO_WORK* work,
                                       GFL_MSGDATA* msgdata, u32 str_id, pPrintCallBack callback,
                                       TAG_REGIST_TYPE type0, const void* data0,
                                       TAG_REGIST_TYPE type1, const void* data1 )
{
  // 消去
  ShinkaDemo_TextWininClear( work, TEXT_WININ_BACK_COLOR );

  // 文字列作成
  work->text_strbuf = MakeStr( work->heap_id,
                               msgdata, str_id,
                               type0, data0,
                               type1, data1 );

  // ストリーム開始
  work->text_stream = PRINTSYS_PrintStreamCallBack(
                              work->text_winin_bmpwin,
                              0, 0,
                              work->text_strbuf,
                              work->font, MSGSPEED_GetWait(),
                              work->text_tcblsys, 2,
                              work->heap_id,
                              TEXT_WININ_BACK_COLOR,
                              callback );
}

//-------------------------------------
/// TEXTのストリーム待ち
//=====================================
static BOOL ShinkaDemo_WaitTextStream( SHINKA_DEMO_WORK* work )
{
  BOOL ret = FALSE;

  switch( PRINTSYS_PrintStreamGetState( work->text_stream ) )
  { 
  case PRINTSTREAM_STATE_RUNNING:
    if( ( GFL_UI_KEY_GetCont() & ( PAD_BUTTON_A | PAD_BUTTON_B ) ) || GFL_UI_TP_GetCont() )
    {
      PRINTSYS_PrintStreamShortWait( work->text_stream, 0 );

      if( GFL_UI_KEY_GetCont() & ( PAD_BUTTON_A | PAD_BUTTON_B ) )
      {
        // タッチorキー
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      }
      else
      {
        // タッチorキー
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
      }
    }
    break;
  case PRINTSTREAM_STATE_PAUSE:
    if( ( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) ) || GFL_UI_TP_GetTrg() )
    {
      PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
      PRINTSYS_PrintStreamReleasePause( work->text_stream );

      if( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) )
      {
        // タッチorキー
        GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      }
      else
      {
        // タッチorキー
        GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
      }
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
/// テキスト後の入力待ち
//=====================================
static BOOL ShinkaDemo_TextWaitInput( SHINKA_DEMO_WORK* work )
{
  if( ( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) ) || GFL_UI_TP_GetTrg() )
  { 
    if( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) )
    {
      // タッチorキー
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
    }
    else
    {
      // タッチorキー
      GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
    }

    return TRUE;
  }

  return FALSE;
}

//-------------------------------------
/// テキストの中身を消去
//=====================================
static void ShinkaDemo_TextWininClear( SHINKA_DEMO_WORK* work, u8 col_code )
{
  // 一旦消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), col_code );
	//GFL_BMPWIN_TransVramCharacter(work->text_winin_bmpwin);      // GFL_BMPWIN_TransVramCharacterでは2回目ウィンドウ内が透明のままだったので、
  GFL_BMPWIN_MakeTransWindow_VBlank( work->text_winin_bmpwin );  // GFL_BMPWIN_MakeTransWindow_VBlankにした。

  // 前のを消す
  if( work->text_stream ) PRINTSYS_PrintStreamDelete( work->text_stream );  // この関数でタスクも削除してくれるので、同時に動くタスクは1つで済む
  if( work->text_strbuf ) GFL_STR_DeleteBuffer( work->text_strbuf );

  work->text_stream = NULL;
  work->text_strbuf = NULL;
}

//-------------------------------------
/// テキストの枠を描画/消去
//=====================================
static void ShinkaDemo_TextWinfrmShow( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work, BOOL is_on )
{
/*
  if( is_on )
  {
    BmpWinFrame_Write(
        work->text_winin_bmpwin,
        WINDOW_TRANS_ON_V,
        GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
        BG_PAL_POS_M_TEXT_FRAME );
  }
  else
  {
    BmpWinFrame_Clear( work->text_winin_bmpwin, WINDOW_TRANS_ON_V );
  }
*/

  if( is_on )
  {
    BmpWinFrame_Write(
        work->text_winin_bmpwin,
        WINDOW_TRANS_ON,
        GFL_ARCUTIL_TRANSINFO_GetPos(work->text_winfrm_tinfo),
        BG_PAL_POS_M_TEXT_FRAME );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_winin_bmpwin), TEXT_WININ_BACK_COLOR );
    GFL_BMPWIN_MakeTransWindow( work->text_winin_bmpwin );
  }
  else
  {
    work->text_show_vblank_req = TEXT_SHOW_VBLANK_REQ_OFF;
  }
}

//-------------------------------------
/// YESNO_MENUの初期化処理
//=====================================
static void ShinkaDemo_YesNoMenuInit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
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
}

//-------------------------------------
/// YESNO_MENUの終了処理
//=====================================
static void ShinkaDemo_YesNoMenuExit( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  // YESNO_MENU
  {
    YESNO_MENU_DeleteRes( work->yesno_menu_work );
    work->yesno_menu_work = NULL;
    if( work->yesno_menu_strbuf_yes ) GFL_STR_DeleteBuffer( work->yesno_menu_strbuf_yes );
    if( work->yesno_menu_strbuf_no ) GFL_STR_DeleteBuffer( work->yesno_menu_strbuf_no );
    GFL_MSG_Delete( work->yesno_menu_msgdata );
  }
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

//-------------------------------------
/// 進化条件を処理する
//=====================================
static void ShinkaDemo_ShinkaCondCheckSpecialLevelup( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  // 「ツチニン」からではなく「技を覚える前のテッカニン」から誕生させる
  // パーティに空きがあって、普通のモンスターボールを持っている場合、ヌケニンが誕生する
  
  // この関数を呼び出す時点でのwork->ppは、既に「技を覚える前のテッカニン」になっていること
  
  {
    MYITEM_PTR myitem_ptr = GAMEDATA_GetMyItem( param->gamedata );

    if(    ( PokeParty_GetPokeCount(param->ppt) < PokeParty_GetPokeCountMax(param->ppt) )  // パーティに空きがある
        && ( MYITEM_CheckItem( myitem_ptr, ITEM_MONSUTAABOORU, 1, work->heap_sys_id ) ) )      // 普通のモンスターボールを持っている
    {
      BOOL ret;
      POKEMON_PARAM* nukenin_pp;

      // ヌケニン誕生
      nukenin_pp = GFL_HEAP_AllocClearMemory( work->heap_sys_id, POKETOOL_GetWorkSize() );
      POKETOOL_CopyPPtoPP( (POKEMON_PARAM*)(work->pp), nukenin_pp );
      PP_ChangeMonsNo( nukenin_pp, MONSNO_NUKENIN );  // 進化


      //-------------// ↓パラメータを設定する↓ //-------------//
      
      //取得ボールをモンスターボールに
      PP_Put( nukenin_pp, ID_PARA_get_ball, ITEM_MONSUTAABOORU );
      
      //装備アイテムをなくす
      PP_Put( nukenin_pp, ID_PARA_item, 0 );
      
      //ポケモンにつけるマークを消す
      PP_Put( nukenin_pp, ID_PARA_mark, 0 );
      
      //リボン系をクリア
      {
        int i;
        for( i=ID_PARA_sinou_champ_ribbon; i<ID_PARA_sinou_amari_ribbon+1; i++ )
        {
          PP_Put( nukenin_pp, i, 0 );
        }
        for( i=ID_PARA_stylemedal_normal; i<ID_PARA_world_ribbon+1; i++ )
        {
          PP_Put( nukenin_pp, i, 0 );
        }
        for( i=ID_PARA_trial_stylemedal_normal; i<ID_PARA_amari_ribbon+1; i++ )
        {
          PP_Put( nukenin_pp, i, 0 );
        }
      }
      
      //ニックネームをデフォルト名に
      {
        u32 nukenin_monsno = PP_Get( nukenin_pp, ID_PARA_monsno, NULL );
        STRBUF* strbuf = GFL_MSG_CreateString( GlobalMsg_PokeName, nukenin_monsno );
        PP_Put( nukenin_pp, ID_PARA_nickname, (u32)strbuf );
        GFL_STR_DeleteBuffer( strbuf );
      }
      
      //状態異常を直す
      PP_Put( nukenin_pp, ID_PARA_condition, 0 );

      //メールデータ
      {
        MAIL_DATA* mail_data = MailData_CreateWork( work->heap_sys_id );
        PP_Put( nukenin_pp, ID_PARA_mail_data, (u32)mail_data );
        GFL_HEAP_FreeMemory( mail_data );
      }

      //カスタムボールID
      PP_Put( nukenin_pp, ID_PARA_cb_id, 0 );

/*
カスタムボールなしっぽい
      //カスタムボールデータ
      {
        CB_CORE cb_core;
        MI_CpuClearFast( &cb_core, sizeof(CB_CORE) );
        PP_Put( nukenin_pp, ID_PARA_cb_core, cb_core );
      }
 */

      //-------------// ↑パラメータを設定する↑ //-------------//


      // パーティに入れる
      PokeParty_Add( (POKEPARTY*)(param->ppt), nukenin_pp );  // constはずし

      // 普通のモンスターボールを減らす
      ret = MYITEM_SubItem( myitem_ptr, ITEM_MONSUTAABOORU, 1, work->heap_sys_id );
      GF_ASSERT_MSG( ret, "SHINKADEMO : モンスターボールを減らせませんでした。\n" );

      // 後始末
      GFL_HEAP_FreeMemory( nukenin_pp );

      // 図鑑登録
      {
        ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( param->gamedata );
        ZUKANSAVE_SetPokeSee( zukan_savedata, nukenin_pp );  // 見た  // 図鑑フラグをセットする
        ZUKANSAVE_SetPokeGet( zukan_savedata, nukenin_pp );  // 捕まえた  // 図鑑フラグをセットする
      }

      {
        // ヌケニンはすれ違い用データをセットしなくていい
        //// すれ違い用データをセット
        //{ 
        //  GAMEBEACON_Set_PokemonEvolution( PP_Get( nukenin_pp, ID_PARA_monsno, NULL ), work->poke_nick_name_strbuf );
        //  // (古い)ニックネームは(新しい)種族名に進化した
        //}

        // 進化させたポケモンの数
        // 1日にポケモン進化させた回数
        {
          RECORD* rec = GAMEDATA_GetRecordPtr(param->gamedata); 
          RECORD_Inc(rec, RECID_POKE_EVOLUTION);    // 進化させたポケモンの数
          RECORD_Inc(rec, RECID_DAYCNT_EVOLUTION);  // 1日にポケモン進化させた回数
        }
      }
    }
  }
}

static void ShinkaDemo_ShinkaCondCheckSoubiUse( SHINKA_DEMO_PARAM* param, SHINKA_DEMO_WORK* work )
{
  //装備アイテムを消す系
  PP_Put( (POKEMON_PARAM*)(work->pp), ID_PARA_item, 0 );
}

