//======================================================================
/**
 *
 * @file  event_debug_menu.c
 * @brief フィールドデバッグメニューの制御イベント
 * @author  kagaya
 * @date  2008.11.13
 * @note  フィールドオーバーレイ削減のため、
 *        このソースにデバッグ用関数を追加せず、各オーバーレイ領域に置いてから、
 *        テーブルに登録して下さい。
 */
//======================================================================
#ifdef  PM_DEBUG

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"

#include "field/field_msgbg.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "field/eventdata_sxy.h"
#include "event_debug_menu.h"
#include "sound/pm_sndsys.h"

#include "field_debug.h" 
#include "field_event_check.h"
#include "event_debug_local.h"

//オーバーレイ切り分け
#include "event_debug_menu_ov_jump.h"
#include "event_debug_menu_ov_field.h"
#include "event_debug_menu_ov_system.h"
#include "event_debug_menu_ov_data.h"
#include "event_debug_menu_ov_net.h"
#include "event_debug_menu_ov_app.h"
#include "event_debug_menu_ov_staff.h"
#include "event_debug_menu_ov_zukan.h"
#include "event_debug_menu_ov_bsubway.h"
#include "event_debug_menu_ov_wbt.h"
#include "event_debug_menu_ov_infinity.h"
#include "event_debug_menu_ov_pokewood.h"
#include "event_debug_menu_ov_medalinfo.h"
#include "event_debug_menu_ov_animal_trail.h"



//======================================================================
//  debugmenu overlay
//======================================================================
FS_EXTERN_OVERLAY( debugmenu_jump );
FS_EXTERN_OVERLAY( debugmenu_field );
FS_EXTERN_OVERLAY( debugmenu_system );
FS_EXTERN_OVERLAY( debugmenu_data );
FS_EXTERN_OVERLAY( debugmenu_net );
FS_EXTERN_OVERLAY( debugmenu_app );
FS_EXTERN_OVERLAY( debugmenu_wbt );
FS_EXTERN_OVERLAY( debugmenu_staff );
FS_EXTERN_OVERLAY( debugmenu_zukan );
FS_EXTERN_OVERLAY( debugmenu_bsubway );
FS_EXTERN_OVERLAY( debugmenu_infinity );
FS_EXTERN_OVERLAY( debugmenu_medalinfo );
FS_EXTERN_OVERLAY( debugmenu_animal_trail );

//======================================================================
//  extern
//======================================================================
extern FSOverlayID debugmenu_overlayID;

//======================================================================
//  define
//======================================================================
#define D_MENU_CHARSIZE_X (18)    //メニュー横幅
#define D_MENU_CHARSIZE_Y (16)    //メニュー縦幅
#define D_MENU_LINE_NUM (9)       //メニューのライン数

//======================================================================
//  typedef struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static void DebugMenu_IineCallBack(BMPMENULIST_WORK* lw,u32 param,u8 y);
static u32 DebugMenu_GetQuickJumpIdx( u32 strID );
static GMEVENT_RESULT DebugMenuEvent( GMEVENT *event, int *seq, void *wk );


//======================================================================
//  デバッグメニューリスト
//======================================================================
//--------------------------------------------------------------
/// デバッグメニューリスト
/// データを追加する事でメニューの項目も増えます。
//--------------------------------------------------------------
static const DEBUG_MENU_LIST DATA_DebugMenuList[] =
{
  // ゾロア不具合対処
  { DEBUG_FIELD_TITLE_08, (void*)BMPMENULIST_LABEL, GFL_OVERLAY_BLANK_ID },       //○不具合対処
  { DEBUG_FIELD_ZOROA_FLAG_CLEAR, debugMenuCallProc_ZoroaFlagClear, FS_OVERLAY_ID(debugmenu_field) },    //平均負荷
  //event_debug_menu_ov_jump.c
  { DEBUG_FIELD_TITLE_00, (void*)BMPMENULIST_LABEL, GFL_OVERLAY_BLANK_ID },       //○ジャンプ（移動用）
  { DEBUG_FIELD_STR38, debugMenuCallProc_DebugSkyJump, FS_OVERLAY_ID(debugmenu_jump) },    //空を飛ぶ
  { DEBUG_FIELD_STR43, debugMenuCallProc_Jump, FS_OVERLAY_ID(debugmenu_jump) },            //ジャンプ（ユニオン等）
  { DEBUG_FIELD_STR05, debugMenuCallProc_MapZoneSelect, FS_OVERLAY_ID(debugmenu_jump) },   //ゾーン選択ジャンプ
  { DEBUG_FIELD_STR06, debugMenuCallProc_MapSeasonSelect, FS_OVERLAY_ID(debugmenu_jump)},  //四季ジャンプ
  { DEBUG_FIELD_STR05_b,   debugMenuCallProc_DebugZoneJump, FS_OVERLAY_ID(debugmenu_jump) }, //ゾーンジャンプ
  { DEBUG_FIELD_STR09, debugMenuCallProc_AllMapCheck, FS_OVERLAY_ID(debugmenu_jump) }, //オールマップチェック
  { DEBUG_FIELD_STR10, debugMenuCallProc_AllConnectCheck, FS_OVERLAY_ID(debugmenu_jump) }, //オール接続チェック

  //event_debug_menu_ov_field.c
  { DEBUG_FIELD_TITLE_02, (void*)BMPMENULIST_LABEL, GFL_OVERLAY_BLANK_ID },       //○フィールド
  { DEBUG_FIELD_STRESS_N, debugMenuCallProc_FieldAveStress, FS_OVERLAY_ID(debugmenu_field) },    //平均負荷
  { DEBUG_FIELD_STRESS_P, debugMenuCallProc_FieldAveStressPalace, FS_OVERLAY_ID(debugmenu_field) },    //パレス平均負荷
  { DEBUG_FIELD_STR17_01, debugMenuCallProc_FieldPosData, FS_OVERLAY_ID(debugmenu_field) },    //座標をみる
  { DEBUG_FIELD_STR17_02, debugMenuCallProc_FieldHitchCheckData, FS_OVERLAY_ID(debugmenu_field) },    //当たり判定をみる
  { DEBUG_FIELD_STR02, debugMenuCallProc_ControlLinerCamera, FS_OVERLAY_ID(debugmenu_field) },  //カメラ簡単操作
  { DEBUG_FIELD_STR52, debugMenuCallProc_ControlDelicateCamera, FS_OVERLAY_ID(debugmenu_field) }, //カメラ全部操作
  { DEBUG_FIELD_EVENT_CONTROL, debugMenuCallProc_EventFlagScript, FS_OVERLAY_ID(debugmenu_field) }, //イベント操作
  { DEBUG_FIELD_STR03, debugMenuCallProc_ScriptSelect, FS_OVERLAY_ID(debugmenu_field) },    //スクリプト実行
  { DEBUG_FIELD_STR13, debugMenuCallProc_MMdlList, FS_OVERLAY_ID(debugmenu_field) },        //モデルリスト
  { DEBUG_FIELD_DISGUISE, debugMenuCallProc_DisguiseList, FS_OVERLAY_ID(debugmenu_field) },  //変装
  { DEBUG_FIELD_STR15, debugMenuCallProc_ControlLight, FS_OVERLAY_ID(debugmenu_field) },      //ライト
  { DEBUG_FIELD_STR16, debugMenuCallProc_WeatherList, FS_OVERLAY_ID(debugmenu_field) },       //てんき
  { DEBUG_FIELD_STR36, debugMenuCallProc_ControlFog, FS_OVERLAY_ID(debugmenu_field) },        //フォグ操作
  { DEBUG_FIELD_STR_SUBSCRN, debugMenuCallProc_SubscreenSelect, FS_OVERLAY_ID(debugmenu_field) }, //下画面操作
  { DEBUG_FIELD_FSKILL, debugMenuCallProc_FieldSkillList, FS_OVERLAY_ID(debugmenu_field) },  //フィールド技（波乗り、怪力等）
  { DEBUG_FIELD_MVPOKE,   debugMenuCallProc_DebugMvPokemon, FS_OVERLAY_ID(debugmenu_field) },     //いどうポケモン
  { DEBUG_FIELD_STR62,   debugMenuCallProc_BBDColor, FS_OVERLAY_ID(debugmenu_field) },            //ビルボードの色
  { DEBUG_FIELD_ENCEFF, debugMenuCallProc_EncEffList, FS_OVERLAY_ID(debugmenu_field) },           //エンカウントエフェクト
  { DEBUG_FIELD_CUTIN, debugMenuCallProc_Cutin, FS_OVERLAY_ID(debugmenu_field) },           //カットイン

  //event_debug_menu_ov_system.c
  { DEBUG_FIELD_TITLE_01, (void*)BMPMENULIST_LABEL, FS_OVERLAY_ID(debugmenu_system) },       //○システム
  { DEBUG_FIELD_NUMINPUT, debugMenuCallProc_NumInput, FS_OVERLAY_ID(debugmenu_system) },     //数値入力
  { DEBUG_FIELD_RESEARCH, debugMenuCallProc_ResearchNumInput, FS_OVERLAY_ID(debugmenu_system) },//すれ違い調査
  { DEBUG_FIELD_RECORD, debugMenuCallProc_RecordNumInput, FS_OVERLAY_ID(debugmenu_system) },//レコード改ざん
  { DEBUG_FIELD_STR04, debugMenuCallProc_GameEnd, FS_OVERLAY_ID(debugmenu_system) },         //ゲーム終了
  { DEBUG_FIELD_STR60, debugMenuCallProc_ForceSave, FS_OVERLAY_ID(debugmenu_system) },       //強制セーブ
  { DEBUG_FIELD_STR53, debugMenuCallProc_UseMemoryDump, FS_OVERLAY_ID(debugmenu_system) },   //メモリ状況
  { DEBUG_FIELD_STR53_1, debugMenuCallProc_FieldMemoryDumpStart, FS_OVERLAY_ID(debugmenu_system) },  //メモリダンプ開始
  { DEBUG_FIELD_STR53_2, debugMenuCallProc_FieldMemoryDumpEnd, FS_OVERLAY_ID(debugmenu_system) },  //メモリダンプ終了
  { DEBUG_FIELD_STR22, debugMenuCallProc_ControlRtcList, FS_OVERLAY_ID(debugmenu_system) },  //時間
  { DEBUG_FIELD_STR61, debugMenuCallProc_CaptureList, FS_OVERLAY_ID(debugmenu_system) },     //キャプチャ
  { DEBUG_FIELD_STR40, debugMenuCallProc_ChangePlayerSex, FS_OVERLAY_ID(debugmenu_system) },   //主人公性別変更
  //{ DEBUG_FIELD_STR35, debugMenuCallProc_FadeSpeedChange, FS_OVERLAY_ID(debugmenu_system) },   //フェード速度変更

  //event_debug_menu_ov_data.c
  { DEBUG_FIELD_TITLE_03, (void*)BMPMENULIST_LABEL, FS_OVERLAY_ID(debugmenu_data) },       //○データ作成
  { DEBUG_FIELD_STR41, debugMenuCallProc_DebugMakePoke, FS_OVERLAY_ID(debugmenu_data) },   //ポケモン作成
  { DEBUG_FIELD_STR65, debugMenuCallProc_DebugReWritePoke, FS_OVERLAY_ID(debugmenu_data) },//ポケモン書き換え
  { DEBUG_FIELD_STR32, debugMenuCallProc_DebugItem, FS_OVERLAY_ID(debugmenu_data) },       //アイテム作成
  { DEBUG_FIELD_STR33, debugMenuCallProc_DebugSecretItem, FS_OVERLAY_ID(debugmenu_data) }, //隠されアイテム作成
  { DEBUG_FIELD_STR34, debugMenuCallProc_DebugPDWItem, FS_OVERLAY_ID(debugmenu_data) },    //PDWアイテム作成
  { DEBUG_FIELD_STR37, debugMenuCallProc_BoxMax, FS_OVERLAY_ID(debugmenu_data) },          //ボックス最大
  { DEBUG_FIELD_STR72, debugMenuCallProc_1BoxMax, FS_OVERLAY_ID(debugmenu_data) },         //ボックス１つ分最大
  { DEBUG_FIELD_STR39, debugMenuCallProc_MyItemMax, FS_OVERLAY_ID(debugmenu_data) },       //アイテム最大
  { DEBUG_FIELD_MAKE_EGG, debugMenuCallProc_MakeEgg, FS_OVERLAY_ID(debugmenu_data) },      //タマゴ作成
  { DEBUG_FIELD_SODATEYA, debugMenuCallProc_Sodateya, FS_OVERLAY_ID(debugmenu_data) },     //育て屋
  { DEBUG_FIELD_MAKE_UNDATA,   debugMenuCallProc_DebugMakeUNData, FS_OVERLAY_ID(debugmenu_data) }, //国連データ作成
  { DEBUG_FIELD_MYSTERY_00, debugMenuCallProc_MakeMysteryCardList, FS_OVERLAY_ID(debugmenu_data) },//ふしぎなおくりものカード作成
  { DEBUG_FIELD_STR63, debugMenuCallProc_SetBtlBox, FS_OVERLAY_ID(debugmenu_data) },  //不正チェックを通るポケモンを作成
  { DEBUG_FIELD_SET_BTLBOX_EGG_00, debugMenuCallProc_SetBtlBoxEgg, FS_OVERLAY_ID(debugmenu_data) },  //不正チェックを通るポケモンを作成
  { DEBUG_FIELD_STR64, debugMenuCallProc_ChangeName, FS_OVERLAY_ID(debugmenu_data) },  //主人公名を再設定
  { DEBUG_FIELD_STR67, debugMenuCallProc_EventPokeCreate, FS_OVERLAY_ID(debugmenu_data) },  //イベントポケモン作成
  { DEBUG_FIELD_STR69, debugMenuCallProc_SymbolPokeList, FS_OVERLAY_ID(debugmenu_data) },  //シンボルポケモン作成
  { DEBUG_FIELD_STR71, debugMenuCallProc_Musical, FS_OVERLAY_ID(debugmenu_data) },  //ミュージカル
  //  { DEBUG_FIELD_STR74, debugMenu_CreateMusicalShotData, FS_OVERLAY_ID(debugmenu_data) },           //ミュージカルデータ作成
  { DEBUG_FIELD_STR75, debugMenu_CreateFeelingData, FS_OVERLAY_ID(debugmenu_data) },           //フィーリング作成

  //event_debug_menu_ov_net.c
  { DEBUG_FIELD_TITLE_06, (void*)BMPMENULIST_LABEL, FS_OVERLAY_ID(debugmenu_net) },       //○つうしん
  { DEBUG_FIELD_SYACHI_COMPATIBLE, debugMenu_SyachiCompatible, FS_OVERLAY_ID(debugmenu_net) },  //syachi通信互換
  { DEBUG_FIELD_STR19, debugMenuCallProc_OpenClubMenu, FS_OVERLAY_ID(debugmenu_net) },      //WIFIクラブ
  { DEBUG_FIELD_GPOWER, debugMenuCallProc_GPowerList, FS_OVERLAY_ID(debugmenu_net)},      //Gパワー
  { DEBUG_FIELD_STR49, debugMenuCallProc_BeaconFriendCode, FS_OVERLAY_ID(debugmenu_net) },  //ともだちコード配信
  { DEBUG_FIELD_STR57, debugMenuCallProc_DebugSake, FS_OVERLAY_ID(debugmenu_net) },             //サケ操作
  { DEBUG_FIELD_GEONET, debugMenuCallProc_Geonet, FS_OVERLAY_ID(debugmenu_net) },         //ジオネット呼び出し
  { DEBUG_FIELD_GEONET_CLEAR, debugMenu_ClearWifiHistory, FS_OVERLAY_ID(debugmenu_net) }, //ジオネット情報クリア
  { DEBUG_FIELD_LIVE_COMM, debugMenu_LiveComm, FS_OVERLAY_ID(debugmenu_net) },  //ライブ通信デバッグ
  { DEBUG_FIELD_LINK_FESTIVAL, debugMenu_LinkFestival, FS_OVERLAY_ID(debugmenu_net) },  //リンクフェスティバルデバッグ
  //{ DEBUG_FIELD_MISSION_RESET, debugMenu_MissionReset, FS_OVERLAY_ID(debugmenu_net) },  //ミッションリスト再作成
#ifndef DISABLE_INTRUDE
  { DEBUG_FIELD_INTRUDE,    debugMenu_Intrude, FS_OVERLAY_ID(debugmenu_net) },      //ハイリンク
#endif
  { DEBUG_FIELD_WIFINOTE_01,  debugMenu_WriteWifiNote, FS_OVERLAY_ID(debugmenu_net) },  //友達手帳書き込み
  { DEBUG_FIELD_RESORT, debugMenu_Resort, FS_OVERLAY_ID(debugmenu_net) }, //レゾナンスリゾート

  //event_debug_menu_ov_app.c
  { DEBUG_FIELD_TITLE_04, (void*)BMPMENULIST_LABEL, GFL_OVERLAY_BLANK_ID },       //○アプリ
  { DEBUG_FIELD_STR44, debugMenuCallProc_UITemplate, FS_OVERLAY_ID(debugmenu_app) },        //UIテンプレート
  { DEBUG_FIELD_DEMO3D,   debugMenuCallProc_Demo3d, FS_OVERLAY_ID(debugmenu_app) },             //３Dデモ
  { DEBUG_FIELD_WBT,      debugMenuCallProc_WBT, FS_OVERLAY_ID(debugmenu_wbt) },      //WBT
  { DEBUG_FIELD_POKEWOOD, debugMenuCallProc_Pokewood, FS_OVERLAY_ID(debugmenu_app) }, //ポケウッド

  //event_debug_menu_ov_staff.c
  { DEBUG_FIELD_TITLE_05, (void*)BMPMENULIST_LABEL, GFL_OVERLAY_BLANK_ID },       //○スタッフ用
  { DEBUG_FIELD_STR70, debugMenuCallProc_PCScript, FS_OVERLAY_ID(debugmenu_staff) },        //パソコン呼び出し
  { DEBUG_FIELD_STR47, debugMenu_ControlShortCut, FS_OVERLAY_ID(debugmenu_staff) },           //Yボタン登録最大
  { DEBUG_FIELD_STR51  , debugMenuCallProc_OpenGTSNegoMenu, FS_OVERLAY_ID(debugmenu_staff) }, //GTSネゴ
  //event_debug_menu_ov_bsubway.c サブウェイ部分だけ大きかったので切り分け
  { DEBUG_FIELD_BSW_00, debugMenuCallProc_BSubway, FS_OVERLAY_ID(debugmenu_bsubway) },                //バトルサブウェイ
  //event_debug_menu_ov_zukan.c
  { DEBUG_FIELD_ZUKAN_04, debugMenuCallProc_Zukan, FS_OVERLAY_ID(debugmenu_zukan) },            //ずかん
  { DEBUG_FIELD_STR66,  debugMenuCallProc_RingTone, FS_OVERLAY_ID(debugmenu_staff) },           //着信音
  { DEBUG_FIELD_STR73, debugMenu_WfbcDebug, FS_OVERLAY_ID(debugmenu_staff) },                   //WFBCオートチェック
  { DEBUG_FIELD_MEDALBOX_01 , debugMenuCallProc_MedalBox , FS_OVERLAY_ID(debugmenu_medalinfo) },    // メダルボックス
  { DEBUG_FIELD_MEDAL_INFO , debugMenuCallProc_MedalInfo , FS_OVERLAY_ID(debugmenu_medalinfo) },    // メダル詳細画面
  { DEBUG_FIELD_INFINITY , debugMenuCallProc_InfinityDungeon , FS_OVERLAY_ID(debugmenu_infinity) },    // 無限ダンジョン
  { DEBUG_FIELD_ANIMAL_TRAIL , debugMenuCallProc_AnimalTrail , FS_OVERLAY_ID(debugmenu_animal_trail) },    // けものみち
  { DEBUG_FIELD_INTRO_DEMO, debugMenuCallProc_IntroDemo, FS_OVERLAY_ID(debugmenu_app) },    // 物語開始デモ
};

//------------------------------------------------------------------------
/*
 *  QUICKSJUMP
 *  上記のリストのSTRIDを書くと、STARTボタンをおしたときに、その位置までカーソルが動きます
 */
//------------------------------------------------------------------------
#if defined DEBUG_ONLY_FOR_nagihashi_toru
#define QuickJumpStart   DEBUG_FIELD_NUMINPUT
#define QuickJumpSelect  DEBUG_FIELD_RESORT
#elif defined DEBUG_ONLY_FOR_matsumiya
#define QuickJumpStart   DEBUG_FIELD_STR03
#define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_masafumi_saitou
#define QuickJumpStart   DEBUG_FIELD_STR03
#define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_suginaka_katsunori
#define QuickJumpStart   DEBUG_FIELD_STR03
#define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_mizuguchi_mai
#define QuickJumpStart   DEBUG_FIELD_STR03
#define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_ohno
#define QuickJumpStart   DEBUG_FIELD_STR19
#elif defined DEBUG_ONLY_FOR_iwasawa
#define QuickJumpStart   DEBUG_FIELD_NUMINPUT
#define QuickJumpSelect  DEBUG_FIELD_STR65
#elif defined DEBUG_ONLY_FOR_shimoyamada
#define QuickJumpStart   DEBUG_FIELD_NUMINPUT
#define QuickJumpSelect  DEBUG_FIELD_RESORT
#elif defined DEBUG_ONLY_FOR_ohkubo_tomohiko
#define QuickJumpStart   DEBUG_FIELD_NUMINPUT
#define QuickJumpSelect  DEBUG_FIELD_RESORT
#elif defined DEBUG_ONLY_FOR_onoue_masayuki
#define QuickJumpStart   DEBUG_FIELD_ZUKAN_04
#define QuickJumpSelect  DEBUG_FIELD_INFINITY
#elif defined DEBUG_ONLY_FOR_nakamura_akira
#define QuickJumpStart   DEBUG_FIELD_INFINITY
#define QuickJumpSelect  DEBUG_FIELD_INFINITY
#elif defined DEBUG_ONLY_FOR_ibe_mana
#define QuickJumpStart   DEBUG_FIELD_INFINITY
#define QuickJumpSelect  DEBUG_FIELD_INFINITY
#elif defined DEBUG_ONLY_FOR_ichiraku_katsuhiko
#define QuickJumpStart   DEBUG_FIELD_WBT
#define QuickJumpSelect  DEBUG_FIELD_WBT
#elif defined DEBUG_ONLY_FOR_tsuruta_saya
#define QuickJumpStart   DEBUG_FIELD_WBT
#define QuickJumpSelect  DEBUG_FIELD_STR13
#elif defined DEBUG_ONLY_FOR_nakatsui || defined DEBUG_ONLY_FOR_mori || defined DEBUG_ONLY_FOR_yoshida || defined DEBUG_ONLY_FOR_mizutani_megumi || defined DEBUG_ONLY_FOR_toyama_kenkichi
#define QuickJumpStart   DEBUG_FIELD_POKEWOOD
#define QuickJumpSelect  DEBUG_FIELD_POKEWOOD

#elif defined DEBUG_ONLY_FOR_ichinose || defined DEBUG_ONLY_FOR_sato_hitomi
#define QuickJumpStart   DEBUG_FIELD_WBT
#define QuickJumpSelect  DEBUG_FIELD_POKEWOOD

#elif defined DEBUG_ONLY_FOR_tamada || defined DEBUG_ONLY_FOR_tanaka_ryota
#define QuickJumpStart   DEBUG_FIELD_WBT
#define QuickJumpSelect  DEBUG_FIELD_POKEWOOD

#elif defined DEBUG_ONLY_FOR_shiiyama_yukiko
#define QuickJumpStart   DEBUG_FIELD_NUMINPUT
#define QuickJumpSelect  DEBUG_FIELD_RESORT

#elif defined DEBUG_ONLY_FOR_fuchino_hiroki
#define QuickJumpStart   DEBUG_FIELD_INTRO_DEMO
#define QuickJumpSelect  DEBUG_FIELD_INTRO_DEMO

#else
//それ以外の場合（提出ロム等）
#define QuickJumpStart  DEBUG_FIELD_STR41
#define QuickJumpSelect DEBUG_FIELD_SYACHI_COMPATIBLE
#endif

//--------------------------------------------------------------
/// メニューヘッダー
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER DATA_DebugMenuListHeader =
{
  1,    //リスト項目数
  D_MENU_LINE_NUM,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  2,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
  DebugMenu_IineCallBack,  //１行ごとのコールバック
};

//--------------------------------------------------------------
/**
 * @brief フィールドデバッグメニュー初期化データ
 */
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugMenuData = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_DebugMenuList),
  DATA_DebugMenuList,
  &DATA_DebugMenuListHeader,
  1, 1, D_MENU_CHARSIZE_X, D_MENU_CHARSIZE_Y,
  NULL,
  NULL,
};


//======================================================================
//  汎用リスト　ヘッダー
//======================================================================
const FLDMENUFUNC_HEADER DATA_DebugMenuList_Default =
{
  1,    //リスト項目数
  9,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

/// どこでもジャンプ　メニューヘッダー
const FLDMENUFUNC_HEADER DATA_DebugMenuList_ZoneSel =
{
  1,    //リスト項目数
  9,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

//======================================================================
//  イベント：フィールドデバッグメニュー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドデバッグメニューイベント起動
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param heapID  HEAPID
 * @param page_id デバッグメニューページ
 * @retval  GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
    HEAPID heapID, u16 page_id )
{
  DEBUG_MENU_EVENT_WORK * dmew;
  GMEVENT * event;

  event = GMEVENT_Create(
      gsys, NULL, DebugMenuEvent, sizeof(DEBUG_MENU_EVENT_WORK));

  dmew = GMEVENT_GetEventWork(event);
  GFL_STD_MemClear( dmew, sizeof(DEBUG_MENU_EVENT_WORK) );

  dmew->gmSys = gsys;
  dmew->gmEvent = event;
  dmew->gdata = GAMESYSTEM_GetGameData( gsys );
  dmew->fieldWork = fieldWork;
  dmew->heapID = heapID;


  // POS表示更新　を　停止
  {
    FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( fieldWork );
    FIELD_DEBUG_SetPosUpdateFlag( debug, FALSE );
  }

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フィールドデバッグメニューで使用しているオーバーレイを読込み
 */
//-----------------------------------------------------------------------------
void DEBUG_EVENT_Init( GAMEDATA * p_gamedata )
{
  if( debugmenu_overlayID != GFL_OVERLAY_BLANK_ID )
  {
    GFL_OVERLAY_Load( debugmenu_overlayID );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フィールドデバッグメニューで使用したデータ消去
 */
//-----------------------------------------------------------------------------
void DEBUG_EVENT_Exit( GAMEDATA * p_gamedata )
{
  if( debugmenu_overlayID != GFL_OVERLAY_BLANK_ID )
  {
    GFL_OVERLAY_Unload( debugmenu_overlayID );
  }
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief
 * @param fieldmap
 * @param heapID
 * @param init
 * @param cb_work
 * @param list_pos
 * @param cursor_pos
 */
//--------------------------------------------------------------
static FLDMENUFUNC * DebugMenuInitCore(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init, void* cb_work, u16 list_pos, u16 cursor_pos, u32 msgarcid )
{
  GAMESYS_WORK * gsys;
  FLDMENUFUNC * ret;
  GFL_MSGDATA *msgData;
  FLDMSGBG *msgBG;
  FLDMENUFUNC_LISTDATA *listdata;
  FLDMENUFUNC_HEADER menuH;
  u16 max;

  gsys = FIELDMAP_GetGameSysWork( fieldmap );
  msgBG = FIELDMAP_GetFldMsgBG( fieldmap );
  msgData = GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, msgarcid, init->msg_dat_id, FIELDMAP_GetHeapID( fieldmap ) );

  if (init->getMaxFunc) {
    max = init->getMaxFunc( gsys, cb_work );
  } else {
    max = init->max;
  }
  if (init->makeListFunc) {
    listdata = FLDMENUFUNC_CreateListData( max, heapID );
    init->makeListFunc( gsys, listdata, heapID, msgData, cb_work );
  } else {
    int i;
    listdata = FLDMENUFUNC_CreateListData( max, heapID );

    for( i = 0; i < max; i++ ){
      BmpMenuWork_ListAddArchiveString(
          (BMP_MENULIST_DATA*)listdata, msgData,
          init->menulist[i].str_id, (u32)init->menulist[i].selectParam, heapID );
    }
    //listdata = FLDMENUFUNC_CreateMakeListData( init->menulist, max, msgData, heapID );
  }

  menuH = *(init->menuH);
  {
    u8 sy = (max * 2 < init->sy) ? max * 2 : init->sy;
    FLDMENUFUNC_InputHeaderListSize( &menuH, max, init->px, init->py, init->sx, sy );
  }
  ret = FLDMENUFUNC_AddMenuList( msgBG, &menuH, listdata, list_pos, cursor_pos );
  GFL_MSG_Delete( msgData );
  return ret;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
FLDMENUFUNC * DEBUGFLDMENU_Init(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init )
{
  return DebugMenuInitCore( fieldmap, heapID, init, NULL, 0, 0, ARCID_DEBUG_MESSAGE );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
FLDMENUFUNC * DEBUGFLDMENU_InitArcID(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init, u32 msg_arc_id )
{
  return DebugMenuInitCore( fieldmap, heapID, init, NULL, 0, 0, msg_arc_id );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
FLDMENUFUNC * DEBUGFLDMENU_InitEx(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init, void* cb_work )
{
  return DebugMenuInitCore( fieldmap, heapID, init, cb_work, 0, 0, ARCID_DEBUG_MESSAGE );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
FLDMENUFUNC * DEBUGFLDMENU_InitExPos(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init, u16 list_pos, u16 cursor_pos )
{
  return DebugMenuInitCore( fieldmap, heapID, init, NULL, list_pos, cursor_pos, ARCID_DEBUG_MESSAGE );
}

//--------------------------------------------------------------
/**
 * @param arcDatIDMsg メッセージが含まれるアーカイブデータID
 * @retval  GFL_MSGDATA*
 */
//--------------------------------------------------------------
GFL_MSGDATA * DEBUGFLDMENU_CreateMSGDATA( FIELDMAP_WORK * fieldmap, u32 arcDatIDMsg )
{
  GFL_MSGDATA *msgData;
  msgData = GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, arcDatIDMsg, FIELDMAP_GetHeapID( fieldmap ) );
  return( msgData );
}
//----------------------------------------------------------------------------
/**
 *	@brief  クイックジャンプ後のカーソル位置計算
 *
 *	@param	strID       関数位置       
 *	@param	*cursor_pos カーソル位置格納場所
 *	@param	*list_pos   リスト位置格納場所
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetJumpEndCursorPosGet( u32 strID ,u32 *cursor_pos ,u32 *list_pos )
{
  u32 idx       = DebugMenu_GetQuickJumpIdx( strID );
  u32 list_move = (NELEMS(DATA_DebugMenuList)) - idx;
  u32 list_center = D_MENU_LINE_NUM/2;
  *cursor_pos   = list_center;
  *list_pos     = idx-list_center;
  if( list_move <= list_center )
  {
    *list_pos = (NELEMS(DATA_DebugMenuList)-D_MENU_LINE_NUM); // 一番上に
    *cursor_pos = D_MENU_LINE_NUM - list_move;
  }
}

//--------------------------------------------------------------
/**
 * イベント：フィールドデバッグメニュー
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT DebugMenuEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MENU_EVENT_WORK *work = wk;

  switch (*seq) {
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
#ifdef QuickJumpStart
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
        {
          u32 cursor_pos,list_pos;
          GetJumpEndCursorPosGet( QuickJumpStart ,&cursor_pos, &list_pos );
          FLDMENUFUNC_DeleteMenu( work->menuFunc );
          work->menuFunc  = DEBUGFLDMENU_InitExPos( work->fieldWork, work->heapID,  &DebugMenuData, list_pos, cursor_pos );
        }
#endif  //QuickJumpStart
#ifdef QuickJumpSelect
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
        {
          u32 cursor_pos,list_pos;
          GetJumpEndCursorPosGet( QuickJumpSelect ,&cursor_pos, &list_pos );
          FLDMENUFUNC_DeleteMenu( work->menuFunc );
          work->menuFunc  = DEBUGFLDMENU_InitExPos( work->fieldWork, work->heapID,  &DebugMenuData, list_pos, cursor_pos );
        }
#endif  //QuickJumpSelect

        break;
      }else if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        (*seq)++;
      }else{              //決定
        work->call_proc = (DEBUG_MENU_CALLPROC)ret;
        (*seq)++;
      }
    }
    break;
  case 2:
    //オーバーレイ読込み
    {
      const u32 idx = FLDMENUFUNC_GetNowIndex( work->menuFunc );
      const FSOverlayID overlayID = DebugMenuData.menulist[ idx ].overlayID;

      //オーバーレイキャッシュ処理
      //同じだったら何もしない
      if( debugmenu_overlayID != overlayID )
      {
        if( debugmenu_overlayID != GFL_OVERLAY_BLANK_ID )
        {
          GFL_OVERLAY_Unload( debugmenu_overlayID );
        }

        if( overlayID != GFL_OVERLAY_BLANK_ID )
        {
          GFL_OVERLAY_Load( overlayID );
        }

        debugmenu_overlayID = overlayID;
      }
    }
    (*seq)++;
    break;
  case 3:
    {
      const u32 idx = FLDMENUFUNC_GetNowIndex( work->menuFunc );
      const FSOverlayID overlayID = DebugMenuData.menulist[ idx ].overlayID;

      //メニュー削除
      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( work->call_proc != NULL ){
        if( work->call_proc(work) == TRUE ){
          return( GMEVENT_RES_CONTINUE );
        }
      }

      // POS表示更新　を　再開
      {
        FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( work->fieldWork );
        FIELD_DEBUG_SetPosUpdateFlag( debug, TRUE );
      }
      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  １行ごとのコールバック
 *
 *  @param  lw    ワーク
 *  @param  param 引数
 *  @param  y     Y位置
 */
//-----------------------------------------------------------------------------
static void DebugMenu_IineCallBack(BMPMENULIST_WORK* lw,u32 param,u8 y)
{
  if(param == BMPMENULIST_LABEL){
    BmpMenuList_TmpColorChange( lw,4,0,4);
  }else{
    BmpMenuList_TmpColorChange(lw,1,0,2);
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  関数の位置を取得
 *
 *  @param  void *p_adrs  サーチする関数
 *
 *  @return 関数のインデックス
 */
//-----------------------------------------------------------------------------
static u32 DebugMenu_GetQuickJumpIdx( u32 strID )
{
  int i;
  for( i = 0; i < NELEMS(DATA_DebugMenuList); i++ )
  {
    if( DATA_DebugMenuList[i].str_id == strID )
    {
      return i;
    }
  }
  GF_ASSERT_MSG( 0, "デバッグメニューのクイックジャンプが間違っています\n" );
  return 0;
}

#endif  //  PM_DEBUG

