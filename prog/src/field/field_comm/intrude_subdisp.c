//==============================================================================
/**
 * @file    intrude_subdisp.c
 * @brief   侵入下画面
 * @author  matsuda
 * @date    2009.10.16(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "infowin/infowin.h"
#include "app_menu_common.naix"
#include "field/fieldmap.h"
#include "field/intrude_subdisp.h"
#include "field/intrude_common.h"
#include "palace.naix"
#include "fieldmap/zone_id.h"
#include "intrude_types.h"
#include "infowin\infowin.h"
#include "intrude_main.h"
#include "intrude_work.h"
#include "intrude_mission.h"
#include "field/zonedata.h"
#include "intrude_comm_command.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_invasion.h"
#include "system/bmp_oam.h"
#include "intrude_mission_field.h"
#include "sound/pm_sndsys.h"
#include "field/event_intrude_subscreen.h"
#include "field/scrcmd_intrude.h"
#include "intrude_field.h"
#include "system/palanm.h"
#include "intrude_snd_def.h"
#include "field/game_beacon_search.h"
#include "net/network_define.h"
#include "net/net_whpipe.h"
#include "gamesystem/game_beacon_accessor.h"
#include "field/field_palace_sys.h"
#include "net/wih_dwc.h"  //WIH_DWC_GetAllBeaconTypeBit


//==============================================================================
//  定数定義
//==============================================================================
///サブ画面BGフレーム定義
enum{
  INTRUDE_FRAME_S_NULL_0 = GFL_BG_FRAME0_S,          ///<未使用
  INTRUDE_FRAME_S_NULL_1 = GFL_BG_FRAME1_S,          ///<未使用
  INTRUDE_FRAME_S_BAR = GFL_BG_FRAME2_S,             ///<プレート面
  INTRUDE_FRAME_S_BACKGROUND = GFL_BG_FRAME3_S,      ///<背景面
};

///インフォメーションメッセージ更新ウェイト
#define INFOMSG_UPDATE_WAIT   (90)

///近くにいるビーコンの人物を表示する最大数
#define INTSUB_BCON_PLAYER_PRINT_MAX    (INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX)

///フォントBGパレット展開位置
#define _FONT_BG_PALNO      (14)

///レンダラー設定
enum{
  INTSUB_CLACT_REND_SUB,
  INTSUB_CLACT_REND_MAX,
};
static const GFL_REND_SURFACE_INIT INTSUB_CLACT_REND[INTSUB_CLACT_REND_MAX] = {
  {
    0, 0,
    256, 192,
    CLSYS_DRAW_SUB,
    CLSYS_REND_CULLING_TYPE_NONE,
  },
};

///アクターIndex
enum{
  INTSUB_ACTOR_TOUCH_TOWN_0,
  INTSUB_ACTOR_TOUCH_TOWN_1,
  INTSUB_ACTOR_TOUCH_TOWN_2,
  INTSUB_ACTOR_TOUCH_TOWN_3,
  INTSUB_ACTOR_TOUCH_TOWN_4,
  INTSUB_ACTOR_TOUCH_TOWN_5,
  INTSUB_ACTOR_TOUCH_TOWN_6,
  INTSUB_ACTOR_TOUCH_TOWN_7,
  INTSUB_ACTOR_TOUCH_TOWN_MAX = INTSUB_ACTOR_TOUCH_TOWN_7,

  INTSUB_ACTOR_TOUCH_PALACE,
  
  INTSUB_ACTOR_AREA_0,
  INTSUB_ACTOR_AREA_1,
  INTSUB_ACTOR_AREA_2,
  INTSUB_ACTOR_AREA_MAX = INTSUB_ACTOR_AREA_2,

  INTSUB_ACTOR_CUR_S_0,
  INTSUB_ACTOR_CUR_S_1,
  INTSUB_ACTOR_CUR_S_2,
  INTSUB_ACTOR_CUR_S_MAX = INTSUB_ACTOR_CUR_S_2,

  INTSUB_ACTOR_CUR_L,       ///<自分の居場所を指す
  INTSUB_ACTOR_ENTRY,       ///<「参加」ボタン

  INTSUB_ACTOR_WARP_NG_0,
  INTSUB_ACTOR_WARP_NG_1,
  INTSUB_ACTOR_WARP_NG_2,
  INTSUB_ACTOR_WARP_NG_MAX = INTSUB_ACTOR_WARP_NG_2,
  
  INTSUB_ACTOR_LV_NUM_KETA_0,

  INTSUB_ACTOR_MAX,
};

///BMPOAMで使用するアクター数
#define INTSUB_ACTOR_BMPOAM_NUM   (16 + 4)

///OBJパレットINDEX
enum{
  INTSUB_ACTOR_PAL_BASE_START = 1,    ///<プレイヤー毎に変わるパレット開始位置
  
  INTSUB_ACTOR_PAL_TOUCH_NORMAL = 6,  ///<タッチ出来る街：ノーマル
  INTSUB_ACTOR_PAL_TOUCH_DECIDE = 7,  ///<タッチ出来る街：決定時
  INTSUB_ACTOR_PAL_TOUCH_MINE = 8,    ///<タッチ出来る街：自分がいる場所
  
  INTSUB_ACTOR_PAL_MAX = 9,
  
  INTSUB_ACTOR_PAL_MISSION_TARGET_PLAYER = 9, //ミッション対象のプレイヤーパレットアニメ
  
  INTSUB_ACTOR_PAL_FONT = 0xd,
};

///palace_obj.nceのアニメーションシーケンス
enum{
  PALACE_ACT_ANMSEQ_AREA,
  PALACE_ACT_ANMSEQ_CUR_S,
  PALACE_ACT_ANMSEQ_CUR_L_MALE,
  PALACE_ACT_ANMSEQ_CUR_L_FEMALE,
  PALACE_ACT_ANMSEQ_SENKYO_EFF,
  PALACE_ACT_ANMSEQ_LV_NUM,
  PALACE_ACT_ANMSEQ_POINT_NUM,
  PALACE_ACT_ANMSEQ_TOUCH_TOWN,
  PALACE_ACT_ANMSEQ_TOUCH_PALACE,
  PALACE_ACT_ANMSEQ_BTN,
  PALACE_ACT_ANMSEQ_WARP_OK,
  PALACE_ACT_ANMSEQ_WARP_NG,
  PALACE_ACT_ANMSEQ_TOUCH_TOWN_EFF,
  PALACE_ACT_ANMSEQ_TOUCH_PALACE_EFF,
};

///アクターソフトプライオリティ
enum{
  SOFTPRI_TOUCH_TOWN = 90,
  SOFTPRI_TOUCH_TOWN_EFF = 100,
  SOFTPRI_AREA = 50,
  SOFTPRI_CUR_S = 20,
  SOFTPRI_CUR_L = 19,
  SOFTPRI_WARP_NG = 30,
  SOFTPRI_LV_NUM = 10,
  SOFTPRI_ENTRY_BUTTON = 5,
  SOFTPRI_ENTRY_BUTTON_MSG = 4,
};
///アクター共通BGプライオリティ
#define BGPRI_ACTOR_COMMON      (2)

///BGパレットINDEX
enum{
  INTSUB_BG_PAL_INFOMSG_BACK_NORMAL = 0, ///<infoメッセージの背景色(ノーマル)
  INTSUB_BG_PAL_INFOMSG_BACK_ACTIVE = 9, ///<infoメッセージの背景色(アクティブ時)
  
  INTSUB_BG_PAL_BASE_START_TOWN = 1,   ///<プレイヤー毎に変わるパレット開始位置(街BG)
  INTSUB_BG_PAL_BASE_START_BACK = 5,   ///<プレイヤー毎に変わるパレット開始位置(背景)
  
  INTSUB_BG_PAL_MAX = 10,
};

///街アイコン更新リクエスト
enum{
  TOWN_UPDATE_REQ_NONE,           ///<リクエスト無し
  TOWN_UPDATE_REQ_NOT_EFFECT,     ///<街アイコンの更新はするが占拠エフェクトは無し
  TOWN_UPDATE_REQ_UPDATE,         ///<街アイコンの更新＋変化があれば占拠エフェクト
};

enum{
  _TOWN_NO_FREE = 0xfe,       ///<街でもパレスでもない
  _TOWN_NO_PALACE = 0xff,     ///<パレス
};

enum{
  NO_TOWN_CURSOR_POS_SPACE_X = 24,
  NO_TOWN_CURSOR_POS_X = 128 - NO_TOWN_CURSOR_POS_SPACE_X * 2,
  NO_TOWN_CURSOR_POS_Y = 0xe*8,
};

enum{
  TOUCH_RANGE_PLAYER_ICON_X = 12,    ///<相手プレイヤーアイコンのタッチ範囲X(半径)
  TOUCH_RANGE_PLAYER_ICON_Y = 12,    ///<相手プレイヤーアイコンのタッチ範囲Y(半径)
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
///侵入エリアによってカラーを変更するBGスクリーンの位置とサイズ
enum{
  _AREA_SCREEN_COLOR_START_X = 0,
  _AREA_SCREEN_COLOR_START_Y = 2,
  _AREA_SCREEN_COLOR_SIZE_X = 32,
  _AREA_SCREEN_COLOR_SIZE_Y = 16,
};

///インフォメーションメッセージのアクティブ化によってカラーを変更するBGスクリーンの位置とサイズ
enum{
  _INFOMSG_SCREEN_COLOR_START_X = 0,
  _INFOMSG_SCREEN_COLOR_START_Y = 0x12,
  _INFOMSG_SCREEN_COLOR_SIZE_X = 32,
  _INFOMSG_SCREEN_COLOR_SIZE_Y = 4,
};

enum{
  INFOMSG_BMP_SIZE_X = 30,  //キャラ単位
  INFOMSG_BMP_SIZE_Y = 4,   //キャラ単位
};

enum{
  ENTRYMSG_BMP_SIZE_X = 6,  //キャラ単位
  ENTRYMSG_BMP_SIZE_Y = 2,   //キャラ単位
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
///街アイコンのタッチ判定の矩形ハーフサイズ
#define TOWN_ICON_HITRANGE_HALF   (16)

enum{
  BG_BAR_TYPE_SINGLE,   ///<一人用のBGスクリーン(palace_bar.nsc)
  BG_BAR_TYPE_COMM,     ///<通信用のBGスクリーン(palace_bar2.nsc)
};

///タイトルメッセージ表示タイプ
enum{
  _TITLE_PRINT_NULL,              ///<何も表示していない
  _TITLE_PRINT_PALACE_GO,         ///<タッチでパレスへいけます
  _TITLE_PRINT_MY_PALACE,         ///<自分のパレスにいます
  _TITLE_PRINT_OTHER_PALACE0,     ///<通信相手のパレスにいます
  _TITLE_PRINT_OTHER_PALACE1,     ///<通信相手のパレスにいます
  _TITLE_PRINT_OTHER_PALACE2,     ///<通信相手のパレスにいます
  _TITLE_PRINT_OTHER_PALACE3,     ///<通信相手のパレスにいます
  _TITLE_PRINT_CLEAR,             ///<クリア
};

///参加ボタンの表示タイプ
enum{
  ENTRY_BUTTON_MSG_PATERN_ENTRY,    ///<「さんか」
  ENTRY_BUTTON_MSG_PATERN_BACK,     ///<「もどる」
  
  ENTRY_BUTTON_MSG_PATERN_MAX,
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
///通信相手が不明ゾーンにいる場合の表示座標X
#define NOTHING_ZONE_SUB_POS_X            (240)
///通信相手が不明ゾーンにいる場合の表示座標Y
#define NOTHING_ZONE_SUB_POS_Y            (80)
///通信相手が不明ゾーンにいる場合の表示座標間隔Y
#define NOTHING_ZONE_SUB_POS_Y_SPACE      (24)

///通信相手のアイコンアニメ速度：通常
#define CURSOR_S_ANM_SPEED_NORMAL         (FX32_ONE)
///通信相手のアイコンアニメ速度：戦闘中
#define CURSOR_S_ANM_SPEED_BATTLE         (FX32_ONE*2)

//--------------------------------------------------------------
//  イベントNo
//--------------------------------------------------------------
enum{
  _EVENT_REQ_NO_NULL,             ///<イベントリクエスト無し
  _EVENT_REQ_NO_TOWN_WARP,        ///<街へワープ
  _EVENT_REQ_NO_PLAYER_WARP,      ///<プレイヤーのいる所へワープ
};

//--------------------------------------------------------------
//  タッチエフェクト
//--------------------------------------------------------------
///決定時のパレットフラッシュウェイト
#define TOWN_DECIDE_PAL_FLASH_WAIT    (2)
///決定時のパレットフラッシュタイマーMAX値
#define TOWN_DECIDE_PAL_FLASH_TIMER_MAX     (TOWN_DECIDE_PAL_FLASH_WAIT * 5)

///プレイヤーがいる街のフェード速度(下位8ビット小数)
#define PLAYER_TOWN_FADE_EVY_ADD      (0x100)

///プレイヤーが居る街のパレットアニメ：開始カラー位置
#define PLAYER_PALANM_START_COLOR     (1)
///プレイヤーが居る街のパレットアニメ：終了カラー位置
#define PLAYER_PALANM_END_COLOR       (4)
///プレイヤーが居る街のパレットアニメ：カラー数
#define PLAYER_PALANM_COLOR_NUM       (PLAYER_PALANM_END_COLOR - PLAYER_PALANM_START_COLOR + 1)
///プレイヤーが居る街のパレットアニメ：バッファサイズ
#define PLAYER_PALANM_BUFFER_SIZE     (PLAYER_PALANM_COLOR_NUM * sizeof(u16))

//--------------------------------------------------------------
//  [TIME]スクリーン
//--------------------------------------------------------------
///[TIME]が書かれているスクリーン位置
#define BG_TIME_SCRN_POS_X      (0x14)
#define BG_TIME_SCRN_POS_Y      (0x16)
#define BG_TIME_SCRN_SIZE_X     (12)
#define BG_TIME_SCRN_SIZE_Y     (2)
///[TIME]が書かれているスクリーンをクリアする時のスクリーンコード
#define BG_TIME_SCRN_CLEAR_CODE   (0x3001)

#define NUM_TRANS_BUFFER_SIZE   (8*2 * 32)  //数字3桁分(横8キャラ縦2キャラ)

//--------------------------------------------------------------
//  ミッションフォーカス
//--------------------------------------------------------------
///ミッションターゲットになっていてフォーカスされるプレイヤーアイコンのEVY値
#define MISSION_FOCUS_PLAYER_EVY      (8)
///ミッションターゲットになっていてフォーカスされるプレイヤーアイコンのカラー
#define MISSION_FOCUS_PLAYER_COLOR    (0x7fff)
///ミッションターゲットになっていてフォーカスされるプレイヤーアイコンのアニメ速度
#define MISSION_FOCUS_PLAYER_ANMWAIT  (15)


//==============================================================================
//  構造体定義
//==============================================================================
///intcommから取得出来るパラメータ類(非通信の時はソロ用の値がセットされる)
typedef struct{
  u8 now_palace_area;       ///<現在自分がいるパレスエリア
  u8 recv_profile;          ///<受信したプロフィール(bit管理)
  u8 recv_num;              ///<受信したプロフィール人数
  u8 palace_in;             ///<TRUE:パレス島におとずれた
  MISSION_STATUS m_status;  ///<ミッション状況
  s32 m_timer;              ///<ミッションタイマー
  const MISSION_DATA *p_md; ///<受信しているミッションデータへのポインタ
  u8 target_palace_area;    ///<ミッションターゲットがいるパレスエリア
  u8 target_mine;           ///<自分自身がミッションのターゲットになっている
  u8 my_area_intrude;       ///<TRUE:誰かが自分と同じパレスエリアにいる
  u8 padding;
}INTRUDE_COMM_PARAM;

///処理の高速化用に記憶するプレイヤー毎のパラメータ
typedef struct{
  const PALACE_ZONE_SETTING *zonesetting;
  u16 zone_id;
  u8 padding[2];
}SUBDISP_PLAYER_PARAM;

///侵入下画面制御ワーク
typedef struct _INTRUDE_SUBDISP{
  GAMESYS_WORK *gsys;
  
  u32 index_cgr;
  u32 index_pltt;
  u32 index_pltt_font;
  u32 index_cell;
  u32 index_cgr_num;
  u32 index_cell_num;

  GFL_FONT *font_handle;
  GFL_TCBLSYS *tcbl_sys;
  WORDSET *wordset;
  PRINT_QUE *print_que;
	GFL_MSGDATA *msgdata;
	GFL_MSGDATA *msgdata_mission;
	GFL_TCB *vintr_tcb;               ///<VBlankTCBへのポインタ
	
	STRBUF *strbuf_temp;
	STRBUF *strbuf_info;
	STRBUF *strbuf_title;
	
	PRINT_UTIL printutil_title;  ///<タイトルメッセージ
	PRINT_UTIL printutil_info;   ///<インフォメーションメッセージ
	
	GFL_CLUNIT *clunit;
	GFL_CLSYS_REND *clrend;
  GFL_CLWK *act[INTSUB_ACTOR_MAX];

  BMPOAM_SYS_PTR bmpoam_sys;
  GFL_BMP_DATA *entrymsg_bmp[ENTRY_BUTTON_MSG_PATERN_MAX];
  GFL_BMP_DATA *backmsg_bmp;
  BMPOAM_ACT_PTR entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_MAX];

  void *numchar_buffer;
  NNSG2dCharacterData *numchara_nnsg2d;

  u8 *num_trans_buffer; //
  u32 num_vram_address; //numchara_nnsg2dが転送されているVramアドレス
  
  INTRUDE_COMM_PARAM comm;  ///<intcommから取得出来るパラメータ類(非通信の時はソロ用の値)
  SUBDISP_PLAYER_PARAM player_param[FIELD_COMM_MEMBER_MAX];
  
  u8 town_update_req;     ///<街アイコン更新リクエスト(TOWN_UPDATE_REQ_???)
  u8 now_bg_pal;          ///<現在のBGのパレット番号
  u8 wfbc_go;             ///<TRUE:WFBCへのワープを押した
  u8 wfbc_seq;
  
  s16 infomsg_wait;       ///<インフォメーションメッセージ更新ウェイト
  s16 print_time;         ///<表示されている時間
  
  u8 title_print_type;    ///<_TITLE_PRINT_xxx
  u8 print_mission_status;  ///<現在表示しているミッション状況
  u8 print_mission_exe_flag;  ///<ミッション実行状況のメッセージ振り分けフラグ
  u8 level_char_trans_req;
  
  u16 warp_zone_id;
  u8 event_req;           ///< _EVENT_REQ_NO_xxx
  u8 decide_town_tblno;      ///<タッチで決定した街のテーブル番号
  
  u8 decide_pal_occ;
  u8 decide_pal_timer;     ///<タッチした時のパレットアニメ用タイマー
  u8 decide_pal_button_occ;     ///<TRUE:「もどる」ボタンを押した時のパレットアニメ実行中
  u8 decide_pal_button_timer;   ///<「もどる」ボタンをタッチした時のパレットアニメ用タイマー

  s16 player_pal_evy;     ///<
  s8 player_pal_dir;      ///<
  u8 player_pal_tblno;    ///<自分がいる所の街のテーブル番号
  
  u8 player_pal_trans_req;  ///<TRUE:パレット転送リクエスト
  u8 vblank_trans;
  u8 add_player_count;    ///<侵入下画面のInit後、増えたプレイヤーをカウント
  u8 bar_type;            ///<BG_BAR_TYPE_xxx

  u16 player_pal_src[PLAYER_PALANM_COLOR_NUM];        ///<変化元
  u16 player_pal_next_src[PLAYER_PALANM_COLOR_NUM];   ///<変化後
  u16 player_pal_buffer[PLAYER_PALANM_COLOR_NUM];     ///<転送バッファ

  u16 scrnbuf_time[BG_TIME_SCRN_SIZE_X * BG_TIME_SCRN_SIZE_Y];  ///< [TIME]が書かれているスクリーンデータを退避するワーク

  u8 back_exit;           ///< TRUE:「もどる」ボタンを押して下画面終了モードになっている
  u8 print_touch_player;  ///< 通信相手のアイコンをタッチした場合、その人物のNetID
  u8 mission_target_focus_netid;    ///<ミッションターゲットでフォーカス対象のプレイヤーNetID
  u8 mission_target_focus_wait;     ///<ミッションターゲットフォーカスアニメウェイト
}INTRUDE_SUBDISP;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _IntSub_SystemSetup(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_SystemExit(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BGLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_BGUnload(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BmpWinAdd(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BmpWinDel(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorResourceLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorResourceUnload(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorCreate(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorDelete(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorCreate_TouchTown(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_Area(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_CursorS(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_CursorL(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_WarpNG(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_LvNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_ActorCreate_EntryButton(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle);
static void _IntSub_Delete_EntryButton(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorUpdate_TouchTown(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_Area(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_CursorS(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy);
static void _IntSub_CursorS_MissionFocus(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_ActorUpdate_CursorL(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, ZONEID my_zone_id);
static void _IntSub_ActorUpdate_EntryButton(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy);
static void _IntSub_ActorUpdate_LvNum(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy);
static OCCUPY_INFO * _IntSub_GetArreaOccupy(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_TitleMsgUpdate(INTRUDE_SUBDISP_PTR intsub, ZONEID my_zone_id);
static void _IntSub_InfoMsgUpdate(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, ZONEID my_zone_id);
static void _SetRect(int x, int y, int half_size_x, int half_size_y, GFL_RECT *rect);
static BOOL _CheckRectHit(int x, int y, const GFL_RECT *rect);
static void _IntSub_TouchUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub, ZONEID my_zone_id);
static void _IntSub_BGBarUpdate(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BGColorUpdate(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm);
static void _IntSub_CommParamInit(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm);
static void _IntSub_CommParamUpdate(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm);
static void _SetPalFlash_DecideTown(INTRUDE_SUBDISP_PTR intsub);
static BOOL _CheckPalFlash_DecideTown(INTRUDE_SUBDISP_PTR intsub);
static void _VblankFunc(GFL_TCB *tcb, void *work);
static void _SetPalFade_PlayerTown(INTRUDE_SUBDISP_PTR intsub, int town_tblno);
static BOOL _TimeNum_CheckEnable(INTRUDE_SUBDISP_PTR intsub);
static void _TimeScrn_Clear(void);
static void _TimeScrn_Recover(INTRUDE_SUBDISP_PTR intsub, BOOL v_req);
static BOOL _TutorialMissionNoRecv(INTRUDE_SUBDISP_PTR intsub);
static void _IntSub_BGColorScreenChange(INTRUDE_SUBDISP_PTR intsub, int palace_area);
static BOOL _IntSub_CheckSameZoneID(ZONEID my_zone_id, ZONEID target_zone_id, ZONEID target_reverse_zone_id);
static void _SetPalFlash_DecideCancelButton(INTRUDE_SUBDISP_PTR intsub);
static void _UpdatePalFlash_DecideCancelButton(INTRUDE_SUBDISP_PTR intsub);


//==============================================================================
//  データ
//==============================================================================
///パレスアイコン座標
enum{
  PALACE_ICON_POS_X = 128,        ///<パレスアイコン座標X
  PALACE_ICON_POS_Y = 0xb*8+4,    ///<パレスアイコン座標Y
  PALACE_ICON_HITRANGE_HALF = 16, ///<パレスアイコンのタッチ判定半径
  
  PALACE_CURSOR_POS_X = PALACE_ICON_POS_X,      ///<パレスアイコンをカーソルが指す場合の座標X
  PALACE_CURSOR_POS_Y = PALACE_ICON_POS_Y - 16, ///<パレスアイコンをカーソルが指す場合の座標Y
};

///エリアアイコン配置座標
enum{
  AREA_POST_LEFT = 12 * 8,                           ///<配置座標範囲の左端X
  AREA_POST_RIGHT = 0x14 * 8,                         ///<配置座標範囲の右端X
  AREA_POST_WIDTH = AREA_POST_RIGHT - AREA_POST_LEFT, ///<配置座標範囲の幅
  
  AREA_POST_Y = 3*8 + 4,                              ///<配置座標Y
};

///参加ボタンのアイコン座標
enum{
  ENTRY_BUTTON_POS_X = 0x1c*8,        ///<参加ボタンのアイコン座標X
  ENTRY_BUTTON_POS_Y = 0x10*8,        ///<参加ボタンのアイコン座標Y
  ENTRY_BUTTON_HITRANGE_HALF_X = 30,  ///<参加ボタンのタッチ判定半径X
  ENTRY_BUTTON_HITRANGE_HALF_Y = 12,   ///<参加ボタンのタッチ判定半径Y
};

///各通信プレイヤー毎にずらす値
static const s8 WearOffset[FIELD_COMM_MEMBER_MAX][2] = {
  {0, -4}, {-4, -4}, {4, -4},
};



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 侵入下画面：初期設定
 *
 * @param   gsys		
 *
 * @retval  INTRUDE_SUBDISP_PTR		
 */
//==================================================================
INTRUDE_SUBDISP_PTR INTRUDE_SUBDISP_Init(GAMESYS_WORK *gsys)
{
  INTRUDE_SUBDISP_PTR intsub;
  ARCHANDLE *handle;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  int i;
  
  intsub = GFL_HEAP_AllocClearMemory(HEAPID_FIELD_SUBSCREEN, sizeof(INTRUDE_SUBDISP));
  intsub->gsys = gsys;
  intsub->now_bg_pal = 0xff;  //初回に必ず更新がかかるように0xff
  
  intsub->player_pal_tblno = PALACE_TOWN_DATA_NULL;
  intsub->print_touch_player = INTRUDE_NETID_NULL;
  intsub->mission_target_focus_netid = INTRUDE_NETID_NULL;
  intsub->print_time = -1;
  for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
    intsub->player_param[i].zone_id = ZONE_ID_MAX;
  }
  
  {
    //INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
    INTRUDE_COMM_SYS_PTR intcomm = NULL;
    
    if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION){
      //画面初期化時のみ、他画面からエラーでの戻りがあるので、game_commから直接引っ張る
      intcomm = GameCommSys_GetAppWork(game_comm);
    }
    
    if(intcomm != NULL && intcomm->subdisp_update_stop == TRUE){
      intcomm = NULL;
    }
    _IntSub_CommParamInit(intsub, intcomm);
  }
  intsub->add_player_count = intsub->comm.recv_num;
  
  handle = GFL_ARC_OpenDataHandle(ARCID_PALACE, HEAPID_FIELDMAP);
  
  _IntSub_SystemSetup(intsub);
  _IntSub_BGLoad(intsub, handle);
  _IntSub_BmpWinAdd(intsub);
  _IntSub_ActorResourceLoad(intsub, handle);
  _IntSub_ActorCreate(intsub, handle);
  
  GFL_ARC_CloseDataHandle(handle);

  //通信アイコンリロード
  GFL_NET_ReloadIconTopOrBottom(FALSE, HEAPID_FIELDMAP);
  
  //OBJWINDOW(通信アイコン) の中だけBlendで輝度が落ちないようにする
  GFL_NET_WirelessIconOBJWinON();
  G2S_SetWndOBJInsidePlane(GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 |
      GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, FALSE);
  G2S_SetWndOutsidePlane(GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 |
      GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ, TRUE);
	GXS_SetVisibleWnd(GX_WNDMASK_OW);

	//VブランクTCB登録
	intsub->vintr_tcb = GFUser_VIntr_CreateTCB(_VblankFunc, intsub, 10);

  //初回更新
  {
    PLAYER_WORK *player_work = GAMESYSTEM_GetMyPlayerWork(intsub->gsys);
    ZONEID my_zone_id = PLAYERWORK_getZoneID(player_work);
    OCCUPY_INFO *area_occupy = _IntSub_GetArreaOccupy(intsub);

    _IntSub_TitleMsgUpdate(intsub, my_zone_id);

    _IntSub_ActorUpdate_TouchTown(intsub, area_occupy);
    _IntSub_ActorUpdate_Area(intsub, area_occupy);
    _IntSub_ActorUpdate_CursorS(intsub, Intrude_Check_CommConnect(game_comm), area_occupy);
    _IntSub_ActorUpdate_CursorL(intsub, area_occupy, my_zone_id);
    _IntSub_ActorUpdate_EntryButton(intsub, area_occupy);
    _IntSub_ActorUpdate_LvNum(intsub, area_occupy);
  }

  return intsub;
}

//==================================================================
/**
 * 侵入下画面：破棄設定
 *
 * @param   intsub		
 */
//==================================================================
void INTRUDE_SUBDISP_Exit(INTRUDE_SUBDISP_PTR intsub)
{
	GFL_TCB_DeleteTask(intsub->vintr_tcb);

  //バックグラウンドの色に黒を設定しておく
  GFL_STD_MemFill16((void*)(HW_DB_BG_PLTT), 0x0000, 2);
  
  _IntSub_Delete_EntryButton(intsub);
  _IntSub_ActorDelete(intsub);
  _IntSub_ActorResourceUnload(intsub);
  _IntSub_BmpWinDel(intsub);
  _IntSub_BGUnload(intsub);
  _IntSub_SystemExit(intsub);

  GFL_HEAP_FreeMemory(intsub);

  //OBJWINDOW(通信アイコン)をOFF
  GFL_NET_WirelessIconOBJWinOFF();
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
  G2S_BlendNone();
}

//==================================================================
/**
 * 侵入下画面：更新
 *
 * @param   intsub		
 */
//==================================================================
void INTRUDE_SUBDISP_Update(INTRUDE_SUBDISP_PTR intsub, BOOL bActive)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  PLAYER_WORK *player_work = GAMESYSTEM_GetMyPlayerWork(intsub->gsys);
  ZONEID my_zone_id = PLAYERWORK_getZoneID(player_work);
  int i;
  
  WIH_DWC_GetAllBeaconTypeBit( GAMEDATA_GetWiFiList(gamedata) );  //ビーコンの電波強度アイコン反映
  
  if(intcomm == NULL || (intcomm != NULL && intcomm->subdisp_update_stop == FALSE)){
    _IntSub_CommParamUpdate(intsub, intcomm);
  }
  if(intsub->add_player_count < intsub->comm.recv_num){
    //誰かのパレスと接続した音
    if(GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE){
      PMSND_PlaySE( INTSE_PALACE_CONNECT );
    }
    intsub->add_player_count = intsub->comm.recv_num;
  }
  
	GFL_TCBL_Main( intsub->tcbl_sys );
	PRINTSYS_QUE_Main( intsub->print_que );
  PRINT_UTIL_Trans( &intsub->printutil_title, intsub->print_que );
  PRINT_UTIL_Trans( &intsub->printutil_info, intsub->print_que );
  for(i = 0; i < ENTRY_BUTTON_MSG_PATERN_MAX; i++){
    if(PRINTSYS_QUE_IsExistTarget(intsub->print_que, intsub->entrymsg_bmp[i]) == FALSE){
      BmpOam_ActorBmpTrans(intsub->entrymsg_bmpoam[i]);
    }
  }
  
  if(bActive == FALSE){
    if(_CheckPalFlash_DecideTown(intsub) == TRUE
        || intsub->decide_pal_button_occ == TRUE){  //決定アニメしている時は暗くしない
    	G2S_BlendNone();
    }
    else{
      G2S_SetBlendBrightness(GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 |
        GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ |
        GX_BLEND_PLANEMASK_BD, -FIELD_NONE_ACTIVE_EVY);
      return;
    }
  }
  else{
  	G2S_BlendNone();
  }

  if(bActive == TRUE){
    //タッチ判定チェック
    _IntSub_TouchUpdate(intcomm, intsub, my_zone_id);
  }
  
  //WFBCへのワープチェック
  if(intcomm != NULL){
    if(intsub->wfbc_go == TRUE){
      int palace_area = Intrude_GetPalaceArea(gamedata);
      
      if(GFL_NET_IsConnectMember(palace_area) == FALSE){
        intsub->wfbc_go = FALSE;
        OS_TPrintf("WFBCへの飛び先相手がいなくなったのでキャンセル netID=%d\n", palace_area);
      }
      else{
        switch(intsub->wfbc_seq){
        case 0:
          if(IntrudeSend_WfbcReq(intcomm, palace_area) == TRUE){
            intsub->wfbc_seq++;
          }
          break;
        case 1:
          if(Intrude_GetRecvWfbc(intcomm) == TRUE){
            FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(intsub->gsys);
            FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);

            intsub->warp_zone_id = ZONE_ID_PLC10;
            intsub->event_req = _EVENT_REQ_NO_TOWN_WARP;
            intsub->wfbc_go = FALSE;
          }
          break;
        default:
          GF_ASSERT(0);
          break;
        }
      }
    }
  }
}

//==================================================================
/**
 * 侵入下画面：描画
 *
 * @param   intsub		
 */
//==================================================================
void INTRUDE_SUBDISP_Draw(INTRUDE_SUBDISP_PTR intsub, BOOL bActive)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  PLAYER_WORK *player_work = GAMESYSTEM_GetMyPlayerWork(intsub->gsys);
  ZONEID my_zone_id = PLAYERWORK_getZoneID(player_work);
  OCCUPY_INFO *area_occupy;
  BOOL update = FALSE;
  
  area_occupy = _IntSub_GetArreaOccupy(intsub);

  //通信が切れている or 既に結果取得済みの場合はBG更新しない
  if(intcomm != NULL 
      && (MISSION_CheckRecvResult(&intcomm->mission) == FALSE || MISSION_CheckResultMissionMine(intcomm, &intcomm->mission) == FALSE)){
    update = TRUE;
  }
  else if(intcomm == NULL && GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE && GameCommSys_BootCheck(game_comm) != GAME_COMM_NO_INVASION && GameCommSys_GetLastStatus(game_comm) == GAME_COMM_LAST_STATUS_NULL){
    //侵入としての通信が完全に終了している場合はハイリンク島にいる時のみ更新してOK
    if(ZONEDATA_IsPalace(my_zone_id) == TRUE){
      update = TRUE;
    }
  }
  
  if(intcomm != NULL && intcomm->subdisp_update_stop == TRUE){
    update = FALSE;
  }
  
  if(update == TRUE){
    _IntSub_BGBarUpdate(intsub);
    //BGスクリーンカラー変更チェック
    _IntSub_BGColorUpdate(intsub, intcomm);
  
    //インフォメーションメッセージ
    _IntSub_TitleMsgUpdate(intsub, my_zone_id);
    _IntSub_InfoMsgUpdate(intsub, intcomm, my_zone_id);
    
    //アクター更新
    _IntSub_ActorUpdate_TouchTown(intsub, area_occupy);
    _IntSub_ActorUpdate_Area(intsub, area_occupy);
    _IntSub_ActorUpdate_CursorS(intsub, intcomm, area_occupy);
    _IntSub_ActorUpdate_CursorL(intsub, area_occupy, my_zone_id);
    _IntSub_ActorUpdate_EntryButton(intsub, area_occupy);
    _IntSub_ActorUpdate_LvNum(intsub, area_occupy);
    
    _UpdatePalFlash_DecideCancelButton(intsub);
  }
}

//==================================================================
/**
 * 侵入下画面：イベント起動チェック
 *
 * @param   intsub		
 * @param   bEvReqOK		TRUE:イベント起動してもよい　FALSE：他のイベントが起動中
 *
 * @retval  GMEVENT*		
 */
//==================================================================
GMEVENT* INTRUDE_SUBDISP_EventCheck(INTRUDE_SUBDISP_PTR intsub, BOOL bEvReqOK, FIELD_SUBSCREEN_WORK* subscreen )
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(intsub->gsys);
  GMEVENT *event = NULL;
  
  if(bEvReqOK == FALSE || fieldWork == NULL){
    return NULL;
  }
  
  if(intsub->back_exit == TRUE || (GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE && Intrude_Check_CommConnect(game_comm) == NULL)){
    PMSND_PlaySE(INTSE_BUTTON_CANCEL);
    _SetPalFlash_DecideCancelButton(intsub);
    return EVENT_ChangeSubScreen_to_CGear(intsub->gsys, fieldWork, subscreen);
  }

  switch(intsub->event_req){
  case _EVENT_REQ_NO_TOWN_WARP:
    PMSND_PlaySE( INTSE_WARP );
    _SetPalFlash_DecideTown(intsub);
    event = EVENT_IntrudeTownWarp(intsub->gsys, fieldWork, intsub->warp_zone_id, FALSE); //FALSE=palace_to_palaceでしか使用しないので関係ない
    break;
  case _EVENT_REQ_NO_PLAYER_WARP:
    PMSND_PlaySE( INTSE_WARP );
    event = EVENT_IntrudePlayerWarp(intsub->gsys, fieldWork, Intrude_GetWarpPlayerNetID(game_comm));
    break;
  }
  if(event != NULL){
    intsub->event_req = _EVENT_REQ_NO_NULL;
    return event;
  }
  
  return NULL;
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
  INTRUDE_SUBDISP_PTR intsub = work;
  
  intsub->vblank_trans ^= 1;
  if(intsub->vblank_trans){ //フィールドの1/30に合わせるように転送も2回に1回
    if(intsub->player_pal_trans_req){
      DC_FlushRange( (void*)intsub->player_pal_buffer, PLAYER_PALANM_BUFFER_SIZE );
      GXS_LoadOBJPltt((const void *)intsub->player_pal_buffer, 
        INTSUB_ACTOR_PAL_TOUCH_MINE * 0x20 + PLAYER_PALANM_START_COLOR * 2, 
        PLAYER_PALANM_BUFFER_SIZE );
      intsub->player_pal_trans_req = FALSE;
    }
    if(intsub->level_char_trans_req){
      DC_FlushRange( (void*)intsub->num_trans_buffer, NUM_TRANS_BUFFER_SIZE );
      GXS_LoadOBJ( intsub->num_trans_buffer, intsub->num_vram_address, NUM_TRANS_BUFFER_SIZE );
      intsub->level_char_trans_req = FALSE;
    }
  }
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_SystemSetup(INTRUDE_SUBDISP_PTR intsub)
{
  intsub->clunit = GFL_CLACT_UNIT_Create(
    INTSUB_ACTOR_MAX + INTSUB_ACTOR_BMPOAM_NUM, 5, HEAPID_FIELD_SUBSCREEN);
  intsub->clrend = GFL_CLACT_USERREND_Create( INTSUB_CLACT_REND, INTSUB_CLACT_REND_MAX, HEAPID_FIELD_SUBSCREEN );
  GFL_CLACT_UNIT_SetUserRend( intsub->clunit, intsub->clrend );

  intsub->bmpoam_sys = BmpOam_Init(HEAPID_FIELD_SUBSCREEN, intsub->clunit);

	intsub->tcbl_sys = GFL_TCBL_Init(HEAPID_FIELD_SUBSCREEN, HEAPID_FIELD_SUBSCREEN, 4, 32);
	intsub->font_handle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_FIELD_SUBSCREEN );
	intsub->wordset = WORDSET_Create(HEAPID_FIELD_SUBSCREEN);
  intsub->print_que = PRINTSYS_QUE_Create( HEAPID_FIELD_SUBSCREEN );
  intsub->msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
    NARC_message_invasion_dat, HEAPID_FIELD_SUBSCREEN );
  intsub->msgdata_mission = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
    NARC_message_mission_dat, HEAPID_FIELD_SUBSCREEN );
  
  intsub->strbuf_temp = GFL_STR_CreateBuffer(128, HEAPID_FIELD_SUBSCREEN);
  intsub->strbuf_info = GFL_STR_CreateBuffer(128, HEAPID_FIELD_SUBSCREEN);
  intsub->strbuf_title = GFL_STR_CreateBuffer(128, HEAPID_FIELD_SUBSCREEN);
}

//--------------------------------------------------------------
/**
 * システム関連の破棄
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_SystemExit(INTRUDE_SUBDISP_PTR intsub)
{
  GFL_STR_DeleteBuffer(intsub->strbuf_temp);
  GFL_STR_DeleteBuffer(intsub->strbuf_info);
  GFL_STR_DeleteBuffer(intsub->strbuf_title);
  
  GFL_MSG_Delete(intsub->msgdata);
  GFL_MSG_Delete(intsub->msgdata_mission);
  PRINTSYS_QUE_Clear(intsub->print_que);
  PRINTSYS_QUE_Delete(intsub->print_que);
  WORDSET_Delete(intsub->wordset);
	GFL_FONT_Delete(intsub->font_handle);
	GFL_TCBL_Exit(intsub->tcbl_sys);

  BmpOam_Exit(intsub->bmpoam_sys);
  GFL_CLACT_USERREND_Delete( intsub->clrend );
  GFL_CLACT_UNIT_Delete(intsub->clunit);
}

//--------------------------------------------------------------
/**
 * BGロード
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_BGLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  u32 scrn_data_idx;
	static const GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
  	{//INTRUDE_FRAME_S_BAR
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x7000,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
  	{//INTRUDE_FRAME_S_BACKGROUND
  		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  		GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x7000,GX_BG_EXTPLTT_01,
  		3, 0, 0, FALSE
  	},
	};
	GFL_BG_SetBGControl( INTRUDE_FRAME_S_BAR, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( INTRUDE_FRAME_S_BACKGROUND, &TextBgCntDat[1], GFL_BG_MODE_TEXT );

	//BG VRAMクリア
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);

	//レジスタOFF
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
	G2S_BlendNone();
	
  //キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    handle, NARC_palace_palace_bg_lz_NCGR, INTRUDE_FRAME_S_BACKGROUND, 0, 
    0, TRUE, HEAPID_FIELDMAP);
  
  //スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, NARC_palace_palace_bg_lz_NSCR, INTRUDE_FRAME_S_BACKGROUND, 0, 
    0x800, TRUE, HEAPID_FIELDMAP);
  if(intsub->comm.recv_num <= 1){
    scrn_data_idx = NARC_palace_palace_bar_lz_NSCR;
    intsub->bar_type = BG_BAR_TYPE_SINGLE;
  }
  else{
    scrn_data_idx = NARC_palace_palace_bar2_lz_NSCR;
    intsub->bar_type = BG_BAR_TYPE_COMM;
  }
  GFL_ARCHDL_UTIL_TransVramScreen(
    handle, scrn_data_idx, INTRUDE_FRAME_S_BAR, 0, 
    0x800, TRUE, HEAPID_FIELDMAP);
  //現在いるパレスエリアのカラースクリーンに変更
  _IntSub_BGColorScreenChange(intsub, intsub->comm.now_palace_area);
  //[TIME]のスクリーンをバッファに読み込む
  {
    NNSG2dScreenData *scrnData;
    void *load_data;
    load_data = GFL_ARCHDL_UTIL_LoadScreen(
      handle, NARC_palace_palace_time_lz_NSCR, TRUE, &scrnData, HEAPID_FIELDMAP);
    GFL_STD_MemCopy16(scrnData->rawData, intsub->scrnbuf_time, 
      BG_TIME_SCRN_SIZE_X * BG_TIME_SCRN_SIZE_Y * sizeof(u16));
    GFL_HEAP_FreeMemory(load_data);
  }
  if(_TimeNum_CheckEnable(intsub) == TRUE){
    _TimeScrn_Recover(intsub, FALSE);
  }
  
  //パレット転送
  GFL_ARCHDL_UTIL_TransVramPalette(handle, NARC_palace_palace_bg_NCLR, PALTYPE_SUB_BG, 0, 
    0x20 * INTSUB_BG_PAL_MAX, HEAPID_FIELDMAP);
	//フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, 
    PALTYPE_SUB_BG, _FONT_BG_PALNO * 0x20, 0x20, HEAPID_FIELDMAP);

	GFL_BG_SetVisible( INTRUDE_FRAME_S_NULL_0, VISIBLE_OFF );
	GFL_BG_SetVisible( INTRUDE_FRAME_S_NULL_1, VISIBLE_OFF );
	GFL_BG_SetVisible( INTRUDE_FRAME_S_BAR, VISIBLE_ON );
	GFL_BG_SetVisible( INTRUDE_FRAME_S_BACKGROUND, VISIBLE_ON );
}

//--------------------------------------------------------------
/**
 * BGアンロード
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BGUnload(INTRUDE_SUBDISP_PTR intsub)
{
	GFL_BG_SetVisible( INTRUDE_FRAME_S_BAR, VISIBLE_OFF );
	GFL_BG_SetVisible( INTRUDE_FRAME_S_BACKGROUND, VISIBLE_OFF );
	GFL_BG_FreeBGControl(INTRUDE_FRAME_S_BAR);
	GFL_BG_FreeBGControl(INTRUDE_FRAME_S_BACKGROUND);
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BmpWinAdd(INTRUDE_SUBDISP_PTR intsub)
{
  GFL_BMPWIN *bmpwin;
  
  //タイトルメッセージ
  bmpwin = GFL_BMPWIN_Create( INTRUDE_FRAME_S_BAR, 
    0, 0, 0x20, 2, _FONT_BG_PALNO, GFL_BMP_CHRAREA_GET_B );
  PRINT_UTIL_Setup( &intsub->printutil_title, bmpwin );
  GFL_BMPWIN_MakeTransWindow(bmpwin);
  
  //インフォメーションメッセージ
  bmpwin = GFL_BMPWIN_Create( INTRUDE_FRAME_S_BAR, 
    1, 0x12, 30, 4, _FONT_BG_PALNO, GFL_BMP_CHRAREA_GET_B );
  PRINT_UTIL_Setup( &intsub->printutil_info, bmpwin );
  GFL_BMPWIN_MakeTransWindow(bmpwin);
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BmpWinDel(INTRUDE_SUBDISP_PTR intsub)
{
  GFL_BMPWIN_Delete(intsub->printutil_title.win);
  GFL_BMPWIN_Delete(intsub->printutil_info.win);
}

//--------------------------------------------------------------
/**
 * アクターで使用するリソースのロード
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorResourceLoad(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  //OBJ共通パレット
  intsub->index_pltt = GFL_CLGRP_PLTT_RegisterEx(
    handle, NARC_palace_palace_obj_NCLR, CLSYS_DRAW_SUB, 0, 0, 
    INTSUB_ACTOR_PAL_MAX, HEAPID_FIELDMAP );

  //OBJ共通リソース
  intsub->index_cgr = GFL_CLGRP_CGR_Register(
    handle, NARC_palace_palace_obj_lz_NCGR, TRUE, CLSYS_DRAW_SUB, HEAPID_FIELDMAP);
  intsub->index_cell = GFL_CLGRP_CELLANIM_Register(
    handle, NARC_palace_palace_obj_NCER, 
    NARC_palace_palace_obj_NANR, HEAPID_FIELD_SUBSCREEN);
  
  //font oam用のフォントパレット
  {
    ARCHANDLE *font_pal_handle;
    font_pal_handle = GFL_ARC_OpenDataHandle(ARCID_FONT, HEAPID_FIELDMAP);
    intsub->index_pltt_font = GFL_CLGRP_PLTT_RegisterComp(
      font_pal_handle, NARC_font_default_nclr, CLSYS_DRAW_SUB, 
      INTSUB_ACTOR_PAL_FONT * 0x20, HEAPID_FIELDMAP );
    GFL_ARC_CloseDataHandle(font_pal_handle);
  }
  
  //数値用のNCGとCell
  intsub->index_cgr_num = GFL_CLGRP_CGR_Register(
    handle, NARC_palace_palace_num_lz_NCGR, TRUE, CLSYS_DRAW_SUB, HEAPID_FIELDMAP);
  intsub->index_cell_num = GFL_CLGRP_CELLANIM_Register(
    handle, NARC_palace_palace_num_dmmy_NCER, 
    NARC_palace_palace_num_dmmy_NANR, HEAPID_FIELD_SUBSCREEN);
  //数値用の転送用キャラクタ
  intsub->numchar_buffer = GFL_ARCHDL_UTIL_LoadBGCharacter( 
    handle, NARC_palace_palace_num_lz_NCGR, TRUE, 
    &intsub->numchara_nnsg2d, HEAPID_FIELD_SUBSCREEN);
  intsub->num_vram_address = GFL_CLGRP_CGR_GetAddr(intsub->index_cgr_num, NNS_G2D_VRAM_TYPE_2DSUB);
  intsub->num_trans_buffer = GFL_HEAP_AllocClearMemory(
    HEAPID_FIELD_SUBSCREEN, NUM_TRANS_BUFFER_SIZE);

  //VRAMからパレットアニメ対象のデータをバッファへコピー
  GFL_STD_MemCopy16((void*)(HW_DB_OBJ_PLTT + INTSUB_ACTOR_PAL_TOUCH_DECIDE * 0x20 + PLAYER_PALANM_START_COLOR * 2), intsub->player_pal_src, PLAYER_PALANM_BUFFER_SIZE);
  GFL_STD_MemCopy16(intsub->player_pal_src, intsub->player_pal_buffer, PLAYER_PALANM_BUFFER_SIZE);
  GFL_STD_MemCopy16((void*)(HW_DB_OBJ_PLTT + INTSUB_ACTOR_PAL_TOUCH_MINE * 0x20 + PLAYER_PALANM_START_COLOR * 2), intsub->player_pal_next_src, PLAYER_PALANM_BUFFER_SIZE);
}

//--------------------------------------------------------------
/**
 * アクターで使用するアクターのアンロード
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_ActorResourceUnload(INTRUDE_SUBDISP_PTR intsub)
{
  GFL_HEAP_FreeMemory(intsub->num_trans_buffer);
  GFL_HEAP_FreeMemory(intsub->numchar_buffer);
  GFL_CLGRP_PLTT_Release(intsub->index_pltt_font);
  GFL_CLGRP_PLTT_Release(intsub->index_pltt);
  GFL_CLGRP_CGR_Release(intsub->index_cgr);
  GFL_CLGRP_CGR_Release(intsub->index_cgr_num);
  GFL_CLGRP_CELLANIM_Release(intsub->index_cell);
  GFL_CLGRP_CELLANIM_Release(intsub->index_cell_num);
}

//--------------------------------------------------------------
/**
 * アクター作成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  _IntSub_ActorCreate_TouchTown(intsub, handle);
  _IntSub_ActorCreate_Area(intsub, handle);
  _IntSub_ActorCreate_CursorS(intsub, handle);
  _IntSub_ActorCreate_CursorL(intsub, handle);
  _IntSub_ActorCreate_WarpNG(intsub, handle);
  _IntSub_ActorCreate_LvNum(intsub, handle);
  _IntSub_ActorCreate_EntryButton(intsub, handle);
}

//--------------------------------------------------------------
/**
 * アクター削除
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_ActorDelete(INTRUDE_SUBDISP_PTR intsub)
{
  int i;
  
  for(i = 0; i < INTSUB_ACTOR_MAX; i++){
    GFL_CLACT_WK_Remove(intsub->act[i]);
  }
}

//--------------------------------------------------------------
/**
 * タッチ出来る街アクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_TouchTown(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_TOUCH_TOWN,   //アニメーションシーケンス
  	SOFTPRI_TOUCH_TOWN,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  GF_ASSERT(PALACE_TOWN_DATA_MAX == (INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0 + 1));
  for(i = INTSUB_ACTOR_TOUCH_TOWN_0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX; i++){
    head.pos_x = PalaceTownData[i - INTSUB_ACTOR_TOUCH_TOWN_0].subscreen_x;
    head.pos_y = PalaceTownData[i - INTSUB_ACTOR_TOUCH_TOWN_0].subscreen_y;

    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, INTSUB_CLACT_REND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
    GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[i], TRUE);  //オートアニメON
  }
  
  //タッチパレス島
  head.anmseq = PALACE_ACT_ANMSEQ_TOUCH_PALACE;
  head.pos_x = PALACE_ICON_POS_X;
  head.pos_y = PALACE_ICON_POS_Y;
  intsub->act[INTSUB_ACTOR_TOUCH_PALACE] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, INTSUB_CLACT_REND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], FALSE);  //表示OFF
  GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], TRUE);  //オートアニメON
}

//--------------------------------------------------------------
/**
 * 接続エリアアイコンアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_Area(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_AREA,     //アニメーションシーケンス
  	SOFTPRI_AREA,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  for(i = INTSUB_ACTOR_AREA_0; i <= INTSUB_ACTOR_AREA_MAX; i++){
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, INTSUB_CLACT_REND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetPlttOffs(intsub->act[i], 
      INTSUB_ACTOR_PAL_BASE_START + i-INTSUB_ACTOR_AREA_0, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
}

//--------------------------------------------------------------
/**
 * 通信相手の居場所を示すカーソルアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_CursorS(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_CUR_S,    //アニメーションシーケンス
  	SOFTPRI_CUR_S,              //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  for(i = INTSUB_ACTOR_CUR_S_0; i <= INTSUB_ACTOR_CUR_S_MAX; i++){
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, INTSUB_CLACT_REND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetPlttOffs(intsub->act[i], 
      INTSUB_ACTOR_PAL_BASE_START + i-INTSUB_ACTOR_CUR_S_0, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
    GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[i], TRUE);  //オートアニメON
  }
}

//--------------------------------------------------------------
/**
 * 自分の居場所を示すカーソルアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_CursorL(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_CUR_L_MALE,     //アニメーションシーケンス
  	SOFTPRI_CUR_L,               //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  MYSTATUS *myst = GAMEDATA_GetMyStatus(gamedata);
  
  if(MyStatus_GetMySex(myst) == PM_FEMALE){
    head.anmseq = PALACE_ACT_ANMSEQ_CUR_L_FEMALE;
  }
  intsub->act[INTSUB_ACTOR_CUR_L] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, INTSUB_CLACT_REND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_CUR_L], 
    INTSUB_ACTOR_PAL_BASE_START + GAMEDATA_GetIntrudeMyID(gamedata), CLWK_PLTTOFFS_MODE_PLTT_TOP);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_CUR_L], FALSE);  //表示OFF
  GFL_CLACT_WK_SetAutoAnmFlag(intsub->act[INTSUB_ACTOR_CUR_L], TRUE);  //オートアニメON
}

//--------------------------------------------------------------
/**
 * 通信相手のワープNGを示すアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_WarpNG(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  int i;
  GFL_CLWK_DATA head = {
  	0, 0,                       //X, Y座標
  	PALACE_ACT_ANMSEQ_WARP_NG,    //アニメーションシーケンス
  	SOFTPRI_WARP_NG,              //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  for(i = INTSUB_ACTOR_WARP_NG_0; i <= INTSUB_ACTOR_WARP_NG_MAX; i++){
    intsub->act[i] = GFL_CLACT_WK_Create(intsub->clunit, 
      intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
      &head, INTSUB_CLACT_REND_SUB, HEAPID_FIELD_SUBSCREEN);
    GFL_CLACT_WK_SetPlttOffs(intsub->act[i], 
      INTSUB_ACTOR_PAL_BASE_START + i-INTSUB_ACTOR_WARP_NG_0, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(intsub->act[i], FALSE);  //表示OFF
  }
}

//--------------------------------------------------------------
/**
 * レベル数値アクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_LvNum(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  BOOL time_enable = FALSE;
  GFL_CLWK_DATA head = {
  	0x1a*8, 0x16*8,                       //X, Y座標
  	0,                          //アニメーションシーケンス
  	SOFTPRI_LV_NUM,             //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  
  time_enable = _TimeNum_CheckEnable(intsub);
  
  intsub->act[INTSUB_ACTOR_LV_NUM_KETA_0] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr_num, intsub->index_pltt, intsub->index_cell_num, 
    &head, INTSUB_CLACT_REND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_LV_NUM_KETA_0], time_enable);
}

//--------------------------------------------------------------
/**
 * 参加ボタンアクター生成
 *
 * @param   intsub		
 * @param   handle		
 */
//--------------------------------------------------------------
static void _IntSub_ActorCreate_EntryButton(INTRUDE_SUBDISP_PTR intsub, ARCHANDLE *handle)
{
  static const GFL_CLWK_DATA head = {
  	ENTRY_BUTTON_POS_X, ENTRY_BUTTON_POS_Y,             //X, Y座標
  	PALACE_ACT_ANMSEQ_BTN,      //アニメーションシーケンス
  	SOFTPRI_ENTRY_BUTTON,       //ソフトプライオリティ
  	BGPRI_ACTOR_COMMON,         //BGプライオリティ
  };
  int i;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  
  intsub->act[INTSUB_ACTOR_ENTRY] = GFL_CLACT_WK_Create(intsub->clunit, 
    intsub->index_cgr, intsub->index_pltt, intsub->index_cell, 
    &head, INTSUB_CLACT_REND_SUB, HEAPID_FIELD_SUBSCREEN);
  GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], FALSE);  //表示OFF

  for(i = 0; i < ENTRY_BUTTON_MSG_PATERN_MAX; i++){//「さんか」BMPOAM
    BMPOAM_ACT_DATA bmpoam_head = {
      NULL,   //OAMとして表示させるBMPデータへのポインタ
      ENTRY_BUTTON_POS_X - 16, ENTRY_BUTTON_POS_Y - 8,   //X, Y
      0,      //適用するパレットのindex(GFL_CLGRP_PLTT_Registerの戻り値)
      0,      //pal_offset(pltt_indexのパレット内でのオフセット)
      SOFTPRI_ENTRY_BUTTON_MSG,
      BGPRI_ACTOR_COMMON,
      INTSUB_CLACT_REND_SUB,
      CLSYS_DRAW_SUB,
    };
    STRBUF *entry_str;
    
    intsub->entrymsg_bmp[i] = GFL_BMP_Create( 
      ENTRYMSG_BMP_SIZE_X, ENTRYMSG_BMP_SIZE_Y, GFL_BMP_16_COLOR, HEAPID_FIELD_SUBSCREEN );
    
    bmpoam_head.bmp = intsub->entrymsg_bmp[i];
    bmpoam_head.pltt_index = intsub->index_pltt_font;
    intsub->entrymsg_bmpoam[i] = BmpOam_ActorAdd(intsub->bmpoam_sys, &bmpoam_head);
    BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[i], FALSE);

    entry_str = GFL_MSG_CreateString( intsub->msgdata, msg_invasion_subdisp_000 + i );
    PRINTSYS_PrintQueColor( intsub->print_que, intsub->entrymsg_bmp[i], 0, 0, entry_str, 
      intsub->font_handle, PRINTSYS_MACRO_LSB(15,2,0) );
    GFL_STR_DeleteBuffer(entry_str);
  }

  if(intsub->back_exit == FALSE && GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], TRUE);
    BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_BACK], TRUE);
  }
}

//--------------------------------------------------------------
/**
 * 「さんか」ボタンの削除
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_Delete_EntryButton(INTRUDE_SUBDISP_PTR intsub)
{
  int i;
  
  //アクターはまとめて削除されるので、それ以外のBMPOAM等を削除
  for(i = 0; i < ENTRY_BUTTON_MSG_PATERN_MAX; i++){
    BmpOam_ActorDel(intsub->entrymsg_bmpoam[i]);
    GFL_BMP_Delete(intsub->entrymsg_bmp[i]);
  }
}

//==============================================================================
//  更新
//==============================================================================
//--------------------------------------------------------------
/**
 * 更新処理：タッチ街アクター
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_TouchTown(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i, palofs;
  
  if(_TutorialMissionNoRecv(intsub) == TRUE){
    //自分が親でチュートリアルが完了していなくて、ミッションも受注していないなら
    //どこもタッチできない
    for(i = 0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0; i++){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + i], FALSE);
    }
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], FALSE);
  }
  else if(intsub->comm.now_palace_area == GAMEDATA_GetIntrudeMyID(gamedata)
      || GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
    BOOL palace_enable = FALSE;
    //自分のエリアの為、パレス島しかタッチ出来ない
    for(i = 0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0; i++){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + i], FALSE);
    }
  #if 0
    //自分のエリアの場合はパレスは表フィールドにいないとタッチできない
    if(GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
      palace_enable = TRUE;
    }
    else{
      palace_enable = FALSE;
    }
  #endif
    
  #if 0
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], 
      INTSUB_ACTOR_PAL_TOUCH_NORMAL, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  #endif
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], palace_enable);
  }
  else{
    for(i = 0; i <= INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0; i++){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + i], TRUE);
  #if 0
      GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + i], 
        INTSUB_ACTOR_PAL_TOUCH_NORMAL, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  #endif
    }
    
  #if 0
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], 
      INTSUB_ACTOR_PAL_TOUCH_NORMAL, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  #endif
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], TRUE);
  }
  
  //現在地のROMのカラーをコピーしてくる
  for(palofs = INTSUB_ACTOR_PAL_TOUCH_NORMAL; palofs <= INTSUB_ACTOR_PAL_TOUCH_MINE; palofs++){
    GFL_STD_MemCopy16(
      (void*)(HW_DB_OBJ_PLTT + (INTSUB_ACTOR_PAL_BASE_START + intsub->comm.now_palace_area) * 0x20 + 1*2), 
      (void*)(HW_DB_OBJ_PLTT + palofs * 0x20 + 6*2), 2);
  }

  //決定した街を光らす
  if(intsub->decide_pal_occ == TRUE){
    if(intsub->decide_pal_timer % TOWN_DECIDE_PAL_FLASH_WAIT == 0){
      int pal_offset;
      if((intsub->decide_pal_timer / TOWN_DECIDE_PAL_FLASH_WAIT) & 1){
        pal_offset = INTSUB_ACTOR_PAL_TOUCH_NORMAL;
      }
      else{
        pal_offset = INTSUB_ACTOR_PAL_TOUCH_DECIDE;
      }
      if(intsub->decide_town_tblno != PALACE_TOWN_DATA_PALACE){
        GFL_CLACT_WK_SetPlttOffs(
          intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + intsub->decide_town_tblno], 
          pal_offset, CLWK_PLTTOFFS_MODE_PLTT_TOP);
      }
      else{
        GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], 
          pal_offset, CLWK_PLTTOFFS_MODE_PLTT_TOP);
      }
    }
    if(intsub->decide_pal_timer < TOWN_DECIDE_PAL_FLASH_TIMER_MAX){
      intsub->decide_pal_timer++;
    }
  }
  //自分がいる街を光らす
  else if(intsub->player_pal_tblno != PALACE_TOWN_DATA_NULL){
    int color_offset;
    if(intsub->player_pal_dir == DIR_UP){
      intsub->player_pal_evy += PLAYER_TOWN_FADE_EVY_ADD;
      if(intsub->player_pal_evy >= (17<<8)){
        intsub->player_pal_evy = (16<<8) - (intsub->player_pal_evy - (16<<8));
        intsub->player_pal_dir = DIR_DOWN;
      }
    }
    else{
      intsub->player_pal_evy -= PLAYER_TOWN_FADE_EVY_ADD;
      if(intsub->player_pal_evy < 0){
        intsub->player_pal_evy = (1<<8) - intsub->player_pal_evy;
        intsub->player_pal_dir = DIR_UP;
      }
    }
    for(color_offset = 0; color_offset < PLAYER_PALANM_COLOR_NUM; color_offset++){
      SoftFade(&intsub->player_pal_src[color_offset], &intsub->player_pal_buffer[color_offset], 1, 
        intsub->player_pal_evy >> 8, intsub->player_pal_next_src[color_offset]);
    }
    intsub->player_pal_trans_req = TRUE;
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：エリアアイコン
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_Area(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i, profile_num, net_id;
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  int my_area, now_tbl_pos = 0;
  u8 pal_tbl[FIELD_COMM_MEMBER_MAX] = {0, 0, 0};
  s32 tbl_count = 0, actno;
  
  profile_num = intsub->comm.recv_num;
  my_area = intsub->comm.now_palace_area;
  pos.y = AREA_POST_Y;
  
  //プロフィール受信済みのユーザーのパレットオフセットテーブルを作る
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(intsub->comm.recv_profile & (1 << net_id)){
      pal_tbl[tbl_count] = net_id;  //net_id = パレットオフセット
      if(net_id == my_area){
        now_tbl_pos = tbl_count;  //自分が今いるパレスエリアのテーブル位置を記憶
      }
      tbl_count++;
    }
  }
  
  actno = 0;
  for(i = 0; i < tbl_count; i++){
    if(now_tbl_pos == i){
      continue;
    }
    if(pal_tbl[i] == my_area+1 || (pal_tbl[i] == 0 && my_area == profile_num-1)){
      pos.x = AREA_POST_RIGHT;
    }
    else{
      pos.x = AREA_POST_LEFT;
    }
    act = intsub->act[INTSUB_ACTOR_AREA_0 + actno];
    GFL_CLACT_WK_SetPos(act, &pos, INTSUB_CLACT_REND_SUB);
    GFL_CLACT_WK_SetPlttOffs(act, 
      INTSUB_ACTOR_PAL_BASE_START + pal_tbl[i], CLWK_PLTTOFFS_MODE_PLTT_TOP);
    GFL_CLACT_WK_SetDrawEnable(act, TRUE);
    actno++;
  }
  for( ; i < FIELD_COMM_MEMBER_MAX; i++){
    GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_AREA_0 + i], FALSE);
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：通信相手カーソル
 *
 * @param   intsub		
 * @param   intcomm		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_CursorS(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int my_area, net_id;
  GFL_CLWK *act, *act_hate;
  GFL_CLACTPOS pos, pos_hate;
  INTRUDE_STATUS *ist;
  const PALACE_ZONE_SETTING *zonesetting, *before_zonesetting;
  u16 before_zone_id;
  
  my_area = intsub->comm.now_palace_area;

  //ミッションターゲットにされているプレイヤーのフォーカス処理
  _IntSub_CursorS_MissionFocus(intsub);

  //座標移動や表示・非表示設定
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    zonesetting = NULL;
    before_zonesetting = intsub->player_param[net_id].zonesetting;
    before_zone_id = intsub->player_param[net_id].zone_id;
    if(net_id != GAMEDATA_GetIntrudeMyID(gamedata)){
      intsub->player_param[net_id].zonesetting = NULL;
      intsub->player_param[net_id].zone_id = ZONE_ID_MAX;
    }
    
    act = intsub->act[INTSUB_ACTOR_CUR_S_0 + net_id];
    act_hate = intsub->act[INTSUB_ACTOR_WARP_NG_0 + net_id];
    if(net_id == GAMEDATA_GetIntrudeMyID(gamedata)){
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
      GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      continue;
    }
    else if(intcomm == NULL){
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
      GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      continue;
    }
    else if(intsub->comm.recv_profile & (1 << net_id)){
      ist = &intcomm->intrude_status[net_id];
    }
    else{
      GFL_CLACT_WK_SetDrawEnable(act, FALSE);
      GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      continue;
    }
    
    //戦闘中はアニメ速度アップ
    if(ist->action_status == INTRUDE_ACTION_BATTLE){
      GFL_CLACT_WK_SetAutoAnmSpeed(act, CURSOR_S_ANM_SPEED_BATTLE);
    }
    else{
      GFL_CLACT_WK_SetAutoAnmSpeed(act, CURSOR_S_ANM_SPEED_NORMAL);
    }
    
    if(ist->palace_area == my_area){  //このプレイヤーがいる街を指す
      if(ZONEDATA_IsPalace(ist->zone_id) == TRUE){
        pos.x = PALACE_CURSOR_POS_X + WearOffset[net_id][0];
        pos.y = PALACE_CURSOR_POS_Y + WearOffset[net_id][1];
        GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      }
      else{
        if(ist->zone_id != before_zone_id){
          zonesetting = IntrudeField_GetZoneSettingData(ist->zone_id);
        }
        else{
          zonesetting = before_zonesetting;
        }
        
        if(zonesetting != NULL){
          pos.x = zonesetting->sub_x + WearOffset[net_id][0];
          pos.y = zonesetting->sub_y + WearOffset[net_id][1];
          GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
        }
        else{ //不明ゾーン
          pos.x = NOTHING_ZONE_SUB_POS_X - 8;
          pos.y = NOTHING_ZONE_SUB_POS_Y + NOTHING_ZONE_SUB_POS_Y_SPACE * net_id;;
          pos_hate = pos;
          pos_hate.x = NOTHING_ZONE_SUB_POS_X;
          GFL_CLACT_WK_SetPos(act_hate, &pos_hate, INTSUB_CLACT_REND_SUB);
          GFL_CLACT_WK_SetDrawEnable(act_hate, TRUE);
        }
      }
      GFL_CLACT_WK_SetPos(act, &pos, INTSUB_CLACT_REND_SUB);
      intsub->player_param[net_id].zonesetting = zonesetting;
      intsub->player_param[net_id].zone_id = ist->zone_id;
    }
    else{ //このプレイヤーがいるパレスエリアを指す
      int s;
      GFL_CLACT_WK_SetDrawEnable(act_hate, FALSE);
      for(s = 0; s < FIELD_COMM_MEMBER_MAX; s++){
        if(GFL_CLACT_WK_GetPlttOffs(intsub->act[INTSUB_ACTOR_AREA_0 + s]) == INTSUB_ACTOR_PAL_BASE_START + net_id){
          GFL_CLACT_WK_GetPos( intsub->act[INTSUB_ACTOR_AREA_0 + s], &pos, INTSUB_CLACT_REND_SUB );
          break;
        }
      }
      if(s == FIELD_COMM_MEMBER_MAX){
        GFL_CLACT_WK_SetDrawEnable(act, FALSE);
        continue;
      }
      pos.x += WearOffset[net_id][0];
      pos.y += WearOffset[net_id][1];
      GFL_CLACT_WK_SetPos(act, &pos, INTSUB_CLACT_REND_SUB);
    }
    GFL_CLACT_WK_SetDrawEnable(act, TRUE);
  }
}

//--------------------------------------------------------------
/**
 * ミッションターゲットにされているプレイヤーアイコンのフォーカス処理
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_CursorS_MissionFocus(INTRUDE_SUBDISP_PTR intsub)
{
  if(intsub->comm.m_status == MISSION_STATUS_EXE && intsub->comm.p_md != NULL){
    int target_netid = intsub->comm.p_md->target_info.net_id;
    int pal_offset = -1;
    
    if(intsub->mission_target_focus_netid != INTRUDE_NETID_NULL
        && intsub->mission_target_focus_netid != target_netid){
      //別のプレイヤーがフォーカスされていた為、そのプレイヤーは元に戻す ※基本的にありえない
      GFL_CLACT_WK_SetPlttOffs(
        intsub->act[INTSUB_ACTOR_CUR_S_0 + intsub->mission_target_focus_netid], 
        INTSUB_ACTOR_PAL_BASE_START + intsub->mission_target_focus_netid, 
        CLWK_PLTTOFFS_MODE_PLTT_TOP);
    }
    if(intsub->mission_target_focus_netid != target_netid){
      //初セットの為、パレットを専用領域へコピー
      u16 palbuf[16];
      
      GFL_STD_MemCopy16(
        (void*)(HW_DB_OBJ_PLTT + (INTSUB_ACTOR_PAL_BASE_START + target_netid) * 0x20), 
        palbuf, 0x20);
      SoftFade(palbuf, palbuf, 16, MISSION_FOCUS_PLAYER_EVY, MISSION_FOCUS_PLAYER_COLOR);
      GFL_STD_MemCopy16(
        palbuf, (void*)(HW_DB_OBJ_PLTT + INTSUB_ACTOR_PAL_MISSION_TARGET_PLAYER * 0x20), 0x20);
      
      intsub->mission_target_focus_netid = target_netid;
    }
    
    intsub->mission_target_focus_wait++;
    if(intsub->mission_target_focus_wait == MISSION_FOCUS_PLAYER_ANMWAIT){
      pal_offset = INTSUB_ACTOR_PAL_MISSION_TARGET_PLAYER;
    }
    else if(intsub->mission_target_focus_wait >= MISSION_FOCUS_PLAYER_ANMWAIT*2){
      pal_offset = INTSUB_ACTOR_PAL_BASE_START + target_netid;
      intsub->mission_target_focus_wait = 0;
    }
    if(pal_offset != -1){
      GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_CUR_S_0 + target_netid], 
        pal_offset, CLWK_PLTTOFFS_MODE_PLTT_TOP);
    }
  }
  else{
    if(intsub->mission_target_focus_netid != INTRUDE_NETID_NULL){
      GFL_CLACT_WK_SetPlttOffs(
        intsub->act[INTSUB_ACTOR_CUR_S_0 + intsub->mission_target_focus_netid], 
        INTSUB_ACTOR_PAL_BASE_START + intsub->mission_target_focus_netid, 
        CLWK_PLTTOFFS_MODE_PLTT_TOP);
      intsub->mission_target_focus_netid = INTRUDE_NETID_NULL;
      intsub->mission_target_focus_wait = 0;
    }
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：自分カーソル
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_CursorL(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy, ZONEID my_zone_id)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GFL_CLWK *act;
  GFL_CLACTPOS pos;
  int my_net_id, player_town_tblno;
  
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  player_town_tblno = PALACE_TOWN_DATA_NULL;
  
  if(ZONEDATA_IsPalace(my_zone_id) == TRUE){
    pos.x = PALACE_CURSOR_POS_X + WearOffset[my_net_id][0];
    pos.y = PALACE_CURSOR_POS_Y + WearOffset[my_net_id][1];
    player_town_tblno = PALACE_TOWN_DATA_PALACE;
    intsub->player_param[my_net_id].zonesetting = NULL;
  }
  else{
    const PALACE_ZONE_SETTING *zonesetting;
    
    if(my_zone_id != intsub->player_param[my_net_id].zone_id){
      zonesetting = IntrudeField_GetZoneSettingData(my_zone_id);
    }
    else{
      zonesetting = intsub->player_param[my_net_id].zonesetting;
    }
    intsub->player_param[my_net_id].zonesetting = zonesetting;
    intsub->player_param[my_net_id].zone_id = my_zone_id;
    
    if(zonesetting != NULL){
      pos.x = zonesetting->sub_x + WearOffset[my_net_id][0];
      pos.y = zonesetting->sub_y + WearOffset[my_net_id][1];
      {
        int i;
        for(i = 0; i < PALACE_TOWN_DATA_MAX; i++){
          if(PalaceTownData[i].front_zone_id == zonesetting->zone_id
              || PalaceTownData[i].reverse_zone_id == zonesetting->zone_id){
            player_town_tblno = i;
            break;
          }
        }
      }
    }
    else{
      pos.x = NOTHING_ZONE_SUB_POS_X - 8;
      pos.y = NOTHING_ZONE_SUB_POS_Y + NOTHING_ZONE_SUB_POS_Y_SPACE * my_net_id;
      player_town_tblno = PALACE_TOWN_DATA_NULL;
      intsub->player_param[my_net_id].zonesetting = NULL;
    }
  }
  act = intsub->act[INTSUB_ACTOR_CUR_L];
#if 0 //自分の色が途中で変わることはありえなくなった 2010.05.29(土)
  GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_CUR_L], 
    INTSUB_ACTOR_PAL_BASE_START + my_net_id, CLWK_PLTTOFFS_MODE_PLTT_TOP);
#endif
  GFL_CLACT_WK_SetPos(act, &pos, INTSUB_CLACT_REND_SUB);
  GFL_CLACT_WK_SetDrawEnable(act, TRUE);
  
  _SetPalFade_PlayerTown(intsub, player_town_tblno);

  intsub->player_param[my_net_id].zone_id = my_zone_id;
}

//--------------------------------------------------------------
/**
 * 更新処理：参加しますボタン
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_EntryButton(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);

  if(intsub->comm.target_mine == FALSE && GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE){
    switch(intsub->comm.m_status){
    case MISSION_STATUS_NULL:
    case MISSION_STATUS_READY:
    case MISSION_STATUS_EXE:
    case MISSION_STATUS_RESULT:
    case MISSION_STATUS_NOT_ENTRY:
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_ENTRY], FALSE);
      BmpOam_ActorSetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_ENTRY], FALSE);
      break;
    default:
      GF_ASSERT(0);
      return;
    }
  }
}

//--------------------------------------------------------------
/**
 * 更新処理：レベル数値
 *
 * @param   intsub		
 * @param   area_occupy		
 */
//--------------------------------------------------------------
static void _IntSub_ActorUpdate_LvNum(INTRUDE_SUBDISP_PTR intsub, OCCUPY_INFO *area_occupy)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int i, level;
  OCCUPY_INFO *my_occupy;
  GFL_CLWK *act;

  if(_TimeNum_CheckEnable(intsub) == FALSE){
    //ミッションをやっていないのに数字が表示されているなら非表示にする
    if(GFL_CLACT_WK_GetDrawEnable(intsub->act[INTSUB_ACTOR_LV_NUM_KETA_0]) == TRUE){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_LV_NUM_KETA_0], FALSE);
      _TimeScrn_Clear();
    }
    return;
  }
  else{
    //ミッション中に数字が表示されていないなら表示
    if(GFL_CLACT_WK_GetDrawEnable(intsub->act[INTSUB_ACTOR_LV_NUM_KETA_0]) == FALSE){
      GFL_CLACT_WK_SetDrawEnable(intsub->act[INTSUB_ACTOR_LV_NUM_KETA_0], TRUE);
      _TimeScrn_Recover(intsub, TRUE);
    }
  }
  
  my_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
//  level = my_occupy->intrude_level;
  level = intsub->comm.m_timer;
  if(intsub->print_time != -1 && intsub->print_time != level && level <= 5
      && intsub->comm.m_status == MISSION_STATUS_EXE){
    PMSND_PlaySE( INTSE_MISSION_COUNTDOWN );
  }
  
  if(intsub->print_time != level){
    int num, i;
    u8 *pSrc = intsub->numchara_nnsg2d->pRawData;
    intsub->print_time = level;
    for(i = 0; i < 3; i++){
      num = level % 10;
      level /= 10;
      
      GFL_STD_MemCopy32(&pSrc[num * 0x40], &intsub->num_trans_buffer[(2-i)*0x40], 0x40);
      GFL_STD_MemCopy32(&pSrc[num * 0x40 + 0x40*10], 
        &intsub->num_trans_buffer[(2-i)*0x40 + 0x20*8], 0x40);
    }
    intsub->level_char_trans_req = TRUE;
  }
}

//--------------------------------------------------------------
/**
 * タイトルメッセージ更新処理
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_TitleMsgUpdate(INTRUDE_SUBDISP_PTR intsub, ZONEID my_zone_id)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  int print_type = _TITLE_PRINT_NULL;
  u32 msg_id;
  
  if(PRINTSYS_QUE_IsExistTarget( 
      intsub->print_que, GFL_BMPWIN_GetBmp(intsub->printutil_title.win) ) == TRUE){
    return;
  }
  
  if(ZONEDATA_IsPalace(my_zone_id) == TRUE || ZONEDATA_IsBingo(my_zone_id) == TRUE){
    if(intsub->comm.now_palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
      print_type = _TITLE_PRINT_MY_PALACE;
      msg_id = msg_invasion_title_001;
    }
    else{
      print_type = _TITLE_PRINT_OTHER_PALACE0 + intsub->comm.now_palace_area;
      msg_id = msg_invasion_title_002;
    }
  }
  else{
    if(intsub->comm.target_mine == TRUE){
      print_type = _TITLE_PRINT_CLEAR;
    }
    else{
      print_type = _TITLE_PRINT_PALACE_GO;
      msg_id = msg_invasion_title_000;
    }
  }
  
  if(intsub->title_print_type == print_type || print_type == _TITLE_PRINT_NULL){
    return; //既に表示済み
  }
  intsub->title_print_type = print_type;
  
  if(print_type == _TITLE_PRINT_CLEAR){
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(intsub->printutil_title.win), 0 );
  	GFL_BMPWIN_TransVramCharacter( intsub->printutil_title.win );
    return;
  }
  
  if(print_type >= _TITLE_PRINT_OTHER_PALACE0 && print_type <= _TITLE_PRINT_OTHER_PALACE3){
    const MYSTATUS *myst;
    myst = GAMEDATA_GetMyStatusPlayer(gamedata, intsub->comm.now_palace_area);
    WORDSET_RegisterPlayerName( intsub->wordset, 0, myst );
  }
  
  GFL_MSG_GetString( intsub->msgdata, msg_id, intsub->strbuf_temp );
  WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_title, intsub->strbuf_temp);
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(intsub->printutil_title.win), 0 );
  PRINT_UTIL_PrintColor( &intsub->printutil_title, intsub->print_que, 
    0, 0, intsub->strbuf_title, intsub->font_handle, PRINTSYS_MACRO_LSB(15,2,0) );
}

//--------------------------------------------------------------
/**
 * インフォメーションメッセージ更新処理
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_InfoMsgUpdate(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm, ZONEID my_zone_id)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GAME_COMM_INFO_MESSAGE infomsg;
  int k, my_netid;
  int print_mission_status_backup;
  BOOL msg_on = FALSE;
#if 0
  MISSION_STATUS mission_status;
#endif

  if(intsub->infomsg_wait > 0){
    intsub->infomsg_wait--;
    if(intsub->infomsg_wait == 0 && intcomm == NULL){ //侵入通信も起動していないならクリア
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(intsub->printutil_info.win), 0 );
      GFL_BMPWIN_TransVramCharacter( intsub->printutil_info.win );
    }
    return;
  }
  
  print_mission_status_backup = intsub->print_mission_status;
  intsub->print_mission_status = MISSION_STATUS_NULL;
  
  my_netid = GAMEDATA_GetIntrudeMyID(gamedata);

  
  if(intcomm != NULL && GFL_NET_GetConnectNum() <= 1){
    if(intcomm->search_count > 0){
      u32 msg_id = (intcomm->search_count == 1) ? msg_invasion_info_012 : msg_invasion_info_013;
      int i;
      for(i = 0; i < intcomm->search_count; i++){
    	  WORDSET_RegisterWord(intsub->wordset, i, intcomm->search_child[i], 
    	    intcomm->search_child_sex[i], TRUE, intcomm->search_child_lang[i]);
    	}
      GFL_MSG_GetString(intsub->msgdata, msg_id, intsub->strbuf_temp );
      WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
    }
    else{
      GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_022, intsub->strbuf_info );
    }
    msg_on = TRUE;
  }
  
  //相手の世界が消えかかっています
  if(msg_on == FALSE && intcomm != NULL && GFL_NET_GetConnectNum() > 1 
      && Intrude_CheckStartTimeoutMsgON(intcomm)){
    GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_024, intsub->strbuf_info );
    msg_on = TRUE;
  }

  //表の世界にいて近くに誰かが来ている
  if(msg_on == FALSE && GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
    const PALACE_ZONE_SETTING *trg_zs, *my_zs;
    int net_id;
    my_zs = intsub->player_param[my_netid].zonesetting;
    if(my_zs != NULL){
      for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
        if(my_netid != net_id){
          trg_zs = intsub->player_param[net_id].zonesetting;
          if(trg_zs != NULL && my_zs->sub_x == trg_zs->sub_x && my_zs->sub_y == trg_zs->sub_y){
            GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_020, intsub->strbuf_info );
            msg_on = TRUE;
            break;
          }
        }
      }
    }

    //だれかがあなたを助けに来ています
    if(msg_on == FALSE && intsub->comm.target_mine == TRUE){
      GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_023, intsub->strbuf_info );
      msg_on = TRUE;
    }
    
    //誰かがこの世界に入り込んでいます
    if(msg_on == FALSE && intsub->comm.my_area_intrude == TRUE){
      GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_021, intsub->strbuf_info );
      msg_on = TRUE;
    }
  }

  if(msg_on == FALSE && intcomm != NULL && intsub->print_touch_player != INTRUDE_NETID_NULL 
      && Intrude_GetMyStatus(gamedata, intsub->print_touch_player) != NULL){
    MYSTATUS *myst = Intrude_GetMyStatus(gamedata, intsub->print_touch_player);
    u32 msg_id;
    if(intcomm->intrude_status[intsub->print_touch_player].action_status == INTRUDE_ACTION_BATTLE){
      msg_id = msg_invasion_info_025;
    }
    else{
      msg_id = msg_invasion_info_011;
    }
    GFL_MSG_GetString(intsub->msgdata, msg_id, intsub->strbuf_temp );
    WORDSET_RegisterPlayerName( intsub->wordset, 0, myst );
    WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
    intsub->print_touch_player = INTRUDE_NETID_NULL;
    msg_on = TRUE;
  }
  else if(intsub->comm.m_status != MISSION_STATUS_NULL && intsub->comm.p_md != NULL
      && (intsub->comm.m_status == MISSION_STATUS_EXE || print_mission_status_backup != intsub->comm.m_status)){
    //ミッション発動中の場合はミッション関連のメッセージを優先して表示
    //ミッションは状況を示すメッセージの為、キューに貯めずに現在の状態をそのまま表示
    
    if(intsub->comm.m_status == MISSION_STATUS_EXE && intcomm != NULL
        && intsub->comm.p_md->cdata.type != MISSION_TYPE_ITEM){
      const PALACE_ZONE_SETTING *tar_zonesetting;
      FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(intsub->gsys);
      INTRUDE_STATUS *ist = &intcomm->intrude_status[intsub->comm.p_md->target_info.net_id];
      VecFx32 player_pos;
      FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
      FIELD_PLAYER_GetPos( fld_player, &player_pos );

      MISSIONDATA_WordsetTargetName(
        intsub->wordset, 0, &intsub->comm.p_md->target_info, HEAPID_FIELD_SUBSCREEN);
      
      //同じゾーンにいれば居場所の方向を表示
      tar_zonesetting = IntrudeField_GetZoneSettingData(ist->zone_id);
      if(intsub->comm.now_palace_area == intsub->comm.target_palace_area
          && tar_zonesetting != NULL && _IntSub_CheckSameZoneID(my_zone_id, tar_zonesetting->zone_id, tar_zonesetting->reverse_zone_id) == TRUE){
        if(ist->player_pack.vanish == TRUE){ //波乗りなどで姿が見えない
          GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_018, intsub->strbuf_temp );
          WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
          msg_on = TRUE;
        }
        else{ //姿は見える
          fx32 offset_x, offset_z;
          u32 msg_id;

          offset_x = ist->player_pack.pos.x - player_pos.x;
          offset_z = ist->player_pack.pos.z - player_pos.z;

          if(MATH_ABS(offset_x) > MATH_ABS(offset_z)){
            msg_id = (offset_x > 0) ? msg_invasion_info_016 : msg_invasion_info_017;
          }
          else{
            msg_id = (offset_z > 0) ? msg_invasion_info_015 : msg_invasion_info_014;
          }
          GFL_MSG_GetString(intsub->msgdata, msg_id, intsub->strbuf_temp );
          WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
          msg_on = TRUE;
        }
      }
      else{    //同じゾーンにはいないがCursorSで差している場所が同じの時のメッセージ
        const PALACE_ZONE_SETTING *tar_zonesetting = IntrudeField_GetZoneSettingData(ist->zone_id);
        const PALACE_ZONE_SETTING *my_zonesetting = IntrudeField_GetZoneSettingData(my_zone_id);
        
        if(tar_zonesetting != NULL && my_zonesetting != NULL 
            && tar_zonesetting->sub_x == my_zonesetting->sub_x
            && tar_zonesetting->sub_y == my_zonesetting->sub_y){
          GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_018, intsub->strbuf_temp );
          WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
          msg_on = TRUE;
        }
      }
    }

    //ミッション説明のメッセージ
    if(msg_on == FALSE && GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE){
      intsub->print_mission_status = MISSION_STATUS_NULL;
      switch(intsub->comm.m_status){
      case MISSION_STATUS_NOT_ENTRY: //ミッションは実施されているが参加していない
        GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_004, intsub->strbuf_info );
        break;
      case MISSION_STATUS_READY:     //ミッション開始待ち
        GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_005, intsub->strbuf_info );
        break;
      case MISSION_STATUS_EXE:       //ミッション中
        if(intsub->print_mission_exe_flag == 0){
          GFL_MSG_GetString(intsub->msgdata, 
            msg_invasion_info_m01 + intsub->comm.p_md->cdata.type, intsub->strbuf_info );
        }
        else{
          GFL_MSG_GetString(intsub->msgdata_mission, 
            intsub->comm.p_md->cdata.msg_id_contents, intsub->strbuf_temp );
          MISSIONDATA_Wordset(&intsub->comm.p_md->cdata, 
            &intsub->comm.p_md->target_info, intsub->wordset, HEAPID_FIELD_SUBSCREEN);
          WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
        }
        intsub->print_mission_exe_flag ^= 1;
        break;
      case MISSION_STATUS_RESULT:   //結果受信
        return;   //特に表示するものはない
      default:
        GF_ASSERT_MSG(0, "m_status=%d\n", intsub->comm.m_status);
        return;
      }
      
      if(intsub->comm.m_status != MISSION_STATUS_EXE){
        intsub->print_mission_exe_flag = 0;
      }
      intsub->print_mission_status = intsub->comm.m_status;
      msg_on = TRUE;
    }
  }
  else if(GameCommInfo_GetMessage(game_comm, &infomsg) == TRUE){
    intsub->print_mission_status = MISSION_STATUS_NULL;
    GFL_MSG_GetString(intsub->msgdata, infomsg.message_id, intsub->strbuf_temp );
    for(k = 0 ; k < INFO_WORDSET_MAX; k++){
      if(infomsg.name[k]!=NULL){
        WORDSET_RegisterWord( intsub->wordset, infomsg.wordset_no[k], infomsg.name[k], 
          infomsg.wordset_sex[k], TRUE, PM_LANG);
      }
    }
    WORDSET_ExpandStr(intsub->wordset, intsub->strbuf_info, intsub->strbuf_temp);
    msg_on = TRUE;
  }

  //裏世界にいて、まだいったことのない場所
  if(msg_on == FALSE && GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE
      && ZONEDATA_IsPalace(my_zone_id) == FALSE 
      && ZONEDATA_IsBingo(my_zone_id) == FALSE
      && FIELD_PALACE_CheckArriveZone(gamedata, my_zone_id) == FALSE){
    GFL_MSG_GetString(intsub->msgdata, msg_invasion_info_019, intsub->strbuf_info );
    msg_on = TRUE;
  }

  intsub->print_touch_player = INTRUDE_NETID_NULL;
  
  if(msg_on == TRUE){
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(intsub->printutil_info.win), 0 );
    PRINT_UTIL_PrintColor( &intsub->printutil_info, intsub->print_que, 
      0, 0, intsub->strbuf_info, intsub->font_handle, PRINTSYS_MACRO_LSB(15,2,0) );
    intsub->infomsg_wait = INFOMSG_UPDATE_WAIT;
  }
}


//==============================================================================
//  ツール
//==============================================================================
//--------------------------------------------------------------
/**
 * 侵入しているエリアの占拠情報ポインタを取得する
 *
 * @param   intsub		
 *
 * @retval  OCCUPY_INFO *		侵入先の占拠情報
 */
//--------------------------------------------------------------
static OCCUPY_INFO * _IntSub_GetArreaOccupy(INTRUDE_SUBDISP_PTR intsub)
{
  OCCUPY_INFO *area_occupy;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);

  if(intsub->comm.now_palace_area == GAMEDATA_GetIntrudeMyID(gamedata)
      || intsub->comm.now_palace_area == PALACE_AREA_NO_NULL){
    area_occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  }
  else{
    area_occupy = GAMEDATA_GetOccupyInfo(gamedata, intsub->comm.now_palace_area);
  }
  
  return area_occupy;
}

//--------------------------------------------------------------
/**
 * 自分のゾーンIDが相手のゾーンID(表、裏)と一致しているか判定(WFBCの変換も込み)
 *
 * @param   my_zone_id		
 * @param   target_zone_id		
 * @param   target_reverse_zone_id		
 *
 * @retval  BOOL		TRUE:同じゾーン　FALSE:違うゾーン
 */
//--------------------------------------------------------------
static BOOL _IntSub_CheckSameZoneID(ZONEID my_zone_id, ZONEID target_zone_id, ZONEID target_reverse_zone_id)
{
  if(my_zone_id == target_zone_id || my_zone_id == target_reverse_zone_id){
    return TRUE;
  }
  if(my_zone_id == ZONE_ID_PLC10 || my_zone_id == ZONE_ID_PLCW10){
    my_zone_id = my_zone_id == ZONE_ID_PLC10 ? ZONE_ID_PLCW10 : ZONE_ID_PLC10;
    if(my_zone_id == target_zone_id || my_zone_id == target_reverse_zone_id){
      return TRUE;
    }
  }
  return FALSE;
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * 中心値と半径を指定してRECTを作成する
 *
 * @param   x		        中心値X
 * @param   y		        中心値Y
 * @param   half_size		半径X
 * @param   half_size		半径Y
 * @param   rect		    代入先
 */
//--------------------------------------------------------------
static void _SetRect(int x, int y, int half_size_x, int half_size_y, GFL_RECT *rect)
{
  rect->top = y - half_size_y;
  rect->bottom = y + half_size_y;
  rect->left = x - half_size_x;
  rect->right = x + half_size_x;
}

//--------------------------------------------------------------
/**
 * 指定座標が矩形内に収まっているかヒットチェック
 *
 * @param   x		    座標X
 * @param   y		    座標Y
 * @param   rect		矩形
 *
 * @retval  BOOL		TRUE:ヒット。　FALSE:未ヒット
 */
//--------------------------------------------------------------
static BOOL _CheckRectHit(int x, int y, const GFL_RECT *rect)
{
  if(rect->left < x && rect->right > x && rect->top < y && rect->bottom > y){
    return TRUE;
  }
  return FALSE;
}


//--------------------------------------------------------------
/**
 * タッチ判定
 *
 * @param   intcomm		
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_TouchUpdate(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_SUBDISP_PTR intsub, ZONEID my_zone_id)
{
  u32 x, y;
  int i, my_net_id, net_id;
  GFL_RECT rect;
  GFL_CLACTPOS pos;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(intsub->gsys);
  FIELD_SUBSCREEN_WORK *subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GFL_CLWK *act;

  if(intsub->event_req != _EVENT_REQ_NO_NULL || intsub->wfbc_go == TRUE 
      || GFL_UI_TP_GetPointTrg(&x, &y) == FALSE 
      || FIELD_SUBSCREEN_GetAction( subscreen ) != FIELD_SUBSCREEN_ACTION_NONE){
    return;
  }
  
  //プレイヤーアイコンタッチ判定
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(net_id != my_net_id && (intsub->comm.recv_profile & (1 << net_id))){
      act = intsub->act[INTSUB_ACTOR_CUR_S_0 + net_id];
      if(GFL_CLACT_WK_GetDrawEnable(act) == TRUE){
        GFL_CLACT_WK_GetPos( act, &pos, INTSUB_CLACT_REND_SUB );
        if(x - TOUCH_RANGE_PLAYER_ICON_X <= pos.x && x + TOUCH_RANGE_PLAYER_ICON_X >= pos.x
            && y - TOUCH_RANGE_PLAYER_ICON_Y <= pos.y && y + TOUCH_RANGE_PLAYER_ICON_Y >= pos.y){
        #ifdef PM_DEBUG
          if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
            if(intcomm!= NULL && ZONEDATA_IsWfbc(intcomm->intrude_status[net_id].zone_id) == TRUE){
              intsub->wfbc_go = TRUE;
              intsub->wfbc_seq = 0;
            }
            else{
              Intrude_SetWarpPlayerNetID(game_comm, net_id);
              intsub->event_req = _EVENT_REQ_NO_PLAYER_WARP;
            }
            return;
          }
          else
        #endif
          if(intcomm != NULL 
              && IntrudeField_GetZoneSettingData(intcomm->intrude_status[net_id].zone_id) != NULL){
            intsub->print_touch_player = net_id;
            return;
          }
        }
      }
    }
  }
  
  if(_TutorialMissionNoRecv(intsub) == TRUE){
    return; //チュートリアル中でミッションしていないならここまで。
  }

  //街タッチ判定
  if(intsub->comm.now_palace_area != GAMEDATA_GetIntrudeMyID(gamedata)){
    for(i = 0; i < PALACE_TOWN_DATA_MAX; i++){
      _SetRect(PalaceTownData[i].subscreen_x, PalaceTownData[i].subscreen_y, 
        TOWN_ICON_HITRANGE_HALF, TOWN_ICON_HITRANGE_HALF, &rect);
      if(_CheckRectHit(x, y, &rect) == TRUE){
        if(PalaceTownData[i].front_zone_id == my_zone_id 
            || PalaceTownData[i].reverse_zone_id == my_zone_id 
            || (ZONEDATA_IsWfbc(my_zone_id) == TRUE 
            && ZONEDATA_IsWfbc(PalaceTownData[i].front_zone_id) == TRUE)){
          PMSND_PlaySE( SEQ_SE_BEEP );
          return;
        }
        if(i == PALACE_TOWN_WFBC){
          intsub->wfbc_go = TRUE;
          intsub->wfbc_seq = 0;
        }
        else{
          const PALACE_ZONE_SETTING *zonesetting = IntrudeField_GetZoneSettingData(PalaceTownData[i].front_zone_id);
          if(zonesetting == NULL){  //念のため
            continue;
          }
          intsub->warp_zone_id = zonesetting->warp_zone_id;
          intsub->event_req = _EVENT_REQ_NO_TOWN_WARP;
        }
        intsub->decide_town_tblno = i;
        return;
      }
    }
  }

  //パレス島タッチ判定
  if(intsub->comm.now_palace_area != GAMEDATA_GetIntrudeMyID(gamedata)
      || GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
    _SetRect(PALACE_ICON_POS_X, PALACE_ICON_POS_Y, 
      PALACE_ICON_HITRANGE_HALF, PALACE_ICON_HITRANGE_HALF, &rect);
    if(_CheckRectHit(x, y, &rect) == TRUE){
      if(ZONEDATA_IsPalace(my_zone_id) == TRUE){
        PMSND_PlaySE( SEQ_SE_BEEP );
        return;
      }
      intsub->warp_zone_id = ZONE_ID_PALACE01;
      intsub->event_req = _EVENT_REQ_NO_TOWN_WARP;
      intsub->decide_town_tblno = PALACE_TOWN_DATA_PALACE;
      return;
    }
  }
  
  ///もどるボタンタッチ判定
  if(intsub->back_exit == FALSE 
      && BmpOam_ActorGetDrawEnable(intsub->entrymsg_bmpoam[ENTRY_BUTTON_MSG_PATERN_BACK]) == TRUE){
    _SetRect(ENTRY_BUTTON_POS_X, ENTRY_BUTTON_POS_Y, 
      ENTRY_BUTTON_HITRANGE_HALF_X, ENTRY_BUTTON_HITRANGE_HALF_Y, &rect);
    if(_CheckRectHit(x, y, &rect) == TRUE){
      intsub->back_exit = TRUE;
    }
  }
}

//--------------------------------------------------------------
/**
 * 接続人数を監視し、Barスクリーンの一人用と通信用を変更する
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BGBarUpdate(INTRUDE_SUBDISP_PTR intsub)
{
  BOOL scrn_load = FALSE;
  
  if(intsub->bar_type == BG_BAR_TYPE_SINGLE){
    if(intsub->comm.recv_num > 1){
      GFL_ARC_UTIL_TransVramScreen(
        ARCID_PALACE, NARC_palace_palace_bar2_lz_NSCR, INTRUDE_FRAME_S_BAR, 0, 
        0x800, TRUE, HEAPID_FIELDMAP);
      intsub->bar_type = BG_BAR_TYPE_COMM;
      scrn_load++;
    }
  }
  else{ //BG_BAR_TYPE_COMM
    if(intsub->comm.recv_num <= 1){
      GFL_ARC_UTIL_TransVramScreen(
        ARCID_PALACE, NARC_palace_palace_bar_lz_NSCR, INTRUDE_FRAME_S_BAR, 0, 
        0x800, TRUE, HEAPID_FIELDMAP);
      intsub->bar_type = BG_BAR_TYPE_SINGLE;
      scrn_load++;
    }
  }
  
  if(scrn_load){
    GFL_BMPWIN_MakeScreen(intsub->printutil_title.win);
    GFL_BMPWIN_MakeScreen(intsub->printutil_info.win);
    GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BAR);
    if(_TimeNum_CheckEnable(intsub) == TRUE){
      _TimeScrn_Recover(intsub, TRUE);
    }
  }
}

//--------------------------------------------------------------
/**
 * 侵入エリアを監視し、BGのカラーを変更する
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _IntSub_BGColorUpdate(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(intsub->gsys);
  int bg_pal;
  
  bg_pal = intsub->comm.now_palace_area;
  if(bg_pal == intsub->now_bg_pal){
    return;
  }
  
  _IntSub_BGColorScreenChange(intsub, bg_pal);
}

//--------------------------------------------------------------
/**
 * 指定パレスの色にBGスクリーンのカラーを変更する
 *
 * @param   intsub		
 * @param   palace_area		
 */
//--------------------------------------------------------------
static void _IntSub_BGColorScreenChange(INTRUDE_SUBDISP_PTR intsub, int palace_area)
{
  GFL_BG_ChangeScreenPalette(
    INTRUDE_FRAME_S_BACKGROUND, _AREA_SCREEN_COLOR_START_X, _AREA_SCREEN_COLOR_START_Y, 
    _AREA_SCREEN_COLOR_SIZE_X, _AREA_SCREEN_COLOR_SIZE_Y, 
    INTSUB_BG_PAL_BASE_START_BACK + palace_area);
  GFL_BG_ChangeScreenPalette(
    INTRUDE_FRAME_S_BAR, _AREA_SCREEN_COLOR_START_X, _AREA_SCREEN_COLOR_START_Y, 
    _AREA_SCREEN_COLOR_SIZE_X, _AREA_SCREEN_COLOR_SIZE_Y, 
    INTSUB_BG_PAL_BASE_START_TOWN + palace_area);

  if(intsub->printutil_title.win != NULL){
    GFL_BMPWIN_MakeScreen(intsub->printutil_title.win);
    GFL_BMPWIN_MakeScreen(intsub->printutil_info.win);
  }

  GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BACKGROUND);
  GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BAR);
  intsub->now_bg_pal = palace_area;
}

//--------------------------------------------------------------
/**
 * intcommから引き出すパラメータバッファの初期化
 *
 * @param   intsub		
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void _IntSub_CommParamInit(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm)
{
  INTRUDE_COMM_PARAM *comm = &intsub->comm;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  
  if(intcomm == NULL){
    comm->now_palace_area = GAMEDATA_GetIntrudeMyID(gamedata);
    comm->recv_profile = 1 << comm->now_palace_area;
    comm->recv_num = 1;
    comm->palace_in = FALSE;
    comm->m_status = MISSION_STATUS_NULL;
    comm->m_timer = 0;
  }
  else{
    _IntSub_CommParamUpdate(intsub, intcomm);
  }
}

//--------------------------------------------------------------
/**
 * intcommから引き出すパラメータの更新処理
 *
 * @param   intsub		
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void _IntSub_CommParamUpdate(INTRUDE_SUBDISP_PTR intsub, INTRUDE_COMM_SYS_PTR intcomm)
{
  INTRUDE_COMM_PARAM *comm = &intsub->comm;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  
  if(intcomm == NULL){
  #if 0
    //一部を除いて最後の状態から何も更新しない
    intsub->comm.m_status = MISSION_STATUS_NULL;
    intsub->comm.p_md = NULL;
  #else
    GFL_STD_MemClear(&intsub->comm, sizeof(INTRUDE_COMM_PARAM));
  #endif
    intsub->add_player_count = 1; //自分
  }
  else{
    const MISSION_DATA *md;
    int i, my_netid;
    
    my_netid = GAMEDATA_GetIntrudeMyID(gamedata);
    comm->now_palace_area = intcomm->intrude_status_mine.palace_area;
    comm->recv_profile = intcomm->recv_profile;
    comm->recv_num = MATH_CountPopulation((u32)(comm->recv_profile));
    comm->palace_in = intcomm->palace_in;
    comm->m_status = MISSION_FIELD_CheckStatus(&intcomm->mission);
    comm->m_timer = MISSION_GetMissionTimer(&intcomm->mission);
    comm->p_md = MISSION_GetRecvData(&intcomm->mission);
    if(comm->p_md != NULL){
      comm->target_palace_area = intcomm->intrude_status[comm->p_md->target_info.net_id].palace_area;
    }
    comm->target_mine = IntrudeField_Check_Tutorial_OR_TargetMine(intcomm);
    comm->my_area_intrude = FALSE;
    for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
      if(i != my_netid && comm->now_palace_area == intcomm->intrude_status[i].palace_area){
        comm->my_area_intrude = TRUE;
        break;
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 街タッチの決定パレットアニメを発動
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _SetPalFlash_DecideTown(INTRUDE_SUBDISP_PTR intsub)
{
  int i;
  
  intsub->decide_pal_timer = 0;
  intsub->decide_pal_occ = TRUE;
  
  //全てのタッチアクターをノーマル色にする
  for(i = 0; i < INTSUB_ACTOR_TOUCH_TOWN_MAX - INTSUB_ACTOR_TOUCH_TOWN_0; i++){
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + i], 
      INTSUB_ACTOR_PAL_TOUCH_NORMAL, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  }
  GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], 
    INTSUB_ACTOR_PAL_TOUCH_NORMAL, CLWK_PLTTOFFS_MODE_PLTT_TOP);
}

//--------------------------------------------------------------
/**
 * 街タッチの決定パレットアニメが動作中かを調べる
 * @param   intsub		
 * @retval  BOOL		  TRUE:動作中　FALSE:動作していない
 */
//--------------------------------------------------------------
static BOOL _CheckPalFlash_DecideTown(INTRUDE_SUBDISP_PTR intsub)
{
  if(intsub->decide_pal_occ == TRUE && intsub->decide_pal_timer < TOWN_DECIDE_PAL_FLASH_TIMER_MAX){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 自分がいる街に対してパレットアニメリクエストをかける
 *
 * @param   intsub		
 * @param   town_tblno		
 */
//--------------------------------------------------------------
static void _SetPalFade_PlayerTown(INTRUDE_SUBDISP_PTR intsub, int town_tblno)
{
  if(intsub->player_pal_tblno == town_tblno || intsub->decide_pal_occ == TRUE){
    return;
  }
  
  //今まで光っていた箇所をノーマル色にする
  if(intsub->player_pal_tblno < PALACE_TOWN_DATA_MAX){
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + intsub->player_pal_tblno], 
      INTSUB_ACTOR_PAL_TOUCH_NORMAL, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  }
  else if(intsub->player_pal_tblno == PALACE_TOWN_DATA_PALACE){
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], 
      INTSUB_ACTOR_PAL_TOUCH_NORMAL, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  }
  
  //次に光らせる箇所を専用のパレットに変える
  if(town_tblno < PALACE_TOWN_DATA_MAX){
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_TOWN_0 + town_tblno], 
      INTSUB_ACTOR_PAL_TOUCH_MINE, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  }
  else if(town_tblno == PALACE_TOWN_DATA_PALACE){
    GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_TOUCH_PALACE], 
      INTSUB_ACTOR_PAL_TOUCH_MINE, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  }
  
  intsub->player_pal_tblno = town_tblno;
  intsub->player_pal_evy = 0;
  intsub->player_pal_dir = DIR_UP;
}

//--------------------------------------------------------------
/**
 * 「もどる」ボタンの決定パレットアニメを発動
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _SetPalFlash_DecideCancelButton(INTRUDE_SUBDISP_PTR intsub)
{
  int i;
  
  intsub->decide_pal_button_timer = 0;
  intsub->decide_pal_button_occ = TRUE;
  GFL_CLACT_WK_SetPlttOffs(intsub->act[INTSUB_ACTOR_ENTRY], 
    INTSUB_ACTOR_PAL_TOUCH_NORMAL, CLWK_PLTTOFFS_MODE_PLTT_TOP);
}

//--------------------------------------------------------------
/**
 * 「もどる」ボタンの決定パレットアニメを更新
 *
 * @param   intsub		
 */
//--------------------------------------------------------------
static void _UpdatePalFlash_DecideCancelButton(INTRUDE_SUBDISP_PTR intsub)
{
  if(intsub->decide_pal_button_occ == FALSE){
    return;
  }
  
  if(intsub->decide_pal_button_timer % TOWN_DECIDE_PAL_FLASH_WAIT == 0){
    int pal_offset;
    if((intsub->decide_pal_button_timer / TOWN_DECIDE_PAL_FLASH_WAIT) & 1){
      pal_offset = INTSUB_ACTOR_PAL_TOUCH_NORMAL;
    }
    else{
      pal_offset = INTSUB_ACTOR_PAL_TOUCH_DECIDE;
    }
    GFL_CLACT_WK_SetPlttOffs(
      intsub->act[INTSUB_ACTOR_ENTRY], pal_offset, CLWK_PLTTOFFS_MODE_PLTT_TOP);
  }
  if(intsub->decide_pal_button_timer < TOWN_DECIDE_PAL_FLASH_TIMER_MAX){
    intsub->decide_pal_button_timer++;
  }
}

//--------------------------------------------------------------
/**
 * [TIME]の文字を表示する必要がある状態かを調べる
 *
 * @param   intsub		
 *
 * @retval  BOOL		  TRUE:表示する必要がある
 */
//--------------------------------------------------------------
static BOOL _TimeNum_CheckEnable(INTRUDE_SUBDISP_PTR intsub)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);

  if(GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE){
    switch(intsub->comm.m_status){
    case MISSION_STATUS_READY:     //ミッション開始待ち
    case MISSION_STATUS_EXE:       //ミッション中
    case MISSION_STATUS_RESULT:    //結果が届いている or 結果表示中
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * [TIME]が書かれているスクリーンをクリアする
 *
 * @param   none		
 */
//--------------------------------------------------------------
static void _TimeScrn_Clear(void)
{
  GFL_BG_FillScreen( INTRUDE_FRAME_S_BAR, BG_TIME_SCRN_CLEAR_CODE, 
    BG_TIME_SCRN_POS_X, BG_TIME_SCRN_POS_Y, BG_TIME_SCRN_SIZE_X, BG_TIME_SCRN_SIZE_Y, 
    GFL_BG_SCRWRT_PALIN );
  GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BAR);
}
//--------------------------------------------------------------
/**
 * [TIME]が書かれているスクリーンに復帰する
 *
 * @param   intsub		
 * @param   v_req     TRUE:Vブランクで転送　FALSE:即転送
 */
//--------------------------------------------------------------
static void _TimeScrn_Recover(INTRUDE_SUBDISP_PTR intsub, BOOL v_req)
{
  GFL_BG_WriteScreenExpand( INTRUDE_FRAME_S_BAR, BG_TIME_SCRN_POS_X, BG_TIME_SCRN_POS_Y, 
    BG_TIME_SCRN_SIZE_X, BG_TIME_SCRN_SIZE_Y,
    intsub->scrnbuf_time, 0, 0, BG_TIME_SCRN_SIZE_X, BG_TIME_SCRN_SIZE_Y);
  if(v_req == TRUE){
    GFL_BG_LoadScreenV_Req(INTRUDE_FRAME_S_BAR);
  }
  else{
    GFL_BG_LoadScreenReq(INTRUDE_FRAME_S_BAR);
  }
}

//--------------------------------------------------------------
/**
 * 親機でありチュートリアル実行中でミッションも受信していない状態か判定
 *
 * @param   intsub		
 *
 * @retval  BOOL		TRUE:その状態。
 */
//--------------------------------------------------------------
static BOOL _TutorialMissionNoRecv(INTRUDE_SUBDISP_PTR intsub)
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(intsub->gsys);
  
  if(intsub->comm.p_md == NULL && GAMEDATA_GetIntrudeMyID(gamedata) == 0 
      && Intrude_CheckTutorialComplete(gamedata) == FALSE){
    return TRUE;
  }
  return FALSE;
}
