//==============================================================================
/**
 * @file  battle_input.c
 * @brief 戦闘入力画面処理
 * @author  matsuda changed by soga
 * @date  2009.04.13(月)
 */
//==============================================================================
#if 0
#include "common.h"
#include "graphic\pl_batt_obj_def.h"
#include "graphic\pl_batt_bg_def.h"
#include "system/arc_tool.h"
#include "system\arc_util.h"
#include "battle_input.h"
#include "gflib/touchpanel.h"
#include "battle/battle_common.h"
#include "system/clact_tool.h"
#include "system/fontproc.h"
#include "gflib/msg_print.h"
#include "application/app_tool.h"
#include "battle/fight_tool.h"
#include "battle_id.h"
#include "client_tool_def.h"
#include "system\msgdata.h"
#include "msgdata/msg_fightmsg_dp.h"
#include "system\pm_str.h"
#include "battle/battle_tcb_pri.h"
#include "battle/wazatype_icon.h"
#include "poketool/waza_tool.h"
#include "system/buflen.h"
#include "battle/wazatype_panel.h"
#include "system/msgdata_util.h"
#include "system/wordset.h"
#include "system/palanm.h"
#include "system/fontoam.h"
#include "poketool/pokeicon.h"
#include "battle/battle_cursor.h"
#include "battle/battle_snd_def.h"
#include "battle/client_tool.h"
#include "battle_input_type.h"
#include "system/pmfprint.h"
#include "battle/finger_cursor.h"
#include "battle/battle_server.h"
#include "system/window.h"
#include "battle/battle_cursor_disp.h"

#ifdef PM_DEBUG
#include "debug/d_fight.h"
#endif PM_DEBUG

#define HAIKEI_CHG  1 //1:変更後
#define PP_CHANGE 1
#endif

//MAKEを通すために
//インクルードするヘッダを限定
//背景IDは暫定で0
//BATTLE_WORKはコメントアウト
//FIGHT_TYPEはコメントアウト
//Snd_SePlayはコメントアウト
//はい／いいえボタンはコメントアウト
//録画再生時のやめるボタンはコメントアウト
//2vs2時の攻撃対象選択ボタンはコメントアウト
//ポケモンアイコンリソース読み込みをコメントアウト（関数の置き換えは済み）
//ポケモンアイコンアクター登録をコメントアウト（関数の置き換えは一部済み）
//CURSOR_SAVE構造体を暫定的に同ソース内で定義（本来は呼び出し側で保持しておく構造体のはず）
//捕獲デモはどうなるかわからないのでコメントアウト
//録画再生停止画面にシステムメッセージを表示させる処理をコメントアウト
//文字表示を暫定のものにしてある

//#define MESSAGE_PRINT_DUMMY //暫定文字表示

#include "gflib.h"
#include "battle/pl_batt_obj_def.h"
#include "battle/pl_batt_bg_def.h"
#include "poke_tool/poke_tool.h"
#include "battle_input.h"
//#include "system/fontproc.h"
//#include "application/app_tool.h"
//#include "battle/fight_tool.h"
//#include "battle_id.h"
//#include "client_tool_def.h"
//#include "msgdata/msg_fightmsg_dp.h"
//#include "system\pm_str.h"
#include "battle_tcb_pri.h"
#include "system/wazatype_icon.h"
#include "waza_tool/waza_tool.h"
//#include "system/buflen.h"
#include "system/palanm.h"
#include "wazatype_panel.h"
#include "print/wordset.h"
//#include "system/fontoam.h"
//#include "poketool/pokeicon.h"
#include "battle_cursor.h"
//#include "battle_snd_def.h"
//#include "battle/client_tool.h"
#include "battle/btl_common.h"
#include "battle_input_type.h"
//#include "system/pmfprint.h"
#include "finger_cursor.h"
//#include "system/window.h"
#include "battle_cursor_disp.h"
#include "system/bmp_oam.h"
#include "print/printsys.h"
#include "arc_def.h"
#include "system/sdkdef.h"
#include "system/gfl_use.h"

#include "btlv_mcss.h"
#include "battle/btl_string.h"

#include "message.naix"
#include "msg/msg_d_soga.h"

//==============================================================================
//  定数定義
//==============================================================================

//デバッグ用定義：１＝S_MSGを使用した正式な形。０＝S_MSG対応Expandが用意されるまでの暫定処理
#define S_MSG_DEBUG   (0)

//MAKEを通すために暫定で定義
///サブサーフェースY(fx32)
#define BATTLE_SUB_ACTOR_DISTANCE   ( 0 ) //((192 + 80) << FX32_SHIFT)
///サブサーフェースY(int)
#define BATTLE_SUB_ACTOR_DISTANCE_INTEGER   (BATTLE_SUB_ACTOR_DISTANCE >> FX32_SHIFT)

enum{
  SELECT_FIGHT_COMMAND,   //たたかう
  SELECT_ITEM_COMMAND,    //バッグ
  SELECT_POKEMON_COMMAND,   //ポケモン
  SELECT_ESCAPE_COMMAND,    //にげる
  SELECT_CANCEL,        //キャンセル
  SELECT_SKILL_1,
  SELECT_SKILL_2,
  SELECT_SKILL_3,
  SELECT_SKILL_4,
  SELECT_TARGET_A,    //ターゲットA
  SELECT_TARGET_B,    //ターゲットB
  SELECT_TARGET_C,    //ターゲットC
  SELECT_TARGET_D,    //ターゲットD

  WAZATYPEICON_OAMSIZE = 32 * 8,

  RANGE_NORMAL = 0,
  RANGE_NONE,
  RANGE_RANDOM,
  RANGE_MINE,
  RANGE_DOUBLE,
  RANGE_MAKIBISHI,
  RANGE_TRIPLE,
  RANGE_TEDASUKE,
  RANGE_FIELD,
  RANGE_TUBOWOTUKU,
  RANGE_MINE_DOUBLE,
  RANGE_SAKIDORI,

  CLIENT_TYPE_AA = BTLV_MCSS_POS_AA,
  CLIENT_TYPE_BB = BTLV_MCSS_POS_BB,
  CLIENT_TYPE_A = BTLV_MCSS_POS_A,
  CLIENT_TYPE_B = BTLV_MCSS_POS_B,
  CLIENT_TYPE_C = BTLV_MCSS_POS_C,
  CLIENT_TYPE_D = BTLV_MCSS_POS_D,

#if 0
  COMSEL_STATUS_NONE = 0,
  COMSEL_STATUS_ALIVE,
  COMSEL_STATUS_DEAD,
  COMSEL_STATUS_NG,
#endif

};

//--------------------------------------------------------------
//  ゲームパラメータ
//--------------------------------------------------------------
///背景フェードのEVY値
#define BACKFADE_EVY    (6)
///背景フェードのフェード後のカラー
#define BACKFADE_COLOR    (0x0000)
///背景フェードのフェードビット
#define BACKFADE_FADEBIT  (0x0001)
///背景フェードのフェードウェイト
#define BACKFADE_FADE_WAIT  (-2)

//--------------------------------------------------------------
//  やる気アニメ
//--------------------------------------------------------------
///やる気アニメのアニメ段階(パーセンテージ)
enum{
  MOTIVATION_ANM_HIGH = 94,   //16分の1 小数切捨て
  MOTIVATION_ANM_MIDDLE = 87,   //8分の1 小数切捨て
  MOTIVATION_ANM_LOW = 75,    //4分の1 小数切捨て
};

///やる気アニメタイプ
enum{
  MOTIVATION_ANM_TYPE_HIGH,
  MOTIVATION_ANM_TYPE_MIDDLE,
  MOTIVATION_ANM_TYPE_LOW,
};

///やる気アニメ：アニメ回転加算値
#define MOTIVATION_ADD_ROTATION   (0x0800)
///やる気アニメ：アニメ移動量X(下位8ビット小数)
#define MOTIVATION_ADD_X      (0x0180)
///やる気アニメ：アニメフレーム数(このフレーム分経過したらアニメ折り返し)
#define MOTIVATION_ANM_FRAME    (2)
///やる気アニメ：アニメ繰り返し回数
#define MOTIVATION_ANM_LOOP     (1)
///やる気アニメ：ウェイトに加算するランダム幅
#define MOTIVATION_ANM_WAIT_RANDOM  (8)

///やる気アニメウェイト
enum{
  MOTIVATION_WAIT_HIGH = 10,
  MOTIVATION_WAIT_MIDDLE = 60,
  MOTIVATION_WAIT_LOW = 150,
};

//--------------------------------------------------------------
//  ローカルTCBプライオリティ
//--------------------------------------------------------------
#define VWAIT_TCBPRI_FRAMEVISIBLE   (10)
#define VWAIT_TCBPRI_BACKSCRN_VISIBLE (10)
#define VWAIT_TCBPRI_CGRPARAM_TRANS   (20)


//--------------------------------------------------------------
//
//--------------------------------------------------------------
///BG指定なし
#define NONE_ID     (0xffff)

///パネルフレーム面
#define BI_FRAME_PANEL  ( GFL_BG_FRAME0_S )
///パネルBGNO面
#define BI_BGNO_PANEL (0)
///黒枠フレーム面
#define BI_FRAME_BF   ( GFL_BG_FRAME1_S )
///黒枠BGNO面
#define BI_BGNO_BF    (1)
///背景フレーム面
#define BI_FRAME_BACK ( GFL_BG_FRAME2_S )
///背景BGNO面
#define BI_BGNO_BACK  (2)
///エフェクトフレーム面(黒枠スライド用)
#define BI_FRAME_EFF  ( GFL_BG_FRAME3_S )
///エフェクトBGNO面(黒枠スライド用)
#define BI_BGNO_EFF   (3)

///半透明第1対象面
#define BLD_PLANE_1   (GX_BLEND_PLANEMASK_BG1)
///半透明第2対象面
#define BLD_PLANE_2   (GX_BLEND_BGALL)
///第1対象面に対するαブレンディング係数
#define BLD_ALPHA_1   8//(0x1b) //(9)
///第2対象面に対するαブレンディング係数
#define BLD_ALPHA_2   12//(4) //(8)

///BG使用面数
#define BI_BG_NUM   (4)

///標準パレットデータ使用本数
#define BASE_PLTT_NUM (7)

///makedata_no初期値(未使用状態)
#define MAKEDATA_NO_INIT  (-1)

///プレイヤーの壁紙サイズ(バイト単位)
#define BG_PLAYER_WALL_SIZE   (32 * 12 * 0x20)
///戦闘入力画面で使用出来るBGのCGRサイズ
#define BI_BG_CGR_SIZE      (0x6000 - BG_PLAYER_WALL_SIZE)
///プレイヤーの壁紙転送オフセット位置(キャラ単位)
#define BG_PLAYER_WALL_OFFSET (BI_BG_CGR_SIZE / 0x20)

///BGスクリーンのクリアコード
#define BG_CLEAR_CODE     (0x6000 / 0x20 - 1)

///フォントOAMデータのBMPキャラクタサイズX(最大文字数の技名基準)
#define FONTOAMDATA_BMP_X_SIZE    (7 * 14 / 8)

///選択出来ないパネルの絵のパレット番号
#define BI_NOT_SELECT_PANEL_PALNO   (0xe) //(6)

///フロンティア背景の時のアニメパレット追加位置
#define FRONTIER_PALANM_POS       (7)

///タッチした時の背景パレット変更カラー開始位置
#define BACKGROUND_CHANGE_PAL_START   (8) //(0xb)

///ポケモンアイコン指定なし
#define NOT_POKE_ICON     (0xff)

//--------------------------------------------------------------
//
//--------------------------------------------------------------
///スクリーンバッファ番号
enum{
  SCRNBUF_BACKGROUND,
  SCRNBUF_COMMAND,
  SCRNBUF_COMMAND_WAZA,
  SCRNBUF_WAZA,
  SCRNBUF_POKESELE,
  SCRNBUF_POKESELE_WAKU,
  SCRNBUF_YESNO,

  SCRNBUF_MAX,
};
///プラチナで追加：これ以上スクリーンバッファを常駐で持たないようにする為YESNOと共通にする
///録画再生画面ではこのスクリーン以外出ないので。
#define SCRNBUF_PLAYBACK_STOP (SCRNBUF_YESNO) //「ストップ」ボタン
#define SCRNBUF_PLAYBACK_SYSWIN (SCRNBUF_POKESELE)  //システムウィンドウ

///スクリーンバッファに対応したスクリーンデータのID
/// ※SCRNBUFと並びを同じにしておくこと！！
ALIGN4 static const u16 ScrnArcDataNo[] = {
  BATTLE_WBG0B_NSCR_BIN,    //SCRNBUF_BACKGROUND,
  BATTLE_WBG1A_NSCR_BIN,    //SCRNBUF_COMMAND
  BATTLE_WBG2A_NSCR_BIN,    //SCRNBUF_COMMAND_WAZA
  BATTLE_WBG1B_NSCR_BIN,    //SCRNBUF_WAZA
  BATTLE_WBG1C_NSCR_BIN,    //SCRNBUF_POKESELE
  BATTLE_WBG3A_NSCR_BIN,    //SCRNBUF_POKESELE_WAKU
  BATTLE_WBG1D_NSCR_BIN,    //SCRNBUF_YESNO
};

//--------------------------------------------------------------
//  スクリーン書き換えオフセット
//--------------------------------------------------------------
///「たたかう」コマンドのキャラネームに足しこむオフセット
#define SCRN_ADD_A_FIGHT  (4)
///「バッグ」コマンドのキャラネームに足しこむオフセット
#define SCRN_ADD_A_ITEM   (7)
///「ポケモン」コマンドのキャラネームに足しこむオフセット
#define SCRN_ADD_A_POKEMON  (7)
///「にげる」コマンドのキャラネームに足しこむオフセット
#define SCRN_ADD_A_ESCAPE (6)

///「技１～４」コマンドのキャラネームに足しこむオフセット
#define SCRN_ADD_B_WAZA   (7)
///技選択画面の「もどる」コマンドのキャラネームに足しこむオフセット
#define SCRN_ADD_B_MODORU (6)

///たたかいをやめますか？「はい」コマンドのキャラネームに足しこむオフセット
#define SCRN_ADD_ESCAPE_YES (3)
///たたかいをやめますか？「はい」コマンドのキャラネームに足しこむオフセット
#define SCRN_ADD_ESCAPE_NO  (3)

//--------------------------------------------------------------
//  スクリーンスクロール値
//--------------------------------------------------------------
///TYPE_AのBG表示時にタッチパネルが押された時にY方向にスクロールさせる値(エフェクト面)
#define SCRN_SCROLL_A_EFF_Y   (-2)
///
#define SCRN_SCROLL_BACK_A_EFF_Y  (1)

///枠タイプ
enum{
  WAKU_TYPE_A,
  WAKU_TYPE_B,
  WAKU_TYPE_C,

  WAKU_TYPE_NONE,   ///<黒枠なし
};

#define WAKU_SP_INIT    (0.09f)
#define WAKU_SP_SUB     (0.025f)
#define WAKU_EFF_FRAME    (6)

//--------------------------------------------------------------
//  連結スクリーン
//--------------------------------------------------------------
///パネル連結BGのスクリーンコード
#define JOINT_SCRN_CODE   (0x02a4)

//--------------------------------------------------------------
//  タッチフェード
//--------------------------------------------------------------
///タッチした時のボタンフェードEVY
#define TOUCH_FADE_EVY    (10 << 8)
///タッチした時のボタンフェードEVY加算値
#define TOUCH_FADE_ADD_EVY  (TOUCH_FADE_EVY / 2)

//--------------------------------------------------------------
//  フォントOBJ
//--------------------------------------------------------------
///フォントOBJの文字間隔X
#define PANEL_MSG_MARGIN    (0)
///フォントOBJの最大使用OAM数
#define PANEL_MSG_MAXOAM    (8)
///フォントOBJのアクターソフトプライオリティ
#define PANEL_MSG_SOFTPRI   (100)
///フォントOBJのBGプライオリティ
#define PANEL_MSG_BGPRI     (0)

enum{
  FONTOAM_LEFT,   ///<X左端座標
  FONTOAM_CENTER,   ///<X中心座標
};

///フォントOBJアクターワークで使用する添え字の位置
enum{
  //画面A
  FA_NO_FIGHT = 0,    ///<たたかう
  FA_NO_ITEM,
  FA_NO_POKEMON,
  FA_NO_ESCAPE,

  //画面B
  FA_NO_WAZA_1 = 0,
  FA_NO_WAZA_2,
  FA_NO_WAZA_3,
  FA_NO_WAZA_4,
  FA_NO_WAZA_MODORU,
  FA_NO_PPMSG_1,
  FA_NO_PPMSG_2,
  FA_NO_PPMSG_3,
  FA_NO_PPMSG_4,
  FA_NO_PP_1,
  FA_NO_PP_2,
  FA_NO_PP_3,
  FA_NO_PP_4,

  //画面C
  FA_NO_NAME_A = 0,
  FA_NO_NAME_B,
  FA_NO_NAME_C,
  FA_NO_NAME_D,
  FA_NO_NAME_MODORU,

  //画面D
  FA_NO_ESCAPE_YES = 0,
  FA_NO_ESCAPE_NO,

  //ポケモン選択
  FA_NO_POKE_A = 0,
  FA_NO_POKE_B,
  FA_NO_POKE_C,
  FA_NO_POKE_D,
  FA_NO_POKE_CANCEL,

  //BINPUT_TYPE_PLAYBACK_STOP
  FA_NO_PLAYBACK_STOP = 0,
  FA_NO_PLAYBACK_SYSTEM_MSG_0,  //1行目
  FA_NO_PLAYBACK_SYSTEM_MSG_1,  //2行目


  FA_NO_MAX = FA_NO_PP_4 + 1,     ///<フォントOBJの管理数（BmpOam_ActorAdd出来る数）
  FA_OBJ_MAX = FA_NO_MAX * 2,     ///<フォントOBJの最大数（BmpOamが表示できるOBJ数）

};

///フォントOBJの表示座標
enum{
  //-- 画面A --//
  FA_POS_FIGHT_X = 0x10 * 8,    //Xは中心座標
  FA_POS_FIGHT_Y = 0xa * 8 + 4,
  FA_POS_ITEM_X = 5 * 8,
  FA_POS_ITEM_Y = 0x15 * 8 + 2,
  FA_POS_POKEMON_X = 0x1b * 8,
  FA_POS_POKEMON_Y = FA_POS_ITEM_Y,
  FA_POS_ESCAPE_X = 0x10 * 8,
  FA_POS_ESCAPE_Y = FA_POS_ITEM_Y + 8,

  //-- 画面B --//
  FA_POS_WAZA_1_X = 8 * 8,  //Xは中心座標
  FA_POS_WAZA_1_Y = 6 * 8-2,
  FA_POS_WAZA_2_X = 0x18 * 8,
  FA_POS_WAZA_2_Y = FA_POS_WAZA_1_Y,
  FA_POS_WAZA_3_X = FA_POS_WAZA_1_X,
  FA_POS_WAZA_3_Y = FA_POS_WAZA_1_Y + 8*8,
  FA_POS_WAZA_4_X = FA_POS_WAZA_2_X,
  FA_POS_WAZA_4_Y = FA_POS_WAZA_3_Y,
  FA_POS_WAZA_MODORU_X = 0x10 * 8,
  FA_POS_WAZA_MODORU_Y = FA_POS_ESCAPE_Y,

  //-- 画面D --//
  FA_POS_ESCAPE_YES_X = 0x10 * 8,     //Xは中心座標
  FA_POS_ESCAPE_YES_Y = 0x8 * 8 + 4,
  FA_POS_ESCAPE_NO_X = FA_POS_ESCAPE_YES_X,
  FA_POS_ESCAPE_NO_Y = 0x11 * 8 + 4,

  //-- 画面BINPUT_TYPE_PLAYBACK_STOP --//
  FA_POS_STOP_X = 0x10 * 8,     //Xは中心座標
  FA_POS_STOP_Y = FA_POS_ESCAPE_Y,
};

//技タイプアイコンの表示座標
ALIGN4 static const u16 WazaIconPos[][2] = {  //0:X, 1:Y
  {4*8,       FA_POS_WAZA_1_Y + 16},
  {0x14*8,      FA_POS_WAZA_2_Y + 16},
  {4*8,       FA_POS_WAZA_3_Y + 16},
  {0x14*8,      FA_POS_WAZA_4_Y + 16},
};

//「PP」の表示座標
ALIGN4 static const u16 PPMsgPos[][2] = { //0:X, 1:Y
  {3*8 + 28-1,    FA_POS_WAZA_1_Y + 16}, //{4*8 + 28-1,     FA_POS_WAZA_1_Y + 16},
  {0x13*8 + 28-1, FA_POS_WAZA_2_Y + 16}, //{0x14*8 + 28-1,  FA_POS_WAZA_2_Y + 16},
  {3*8 + 28-1,    FA_POS_WAZA_3_Y + 16}, //{4*8 + 28-1,     FA_POS_WAZA_3_Y + 16},
  {0x13*8 + 28-1, FA_POS_WAZA_4_Y + 16}, //{0x14*8 + 28-1,  FA_POS_WAZA_4_Y + 16},
};
//現在PP(数値)の表示座標
ALIGN4 static const u16 NowPPPos[][2] = { //0:X, 1:Y
  {3*8 + 44,    FA_POS_WAZA_1_Y + 16}, //{4*8 + 44,     FA_POS_WAZA_1_Y + 16},
  {0x13*8 + 44,   FA_POS_WAZA_2_Y + 16}, //{0x14*8 + 44,    FA_POS_WAZA_2_Y + 16},
  {3*8 + 44,    FA_POS_WAZA_3_Y + 16}, //{4*8 + 44,     FA_POS_WAZA_3_Y + 16},
  {0x13*8 + 44,   FA_POS_WAZA_4_Y + 16}, //{0x14*8 + 44,    FA_POS_WAZA_4_Y + 16},
};

//技分類の表示座標(X：左端座標)
ALIGN4 static const u16 WazaClassPos[][2] = { //0:X, 1:Y
  {11*8, 8*8 - 16,},
  {0x1b*8, 8*8 - 16,},
  {11*8, 0x11*8 - 16,},
  {0x1b*8, 0x11*8 - 16,},
};

//ポケモン選択(技効果範囲)の表示座標
ALIGN4 static const u16 PokeSelePos[][2] = {  //0:X, 1:Y
  {7*8 + 4, 0xe*8 + 4},     //A
  {0x18*8 + 4, 4*8},        //B
  {0x18*8 + 4, 0xe*8 + 4},    //C
  {7*8 + 4, 4*8},         //D
  {0x10*8, FA_POS_WAZA_MODORU_Y}, //もどる
};

//ポケモンアイコンの表示座標
ALIGN4 static const u16 PokeIconPos[][2] = {  //0:X, 1:Y
  {7*8 + 4, 0xe*8 + 4},     //A
  {0x18*8 + 4, 7*8},        //B
  {0x18*8 + 4, 0xe*8 + 4},    //C
  {7*8 + 4, 7*8},         //D
};

//--------------------------------------------------------------
//  パレット関連定義
//--------------------------------------------------------------
///フォントOBJのパレット番号オフセット
enum{
  PALOFS_FIGHT = 2,
  PALOFS_ITEM = 2,
  PALOFS_POKEMON = 2,
  PALOFS_ESCAPE = 2,

  PALOFS_WAZA = 2,  //3,
  PALOFS_PP = 2,    //3,
  PALOFS_WAZA_MODORU = 2,

  PALOFS_SYSMSG = 3,
  PALOFS_ESCAPE_YES = 2,
  PALOFS_ESCAPE_NO = 2,

  PALOFS_CLASS = 3,

  PALOFS_POKESELE = 6,
  PALOFS_POKESELE_MODORU = PALOFS_WAZA_MODORU,
};

#define MSGCOLOR_FIGHT      (PRINTSYS_LSB_Make(1, 2, 3))
#define MSGCOLOR_ITEM     (PRINTSYS_LSB_Make(4, 5, 6))
#define MSGCOLOR_POKEMON    (PRINTSYS_LSB_Make(7, 8, 9))
#define MSGCOLOR_ESCAPE     (PRINTSYS_LSB_Make(10, 11, 12))

#define MSGCOLOR_WAZA     (PRINTSYS_LSB_Make(14, 14, 0))  //#define MSGCOLOR_WAZA     (PRINTSYS_LSB_Make(7, 8, 9))
#define MSGCOLOR_PP_BLACK   (PRINTSYS_LSB_Make(14, 14, 0))  //FONT_SYSTEMなので背景抜き色指定 (PRINTSYS_LSB_Make(1, 2, 0))
#define MSGCOLOR_PP_YELLOW    (PRINTSYS_LSB_Make(14, 14, 0))  //FONT_SYSTEMなので背景抜き色指定 (PRINTSYS_LSB_Make(3, 4, 0))
#define MSGCOLOR_PP_ORANGE    (PRINTSYS_LSB_Make(14, 14, 0))  //FONT_SYSTEMなので背景抜き色指定 (PRINTSYS_LSB_Make(5, 6, 0))
#define MSGCOLOR_PP_RED     (PRINTSYS_LSB_Make(14, 14, 0))  //FONT_SYSTEMなので背景抜き色指定 (PRINTSYS_LSB_Make(7, 8, 0))
#define MSGCOLOR_WAZA_MODORU  (PRINTSYS_LSB_Make(10, 11, 12))

#define MSGCOLOR_SYSMSG     (PRINTSYS_LSB_Make(9, 8, 0))//FONT_SYSTEMなので背景抜き色指定
#define MSGCOLOR_ESCAPE_YES   (PRINTSYS_LSB_Make(1, 2, 3))
#define MSGCOLOR_ESCAPE_NO    (PRINTSYS_LSB_Make(10, 11, 12))

#define MSGCOLOR_CLASS      (PRINTSYS_LSB_Make(7, 8, 0))//FONT_SYSTEMなので背景抜き色指定

#define MSGCOLOR_POKESELE_ENEMY (PRINTSYS_LSB_Make(1, 2, 3))
#define MSGCOLOR_POKESELE_MINE  (PRINTSYS_LSB_Make(4, 5, 6))
#define MSGCOLOR_POKESELE_MODORU  (MSGCOLOR_WAZA_MODORU)

//--------------------------------------------------------------
//  エフェクトTCB
//--------------------------------------------------------------
///パネルをタッチした時のタッチアニメウェイト
#define SCRN_TOUCH_ANM_WAIT   0//(1)
///パネルをタッチした後、エフェクトで待つフレーム数
#define SCRN_TOUCH_WAIT     0//(1)  //(2)


//--------------------------------------------------------------
//  手持ちボール
//--------------------------------------------------------------
///手持ちボール(自機側)のアクターソフトプライオリティ
#define SOFTPRI_STOCK_MINE    (10)
///手持ちボール(敵側の)のアクターソフトプライオリティ
#define SOFTPRI_STOCK_ENEMY   (11)

///手持ちボール(自機側)の左端の手持ちボールX座標
#define STOCK_MINE_BASE_POS_X   (12)
///手持ちボール(自機側)の左端の手持ちボールY座標
#define STOCK_MINE_BASE_POS_Y   (13)
///手持ちボール(自機側)の連続でボールを配置する時の間隔X幅
#define STOCK_MINE_BASE_SPACE_X   (19)

///手持ちボール(敵側)の左端の手持ちボールX座標
#define STOCK_ENEMY_BASE_POS_X    (246)
///手持ちボール(敵側)の左端の手持ちボールY座標
#define STOCK_ENEMY_BASE_POS_Y    (9)
///手持ちボール(敵側)の連続でボールを配置する時の間隔X幅
#define STOCK_ENEMY_BASE_SPACE_X  (-12)

///戦闘カーソルのアクターソフトプライオリティ
#define SOFTPRI_CURSOR        (5)
///戦闘カーソルのBGプライオリティ
#define BGPRI_CURSOR        (0)

//--------------------------------------------------------------
//  技タイプ
//--------------------------------------------------------------
///技タイプのパレット開始位置
#define WAZATYPE_START_PALPOS (8)

//--------------------------------------------------------------
//  コマンド選択画面：スライドインエフェクト
//--------------------------------------------------------------
///コマンド選択画面INエフェクト：ウィンドウ0の初期座標X
#define COMMANDIN_WND_START_X0    (0)
///コマンド選択画面INエフェクト：ウィンドウ0の初期座標X
#define COMMANDIN_WND_END_X0    (255)
///コマンド選択画面INエフェクト：ウィンドウ1の初期座標X
#define COMMANDIN_WND_START_X1    (0)
///コマンド選択画面INエフェクト：ウィンドウ1の初期座標X
#define COMMANDIN_WND_END_X1    (255)
///コマンド選択画面INエフェクト：ウィンドウ0の初期座標Y
#define COMMANDIN_WND_START_Y0    (0)
///コマンド選択画面INエフェクト：ウィンドウ0の初期座標Y
#define COMMANDIN_WND_END_Y0    (18 * 8)
///コマンド選択画面INエフェクト：ウィンドウ1の初期座標Y
#define COMMANDIN_WND_START_Y1    (COMMANDIN_WND_END_Y0)
///コマンド選択画面INエフェクト：ウィンドウ1の初期座標Y
#define COMMANDIN_WND_END_Y1    (192)

#define COMMANDIN_SCR_X0_START    (255 * 100)
#define COMMANDIN_SCR_Y1_START    (40 * 100)
///コマンドインスライドエフェクトにかけるフレーム数
#define COMMANDIN_EFF_FRAME     (4) //(8)
#define COMMANDIN_SCR_X0_ADD    (COMMANDIN_SCR_X0_START / COMMANDIN_EFF_FRAME)
#define COMMANDIN_SCR_Y1_ADD    (COMMANDIN_SCR_Y1_START / COMMANDIN_EFF_FRAME)

//--------------------------------------------------------------
//  捕獲デモ
//--------------------------------------------------------------
///捕獲デモ用指カーソル：ソフトプライオリティ
#define FINGER_SOFTPRI    (10)
///捕獲デモ用指カーソル：BGプライオリティ
#define FINGER_BGPRI    (0)
///捕獲デモ用指カーソル：タッチするまでのウェイト
#define FINGER_TOUCH_WAIT (60)


//==============================================================================
//  構造体定義
//==============================================================================
///コマンド選択画面：スクロールインエフェクトワーク
typedef struct{
  BI_PARAM_PTR bip; ///<BIシステムワークへのポインタ
  GFL_TCB *vtask_tcb; ///<Vブランク動作タスクのポインタ
  GFL_TCB *htask_tcb; ///<Hブランク動作タスクのポインタ

  s16 x0_l;   ///<100の除算が整数
  s16 y1_d;
  s16 set_x0_l;
  s16 set_y1_d;
  s16 h_set_y1_d;

  u8 client_type;
  u8 seq;
}COMMAND_IN_EFF_WORK;

typedef struct{
  s16 x;
  s16 y;
}POINT_S16;

///スクリーン書き換え時の範囲指定構造体
typedef struct{
  u8 top;
  u8 bottom;
  u8 left;
  u8 right;
}REWRITE_SCRN_RECT;

///CGR転送アニメ実行時の範囲データ
typedef struct{
  u16 dest_x;     ///<転送先X開始位置(キャラ単位)
  u16 dest_y;     ///<転送先Y開始位置(キャラ単位)
  u16 src_x;      ///<転送元X開始位置(キャラ単位)
  u16 src_y;      ///<転送元Y開始位置(キャラ単位)
  u16 size_x;     ///<X転送サイズ(キャラ単位)
  u16 size_y;     ///<Y転送サイズ(キャラ単位)
}CGR_TRANS_RECTDATA;

///CGR転送アニメ実行時のパラメータ
typedef struct{
  GFL_TCB *v_tcb;   ///<Vブランク転送を実行するTCBのポインタ
  u8 *dest_vram;    ///<VRAM転送先アドレス
  void *arc_data;   ///<アーカイブしたキャラファイルのポインタ
  u8 *raw_data;   ///<アーカイブしたキャラファイルのキャラデータ先頭アドレス
  CGR_TRANS_RECTDATA rd;  ///<CGR転送アニメ実行時の範囲データ
  u16 x_len;      ///<キャラファイルのX幅(バイト単位)
}CGR_TRANS_PARAM;

///ボタンのアニメパターン数
#define BUTTON_ANM_NUM      (2)

///エフェクトTCB用のワーク
typedef struct{
  s16 seq;
  s16 wait;
  union{
    struct{   //汎用
      u8 fa_no;           ///<対象フォントOBJアクターワークの添え字
    }para;
    struct{   //技選択用
      int tp_ret;
    }waza;
    struct{   //CGR転送用(汎用)
//      CGR_TRANS_PARAM *ctp;
//      const CGR_TRANS_RECTDATA *ctp_rd_ptr[BUTTON_ANM_NUM];
//      u32 transcgr_id[BUTTON_ANM_NUM];
      const s16 *scrn_offset;
      const REWRITE_SCRN_RECT *scrn_range;
      int tp_ret;
      u8 scrnbuf_no;
      u8 fa_no;           ///<対象フォントOBJアクターワークの添え字
      u8 pokeicon_no;         ///<対象ポケアイコンアクターワークの添え字
      u8 waku_type;
      u8 obj_del;
      POINT_S16 waku_pos;
    }paracgr;
    struct{
      CGR_TRANS_PARAM *ctp[ BTL_CLIENT_MAX ];
      CGR_TRANS_PARAM *purple_ctp;
      NNSG2dCharacterData *cgr_char_data;
      void *cgr_arc_data;
      int tp_ret;
    }pokesele;
    s16 work[2];
  };
}BI_EFFECT_WORK;

///カラーエフェクトTCB用のワーク
typedef struct{
  int tp_ret;
  s16 evy;    ///<EVY値(下位8ビット小数)
  s16 evy_add;  ///<EVY加減算値(下位8ビット小数)
  u8 seq;
  u8 pal_pos; ///<フェード対象のパレット番号
}BI_COLOR_EFF_WORK;

///エフェクトBG書き換えパラメータ
typedef struct{
  const REWRITE_SCRN_RECT *rsr; ///<スクリーン書き換え範囲データへのポインタ
  u8 rsr_num;           ///<rsrデータ個数
  s8 add_charname;        ///<キャラクタネームに足しこむオフセット値
}EFFBG_WRITE_PARAM;

///フォントアクターワーク
typedef struct{
  BMPOAM_ACT_PTR      fontoam;
//  CHAR_MANAGER_ALLOCDATA  cma;
  u16 font_len;
}FONT_ACTOR;

///やる気アニメ用ワーク
typedef struct{
  s16 x_offset;     ///<X座標オフセット(下位8ビット小数)
  u8 motivation;      ///<現在のモチベーション(0～100)
  u8 seq;
  u8 anm_type;
  u8 wait;
  u8 count;       ///<移動フレームをカウント
  u8 count_max;     ///<移動フレーム最大数(この分移動したら折り返し)
  u8 loop;        ///<アニメ繰り返し回数
}MOTIVATION_WORK;

///カーソル移動用ワーク
typedef struct{
  u8 cursor_on;     ///<TRUE：カーソル表示状態
  s8 y_menu;        ///<階層メニューの位置(Y)
  s8 x_menu;        ///<水平メニューの位置(X)

  u8 dummy;
}CURSOR_MOVE;

///シーンワーク
typedef union{
  BINPUT_SCENE_COMMAND bsc;
  BINPUT_SCENE_WAZA bsw;
  BINPUT_SCENE_POKE bsp;
  BINPUT_SCENE_YESNO bsy;
}SCENE_WORK;

///フォントOAM作成時、外側でBMP指定する場合に使用
typedef struct{
  GFL_BMP_DATA *bmp;
  u16 char_len;
  u16 font_len;
}FONT_EX_BMP;

///描画処理高速化の為、あらかじめデータを作成しておく為のワーク
typedef struct{
  BINPUT_WAZA_PARAM wazapara;       ///<データ比較用に現在のパラメータ保存

  //技タイプアイコン：キャラデータ展開メモリ
  u16 *typeicon_cgx[ PTL_WAZA_MAX ];    ///<技タイプアイコン：キャラデータ展開メモリ

  //フォントOAMに関連付けるBMPWIN
  FONT_EX_BMP exbmp_waza[ PTL_WAZA_MAX ];
  FONT_EX_BMP exbmp_pp[ PTL_WAZA_MAX ];
  FONT_EX_BMP exbmp_ppmax[ PTL_WAZA_MAX ];
}MEMORY_DECORD_WORK;

///戦闘入力画面カーソル位置記憶（本来は、battle_inputがデリートされても保持されていなければならない構造体
typedef struct{
  s8 command_x;
  s8 command_y;

  s8 waza_x;
  s8 waza_y;

  s8 poke_x;
  s8 poke_y;
  s8 poke_save_type;

  u8 dummy[1];
}CURSOR_SAVE;

///戦闘入力画面制御ワーク
typedef struct _BI_PARAM{
//  BATTLE_WORK *bw;
  BATTLE_CURSOR_DISP *cursor_disp;
  GFL_TCBSYS  *tcbsys;
  GFL_TCB *ball_tcb;    ///<ボールやる気アニメTCBへのポインタ
  GFL_TCB *effect_tcb;  ///<エフェクト動作TCBへのポインタ
  GFL_TCB *color_eff_tcb; ///<カラーエフェクト動作TCBへのポインタ
  EFFBG_WRITE_PARAM ewp;  ///<エフェクトBG書き換えパラメータ

  //シーンワーク
  SCENE_WORK scene; ///<シーン毎に異なる必要なデータ類

  //スクリーンバッファ
  u16 *scrn_buf[SCRNBUF_MAX]; ///<スクリーンバッファ
  //パレットバッファ
  u16 *pal_buf;       ///<パレットバッファ

  u16 *background_pal_normal; ///<通常時の背景用パレット
  u16 *background_pal_touch;  ///<タッチしている時の背景用パレット
  GFL_TCB *background_tcb;    ///<背景パレット切り替え監視タスクへのポインタ

  MEMORY_DECORD_WORK memory_decord[ BTL_CLIENT_MAX ]; ///<高速化の為、あらかじめデータを作成しておく

  //フォントOAM
  GFL_CLUNIT  *fontclunit;
  BMPOAM_SYS_PTR fontoam_sys; ///<フォントシステムへのポインタ
  FONT_ACTOR font_actor[FA_NO_MAX]; ///<フォントアクターワーク
  BMPOAM_ACT_DATA *font_oamdata;

  //手持ちボール
  GFL_CLUNIT  *stock_clunit;
  GFL_CLWK *stock_mine_cap[ BTL_PARTY_MEMBER_MAX ]; ///<自機側手持ちボールアクターポインタ
  GFL_CLWK *stock_enemy_cap[ BTL_PARTY_MEMBER_MAX ];  ///<敵側手持ちボールアクターポインタ

  //技タイプアイコン
  GFL_CLWK *icon_cap[ PTL_WAZA_MAX ];         ///<技タイプアイコンアクターポインタ
  //技分類アイコン
  GFL_CLWK *kindicon_cap[ PTL_WAZA_MAX ];       ///<技分類アイコンアクターポインタ
  //ポケモンアイコン
  GFL_CLWK *pokeicon_cap[ BTL_CLIENT_MAX ];     ///<ポケモンアイコンアクターポインタ
  GFL_TCB  *pokeicon_tcb[ BTL_CLIENT_MAX ];     ///<ポケモンアイコンアクターアニメ更新タスク

  //やる気アニメ
  MOTIVATION_WORK motivation_work[ BTL_PARTY_MEMBER_MAX ];  ///<やる気アニメ用ワーク

  //背景フェード
  GFL_TCB *backfade_tcb;      ///<背景フェード動作タスクへのポインタ
  u8 backfade_seq;        ///<背景フェード動作タスクのシーケンス番号
  u8 backfade_flag;       ///<背景フェードの現状のフェード状態

  u8 client_type;   ///<クライアントタイプ
  s8 makedata_no;   ///<現在使用しているBgMakeDataの番号
  u8 pokesele_type; ///<ポケモン選択の時の範囲タイプ
  u8 sex;       ///<背景を出す時の性別
  u8 touch_invalid; ///<TRUE:タッチ処理無効
  u8 command_modoru_type;   ///<TRUE:コマンド選択画面の「にげる」が「もどる」になっている

  //常駐フェード
  u8 def_fade_dir;    ///<0:＋方向へのフェード、1:－方向へのフェード
  s16 def_fade_evy;   ///<常駐フェードのEVY値(下位8ビット小数)
  u8 waku_fade_dir;   ///<枠用、0:＋方向へのフェード、1:－方向へのフェード
  s16 waku_fade_evy;    ///<枠用、常駐フェードのEVY値(下位8ビット小数)
  GFL_TCB *def_fade_tcb;  ///<常駐フェード実行タスクへのポインタ

  BI_EFFECT_WORK effect_work; ///<エフェクトTCB用ワーク
  BI_COLOR_EFF_WORK color_work; ///<カラーエフェクトTCB用ワーク

  s32 wall_x;         ///<背景スクロール座標X(下位8ビット小数)
  s32 wall_sp_x;        ///<背景スクロール速度X(下位8ビット小数)
  s32 wall_end_x;       ///<背景スクロール座標X(下位8ビット小数)

  BCURSOR_PTR cursor;     ///<カーソルワークへのポインタ
  BCURSOR_RES_PTR cursor_res; ///<カーソルリソースへのポインタ
  CURSOR_MOVE cursor_move;  ///<カーソル移動用ワーク
  u8 decend_key;        ///<TRUE:決定ボタンを押したのがカーソル
  u8 cancel_escape;     ///<コマンド選択で「にげる」「もどる」のどちらを出すか
  //捕獲デモ用ワーク
  struct{
    FINGER_PTR finger;
    u8 main_seq;
    u8 sub_seq;
    u8 sub_wait;
  }demo;

  //ここ以下は、WBに移植したときに追加（MAKEを通すため）
  PALETTE_FADE_PTR pfd;
  GFL_MSGDATA *msg;
  GFL_FONT    *font;

  HEAPID  heapID;

  //常駐パレット
  u32   objplttID;

  //技タイプアイコン
  GFL_CLUNIT  *wazatype_clunit;
  u32   wazatype_charID[ PTL_WAZA_MAX ];
  u32   wazatype_plttID;
  u32   wazatype_cellID;

  //ストックボールリソース
  u32   sb_charID[ 2 ];
  u32   sb_plttID[ 2 ];
  u32   sb_cellID[ 2 ];

  //ポケモンアイコン
  GFL_CLUNIT  *pokeicon_clunit;
  u32   pokeicon_charID[ BTL_CLIENT_MAX ];
  u32   pokeicon_plttID;
  u32   pokeicon_cellID;

///戦闘入力画面カーソル位置記憶（本来は、battle_inputがデリートされても保持されていなければならない構造体
  CURSOR_SAVE cursor_save;

}BI_PARAM;

///BG作成データ構造体
typedef struct{
  u16 cgr_id;     ///<使用CGRのアーカイブID
  u16 pal_id;     ///<使用パレットのアーカイブID
  union{
    struct{
      u16 scr0_id;
      u16 scr1_id;
      u16 scr2_id;
      u16 scr3_id;
    };
    u16 scr_id[BI_BG_NUM];    ///<使用スクリーンのアーカイブID
  };
  union{
    struct{
      u16 pri0;   ///<BG0面のプライオリティ
      u16 pri1;   ///<BG1面のプライオリティ
      u16 pri2;
      u16 pri3;
    };
    u16 pri[BI_BG_NUM];   ///<BGのプライオリティ
  };

  //タッチパネル
  const GFL_UI_TP_HITTBL *tpd;  ///<使用するタッチパネルデータへのポインタ
  const int *tpd_ret;       ///<タッチパネルの結果
  const u8 *tpd_pal;        ///<タッチした部分のパネルのパレット番号データへのポインタ

  //カーソル移動
  int (*cursor_move_func)(BI_PARAM_PTR, int); ///<キー入力をした時のコールバック関数
  void (*cursor_save_func)(BI_PARAM_PTR, int);  ///<キー位置記憶のコールバック関数

  //関数ポインタ
  void (*callback_bg)(BI_PARAM_PTR, int, int);  ///<BGCreate時に呼び出す関数
  int (*callback_tp)(BI_PARAM_PTR, int, int);     ///<タッチパネル反応時に呼び出す関数
}BG_MAKE_DATA;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void * BINPUT_WorkInit( HEAPID heapID );
static void BGCallback_CommandIn(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_CommandSelect(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_CommandInFightOnly(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_FightOnly(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_ParkCommandIn(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_ParkCommand(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_PlayBackStop(BI_PARAM_PTR bip, int select_bg, int force_put);
static int TPCallback_A(BI_PARAM_PTR bip, int tp_ret, int panel_pal);
static int TPCallback_Waza(BI_PARAM_PTR bip, int tp_ret, int panel_pal);
static void BGCallback_Waza(BI_PARAM_PTR bip, int select_bg, int force_put);
static void Sub_ScrnAddOffset(const REWRITE_SCRN_RECT rsr[], int rsr_num, int frame_no, int add_charname);
static void Sub_ScrnCopy(const REWRITE_SCRN_RECT rsr[], int rsr_num, int src_frame, int dest_frame);
static void Sub_ScrnErase(const REWRITE_SCRN_RECT rsr[], int rsr_num, int frame_no, u16 clear_code);
static void Sub_ScrnTouchChange(const REWRITE_SCRN_RECT rsr[], int rsr_num, int add_charname);
static void Sub_ScrnTouchChangeReverse(const REWRITE_SCRN_RECT rsr[], int rsr_num, int add_charname);
static void Sub_WazaTypeIconCreateAll(BI_PARAM_PTR bip);
static void Sub_WazaTypeIconDeleteAll(BI_PARAM_PTR bip);
static void Sub_WazaKindIconCreateAll(BI_PARAM_PTR bip);
static void Sub_WazaKindIconDeleteAll(BI_PARAM_PTR bip);
static void EffectTCB_Add(GFL_TCB_FUNC func, BI_PARAM_PTR bip);
static void EffectTCB_Delete(BI_PARAM_PTR bip);
static void Effect_ScrnTouch(GFL_TCB *tcb, void *work);
static void Effect_WazaScrnTouch(GFL_TCB *tcb, void *work);
static int TPCallback_D(BI_PARAM_PTR bip, int tp_ret, int panel_pal);
static int TPCallback_PlayBackStop(BI_PARAM_PTR bip, int tp_ret, int panel_pal);
static int TPCallback_PokeSele(BI_PARAM_PTR bip, int tp_ret, int panel_pal);
static void BGCallback_YesNo(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_Wasureru(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_Akirameru(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_NextPokemon(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_ChangePokemon(BI_PARAM_PTR bip, int select_bg, int force_put);
static void BGCallback_PokeSelect(BI_PARAM_PTR bip, int select_bg, int force_put);
static void Sub_WazaTypeCGRTrans(BI_PARAM_PTR bip, int waza_type, int waza_pos);
static void Sub_WazaTypeNotTemotiChainCGRTrans(BI_PARAM_PTR bip, int waza_pos);
static void Sub_PokemonNotPanelTrans(BI_PARAM_PTR bip, int poke_pos);
static int Sub_StockBallAnmSeqGet(u8 status);
static void BINPUT_StockBallActorResourceFree(BI_PARAM_PTR bip);
void BINPUT_StockBallAnimeUpdate(BI_PARAM_PTR bip, u8 stock_mine[], u8 stock_enemy[]);
void BINPUT_StockBallON(BI_PARAM_PTR bip);
void BINPUT_StockBallOFF(BI_PARAM_PTR bip);
static void VWait_FrameVisibleUpdate(GFL_TCB *tcb, void *work);
static void Sub_BackScrnOnlyVisible(void);
static void VWait_BackScrnOnlyVisible(GFL_TCB *tcb, void *work);
//static void Sub_TouchEndDelete(BI_PARAM_PTR bip, int obj, int bg);
static void Sub_SceneOBJDelete(BI_PARAM_PTR bip);
static void Effect_ButtonDown(GFL_TCB *tcb, void *work);
static CGR_TRANS_PARAM * Sub_CgrTransTaskSet(
  int frame_no, u32 cgr_id, const CGR_TRANS_RECTDATA *rd, NNSG2dCharacterData **loaded_char);
static BOOL Sub_CgrTransEndCheck(CGR_TRANS_PARAM *ctp);
static void VWait_CgrParamTrans(GFL_TCB *tcb, void *work);
static void Effect_WazaButtonDown(GFL_TCB *tcb, void *work);
static void Effect_PokeSeleButtonDown(GFL_TCB *tcb, void *work);
static void Sub_PokeSelectPanelConnectRange(BI_PARAM_PTR bip, int target, u8 *connect_range,
  int check_exist);
static void Sub_PokeSelectHitRange(BI_PARAM_PTR bip, u8 *hit_range, int check_exist);
static void Sub_CgrParamTrans(CGR_TRANS_PARAM *ctp);
static void ColorEffTCB_Add(GFL_TCB_FUNC func, BI_PARAM_PTR bip);
static void ColorEffTCB_Delete(BI_PARAM_PTR bip);
static void ColorEffTCB_PointFadeSet(BI_PARAM_PTR bip, int pal_pos);
static void ColorEff_PointFade(GFL_TCB *tcb, void *work);
void BINPUT_PlayerBG_SetX(BI_PARAM_PTR bip, int x);
void BINPUT_PlayerBG_ScrollX(BI_PARAM_PTR bip, int sp_x, int end_x);
static void PlayerBGScrollX(GFL_TCB *tcb, void *work);
static void ColorEffTCB_PokeSeleFadeSet(BI_PARAM_PTR bip, int tp_ret);
static void ColorEff_PokeSeleFade(GFL_TCB *tcb, void *work);
static void Sub_FontOamCreate(BI_PARAM_PTR bip, FONT_ACTOR *font_actor, const STRBUF *str,
  PRINTSYS_LSB color, int pal_offset, int pal_id,
  int x, int y, int pos_center, FONT_EX_BMP *ex_bmp );
static void Sub_FontOamDeleteAll(BI_PARAM_PTR bip);
static void Sub_PokeIconResourceLoad(BI_PARAM_PTR bip);
static void Sub_PokeIconResourceFree(BI_PARAM_PTR bip);
static void Sub_PokeIconActorAllDel(BI_PARAM_PTR bip);
static GFL_CLWK *Sub_PokeIconCharActorSet(BI_PARAM_PTR bip, POKEMON_PARAM *pp, int client_type,
  int hp, int maxhp, int status);
static void PokeIconAnimeUpdate(GFL_TCB *tcb, void *work);
static void CommandInEffTask(GFL_TCB *tcb, void *work);
static void VBlankTCB_CommandInEff(GFL_TCB *tcb, void *work);
static void HBlankTCB_CommandInEff(GFL_TCB *tcb, void *work);
static PRINTSYS_LSB PP_FontColorGet(int pp, int pp_max);
void BINPUT_BackFadeReq(BI_PARAM_PTR bip, int fade_dir);
static void BackFadeTask(GFL_TCB *tcb, void *work);
BOOL BINPUT_BackFadeExeCheck(BI_PARAM_PTR bip);
static void DefaultFadeAnimeTask(GFL_TCB *tcb, void *work);
static void StockBallMain(GFL_TCB *tcb, void *work);
static int CursorCheck(BI_PARAM_PTR bip);
static int CursorMove_CommandSelect(BI_PARAM_PTR bip, int init_flag);
static int CursorMove_WazaSelect(BI_PARAM_PTR bip, int init_flag);
static int CursorMove_PokeSelect(BI_PARAM_PTR bip, int init_flag);
static int CursorMove_YesNo(BI_PARAM_PTR bip, int init_flag);
static u32 CursorMove_KeyCheckMove(CURSOR_MOVE *move, int x_menu_num, int y_menu_num,
  const u8 *move_data);
static void CursorSave_CommandSelect(BI_PARAM_PTR bip, int index);
static void CursorSave_WazaSelect(BI_PARAM_PTR bip, int index);
static void CursorSave_PokeSelect(BI_PARAM_PTR bip, int index);
static void Sub_ScrnOffsetRewrite(BI_PARAM_PTR bip, const s16 *scrn_offset,
  const REWRITE_SCRN_RECT *range, int scrnbuf_no, int anm_no);
static MEMORY_DECORD_WORK *MemoryDecordWorkGet(BI_PARAM_PTR bip, int client_type);
static void MemoryDecordWorkFree(BI_PARAM_PTR bip);
static void FontLenGet(const STRBUF *str, GFL_FONT *font,
  int *ret_dot_len, int *ret_char_len);
static void FontExBmpwin_FontSet(BI_PARAM_PTR bip, const STRBUF *str,
  FONT_EX_BMP *ex_bmp, PRINTSYS_LSB color);
static void Sub_JointScreenWrite(BI_PARAM_PTR bip);
static int CursorMove_ParkCommandSelect(BI_PARAM_PTR bip, int init_flag);
static void BackGroundTouchPaletteCheck(GFL_TCB *tcb, void *work);
static int CaptureDemo_Main(BI_PARAM_PTR bip);
static int CaptureDemoSeq_CommandFight(BI_PARAM_PTR bip);
static int CaptureDemoSeq_Waza(BI_PARAM_PTR bip);
static int CaptureDemoSeq_CommandBag(BI_PARAM_PTR bip);

//--------------------------------------------------------------
//  インライン関数のプロトタイプ宣言
//--------------------------------------------------------------
inline void SubInline_EffBgWriteParamSet(EFFBG_WRITE_PARAM *ewp, const REWRITE_SCRN_RECT *rsr,
  u8 rsr_num, s8 add_charname);


//==============================================================================
//  データ
//==============================================================================
///戦闘入力画面用のBGフレーム構成
static const GFL_BG_BGCNT_HEADER BiBgCntDat[] = {
  {//GF_BGL_FRAME0_S
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, 0x6000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  },
  {//GF_BGL_FRAME1_S
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x6000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  },
  {//GF_BGL_FRAME2_S
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x6000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  },
  {//GF_BGL_FRAME3_S
    0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x08000, 0x8000,
    GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  },
};

//============================================================================================
//  タッチパネル領域設定
//============================================================================================
//--------------------------------------------------------------
//
//--------------------------------------------------------------
///コマンド選択タッチパネル領域設定
static const GFL_UI_TP_HITTBL BattleMenuTouchData[] = {
  //UP DOWN LEFT RIGHT
  {3*8, 0x12*8, 0*8, 255},      //たたかう
  {0x12*8, 0x18*8, 0*8, 0xa*8},   //バッグ
  {0x12*8, 0x18*8, 0x16*8, 255},  //ポケモン
  {0x13*8, 0x18*8, 0xb*8, 0x15*8},  //にげる
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
///コマンド選択タッチパネルのタッチ結果返事
static const int BattleMenuTouchRet[NELEMS(BattleMenuTouchData) - 1] = {
  SELECT_FIGHT_COMMAND,   //たたかう
  SELECT_ITEM_COMMAND,    //バッグ
  SELECT_POKEMON_COMMAND,   //ポケモン
  SELECT_ESCAPE_COMMAND,    //にげる
};
///コマンド選択タッチパネルの各パネルのパレット番号
ALIGN4 static const u8 BattleMenuPaletteNo[NELEMS(BattleMenuTouchData) - 1] = {
  1,    //たたかう
  2,    //バッグ
  3,    //ポケモン
  4,    //にげる
};

///カーソル移動：コマンド選択の水平メニューの最大数
#define CURSOR_COMMAND_SELECT_X_MENU_NUM    (3)
///カーソル移動：コマンド選択のメニューの階層の数
#define CURSOR_COMMAND_SELECT_Y_MENU_NUM    (2)// + 1)
///コマンド選択のタッチパネルINDEX(BattleMenuTouchRetの順番)
enum{
  COMMSELE_INDEX_FIGHT,
  COMMSELE_INDEX_ITEM,
  COMMSELE_INDEX_POKEMON,
  COMMSELE_INDEX_ESCAPE,
};
///カーソル移動：コマンド選択の移動範囲データ(TouchDataのindex番号)
ALIGN4 static const u8 CursorMoveDataCommandSelect[CURSOR_COMMAND_SELECT_Y_MENU_NUM][CURSOR_COMMAND_SELECT_X_MENU_NUM] = {
  {COMMSELE_INDEX_FIGHT, COMMSELE_INDEX_FIGHT, COMMSELE_INDEX_FIGHT},   //たたかう
  {COMMSELE_INDEX_ITEM, COMMSELE_INDEX_ESCAPE, COMMSELE_INDEX_POKEMON},//バッグ、にげる、ポケモン
  //バッグorポケモンの箇所で下を押したら「にげる」に移動するように3段目を作る
//  {COMMSELE_INDEX_ESCAPE, COMMSELE_INDEX_ESCAPE, COMMSELE_INDEX_ESCAPE},//にげる、にげる、にげる
};

//--------------------------------------------------------------
//
//--------------------------------------------------------------
///「たたかう」のみのコマンド選択タッチパネル領域設定
static const GFL_UI_TP_HITTBL FightOnlyBattleMenuTouchData[] = {
  //UP DOWN LEFT RIGHT
  {3*8, 0x12*8, 0*8, 255},      //たたかう
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};

///カーソル移動：「たたかう」のみのコマンド選択の水平メニューの最大数
#define CURSOR_COMMAND_FIGHTONLY_X_MENU_NUM   (1)
///カーソル移動：「たたかう」のみのコマンド選択のメニューの階層の数
#define CURSOR_COMMAND_FIGHTONLY_Y_MENU_NUM   (1)

//--------------------------------------------------------------
//  ポケパーク
//--------------------------------------------------------------
///「ボール」と「逃げる」のみのコマンド選択タッチパネル領域設定
static const GFL_UI_TP_HITTBL ParkCommandMenuTouchData[] = {
  //UP DOWN LEFT RIGHT
  {5*8, 0x10*8, 3*8, 0x1d*8},     //たたかう
  {0x13*8, 0x18*8, 0xb*8, 0x15*8},  //にげる
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};

///ポケパーク用コマンド選択タッチパネルのタッチ結果返事
static const int ParkMenuTouchRet[NELEMS(BattleMenuTouchData) - 1] = {
  SELECT_FIGHT_COMMAND,   //たたかう
  SELECT_ESCAPE_COMMAND,    //にげる
};
///ポケパーク用コマンド選択タッチパネルの各パネルのパレット番号
ALIGN4 static const u8 ParkMenuPaletteNo[NELEMS(BattleMenuTouchData) - 1] = {
  1,    //たたかう
  4,    //にげる
};

///カーソル移動：コマンド選択の水平メニューの最大数
#define CURSOR_PARK_COMMAND_SELECT_X_MENU_NUM   (1)
///カーソル移動：コマンド選択のメニューの階層の数
#define CURSOR_PARK_COMMAND_SELECT_Y_MENU_NUM   (2)
///コマンド選択のタッチパネルINDEX(ParkMenuTouchRetの順番)
enum{
  PARKSELE_INDEX_FIGHT,
  PARKSELE_INDEX_ESCAPE,
};
///カーソル移動：コマンド選択の移動範囲データ(TouchDataのindex番号)
ALIGN4 static const u8 CursorMoveDataParkCommandSelect[CURSOR_PARK_COMMAND_SELECT_Y_MENU_NUM][CURSOR_PARK_COMMAND_SELECT_X_MENU_NUM] = {
  {PARKSELE_INDEX_FIGHT},   //たたかう
  {PARKSELE_INDEX_ESCAPE},  //にげる
};

//--------------------------------------------------------------
//
//--------------------------------------------------------------
///技選択タッチパネル領域設定
static const GFL_UI_TP_HITTBL SkillMenuTouchData[] = {
  //UP DOWN LEFT RIGHT
  {0x13*8, 0x18*8, 1*8, 0x1f*8},    //キャンセル
  {3*8, 10*8, 0*8, 0x10*8},   //技1
  {3*8, 10*8, 0x10*8, 255}, //技2
  {0xb*8, 0x12*8, 0*8, 0x10*8}, //技3
  {0xb*8, 0x12*8, 0x10*8, 255}, //技4
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
///技選択タッチパネルのタッチ結果返事
static const int SkillMenuTouchRet[NELEMS(SkillMenuTouchData) - 1] = {
  SELECT_CANCEL,        //キャンセル
  SELECT_SKILL_1,
  SELECT_SKILL_2,
  SELECT_SKILL_3,
  SELECT_SKILL_4,
};
///技選択タッチパネルの各パネルのパレット番号
ALIGN4 static const u8 SkillMenuPaletteNo[NELEMS(SkillMenuTouchData) - 1] = {
  4,    //SELECT_CANCEL,        //キャンセル
  8,    //SELECT_SKILL_1,
  9,    //SELECT_SKILL_2,
  0xa,  //SELECT_SKILL_3,
  0xb,  //SELECT_SKILL_4,
};

///カーソル移動：技選択の水平メニューの最大数
#define CURSOR_WAZA_SELECT_X_MENU_NUM   (2)
///カーソル移動：技選択のメニューの階層の数
#define CURSOR_WAZA_SELECT_Y_MENU_NUM   (3)
///技選択のタッチパネルINDEX(SkillMenuTouchRetの順番)
enum{
  WAZASELE_INDEX_CANCEL,
  WAZASELE_INDEX_SKILL_1,
  WAZASELE_INDEX_SKILL_2,
  WAZASELE_INDEX_SKILL_3,
  WAZASELE_INDEX_SKILL_4,
};
///カーソル移動：技選択の移動範囲データ
ALIGN4 static const u8 CursorMoveDataWazaSelect[CURSOR_WAZA_SELECT_Y_MENU_NUM][CURSOR_WAZA_SELECT_X_MENU_NUM] = {
  {WAZASELE_INDEX_SKILL_1, WAZASELE_INDEX_SKILL_2},   //技１、技２
  {WAZASELE_INDEX_SKILL_3, WAZASELE_INDEX_SKILL_4},   //技３、技４
  {WAZASELE_INDEX_CANCEL, WAZASELE_INDEX_CANCEL},   //キャンセル、キャンセル
};

//--------------------------------------------------------------
//
//--------------------------------------------------------------
///「たたかいをやめますか？」選択タッチパネル領域設定
static const GFL_UI_TP_HITTBL EscapeMenuTouchData[] = {
  //UP DOWN LEFT RIGHT
  {5*8, 0xc*8, 1*8, 0x1f*8},    //はい
  {0xe*8, 0x15*8, 1*8, 0x1f*8},   //いいえ
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
///「たたかいをやめますか？」選択タッチパネルのタッチ結果返事
static const int EscapeMenuTouchRet[NELEMS(EscapeMenuTouchData) - 1] = {
  TRUE,     //はい
  SELECT_CANCEL,      //いいえ
};
///「たたかいをやめますか？」選択タッチパネルの各パネルのパレット番号
ALIGN4 static const u8 EscapeMenuPaletteNo[NELEMS(EscapeMenuTouchData) - 1] = {
  1,    //TRUE,     //はい
  4,    //SELECT_CANCEL,      //いいえ
};
///カーソル移動：「はい/いいえ」の水平メニューの最大数
#define CURSOR_YESNO_SELECT_X_MENU_NUM    (1)
///カーソル移動：「はい/いいえ」のメニューの階層の数
#define CURSOR_YESNO_SELECT_Y_MENU_NUM    (2)
///技選択のタッチパネルINDEX(SkillMenuTouchRetの順番)
enum{
  YESNO_INDEX_TRUE,
  YESNO_INDEX_CANCEL,
};
///カーソル移動：「はい/いいえ」の移動範囲データ
ALIGN4 static const u8 CursorMoveDataYesNoSelect[CURSOR_YESNO_SELECT_Y_MENU_NUM][CURSOR_YESNO_SELECT_X_MENU_NUM] = {
  {YESNO_INDEX_TRUE},
  {YESNO_INDEX_CANCEL},
};

//--------------------------------------------------------------
//  ポケモン選択(技効果)
//--------------------------------------------------------------
///ポケモン選択(技効果)タッチパネル領域設定
static const GFL_UI_TP_HITTBL PokeSeleMenuTouchData[] = {
  //UP DOWN LEFT RIGHT
  {0xb*8, 0x12*8, 0*8, 0xf*8},      //ターゲットA
  {1*8, 0xa*8, 0x11*8, 255},    //ターゲットB
  {0xb*8, 0x12*8, 0x11*8, 255},   //ターゲットC
  {1*8, 0xa*8, 0*8, 0xf*8},     //ターゲットD
  {0x13*8, 0x18*8, 1*8, 0x1f*8},    //キャンセル
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
///ポケモン選択タッチパネルのタッチ結果返事
static const int PokeSeleMenuTouchRet[NELEMS(PokeSeleMenuTouchData) - 1] = {
  SELECT_TARGET_A,    //ターゲットA
  SELECT_TARGET_B,    //ターゲットB
  SELECT_TARGET_C,    //ターゲットC
  SELECT_TARGET_D,    //ターゲットD
  SELECT_CANCEL,      //もどる
};
///ポケモン選択タッチパネルの各パネルのパレット番号
ALIGN4 static const u8 PokeSeleMenuPaletteNo[NELEMS(PokeSeleMenuTouchData) - 1] = {
  6,    //SELECT_TARGET_A,    //ターゲットA
  0xc,  //SELECT_TARGET_B,    //ターゲットB
  0xd,  //SELECT_TARGET_C,    //ターゲットC
  5,    //SELECT_TARGET_D,    //ターゲットD
  4,    //SELECT_CANCEL,      //もどる
};

///ポケモン選択のタッチパネルINDEX(PokeSeleMenuTouchRetの順番)
enum{
  POKESELE_INDEX_TARGET_A,
  POKESELE_INDEX_TARGET_B,
  POKESELE_INDEX_TARGET_C,
  POKESELE_INDEX_TARGET_D,
  POKESELE_INDEX_TARGET_CANCEL,
};

///カーソル移動：ポケモン選択(単数選択の場合)の水平メニューの最大数
#define CURSOR_POKE_SELECT_X_MENU_NUM   (2)
///カーソル移動：ポケモン選択(単数選択の場合)のメニューの階層の数
#define CURSOR_POKE_SELECT_Y_MENU_NUM   (3)
///カーソル移動：ポケモン選択(単数選択の場合)の移動範囲データ
ALIGN4 static const u8 CursorMoveDataPokeSelect[CURSOR_POKE_SELECT_Y_MENU_NUM][CURSOR_POKE_SELECT_X_MENU_NUM] = {
  {POKESELE_INDEX_TARGET_D, POKESELE_INDEX_TARGET_B},
  {POKESELE_INDEX_TARGET_A, POKESELE_INDEX_TARGET_C},
  {POKESELE_INDEX_TARGET_CANCEL, POKESELE_INDEX_TARGET_CANCEL},
};

//--------------------------------------------------------------
//  録画再生の停止ボタン画面
//--------------------------------------------------------------
///「さいせい　ていし」のみのコマンド選択タッチパネル領域設定
static const GFL_UI_TP_HITTBL PlayBackStopOnlyMenuTouchData[] = {
  //UP DOWN LEFT RIGHT
  {0x13*8, 0x18*8, 0x0*8, 0x20*8 - 1},      //ていし
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
///「さいせい　ていし」タッチパネルのタッチ結果返事
static const int PlayBackStopOnlyMenuTouchRet[NELEMS(PlayBackStopOnlyMenuTouchData) - 1] = {
  TRUE,   //さいせい　ていし
};
///「さいせい　ていし」タッチパネルの各パネルのパレット番号
ALIGN4 static const u8 PlayBackStopMenuPaletteNo[NELEMS(PlayBackStopOnlyMenuTouchData) - 1] = {
  4,    //さいせい　ていし
};
///カーソル移動：「さいせい　ていし」のみのコマンド選択の水平メニューの最大数
#define CURSOR_COMMAND_PLAYBACKSTOP_X_MENU_NUM    (1)
///カーソル移動：「さいせい　ていし」のみのコマンド選択のメニューの階層の数
#define CURSOR_COMMAND_PLAYBACKSTOP_Y_MENU_NUM    (1)

//==============================================================================
//
//==============================================================================
///BG作成データ(※ここの並びを追加、変更したらBINPUT_TYPE_???の定義も変更すること！！)
static const BG_MAKE_DATA BgMakeData[] = {
  {//BINPUT_TYPE_WALL
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      NONE_ID,    //パネル
      NONE_ID,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    NULL,       //タッチパネルデータ
    NULL,       //タッチパネル結果
    NULL,       //タッチパネルの各パレットのパレット番号
    NULL,       //キー入力をした時のコールバック関数
    NULL,       //キー位置記憶のコールバック関数
    NULL,       //BG作成時のコールバック関数
    NULL,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_COMMAND_IN
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      SCRNBUF_COMMAND_WAZA,   //技選択パネルの影
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    BattleMenuTouchData,        //タッチパネルデータ
    BattleMenuTouchRet,         //タッチパネル結果
    BattleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_CommandSelect,       //キー入力をした時のコールバック関数
    CursorSave_CommandSelect,     ///<キー位置記憶のコールバック関数
    BGCallback_CommandIn,       //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_COMMAND_IN_2    2体目
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      SCRNBUF_COMMAND_WAZA,   //技選択パネルの影
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    BattleMenuTouchData,        //タッチパネルデータ
    BattleMenuTouchRet,         //タッチパネル結果
    BattleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_CommandSelect,       //キー入力をした時のコールバック関数
    CursorSave_CommandSelect,     ///<キー位置記憶のコールバック関数
    BGCallback_CommandIn,       //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_A
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      SCRNBUF_COMMAND_WAZA,   //技選択パネルの影
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    BattleMenuTouchData,        //タッチパネルデータ
    BattleMenuTouchRet,         //タッチパネル結果
    BattleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_CommandSelect,       //キー入力をした時のコールバック関数
    CursorSave_CommandSelect,     ///<キー位置記憶のコールバック関数
    BGCallback_CommandSelect,       //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_A_2    2体目
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      SCRNBUF_COMMAND_WAZA,   //技選択パネルの影
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    BattleMenuTouchData,        //タッチパネルデータ
    BattleMenuTouchRet,         //タッチパネル結果
    BattleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_CommandSelect,       //キー入力をした時のコールバック関数
    CursorSave_CommandSelect,     ///<キー位置記憶のコールバック関数
    BGCallback_CommandSelect,       //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_COMMAND_IN_FIGHTONLY
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      SCRNBUF_COMMAND_WAZA,   //技選択パネルの影
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    FightOnlyBattleMenuTouchData,       //タッチパネルデータ
    BattleMenuTouchRet,         //タッチパネル結果
    BattleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_CommandSelect,       //キー入力をした時のコールバック関数
    CursorSave_CommandSelect,     ///<キー位置記憶のコールバック関数
    BGCallback_CommandInFightOnly,        //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_FIGHTONLY
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      SCRNBUF_COMMAND_WAZA,   //技選択パネルの影
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    FightOnlyBattleMenuTouchData,       //タッチパネルデータ
    BattleMenuTouchRet,         //タッチパネル結果
    BattleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_CommandSelect,       //キー入力をした時のコールバック関数
    CursorSave_CommandSelect,     ///<キー位置記憶のコールバック関数
    BGCallback_FightOnly,       //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_COMMAND_IN_SAFARI
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      NONE_ID,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    BattleMenuTouchData,        //タッチパネルデータ
    BattleMenuTouchRet,         //タッチパネル結果
    BattleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_CommandSelect,       //キー入力をした時のコールバック関数
    CursorSave_CommandSelect,     ///<キー位置記憶のコールバック関数
    BGCallback_CommandIn,       //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_A_SAFARI
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      NONE_ID,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    BattleMenuTouchData,        //タッチパネルデータ
    BattleMenuTouchRet,         //タッチパネル結果
    BattleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_CommandSelect,       //キー入力をした時のコールバック関数
    CursorSave_CommandSelect,     ///<キー位置記憶のコールバック関数
    BGCallback_CommandSelect,       //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_COMMAND_IN_PARK
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      NONE_ID,    //技選択パネルの影
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    ParkCommandMenuTouchData,       //タッチパネルデータ
    ParkMenuTouchRet,         //タッチパネル結果
    ParkMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_ParkCommandSelect,       //キー入力をした時のコールバック関数
    NULL,     ///<キー位置記憶のコールバック関数
    BGCallback_ParkCommandIn,     //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_PARK
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_COMMAND,    //パネル
      NONE_ID,    //技選択パネルの影
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    ParkCommandMenuTouchData,       //タッチパネルデータ
    ParkMenuTouchRet,         //タッチパネル結果
    ParkMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_ParkCommandSelect,       //キー入力をした時のコールバック関数
    NULL,     ///<キー位置記憶のコールバック関数
    BGCallback_ParkCommand,     //BG作成時のコールバック関数
    TPCallback_A,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_WAZA
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_WAZA,   //パネル
      NONE_ID,          //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    SkillMenuTouchData,       //タッチパネルデータ
    SkillMenuTouchRet,        //タッチパネル結果
    SkillMenuPaletteNo,       //タッチパネルの各パレットのパレット番号
    CursorMove_WazaSelect,        //キー入力をした時のコールバック関数
    CursorSave_WazaSelect,      ///<キー位置記憶のコールバック関数
    BGCallback_Waza,        //BG作成時のコールバック関数
    TPCallback_Waza,        //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_POKE
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_POKESELE,   //パネル
      SCRNBUF_POKESELE_WAKU,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    PokeSeleMenuTouchData,        //タッチパネルデータ
    PokeSeleMenuTouchRet,       //タッチパネル結果
    PokeSeleMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_PokeSelect,        //キー入力をした時のコールバック関数
    CursorSave_PokeSelect,        //キー位置記憶のコールバック関数
    BGCallback_PokeSelect,        //BG作成時のコールバック関数
    TPCallback_PokeSele,        //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_YESNO
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_YESNO,    //パネル
      NONE_ID,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    EscapeMenuTouchData,        //タッチパネルデータ
    EscapeMenuTouchRet,       //タッチパネル結果
    EscapeMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_YesNo,       //キー入力をした時のコールバック関数
    NULL,       //キー位置記憶のコールバック関数
    BGCallback_YesNo,       //BG作成時のコールバック関数
    TPCallback_D,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_WASURERU
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_YESNO,    //パネル
      NONE_ID,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    EscapeMenuTouchData,        //タッチパネルデータ
    EscapeMenuTouchRet,       //タッチパネル結果
    EscapeMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_YesNo,       //キー入力をした時のコールバック関数
    NULL,       //キー位置記憶のコールバック関数
    BGCallback_Wasureru,        //BG作成時のコールバック関数
    TPCallback_D,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_AKIRAMERU
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_YESNO,    //パネル
      NONE_ID,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    EscapeMenuTouchData,        //タッチパネルデータ
    EscapeMenuTouchRet,       //タッチパネル結果
    EscapeMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_YesNo,       //キー入力をした時のコールバック関数
    NULL,       //キー位置記憶のコールバック関数
    BGCallback_Akirameru,       //BG作成時のコールバック関数
    TPCallback_D,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_NEXT_POKEMON
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_YESNO,    //パネル
      NONE_ID,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    EscapeMenuTouchData,        //タッチパネルデータ
    EscapeMenuTouchRet,       //タッチパネル結果
    EscapeMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_YesNo,       //キー入力をした時のコールバック関数
    NULL,       //キー位置記憶のコールバック関数
    BGCallback_NextPokemon,       //BG作成時のコールバック関数
    TPCallback_D,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_CHANGE_POKEMON
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_YESNO,    //パネル
      NONE_ID,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 1, 3, 0},
    EscapeMenuTouchData,        //タッチパネルデータ
    EscapeMenuTouchRet,       //タッチパネル結果
    EscapeMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    CursorMove_YesNo,       //キー入力をした時のコールバック関数
    NULL,       //キー位置記憶のコールバック関数
    BGCallback_ChangePokemon,       //BG作成時のコールバック関数
    TPCallback_D,       //タッチパネル反応時のコールバック関数
  },
  {//BINPUT_TYPE_PLAYBACK_STOP
    BATTLE_W_NCGR_BIN,
    BATTLE_W_NCLR,
    {
      SCRNBUF_PLAYBACK_STOP,    //パネル
      SCRNBUF_PLAYBACK_SYSWIN,    //黒枠
      SCRNBUF_BACKGROUND,   //背景
      NONE_ID,              //黒枠スライド用
    },
    {2, 3, 3, 0},
    PlayBackStopOnlyMenuTouchData,        //タッチパネルデータ
    PlayBackStopOnlyMenuTouchRet,         //タッチパネル結果
    PlayBackStopMenuPaletteNo,        //タッチパネルの各パレットのパレット番号
    NULL,       //キー入力をした時のコールバック関数
    NULL,     ///<キー位置記憶のコールバック関数
    BGCallback_PlayBackStop,        //BG作成時のコールバック関数
    TPCallback_PlayBackStop,        //タッチパネル反応時のコールバック関数
  },
};


//==============================================================================
//
//  アクターヘッダ
//
//==============================================================================
///手持ちボール：自機側アクターヘッダ
static const GFL_CLWK_DATA StockObjParam_Mine = {
  STOCK_MINE_BASE_POS_X, STOCK_MINE_BASE_POS_Y, //x, y
  0, SOFTPRI_STOCK_MINE,  1,            //アニメ番号、優先順位、BGプライオリティ
};

///手持ちボール：敵側アクターヘッダ
static const GFL_CLWK_DATA StockObjParam_Enemy = {
  STOCK_ENEMY_BASE_POS_X, STOCK_ENEMY_BASE_POS_Y,   //x, y
  0, SOFTPRI_STOCK_ENEMY, 1,              //アニメ番号、優先順位、BGプライオリティ
};

///技タイプアイコン：アクターヘッダ
static const GFL_CLWK_DATA WazaTypeIconObjParam = {
  0, 0,   //x, y
  0, 100, 1,  //アニメ番号、優先順位、BGプライオリティ
};

///技分類アイコン：アクターヘッダ
static const GFL_CLWK_DATA WazaKindIconObjParam = {
  0, 0,   //x, y
  0, 100, 1,  //アニメ番号、優先順位、BGプライオリティ
};

///黒枠アクターヘッダ
static const GFL_CLWK_DATA BlackWakuObjParam = {
  0, 0,   //x, y
  0, 150, 1,  //アニメ番号、優先順位、BGプライオリティ
};

///ポケモンアイコン：アクターヘッダ
static const GFL_CLWK_DATA PokeIconObjParam = {
  0, 0,   //x, y
  0, 100, 1,  //アニメ番号、優先順位、BGプライオリティ
};


//==============================================================================
//
//  技タイプ
//
//==============================================================================
///技の技タイプキャラ転送位置(キャラクタ単位)
ALIGN4 static const u16 WazaTypeCgrPos[] = {0, 6, 12, 18};


//==============================================================================
//
//
//
//==============================================================================
///ボタンの押した時のアニメパターン数
#define BUTTON_ANM_MAX    (3)

//--------------------------------------------------------------
//  押せないパネル
//--------------------------------------------------------------
///スクリーン番号：押せないパネル　アニメ0、左上
#define SCRN_NO_NOT_TOUCH_0   (0x260)
///押せないパネルのスクリーンオフセットを求める時の補正値
///※押せないパネルは押せるパネルよりも1段少ない位置からしかデータがないので
#define SCRN_NO_NOT_TOUCH_HOSEI (32)

//--------------------------------------------------------------
//  コマンド選択
//--------------------------------------------------------------
///スクリーン番号：コマンド選択「たたかう」アニメ0、左上
#define SCRN_NO_COMMAND_FIGHT_0   (0x20)
///スクリーン番号：コマンド選択「たたかう」アニメ1、左上
#define SCRN_NO_COMMAND_FIGHT_1   (0xe0)
///スクリーン番号：コマンド選択「たたかう」アニメ2、左上
#define SCRN_NO_COMMAND_FIGHT_2   (0x1a0)

///スクリーン番号：コマンド選択「バッグ」アニメ0、左上
#define SCRN_NO_COMMAND_ITEM_0    (0x5)
///スクリーン番号：コマンド選択「バッグ」アニメ1、左上
#define SCRN_NO_COMMAND_ITEM_1    (0xc5)
///スクリーン番号：コマンド選択「バッグ」アニメ2、左上
#define SCRN_NO_COMMAND_ITEM_2    (0x185)

///スクリーン番号：コマンド選択「ポケモン」アニメ0、左上
#define SCRN_NO_COMMAND_POKEMON_0 (0x8)
///スクリーン番号：コマンド選択「ポケモン」アニメ1、左上
#define SCRN_NO_COMMAND_POKEMON_1 (0xc8)
///スクリーン番号：コマンド選択「ポケモン」アニメ2、左上
#define SCRN_NO_COMMAND_POKEMON_2 (0x188)

///スクリーン番号：コマンド選択「にげる」アニメ0、左上
#define SCRN_NO_COMMAND_ESCAPE_0  (0x1d)
///スクリーン番号：コマンド選択「にげる」アニメ1、左上
#define SCRN_NO_COMMAND_ESCAPE_1  (0xdd)
///スクリーン番号：コマンド選択「にげる」アニメ2、左上
#define SCRN_NO_COMMAND_ESCAPE_2  (0x19d)

///コマンド選択パネルを押した時のボタンアニメ用スクリーンオフセット値
ALIGN4 static const s16 CommandButtonScrnOffset[][BUTTON_ANM_MAX] = {
  {//SELECT_FIGHT_COMMAND
    SCRN_NO_COMMAND_FIGHT_0 - SCRN_NO_COMMAND_FIGHT_0,
    SCRN_NO_COMMAND_FIGHT_1 - SCRN_NO_COMMAND_FIGHT_0,
    SCRN_NO_COMMAND_FIGHT_2 - SCRN_NO_COMMAND_FIGHT_0,
  },
  {//SELECT_ITEM_COMMAND
    SCRN_NO_COMMAND_ITEM_0 - SCRN_NO_COMMAND_ITEM_0,
    SCRN_NO_COMMAND_ITEM_1 - SCRN_NO_COMMAND_ITEM_0,
    SCRN_NO_COMMAND_ITEM_2 - SCRN_NO_COMMAND_ITEM_0,
  },
  {//SELECT_POKEMON_COMMAND
    SCRN_NO_COMMAND_POKEMON_0 - SCRN_NO_COMMAND_POKEMON_0,
    SCRN_NO_COMMAND_POKEMON_1 - SCRN_NO_COMMAND_POKEMON_0,
    SCRN_NO_COMMAND_POKEMON_2 - SCRN_NO_COMMAND_POKEMON_0,
  },
  {//SELECT_ESCAPE_COMMAND
    SCRN_NO_COMMAND_ESCAPE_0 - SCRN_NO_COMMAND_ESCAPE_0,
    SCRN_NO_COMMAND_ESCAPE_1 - SCRN_NO_COMMAND_ESCAPE_0,
    SCRN_NO_COMMAND_ESCAPE_2 - SCRN_NO_COMMAND_ESCAPE_0,
  },
};

///コマンド選択パネルを押した時のスクリーンアニメ書き換え範囲
ALIGN4 static const REWRITE_SCRN_RECT CommandButtonScrnRect[] = {
  {4, 0xf, 2, 0x1d},
  {0x11, 0x17, 0, 0x9},
  {0x11, 0x17, 0x16, 0x1f},
  {0x12, 0x17, 0xb, 0x14},
};

//--------------------------------------------------------------
//  技選択
//--------------------------------------------------------------
///スクリーン番号：技選択「技0」アニメ0、左上
#define SCRN_NO_WAZA_SKILL_0_0    (0x11)
///スクリーン番号：技選択「技0」アニメ1、左上
#define SCRN_NO_WAZA_SKILL_0_1    (0xd1)
///スクリーン番号：技選択「技0」アニメ2、左上
#define SCRN_NO_WAZA_SKILL_0_2    (0x191)

///スクリーン番号：技選択「技1」アニメ0、左上
#define SCRN_NO_WAZA_SKILL_1_0    (0x14)
///スクリーン番号：技選択「技1」アニメ1、左上
#define SCRN_NO_WAZA_SKILL_1_1    (0xd4)
///スクリーン番号：技選択「技1」アニメ2、左上
#define SCRN_NO_WAZA_SKILL_1_2    (0x194)

///スクリーン番号：技選択「技2」アニメ0、左上
#define SCRN_NO_WAZA_SKILL_2_0    (0x17)
///スクリーン番号：技選択「技2」アニメ1、左上
#define SCRN_NO_WAZA_SKILL_2_1    (0xd7)
///スクリーン番号：技選択「技2」アニメ2、左上
#define SCRN_NO_WAZA_SKILL_2_2    (0x197)

///スクリーン番号：技選択「技3」アニメ0、左上
#define SCRN_NO_WAZA_SKILL_3_0    (0x1a)
///スクリーン番号：技選択「技3」アニメ1、左上
#define SCRN_NO_WAZA_SKILL_3_1    (0xda)
///スクリーン番号：技選択「技3」アニメ2、左上
#define SCRN_NO_WAZA_SKILL_3_2    (0x19a)

///スクリーン番号：技選択「キャンセル」アニメ0、左上
#define SCRN_NO_WAZA_CANCEL_0   (0x1d)
///スクリーン番号：技選択「キャンセル」アニメ1、左上
#define SCRN_NO_WAZA_CANCEL_1   (0xdd)
///スクリーン番号：技選択「キャンセル」アニメ2、左上
#define SCRN_NO_WAZA_CANCEL_2   (0x19d)

///技選択パネルを押した時のボタンアニメ用スクリーンオフセット値
ALIGN4 static const s16 WazaButtonScrnOffset[][BUTTON_ANM_MAX] = {
  {//SELECT_SKILL_1
    SCRN_NO_WAZA_SKILL_0_0 - SCRN_NO_WAZA_SKILL_0_0,
    SCRN_NO_WAZA_SKILL_0_1 - SCRN_NO_WAZA_SKILL_0_0,
    SCRN_NO_WAZA_SKILL_0_2 - SCRN_NO_WAZA_SKILL_0_0,
  },
  {//SELECT_SKILL_2
    SCRN_NO_WAZA_SKILL_1_0 - SCRN_NO_WAZA_SKILL_1_0,
    SCRN_NO_WAZA_SKILL_1_1 - SCRN_NO_WAZA_SKILL_1_0,
    SCRN_NO_WAZA_SKILL_1_2 - SCRN_NO_WAZA_SKILL_1_0,
  },
  {//SELECT_SKILL_3
    SCRN_NO_WAZA_SKILL_2_0 - SCRN_NO_WAZA_SKILL_2_0,
    SCRN_NO_WAZA_SKILL_2_1 - SCRN_NO_WAZA_SKILL_2_0,
    SCRN_NO_WAZA_SKILL_2_2 - SCRN_NO_WAZA_SKILL_2_0,
  },
  {//SELECT_SKILL_4
    SCRN_NO_WAZA_SKILL_3_0 - SCRN_NO_WAZA_SKILL_3_0,
    SCRN_NO_WAZA_SKILL_3_1 - SCRN_NO_WAZA_SKILL_3_0,
    SCRN_NO_WAZA_SKILL_3_2 - SCRN_NO_WAZA_SKILL_3_0,
  },
  {//SELECT_CANCEL
    SCRN_NO_WAZA_CANCEL_0 - SCRN_NO_WAZA_CANCEL_0,
    SCRN_NO_WAZA_CANCEL_1 - SCRN_NO_WAZA_CANCEL_0,
    SCRN_NO_WAZA_CANCEL_2 - SCRN_NO_WAZA_CANCEL_0,
  },
};

///技選択パネルを押した時のスクリーンアニメ書き換え範囲
ALIGN4 static const REWRITE_SCRN_RECT WazaButtonScrnRect[] = {
  {2, 9, 0, 0xf},
  {2, 9, 0x10, 0x1f},
  {0xa, 0x11, 0, 0xf},
  {0xa, 0x11, 0x10, 0x1f},
  {0x12, 0x17, 1, 0x1e},
};

///技選択パネルを押せないボタンにする時のスクリーンオフセット値
ALIGN4 static const s16 WazaButtonNotTouchScrnOffset[] = {
  SCRN_NO_NOT_TOUCH_0 - SCRN_NO_WAZA_SKILL_0_0 - SCRN_NO_NOT_TOUCH_HOSEI, //SELECT_SKILL_1
  SCRN_NO_NOT_TOUCH_0 - SCRN_NO_WAZA_SKILL_1_0 - SCRN_NO_NOT_TOUCH_HOSEI, //SELECT_SKILL_1
  SCRN_NO_NOT_TOUCH_0 - SCRN_NO_WAZA_SKILL_2_0 - SCRN_NO_NOT_TOUCH_HOSEI, //SELECT_SKILL_1
  SCRN_NO_NOT_TOUCH_0 - SCRN_NO_WAZA_SKILL_3_0 - SCRN_NO_NOT_TOUCH_HOSEI, //SELECT_SKILL_1
};

///技選択パネルを押せないボタンにする時のスクリーンアニメ書き換え範囲
ALIGN4 static const REWRITE_SCRN_RECT WazaButtonNotTouchScrnRect[] = {
  {3, 9, 0, 0xf},
  {3, 9, 0x10, 0x1f},
  {0xb, 0x11, 0, 0xf},
  {0xb, 0x11, 0x10, 0x1f},
};

//--------------------------------------------------------------
//  ポケモン選択
//--------------------------------------------------------------
///スクリーン番号：ポケモン選択「CLIENT_TYPE_A」アニメ0、左上
#define SCRN_NO_POKEMON_A_0   (0x8)
///スクリーン番号：ポケモン選択「CLIENT_TYPE_A」アニメ1、左上
#define SCRN_NO_POKEMON_A_1   (0xc8)
///スクリーン番号：ポケモン選択「CLIENT_TYPE_A」アニメ2、左上
#define SCRN_NO_POKEMON_A_2   (0x188)

///スクリーン番号：ポケモン選択「CLIENT_TYPE_B」アニメ0、左上
#define SCRN_NO_POKEMON_B_0   (0xb)
///スクリーン番号：ポケモン選択「CLIENT_TYPE_B」アニメ1、左上
#define SCRN_NO_POKEMON_B_1   (0xcb)
///スクリーン番号：ポケモン選択「CLIENT_TYPE_B」アニメ2、左上
#define SCRN_NO_POKEMON_B_2   (0x18b)

///スクリーン番号：ポケモン選択「CLIENT_TYPE_C」アニメ0、左上
#define SCRN_NO_POKEMON_C_0   (0xe)
///スクリーン番号：ポケモン選択「CLIENT_TYPE_C」アニメ1、左上
#define SCRN_NO_POKEMON_C_1   (0xce)
///スクリーン番号：ポケモン選択「CLIENT_TYPE_C」アニメ2、左上
#define SCRN_NO_POKEMON_C_2   (0x18e)

///スクリーン番号：ポケモン選択「CLIENT_TYPE_D」アニメ0、左上
#define SCRN_NO_POKEMON_D_0   (0x5)
///スクリーン番号：ポケモン選択「CLIENT_TYPE_D」アニメ1、左上
#define SCRN_NO_POKEMON_D_1   (0xc5)
///スクリーン番号：ポケモン選択「CLIENT_TYPE_D」アニメ2、左上
#define SCRN_NO_POKEMON_D_2   (0x185)

///スクリーン番号：ポケモン選択「キャンセル」アニメ0、左上
#define SCRN_NO_POKEMON_CANCEL_0    (0x1d)
///スクリーン番号：ポケモン選択「キャンセル」アニメ1、左上
#define SCRN_NO_POKEMON_CANCEL_1    (0xdd)
///スクリーン番号：ポケモン選択「キャンセル」アニメ2、左上
#define SCRN_NO_POKEMON_CANCEL_2    (0x19d)

///ポケモン選択パネルを押した時のボタンアニメ用スクリーンオフセット値
ALIGN4 static const s16 PokemonButtonScrnOffset[][BUTTON_ANM_MAX] = {
  {//A
    SCRN_NO_POKEMON_A_0 - SCRN_NO_POKEMON_A_0,
    SCRN_NO_POKEMON_A_1 - SCRN_NO_POKEMON_A_0,
    SCRN_NO_POKEMON_A_2 - SCRN_NO_POKEMON_A_0,
  },
  {//B
    SCRN_NO_POKEMON_B_0 - SCRN_NO_POKEMON_B_0,
    SCRN_NO_POKEMON_B_1 - SCRN_NO_POKEMON_B_0,
    SCRN_NO_POKEMON_B_2 - SCRN_NO_POKEMON_B_0,
  },
  {//C
    SCRN_NO_POKEMON_C_0 - SCRN_NO_POKEMON_C_0,
    SCRN_NO_POKEMON_C_1 - SCRN_NO_POKEMON_C_0,
    SCRN_NO_POKEMON_C_2 - SCRN_NO_POKEMON_C_0,
  },
  {//D
    SCRN_NO_POKEMON_D_0 - SCRN_NO_POKEMON_D_0,
    SCRN_NO_POKEMON_D_1 - SCRN_NO_POKEMON_D_0,
    SCRN_NO_POKEMON_D_2 - SCRN_NO_POKEMON_D_0,
  },
  {//CANCEL
    SCRN_NO_POKEMON_CANCEL_0 - SCRN_NO_POKEMON_CANCEL_0,
    SCRN_NO_POKEMON_CANCEL_1 - SCRN_NO_POKEMON_CANCEL_0,
    SCRN_NO_POKEMON_CANCEL_2 - SCRN_NO_POKEMON_CANCEL_0,
  },
};

///ポケモン選択パネルを押した時のスクリーンアニメ書き換え範囲
ALIGN4 static const REWRITE_SCRN_RECT PokemonButtonScrnRect[] = {
  {0xa, 0x11, 0, 0xe},
  {0, 9, 0x11, 0x1f},
  {0xa, 0x11, 0x11, 0x1f},
  {0, 9, 0, 0xe},
  {0x12, 0x17, 1, 0x1e},
};

///ポケモン選択パネルを押せないパネルにする時のスクリーンオフセット値
ALIGN4 static const s16 PokemonButtonNotTouchScrnOffset[] = {
  SCRN_NO_NOT_TOUCH_0 - SCRN_NO_POKEMON_A_0 - SCRN_NO_NOT_TOUCH_HOSEI,    //A
  SCRN_NO_NOT_TOUCH_0 - SCRN_NO_POKEMON_B_0 - SCRN_NO_NOT_TOUCH_HOSEI,    //B
  SCRN_NO_NOT_TOUCH_0 - SCRN_NO_POKEMON_C_0 - SCRN_NO_NOT_TOUCH_HOSEI,    //C
  SCRN_NO_NOT_TOUCH_0 - SCRN_NO_POKEMON_D_0 - SCRN_NO_NOT_TOUCH_HOSEI,    //D
};

///ポケモン選択パネルを押せないパネルにする時のスクリーンアニメ書き換え範囲
ALIGN4 static const REWRITE_SCRN_RECT PokemonButtonNotTouchScrnRect[] = {
  {0xb, 0x11, 0, 0xe},
  {1, 9, 0x11, 0x1f},
  {0xb, 0x11, 0x11, 0x1f},
  {1, 9, 0, 0xe},
};

//--------------------------------------------------------------
//  「はい・いいえ」選択
//--------------------------------------------------------------
///スクリーン番号：はい・いいえ選択「いいえ」アニメ0、左上
#define SCRN_NO_NO_0    (0x246)
///スクリーン番号：はい・いいえ選択「いいえ」アニメ1、左上
#define SCRN_NO_NO_1    (0x24b)
///スクリーン番号：はい・いいえ選択「いいえ」アニメ2、左上
#define SCRN_NO_NO_2    (0x250)

///スクリーン番号：はい・いいえ選択「はい」アニメ0、左上
#define SCRN_NO_YES_0   (0x20)
///スクリーン番号：はい・いいえ選択「はい」アニメ1、左上
#define SCRN_NO_YES_1   (0xe0)
///スクリーン番号：はい・いいえ選択「はい」アニメ2、左上
#define SCRN_NO_YES_2   (0x1a0)

///はい・いいえ選択パネルを押した時のボタンアニメ用スクリーンオフセット値
ALIGN4 static const s16 YesNoButtonScrnOffset[][BUTTON_ANM_MAX] = {
  {//いいえ
    SCRN_NO_NO_0 - SCRN_NO_NO_0,
    SCRN_NO_NO_1 - SCRN_NO_NO_0,
    SCRN_NO_NO_2 - SCRN_NO_NO_0,
  },
  {//はい
    SCRN_NO_YES_0 - SCRN_NO_YES_0,
    SCRN_NO_YES_1 - SCRN_NO_YES_0,
    SCRN_NO_YES_2 - SCRN_NO_YES_0,
  },
};

///はい・いいえ選択パネルを押した時のスクリーンアニメ書き換え範囲
ALIGN4 static const REWRITE_SCRN_RECT YesNoButtonScrnRect[] = {
  {0xd, 0x14, 0, 0x1f},   //いいえ
  {4, 0xb, 0, 0x1f},      //はい
};

//--------------------------------------------------------------
//  「さいせい　ていし」ボタン
//--------------------------------------------------------------
///スクリーン番号：再生停止「ストップ」アニメ0、左上
#define SCRN_NO_STOP_0    (0x1d)
///スクリーン番号：再生停止「ストップ」アニメ1、左上
#define SCRN_NO_STOP_1    (0xdd)
///スクリーン番号：再生停止「ストップ」アニメ2、左上
#define SCRN_NO_STOP_2    (0x19d)

///はい・いいえ選択パネルを押した時のボタンアニメ用スクリーンオフセット値
ALIGN4 static const s16 StopButtonScrnOffset[BUTTON_ANM_MAX] = {
  SCRN_NO_STOP_0 - SCRN_NO_STOP_0,
  SCRN_NO_STOP_1 - SCRN_NO_STOP_0,
  SCRN_NO_STOP_2 - SCRN_NO_STOP_0,
};

///はい・いいえ選択パネルを押した時のスクリーンアニメ書き換え範囲
ALIGN4 static const REWRITE_SCRN_RECT StopButtonScrnRect[] = {
  {0x12, 0x17, 0, 0x1f},    //いいえ
};


//==============================================================================
//
//  枠ポジション
//
//==============================================================================
///コマンド選択画面での枠の登場座標
static const POINT_S16 CommandWakuPos[] = {
  {0x10 * 8, 9 * 8 + 4},    //SELECT_FIGHT_COMMAND
  {0x5 * 8, 0x14 * 8 + 4},    //SELECT_ITEM_COMMAND
  {0x1b * 8, 0x14 * 8 + 4},   //SELECT_POKEMON_COMMAND
  {0x10 * 8, 0x15 * 8},   //SELECT_ESCAPE_COMMAND
};

///技選択画面での枠の登場座標
static const POINT_S16 WazaWakuPos[] = {
  {0x10 * 8, 9 * 8 + 4},    //SELECT_SKILL_1
  {0x5 * 8, 0x14 * 8},    //SELECT_SKILL_2
  {0x1b * 8, 0x14 * 8},   //SELECT_SKILL_3
  {0x10 * 8, 0x15 * 8},   //SELECT_SKILL_4
};

///ポケモン選択画面での枠の登場座標
static const POINT_S16 PokemonWakuPos[] = {
  {0x10 * 8, 9 * 8 + 4},    //A
  {0x5 * 8, 0x14 * 8},    //B
  {0x1b * 8, 0x14 * 8},   //C
  {0x10 * 8, 0x15 * 8},   //D
};


//==============================================================================
//
//  その他のデータ
//
//==============================================================================
///表示BG毎の選択出来るポケモン　TRUE：選択出来る　FALSE：出来ない
ALIGN4 static const PokeSelectHitRange[][ BTL_CLIENT_MAX ] = {
  //CLIENT_TYPE_A, B, C, Dの順
  {TRUE,  TRUE, TRUE, TRUE},    //POKESELE_A_B_C_D
  {FALSE, TRUE, FALSE,  TRUE},    //POKESELE_BD
  {FALSE, TRUE, TRUE, TRUE},    //POKESELE_BCD
  {TRUE,  TRUE, TRUE, TRUE},    //POKESELE_ABCD
  {TRUE,  FALSE,  FALSE,  FALSE},   //POKESELE_A
  {TRUE,  FALSE,  TRUE, FALSE},   //POKESELE_AC
  {FALSE, FALSE,  TRUE, FALSE},   //POKESELE_C
  {TRUE,  TRUE, FALSE,  TRUE},    //POKESELE_ABD
  {FALSE, TRUE, TRUE, TRUE},    //POKESELE_B_C_D
  {TRUE,  TRUE, FALSE,  TRUE},    //POKESELE_A_B_D
  {TRUE,  FALSE,  TRUE, FALSE},   //POKESELE_A_C
  {FALSE, TRUE, FALSE,  TRUE},    //POKESELE_B_D
};

///枠タイプ毎のキャラIDなどのテーブル
ALIGN4 static const u32 WakuTypeID[][3] = { //char, cell, anm
  {
    BATTLE_W_WAKU1_NCGR_BIN,
    BATTLE_W_WAKU1_NCER,
    BATTLE_W_WAKU1_NANR,
  },
  {
    BATTLE_W_WAKU2_NCGR_BIN,
    BATTLE_W_WAKU2_NCER,
    BATTLE_W_WAKU2_NANR,
  },
  {
    BATTLE_W_WAKU3_NCGR_BIN,
    BATTLE_W_WAKU3_NCER,
    BATTLE_W_WAKU3_NANR,
  },
};

//--------------------------------------------------------------
//  地形ID毎に背景パレットを変更する
//--------------------------------------------------------------
///地形ID毎に背景パレットを変えない場合の指定
#define BACK_GROUND_PALETTE_NONE    (0xffff)

static const struct{
  u16 base_nclr_index;    ///<基本パレット
  u16 ani_nclr_index;     ///<アニメーション用パレット
}BackGroundPaletteID[] = {
  {//BG_ID_BASIC
    BATTLE_W_00_NCLR,
    BATTLE_W_00ANI_NCLR,
  },
  {//BG_ID_SEA
    BATTLE_W_01_NCLR,
    BATTLE_W_01ANI_NCLR,
  },
  {//BG_ID_CITY
    BATTLE_W_02_NCLR,
    BATTLE_W_02ANI_NCLR,
  },
  {//BG_ID_FOREST
    BATTLE_W_03_NCLR,
    BATTLE_W_03ANI_NCLR,
  },
  {//BG_ID_MOUNTAIN
    BATTLE_W_04_NCLR,
    BATTLE_W_04ANI_NCLR,
  },
  {//BG_ID_SNOW_MOUNTAIN
    BATTLE_W_05_NCLR,
    BATTLE_W_05ANI_NCLR,
  },
  {//BG_ID_ROOM_A
    BATTLE_W_06_NCLR,
    BATTLE_W_06ANI_NCLR,
  },
  {//BG_ID_ROOM_B
    BATTLE_W_07_NCLR,
    BATTLE_W_07ANI_NCLR,
  },
  {//BG_ID_ROOM_C
    BATTLE_W_08_NCLR,
    BATTLE_W_08ANI_NCLR,
  },
  {//BG_ID_CAVE_A
    BATTLE_W_09_NCLR,
    BATTLE_W_09ANI_NCLR,
  },
  {//BG_ID_CAVE_B
    BATTLE_W_10_NCLR,
    BATTLE_W_10ANI_NCLR,
  },
  {//BG_ID_CAVE_C
    BATTLE_W_11_NCLR,
    BATTLE_W_11ANI_NCLR,
  },
  {//BG_ID_BIG_FOUR_A
    BATTLE_W_12_NCLR,
    BATTLE_W_12ANI_NCLR,
  },
  {//BG_ID_BIG_FOUR_B
    BATTLE_W_13_NCLR,
    BATTLE_W_13ANI_NCLR,
  },
  {//BG_ID_BIG_FOUR_C
    BATTLE_W_14_NCLR,
    BATTLE_W_14ANI_NCLR,
  },
  {//BG_ID_BIG_FOUR_D
    BATTLE_W_15_NCLR,
    BATTLE_W_15ANI_NCLR,
  },
  {//BG_ID_CHAMPION
    BATTLE_W_16_NCLR,
    BATTLE_W_16ANI_NCLR,
  },
  {//BG_ID_HAKAI
    BATTLE_W_YAB_NCLR,
    BATTLE_W_YABANI_NCLR,
  },
  {//BG_ID_TOWER
    BACK_GROUND_PALETTE_NONE,
    BACK_GROUND_PALETTE_NONE,
  },
  {//BG_ID_FACTORY
    BACK_GROUND_PALETTE_NONE,
    BACK_GROUND_PALETTE_NONE,
  },
  {//BG_ID_ROULETTE
    BACK_GROUND_PALETTE_NONE,
    BACK_GROUND_PALETTE_NONE,
  },
  {//BG_ID_CASTLE
    BACK_GROUND_PALETTE_NONE,
    BACK_GROUND_PALETTE_NONE,
  },
  {//BG_ID_STAGE
    BACK_GROUND_PALETTE_NONE,
    BACK_GROUND_PALETTE_NONE,
  },
};


//==============================================================================
//
//
//
//==============================================================================

//--------------------------------------------------------------
/**
 * @brief   BIシステムワークの確保と初期化
 *
 * @retval  確保したBIシステムワークのポインタ
 */
//--------------------------------------------------------------
static void * BINPUT_WorkInit( HEAPID heapID )
{
  BI_PARAM *bip;
  int i;

  bip = GFL_HEAP_AllocMemory( heapID, sizeof( BI_PARAM ) );
  MI_CpuClear8( bip, sizeof( BI_PARAM ) );

  bip->heapID = heapID;
  bip->makedata_no = MAKEDATA_NO_INIT;
  bip->objplttID = GFL_CLGRP_REGISTER_FAILED;

  for( i = 0 ; i < BTL_CLIENT_MAX ; i++ ){
    bip->pokeicon_charID[ i ] = GFL_CLGRP_REGISTER_FAILED;
  }

  return bip;
}

//--------------------------------------------------------------
/**
 * @brief   BG構成を標準フレーム構成で設定する
 * @param   bgl   BGLデータ
 */
//--------------------------------------------------------------
void BINPUT_DefaultFrameSet( void )
{
  int i;

  for(i = 0; i < NELEMS(BiBgCntDat); i++){
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S + i, &BiBgCntDat[i], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreenCode( GFL_BG_FRAME0_S + i, BG_CLEAR_CODE );
    GFL_BG_SetScroll( GFL_BG_FRAME0_S + i, GFL_BG_SCROLL_X_SET, 0 );
    GFL_BG_SetScroll( GFL_BG_FRAME0_S + i, GFL_BG_SCROLL_Y_SET, 0 );
  }
}

//--------------------------------------------------------------
/**
 * @brief   BGフレーム終了処理
 * @param   bgl   BGLデータ
 */
//--------------------------------------------------------------
void BINPUT_FrameExit( void )
{
  int i;

  for(i = 0; i < NELEMS(BiBgCntDat); i++){
    GFL_BG_SetVisible( GFL_BG_FRAME0_S + i, VISIBLE_OFF );
    GFL_BG_FreeBGControl( GFL_BG_FRAME0_S + i );
  }
}

//--------------------------------------------------------------
/**
 * @brief   戦闘入力画面のシステムを初期化
 * @param   bgl   BGLデータ
 * @param   sex   性別(男女別壁紙の選択に必要)
 * @param   cursor_disp カーソルの初期表示状態
 * @retval  Allocした戦闘入力画面のシステムワークのポインタ
 */
//--------------------------------------------------------------
void * BINPUT_SystemInit( ARCHANDLE* hdl_bg, ARCHANDLE* hdl_obj,
//    BATTLE_WORK *bw,
    GFL_MSGDATA *msg,
    GFL_FONT *font,
    GFL_TCBSYS *tcbsys,
    PALETTE_FADE_PTR pfd,
    BATTLE_CURSOR_DISP * cursor_disp,
    int sex, HEAPID heapID )
{
  BI_PARAM *bip;
  int bg_id;

  //ワーク確保
  bip = BINPUT_WorkInit( heapID );
//  bip->bw = bw;
  bip->pfd = pfd;
#ifdef DEBUG_ONLY_FOR_sogabe
#warning メッセージは仮です
#endif
  bip->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_soga_dat, heapID );
  bip->font = font;
  bip->tcbsys = tcbsys;
  bip->sex = sex;
  bip->cursor_disp = cursor_disp;
  if(bip->cursor_disp != NULL){
    bip->decend_key = cursor_disp->on_off;
  #ifdef OSP_BINPUT_ON
    OS_TPrintf("cursor_disp = %d\n", bip->cursor_disp->on_off);
  #endif
  }
  else{
  #ifdef OSP_BINPUT_ON
    OS_TPrintf("カーソルなし\n");
  #endif
  }

  //背景IDは暫定で0
#if 0
  pfd = BattleWorkPfdGet(bw);
  bg_id = BattleWorkBGIDGet(bw);
  if(bg_id >= NELEMS(BackGroundPaletteID)){
    bg_id = 0;
    GF_ASSERT(0);
  }
#else
  bg_id = 0;
#endif

  //フレーム構成設定  ここでやるかは微妙？
//  BINPUT_DefaultFrameSet(bgl);

  //フォントOAMシステム作成
  bip->fontclunit = GFL_CLACT_UNIT_Create( FA_OBJ_MAX, 0, heapID );
  bip->fontoam_sys = BmpOam_Init( heapID, bip->fontclunit );

  //常駐フェードタスク生成
  bip->def_fade_tcb = GFL_TCB_AddTask( bip->tcbsys, DefaultFadeAnimeTask, bip, TCBPRI_BINPUT_DEF_COLOR_EFFECT );

  //スクリーン全読み込み
  {
    NNSG2dScreenData *scrnData;
    void *arc_data;
    int i;
    int scrn_data_id;

    for(i = 0; i < SCRNBUF_MAX; i++){
      //スクリーンバッファ確保
      bip->scrn_buf[ i ] = GFL_HEAP_AllocMemory( heapID, 0x800 );

      //スクリーン読み込み
#if 0
      if((BattleWorkFightTypeGet(bw) & FIGHT_TYPE_TOWER)
          && ScrnArcDataNo[i] == BATTLE_WBG0B_NSCR_BIN){
        scrn_data_id = BATTLE_WBG0B_BF_NSCR_BIN;
      }
      else{
        scrn_data_id = ScrnArcDataNo[i];
      }
#else
        scrn_data_id = ScrnArcDataNo[ i ];
#endif
      arc_data = GFL_ARC_UTIL_LoadScreen( ARCID_BATT_BG, scrn_data_id, 1, &scrnData, heapID );
      MI_CpuCopy32( scrnData->rawData, bip->scrn_buf[i], 0x800 );
      GFL_HEAP_FreeMemory( arc_data );
    }
  }

  //パレット読み込み
  {
    u16 *def_wk;
    int bg_data_id;

#if 0
    bg_data_id = (BattleWorkFightTypeGet(bw) & FIGHT_TYPE_TOWER) ? BATTLE_W_BF_NCLR : BATTLE_W_NCLR;
#else
    bg_data_id = BATTLE_W_NCLR;
#endif

    bip->pal_buf = GFL_HEAP_AllocMemory( heapID, 0x200 );
    PaletteWorkSet_Arc( bip->pfd, ARCID_BATT_BG, bg_data_id, heapID, FADE_SUB_BG, 0, 0);
    if(BackGroundPaletteID[bg_id].base_nclr_index != BACK_GROUND_PALETTE_NONE){
      PaletteWorkSet_Arc( bip->pfd, ARCID_BATT_BG,
        BackGroundPaletteID[bg_id].base_nclr_index, heapID, FADE_SUB_BG, 0x20, 0);
    }
    def_wk = PaletteWorkDefaultWorkGet( bip->pfd, FADE_SUB_BG );
    MI_CpuCopy16(def_wk, bip->pal_buf, 0x200);
  }

  //MEMORY_DECORD_WORK初期化
  {
    int i, client;
    for(client = 0; client < BTL_CLIENT_MAX; client++){
      MI_CpuFill8(&bip->memory_decord[client].wazapara, 0xff, sizeof(BINPUT_WAZA_PARAM));
    }
  }

  //技タイプアイコン用展開メモリ確保
  {
    int i, client;
    for(client = 0; client < BTL_CLIENT_MAX; client++){
      for(i = 0; i < PTL_WAZA_MAX; i++){
        bip->memory_decord[client].typeicon_cgx[i] = GFL_HEAP_AllocMemory( heapID,
          WAZATYPEICON_OAMSIZE );
      }
    }
  }

  //背景パレット
  {
    NNSG2dPaletteData *pal_data;
    void *arc_data;
    int ani_index;

    bip->background_pal_normal = GFL_HEAP_AllocMemory( heapID, 0x40 );
    bip->background_pal_touch = GFL_HEAP_AllocMemory( heapID, 0x40 );
    MI_CpuCopy16(bip->pal_buf, bip->background_pal_normal, 0x20);
    MI_CpuCopy16(&bip->pal_buf[FRONTIER_PALANM_POS * 16],
      &bip->background_pal_normal[16*1], 0x20);

#if 0
    if(BattleWorkFightTypeGet(bw) & FIGHT_TYPE_TOWER){
      ani_index = BATTLE_W_BFANI_NCLR;
    }
    else if(BackGroundPaletteID[bg_id].ani_nclr_index != BACK_GROUND_PALETTE_NONE){
      ani_index = BackGroundPaletteID[bg_id].ani_nclr_index;
    }
    else{
      ani_index = BATTLE_W_00ANI_NCLR;  //BATTLE_W_ANI_NCLR;
    }
#else
    if(BackGroundPaletteID[bg_id].ani_nclr_index != BACK_GROUND_PALETTE_NONE){
      ani_index = BackGroundPaletteID[bg_id].ani_nclr_index;
    }
    else{
      ani_index = BATTLE_W_00ANI_NCLR;  //BATTLE_W_ANI_NCLR;
    }
#endif
    arc_data = GFL_ARC_UTIL_LoadPalette( ARCID_BATT_BG, ani_index, &pal_data, heapID );
#if 0
    if(BattleWorkFightTypeGet(bw) & FIGHT_TYPE_TOWER){
      MI_CpuCopy16(pal_data->pRawData, bip->background_pal_touch, 0x40);
    }
    else{
      MI_CpuCopy16(pal_data->pRawData, bip->background_pal_touch, 0x20);
    }
#else
    MI_CpuCopy16(pal_data->pRawData, bip->background_pal_touch, 0x20);
#endif
    GFL_HEAP_FreeMemory( arc_data );

    //戦闘用のスキップ、メッセージ送り用のフラグ操作（現状はないはず）
    //MsgPrintBattleSkipPushFlagClear();
    //MsgPrintBattleWaitPushFlagClear();
    bip->background_tcb = GFL_TCB_AddTask( bip->tcbsys, BackGroundTouchPaletteCheck, bip, TCBPRI_BACKGROUND_PAL );
  }

  //技タイプアイコン
  bip->wazatype_clunit = GFL_CLACT_UNIT_Create( PTL_WAZA_MAX, 0, bip->heapID );

  //ポケモンアイコン
  bip->pokeicon_clunit = GFL_CLACT_UNIT_Create( BTL_CLIENT_MAX, 0, bip->heapID );

  return bip;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘入力画面のシステムを解放する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
void BINPUT_SystemFree(BI_PARAM_PTR bip)
{
  int i;

  if(bip->cursor_disp != NULL){
    bip->cursor_disp->on_off = bip->decend_key;
  #ifdef OSP_BINPUT_ON
    OS_TPrintf("cursor_disp = %d\n", bip->cursor_disp->on_off);
  #endif
  }

#ifdef DEBUG_ONLY_FOR_sogabe
#warning メッセージは仮です
#endif
  GFL_MSG_Delete( bip->msg );

  Sub_SceneOBJDelete(bip);
//  BINPUT_StockBallActorResourceFree(bip);

  BINPUT_DefaultDataFree(bip);

  BmpOam_Exit( bip->fontoam_sys );

  //生成したセルユニットの解放
  GFL_CLACT_UNIT_Delete( bip->fontclunit );
  GFL_CLACT_UNIT_Delete( bip->wazatype_clunit );
  GFL_CLACT_UNIT_Delete( bip->pokeicon_clunit );

  GFL_TCB_DeleteTask( bip->def_fade_tcb );

  for(i = 0; i < SCRNBUF_MAX; i++){
    GFL_HEAP_FreeMemory( bip->scrn_buf[ i ] );
  }
  GFL_HEAP_FreeMemory( bip->pal_buf );

  MemoryDecordWorkFree(bip);

  GFL_HEAP_FreeMemory( bip->background_pal_normal );
  GFL_HEAP_FreeMemory( bip->background_pal_touch );
  GFL_TCB_DeleteTask( bip->background_tcb );

  GFL_HEAP_FreeMemory( bip );
}

//--------------------------------------------------------------
/**
 * @brief   常駐キャラの展開などを行う
 *
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
void BINPUT_DefaultDataSet(BI_PARAM_PTR bip)
{
  int i;
#if 0
  CATS_SYS_PTR csp;
  CATS_RES_PTR crp;
  PALETTE_FADE_PTR pfd;

  csp = BattleWorkCATS_SYS_PTRGet(bip->bw);
  crp = BattleWorkCATS_RES_PTRGet(bip->bw);
  bgl = BattleWorkGF_BGL_INIGet(bip->bw);
  pfd = BattleWorkPfdGet(bip->bw);
#endif

  //キャラデータVRAM転送
  {
    int cgr_data_id;

#if 0
    if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_TOWER){
      cgr_data_id = BATTLE_W_BF_NCGR_BIN;
    }
    else{
      cgr_data_id = BATTLE_W_NCGR_BIN;
    }
#else
    cgr_data_id = BATTLE_W_NCGR_BIN;
#endif
    GFL_ARC_UTIL_TransVramBgCharacter( ARCID_BATT_BG, cgr_data_id, GFL_BG_FRAME0_S,
      0, 0x6000, 1, bip->heapID );
  }

  //技タイプアイコン
  {
    ARCHANDLE     *hdl;

    hdl = GFL_ARC_OpenDataHandle( WazaTypeIcon_ArcIDGet(), bip->heapID );
    bip->wazatype_cellID = GFL_CLGRP_CELLANIM_Register( hdl, WazaTypeIcon_CellIDGet(), WazaTypeIcon_CellAnmIDGet(), bip->heapID );
    bip->wazatype_plttID = GFL_CLGRP_PLTT_Register( hdl, WazaTypeIcon_PlttIDGet(), CLSYS_DRAW_SUB, 0x20 * 3, bip->heapID );
    PaletteWorkSet_VramCopy( bip->pfd, FADE_SUB_OBJ,
                             GFL_CLGRP_PLTT_GetAddr( bip->wazatype_plttID, CLSYS_DRAW_SUB ) / 2, 0x20 * 3 );
    for(i = 0; i < PTL_WAZA_MAX; i++){
      bip->wazatype_charID[ i ] = GFL_CLGRP_CGR_Register( hdl, WazaTypeIcon_CgrIDGet( POKETYPE_NORMAL ), FALSE,
                             CLSYS_DRAW_SUB, bip->heapID );
    }
    GFL_ARC_CloseDataHandle( hdl );
  }

  //捕獲デモ用指カーソル
#if 0
  if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_GET_DEMO){
    FINGER_ResourceLoad(csp, crp, HEAPID_BATTLE, pfd, CHARID_FINGER_CURSOR, PLTTID_FINGER_CURSOR,
      CELLID_FINGER_CURSOR, CELLANMID_FINGER_CURSOR);

    bip->demo.finger = FINGER_ActorCreate(csp, crp, HEAPID_BATTLE,
      CHARID_FINGER_CURSOR, PLTTID_FINGER_CURSOR, CELLID_FINGER_CURSOR,
      CELLANMID_FINGER_CURSOR, FINGER_SOFTPRI, FINGER_BGPRI);
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   常駐キャラの破棄
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
void BINPUT_DefaultDataFree(BI_PARAM_PTR bip)
{
  int i;

  //-- 技タイプアイコン --//
  for(i = 0; i < PTL_WAZA_MAX; i++){
    GFL_CLGRP_CGR_Release( bip->wazatype_charID[ i ] );
  }
  GFL_CLGRP_PLTT_Release( bip->wazatype_plttID );
  GFL_CLGRP_CELLANIM_Release( bip->wazatype_cellID );

#if 0
  //捕獲デモ用指カーソル
  if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_GET_DEMO){
    FINGER_ActorDelete(bip->demo.finger);
    FINGER_ResourceFree(crp,
      CHARID_FINGER_CURSOR, PLTTID_FINGER_CURSOR,
      CELLID_FINGER_CURSOR, CELLANMID_FINGER_CURSOR);
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   指定タイプのBG画面を形成する
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 * @param   work      シーン毎に異なる必要なデータへのポインタ
 */
//--------------------------------------------------------------
void BINPUT_CreateBG(ARCHANDLE* hdl_bg, ARCHANDLE* hdl_obj, BI_PARAM_PTR bip, int select_bg, int force_put, void *scene_work)
{
  const BG_MAKE_DATA *bmd, *old_bmd;
  int i;

  if(scene_work != NULL){
    MI_CpuCopy8(scene_work, &bip->scene, sizeof(SCENE_WORK));
  }
  bip->touch_invalid = FALSE;

  if(bip->makedata_no == MAKEDATA_NO_INIT){
    force_put = TRUE;
    old_bmd = NULL;
  }
  else{
    old_bmd = &BgMakeData[bip->makedata_no];
  }
  bmd = &BgMakeData[select_bg];

  //背景以外は全て非表示
//  Sub_BackScrnOnlyVisible();

  //CGR
#if 1
  if(0){  //キャラデータは書き換わる事がないので何もしない
#else
  if(bmd->cgr_id != NONE_ID && (force_put == TRUE || bmd->cgr_id != old_bmd->cgr_id)){
#endif
    int cgr_data_id;

#if 0
    if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_TOWER){
      cgr_data_id = BATTLE_W_BF_NCGR_BIN;
    }
    else{
      cgr_data_id = bmd->cgr_id;
    }
#else
    cgr_data_id = bmd->cgr_id;
#endif
    GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl_bg, cgr_data_id, GFL_BG_FRAME0_S, 0, BI_BG_CGR_SIZE, TRUE, bip->heapID );
    if(bip->sex == PM_MALE){
      GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl_bg, BATTLE_W_BOY_NCGR_BIN, GFL_BG_FRAME0_S,
                          BG_PLAYER_WALL_OFFSET, BG_PLAYER_WALL_SIZE, TRUE, bip->heapID );
    }
    else{
      GFL_ARCHDL_UTIL_TransVramBgCharacter( hdl_bg, BATTLE_W_GIRL_NCGR_BIN, GFL_BG_FRAME0_S,
                          BG_PLAYER_WALL_OFFSET, BG_PLAYER_WALL_SIZE, TRUE, bip->heapID );
    }
  }
  //PALETTE
#if 1
  if(1){
#else
  if(bmd->pal_id != NONE_ID && (force_put == TRUE || bmd->pal_id != old_bmd->pal_id)){
#endif
    PaletteWorkSet( bip->pfd, bip->pal_buf, FADE_SUB_BG, 0, 0x200 );
  }
  //SCREEN
  for(i = 0; i < BI_BG_NUM; i++){
    if(bmd->scr_id[i] != NONE_ID
        && (force_put == TRUE || bmd->scr_id[i] != old_bmd->scr_id[i])){
      GFL_BG_LoadScreenBuffer( GFL_BG_FRAME0_S + i, bip->scrn_buf[bmd->scr_id[i]], 0x800 );
      GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S + i );
    }
  }

  //OBJパレット転送
  //※check 最初の1回だけの登録だし、とりあえずここで 2006.05.09(火)
//  bip->objplttID = GFL_CLGRP_PLTT_Register( hdl_obj, BATTLE_WOBJ_NCLR, CLSYS_DRAW_SUB, 0, bip->heapID );
  bip->objplttID = GFL_CLGRP_PLTT_RegisterComp( hdl_obj, BATTLE_WOBJ_NCLR, CLSYS_DRAW_SUB, 0, bip->heapID );
  PaletteWorkSet_VramCopy( bip->pfd, FADE_SUB_OBJ,
               GFL_CLGRP_PLTT_GetAddr( bip->objplttID, CLSYS_DRAW_SUB ) / 2, 0x20 * 3 );
  bip->makedata_no = select_bg;

  //ブレンド設定
  G2S_SetBlendAlpha(BLD_PLANE_1, BLD_PLANE_2, BLD_ALPHA_1, BLD_ALPHA_2);

  Sub_SceneOBJDelete(bip);
  //コールバック関数呼び出し
  if(bmd->callback_bg != NULL){
    bmd->callback_bg(bip, select_bg, force_put);
  }

  GFUser_VIntr_CreateTCB( VWait_FrameVisibleUpdate, bip, VWAIT_TCBPRI_FRAMEVISIBLE );
}

//--------------------------------------------------------------
/**
 * @brief   手持ちボールで使用するリソースのロードとアクターの生成
 *
 * @param   bip   BIシステムワークへのポインタ
 *
 * この関数でアクター生成後、アクターは表示OFFの状態になっています。
 */
//--------------------------------------------------------------
void BINPUT_StockBallActorResourceLoad(ARCHANDLE* hdl, BI_PARAM_PTR bip)
{
  int i;

  GF_ASSERT(bip->stock_mine_cap[0] == NULL && bip->stock_enemy_cap[0] == NULL);

  //-- リソース読み込み --//
  //パレット　フォントOBJと共通(入力画面は1つのファイルに全部入ってる)のでここではしなくていいか
//  CATS_LoadResourcePlttArc(csp, crp, ARC_BATT_OBJ, BATTLE_WOBJ_NCLR, 0,
//    1, NNS_G2D_VRAM_TYPE_2DSUB, PLTTID_INPUT_FONTOBJ_COMMON);

  //自機側
  bip->sb_charID[ 0 ] = GFL_CLGRP_CGR_Register( hdl, BATTLE_STOCK_M_NCGR_BIN, TRUE, CLSYS_DRAW_SUB, bip->heapID );
//  bip->sb_cellID[ 0 ] = GFL_CLGRP_CELLANIM_Register( hdl, BATTLE_STOCK_M_NCER_BIN, BATTLE_STOCK_M_NANR_BIN, bip->heapID );
  bip->sb_cellID[ 0 ] = GFL_CLGRP_CELLANIM_Register( hdl, BATTLE_STOCK_M_NCER, BATTLE_STOCK_M_NANR, bip->heapID );

  //敵側
  bip->sb_charID[ 1 ] = GFL_CLGRP_CGR_Register( hdl, BATTLE_STOCK_E_NCGR_BIN, TRUE, CLSYS_DRAW_SUB, bip->heapID );
//  bip->sb_cellID[ 1 ] = GFL_CLGRP_CELLANIM_Register( hdl, BATTLE_STOCK_E_NCER_BIN, BATTLE_STOCK_E_NANR_BIN, bip->heapID );
  bip->sb_cellID[ 1 ] = GFL_CLGRP_CELLANIM_Register( hdl, BATTLE_STOCK_E_NCER, BATTLE_STOCK_E_NANR, bip->heapID );

  //-- アクター生成 --//
  bip->stock_clunit = GFL_CLACT_UNIT_Create( BTL_PARTY_MEMBER_MAX * 2, 0, bip->heapID );
  for(i = 0; i < BTL_PARTY_MEMBER_MAX; i++){
    GFL_CLACTPOS  pos;
    bip->stock_mine_cap[ i ] = GFL_CLACT_WK_Create( bip->stock_clunit,
                         bip->sb_charID[ 0 ], bip->objplttID, bip->sb_cellID[ 0 ],
                         &StockObjParam_Mine, CLSYS_DEFREND_SUB, bip->heapID );
    pos.x = STOCK_MINE_BASE_POS_X + STOCK_MINE_BASE_SPACE_X * i;
    pos.y = STOCK_MINE_BASE_POS_Y;
    GFL_CLACT_WK_SetPos( bip->stock_mine_cap[i], &pos, CLSYS_DEFREND_SUB );
    GFL_CLACT_WK_SetAffineParam( bip->stock_mine_cap[i], CLSYS_AFFINETYPE_NORMAL );

    bip->stock_enemy_cap[ i ] = GFL_CLACT_WK_Create( bip->stock_clunit,
                         bip->sb_charID[ 1 ], bip->objplttID, bip->sb_cellID[ 1 ],
                         &StockObjParam_Enemy, CLSYS_DEFREND_SUB, bip->heapID );
    pos.x = STOCK_ENEMY_BASE_POS_X + STOCK_ENEMY_BASE_SPACE_X * i;
    pos.y = STOCK_ENEMY_BASE_POS_Y;
    GFL_CLACT_WK_SetPos( bip->stock_enemy_cap[i], &pos, CLSYS_DEFREND_SUB );
  }

  BINPUT_StockBallOFF(bip);

  GF_ASSERT(bip->ball_tcb == NULL);
  bip->ball_tcb = GFL_TCB_AddTask( bip->tcbsys, StockBallMain, bip, TCBPRI_BINPUT_EFFECT);


  //-- ついでにここで戦闘カーソルも生成 --//
  {
    bip->cursor_res = BCURSOR_ResourceLoad( bip->pfd, bip->heapID );
    bip->cursor = BCURSOR_ActorCreate( bip->cursor_res, bip->heapID, SOFTPRI_CURSOR, BGPRI_CURSOR );
  }
}

//--------------------------------------------------------------
/**
 * @brief   手持ちボールのリソースとアクターを解放
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void BINPUT_StockBallActorResourceFree(BI_PARAM_PTR bip)
{
  int i;

  GF_ASSERT(bip->stock_mine_cap[0] != NULL && bip->stock_enemy_cap[0] != NULL);

  //-- リソース解放 --//
  for( i = 0 ; i < 2 ; i++ ){
    GFL_CLGRP_CGR_Release( bip->sb_charID[ i ] );
    GFL_CLGRP_CELLANIM_Release( bip->sb_cellID[ i ] );
  }

  //共通パレットも一応ここで削除
  GFL_CLGRP_PLTT_Release( bip->objplttID );

  //-- アクター削除 --//
  for(i = 0; i < BTL_PARTY_MEMBER_MAX; i++){
    GFL_CLACT_WK_Remove( bip->stock_mine_cap[ i ] );
    GFL_CLACT_WK_Remove( bip->stock_enemy_cap[ i ] );
  }
  GFL_CLACT_UNIT_Delete( bip->stock_clunit );

  GFL_TCB_DeleteTask( bip->ball_tcb );
  bip->ball_tcb = NULL;

  //-- 戦闘カーソル --//
  BCURSOR_ResourceFree( bip->cursor_res );
  BCURSOR_ActorDelete( bip->cursor );
}

//--------------------------------------------------------------
/**
 * @brief   手持ちやる気アニメ：メインタスク
 * @param   tcb     TCBへのポインタ
 * @param   work    BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void StockBallMain(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  int i;
  MOTIVATION_WORK *mw;
  enum{
    SEQ_INIT,   //初期化
    SEQ_WAIT,   //ウェイト
    SEQ_FIRST,    //行き
    SEQ_BACK,   //戻り
    SEQ_RET,    //元の位置へ
  };

#if 0 //SEQ_INITのアクター表示、非表示で判定するように変更 2006.07.13(木)
    //でないと途中動作のまま次画面で再開してしまう
  switch(bip->makedata_no){
  case BINPUT_COMMAND_IN:
  case BINPUT_COMMAND_IN_2:
  case BINPUT_TYPE_A:
  case BINPUT_TYPE_A_2:
    break;
  default:
    return;
  }
#endif

  for(i = 0; i < BTL_PARTY_MEMBER_MAX; i++){
    mw = &bip->motivation_work[i];
    switch(mw->seq){
    case SEQ_INIT:
    default:
      if( GFL_CLACT_WK_GetDrawEnable( bip->stock_mine_cap[i] ) == FALSE ){
        break;
      }
    #if 0
      if(mw->motivation >= MOTIVATION_ANM_HIGH){
        mw->anm_type = MOTIVATION_ANM_TYPE_HIGH;
        mw->wait = MOTIVATION_WAIT_HIGH;
      }
      else if(mw->motivation >= MOTIVATION_ANM_MIDDLE){
        mw->anm_type = MOTIVATION_ANM_TYPE_MIDDLE;
        mw->wait = MOTIVATION_WAIT_MIDDLE;
      }
      else if(mw->motivation >= MOTIVATION_ANM_LOW){
        mw->anm_type = MOTIVATION_ANM_TYPE_LOW;
        mw->wait = MOTIVATION_WAIT_LOW;
      }
    #else
      if(mw->motivation >= MOTIVATION_ANM_MIDDLE){
        mw->anm_type = MOTIVATION_ANM_TYPE_HIGH;
        mw->wait = MOTIVATION_WAIT_HIGH;
      }
    #endif
      else{
        break;
      }
      mw->wait += GFL_STD_MtRand0( MOTIVATION_ANM_WAIT_RANDOM );
      mw->count_max = MOTIVATION_ANM_FRAME;
      mw->seq++;
      break;
    case SEQ_WAIT:
      if(mw->wait > 0){
        mw->wait--;
        break;
      }
      mw->seq++;
      break;
    case SEQ_FIRST:
    case SEQ_RET:
      {
        u16       rotation;
        GFL_CLACTPOS  pos;

        rotation = GFL_CLACT_WK_GetRotation( bip->stock_mine_cap[i] );
        rotation += MOTIVATION_ADD_ROTATION;
        GFL_CLACT_WK_SetRotation( bip->stock_mine_cap[i], rotation );

        mw->x_offset += MOTIVATION_ADD_X;
        pos.x = STOCK_MINE_BASE_POS_X + STOCK_MINE_BASE_SPACE_X * i + mw->x_offset / 0x100;
        pos.y = STOCK_MINE_BASE_POS_Y;
        GFL_CLACT_WK_SetPos( bip->stock_mine_cap[i], &pos, CLSYS_DEFREND_SUB );

        mw->count++;
        if(mw->count >= mw->count_max){
          mw->count = 0;
          if(mw->seq == SEQ_FIRST){
            mw->seq++;
          }
          else{
            GFL_CLACT_WK_SetRotation( bip->stock_mine_cap[i], 0 );
            pos.x = STOCK_MINE_BASE_POS_X + STOCK_MINE_BASE_SPACE_X * i;
            pos.y = STOCK_MINE_BASE_POS_Y;
            GFL_CLACT_WK_SetPos( bip->stock_mine_cap[i], &pos, CLSYS_DEFREND_SUB );
            mw->x_offset = 0;

            if(mw->loop < MOTIVATION_ANM_LOOP){
              mw->loop++;
              if(mw->count_max > 1){
                mw->count_max--;
              }
              mw->seq = SEQ_FIRST;
            }
            else{
              mw->loop = 0;
              mw->seq = SEQ_INIT;
            }
          }
        }
      }
      break;
    case SEQ_BACK:
      {
        u16       rotation;
        GFL_CLACTPOS  pos;

        rotation = GFL_CLACT_WK_GetRotation( bip->stock_mine_cap[i] );
        rotation -= MOTIVATION_ADD_ROTATION;
        GFL_CLACT_WK_SetRotation( bip->stock_mine_cap[i], rotation );

        mw->x_offset -= MOTIVATION_ADD_X;
        pos.x = STOCK_MINE_BASE_POS_X + STOCK_MINE_BASE_SPACE_X * i + mw->x_offset / 0x100;
        pos.y = STOCK_MINE_BASE_POS_Y;
        GFL_CLACT_WK_SetPos( bip->stock_mine_cap[i], &pos, CLSYS_DEFREND_SUB );

        mw->count++;
        if(mw->count >= mw->count_max * 2){
          mw->count = 0;
          mw->seq++;
        }
      }
      break;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   手持ちポケモンのモチベーションパラメータをセットする
 * @param   bip       BIシステムワークへのポインタ
 * @param   motivation    手持ち分のモチベーションが入った配列へのポインタ
 */
//--------------------------------------------------------------
void BINPUT_TemotiMotivationParamSet(BI_PARAM_PTR bip, const u8 *motivation)
{
  int i;

  for(i = 0; i < BTL_PARTY_MEMBER_MAX; i++){
    bip->motivation_work[i].motivation = motivation[i];
  }
}

//--------------------------------------------------------------
/**
 * @brief   手持ちボールのアニメを最新の情報をもらって更新する
 *
 * @param   bip         BIシステムワークへのポインタ
 * @param   stock_mine[]    自機側の手持ちボール情報
 * @param   stock_enemy[]   敵側の手持ちボール情報
 */
//--------------------------------------------------------------
void BINPUT_StockBallAnimeUpdate(BI_PARAM_PTR bip, u8 stock_mine[], u8 stock_enemy[])
{
  int i, anm_seq;

  for(i = 0; i < BTL_PARTY_MEMBER_MAX; i++){
    //自機
    anm_seq = Sub_StockBallAnmSeqGet(stock_mine[i]);
    GFL_CLACT_WK_SetAnmSeq( bip->stock_mine_cap[i], anm_seq );
    GFL_CLACT_WK_SetAnmFrame( bip->stock_mine_cap[i], FX32_ONE );
    //敵
    anm_seq = Sub_StockBallAnmSeqGet(stock_enemy[i]);
    GFL_CLACT_WK_SetAnmSeq( bip->stock_enemy_cap[i], anm_seq );
    GFL_CLACT_WK_SetAnmFrame( bip->stock_enemy_cap[i], FX32_ONE );
  }
}

//--------------------------------------------------------------
/**
 * @brief   手持ちボールを表示する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
void BINPUT_StockBallON(BI_PARAM_PTR bip)
{
  int i;

  GF_ASSERT(bip->stock_mine_cap[0] != NULL && bip->stock_enemy_cap[0] != NULL);

  for(i = 0; i < BTL_PARTY_MEMBER_MAX; i++){
    GFL_CLACT_WK_SetDrawEnable( bip->stock_mine_cap[i], TRUE );
  }

#if 0
  if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_TRAINER){
    for(i = 0; i < POKEMON_TEMOTI_MAX; i++){
      CATS_ObjectEnable(bip->stock_enemy_cap[i]->act, CATS_ENABLE_TRUE);
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   手持ちボールの表示を消す
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
void BINPUT_StockBallOFF(BI_PARAM_PTR bip)
{
  int i;

  GF_ASSERT(bip->stock_mine_cap[0] != NULL && bip->stock_enemy_cap[0] != NULL);

  for(i = 0; i < BTL_PARTY_MEMBER_MAX; i++){
    GFL_CLACT_WK_SetDrawEnable( bip->stock_mine_cap[i], FALSE );
    GFL_CLACT_WK_SetDrawEnable( bip->stock_enemy_cap[i], FALSE );
  }
}

//--------------------------------------------------------------
/**
 * @brief   タッチパネル状態を調べる
 * @param   bip   BIシステムワークへのポインタ
 * @retval  結果(当たりなしの場合はGFL_UI_TP_HIT_NONE)
 */
//--------------------------------------------------------------
int BINPUT_TouchCheck(BI_PARAM_PTR bip)
{
  int tp_ret, hit, ret, panel_pal;
  const BG_MAKE_DATA *bmd;
  int cursor_check = 0;

  GF_ASSERT(bip->makedata_no != MAKEDATA_NO_INIT);

  bmd = &BgMakeData[bip->makedata_no];
  if(bmd->tpd == NULL || bip->touch_invalid == TRUE){
    return GFL_UI_TP_HIT_NONE;
  }

  GF_ASSERT(bmd->tpd_ret != NULL);


#if 0
  //捕獲デモチェック
  if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_GET_DEMO){
    hit = CaptureDemo_Main(bip);
  }
  else{ //通常処理
    hit = GFL_UI_TP_HitTrg(bmd->tpd);
    if(hit == GFL_UI_TP_HIT_NONE){
      hit = CursorCheck(bip);
      cursor_check++;
    }
  }
#else
  hit = GFL_UI_TP_HitTrg(bmd->tpd);
  if(hit == GFL_UI_TP_HIT_NONE){
    hit = CursorCheck(bip);
    cursor_check++;
  }
#endif
  if(hit == GFL_UI_TP_HIT_NONE){
    tp_ret = GFL_UI_TP_HIT_NONE;
    panel_pal = 0xff;
  }
  else{
    tp_ret = bmd->tpd_ret[hit];
    panel_pal = bmd->tpd_pal[hit];
  }

  //とりあえずデバッグ機能はいらない
#if 0
#ifdef PM_DEBUG
  {
    int debug_tp_ret;

    //デバッグ機能でコマンド制御
    if(BattleWorkBattleStatusFlagGet(bip->bw)&BATTLE_STATUS_FLAG_COM_SELECT){
      debug_tp_ret=DebugFightCommandBufferGet();
      if(debug_tp_ret){
        tp_ret=debug_tp_ret;
      }
    }
  }
#endif PM_DEBUG
#endif

  //コールバック関数呼び出し
  if(bmd->callback_tp != NULL){
    tp_ret = bmd->callback_tp(bip, tp_ret, panel_pal);
    if( tp_ret != GFL_UI_TP_HIT_NONE ){
      if(bmd->cursor_save_func != NULL){
        bmd->cursor_save_func(bip, hit);
      }
      MI_CpuClear8(&bip->cursor_move, sizeof(CURSOR_MOVE));
      BCURSOR_OFF(bip->cursor);
      if(cursor_check > 0){
        bip->decend_key = TRUE;
      }
      else{
        bip->decend_key = FALSE;
      }
    }
  }

  return tp_ret;
}

//--------------------------------------------------------------
/**
 * @brief   エフェクトTCBが終了しているかチェック
 * @param   bip   BIシステムワークへのポインタ
 * @retval  TRUE:終了している。　FALSE:終了していない
 */
//--------------------------------------------------------------
BOOL BINPUT_EffectEndCheck(BI_PARAM_PTR bip)
{
  if(bip->effect_tcb == NULL && bip->color_eff_tcb == NULL
      && BINPUT_BackFadeExeCheck(bip) == TRUE){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   技効果範囲から表示するBGタイプを取得する
 * @param   range   技効果範囲
 * @param   client_type クライアントタイプ
 * @retval  BGタイプ(POKESELE_???)
 */
//--------------------------------------------------------------
int BINPUT_RangeBgTypeGet(int range, int client_type)
{
  switch(range){
  case RANGE_NORMAL:
    return (client_type == CLIENT_TYPE_C) ? POKESELE_A_B_D : POKESELE_B_C_D;
  case RANGE_NONE:
  case RANGE_RANDOM:
  case RANGE_MINE:
    return (client_type == CLIENT_TYPE_C) ? POKESELE_C : POKESELE_A;
  case RANGE_DOUBLE:
  case RANGE_MAKIBISHI:
    return POKESELE_BD;
  case RANGE_TRIPLE:
    return (client_type == CLIENT_TYPE_C) ? POKESELE_ABD : POKESELE_BCD;
  case RANGE_TEDASUKE:
    return (client_type == CLIENT_TYPE_C) ? POKESELE_A : POKESELE_C;
  case RANGE_FIELD:
    return POKESELE_ABCD;
  case RANGE_TUBOWOTUKU:
    return POKESELE_A_C;
  case RANGE_MINE_DOUBLE:
    return POKESELE_AC;
  case RANGE_SAKIDORI:
    return POKESELE_B_D;
  default:
    GF_ASSERT(0);// && "存在しないレンジタイプです");
    return POKESELE_A_B_C_D;
  }
}

//--------------------------------------------------------------
/**
 * @brief   壁紙のスクロール座標Xを設定する
 *
 * @param   bip   BIシステムワークへのポインタ
 * @param   x   座標X(整数)
 */
//--------------------------------------------------------------
void BINPUT_PlayerBG_SetX(BI_PARAM_PTR bip, int x)
{
  bip->wall_x = x * 0x100;
  GFL_BG_SetScroll( BI_FRAME_BACK, GFL_BG_SCROLL_X_SET, x );
  BINPUT_PlayerBG_MaskWindow(bip);
}

//--------------------------------------------------------------
/**
 * @brief   現在の壁紙の位置によってウィンドウのマスク処理をかける
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
void BINPUT_PlayerBG_MaskWindow(BI_PARAM_PTR bip)
{
  int x1;

  GXS_SetVisibleWnd(GX_WNDMASK_W0);
  G2S_SetWndOutsidePlane(GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2
    | GX_WND_PLANEMASK_BG3| GX_WND_PLANEMASK_OBJ, TRUE);

  G2S_SetWnd0InsidePlane(GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1
    | GX_WND_PLANEMASK_BG3| GX_WND_PLANEMASK_OBJ, TRUE);

  x1 = 255 - (bip->wall_x / 0x100);
  if(x1 < 0){
    x1 = 0;
  }
  G2S_SetWnd0Position(x1, 0, 255, 192);
}

//--------------------------------------------------------------
/**
 * @brief   壁紙をスクロールさせる
 * @param   bip     BIシステムワークへのポインタ
 * @param   sp_x    スクロール速度(下位8ビット小数)
 * @param   end_x   スクロール停止座標X(整数)
 */
//--------------------------------------------------------------
void BINPUT_PlayerBG_ScrollX(BI_PARAM_PTR bip, int sp_x, int end_x)
{
  if(bip->wall_x == end_x * 0x100){
    return;   //既に停止座標に到達している
  }

  bip->wall_sp_x = sp_x;
  bip->wall_end_x = end_x << 8;
  GFL_TCB_AddTask( bip->tcbsys, PlayerBGScrollX, bip, TCBPRI_BINPUT_PLAYER_SCROLL_EFFECT );
}

//--------------------------------------------------------------
/**
 * @brief   壁紙スクロールタスク
 *
 * @param   tcb     TCBへのポインタ
 * @param   work    BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void PlayerBGScrollX(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  int end = FALSE;

  bip->wall_x += bip->wall_sp_x;
  if((bip->wall_sp_x <= 0 && bip->wall_x <= bip->wall_end_x) ||
      (bip->wall_sp_x > 0 && bip->wall_x >= bip->wall_end_x)){
    bip->wall_x = bip->wall_end_x;
    end = TRUE;
  }
  GFL_BG_SetScroll( BI_FRAME_BACK, GFL_BG_SCROLL_X_SET, bip->wall_x / 0x100 );
  BINPUT_PlayerBG_MaskWindow(bip);

  if(end == TRUE){
    GFL_BG_ClearScreenCode( BI_FRAME_EFF, BG_CLEAR_CODE );  //エフェクト面クリア
    GFL_BG_SetVisible( BI_FRAME_EFF, VISIBLE_OFF );
    GFL_BG_SetPriority( BI_FRAME_EFF, 0 );
    GXS_SetVisibleWnd(GX_WNDMASK_NONE);
    GFL_TCB_DeleteTask(tcb);
    return;
  }
}

//--------------------------------------------------------------
/**
 * @brief   エフェクト面に対して壁紙の絵を設定する
 *
 * @param   bip   BIシステムワークへのポインタ
 *
 * 背景の主人公グラフィックスクロール時の、マスクされた部分の背景模様描画用です。
 */
//--------------------------------------------------------------
void BINPUT_EffBGWallSet(BI_PARAM_PTR bip)
{
  GFL_BG_SetScroll( BI_FRAME_EFF, GFL_BG_SCROLL_X_SET, 0 );
  GFL_BG_SetScroll( BI_FRAME_EFF, GFL_BG_SCROLL_Y_SET, 0 );

//  ArcUtil_ScrnSet(ARC_BATT_BG, BATTLE_WBG0B_NSCR_BIN, bgl, BI_FRAME_EFF, 0, 0, 1, HEAPID_BATTLE);
//  GF_BGL_VisibleSet(BI_FRAME_EFF, VISIBLE_ON);
  GFL_BG_SetPriority( BI_FRAME_EFF, 3 );
}


//==============================================================================
//
//  BG作成時に呼ばれるコールバック関数
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   スライドエフェクトセット
 * @param   bip       BIシステムワークへのポインタ
 * @param   client_type   クライアントタイプ
 */
//--------------------------------------------------------------
static void BG_SlideSetInit(BI_PARAM_PTR bip, int client_type)
{
  COMMAND_IN_EFF_WORK *ciew;

  //タッチパネル無効
  bip->touch_invalid = TRUE;

  //ウィンドウマスク設定
  G2S_SetWndOutsidePlane(GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2
    | GX_WND_PLANEMASK_BG3, TRUE);

  G2S_SetWnd0InsidePlane(GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3, TRUE);  //上側
  G2S_SetWnd1InsidePlane(GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3, TRUE);  //下側

  G2S_SetWnd0Position(COMMANDIN_WND_START_X0, COMMANDIN_WND_START_Y0,
    COMMANDIN_WND_END_X0, COMMANDIN_WND_END_Y0);
  G2S_SetWnd1Position(COMMANDIN_WND_START_X1, COMMANDIN_WND_START_Y1,
    COMMANDIN_WND_END_X1, COMMANDIN_WND_END_Y1);

  GXS_SetVisibleWnd(GX_WNDMASK_W0 | GX_WNDMASK_W1);

  ciew = GFL_HEAP_AllocMemory( bip->heapID, sizeof( COMMAND_IN_EFF_WORK ) );
  MI_CpuClear8(ciew, sizeof(COMMAND_IN_EFF_WORK));
  ciew->bip = bip;
  ciew->client_type = client_type;
  ciew->x0_l = -COMMANDIN_SCR_X0_START;
  ciew->y1_d = COMMANDIN_SCR_Y1_START;
  ciew->set_x0_l = (COMMANDIN_SCR_X0_START - ciew->x0_l) / 100;
  ciew->set_y1_d = (COMMANDIN_SCR_Y1_START - ciew->y1_d) / 100;

  //Snd_SePlay(SEQ_SE_DP_SLIDEIN);
  GFL_TCB_AddTask( bip->tcbsys, CommandInEffTask, ciew, TCBPRI_BINPUT_COMMAND_IN_EFFECT );

  //Hブランク関数設定
  ciew->htask_tcb = GFUser_HIntr_CreateTCB( HBlankTCB_CommandInEff, ciew, 10 );
  //Vブランクタスク生成
  ciew->vtask_tcb = GFUser_VIntr_CreateTCB( VBlankTCB_CommandInEff, ciew, 10 );
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_COMMAND_INのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_CommandIn(BI_PARAM_PTR bip, int select_bg, int force_put)
{
  int client_type;

  client_type = bip->scene.bsc.client_type;

  //画面構築
  BGCallback_CommandSelect(bip, select_bg, force_put);
  //スライドエフェクトセット
  BG_SlideSetInit(bip, client_type);
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_AのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_CommandSelect(BI_PARAM_PTR bip, int select_bg, int force_put)
{
  STRBUF *fight_src, *item_src, *pokemon_src, *escape_src;
  int client_type;
  BINPUT_SCENE_COMMAND *bsc;

  bsc = &bip->scene.bsc;
  client_type = bsc->client_type;
  bip->cancel_escape = bsc->cancel_escape;

  bip->client_type = client_type;

#if 0
  if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_SAFARI){
    fight_src = MSGMAN_AllocString(msg_man, SafariCommnadBallMsg);
    item_src = MSGMAN_AllocString(msg_man, SafariCommnadEsaMsg);
    pokemon_src = MSGMAN_AllocString(msg_man, SafariCommnadDoroMsg);
  }
  else if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_POKE_PARK){
    fight_src = MSGMAN_AllocString(msg_man, ParkCommnadBallMsg);
    item_src = MSGMAN_AllocString(msg_man, SafariCommnadEsaMsg);
    pokemon_src = MSGMAN_AllocString(msg_man, SafariCommnadDoroMsg);
  }
  else{
    fight_src = MSGMAN_AllocString(msg_man, CommandFightMsg);
    item_src = MSGMAN_AllocString(msg_man, CommandBagMsg);
    pokemon_src = MSGMAN_AllocString(msg_man, CommandPokemonMsg);
  }

//  if(*client_type == CLIENT_TYPE_C && (BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_MULTI) == 0){
  if(bip->cancel_escape == TRUE){
    escape_src = MSGMAN_AllocString(msg_man, CommandModoruMsg);
    bip->command_modoru_type = TRUE;
  }
  else{
    escape_src = MSGMAN_AllocString(msg_man, CommandEscapeMsg);
    bip->command_modoru_type = FALSE;
  }
#else
#ifdef MESSAGE_PRINT_DUMMY
  fight_src = GFL_MSG_CreateString( bip->msg, BINPUT_FIGHT );
  item_src = GFL_MSG_CreateString( bip->msg, BINPUT_BAG );
  pokemon_src = GFL_MSG_CreateString( bip->msg, BINPUT_POKEMON );
  escape_src = GFL_MSG_CreateString( bip->msg, BINPUT_ESCAPE );
#else
//実際の戦闘に組み込んだときはこっちの文字処理を使用する
  fight_src = GFL_STR_CreateBuffer( 128, bip->heapID );
  item_src = GFL_STR_CreateBuffer( 128, bip->heapID );
  pokemon_src = GFL_STR_CreateBuffer( 128, bip->heapID );
  escape_src = GFL_STR_CreateBuffer( 128, bip->heapID );
  BTL_STR_GetUIString( fight_src, BTLMSG_UI_SEL_FIGHT );
  BTL_STR_GetUIString( item_src, BTLMSG_UI_SEL_ITEM );
  BTL_STR_GetUIString( pokemon_src, BTLMSG_UI_SEL_POKEMON );
  BTL_STR_GetUIString( escape_src, BTLMSG_UI_SEL_ESCAPE );
#endif
#endif

  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_FIGHT], fight_src,
    MSGCOLOR_FIGHT, PALOFS_FIGHT, bip->objplttID,
    FA_POS_FIGHT_X, FA_POS_FIGHT_Y, FONTOAM_CENTER, NULL);
  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ITEM], item_src,
    MSGCOLOR_ITEM, PALOFS_ITEM, bip->objplttID,
    FA_POS_ITEM_X, FA_POS_ITEM_Y, FONTOAM_CENTER, NULL);
  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_POKEMON], pokemon_src,
    MSGCOLOR_POKEMON, PALOFS_POKEMON, bip->objplttID,
    FA_POS_POKEMON_X, FA_POS_POKEMON_Y, FONTOAM_CENTER, NULL);
  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE], escape_src,
    MSGCOLOR_ESCAPE, PALOFS_ESCAPE, bip->objplttID,
    FA_POS_ESCAPE_X, FA_POS_ESCAPE_Y, FONTOAM_CENTER, NULL);

  GFL_STR_DeleteBuffer( fight_src );
  GFL_STR_DeleteBuffer( item_src );
  GFL_STR_DeleteBuffer( pokemon_src );
  GFL_STR_DeleteBuffer( escape_src );

#if 0
  //ポケモンアイコン（サファリとパーク以外ではアイコンを出していた模様）
  if((BattleWorkFightTypeGet(bip->bw) & (FIGHT_TYPE_SAFARI|FIGHT_TYPE_POKE_PARK)) == 0){
    POKEMON_PARAM *pp;
    CATS_ACT_PTR cap;
    int x, y;

    //リソースロード＆アクター生成
    pp = BattleWorkPokemonParamGet(bip->bw, bsc->client_no, bsc->sel_mons_no);
    Sub_PokeIconResourceLoad(bip);
    cap = Sub_PokeIconCharActorSet(bip, pp, client_type,
      bsc->icon_hp, bsc->icon_hpmax, bsc->icon_status);

    //座標設定
    FONTOAM_GetMat(bip->font_actor[FA_NO_FIGHT].fontoam, &x, &y);
    if(client_type == CLIENT_TYPE_C){
      x += bip->font_actor[FA_NO_FIGHT].font_len + POKEICON_SIZE_X/2;
    }
    else{
      x -= POKEICON_SIZE_X/2;
    }
    CATS_ObjectPosSetCap_SubSurface(cap, x, y - BATTLE_SUB_ACTOR_DISTANCE_INTEGER,
      BATTLE_SUB_ACTOR_DISTANCE);
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_COMMAND_IN_FIGHTONLYのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_CommandInFightOnly(BI_PARAM_PTR bip, int select_bg, int force_put)
{
  int client_type;

  client_type = bip->scene.bsc.client_type;

  //画面構築
  BGCallback_FightOnly(bip, select_bg, force_put);

  //スライドエフェクトセット
  BG_SlideSetInit(bip, client_type);
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_FIGHTONLYのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_FightOnly(BI_PARAM_PTR bip, int select_bg, int force_put)
{
  int client_type;

  client_type = bip->scene.bsc.client_type;

  //画面構築
  BGCallback_CommandSelect(bip, select_bg, force_put);

  //不必要なスクリーンをクリア(バッグ、ポケモン、にげる)
  {
    GFL_BG_FillScreen( BI_FRAME_PANEL, BG_CLEAR_CODE,
      0, 0x10, 32, 8, GFL_BG_SCRWRT_PALIN);
    GFL_BG_LoadScreenV_Req( BI_FRAME_BF );
  }

  //不必要なフォントOAMをバニッシュ
  BmpOam_ActorSetDrawEnable( bip->font_actor[FA_NO_ITEM].fontoam, FALSE );
  BmpOam_ActorSetDrawEnable( bip->font_actor[FA_NO_POKEMON].fontoam, FALSE );
  BmpOam_ActorSetDrawEnable( bip->font_actor[FA_NO_ESCAPE].fontoam, FALSE );
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_COMMAND_IN_FIGHTONLYのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_ParkCommandIn(BI_PARAM_PTR bip, int select_bg, int force_put)
{
  int client_type;

  client_type = bip->scene.bsc.client_type;

  //画面構築
  BGCallback_ParkCommand(bip, select_bg, force_put);

  //スライドエフェクトセット
  BG_SlideSetInit(bip, client_type);
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_FIGHTONLYのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_ParkCommand(BI_PARAM_PTR bip, int select_bg, int force_put)
{
  int client_type;

  client_type = bip->scene.bsc.client_type;

  //画面構築
  BGCallback_CommandSelect(bip, select_bg, force_put);

  //不必要なスクリーンをクリア(バッグ、ポケモン)
  {
    GFL_BG_FillScreen( BI_FRAME_PANEL, BG_CLEAR_CODE,
      0, 0x10, 10, 8, GFL_BG_SCRWRT_PALIN );
    GFL_BG_FillScreen( BI_FRAME_PANEL, BG_CLEAR_CODE,
      0x16, 0x10, 10, 8, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenV_Req( BI_FRAME_BF );
  }

  //不必要なフォントOAMをバニッシュ
  BmpOam_ActorSetDrawEnable( bip->font_actor[FA_NO_ITEM].fontoam, FALSE );
  BmpOam_ActorSetDrawEnable( bip->font_actor[FA_NO_POKEMON].fontoam, FALSE );
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_WAZAのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_Waza(BI_PARAM_PTR bip, int select_bg, int force_put)
{
  BINPUT_SCENE_WAZA *bsw;
  int i;
  MEMORY_DECORD_WORK *mdw;
  PRINTSYS_LSB color;

  bsw = &bip->scene.bsw;
  GF_ASSERT(bsw != NULL);

  bip->client_type = bsw->client_type;

  mdw = MemoryDecordWorkGet(bip, bsw->client_type);

  //技タイプアイコン
  Sub_WazaTypeIconCreateAll(bip);
  //技分類アイコン
//  Sub_WazaKindIconCreateAll(bip);

  //技名
  {
    Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_WAZA_1], NULL,
      MSGCOLOR_WAZA, PALOFS_WAZA,
      bip->objplttID, FA_POS_WAZA_1_X, FA_POS_WAZA_1_Y, FONTOAM_CENTER,
      &mdw->exbmp_waza[0]);

    Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_WAZA_2], NULL,
      MSGCOLOR_WAZA, PALOFS_WAZA,
      bip->objplttID, FA_POS_WAZA_2_X, FA_POS_WAZA_2_Y, FONTOAM_CENTER,
      &mdw->exbmp_waza[1]);

    Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_WAZA_3], NULL,
      MSGCOLOR_WAZA, PALOFS_WAZA,
      bip->objplttID, FA_POS_WAZA_3_X, FA_POS_WAZA_3_Y, FONTOAM_CENTER,
      &mdw->exbmp_waza[2]);

    Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_WAZA_4], NULL,
      MSGCOLOR_WAZA, PALOFS_WAZA,
      bip->objplttID, FA_POS_WAZA_4_X, FA_POS_WAZA_4_Y, FONTOAM_CENTER,
      &mdw->exbmp_waza[3]);
  }

  //PP num/max
  {
    for(i = 0 ; i < PTL_WAZA_MAX ; i++){
      color = PP_FontColorGet(bsw->pp[i], bsw->ppmax[i]);
      Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_PP_1 + i], NULL,
        color, PALOFS_PP, bip->objplttID,
        NowPPPos[i][0], NowPPPos[i][1], FONTOAM_LEFT, &mdw->exbmp_pp[i]);

      Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_PPMSG_1 + i], NULL,
        color, PALOFS_PP, bip->objplttID,
        PPMsgPos[i][0], PPMsgPos[i][1], FONTOAM_LEFT, &mdw->exbmp_ppmax[i]);
    }
  }

  //もどる
  {
    STRBUF *modoru_src;

#ifdef DEBUG_ONLY_FOR_sogabe
#warning  メッセージが違います
#endif
#if 0
#ifdef MESSAGE_PRINT_DUMMY
    modoru_src = GFL_MSG_CreateString( bip->msg, BINPUT_MODORU );
#else
    //実際の戦闘に組み込んだときはこっちの文字処理を使用する
    modoru_src = GFL_STR_CreateBuffer( 128, bip->heapID );
    BTL_STR_GetUIString( modoru_src, BTLMSG_UI_SEL_ESCAPE );
#endif
#else
    modoru_src = GFL_MSG_CreateString( bip->msg, BINPUT_MODORU );
#endif
    Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_WAZA_MODORU], modoru_src,
      MSGCOLOR_WAZA_MODORU, PALOFS_WAZA_MODORU,
      bip->objplttID, FA_POS_WAZA_MODORU_X, FA_POS_WAZA_MODORU_Y, FONTOAM_CENTER,
      NULL);
    GFL_STR_DeleteBuffer( modoru_src );
  }

  //-- 技タイプ毎のCGRを転送 --//
  {
    int waza_type;

    for(i = 0; i < PTL_WAZA_MAX; i++){
      if(bsw->wazano[i] != 0){
        waza_type = WT_WazaDataParaGet(bsw->wazano[i], ID_WTD_wazatype);
        Sub_WazaTypeCGRTrans(bip, waza_type, i);
      }
      else{
        Sub_WazaTypeNotTemotiChainCGRTrans(bip, i);
        break;
      }
    }
  }

  //-- 手持ち技がない時はフォントOBJ非表示 --//
  {
    if(bsw->wazano[0] == 0){
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_WAZA_1].fontoam, FALSE);
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_PPMSG_1].fontoam, FALSE);
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_PP_1].fontoam, FALSE);
    }
    if(bsw->wazano[1] == 0){
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_WAZA_2].fontoam, FALSE);
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_PPMSG_2].fontoam, FALSE);
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_PP_2].fontoam, FALSE);
    }
    if(bsw->wazano[2] == 0){
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_WAZA_3].fontoam, FALSE);
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_PPMSG_3].fontoam, FALSE);
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_PP_3].fontoam, FALSE);
    }
    if(bsw->wazano[3] == 0){
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_WAZA_4].fontoam, FALSE);
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_PPMSG_4].fontoam, FALSE);
      BmpOam_ActorSetDrawEnable(bip->font_actor[FA_NO_PP_4].fontoam, FALSE);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_DのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_YesNo(BI_PARAM_PTR bip, int select_bg, int force_put)
{
#if 0
  MSGDATA_MANAGER *msg_man;
  STRBUF *yes_src, *no_src;

  msg_man = BattleWorkFightMsgGet(bip->bw);

  yes_src = MSGMAN_AllocString(msg_man, BattleYesMsg);
  no_src = MSGMAN_AllocString(msg_man, BattleNoMsg);

  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_YES], yes_src, FONT_BUTTON,
    MSGCOLOR_ESCAPE_YES, PALOFS_ESCAPE_YES,
    PLTTID_INPUT_ESCAPE_YES, FA_POS_ESCAPE_YES_X, FA_POS_ESCAPE_YES_Y, FONTOAM_CENTER,
    NULL);
  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_NO], no_src, FONT_BUTTON,
    MSGCOLOR_ESCAPE_NO, PALOFS_ESCAPE_NO,
    PLTTID_INPUT_ESCAPE_NO, FA_POS_ESCAPE_NO_X, FA_POS_ESCAPE_NO_Y, FONTOAM_CENTER,
    NULL);

  STRBUF_Delete(yes_src);
  STRBUF_Delete(no_src);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_WASURERUのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_Wasureru(BI_PARAM_PTR bip, int select_bg, int force_put)
{
#if 0
  MSGDATA_MANAGER *msg_man;
  STRBUF *yes_src, *no_src;

  msg_man = BattleWorkFightMsgGet(bip->bw);

  yes_src = MSGMAN_AllocString(msg_man, battle_WazaWasureru_Msg);
  no_src = MSGMAN_AllocString(msg_man, battle_WazaWasurenai_Msg);

  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_YES], yes_src, FONT_BUTTON,
    MSGCOLOR_ESCAPE_YES, PALOFS_ESCAPE_YES,
    PLTTID_INPUT_ESCAPE_YES, FA_POS_ESCAPE_YES_X, FA_POS_ESCAPE_YES_Y, FONTOAM_CENTER,
    NULL);
  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_NO], no_src, FONT_BUTTON,
    MSGCOLOR_ESCAPE_NO, PALOFS_ESCAPE_NO,
    PLTTID_INPUT_ESCAPE_NO, FA_POS_ESCAPE_NO_X, FA_POS_ESCAPE_NO_Y, FONTOAM_CENTER,
    NULL);

  STRBUF_Delete(yes_src);
  STRBUF_Delete(no_src);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_AKIRAMERUのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_Akirameru(BI_PARAM_PTR bip, int select_bg, int force_put)
{
#if 0
  MSGDATA_MANAGER *msg_man;
  STRBUF *yes_src, *no_src, *yes_dest, *no_dest;
  WORDSET *wordset;
  BINPUT_SCENE_YESNO *bsy;

  bsy = &bip->scene.bsy;

  msg_man = BattleWorkFightMsgGet(bip->bw);
  wordset = BattleWorkWORDSETGet(bip->bw);

  yes_src = MSGMAN_AllocString(msg_man, battle_WazaAkirameru_Msg);
  no_src = MSGMAN_AllocString(msg_man, battle_WazaAkiramenai_Msg);

  yes_dest = STRBUF_Create(BUFLEN_BI_WASURERU, HEAPID_BATTLE);
  no_dest = STRBUF_Create(BUFLEN_BI_WASURENAI, HEAPID_BATTLE);

  WORDSET_RegisterWazaName(wordset, 0, bsy->waza_no);
  WORDSET_ExpandStr(wordset, yes_dest, yes_src);
  WORDSET_ExpandStr(wordset, no_dest, no_src);

  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_YES], yes_dest, FONT_BUTTON,
    MSGCOLOR_ESCAPE_YES, PALOFS_ESCAPE_YES,
    PLTTID_INPUT_ESCAPE_YES, FA_POS_ESCAPE_YES_X, FA_POS_ESCAPE_YES_Y, FONTOAM_CENTER,
    NULL);
  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_NO], no_dest, FONT_BUTTON,
    MSGCOLOR_ESCAPE_NO, PALOFS_ESCAPE_NO,
    PLTTID_INPUT_ESCAPE_NO, FA_POS_ESCAPE_NO_X, FA_POS_ESCAPE_NO_Y, FONTOAM_CENTER,
    NULL);

  STRBUF_Delete(yes_src);
  STRBUF_Delete(no_src);
  STRBUF_Delete(yes_dest);
  STRBUF_Delete(no_dest);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_PLAYBACK_STOPのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_PlayBackStop(BI_PARAM_PTR bip, int select_bg, int force_put)
{
#if 0
  MSGDATA_MANAGER *msg_man;
  STRBUF *stop_src;

  //システムウィンドウが見えなくなるので半透明切る
  G2S_SetBlendAlpha(BLD_PLANE_1, BLD_PLANE_2, 31, 0);

  msg_man = BattleWorkFightMsgGet(bip->bw);

  stop_src = MSGMAN_AllocString(msg_man, PlayBack_Msg1);

  //スクリーンバッファを停止ボタン用スクリーンで上書き(別のスクリーンバッファと共用しているので)
  {
    GF_BGL_INI *bgl;
    NNSG2dScreenData *scrnData;
    void *arc_data;

    //スクリーンバッファに読み込み
    arc_data = ArcUtil_ScrnDataGet(ARC_BATT_BG, BATTLE_WBG1STOP_NSCR_BIN, 1,
      &scrnData, HEAPID_BATTLE);
    MI_CpuCopy32(scrnData->rawData, bip->scrn_buf[SCRNBUF_PLAYBACK_STOP], 0x800);
    sys_FreeMemoryEz(arc_data);

    //BGLセット
    bgl = BattleWorkGF_BGL_INIGet(bip->bw);
    GF_BGL_ScreenBufSet(bgl, BI_FRAME_PANEL, bip->scrn_buf[SCRNBUF_PLAYBACK_STOP], 0x800);
    GF_BGL_LoadScreenV_Req(bgl, BI_FRAME_PANEL);
  }

  //システムメッセージ描画用のスクリーンバッファをクリアしておく(他スクリーンと共用しているので)
  {
    GF_BGL_INI *bgl = BattleWorkGF_BGL_INIGet(bip->bw);
    MI_CpuClear32(bip->scrn_buf[SCRNBUF_PLAYBACK_SYSWIN], 0x800);
    GF_BGL_ScreenBufSet(bgl, BI_FRAME_BF, bip->scrn_buf[SCRNBUF_PLAYBACK_SYSWIN], 0x800);
    GF_BGL_LoadScreenV_Req(bgl, BI_FRAME_BF);
  }

  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_PLAYBACK_STOP], stop_src, FONT_BUTTON,
    MSGCOLOR_ESCAPE_YES, PALOFS_ESCAPE_YES,
    PLTTID_INPUT_ESCAPE_YES, FA_POS_STOP_X, FA_POS_STOP_Y, FONTOAM_CENTER,
    NULL);

  STRBUF_Delete(stop_src);


#ifdef PM_DEBUG
  if(sys.cont & PAD_BUTTON_SELECT){
    int i;
    GAUGE_WORK *gauge;

    for(i=0;i<BattleWorkClientSetMaxGet(bip->bw);i++){
      gauge = BattleWorkGaugeWorkGet(bip->bw, i);
      Gauge_NumModeChange(gauge);
      Gauge_DebugHPPutFlagFlip(gauge);
      Gauge_PartsDraw(gauge, gauge->hp, GAUGE_DRAW_ALL);
    }
  }
#endif
#endif
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_NEXT_POKEMONのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_NextPokemon(BI_PARAM_PTR bip, int select_bg, int force_put)
{
#if 0
  MSGDATA_MANAGER *msg_man;
  STRBUF *yes_src, *no_src, *yes_dest, *no_dest;
  WORDSET *wordset;
  BINPUT_SCENE_YESNO *bsy;

  bsy = &bip->scene.bsy;

  msg_man = BattleWorkFightMsgGet(bip->bw);
  wordset = BattleWorkWORDSETGet(bip->bw);

  yes_src = MSGMAN_AllocString(msg_man, NextPokeYesMsg);
  no_src = MSGMAN_AllocString(msg_man, NextPokeNoMsg);

  yes_dest = STRBUF_Create(BUFLEN_BI_NEXT_POKEMON, HEAPID_BATTLE);
  no_dest = STRBUF_Create(BUFLEN_BI_NEXT_ESCAPE, HEAPID_BATTLE);

  WORDSET_RegisterWazaName(wordset, 0, bsy->waza_no);
  WORDSET_ExpandStr(wordset, yes_dest, yes_src);
  WORDSET_ExpandStr(wordset, no_dest, no_src);

  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_YES], yes_dest, FONT_BUTTON,
    MSGCOLOR_ESCAPE_YES, PALOFS_ESCAPE_YES,
    PLTTID_INPUT_ESCAPE_YES, FA_POS_ESCAPE_YES_X, FA_POS_ESCAPE_YES_Y, FONTOAM_CENTER,
    NULL);
  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_NO], no_dest, FONT_BUTTON,
    MSGCOLOR_ESCAPE_NO, PALOFS_ESCAPE_NO,
    PLTTID_INPUT_ESCAPE_NO, FA_POS_ESCAPE_NO_X, FA_POS_ESCAPE_NO_Y, FONTOAM_CENTER,
    NULL);

  STRBUF_Delete(yes_src);
  STRBUF_Delete(no_src);
  STRBUF_Delete(yes_dest);
  STRBUF_Delete(no_dest);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_CHANGE_POKEMONのBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_ChangePokemon(BI_PARAM_PTR bip, int select_bg, int force_put)
{
#if 0
  MSGDATA_MANAGER *msg_man;
  STRBUF *yes_src, *no_src, *yes_dest, *no_dest;
  WORDSET *wordset;
  BINPUT_SCENE_YESNO *bsy;

  bsy = &bip->scene.bsy;

  msg_man = BattleWorkFightMsgGet(bip->bw);
  wordset = BattleWorkWORDSETGet(bip->bw);

  yes_src = MSGMAN_AllocString(msg_man, TorikaeruYesMsg);
  no_src = MSGMAN_AllocString(msg_man, TorikaeruNoMsg);

  yes_dest = STRBUF_Create(BUFLEN_BI_CHANGE_POKEMON, HEAPID_BATTLE);
  no_dest = STRBUF_Create(BUFLEN_BI_CHANGE_NOT, HEAPID_BATTLE);

  WORDSET_RegisterWazaName(wordset, 0, bsy->waza_no);
  WORDSET_ExpandStr(wordset, yes_dest, yes_src);
  WORDSET_ExpandStr(wordset, no_dest, no_src);

  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_YES], yes_dest, FONT_BUTTON,
    MSGCOLOR_ESCAPE_YES, PALOFS_ESCAPE_YES,
    PLTTID_INPUT_ESCAPE_YES, FA_POS_ESCAPE_YES_X, FA_POS_ESCAPE_YES_Y, FONTOAM_CENTER,
    NULL);
  Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_ESCAPE_NO], no_dest, FONT_BUTTON,
    MSGCOLOR_ESCAPE_NO, PALOFS_ESCAPE_NO,
    PLTTID_INPUT_ESCAPE_NO, FA_POS_ESCAPE_NO_X, FA_POS_ESCAPE_NO_Y, FONTOAM_CENTER,
    NULL);

  STRBUF_Delete(yes_src);
  STRBUF_Delete(no_src);
  STRBUF_Delete(yes_dest);
  STRBUF_Delete(no_dest);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ポケモン選択のBGCreate時に呼ばれるコールバック関数
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   select_bg   表示BGタイプ(BINPUT_TYPE_???)
 * @param   force_put   TRUE:強制転送、FALSE:既に同じデータが転送されているなら再転送はしない
 */
//--------------------------------------------------------------
static void BGCallback_PokeSelect(BI_PARAM_PTR bip, int select_bg, int force_put)
{
  BINPUT_SCENE_POKE *bsp;
  int i;
  u8 hit_range[ BTL_CLIENT_MAX ];
  STRBUF *monsname_p, *monsname_src;
  WORDSET *wordset;
  PRINTSYS_LSB color;

  bsp = &bip->scene.bsp;
  bip->pokesele_type = bsp->pokesele_type;
  bip->client_type = bsp->client_type;
  wordset = WORDSET_Create( bip->heapID );

  Sub_PokeSelectHitRange( bip, hit_range, FALSE );

  Sub_PokeIconResourceLoad( bip );

  Sub_JointScreenWrite( bip );

  //ポケモン名
  monsname_p = GFL_STR_CreateBuffer( BUFLEN_BI_POKESELENAME, bip->heapID );
  for( i = 0; i < BTL_CLIENT_MAX ; i++ ){
    if(bsp->dspp[ i ].exist == TRUE && hit_range[ i ] == TRUE){
#ifdef DEBUG_ONLY_FOR_sogabe
#warning 性別判断現状なし
#endif //DEBUG_ONLY_FOR_sogabe
      if(bsp->dspp[ i ].sex == PTL_SEX_MALE ){
        monsname_src = GFL_MSG_CreateString( bip->msg, TargetPokemonMaleMsg );
      }
      else if(bsp->dspp[ i ].sex == PTL_SEX_FEMALE){
        monsname_src = GFL_MSG_CreateString( bip->msg, TargetPokemonFemaleMsg );
      }
      else{
        monsname_src = GFL_MSG_CreateString( bip->msg, TargetPokemonMsg );
      }

      WORDSET_RegisterPokeNickName( wordset, 0, bsp->dspp[ i ].pp );
      WORDSET_ExpandStr(wordset, monsname_p, monsname_src);

      if(i & 1){
        color = MSGCOLOR_POKESELE_ENEMY;
      }
      else{
        color = MSGCOLOR_POKESELE_MINE;
      }
      Sub_FontOamCreate(bip, &bip->font_actor[ FA_NO_POKE_A + i ], monsname_p,
        color, PALOFS_POKESELE, bip->objplttID,
        PokeSelePos[ i ][ 0 ], PokeSelePos[ i ][ 1 ], FONTOAM_CENTER, NULL);

      GFL_STR_DeleteBuffer( monsname_src );

#ifdef DEBUG_ONLY_FOR_sogabe
#warning ポケモンアイコンは今はださない
#endif //DEBUG_ONLY_FOR_sogabe
#if 0
      {//ポケモンアイコン
        switch(i){
        case 1:
          Sub_PokeIconCharActorSet(bip, pp, CLIENT_TYPE_B, bsp->dspp[client_no].hp,
            bsp->dspp[client_no].hpmax, bsp->dspp[client_no].status);
          break;
        case 3:
          Sub_PokeIconCharActorSet(bip, pp, CLIENT_TYPE_D, bsp->dspp[client_no].hp,
            bsp->dspp[client_no].hpmax, bsp->dspp[client_no].status);
          break;
        }
      }
#endif
    }
    else{   //選択出来ないポケモン
    #if 0 //2006.05.10(水)
      if(hit_range[i] == TRUE){
        //通常選択出来るんだけど、死亡などで選択出来ないポケモンに限り壁パネル転送
        Sub_PokemonNotPanelTrans(bip, i);
      }
    #else
      if(hit_range[i] == FALSE){
        //元々選択出来ないパネルのみ、壁パネルを転送する
        //死亡などで選択出来ないのはジョイントOBJの都合上元のまま残す
        Sub_PokemonNotPanelTrans(bip, i);
      }
    #endif
    }
  }
  WORDSET_Delete( wordset );
  GFL_STR_DeleteBuffer( monsname_p );

  //もどる
  {
    STRBUF *modoru_src;
    modoru_src = GFL_MSG_CreateString( bip->msg, BINPUT_MODORU );
    Sub_FontOamCreate(bip, &bip->font_actor[ FA_NO_POKE_CANCEL ], modoru_src,
      MSGCOLOR_POKESELE_MODORU, PALOFS_POKESELE_MODORU, bip->objplttID,
      PokeSelePos[ BTL_CLIENT_MAX ][ 0 ], PokeSelePos[ BTL_CLIENT_MAX ][ 1 ], FONTOAM_CENTER, NULL);
    GFL_STR_DeleteBuffer( modoru_src );
  }
}

//==============================================================================
//
//  タッチパネル反応時に呼ばれるコールバック関数
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_Aでタッチパネル反応時に呼び出されるコールバック関数
 *
 * @param   bip     BIシステムワークへのポインタ
 * @param   tp_ret    タッチパネル結果(bmd->tpd_retの値かGFL_UI_TP_HIT_NONE)
 * @param   panel_pal 押したパネルのパレット番号
 *
 * @retval  tp_retを返す(処理の中で変更する必要がないなら、引数で渡された値をそのまま返す)
 */
//--------------------------------------------------------------
static int TPCallback_A(BI_PARAM_PTR bip, int tp_ret, int panel_pal)
{
  int fa_no, waku_type, pokeicon_no;

  pokeicon_no = NOT_POKE_ICON;

  switch(tp_ret){
  case GFL_UI_TP_HIT_NONE:
  default:
    return tp_ret;
  case SELECT_FIGHT_COMMAND:
    fa_no = FA_NO_FIGHT;
    waku_type = WAKU_TYPE_A;

    switch(bip->client_type){
    case CLIENT_TYPE_AA:
      pokeicon_no = bip->client_type;
      break;
    case CLIENT_TYPE_A:
    case CLIENT_TYPE_C:
      pokeicon_no = bip->client_type - CLIENT_TYPE_A;
      break;
    }
    break;
  case SELECT_ITEM_COMMAND:
    fa_no = FA_NO_ITEM;
    waku_type = WAKU_TYPE_B;
    break;
  case SELECT_POKEMON_COMMAND:
    fa_no = FA_NO_POKEMON;
    waku_type = WAKU_TYPE_B;
    break;
  case SELECT_ESCAPE_COMMAND:
    fa_no = FA_NO_ESCAPE;
    waku_type = WAKU_TYPE_B;
    break;
  }

  ColorEffTCB_PointFadeSet(bip, panel_pal);

  EffectTCB_Add(Effect_ButtonDown, bip);
  bip->effect_work.paracgr.scrn_offset = CommandButtonScrnOffset[tp_ret - SELECT_FIGHT_COMMAND];
  bip->effect_work.paracgr.scrn_range = &CommandButtonScrnRect[tp_ret - SELECT_FIGHT_COMMAND];
  bip->effect_work.paracgr.scrnbuf_no = SCRNBUF_COMMAND;
  bip->effect_work.paracgr.tp_ret = tp_ret;
  bip->effect_work.paracgr.fa_no = fa_no;
  bip->effect_work.paracgr.pokeicon_no = pokeicon_no;
  bip->effect_work.paracgr.waku_type = waku_type;
  bip->effect_work.paracgr.waku_pos = CommandWakuPos[tp_ret - SELECT_FIGHT_COMMAND];
  bip->effect_work.paracgr.obj_del = TRUE;
  return tp_ret;
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_WAZAでタッチパネル反応時に呼び出されるコールバック関数
 *
 * @param   bip     BIシステムワークへのポインタ
 * @param   tp_ret    タッチパネル結果(bmd->tpd_retの値かGFL_UI_TP_HIT_NONE)
 * @param   panel_pal 押したパネルのパレット番号
 *
 * @retval  tp_retを返す(処理の中で変更する必要がないなら、引数で渡された値をそのまま返す)
 */
//--------------------------------------------------------------
static int TPCallback_Waza(BI_PARAM_PTR bip, int tp_ret, int panel_pal)
{
  BINPUT_SCENE_WAZA *bsw;
  u32 transcgr_id[BUTTON_ANM_NUM];
  u32 rd_no;
  const CGR_TRANS_RECTDATA *ctr[BUTTON_ANM_NUM];
  const s16 *scrn_offset;
  const REWRITE_SCRN_RECT *scrn_range;

  if(tp_ret == GFL_UI_TP_HIT_NONE){
    return tp_ret;
  }

  bsw = &bip->scene.bsw;
  if(tp_ret >= SELECT_SKILL_1 && tp_ret <= SELECT_SKILL_4){
    if(bsw->wazano[tp_ret - SELECT_SKILL_1] == 0){
      return GFL_UI_TP_HIT_NONE;  //押された場所の手持ち技がない
    }
  }

  rd_no = tp_ret - SELECT_SKILL_1;
  switch(tp_ret){
  case GFL_UI_TP_HIT_NONE:
  default:
    return GFL_UI_TP_HIT_NONE;
  case SELECT_SKILL_1:
  case SELECT_SKILL_2:
  case SELECT_SKILL_3:
  case SELECT_SKILL_4:
    scrn_offset = WazaButtonScrnOffset[tp_ret - SELECT_SKILL_1];
    scrn_range = &WazaButtonScrnRect[tp_ret - SELECT_SKILL_1];
    break;
  case SELECT_CANCEL:
    scrn_offset = WazaButtonScrnOffset[NELEMS(WazaButtonScrnOffset) - 1];
    scrn_range = &WazaButtonScrnRect[NELEMS(WazaButtonScrnOffset) - 1];
    break;
  }

  ColorEffTCB_PointFadeSet(bip, panel_pal);

  EffectTCB_Add(Effect_WazaButtonDown, bip);
  bip->effect_work.paracgr.scrn_offset = scrn_offset;
  bip->effect_work.paracgr.scrn_range = scrn_range;
  bip->effect_work.paracgr.scrnbuf_no = SCRNBUF_WAZA;
  bip->effect_work.paracgr.tp_ret = tp_ret;
  bip->effect_work.paracgr.obj_del = TRUE;  //WazaButtonDownでは使用しないけど一応
//  bip->effect_work.paracgr.fa_no = fa_no;
  return tp_ret;
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_Dでタッチパネル反応時に呼び出されるコールバック関数
 *
 * @param   bip     BIシステムワークへのポインタ
 * @param   tp_ret    タッチパネル結果(bmd->tpd_retの値かGFL_UI_TP_HIT_NONE)
 * @param   panel_pal 押したパネルのパレット番号
 *
 * @retval  tp_retを返す(処理の中で変更する必要がないなら、引数で渡された値をそのまま返す)
 */
//--------------------------------------------------------------
static int TPCallback_D(BI_PARAM_PTR bip, int tp_ret, int panel_pal)
{
  int fa_no;

  switch(tp_ret){
  case GFL_UI_TP_HIT_NONE:
  default:
    return tp_ret;
  case TRUE:
    fa_no = FA_NO_ESCAPE_YES;
    break;
  case SELECT_CANCEL:
    fa_no = FA_NO_ESCAPE_NO;
    break;
  }

  ColorEffTCB_PointFadeSet(bip, panel_pal);

  EffectTCB_Add(Effect_ButtonDown, bip);
  if(tp_ret == TRUE){
    bip->effect_work.paracgr.scrn_offset = YesNoButtonScrnOffset[1];
    bip->effect_work.paracgr.scrn_range = &YesNoButtonScrnRect[1];
  }
  else{
    bip->effect_work.paracgr.scrn_offset = YesNoButtonScrnOffset[0];
    bip->effect_work.paracgr.scrn_range = &YesNoButtonScrnRect[0];
  }
  bip->effect_work.paracgr.scrnbuf_no = SCRNBUF_YESNO;
  bip->effect_work.paracgr.tp_ret = tp_ret;
  bip->effect_work.paracgr.fa_no = fa_no;
  bip->effect_work.paracgr.waku_type = WAKU_TYPE_NONE;
  bip->effect_work.paracgr.obj_del = TRUE;
  return tp_ret;
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_Dでタッチパネル反応時に呼び出されるコールバック関数
 *
 * @param   bip     BIシステムワークへのポインタ
 * @param   tp_ret    タッチパネル結果(bmd->tpd_retの値かGFL_UI_TP_HIT_NONE)
 * @param   panel_pal 押したパネルのパレット番号
 *
 * @retval  tp_retを返す(処理の中で変更する必要がないなら、引数で渡された値をそのまま返す)
 */
//--------------------------------------------------------------
static int TPCallback_PokeSele(BI_PARAM_PTR bip, int tp_ret, int panel_pal)
{
  BINPUT_SCENE_POKE *bsp;
  u8 hit_range[ BTL_CLIENT_MAX ];

  bsp = &bip->scene.bsp;

  Sub_PokeSelectHitRange(bip, hit_range, TRUE);

  switch(tp_ret){
  case GFL_UI_TP_HIT_NONE:
  default:
    return tp_ret;
  case SELECT_TARGET_A:
    if(hit_range[CLIENT_TYPE_A - CLIENT_TYPE_A] == FALSE){
      return GFL_UI_TP_HIT_NONE;
    }
    break;
  case SELECT_TARGET_B:
    if(hit_range[CLIENT_TYPE_B - CLIENT_TYPE_A] == FALSE){
      return GFL_UI_TP_HIT_NONE;
    }
    break;
  case SELECT_TARGET_C:
    if(hit_range[CLIENT_TYPE_C - CLIENT_TYPE_A] == FALSE){
      return GFL_UI_TP_HIT_NONE;
    }
    break;
  case SELECT_TARGET_D:
    if(hit_range[CLIENT_TYPE_D - CLIENT_TYPE_A] == FALSE){
      return GFL_UI_TP_HIT_NONE;
    }
    break;
  case SELECT_CANCEL:
    ColorEffTCB_PointFadeSet(bip, NELEMS(PokeSeleMenuPaletteNo) - 1);

    EffectTCB_Add(Effect_ButtonDown, bip);
    bip->effect_work.paracgr.scrn_offset
      = PokemonButtonScrnOffset[NELEMS(PokemonButtonScrnOffset) - 1];
    bip->effect_work.paracgr.scrn_range
      = &PokemonButtonScrnRect[NELEMS(PokemonButtonScrnRect) - 1];
    bip->effect_work.paracgr.scrnbuf_no = SCRNBUF_POKESELE;
    bip->effect_work.paracgr.tp_ret = tp_ret;
    bip->effect_work.paracgr.fa_no = FA_NO_POKE_CANCEL;
    bip->effect_work.paracgr.waku_type = WAKU_TYPE_NONE;
    bip->effect_work.paracgr.obj_del = TRUE;
    return tp_ret;
  }

  //枠スクリーンクリア
  {
    GFL_BG_ClearScreenCode( BI_FRAME_BF, BG_CLEAR_CODE );
    GFL_BG_LoadScreenV_Req( BI_FRAME_BF );
  }

  //ColorEffTCB_PointFadeSet(bip, panel_pal);
  ColorEffTCB_PokeSeleFadeSet(bip, tp_ret);

  EffectTCB_Add(Effect_PokeSeleButtonDown, bip);
  bip->effect_work.pokesele.tp_ret = tp_ret;
  return tp_ret;
}

//--------------------------------------------------------------
/**
 * @brief   BINPUT_TYPE_PLAYBACK_STOPでタッチパネル反応時に呼び出されるコールバック関数
 *
 * @param   bip     BIシステムワークへのポインタ
 * @param   tp_ret    タッチパネル結果(bmd->tpd_retの値かGFL_UI_TP_HIT_NONE)
 * @param   panel_pal 押したパネルのパレット番号
 *
 * @retval  tp_retを返す(処理の中で変更する必要がないなら、引数で渡された値をそのまま返す)
 */
//--------------------------------------------------------------
static int TPCallback_PlayBackStop(BI_PARAM_PTR bip, int tp_ret, int panel_pal)
{
  switch(tp_ret){
  case GFL_UI_TP_HIT_NONE:
  default:
    return tp_ret;
  case TRUE:
    break;
  }

  ColorEffTCB_PointFadeSet(bip, panel_pal);

  EffectTCB_Add(Effect_ButtonDown, bip);
  bip->effect_work.paracgr.scrn_offset = StopButtonScrnOffset;
  bip->effect_work.paracgr.scrn_range = &StopButtonScrnRect[0];
  bip->effect_work.paracgr.scrnbuf_no = SCRNBUF_PLAYBACK_STOP;
  bip->effect_work.paracgr.tp_ret = tp_ret;
  bip->effect_work.paracgr.fa_no = FA_NO_PLAYBACK_STOP;
  bip->effect_work.paracgr.waku_type = WAKU_TYPE_NONE;
  bip->effect_work.paracgr.obj_del = FALSE;
  return tp_ret;
}



//==============================================================================
//
//  サブ関数
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコンのリソースをロードする
 * @param   bip   BIシステムワークへのポインタ
 *
 * キャラのリソースはアクター生成と一緒にやるので、ここでは登録しません
 */
//--------------------------------------------------------------
static void Sub_PokeIconResourceLoad(BI_PARAM_PTR bip)
{
#if 0
  int cell_id, char_id, anm_id;
  ARCHANDLE     *hdl;

  hdl = GFL_ARC_OpenDataHandle( ARCID_POKEICON, bip->heapID );

  //パレット
  bip->pokeicon_plttID = GFL_CLGRP_PLTT_Register( hdl, PokeIconPalArcIndexGet(), CLSYS_DRAW_SUB, 0, bip->heapID );
  //セル
  //セルアニメ
  bip->pokeicon_cellID = GFL_CLGRP_CELLANIM_Register( hdl, PokeIconAnmCellArcIndexGet(), PokeIconAnmCellAnmArcIndexGet(), 0, bip->heapID );

  GFL_ARC_CloseDataHandle( hdl );
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコンのリソースを全て解放する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_PokeIconResourceFree(BI_PARAM_PTR bip)
{
  int i;

  for( i = 0 ; i < BTL_CLIENT_MAX ; i++ ){
    if( bip->pokeicon_charID[ i ] != GFL_CLGRP_REGISTER_FAILED ){
      GFL_CLGRP_CGR_Release( bip->pokeicon_charID[ i ] );
      bip->pokeicon_charID[ i ] = GFL_CLGRP_REGISTER_FAILED;
    }
  }
  GFL_CLGRP_PLTT_Release( bip->pokeicon_plttID );
  GFL_CLGRP_CELLANIM_Release( bip->pokeicon_cellID );
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコンアクターを全て削除する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_PokeIconActorAllDel(BI_PARAM_PTR bip)
{
  int i;

  for(i = 0; i < BTL_CLIENT_MAX; i++){
    if(bip->pokeicon_cap[i] != NULL){
      GFL_CLACT_WK_Remove( bip->pokeicon_cap[ i ] );
      bip->pokeicon_cap[i] = NULL;
    }
    if(bip->pokeicon_tcb[i] != NULL){
      GFL_TCB_DeleteTask( bip->pokeicon_tcb [ i ] );
      bip->pokeicon_tcb[i] = NULL;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコンアクターを生成する
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   pp        対象のポケモンパラメータ
 * @param   client_type   CIENT_TYPE_???
 *
 * @retval  生成したポケモンアイコンアクターのポインタ
 */
//--------------------------------------------------------------
static GFL_CLWK * Sub_PokeIconCharActorSet(BI_PARAM_PTR bip, POKEMON_PARAM *pp, int client_type,
  int hp, int maxhp, int status)
{
  GFL_CLWK *cap;
#if 0
  u32 char_id;
  GFL_CLWK_DATA obj_param;
  int tbl_no;
  ARCHANDLE     *hdl;

  if(client_type >= CLIENT_TYPE_A){
    tbl_no = client_type - CLIENT_TYPE_A; //0originに修正
  }
  else{
    tbl_no = client_type; //AA or BB
  }

  GF_ASSERT(bip->pokeicon_cap[tbl_no] == NULL);
  GF_ASSERT(bip->pokeicon_tcb[tbl_no] == NULL);

  //キャラリソース
  hdl = GFL_ARC_OpenDataHandle( ARCID_POKEICON, bip->heapID );

  bip->pokeicon_charID[ tbl_no ] = GFL_CLGRP_CGR_Register( hdl, PokeIconCgxArcIndexGetByPP( pp ),
                               FALSE, CLSYS_DRAW_SUB, bip->heapID );
  GFL_ARC_CloseDataHandle( hdl );

  //アクター
  obj_param = PokeIconObjParam;
  obj_param.x = PokeIconPos[tbl_no][0];
  obj_param.y = PokeIconPos[tbl_no][1];
  cap = GFL_CLACT_WK_Create( bip->pokeicon_clunit, bip->pokeicon_charID[ tbl_no ], bip->pokeicon_plttID, bip->pokeicon_cellID,
                 &PokeIconObjParam, CLSYS_DEFREND_SUB, bip->heapID );
  //パレット切り替え
//  monsno = PokeParaGet(pp, ID_PARA_monsno, NULL);
//  egg = PokeParaGet(pp, ID_PARA_tamago_flag, NULL);
  GFL_CLACT_WK_SetPlttOffs( cap, PokeIconPalNumGetByPP(pp), CLWK_PLTTOFFS_MODE_OAM_COLOR );

  {//アイコンアニメ設定
    int anmno = 0;

    if(hp == 0 || status == COMSEL_STATUS_DEAD){
      anmno = POKEICON_ANM_DEATH;
    }
    else if(status == COMSEL_STATUS_NG){
      anmno = POKEICON_ANM_STCHG;
    }
    else{
      switch(GetHPGaugeDottoColor(hp, maxhp, GAUGE_HP_DOTTOMAX)){
      case HP_DOTTO_MAX:
        anmno = POKEICON_ANM_HPMAX;
        break;
      case HP_DOTTO_GREEN:  // 緑
        anmno = POKEICON_ANM_HPGREEN;
        break;
      case HP_DOTTO_YELLOW: // 黄
        anmno = POKEICON_ANM_HPYERROW;
        break;
      case HP_DOTTO_RED:    // 赤
        anmno = POKEICON_ANM_HPRED;
        break;
      }
      CATS_ObjectAnimeSeqSetCap(cap, anmno);
      //CATS_ObjectAutoAnimeSetCap(cap, CATS_ANM_AUTO_ON);  //オートアニメ
    }
  #ifdef OSP_BINPUT_ON
    OS_TPrintf("anmno = %d, hp = %d, max = %d, status = %d\n", anmno, hp, maxhp, status);
  #endif
  }

  CATS_ObjectUpdateCap(cap);
  bip->pokeicon_cap[tbl_no] = cap;
  bip->pokeicon_tcb[tbl_no] = TCB_Add(PokeIconAnimeUpdate, cap, TCBPRI_BINPUT_EFFECT);
#endif
  return cap;
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコンアニメ更新処理
 *
 * @param   tcb   TCBへのポインタ
 * @param   work  ポケモンアイコンアクターへのポインタ
 */
//--------------------------------------------------------------
static void PokeIconAnimeUpdate(GFL_TCB *tcb, void *work)
{
  GFL_CLWK *cap = work;
  GFL_CLACT_WK_SetAnmFrame( cap, FX32_ONE );
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンのステータスから手持ちボールのアニメシーケンス番号を取得する
 * @param   status    ポケモンのステータス(COMSEL_STATUS_???)
 * @retval  手持ちボールのアニメシーケンス番号
 */
//--------------------------------------------------------------
static int Sub_StockBallAnmSeqGet(u8 status)
{
  int anm_seq;

  switch(status){
  case COMSEL_STATUS_NONE:
  default:
    anm_seq = STOCK_ANMSEQ_NONE;
    break;
  case COMSEL_STATUS_ALIVE:
    anm_seq = STOCK_ANMSEQ_ALIVE;
    break;
  case COMSEL_STATUS_DEAD:
    anm_seq = STOCK_ANMSEQ_DEAD;
    break;
  case COMSEL_STATUS_NG:
    anm_seq = STOCK_ANMSEQ_NG;
    break;
  }
  return anm_seq;
}

//--------------------------------------------------------------
/**
 * @brief   ewpにSub_ScrnTouchChange関数用のパラメータをセットする
 *
 * @param   ewp       エフェクトBG書き換えパラメータ代入先
 * @param   rsr       書き換え範囲
 * @param   rsr_num     rsrデータ個数
 * @param   add_charname  キャラクターネームに足しこむ値(マイナスも有り)
 * @param   fa_no     対象フォントOBJアクターワークの添え字
 */
//--------------------------------------------------------------
inline void SubInline_EffBgWriteParamSet(EFFBG_WRITE_PARAM *ewp, const REWRITE_SCRN_RECT *rsr,
  u8 rsr_num, s8 add_charname)
{
  ewp->rsr = rsr;
  ewp->rsr_num = rsr_num;
  ewp->add_charname = add_charname;
}

//--------------------------------------------------------------
/**
 * @brief   タッチパネル反応時に動作させるBGスクリーン書き換えをひとまとめにしたもの
 *
 * @param   bgl         BGLデータ
 * @param   rsr[]       書き換え範囲
 * @param   rsr_num       rsrデータ個数
 * @param   add_charname    キャラクターネームに足しこむ値(マイナスも有り)
 */
//--------------------------------------------------------------
static void Sub_ScrnTouchChange( const REWRITE_SCRN_RECT rsr[], int rsr_num, int add_charname )
{
  u16 *eff_scrn;
  u32 eff_size;

  //エフェクト面のスクリーンをクリア
  eff_scrn = GFL_BG_GetScreenBufferAdrs( BI_FRAME_EFF );
  eff_size = GFL_BG_GetScreenBufferSize( BI_FRAME_EFF );
  MI_CpuClear16(eff_scrn, eff_size);

  Sub_ScrnAddOffset(rsr, rsr_num,  BI_FRAME_PANEL, add_charname);
  Sub_ScrnCopy(rsr, rsr_num,  BI_FRAME_BF, BI_FRAME_EFF);
  Sub_ScrnErase(rsr, rsr_num,  BI_FRAME_BF, BG_CLEAR_CODE);

//  GFL_BG_SetScroll( BI_FRAME_EFF, GF_BGL_SCROLL_Y_SET, SCRN_SCROLL_A_EFF_Y);
  GFL_BG_SetVisible( BI_FRAME_EFF, VISIBLE_ON );

  GFL_BG_LoadScreenReq( BI_FRAME_PANEL );
  GFL_BG_LoadScreenReq( BI_FRAME_BF );
  GFL_BG_LoadScreenReq( BI_FRAME_EFF );
}

//--------------------------------------------------------------
/**
 * @brief   タッチパネル反応時に動作させるBGスクリーン書き換えをひとまとめにしたもの
 *
 * @param   bgl         BGLデータ
 * @param   rsr[]       書き換え範囲
 * @param   rsr_num       rsrデータ個数
 * @param   add_charname    キャラクターネームに足しこむ値(マイナスも有り)
 */
//--------------------------------------------------------------
static void Sub_ScrnTouchChangeReverse( const REWRITE_SCRN_RECT rsr[], int rsr_num, int add_charname )
{
  u16 *eff_scrn;
  u32 eff_size;

  GFL_BG_SetVisible( BI_FRAME_EFF, VISIBLE_OFF );

  Sub_ScrnAddOffset(rsr, rsr_num, BI_FRAME_PANEL, -add_charname);
  Sub_ScrnCopy(rsr, rsr_num, BI_FRAME_EFF, BI_FRAME_BF);

  GFL_BG_LoadScreenReq( BI_FRAME_PANEL );
  GFL_BG_LoadScreenReq( BI_FRAME_BF );
  GFL_BG_LoadScreenReq( BI_FRAME_EFF );
}

//--------------------------------------------------------------
/**
 * @brief   スクリーンバッファを読み取り、キャラクターネームに指定値分足しこむ
 *
 * @param   rsr[]       書き換え範囲
 * @param   rsr_num       rsrデータ個数
 * @param   screen_buf      スクリーンバッファへのポインタ
 * @param   add_charname    キャラクターネームに足しこむ値(マイナスも有り)
 */
//--------------------------------------------------------------
static void Sub_ScrnAddOffset(const REWRITE_SCRN_RECT rsr[], int rsr_num, int frame_no, int add_charname)
{
  int i, x, y, charname;
  u16 *screen_buf, *buf;

  screen_buf = GFL_BG_GetScreenBufferAdrs( frame_no );
  GF_ASSERT(screen_buf != NULL);

  for(i = 0; i < rsr_num; i++){
    for(y = rsr[i].top; y <= rsr[i].bottom; y++){
      buf = &screen_buf[0x20 * y];
      for(x = rsr[i].left; x <= rsr[i].right; x++){
        charname = (buf[x] & GX_SCRFMT_TEXT_CHARNAME_MASK) + add_charname;
        buf[x] -= buf[x] & GX_SCRFMT_TEXT_CHARNAME_MASK;
        buf[x] |= charname;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   指定範囲のスクリーンを別のスクリーンバッファにコピーする
 *
 * @param   rsr[]     コピー範囲
 * @param   rsr_num     rsrデータ個数
 * @param   bgl       BGLデータ
 * @param   src_frame   コピー元スクリーンのフレームNo
 * @param   dest_frame    コピー先スクリーンのフレームNo
 */
//--------------------------------------------------------------
static void Sub_ScrnCopy(const REWRITE_SCRN_RECT rsr[], int rsr_num, int src_frame, int dest_frame)
{
  u16 *src_buf, *dest_buf;
  int i, y;

  src_buf = GFL_BG_GetScreenBufferAdrs( src_frame );
  dest_buf = GFL_BG_GetScreenBufferAdrs( dest_frame );
  GF_ASSERT(src_buf != NULL);
  GF_ASSERT(dest_frame != NULL);

  for(i = 0; i < rsr_num; i++){
    for(y = rsr[i].top; y <= rsr[i].bottom; y++){
      MI_CpuCopy16(&src_buf[0x20*y + rsr[i].left], &dest_buf[0x20*y + rsr[i].left],
        (rsr[i].right - rsr[i].left + 1) * 2);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   指定範囲のスクリーンバッファを指定コードでクリアする
 *
 * @param   rsr[]     クリア範囲
 * @param   rsr_num     rsrデータ個数
 * @param   bgl       BGLデータ
 * @param   frame_no    フレーム番号
 * @param   clear_code    クリアーコード
 */
//--------------------------------------------------------------
static void Sub_ScrnErase(const REWRITE_SCRN_RECT rsr[], int rsr_num, int frame_no, u16 clear_code)
{
  u16 *buf;
  int i, y;

  buf = GFL_BG_GetScreenBufferAdrs( frame_no );
  GF_ASSERT(buf != NULL);

  for(i = 0; i < rsr_num; i++){
    for(y = rsr[i].top; y <= rsr[i].bottom; y++){
      MI_CpuFill16(&buf[0x20*y + rsr[i].left], clear_code,
        (rsr[i].right - rsr[i].left + 1) * 2);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   CGR転送アニメを行います
 * @param   ctp   CGR転送アニメパラメータワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_CgrParamTrans(CGR_TRANS_PARAM *ctp)
{
  int x, y, dest_x_len;
  u32 src_y, dest_y;

  dest_x_len = 32*32;

  src_y = ctp->rd.src_y * ctp->x_len;
  dest_y = ctp->rd.dest_y * dest_x_len;

  for(y = 0; y < ctp->rd.size_y; y++){
    if(ctp->raw_data != NULL){
      MI_CpuCopy16(&ctp->raw_data[ctp->x_len * y + src_y + ctp->rd.src_x*0x20],
        &ctp->dest_vram[dest_x_len * y + dest_y + ctp->rd.dest_x*0x20],
        ctp->rd.size_x * 0x20);
    }
    else{
      MI_CpuClear16(&ctp->dest_vram[dest_x_len * y + dest_y + ctp->rd.dest_x*0x20],
        ctp->rd.size_x * 0x20);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   文字列の長さを取得する
 *
 * @param   str       文字列へのポインタ
 * @param   font      フォントタイプ
 * @param   ret_dot_len   ドット幅代入先
 * @param   ret_char_len  キャラ幅代入先
 */
//--------------------------------------------------------------
static void FontLenGet(const STRBUF *str, GFL_FONT *font, int *ret_dot_len, int *ret_char_len)
{
  int dot_len, char_len;

  //文字列のドット幅から、使用するキャラ数を算出する（今日のところは時間がないので、明日頼む）
  dot_len = PRINTSYS_GetStrWidth( str, font, PANEL_MSG_MARGIN );
  char_len = dot_len / 8;
  if(FX_ModS32(dot_len, 8) != 0){
    char_len++;
  }

  *ret_dot_len = dot_len;
  *ret_char_len = char_len;
}

//--------------------------------------------------------------
/**
 * @brief   フォントOAMを作成する
 *
 * @param   bip     BIシステムワークへのポインタ
 * @param   font_actor  生成したフォントOAM関連のワーク代入先
 * @param   str     文字列(ex_bmpwinを使用する場合はNULLでOK)
 * @param   color   フォントカラー構成
 * @param   pal_offset  パレット番号オフセット
 * @param   pal_index   適用するパレットのindex(GFL_CLGRP_PLTT_Registerの戻り値)
 * @param   x     座標X
 * @param   y     座標Y
 * @param   pos_center  FONTOAM_LEFT(X左端座標) or FONTOAM_CENTER(X中心座標)
 * @param   ex_bmpwin 呼び出し側でフォントOAMに関連付けるBMPWINを持っている場合はここで渡す。
 *                NULLの場合は中で作成されます。(ex_bmpwinを使用する場合はstrはNULLでOK)
 */
//--------------------------------------------------------------
static void Sub_FontOamCreate(BI_PARAM_PTR bip, FONT_ACTOR *font_actor, const STRBUF *str,
  PRINTSYS_LSB color, int pal_offset, int pal_index,
  int x, int y, int pos_center, FONT_EX_BMP *ex_bmp )
{
  BMPOAM_ACT_DATA finit;
  GFL_BMP_DATA   *bmp;
//  CHAR_MANAGER_ALLOCDATA cma;
  int vram_size;
  BMPOAM_ACT_PTR fontoam;
  int font_len, char_len;

  GF_ASSERT(font_actor->fontoam == NULL);

  //文字列のドット幅から、使用するキャラ数を算出する
  if( ex_bmp == NULL){
    FontLenGet(str, bip->font, &font_len, &char_len);
  }
  else{
    font_len = ex_bmp->font_len;
    char_len = ex_bmp->char_len;
  }

  //BMP作成
  if( ex_bmp == NULL ){
    u8 letter, shadow, back;
    bmp = GFL_BMP_Create( char_len, 16 / 8, GFL_BMP_16_COLOR, bip->heapID );
    GFL_FONTSYS_GetColor( &letter, &shadow, &back );
    GFL_FONTSYS_SetColor( PRINTSYS_LSB_GetL( color ), PRINTSYS_LSB_GetS( color ), PRINTSYS_LSB_GetB( color ) );
    PRINTSYS_Print( bmp, 0, 0, str, bip->font );
    GFL_FONTSYS_SetColor( letter, shadow, back );
//    GF_STR_PrintExpand(&bmpwin, font_type, str, 0, 0, MSG_NO_PUT, color,
//      PANEL_MSG_MARGIN, 0, NULL);
//    GF_STR_PrintColor(&bmpwin, font_type, str, 0, 0, MSG_NO_PUT, color, NULL );
  }
  else{
    bmp = ex_bmp->bmp;
  }

//  vram_size = FONTOAM_NeedCharSize(&bmpwin, NNS_G2D_VRAM_TYPE_2DSUB,  HEAPID_BATTLE);
//  CharVramAreaAlloc(vram_size, CHARM_CONT_AREACONT, NNS_G2D_VRAM_TYPE_2DSUB, &cma);

  //座標位置修正
  if(pos_center == FONTOAM_CENTER){
    x -= font_len / 2;
  }
  y += BATTLE_SUB_ACTOR_DISTANCE_INTEGER - 8;

  finit.bmp = bmp;
  finit.pltt_index = bip->objplttID;
  finit.pal_offset = pal_offset;
  finit.x = x;
  finit.y = y;
  finit.bg_pri = PANEL_MSG_BGPRI;
  finit.soft_pri = PANEL_MSG_SOFTPRI;
  finit.setSerface = CLSYS_DEFREND_SUB;
  finit.draw_type = CLSYS_DRAW_SUB; // サブ画面

  fontoam = BmpOam_ActorAdd( bip->fontoam_sys, &finit );
  BmpOam_ActorBmpTrans( fontoam );

  //解放処理
  if(ex_bmp == NULL){
    GFL_BMP_Delete( bmp );
  }

  font_actor->fontoam = fontoam;
//  font_actor->cma = cma;
  font_actor->font_len = font_len;
}

//--------------------------------------------------------------
/**
 * @brief   生成されているフォントOAMを全て削除をする
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_FontOamDeleteAll(BI_PARAM_PTR bip)
{
  int i;

  for(i = 0; i < FA_NO_MAX; i++){
    if(bip->font_actor[i].fontoam != NULL){
      BmpOam_ActorDel( bip->font_actor[i].fontoam );
//      CharVramAreaFree(&bip->font_actor[i].cma);
      bip->font_actor[i].fontoam = NULL;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   パネル連結スクリーンを書き込む
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_JointScreenWrite(BI_PARAM_PTR bip)
{
  u8 hit_range[ BTL_CLIENT_MAX ], connect_range[ BTL_CLIENT_MAX ];
  int x, y;
  u16 *scrn, *write_scrn;

  scrn = GFL_BG_GetScreenBufferAdrs( BI_FRAME_BF );

  Sub_PokeSelectHitRange( bip, hit_range, FALSE );

  //-- CLIENT_TYPE_AとBの2つの連結箇所さえ調べれば4つのパネルの連結全てがチェック出来る --//

  //CLIENT_TYPE_A
  if(hit_range[0] == TRUE){
    Sub_PokeSelectPanelConnectRange(bip, SELECT_TARGET_A, connect_range, FALSE);

    if(connect_range[SELECT_TARGET_C - SELECT_TARGET_A] == TRUE){
      //A-Cが繋がっている
      for(y = 12; y <= 16; y++){
        write_scrn = &scrn[y * 32];
        for(x = 0xf; x <= 0x10; x++){
          write_scrn[x] = JOINT_SCRN_CODE;
        }
      }
    }
    if(connect_range[SELECT_TARGET_D - SELECT_TARGET_A] == TRUE){
      //A-Dが繋がっている
      for(y = 10; y <= 10; y++){
        write_scrn = &scrn[y * 32];
        for(x = 2; x <= 12; x++){
          write_scrn[x] = JOINT_SCRN_CODE;
        }
      }
    }
  }

  //CLIENT_TYPE_B
  if(hit_range[1] == TRUE){
    Sub_PokeSelectPanelConnectRange(bip, SELECT_TARGET_B, connect_range, FALSE);

    if(connect_range[SELECT_TARGET_C - SELECT_TARGET_A] == TRUE){
      //B-Cが繋がっている
      for(y = 10; y <= 10; y++){
        write_scrn = &scrn[y * 32];
        for(x = 0x13; x <= 0x1d; x++){
          write_scrn[x] = JOINT_SCRN_CODE;
        }
      }
    }
    if(connect_range[SELECT_TARGET_D - SELECT_TARGET_A] == TRUE){
      //B-Dが繋がっている
      for(y = 3; y <= 7; y++){
        write_scrn = &scrn[y * 32];
        for(x = 0xf; x <= 0x10; x++){
          write_scrn[x] = JOINT_SCRN_CODE;
        }
      }
    }
  }
}


//--------------------------------------------------------------
/**
 * @brief   指定クライアントタイプのMEMORY_DECORD_WORKを取得する
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   client_type   クライアントタイプ
 *
 * @retval  MEMORY_DECORD_WORKへのポインタ
 */
//--------------------------------------------------------------
static MEMORY_DECORD_WORK *MemoryDecordWorkGet(BI_PARAM_PTR bip, int client_type)
{
  if(client_type >= CLIENT_TYPE_A){
    client_type -= CLIENT_TYPE_A;
  }
  return &bip->memory_decord[client_type];
}

//--------------------------------------------------------------
/**
 * @brief   MEMORY_DECORD_WORKを解放する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void MemoryDecordWorkFree(BI_PARAM_PTR bip)
{
  int i, client;

  for(client = 0; client < BTL_CLIENT_MAX; client++){
    for(i = 0; i < PTL_WAZA_MAX; i++){
      GFL_HEAP_FreeMemory(bip->memory_decord[client].typeicon_cgx[i]);
      if(bip->memory_decord[client].exbmp_waza[i].bmp != NULL){
        GFL_BMP_Delete( bip->memory_decord[client].exbmp_waza[i].bmp );
      }
      if(bip->memory_decord[client].exbmp_pp[i].bmp != NULL){
        GFL_BMP_Delete( bip->memory_decord[client].exbmp_pp[i].bmp );
      }
      if(bip->memory_decord[client].exbmp_ppmax[i].bmp != NULL){
        GFL_BMP_Delete( bip->memory_decord[client].exbmp_ppmax[i].bmp );
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   技パラメータのデータをメモリに展開する
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   client_type   クライアントタイプ
 * @param   wazapara    技パラメータ
 */
//--------------------------------------------------------------
void BINPUT_WazaParaMemoryDecord(BI_PARAM_PTR bip, int client_type,
  const BINPUT_WAZA_PARAM *wazapara)
{
  MEMORY_DECORD_WORK *mdw;
  void *arc_data;
  NNSG2dCharacterData *char_data;
  int char_size, i, waza_type;
  STRBUF *wazaname_p;
  STRBUF *wazaname_src;
  STRBUF *pp_p;
  STRBUF *pp_src;
  STRBUF *ppmsg_src;
  WORDSET *wordset;
  PRINTSYS_LSB color;

  mdw = MemoryDecordWorkGet(bip, client_type);

#ifdef OSP_BINPUT_ON
  OS_TPrintf("client_type = %d\n", client_type);
#endif

  char_size = WAZATYPEICON_OAMSIZE;

  wazaname_p = GFL_STR_CreateBuffer( BUFLEN_WAZA_NAME, bip->heapID );
  wazaname_src = GFL_MSG_CreateString( bip->msg,  WazaNameMsg );
  ppmsg_src = GFL_MSG_CreateString( bip->msg,  PPMsg );
  wordset = WORDSET_Create( bip->heapID );
  pp_p = GFL_STR_CreateBuffer( BUFLEN_BI_WAZAPP, bip->heapID );
  pp_src = GFL_MSG_CreateString( bip->msg,  PPNowMaxMsg );

  for(i = 0; i < PTL_WAZA_MAX; i++){
    if(wazapara->wazano[i] != mdw->wazapara.wazano[i] && wazapara->wazano[i] != 0){
      //技タイプアイコン
      waza_type = WT_WazaDataParaGet(wazapara->wazano[i], ID_WTD_wazatype);
      arc_data = GFL_ARC_UTIL_LoadOBJCharacter( WazaTypeIcon_ArcIDGet(),
        WazaTypeIcon_CgrIDGet( waza_type ), WAZATYPEICON_COMP_CHAR,
        &char_data, bip->heapID );
      MI_CpuCopy32(char_data->pRawData, mdw->typeicon_cgx[i], char_size);
      GFL_HEAP_FreeMemory( arc_data );
    }

    if(mdw->exbmp_waza[i].bmp == NULL
        || (wazapara->wazano[i] != mdw->wazapara.wazano[i] && wazapara->wazano[i] != 0)){
      //BMPWIN：技名
      WORDSET_RegisterWazaName( wordset, 0, wazapara->wazano[i] );
      WORDSET_ExpandStr( wordset, wazaname_p, wazaname_src );
      FontExBmpwin_FontSet( bip, wazaname_p, &mdw->exbmp_waza[i], MSGCOLOR_WAZA );
    }

    if(mdw->exbmp_pp[i].bmp == NULL || mdw->exbmp_ppmax[i].bmp == NULL
        || wazapara->wazano[i] != 0 || wazapara->wazano[i] != mdw->wazapara.wazano[i]
        || wazapara->pp[i] != mdw->wazapara.pp[i]
        || wazapara->ppmax[i] != mdw->wazapara.ppmax[i]){

      WORDSET_RegisterNumber(wordset, 0, wazapara->pp[i], 2,
        STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
      WORDSET_RegisterNumber(wordset, 1, wazapara->ppmax[i], 2,
        STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
      WORDSET_ExpandStr(wordset, pp_p, pp_src);
      color = PP_FontColorGet(wazapara->pp[i], wazapara->ppmax[i]);

      if(mdw->exbmp_pp[i].bmp == NULL
          || wazapara->wazano[i] != mdw->wazapara.wazano[i]
          || wazapara->pp[i] != mdw->wazapara.pp[i]){
        //BMPWIN：PP
        FontExBmpwin_FontSet(bip, pp_p, &mdw->exbmp_pp[i], color);
      }
      if(mdw->exbmp_ppmax[i].bmp == NULL
          || wazapara->wazano[i] != mdw->wazapara.wazano[i]
          || wazapara->pp[i] != mdw->wazapara.pp[i]){
        //BMPWIN：PPMAX
        FontExBmpwin_FontSet(bip, ppmsg_src, &mdw->exbmp_ppmax[i], color);
      }
    }
  }

  WORDSET_Delete( wordset );
  GFL_STR_DeleteBuffer( wazaname_p );
  GFL_STR_DeleteBuffer(ppmsg_src);
  GFL_STR_DeleteBuffer(wazaname_src);
  GFL_STR_DeleteBuffer(pp_src);
  GFL_STR_DeleteBuffer(pp_p);

  mdw->wazapara = *wazapara;
}

//--------------------------------------------------------------
/**
 * @brief   FONT_EX_BMPに対してフォントデータを作成し、セットする
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   str       文字列へのポインタ
 * @param   font_type   フォントタイプ
 * @param   ex_bmpwin   データ代入先
 * @param   color     文字カラー
 */
//--------------------------------------------------------------
static void FontExBmpwin_FontSet(BI_PARAM_PTR bip, const STRBUF *str,
  FONT_EX_BMP *ex_bmp, PRINTSYS_LSB color)
{
  int font_len, char_len;
  u8 letter, shadow, back;

  FontLenGet(str, bip->font, &font_len, &char_len);
  ex_bmp->font_len = font_len;
  ex_bmp->char_len = char_len;
  if(ex_bmp->bmp != NULL){
    GFL_BMP_Delete( ex_bmp->bmp );
  }
  ex_bmp->bmp = GFL_BMP_Create( char_len, 16 / 8, GFL_BMP_16_COLOR, bip->heapID );
  GFL_FONTSYS_GetColor( &letter, &shadow, &back );
  GFL_FONTSYS_SetColor( PRINTSYS_LSB_GetL( color ), PRINTSYS_LSB_GetS( color ), PRINTSYS_LSB_GetB( color ) );
  PRINTSYS_Print( ex_bmp->bmp, 0, 0, str, bip->font );
  GFL_FONTSYS_SetColor( letter, shadow, back );
}


//--------------------------------------------------------------
/**
 * @brief   技タイプアイコンを全て生成する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_WazaTypeIconCreateAll(BI_PARAM_PTR bip)
{
  int i;
  GFL_CLWK_DATA obj_param;
  BINPUT_SCENE_WAZA *bsw;
  int waza_type;
  int ret;
  MEMORY_DECORD_WORK *mdw;

  bsw = &bip->scene.bsw;
  mdw = MemoryDecordWorkGet(bip, bip->client_type);

  obj_param = WazaTypeIconObjParam;

  for(i = 0; i < PTL_WAZA_MAX; i++){
    GF_ASSERT(bip->icon_cap[i] == NULL);
    if(bsw->wazano[i] != 0){
      waza_type = WT_WazaDataParaGet(bsw->wazano[i], ID_WTD_wazatype);

      obj_param.pos_x = WazaIconPos[i][0];
      obj_param.pos_y = WazaIconPos[i][1];
      bip->icon_cap[i] = GFL_CLACT_WK_Create( bip->wazatype_clunit,
                          bip->wazatype_charID[ i ], bip->wazatype_plttID, bip->wazatype_cellID,
                          &obj_param, CLSYS_DEFREND_SUB, bip->heapID );

      GFL_CLACT_WK_SetPlttOffs( bip->icon_cap[ i ], WazaTypeIcon_PlttOffsetGet(waza_type), CLWK_PLTTOFFS_MODE_PLTT_TOP );

      {//キャラ転送
        void *obj_vram;
        NNSG2dImageProxy image;

        obj_vram = G2S_GetOBJCharPtr();
        GFL_CLACT_WK_GetImgProxy( bip->icon_cap[ i ],  &image );
        MI_CpuCopy16(mdw->typeicon_cgx[i], (void*)((u32)obj_vram
          + image.vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_2DSUB]),
          WAZATYPEICON_OAMSIZE );
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   技タイプアイコンを全て削除する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_WazaTypeIconDeleteAll(BI_PARAM_PTR bip)
{
  int i;

  for(i = 0; i < PTL_WAZA_MAX; i++){
    if(bip->icon_cap[i] != NULL){
      GFL_CLACT_WK_Remove( bip->icon_cap[ i ] );
      bip->icon_cap[i] = NULL;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   技分類アイコンを全て生成する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_WazaKindIconCreateAll(BI_PARAM_PTR bip)
{
  //使用されていない模様
#if 0
  int i;
  CATS_SYS_PTR csp;
  CATS_RES_PTR crp;
  GFL_CLWK_DATA obj_param;
  int waza_kind;
  BINPUT_SCENE_WAZA *bsw;

  bsw = &bip->scene.bsw;
  csp = BattleWorkCATS_SYS_PTRGet(bip->bw);
  crp = BattleWorkCATS_RES_PTRGet(bip->bw);

  obj_param = WazaKindIconObjParam;
  WazaTypeIcon_PlttWorkResourceLoad(BattleWorkPfdGet(bip->bw), FADE_SUB_OBJ,
    csp, crp, NNS_G2D_VRAM_TYPE_2DSUB, PLTTID_WAZAKIND_ICON);
  WazaTypeIcon_CellAnmResourceLoad(csp, crp, CELLID_WAZAKIND_ICON, CELLANMID_WAZAKIND_ICON);
  for(i = 0; i < WAZA_TEMOTI_MAX; i++){
    GF_ASSERT(bip->kindicon_cap[i] == NULL);
    if(bsw->wazano[i] != 0){
      waza_kind = WT_WazaDataParaGet(bsw->wazano[i], ID_WTD_kind);
      WazaKindIcon_CharResourceLoad(csp, crp, NNS_G2D_VRAM_TYPE_2DSUB,
        waza_kind, CHARID_WAZAKIND_ICON_1 + i);

      obj_param.id[CLACT_U_CHAR_RES] = CHARID_WAZAKIND_ICON_1 + i;
      obj_param.x = WazaClassPos[i][0];
      obj_param.y = WazaClassPos[i][1];
      bip->kindicon_cap[i] = WazaKindIcon_ActorCreate(csp, crp,
        waza_kind, &obj_param);
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   技分類アイコンを全て削除する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_WazaKindIconDeleteAll(BI_PARAM_PTR bip)
{
  //使用されていない模様
#if 0
  int i;

  for(i = 0; i < PTL_WAZA_MAX; i++){
    if(bip->kindicon_cap[i] != NULL){
      WazaKindIcon_ActorDelete(bip->kindicon_cap[i]);
      WazaKindIcon_CharResourceFree(crp, CHARID_WAZAKIND_ICON_1 + i);
      bip->kindicon_cap[i] = NULL;
    }
  }
#if 0 //技タイプアイコンのリソースが常駐になったので、同じIDを使用しているこれを解放しない
  WazaKindIcon_PlttResourceFree(crp, PLTTID_WAZAKIND_ICON);
  WazaKindIcon_CellAnmResourceFree(crp, CELLID_WAZAKIND_ICON, CELLANMID_WAZAKIND_ICON);
#endif
#endif
}

//--------------------------------------------------------------
/**
 * @brief   技タイプ毎にCGRデータを書き換える
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   waza_type   技タイプ(手持ち無しの場合は-1)
 * @param   waza_pos    手持ちの技の位置(0～3)
 */
//--------------------------------------------------------------
static void Sub_WazaTypeCGRTrans(BI_PARAM_PTR bip, int waza_type, int waza_pos)
{
#if 0
  GF_BGL_INI *bgl;
  u32 screen_type;

  bgl = BattleWorkGF_BGL_INIGet(bip->bw);
  screen_type = GF_BGL_ScreenTypeGet(bgl, BI_FRAME_PANEL);

  if(waza_type == -1){  //手持ちありの場合はデフォルトでOK
    WazaPanel_EasyCharLoad(bgl, waza_type, HEAPID_BATTLE, BI_FRAME_PANEL,
      WazaTypeCgrPos[waza_pos], screen_type);
  }
#endif
  WazaPanel_EasyPalLoad( bip->pfd, waza_type, bip->heapID, FADE_SUB_BG,
    WAZATYPE_START_PALPOS + waza_pos );
}

//--------------------------------------------------------------
/**
 * @brief   手持ち無しの技タイプ毎のCGRデータをまとめてセット
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   waza_pos    手持ち無しの技の開始位置(0～3)
 */
//--------------------------------------------------------------
static void Sub_WazaTypeNotTemotiChainCGRTrans(BI_PARAM_PTR bip, int waza_pos)
{
#if 0
  GF_BGL_INI *bgl;
  u32 screen_type;
  PALETTE_FADE_PTR pfd;
  int i;

  pfd = BattleWorkPfdGet(bip->bw);
  bgl = BattleWorkGF_BGL_INIGet(bip->bw);
  screen_type = GF_BGL_ScreenTypeGet(bgl, BI_FRAME_PANEL);

  WazaPanel_EasyCharLoad_ChainSet(bgl, -1, HEAPID_BATTLE, BI_FRAME_PANEL,
    &WazaTypeCgrPos[waza_pos], WAZA_TEMOTI_MAX - waza_pos, screen_type);
  for(i = waza_pos; i < WAZA_TEMOTI_MAX; i++){
    WazaPanel_EasyPalLoad(pfd, -1, HEAPID_BATTLE, FADE_SUB_BG,
      WAZATYPE_START_PALPOS + i);
  }
#else
  int i;

  for(i = waza_pos; i < PTL_WAZA_MAX; i++){
    Sub_ScrnOffsetRewrite(bip, &WazaButtonNotTouchScrnOffset[i],
      &WazaButtonNotTouchScrnRect[i], SCRNBUF_WAZA, 0);
    PaletteWorkSet(bip->pfd, &bip->pal_buf[BI_NOT_SELECT_PANEL_PALNO * 16],
      FADE_SUB_BG, (WAZATYPE_START_PALPOS + i) * 16, 0x20);
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   選択出来ないポケモンのパネルをセットする
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   poke_pos    ポケモンの位置(CLIENT_TYPE_???)
 */
//--------------------------------------------------------------
static void Sub_PokemonNotPanelTrans(BI_PARAM_PTR bip, int poke_pos)
{
  PALETTE_FADE_PTR pfd;
  int i;
  const u8 PokePalNo[] = {6, 0xc, 0xd, 5};  //CLIENT_TYPE順

  Sub_ScrnOffsetRewrite(bip, &PokemonButtonNotTouchScrnOffset[poke_pos],
    &PokemonButtonNotTouchScrnRect[poke_pos], SCRNBUF_POKESELE, 0);
  PaletteWorkSet(bip->pfd, &bip->pal_buf[BI_NOT_SELECT_PANEL_PALNO * 16],
    FADE_SUB_BG, PokePalNo[poke_pos] * 16, 0x20);

  GFL_BG_LoadScreenV_Req( BI_FRAME_PANEL );

  //枠の削除
  GFL_BG_FillScreen( BI_FRAME_BF, BG_CLEAR_CODE,
    PokemonButtonNotTouchScrnRect[poke_pos].left,
    PokemonButtonNotTouchScrnRect[poke_pos].top,
    PokemonButtonNotTouchScrnRect[poke_pos].right
    - PokemonButtonNotTouchScrnRect[poke_pos].left + 1,
    PokemonButtonNotTouchScrnRect[poke_pos].bottom
    - PokemonButtonNotTouchScrnRect[poke_pos].top + 1,
    GFL_BG_SCRWRT_PALIN);
  GFL_BG_LoadScreenV_Req( BI_FRAME_BF );
}

//--------------------------------------------------------------
/**
 * @brief   背景以外のフレームを全て非表示にする
 */
//--------------------------------------------------------------
static void Sub_BackScrnOnlyVisible(void)
{
  int i;

  for(i = 0; i < BI_BG_NUM; i++){
    if(GFL_BG_FRAME0_S + i != BI_FRAME_BACK){
      GFL_BG_SetVisible(GFL_BG_FRAME0_S + i, VISIBLE_OFF);
    }
    else{
      GFL_BG_SetVisible(GFL_BG_FRAME0_S + i, VISIBLE_ON);
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   パネルタッチエフェクト終了後、背景のみにするための処理を色々する(フォントOBJの削除とか)
 * @param   none    BIシステムワークへのポインタ
 * @param   obj     TRUE：OBJ削除
 * @param   bg      TRUE：BG削除
 */
//--------------------------------------------------------------
//static void Sub_TouchEndDelete(BI_PARAM_PTR bip, int obj, int bg)
void Sub_TouchEndDelete(BI_PARAM_PTR bip, int obj, int bg)
{
  if(obj == TRUE){
    if( bip->objplttID != GFL_CLGRP_REGISTER_FAILED){
      GFL_CLGRP_PLTT_Release( bip->objplttID );
      bip->objplttID = GFL_CLGRP_REGISTER_FAILED;
    }
    Sub_SceneOBJDelete(bip);
  }
  if(bg == TRUE){
    GFUser_VIntr_CreateTCB(VWait_BackScrnOnlyVisible, bip, VWAIT_TCBPRI_BACKSCRN_VISIBLE);
  }
}

//--------------------------------------------------------------
/**
 * @brief   BGタイプが切り替わる毎に削除する必要のあるOBJのDelete処理
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Sub_SceneOBJDelete(BI_PARAM_PTR bip)
{
  Sub_WazaTypeIconDeleteAll(bip);
  //使用されていない模様
//  Sub_WazaKindIconDeleteAll(bip);

  Sub_FontOamDeleteAll(bip);

//  Sub_PokeIconResourceFree(bip);
//  Sub_PokeIconActorAllDel(bip);
}

//--------------------------------------------------------------
/**
 * @brief   現在表示中のポケモン選択BGで、どのポケモンが選択可能なのかを取得する
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   hit_range   結果代入先(CLIENT_MAX分の配列要素を持つワークのポインタ)
 * @param   check_exist   TRUE=サーバーから渡されているexistフラグも加味する。
 *
 * hit_rangeに入る値はTRUEならば選択可能、FALSEは選択不可です。
 */
//--------------------------------------------------------------
static void Sub_PokeSelectHitRange(BI_PARAM_PTR bip, u8 *hit_range, int check_exist)
{
#if 0
  BINPUT_SCENE_POKE *bsp;
  int i;
  u8 client_no_buf[ BTL_CLIENT_MAX ];
  int client_no;

  bsp = &bip->scene.bsp;
  BattleClientNoBufMake(bip->bw, client_no_buf);
  for(i = 0; i < BTL_CLIENT_MAX; i++){
    client_no = client_no_buf[ CLIENT_TYPE_A + i];
    if(check_exist == TRUE && bsp->dspp[client_no].exist == FALSE){
      hit_range[i] = FALSE;
    }
    else{
      hit_range[i] = PokeSelectHitRange[bip->pokesele_type][i];
    }
  }
#else
  int i;

  for(i = 0; i < BTL_CLIENT_MAX; i++){
    hit_range[i] = TRUE;
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   選択したポケモンとパネルが繋がっているものを取得する
 *
 * @param   bip         BIシステムワークへのポインタ
 * @param   target        選択したポケモン(SELECT_TARGET_A等)
 * @param   connect_range   結果代入先(CLIENT_MAX分の配列要素を持つワークのポインタ)
 * @param   check_exist   TRUE=サーバーから渡されているexistフラグも加味する。
 *
 * connect_rangeに入る値はTRUEならば接続、FALSEは接続していません。
 */
//--------------------------------------------------------------
static void Sub_PokeSelectPanelConnectRange(BI_PARAM_PTR bip, int target, u8 *connect_range,
  int check_exist)
{
  BINPUT_SCENE_POKE *bsp;
  int i;

  bsp = &bip->scene.bsp;
  switch(bip->pokesele_type){
  case POKESELE_A_B_C_D:
  case POKESELE_B_C_D:
  case POKESELE_A_B_D:
  case POKESELE_A_C:
  case POKESELE_B_D:
    for(i = 0; i < BTL_CLIENT_MAX; i++){
      if(i == target - SELECT_TARGET_A){
        connect_range[i] = TRUE;
      }
      else{
        connect_range[i] = FALSE;
      }
    }
    break;
  default:
    Sub_PokeSelectHitRange(bip, connect_range, check_exist);
    break;
  }
}



//==============================================================================
//
//  エフェクトTCB
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   エフェクトTCBを生成する
 *
 * @param   func    TCBにセットする関数ポインタ
 * @param   bip     BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void EffectTCB_Add(GFL_TCB_FUNC func, BI_PARAM_PTR bip)
{
  GF_ASSERT(bip->effect_tcb == NULL);

  MI_CpuClear8(&bip->effect_work, sizeof(BI_EFFECT_WORK));
  bip->effect_tcb = GFL_TCB_AddTask( bip->tcbsys, func, bip, TCBPRI_BINPUT_EFFECT );
}

//--------------------------------------------------------------
/**
 * @brief   エフェクトTCBを削除する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void EffectTCB_Delete(BI_PARAM_PTR bip)
{
  if(bip->effect_tcb != NULL){
    GFL_TCB_DeleteTask( bip->effect_tcb );
    bip->effect_tcb = NULL;
    MI_CpuClear8(&bip->effect_work, sizeof(BI_EFFECT_WORK));
  }
}

//--------------------------------------------------------------
/**
 * @brief   カラーエフェクトTCBを生成する
 *
 * @param   func    TCBにセットする関数ポインタ
 * @param   bip     BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void ColorEffTCB_Add(GFL_TCB_FUNC func, BI_PARAM_PTR bip)
{
  GF_ASSERT(bip->color_eff_tcb == NULL);

  MI_CpuClear8(&bip->color_work, sizeof(BI_COLOR_EFF_WORK));
  bip->color_eff_tcb = GFL_TCB_AddTask( bip->tcbsys, func, bip, TCBPRI_BINPUT_COLOR_EFFECT );
}

//--------------------------------------------------------------
/**
 * @brief   カラーエフェクトTCBを削除する
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void ColorEffTCB_Delete(BI_PARAM_PTR bip)
{
  if(bip->color_eff_tcb != NULL){
    GFL_TCB_DeleteTask( bip->color_eff_tcb );
    bip->color_eff_tcb = NULL;
    MI_CpuClear8(&bip->color_work, sizeof(BI_COLOR_EFF_WORK));
  }
}

//--------------------------------------------------------------
/**
 * @brief   ポケモン選択用のカラーフェードタスクを生成
 * @param   bip   BIシステムワークへのポインタ
 * @param   tp_ret
 */
//--------------------------------------------------------------
static void ColorEffTCB_PokeSeleFadeSet(BI_PARAM_PTR bip, int tp_ret)
{
  return;   //光らせないようにした 2006.05.26(金)

  ColorEffTCB_Add(ColorEff_PokeSeleFade, bip);
  bip->color_work.tp_ret = tp_ret;
}

//--------------------------------------------------------------
/**
 * @brief   特定の位置のカラーだけをフェードさせるタスクを生成する
 * @param   bip   BIシステムワークへのポインタ
 * @param   pal_pos フェードさせるパレット番号
 */
//--------------------------------------------------------------
static void ColorEffTCB_PointFadeSet(BI_PARAM_PTR bip, int pal_pos)
{
  return;   //光らせないようにした 2006.05.26(金)

  if(pal_pos == 0xff){
    return;
  }

  ColorEffTCB_Add(ColorEff_PointFade, bip);
  bip->color_work.pal_pos = pal_pos;
}

//--------------------------------------------------------------
/**
 * @brief   特定の位置のカラーだけをフェードさせる
 *
 * @param   tcb     TCBへのポインタ
 * @param   work    BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void ColorEff_PointFade(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  BI_COLOR_EFF_WORK *cw;
  int i, end;

  cw = &bip->color_work;
  end = FALSE;

  switch(cw->seq){
  case 0:
    cw->evy = TOUCH_FADE_EVY;
    cw->evy_add = -TOUCH_FADE_ADD_EVY;
    //最初の計算分を足しこんでおく
    cw->evy -= cw->evy_add;

    cw->seq++;
    //break;
  case 1:
    cw->evy += cw->evy_add;
    if(cw->evy_add >= 0 && cw->evy >= (16<<8)){
      cw->evy = 16<<8;
      end = TRUE;
    }
    else if(cw->evy_add < 0 && cw->evy <= 0){
      cw->evy = 0;
      end = TRUE;
    }

  #if 0
    //全体を暗く
    SoftFadePfd(pfd, FADE_SUB_BG, cw->pal_pos * 16, 16, cw->evy >> 8, 0x0000);
    //枠だけ白く
    SoftFadePfd(pfd, FADE_SUB_BG, cw->pal_pos * 16 + 3, 1, cw->evy >> 8, 0x7fff);
  #else
    SoftFadePfd(bip->pfd, FADE_SUB_BG, cw->pal_pos * 16, 16, (cw->evy >> 8) / 2, 0x7fff);
    //全体を暗く
    SoftFadePfd(bip->pfd, FADE_SUB_BG, cw->pal_pos * 16 + 1, 1, cw->evy >> 8, 0x7b1a);
    //枠だけ白く
    SoftFadePfd(bip->pfd, FADE_SUB_BG, cw->pal_pos * 16 + 0xa, 1, cw->evy >> 8, 0x4634);
  #endif
    if(end == TRUE){
      ColorEffTCB_Delete(bip);
      return;
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   ポケモン選択用のカラーフェードタスク
 *
 * @param   tcb     TCBへのポインタ
 * @param   work    BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void ColorEff_PokeSeleFade(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  BI_COLOR_EFF_WORK *cw;
  int i, end;
  u8 connect_range[ BTL_CLIENT_MAX ];

  cw = &bip->color_work;
  end = FALSE;

  switch(cw->seq){
  case 0:
    cw->evy = TOUCH_FADE_EVY;
    cw->evy_add = -TOUCH_FADE_ADD_EVY;
    //最初の計算分を足しこんでおく
    cw->evy -= cw->evy_add;

    cw->seq++;
    //break;
  case 1:
    cw->evy += cw->evy_add;
    if(cw->evy_add >= 0 && cw->evy >= (16<<8)){
      cw->evy = 16<<8;
      end = TRUE;
    }
    else if(cw->evy_add < 0 && cw->evy <= 0){
      cw->evy = 0;
      end = TRUE;
    }

    Sub_PokeSelectPanelConnectRange(bip, cw->tp_ret, connect_range, TRUE);
    for(i = 0; i < BTL_CLIENT_MAX; i++){
      if(connect_range[i] == TRUE){
      #if 0
        //全体を暗く
        SoftFadePfd(pfd, FADE_SUB_BG, PokeSeleMenuPaletteNo[i] * 16,
          16, cw->evy >> 8, 0x0000);
        //枠だけ白く
        SoftFadePfd(pfd, FADE_SUB_BG, PokeSeleMenuPaletteNo[i] * 16 + 3, 1,
          cw->evy >> 8, 0x7fff);
      #else
        SoftFadePfd( bip->pfd, FADE_SUB_BG, PokeSeleMenuPaletteNo[i] * 16,
          16, (cw->evy >> 8) / 2, 0x7fff);
        //全体を暗く
        SoftFadePfd( bip->pfd, FADE_SUB_BG, PokeSeleMenuPaletteNo[i] * 16 + 1,
          1, cw->evy >> 8, 0x7b1a);
        //枠だけ白く
        SoftFadePfd( bip->pfd, FADE_SUB_BG, PokeSeleMenuPaletteNo[i] * 16 + 0xa, 1,
          cw->evy >> 8, 0x4634);
      #endif
      }
    }

    if(end == TRUE){
      ColorEffTCB_Delete(bip);
      return;
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   スクリーン書き換え
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   scrn_offset   加算するスクリーン値
 * @param   range     スクリーン書き換え範囲
 * @param   scrnbuf_no    元データとなるスクリーンバッファの番号
 * @param   anm_no      アニメ番号
 */
//--------------------------------------------------------------
static void Sub_ScrnOffsetRewrite(BI_PARAM_PTR bip, const s16 *scrn_offset,
  const REWRITE_SCRN_RECT *range, int scrnbuf_no, int anm_no)
{
  int x, y;
  u16 *scrn, *write_scrn;
  u16 *src_scrn, *read_scrn;
  int add_scrn;

  scrn = GFL_BG_GetScreenBufferAdrs( BI_FRAME_PANEL );
  src_scrn = bip->scrn_buf[scrnbuf_no];
  add_scrn = scrn_offset[anm_no];

  for(y = range->top; y <= range->bottom; y++){
    write_scrn = &scrn[y * 32];
    read_scrn = &src_scrn[y * 32];
    for(x = range->left; x <= range->right; x++){
      write_scrn[x] = read_scrn[x] + add_scrn;
    }
  }

  GFL_BG_LoadScreenV_Req( BI_FRAME_PANEL );
}

//--------------------------------------------------------------
/**
 * @brief   パネル部分をキャラ転送アニメを実行して書き換えます。
 *          1つのパネルに対して1つのフォントOBJがセットになっているもの用の汎用タスクです
 *
 * @param   tcb   TCBへのポインタ
 * @param   work  BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Effect_ButtonDown(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  s16 x, y;

  switch(bip->effect_work.seq){
  case 0:
    Sub_ScrnOffsetRewrite(bip, bip->effect_work.paracgr.scrn_offset,
      bip->effect_work.paracgr.scrn_range, bip->effect_work.paracgr.scrnbuf_no, 2);

    BmpOam_ActorGetPos( bip->font_actor[bip->effect_work.paracgr.fa_no].fontoam, &x, &y );
    BmpOam_ActorSetPos( bip->font_actor[bip->effect_work.paracgr.fa_no].fontoam, x, y + SCRN_SCROLL_A_EFF_Y );

    if(bip->effect_work.paracgr.pokeicon_no != NOT_POKE_ICON
        && bip->pokeicon_cap[bip->effect_work.paracgr.pokeicon_no] != NULL){
      y = GFL_CLACT_WK_GetTypePos( bip->pokeicon_cap[ bip->effect_work.paracgr.pokeicon_no ],
                     CLSYS_DRAW_SUB, CLSYS_MAT_Y );
      y += SCRN_SCROLL_A_EFF_Y;
      GFL_CLACT_WK_SetTypePos( bip->pokeicon_cap[ bip->effect_work.paracgr.pokeicon_no ],
                   y, CLSYS_DRAW_SUB, CLSYS_MAT_Y );
    }

    bip->effect_work.seq++;
    break;
  case 1:
    bip->effect_work.wait++;
    if(bip->effect_work.wait <= SCRN_TOUCH_ANM_WAIT){
      break;
    }

    bip->effect_work.wait = 0;
    bip->effect_work.seq++;
    //break;

  case 2:
    Sub_ScrnOffsetRewrite(bip, bip->effect_work.paracgr.scrn_offset,
      bip->effect_work.paracgr.scrn_range, bip->effect_work.paracgr.scrnbuf_no, 1);

    BmpOam_ActorGetPos( bip->font_actor[bip->effect_work.paracgr.fa_no].fontoam, &x, &y );
    BmpOam_ActorSetPos( bip->font_actor[bip->effect_work.paracgr.fa_no].fontoam, x, y + SCRN_SCROLL_BACK_A_EFF_Y);

    if(bip->effect_work.paracgr.pokeicon_no != NOT_POKE_ICON
        && bip->pokeicon_cap[bip->effect_work.paracgr.pokeicon_no] != NULL){
      y = GFL_CLACT_WK_GetTypePos( bip->pokeicon_cap[ bip->effect_work.paracgr.pokeicon_no ],
                     CLSYS_DRAW_SUB, CLSYS_MAT_Y );
      y += SCRN_SCROLL_BACK_A_EFF_Y;
      GFL_CLACT_WK_SetTypePos( bip->pokeicon_cap[ bip->effect_work.paracgr.pokeicon_no ],
                   y, CLSYS_DRAW_SUB, CLSYS_MAT_Y );
    }

    bip->effect_work.seq++;
    break;

  default:
    bip->effect_work.wait++;
    if(bip->effect_work.wait > SCRN_TOUCH_WAIT){
      Sub_TouchEndDelete(bip, bip->effect_work.paracgr.obj_del, FALSE);
      EffectTCB_Delete(bip);
      return;
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   パネル部分をキャラ転送アニメを実行して書き換えます。
 *          技選択時専用です。
 *
 * @param   tcb   TCBへのポインタ
 * @param   work  BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Effect_WazaButtonDown(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  s16 x, y;
  int fa_waza, fa_pp, fa_ppmsg;
  int cap_icon, kind_icon, wazano;
  BINPUT_SCENE_WAZA *bsw;

  bsw = &bip->scene.bsw;

  switch(bip->effect_work.paracgr.tp_ret){
  case SELECT_SKILL_1:
    fa_waza = FA_NO_WAZA_1;
    fa_pp = FA_NO_PP_1;
    fa_ppmsg = FA_NO_PPMSG_1;
    cap_icon = 0;
    kind_icon = 0;
    wazano = bsw->wazano[0];
    break;
  case SELECT_SKILL_2:
    fa_waza = FA_NO_WAZA_2;
    fa_pp = FA_NO_PP_2;
    fa_ppmsg = FA_NO_PPMSG_2;
    cap_icon = 1;
    kind_icon = 1;
    wazano = bsw->wazano[1];
    break;
  case SELECT_SKILL_3:
    fa_waza = FA_NO_WAZA_3;
    fa_pp = FA_NO_PP_3;
    fa_ppmsg = FA_NO_PPMSG_3;
    cap_icon = 2;
    kind_icon = 2;
    wazano = bsw->wazano[2];
    break;
  case SELECT_SKILL_4:
    fa_waza = FA_NO_WAZA_4;
    fa_pp = FA_NO_PP_4;
    fa_ppmsg = FA_NO_PPMSG_4;
    cap_icon = 3;
    kind_icon = 3;
    wazano = bsw->wazano[3];
    break;
  case SELECT_CANCEL:
  default:
    fa_waza = 0;
    fa_pp = 0;
    fa_ppmsg = 0;
    cap_icon = 0;
    kind_icon = 0;
    wazano = 0;
    break;
  }

  switch(bip->effect_work.seq){
  case 0:
    Sub_ScrnOffsetRewrite(bip, bip->effect_work.paracgr.scrn_offset,
      bip->effect_work.paracgr.scrn_range, bip->effect_work.paracgr.scrnbuf_no, 2);

    if(bip->effect_work.paracgr.tp_ret != SELECT_CANCEL){
      BmpOam_ActorGetPos(bip->font_actor[fa_waza].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_waza].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      BmpOam_ActorGetPos(bip->font_actor[fa_pp].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_pp].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      BmpOam_ActorGetPos(bip->font_actor[fa_ppmsg].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_ppmsg].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      if(wazano != 0){
        y = GFL_CLACT_WK_GetTypePos( bip->icon_cap[cap_icon], CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        y += SCRN_SCROLL_A_EFF_Y;
        GFL_CLACT_WK_SetTypePos( bip->icon_cap[cap_icon], y, CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        //技分類アイコンは未使用なのでコメントアウト
#if 0
        if(bip->kindicon_cap[cap_icon] != NULL){
          CATS_ObjectPosMove(bip->kindicon_cap[cap_icon]->act, 0, SCRN_SCROLL_A_EFF_Y);
        }
#endif
      }
    }
    else{
      BmpOam_ActorGetPos(bip->font_actor[FA_NO_WAZA_MODORU].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[FA_NO_WAZA_MODORU].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
    }

    bip->effect_work.seq++;
    break;
  case 1:
    bip->effect_work.wait++;
    if(bip->effect_work.wait <= SCRN_TOUCH_ANM_WAIT){
      break;
    }
    bip->effect_work.wait = 0;
    bip->effect_work.seq++;
    //break;

  case 2:
    Sub_ScrnOffsetRewrite(bip, bip->effect_work.paracgr.scrn_offset,
      bip->effect_work.paracgr.scrn_range, bip->effect_work.paracgr.scrnbuf_no, 1);

    if(bip->effect_work.paracgr.tp_ret != SELECT_CANCEL){
      BmpOam_ActorGetPos(bip->font_actor[fa_waza].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_waza].fontoam, x, y + SCRN_SCROLL_BACK_A_EFF_Y);
      BmpOam_ActorGetPos(bip->font_actor[fa_pp].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_pp].fontoam, x, y + SCRN_SCROLL_BACK_A_EFF_Y);
      BmpOam_ActorGetPos(bip->font_actor[fa_ppmsg].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_ppmsg].fontoam, x, y + SCRN_SCROLL_BACK_A_EFF_Y);
      if(wazano != 0){
        y = GFL_CLACT_WK_GetTypePos( bip->icon_cap[cap_icon], CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        y += SCRN_SCROLL_BACK_A_EFF_Y;
        GFL_CLACT_WK_SetTypePos( bip->icon_cap[cap_icon], y, CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        //技分類アイコンは未使用なのでコメントアウト
#if 0
        if(bip->kindicon_cap[cap_icon] != NULL){
          CATS_ObjectPosMove(bip->kindicon_cap[cap_icon]->act,
            0, SCRN_SCROLL_BACK_A_EFF_Y);
        }
#endif
      }
    }
    else{
      BmpOam_ActorGetPos(bip->font_actor[FA_NO_WAZA_MODORU].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[FA_NO_WAZA_MODORU].fontoam, x, y + SCRN_SCROLL_BACK_A_EFF_Y);
    }

    bip->effect_work.seq++;
    break;

  default:
    bip->effect_work.wait++;
    if(bip->effect_work.wait > SCRN_TOUCH_WAIT){
      Sub_TouchEndDelete(bip, TRUE, TRUE);
      EffectTCB_Delete(bip);
      return;
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   パネル部分をキャラ転送アニメを実行して書き換えます。
 *          ポケモン選択時(技効果範囲)専用です。
 *
 * @param   tcb   TCBへのポインタ
 * @param   work  BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Effect_PokeSeleButtonDown(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  s16 x, y;
  u8 connect_range[ BTL_CLIENT_MAX ];
  int i;

  switch(bip->effect_work.seq){
  case 0:
    Sub_PokeSelectPanelConnectRange(bip, bip->effect_work.pokesele.tp_ret,
      connect_range, TRUE);

    for(i = 0; i < BTL_CLIENT_MAX; i++){
      if(connect_range[i] == TRUE){
        Sub_ScrnOffsetRewrite(bip, PokemonButtonScrnOffset[i],
          &PokemonButtonScrnRect[i], SCRNBUF_POKESELE, 2);

        if(bip->font_actor[FA_NO_POKE_A + i].fontoam != NULL){
          BmpOam_ActorGetPos(bip->font_actor[FA_NO_POKE_A + i].fontoam, &x, &y);
          BmpOam_ActorSetPos(bip->font_actor[FA_NO_POKE_A + i].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
        }
        if(bip->pokeicon_cap[i] != NULL){
          y = GFL_CLACT_WK_GetTypePos( bip->pokeicon_cap[ i ], CLSYS_DRAW_SUB, CLSYS_MAT_Y );
          y += SCRN_SCROLL_A_EFF_Y;
          GFL_CLACT_WK_SetTypePos( bip->pokeicon_cap[ i ], y, CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        }
      }
    }
    bip->effect_work.seq++;
    break;
  case 1:
    bip->effect_work.wait++;
    if(bip->effect_work.wait <= SCRN_TOUCH_ANM_WAIT){
      break;
    }
    bip->effect_work.wait = 0;
    bip->effect_work.seq++;
    //break;

  case 2:
    Sub_PokeSelectPanelConnectRange(bip, bip->effect_work.pokesele.tp_ret,
      connect_range, TRUE);

    for(i = 0; i < BTL_CLIENT_MAX; i++){
      if(connect_range[i] == TRUE){
        Sub_ScrnOffsetRewrite(bip, PokemonButtonScrnOffset[i],
          &PokemonButtonScrnRect[i], SCRNBUF_POKESELE, 1);

        if(bip->font_actor[FA_NO_POKE_A + i].fontoam != NULL){
          BmpOam_ActorGetPos(bip->font_actor[FA_NO_POKE_A + i].fontoam, &x, &y);
          BmpOam_ActorSetPos(bip->font_actor[FA_NO_POKE_A + i].fontoam,
            x, y + SCRN_SCROLL_BACK_A_EFF_Y);
        }
        if(bip->pokeicon_cap[i] != NULL){
          y = GFL_CLACT_WK_GetTypePos( bip->pokeicon_cap[ i ], CLSYS_DRAW_SUB, CLSYS_MAT_Y );
          y += SCRN_SCROLL_BACK_A_EFF_Y;
          GFL_CLACT_WK_SetTypePos( bip->pokeicon_cap[ i ], y, CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        }
      }
    }

    bip->effect_work.seq++;
    break;
  case 3:
    bip->effect_work.wait++;
    if(bip->effect_work.wait > SCRN_TOUCH_WAIT){
      bip->effect_work.wait = 0;
      bip->effect_work.seq++;
    }
    break;

  default:
    Sub_TouchEndDelete(bip, TRUE, TRUE);
    EffectTCB_Delete(bip);
    return;
  }
}

//--------------------------------------------------------------
/**
 * @brief   ewpに入っているデータを元にスクリーン書き換えエフェクトを実行する
 *          1つのパネルに対して1つのフォントOBJがセットになっているもの用の汎用タスクです
 *
 * @param   tcb   TCBへのポインタ
 * @param   work  BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Effect_ScrnTouch(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  s16 x, y;


  switch(bip->effect_work.seq){
  case 0:
    Sub_ScrnTouchChange( bip->ewp.rsr, bip->ewp.rsr_num, bip->ewp.add_charname);

    BmpOam_ActorGetPos(bip->font_actor[bip->effect_work.para.fa_no].fontoam, &x, &y);
    BmpOam_ActorSetPos(bip->font_actor[bip->effect_work.para.fa_no].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);

    bip->effect_work.seq++;
    break;
  case 1:
    bip->effect_work.wait++;
    if(bip->effect_work.wait > SCRN_TOUCH_WAIT){
      bip->effect_work.wait = 0;
      bip->effect_work.seq++;
    }
    break;
  case 20:
    Sub_ScrnTouchChangeReverse( bip->ewp.rsr, bip->ewp.rsr_num, bip->ewp.add_charname);

    BmpOam_ActorGetPos(bip->font_actor[bip->effect_work.para.fa_no].fontoam, &x, &y);
    BmpOam_ActorSetPos(bip->font_actor[bip->effect_work.para.fa_no].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);

    bip->effect_work.seq++;
    break;
  default:
    bip->effect_work.wait++;
    if(bip->effect_work.wait > SCRN_TOUCH_WAIT){
      Sub_TouchEndDelete(bip, TRUE, TRUE);
      EffectTCB_Delete(bip);
      return;
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   技選択用のスクリーン書き換えエフェクトを実行する
 *
 * @param   tcb   TCBへのポインタ
 * @param   work  BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void Effect_WazaScrnTouch(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  s16 x, y;
  int fa_waza, fa_pp, fa_ppmsg;
  int cap_icon, kind_icon, wazano;
  BINPUT_SCENE_WAZA *bsw;

  bsw = &bip->scene.bsw;

  switch(bip->effect_work.waza.tp_ret){
  case SELECT_SKILL_1:
    fa_waza = FA_NO_WAZA_1;
    fa_pp = FA_NO_PP_1;
    fa_ppmsg = FA_NO_PPMSG_1;
    cap_icon = 0;
    kind_icon = 0;
    wazano = bsw->wazano[0];
    break;
  case SELECT_SKILL_2:
    fa_waza = FA_NO_WAZA_2;
    fa_pp = FA_NO_PP_2;
    fa_ppmsg = FA_NO_PPMSG_2;
    cap_icon = 1;
    kind_icon = 1;
    wazano = bsw->wazano[1];
    break;
  case SELECT_SKILL_3:
    fa_waza = FA_NO_WAZA_3;
    fa_pp = FA_NO_PP_3;
    fa_ppmsg = FA_NO_PPMSG_3;
    cap_icon = 2;
    kind_icon = 2;
    wazano = bsw->wazano[2];
    break;
  case SELECT_SKILL_4:
    fa_waza = FA_NO_WAZA_4;
    fa_pp = FA_NO_PP_4;
    fa_ppmsg = FA_NO_PPMSG_4;
    cap_icon = 3;
    kind_icon = 3;
    wazano = bsw->wazano[3];
    break;
  case SELECT_CANCEL:
  default:
    fa_waza = 0;
    fa_ppmsg = 0;
    fa_pp = 0;
    cap_icon = 0;
    kind_icon = 0;
    wazano = 0;
    break;
  }

  switch(bip->effect_work.seq){
  case 0:
    Sub_ScrnTouchChange( bip->ewp.rsr, bip->ewp.rsr_num, bip->ewp.add_charname);

    if(bip->effect_work.waza.tp_ret != SELECT_CANCEL){
      BmpOam_ActorGetPos(bip->font_actor[fa_waza].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_waza].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      BmpOam_ActorGetPos(bip->font_actor[fa_pp].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_pp].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      BmpOam_ActorGetPos(bip->font_actor[fa_ppmsg].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_ppmsg].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      if(wazano != 0){
        y = GFL_CLACT_WK_GetTypePos( bip->icon_cap[cap_icon], CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        y += SCRN_SCROLL_A_EFF_Y;
        GFL_CLACT_WK_SetTypePos( bip->icon_cap[cap_icon], y, CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        //技分類アイコンは未使用なのでコメントアウト
#if 0
        if(bip->kindicon_cap[cap_icon] != NULL){
          CATS_ObjectPosMove(bip->kindicon_cap[cap_icon]->act, 0, SCRN_SCROLL_A_EFF_Y);
        }
#endif
      }
    }
    else{
      BmpOam_ActorGetPos(bip->font_actor[FA_NO_WAZA_MODORU].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[FA_NO_WAZA_MODORU].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
    }

    bip->effect_work.seq++;
    break;
  case 1:
    bip->effect_work.wait++;
    if(bip->effect_work.wait > SCRN_TOUCH_WAIT){
      bip->effect_work.wait = 0;
      bip->effect_work.seq++;
    }
    break;
  case 20:
    Sub_ScrnTouchChangeReverse( bip->ewp.rsr, bip->ewp.rsr_num, bip->ewp.add_charname);

    if(bip->effect_work.waza.tp_ret != SELECT_CANCEL){
      BmpOam_ActorGetPos(bip->font_actor[fa_waza].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_waza].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      BmpOam_ActorGetPos(bip->font_actor[fa_pp].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_pp].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      BmpOam_ActorGetPos(bip->font_actor[fa_ppmsg].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[fa_ppmsg].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
      if(wazano != 0){
        y = GFL_CLACT_WK_GetTypePos( bip->icon_cap[cap_icon], CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        y += SCRN_SCROLL_A_EFF_Y;
        GFL_CLACT_WK_SetTypePos( bip->icon_cap[cap_icon], y, CLSYS_DRAW_SUB, CLSYS_MAT_Y );
        //技分類アイコンは未使用なのでコメントアウト
#if 0
        if(bip->kindicon_cap[cap_icon] != NULL){
          CATS_ObjectPosMove(bip->kindicon_cap[cap_icon]->act, 0, SCRN_SCROLL_A_EFF_Y);
        }
#endif
      }
    }
    else{
      BmpOam_ActorGetPos(bip->font_actor[FA_NO_WAZA_MODORU].fontoam, &x, &y);
      BmpOam_ActorSetPos(bip->font_actor[FA_NO_WAZA_MODORU].fontoam, x, y + SCRN_SCROLL_A_EFF_Y);
    }

    bip->effect_work.seq++;
    break;
  default:
    bip->effect_work.wait++;
    if(bip->effect_work.wait > SCRN_TOUCH_WAIT){
      Sub_TouchEndDelete(bip, TRUE, TRUE);
      EffectTCB_Delete(bip);
      return;
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   PPの表示色を取得する
 *
 * @param   pp      現在のPP
 * @param   pp_max    最大PP
 *
 * @retval  カラー
 */
//--------------------------------------------------------------
static PRINTSYS_LSB PP_FontColorGet(int pp, int pp_max)
{
  if(pp == 0){
    return MSGCOLOR_PP_RED;
  }
  if(pp_max == pp){
    return MSGCOLOR_PP_BLACK;
  }
  if(pp_max <= 2){
    if(pp == 1){
      return MSGCOLOR_PP_ORANGE;
    }
  }
  else if(pp_max <= 7){
    switch(pp){
    case 1:
      return MSGCOLOR_PP_ORANGE;
    case 2:
      return MSGCOLOR_PP_YELLOW;
    }
  }
  else{
    if(pp <= pp_max / 4){
      return MSGCOLOR_PP_ORANGE;
    }
    if(pp <= pp_max / 2){
      return MSGCOLOR_PP_YELLOW;
    }
  }
  return MSGCOLOR_PP_BLACK;
}



//==============================================================================
//
//  コマンド選択画面スクロールインエフェクト
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   コマンド選択画面：スクロールインエフェクト実行タスク
 *
 * @param   tcb
 * @param   work
 */
//--------------------------------------------------------------
static void CommandInEffTask(GFL_TCB *tcb, void *work)
{
  COMMAND_IN_EFF_WORK *ciew = work;

  switch(ciew->seq){
  case 0:
    ciew->x0_l += COMMANDIN_SCR_X0_ADD;
    ciew->y1_d -= COMMANDIN_SCR_Y1_ADD;
    ciew->set_x0_l = (COMMANDIN_SCR_X0_START - ciew->x0_l) / 100;
    ciew->set_y1_d = -ciew->y1_d / 100;

    if(ciew->x0_l >= COMMANDIN_WND_START_X0 || ciew->y1_d <= COMMANDIN_WND_START_Y0){
      ciew->x0_l = COMMANDIN_WND_START_X0;
      ciew->y1_d = COMMANDIN_WND_START_Y1;
      ciew->set_x0_l = COMMANDIN_WND_START_X0 / 100;
      ciew->set_y1_d = COMMANDIN_WND_START_Y0 / 100;
      ciew->seq++;
    }

    break;

  case 1:
    ciew->bip->touch_invalid = FALSE;
    GFL_TCB_DeleteTask(ciew->vtask_tcb);
    GFL_TCB_DeleteTask(ciew->htask_tcb);
    GXS_SetVisibleWnd(GX_WNDMASK_NONE);

    BINPUT_BackFadeReq(ciew->bip, BINPUT_BACKFADE_DARK);
    ciew->seq++;
    break;
  default:
    if(BINPUT_BackFadeExeCheck(ciew->bip) == FALSE){
      break;
    }

    GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_X_SET, 0 );
    GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_Y_SET, 0 );
    GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_X_SET, 0 );
    GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_Y_SET, 0 );

//    BattleWorkCommandSelectFlagSet(ciew->bip->bw, SLIDE_IN_AFTER);

    GFL_HEAP_FreeMemory(work);
    GFL_TCB_DeleteTask(tcb);
    return;
  }
}

//--------------------------------------------------------------
/**
 * @brief   コマンド選択画面スクロールインエフェクト：Vブランク転送タスク
 *
 * @param   tcb
 * @param   work
 */
//--------------------------------------------------------------
static void VBlankTCB_CommandInEff(GFL_TCB *tcb, void *work)
{
  COMMAND_IN_EFF_WORK *ciew = work;
  s32 set_x, set_y, x;

  if(ciew->client_type == CLIENT_TYPE_C){
    x = 255 - ciew->set_x0_l;
    if(x > 0){
      x = 0;
    }
    GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_X_SET, x);
    GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_X_SET, 255 - ciew->set_x0_l);
    set_x = -ciew->x0_l / 100;
  }
  else{
    x = ciew->set_x0_l;
    if(x < 0){
      x = 0;
    }
    GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_X_SET, x);
    GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_X_SET, ciew->set_x0_l);
    set_x = COMMANDIN_WND_END_X0 + ciew->x0_l / 100;
  }
  GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_Y_SET, 0);
  GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_Y_SET, 0);

  set_y = COMMANDIN_WND_START_Y1 + (ciew->y1_d) / 100;
  if(set_x < 0){
    set_x = 0;
  }
  else if(set_x > 255){
    set_x = 255;
  }
  if(set_y > 192){
    set_y = 192;
  }
  else if(set_y < 0){
    set_y = 0;
  }

  if(ciew->client_type == CLIENT_TYPE_C){
    G2S_SetWnd0Position(COMMANDIN_WND_START_X0, COMMANDIN_WND_START_Y0,
      set_x, COMMANDIN_WND_END_Y0);
  }
  else{
    if(set_x == 0){
      set_x = 1;  //RIGHTを0始まりにして、右端から囲っている為のケア
    }
    G2S_SetWnd0Position(set_x, COMMANDIN_WND_START_Y0,
      0/*COMMANDIN_WND_END_X0*/, COMMANDIN_WND_END_Y0);
  }
  G2S_SetWnd1Position(COMMANDIN_WND_START_X1, COMMANDIN_WND_START_Y1,
    COMMANDIN_WND_END_X0, set_y);

  ciew->h_set_y1_d = ciew->set_y1_d;
}

//--------------------------------------------------------------
/**
 * @brief   コマンド選択画面スクロールインエフェクト：Hブランク割り込み関数
 *
 * @param   work
 */
//--------------------------------------------------------------
static void HBlankTCB_CommandInEff( GFL_TCB *tcb, void *work )
{
  COMMAND_IN_EFF_WORK *ciew = work;
  s32 vcount, x;

  vcount = GX_GetVCount();
  if(vcount == COMMANDIN_WND_START_Y1){
    GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_X_SET, COMMANDIN_WND_START_X1);
    GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_Y_SET, ciew->h_set_y1_d);
    GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_X_SET, COMMANDIN_WND_START_X1);
    GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_Y_SET, ciew->h_set_y1_d);
  }
  else if(vcount > 192){
    if(ciew->client_type == CLIENT_TYPE_C){
      x = 255 - ciew->set_x0_l;
      if(x > 0){
        x = 0;
      }
      GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_X_SET, 255 - ciew->set_x0_l);
      GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_X_SET, 255 - ciew->set_x0_l);
    }
    else{
      x = ciew->set_x0_l;
      if(x < 0){
        x = 0;
      }
      GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_X_SET, ciew->set_x0_l);
      GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_X_SET, ciew->set_x0_l);
    }
    GFL_BG_SetScroll( BI_FRAME_PANEL, GFL_BG_SCROLL_Y_SET, 0);
    GFL_BG_SetScroll( BI_FRAME_BF, GFL_BG_SCROLL_Y_SET, 0);
  }
}




//==============================================================================
//
//  Vブランク処理
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   各フレームのVisible設定を行う
 *
 * @param   tcb     TCBへのポインタ
 * @param   work    BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void VWait_FrameVisibleUpdate(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  const BG_MAKE_DATA *bmd;
  int i;

  bmd = &BgMakeData[bip->makedata_no];

  //各BG面の表示・非表示設定
  for(i = 0; i < BI_BG_NUM; i++){
    if(bmd->scr_id[i] == NONE_ID){
      GFL_BG_SetVisible(GFL_BG_FRAME0_S + i, VISIBLE_OFF);
    }
    else{
      GFL_BG_SetVisible(GFL_BG_FRAME0_S + i, VISIBLE_ON);
    }
  }

  //各BG面のBGプライオリティ更新
  for(i = 0; i < BI_BG_NUM; i++){
    GFL_BG_SetPriority(GFL_BG_FRAME0_S + i, bmd->pri[i]);
  }

  GFL_TCB_DeleteTask(tcb);
}

//--------------------------------------------------------------
/**
 * @brief   背景以外のフレーム全てを非表示にする
 *
 * @param   tcb   TCBへのポインタ
 * @param   work  BIシステムワークへのポインタ
 *
 * @retval
 *
 *
 */
//--------------------------------------------------------------
static void VWait_BackScrnOnlyVisible(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;

  Sub_BackScrnOnlyVisible();
  GFL_TCB_DeleteTask(tcb);
}

//--------------------------------------------------------------
/**
 * @brief   Vブランク中にCGR転送アニメを実行します
 *
 * @param   tcb     TCBへのポインタ
 * @param   work    CGR転送アニメパラメータワークへのポインタ
 */
//--------------------------------------------------------------
static void VWait_CgrParamTrans(GFL_TCB *tcb, void *work)
{
  CGR_TRANS_PARAM *ctp = work;

  Sub_CgrParamTrans(ctp);
  ctp->v_tcb = NULL;
  GFL_TCB_DeleteTask(tcb);
}





//==============================================================================
//
//  背景フェード
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   戦闘入力画面、背景フェードリクエスト
 * @param   bip       BIシステムワークへのポインタ
 * @param   fade_dir    BINPUT_BACKFADE_???
 */
//--------------------------------------------------------------
void BINPUT_BackFadeReq(BI_PARAM_PTR bip, int fade_dir)
{
#if 0 //フェードなくなった 2006.05.19(金)
  if(bip->backfade_flag == fade_dir){
    return;   //既にリクエストした状態になっている
  }

  if(bip->backfade_tcb != NULL){
    GF_ASSERT(0 && "背景フェードのリクエストが既に動作中です！\n");
    return;
  }

  bip->backfade_seq = 0;
  bip->backfade_flag = fade_dir;
  bip->backfade_tcb = TCB_Add(BackFadeTask, bip, TCBPRI_BINPUT_BACKFADE);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   背景フェード実行タスク
 * @param   tcb     TCBへのポインタ
 * @param   work    BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void BackFadeTask(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;

  switch(bip->backfade_seq){
  case 0:
    if(bip->backfade_flag == BINPUT_BACKFADE_DARK){
      PaletteFadeReq( bip->pfd, PF_BIT_SUB_BG, BACKFADE_FADEBIT,
        BACKFADE_FADE_WAIT, 0, BACKFADE_EVY, BACKFADE_COLOR, bip->tcbsys );
    }
    else{
      PaletteFadeReq( bip->pfd, PF_BIT_SUB_BG, BACKFADE_FADEBIT,
        BACKFADE_FADE_WAIT, BACKFADE_EVY, 0, BACKFADE_COLOR, bip->tcbsys );
    }
    bip->backfade_seq++;
    break;
  default:
    if(PaletteFadeCheck( bip->pfd ) == 0){
      bip->backfade_seq = 0;
      GFL_TCB_DeleteTask(tcb);
      bip->backfade_tcb = NULL;
      return;
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * @brief   背景フェードが実行されているか確認
 * @param   bip   BIシステムワークへのポインタ
 * @retval  TRUE:終了している。　FALSE:動いている
 */
//--------------------------------------------------------------
BOOL BINPUT_BackFadeExeCheck(BI_PARAM_PTR bip)
{
  if(bip->backfade_tcb == NULL){
    return TRUE;
  }
  return FALSE;
}

///常駐フェードのEVY最大値
#define DEF_FADE_EVY_MAX      (10)
///常駐フェードのEVY加算値
#define DEF_FADE_EVY_ADD      (0x0080)
///常駐フェードのカラーコード
#define DEF_FADE_COLOR        (0x7b1a)

///常駐フェード：枠のEVY最大値
#define DEF_FADE_WAKU_EVY_MAX   (16)
///常駐フェード：枠のEVY加算値
#define DEF_FADE_WAKU_EVY_ADD   (0x0200)
///常駐フェード：枠のカラーコード
#define DEF_FADE_WAKU_COLOR     (0x7e37)  //(0x7fff)
//--------------------------------------------------------------
/**
 * @brief   常駐フェード実行タスク
 * @param   tcb     TCBへのポインタ
 * @param   work    BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void DefaultFadeAnimeTask(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  int i;
  u16 color;
  PALETTE_FADE_PTR pfd;
  s16 evy;

  if(PaletteFadeCheck(bip->pfd) != 0){
    return;   //全体フェード中は実行しない
  }

#if 0   //画面全体フェード停止 2006.05.12(金)
  color = DEF_FADE_COLOR;
  evy = bip->def_fade_evy >> 8;

  //パレット反映
  for(i = 1; i < 7; i++){
    SoftFadePfd(pfd, FADE_SUB_BG, 16 * i + 0xa, 1, evy, color);
  }
  for(i = 8; i < 0xf; i++){
    SoftFadePfd(pfd, FADE_SUB_BG, 16 * i + 0xa, 1, evy, color);
  }

  //EVY更新
  if(bip->def_fade_dir == 0){
    bip->def_fade_evy += DEF_FADE_EVY_ADD;
  }
  else{
    bip->def_fade_evy -= DEF_FADE_EVY_ADD;
  }
  if(bip->def_fade_evy >= (DEF_FADE_EVY_MAX+1) << 8){
    bip->def_fade_evy = (DEF_FADE_EVY_MAX-1) << 8;
    bip->def_fade_dir = 1;
  }
  else if(bip->def_fade_evy <= 0){
    bip->def_fade_evy = 1 << 8;
    bip->def_fade_dir = 0;
  }
#endif

  //-- ポケモン選択画面のみの常駐フェード --//
  {
    SoftFadePfd(bip->pfd, FADE_SUB_BG, 16 * 0 + 1, 1, bip->waku_fade_evy >> 8, DEF_FADE_WAKU_COLOR);
    //EVY更新
    if(bip->waku_fade_dir == 0){
      bip->waku_fade_evy += DEF_FADE_WAKU_EVY_ADD;
    }
    else{
      bip->waku_fade_evy -= DEF_FADE_WAKU_EVY_ADD;
    }
    if(bip->waku_fade_evy >= (DEF_FADE_WAKU_EVY_MAX+1) << 8){
      bip->waku_fade_evy = (DEF_FADE_WAKU_EVY_MAX-1) << 8;
      bip->waku_fade_dir = 1;
    }
    else if(bip->waku_fade_evy <= 0){
      bip->waku_fade_evy = 1 << 8;
      bip->waku_fade_dir = 0;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   タッチされている時は背景のパレットを専用のパレットに変更する
 *
 * @param   tcb     TCBへのポインタ
 * @param   work    BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static void BackGroundTouchPaletteCheck(GFL_TCB *tcb, void *work)
{
  BI_PARAM_PTR bip = work;
  PALETTE_FADE_PTR pfd;
  u16 *trans_wk;
  int size;

  //メッセージスキップの挙動がどうなるかわからないので、保留
#if 0
  int msg_skip, msg_wait;

  msg_skip = MsgPrintBattleSkipPushFlagGet();
  msg_wait = MsgPrintBattleWaitPushFlagGet();
  MsgPrintBattleSkipPushFlagClear();
  MsgPrintBattleWaitPushFlagClear();
#endif

  if(PaletteFadeCheck(bip->pfd) != 0){
    return;
  }

  trans_wk = PaletteWorkTransWorkGet(bip->pfd, FADE_SUB_BG);
  size = (16 - BACKGROUND_CHANGE_PAL_START) * 2;
  //メッセージスキップの挙動がどうなるかわからないので、保留
//  if(sys.tp_cont && (msg_skip == 1 || msg_wait == 1)){
  if( GFL_UI_TP_GetCont() ){
    if(memcmp(&trans_wk[BACKGROUND_CHANGE_PAL_START],
        &bip->background_pal_normal[BACKGROUND_CHANGE_PAL_START], size) == 0){
#if 0
      if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_TOWER){
        MI_CpuCopy16(&bip->background_pal_touch[0],
          &trans_wk[0], 0x20);
        MI_CpuCopy16(&bip->background_pal_touch[16],
          &trans_wk[FRONTIER_PALANM_POS * 16], 0x20);
      }
      else{
        MI_CpuCopy16(&bip->background_pal_touch[BACKGROUND_CHANGE_PAL_START],
          &trans_wk[BACKGROUND_CHANGE_PAL_START], size);
      }
#else
      MI_CpuCopy16(&bip->background_pal_touch[BACKGROUND_CHANGE_PAL_START],
        &trans_wk[BACKGROUND_CHANGE_PAL_START], size);
#endif
    }
  }
  else{
    if(memcmp(&trans_wk[BACKGROUND_CHANGE_PAL_START],
        &bip->background_pal_touch[BACKGROUND_CHANGE_PAL_START], size) == 0){
#if 0
      if(BattleWorkFightTypeGet(bip->bw) & FIGHT_TYPE_TOWER){
        MI_CpuCopy16(&bip->background_pal_normal[0],
          &trans_wk[0], 0x20);
        MI_CpuCopy16(&bip->background_pal_normal[16],
          &trans_wk[FRONTIER_PALANM_POS * 16], 0x20);
      }
      else{
        MI_CpuCopy16(&bip->background_pal_normal[BACKGROUND_CHANGE_PAL_START],
          &trans_wk[BACKGROUND_CHANGE_PAL_START], size);
      }
#else
      MI_CpuCopy16(&bip->background_pal_normal[BACKGROUND_CHANGE_PAL_START],
        &trans_wk[BACKGROUND_CHANGE_PAL_START], size);
#endif
    }
  }
}


//==============================================================================
//
//  キー入力
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   キー入力判定処理
 * @param   bip   BIシステムワークへのポインタ
 * @retval  結果(当たりなしの場合はGFL_UI_TP_HIT_NONE)
 */
//--------------------------------------------------------------
static int CursorCheck(BI_PARAM_PTR bip)
{
  CURSOR_MOVE *move;
  const BG_MAKE_DATA *bmd;
  int trg = GFL_UI_KEY_GetTrg();

  move = &bip->cursor_move;
  bmd = &BgMakeData[bip->makedata_no];

  if(bmd->cursor_move_func == NULL){
    return GFL_UI_TP_HIT_NONE;
  }

  if(move->cursor_on == FALSE){
    if(bip->decend_key == TRUE ||
        (trg & (PAD_BUTTON_A|PAD_BUTTON_B|PAD_BUTTON_X|PAD_BUTTON_Y
        |PAD_KEY_RIGHT|PAD_KEY_LEFT|PAD_KEY_UP|PAD_KEY_DOWN))){
      if(bip->decend_key == FALSE){
//        Snd_SePlay(BCURSOR_MOVE_SE);  //キーを押してカーソルを表示した時音を鳴らす
      }
      move->cursor_on = TRUE;
      bip->decend_key = FALSE;
      bmd->cursor_move_func(bip, TRUE);
    }
    return GFL_UI_TP_HIT_NONE;
  }

  return bmd->cursor_move_func(bip, FALSE);
}

//--------------------------------------------------------------
/**
 * @brief   カーソル移動：コマンド選択
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   init_flag   TRUE:初期化処理
 *
 * @retval  結果(当たりなしの場合はGFL_UI_TP_HIT_NONE)
 */
//--------------------------------------------------------------
static int CursorMove_CommandSelect(BI_PARAM_PTR bip, int init_flag)
{
  CURSOR_MOVE *move;
  u32 key;
  const BG_MAKE_DATA *bmd;
  int i, index, client_no;
  CURSOR_SAVE *cursor_save;
  int trg = GFL_UI_KEY_GetTrg();

  move = &bip->cursor_move;
  bmd = &BgMakeData[bip->makedata_no];
  cursor_save = &bip->cursor_save;

  if(init_flag == TRUE){
    move->x_menu = cursor_save->command_x;
    move->y_menu = cursor_save->command_y;
    index = CursorMoveDataCommandSelect[move->y_menu][move->x_menu];
    BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
      bmd->tpd[index].rect.right-8,
      bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    return GFL_UI_TP_HIT_NONE;
  }

  switch(bip->makedata_no){
  case BINPUT_TYPE_FIGHTONLY:
  case BINPUT_COMMAND_IN_FIGHTONLY:
    key = CursorMove_KeyCheckMove(move,
      CURSOR_COMMAND_FIGHTONLY_X_MENU_NUM, CURSOR_COMMAND_FIGHTONLY_Y_MENU_NUM,
      CursorMoveDataCommandSelect[0]);
    break;
  default:
    index = CursorMoveDataCommandSelect[move->y_menu][move->x_menu];
    if(index == COMMSELE_INDEX_ESCAPE && (trg & PAD_KEY_UP)){
      ; //「にげる」にカーソルがあっている時の”上キー”は無効
    }
    else{
      key = CursorMove_KeyCheckMove(move,
        CURSOR_COMMAND_SELECT_X_MENU_NUM, CURSOR_COMMAND_SELECT_Y_MENU_NUM,
        CursorMoveDataCommandSelect[0]);
      if(key == 0 && index == COMMSELE_INDEX_FIGHT){
        //「たたかう」にカーソルがあっている場合、左右キーを押したらポケモン、バッグへ行く
        if(trg & PAD_KEY_LEFT){
          move->x_menu = 0;
          move->y_menu = 1;
//          Snd_SePlay(BCURSOR_MOVE_SE);
          key = PAD_KEY_LEFT;
        }
        else if(trg & PAD_KEY_RIGHT){
          move->x_menu = 2;
          move->y_menu = 1;
//          Snd_SePlay(BCURSOR_MOVE_SE);
          key = PAD_KEY_RIGHT;
        }
      }
    }
  #if 0 //3段目「にげる」の処理はなくなった 2006.06.29(木)
    if(move->y_menu == 2){
      //3段目の「にげる」にカーソルが移動した場合は2段目の正規の場所の
      //「にげる」にカーソルを移動させる
      move->y_menu = 1;
      move->x_menu = 1;
    }
  #endif
    break;
  }

  switch(key){
  case PAD_KEY_UP:
  case PAD_KEY_DOWN:
  case PAD_KEY_LEFT:
  case PAD_KEY_RIGHT:
    index = CursorMoveDataCommandSelect[move->y_menu][move->x_menu];
    BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
      bmd->tpd[index].rect.right-8,
      bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    break;
  case PAD_BUTTON_DECIDE:
    return CursorMoveDataCommandSelect[move->y_menu][move->x_menu];
  case PAD_BUTTON_CANCEL:
    if(bip->command_modoru_type == TRUE){
      for(i = 0; i < bmd->tpd[i].rect.top != GFL_UI_TP_HIT_END; i++){
        if(SELECT_ESCAPE_COMMAND == bmd->tpd_ret[i]){
          return i;
        }
      }
    }
    break;
  }

  return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief   カーソル位置保存：コマンド選択
 * @param   bip       BIシステムワークへのポインタ
 * @param   index     選択した項目
 */
//--------------------------------------------------------------
static void CursorSave_CommandSelect(BI_PARAM_PTR bip, int index)
{
  CURSOR_SAVE *cursor_save;
  int x, y;

  if(index != COMMSELE_INDEX_ESCAPE || bip->cancel_escape == FALSE){
    cursor_save = &bip->cursor_save;
    for(y = 0; y < CURSOR_COMMAND_SELECT_Y_MENU_NUM; y++){
      for(x = 0; x < CURSOR_COMMAND_SELECT_X_MENU_NUM; x++){
        if(index == CursorMoveDataCommandSelect[y][x]){
          cursor_save->command_x = x;
          cursor_save->command_y = y;
          return;
        }
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   カーソル移動：ポケパーク用コマンド選択
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   init_flag   TRUE:初期化処理
 *
 * @retval  結果(当たりなしの場合はGFL_UI_TP_HIT_NONE)
 */
//--------------------------------------------------------------
static int CursorMove_ParkCommandSelect(BI_PARAM_PTR bip, int init_flag)
{
  CURSOR_MOVE *move;
  u32 key;
  const BG_MAKE_DATA *bmd;
  int i, index;

  move = &bip->cursor_move;
  bmd = &BgMakeData[bip->makedata_no];

  if(init_flag == TRUE){
    move->x_menu = 0;
    move->y_menu = 0;
    index = CursorMoveDataParkCommandSelect[move->y_menu][move->x_menu];
    BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
      bmd->tpd[index].rect.right-8,
      bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    return GFL_UI_TP_HIT_NONE;
  }

  key = CursorMove_KeyCheckMove(move,
    CURSOR_PARK_COMMAND_SELECT_X_MENU_NUM, CURSOR_PARK_COMMAND_SELECT_Y_MENU_NUM,
    CursorMoveDataParkCommandSelect[0]);

  switch(key){
  case PAD_KEY_UP:
  case PAD_KEY_DOWN:
  case PAD_KEY_LEFT:
  case PAD_KEY_RIGHT:
    index = CursorMoveDataParkCommandSelect[move->y_menu][move->x_menu];
    BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
      bmd->tpd[index].rect.right-8,
      bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    break;
  case PAD_BUTTON_DECIDE:
    return CursorMoveDataParkCommandSelect[move->y_menu][move->x_menu];
  case PAD_BUTTON_CANCEL:
    break;
  }

  return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief   カーソル移動：技選択
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   init_flag   TRUE:初期化処理
 *
 * @retval  結果(当たりなしの場合はGFL_UI_TP_HIT_NONE)
 */
//--------------------------------------------------------------
static int CursorMove_WazaSelect(BI_PARAM_PTR bip, int init_flag)
{
  CURSOR_MOVE *move;
  u32 key;
  const BG_MAKE_DATA *bmd;
  int index, i;
  u8 move_data[CURSOR_WAZA_SELECT_Y_MENU_NUM][CURSOR_WAZA_SELECT_X_MENU_NUM];
  BINPUT_SCENE_WAZA *bsw;
  CURSOR_SAVE *cursor_save;

  cursor_save = &bip->cursor_save;
  move = &bip->cursor_move;
  bmd = &BgMakeData[bip->makedata_no];
  bsw = &bip->scene.bsw;

  if(init_flag == TRUE){
    move->x_menu = cursor_save->waza_x;
    move->y_menu = cursor_save->waza_y;
    index = CursorMoveDataWazaSelect[move->y_menu][move->x_menu];
    if(index != WAZASELE_INDEX_CANCEL && bsw->wazano[index - WAZASELE_INDEX_SKILL_1] == 0){
      cursor_save->waza_x = 0;
      cursor_save->waza_y = 0;
      move->x_menu = 0;
      move->y_menu = 0;
      index = CursorMoveDataWazaSelect[move->y_menu][move->x_menu];
    }
    BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
      bmd->tpd[index].rect.right-8,
      bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    return GFL_UI_TP_HIT_NONE;
  }

  MI_CpuCopy8(CursorMoveDataWazaSelect, move_data,
    CURSOR_WAZA_SELECT_Y_MENU_NUM * CURSOR_WAZA_SELECT_X_MENU_NUM);
#if 0 //手持ち技がない場所もカーソルが移動出来るように変更 2006.05.27(土)
  for(i = 0; i < WAZA_TEMOTI_MAX; i++){
    if(bsw->wazano[i] == 0){
      ((u8*)move_data)[i] = 0xff; //手持ち技がない所は範囲からはずす
    }
  }
#endif

  key = CursorMove_KeyCheckMove(move,
    CURSOR_WAZA_SELECT_X_MENU_NUM, CURSOR_WAZA_SELECT_Y_MENU_NUM,
    move_data[0]);
  switch(key){
  case PAD_KEY_UP:
  case PAD_KEY_DOWN:
  case PAD_KEY_LEFT:
  case PAD_KEY_RIGHT:
    index = CursorMoveDataWazaSelect[move->y_menu][move->x_menu];
    BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
      bmd->tpd[index].rect.right-8,
      bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    break;
  case PAD_BUTTON_DECIDE:
    return CursorMoveDataWazaSelect[move->y_menu][move->x_menu];
  case PAD_BUTTON_CANCEL:
    for(i = 0; i < bmd->tpd[i].rect.top != GFL_UI_TP_HIT_END; i++){
      if(SELECT_CANCEL == bmd->tpd_ret[i]){
        return i;
      }
    }
    break;
  }

  return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief   カーソル位置保存：技選択
 * @param   bip       BIシステムワークへのポインタ
 * @param   index     選択した項目
 */
//--------------------------------------------------------------
static void CursorSave_WazaSelect(BI_PARAM_PTR bip, int index)
{
  CURSOR_SAVE *cursor_save;
  int x, y;

  if(index == WAZASELE_INDEX_CANCEL){
    return;
  }

  cursor_save = &bip->cursor_save;
  for(y = 0; y < CURSOR_WAZA_SELECT_Y_MENU_NUM; y++){
    for(x = 0; x < CURSOR_WAZA_SELECT_X_MENU_NUM; x++){
      if(index == CursorMoveDataWazaSelect[y][x]){
        cursor_save->waza_x = x;
        cursor_save->waza_y = y;
        return;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   カーソル移動：ポケモン選択
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   init_flag   TRUE:初期化処理
 *
 * @retval  結果(当たりなしの場合はGFL_UI_TP_HIT_NONE)
 */
//--------------------------------------------------------------
static int CursorMove_PokeSelect(BI_PARAM_PTR bip, int init_flag)
{
  CURSOR_MOVE *move;
  u32 key;
  const BG_MAKE_DATA *bmd;
  int i, index;
  int plural = FALSE;
  u8 hit_range[BTL_CLIENT_MAX], connect_range[BTL_CLIENT_MAX];
  u8 move_data[CURSOR_POKE_SELECT_Y_MENU_NUM][CURSOR_POKE_SELECT_X_MENU_NUM];
  int y, x, x_menu_num, y_menu_num;
  int left, right, top, bottom, ex_x, ex_y;
  int first_top, first_bottom, first_left, first_right;
  int second_top, second_bottom, second_left, second_right;
  int l_bottom, r_bottom;
  BCURSOR_ANMTYPE ex_anm;
  CURSOR_SAVE *cursor_save;

  cursor_save = &bip->cursor_save;
  move = &bip->cursor_move;
  bmd = &BgMakeData[bip->makedata_no];

  MI_CpuCopy8(CursorMoveDataPokeSelect, move_data,
    CURSOR_POKE_SELECT_Y_MENU_NUM * CURSOR_POKE_SELECT_X_MENU_NUM);
  Sub_PokeSelectHitRange(bip, hit_range, TRUE);
  Sub_PokeSelectHitRange(bip, connect_range, FALSE);

  for(y = 0; y < 2;y ++){
    for(x = 0; x < 2; x++){
    #if 0
      //if(hit_range[move_data[y][x]] == FALSE){
    #else //死んでいる所でもカーソル移動出来るようにしないと、例えば、
        //CLIENT_TYPE_B,C,Dに単数選択出来る場合、Bが死亡していると
        //初期カーソル位置がDの場合、Cにカーソルを合わせる事が出来なくなる
      if(connect_range[move_data[y][x]] == FALSE){
    #endif
        //MoveDataで選択出来ない場所は0xffで埋める
        move_data[y][x] = 0xff;
      }
    }
  }

  switch(bip->pokesele_type){
  //単数選択
  default:
  case POKESELE_A_B_C_D:
  case POKESELE_A:
  case POKESELE_C:
  case POKESELE_B_C_D:
  case POKESELE_A_B_D:
  case POKESELE_A_C:
  case POKESELE_B_D:
    x_menu_num = CURSOR_POKE_SELECT_X_MENU_NUM;
    y_menu_num = CURSOR_POKE_SELECT_Y_MENU_NUM;
    break;

  //複数選択
  case POKESELE_BD:
  case POKESELE_BCD:
  case POKESELE_ABCD:
  case POKESELE_AC:
  case POKESELE_ABD:
    plural = TRUE;
    x_menu_num = 1;
    y_menu_num = 2;
    //上段座標
    if(connect_range[CLIENT_TYPE_D - CLIENT_TYPE_A] == TRUE
        && connect_range[CLIENT_TYPE_B - CLIENT_TYPE_A] == TRUE){
      first_left = bmd->tpd[POKESELE_INDEX_TARGET_D].rect.left+8;
      first_right = bmd->tpd[POKESELE_INDEX_TARGET_B].rect.right-8;
      first_top = bmd->tpd[POKESELE_INDEX_TARGET_D].rect.top+8;
      first_bottom = bmd->tpd[POKESELE_INDEX_TARGET_D].rect.bottom-8;
    }
    else if(connect_range[CLIENT_TYPE_D - CLIENT_TYPE_A] == TRUE
        && connect_range[CLIENT_TYPE_B - CLIENT_TYPE_A] == FALSE){
      first_left = bmd->tpd[POKESELE_INDEX_TARGET_D].rect.left+8;
      first_right = bmd->tpd[POKESELE_INDEX_TARGET_D].rect.right-8;
      first_top = bmd->tpd[POKESELE_INDEX_TARGET_D].rect.top+8;
      first_bottom = bmd->tpd[POKESELE_INDEX_TARGET_D].rect.bottom-8;
    }
    else if(connect_range[CLIENT_TYPE_D - CLIENT_TYPE_A] == FALSE
        && connect_range[CLIENT_TYPE_B - CLIENT_TYPE_A] == TRUE){
      first_left = bmd->tpd[POKESELE_INDEX_TARGET_B].rect.left+8;
      first_right = bmd->tpd[POKESELE_INDEX_TARGET_B].rect.right-8;
      first_top = bmd->tpd[POKESELE_INDEX_TARGET_B].rect.top+8;
      first_bottom = bmd->tpd[POKESELE_INDEX_TARGET_B].rect.bottom-8;
    }
    else{
      first_left = -1;
      first_right = -1;
      first_top = -1;
      first_bottom = -1;
    }
    //下段座標
    if(connect_range[CLIENT_TYPE_A - CLIENT_TYPE_A] == TRUE
        && connect_range[CLIENT_TYPE_C - CLIENT_TYPE_A] == TRUE){
      second_left = bmd->tpd[POKESELE_INDEX_TARGET_A].rect.left+8;
      second_right = bmd->tpd[POKESELE_INDEX_TARGET_C].rect.right-8;
      second_top = bmd->tpd[POKESELE_INDEX_TARGET_A].rect.top+8;
      second_bottom = bmd->tpd[POKESELE_INDEX_TARGET_A].rect.bottom-8;
    }
    else if(connect_range[CLIENT_TYPE_A - CLIENT_TYPE_A] == TRUE
        && connect_range[CLIENT_TYPE_C - CLIENT_TYPE_A] == FALSE){
      second_left = bmd->tpd[POKESELE_INDEX_TARGET_A].rect.left+8;
      second_right = bmd->tpd[POKESELE_INDEX_TARGET_A].rect.right-8;
      second_top = bmd->tpd[POKESELE_INDEX_TARGET_A].rect.top+8;
      second_bottom = bmd->tpd[POKESELE_INDEX_TARGET_A].rect.bottom-8;
    }
    else if(connect_range[CLIENT_TYPE_A - CLIENT_TYPE_A] == FALSE
        && connect_range[CLIENT_TYPE_C - CLIENT_TYPE_A] == TRUE){
      second_left = bmd->tpd[POKESELE_INDEX_TARGET_C].rect.left+8;
      second_right = bmd->tpd[POKESELE_INDEX_TARGET_C].rect.right-8;
      second_top = bmd->tpd[POKESELE_INDEX_TARGET_C].rect.top+8;
      second_bottom = bmd->tpd[POKESELE_INDEX_TARGET_C].rect.bottom-8;
    }
    else{
      second_left = -1;
      second_right = -1;
      second_top = -1;
      second_bottom = -1;
    }
    //矩形座標とEX座標を求める
    ex_x = -1;
    ex_y = -1;
    l_bottom = -1;
    r_bottom = -1;
    //左
    if(first_left == second_left){
      left = first_left;
    }
    else if(first_left != -1 && second_left == -1){
      left = first_left;
    }
    else if(first_left == -1 && second_left != -1){
      left = second_left;
    }
    else{
      left = first_left;
      ex_x = second_left;
      ex_anm = BCURSOR_ANMTYPE_LD;
    }
    //右
    if(first_right == second_right){
      right = first_right;
    }
    else if(first_right != -1 && second_right == -1){
      right = first_right;
    }
    else if(first_right == -1 && second_right != -1){
      right = second_right;
    }
    else{
      right = first_right;
      ex_x = second_right;
      ex_anm = BCURSOR_ANMTYPE_RD;
    }
    //上
    if(first_top != -1 && second_top == -1){
      top = first_top;
    }
    else if(first_top == -1 && second_top != -1){
      top = second_top;
    }
    else{
      top = first_top;
      ex_y = first_bottom;
    }
    //下
    if(first_bottom != -1 && second_bottom == -1){
      bottom = first_bottom;
    }
    else if(first_bottom == -1 && second_bottom != -1){
      bottom = second_bottom;
    }
    else{
      bottom = second_bottom;
      ex_y = second_bottom;
      if(ex_anm == BCURSOR_ANMTYPE_LD){
        r_bottom = first_bottom;
      }
      else{
        l_bottom = first_bottom;
      }
    }
    break;
  }

  if(init_flag == TRUE){
    if(plural == FALSE){
      //カーソル初期位置決定(※check 最終的には仕様書にある通り、技毎のデフォルト選択位置を
      //サーバーからもらうようにしないと駄目)
      if(cursor_save->poke_save_type == bip->pokesele_type){//bip->makedata_no){
        move->x_menu = cursor_save->poke_x;
        move->y_menu = cursor_save->poke_y;
      }
      else if(hit_range[CLIENT_TYPE_D - CLIENT_TYPE_A] == TRUE){
        move->x_menu = 0;
        move->y_menu = 0;
      }
      else if(hit_range[CLIENT_TYPE_B - CLIENT_TYPE_A] == TRUE){
        move->x_menu = 1;
        move->y_menu = 0;
      }
      else if(hit_range[CLIENT_TYPE_A - CLIENT_TYPE_A] == TRUE){
        move->x_menu = 0;
        move->y_menu = 1;
      }
      else{// if(hit_range[CLIENT_TYPE_C - CLIENT_TYPE_A] == TRUE){
        move->x_menu = 1;
        move->y_menu = 1;
      }
      index = CursorMoveDataPokeSelect[move->y_menu][move->x_menu];
      BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
        bmd->tpd[index].rect.right-8,
        bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    }
    else{
    #if 0
      if(cursor_save->poke_save_type == bip->pokesele_type){//bip->makedata_no){
        move->x_menu = cursor_save->poke_x;
        move->y_menu = cursor_save->poke_y;
        if(top == second_top && move->y_menu > 0){
          move->y_menu = 0;
        }
      }
      else{
        move->x_menu = 0;
        move->y_menu = 0;
      }
    #else
      //複数選択の時は1箇所の対象選択と「もどる」だけなので、カーソル記憶が意味ないので
      //0固定にする。むしろカーソル記憶が単数選択用になっているので今のまま記憶位置
      //を採用すると不具合が出る 2006.07.07(金)
      move->x_menu = 0;
      move->y_menu = 0;
    #endif

    #ifdef OSP_BINPUT_ON
      OS_TPrintf("move->x_menu = %d, move->y_menu = %d\n", move->x_menu, move->y_menu);
      OS_TPrintf("first_bottom = %d, second_bottom = %d, r_bottom = %d, l_bottom = %d\n",
        first_bottom, second_bottom, r_bottom, l_bottom);
      OS_TPrintf("ex_x = %d, \n", ex_x);
    #endif
      if((r_bottom == -1 && l_bottom == -1) || ex_x == -1){
        BCURSOR_PosSetON(bip->cursor, left, right, top, bottom);
      }
      else if(r_bottom != -1 && l_bottom == -1){
        BCURSOR_IndividualPosSetON(bip->cursor, left, top, right, top,
          left, r_bottom, right, bottom);
      }
      else{
        BCURSOR_IndividualPosSetON(bip->cursor, left, top, right, top,
          left, bottom, right, l_bottom);
      }
      if(ex_x != -1){
        BCURSOR_ExPosSetON(bip->cursor, ex_x, ex_y, ex_anm);
      }
    }
    return GFL_UI_TP_HIT_NONE;
  }

  if(plural == FALSE){
    key = CursorMove_KeyCheckMove(move, x_menu_num, y_menu_num, move_data[0]);
  }
  else{
    key = CursorMove_KeyCheckMove(move, x_menu_num, y_menu_num, NULL);
  }

  switch(key){
  case PAD_KEY_UP:
  case PAD_KEY_DOWN:
  case PAD_KEY_LEFT:
  case PAD_KEY_RIGHT:
    if(plural == FALSE){
      index = CursorMoveDataPokeSelect[move->y_menu][move->x_menu];
      BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
        bmd->tpd[index].rect.right-8,
        bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    }
    else{
      if(move->y_menu == 0){
        if((l_bottom == -1 && r_bottom == -1) || ex_x == -1){
          BCURSOR_PosSetON(bip->cursor, left, right, top, bottom);
        }
        else if(r_bottom != -1 && l_bottom == -1){
          BCURSOR_IndividualPosSetON(bip->cursor, left, top, right, top,
            left, r_bottom, right, bottom);
        }
        else{
          BCURSOR_IndividualPosSetON(bip->cursor, left, top, right, top,
            left, bottom, right, l_bottom);
        }
        if(ex_x != -1){
          BCURSOR_ExPosSetON(bip->cursor, ex_x, ex_y, ex_anm);
        }
        else{
          BCURSOR_ExOFF(bip->cursor);
        }
      }
      else{
        BCURSOR_PosSetON(bip->cursor,
          bmd->tpd[POKESELE_INDEX_TARGET_CANCEL].rect.left+8,
          bmd->tpd[POKESELE_INDEX_TARGET_CANCEL].rect.right-8,
          bmd->tpd[POKESELE_INDEX_TARGET_CANCEL].rect.top+8,
          bmd->tpd[POKESELE_INDEX_TARGET_CANCEL].rect.bottom-8);
        BCURSOR_ExOFF(bip->cursor);
      }
    }
    break;
  case PAD_BUTTON_DECIDE:
    if(plural == TRUE){
      //複数選択の場合
      if(move->y_menu > 0){
        return POKESELE_INDEX_TARGET_CANCEL;
      }
      else{
        for(i = 0; i < BTL_CLIENT_MAX; i++){
          if(hit_range[i] == TRUE){
            //複数選択なので、有効なやつなら返すのはどれでもいい
            return POKESELE_INDEX_TARGET_A + i;
          }
        }
      }
    }
    else{
      //単数選択
      index = CursorMoveDataPokeSelect[move->y_menu][move->x_menu];
      if(index == POKESELE_INDEX_TARGET_CANCEL
          || hit_range[index - POKESELE_INDEX_TARGET_A] == TRUE){
        return index; //死亡確認して大丈夫ならそのまま返す
      }
    }
    break;
  case PAD_BUTTON_CANCEL:
    return POKESELE_INDEX_TARGET_CANCEL;
  }

  return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief   カーソル位置保存：ポケモン選択
 * @param   bip       BIシステムワークへのポインタ
 * @param   index     選択した項目
 */
//--------------------------------------------------------------
static void CursorSave_PokeSelect(BI_PARAM_PTR bip, int index)
{
  CURSOR_SAVE *cursor_save;
  int x, y;

  if(index == POKESELE_INDEX_TARGET_CANCEL){
    return;
  }

  cursor_save = &bip->cursor_save;
  cursor_save->poke_save_type = bip->pokesele_type; //bip->makedata_no;
  for(y = 0; y < CURSOR_POKE_SELECT_Y_MENU_NUM; y++){
    for(x = 0; x < CURSOR_POKE_SELECT_X_MENU_NUM; x++){
      if(index == CursorMoveDataPokeSelect[y][x]){
        cursor_save->poke_x = x;
        cursor_save->poke_y = y;
      #ifdef OSP_BINPUT_ON
        OS_TPrintf("save x = %d, y = %d\n", x, y);
      #endif
        return;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief   カーソル移動：2択
 *
 * @param   bip       BIシステムワークへのポインタ
 * @param   init_flag   TRUE:初期化処理
 *
 * @retval  結果(当たりなしの場合はGFL_UI_TP_HIT_NONE)
 */
//--------------------------------------------------------------
static int CursorMove_YesNo(BI_PARAM_PTR bip, int init_flag)
{
  CURSOR_MOVE *move;
  u32 key;
  const BG_MAKE_DATA *bmd;
  int index, i;

  move = &bip->cursor_move;
  bmd = &BgMakeData[bip->makedata_no];

  if(init_flag == TRUE){
    index = CursorMoveDataYesNoSelect[move->y_menu][move->x_menu];
    BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
      bmd->tpd[index].rect.right-8,
      bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    return GFL_UI_TP_HIT_NONE;
  }

  key = CursorMove_KeyCheckMove(move,
    CURSOR_YESNO_SELECT_X_MENU_NUM, CURSOR_YESNO_SELECT_Y_MENU_NUM,
    CursorMoveDataYesNoSelect[0]);
  switch(key){
  case PAD_KEY_UP:
  case PAD_KEY_DOWN:
  case PAD_KEY_LEFT:
  case PAD_KEY_RIGHT:
    index = CursorMoveDataYesNoSelect[move->y_menu][move->x_menu];
    BCURSOR_PosSetON(bip->cursor, bmd->tpd[index].rect.left+8,
      bmd->tpd[index].rect.right-8,
      bmd->tpd[index].rect.top+8, bmd->tpd[index].rect.bottom-8);
    break;
  case PAD_BUTTON_DECIDE:
    index = CursorMoveDataYesNoSelect[move->y_menu][move->x_menu];
    return index;
  case PAD_BUTTON_CANCEL:
    return YESNO_INDEX_CANCEL;
  }

  return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief   キー入力によるカーソル移動処理
 *
 * @param   move      カーソル移動用ワークへのポインタ
 * @param   x_menu_num    水平方向のメニュー数
 * @param   y_menu_num    垂直方向のメニュー数
 * @param   move_data   移動データへのポインタ(移動先が同じ場所なら移動しない判定に必要)
 *                          その判定が必要でないならNULLでOK
 *
 * @retval  押されたキー
 */
//--------------------------------------------------------------
static u32 CursorMove_KeyCheckMove(CURSOR_MOVE *move, int x_menu_num, int y_menu_num,
  const u8 *move_data)
{
  int old_y_menu, old_x_menu;
  u32 ret_key;
  int trg = GFL_UI_KEY_GetTrg();

  if(move->x_menu >= x_menu_num){
    move->x_menu = x_menu_num - 1;
  }
  if(move->y_menu >= y_menu_num){
    move->y_menu = y_menu_num - 1;
  }
  old_x_menu = move->x_menu;
  old_y_menu = move->y_menu;

  if(trg & PAD_KEY_UP){
    move->y_menu--;
    if(move->y_menu < 0){
      move->y_menu = 0;//y_menu_num - 1;
    }
    if(move_data != NULL){
      while(move_data[x_menu_num * move->y_menu + move->x_menu] == 0xff){
        move->y_menu--;
        if(move->y_menu < 0){
          move->y_menu = old_y_menu;
          break;
        }
      }
    }
    ret_key = PAD_KEY_UP;
  }
  else if(trg & PAD_KEY_DOWN){
    move->y_menu++;
    if(move->y_menu >= y_menu_num){
      move->y_menu = y_menu_num - 1;//0;
    }
    if(move_data != NULL){
      while(move_data[x_menu_num * move->y_menu + move->x_menu] == 0xff){
        move->y_menu++;
        if(move->y_menu >= y_menu_num){
          move->y_menu = old_y_menu;
          break;
        }
      }
    }
    ret_key = PAD_KEY_DOWN;
  }
  else if(trg & PAD_KEY_LEFT){
    move->x_menu--;
    if(move->x_menu < 0){
      move->x_menu = 0;//x_menu_num - 1;
    }
    if(move_data != NULL){
      while(move_data[x_menu_num * move->y_menu + move->x_menu] == 0xff){
        move->x_menu--;
        if(move->x_menu < 0){
          move->x_menu = old_x_menu;
          break;
        }
      }
    }
    ret_key = PAD_KEY_LEFT;
  }
  else if(trg & PAD_KEY_RIGHT){
    move->x_menu++;
    if(move->x_menu >= x_menu_num){
      move->x_menu = x_menu_num-1;//0;
    }
    if(move_data != NULL){
      while(move_data[x_menu_num * move->y_menu + move->x_menu] == 0xff){
        move->x_menu++;
        if(move->x_menu >= x_menu_num){
          move->x_menu = old_x_menu;
          break;
        }
      }
    }
    ret_key = PAD_KEY_RIGHT;
  }
  else if(trg & PAD_BUTTON_DECIDE){
    ret_key = PAD_BUTTON_DECIDE;
  }
  else if(trg & PAD_BUTTON_CANCEL){
    ret_key = PAD_BUTTON_CANCEL;
  }
  else{
    return 0;
  }

  if(move_data != NULL){
    int old_index, new_index;

    old_index = move_data[x_menu_num * old_y_menu + old_x_menu];
    new_index = move_data[x_menu_num * move->y_menu + move->x_menu];

    //カーソル移動後位置確認
    if(old_index == new_index){
      //移動後のindexが変わらないのであればカーソル座標の移動はしない
      move->x_menu = old_x_menu;
      move->y_menu = old_y_menu;
    }
  }

  if(move->x_menu != old_x_menu || move->y_menu != old_y_menu){
//    Snd_SePlay(BCURSOR_MOVE_SE);
  }
  else{
    if(ret_key & (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT)){
      return 0;
    }
  }

  return ret_key;
}

//--------------------------------------------------------------
/**
 * @brief   決定ボタンを押したのがキーだったか、タッチだったかを取得
 * @param   bip   BIシステムワークへのポインタ
 * @retval  TRUE:キーで決定してた。　FALSE:タッチパネルで決定してた
 */
//--------------------------------------------------------------
int BINPUT_CursorDecendGet(BI_PARAM_PTR bip)
{
  return bip->decend_key;
}

//--------------------------------------------------------------
/**
 * @brief   決定ボタンを押したのがキーだったか、タッチだったかをセット
 * @param   bip   BIシステムワークへのポインタ
 * @param   TRUE:キーで決定。　FALSE:タッチパネルで決定
 */
//--------------------------------------------------------------
void BINPUT_CursorDecendSet(BI_PARAM_PTR bip, int decend_key)
{
  bip->decend_key = decend_key;
}


//==============================================================================
//
//  捕獲デモ
//
//==============================================================================
///捕獲デモシーケンステーブル
static int (* const CaptureDemoSeqTbl[])(BI_PARAM_PTR bip) = {
  CaptureDemoSeq_CommandFight,
  CaptureDemoSeq_Waza,
//  CaptureDemoSeq_CommandBag,
};

//--------------------------------------------------------------
/**
 * @brief   捕獲デモ呼び出しメイン処理
 *
 * @param   bip   BIシステムワークへのポインタ
 */
//--------------------------------------------------------------
static int CaptureDemo_Main(BI_PARAM_PTR bip)
{
  int tp_ret;

  tp_ret = GFL_UI_TP_HIT_NONE;

#if 0
  if(BattleWorkDemoSeqNoGet(bip->bw) == 0){
    GF_ASSERT(bip->demo.main_seq < NELEMS(CaptureDemoSeqTbl));
    tp_ret = CaptureDemoSeqTbl[bip->demo.main_seq](bip);
  }
  else{
    tp_ret = CaptureDemoSeq_CommandBag(bip);
  }

  if(tp_ret != GFL_UI_TP_HIT_NONE){
    bip->demo.sub_seq = 0;
    bip->demo.sub_wait = 0;
    bip->demo.main_seq++;
  }

#endif
  return tp_ret;
}

//--------------------------------------------------------------
/**
 * @brief   捕獲デモシーケンス：コマンド選択＞たたかう
 * @param   bip   BIシステムワークへのポインタ
 * @retval  押したボタン
 */
//--------------------------------------------------------------
static int CaptureDemoSeq_CommandFight(BI_PARAM_PTR bip)
{
  switch(bip->demo.sub_seq){
  case 0:
    FINGER_PosSetON_Surface(bip->demo.finger, FA_POS_FIGHT_X, FA_POS_FIGHT_Y - 24,
      BATTLE_SUB_ACTOR_DISTANCE);
    FINGER_TouchReq(bip->demo.finger, FINGER_TOUCH_WAIT);
    bip->demo.sub_seq++;
    break;
  default:
    if(FINGER_TouchAnimeCheck(bip->demo.finger) == TRUE){
      bip->demo.sub_seq++;
      return COMMSELE_INDEX_FIGHT;
    }
    break;
  }

  return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief   捕獲デモシーケンス：技１
 * @param   bip   BIシステムワークへのポインタ
 * @retval  押したボタン
 */
//--------------------------------------------------------------
static int CaptureDemoSeq_Waza(BI_PARAM_PTR bip)
{
  switch(bip->demo.sub_seq){
  case 0:
    FINGER_PosSetON_Surface(bip->demo.finger, FA_POS_WAZA_1_X, FA_POS_WAZA_1_Y - 24,
      BATTLE_SUB_ACTOR_DISTANCE);
    FINGER_TouchReq(bip->demo.finger, FINGER_TOUCH_WAIT);
    bip->demo.sub_seq++;
    break;
  default:
    if(FINGER_TouchAnimeCheck(bip->demo.finger) == TRUE){
      bip->demo.sub_seq++;
      return WAZASELE_INDEX_SKILL_1;
    }
    break;
  }

  return GFL_UI_TP_HIT_NONE;
}

//--------------------------------------------------------------
/**
 * @brief   捕獲デモシーケンス：コマンド選択＞バッグ
 * @param   bip   BIシステムワークへのポインタ
 * @retval  押したボタン
 */
//--------------------------------------------------------------
static int CaptureDemoSeq_CommandBag(BI_PARAM_PTR bip)
{
  switch(bip->demo.sub_seq){
  case 0:
    bip->demo.sub_wait++;
    if(bip->demo.sub_wait > 60){
      bip->demo.sub_wait = 0;
      bip->demo.sub_seq++;
    }
    break;
  case 1:
    FINGER_PosSetON_Surface(bip->demo.finger, FA_POS_ITEM_X, FA_POS_ITEM_Y - 24,
      BATTLE_SUB_ACTOR_DISTANCE);
    FINGER_TouchReq(bip->demo.finger, FINGER_TOUCH_WAIT);
    bip->demo.sub_seq++;
    break;
  default:
    if(FINGER_TouchAnimeCheck(bip->demo.finger) == TRUE){
      bip->demo.sub_seq++;
      return COMMSELE_INDEX_ITEM;
    }
    break;
  }

  return GFL_UI_TP_HIT_NONE;
}



//==============================================================================
//
//  録画再生停止画面の特別処理
//
//==============================================================================

///システムウィンドウ:キャラクタ展開位置
///(空いているキャラ領域は無い為、録画再生停止画面では表示されないボタンの上に描いていく)
#define SYSWINDOW_CGX_NO    (0x20)
///システムウィンドウ:パレット展開位置
///(空いているパレット領域は無い為、録画再生停止画面では表示されないボタンのパレットを潰す)
#define SYSWINDOW_PAL_NO    (1)

///システムウィンドウを描画するBGフレーム
#define SYSWINDOW_FRAME     (BI_FRAME_BF)

///システムウィンドウの矩形サイズ
enum{
  SYSWINDOW_PX = 1, ///<ウィンドウ左端X位置
  SYSWINDOW_PY = 6, ///<ウィンドウ上端Y位置
  SYSWINDOW_SX = 30,  ///<ウィンドウX幅
  SYSWINDOW_SY = 6, ///<ウィンドウY幅
};

//--------------------------------------------------------------
/**
 * @brief   録画再生停止画面にシステムメッセージを表示させる
 *
 * @param   bip   BIシステムワークへのポインタ
 *
 *  録画再生停止画面専用です
 */
//--------------------------------------------------------------
void BINPUT_SystemMessagePrint(BI_PARAM_PTR bip, int sysmsg_type)
{
#if 0
  GF_BGL_INI *bgl = BattleWorkGF_BGL_INIGet(bip->bw);
  PALETTE_FADE_PTR pfd = BattleWorkPfdGet(bip->bw);

  GF_ASSERT(bip->makedata_no == BINPUT_TYPE_PLAYBACK_STOP);

  //システムウィンドウ　キャラ＆パレット
  MenuWinGraphicSet(bgl, SYSWINDOW_FRAME, SYSWINDOW_CGX_NO, SYSWINDOW_PAL_NO,
    MENU_TYPE_SYSTEM, HEAPID_BATTLE);
  PaletteWorkSet_VramCopy(pfd, FADE_SUB_BG, SYSWINDOW_PAL_NO * 16, 0x20);

  //BMPを使えるようなキャラ領域が無い為、スクリーンにウィンドウだけ描き、
  //フォントはOAMで作成する。

  //スクリーン描画(BMPが使えない為、スクリーン自力描画)
  {
    int cgx = SYSWINDOW_CGX_NO, pal = SYSWINDOW_PAL_NO;
    int px, py, sx, sy;

    px = SYSWINDOW_PX;
    py = SYSWINDOW_PY;
    sx = SYSWINDOW_SX;
    sy = SYSWINDOW_SY;

    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx,   px,  py, 1,  1, pal ); //左上
    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx+1, px+1,    py, sx-2, 1, pal ); //上真ん中
    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx+2, px+sx-1, py, 1,  1, pal ); //右上

    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx+3, px,  py+1, 1, sy-2, pal ); //左端
    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx+5, px+sx-1, py+1, 1, sy-2, pal ); //右端

    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx+6, px,  py+sy-1, 1,  1, pal );  //左下
    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx+7, px+1,    py+sy-1, sx-2, 1, pal );  //下真ん中
    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx+8, px+sx-1, py+sy-1, 1,  1, pal );  //右下

    GF_BGL_ScrFill( bgl, SYSWINDOW_FRAME, cgx+4, px+1, py+1, sx-2, sy-2,pal);//ウィンドウ内空白

    GF_BGL_LoadScreenV_Req(bgl, SYSWINDOW_FRAME);
  }

  //文字列描画(BG領域が無い為フォントOAM)
  {
    STRBUF *msg_src;
    MSGDATA_MANAGER *msg_man = BattleWorkFightMsgGet(bip->bw);
    int i;
    const u16 msg_id[][2] = { //BI_BI_SYSMSG_???と並びを同じにしておくこと！
      {PlayBack_Msg2, PlayBack_Msg3},   //1行目、2行目
      {PlayBack_Msg4, PlayBack_Msg5},
      {PlayBack_Msg6, PlayBack_Msg7},
    };

    for(i = 0; i < 2; i++){
      if(bip->font_actor[FA_NO_PLAYBACK_SYSTEM_MSG_0 + i].fontoam != NULL){
        GF_ASSERT(0);
        return; //既に別のシステムメッセージが出ている場合は何もしない
      }

      msg_src = MSGMAN_AllocString(msg_man, msg_id[sysmsg_type][i]);

      Sub_FontOamCreate(bip, &bip->font_actor[FA_NO_PLAYBACK_SYSTEM_MSG_0 + i],
        msg_src, FONT_SYSTEM,
        MSGCOLOR_PP_BLACK, PALOFS_PP,
        PLTTID_INPUT_PPMSG_1, SYSWINDOW_PX * 8 + 8, SYSWINDOW_PY * 8 + 16 + 16*i,
        FONTOAM_LEFT, NULL);

      STRBUF_Delete(msg_src);
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   cancel_escapeフラグを取得する
 * @param   bip   BIシステムワークへのポインタ
 * @param   cancel_escape
 */
//--------------------------------------------------------------
u8 BINPUT_CancelEscapeFlagGet(BI_PARAM_PTR bip)
{
  return bip->cancel_escape;
}
