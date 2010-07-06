//============================================================================================
/**
 * @file  wo_main.c
 * @brief 技教え/思い出し処理　メイン
 * @author  Hiroyuki Nakamura / Akito Mori(WBに引越し）
 * @date  109.10.13
 */
//============================================================================================
#include <gflib.h>


#include "system/gfl_use.h"   //GFUser_VIntr_CreateTCB
#include "system/main.h"      //GFL_HEAPID_APP参照
#include "net/network_define.h"
#include "arc_def.h"

#include "item/item.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/msgspeed.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"

#include "message.naix"
#include "bag.naix"
#include "msg/msg_waza_oboe.h"
#include "font/font.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/gfl_use.h"

#include "system/wipe.h"
#include "system/bmp_menu.h"

#include "system/cursor_move.h"
#include "sound/pm_sndsys.h"

#include "savedata/config.h"
#include "app/app_keycursor.h"      // APP_KEY_CURSOR
#include "app/app_menu_common.h"
#include "waza_tool/wazadata.h"
#include "app/p_status.h"
#include "app/app_taskmenu.h"
#include "system/poke2dgra.h"

#include "app/waza_oshie.h"
#include "wo_bmp_def.h"
#include "wo_snd_def.h"
#include "waza_oshie_gra.naix"


FS_EXTERN_OVERLAY( poke_status );

//============================================================================================
//  定数定義
//============================================================================================

#define MFRM_POKE     (GFL_BG_FRAME0_M)
#define MFRM_PARAM    (GFL_BG_FRAME2_M)
#define MFRM_BACK     (GFL_BG_FRAME3_M)

#define SFRM_MSG      (GFL_BG_FRAME0_S)
#define SFRM_TOUCHBAR (GFL_BG_FRAME1_S)
#define SFRM_PARAM    (GFL_BG_FRAME2_S)
#define SFRM_BACK     (GFL_BG_FRAME3_S)

#define MFRM_POKE_PRI (1)
#define MFRM_PARAM_PRI  (2)
#define MFRM_BACK_PRI (3)
#define SFRM_MSG_PRI  (0)
#define SFRM_PARAM_PRI  (2)
#define SFRM_BACK_PRI (3)


enum {
  WIN_SPACE_M=0,
  WIN_SPACE_S,
  WIN_STR_ATTACK, // 「いりょく」
  WIN_STR_HIT,    // 「めいちゅう」
  WIN_PRM_ATTACK, // 威力値
  WIN_PRM_HIT,    // 命中値
  WIN_BTL_INFO,   // 戦闘説明
  WIN_MSG,        // メッセージウィンドウ
  WIN_EXP,        // 説明ウィンドウ
  WIN_ABTN,       // 決定ボタンウィンドウ
  WIN_BACK,       // 戻るボタンウィンドウ
  WIN_LIST,       // 技リスト
  WIN_YESNO,      // はい/いいえ

  WIN_TITLE,      // タイトル
  WIN_MWAZA,      // 上 技リスト
  WIN_MPRM,       // 上 ポケモン名とレベル

  WIN_MAX
};

enum {
  WO_CLA_ARROW_D = 0,// リストカーソル下
  WO_CLA_ARROW_U,    // リストカーソル上
  WO_CLA_CURSOR,     // 選択カーソル
  WO_CLA_KIND,       // 分類アイコン
  WO_CLA_TYPE1,      // タイプアイコン１
  WO_CLA_TYPE2,      // タイプアイコン２
  WO_CLA_TYPE3,      // タイプアイコン３
  WO_CLA_TYPE4,      // タイプアイコン４

  WO_CLA_TYPE5,      // タイプアイコン１
  WO_CLA_TYPE6,      // タイプアイコン２
  WO_CLA_TYPE7,      // タイプアイコン３
  WO_CLA_TYPE8,      // タイプアイコン４

  WO_CLA_POKEGRA,    // ポケモン正面絵
  WO_CLA_EXIT,       // 戻るボタン

  WO_CLA_MAX
};

enum{
  WO_ANIME_OBJ=0,   ///< OBJアニメを待つ
  WO_ANIME_APPTASK, ///< APPTASKUMENUのアニメを待つ
};


// ボタンアニメ
typedef struct {
  u8  px;
  u8  py;
  u8  sx;
  u8  sy;
  u16 cnt;
  u16 index;
  u16 next;
  u16 mode;
}BUTTON_ANM_WORK;

// 技教え画面ワーク
typedef struct {
  WAZAOSHIE_DATA * dat;     // 外部設定データ

  GFL_BMPWIN  *win[WIN_MAX];  // BMPウィンドウデータ

  GFL_MSGDATA * mman;   // メッセージデータマネージャ

  WORDSET * wset;       // 単語セット
  STRBUF * mbuf;        // 汎用文字列展開領域
  STRBUF * pp1_str;     // PP用文字列展開領域(PP/MPP)
  STRBUF * pp2_str;     // PP用文字列展開領域(PP/MPP)

  BMP_MENULIST_DATA * ld;   // BMPリストデータ

  BMPMENU_WORK * mw;      // BMPメニューワーク

  GFL_FONT     *fontHandle;  // フォントハンドル
  GFL_TCBLSYS  *pMsgTcblSys;  // メッセージ表示用タスクSYS
  GFL_TCB      *vBlankTcb;
  PRINT_STREAM *printStream;

  u32 clres[4][WO_CHR_ID_MAX];  // セルアクターリソースインデックス用ワーク

  // セルアクター
  GFL_CLUNIT    *clUnit;
//  CATS_RES_PTR  crp;
  GFL_CLWK    *cap[WO_CLA_MAX];
  ARCHANDLE   *TypeIconHandle;
  int         WazaIconReq[WAZATYPE_ICON_NUM][2];  // 転送用の情報 0:タイプ 1:転送するかどうか

  PSTATUS_DATA  psd;
  GFL_PROC * subProc;

  int seq;          // 現在のシーケンス
  int next_seq;     // 次のシーケンス
  int key_mode;

  u8  sel_max;      // 思い出しできる技の総数

  u8  midx;         // メッセージインデックス
  u8  ynidx;        // はい・いいえID

  u8  cpos_x;       // カーソルポジション
  u8  cpos_y;       // カーソルポジション
  u8  cur_pos;      // カーソルポジション

  void* pSBufDParts;
  NNSG2dScreenData* pSDParts;

  CURSORMOVE_WORK * cmwk; // カーソル移動ワーク
  u16 cm_pos;         // カーソル移動の現在の位置
  u16 enter_flg;        //「おぼえる」表示フラグ

  BUTTON_ANM_WORK button_anm_work;

  // APPMENU関連
  APP_TASKMENU_RES   *app_res;
  APP_TASKMENU_WORK  *app_menuwork;
  APP_TASKMENU_ITEMWORK menuitem[2];
  APP_TASKMENU_ITEMWORK yn_menuitem[2];
  PRINT_UTIL            *printUtil;
  PRINT_QUE             *printQue;
  APP_KEYCURSOR_WORK *printCursor;
  APP_TASKMENU_WIN_WORK *oboe_menu_work[2];
}WO_WORK;


typedef int (*WO_SEQ_FUNC)(WO_WORK*);

// はい・いいえ処理
typedef struct {
  WO_SEQ_FUNC yes;  // はい
  WO_SEQ_FUNC no;   // いいえ
}YESNO_FUNC;

// メインシーケンス
enum {
  SEQ_FADE_WAIT = 0,  // フェード終了待ち
  SEQ_SELECT,     // 技セレクト
  SEQ_MSG_WAIT,   // メッセージ終了待ち
  SEQ_YESNO_PUT,    // はい・いいえセット
  SEQ_YESNO_WAIT,   // はい・いいえ選択待ち
  SEQ_WAZA_SET,   // 技覚え
  SEQ_WAZADEL_SET,  // 忘れて技覚え
  SEQ_DEL_CHECK,    // 忘れさせる技をチェック
  SEQ_FADEOUT_SET,  // フェードアウトセット
  SEQ_END,      // 終了

  SEQ_PST_INIT,   // ステータス画面へ
  SEQ_PST_CALL,   // ステータス画面呼び出し
  SEQ_PST_WAIT,   // ステータス画面終了待ち

  SEQ_BUTTON_ANM,   // ボタンアニメ
  SEQ_RET_BUTTON,   //「もどる」
  SEQ_ENTER_BUTTON, //「おぼえる」
};

//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void WO_DispInit( WO_WORK * wk );
static void WO_DispExit( WO_WORK * wk );

static void WO_VBlank( GFL_TCB *tcb, void * work );
static void WO_VramBankSet(void);
static void WO_BgSet( void);
static void WO_BgExit( void );
static void WO_BgGraphicSet( WO_WORK * wk, ARCHANDLE* p_handle );
static void WO_BgGraphicRelease( WO_WORK * wk );
static void WO_BmpWinSet( WO_WORK * wk );
static void WO_BmpWinExit( WO_WORK * wk );
static void WO_MsgCreate( WO_WORK * wk );
static void WO_MsgRelease( WO_WORK * wk );
static int WO_SeqFadeWait( WO_WORK * wk );

static int WO_SeqSelectDecide(WO_WORK* wk);
static void WO_InputModeChange( WO_WORK * wk );
static int WO_SeqSelectInputCheck( WO_WORK * wk );

static int WO_SeqSelect( WO_WORK * wk );
static int WO_SeqFadeOutSet( WO_WORK * wk );
static int WO_SeqMsgWait( WO_WORK * wk );
static int WO_SeqYesNoPut( WO_WORK * wk );
static int WO_SeqYesNoWait( WO_WORK * wk );
static int WO_SeqWazaSet( WO_WORK * wk );
static int WO_SeqWazaDelSet( WO_WORK * wk );
static int WO_SeqDelCheck( WO_WORK * wk );
static int WO_SeqPstInit( WO_WORK * wk );
static int WO_SeqPstCall( WO_WORK * wk );
static int WO_SeqPstWait( WO_WORK * wk );

static void StrPut( WO_WORK * wk, u32 widx, GFL_FONT *font, PRINTSYS_LSB col, u32 mode, u8 py );
static void WO_PageChange( WO_WORK * wk );
static void WO_DefStrWrite( WO_WORK * wk );
static void WO_WazaListDraw( WO_WORK * wk );
static void WO_WazaListMake( WO_WORK * wk );
static void WO_WazaListExit( WO_WORK * wk );

static void BattleWazaParamPut( WO_WORK * wk, u32 prm );

static void WO_MsgMake( WO_WORK * wk, u32 id );
static void WO_TalkMsgSet( WO_WORK * wk, u32 id );
static void WO_TalkMsgOff( WO_WORK * wk);
static BOOL WO_TalkMsgCallBack( u32 value );
static u16 WO_SelWazaGet( WO_WORK * wk );
static u16 WO_DelWazaGet( WO_WORK * wk );
static u8 WO_WazaSetCheck( WO_WORK * wk );

static int WO_SetCheck_Yes( WO_WORK * wk );
static int WO_SetCheck_No( WO_WORK * wk );
static int WO_EndCheck_Yes( WO_WORK * wk );
static int WO_EndCheck_No( WO_WORK * wk );
static int WO_ChgCheck_Yes( WO_WORK * wk );
static int WO_ChgCheck_No( WO_WORK * wk );
static int WO_DelCheck_Yes( WO_WORK * wk );
static int WO_DelCheck_No( WO_WORK * wk );
static int WO_DelEnter_Yes( WO_WORK * wk );
static int WO_DelEnter_No( WO_WORK * wk );

static void WO_ObjInit( WO_WORK * wk, ARCHANDLE* p_handle );
static void WO_ObjFree( WO_WORK * wk );
static void WO_TypeIconChangeRequest( WO_WORK * wk, u16 waza, u16 res_offset );
static void WO_TepeIconChangeVblank( WO_WORK *wk );
static void WO_TypeIconInit( WO_WORK * wk );
static void WO_KindIconChange( WO_WORK * wk, u16 waza );
static void WO_SelCursorChange( WO_WORK * wk, u8 pos, u8 pal );
static void WO_ScrollCursorPut( WO_WORK * wk ,u8 idx,BOOL anm_f);
//static void WO_ScrollCursorOff( WO_WORK * wk );


static void CursorMoveInit( WO_WORK * wk );
static void CursorMoveExit( WO_WORK * wk );

static void ScrollButtonOnOff( WO_WORK * wk );
static void EnterButtonOnOff( WO_WORK * wk, BOOL flg );

static int  RetButtonAnmInit( WO_WORK * wk, int next );
static int  EnterButtonAnmInit( WO_WORK * wk, int next );
static int  YameruButtonAnmInit( WO_WORK *wk, int next );
static int  ButtonAnmMaun( WO_WORK * wk );
static int  WO_SeqRetButton( WO_WORK * wk );
static int  WazaSelectEnter( WO_WORK * wk );
static void PassiveSet( BOOL flg );
static void wazaoshie_3d_setup( void );
static void WO_AppTaskMenuMain( WO_WORK *wk );
static int  EnterButtonCheck( WO_WORK *wk );
static void ScrollButtonAnmChange( WO_WORK * wk, s32 mv );
static void ScrollButtonInit( WO_WORK *wk );



typedef struct{
  int frm, x, y, w, h, pal, offset;
}BMPWIN_DAT;

const GFL_PROC_DATA WazaOshieProcData = {
  WazaOshieProc_Init,
  WazaOshieProc_Main,
  WazaOshieProc_End,
};


//============================================================================================
//  グローバル変数
//============================================================================================
// BMPウィンドウデータ
static const BMPWIN_DAT BmpWinData[] =
{

  { // メイン面用１キャラ押さえ
    MFRM_PARAM, 0, 25,
    1, 1, 0, 0
  },
  { // サブ面用１キャラ押さえWIN_SPACE_M
    SFRM_PARAM, 0,25,
    1, 1, 0, 0
  },

  { // 「いりょく」
    SFRM_PARAM, WIN_STR_ATTACK_PX, WIN_STR_ATTACK_PY,
    WIN_STR_ATTACK_SX, WIN_STR_ATTACK_SY, WIN_STR_ATTACK_PAL, WIN_STR_ATTACK_CGX
  },
  { // 「めいちゅう」
    SFRM_PARAM, WIN_STR_HIT_PX, WIN_STR_HIT_PY,
    WIN_STR_HIT_SX, WIN_STR_HIT_SY, WIN_STR_HIT_PAL, WIN_STR_HIT_CGX
  },
  { // 威力値
    SFRM_PARAM, WIN_PRM_ATTACK_PX, WIN_PRM_ATTACK_PY,
    WIN_PRM_ATTACK_SX, WIN_PRM_ATTACK_SY, WIN_PRM_ATTACK_PAL, WIN_PRM_ATTACK_CGX
  },
  { // 命中値
    SFRM_PARAM, WIN_PRM_HIT_PX, WIN_PRM_HIT_PY,
    WIN_PRM_HIT_SX, WIN_PRM_HIT_SY, WIN_PRM_HIT_PAL, WIN_PRM_HIT_CGX
  },
  { // 戦闘説明
    SFRM_PARAM, WIN_BTL_INFO_PX, WIN_BTL_INFO_PY,
    WIN_BTL_INFO_SX, WIN_BTL_INFO_SY, WIN_BTL_INFO_PAL, WIN_BTL_INFO_CGX
  },
  { // メッセージウィンドウ
    SFRM_MSG, WIN_MSG_PX, WIN_MSG_PY,
    WIN_MSG_SX, WIN_MSG_SY, WIN_MSG_PAL, WIN_MSG_CGX,
  },
  { // 操作説明ウィンドウ
    SFRM_PARAM, WIN_EXP_PX, WIN_EXP_PY,
    WIN_EXP_SX, WIN_EXP_SY, WIN_EXP_PAL, WIN_EXP_CGX,
  },
  { // Aボタン
    SFRM_PARAM, WIN_ABTN_PX, WIN_ABTN_PY,
    WIN_ABTN_SX, WIN_ABTN_SY, WIN_ABTN_PAL, WIN_ABTN_CGX
  },
  { // 戻るボタンウィンドウ
    SFRM_PARAM, WIN_BACK_PX, WIN_BACK_PY,
    WIN_BACK_SX, WIN_BACK_SY, WIN_BACK_PAL, WIN_BACK_CGX,
  },
  { // 技リスト
    SFRM_PARAM, WIN_LIST_PX, WIN_LIST_PY,
    WIN_LIST_SX, WIN_LIST_SY, WIN_LIST_PAL, WIN_LIST_CGX
  },
  { // はい/いいえ
    SFRM_MSG, WIN_YESNO_PX, WIN_YESNO_PY,
    WIN_YESNO_SX, WIN_YESNO_SY, WIN_YESNO_PAL, WIN_YESNO_CGX
  },

  { // タイトル
    MFRM_PARAM, WIN_TITLE_PX, WIN_TITLE_PY,
    WIN_TITLE_SX, WIN_TITLE_SY, WIN_TITLE_PAL, WIN_TITLE_CGX
  },
  { // 技リスト
    MFRM_PARAM, WIN_MWAZA_PX, WIN_MWAZA_PY,
    WIN_MWAZA_SX, WIN_MWAZA_SY, WIN_MWAZA_PAL, WIN_MWAZA_CGX
  },
  { // パラメータ
    MFRM_PARAM, WIN_MPRM_PX, WIN_MPRM_PY,
    WIN_MPRM_SX, WIN_MPRM_SY, WIN_MPRM_PAL, WIN_MPRM_CGX
  },
};

// メッセージテーブル
static const u32 MsgDataTbl[2][11] =
{
  { // 技教え
    msg_waza_oboe_01, // @0に　どのわざを　おぼえさせる？
    msg_waza_oboe_02, // @1　を  おぼえさせますか？
    msg_waza_oboe_03, // @0に　わざを  おぼえさせるのを　あきらめますか？
    msg_waza_oboe_04, // @0は　あたらしく　@1を　おぼえた！▼"
    msg_waza_oboe_05, // @0は　あたらしく　@1を　おぼえたい……▼　しかし　～
    msg_waza_oboe_06, // １　２の……　ポカン！▼　@0は　@1の つかいかたを　～
    msg_waza_oboe_07, // @0は　あたらしく @1を　おぼえた！▼
    msg_waza_oboe_08, // それでは……　@1を　おぼえるのを　あきらめますか？"
    msg_waza_oboe_09, // @0は　@1を　おぼえずに　おわった！▼
    msg_waza_oboe_10, // @2は　おれいに　ハートのウロコを　１まい　あげた！▼
    msg_waza_oboe_11, // @0を　わすれさせて　よろしいですね？"
  },
  { // 技思い出し
    msg_waza_omoi_01, // @0に どのわざを　おもいださせる？
    msg_waza_omoi_02, // @1　を おもいだしますか？
    msg_waza_omoi_03, // @0に　わざを おもいださせるのを　あきらめますか？
    msg_waza_omoi_04, // @0は　わすれていた　@1を　おもいだした！▼
    msg_waza_omoi_05, // @0は　@1を　おもいだしたい……▼　しかし　～
    msg_waza_omoi_06, // １　２の……　ポカン！▼　@0は　@1の つかいかたを　～
    msg_waza_omoi_07, // @0は　わすれていた　@1を　おもいだした！▼
    msg_waza_omoi_08, // それでは……　@1を　おもいだすのを　あきらめますか？
    msg_waza_omoi_09, // @0は　@1を　おもいださずに　おわった！▼
    msg_waza_omoi_10, // @2は　おれいに ハートのウロコを　１まい　あげた！▼
    msg_waza_omoi_11, // @0を　わすれさせて　よろしいですね？"
  }
};

// はい・いいえ処理テーブル
static const YESNO_FUNC YesNoFunc[] =
{
  { WO_SetCheck_Yes, WO_SetCheck_No },  // @1　を  おぼえさせますか？
  { WO_EndCheck_Yes, WO_EndCheck_No },  // @0に　わざを  おぼえさせるのを　あきらめますか？
  { WO_ChgCheck_Yes, WO_ChgCheck_No },  // @0は　あたらしく　@1を　おぼえたい……▼　しかし　～
  { WO_DelCheck_Yes, WO_DelCheck_No },  // それでは……　@1を　おぼえるのを　あきらめますか？
  { WO_DelEnter_Yes, WO_DelEnter_No },  // @0を　わすれさせて　よろしいですね？"
};


#define CLACT_U_RES_MAX   ( 4 )

typedef struct{
  s16 x;              ///< [ X ] 座標
  s16 y;              ///< [ Y ] 座標
  s16 z;              ///< [ Z ] 座標

  u16 anm;            ///< アニメ番号
  int pri;            ///< 優先順位
  int pal;            ///< パレット番号 ※この値を TCATS_ADD_S_PAL_AUTO にすることで、
                      ///< NCEデータのカラーNo指定を受け継ぐ
  int d_area;           ///< 描画エリア

  int id[ CLACT_U_RES_MAX ];    ///< 使用リソースIDテーブル

  int bg_pri;           ///< BG面への優先度
  int vram_trans;         ///< Vram転送フラグ
} CLACT_ENTRY_DATA;


// セルアクターデータ
static const CLACT_ENTRY_DATA ClactParamTbl[] =
{
  { // リストカーソル下
    LIST_CUR_D_PX, LIST_CUR_D_PY, 0,
    APP_COMMON_BARICON_CURSOR_DOWN, 1, 2, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_APP_COMMON, WO_PAL_ID_APP_COMMON, WO_CEL_ID_APP_COMMON, WO_CEL_ID_APP_COMMON, },
    2, 0
  },
  { // リストカーソル上
    LIST_CUR_U_PX, LIST_CUR_U_PY, 0,
    APP_COMMON_BARICON_CURSOR_UP, 1, 2, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_APP_COMMON, WO_PAL_ID_APP_COMMON, WO_CEL_ID_APP_COMMON, WO_CEL_ID_APP_COMMON, },
    2, 0
  },
  { // 選択カーソル
    SEL_CURSOR_PX, SEL_CURSOR_PY, 0,
    ANMDW_CURSOR, 0, 3, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_CURSOR, WO_PAL_ID_OBJ, WO_CEL_ID_CURSOR, WO_CEL_ID_CURSOR, },
    2, 0
  },
  { // 分類アイコン(下画面）
    KIND_ICON_PX, KIND_ICON_PY, 0,
    0, 0, TICON_ACTPAL_IDX, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_KIND, WO_PAL_ID_TYPE, WO_CEL_ID_KIND, WO_CEL_ID_KIND, },
    2, 0
  },
  { // タイプアイコン１(下画面）
    TYPE_ICON1_PX, TYPE_ICON1_PY, 0,
    0, 1, TICON_ACTPAL_IDX, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_TYPE, WO_PAL_ID_TYPE, WO_CEL_ID_D_TYPE, WO_CEL_ID_D_TYPE,  },
    2, 0
  },
  { // タイプアイコン２(下画面）
    TYPE_ICON2_PX, TYPE_ICON2_PY, 0,
    0, 1, TICON_ACTPAL_IDX, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_TYPE, WO_PAL_ID_TYPE, WO_CEL_ID_D_TYPE, WO_CEL_ID_D_TYPE,  },
    2, 0
  },
  { // タイプアイコン３(下画面）
    TYPE_ICON3_PX, TYPE_ICON3_PY, 0,
    0, 1, TICON_ACTPAL_IDX, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_TYPE, WO_PAL_ID_TYPE, WO_CEL_ID_D_TYPE, WO_CEL_ID_D_TYPE,  },
    2, 0
  },
  { // タイプアイコン４(下画面）
    TYPE_ICON4_PX, TYPE_ICON4_PY, 0,
    0, 1, TICON_ACTPAL_IDX, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_TYPE, WO_PAL_ID_TYPE, WO_CEL_ID_D_TYPE, WO_CEL_ID_D_TYPE,  },
    2, 0
  },

  { // タイプアイコン５(上画面）
    TYPE_ICON5_PX, TYPE_ICON5_PY, 0,
    0, 1, TICON_ACTPAL_IDX_M, CLSYS_DEFREND_MAIN,
    { WO_CHR_ID_U_TYPE1, WO_PAL_ID_TYPE_M, WO_CEL_ID_U_TYPE, WO_CEL_ID_U_TYPE, },
    2, 0
  },
  { // タイプアイコン６(上画面）
    TYPE_ICON6_PX, TYPE_ICON6_PY, 0,
    0, 1, TICON_ACTPAL_IDX_M, CLSYS_DEFREND_MAIN,
    { WO_CHR_ID_U_TYPE2, WO_PAL_ID_TYPE_M, WO_CEL_ID_U_TYPE, WO_CEL_ID_U_TYPE, },
    2, 0
  },
  { // タイプアイコン７(上画面）
    TYPE_ICON7_PX, TYPE_ICON7_PY, 0,
    0, 1, TICON_ACTPAL_IDX_M, CLSYS_DEFREND_MAIN,
    { WO_CHR_ID_U_TYPE3, WO_PAL_ID_TYPE_M, WO_CEL_ID_U_TYPE, WO_CEL_ID_U_TYPE, },
    2, 0
  },
  { // タイプアイコン８(上画面）
    TYPE_ICON8_PX, TYPE_ICON8_PY, 0,
    0, 1, TICON_ACTPAL_IDX_M, CLSYS_DEFREND_MAIN,
    { WO_CHR_ID_U_TYPE4, WO_PAL_ID_TYPE_M, WO_CEL_ID_U_TYPE, WO_CEL_ID_U_TYPE, },
    2, 0
  },

  { // ポケモン正面絵(上画面）
    POKE_PX, POKE_PY, 0,
    0, 0, 0, CLSYS_DEFREND_MAIN,
    { WO_CHR_ID_POKEGRA, WO_PAL_ID_POKEGRA, WO_CEL_ID_POKEGRA, WO_CEL_ID_POKEGRA, },
    2, 0
  },
  { // 戻るボタン
    224, 168, 0,
    APP_COMMON_BARICON_RETURN, 0, 0, CLSYS_DEFREND_SUB,
    { WO_CHR_ID_APP_COMMON, WO_PAL_ID_APP_COMMON, WO_CEL_ID_APP_COMMON, WO_CEL_ID_APP_COMMON, },
    2, 0
  },
};


//============================================================================================
//  プロセス関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：初期化
 *
 * @param proc  プロセスデータ
 * @param pwk   パラメータワーク
 * @param mywk  アプリケーションワーク
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT WazaOshieProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  WO_WORK * wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WAZAOSHIE, 0x50000 );   // ヒープ作成

  wk = GFL_PROC_AllocWork( proc, sizeof(WO_WORK), HEAPID_WAZAOSHIE ); // ワーク確保＆クリア
  GFL_STD_MemFill( wk, 0, sizeof(WO_WORK) );

  wk->dat = pwk;  // PROC作成時パラメータ引渡し

  wk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                                    GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_WAZAOSHIE );

  wk->pMsgTcblSys = GFL_TCBL_Init( HEAPID_WAZAOSHIE , HEAPID_WAZAOSHIE , 32 , 32 );

  WO_DispInit( wk );

  OS_Printf("first pos = %d\n", wk->dat->pos);

  WO_SelCursorChange( wk, wk->dat->pos, PALDW_CURSOR );
  wk->next_seq = SEQ_SELECT;

  GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_WAZAOSHIE );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：メイン
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT WazaOshieProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  WO_WORK * wk  = mywk;

  switch( *seq ){
  case SEQ_FADE_WAIT:   // フェード終了待ち
    *seq = WO_SeqFadeWait( wk );
    break;

  case SEQ_SELECT:    // 技セレクト
    *seq = WO_SeqSelect( wk );
    break;

  case SEQ_MSG_WAIT:    // メッセージ終了待ち
    *seq = WO_SeqMsgWait( wk );
    break;

  case SEQ_YESNO_PUT:   // はい・いいえセット
    *seq = WO_SeqYesNoPut( wk );
    break;

  case SEQ_YESNO_WAIT:  // はい・いいえ選択待ち
    *seq = WO_SeqYesNoWait( wk );
    break;

  case SEQ_WAZA_SET:    // 技覚え
    *seq = WO_SeqWazaSet( wk );
    break;

  case SEQ_WAZADEL_SET: // 忘れて技覚え
    *seq = WO_SeqWazaDelSet( wk );
    break;

  case SEQ_DEL_CHECK:   // 忘れさせる技をチェック
    *seq = WO_SeqDelCheck( wk );
    break;

  case SEQ_FADEOUT_SET: // フェードアウトセット
    *seq = WO_SeqFadeOutSet( wk );
    break;

  case SEQ_END:     // 終了
    return GFL_PROC_RES_FINISH;

  case SEQ_PST_INIT:    // ステータス画面へ
    *seq = WO_SeqPstInit( wk );
    break;

  case SEQ_PST_CALL:    // ステータス画面呼び出し
    *seq = WO_SeqPstCall( wk );
    return GFL_PROC_RES_CONTINUE;

  case SEQ_PST_WAIT:    // ステータス画面終了待ち
    *seq = WO_SeqPstWait( wk );
    return GFL_PROC_RES_CONTINUE;

  case SEQ_BUTTON_ANM:
    *seq = ButtonAnmMaun( wk );
    break;

  case SEQ_RET_BUTTON:
    *seq = WO_SeqRetButton( wk );
    break;

  case SEQ_ENTER_BUTTON:
    *seq = WazaSelectEnter( wk );
    break;
  }
  
  WO_AppTaskMenuMain(wk);
  GFL_CLACT_SYS_Main( );
  GFL_TCBL_Main( wk->pMsgTcblSys );

  PRINTSYS_QUE_Main( wk->printQue );

  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：終了
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
GFL_PROC_RESULT WazaOshieProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  WO_WORK * wk  = mywk;

  WO_DispExit( wk );

  GFL_TCBL_Exit( wk->pMsgTcblSys );

  GFL_FONT_Delete( wk->fontHandle );


  GFL_PROC_FreeWork( proc );          // ワーク開放

  GFL_HEAP_DeleteHeap( HEAPID_WAZAOSHIE );

  return GFL_PROC_RES_FINISH;
}


static const int menu_item[][2]={
  { msg_exp_decide, APP_TASKMENU_WIN_TYPE_NORMAL },
  { msg_exp_back,   APP_TASKMENU_WIN_TYPE_RETURN },
};

static const int yn_item[][2]={
  { msg_wazaoshie_yes,  APP_TASKMENU_WIN_TYPE_NORMAL },
  { msg_wazaoshie_no,   APP_TASKMENU_WIN_TYPE_NORMAL },
};

//----------------------------------------------------------------------------------
/**
 * @brief タスクメニュー用初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void InitTaskMenu( WO_WORK *wk )
{
  int i;
  wk->printQue = PRINTSYS_QUE_Create( HEAPID_WAZAOSHIE );
  
  // APPMENUリソース読み込み
  wk->app_res = APP_TASKMENU_RES_Create( SFRM_MSG, 10, wk->fontHandle, wk->printQue, HEAPID_WAZAOSHIE );

  // タッチバーメニュー項目
  for(i=0;i<2;i++){
    wk->menuitem[i].str      = GFL_MSG_CreateString( wk->mman, menu_item[i][0] ); //メニューに表示する文字列
    wk->menuitem[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    wk->menuitem[i].type     = menu_item[i][1];
  }
  // はい・いいえ
  for(i=0;i<2;i++){
    wk->yn_menuitem[i].str      = GFL_MSG_CreateString( wk->mman, yn_item[i][0] ); //メニューに表示する文字列
    wk->yn_menuitem[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;   //文字色。デフォルトでよいならばAPP_TASKMENU_ITEM_MSGCOLOR
    wk->yn_menuitem[i].type     = yn_item[i][1];
  }
  wk->oboe_menu_work[0] = NULL;
  wk->oboe_menu_work[1] = NULL;

}

//----------------------------------------------------------------------------------
/**
 * @brief タスクメニュー終了
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ExitTaskMenu( WO_WORK *wk )
{
  // メニュー解放忘れの場合用に
  EnterButtonOnOff( wk, FALSE );
  
  // 文字列破棄
  GFL_STR_DeleteBuffer( wk->menuitem[0].str);
  GFL_STR_DeleteBuffer( wk->menuitem[1].str);
  GFL_STR_DeleteBuffer( wk->yn_menuitem[0].str);
  GFL_STR_DeleteBuffer( wk->yn_menuitem[1].str);


  // APPMENUリソース解放
  APP_TASKMENU_RES_Delete( wk->app_res );

  PRINTSYS_QUE_Delete( wk->printQue );
}

//----------------------------------------------------------------------------------
/**
 * @brief タスクメニューメイン関数
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void WO_AppTaskMenuMain( WO_WORK *wk )
{
  // 「おぼえる」「もどる」メニューの表示メイン処理
  if(wk->oboe_menu_work[0]!=NULL){
    APP_TASKMENU_WIN_Update( wk->oboe_menu_work[0] );
    APP_TASKMENU_WIN_Update( wk->oboe_menu_work[1] );
  }

}

//--------------------------------------------------------------------------------------------
/**
 * 初期設定
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_DispInit( WO_WORK * wk )
{
  ARCHANDLE* p_handle;

  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );


  p_handle = GFL_ARC_OpenDataHandle( ARCID_WAZAOSHIE_GRA, HEAPID_WAZAOSHIE );

  WO_VramBankSet();       // VRAM設定
  WO_BgSet( );      // BG設定
  WO_BgGraphicSet( wk, p_handle );      // BGグラフィックセット

  WO_ObjInit( wk, p_handle );

  WO_MsgCreate( wk );
  WO_BmpWinSet( wk );
  WO_WazaListMake( wk );
  WO_PageChange( wk );

  WO_DefStrWrite( wk );

  CursorMoveInit( wk );
  ScrollButtonInit(wk);

  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN,   WIPE_TYPE_FADEIN,
                  WIPE_FADE_BLACK,  WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WAZAOSHIE );

  // 上下画面表示設定
  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);

  // VblankFuncセット
  wk->vBlankTcb = GFUser_VIntr_CreateTCB( WO_VBlank , wk , 16 );

  GFL_ARC_CloseDataHandle( p_handle );
  
  InitTaskMenu(wk); // タスクメニュー初期化
}

//--------------------------------------------------------------------------------------------
/**
 * 解放処理
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_DispExit( WO_WORK * wk )
{

  ExitTaskMenu(wk);
  
  CursorMoveExit( wk );

  WO_WazaListExit( wk );
  WO_BmpWinExit( wk );
  WO_BgGraphicRelease( wk );
  WO_BgExit(  );    // BGL削除

  WO_MsgRelease( wk );

  WO_ObjFree( wk );

  // VBlankFuncクリア
  GFL_TCB_DeleteTask( wk->vBlankTcb );
}


//--------------------------------------------------------------------------------------------
/**
 * VBlank関数
 *
 * @param work  ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_VBlank( GFL_TCB *tcb, void * work )
{
  WO_WORK * wk = work;

  WO_TepeIconChangeVblank( wk );

  GFL_BG_VBlankFunc( );

  // セルアクター
  GFL_CLACT_SYS_VBlankFunc();

  OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

static const GFL_DISP_VRAM waza_oshie_vram = {
    GX_VRAM_BG_128_B,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット

    GX_VRAM_OBJ_64_E,       // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

    GX_VRAM_SUB_OBJ_128_D,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット

    GX_VRAM_TEX_0_A,        // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_0_F,      // テクスチャパレットスロット

    GX_OBJVRAMMODE_CHAR_1D_64K,
    GX_OBJVRAMMODE_CHAR_1D_64K

};


//--------------------------------------------------------------------------------------------
/**
 * VRAM設定
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_VramBankSet(void)
{

  GFL_DISP_SetBank( &waza_oshie_vram );
}

//--------------------------------------------------------------------------------------------
/**
 * BG設定
 *
 * @param ini   BGLデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_BgSet( void )
{

  GFL_BG_Init( HEAPID_WAZAOSHIE );
  // BMPWINシステム開始
  GFL_BMPWIN_Init( HEAPID_WAZAOSHIE );

  // メイン
  { // BG SYSTEM
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &BGsys_data );
  }

  { // PARAM FONT (BMP)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, MFRM_PARAM_PRI, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( MFRM_PARAM, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  MFRM_PARAM );
  }
  { // PLATE (CHAR)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, MFRM_BACK_PRI, 0, 0, FALSE
    };
    GFL_BG_SetBGControl(  MFRM_BACK, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(   MFRM_BACK );
  }


  // サブ
  { // WINDOW (BMP)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, SFRM_MSG_PRI, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( SFRM_MSG, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  SFRM_MSG );
  }

  { // PARAM FONT (BMP)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x256,
      GX_BG_EXTPLTT_01, SFRM_PARAM_PRI, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( SFRM_PARAM, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  SFRM_PARAM );
  }

  { // TOUCHBAR_BG (CHAR)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xc000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
      GX_BG_EXTPLTT_01, SFRM_PARAM_PRI, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( SFRM_TOUCHBAR, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  SFRM_TOUCHBAR );
  }

  { // PLATE (CHAR)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_128x128,
      GX_BG_EXTPLTT_01,   SFRM_BACK_PRI, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( SFRM_BACK, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(  SFRM_BACK );
  }

  // BG面表示ON
  GFL_BG_SetVisible( MFRM_PARAM,    VISIBLE_ON );
  GFL_BG_SetVisible( MFRM_BACK,     VISIBLE_ON );
  GFL_BG_SetVisible( SFRM_MSG,      VISIBLE_ON );
  GFL_BG_SetVisible( SFRM_TOUCHBAR, VISIBLE_ON );
  GFL_BG_SetVisible( SFRM_PARAM,    VISIBLE_ON );
  GFL_BG_SetVisible( SFRM_BACK,     VISIBLE_ON );

  GFL_BG_SetClearCharacter( MFRM_PARAM,    32, 0, HEAPID_WAZAOSHIE );
  GFL_BG_SetClearCharacter( SFRM_MSG,      32, 0, HEAPID_WAZAOSHIE );
  GFL_BG_SetClearCharacter( SFRM_TOUCHBAR, 32, 0, HEAPID_WAZAOSHIE );

  GFL_BG_SetClearCharacter( MFRM_PARAM, 32, 0, HEAPID_WAZAOSHIE );
  GFL_BG_SetClearCharacter( SFRM_MSG,   32, 0, HEAPID_WAZAOSHIE );

  // 半透明OFF
  G2_BlendNone();
  G2S_BlendNone();
}

//--------------------------------------------------------------------------------------------
/**
 * BG解放
 *
 * @param ini   BGLデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_BgExit( void )
{
  GFL_DISP_GXS_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG2 |
    GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ, VISIBLE_OFF );

  GFL_BG_FreeBGControl( SFRM_TOUCHBAR );
  GFL_BG_FreeBGControl( SFRM_BACK );
  GFL_BG_FreeBGControl( SFRM_PARAM );
  GFL_BG_FreeBGControl( SFRM_MSG );
  GFL_BG_FreeBGControl( MFRM_BACK );
  GFL_BG_FreeBGControl( MFRM_PARAM );

  //上画面をメイン,下画面をサブに戻す
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

  // BG処理解放
  GFL_BG_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * グラフィックデータセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_BgGraphicSet( WO_WORK * wk, ARCHANDLE* p_handle )
{
  // 上画面背景
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_waza_oshie_gra_bgd_NCGR,
                                        MFRM_BACK, 0,0,0, HEAPID_WAZAOSHIE );
  GFL_ARCHDL_UTIL_TransVramScreen( p_handle,NARC_waza_oshie_gra_bgu_back_NSCR,
                                        MFRM_BACK, 0, 0x800,0,HEAPID_WAZAOSHIE);
  GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_waza_oshie_gra_bgd_NCLR,
                                        PALTYPE_MAIN_BG, 0,0, HEAPID_WAZAOSHIE);

  // 下画面背景
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_waza_oshie_gra_bgd_NCGR,
                                        SFRM_BACK, 0,0,0, HEAPID_WAZAOSHIE);
  GFL_ARCHDL_UTIL_TransVramScreen( p_handle,NARC_waza_oshie_gra_bgd_back_NSCR,
                                        SFRM_BACK, 0,0x800,0, HEAPID_WAZAOSHIE);
  GFL_ARCHDL_UTIL_TransVramPalette( p_handle,NARC_waza_oshie_gra_bgd_NCLR,
                                        PALTYPE_SUB_BG,0,0, HEAPID_WAZAOSHIE);

  //スクリーンリソース取得
  wk->pSBufDParts = GFL_ARCHDL_UTIL_LoadScreen(
            p_handle,
            NARC_waza_oshie_gra_bgd_parts_NSCR,
            FALSE,
            &wk->pSDParts,
            HEAPID_WAZAOSHIE );

  // フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG ,
                                  WO_PAL_TALK_FONT * 32, 16*2, HEAPID_WAZAOSHIE );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG ,
                                  WO_PAL_SYS_FONT*32, 16*2,HEAPID_WAZAOSHIE );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG ,
                                  WO_PAL_SYS_FONT*32, 16*2,HEAPID_WAZAOSHIE );

  // 会話ウィンドウセット
  BmpWinFrame_GraphicSet( SFRM_MSG, WO_TALK_WIN_CGX,WO_PAL_TALK_WIN,
                          MENU_TYPE_SYSTEM, HEAPID_WAZAOSHIE );

}

//----------------------------------------------------------------------------------
/**
 * @brief 背景画面バッファ解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void WO_BgGraphicRelease( WO_WORK * wk )
{
  GFL_HEAP_FreeMemory(wk->pSBufDParts);
}

//--------------------------------------------------------------------------------------------
/**
 * BMPウィンドウセット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_BmpWinSet( WO_WORK * wk )
{
  u32 i;
  const BMPWIN_DAT *bmp;
  GFL_BMP_DATA *bmpdat;

  for( i=0; i<WIN_MAX; i++ ){
    bmp = &BmpWinData[i];
    OS_Printf("frm=%d, x=%d, y=%d,w=%d, h=%d, pal=%d\n", bmp->frm, bmp->x, bmp->y, bmp->w, bmp->h, bmp->pal);
    wk->win[i] = GFL_BMPWIN_Create( bmp->frm, bmp->x, bmp->y, bmp->w, bmp->h, bmp->pal, GFL_BMP_CHRAREA_GET_B );
    bmpdat = GFL_BMPWIN_GetBmp( wk->win[i] );
    GFL_BMP_Clear( bmpdat, 0 );
  }

  GFL_BMPWIN_ClearTransWindow_VBlank( wk->win[WIN_ABTN] );

  wk->printCursor = APP_KEYCURSOR_Create( 15, TRUE, FALSE, HEAPID_WAZAOSHIE );
  

}

//--------------------------------------------------------------------------------------------
/**
 * BMPウィンドウ削除
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_BmpWinExit( WO_WORK * wk )
{
  u32 i;

  APP_KEYCURSOR_Delete(wk->printCursor);

  for( i=0; i<WIN_MAX; i++ ){
    GFL_BMPWIN_Delete( wk->win[i] );
  }
//  MsgPrintAutoFlagSet( MSG_TP_RECT_OFF );

  // BMPWINシステム終了
  GFL_BMPWIN_Exit( );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連作成
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_MsgCreate( WO_WORK * wk )
{
  wk->mman = GFL_MSG_Create(
          GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_waza_oboe_dat, HEAPID_WAZAOSHIE );

  wk->wset = WORDSET_Create( HEAPID_WAZAOSHIE );

  wk->mbuf = GFL_STR_CreateBuffer( TMP_MSGBUF_SIZ, HEAPID_WAZAOSHIE );
  wk->pp1_str = GFL_MSG_CreateString( wk->mman, msg_waza_oboe_01_04);
  wk->pp2_str = GFL_MSG_CreateString( wk->mman, msg_waza_oboe_01_10);
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連削除
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_MsgRelease( WO_WORK * wk )
{
  GFL_MSG_Delete( wk->mman );
  WORDSET_Delete( wk->wset );
  GFL_STR_DeleteBuffer( wk->mbuf );
  GFL_STR_DeleteBuffer( wk->pp1_str );
  GFL_STR_DeleteBuffer( wk->pp2_str );
}



//--------------------------------------------------------------------------------------------
/**
 * シーケンス：フェードイン
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqFadeWait( WO_WORK * wk )
{
  if( WIPE_SYS_EndCheck() == TRUE ){
    return wk->next_seq;
  }
  return SEQ_FADE_WAIT;
}


//--------------------------------------------------------------------------------------------
/**
 * シーケンス：入力　決定位置操作実行
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqSelectDecide(WO_WORK* wk)
{
  //戻る
  if(wk->cur_pos == CPOS_BACK){
    PMSND_PlaySE( WO_SE_CANCEL );
    WO_SelCursorChange( wk, wk->dat->pos, PALDW_CURSOR);
    WO_TalkMsgSet( wk, MSG_END_CHECK );
    wk->ynidx = YESNO_END_CHECK;
    wk->next_seq = SEQ_YESNO_PUT;
    return SEQ_MSG_WAIT;
  }
  //ページスクロール
  if(wk->cur_pos == CPOS_PAGE_DW){
    if(wk->dat->scr + LIST_NUM >= wk->sel_max){
      return SEQ_SELECT;
    }
    wk->dat->scr += LIST_NUM;
    WO_WazaListDraw( wk );
    return SEQ_SELECT;
  }
  if(wk->cur_pos == CPOS_PAGE_UP){
    if(wk->dat->scr == 0){
      return SEQ_SELECT;
    }
    wk->dat->scr -= LIST_NUM;
    WO_WazaListDraw( wk );
    return SEQ_SELECT;
  }
  //覚えたい技決定
  if(wk->dat->scr + wk->dat->pos >= wk->sel_max){
    return SEQ_SELECT;
  }
  PMSND_PlaySE( WO_SE_DECIDE );
  WO_SelCursorChange( wk, wk->dat->pos, PALDW_CURSOR);
  if( WO_WazaSetCheck( wk ) < 4 ){
    WO_TalkMsgSet( wk, MSG_SET_CHECK );
    wk->ynidx = YESNO_SET_CHECK;
    wk->next_seq = SEQ_YESNO_PUT;
  }else{
    WO_TalkMsgSet( wk, MSG_CHG_CHECK );
    wk->ynidx = YESNO_CHG_CHECK;
    wk->next_seq = SEQ_YESNO_PUT;
  }
  return SEQ_MSG_WAIT;
}


//--------------------------------------------------------------------------------------------
/**
 * シーケンス：技選択　キーorタッチ表示状態セット
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static void WO_InputModeChange( WO_WORK * wk )
{
  if(wk->key_mode == GFL_APP_KTST_TOUCH){
    GFL_CLACT_WK_SetDrawEnable( wk->cap[WO_CLA_CURSOR], FALSE);
  }else{
    GFL_CLACT_WK_SetDrawEnable( wk->cap[WO_CLA_CURSOR], TRUE);
    WO_SelCursorChange( wk, wk->cur_pos, PALDW_CURSOR);
  }
  WO_WazaListDraw(wk);
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：技選択　キーorタッチ切替
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqSelectInputCheck( WO_WORK * wk )
{
  if(wk->key_mode == GFL_APP_KTST_TOUCH){
    if(GFL_UI_TP_GetCont() != 0){ //
      return FALSE;
    }
    if(GFL_UI_KEY_GetCont() != 0){
      wk->key_mode = GFL_APP_KTST_KEY;
      WO_InputModeChange(wk);
      return TRUE;
    }
  }else{
    if(GFL_UI_KEY_GetCont() != 0){
      return FALSE;
    }
    if(GFL_UI_TP_GetCont() != 0){
      wk->key_mode = GFL_APP_KTST_TOUCH;
      WO_InputModeChange(wk);
      return FALSE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：技選択
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqSelect( WO_WORK * wk )
{
  u32 ret;

  wk->cm_pos = CURSORMOVE_PosGet( wk->cmwk );   // 前回の位置保存
  ret        = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case 0:   // 技１
  case 1:   // 技２
  case 2:   // 技３
  case 3:   // 技４
    if( GFL_UI_TP_GetTrg() == FALSE ){
      // キーで決定なら直接「はい・いいえ」へ
      if( wk->dat->scr + ret < wk->sel_max ){
        PMSND_PlaySE( WO_SE_DECIDE );
        return WazaSelectEnter( wk );
      }
    }else{
      // タッチで決定の場合はカーソル移動のみ（ボタン表示はcallback_touchで）
      if( wk->dat->scr + ret < wk->sel_max ){
        PMSND_PlaySE( WO_SE_LIST_MOVE );
      }
    }
    break;

  case 4:   // 下矢印
    break;

  case 5:   // 上矢印
    break;

  case 6:                   // もどる
    PMSND_PlaySE( WO_SE_CANCEL );
    if(EnterButtonCheck(wk)){
      return YameruButtonAnmInit( wk, SEQ_RET_BUTTON );
    }else{
      return RetButtonAnmInit( wk, SEQ_RET_BUTTON );
    }
    break;
  case CURSORMOVE_CANCEL:   // キャンセル
    PMSND_PlaySE( WO_SE_CANCEL );
    EnterButtonOnOff( wk, FALSE );  // 決定を消す
    return RetButtonAnmInit( wk, SEQ_RET_BUTTON );
  case 7:   // おぼえる
    if(EnterButtonCheck( wk )){
      PMSND_PlaySE( WO_SE_DECIDE );
      return EnterButtonAnmInit( wk, SEQ_ENTER_BUTTON );
    }
  case CURSORMOVE_CURSOR_MOVE:  // 移動
    break;

  case CURSORMOVE_NO_MOVE_UP:
    if(wk->dat->scr!=0){
      PMSND_PlaySE( WO_SE_PAGE_MOVE );
      wk->dat->scr--;
      BattleWazaParamPut( wk, WO_SelWazaGet( wk ) );
      WO_WazaListDraw( wk );
      ScrollButtonOnOff( wk );
      ScrollButtonAnmChange( wk, -1 );
    }
    break;

  case CURSORMOVE_CURSOR_ON:    // カーソル表示
  case CURSORMOVE_NONE:     // 動作なし
    break;
  }

  return SEQ_SELECT;

}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：メッセージ終了待ち
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqMsgWait( WO_WORK * wk )
{ 
  int status = PRINTSYS_PrintStreamGetState( wk->printStream );
  APP_KEYCURSOR_Main( wk->printCursor, wk->printStream, wk->win[WIN_MSG] );

  if(status == PRINTSTREAM_STATE_RUNNING){
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_DECIDE){
      PRINTSYS_PrintStreamShortWait( wk->printStream, 0 );
    }
  }else if( status == PRINTSTREAM_STATE_DONE ){
    PRINTSYS_PrintStreamDelete( wk->printStream );
    return wk->next_seq;
  }else if(status==PRINTSTREAM_STATE_PAUSE){
    if(GFL_UI_KEY_GetTrg()&(PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) || GFL_UI_TP_GetTrg()){
       PRINTSYS_PrintStreamReleasePause( wk->printStream );
       PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
    }
  }
  return SEQ_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：はい・いいえ表示
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqYesNoPut( WO_WORK * wk )
{
  APP_TASKMENU_INITWORK init;

  init.heapId   = HEAPID_WAZAOSHIE;
  init.itemNum  = 2;
  init.itemWork = wk->yn_menuitem;
  init.posType  = ATPT_LEFT_UP;
  init.charPosX = 21; //ウィンドウ開始位置(キャラ単位
  init.charPosY =  8;
  init.w = 10;  //キャラ単位
  init.h =  3;  //キャラ単位

  // はい・いいえメニュー開始
  wk->app_menuwork = APP_TASKMENU_OpenMenu( &init, wk->app_res );

  PassiveSet( TRUE );
  return SEQ_YESNO_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：はい・いいえ処理
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqYesNoWait( WO_WORK * wk )
{

  u32 ret=SEQ_YESNO_WAIT;
  if(APP_TASKMENU_IsFinish( wk->app_menuwork )){
    if(APP_TASKMENU_GetCursorPos(wk->app_menuwork)==0){
      ret = YesNoFunc[wk->ynidx].yes( wk );
    }else{
      ret = YesNoFunc[wk->ynidx].no( wk );
    }
    PassiveSet( FALSE );
  }
  APP_TASKMENU_UpdateMenu( wk->app_menuwork );
  if(ret!=SEQ_YESNO_WAIT){
    APP_TASKMENU_CloseMenu( wk->app_menuwork );
  }

  return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：技セット
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqWazaSet( WO_WORK * wk )
{
  // 技セット
  PP_Put( wk->dat->pp, ID_PARA_waza1+wk->dat->del_pos, WO_SelWazaGet( wk ) );
  // セットした技のPP＋分を0に初期化
  PP_Put( wk->dat->pp, ID_PARA_pp_count1+wk->dat->del_pos, 0 );
  // セットした技のPPMAXをセット
  PP_Put( wk->dat->pp, ID_PARA_pp1+wk->dat->del_pos, WAZADATA_GetMaxPP( WO_SelWazaGet(wk), 0 ) );

  wk->dat->ret = WAZAOSHIE_RET_SET;

  return SEQ_FADEOUT_SET;
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：技セット完了メッセージセット
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqWazaDelSet( WO_WORK * wk )
{
  WO_TalkMsgSet( wk, MSG_CHG_WAZA );
  wk->next_seq = SEQ_WAZA_SET;
  return SEQ_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：技選択チェック
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqDelCheck( WO_WORK * wk )
{
  if( wk->dat->del_pos < 4 ){
    WO_TalkMsgSet( wk, MSG_DEL_ENTER );
    wk->ynidx = YESNO_DEL_ENTER;
  }else{
    WO_TalkMsgSet( wk, MSG_DEL_CHECK );
    wk->ynidx = YESNO_DEL_CHECK;
  }
  wk->next_seq = SEQ_YESNO_PUT;

  return SEQ_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * シーケンス：フェードアウトセット
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqFadeOutSet( WO_WORK * wk )
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT,  WIPE_TYPE_FADEOUT,
                  WIPE_FADE_BLACK,  WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WAZAOSHIE );
  wk->next_seq = SEQ_END;
  return SEQ_FADE_WAIT;
}


//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_PageChange( WO_WORK * wk )
{
  u16 waza;

  waza = WO_SelWazaGet( wk );

  if( waza != WAZAOSHIE_TBL_MAX ){
    BattleWazaParamPut( wk, waza );
  }else{
    BattleWazaParamPut( wk, BMPMENULIST_CANCEL );
  }
  WO_TypeIconInit( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * 文字表示
 *
 * @param wk    ワーク
 * @param widx  ウィンドウインデックス
 * @param font    表示フォント
 * @param col   表示カラー
 * @param mode  表示モード
 * @param py    y方向オフセット
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void StrPut( WO_WORK * wk, u32 widx, GFL_FONT *font, PRINTSYS_LSB  col, u32 mode, u8 py )
{
  u8  siz;
  u8  wsx;
  u8  px;

  switch( mode ){
  case STR_MODE_LEFT:   // 左詰め
    px = 0;
    break;
  case STR_MODE_RIGHT:  // 右詰め
    siz = PRINTSYS_GetStrWidth( wk->mbuf, font, 0 );
    wsx = GFL_BMPWIN_GetSizeX( wk->win[widx] ) * 8;
    px  = wsx - siz;
    break;
  case STR_MODE_CENTER: // 中央
    siz = PRINTSYS_GetStrWidth( wk->mbuf, font, 0 );
    wsx = GFL_BMPWIN_GetSizeX( wk->win[widx] ) * 8;
    px  = (wsx-siz)/2;
    break;
  }

  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->win[widx]), px, py, wk->mbuf, font, col );
//  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[widx] );


}

//--------------------------------------------------------------------------------------------
/**
 * 数字入り文字列展開
 *
 * @param wk    ワーク
 * @param msg_id  メッセージID
 * @param num   数字
 * @param keta  桁
 * @param type  表示タイプ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void NumPrmSet( WO_WORK * wk, u32 msg_id, u32 num, u8 keta, u8 type )
{
  STRBUF * str;

  str = GFL_MSG_CreateString( wk->mman, msg_id );
  WORDSET_RegisterNumber( wk->wset, 0, num, keta, type, STR_NUM_CODE_ZENKAKU );
  WORDSET_ExpandStr( wk->wset, wk->mbuf, str );
  GFL_STR_DeleteBuffer( str );
}


//--------------------------------------------------------------------------------------------
/**
 * 固定文字列描画
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_DefStrWrite( WO_WORK * wk )
{
  STRBUF  *str;
  GFL_MSGDATA * waza_man;
  const POKEMON_PASO_PARAM* ppp;
  u32 siz,i;

  //　もどる
  GFL_MSG_GetString( wk->mman, msg_exp_decide, wk->mbuf );
  StrPut( wk, WIN_ABTN, wk->fontHandle, WOFCOL_N_WHITE, STR_MODE_CENTER ,4);

  // 「いりょく」
  GFL_MSG_GetString( wk->mman, msg_waza_oboe_01_02, wk->mbuf );
  StrPut( wk, WIN_STR_ATTACK, wk->fontHandle, WOFCOL_N_BLACK, STR_MODE_LEFT ,0);
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_STR_ATTACK] );

  // 「めいちゅう」
  GFL_MSG_GetString( wk->mman, msg_waza_oboe_01_03, wk->mbuf );
  StrPut( wk, WIN_STR_HIT, wk->fontHandle, WOFCOL_N_BLACK, STR_MODE_LEFT ,0);
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_STR_HIT] );

  ///////////////////////////////////////////////////////////////////////////
  //上画面
  str = GFL_STR_CreateBuffer( TMP_MSGBUF_SIZ, HEAPID_WAZAOSHIE );

  // タイトル
  GFL_MSG_GetString( wk->mman, msg_param_exp, str );
  WORDSET_RegisterPokeNickName( wk->wset, 0, wk->dat->pp );
  WORDSET_ExpandStr( wk->wset, wk->mbuf, str );
  StrPut( wk, WIN_TITLE, wk->fontHandle, WOFCOL_N_WHITE, STR_MODE_LEFT ,4);
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_TITLE] );

  waza_man = GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wazaname_dat, HEAPID_WAZAOSHIE );
  ppp = PP_GetPPPPointerConst(wk->dat->pp);

  //技リスト
  for(i = 0;i < 4;i++){
    u16 waza = PPP_Get(ppp,ID_PARA_waza1+i,NULL);
    if(waza == 0){
      GFL_CLACT_WK_SetDrawEnable( wk->cap[WO_CLA_TYPE5+i], 0 );
      continue;
    }else{
      GFL_CLACT_WK_SetDrawEnable( wk->cap[WO_CLA_TYPE5+i], 1 );
      
      // 技タイプアイコン書き換え
      WO_TypeIconChangeRequest( wk, waza, i );
    }
    //技名
    GFL_MSG_GetString( waza_man, waza , wk->mbuf );
    StrPut( wk, WIN_MWAZA, wk->fontHandle, WOFCOL_N_WHITE, STR_MODE_LEFT ,32*i);

    //PP
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->win[WIN_MWAZA]), U_WLIST_PP1_X, 32*i+16, wk->pp1_str,
                         wk->fontHandle, WOFCOL_N_BLACK );
    GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_MWAZA] );


    //PP/MPP
    WORDSET_RegisterNumber( wk->wset, 0,
      PPP_Get(ppp,ID_PARA_pp1+i,NULL),2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterNumber( wk->wset, 1,
      PPP_Get(ppp,ID_PARA_pp_max1+i,NULL),2, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );
    WORDSET_ExpandStr( wk->wset, wk->mbuf, wk->pp2_str );
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->win[WIN_MWAZA]), U_WLIST_PP2_X, 32*i+16,
                         wk->mbuf, wk->fontHandle, WOFCOL_N_BLACK );
    GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_MWAZA] );

  }
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_MWAZA] );

  //ポケモン名とレベル
  GFL_MSG_GetString( wk->mman, msg_param_name, str );
  WORDSET_RegisterPokeMonsName(wk->wset,0,wk->dat->pp );
  WORDSET_ExpandStr( wk->wset, wk->mbuf, str );
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->win[WIN_MPRM]), F_WLIST_NAME_OX, 0,wk->mbuf,
                       wk->fontHandle,  WOFCOL_N_BLACK);
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_MPRM] );

  GFL_MSG_GetString( wk->mman, msg_param_level, str );
  WORDSET_RegisterNumber( wk->wset, 0,
      PPP_Get(ppp,ID_PARA_level,NULL),3, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );

  WORDSET_ExpandStr( wk->wset, wk->mbuf, str );
  StrPut( wk, WIN_MPRM, wk->fontHandle, WOFCOL_N_BLACK, STR_MODE_RIGHT ,16);

  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_MPRM] );

  GFL_MSG_Delete( waza_man );

  GFL_STR_DeleteBuffer(str);
}


//--------------------------------------------------------------------------------------------
/**
 * 技数取得
 *
 * @param wk    ワーク
 *
 * @return  技数
 */
//--------------------------------------------------------------------------------------------
static u32 WO_WazaTableNumGet( WO_WORK * wk )
{
  u32 i;

  for( i=0; i<256; i++ ){
    if( wk->dat->waza_tbl[i] == WAZAOSHIE_TBL_MAX ){
      break;
    }
  }
  return i;
}


//--------------------------------------------------------------------------------------------
/**
 * 技リスト ライン描画
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_WazaListLineDraw( WO_WORK * wk ,u8 scr,u8 pos)
{
  u32 tmp,py;

  py = pos*LIST_SY;
  tmp = 0;

  if(scr+pos >= wk->sel_max){
    return;
  }

  //技名描画
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->win[WIN_LIST]),0, py,wk->ld[scr+pos].str,
                      wk->fontHandle, WOFCOL_N_WHITE );
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_LIST] );

  //PP
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->win[WIN_LIST]), F_WLIST_PP1_OX, py,wk->pp1_str, wk->fontHandle, WOFCOL_N_BLACK);
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_LIST] );

  // PP/MPP
  tmp = WAZADATA_GetMaxPP( wk->ld[scr+pos].param, 0 );
  WORDSET_RegisterNumber( wk->wset, 0, tmp,2, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );

  WORDSET_RegisterNumber( wk->wset, 1, tmp,2, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );

  WORDSET_ExpandStr( wk->wset, wk->mbuf, wk->pp2_str );
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(wk->win[WIN_LIST]), F_WLIST_PP2_OX, py, wk->mbuf,
                       wk->fontHandle, WOFCOL_N_BLACK );
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_LIST] );
}

//--------------------------------------------------------------------------------------------
/**
 * 技リスト ページ描画
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_WazaListDraw( WO_WORK * wk )
{
  u32 i;

  //領域クリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[WIN_LIST]),0);

  for(i = 0;i < LIST_NUM;i++){
    WO_WazaListLineDraw(wk ,wk->dat->scr,i);
  }
  //ウィンドウプット
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_LIST]);
  GFL_BG_LoadScreenV_Req( SFRM_BACK );

  WO_TypeIconInit( wk );
}


//--------------------------------------------------------------------------------------------
/**
 * 技リスト作成
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_WazaListMake( WO_WORK * wk )
{
  GFL_MSGDATA * mman;
  u32 i;

  wk->sel_max = (u8)WO_WazaTableNumGet( wk );

  wk->ld = BmpMenuWork_ListCreate( wk->sel_max, HEAPID_WAZAOSHIE );
  i = wk->ld[0].param;

  mman = GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wazaname_dat, HEAPID_WAZAOSHIE );

  // BMPlistに技名登録
  for( i=0; i<wk->sel_max; i++ ){
    if( wk->dat->waza_tbl[i] != WAZAOSHIE_TBL_MAX ){
      BmpMenuWork_ListAddArchiveString( wk->ld, mman, wk->dat->waza_tbl[i],
                                        wk->dat->waza_tbl[i], HEAPID_WAZAOSHIE );

    }else{
      BmpMenuWork_ListAddArchiveString(
        wk->ld, wk->mman, msg_waza_oboe_01_11, BMPMENULIST_CANCEL, HEAPID_WAZAOSHIE );
      break;
    }
  }

  GFL_MSG_Delete( mman );
  wk->cpos_x = 0;
  wk->cpos_y = wk->dat->pos;
  WO_WazaListDraw( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * 技リスト削除
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_WazaListExit( WO_WORK * wk )
{
    BmpMenuWork_ListDelete( wk->ld );
}



//--------------------------------------------------------------------------------------------
/**
 * 戦闘技パラメータ表示
 *
 * @param wk    ワーク
 * @param prm   BMPリストデータ（主に技番号）
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BattleWazaParamPut( WO_WORK * wk, u32 prm )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[WIN_BTL_INFO]), 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[WIN_PRM_ATTACK]), 0 );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[WIN_PRM_HIT]), 0 );

  GFL_CLACT_WK_SetDrawEnable( wk->cap[WO_CLA_KIND], 0 );

  if( prm != BMPMENULIST_CANCEL ){
    GFL_MSGDATA * mman;
    u32 tmp;

    // 威力
    tmp = WAZADATA_GetParam( prm, WAZAPARAM_POWER );
    if( tmp <= 1 ){
      GFL_MSG_GetString( wk->mman, msg_waza_oboe_01_12, wk->mbuf );
    }else{
      NumPrmSet( wk, msg_waza_oboe_01_08, tmp, 3, STR_NUM_DISP_LEFT );
    }
    StrPut( wk, WIN_PRM_ATTACK, wk->fontHandle, WOFCOL_N_BLACK, STR_MODE_RIGHT ,0);

    // 命中
    tmp = WAZADATA_GetParam( prm, WAZAPARAM_HITPER );
    if( tmp == 0 || WAZADATA_IsAlwaysHit( prm )){
      GFL_MSG_GetString( wk->mman, msg_waza_oboe_01_12, wk->mbuf );
    }else{
      NumPrmSet( wk, msg_waza_oboe_01_09, tmp, 3, STR_NUM_DISP_LEFT );
    }
    StrPut( wk, WIN_PRM_HIT, wk->fontHandle, WOFCOL_N_BLACK, STR_MODE_RIGHT ,0);

    // 説明
    mman = GFL_MSG_Create(
        GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wazainfo_dat, HEAPID_WAZAOSHIE );
    GFL_MSG_GetString( mman, prm, wk->mbuf );
    StrPut( wk, WIN_BTL_INFO, wk->fontHandle, WOFCOL_N_BLACK, STR_MODE_LEFT ,0);
    GFL_MSG_Delete( mman );

    WO_KindIconChange( wk, prm );
    GFL_CLACT_WK_SetDrawEnable( wk->cap[WO_CLA_KIND], 1 );

    GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_STR_ATTACK] );
    GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_STR_HIT] );
  }else{
    GFL_BMPWIN_ClearTransWindow_VBlank( wk->win[WIN_STR_ATTACK] );
    GFL_BMPWIN_ClearTransWindow_VBlank( wk->win[WIN_STR_HIT] );
  }

  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_BTL_INFO] );
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_PRM_ATTACK] );
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->win[WIN_PRM_HIT] );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ作成
 *
 * @param wk    ワーク
 * @param id    メッセージID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_MsgMake( WO_WORK * wk, u32 id )
{
  STRBUF * str;

  switch( id ){
  case MSG_SEL_WAZA:  // @0に　どのわざを　おぼえさせる？
    WORDSET_RegisterPokeNickName( wk->wset, 0, wk->dat->pp );
    break;
  case MSG_SET_CHECK: // @1　を  おぼえさせますか？
    WORDSET_RegisterWazaName( wk->wset, 1, WO_SelWazaGet( wk ) );
    break;
  case MSG_END_CHECK: // @0に　わざを  おぼえさせるのを　あきらめますか？
    WORDSET_RegisterPokeNickName( wk->wset, 0, wk->dat->pp );
    break;
  case MSG_SET_WAZA:  // @0は　あたらしく　@1を　おぼえた！▼"
    WORDSET_RegisterPokeNickName( wk->wset, 0, wk->dat->pp );
    WORDSET_RegisterWazaName( wk->wset, 1, WO_SelWazaGet( wk ) );
    break;
  case MSG_CHG_CHECK: // @0は　あたらしく　@1を　おぼえたい……▼　しかし　～
    WORDSET_RegisterPokeNickName( wk->wset, 0, wk->dat->pp );
    WORDSET_RegisterWazaName( wk->wset, 1, WO_SelWazaGet( wk ) );
    break;
  case MSG_DEL_WAZA:  // １　２の……　ポカン！▼　@0は　@1の つかいかたを　～
    WORDSET_RegisterPokeNickName( wk->wset, 0, wk->dat->pp );
    WORDSET_RegisterWazaName( wk->wset, 1, WO_DelWazaGet( wk ) );
    break;
  case MSG_CHG_WAZA:  // @0は　あたらしく @1を　おぼえた！▼
    WORDSET_RegisterPokeNickName( wk->wset, 0, wk->dat->pp );
    WORDSET_RegisterWazaName( wk->wset, 1, WO_SelWazaGet( wk ) );
    break;
  case MSG_DEL_CHECK: // それでは……　@1を　おぼえるのを　あきらめますか？
    WORDSET_RegisterWazaName( wk->wset, 1, WO_SelWazaGet( wk ) );
    break;
  case MSG_CANCEL:  // @0は　@1を　おぼえずに　おわった！▼
    WORDSET_RegisterPokeNickName( wk->wset, 0, wk->dat->pp );
    WORDSET_RegisterWazaName( wk->wset, 1, WO_SelWazaGet( wk ) );
    break;
  case MSG_SUB_ITEM:  // @2は　おれいに　ハートのウロコを　１まい　あげた！▼
    WORDSET_RegisterPlayerName( wk->wset, 2, wk->dat->myst );
    break;
  case MSG_DEL_ENTER: // @0を　わすれさせて　よろしいですね？
    WORDSET_RegisterWazaName( wk->wset, 0, WO_DelWazaGet( wk ) );
    break;
  }

  str = GFL_MSG_CreateString( wk->mman, MsgDataTbl[wk->dat->mode][id] );
  WORDSET_ExpandStr( wk->wset, wk->mbuf, str );
  GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------------------------
/**
 * 覚える技を取得
 *
 * @param wk    ワーク
 *
 * @return  覚える技
 */
//--------------------------------------------------------------------------------------------
static u16 WO_SelWazaGet( WO_WORK * wk )
{
  return wk->dat->waza_tbl[ wk->dat->scr + wk->dat->pos ];
}

//--------------------------------------------------------------------------------------------
/**
 * 忘れる技取得
 *
 * @param wk    ワーク
 *
 * @return  忘れる技
 */
//--------------------------------------------------------------------------------------------
static u16 WO_DelWazaGet( WO_WORK * wk )
{
  return (u16)PP_Get( wk->dat->pp, ID_PARA_waza1+wk->dat->del_pos, NULL );
}

//--------------------------------------------------------------------------------------------
/**
 * 会話（風）メッセージ表示
 *
 * @param wk    ワーク
 * @param id    メッセージID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_TalkMsgSet( WO_WORK * wk, u32 id )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[WIN_MSG]), 15 );
  BmpWinFrame_Write( wk->win[WIN_MSG], WINDOW_TRANS_ON, WO_TALK_WIN_CGX, WO_PAL_TALK_WIN );

  WO_MsgMake( wk, id );

  wk->printStream = PRINTSYS_PrintStreamCallBack(
    wk->win[WIN_MSG], 0, 0, wk->mbuf, wk->fontHandle,
    MSGSPEED_GetWait(), wk->pMsgTcblSys,
    0, HEAPID_WAZAOSHIE, 0xffff, WO_TalkMsgCallBack );

  GFL_BMPWIN_MakeTransWindow( wk->win[WIN_MSG] );

}

//--------------------------------------------------------------------------------------------
/**
 * 会話（風）メッセージ表示OFF
 *
 * @param wk    ワーク
 * @param id    メッセージID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_TalkMsgOff( WO_WORK * wk)
{
  BmpWinFrame_Clear( wk->win[WIN_MSG],WINDOW_TRANS_OFF);
  GFL_BMPWIN_ClearTransWindow_VBlank( wk->win[WIN_MSG]);
}

//--------------------------------------------------------------------------------------------
/**
 * 会話（風）メッセージコールバック
 *
 * @param mph   メッセージ描画データ
 * @param value コールバックナンバ
 *
 * @retval  "TRUE = 待ち"
 * @retval  "FALSE = 次の処理へ"
 */
//--------------------------------------------------------------------------------------------
static BOOL WO_TalkMsgCallBack( u32 value )
{
  switch( value ){
  case 1:   // SE終了待ち
    return PMSND_CheckPlaySE();

  case 2:   // SE終了待ち
    return PMSND_CheckPlaySE();
  case 3:   // "ポカン"
    PMSND_PlaySE( SEQ_SE_KON );
    break;

  case 4:   // "おぼえた"
    PMSND_PauseBGM( TRUE );
    PMSND_PushBGM();
    PMSND_PlayBGM( SEQ_ME_LVUP );
    break;

  case 5:   // "ポカン"のSE終了待ち
    if(PMSND_CheckPlayBGM()==FALSE){
      PMSND_PopBGM();
      PMSND_PauseBGM( FALSE );
    }else{
      return TRUE;
    }
    
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 技を４つ覚えているかチェック
 *
 * @param wk    ワーク
 *
 * @retval  "覚えている場合 = 4"
 * @retval  "覚えていない場合 = 0 ～ 3"
 */
//--------------------------------------------------------------------------------------------
static u8 WO_WazaSetCheck( WO_WORK * wk )
{
  u8  i;

  for( i=0; i<4; i++ ){
    if( PP_Get( wk->dat->pp, ID_PARA_waza1+i, NULL ) == 0 ){
      break;
    }
  }
  return i;
}


//--------------------------------------------------------------------------------------------
/**
 * 「@1　を  おぼえさせますか？」で「はい」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SetCheck_Yes( WO_WORK * wk )
{
  WO_TalkMsgSet( wk, MSG_SET_WAZA );
  wk->dat->del_pos = WO_WazaSetCheck( wk );
  wk->next_seq = SEQ_WAZA_SET;
  return SEQ_MSG_WAIT;
}
//--------------------------------------------------------------------------------------------
/**
 * 「@1　を  おぼえさせますか？」で「いいえ」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SetCheck_No( WO_WORK * wk )
{
  WO_TalkMsgOff(wk);

  return SEQ_SELECT;
}

//--------------------------------------------------------------------------------------------
/**
 * 「@0に　わざを  おぼえさせるのを　あきらめますか？」で「はい」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_EndCheck_Yes( WO_WORK * wk )
{
  wk->dat->ret = WAZAOSHIE_RET_CANCEL;
  return SEQ_FADEOUT_SET;
}
//--------------------------------------------------------------------------------------------
/**
 * 「@0に　わざを  おぼえさせるのを　あきらめますか？」で「いいえ」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_EndCheck_No( WO_WORK * wk )
{
  WO_TalkMsgOff(wk);
//  WO_SelCursorChange( wk, wk->dat->pos, PALDW_CURSOR );
  return SEQ_SELECT;
}

//--------------------------------------------------------------------------------------------
/**
 * 「@0は　あたらしく　@1を　おぼえたい……▼　しかし　～」で「はい」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_ChgCheck_Yes( WO_WORK * wk )
{
  wk->next_seq = SEQ_PST_INIT;
  return SEQ_FADE_WAIT;
}
//--------------------------------------------------------------------------------------------
/**
 * 「@0は　あたらしく　@1を　おぼえたい……▼　しかし　～」で「いいえ」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_ChgCheck_No( WO_WORK * wk )
{
  WO_TalkMsgSet( wk, MSG_DEL_CHECK );
  wk->ynidx = YESNO_DEL_CHECK;
  wk->next_seq = SEQ_YESNO_PUT;
  return SEQ_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * 「それでは……　@1を　おぼえるのを　あきらめますか？」で「はい」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_DelCheck_Yes( WO_WORK * wk )
{
  WO_TalkMsgSet( wk, MSG_CANCEL );
  wk->next_seq = SEQ_FADEOUT_SET;
  wk->dat->ret = WAZAOSHIE_RET_CANCEL;
  return SEQ_MSG_WAIT;
}
//--------------------------------------------------------------------------------------------
/**
 * 「それでは……　@1を　おぼえるのを　あきらめますか？」で「いいえ」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_DelCheck_No( WO_WORK * wk )
{
  WO_TalkMsgSet( wk, MSG_CHG_CHECK );
  wk->ynidx = YESNO_CHG_CHECK;
  wk->next_seq = SEQ_YESNO_PUT;
  return SEQ_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * 「@0を　わすれさせて　よろしいですね？」で「はい」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_DelEnter_Yes( WO_WORK * wk )
{
  WO_TalkMsgSet( wk, MSG_DEL_WAZA );
  wk->next_seq = SEQ_WAZADEL_SET;
  return SEQ_MSG_WAIT;
}
//--------------------------------------------------------------------------------------------
/**
 * 「@0を　わすれさせて　よろしいですね？」で「いいえ」が選択された場合
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_DelEnter_No( WO_WORK * wk )
{
  wk->next_seq = SEQ_PST_INIT;
  return SEQ_FADE_WAIT;
}



//============================================================================================
//  セルアクター
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * リソースマネージャー初期化
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_ClactResManInit( WO_WORK * wk )
{

  static const GFL_CLSYS_INIT clsys_init  =
  {
    0, 0,   //メインサーフェースの左上X,Y座標
    0, 512, //サブサーフェースの左上X,Y座標
    4, 124, //メインOAM管理開始～終了 （通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
    4, 124, //差bOAM管理開始～終了  （通信アイコン用に開始は4以上に、またすべて4の倍数いしてください）
    0,      //セルVram転送管理数
    WO_CHR_ID_MAX, WO_PAL_ID_MAX, WO_CEL_ID_MAX, 32,
    16, 16,       //メイン、サブのCGRVRAM管理領域開始オフセット（通信アイコン用に16以上にしてください）
  };


  //システム作成
  GFL_CLACT_SYS_Create( &clsys_init, &waza_oshie_vram, HEAPID_WAZAOSHIE );
  wk->clUnit = GFL_CLACT_UNIT_Create( WO_CLA_MAX, 0, HEAPID_WAZAOSHIE );
  GFL_CLACT_UNIT_SetDefaultRend( wk->clUnit );



  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}


//--------------------------------------------------------------------------------------------
/**
 * セルアクター解放
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_ObjFree( WO_WORK * wk )
{
  u32 i;

  for( i=0; i<WO_CLA_MAX; i++ ){
    GFL_CLACT_WK_Remove( wk->cap[i] );
  }

  //システム破棄
  GFL_CLACT_UNIT_Delete( wk->clUnit );
  GFL_CLACT_SYS_Delete();

  GFL_ARC_CloseDataHandle( wk->TypeIconHandle );

}

//--------------------------------------------------------------------------------------------
/**
 * 状態異常アイコンのリソース設定
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_ResourceLoad( WO_WORK * wk, ARCHANDLE* p_handle )
{
  u32 i;
  ARCHANDLE *c_handle;
  POKEMON_PASO_PARAM * ppp;
  BOOL  fast;
  // キャラ
  // メニュー共通リソースハンドルオープン
  c_handle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_WAZAOSHIE );

    // タッチバーキャラ登録
  wk->clres[0][WO_CHR_ID_APP_COMMON] = GFL_CLGRP_CGR_Register(
                            c_handle, APP_COMMON_GetBarIconCharArcIdx(), 0,
                            CLSYS_DRAW_SUB,   HEAPID_WAZAOSHIE );
  wk->clres[0][WO_CHR_ID_CURSOR] = GFL_CLGRP_CGR_Register(
                            p_handle, NARC_waza_oshie_gra_scroll_cur_NCGR, 0,
                            CLSYS_DRAW_SUB, HEAPID_WAZAOSHIE );

  // 種類アイコン
  wk->clres[0][WO_CHR_ID_KIND] = GFL_CLGRP_CGR_Register( p_handle,
                                    NARC_waza_oshie_gra_p_st_bunrui_NCGR, 0,
                                    CLSYS_DRAW_SUB, HEAPID_WAZAOSHIE );


  // 上画面技タイプアイコン(上画面の技タイプアイコンはキャラ転送で切り替える）
  for( i=WO_CHR_ID_U_TYPE1; i<WO_CHR_ID_U_TYPE1+4; i++ ){
    wk->clres[0][i] = GFL_CLGRP_CGR_Register( c_handle,
                                      APP_COMMON_GetPokeTypeCharArcIdx(0), 0,
                                      CLSYS_DRAW_MAIN, HEAPID_WAZAOSHIE );
  }

  // 下画面タイプアイコン（下画面の技タイプアイコンは全種類転送しておきセル参照で切り替える）
  wk->clres[0][WO_CHR_ID_TYPE] = GFL_CLGRP_CGR_Register( p_handle,
                                      NARC_waza_oshie_gra_p_st_type_NCGR, 0,
                                      CLSYS_DRAW_SUB, HEAPID_WAZAOSHIE );

  // パレット
  wk->clres[1][WO_PAL_ID_APP_COMMON] = GFL_CLGRP_PLTT_RegisterEx(
                                        c_handle, APP_COMMON_GetBarIconPltArcIdx(),
                                        CLSYS_DRAW_SUB, 0, 0, 4, HEAPID_WAZAOSHIE );
  wk->clres[1][WO_PAL_ID_OBJ] = GFL_CLGRP_PLTT_RegisterEx(
                                p_handle, NARC_waza_oshie_gra_oam_dw_NCLR,
                                CLSYS_DRAW_SUB, 4*32,0,1, HEAPID_WAZAOSHIE );

  wk->clres[1][WO_PAL_ID_TYPE]   = GFL_CLGRP_PLTT_RegisterEx(
                                    c_handle, APP_COMMON_GetPokeTypePltArcIdx(),
                                    CLSYS_DRAW_SUB, DW_ACTPAL_NUM*32,
                                    0,3,HEAPID_WAZAOSHIE );

  wk->clres[1][WO_PAL_ID_TYPE_M] = GFL_CLGRP_PLTT_Register(
                                    c_handle, APP_COMMON_GetPokeTypePltArcIdx(),
                                    CLSYS_DRAW_MAIN, 0, HEAPID_WAZAOSHIE );

  OS_Printf("wk->clres[1][WO_PAL_ID_TYPE_M]=%d\n",wk->clres[1][WO_PAL_ID_TYPE_M]);

  // セル
  wk->clres[2][WO_CEL_ID_APP_COMMON] = GFL_CLGRP_CELLANIM_Register(
                                        c_handle, 
                                        APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_64K),
                                        APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_64K), 
                                        HEAPID_WAZAOSHIE );
  wk->clres[2][WO_CEL_ID_CURSOR] = GFL_CLGRP_CELLANIM_Register(
                                      p_handle, NARC_waza_oshie_gra_oam_dw_NCER,
                                      NARC_waza_oshie_gra_oam_dw_NANR, HEAPID_WAZAOSHIE );

  wk->clres[2][WO_CEL_ID_U_TYPE] = GFL_CLGRP_CELLANIM_Register(
                                      c_handle,
                                      APP_COMMON_GetPokeTypeCellArcIdx(APP_COMMON_MAPPING_64K),
                                      APP_COMMON_GetPokeTypeAnimeArcIdx(APP_COMMON_MAPPING_64K),
                                      HEAPID_WAZAOSHIE );

  wk->clres[2][WO_CEL_ID_D_TYPE] = GFL_CLGRP_CELLANIM_Register(
                                      p_handle,
                                      NARC_waza_oshie_gra_p_st_type_NCER,
                                      NARC_waza_oshie_gra_p_st_type_NANR,
                                      HEAPID_WAZAOSHIE );

  wk->clres[2][WO_CEL_ID_KIND] = GFL_CLGRP_CELLANIM_Register(
                                      p_handle,
                                      NARC_waza_oshie_gra_p_st_bunrui_NCER,
                                      NARC_waza_oshie_gra_p_st_bunrui_NANR,
                                      HEAPID_WAZAOSHIE );


  GFL_ARC_CloseDataHandle( c_handle );

  // ポケモン正面絵
  c_handle = POKE2DGRA_OpenHandle( HEAPID_WAZAOSHIE );

  ppp  = PP_GetPPPPointer( wk->dat->pp );
  fast = PPP_FastModeOn( ppp );
  wk->clres[0][WO_CHR_ID_POKEGRA] = POKE2DGRA_OBJ_CGR_RegisterPPP( c_handle, ppp, POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, HEAPID_WAZAOSHIE );
  wk->clres[1][WO_PAL_ID_POKEGRA] = POKE2DGRA_OBJ_PLTT_RegisterPPP( c_handle, ppp, POKEGRA_DIR_FRONT, CLSYS_DRAW_MAIN, 4*32, HEAPID_WAZAOSHIE );
  wk->clres[2][WO_CEL_ID_POKEGRA] = POKE2DGRA_OBJ_CELLANM_RegisterPPP( ppp, POKEGRA_DIR_FRONT, APP_COMMON_MAPPING_64K, CLSYS_DRAW_MAIN, HEAPID_WAZAOSHIE );
  PPP_FastModeOff( ppp, fast );

  GFL_ARC_CloseDataHandle( c_handle );
}


static void CGR_Replace( ARCHANDLE *handle, u32 res_index, u32 ArcId, BOOL compressFlag, HEAPID heapId );
//----------------------------------------------------------------------------------
/**
 * @brief 常駐型OBJのキャラリソースを差し替える
 *
 * @param   handle        読み出すデータのファイルハンドル
 * @param   res_index     差し替えるOBJキャラリソースハンドル
 * @param   ArcId         読み込むOBJキャラデータ
 * @param   compressFlag  圧縮フラグ
 * @param   heapId        ヒープID
 */
//----------------------------------------------------------------------------------
static void CGR_Replace( ARCHANDLE *handle, u32 res_index, u32 ArcId, BOOL compressFlag, HEAPID heapId )
{
  NNSG2dCharacterData* charData;
  void *buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( handle, ArcId, compressFlag, &charData, heapId );

  GFL_CLGRP_CGR_Replace( res_index, charData );

  GFL_HEAP_FreeMemory( buf );
}

//--------------------------------------------------------------------------------------------
/**
 * タイプアイコン切り替えリクエスト
 *
 * @param wk    ワーク
 * @param waza  技番号
 * @param num   アイコン番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_TypeIconChangeRequest( WO_WORK * wk, u16 waza, u16 res_offset )
{
    u32 type = WAZADATA_GetParam( waza, WAZAPARAM_TYPE );
    wk->WazaIconReq[res_offset][0] = type;
    wk->WazaIconReq[res_offset][1] = TRUE;
  
    // パレットオフセット変更
    GFL_CLACT_WK_SetPlttOffs( wk->cap[WO_CLA_TYPE5+res_offset],
                              APP_COMMON_GetPokeTypePltOffset(type),
                              CLWK_PLTTOFFS_MODE_PLTT_TOP);
}


//----------------------------------------------------------------------------------
/**
 * @brief 技タイプアイコン切り替え&転送
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void WO_TepeIconChangeVblank( WO_WORK *wk )
{
  // キャラ書き換え
  int i;
  
  for(i=0;i<WAZATYPE_ICON_NUM;i++){
    if(wk->WazaIconReq[i][1]){
      u32 type = wk->WazaIconReq[i][0];

      OS_Printf("icon[%d] rewrite\n",i);

      // キャラ転送
      CGR_Replace( wk->TypeIconHandle, wk->clres[0][i+WO_CHR_ID_U_TYPE1],
                   APP_COMMON_GetPokeTypeCharArcIdx(type), 0, HEAPID_WAZAOSHIE );
    
      wk->WazaIconReq[i][1] = FALSE;
    }
  }
}

//--------------------------------------------------------------------------------------------
/**
 * タイプアイコン初期化
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_TypeIconInit( WO_WORK * wk )
{
  u32 i;
  GFL_CLACTPOS clpos;

  for( i=0; i<LIST_NUM; i++ ){
    clpos.x = TYPE_ICON1_PX;
    clpos.y = TYPE_ICON1_PY+SEL_CURSOR_SY*i;
    GFL_CLACT_WK_SetPos( wk->cap[WO_CLA_TYPE1+i], &clpos, CLSYS_DEFREND_SUB );
    if( wk->dat->scr + i >= wk->sel_max ){
      GFL_CLACT_WK_SetDrawEnable( wk->cap[WO_CLA_TYPE1+i], 0 );
    }else{
      u32 waza = wk->dat->waza_tbl[wk->dat->scr+i];
      u32 type = WAZADATA_GetParam( waza, WAZAPARAM_TYPE );
      GFL_CLACT_WK_SetDrawEnable( wk->cap[WO_CLA_TYPE1+i], 1 );
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_TYPE1+i], type );
    }
  }
}


//--------------------------------------------------------------------------------------------
/**
 * 分類アイコン切り替え
 *
 * @param wk    ワーク
 * @param waza  技番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_KindIconChange( WO_WORK * wk, u16 waza )
{
  u32 kind = WAZADATA_GetParam( waza, WAZAPARAM_DAMAGE_TYPE );

  // アイコン切り替え
  GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_KIND], kind );


}


//--------------------------------------------------------------------------------------------
/**
 * セルアクター初期化
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_ObjInit( WO_WORK * wk, ARCHANDLE* p_handle )
{
  u32 i;

  WO_ClactResManInit( wk );
  WO_ResourceLoad( wk, p_handle );

  // 技タイプアイコンハンドルオープン
  wk->TypeIconHandle = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_WAZAOSHIE );

  for( i=0; i<WO_CLA_MAX; i++ ){
    GFL_CLWK_DATA dat;
    dat.pos_x = ClactParamTbl[i].x;       // ｘ座標
    dat.pos_y = ClactParamTbl[i].y;       // ｙ座標
    dat.anmseq  = ClactParamTbl[i].anm;       // アニメーションシーケンス
    dat.softpri = ClactParamTbl[i].pri;     // ソフト優先順位 0>0xff
    dat.bgpri   = ClactParamTbl[i].bg_pri;        // BG優先順位

    OS_Printf("%d:pal=%d handle=%d\n", i, ClactParamTbl[i].id[1], wk->clres[1][ClactParamTbl[i].id[1]]);

    wk->cap[i] = GFL_CLACT_WK_Create( wk->clUnit, wk->clres[0][ClactParamTbl[i].id[0]],
                                               wk->clres[1][ClactParamTbl[i].id[1]],
                                               wk->clres[2][ClactParamTbl[i].id[2]], &dat,
                                               ClactParamTbl[i].d_area, HEAPID_WAZAOSHIE );
    GFL_CLACT_WK_SetAutoAnmFlag(wk->cap[i],FALSE);
  }
  GFL_CLACT_WK_SetAutoAnmFlag( wk->cap[WO_CLA_EXIT],TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->cap[WO_CLA_CURSOR],TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->cap[WO_CLA_ARROW_U], TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->cap[WO_CLA_ARROW_D], TRUE );

}



//--------------------------------------------------------------------------------------------
/**
 * 技選択カーソル切り替え
 *
 * @param wk    ワーク
 * @param pos   表示位置
 * @param pal   表示パレット
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void WO_SelCursorChange( WO_WORK * wk, u8 pos, u8 pal )
{
  GFL_CLACTPOS clpos;
  if(pos < LIST_NUM){
    clpos.x = SEL_CURSOR_PX; clpos.y = SEL_CURSOR_PY+SEL_CURSOR_SY*pos;
    GFL_CLACT_WK_SetPos(wk->cap[WO_CLA_CURSOR], &clpos, CLSYS_DEFREND_SUB );
    GFL_CLACT_WK_SetAnmFrame(wk->cap[WO_CLA_CURSOR],0);
  }else if(pos < (LIST_NUM+2)){
    clpos.x = SEL_CURSOR_P_PX+(pos-LIST_NUM)*SEL_CURSOR_P_SX; clpos.y = SEL_CURSOR_P_PY;
    GFL_CLACT_WK_SetPos(wk->cap[WO_CLA_CURSOR], &clpos, CLSYS_DEFREND_SUB );

    GFL_CLACT_WK_SetAnmFrame(wk->cap[WO_CLA_CURSOR],1);
  }else{
    clpos.x = SEL_CURSOR_B_PX; clpos.y = SEL_CURSOR_B_PY;
    GFL_CLACT_WK_SetPos(wk->cap[WO_CLA_CURSOR], &clpos, CLSYS_DEFREND_SUB );
    GFL_CLACT_WK_SetAnmFrame(wk->cap[WO_CLA_CURSOR],2);
  }
}



//----------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル表示切替
 *
 * @param   wk      技教えワーク
 * @param   idx     0だと下、1だと上
 * @param   anm_f   
 */
//----------------------------------------------------------------------------------
static void WO_ScrollCursorPut( WO_WORK *wk, u8 idx, BOOL anm_f )
{
  u8 flag;

  // 下向き
  if(idx == 0 && anm_f){
    flag = 1;
    GFL_CLACT_WK_ResetAnm(wk->cap[WO_CLA_ARROW_D]);
  }else{
    flag = 0;
  }

  // 上向き
  if(idx == 1 && anm_f){
    flag = 1;
    GFL_CLACT_WK_ResetAnm(wk->cap[WO_CLA_ARROW_U]);
  }else{
    flag = 0;
  }
  
}



//--------------------------------------------------------------------------------------------
/**
 * ステータス画面呼び出しセット
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqPstInit( WO_WORK * wk )
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT,  WIPE_TYPE_FADEOUT,
                  WIPE_FADE_BLACK,  WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WAZAOSHIE );
  wk->next_seq = SEQ_PST_CALL;
  return SEQ_FADE_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス画面呼び出し
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqPstCall( WO_WORK * wk )
{

  WO_DispExit( wk );

  wk->psd.ppd  = wk->dat->pp;
  wk->psd.cfg  = wk->dat->cfg;
  wk->psd.game_data  = GAMESYSTEM_GetGameData( wk->dat->gsys );
  wk->psd.ppt  = PST_PP_TYPE_POKEPARAM;
  wk->psd.pos  = 0;
  wk->psd.max  = 1;
  wk->psd.waza = WO_SelWazaGet( wk );
  wk->psd.mode = PST_MODE_WAZAADD;
  wk->psd.page = PPT_INFO;

  wk->psd.zukan_mode = 0;


  // WBでのPROC呼び出しはアプリと平行の存在になるので、
  // 呼び出した時点から現在のアプリには来なくなる。
  GAMESYSTEM_CallProc( wk->dat->gsys,
                        FS_OVERLAY_ID(poke_status), &PokeStatus_ProcData, &wk->psd );

  return SEQ_PST_WAIT;
}


// 忘れる技を選ばなかった
#define PSTATUS_SELECT_WAZA_CANCEL    ( 4 )

//--------------------------------------------------------------------------------------------
/**
 * ステータス画面終了待ち
 *
 * @param wk    ワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int WO_SeqPstWait( WO_WORK * wk )
{
  // ステータス画面終了時にここに来る
    WO_DispInit( wk );
    WO_SelCursorChange( wk, wk->dat->pos, PALDW_CURSOR);
    GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_WAZAOSHIE );
    if(wk->psd.ret_mode==PST_RET_DECIDE){
      wk->dat->del_pos = wk->psd.ret_sel;
    }else{
      wk->dat->del_pos = PSTATUS_SELECT_WAZA_CANCEL;
    }
    wk->next_seq = SEQ_DEL_CHECK;
    return SEQ_FADE_WAIT;

  return SEQ_PST_WAIT;
}






//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
static void CursorMoveCallBack( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos );

static void ScrollButtonOnOff( WO_WORK * wk );
static void EnterButtonOnOff( WO_WORK * wk, BOOL flg );


// CURSORMOVEに渡すボタン情報
static const CURSORMOVE_DATA ListKeyTbl[]={
  { 40,  52, 0, 0,  0, 1, 0, 0 ,  {TP_WAZA1_PY,TP_WAZA1_PY+TP_WAZA_SY-1,TP_WAZA_PX,TP_WAZA_PX+TP_WAZA_SX-1},},// 00: 技１（ダミーで「もどる」にループ）
  { 80,  60, 0, 0,  0, 2, 1, 1 ,  {TP_WAZA2_PY,TP_WAZA2_PY+TP_WAZA_SY-1,TP_WAZA_PX,TP_WAZA_PX+TP_WAZA_SX-1},},// 01: 技２
  { 40,  84, 0, 0,  1, 3, 2, 2 ,  {TP_WAZA3_PY,TP_WAZA3_PY+TP_WAZA_SY-1,TP_WAZA_PX,TP_WAZA_PX+TP_WAZA_SX-1},},// 02: 技３
  { 80,  92, 0, 0,  2, 6, 3, 3 ,  {TP_WAZA4_PY,TP_WAZA4_PY+TP_WAZA_SY-1,TP_WAZA_PX,TP_WAZA_PX+TP_WAZA_SX-1},},// 03: 技４
  { 40, 116, 0, 0,  4, 4, 4, 4 ,  {TP_SB_PY,TP_SB_PY+TP_SB_SY-1,TP_SBD_PX,TP_SBD_PX+TP_SB_SX-1},},// 04: 下矢印
  { 80, 124, 0, 0,  5, 5, 5, 5 ,  {TP_SB_PY,TP_SB_PY+TP_SB_SY-1,TP_SBU_PX,TP_SBU_PX+TP_SB_SX-1},},// 05: 上矢印
  { 224, 168, 0, 0, 3, 6, 6, 6 ,  {TP_BACK_PY,TP_BACK_PY+TP_BACK_SY-1,TP_BACK_PX,TP_BACK_PX+TP_BACK_SX-1},},// 06: もどる
  { 224, 168, 0, 0, 7, 7, 7, 7 ,  {TP_ABTN_PY,TP_ABTN_PY+TP_ABTN_SY-1,TP_ABTN_PX,TP_ABTN_PX+TP_ABTN_SX-1},},// 07: おぼえる
  { 0, 0, 0, 0, 0, 0, 0, 0,       {GFL_UI_TP_HIT_END, 0, 0, 0 }},

};

// CURSORMOVEに渡すコールバック関数データ
static const CURSORMOVE_CALLBACK ListCallBack = {
  CursorMoveCallBack,
  CursorMoveCallBack,
  CursorMoveCallBack_Move,
  CursorMoveCallBack_Touch
};

//----------------------------------------------------------------------------------
/**
 * @brief カーソル・タッチ共通処理初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void CursorMoveInit( WO_WORK * wk )
{
  wk->cmwk = CURSORMOVE_Create( ListKeyTbl, &ListCallBack, wk, TRUE, 0, HEAPID_WAZAOSHIE );

  ScrollButtonOnOff( wk );    // スクロール設定
}


//----------------------------------------------------------------------------------
/**
 * @brief カーソル・タッチ共通処理解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void CursorMoveExit( WO_WORK * wk )
{
  CURSORMOVE_Exit( wk->cmwk );
}

//----------------------------------------------------------------------------------
/**
 * @brief カーソルタッチ共通処理コールバック
 *
 * @param   work      
 * @param   now_pos   現在位置
 * @param   old_pos   過去位置
 */
//----------------------------------------------------------------------------------
static void CursorMoveCallBack( void * work, int now_pos, int old_pos )
{
  // ダミー関数
}

//----------------------------------------------------------------------------------
/**
 * @brief 【CURSORMOVEコールバック】キー操作時
 *
 * @param   work      
 * @param   now_pos   現在位置
 * @param   old_pos   過去位置
 */
//----------------------------------------------------------------------------------
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
  WO_WORK * wk = work;

  // 決定を消す
  EnterButtonOnOff( wk, FALSE );

  // 技
  if( now_pos <= 3 ){
    PMSND_PlaySE( WO_SE_LIST_MOVE );
    wk->dat->pos = now_pos;
    if( wk->dat->scr + wk->dat->pos < wk->sel_max ){
      BattleWazaParamPut( wk, WO_SelWazaGet( wk ) );
    }else{
      BattleWazaParamPut( wk, BMPMENULIST_CANCEL );
    }
  //「もどる」
  }else if( now_pos == 6 ){
    // 下の技から
    if( old_pos == 3 && wk->dat->scr + 4 < wk->sel_max ){
      PMSND_PlaySE( WO_SE_PAGE_MOVE );
      now_pos = 3;
      CURSORMOVE_PosSet( wk->cmwk, now_pos );
      wk->dat->scr++;
      WO_WazaListDraw( wk );
      BattleWazaParamPut( wk, WO_SelWazaGet( wk ) );
      ScrollButtonOnOff( wk );
      ScrollButtonAnmChange( wk, 1 );
      
    // 上の技から
    }else if( old_pos == 0 ){
      if( wk->dat->scr != 0 ){
        PMSND_PlaySE( WO_SE_PAGE_MOVE );
        wk->dat->scr--;
        WO_WazaListDraw( wk );
        BattleWazaParamPut( wk, WO_SelWazaGet( wk ) );
        ScrollButtonOnOff( wk );
        ScrollButtonAnmChange( wk, -1 );
      }
      now_pos = 0;
      CURSORMOVE_PosSet( wk->cmwk, now_pos );

    // 技の最後なので「もどるに移動」、、、させない
    }else{
      now_pos = 3;
      CURSORMOVE_PosSet( wk->cmwk, 3 );
    }
  // その他
  }else{
    PMSND_PlaySE( WO_SE_LIST_MOVE );
  }

  WO_SelCursorChange( wk, now_pos, PALDW_CURSOR );
}

//----------------------------------------------------------------------------------
/**
 * @brief 【CURSORMOVEコールバック】タッチ時
 *
 * @param   work    
 * @param   now_pos   現在位置
 * @param   old_pos   過去位置
 */
//----------------------------------------------------------------------------------
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
  WO_WORK * wk = work;

  // 技
  if( now_pos <= 3 ){
    wk->dat->pos = now_pos;
    if( wk->dat->scr + wk->dat->pos < wk->sel_max ){
      EnterButtonOnOff( wk, TRUE ); // 決定を表示
      BattleWazaParamPut( wk, WO_SelWazaGet( wk ) );
    }else{
      EnterButtonOnOff( wk, FALSE );  // 決定を消す
      BattleWazaParamPut( wk, BMPMENULIST_CANCEL );
    }
  // 下
  }else if( now_pos == 4 ){
    PMSND_PlaySE( WO_SE_PAGE_MOVE );
    now_pos = wk->dat->pos;
    wk->dat->scr++;
    BattleWazaParamPut( wk, WO_SelWazaGet( wk ) );
    WO_WazaListDraw( wk );
    EnterButtonOnOff( wk, FALSE );  // 決定を消す
    ScrollButtonOnOff( wk );
    ScrollButtonAnmChange( wk, 1 );
    CURSORMOVE_PosSet( wk->cmwk, now_pos );
  // 上
  }else if( now_pos == 5 ){
    PMSND_PlaySE( WO_SE_PAGE_MOVE );
    now_pos = wk->dat->pos;
    wk->dat->scr--;
    BattleWazaParamPut( wk, WO_SelWazaGet( wk ) );
    WO_WazaListDraw( wk );
    EnterButtonOnOff( wk, FALSE );  // 決定を消す
    ScrollButtonOnOff( wk );
    ScrollButtonAnmChange( wk, -1 );
    CURSORMOVE_PosSet( wk->cmwk, now_pos );
  // もどる
  }else if( now_pos == 6 ){
    if( wk->cm_pos != 6 ){
      now_pos = wk->dat->pos;
      CURSORMOVE_PosSet( wk->cmwk, now_pos );
    }
  // けってい
  }else if( now_pos == 7 ){
    now_pos = wk->dat->pos;
    CURSORMOVE_PosSet( wk->cmwk, now_pos );
  }

  OS_Printf("now_pos=%d\n", now_pos);
  WO_SelCursorChange( wk, now_pos, PALDW_CURSOR );
}

//----------------------------------------------------------------------------------
/**
 * @brief ボタン表示ON/OFF
 *
 * @param   wk    
 * @param   flg   TRUE:表示開始 FALSE:削除
 */
//----------------------------------------------------------------------------------
static void EnterButtonOnOff( WO_WORK * wk, BOOL flg )
{
  if(flg==TRUE)
  {
    if(wk->oboe_menu_work[0]==NULL){
      wk->oboe_menu_work[0] = APP_TASKMENU_WIN_Create( wk->app_res, &wk->menuitem[0], 13, 21, 9, HEAPID_WAZAOSHIE );
      wk->oboe_menu_work[1] = APP_TASKMENU_WIN_Create( wk->app_res, &wk->menuitem[1], 22, 21, 9, HEAPID_WAZAOSHIE );
      wk->enter_flg = 1;
    }
  }else {
    if(wk->oboe_menu_work[0]!=NULL){
      APP_TASKMENU_WIN_Delete( wk->oboe_menu_work[0] );
      APP_TASKMENU_WIN_Delete( wk->oboe_menu_work[1] );
      GFL_BG_LoadScreenReq( SFRM_MSG );
      wk->oboe_menu_work[0] = NULL;
      wk->oboe_menu_work[1] = NULL;
    }
    wk->enter_flg = 0;
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief 「おぼえる」「やめる」は表示されているか
 *
 * @param   wk    
 *
 * @retval  int   1:いる  0:いない
 */
//----------------------------------------------------------------------------------
static int EnterButtonCheck( WO_WORK *wk )
{
  if( wk->oboe_menu_work[0]!=NULL ){
    return  1;
  }
  
  return 0;
}

//----------------------------------------------------------------------------------
/**
 * @brief カーソルの位置で上下矢印表示を制御する
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ScrollButtonOnOff( WO_WORK * wk )
{
  if( wk->sel_max < 4 ){
    CURSORMOVE_MoveTableBitOff( wk->cmwk, 4 );
    CURSORMOVE_MoveTableBitOff( wk->cmwk, 5 );
    return;
  }

  if( wk->dat->scr == 0 ){
    CURSORMOVE_MoveTableBitOff( wk->cmwk, 5 );
  }else{
    CURSORMOVE_MoveTableBitOn( wk->cmwk, 5 );
  }

  if( wk->dat->scr + 4 >= wk->sel_max ){
    CURSORMOVE_MoveTableBitOff( wk->cmwk, 4 );
  }else{
    CURSORMOVE_MoveTableBitOn( wk->cmwk, 4 );
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief 上下ボタン表示アニメ変更
 *
 * @param   wk    
 * @param   mv    １：下に移動  0以下：上に移動
 */
//----------------------------------------------------------------------------------
static void ScrollButtonAnmChange( WO_WORK * wk, s32 mv )
{
  GFL_CLACT_WK_SetAnmFrame( wk->cap[WO_CLA_ARROW_U], 0 );
  GFL_CLACT_WK_SetAnmFrame( wk->cap[WO_CLA_ARROW_D], 0 );

  // 下に移動
  if( mv > 0 ){
    if( wk->dat->scr + 4 < wk->sel_max ){
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_D], ANMDW_ARROW_DTA);
    }else{
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_D], ANMDW_ARROW_DFA );
    }
    GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_U], ANMDW_ARROW_UT );
  }else{
  // 上に移動
    if( wk->dat->scr == 0 ){
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_U], ANMDW_ARROW_UFA);
    }else{
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_U], ANMDW_ARROW_UTA );
    }
    GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_D], ANMDW_ARROW_DT );
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief 画面初期化時のスクロール矢印アイコンの見た目設定
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void ScrollButtonInit( WO_WORK *wk )
{
  // 技の総数が４以下
  if(wk->sel_max<=4){
    GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_U], ANMDW_ARROW_UF );  //上タッチできない
    GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_D], ANMDW_ARROW_DF );  //下タッチできない
  }else{
    if(wk->dat->scr+4 < wk->sel_max){
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_U], ANMDW_ARROW_UF );  //上タッチできない
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_D], ANMDW_ARROW_DT );  //下タッチできる
    }else if(wk->dat->scr==0){
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_U], ANMDW_ARROW_UT );  // 上タッチできる
      GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_ARROW_D], ANMDW_ARROW_DF );  // 下タッチできない
    }
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief 終了ボタンアニメ開始
 *
 * @param   wk    
 * @param   next  やめるボタンアニメ終了後に遷移するシーケンス番号
 *
 * @retval  int   SEQ_BUTTON_ANM
 */
//----------------------------------------------------------------------------------
static int RetButtonAnmInit( WO_WORK * wk, int next )
{

  BUTTON_ANM_WORK * bawk = &wk->button_anm_work;

  bawk->mode  = WO_ANIME_OBJ;
  bawk->index = WO_CLA_EXIT;
  bawk->next  = next;
  // もどるマークアニメ
  GFL_CLACT_WK_SetAnmSeq( wk->cap[WO_CLA_EXIT], 9 );
  return SEQ_BUTTON_ANM;
}

//----------------------------------------------------------------------------------
/**
 * @brief 「おぼえる」ボタンアニメ開始
 *
 * @param   wk    
 * @param   next  おぼえるアニメの後に遷移するシーケンス番号
 *
 * @retval  int   SEQ_BUTTON_ANM
 */
//----------------------------------------------------------------------------------
static int EnterButtonAnmInit( WO_WORK * wk, int next )
{
  BUTTON_ANM_WORK * bawk = &wk->button_anm_work;

  bawk->mode  = WO_ANIME_APPTASK;
  bawk->index = 0;
  bawk->next  = next;
  OS_Printf("index=%d, mode=%d,next=%d, adr=%08x", bawk->index, bawk->mode, bawk->next, (u32)wk->oboe_menu_work[bawk->index]);
  APP_TASKMENU_WIN_SetDecide( wk->oboe_menu_work[bawk->index], TRUE );
  return SEQ_BUTTON_ANM;
}

//----------------------------------------------------------------------------------
/**
 * @brief 「やめる」ボタンアニメ開始
 *
 * @param   wk    
 * @param   next  やめるアニメの後に遷移するシーケンス番号
 *
 * @retval  int   SEQ_BUTTON_ANM
 */
//----------------------------------------------------------------------------------
static int YameruButtonAnmInit( WO_WORK *wk, int next )
{

  BUTTON_ANM_WORK * bawk = &wk->button_anm_work;
  bawk->mode  = WO_ANIME_APPTASK;
  bawk->index = 1;
  bawk->next  = next;
  OS_Printf("index=%d, mode=%d,next=%d, adr=%08x", bawk->index, bawk->mode, bawk->next, (u32)wk->oboe_menu_work[bawk->index]);
  APP_TASKMENU_WIN_SetDecide( wk->oboe_menu_work[bawk->index], TRUE );
  return SEQ_BUTTON_ANM;

}

//----------------------------------------------------------------------------------
/**
 * @brief ボタンアニメ待ち
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int ButtonAnmMaun( WO_WORK * wk )
{
  BUTTON_ANM_WORK * bawk = &wk->button_anm_work;

  // 指定のOBJかAPPTASKMENUかで終了待ちを行う
  if(bawk->mode==WO_ANIME_OBJ){
    if(GFL_CLACT_WK_CheckAnmActive( wk->cap[bawk->index])==FALSE){
      return bawk->next;
    }
  }else if(bawk->mode==WO_ANIME_APPTASK){
    if(APP_TASKMENU_WIN_IsFinish(wk->oboe_menu_work[bawk->index])){
        EnterButtonOnOff( wk, FALSE );
      return bawk->next;
    }
  }

  return SEQ_BUTTON_ANM;
}


//----------------------------------------------------------------------------------
/**
 * @brief 戻るボタン押した時の処理
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int WO_SeqRetButton( WO_WORK * wk )
{
  WO_TalkMsgSet( wk, MSG_END_CHECK );
  wk->ynidx = YESNO_END_CHECK;
  wk->next_seq = SEQ_YESNO_PUT;
  return SEQ_MSG_WAIT;
}


//----------------------------------------------------------------------------------
/**
 * @brief 技選択時の処理
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int WazaSelectEnter( WO_WORK * wk )
{
  if( WO_WazaSetCheck( wk ) < 4 ){
    WO_TalkMsgSet( wk, MSG_SET_CHECK );
    wk->ynidx = YESNO_SET_CHECK;
    wk->next_seq = SEQ_YESNO_PUT;
  }else{
    WO_TalkMsgSet( wk, MSG_CHG_CHECK );
    wk->ynidx = YESNO_CHG_CHECK;
    wk->next_seq = SEQ_YESNO_PUT;
  }
  return SEQ_MSG_WAIT;
}

//----------------------------------------------------------------------------------
/**
 * @brief はい・いいえ待ちの際に画面を暗くする
 *
 * @param flg   TRUE:暗く,FALSE：解除
 */
//----------------------------------------------------------------------------------
static void PassiveSet( BOOL flg )
{
  if( flg == TRUE ){
    G2S_SetBlendBrightness(
      GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 |
      GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, -8 );
  }else{
    G2S_BlendNone();
  }
}

