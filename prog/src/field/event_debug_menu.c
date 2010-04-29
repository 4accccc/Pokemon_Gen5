//======================================================================
/**
 *
 * @file  event_debug_menu.c
 * @brief フィールドデバッグメニューの制御イベント
 * @author  kagaya
 * @date  2008.11.13
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
#include "msg/msg_d_field.h"

#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "field/eventdata_sxy.h"

#include "event_debug_menu.h"
#include "event_mapchange.h"
#include "event_gamestart.h"

#include "net_app/irc_match/ircbattlemenu.h"
#include "event_ircbattle.h"
#include "event_wificlub.h"
#include "event_gtsnego.h"
#include "field_subscreen.h"
#include "sound/pm_sndsys.h"

#include "font/font.naix"

#include  "field/weather_no.h"
#include  "weather.h"
#include  "msg/msg_d_tomoya.h"

#include "field_debug.h"

#include "field_event_check.h"
#include "event_debug_item.h" //EVENT_DebugItemMake
#include "event_debug_numinput.h"
#include "event_debug_numinput_research.h"
#include "event_debug_numinput_record.h"
#include "savedata/box_savedata.h"  //デバッグアイテム生成用
#include  "item/itemsym.h"  //ITEM_DATA_MAX
#include  "item/item.h"  //ITEM_CheckEnable
#include "app/townmap.h"
#include "../ui/debug/ui_template.h"
#include "savedata/shortcut.h"
#include "event_debug_beacon.h"
#include "app/waza_oshie.h"

#include "field_buildmodel.h"

#include "field_sound.h"

#include "script.h" //SCRIPT_ChangeScript
#include "../../../resource/fldmapdata/script/debug_scr_def.h"  //SCRID_DEBUG_COMMON
#include "../../../resource/fldmapdata/script/hiden_def.h"  //SCRID_HIDEN_DIVING
#include "../../../resource/fldmapdata/script/pasokon_def.h"  //SCRID_PC

#include "field/eventwork.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#include "event_wifibattlematch.h"

#include "field/field_comm/intrude_field.h" //PALACE_MAP_LEN

//CGEAR PICTURE
#include "c_gear.naix"
#include "./c_gear/c_gear.h"
#include "./c_gear/event_cgearget.h"
#include "savedata/save_tbl.h"
#include "savedata/c_gear_picture.h"

#include "event_debug_local.h"

#include "event_debug_demo3d.h" // for DEBUG_EVENT_FLDMENU_Demo3DSelect
#include "event_debug_sodateya.h" // for DEBUG_EVENT_DebugMenuSodateya
#include "event_debug_menu_make_egg.h"  // for DEBUG_EVENT_FLDMENU_MakeEgg
#include "event_debug_all_connect_check.h" // for EVENT_DEBUG_AllConnectCheck

#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SJIStoStrcode

#include "event_debug_wifimatch.h"
#include "event_battlerecorder.h"
#include "event_debug_mvpoke.h"
#include "field_bbd_color.h"

#include "gamesystem/pm_weather.h"

#include "debug/debug_mystery_card.h"

#include "bsubway_scr.h"
#include "event_wifi_bsubway.h"
#include "event_bsubway.h"
#include "scrcmd_bsubway.h"

#include "savedata/battle_box_save.h"
#include "event_geonet.h"
#include "app/name_input.h"
#include "waza_tool\wazano_def.h"

#include "savedata/symbol_save.h"
#include "savedata/symbol_save_notwifi.h"
#include "savedata/symbol_save_field.h"
#include "event_gts.h"
#include "src\musical\musical_debug.h"

#include "test/performance.h"

#include "field_camera_debug.h"

#include "../../../resource/fld3d_ci/fldci_id_def.h"  // for FLDCIID_MAX

FS_EXTERN_OVERLAY( d_iwasawa );

//======================================================================
//  global
//======================================================================
int DbgCutinNo = 0;

//======================================================================
//  define
//======================================================================
#define DEBUG_MENU_PANO (14)
#define DEBUG_FONT_PANO (15)

#define D_MENU_CHARSIZE_X (18)    //メニュー横幅
#define D_MENU_CHARSIZE_Y (16)    //メニュー縦幅

#define LINER_CAM_KEY_WAIT    (30)
#define LINER_CAM_KEY_COUNT    (15)

//======================================================================
//  typedef struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static FLDMENUFUNC * DEBUGFLDMENU_InitExPos(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init, u16 list_pos, u16 cursor_pos );
static void DebugMenu_IineCallBack(BMPMENULIST_WORK* lw,u32 param,u8 y);
static u32 DebugMenu_GetQuickJumpIdx( u32 strID );

static GMEVENT_RESULT DebugMenuEvent( GMEVENT *event, int *seq, void *wk );

static BOOL debugMenuCallProc_OpenStartComm( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_OpenStartInvasion( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_MapZoneSelect( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_OpenCommDebugMenu( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_OpenClubMenu( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_OpenGTSNegoMenu( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_CGEARPictureSave( DEBUG_MENU_EVENT_WORK *wk );


static debugMenuCallProc_MapSeasonSelect( DEBUG_MENU_EVENT_WORK *wk );
static debugMenuCallProc_SubscreenSelect( DEBUG_MENU_EVENT_WORK *wk );
static debugMenuCallProc_MusicalSelect( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_EventFlagScript( DEBUG_MENU_EVENT_WORK *now_wk );
static BOOL debugMenuCallProc_ScriptSelect( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_GameEnd( DEBUG_MENU_EVENT_WORK * wk );

static BOOL debugMenuCallProc_ControlCamera( DEBUG_MENU_EVENT_WORK *wk );


static BOOL debugMenuCallProc_MMdlList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_ForceSave( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_ControlLight( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ControlFog( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_WeatherList( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_CaptureList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_FieldAveStress( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_FieldAveStressPalace( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_FieldPosData( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_ControlRtcList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_Naminori( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_DebugMakePoke( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_DebugReWritePoke( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_DebugItem( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_DebugSecretItem( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_DebugPDWItem( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_BoxMax( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_MyItemMax( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_SetBtlBox( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ChangeName( DEBUG_MENU_EVENT_WORK *p_wk );

static BOOL debugMenuCallProc_DebugSkyJump( DEBUG_MENU_EVENT_WORK *p_wk );

static BOOL debugMenuCallProc_ChangePlayerSex( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_WifiGts( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_GDS( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_UITemplate( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Jump( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_NumInput( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ResearchNumInput( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_RecordNumInput( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_Kairiki( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ControlLinerCamera( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ControlDelicateCamera( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_ControlShortCut( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_CreateMusicalShotData( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_BeaconFriendCode( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_WifiBattleMatch( DEBUG_MENU_EVENT_WORK *wk );
static GMEVENT_RESULT debugMenuWazaOshie( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
static BOOL debugMenuCallProc_WazaOshie( DEBUG_MENU_EVENT_WORK *p_wk );


static BOOL debugMenuCallProc_UseMemoryDump( DEBUG_MENU_EVENT_WORK *p_wk );

static BOOL debugMenuCallProc_DebugSake( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_DebugAtlas( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Geonet( DEBUG_MENU_EVENT_WORK * p_wk );
static BOOL debugMenuCallProc_BattleRecorder( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Ananukenohimo( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Anawohoru( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Teleport( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Diving( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Demo3d( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_DebugMvPokemon( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_BBDColor( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_MakeEgg( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_Sodateya( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_EncEffList( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_Cutin( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_DebugMakeUNData( DEBUG_MENU_EVENT_WORK *p_wk );

static BOOL debugMenuCallProc_BSubway( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_GPowerList( DEBUG_MENU_EVENT_WORK *wk );
static GMEVENT_RESULT debugMenuGPowerListEvent(GMEVENT *event, int *seq, void *wk );
static void DEBUG_SetMenuWorkGPower(GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list,
  HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );
static u16 DEBUG_GetGPowerMax( GAMESYS_WORK* gsys, void* cb_work );

static BOOL debugMenuCallProc_FieldSkillList( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardList( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardPoke( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardItem( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardGPower( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardGLiberty( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardEgg( DEBUG_MENU_EVENT_WORK *p_wk );

static BOOL debugMenuCallProc_Zukan( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_DebugZoneJump( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_AllMapCheck( DEBUG_MENU_EVENT_WORK * p_wk );
static BOOL debugMenuCallProc_AllConnectCheck( DEBUG_MENU_EVENT_WORK * p_wk );
static BOOL debugMenuCallProc_RingTone( DEBUG_MENU_EVENT_WORK * p_wk );
static BOOL debugMenuCallProc_PCScript( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_EventPokeCreate( DEBUG_MENU_EVENT_WORK * p_wk );
static BOOL debugMenuCallProc_SymbolPokeCreate( DEBUG_MENU_EVENT_WORK * p_wk );
static BOOL debugMenuCallProc_SymbolPokeList( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_SymbolPokeKeepLargeFull( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_SymbolPokeKeepSmallFull( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_SymbolPokeFreeLargeFull( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_SymbolPokeFreeSmallFull( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_SymbolPokeCountup( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_FadeSpeedChange( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_Musical( DEBUG_MENU_EVENT_WORK * wk );

//======================================================================
//  デバッグメニューリスト
//======================================================================
//--------------------------------------------------------------
/// デバッグメニューリスト
/// データを追加する事でメニューの項目も増えます。
//--------------------------------------------------------------
static const FLDMENUFUNC_LIST DATA_DebugMenuList[] =
{
  { DEBUG_FIELD_TITLE_00, (void*)BMPMENULIST_LABEL },       //○ジャンプ（移動用）
  { DEBUG_FIELD_STR38, debugMenuCallProc_DebugSkyJump },    //空を飛ぶ
  { DEBUG_FIELD_STR43, debugMenuCallProc_Jump },            //ジャンプ（ユニオン等）
  { DEBUG_FIELD_STR05, debugMenuCallProc_MapZoneSelect },   //ゾーン選択ジャンプ
  { DEBUG_FIELD_STR06, debugMenuCallProc_MapSeasonSelect},  //四季ジャンプ
  { DEBUG_FIELD_STR05_b,   debugMenuCallProc_DebugZoneJump }, //ゾーンジャンプ
  { DEBUG_FIELD_STR09, debugMenuCallProc_AllMapCheck }, //オールマップチェック
  { DEBUG_FIELD_STR10, debugMenuCallProc_AllConnectCheck }, //オール接続チェック

  { DEBUG_FIELD_TITLE_02, (void*)BMPMENULIST_LABEL },       //○フィールド
  { DEBUG_FIELD_STRESS_N, debugMenuCallProc_FieldAveStress },    //平均負荷
  { DEBUG_FIELD_STRESS_P, debugMenuCallProc_FieldAveStressPalace },    //パレス平均負荷
  { DEBUG_FIELD_STR17, debugMenuCallProc_FieldPosData },    //座標をみる
  { DEBUG_FIELD_STR02, debugMenuCallProc_ControlLinerCamera },  //カメラ簡単操作
  { DEBUG_FIELD_STR52, debugMenuCallProc_ControlDelicateCamera }, //カメラ全部操作
  { DEBUG_FIELD_EVENT_CONTROL, debugMenuCallProc_EventFlagScript }, //イベント操作
  { DEBUG_FIELD_STR03, debugMenuCallProc_ScriptSelect },    //スクリプト実行
  { DEBUG_FIELD_STR13, debugMenuCallProc_MMdlList },        //モデルリスト
  { DEBUG_FIELD_STR15, debugMenuCallProc_ControlLight },      //ライト
  { DEBUG_FIELD_STR16, debugMenuCallProc_WeatherList },       //てんき
  { DEBUG_FIELD_STR36, debugMenuCallProc_ControlFog },        //フォグ操作
  { DEBUG_FIELD_STR_SUBSCRN, debugMenuCallProc_SubscreenSelect }, //下画面操作
  { DEBUG_FIELD_FSKILL, debugMenuCallProc_FieldSkillList },  //フィールド技（波乗り、怪力等）
  { DEBUG_FIELD_MVPOKE,   debugMenuCallProc_DebugMvPokemon },     //いどうポケモン
  { DEBUG_FIELD_STR62,   debugMenuCallProc_BBDColor },            //ビルボードの色
  { DEBUG_FIELD_ENCEFF, debugMenuCallProc_EncEffList },           //エンカウントエフェクト
  { DEBUG_FIELD_CUTIN, debugMenuCallProc_Cutin },           //カットイン

  { DEBUG_FIELD_TITLE_01, (void*)BMPMENULIST_LABEL },       //○システム
  { DEBUG_FIELD_NUMINPUT, debugMenuCallProc_NumInput },     //数値入力
  { DEBUG_FIELD_RESEARCH, debugMenuCallProc_ResearchNumInput },//すれ違い調査
  { DEBUG_FIELD_RECORD, debugMenuCallProc_RecordNumInput },//レコード改ざん
  { DEBUG_FIELD_STR04, debugMenuCallProc_GameEnd },         //ゲーム終了
  { DEBUG_FIELD_STR60, debugMenuCallProc_ForceSave },       //強制セーブ
  { DEBUG_FIELD_STR53, debugMenuCallProc_UseMemoryDump },   //メモリ状況
  { DEBUG_FIELD_STR22, debugMenuCallProc_ControlRtcList },  //時間
  { DEBUG_FIELD_STR61, debugMenuCallProc_CaptureList },     //キャプチャ
  { DEBUG_FIELD_STR40, debugMenuCallProc_ChangePlayerSex },   //主人公性別変更
  //{ DEBUG_FIELD_STR35, debugMenuCallProc_FadeSpeedChange },   //フェード速度変更

  { DEBUG_FIELD_TITLE_03, (void*)BMPMENULIST_LABEL },       //○データ作成
  { DEBUG_FIELD_STR41, debugMenuCallProc_DebugMakePoke },   //ポケモン作成
  { DEBUG_FIELD_STR65, debugMenuCallProc_DebugReWritePoke },//ポケモン書き換え
  { DEBUG_FIELD_STR32, debugMenuCallProc_DebugItem },       //アイテム作成
  { DEBUG_FIELD_STR33, debugMenuCallProc_DebugSecretItem }, //隠されアイテム作成
  { DEBUG_FIELD_STR34, debugMenuCallProc_DebugPDWItem },    //PDWアイテム作成
  { DEBUG_FIELD_STR37, debugMenuCallProc_BoxMax },          //ボックス最大
  { DEBUG_FIELD_STR39, debugMenuCallProc_MyItemMax },       //アイテム最大
  { DEBUG_FIELD_MAKE_EGG, debugMenuCallProc_MakeEgg },      //タマゴ作成
  { DEBUG_FIELD_SODATEYA, debugMenuCallProc_Sodateya },     //育て屋
  { DEBUG_FIELD_MAKE_UNDATA,   debugMenuCallProc_DebugMakeUNData }, //国連データ作成
  { DEBUG_FIELD_MYSTERY_00, debugMenuCallProc_MakeMysteryCardList },//ふしぎなおくりものカード作成
  { DEBUG_FIELD_STR63, debugMenuCallProc_SetBtlBox },  //不正チェックを通るポケモンを作成
  { DEBUG_FIELD_STR64, debugMenuCallProc_ChangeName },  //主人公名を再設定
  { DEBUG_FIELD_STR67, debugMenuCallProc_EventPokeCreate },  //イベントポケモン作成
  //{ DEBUG_FIELD_STR69, debugMenuCallProc_SymbolPokeCreate },  //シンボルポケモン作成
  { DEBUG_FIELD_STR69, debugMenuCallProc_SymbolPokeList },  //シンボルポケモン作成
  { DEBUG_FIELD_STR71, debugMenuCallProc_Musical },  //ミュージカル

  { DEBUG_FIELD_TITLE_06, (void*)BMPMENULIST_LABEL },       //○つうしん
  { DEBUG_FIELD_STR19, debugMenuCallProc_OpenClubMenu },      //WIFIクラブ
  { DEBUG_FIELD_GPOWER, debugMenuCallProc_GPowerList},      //Gパワー
  { DEBUG_FIELD_C_CHOICE00, debugMenuCallProc_OpenCommDebugMenu },  //通信開始
  { DEBUG_FIELD_STR49, debugMenuCallProc_BeaconFriendCode },  //ともだちコード配信
  { DEBUG_FIELD_STR57, debugMenuCallProc_DebugSake },             //サケ操作
  { DEBUG_FIELD_STR58, debugMenuCallProc_DebugAtlas },            //アトラス操作
  { DEBUG_FIELD_GEONET, debugMenuCallProc_Geonet },         //ジオネット呼び出し

  { DEBUG_FIELD_TITLE_04, (void*)BMPMENULIST_LABEL },       //○アプリ
  { DEBUG_FIELD_STR44, debugMenuCallProc_UITemplate },        //UIテンプレート
  { DEBUG_FIELD_DEMO3D,   debugMenuCallProc_Demo3d },             //３Dデモ

  { DEBUG_FIELD_TITLE_05, (void*)BMPMENULIST_LABEL },       //○スタッフ用
  { DEBUG_FIELD_STR70, debugMenuCallProc_PCScript },        //パソコン呼び出し
  { DEBUG_FIELD_STR47, debugMenu_ControlShortCut },           //Yボタン登録最大
  { DEBUG_FIELD_STR51  , debugMenuCallProc_OpenGTSNegoMenu }, //GTSネゴ
  { DEBUG_FIELD_STR55,   debugMenuCallProc_CGEARPictureSave },//Cギアー写真
  { DEBUG_FIELD_STR42, debugMenuCallProc_WifiGts },           //GTS
  { DEBUG_FIELD_STR48, debugMenuCallProc_GDS },               //GDS
  { DEBUG_FIELD_STR50, debugMenuCallProc_WazaOshie },         //技教え
  { DEBUG_FIELD_STR56, debugMenuCallProc_WifiBattleMatch },   //WIFI世界対戦
  { DEBUG_FIELD_STR59, debugMenuCallProc_BattleRecorder },        //バトルレコーダー
  { DEBUG_FIELD_BSW_00, debugMenuCallProc_BSubway },                //バトルサブウェイ
  { DEBUG_FIELD_ZUKAN_04, debugMenuCallProc_Zukan },            //ずかん
  { DEBUG_FIELD_STR66,  debugMenuCallProc_RingTone },           //着信音
  { DEBUG_FIELD_STR47, debugMenu_CreateMusicalShotData },           //ミュージカルデータ作成
};


//------------------------------------------------------------------------
/*
 *  QUICKSJUMP
 *  上記のリストのSTRIDを書くと、STARTボタンをおしたときに、その位置までカーソルが動きます
 */
//------------------------------------------------------------------------
#if defined DEBUG_ONLY_FOR_toru_nagihashi
  #define QuickJumpStart   DEBUG_FIELD_STR48
#elif defined DEBUG_ONLY_FOR_matsumiya
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17
#elif defined DEBUG_ONLY_FOR_masafumi_saitou
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17
#elif defined DEBUG_ONLY_FOR_suginaka_katsunori
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17
#elif defined DEBUG_ONLY_FOR_mizuguchi_mai || defined DEBUG_ONLY_FOR_mai_ando
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17
#elif defined DEBUG_ONLY_FOR_murakami_naoto
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17
#elif defined DEBUG_ONLY_FOR_ohno
  #define QuickJumpStart   DEBUG_FIELD_STR19
//#elif defined DEBUG_ONLY_FOR_
//  #define QuickJumpStart
#endif

//--------------------------------------------------------------
/// メニューヘッダー
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER DATA_DebugMenuListHeader =
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
  NARC_message_d_field_dat,
  NELEMS(DATA_DebugMenuList),
  DATA_DebugMenuList,
  &DATA_DebugMenuListHeader,
  1, 1, D_MENU_CHARSIZE_X, D_MENU_CHARSIZE_Y,
  NULL,
  NULL,
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

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static FLDMENUFUNC * DebugMenuInitCore(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init, void* cb_work, u16 list_pos, u16 cursor_pos )
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
  msgData = FLDMSGBG_CreateMSGDATA( msgBG, init->msg_arc_id );

  if (init->getMaxFunc) {
    max = init->getMaxFunc( gsys, cb_work );
  } else {
    max = init->max;
  }
  if (init->makeListFunc) {
    listdata = FLDMENUFUNC_CreateListData( max, heapID );
    init->makeListFunc( gsys, listdata, heapID, msgData, cb_work );
  } else {
    listdata = FLDMENUFUNC_CreateMakeListData( init->menulist, max, msgData, heapID );
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
  return DebugMenuInitCore( fieldmap, heapID, init, NULL, 0, 0 );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
FLDMENUFUNC * DEBUGFLDMENU_InitEx(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init, void* cb_work )
{
  return DebugMenuInitCore( fieldmap, heapID, init, cb_work, 0, 0 );
}

static FLDMENUFUNC * DEBUGFLDMENU_InitExPos(
    FIELDMAP_WORK * fieldmap, HEAPID heapID,
    const DEBUG_MENU_INITIALIZER * init, u16 list_pos, u16 cursor_pos )
{
  return DebugMenuInitCore( fieldmap, heapID, init, NULL, list_pos, cursor_pos );
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
          u32 idx = DebugMenu_GetQuickJumpIdx( QuickJumpStart );
          FLDMENUFUNC_DeleteMenu( work->menuFunc );
          work->menuFunc  = DEBUGFLDMENU_InitExPos( work->fieldWork, work->heapID,  &DebugMenuData, idx-4, 4 );
        }
#endif  //QuickJumpStart
#ifdef QuickJumpSelect
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
        {
          u32 idx = DebugMenu_GetQuickJumpIdx( QuickJumpSelect );
          FLDMENUFUNC_DeleteMenu( work->menuFunc );
          work->menuFunc  = DEBUGFLDMENU_InitExPos( work->fieldWork, work->heapID,  &DebugMenuData, idx-4, 4 );
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
    {
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
//  デバッグメニュー呼び出し
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_EventFlagScript( DEBUG_MENU_EVENT_WORK *now_wk )
{
  SCRIPT_ChangeScript( now_wk->gmEvent, SCRID_DEBUG_COMMON, NULL, HEAPID_FIELDMAP );
  return TRUE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　デバッグスクリプト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ScriptSelect( DEBUG_MENU_EVENT_WORK *now_wk )
{
  GMEVENT * new_event = DEBUG_EVENT_FLDMENU_DebugScript( now_wk );
  GMEVENT_ChangeEvent( now_wk->gmEvent, new_event );

  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　通信デバッグ子メニュー
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_GameEnd( DEBUG_MENU_EVENT_WORK * wk )
{
  GMEVENT * new_event = DEBUG_EVENT_GameEnd( wk->gmSys, wk->fieldWork );
  GMEVENT_ChangeEvent( wk->gmEvent, new_event );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　ジャンプ
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Jump( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_jump ), DEBUG_EVENT_FLDMENU_JumpEasy, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  //GMEVENT_ChangeEvent( wk->gmEvent, DEBUG_EVENT_FLDMENU_JumpEasy( wk->gmSys, NULL ) );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　四季マップ間移動
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static debugMenuCallProc_MapSeasonSelect( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_jump ), DEBUG_EVENT_FLDMENU_ChangeSeason, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  //GMEVENT_ChangeEvent( wk->gmEvent, DEBUG_EVENT_FLDMENU_ChangeSeason( wk->gmSys, NULL ) );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　どこでもジャンプ
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MapZoneSelect( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_jump ), DEBUG_EVENT_FLDMENU_JumpAllZone, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  //GMEVENT_ChangeEvent( wk->gmEvent, DEBUG_EVENT_FLDMENU_JumpAllZone( wk->gmSys, NULL ) );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief タマゴさくせい
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_MakeEgg( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_make_egg ), DEBUG_EVENT_FLDMENU_MakeEgg, wk->fieldWork );

  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief 育て屋
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Sodateya( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_sodateya ), DEBUG_EVENT_DebugMenuSodateya, wk->fieldWork );

  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　通信デバッグ子メニュー
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_OpenCommDebugMenu( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  const HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
#if 0 //通信システム変更の為、Fix 2009.09.03(木)
  FIELD_COMM_DEBUG_WORK *work;

  GMEVENT_Change( event,
    FIELD_COMM_DEBUG_CommDebugMenu, FIELD_COMM_DEBUG_GetWorkSize() );
  work = GMEVENT_GetEventWork( event );
  FIELD_COMM_DEBUG_InitWork( heapID , gameSys , fieldWork , event , work );
#else
  GF_ASSERT(0);
#endif

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * WiFiClubメニュー呼びだし
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_OpenClubMenu( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  GMEVENT *next_event;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;

  EVENT_WiFiClubChange(gameSys, fieldWork, event);

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * GTSNegoメニュー呼びだし
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_OpenGTSNegoMenu( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;

  EVENT_GTSNegoChangeDebug(gameSys, fieldWork, event);
  return( TRUE );
}


//#if 0
#if DEBUG_ONLY_FOR_ohno
//--------------------------------------------------------------
/**
 * @brief   デバッグ ポケモンをセーブデータに入れる
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
#include "savedata/dreamworld_data.h"
#include "savedata/mystatus.h"
#include "savedata/mystatus_local.h"
#include "savedata/record.h"
#include "savedata/misc.h"
#include "savedata/undata_update.h"
#include "savedata/wifihistory.h"
static BOOL debugMenuCallProc_CGEARPictureSave( DEBUG_MENU_EVENT_WORK *wk )
{
  int i;
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData(wk->gmSys);

    {//MISC
      MISC* pMisc = SaveData_GetMisc(GAMEDATA_GetSaveControlWork(gamedata));
      MISC_SetGold( pMisc, GFUser_GetPublicRand( 12345 ));
      OS_TPrintf("pMisc gold=%d\n",MISC_GetGold(pMisc));
    }

    {//レコード
      RECORD* pRec = GAMEDATA_GetRecordPtr(GAMESYSTEM_GetGameData(wk->gmSys));

      RECORD_Set(pRec, RECID_FISHING_SUCCESS, GFUser_GetPublicRand( 12345 ));
      RECORD_Set(pRec, RECID_WIFI_TRADE, GFUser_GetPublicRand( 12345 ));
      RECORD_Set(pRec, RECID_COMM_TRADE, GFUser_GetPublicRand( 12345 ));
      RECORD_Set(pRec, RECID_IRC_TRADE, GFUser_GetPublicRand( 12345 ));
      RECORD_Set(pRec, RECID_CAPTURE_POKE, GFUser_GetPublicRand( 12345 ));
      OS_TPrintf("RECID_FISHING_SUCCESS=%d\n",RECORD_Get(pRec,RECID_FISHING_SUCCESS));
      OS_TPrintf("RECID_CAPTURE_POKE=%d\n",RECORD_Get(pRec,RECID_CAPTURE_POKE));
      OS_TPrintf("RECID_WIFI_TRADE=%d\n",RECORD_Get(pRec,RECID_WIFI_TRADE));
      OS_TPrintf("RECID_COMM_TRADE=%d\n",RECORD_Get(pRec,RECID_COMM_TRADE));
      OS_TPrintf("RECID_IRC_TRADE=%d\n",RECORD_Get(pRec,RECID_IRC_TRADE));

    }
    {//国連
      WIFI_HISTORY* pHis = SaveData_GetWifiHistory(GAMEDATA_GetSaveControlWork(gamedata));
      for(i=0;i<20;i++){
        UNITEDNATIONS_SAVE add_data;
        MYSTATUS aMy;
        aMy.profileID= GFUser_GetPublicRand(3333);
        aMy.nation = i+1;
        MyStatus_Copy( &aMy , &add_data.aMyStatus);
        add_data.recvPokemon = 1;  //貰ったポケモン
        add_data.sendPokemon = 100;  //あげたポケモン
        add_data.favorite = 1;   //趣味
        add_data.countryCount = 2;  //交換した国の回数
        add_data.nature = 1;   //性格
        UNDATAUP_Update(pHis, &add_data);
      }
      for(i=0;i<20;i++){
        MYSTATUS* pMy = WIFIHISTORY_GetUnMyStatus(pHis, i);
        OS_TPrintf("GTSID = %d\n",pMy->profileID);
      }
    }
  }

 // GMEVENT_ChangeEvent( wk->gmEvent, DEBUG_EVENT_FLDMENU_JumpEasy( wk->gmSys, wk->heapID ) );
  return( FALSE );
}


//--------------------------------------------------------------
/**
 * @brief     CGearの絵をROMから読み出しセーブ領域に入れる
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval    BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
//#endif
//#if 1
#else

static BOOL debugMenuCallProc_CGEARPictureSave( DEBUG_MENU_EVENT_WORK *wk )
{
  u16 crc=0;
  ARCHANDLE* p_handle;
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gameSys));

  NNSG2dCharacterData* charData;
  NNSG2dPaletteData* palData;
  void* pArc;
  u8* pCGearWork = GFL_HEAP_AllocMemory(HEAPID_FIELDMAP,SAVESIZE_EXTRA_CGEAR_PICTURE);
  CGEAR_PICTURE_SAVEDATA* pPic=(CGEAR_PICTURE_SAVEDATA*)pCGearWork;

  SaveControl_Extra_LoadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE, HEAPID_FIELDMAP,
                             pCGearWork,SAVESIZE_EXTRA_CGEAR_PICTURE);

  p_handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, HEAPID_FIELDMAP );
  pArc = GFL_ARCHDL_UTIL_Load( p_handle, NARC_c_gear_decal_NCGR, FALSE, HEAPID_FIELDMAP);
  if( NNS_G2dGetUnpackedBGCharacterData( pArc, &charData ) ){
    GFL_STD_MemCopy(charData->pRawData, pPic->picture, CGEAR_DECAL_SIZE_MAX);
  }
  GFL_HEAP_FreeMemory(pArc);

  pArc = GFL_ARCHDL_UTIL_Load( p_handle, NARC_c_gear_decal_NCLR, FALSE, HEAPID_FIELDMAP);
  if( NNS_G2dGetUnpackedPaletteData( pArc, &palData ) ){
    GFL_STD_MemCopy(palData->pRawData, pPic->palette, CGEAR_PICTURTE_PAL_SIZE);
  }
  GFL_HEAP_FreeMemory(pArc);

  pArc = GFL_ARCHDL_UTIL_Load( p_handle, NARC_c_gear_decal_NCLR, FALSE, HEAPID_FIELDMAP);
  if( NNS_G2dGetUnpackedPaletteData( pArc, &palData ) ){
    GFL_STD_MemCopy(palData->pRawData, pPic->palette, CGEAR_PICTURTE_PAL_SIZE);
  }
  GFL_HEAP_FreeMemory(pArc);


  GFL_ARC_CloseDataHandle( p_handle );

  crc = GFL_STD_CrcCalc( pPic, CGEAR_PICTURTE_ALL_SIZE );

  GAMEDATA_ExtraSaveAsyncStart(GAMESYSTEM_GetGameData(gameSys),SAVE_EXTRA_ID_CGEAR_PICUTRE);
  while(1){
    if(SAVE_RESULT_OK==GAMEDATA_ExtraSaveAsyncMain(GAMESYSTEM_GetGameData(gameSys),SAVE_EXTRA_ID_CGEAR_PICUTRE)){
      break;
    }
    OS_WaitIrq(TRUE, OS_IE_V_BLANK);
  }
  SaveControl_Extra_UnloadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE);
  GFL_HEAP_FreeMemory(pCGearWork);
  CGEAR_SV_SetCGearONOFF(CGEAR_SV_GetCGearSaveData(pSave), TRUE);  //CGEAR表示有効
  CGEAR_SV_SetCGearPictureONOFF(CGEAR_SV_GetCGearSaveData(pSave),TRUE);  //CGEARデカール有効
  CGEAR_SV_SetCGearPictureCRC(CGEAR_SV_GetCGearSaveData(pSave),crc);  //CGEARデカール有効

  return( FALSE );
}
#endif
//#endif

#if 0    //図鑑テスト

#include "include/savedata/zukan_wp_savedata.h"
// セーブデータ
typedef struct  {
  // カスタムグラフィックキャラ
  u8  customChar[ZUKANWP_SAVEDATA_CHAR_SIZE];
  // カスタムグラフィックパレット
  u16 customPalette[ZUKANWP_SAVEDATA_PAL_SIZE];
  // データ有無フラグ
  BOOL  flg;
} TESTZUKAN_DATA;



static BOOL debugMenuCallProc_CGEARPictureSave( DEBUG_MENU_EVENT_WORK *wk )
{
  int size,i;
  u16 crc=0;
  ARCHANDLE* p_handle;
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gameSys));
  NNSG2dCharacterData* charData;
  NNSG2dPaletteData* palData;
  NNSG2dScreenData* pScrData;
  void* pArc;
  u8* pCGearWork = GFL_HEAP_AllocMemory(HEAPID_FIELDMAP,SAVESIZE_EXTRA_ZUKAN_WALLPAPER);
  TESTZUKAN_DATA* pPic=(TESTZUKAN_DATA*)pCGearWork;

  SaveControl_Extra_LoadWork(pSave, SAVE_EXTRA_ID_ZUKAN_WALLPAPER, HEAPID_FIELDMAP,
                             pCGearWork,SAVESIZE_EXTRA_ZUKAN_WALLPAPER);

  pPic->flg=TRUE;

  p_handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, HEAPID_FIELDMAP );

  pArc = GFL_ARCHDL_UTIL_Load( p_handle, NARC_c_gear_zukantest_NCGR, FALSE, HEAPID_FIELDMAP);
  if( NNS_G2dGetUnpackedBGCharacterData( pArc, &charData ) ){
    GFL_STD_MemCopy(charData->pRawData, pPic->customChar, ZUKANWP_SAVEDATA_CHAR_SIZE);
  }
  GFL_HEAP_FreeMemory(pArc);

  pArc = GFL_ARCHDL_UTIL_Load( p_handle, NARC_c_gear_zukantest_NCLR, FALSE, HEAPID_FIELDMAP);
  if( NNS_G2dGetUnpackedPaletteData( pArc, &palData ) ){
    u16 * pal = (u16 *)palData->pRawData;
    for(i=0;i<16;i++){
      GFL_STD_MemCopy( &pal[16*i], &pPic->customPalette[16*i], 32);
    }
  }
  GFL_HEAP_FreeMemory(pArc);

  GFL_ARC_CloseDataHandle( p_handle );

  size = ZUKANWP_SAVEDATA_CHAR_SIZE+ZUKANWP_SAVEDATA_PAL_SIZE*2;
  crc = GFL_STD_CrcCalc( pPic, size );

  {
    int i;
    u8* pU8 = (u8*)pPic;
    for(i=0;i<size;i++){
      OS_TPrintf("0x%x ",pU8[i]);
      if((i%16)==15){
        OS_TPrintf("\n");
      }
    }
  }

  OS_TPrintf("-----%x \n",crc);

  SaveControl_Extra_SaveAsyncInit(pSave,SAVE_EXTRA_ID_ZUKAN_WALLPAPER);
  while(1){
    if(SAVE_RESULT_OK==SaveControl_Extra_SaveAsyncMain(pSave,SAVE_EXTRA_ID_ZUKAN_WALLPAPER)){
      break;
    }
    OS_WaitIrq(TRUE, OS_IE_V_BLANK);
  }
  SaveControl_Extra_UnloadWork(pSave, SAVE_EXTRA_ID_ZUKAN_WALLPAPER);


  GFL_HEAP_FreeMemory(pCGearWork);

  return( FALSE );
}
#endif


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し 数値入力
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_NumInput( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
    FS_OVERLAY_ID( d_numinput ), DEBUG_EVENT_FLDMENU_NumInput, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し 数値入力
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ResearchNumInput( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
    FS_OVERLAY_ID( d_numinput_research ), DEBUG_EVENT_FLDMENU_ResearchNumInput, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し レコード改ざん
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_RecordNumInput( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
    FS_OVERLAY_ID( d_numinput_record ), DEBUG_EVENT_FLDMENU_RecordNumInput, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE );
}


//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
typedef struct {
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  FIELD_SUBSCREEN_WORK * subscreen;
}DEBUG_MENU_EVENT_SUBSCRN_SELECT_WORK, DMESSWORK;

//--------------------------------------------------------------
//  proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSubscreenSelectEvent(
    GMEVENT *event, int *seq, void *wk );
static void setupTouchCameraSubscreen(DMESSWORK * dmess);
static void setupSoundViewerSubscreen(DMESSWORK * dmess);
static void setupNormalSubscreen(DMESSWORK * dmess);
static void setupTopMenuSubscreen(DMESSWORK * dmess);
static void setupUnionSubscreen(DMESSWORK * dmess);
static void setupDebugLightSubscreen(DMESSWORK * dmess);
static void setupCGearFirstSubscreen(DMESSWORK * dmess);
static void setupNoneSubscreen(DMESSWORK * dmess);


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　四季マップ間移動
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static debugMenuCallProc_SubscreenSelect( DEBUG_MENU_EVENT_WORK *wk )
{
  HEAPID heapID = wk->heapID;
  GMEVENT *event = wk->gmEvent;
  GAMESYS_WORK *gsys = wk->gmSys;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DMESSWORK *work;

    GMEVENT_Change( event,
      debugMenuSubscreenSelectEvent, sizeof(DMESSWORK) );
    work = GMEVENT_GetEventWork( event );
    GFL_STD_MemClear( work, sizeof(DMESSWORK) );

    work->gmSys = gsys;
    work->gmEvent = event;
    work->heapID = heapID;
    work->fieldWork = fieldWork;
    work->subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
    return( TRUE );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static const FLDMENUFUNC_LIST DATA_SubcreenMenuList[] =
{
  { DEBUG_FIELD_STR_SUBSCRN01, (void*)setupTouchCameraSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN02, (void*)setupSoundViewerSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN03, (void*)setupNormalSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN04, (void*)setupTopMenuSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN01, (void*)setupDebugLightSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN05, (void*)setupUnionSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN06, (void*)setupCGearFirstSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN07, (void*)setupNoneSubscreen },
};

static const DEBUG_MENU_INITIALIZER DebugSubscreenSelectData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_SubcreenMenuList),
  DATA_SubcreenMenuList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * イベント：した画面ジャンプ
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSubscreenSelectEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DMESSWORK *work = wk;

  switch( *seq )
  {
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugSubscreenSelectData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }else{
        typedef void (* CHANGE_FUNC)(DMESSWORK*);
        CHANGE_FUNC func = (CHANGE_FUNC)ret;
        func(work);
        //FIELD_SUBSCREEN_ChangeForce(FIELDMAP_GetFieldSubscreenWork(work->fieldWork), ret);
        return( GMEVENT_RES_FINISH );
      }
    }
    break;
  }

  return GMEVENT_RES_CONTINUE ;
}

//--------------------------------------------------------------
/**
 * @brief サブスクリーンのカメラ操作に同期させる
 *
 * @todo  カメラをバインドしてから、どのように切り離すか？を確定させる
 */
//--------------------------------------------------------------
static void setupTouchCameraSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_DEBUG_TOUCHCAMERA);
  {
    void * inner_work;
    FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(dmess->fieldWork);
    inner_work = FIELD_SUBSCREEN_DEBUG_GetControl(dmess->subscreen);
    FIELD_CAMERA_DEBUG_BindSubScreen(cam, inner_work, FIELD_CAMERA_DEBUG_BIND_CAMERA_POS, dmess->heapID);
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupSoundViewerSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_DEBUG_SOUNDVIEWER);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupNormalSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_NORMAL);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupTopMenuSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_TOPMENU);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupUnionSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_UNION);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupDebugLightSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_DEBUG_LIGHT);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupCGearFirstSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_CGEARFIRST);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupNoneSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_NOGEAR);
}


//======================================================================
//  デバッグメニュー　カメラ操作
//======================================================================
//--------------------------------------------------------------
/// DEBUG_CTL_LINERCAMERA_WORK 線形カメラ操作ワーク1
//--------------------------------------------------------------
typedef struct
{
  int vanish;
  GAMESYS_WORK *gsys;
  GMEVENT *event;
  HEAPID heapID;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *pMsgBG;
  FLDMSGWIN *pMsgWin;
  STRBUF *pStrBuf;
  u16 Wait;
  u16 Count;
}DEBUG_CTL_LINERCAMERA_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLinerCamera(
    GMEVENT *event, int *seq, void *wk );
static void DampCameraInfo(FIELD_CAMERA * cam);
static BOOL LinerCamKeyContCtl(DEBUG_CTL_LINERCAMERA_WORK *work, const fx32 inAddVal, fx32 *outVal);


//======================================================================
//  デバッグメニュー　動作モデル一覧
//======================================================================
//--------------------------------------------------------------
/// DEBUG_MMDL_LIST_EVENT_WORK 動作モデルリスト処理用ワーク
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
  MMDLSYS *fldmmdlsys;

  u16 obj_code;
  u8 padding[2];
  MMDL *fmmdl;
}DEBUG_MMDLLIST_EVENT_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMMdlListEvent(
    GMEVENT *event, int *seq, void *wk );
static u16 * DEBUG_GetOBJCodeStrBuf( HEAPID heapID, u16 code );
static void DEBUG_SetMenuWorkMMdlList(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work );

/// 動作モデルリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_MMdlList =
{
  1,    //リスト項目数
  9,   //表示最大項目数
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

static const DEBUG_MENU_INITIALIZER DebugMMdlListData = {
  NARC_message_d_field_dat,
  OBJCODETOTAL,
  NULL,
  &DATA_DebugMenuList_MMdlList,
  1, 1, 11, 16,
  DEBUG_SetMenuWorkMMdlList,
  NULL,
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　動作モデル一覧
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MMdlList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_MMDLLIST_EVENT_WORK *work;

  GMEVENT_Change( event,
    debugMenuMMdlListEvent, sizeof(DEBUG_MMDLLIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MMDLLIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    work->fldmmdlsys = GAMEDATA_GetMMdlSys( gdata );
  }

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：動作モデル一覧
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMMdlListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MMDLLIST_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugMMdlListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      if( ret == FLDMENUFUNC_CANCEL || ret == NONDRAW ){ //キャンセル
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        return( GMEVENT_RES_FINISH );
      }

      work->obj_code = ret;

      {
        MMDL *jiki;
        MMDL_HEADER head = {
          0,  ///<識別ID
          0,  ///<表示するOBJコード
          MV_DMY, ///<動作コード
          0,  ///<イベントタイプ
          0,  ///<イベントフラグ
          0,  ///<イベントID
          0,  ///<指定方向
          0,  ///<指定パラメタ 0
          0,  ///<指定パラメタ 1
          0,  ///<指定パラメタ 2
          4,  ///<X方向移動制限
          4,  ///<Z方向移動制限
          0,  ///<グリッドX
          0,  ///<グリッドZ
          0,  ///<Y値 fx32型
        };

        jiki = MMDLSYS_SearchOBJID(
          work->fldmmdlsys, MMDL_ID_PLAYER );

        head.id = 250;

        MMDLHEADER_SetGridPos( &head,
            MMDL_GetGridPosX( jiki ) + 2,
            MMDL_GetGridPosZ( jiki ),
            MMDL_GetVectorPosY( jiki ) );

        head.obj_code = work->obj_code;

        work->fmmdl = MMDLSYS_AddMMdl(
          work->fldmmdlsys, &head, 0 );
      }

      (*seq)++;
      break;
    case 2:
      {
        MMDL *mmdl = work->fmmdl;
        int key_cont = GFL_UI_KEY_GetCont();
        u16 code = MMDL_GetOBJCode( mmdl );
        u16 ac = ACMD_NOT;

        if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
          if( (key_cont & PAD_BUTTON_B) ){
            MMDL_Delete( work->fmmdl );
            (*seq) = 1;
            return GMEVENT_RES_CONTINUE;
          }

          if( code == SHIN_A || code == MU_A ){
            if( key_cont & PAD_KEY_UP ){
              ac = AC_SHIN_MU_FLY_L;
            }else if( key_cont & PAD_KEY_DOWN ){
              ac = AC_DIR_D;
            }else if( key_cont & PAD_KEY_LEFT ){
              ac = AC_SHIN_MU_GUTARI;
            }else if( key_cont & PAD_KEY_RIGHT ){
              ac = AC_SHIN_MU_FLY_UPPER;
            }

            if( ac != ACMD_NOT ){
              VecFx32 offs = {0,0,0};
              MMDL_SetVectorDrawOffsetPos( mmdl, &offs );
            }
          }else if( code == SHIN_B || code == MU_B ){
            if( key_cont & PAD_KEY_UP ){
              ac = AC_DIR_U;
            }else if( key_cont & PAD_KEY_DOWN ){
              ac = AC_DIR_D;
            }else if( key_cont & PAD_KEY_LEFT ){
              ac = AC_SHIN_MU_HOERU;
            }else if( key_cont & PAD_KEY_RIGHT ){
              ac = AC_SHIN_MU_TURN;
            }
          }else{
            u16 ac_act_u = ACMD_NOT;
            u16 ac_act_d = ACMD_NOT;
            u16 ac_act_l = ACMD_NOT;
            u16 ac_act_r = ACMD_NOT;

            const OBJCODE_PARAM *param =
                MMDL_GetOBJCodeParam( mmdl, code );

            if( key_cont & PAD_BUTTON_A ){
              switch( param->anm_id ){
              case MMDL_BLACT_ANMTBLNO_PCWOMAN:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_PC_BOW;
                break;
              case MMDL_BLACT_ANMTBLNO_RIVEL:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_RIVEL_ACT0;
                break;
              case MMDL_BLACT_ANMTBLNO_SUPPORT:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_SUPPORT_ACT0;
                break;
              case MMDL_BLACT_ANMTBLNO_BOSS_N:
                ac_act_u = ac_act_d = AC_N_ACT0;
                ac_act_l = ac_act_r = AC_N_ACT1;
                break;
              case MMDL_BLACT_ANMTBLNO_SAGE1:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_SAGE1_ACT0;
                break;
              case MMDL_BLACT_ANMTBLNO_NINJA:
                ac_act_u = ac_act_d = AC_NINJA_ACT0;
                ac_act_l = ac_act_r = AC_NINJA_ACT1;
                break;
              case MMDL_BLACT_ANMTBLNO_CHAMPION:
                ac_act_u = ac_act_d = ac_act_l = ac_act_r = AC_CHAMPION_ACT0;
                break;
              }
            }

            if( key_cont & PAD_KEY_UP ){
              if( ac_act_u != ACMD_NOT ){
                ac = ac_act_u;
              }else{
                ac = AC_STAY_WALK_U_32F;
              }
            }else if( key_cont & PAD_KEY_DOWN ){
              if( ac_act_d != ACMD_NOT ){
                ac = ac_act_d;
              }else{
                ac = AC_STAY_WALK_D_32F;
              }
            }else if( key_cont & PAD_KEY_LEFT ){
              if( ac_act_l != ACMD_NOT ){
                ac = ac_act_l;
              }else{
                ac = AC_STAY_WALK_L_32F;
              }
            }else if( key_cont & PAD_KEY_RIGHT ){
              if( ac_act_r != ACMD_NOT ){
                ac = ac_act_r;
              }else{
                ac = AC_STAY_WALK_R_32F;
              }
            }
          }

          if( ac != ACMD_NOT ){
            MMDL_SetAcmd( mmdl, ac );
          }
        }
      }
      break;
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * 動作モデルOBJコード->STRCODE
 * @param heapID  文字列バッファ確保用ヒープID
 * @param code  文字列を取得したいOBJコード
 * @retval  u16*  文字列が格納されたu16*(開放が必要
 */
//--------------------------------------------------------------
static u16 * DEBUG_GetOBJCodeStrBuf( HEAPID heapID, u16 code )
{
  int i;
  u16 utf16,utf16_eom;
  u16 *pStrBuf;
  u8 *name8;

  pStrBuf = GFL_HEAP_AllocClearMemory( heapID,
      sizeof(u16)*DEBUG_OBJCODE_STR_LENGTH );
  name8 = DEBUG_MMDL_GetOBJCodeString( code, heapID );
  //DEBUG_ConvertAsciiToUTF16( name8, DEBUG_OBJCODE_STR_LENGTH, pStrBuf );
  DEB_STR_CONV_SjisToStrcode( (const char *)name8, pStrBuf, DEBUG_OBJCODE_STR_LENGTH );
  GFL_HEAP_FreeMemory( name8 );

  return( pStrBuf );
}

//--------------------------------------------------------------
/**
 * 動作モデルリスト用BMP_MENULIST_DATAセット
 * @param list  セット先BMP_MENULIST_DATA
 * @param heapID  文字列バッファ確保用HEAPID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkMMdlList(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  u16 *str;
  int id,max;
  STRBUF *strBuf = GFL_STR_CreateBuffer( DEBUG_OBJCODE_STR_LENGTH, heapID );

  for( id = OBJCODESTART_BBD; id < OBJCODEEND_BBD; id++ ){
    GFL_STR_ClearBuffer( strBuf );
    str = DEBUG_GetOBJCodeStrBuf( heapID, id );
    GFL_STR_SetStringCode( strBuf, str );
    GFL_HEAP_FreeMemory( str );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }

  for( id = OBJCODESTART_TPOKE; id < OBJCODEEND_TPOKE; id++ ){
    GFL_STR_ClearBuffer( strBuf );
    str = DEBUG_GetOBJCodeStrBuf( heapID, id );
    GFL_STR_SetStringCode( strBuf, str );
    GFL_HEAP_FreeMemory( str );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }

  for( id = OBJCODESTART_MDL; id < OBJCODEEND_MDL; id++ ){
    GFL_STR_ClearBuffer( strBuf );
    str = DEBUG_GetOBJCodeStrBuf( heapID, id );
    GFL_STR_SetStringCode( strBuf, str );
    GFL_HEAP_FreeMemory( str );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }

  GFL_HEAP_FreeMemory( strBuf );
}

//======================================================================
//  デバッグメニュー　ライト操作
//======================================================================

//--------------------------------------------------------------
/// DEBUG_CTLIGHT_WORK ライト操作ワーク
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  GMEVENT *event;
  HEAPID heapID;
  FIELDMAP_WORK *fieldWork;

  GFL_BMPWIN* p_win;
}DEBUG_CTLLIGHT_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLight(
    GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　カメラ操作
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ControlLight( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_CTLLIGHT_WORK *work;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;

  GMEVENT_Change( event, debugMenuControlLight, sizeof(DEBUG_CTLLIGHT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_CTLLIGHT_WORK) );

  work->gsys = gsys;
  work->event = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}


//--------------------------------------------------------------
/**
 * イベント：カメラ操作
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLight(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_CTLLIGHT_WORK *work = wk;
  FIELD_LIGHT* p_light;

  // ライト取得
  p_light = FIELDMAP_GetFieldLight( work->fieldWork );

  switch( (*seq) ){
  case 0:
    // ライト管理開始
    FIELD_LIGHT_DEBUG_Init( p_light, work->heapID );

    // インフォーバーの非表示
    FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(work->fieldWork));
    GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

    // ビットマップウィンドウ初期化
    {
      static const GFL_BG_BGCNT_HEADER header_sub3 = {
        0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
        GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
      };

      GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
      GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
      GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

      // パレット情報を転送
      GFL_ARC_UTIL_TransVramPalette(
        ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, work->heapID );

      // ビットマップウィンドウを作成
      work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
        1, 1, 30, 22,
        FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_B );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
      GFL_BMPWIN_MakeScreen( work->p_win );
      GFL_BMPWIN_TransVramCharacter( work->p_win );
      GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );

      // ウィンドウ
      BmpWinFrame_GraphicSet( FIELD_SUBSCREEN_BGPLANE, 1, 15, 0, work->heapID );
      BmpWinFrame_Write( work->p_win, TRUE, 1, 15 );
    }

    FIELD_LIGHT_DEBUG_PrintData( p_light, work->p_win );

    (*seq)++;
  case 1:
    // ライト管理メイン
    FIELD_LIGHT_DEBUG_Control( p_light );
    FIELD_LIGHT_DEBUG_PrintData( p_light, work->p_win );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      (*seq)++;
    }
    break;
  case 2:
    // ライト管理終了
    FIELD_LIGHT_DEBUG_Exit( p_light );

    // ビットマップウィンドウ破棄
    {
      GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
      GFL_BMPWIN_Delete( work->p_win );
    }

    // インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(work->fieldWork,
        FIELD_SUBSCREEN_Init( work->heapID, work->fieldWork, FIELD_SUBSCREEN_NORMAL ));

    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}


//======================================================================
//  デバッグメニュー　フォグ操作
//======================================================================

//--------------------------------------------------------------
/// DEBUG_CTLFOG_WORK フォグ操作ワーク
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  GMEVENT *event;
  HEAPID heapID;
  FIELDMAP_WORK *fieldWork;

  GFL_BMPWIN* p_win;

}DEBUG_CTLFOG_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlFog(
    GMEVENT *event, int *seq, void *wk );

static BOOL debugMenuCallProc_ControlFog( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_CTLFOG_WORK *work;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;

  GMEVENT_Change( event, debugMenuControlFog, sizeof(DEBUG_CTLFOG_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_CTLFOG_WORK) );

  work->gsys = gsys;
  work->event = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}


//--------------------------------------------------------------
/**
 * イベント：ふぉぐ操作
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlFog(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_CTLFOG_WORK *work = wk;
  FIELD_FOG_WORK* p_fog;

  // フォグ取得
  p_fog = FIELDMAP_GetFieldFog( work->fieldWork );

  switch( (*seq) ){
  case 0:

    // インフォーバーの非表示
    FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(work->fieldWork));
    GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

    // ビットマップウィンドウ初期化
    {
      static const GFL_BG_BGCNT_HEADER header_sub3 = {
        0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
        GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
      };

      GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
      GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
      GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

      // パレット情報を転送
      GFL_ARC_UTIL_TransVramPalette(
        ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, work->heapID );

      // ビットマップウィンドウを作成
      work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
        1, 1, 30, 22,
        FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_B );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
      GFL_BMPWIN_MakeScreen( work->p_win );
      GFL_BMPWIN_TransVramCharacter( work->p_win );
      GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );

      // ウィンドウ
      BmpWinFrame_GraphicSet( FIELD_SUBSCREEN_BGPLANE, 1, 15, 0, work->heapID );
      BmpWinFrame_Write( work->p_win, TRUE, 1, 15 );
    }

    FIELD_FOG_DEBUG_Init( p_fog, work->heapID );


    (*seq)++;
  case 1:
    // フォグ管理メイン
    FIELD_FOG_DEBUG_Control( p_fog );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
    FIELD_FOG_DEBUG_PrintData( p_fog, work->p_win );
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      (*seq)++;
    }
    break;
  case 2:

    // ビットマップウィンドウ破棄
    {
      GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
      GFL_BMPWIN_Delete( work->p_win );
    }

    FIELD_FOG_DEBUG_Exit( p_fog );


    // インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(work->fieldWork,
        FIELD_SUBSCREEN_Init( work->heapID, work->fieldWork, FIELD_SUBSCREEN_NORMAL ));

    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}


//======================================================================
//  デバッグメニュー　てんきリスト
//======================================================================
//--------------------------------------------------------------
/// DEBUG_WEATERLIST_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DEBUG_WEATERLIST_EVENT_WORK;


//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuWeatherListEvent(
    GMEVENT *event, int *seq, void *work );

///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_WeatherList =
{
  1,    //リスト項目数
  6,    //表示最大項目数
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

///てんきメニューリスト
static const FLDMENUFUNC_LIST DATA_WeatherMenuList[] =
{
  { D_TOMOYA_WEATEHR00, (void*)WEATHER_NO_SUNNY },
  { D_TOMOYA_WEATEHR01, (void*)WEATHER_NO_SNOW },
  { D_TOMOYA_WEATEHR02, (void*)WEATHER_NO_RAIN },
  { D_TOMOYA_WEATEHR03, (void*)WEATHER_NO_STORMHIGH },
  { D_TOMOYA_WEATEHR06, (void*)WEATHER_NO_SNOWSTORM },
  { D_TOMOYA_WEATEHR07, (void*)WEATHER_NO_ARARE },
  { D_TOMOYA_WEATEHR09, (void*)WEATHER_NO_RAIKAMI },
  { D_TOMOYA_WEATEHR10, (void*)WEATHER_NO_KAZAKAMI },
  { D_TOMOYA_WEATEHR11, (void*)WEATHER_NO_DIAMONDDUST },
  { D_TOMOYA_WEATEHR12, (void*)WEATHER_NO_MIST },

  /*  未使用
  { D_TOMOYA_WEATEHR13, (void*)WEATHER_NO_EVENING_RAIN },
  { D_TOMOYA_WEATEHR05, (void*)WEATHER_NO_SPARK },
  { D_TOMOYA_WEATEHR08, (void*)WEATHER_NO_MIRAGE },
  */
};
#define DEBUG_WEATHERLIST_LIST_MAX  ( NELEMS(DATA_WeatherMenuList) )

static const DEBUG_MENU_INITIALIZER DebugWeatherMenuListData = {
  NARC_message_d_tomoya_dat,
  NELEMS(DATA_WeatherMenuList),
  DATA_WeatherMenuList,
  &DATA_DebugMenuList_WeatherList,
  1, 1, 12, 11,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　天気リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_WeatherList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_WEATERLIST_EVENT_WORK *work;

  GMEVENT_Change( event,
    debugMenuWeatherListEvent, sizeof(DEBUG_WEATERLIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_WEATERLIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：てんきリスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuWeatherListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_WEATERLIST_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugWeatherMenuListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret != FLDMENUFUNC_CANCEL ){  //決定
        GAMEDATA* gdata = GAMESYSTEM_GetGameData( work->gmSys );
        FIELD_WEATHER_Change( FIELDMAP_GetFieldWeather( work->fieldWork ), ret );
        GAMEDATA_SetWeatherNo( gdata, ret );
      }

      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  デバッグメニュー　平均負荷
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　平均負荷計測開始
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_FieldAveStress( DEBUG_MENU_EVENT_WORK *wk )
{
  //平均負荷通常
  DEBUG_PerformanceSetAveTest(FALSE);
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　パレス平均負荷計測開始
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_FieldAveStressPalace( DEBUG_MENU_EVENT_WORK *wk )
{
  //平均負荷パレス
  DEBUG_PerformanceSetAveTest(TRUE);
  return( FALSE );
}

//======================================================================
//  デバッグメニュー　位置情報
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　フィールド位置情報
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_FieldPosData( DEBUG_MENU_EVENT_WORK *wk )
{
  FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( wk->fieldWork );
  FIELD_DEBUG_SetPosPrint( debug );

  //描画コールバックのセット
  FIELD_DEBUG_SetDrawCallBackFunc(TRUE);

  return( FALSE );
}

//======================================================================
/// デバックメニュー　時間
//======================================================================
//--------------------------------------------------------------
/// DEBUG_CONTROL_TIME_LIST_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DEBUG_CONTROL_TIME_LIST_EVENT_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlTimeListEvent(
    GMEVENT *event, int *seq, void *work );


//--------------------------------------------------------------
/// data
//--------------------------------------------------------------
/// 時間管理タイプ数
enum{
  CONT_TIME_TYPE_NORMAL,
  CONT_TIME_TYPE_8HOUR,
  CONT_TIME_TYPE_12HOUR,
  CONT_TIME_TYPE_18HOUR,
  CONT_TIME_TYPE_22HOUR,
  CONT_TIME_TYPE_2HOUR,
  CONT_TIME_TYPE_600RATE,
  CONT_TIME_TYPE_60RATE,

  CONT_TIME_TYPE_NUM,
} ;


///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_ContTimeList =
{
  1,    //リスト項目数
  CONT_TIME_TYPE_NUM,   //表示最大項目数
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

///テストカメラメニューリスト
static const FLDMENUFUNC_LIST DATA_ControlTimeMenuList[CONT_TIME_TYPE_NUM] =
{
  { DEBUG_FIELD_STR30, (void*)CONT_TIME_TYPE_NORMAL,  },
  { DEBUG_FIELD_STR23, (void*)CONT_TIME_TYPE_8HOUR,   },
  { DEBUG_FIELD_STR24, (void*)CONT_TIME_TYPE_12HOUR,  },
  { DEBUG_FIELD_STR25, (void*)CONT_TIME_TYPE_18HOUR,  },
  { DEBUG_FIELD_STR26, (void*)CONT_TIME_TYPE_22HOUR,  },
  { DEBUG_FIELD_STR27, (void*)CONT_TIME_TYPE_2HOUR,   },
  { DEBUG_FIELD_STR28, (void*)CONT_TIME_TYPE_600RATE, },
  { DEBUG_FIELD_STR29, (void*)CONT_TIME_TYPE_60RATE,  },
};

static const DEBUG_MENU_INITIALIZER DebugControlTimeMenuListData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_ControlTimeMenuList),
  DATA_ControlTimeMenuList,
  &DATA_DebugMenuList_ContTimeList,
  1, 1, 9, 13,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　テストカメラリスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ControlRtcList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_CONTROL_TIME_LIST_EVENT_WORK *work;

  GMEVENT_Change( event,
    debugMenuControlTimeListEvent, sizeof(DEBUG_CONTROL_TIME_LIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_CONTROL_TIME_LIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：時間操作リスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlTimeListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_CONTROL_TIME_LIST_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugControlTimeMenuListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      if( ret != FLDMENUFUNC_CANCEL ){  //決定
        switch( ret ){
        case CONT_TIME_TYPE_NORMAL:
          GFL_RTC_DEBUG_StopFakeTime();
          break;
        case CONT_TIME_TYPE_8HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 8,0 );
        case CONT_TIME_TYPE_12HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 12,0 );
          break;
        case CONT_TIME_TYPE_18HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 18,0 );
          break;
        case CONT_TIME_TYPE_22HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 22,0 );
          break;
        case CONT_TIME_TYPE_2HOUR:
          GFL_RTC_DEBUG_StartFakeFixTime( 2,0 );
          break;
        case CONT_TIME_TYPE_600RATE:
          GFL_RTC_DEBUG_StartFakeTime( 600 );
          break;
        case CONT_TIME_TYPE_60RATE:
          GFL_RTC_DEBUG_StartFakeTime( 60 );
          break;
        default:
          break;
        }
      }else{

        FLDMENUFUNC_DeleteMenu( work->menuFunc );


        // オワリ
        return( GMEVENT_RES_FINISH );
      }
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  デバッグメニュー　波乗り
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りイベント呼び出し
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @retval BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Naminori( DEBUG_MENU_EVENT_WORK *wk )
{
  if( FIELDMAP_GetMapControlType(wk->fieldWork) != FLDMAP_CTRLTYPE_GRID )
  {
    // レールのとき
    FIELD_PLAYER* p_fld_player = FIELDMAP_GetFieldPlayer( wk->fieldWork );
    if( FIELD_PLAYER_GetMoveForm( p_fld_player ) != PLAYER_MOVE_FORM_SWIM ){
      FIELD_PLAYER_SetNaminori( p_fld_player );
    }else{
      FIELD_PLAYER_SetNaminoriEnd( p_fld_player );
    }
    return( FALSE );
  }

  FIELD_EVENT_ChangeNaminoriStart( wk->gmEvent, wk->gmSys, wk->fieldWork );
  return( TRUE );
}

//======================================================================
//  デバッグメニュー ポケモン作成
//======================================================================
#include "debug/debug_makepoke.h"
FS_EXTERN_OVERLAY(debug_makepoke);
static GMEVENT_RESULT debugMenuMakePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//-------------------------------------
/// デバッグポケモン作成用ワーク
//=====================================
typedef struct
{
  HEAPID heapID;
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  PROCPARAM_DEBUG_MAKEPOKE p_mp_work;
  POKEMON_PARAM *pp;
} DEBUG_MAKEPOKE_EVENT_WORK;

//----------------------------------------------------------------------------
/**
 *  @brief  ポケモン作成
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugMakePoke( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_MAKEPOKE_EVENT_WORK *p_mp_work;

  //イヴェント
  GMEVENT_Change( p_event, debugMenuMakePoke, sizeof(DEBUG_MAKEPOKE_EVENT_WORK) );
  p_mp_work = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_mp_work, sizeof(DEBUG_MAKEPOKE_EVENT_WORK) );

  //ワーク設定
  p_mp_work->p_gamesys  = p_gamesys;
  p_mp_work->p_event    = p_event;
  p_mp_work->p_field    = p_field;
  p_mp_work->heapID     = heapID;
  p_mp_work->pp = PP_Create( 1,1,PTL_SETUP_ID_AUTO,p_mp_work->heapID );
  p_mp_work->p_mp_work.dst = p_mp_work->pp;
  p_mp_work->p_mp_work.oyaStatus = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_gamesys) );

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグポケモン作成イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuMakePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
  };

  DEBUG_MAKEPOKE_EVENT_WORK *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(debug_makepoke), &ProcData_DebugMakePoke, &p_wk->p_mp_work ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    if( p_wk->pp != NULL )
    {
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(p_wk->p_gamesys);
      POKEPARTY *party = GAMEDATA_GetMyPokemon(gmData);

      {
        u16 oyaName[6] = {L'で',L'ば',L'ぐ',L'ぽ',L'け',0xFFFF};
        PP_Put( p_wk->pp , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
        PP_Put( p_wk->pp , ID_PARA_oyasex , PTL_SEX_MALE );
      }
      //手持ちに空きがあれば入れる
      if( PokeParty_GetPokeCount( party ) < 6 )
      {
        PokeParty_Add( party , p_wk->pp );
      }

      GFL_HEAP_FreeMemory( p_wk->pp );
    }
    return GMEVENT_RES_FINISH;
    break;
  }

  return GMEVENT_RES_CONTINUE ;
}


//-------------------------------------
/// デバッグポケモン作成用ワーク
//=====================================
typedef struct
{
  HEAPID heapID;
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  PROCPARAM_DEBUG_MAKEPOKE p_mp_work;
  POKEMON_PARAM *pp;
  POKEMON_PARAM *p_src_pp;
} DEBUG_REWRITEPOKE_EVENT_WORK;
static GMEVENT_RESULT debugMenuReWritePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *  @brief  ポケモン書き換え
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugReWritePoke( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_REWRITEPOKE_EVENT_WORK *p_ev_wk;

  //イヴェント
  GMEVENT_Change( p_event, debugMenuReWritePoke, sizeof(DEBUG_REWRITEPOKE_EVENT_WORK) );
  p_ev_wk = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_ev_wk, sizeof(DEBUG_REWRITEPOKE_EVENT_WORK) );

  //ワーク設定
  p_ev_wk->p_gamesys  = p_gamesys;
  p_ev_wk->p_event    = p_event;
  p_ev_wk->p_field    = p_field;
  p_ev_wk->heapID     = heapID;
  p_ev_wk->p_src_pp   = PP_Create( 1, 1, 1, heapID );
  {
    POKEPARTY *p_party  = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(p_gamesys) );
    p_ev_wk->pp =  PokeParty_GetMemberPointer( p_party, 0 );

    POKETOOL_CopyPPtoPP( p_ev_wk->pp, p_ev_wk->p_src_pp );
  }
  p_ev_wk->p_mp_work.dst  = p_ev_wk->pp;
  p_ev_wk->p_mp_work.oyaStatus = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_gamesys) );

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグポケモン書き換えイベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuReWritePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
  };

  DEBUG_REWRITEPOKE_EVENT_WORK *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(debug_makepoke), &ProcData_DebugMakePoke, &p_wk->p_mp_work ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    {
      //書き換えないデータ
      static const u16 sc_id_tbl[]  =
      {
       ID_PARA_pref_code,
       ID_PARA_get_cassette,
       ID_PARA_get_year,
       ID_PARA_get_month,
       ID_PARA_get_day,
       ID_PARA_birth_year,
       ID_PARA_birth_month,
       ID_PARA_birth_day,
       ID_PARA_get_place,
       ID_PARA_birth_place,
       ID_PARA_pokerus,
       ID_PARA_get_ball,
       ID_PARA_get_level,
       ID_PARA_oyasex,
       ID_PARA_get_ground_id,
       ID_PARA_country_code,
       ID_PARA_style,                    //かっこよさ
       ID_PARA_beautiful,                //うつくしさ
       ID_PARA_cute,                     //かわいさ
       ID_PARA_clever,                   //かしこさ
       ID_PARA_strong,                   //たくましさ
       ID_PARA_fur,                      //毛艶
       ID_PARA_sinou_champ_ribbon,       //シンオウチャンプリボン
       ID_PARA_sinou_battle_tower_ttwin_first,     //シンオウバトルタワータワータイクーン勝利1回目
       ID_PARA_sinou_battle_tower_ttwin_second,    //シンオウバトルタワータワータイクーン勝利2回目
       ID_PARA_sinou_battle_tower_2vs2_win50,      //シンオウバトルタワータワーダブル50連勝
       ID_PARA_sinou_battle_tower_aimulti_win50,   //シンオウバトルタワータワーAIマルチ50連勝
       ID_PARA_sinou_battle_tower_siomulti_win50,  //シンオウバトルタワータワー通信マルチ50連勝
       ID_PARA_sinou_battle_tower_wifi_rank5,      //シンオウバトルタワーWifiランク５入り
       ID_PARA_sinou_syakki_ribbon,        //シンオウしゃっきリボン
       ID_PARA_sinou_dokki_ribbon,         //シンオウどっきリボン
       ID_PARA_sinou_syonbo_ribbon,        //シンオウしょんぼリボン
       ID_PARA_sinou_ukka_ribbon,          //シンオウうっかリボン
       ID_PARA_sinou_sukki_ribbon,         //シンオウすっきリボン
       ID_PARA_sinou_gussu_ribbon,         //シンオウぐっすリボン
       ID_PARA_sinou_nikko_ribbon,         //シンオウにっこリボン
       ID_PARA_sinou_gorgeous_ribbon,      //シンオウゴージャスリボン
       ID_PARA_sinou_royal_ribbon,         //シンオウロイヤルリボン
       ID_PARA_sinou_gorgeousroyal_ribbon, //シンオウゴージャスロイヤルリボン
       ID_PARA_sinou_ashiato_ribbon,       //シンオウあしあとリボン
       ID_PARA_sinou_record_ribbon,        //シンオウレコードリボン
       ID_PARA_sinou_history_ribbon,       //シンオウヒストリーリボン
       ID_PARA_sinou_legend_ribbon,        //シンオウレジェンドリボン
       ID_PARA_sinou_red_ribbon,           //シンオウレッドリボン
       ID_PARA_sinou_green_ribbon,         //シンオウグリーンリボン
       ID_PARA_sinou_blue_ribbon,          //シンオウブルーリボン
       ID_PARA_sinou_festival_ribbon,      //シンオウフェスティバルリボン
       ID_PARA_sinou_carnival_ribbon,      //シンオウカーニバルリボン
       ID_PARA_sinou_classic_ribbon,       //シンオウクラシックリボン
       ID_PARA_sinou_premiere_ribbon,      //シンオウプレミアリボン
       ID_PARA_stylemedal_normal,          //かっこよさ勲章(ノーマル)AGBコンテスト
       ID_PARA_stylemedal_super,         //かっこよさ勲章(スーパー)AGBコンテスト
       ID_PARA_stylemedal_hyper,         //かっこよさ勲章(ハイパー)AGBコンテスト
       ID_PARA_stylemedal_master,          //かっこよさ勲章(マスター)AGBコンテスト
       ID_PARA_beautifulmedal_normal,        //うつくしさ勲章(ノーマル)AGBコンテスト
       ID_PARA_beautifulmedal_super,       //うつくしさ勲章(スーパー)AGBコンテスト
       ID_PARA_beautifulmedal_hyper,       //うつくしさ勲章(ハイパー)AGBコンテスト
       ID_PARA_beautifulmedal_master,        //うつくしさ勲章(マスター)AGBコンテスト
       ID_PARA_cutemedal_normal,         //かわいさ勲章(ノーマル)AGBコンテスト
       ID_PARA_cutemedal_super,          //かわいさ勲章(スーパー)AGBコンテスト
       ID_PARA_cutemedal_hyper,          //かわいさ勲章(ハイパー)AGBコンテスト
       ID_PARA_cutemedal_master,         //かわいさ勲章(マスター)AGBコンテスト
       ID_PARA_clevermedal_normal,         //かしこさ勲章(ノーマル)AGBコンテスト
       ID_PARA_clevermedal_super,          //かしこさ勲章(スーパー)AGBコンテスト
       ID_PARA_clevermedal_hyper,          //かしこさ勲章(ハイパー)AGBコンテスト
       ID_PARA_clevermedal_master,         //かしこさ勲章(マスター)AGBコンテスト
       ID_PARA_strongmedal_normal,         //たくましさ勲章(ノーマル)AGBコンテスト
       ID_PARA_strongmedal_super,          //たくましさ勲章(スーパー)AGBコンテスト
       ID_PARA_strongmedal_hyper,          //たくましさ勲章(ハイパー)AGBコンテスト
       ID_PARA_strongmedal_master,         //たくましさ勲章(マスター)AGBコンテスト
       ID_PARA_champ_ribbon,           //チャンプリボン
       ID_PARA_winning_ribbon,           //ウィニングリボン
       ID_PARA_victory_ribbon,           //ビクトリーリボン
       ID_PARA_bromide_ribbon,           //ブロマイドリボン
       ID_PARA_ganba_ribbon,           //がんばリボン
       ID_PARA_marine_ribbon,            //マリンリボン
       ID_PARA_land_ribbon,            //ランドリボン
       ID_PARA_sky_ribbon,             //スカイリボン
       ID_PARA_country_ribbon,           //カントリーリボン
       ID_PARA_national_ribbon,          //ナショナルリボン
       ID_PARA_earth_ribbon,           //アースリボン
       ID_PARA_condition,              //コンディション
      };
      int i;
      int id;
      //書き換えて欲しくないデータを入れる

      for( i = 0; i < NELEMS(sc_id_tbl); i++ )
      {
        id  = sc_id_tbl[i];
        PP_Put( p_wk->pp, id, PP_Get( p_wk->p_src_pp, id, NULL ) );
      }
    }
    GFL_HEAP_FreeMemory( p_wk->p_src_pp );
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}


//======================================================================
//  デバッグメニュー アイテム
//======================================================================
//--------------------------------------------------------------
/**
 * @brief   デバッグアイテム作成
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_DebugItem( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT * child_event;

  child_event = GMEVENT_CreateOverlayEventCall(
      gameSys, FS_OVERLAY_ID( debug_item ), EVENT_DebugItemMake, NULL );
  //child_event = EVENT_DebugItemMake(gameSys, wk->heapID);
  GMEVENT_ChangeEvent( event, child_event );

  return( TRUE );
}
//--------------------------------------------------------------
/**
 * @brief   デバッグ隠されアイテム作成
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_DebugSecretItem( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT * child_event;

  child_event = GMEVENT_CreateOverlayEventCall(
      gameSys, FS_OVERLAY_ID( debug_item ), EVENT_DebugSecretItemMake, NULL );
  GMEVENT_ChangeEvent( event, child_event );

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * @brief   デバッグPDWアイテム作成
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_DebugPDWItem( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  GMEVENT * child_event;

  child_event = GMEVENT_CreateOverlayEventCall(
      gameSys, FS_OVERLAY_ID( debug_item ), EVENT_DebugPDWItemMake, NULL );
  GMEVENT_ChangeEvent( event, child_event );

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンボックスにポケモンでいっぱいにする
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------

static const u16 HangulCodeTable[] = {
  0xAC00, 0xAC01, 0xAC04, 0xAC07, 0xAC08, 0xAC09, 0xAC0A, 0xAC10, 0xAC11, 0xAC12, 0xAC13, 0xAC14, 0xAC15, 0xAC16, 0xAC17, 0xAC19,
  0xAC1A, 0xAC1B, 0xAC1C, 0xAC1D, 0xAC20, 0xAC24, 0xAC2C, 0xAC2D, 0xAC2F, 0xAC30, 0xAC31, 0xAC38, 0xAC39, 0xAC3C, 0xAC40, 0xAC4B,
  0xAC4D, 0xAC54, 0xAC58, 0xAC5C, 0xAC70, 0xAC71, 0xAC74, 0xAC77, 0xAC78, 0xAC7A, 0xAC80, 0xAC81, 0xAC83, 0xAC84, 0xAC85, 0xAC86,
  0xAC89, 0xAC8A, 0xAC8B, 0xAC8C, 0xAC90, 0xAC94, 0xAC9C, 0xAC9D, 0xAC9F, 0xACA0, 0xACA1, 0xACA8, 0xACA9, 0xACAA, 0xACAC, 0xACAF,
  0xACB0, 0xACB8, 0xACB9, 0xACBB, 0xACBC, 0xACBD, 0xACC1, 0xACC4, 0xACC8, 0xACCC, 0xACD5, 0xACD7, 0xACE0, 0xACE1, 0xACE4, 0xACE7,
  0xACE8, 0xACEA, 0xACEC, 0xACEF, 0xACF0, 0xACF1, 0xACF3, 0xACF5, 0xACF6, 0xACFC, 0xACFD, 0xAD00, 0xAD04, 0xAD06, 0xAD0C, 0xAD0D,
  0xAD0F, 0xAD11, 0xAD18, 0xAD1C, 0xAD20, 0xAD29, 0xAD2C, 0xAD2D, 0xAD34, 0xAD35, 0xAD38, 0xAD3C, 0xAD44, 0xAD45, 0xAD47, 0xAD49,
  0xAD50, 0xAD54, 0xAD58, 0xAD61, 0xAD63, 0xAD6C, 0xAD6D, 0xAD70, 0xAD73, 0xAD74, 0xAD75, 0xAD76, 0xAD7B, 0xAD7C, 0xAD7D, 0xAD7F,
  0xAD81, 0xAD82, 0xAD88, 0xAD89, 0xAD8C, 0xAD90, 0xAD9C, 0xAD9D, 0xADA4, 0xADB7, 0xADC0, 0xADC1, 0xADC4, 0xADC8, 0xADD0, 0xADD1,
  0xADD3, 0xADDC, 0xADE0, 0xADE4, 0xADF8, 0xADF9, 0xADFC, 0xADFF, 0xAE00, 0xAE01, 0xAE08, 0xAE09, 0xAE0B, 0xAE0D, 0xAE14, 0xAE30,
  0xAE31, 0xAE34, 0xAE37, 0xAE38, 0xAE3A, 0xAE40, 0xAE41, 0xAE43, 0xAE45, 0xAE46, 0xAE4A, 0xAE4C, 0xAE4D, 0xAE4E, 0xAE50, 0xAE54,
  0xAE56, 0xAE5C, 0xAE5D, 0xAE5F, 0xAE60, 0xAE61, 0xAE65, 0xAE68, 0xAE69, 0xAE6C, 0xAE70, 0xAE78, 0xAE79, 0xAE7B, 0xAE7C, 0xAE7D,
  0xAE84, 0xAE85, 0xAE8C, 0xAEBC, 0xAEBD, 0xAEBE, 0xAEC0, 0xAEC4, 0xAECC, 0xAECD, 0xAECF, 0xAED0, 0xAED1, 0xAED8, 0xAED9, 0xAEDC,
  0xAEE8, 0xAEEB, 0xAEED, 0xAEF4, 0xAEF8, 0xAEFC, 0xAF07, 0xAF08, 0xAF0D, 0xAF10, 0xAF2C, 0xAF2D, 0xAF30, 0xAF32, 0xAF34, 0xAF3C,
  0xAF3D, 0xAF3F, 0xAF41, 0xAF42, 0xAF43, 0xAF48, 0xAF49, 0xAF50, 0xAF5C, 0xAF5D, 0xAF64, 0xAF65, 0xAF79, 0xAF80, 0xAF84, 0xAF88,
  0xAF90, 0xAF91, 0xAF95, 0xAF9C, 0xAFB8, 0xAFB9, 0xAFBC, 0xAFC0, 0xAFC7, 0xAFC8, 0xAFC9, 0xAFCB, 0xAFCD, 0xAFCE, 0xAFD4, 0xAFDC,
  0xAFE8, 0xAFE9, 0xAFF0, 0xAFF1, 0xAFF4, 0xAFF8, 0xB000, 0xB001, 0xB004, 0xB00C, 0xB010, 0xB014, 0xB01C, 0xB01D, 0xB028, 0xB044,
  0xB045, 0xB048, 0xB04A, 0xB04C, 0xB04E, 0xB053, 0xB054, 0xB055, 0xB057, 0xB059, 0xB05D, 0xB07C, 0xB07D, 0xB080, 0xB084, 0xB08C,
  0xB08D, 0xB08F, 0xB091, 0xB098, 0xB099, 0xB09A, 0xB09C, 0xB09F, 0xB0A0, 0xB0A1, 0xB0A2, 0xB0A8, 0xB0A9, 0xB0AB, 0xB0AC, 0xB0AD,
  0xB0AE, 0xB0AF, 0xB0B1, 0xB0B3, 0xB0B4, 0xB0B5, 0xB0B8, 0xB0BC, 0xB0C4, 0xB0C5, 0xB0C7, 0xB0C8, 0xB0C9, 0xB0D0, 0xB0D1, 0xB0D4,
  0xB0D8, 0xB0E0, 0xB0E5, 0xB108, 0xB109, 0xB10B, 0xB10C, 0xB110, 0xB112, 0xB113, 0xB118, 0xB119, 0xB11B, 0xB11C, 0xB11D, 0xB123,
  0xB124, 0xB125, 0xB128, 0xB12C, 0xB134, 0xB135, 0xB137, 0xB138, 0xB139, 0xB140, 0xB141, 0xB144, 0xB148, 0xB150, 0xB151, 0xB154,
  0xB155, 0xB158, 0xB15C, 0xB160, 0xB178, 0xB179, 0xB17C, 0xB180, 0xB182, 0xB188, 0xB189, 0xB18B, 0xB18D, 0xB192, 0xB193, 0xB194,
  0xB198, 0xB19C, 0xB1A8, 0xB1CC, 0xB1D0, 0xB1D4, 0xB1DC, 0xB1DD, 0xB1DF, 0xB1E8, 0xB1E9, 0xB1EC, 0xB1F0, 0xB1F9, 0xB1FB, 0xB1FD,
  0xB204, 0xB205, 0xB208, 0xB20B, 0xB20C, 0xB214, 0xB215, 0xB217, 0xB219, 0xB220, 0xB234, 0xB23C, 0xB258, 0xB25C, 0xB260, 0xB268,
  0xB269, 0xB274, 0xB275, 0xB27C, 0xB284, 0xB285, 0xB289, 0xB290, 0xB291, 0xB294, 0xB298, 0xB299, 0xB29A, 0xB2A0, 0xB2A1, 0xB2A3,
  0xB2A5, 0xB2A6, 0xB2AA, 0xB2AC, 0xB2B0, 0xB2B4, 0xB2C8, 0xB2C9, 0xB2CC, 0xB2D0, 0xB2D2, 0xB2D8, 0xB2D9, 0xB2DB, 0xB2DD, 0xB2E2,
  0xB2E4, 0xB2E5, 0xB2E6, 0xB2E8, 0xB2EB, 0xB2EC, 0xB2ED, 0xB2EE, 0xB2EF, 0xB2F3, 0xB2F4, 0xB2F5, 0xB2F7, 0xB2F8, 0xB2F9, 0xB2FA,
  0xB2FB, 0xB2FF, 0xB300, 0xB301, 0xB304, 0xB308, 0xB310, 0xB311, 0xB313, 0xB314, 0xB315, 0xB31C, 0xB354, 0xB355, 0xB356, 0xB358,
  0xB35B, 0xB35C, 0xB35E, 0xB35F, 0xB364, 0xB365, 0xB367, 0xB369, 0xB36B, 0xB36E, 0xB370, 0xB371, 0xB374, 0xB378, 0xB380, 0xB381,
  0xB383, 0xB384, 0xB385, 0xB38C, 0xB390, 0xB394, 0xB3A0, 0xB3A1, 0xB3A8, 0xB3AC, 0xB3C4, 0xB3C5, 0xB3C8, 0xB3CB, 0xB3CC, 0xB3CE,
  0xB3D0, 0xB3D4, 0xB3D5, 0xB3D7, 0xB3D9, 0xB3DB, 0xB3DD, 0xB3E0, 0xB3E4, 0xB3E8, 0xB3FC, 0xB410, 0xB418, 0xB41C, 0xB420, 0xB428,
  0xB429, 0xB42B, 0xB434, 0xB450, 0xB451, 0xB454, 0xB458, 0xB460, 0xB461, 0xB463, 0xB465, 0xB46C, 0xB480, 0xB488, 0xB49D, 0xB4A4,
  0xB4A8, 0xB4AC, 0xB4B5, 0xB4B7, 0xB4B9, 0xB4C0, 0xB4C4, 0xB4C8, 0xB4D0, 0xB4D5, 0xB4DC, 0xB4DD, 0xB4E0, 0xB4E3, 0xB4E4, 0xB4E6,
  0xB4EC, 0xB4ED, 0xB4EF, 0xB4F1, 0xB4F8, 0xB514, 0xB515, 0xB518, 0xB51B, 0xB51C, 0xB524, 0xB525, 0xB527, 0xB528, 0xB529, 0xB52A,
  0xB530, 0xB531, 0xB534, 0xB538, 0xB540, 0xB541, 0xB543, 0xB544, 0xB545, 0xB54B, 0xB54C, 0xB54D, 0xB550, 0xB554, 0xB55C, 0xB55D,
  0xB55F, 0xB560, 0xB561, 0xB5A0, 0xB5A1, 0xB5A4, 0xB5A8, 0xB5AA, 0xB5AB, 0xB5B0, 0xB5B1, 0xB5B3, 0xB5B4, 0xB5B5, 0xB5BB, 0xB5BC,
  0xB5BD, 0xB5C0, 0xB5C4, 0xB5CC, 0xB5CD, 0xB5CF, 0xB5D0, 0xB5D1, 0xB5D8, 0xB5EC, 0xB610, 0xB611, 0xB614, 0xB618, 0xB625, 0xB62C,
  0xB634, 0xB648, 0xB664, 0xB668, 0xB69C, 0xB69D, 0xB6A0, 0xB6A4, 0xB6AB, 0xB6AC, 0xB6B1, 0xB6D4, 0xB6F0, 0xB6F4, 0xB6F8, 0xB700,
  0xB701, 0xB705, 0xB728, 0xB729, 0xB72C, 0xB72F, 0xB730, 0xB738, 0xB739, 0xB73B, 0xB744, 0xB748, 0xB74C, 0xB754, 0xB755, 0xB760,
  0xB764, 0xB768, 0xB770, 0xB771, 0xB773, 0xB775, 0xB77C, 0xB77D, 0xB780, 0xB784, 0xB78C, 0xB78D, 0xB78F, 0xB790, 0xB791, 0xB792,
  0xB796, 0xB797, 0xB798, 0xB799, 0xB79C, 0xB7A0, 0xB7A8, 0xB7A9, 0xB7AB, 0xB7AC, 0xB7AD, 0xB7B4, 0xB7B5, 0xB7B8, 0xB7C7, 0xB7C9,
  0xB7EC, 0xB7ED, 0xB7F0, 0xB7F4, 0xB7FC, 0xB7FD, 0xB7FF, 0xB800, 0xB801, 0xB807, 0xB808, 0xB809, 0xB80C, 0xB810, 0xB818, 0xB819,
  0xB81B, 0xB81D, 0xB824, 0xB825, 0xB828, 0xB82C, 0xB834, 0xB835, 0xB837, 0xB838, 0xB839, 0xB840, 0xB844, 0xB851, 0xB853, 0xB85C,
  0xB85D, 0xB860, 0xB864, 0xB86C, 0xB86D, 0xB86F, 0xB871, 0xB878, 0xB87C, 0xB88D, 0xB8A8, 0xB8B0, 0xB8B4, 0xB8B8, 0xB8C0, 0xB8C1,
  0xB8C3, 0xB8C5, 0xB8CC, 0xB8D0, 0xB8D4, 0xB8DD, 0xB8DF, 0xB8E1, 0xB8E8, 0xB8E9, 0xB8EC, 0xB8F0, 0xB8F8, 0xB8F9, 0xB8FB, 0xB8FD,
  0xB904, 0xB918, 0xB920, 0xB93C, 0xB93D, 0xB940, 0xB944, 0xB94C, 0xB94F, 0xB951, 0xB958, 0xB959, 0xB95C, 0xB960, 0xB968, 0xB969,
  0xB96B, 0xB96D, 0xB974, 0xB975, 0xB978, 0xB97C, 0xB984, 0xB985, 0xB987, 0xB989, 0xB98A, 0xB98D, 0xB98E, 0xB9AC, 0xB9AD, 0xB9B0,
  0xB9B4, 0xB9BC, 0xB9BD, 0xB9BF, 0xB9C1, 0xB9C8, 0xB9C9, 0xB9CC, 0xB9CE, 0xB9CF, 0xB9D0, 0xB9D1, 0xB9D2, 0xB9D8, 0xB9D9, 0xB9DB,
  0xB9DD, 0xB9DE, 0xB9E1, 0xB9E3, 0xB9E4, 0xB9E5, 0xB9E8, 0xB9EC, 0xB9F4, 0xB9F5, 0xB9F7, 0xB9F8, 0xB9F9, 0xB9FA, 0xBA00, 0xBA01,
  0xBA08, 0xBA15, 0xBA38, 0xBA39, 0xBA3C, 0xBA40, 0xBA42, 0xBA48, 0xBA49, 0xBA4B, 0xBA4D, 0xBA4E, 0xBA53, 0xBA54, 0xBA55, 0xBA58,
  0xBA5C, 0xBA64, 0xBA65, 0xBA67, 0xBA68, 0xBA69, 0xBA70, 0xBA71, 0xBA74, 0xBA78, 0xBA83, 0xBA84, 0xBA85, 0xBA87, 0xBA8C, 0xBAA8,
  0xBAA9, 0xBAAB, 0xBAAC, 0xBAB0, 0xBAB2, 0xBAB8, 0xBAB9, 0xBABB, 0xBABD, 0xBAC4, 0xBAC8, 0xBAD8, 0xBAD9, 0xBAFC, 0xBB00, 0xBB04,
  0xBB0D, 0xBB0F, 0xBB11, 0xBB18, 0xBB1C, 0xBB20, 0xBB29, 0xBB2B, 0xBB34, 0xBB35, 0xBB36, 0xBB38, 0xBB3B, 0xBB3C, 0xBB3D, 0xBB3E,
  0xBB44, 0xBB45, 0xBB47, 0xBB49, 0xBB4D, 0xBB4F, 0xBB50, 0xBB54, 0xBB58, 0xBB61, 0xBB63, 0xBB6C, 0xBB88, 0xBB8C, 0xBB90, 0xBBA4,
  0xBBA8, 0xBBAC, 0xBBB4, 0xBBB7, 0xBBC0, 0xBBC4, 0xBBC8, 0xBBD0, 0xBBD3, 0xBBF8, 0xBBF9, 0xBBFC, 0xBBFF, 0xBC00, 0xBC02, 0xBC08,
  0xBC09, 0xBC0B, 0xBC0C, 0xBC0D, 0xBC0F, 0xBC11, 0xBC14, 0xBC15, 0xBC16, 0xBC17, 0xBC18, 0xBC1B, 0xBC1C, 0xBC1D, 0xBC1E, 0xBC1F,
  0xBC24, 0xBC25, 0xBC27, 0xBC29, 0xBC2D, 0xBC30, 0xBC31, 0xBC34, 0xBC38, 0xBC40, 0xBC41, 0xBC43, 0xBC44, 0xBC45, 0xBC49, 0xBC4C,
  0xBC4D, 0xBC50, 0xBC5D, 0xBC84, 0xBC85, 0xBC88, 0xBC8B, 0xBC8C, 0xBC8E, 0xBC94, 0xBC95, 0xBC97, 0xBC99, 0xBC9A, 0xBCA0, 0xBCA1,
  0xBCA4, 0xBCA7, 0xBCA8, 0xBCB0, 0xBCB1, 0xBCB3, 0xBCB4, 0xBCB5, 0xBCBC, 0xBCBD, 0xBCC0, 0xBCC4, 0xBCCD, 0xBCCF, 0xBCD0, 0xBCD1,
  0xBCD5, 0xBCD8, 0xBCDC, 0xBCF4, 0xBCF5, 0xBCF6, 0xBCF8, 0xBCFC, 0xBD04, 0xBD05, 0xBD07, 0xBD09, 0xBD10, 0xBD14, 0xBD24, 0xBD2C,
  0xBD40, 0xBD48, 0xBD49, 0xBD4C, 0xBD50, 0xBD58, 0xBD59, 0xBD64, 0xBD68, 0xBD80, 0xBD81, 0xBD84, 0xBD87, 0xBD88, 0xBD89, 0xBD8A,
  0xBD90, 0xBD91, 0xBD93, 0xBD95, 0xBD99, 0xBD9A, 0xBD9C, 0xBDA4, 0xBDB0, 0xBDB8, 0xBDD4, 0xBDD5, 0xBDD8, 0xBDDC, 0xBDE9, 0xBDF0,
  0xBDF4, 0xBDF8, 0xBE00, 0xBE03, 0xBE05, 0xBE0C, 0xBE0D, 0xBE10, 0xBE14, 0xBE1C, 0xBE1D, 0xBE1F, 0xBE44, 0xBE45, 0xBE48, 0xBE4C,
  0xBE4E, 0xBE54, 0xBE55, 0xBE57, 0xBE59, 0xBE5A, 0xBE5B, 0xBE60, 0xBE61, 0xBE64, 0xBE68, 0xBE6A, 0xBE70, 0xBE71, 0xBE73, 0xBE74,
  0xBE75, 0xBE7B, 0xBE7C, 0xBE7D, 0xBE80, 0xBE84, 0xBE8C, 0xBE8D, 0xBE8F, 0xBE90, 0xBE91, 0xBE98, 0xBE99, 0xBEA8, 0xBED0, 0xBED1,
  0xBED4, 0xBED7, 0xBED8, 0xBEE0, 0xBEE3, 0xBEE4, 0xBEE5, 0xBEEC, 0xBF01, 0xBF08, 0xBF09, 0xBF18, 0xBF19, 0xBF1B, 0xBF1C, 0xBF1D,
  0xBF40, 0xBF41, 0xBF44, 0xBF48, 0xBF50, 0xBF51, 0xBF55, 0xBF94, 0xBFB0, 0xBFC5, 0xBFCC, 0xBFCD, 0xBFD0, 0xBFD4, 0xBFDC, 0xBFDF,
  0xBFE1, 0xC03C, 0xC051, 0xC058, 0xC05C, 0xC060, 0xC068, 0xC069, 0xC090, 0xC091, 0xC094, 0xC098, 0xC0A0, 0xC0A1, 0xC0A3, 0xC0A5,
  0xC0AC, 0xC0AD, 0xC0AF, 0xC0B0, 0xC0B3, 0xC0B4, 0xC0B5, 0xC0B6, 0xC0BC, 0xC0BD, 0xC0BF, 0xC0C0, 0xC0C1, 0xC0C5, 0xC0C8, 0xC0C9,
  0xC0CC, 0xC0D0, 0xC0D8, 0xC0D9, 0xC0DB, 0xC0DC, 0xC0DD, 0xC0E4, 0xC0E5, 0xC0E8, 0xC0EC, 0xC0F4, 0xC0F5, 0xC0F7, 0xC0F9, 0xC100,
  0xC104, 0xC108, 0xC110, 0xC115, 0xC11C, 0xC11D, 0xC11E, 0xC11F, 0xC120, 0xC123, 0xC124, 0xC126, 0xC127, 0xC12C, 0xC12D, 0xC12F,
  0xC130, 0xC131, 0xC136, 0xC138, 0xC139, 0xC13C, 0xC140, 0xC148, 0xC149, 0xC14B, 0xC14C, 0xC14D, 0xC154, 0xC155, 0xC158, 0xC15C,
  0xC164, 0xC165, 0xC167, 0xC168, 0xC169, 0xC170, 0xC174, 0xC178, 0xC185, 0xC18C, 0xC18D, 0xC18E, 0xC190, 0xC194, 0xC196, 0xC19C,
  0xC19D, 0xC19F, 0xC1A1, 0xC1A5, 0xC1A8, 0xC1A9, 0xC1AC, 0xC1B0, 0xC1BD, 0xC1C4, 0xC1C8, 0xC1CC, 0xC1D4, 0xC1D7, 0xC1D8, 0xC1E0,
  0xC1E4, 0xC1E8, 0xC1F0, 0xC1F1, 0xC1F3, 0xC1FC, 0xC1FD, 0xC200, 0xC204, 0xC20C, 0xC20D, 0xC20F, 0xC211, 0xC218, 0xC219, 0xC21C,
  0xC21F, 0xC220, 0xC228, 0xC229, 0xC22B, 0xC22D, 0xC22F, 0xC231, 0xC232, 0xC234, 0xC248, 0xC250, 0xC251, 0xC254, 0xC258, 0xC260,
  0xC265, 0xC26C, 0xC26D, 0xC270, 0xC274, 0xC27C, 0xC27D, 0xC27F, 0xC281, 0xC288, 0xC289, 0xC290, 0xC298, 0xC29B, 0xC29D, 0xC2A4,
  0xC2A5, 0xC2A8, 0xC2AC, 0xC2AD, 0xC2B4, 0xC2B5, 0xC2B7, 0xC2B9, 0xC2DC, 0xC2DD, 0xC2E0, 0xC2E3, 0xC2E4, 0xC2EB, 0xC2EC, 0xC2ED,
  0xC2EF, 0xC2F1, 0xC2F6, 0xC2F8, 0xC2F9, 0xC2FB, 0xC2FC, 0xC300, 0xC308, 0xC309, 0xC30C, 0xC30D, 0xC313, 0xC314, 0xC315, 0xC318,
  0xC31C, 0xC324, 0xC325, 0xC328, 0xC329, 0xC345, 0xC368, 0xC369, 0xC36C, 0xC370, 0xC372, 0xC378, 0xC379, 0xC37C, 0xC37D, 0xC384,
  0xC388, 0xC38C, 0xC3C0, 0xC3D8, 0xC3D9, 0xC3DC, 0xC3DF, 0xC3E0, 0xC3E2, 0xC3E8, 0xC3E9, 0xC3ED, 0xC3F4, 0xC3F5, 0xC3F8, 0xC408,
  0xC410, 0xC424, 0xC42C, 0xC430, 0xC434, 0xC43C, 0xC43D, 0xC448, 0xC464, 0xC465, 0xC468, 0xC46C, 0xC474, 0xC475, 0xC479, 0xC480,
  0xC494, 0xC49C, 0xC4B8, 0xC4BC, 0xC4E9, 0xC4F0, 0xC4F1, 0xC4F4, 0xC4F8, 0xC4FA, 0xC4FF, 0xC500, 0xC501, 0xC50C, 0xC510, 0xC514,
  0xC51C, 0xC528, 0xC529, 0xC52C, 0xC530, 0xC538, 0xC539, 0xC53B, 0xC53D, 0xC544, 0xC545, 0xC548, 0xC549, 0xC54A, 0xC54C, 0xC54D,
  0xC54E, 0xC553, 0xC554, 0xC555, 0xC557, 0xC558, 0xC559, 0xC55D, 0xC55E, 0xC560, 0xC561, 0xC564, 0xC568, 0xC570, 0xC571, 0xC573,
  0xC574, 0xC575, 0xC57C, 0xC57D, 0xC580, 0xC584, 0xC587, 0xC58C, 0xC58D, 0xC58F, 0xC591, 0xC595, 0xC597, 0xC598, 0xC59C, 0xC5A0,
  0xC5A9, 0xC5B4, 0xC5B5, 0xC5B8, 0xC5B9, 0xC5BB, 0xC5BC, 0xC5BD, 0xC5BE, 0xC5C4, 0xC5C5, 0xC5C6, 0xC5C7, 0xC5C8, 0xC5C9, 0xC5CA,
  0xC5CC, 0xC5CE, 0xC5D0, 0xC5D1, 0xC5D4, 0xC5D8, 0xC5E0, 0xC5E1, 0xC5E3, 0xC5E5, 0xC5EC, 0xC5ED, 0xC5EE, 0xC5F0, 0xC5F4, 0xC5F6,
  0xC5F7, 0xC5FC, 0xC5FD, 0xC5FE, 0xC5FF, 0xC600, 0xC601, 0xC605, 0xC606, 0xC607, 0xC608, 0xC60C, 0xC610, 0xC618, 0xC619, 0xC61B,
  0xC61C, 0xC624, 0xC625, 0xC628, 0xC62C, 0xC62D, 0xC62E, 0xC630, 0xC633, 0xC634, 0xC635, 0xC637, 0xC639, 0xC63B, 0xC640, 0xC641,
  0xC644, 0xC648, 0xC650, 0xC651, 0xC653, 0xC654, 0xC655, 0xC65C, 0xC65D, 0xC660, 0xC66C, 0xC66F, 0xC671, 0xC678, 0xC679, 0xC67C,
  0xC680, 0xC688, 0xC689, 0xC68B, 0xC68D, 0xC694, 0xC695, 0xC698, 0xC69C, 0xC6A4, 0xC6A5, 0xC6A7, 0xC6A9, 0xC6B0, 0xC6B1, 0xC6B4,
  0xC6B8, 0xC6B9, 0xC6BA, 0xC6C0, 0xC6C1, 0xC6C3, 0xC6C5, 0xC6CC, 0xC6CD, 0xC6D0, 0xC6D4, 0xC6DC, 0xC6DD, 0xC6E0, 0xC6E1, 0xC6E8,
  0xC6E9, 0xC6EC, 0xC6F0, 0xC6F8, 0xC6F9, 0xC6FD, 0xC704, 0xC705, 0xC708, 0xC70C, 0xC714, 0xC715, 0xC717, 0xC719, 0xC720, 0xC721,
  0xC724, 0xC728, 0xC730, 0xC731, 0xC733, 0xC735, 0xC737, 0xC73C, 0xC73D, 0xC740, 0xC744, 0xC74A, 0xC74C, 0xC74D, 0xC74F, 0xC751,
  0xC752, 0xC753, 0xC754, 0xC755, 0xC756, 0xC757, 0xC758, 0xC75C, 0xC760, 0xC768, 0xC76B, 0xC774, 0xC775, 0xC778, 0xC77C, 0xC77D,
  0xC77E, 0xC783, 0xC784, 0xC785, 0xC787, 0xC788, 0xC789, 0xC78A, 0xC78E, 0xC790, 0xC791, 0xC794, 0xC796, 0xC797, 0xC798, 0xC79A,
  0xC7A0, 0xC7A1, 0xC7A3, 0xC7A4, 0xC7A5, 0xC7A6, 0xC7AC, 0xC7AD, 0xC7B0, 0xC7B4, 0xC7BC, 0xC7BD, 0xC7BF, 0xC7C0, 0xC7C1, 0xC7C8,
  0xC7C9, 0xC7CC, 0xC7CE, 0xC7D0, 0xC7D8, 0xC7DD, 0xC7E4, 0xC7E8, 0xC7EC, 0xC800, 0xC801, 0xC804, 0xC808, 0xC80A, 0xC810, 0xC811,
  0xC813, 0xC815, 0xC816, 0xC81C, 0xC81D, 0xC820, 0xC824, 0xC82C, 0xC82D, 0xC82F, 0xC831, 0xC838, 0xC83C, 0xC840, 0xC848, 0xC849,
  0xC84C, 0xC84D, 0xC854, 0xC870, 0xC871, 0xC874, 0xC878, 0xC87A, 0xC880, 0xC881, 0xC883, 0xC885, 0xC886, 0xC887, 0xC88B, 0xC88C,
  0xC88D, 0xC894, 0xC89D, 0xC89F, 0xC8A1, 0xC8A8, 0xC8BC, 0xC8BD, 0xC8C4, 0xC8C8, 0xC8CC, 0xC8D4, 0xC8D5, 0xC8D7, 0xC8D9, 0xC8E0,
  0xC8E1, 0xC8E4, 0xC8F5, 0xC8FC, 0xC8FD, 0xC900, 0xC904, 0xC905, 0xC906, 0xC90C, 0xC90D, 0xC90F, 0xC911, 0xC918, 0xC92C, 0xC934,
  0xC950, 0xC951, 0xC954, 0xC958, 0xC960, 0xC961, 0xC963, 0xC96C, 0xC970, 0xC974, 0xC97C, 0xC988, 0xC989, 0xC98C, 0xC990, 0xC998,
  0xC999, 0xC99B, 0xC99D, 0xC9C0, 0xC9C1, 0xC9C4, 0xC9C7, 0xC9C8, 0xC9CA, 0xC9D0, 0xC9D1, 0xC9D3, 0xC9D5, 0xC9D6, 0xC9D9, 0xC9DA,
  0xC9DC, 0xC9DD, 0xC9E0, 0xC9E2, 0xC9E4, 0xC9E7, 0xC9EC, 0xC9ED, 0xC9EF, 0xC9F0, 0xC9F1, 0xC9F8, 0xC9F9, 0xC9FC, 0xCA00, 0xCA08,
  0xCA09, 0xCA0B, 0xCA0C, 0xCA0D, 0xCA14, 0xCA18, 0xCA29, 0xCA4C, 0xCA4D, 0xCA50, 0xCA54, 0xCA5C, 0xCA5D, 0xCA5F, 0xCA60, 0xCA61,
  0xCA68, 0xCA7D, 0xCA84, 0xCA98, 0xCABC, 0xCABD, 0xCAC0, 0xCAC4, 0xCACC, 0xCACD, 0xCACF, 0xCAD1, 0xCAD3, 0xCAD8, 0xCAD9, 0xCAE0,
  0xCAEC, 0xCAF4, 0xCB08, 0xCB10, 0xCB14, 0xCB18, 0xCB20, 0xCB21, 0xCB41, 0xCB48, 0xCB49, 0xCB4C, 0xCB50, 0xCB58, 0xCB59, 0xCB5D,
  0xCB64, 0xCB78, 0xCB79, 0xCB9C, 0xCBB8, 0xCBD4, 0xCBE4, 0xCBE7, 0xCBE9, 0xCC0C, 0xCC0D, 0xCC10, 0xCC14, 0xCC1C, 0xCC1D, 0xCC21,
  0xCC22, 0xCC27, 0xCC28, 0xCC29, 0xCC2C, 0xCC2E, 0xCC30, 0xCC38, 0xCC39, 0xCC3B, 0xCC3C, 0xCC3D, 0xCC3E, 0xCC44, 0xCC45, 0xCC48,
  0xCC4C, 0xCC54, 0xCC55, 0xCC57, 0xCC58, 0xCC59, 0xCC60, 0xCC64, 0xCC66, 0xCC68, 0xCC70, 0xCC75, 0xCC98, 0xCC99, 0xCC9C, 0xCCA0,
  0xCCA8, 0xCCA9, 0xCCAB, 0xCCAC, 0xCCAD, 0xCCB4, 0xCCB5, 0xCCB8, 0xCCBC, 0xCCC4, 0xCCC5, 0xCCC7, 0xCCC9, 0xCCD0, 0xCCD4, 0xCCE4,
  0xCCEC, 0xCCF0, 0xCD01, 0xCD08, 0xCD09, 0xCD0C, 0xCD10, 0xCD18, 0xCD19, 0xCD1B, 0xCD1D, 0xCD24, 0xCD28, 0xCD2C, 0xCD39, 0xCD5C,
  0xCD60, 0xCD64, 0xCD6C, 0xCD6D, 0xCD6F, 0xCD71, 0xCD78, 0xCD88, 0xCD94, 0xCD95, 0xCD98, 0xCD9C, 0xCDA4, 0xCDA5, 0xCDA7, 0xCDA9,
  0xCDB0, 0xCDC4, 0xCDCC, 0xCDD0, 0xCDE8, 0xCDEC, 0xCDF0, 0xCDF8, 0xCDF9, 0xCDFB, 0xCDFD, 0xCE04, 0xCE08, 0xCE0C, 0xCE14, 0xCE19,
  0xCE20, 0xCE21, 0xCE24, 0xCE28, 0xCE30, 0xCE31, 0xCE33, 0xCE35, 0xCE58, 0xCE59, 0xCE5C, 0xCE5F, 0xCE60, 0xCE61, 0xCE68, 0xCE69,
  0xCE6B, 0xCE6D, 0xCE74, 0xCE75, 0xCE78, 0xCE7C, 0xCE84, 0xCE85, 0xCE87, 0xCE89, 0xCE90, 0xCE91, 0xCE94, 0xCE98, 0xCEA0, 0xCEA1,
  0xCEA3, 0xCEA4, 0xCEA5, 0xCEAC, 0xCEAD, 0xCEC1, 0xCEE4, 0xCEE5, 0xCEE8, 0xCEEB, 0xCEEC, 0xCEF4, 0xCEF5, 0xCEF7, 0xCEF8, 0xCEF9,
  0xCF00, 0xCF01, 0xCF04, 0xCF08, 0xCF10, 0xCF11, 0xCF13, 0xCF15, 0xCF1C, 0xCF20, 0xCF24, 0xCF2C, 0xCF2D, 0xCF2F, 0xCF30, 0xCF31,
  0xCF38, 0xCF54, 0xCF55, 0xCF58, 0xCF5C, 0xCF64, 0xCF65, 0xCF67, 0xCF69, 0xCF70, 0xCF71, 0xCF74, 0xCF78, 0xCF80, 0xCF85, 0xCF8C,
  0xCFA1, 0xCFA8, 0xCFB0, 0xCFC4, 0xCFE0, 0xCFE1, 0xCFE4, 0xCFE8, 0xCFF0, 0xCFF1, 0xCFF3, 0xCFF5, 0xCFFC, 0xD000, 0xD004, 0xD011,
  0xD018, 0xD02D, 0xD034, 0xD035, 0xD038, 0xD03C, 0xD044, 0xD045, 0xD047, 0xD049, 0xD050, 0xD054, 0xD058, 0xD060, 0xD06C, 0xD06D,
  0xD070, 0xD074, 0xD07C, 0xD07D, 0xD081, 0xD0A4, 0xD0A5, 0xD0A8, 0xD0AC, 0xD0B4, 0xD0B5, 0xD0B7, 0xD0B9, 0xD0C0, 0xD0C1, 0xD0C4,
  0xD0C8, 0xD0C9, 0xD0D0, 0xD0D1, 0xD0D3, 0xD0D4, 0xD0D5, 0xD0DC, 0xD0DD, 0xD0E0, 0xD0E4, 0xD0EC, 0xD0ED, 0xD0EF, 0xD0F0, 0xD0F1,
  0xD0F8, 0xD10D, 0xD130, 0xD131, 0xD134, 0xD138, 0xD13A, 0xD140, 0xD141, 0xD143, 0xD144, 0xD145, 0xD14C, 0xD14D, 0xD150, 0xD154,
  0xD15C, 0xD15D, 0xD15F, 0xD161, 0xD168, 0xD16C, 0xD17C, 0xD184, 0xD188, 0xD1A0, 0xD1A1, 0xD1A4, 0xD1A8, 0xD1B0, 0xD1B1, 0xD1B3,
  0xD1B5, 0xD1BA, 0xD1BC, 0xD1C0, 0xD1D8, 0xD1F4, 0xD1F8, 0xD207, 0xD209, 0xD210, 0xD22C, 0xD22D, 0xD230, 0xD234, 0xD23C, 0xD23D,
  0xD23F, 0xD241, 0xD248, 0xD25C, 0xD264, 0xD280, 0xD281, 0xD284, 0xD288, 0xD290, 0xD291, 0xD295, 0xD29C, 0xD2A0, 0xD2A4, 0xD2AC,
  0xD2B1, 0xD2B8, 0xD2B9, 0xD2BC, 0xD2BF, 0xD2C0, 0xD2C2, 0xD2C8, 0xD2C9, 0xD2CB, 0xD2D4, 0xD2D8, 0xD2DC, 0xD2E4, 0xD2E5, 0xD2F0,
  0xD2F1, 0xD2F4, 0xD2F8, 0xD300, 0xD301, 0xD303, 0xD305, 0xD30C, 0xD30D, 0xD30E, 0xD310, 0xD314, 0xD316, 0xD31C, 0xD31D, 0xD31F,
  0xD320, 0xD321, 0xD325, 0xD328, 0xD329, 0xD32C, 0xD330, 0xD338, 0xD339, 0xD33B, 0xD33C, 0xD33D, 0xD344, 0xD345, 0xD37C, 0xD37D,
  0xD380, 0xD384, 0xD38C, 0xD38D, 0xD38F, 0xD390, 0xD391, 0xD398, 0xD399, 0xD39C, 0xD3A0, 0xD3A8, 0xD3A9, 0xD3AB, 0xD3AD, 0xD3B4,
  0xD3B8, 0xD3BC, 0xD3C4, 0xD3C5, 0xD3C8, 0xD3C9, 0xD3D0, 0xD3D8, 0xD3E1, 0xD3E3, 0xD3EC, 0xD3ED, 0xD3F0, 0xD3F4, 0xD3FC, 0xD3FD,
  0xD3FF, 0xD401, 0xD408, 0xD41D, 0xD440, 0xD444, 0xD45C, 0xD460, 0xD464, 0xD46D, 0xD46F, 0xD478, 0xD479, 0xD47C, 0xD47F, 0xD480,
  0xD482, 0xD488, 0xD489, 0xD48B, 0xD48D, 0xD494, 0xD4A9, 0xD4CC, 0xD4D0, 0xD4D4, 0xD4DC, 0xD4DF, 0xD4E8, 0xD4EC, 0xD4F0, 0xD4F8,
  0xD4FB, 0xD4FD, 0xD504, 0xD508, 0xD50C, 0xD514, 0xD515, 0xD517, 0xD53C, 0xD53D, 0xD540, 0xD544, 0xD54C, 0xD54D, 0xD54F, 0xD551,
  0xD558, 0xD559, 0xD55C, 0xD560, 0xD565, 0xD568, 0xD569, 0xD56B, 0xD56D, 0xD574, 0xD575, 0xD578, 0xD57C, 0xD584, 0xD585, 0xD587,
  0xD588, 0xD589, 0xD590, 0xD5A5, 0xD5C8, 0xD5C9, 0xD5CC, 0xD5D0, 0xD5D2, 0xD5D8, 0xD5D9, 0xD5DB, 0xD5DD, 0xD5E4, 0xD5E5, 0xD5E8,
  0xD5EC, 0xD5F4, 0xD5F5, 0xD5F7, 0xD5F9, 0xD600, 0xD601, 0xD604, 0xD608, 0xD610, 0xD611, 0xD613, 0xD614, 0xD615, 0xD61C, 0xD620,
  0xD624, 0xD62D, 0xD638, 0xD639, 0xD63C, 0xD640, 0xD645, 0xD648, 0xD649, 0xD64B, 0xD64D, 0xD651, 0xD654, 0xD655, 0xD658, 0xD65C,
  0xD667, 0xD669, 0xD670, 0xD671, 0xD674, 0xD683, 0xD685, 0xD68C, 0xD68D, 0xD690, 0xD694, 0xD69D, 0xD69F, 0xD6A1, 0xD6A8, 0xD6AC,
  0xD6B0, 0xD6B9, 0xD6BB, 0xD6C4, 0xD6C5, 0xD6C8, 0xD6CC, 0xD6D1, 0xD6D4, 0xD6D7, 0xD6D9, 0xD6E0, 0xD6E4, 0xD6E8, 0xD6F0, 0xD6F5,
  0xD6FC, 0xD6FD, 0xD700, 0xD704, 0xD711, 0xD718, 0xD719, 0xD71C, 0xD720, 0xD728, 0xD729, 0xD72B, 0xD72D, 0xD734, 0xD735, 0xD738,
  0xD73C, 0xD744, 0xD747, 0xD749, 0xD750, 0xD751, 0xD754, 0xD756, 0xD757, 0xD758, 0xD759, 0xD760, 0xD761, 0xD763, 0xD765, 0xD769,
  0xD76C, 0xD770, 0xD774, 0xD77C, 0xD77D, 0xD781, 0xD788, 0xD789, 0xD78C, 0xD790, 0xD798, 0xD799, 0xD79B, 0xD79D,
  0x3131, 0x3132, 0x3134, 0x3137, 0x3138, 0x3139, 0x3141, 0x3142, 0x3143, 0x3145, 0x3146, 0x3147, 0x3148, 0x3149, 0x314A, 0x314B,
  0x314C, 0x314D, 0x314E, 0x314F, 0x3150, 0x3151, 0x3152, 0x3153, 0x3154, 0x3155, 0x3156, 0x3157, 0x315B, 0x315C, 0x3160, 0x3161,
  0x3163,
  0xB894, 0xC330, 0xC3BC, 0xC4D4, 0xCB2C,
};

static BOOL debugMenuCallProc_BoxMax( DEBUG_MENU_EVENT_WORK *wk )
{
#if 1
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  MYSTATUS *myStatus;
  POKEMON_PARAM *pp;
  POKEMON_PASO_PARAM  *ppp;
  const STRCODE *name;
  BOOL fHangulMode = ((GFL_UI_KEY_GetCont() & PAD_BUTTON_L)!=0);
  u16  hangulPointer = 0;


  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gameSys));
  name = MyStatus_GetMyName( myStatus );

  pp = PP_Create(100, 100, 123456, HEAPID_FIELDMAP);

  {
    int i,j,k=1;
    int monsno=1;
    BOX_MANAGER* pBox = GAMEDATA_GetBoxManager(GAMESYSTEM_GetGameData(gameSys));

    if(GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
      k=BOX_MAX_TRAY;
    }
    BOXDAT_AddTrayMax( pBox );
    BOXDAT_AddTrayMax( pBox );
    for(i=0;i < k;i++){
      for(j=0;j < 30;j++){
        OS_TPrintf("%d  %d %d作成\n",monsno, i, j);
        PP_Setup(pp,  monsno , 100, MyStatus_GetID( myStatus ));

        ppp = (POKEMON_PASO_PARAM  *)PP_GetPPPPointerConst( pp );
        PPP_Put( ppp , ID_PARA_oyaname_raw , (u32)name );
        PPP_Put( ppp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );

        if( fHangulMode )
        {
          enum {
            HANGUL_NAME_LEN = 5,
          };
          STRCODE  namebuf[ HANGUL_NAME_LEN + 1 ];
          u32 i;
          for(i=0; i<HANGUL_NAME_LEN; ++i){
            namebuf[i] = HangulCodeTable[ hangulPointer++ ];
            if( hangulPointer >= NELEMS(HangulCodeTable) ){
              hangulPointer = 0;
              fHangulMode = FALSE;
            }
          }
          namebuf[ i ] = GFL_STR_GetEOMCode();
          PPP_Put( ppp, ID_PARA_nickname_raw, (u32)((void*)namebuf) );
        }

        BOXDAT_PutPokemonBox(pBox, i, ppp);

        monsno++;
        if( monsno > MONSNO_END ){
          monsno = 1;
        }
      }
    }
  }

  GFL_HEAP_FreeMemory(pp);
#endif
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * @brief   バッグにアイテムを限界までいれる
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------

static BOOL debugMenuCallProc_MyItemMax( DEBUG_MENU_EVENT_WORK *wk )
{
  u32 i;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(wk->gmSys));

  for( i = 0; i < ITEM_DATA_MAX; i++ ){
    if( ITEM_CheckEnable( i ) ){
      MYITEM_AddItem(myitem , i, 2, GFL_HEAPID_APP );
    }
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * @brief   不正チェックを通るポケモンをバトルBOXへセットする
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_SetBtlBox( DEBUG_MENU_EVENT_WORK *wk )
{
#if 1
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gameSys));
  MYSTATUS *myStatus;
  POKEMON_PASO_PARAM  *ppp;
  const STRCODE *name;
  POKEPARTY *party = PokeParty_AllocPartyWork( HEAPID_FIELDMAP );
  BATTLE_BOX_SAVE* btlbox = BATTLE_BOX_SAVE_GetBattleBoxSave( sv );
  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gameSys));
  name = MyStatus_GetMyName( myStatus );
  BATTLE_BOX_SAVE_InitWork(btlbox);



  {
    int i,j;

    static const u16 monsno_tbl1[] =
    {
      MONSNO_KORATTA,
      MONSNO_OTATI,
      MONSNO_POTIENA,
      MONSNO_BIPPA,
      MONSNO_PHI,
      MONSNO_ZIGUZAGUMA,
    };
    static const u16 waza_tbl1[] =
    {
      WAZANO_TAIATARI,
      WAZANO_HIKKAKU,
      WAZANO_TAIATARI,
      WAZANO_TAIATARI,
      WAZANO_HATAKU,
      WAZANO_TAIATARI,
    };
    static const u16 monsno_tbl2[] =
    {
      MONSNO_KAIRIKII,
      MONSNO_OKORIZARU,
      MONSNO_GOUKAZARU,
      MONSNO_HARITEYAMA,
      MONSNO_HERAKUROSU,
      MONSNO_RUKARIO,
    };
    static const u16 waza_tbl2[] =
    {
      WAZANO_BAKURETUPANTI,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
      WAZANO_INFAITO,
    };
    u16 poke_name[6] =
      L"デバグポケ";

    const u16 *p_monsno;
    const u16 *p_waza;
    u32 pow = 0;

    poke_name[ 5 ]  = 0xFFFF;


    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
    {
      p_monsno  = monsno_tbl1;
      p_waza    =waza_tbl1;
      pow       =0;
    }
    else
    {
      p_monsno  = monsno_tbl2;
      p_waza    =waza_tbl2;
      pow       =252;
    }


    for(i=0;i < PokeParty_GetPokeCountMax(party);i++){
      POKEMON_PARAM *p_temp = PP_Create( p_monsno[ i ], 100, 0, HEAPID_PROC );
      PP_Put( p_temp, ID_PARA_waza1, p_waza[ i ] );
      PP_Put( p_temp, ID_PARA_waza2, WAZANO_NULL );
      PP_Put( p_temp, ID_PARA_waza3, WAZANO_NULL );
      PP_Put( p_temp, ID_PARA_waza4, WAZANO_NULL );
      PP_Put( p_temp, ID_PARA_oyaname_raw, (u32)poke_name );
      PP_Put( p_temp, ID_PARA_pow_exp, pow );

      PP_Renew( p_temp );
      PokeParty_Add( party, p_temp );

      GFL_HEAP_FreeMemory( p_temp );
    }
    BATTLE_BOX_SAVE_SetPokeParty( btlbox, party );

  }

  GFL_HEAP_FreeMemory(party);
#endif
  return( FALSE );
}
//======================================================================
//  デバッグメニュー 主人公名前設定
//======================================================================
FS_EXTERN_OVERLAY(namein);
//-------------------------------------
/// デバッグ主人公名変更
//=====================================
typedef struct
{
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  NAMEIN_PARAM   *p_param;
  STRBUF          *p_default_str;
} DEBUG_CHANGENAME_EVENT_WORK;
//-------------------------------------
///   PROTOTYPE
//=====================================
static GMEVENT_RESULT debugEvnetChangeName( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//--------------------------------------------------------------
/**
 * @brief   主人公名前を再設定する
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ChangeName( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  DEBUG_CHANGENAME_EVENT_WORK  *p_ev_wk;
  GAMEDATA  *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  MYSTATUS  *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );
  MISC      *p_misc     = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(p_gamedata) );

  //イヴェント
  GMEVENT_Change( p_event, debugEvnetChangeName, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );
  p_ev_wk = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_ev_wk, sizeof(DEBUG_CHANGENAME_EVENT_WORK) );

  //ワーク設定
  p_ev_wk->p_gamesys  = p_gamesys;
  p_ev_wk->p_event    = p_event;
  p_ev_wk->p_field    = p_field;

  //デフォルト名
  p_ev_wk->p_default_str  = MyStatus_CreateNameString(p_mystatus, HEAPID_PROC);

  //名前入力ワーク設定
  p_ev_wk->p_param  = NAMEIN_AllocParam( HEAPID_PROC, NAMEIN_MYNAME, MyStatus_GetMySex(p_mystatus), 0, NAMEIN_PERSON_LENGTH, p_ev_wk->p_default_str, NULL );

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  主人公名前再設定イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugEvnetChangeName( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
    SEQ_EXIT,
  };

  DEBUG_CHANGENAME_EVENT_WORK  *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(namein), &NameInputProcData, p_wk->p_param ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    if( !NAMEIN_IsCancel( p_wk->p_param ) )
    {
      MYSTATUS      *p_mystatus  = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_wk->p_gamesys) );
      NAMEIN_CopyStr( p_wk->p_param, p_wk->p_default_str );
      MyStatus_SetMyNameFromString(p_mystatus, p_wk->p_default_str );
    }

    *p_seq  = SEQ_EXIT;
    break;


  case SEQ_EXIT:
    NAMEIN_FreeParam( p_wk->p_param );
    GFL_STR_DeleteBuffer( p_wk->p_default_str );

    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}



//======================================================================
//  デバッグメニュー そらを飛ぶ
//======================================================================
FS_EXTERN_OVERLAY(townmap);
//-------------------------------------
/// デバッグ空を飛ぶ用ワーク
//=====================================
typedef struct
{
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  TOWNMAP_PARAM   *p_param;
} DEBUG_SKYJUMP_EVENT_WORK;
//-------------------------------------
///   PROTOTYPE
//=====================================
static GMEVENT_RESULT debugMenuSkyJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ空を飛ぶ
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugSkyJump( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  DEBUG_SKYJUMP_EVENT_WORK  *p_sky;

  //イヴェント
  GMEVENT_Change( p_event, debugMenuSkyJump, sizeof(DEBUG_SKYJUMP_EVENT_WORK) );
  p_sky = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_sky, sizeof(DEBUG_SKYJUMP_EVENT_WORK) );

  //ワーク設定
  p_sky->p_gamesys  = p_gamesys;
  p_sky->p_event    = p_event;
  p_sky->p_field    = p_field;

  //タウンマップワーク設定
  p_sky->p_param  = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(TOWNMAP_PARAM) );
  GFL_STD_MemClear( p_sky->p_param, sizeof(TOWNMAP_PARAM) );
  p_sky->p_param->mode      = TOWNMAP_MODE_DEBUGSKY;
  {
    GAMEDATA    *p_gamedata;
    PLAYER_WORK *p_player;
    p_gamedata  = GAMESYSTEM_GetGameData( p_sky->p_gamesys );
    p_player    = GAMEDATA_GetMyPlayerWork( p_gamedata );
    p_sky->p_param->zoneID    = PLAYERWORK_getZoneID(p_player);
    p_sky->p_param->escapeID  = GAMEDATA_GetEscapeLocation( p_gamedata )->zone_id;
    p_sky->p_param->p_gamesys = p_gamesys;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ空を飛ぶイベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuSkyJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
    SEQ_CHANGE_MAP,
    SEQ_EXIT,
  };

  DEBUG_SKYJUMP_EVENT_WORK  *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(townmap), &TownMap_ProcData, p_wk->p_param ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    if( p_wk->p_param->select == TOWNMAP_SELECT_SKY )
    {
      *p_seq  = SEQ_CHANGE_MAP;
    }
    else
    {
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_CHANGE_MAP:
    NAGI_Printf("ゾーンID %d",p_wk->p_param->zoneID);
    GMEVENT_CallEvent( p_wk->p_event, DEBUG_EVENT_QuickChangeMapDefaultPos( p_wk->p_gamesys, p_wk->p_field, p_wk->p_param->zoneID ) );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    if( p_wk->p_param )
    {
      GFL_HEAP_FreeMemory(p_wk->p_param);
    }
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

//======================================================================
//  デバッグメニュー　自機男女切り替え
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー　自機男女切り替え
 * @param wk DEBUG_MENU_EVENT_WORK
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ChangePlayerSex( DEBUG_MENU_EVENT_WORK *wk )
{
  int sex;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldMap = wk->fieldWork;
  u16 zone_id = FIELDMAP_GetZoneID( fieldMap );
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( wk->gdata );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( wk->gdata );
  MYSTATUS *s_mystatus = SaveData_GetMyStatus( save );

  sex = MyStatus_GetMySex( mystatus );
  sex ^= 1;
  MyStatus_SetMySex( mystatus, sex );
  MyStatus_SetMySex( s_mystatus, sex );

  {
    GMEVENT * mapchange_event;
    mapchange_event = DEBUG_EVENT_ChangeMapDefaultPos( gsys, fieldMap, zone_id );
    GMEVENT_ChangeEvent( event, mapchange_event );
  }
  return( TRUE );
}

//======================================================================
//  デバッグメニュー　GTS画面へ
//======================================================================
FS_EXTERN_OVERLAY(worldtrade);
#include "net/dwc_rapcommon.h"
//----------------------------------------------------------------------------
/**
 *  @brief  GTS画面へいくイベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_WifiGts( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_Gts( p_wk->gmSys, p_wk->fieldWork, TRUE ) );
  return TRUE;
}


//======================================================================
//  デバッグメニュー　GDS接続
//======================================================================
#include "net_app/gds_main.h"
FS_EXTERN_OVERLAY(gds_comm);
//-------------------------------------
/// デバッグGDS用ワーク
//=====================================
typedef struct
{
  GAMESYS_WORK        *gsys;
  FIELDMAP_WORK     *fieldWork;
} DEBUG_GDS_EVENT_WORK;
static GMEVENT_RESULT debugMenuGDS( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *  @brief  GTS画面へいくイベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_GDS( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK *gsys = p_wk->gmSys;
  FIELDMAP_WORK *fieldWork = p_wk->fieldWork;
  GAMEDATA *p_gamedata      = GAMESYSTEM_GetGameData(gsys);
  GMEVENT       *p_event    = p_wk->gmEvent;
  DEBUG_GDS_EVENT_WORK *p_gds;

  if( 1)//WifiList_CheckMyGSID(GAMEDATA_GetWiFiList(p_gamedata) ) )
  {
    //イヴェント
    GMEVENT_Change( p_event, debugMenuGDS, sizeof(DEBUG_GDS_EVENT_WORK) );
    p_gds = GMEVENT_GetEventWork( p_event );
    p_gds->gsys = gsys;
    p_gds->fieldWork = fieldWork;
  }
  else
  {
    OS_Printf( "GameSpyIDが不正なので、GTSを開始しなかった\n" );
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグGTS接続用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuGDS( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  DEBUG_GDS_EVENT_WORK  *p_gds  = p_wk_adrs;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(p_gds->gsys);

  switch(*p_seq )
  {
  case 0:
    if(GameCommSys_BootCheck(game_comm) != GAME_COMM_STATUS_NULL){
      GameCommSys_ExitReq(game_comm);
    }
    (*p_seq)++;
    break;
  case 1:
    if(GameCommSys_BootCheck(game_comm) == GAME_COMM_STATUS_NULL){
      (*p_seq)++;
    }
    break;
  case 2:
    {
      GDSPROC_PARAM *gds_param;

      gds_param = GFL_HEAP_AllocClearMemory(HEAPID_PROC, sizeof(GDSPROC_PARAM));
      gds_param->gamedata = GAMESYSTEM_GetGameData(p_gds->gsys);
      gds_param->gds_mode = BR_MODE_GLOBAL_MUSICAL;//BR_MODE_GLOBAL_BV;

      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
      {
        gds_param->gds_mode = BR_MODE_GLOBAL_BV;
      }

      GMEVENT_CallEvent( p_event, EVENT_FieldSubProc( p_gds->gsys, p_gds->fieldWork,
          NO_OVERLAY_ID, &GdsMainProcData, gds_param ) );
      (*p_seq)++;
    }
    break;

  case 3:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//======================================================================
//  デバッグメニュー UIテンプレートへ
//======================================================================
FS_EXTERN_OVERLAY(ui_debug);
//-------------------------------------
/// デバッグUIテンプレート用ワーク
//=====================================
typedef struct
{
  GAMESYS_WORK        *p_gamesys;
  GMEVENT             *p_event;
  FIELDMAP_WORK       *p_field;
  UI_TEMPLATE_PARAM   param;
} DEBUG_UITEMPLATE_EVENT_WORK;
static GMEVENT_RESULT debugMenuUITemplate( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *  @brief  UIテンプレート接続
 *
 *  @param  DEBUG_MENU_EVENT_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_UITemplate( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field    = p_wk->fieldWork;
  DEBUG_UITEMPLATE_EVENT_WORK *p_param;

  //イヴェント
  GMEVENT_Change( p_event, debugMenuUITemplate, sizeof(DEBUG_UITEMPLATE_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_param, sizeof(DEBUG_UITEMPLATE_EVENT_WORK) );

  p_param->p_gamesys  = p_gamesys;
  p_param->param.gamesys  = p_gamesys;
  p_param->p_event    = p_event;
  p_param->p_field    = p_field;

  OS_Printf( "UIテンプレート Start\n" );

  return TRUE;

}
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグGTS接続用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuUITemplate( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  DEBUG_UITEMPLATE_EVENT_WORK *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
      GMEVENT_CallEvent( p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(ui_debug), &UITemplateProcData, &p_wk->param ) );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

//======================================================================
//  怪力
//======================================================================
//--------------------------------------------------------------
/**
 * 怪力実行
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @retval BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Kairiki( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  EVENTWORK *evwork = GAMEDATA_GetEventWork( wk->gdata );
  EVENTWORK_SetEventFlag( evwork, SYS_FLAG_KAIRIKI );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　線形カメラ操作
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ControlLinerCamera( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_CTL_LINERCAMERA_WORK *work;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;

  GMEVENT_Change( event, debugMenuControlLinerCamera, sizeof(DEBUG_CTL_LINERCAMERA_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_CTL_LINERCAMERA_WORK) );

  work->gsys = gsys;
  work->event = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  {
    FIELD_SUBSCREEN_WORK * subscreen;


    // カメラ操作は下画面で行う
    subscreen = FIELDMAP_GetFieldSubscreenWork(work->fieldWork);
    FIELD_SUBSCREEN_ChangeForce(subscreen, FIELD_SUBSCREEN_DEBUG_TOUCHCAMERA);
    {
      void * inner_work;
      FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);
      inner_work = FIELD_SUBSCREEN_DEBUG_GetControl(subscreen);
      FIELD_CAMERA_DEBUG_BindSubScreen(cam, inner_work, FIELD_CAMERA_DEBUG_BIND_CAMERA_POS, heapID);

      //カメラのバインドを切る
      FIELD_CAMERA_FreeTarget(cam);
    }
  }
  // レールカメラ反映の停止
  {
    FLDNOGRID_MAPPER* mapper;
    FLD_SCENEAREA* scenearea;


    if( FIELDMAP_GetBaseSystemType( work->fieldWork ) == FLDMAP_BASESYS_RAIL )
    {
      mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
      FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    }
    else
    {
      // カメラシーンの停止
      scenearea = FIELDMAP_GetFldSceneArea( work->fieldWork );
      FLD_SCENEAREA_SetActiveFlag( scenearea, FALSE );
    }
  }

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLinerCamera(
    GMEVENT *event, int *seq, void *wk )
{
  VecFx32 add = {0,0,0};
  BOOL shift = FALSE;

  DEBUG_CTL_LINERCAMERA_WORK *work = wk;
  FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);


  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    FIELD_CAMERA_DEBUG_ReleaseSubScreen( cam );

    // レールカメラ反映の再開
    {
      FLDNOGRID_MAPPER* mapper;
      FLD_SCENEAREA* scenearea;

      if( FIELDMAP_GetBaseSystemType( work->fieldWork ) == FLDMAP_BASESYS_RAIL )
      {
        mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
        FLDNOGRID_MAPPER_SetRailCameraActive( mapper, TRUE );
      }
      else
      {
        // カメラシーンの停止
        scenearea = FIELDMAP_GetFldSceneArea( work->fieldWork );
        FLD_SCENEAREA_SetActiveFlag( scenearea, TRUE );
      }
    }
    //バインド復帰
    FIELD_CAMERA_BindDefaultTarget(cam);
    return( GMEVENT_RES_FINISH );
  }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG ){
    DampCameraInfo(cam);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_UP ){             //奥
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.z);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN ){             //手前
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.z);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT ){             //左
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.x);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ){             //右
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.x);
  }else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){             //上
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.y);
  }else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){             //下
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.y);
  }else{
    work->Wait = 0;
    work->Count = 0;
  }
  if(shift){
    VecFx32 pos;
    FIELD_CAMERA_GetTargetPos( cam, &pos );
    VEC_Add( &pos, &add, &pos );
    FIELD_CAMERA_SetTargetPos( cam, &pos );
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作キー入力
 * @param work      カメラワーク
 * @param inAddval  気入力結果の際の加算値
 * @param outVal    格納バッファ
 * @retval  BOOL    加算したらTRUE
 */
//--------------------------------------------------------------
static BOOL LinerCamKeyContCtl(DEBUG_CTL_LINERCAMERA_WORK *work, const fx32 inAddVal, fx32 *outVal)
{
  if (work->Wait >= LINER_CAM_KEY_WAIT){
    *outVal = inAddVal;
    return TRUE;
  }else{
    work->Wait++;
    if (work->Count>0){
      work->Count--;
    }else{
      *outVal = inAddVal;
      work->Count = LINER_CAM_KEY_COUNT;
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作 情報ダンプ
 * @param cam   フィールドカメラポインタ
 * @retval  none
 */
//--------------------------------------------------------------
static void DampCameraInfo(FIELD_CAMERA * cam)
{
  u16 pitch;
  u16 yaw;
  fx32 len;
  VecFx32 target;
  u16 fovy;
  VecFx32 shift;

  OS_Printf("--DUMP_FIELD_CAMERA_PARAM--\n");

  pitch = FIELD_CAMERA_GetAnglePitch(cam);
  yaw = FIELD_CAMERA_GetAngleYaw(cam);
  len = FIELD_CAMERA_GetAngleLen(cam);
  fovy = FIELD_CAMERA_GetFovy(cam);
  FIELD_CAMERA_GetTargetOffset( cam, &shift );
  FIELD_CAMERA_GetTargetPos( cam, &target );

  OS_Printf("%d,%d,%d,%d,%d,%d\n", pitch, yaw, len, target.x, target.y, target.z );
#ifdef DEBUG_ONLY_FOR_saitou
  OS_Printf("%d,%d,%d,%d\n", fovy, shift.x, shift.y, shift.z );
#endif
}

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　Yボタン登録操作
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenu_ControlShortCut( DEBUG_MENU_EVENT_WORK *wk )
{
  int i;
  GAMESYS_WORK *gsys = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  SHORTCUT *p_shortcut_sv = SaveData_GetShortCut( p_sv );
  for( i = 0; i < SHORTCUT_ID_MAX; i++ )
  {
    SHORTCUT_SetRegister( p_shortcut_sv, i, TRUE );
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー ミュージカルショット作成
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
FS_EXTERN_OVERLAY( gds_debug );
static BOOL debugMenu_CreateMusicalShotData( DEBUG_MENU_EVENT_WORK *wk )
{
  int i;
  GAMESYS_WORK *gsys = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  MUSICAL_SAVE *musSave   = MUSICAL_SAVE_GetMusicalSave( p_sv );

  GFL_OVERLAY_Load( FS_OVERLAY_ID(gds_debug) );

  MUSICAL_DEBUG_CreateDummyData( MUSICAL_SAVE_GetMusicalShotData( musSave ), MONSNO_PIKATYUU , HEAPID_PROC );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(gds_debug));

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  ビーコンでの友達コード配信
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_BeaconFriendCode( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * new_event;
  new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_beacon ), EVENT_DebugBeacon, wk->fieldWork );
  //new_event = EVENT_DebugBeacon( wk->gmSys, wk->fieldWork );
  GMEVENT_ChangeEvent( wk->gmEvent, new_event );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  WiFi世界対戦へ接続
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_WifiBattleMatch( DEBUG_MENU_EVENT_WORK *wk )
{
  WIFIBATTLEMATCH_MODE mode = WIFIBATTLEMATCH_MODE_RANDOM;
  WIFIBATTLEMATCH_BTLRULE btl_rule  = WIFIBATTLEMATCH_BTLRULE_SINGLE;
#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L && GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    btl_rule  = WIFIBATTLEMATCH_BTLRULE_ROTATE;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    btl_rule  = WIFIBATTLEMATCH_BTLRULE_DOUBLE;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    btl_rule  = WIFIBATTLEMATCH_BTLRULE_TRIPLE;
  }
#endif

  GMEVENT_ChangeEvent( wk->gmEvent, EVENT_WifiBattleMatch( wk->gmSys, wk->fieldWork, DEBUG_MODE(mode), WIFIBATTLEMATCH_POKE_BTLBOX, btl_rule ) );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  サケサーバーデバッグモードへ
 *
 *  @param  DEBUG_MENU_EVENT_WORK *p_wk   ワーク
 *
 *  @return TRUEでイベンﾄ継続
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugSake( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_DEBUG_WifiMatch( p_wk->gmSys, p_wk->fieldWork, DEBUG_WIFIBATTLEMATCH_MODE_SAKE ) );
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  アトラスサーバーデバッグモードへ
 *
 *  @param  DEBUG_MENU_EVENT_WORK *p_wk   ワーク
 *
 *  @return TRUEでイベンﾄ継続
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugAtlas( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_DEBUG_WifiMatch( p_wk->gmSys, p_wk->fieldWork, DEBUG_WIFIBATTLEMATCH_MODE_ATLAS ) );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief   デバッグメニュー：ジオネット呼び出し
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Geonet( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GMEVENT_ChangeEvent( p_wk->gmEvent, EVENT_Geonet( p_wk->gmSys ) );
  return TRUE;
}
//======================================================================
//  バトルレコーダー
//======================================================================
#include "net_app/battle_recorder.h"
FS_EXTERN_OVERLAY( battle_recorder );

static GMEVENT_RESULT debugBattleRecorder( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//-------------------------------------
/// バトルレコーダーPROCイベントワーク
//=====================================
typedef struct
{
  HEAPID heapID;
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK   *p_field;
  BATTLERECORDER_PARAM  param;
} DEBUG_BR_EVENT_WORK;

//----------------------------------------------------------------------------
/**
 *  @brief  バトルレコーダーへ
 *
 *  @param  DEBUG_MENU_EVENT_WORK *p_wk   ワーク
 *
 *  @return TRUEでイベンﾄ継続
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_BattleRecorder( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field  = p_wk->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_BR_EVENT_WORK *p_param;

  //イヴェント
  GMEVENT_Change( p_event, debugBattleRecorder, sizeof(DEBUG_BR_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_param, sizeof(DEBUG_BR_EVENT_WORK) );

  //ワーク設定
  p_param->p_gamesys  = p_gamesys;
  p_param->p_event    = p_event;
  p_param->p_field    = p_field;
  p_param->heapID     = heapID;
  p_param->param.p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  p_param->param.mode       = BR_MODE_BROWSE;

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    p_param->param.mode  = BR_MODE_GLOBAL_BV;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    p_param->param.mode  = BR_MODE_GLOBAL_MUSICAL;
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  バトルレコーダー呼び出しイベント
 *
 *  @param  GMEVENT *p_event  イベント
 *  @param  *p_seq      シーケンス
 *  @param  *p_wk_adrs  ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugBattleRecorder( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
  };

  DEBUG_BR_EVENT_WORK *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(battle_recorder), &BattleRecorder_ProcData, &p_wk->param ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}

//-----------------------------------------------------------------------------
/**
 *    グラフィッカー用
 *      カメラ　こまかく設定
 */
//-----------------------------------------------------------------------------
//-------------------------------------
/// デバッグカメラ　こまかく設定用ワーク
//=====================================
typedef struct
{
  FIELDMAP_WORK*  p_field;
  FIELD_CAMERA*   p_camera;
  HEAPID heapID;

  BOOL draw_help;
  BOOL param_change;

  GFL_BMPWIN* p_win;
} DEBUG_DELICATE_CAMERA_EVENT_WORK;
static GMEVENT_RESULT debugMenuDelicateCamera( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  起動
 *
 *  @param  wk
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_ControlDelicateCamera( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_DELICATE_CAMERA_EVENT_WORK* p_work;
  GMEVENT* p_event = wk->gmEvent;
  FIELDMAP_WORK* p_fieldWork = wk->fieldWork;
  HEAPID heapID = wk->heapID;

  GMEVENT_Change( p_event, debugMenuDelicateCamera, sizeof(DEBUG_DELICATE_CAMERA_EVENT_WORK) );
  p_work = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_work, sizeof(DEBUG_DELICATE_CAMERA_EVENT_WORK) );

  p_work->p_field   = p_fieldWork;
  p_work->p_camera  = FIELDMAP_GetFieldCamera( p_fieldWork );
  p_work->heapID    = heapID;


  // DEBUG_ CAMERA
  GFL_OVERLAY_Load( FS_OVERLAY_ID(debug_camera) );

  FIELD_CAMERA_DEBUG_InitControl( p_work->p_camera, heapID );

  // レールカメラ反映の停止
  {
    FLDNOGRID_MAPPER* mapper;
    FLD_SCENEAREA* scenearea;

    if( FIELDMAP_GetBaseSystemType( p_work->p_field ) == FLDMAP_BASESYS_RAIL )
    {
      mapper = FIELDMAP_GetFldNoGridMapper( p_work->p_field );
      FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    }
    else
    {
      // カメラシーンの停止
      scenearea = FIELDMAP_GetFldSceneArea( p_work->p_field );
      FLD_SCENEAREA_SetActiveFlag( scenearea, FALSE );
    }
  }


  // 表示面の作成
  {
    const FLDMAPPER* cp_mapper = FIELDMAP_GetFieldG3Dmapper( p_fieldWork );
    fx32 map_size_x, map_size_z;

    FLDMAPPER_GetSize( cp_mapper, &map_size_x, &map_size_z );

    // インフォーバーの非表示
    FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(p_fieldWork));
    GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

    // ビットマップウィンドウ初期化
    {
      static const GFL_BG_BGCNT_HEADER header_sub3 = {
        0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
        GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
      };

      GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
      GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
      GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

      // パレット情報を転送
      GFL_ARC_UTIL_TransVramPalette(
        ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, heapID );

      // ビットマップウィンドウを作成
      p_work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
        1, 1, 30, 22,
        FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_B );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_work->p_win ), 0xf );
      GFL_BMPWIN_MakeScreen( p_work->p_win );
      GFL_BMPWIN_TransVramCharacter( p_work->p_win );
      GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );

      // ウィンドウ
      BmpWinFrame_GraphicSet( FIELD_SUBSCREEN_BGPLANE, 1, 15, 0, heapID );
      BmpWinFrame_Write( p_work->p_win, TRUE, 1, 15 );

      FIELD_CAMERA_DEBUG_DrawInfo( p_work->p_camera, p_work->p_win, map_size_x, map_size_z );
    }
  }

  return( TRUE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カメラ　詳細操作　メイン
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuDelicateCamera( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  DEBUG_DELICATE_CAMERA_EVENT_WORK* p_work = p_wk_adrs;
  BOOL result;
  int trg = GFL_UI_KEY_GetTrg();
  int cont = GFL_UI_KEY_GetCont();
  int repeat = GFL_UI_KEY_GetRepeat();
  const FLDMAPPER* cp_mapper = FIELDMAP_GetFieldG3Dmapper( p_work->p_field );
  fx32 map_size_x, map_size_z;

  FLDMAPPER_GetSize( cp_mapper, &map_size_x, &map_size_z );

  // SELECT終了
  if( trg & PAD_BUTTON_SELECT )
  {
    // ビットマップウィンドウ破棄
    {
      BmpWinFrame_Clear( p_work->p_win, TRUE );
      GFL_BMPWIN_Delete( p_work->p_win );
      GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
    }

    FIELD_CAMERA_DEBUG_ExitControl( p_work->p_camera );

    // インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(p_work->p_field,
        FIELD_SUBSCREEN_Init( p_work->heapID, p_work->p_field, FIELD_SUBSCREEN_NORMAL ));


    // DEBUG_ CAMERA
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(debug_camera) );

    return ( GMEVENT_RES_FINISH );
  }

  // DEBUGで、スクリプト操作用の表示を出す。
  // バッファリングモードも変わってしまいますが・・・。
  if( trg & PAD_BUTTON_DEBUG )
  {
    VecFx32 offset;
    VecFx32 target;
    u16 pitch;
    u16 yaw;
    fx32 len;

    // 線形カメラ用表示パラメータをカメラに設定
    FIELD_CAMERA_GetTargetOffset( p_work->p_camera, &offset );
    FIELD_CAMERA_GetTargetPos( p_work->p_camera, &target );
    pitch = FIELD_CAMERA_GetAnglePitch( p_work->p_camera );
    yaw   = FIELD_CAMERA_GetAngleYaw( p_work->p_camera );
    len   = FIELD_CAMERA_GetAngleLen( p_work->p_camera );
    FIELD_CAMERA_DEBUG_SetUpLinerCameraInfoData( p_work->p_camera );

    // 平行移動も含めた座標を返す。
    DampCameraInfo( p_work->p_camera );

    // 元に戻す
    FIELD_CAMERA_SetTargetPos( p_work->p_camera, &target );
    FIELD_CAMERA_SetTargetOffset( p_work->p_camera, &offset );
    FIELD_CAMERA_SetAnglePitch( p_work->p_camera, pitch );
    FIELD_CAMERA_SetAngleYaw( p_work->p_camera, yaw );
    FIELD_CAMERA_SetAngleLen( p_work->p_camera, len );
  }


  // カメラデバック入力
  result = FIELD_CAMERA_DEBUG_Control( p_work->p_camera, trg, cont, repeat );

  // タッチでヘルプ表示切替
  if( GFL_UI_TP_GetTrg() )
  {
    p_work->draw_help ^= 1;
    result = TRUE;
  }

  // ヘルプ表示
  if( result )
  {
    p_work->param_change = TRUE;
  }
  // 描画が重いので、操作が完了してから描画更新
  else if( p_work->param_change )
  {
    p_work->param_change = FALSE;
    if( p_work->draw_help )
    {
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_work->p_win ), 0xf );
      FIELD_CAMERA_DEBUG_DrawControlHelp( p_work->p_camera, p_work->p_win );
    }
    else
    {
      // カメラ情報を更新
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_work->p_win ), 0xf );
      FIELD_CAMERA_DEBUG_DrawInfo( p_work->p_camera, p_work->p_win, map_size_x, map_size_z );
    }
  }


  return( GMEVENT_RES_CONTINUE );
}




//======================================================================
//  技思い出し画面テスト呼び出し
//======================================================================
FS_EXTERN_OVERLAY(waza_oshie);
//-------------------------------------
/// 技思い出し画面テスト用ワーク
//=====================================
typedef struct
{
  GAMESYS_WORK        *p_gamesys;
  GMEVENT             *p_event;
  FIELDMAP_WORK       *p_field;
  WAZAOSHIE_DATA      param;
} DEBUG_WAZAOSHIE_EVENT_WORK;
static GMEVENT_RESULT debugMenuWazaOshie( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  技思い出し画面
 *
 *  @param  DEBUG_MENU_EVENT_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_WazaOshie( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field    = p_wk->fieldWork;
  DEBUG_WAZAOSHIE_EVENT_WORK  *p_param;
  GAMEDATA *gmData = GAMESYSTEM_GetGameData(p_gamesys);
  POKEPARTY *party = GAMEDATA_GetMyPokemon(gmData);
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gmData);

  // イベント起動
  GMEVENT_Change( p_event, debugMenuWazaOshie, sizeof(DEBUG_WAZAOSHIE_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_param, sizeof(DEBUG_WAZAOSHIE_EVENT_WORK) );

  p_param->p_gamesys     = p_gamesys;
  p_param->p_event       = p_event;
  p_param->p_field       = p_field;

//  p_param->param.gsys    = p_gamesys;
  p_param->param.pp   = PokeParty_GetMemberPointer( party, 0 );
  p_param->param.myst = SaveData_GetMyStatus(sv);   // 自分データ
  p_param->param.cfg  = SaveData_GetConfig(sv);     // コンフィグデータ
  p_param->param.gsys = p_gamesys;
  p_param->param.waza_tbl   = WAZAOSHIE_GetRemaindWaza( p_param->param.pp, HEAPID_WORLD );

  OS_Printf( "技教え Start\n" );

  return TRUE;

}
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ技思い出し画面接続用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuWazaOshie( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  DEBUG_WAZAOSHIE_EVENT_WORK  *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:

    GMEVENT_CallEvent( p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
                                                    FS_OVERLAY_ID(waza_oshie), &WazaOshieProcData, &p_wk->param ) );

    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}


//======================================================================
//  使用メモリの表示（主要な）
//======================================================================
//-------------------------------------
/// 使用メモリの表示（主要な）
//=====================================
typedef struct
{
  FIELDMAP_WORK       *p_field;
  GFL_BMPWIN* p_win;
  HEAPID heapID;

  // MESSAGEデータ
  WORDSET*    p_debug_wordset;
  GFL_FONT*   p_debug_font;
  GFL_MSGDATA*  p_debug_msgdata;
  STRBUF*     p_debug_strbuff;
  STRBUF*     p_debug_strbuff_tmp;
} DEBUG_USEMEMORY_EVENT_WORK;
static GMEVENT_RESULT debugMenuUseMemoryDump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
static void debugMenuWriteUseMemoryDump( DEBUG_USEMEMORY_EVENT_WORK* p_wk );


//----------------------------------------------------------------------------
/**
 *  @brief  メモリ使用情報表示
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_UseMemoryDump( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DEBUG_USEMEMORY_EVENT_WORK* p_work;
  GMEVENT* p_event = p_wk->gmEvent;
  FIELDMAP_WORK* p_fieldWork = p_wk->fieldWork;
  HEAPID heapID = p_wk->heapID;

  GMEVENT_Change( p_event, debugMenuUseMemoryDump, sizeof(DEBUG_USEMEMORY_EVENT_WORK) );
  p_work = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_work, sizeof(DEBUG_USEMEMORY_EVENT_WORK) );

  p_work->p_field   = p_fieldWork;
  p_work->heapID    = heapID;

  // MESSAGE情報読み込み
  {
    // ワードセット作成
    p_work->p_debug_wordset = WORDSET_Create( heapID );
    p_work->p_debug_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_field_dat, heapID );

    p_work->p_debug_strbuff    = GFL_STR_CreateBuffer( 256, heapID );
    p_work->p_debug_strbuff_tmp  = GFL_STR_CreateBuffer( 256, heapID );

    // フォントデータ
    p_work->p_debug_font = GFL_FONT_Create(
      ARCID_FONT, NARC_font_large_gftr,
      GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
  }


  // 表示面の作成
  {
    // インフォーバーの非表示
    FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(p_fieldWork));
    GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

    // ビットマップウィンドウ初期化
    {
      static const GFL_BG_BGCNT_HEADER header_sub3 = {
        0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
        GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
      };

      GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
      GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
      GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

      // パレット情報を転送
      GFL_ARC_UTIL_TransVramPalette(
        ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, heapID );

      // ビットマップウィンドウを作成
      p_work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
        1, 1, 30, 22,
        FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_B );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_work->p_win ), 0xf );
      GFL_BMPWIN_MakeScreen( p_work->p_win );
      GFL_BMPWIN_TransVramCharacter( p_work->p_win );
      GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );

      // ウィンドウ
      BmpWinFrame_GraphicSet( FIELD_SUBSCREEN_BGPLANE, 1, 15, 0, heapID );
      BmpWinFrame_Write( p_work->p_win, TRUE, 1, 15 );
    }

    // 描画処理
    {
      debugMenuWriteUseMemoryDump( p_work );
    }
  }

  return( TRUE );
}


//----------------------------------------------------------------------------
/**
 *  @brief  メモリーダンプメイン
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuUseMemoryDump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  DEBUG_USEMEMORY_EVENT_WORK* p_work = p_wk_adrs;

  // 表示OFF
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    // 全破棄
    {
      BmpWinFrame_Clear( p_work->p_win, TRUE );
      GFL_BMPWIN_Delete( p_work->p_win );
      GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
    }

    // MESSAGE情報破棄
    {
      // フォントデータ
      GFL_FONT_Delete( p_work->p_debug_font );
      p_work->p_debug_font = NULL;

      GFL_MSG_Delete( p_work->p_debug_msgdata );
      p_work->p_debug_msgdata = NULL;

      WORDSET_Delete( p_work->p_debug_wordset );
      p_work->p_debug_wordset = NULL;

      GFL_STR_DeleteBuffer( p_work->p_debug_strbuff );
      p_work->p_debug_strbuff = NULL;
      GFL_STR_DeleteBuffer( p_work->p_debug_strbuff_tmp );
      p_work->p_debug_strbuff_tmp = NULL;
    }
    // インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(p_work->p_field,
        FIELD_SUBSCREEN_Init( p_work->heapID, p_work->p_field, FIELD_SUBSCREEN_NORMAL ));

    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  メモリーダンプ表示
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void debugMenuWriteUseMemoryDump( DEBUG_USEMEMORY_EVENT_WORK* p_wk )
{
  /* 1段目 */
  // 配置オブジェクト
#ifdef BMODEL_DEBUG_RESOURCE_MEMORY_SIZE

  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 1, FIELD_BMODEL_MAN_GetUseResourceMemorySize(), 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

#else // BMODEL_DEBUG_RESOURCE_MEMORY_SIZE

  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 1, 0, 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

#endif // BMODEL_DEBUG_RESOURCE_MEMORY_SIZE

  // 動作オブジェ
#ifdef DEBUG_MMDL_RESOURCE_MEMORY_SIZE

  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 3, DEBUG_MMDL_GetUseResourceMemorySize(), 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

#else // DEBUG_MMDL_RESOURCE_MEMORY_SIZE

  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 3, 0, 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

#endif  // DEBUG_MMDL_RESOURCE_MEMORY_SIZE

  // FIELDヒープの残り
  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 4, GFI_HEAP_GetHeapFreeSize(p_wk->heapID), 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 5, GFI_HEAP_GetHeapAllocatableSize(p_wk->heapID), 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );

  // 表示
  GFL_MSG_GetString( p_wk->p_debug_msgdata, DEBUG_FIELD_STR54, p_wk->p_debug_strbuff_tmp );

  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );

  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_wk->p_win ), 0, 0, p_wk->p_debug_strbuff, p_wk->p_debug_font );



  /* 2段目 */
  // HEAPID FIELD_PRBUF
  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 0, GFI_HEAP_GetHeapFreeSize(HEAPID_FIELD_PRBUF), 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 1, GFI_HEAP_GetHeapAllocatableSize(HEAPID_FIELD_PRBUF), 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  // HEAPID SUBSCREEN
  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 2, GFI_HEAP_GetHeapFreeSize(HEAPID_FIELD_SUBSCREEN), 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterHexNumber( p_wk->p_debug_wordset, 3, GFI_HEAP_GetHeapAllocatableSize(HEAPID_FIELD_SUBSCREEN), 8, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );


  // 表示
  GFL_MSG_GetString( p_wk->p_debug_msgdata, DEBUG_FIELD_STR68, p_wk->p_debug_strbuff_tmp );

  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_wk->p_win ), 96, 0, p_wk->p_debug_strbuff, p_wk->p_debug_font );



  GFL_BMPWIN_TransVramCharacter( p_wk->p_win );
}


//----------------------------------------------------------------------------
/**
 * @brief フェード速度変更
 */
//----------------------------------------------------------------------------
static BOOL debugMenuCallProc_FadeSpeedChange( DEBUG_MENU_EVENT_WORK * wk )
{
  if (GFL_FADE_GetFadeSpeed() != 2)
  {
    GFL_FADE_SetFadeSpeed( 2 );
  } else {
    GFL_FADE_SetDefaultFadeSpeed();
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  あなぬけのヒモ
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Ananukenohimo( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT*      parent = wk->gmEvent;
  GAMESYS_WORK* gsys = wk->gmSys;
  FIELDMAP_WORK* fieldmap = wk->fieldWork;

  GMEVENT_ChangeEvent( parent, EVENT_ChangeMapByAnanukenohimo( fieldmap, gsys ) );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  あなをほる
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Anawohoru( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT*      parent = wk->gmEvent;
  GAMESYS_WORK* gsys = wk->gmSys;

  GMEVENT_ChangeEvent( parent, EVENT_ChangeMapByAnawohoru( gsys ) );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  テレポート
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Teleport( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT*      parent = wk->gmEvent;
  GAMESYS_WORK* gsys = wk->gmSys;

  GMEVENT_ChangeEvent( parent, EVENT_ChangeMapByTeleport( gsys ) );
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  フィールド技：ダイビング
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Diving( DEBUG_MENU_EVENT_WORK *wk )
{
  if( FIELDMAP_GetMapControlType(wk->fieldWork) != FLDMAP_CTRLTYPE_GRID )
  {
    FIELD_PLAYER* p_fld_player = FIELDMAP_GetFieldPlayer( wk->fieldWork );

    // レール上ではこっち
    FIELD_PLAYER_SetRequest( p_fld_player, FIELD_PLAYER_REQBIT_DIVING );
    return FALSE;
  }
  SCRIPT_ChangeScript( wk->gmEvent, SCRID_HIDEN_DIVING_MENU, NULL, HEAPID_FIELDMAP );
  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 3Dデモ再生
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_Demo3d( DEBUG_MENU_EVENT_WORK* wk )
{
  GMEVENT*         parent = wk->gmEvent;
  GAMESYS_WORK*      gsys = wk->gmSys;
  FIELDMAP_WORK* fieldmap = wk->fieldWork;
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );

  GMEVENT_ChangeEvent( parent, DEBUG_EVENT_FLDMENU_Demo3DSelect( gsys, heap_id ) );

  return TRUE;
}

//--------------------------------------------------------------
/**
 * 強制セーブ
 *
 * @param   wk
 *
 * @retval  BOOL    TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_ForceSave( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  GMEVENT *event;
  FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
  GFL_MSGDATA *msgdata;
  FLDMSGWIN *fldmsgwin;

  msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_field_dat, heapID );

  fldmsgwin = FLDMSGWIN_Add(msgBG, msgdata, 1, 19, 30, 4);

  FLDMSGWIN_Print( fldmsgwin, 0, 0, DEBUG_FIELD_STR60_message );
  FLDMSGBG_AllPrint(msgBG);

  //セーブ
  GAMEDATA_Save(GAMESYSTEM_GetGameData(gsys));

  FLDMSGWIN_Delete(fldmsgwin);
  GFL_MSG_Delete(msgdata);

  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 着信音
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_RingTone( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GAMESYS_WORK * gsys = p_wk->gmSys;
  GAMEDATA* gamedata  = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND * fsnd  = GAMEDATA_GetFieldSound( gamedata );

  if ( PMSND_CheckPlaySE_byPlayerID( SEPLAYER_SE3 ) == FALSE ) {
    FSND_RequestTVTRingTone( fsnd );
  } else {
    FSND_StopTVTRingTone( fsnd );
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * パソコン
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_PCScript( DEBUG_MENU_EVENT_WORK *wk )
{
  SCRIPT_ChangeScript( wk->gmEvent, SCRID_PC, NULL, HEAPID_FIELDMAP );
  return TRUE;
}

//======================================================================
//  デバッグメニュー　キャプチャ
//======================================================================
//--------------------------------------------------------------
/// DEBUG_CAPTURELIST_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
  int select_mode;
}DEBUG_CAPTURELIST_EVENT_WORK;


//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuCaptureListEvent(
    GMEVENT *event, int *seq, void *work );

enum{
  CAPTURE_MODE_NORMAL,
  CAPTURE_MODE_PLAYER_OFF,
  CAPTURE_MODE_NPC_OFF,
};

///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_CaptureList =
{
  1,    //リスト項目数
  3,    //表示最大項目数
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

///てんきメニューリスト
static const FLDMENUFUNC_LIST DATA_CaptureMenuList[] =
{
  { DEBUG_FIELD_STR61_000, (void*)CAPTURE_MODE_NORMAL },
  { DEBUG_FIELD_STR61_001, (void*)CAPTURE_MODE_PLAYER_OFF },
  { DEBUG_FIELD_STR61_002, (void*)CAPTURE_MODE_NPC_OFF },
};

static const DEBUG_MENU_INITIALIZER DebugCaptureMenuListData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_CaptureMenuList),
  DATA_CaptureMenuList,
  &DATA_DebugMenuList_CaptureList,
  1, 1, 12, 11,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　天気リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_CaptureList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_CAPTURELIST_EVENT_WORK *work;

  GMEVENT_Change( event,
    debugMenuCaptureListEvent, sizeof(DEBUG_CAPTURELIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_CAPTURELIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：てんきリスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuCaptureListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_CAPTURELIST_EVENT_WORK *work = wk;
  MMDL *fmmdl;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugCaptureMenuListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){  //決定
        return GMEVENT_RES_FINISH;
      }

      work->select_mode = ret;
      (*seq)++;
    }
    break;
  case 2:
    switch(work->select_mode){
    case CAPTURE_MODE_NORMAL:
      break;
    case CAPTURE_MODE_PLAYER_OFF:
      {
        FIELD_PLAYER *player = FIELDMAP_GetFieldPlayer( work->fieldWork );
        fmmdl = FIELD_PLAYER_GetMMdl( player );
        MMDL_SetStatusBitVanish( fmmdl, TRUE );
      }
      break;
    case CAPTURE_MODE_NPC_OFF:
      {
        u32 no = 0;
        BOOL ret;
        MMDLSYS *fos = FIELDMAP_GetMMdlSys( work->fieldWork );

        do{
          ret = MMDLSYS_SearchUseMMdl(fos, &fmmdl, &no );
          if(ret == TRUE){
            MMDL_SetStatusBitVanish( fmmdl, TRUE );
          }
        }while(ret == TRUE);
      }
      break;
    }
    (*seq)++;
    break;
  case 3:
    MI_CpuClearFast( (void*)HW_LCDC_VRAM_D, HW_VRAM_D_SIZE );
    GX_SetCapture(
        GX_CAPTURE_SIZE_256x192,      // キャプチャサイズ
        GX_CAPTURE_MODE_A,      // キャプチャモード
        GX_CAPTURE_SRCA_3D,       // キャプチャブレンドA
        GX_CAPTURE_SRCB_VRAM_0x00000,       // キャプチャブレンドB
        GX_CAPTURE_DEST_VRAM_D_0x00000,     // 転送Vram
        0,            // ブレンド係数A
        0);           // ブレンド係数B

    //LCDCにメモリマッピング
    GX_SetBankForLCDC(GX_VRAM_LCDC_D);
  //  GX_SetGraphicsMode(GX_DISPMODE_VRAM_C, GX_BGMODE_0, GX_BG0_AS_3D);

    return GMEVENT_RES_FINISH;
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * @brief   移動ポケモンデバッグ
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_DebugMvPokemon( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
    FS_OVERLAY_ID( d_iwasawa ), EVENT_DebugMovePoke, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE );
}



//-----------------------------------------------------------------------------
// ビルボードカラー　の調整
//-----------------------------------------------------------------------------

//--------------------------------------------------------------
/// DEBUG_CTLIGHT_WORK ライト操作ワーク
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  GMEVENT *event;
  HEAPID heapID;
  FIELDMAP_WORK *fieldWork;

  GFL_BMPWIN* p_win;
}DEBUG_BBDCOLOR_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlBbdColor(
    GMEVENT *event, int *seq, void *wk );

//----------------------------------------------------------------------------
/**
 *  @brief  ビルボードカラーを指定
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_BBDColor( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_BBDCOLOR_WORK *work;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;

  GMEVENT_Change( event, debugMenuControlBbdColor, sizeof(DEBUG_BBDCOLOR_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BBDCOLOR_WORK) );

  work->gsys = gsys;
  work->event = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}


static GMEVENT_RESULT debugMenuControlBbdColor(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BBDCOLOR_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    {
      HEAPID heapID = FIELDMAP_GetHeapID( work->fieldWork );
      FLD_BBD_COLOR* p_color = FLD_BBD_COLOR_Create( heapID );
      GFL_BBDACT_SYS* p_bbd_act = FIELDMAP_GetBbdActSys( work->fieldWork );
      GFL_BBD_SYS* p_bbd_sys = GFL_BBDACT_GetBBDSystem( p_bbd_act );
      AREADATA* p_areadata = FIELDMAP_GetAreaData( work->fieldWork );

      FLD_BBD_COLOR_Load( p_color, AREADATA_GetBBDColor(p_areadata) );

      // モデル管理開始
      FLD_BBD_COLOR_DEBUG_Init( p_bbd_sys, p_color, work->heapID );

      FLD_BBD_COLOR_Delete( p_color );
    }

    // インフォーバーの非表示
    FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(work->fieldWork));
    GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

    // ビットマップウィンドウ初期化
    {
      static const GFL_BG_BGCNT_HEADER header_sub3 = {
        0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
        GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
      };

      GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
      GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
      GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

      // パレット情報を転送
      GFL_ARC_UTIL_TransVramPalette(
        ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, work->heapID );

      // ビットマップウィンドウを作成
      work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
        1, 1, 30, 22,
        FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_B );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
      GFL_BMPWIN_MakeScreen( work->p_win );
      GFL_BMPWIN_TransVramCharacter( work->p_win );
      GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );

      // ウィンドウ
      BmpWinFrame_GraphicSet( FIELD_SUBSCREEN_BGPLANE, 1, 15, 0, work->heapID );
      BmpWinFrame_Write( work->p_win, TRUE, 1, 15 );
    }

    FLD_BBD_COLOR_DEBUG_PrintData( work->p_win );

    (*seq)++;
  case 1:
    // ライト管理メイン
    FLD_BBD_COLOR_DEBUG_Control();
    FLD_BBD_COLOR_DEBUG_PrintData( work->p_win );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      (*seq)++;
    }
    break;
  case 2:
    FLD_BBD_COLOR_DEBUG_Exit();

    // ビットマップウィンドウ破棄
    {
      GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
      GFL_BMPWIN_Delete( work->p_win );
    }

    // インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(work->fieldWork,
        FIELD_SUBSCREEN_Init( work->heapID, work->fieldWork, FIELD_SUBSCREEN_NORMAL ));

    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}


//======================================================================
//  デバッグメニュー　エンカウントエフェクトリスト
//======================================================================
//--------------------------------------------------------------
/// DEBUG_ENCEFF_LIST_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DEBUG_ENCEFF_LIST_EVENT_WORK;

///リスト最大
#define ENCEFFLISTMAX (37)

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_EncEffList =
{
  1,    //リスト項目数
  6,    //表示最大項目数
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

///メニューリスト
static const FLDMENUFUNC_LIST DATA_EncEffMenuList[ENCEFFLISTMAX] =
{
  { DEBUG_FIELD_ENCEFF01, (void*)0 },
  { DEBUG_FIELD_ENCEFF02, (void*)1 },
  { DEBUG_FIELD_ENCEFF03, (void*)2 },
  { DEBUG_FIELD_ENCEFF04, (void*)3 },
  { DEBUG_FIELD_ENCEFF05, (void*)4 },
  { DEBUG_FIELD_ENCEFF06, (void*)5 },
  { DEBUG_FIELD_ENCEFF07, (void*)6 },
  { DEBUG_FIELD_ENCEFF08, (void*)7 },
  { DEBUG_FIELD_ENCEFF09, (void*)8 },
  { DEBUG_FIELD_ENCEFF10, (void*)9 },
  { DEBUG_FIELD_ENCEFF11, (void*)10 },
  { DEBUG_FIELD_ENCEFF12, (void*)11 },
  { DEBUG_FIELD_ENCEFF13, (void*)12 },
  { DEBUG_FIELD_ENCEFF14, (void*)13 },
  { DEBUG_FIELD_ENCEFF15, (void*)14 },
  { DEBUG_FIELD_ENCEFF16, (void*)15 },
  { DEBUG_FIELD_ENCEFF17, (void*)16 },
  { DEBUG_FIELD_ENCEFF18, (void*)17 },
  { DEBUG_FIELD_ENCEFF19, (void*)18 },
  { DEBUG_FIELD_ENCEFF20, (void*)19 },
  { DEBUG_FIELD_ENCEFF21, (void*)20 },
  { DEBUG_FIELD_ENCEFF22, (void*)21 },
  { DEBUG_FIELD_ENCEFF23, (void*)22 },
  { DEBUG_FIELD_ENCEFF24, (void*)23 },
  { DEBUG_FIELD_ENCEFF25, (void*)24 },
  { DEBUG_FIELD_ENCEFF26, (void*)25 },
  { DEBUG_FIELD_ENCEFF27, (void*)26 },
  { DEBUG_FIELD_ENCEFF28, (void*)27 },
  { DEBUG_FIELD_ENCEFF30, (void*)28 },
  { DEBUG_FIELD_ENCEFF31, (void*)29 },
  { DEBUG_FIELD_ENCEFF29, (void*)30 },
  { DEBUG_FIELD_ENCEFF32, (void*)31 },
  { DEBUG_FIELD_ENCEFF33, (void*)32 },
  { DEBUG_FIELD_ENCEFF34, (void*)33 },
  { DEBUG_FIELD_ENCEFF35, (void*)34 },
  { DEBUG_FIELD_ENCEFF36, (void*)35 },
  { DEBUG_FIELD_EFF_PALACE, (void*)36 },
};

static const DEBUG_MENU_INITIALIZER DebugEncEffMenuListData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_EncEffMenuList),
  DATA_EncEffMenuList,
  &DATA_DebugMenuList_EncEffList, //流用
  1, 1, 16, 11,
  NULL,
  NULL
};


//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuEncEffListEvent( GMEVENT *event, int *seq, void *wk );


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　エンカウントエフェクト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_EncEffList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_ENCEFF_LIST_EVENT_WORK *work;

  GMEVENT_Change( event,
    debugMenuEncEffListEvent, sizeof(DEBUG_ENCEFF_LIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_ENCEFF_LIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：エンカウントエフェクトリスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuEncEffListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ENCEFF_LIST_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugEncEffMenuListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret != FLDMENUFUNC_CANCEL ) //決定
      {
        //エンカウントエフェクトコール
        OS_Printf("enceff= %d\n",ret);
        ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(work->fieldWork), event, ret);
        (*seq)++;
        return( GMEVENT_RES_CONTINUE );
      }
      else return( GMEVENT_RES_FINISH );
    }
    break;
  case 2:
    {
      ENCEFF_CNT_PTR cnt_ptr = FIELDMAP_GetEncEffCntPtr(work->fieldWork);
      ENCEFF_FreeUserWork(cnt_ptr);
      //オーバーレイアンロード
      ENCEFF_UnloadEffOverlay(cnt_ptr);
    }

    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, -1 );
    (*seq)++;
    break;
  case 3:
    if( GFL_FADE_CheckFade() == FALSE ){
      return( GMEVENT_RES_FINISH );
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  デバッグメニュー 国連データ作成
//======================================================================
#include "debug/debug_make_undata.h"
FS_EXTERN_OVERLAY(debug_make_undata);
static GMEVENT_RESULT debugMenuMakeUNData( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//-------------------------------------
/// デバッグ国連データ作成用ワーク
//=====================================
typedef struct
{
  HEAPID HeapID;
  GAMESYS_WORK    *gsys;
  GMEVENT         *Event;
  FIELDMAP_WORK *FieldWork;
  PROCPARAM_DEBUG_MAKE_UNDATA p_work;
} DEBUG_MAKE_UNDATA_EVENT_WORK;

//----------------------------------------------------------------------------
/**
 *  @brief  国連データ作成
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugMakeUNData( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *gsys  = p_wk->gmSys;
  GMEVENT       *event    = p_wk->gmEvent;
  FIELDMAP_WORK *fieldWork  = p_wk->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_MAKE_UNDATA_EVENT_WORK *evt_work;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));

  //イベントチェンジ
  GMEVENT_Change( event, debugMenuMakeUNData, sizeof(DEBUG_MAKE_UNDATA_EVENT_WORK) );
  evt_work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( evt_work, sizeof(DEBUG_MAKE_UNDATA_EVENT_WORK) );

  //ワーク設定
  evt_work->gsys = gsys;
  evt_work->Event = event;
  evt_work->FieldWork = fieldWork;
  evt_work->HeapID = heapID;
  evt_work->p_work.wh = SaveData_GetWifiHistory(pSave);

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグ国連データ作成イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuMakeUNData( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
  };

  DEBUG_MAKE_UNDATA_EVENT_WORK *evt_work = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( evt_work->Event, EVENT_FieldSubProc( evt_work->gsys, evt_work->FieldWork,
        FS_OVERLAY_ID(debug_make_undata), &ProcData_DebugMakeUNData, &evt_work->p_work ) );
    *p_seq  = SEQ_PROC_END;
    break;
  case SEQ_PROC_END:
    return GMEVENT_RES_FINISH;
    break;
  }

  return GMEVENT_RES_CONTINUE ;
}

//======================================================================
//  デバッグメニュー　バトルサブウェイ
//======================================================================
#include "../../../resource/fldmapdata/script/bsubway_scr_def.h"

//--------------------------------------------------------------
/// DEBUG_BSUBWAY_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DEBUG_BSUBWAY_EVENT_WORK;

static const FLDMENUFUNC_HEADER DATA_DebugMenuList_BSubway =
{
  1,    //リスト項目数
  6,    //表示最大項目数
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

enum
{
  DEBUG_BSWAY_ZONE_HOME,
  DEBUG_BSWAY_ZONE_TRAIN,

  DEBUG_BSWAY_SCDATA_VIEW,
  DEBUG_BSWAY_WIFI_GAMEDATA_DOWNLOAD,
  DEBUG_BSWAY_WIFI_RIREKI_DOWNLOAD,
  DEBUG_BSWAY_WIFI_UPLOAD,

  DEBUG_BSWAY_ZONE_SINGLE,
  DEBUG_BSWAY_ZONE_DOUBLE,
  DEBUG_BSWAY_ZONE_MULTI,
  DEBUG_BSWAY_ZONE_WIFI,
  DEBUG_BSWAY_ZONE_S_SINGLE,
  DEBUG_BSWAY_ZONE_S_DOUBLE,
  DEBUG_BSWAY_ZONE_S_MULTI,
  DEBUG_BSWAY_AUTO_SINGLE,
  DEBUG_BSWAY_AUTO_DOUBLE,
  DEBUG_BSWAY_AUTO_MULTI,
  DEBUG_BSWAY_AUTO_S_SINGLE,
  DEBUG_BSWAY_AUTO_S_DOUBLE,
  DEBUG_BSWAY_AUTO_S_MULTI,
  DEBUG_BSWAY_BTL_SINGLE_7,
  DEBUG_BSWAY_BTL_DOUBLE_7,
  DEBUG_BSWAY_BTL_MULTI_7,
  DEBUG_BSWAY_BTL_SINGLE_21,
  DEBUG_BSWAY_BTL_DOUBLE_21,
  DEBUG_BSWAY_BTL_MULTI_21,
  DEBUG_BSWAY_BTL_S_SINGLE_49,
  DEBUG_BSWAY_BTL_S_DOUBLE_49,
  DEBUG_BSWAY_BTL_S_MULTI_49,

  DEBUG_BSWAY_SET_REGU_OFF,
  DEBUG_BSWAY_SET_BTL_SKIP,
  DEBUG_BSWAY_ANYSTAGE,
};

static const FLDMENUFUNC_LIST DATA_BSubwayMenuList[] =
{
  { DEBUG_FIELD_BSW_01, (void*)DEBUG_BSWAY_ZONE_SINGLE},
  { DEBUG_FIELD_BSW_02, (void*)DEBUG_BSWAY_ZONE_DOUBLE},
  { DEBUG_FIELD_BSW_03, (void*)DEBUG_BSWAY_ZONE_MULTI},
  { DEBUG_FIELD_BSW_04, (void*)DEBUG_BSWAY_ZONE_WIFI},
  { DEBUG_FIELD_BSW_05, (void*)DEBUG_BSWAY_ZONE_S_SINGLE},
  { DEBUG_FIELD_BSW_06, (void*)DEBUG_BSWAY_ZONE_S_DOUBLE},
  { DEBUG_FIELD_BSW_07, (void*)DEBUG_BSWAY_ZONE_S_MULTI},
  { DEBUG_FIELD_BSW_08, (void*)DEBUG_BSWAY_ZONE_TRAIN},
  { DEBUG_FIELD_BSW_09, (void*)DEBUG_BSWAY_ZONE_HOME},

  { DEBUG_FIELD_BSW_10, (void*)DEBUG_BSWAY_SCDATA_VIEW },
  { DEBUG_FIELD_WIFI_BSW_01, (void*)DEBUG_BSWAY_WIFI_GAMEDATA_DOWNLOAD },
  { DEBUG_FIELD_WIFI_BSW_02, (void*)DEBUG_BSWAY_WIFI_RIREKI_DOWNLOAD },
  { DEBUG_FIELD_WIFI_BSW_03, (void*)DEBUG_BSWAY_WIFI_UPLOAD },

  { DEBUG_FIELD_BSW_26, (void*)DEBUG_BSWAY_SET_REGU_OFF},
  { DEBUG_FIELD_BSW_23, (void*)DEBUG_BSWAY_SET_BTL_SKIP},

  { DEBUG_FIELD_BSW_11, (void*)DEBUG_BSWAY_AUTO_SINGLE},
  { DEBUG_FIELD_BSW_12, (void*)DEBUG_BSWAY_AUTO_DOUBLE},
  { DEBUG_FIELD_BSW_13, (void*)DEBUG_BSWAY_AUTO_MULTI},
  { DEBUG_FIELD_BSW_14, (void*)DEBUG_BSWAY_AUTO_S_SINGLE},
  { DEBUG_FIELD_BSW_15, (void*)DEBUG_BSWAY_AUTO_S_DOUBLE},
  { DEBUG_FIELD_BSW_16, (void*)DEBUG_BSWAY_AUTO_S_MULTI},
  { DEBUG_FIELD_BSW_17, (void*)DEBUG_BSWAY_BTL_SINGLE_7},
  { DEBUG_FIELD_BSW_18, (void*)DEBUG_BSWAY_BTL_DOUBLE_7},
  { DEBUG_FIELD_BSW_19, (void*)DEBUG_BSWAY_BTL_MULTI_7},
  { DEBUG_FIELD_BSW_20, (void*)DEBUG_BSWAY_BTL_SINGLE_21},
  { DEBUG_FIELD_BSW_21, (void*)DEBUG_BSWAY_BTL_DOUBLE_21},
  { DEBUG_FIELD_BSW_22, (void*)DEBUG_BSWAY_BTL_MULTI_21},
  { DEBUG_FIELD_BSW_27, (void*)DEBUG_BSWAY_BTL_S_SINGLE_49},
  { DEBUG_FIELD_BSW_28, (void*)DEBUG_BSWAY_BTL_S_DOUBLE_49},
  { DEBUG_FIELD_BSW_29, (void*)DEBUG_BSWAY_BTL_S_MULTI_49},
  { DEBUG_FIELD_BSW_30, (void*)DEBUG_BSWAY_ANYSTAGE},
};

#define DEBUG_BSUBWAY_LIST_MAX ( NELEMS(DATA_BSubwayMenuList) )

static const DEBUG_MENU_INITIALIZER DebugBSubwayMenuData = {
  NARC_message_d_field_dat,
  DEBUG_BSUBWAY_LIST_MAX,
  DATA_BSubwayMenuList,
  &DATA_DebugMenuList_BSubway,
  1, 1, 20, 12,
  NULL,
  NULL
};

static const FLDMENUFUNC_LIST DATA_BSubwayAnyStageMenuList[] =
{
  { DEBUG_FIELD_BSW_31, (void*)BSWAY_MODE_SINGLE },
  { DEBUG_FIELD_BSW_32, (void*)BSWAY_MODE_DOUBLE },
  { DEBUG_FIELD_BSW_33, (void*)BSWAY_MODE_MULTI },
  { DEBUG_FIELD_BSW_34, (void*)BSWAY_MODE_S_SINGLE },
  { DEBUG_FIELD_BSW_35, (void*)BSWAY_MODE_S_DOUBLE },
  { DEBUG_FIELD_BSW_36, (void*)BSWAY_MODE_S_MULTI },
};

#define DEBUG_BSUBWAY_ANYSTAGE_LIST_MAX ( NELEMS(DATA_BSubwayAnyStageMenuList) )

static const DEBUG_MENU_INITIALIZER DebugBSubwayAnyStageMenuData = {
  NARC_message_d_field_dat,
  DEBUG_BSUBWAY_ANYSTAGE_LIST_MAX,
  DATA_BSubwayAnyStageMenuList,
  &DATA_DebugMenuList_BSubway,
  1, 1, 20, 12,
  NULL,
  NULL
};


static void debug_bsw_SetAuto( GAMESYS_WORK *gsys )
{
  u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( gsys );
  flag |= BSW_DEBUG_FLAG_AUTO;
  BSUBWAY_SCRWORK_DebugSetFlag( gsys, flag );
}

typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;

  int play_mode;
  int game_round;
  int before_round;
  FLDMSGWIN *msgWin;
  STRBUF *strBuf;

  int key_repeat;
  int key_repeat_wait;
}DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK;

#define PAD_KEY_ALL (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT)

//--------------------------------------------------------------
/**
 * イベント：バトルサブウェイデバッグメニュー派生　任意のステージに
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuBSubwayAnyStageEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK  *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init(
        work->fieldWork, work->heapID,  &DebugBSubwayAnyStageMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret,param;
      u16 dummy_ret;
      GMEVENT *next_event = NULL;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        GFL_STR_DeleteBuffer( work->strBuf );
        return( GMEVENT_RES_FINISH );
      }

      work->play_mode = ret;
      (*seq)++;
      break;
    }
  case 2:
    {
      FLDMSGBG * msgbg = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->msgWin = FLDMSGWIN_Add( msgbg, NULL, 1, 1, 8, 2 );
      work->game_round = 1;
      work->before_round = 0xffff;
    }
    (*seq)++;
    break;
  case 3:
    {
      int add1 = 0, add10 = 0;
      int trg = GFL_UI_KEY_GetTrg();
      int cont = GFL_UI_KEY_GetCont();

      if( trg & PAD_BUTTON_B ){
        FLDMSGWIN_Delete( work->msgWin );
        GFL_STR_DeleteBuffer( work->strBuf );
        return( GMEVENT_RES_FINISH );
      }

      if( trg & PAD_BUTTON_A ){
        FLDMSGWIN_Delete( work->msgWin );
        GFL_STR_DeleteBuffer( work->strBuf );

        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, work->play_mode );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, work->game_round );

        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      }

      if( cont && cont == work->key_repeat ){
        if( work->key_repeat_wait < 30*10 ){
          work->key_repeat_wait++;
        }
      }else{
        work->key_repeat_wait = 0;
      }

      work->key_repeat = cont;

      if( (trg & PAD_KEY_ALL) ){
        add1 = 1;
        add10 = 10;
      }else if( work->key_repeat ){
        int rep = work->key_repeat;
        int wait = work->key_repeat_wait;

        if( wait < 15 ){
          //none
        }else if( wait < 30*3 ){
          if( wait % 6 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*4 ){
          if( wait % 4 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*5 ){
          if( wait % 2 == 0 ){
            add1 = 1;
            add10 = 10;
          }
        }else if( wait < 30*6 ){
          add1 = 1;
          add10 = 10;
        }else if( wait < 30*7 ){
          add1 = 2;
          add10 = 20;
        }else if( wait < 30*8 ){
          add1 = 4;
          add10 = 40;
        }else if( wait < 30*9 ){
          add1 = 8;
          add10 = 80;
        }else if( wait >= 30*9 ){
          add1 = 16;
          add10 = 160;
        }
      }

      if( add1 || add10 ){
        int check = trg & PAD_KEY_ALL;

        if( check == 0 ){
          check = cont;
        }

        if( (check & PAD_KEY_UP) ){
          work->game_round -= add1;
        }else if( (check & PAD_KEY_DOWN) ){
          work->game_round += add1;
        }else if( (check & PAD_KEY_LEFT) ){
          work->game_round -= add10;
        }else if( (check & PAD_KEY_RIGHT) ){
          work->game_round += add10;
        }
      }

      if( work->game_round <= 0 ){
        work->game_round = 1;
      }else if( work->game_round > 99999 ){
        work->game_round = 99999;
      }

      switch( work->play_mode ){
      case BSWAY_MODE_SINGLE:
      case BSWAY_MODE_DOUBLE:
      case BSWAY_MODE_MULTI:
        if( work->game_round > 21 ){
          work->game_round = 21;
        }
      }

      if( work->game_round != work->before_round ){
        work->before_round = work->game_round;
        STRTOOL_SetNumber(
            work->strBuf, work->game_round, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT  );
        FLDMSGWIN_ClearWindow( work->msgWin );
        FLDMSGWIN_PrintStrBuf( work->msgWin, 16, 1, work->strBuf );
      }
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void debugMenuCallProc_BSubwayAnyStage(
    GMEVENT *event, DEBUG_BSUBWAY_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK *work;

  GMEVENT_Change( event, debugMenuBSubwayAnyStageEvent,
      sizeof(DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BSUBWAY_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  work->strBuf = GFL_STR_CreateBuffer( 32, work->heapID );
}

//--------------------------------------------------------------
/**
 * イベント：バトルサブウェイデバッグメニュー
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuBSubwayEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSUBWAY_EVENT_WORK  *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init(
        work->fieldWork, work->heapID,  &DebugBSubwayMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret,param;
      u16 dummy_ret;
      VecFx32 receipt_map_pos = {GRID_SIZE_FX32(9),0,GRID_SIZE_FX32(12)};
      VecFx32 home_map_pos = {GRID_SIZE_FX32(27),0,GRID_SIZE_FX32(15)};
      GMEVENT *next_event = NULL;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }

      switch( ret ){
      case DEBUG_BSWAY_ZONE_HOME: //ホームへ移動
        param = ZONE_ID_C04R0111;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
          work->gmSys, work->fieldWork, param, &home_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_TRAIN: //車内へ移動
        param = ZONE_ID_C04R0110;
        next_event = DEBUG_EVENT_QuickChangeMapDefaultPos(
          work->gmSys, work->fieldWork, param );
        break;
      case DEBUG_BSWAY_SCDATA_VIEW: //歴代リーダー表示
        next_event = BSUBWAY_EVENT_CallLeaderBoard( work->gmSys );
        break;
      case DEBUG_BSWAY_WIFI_GAMEDATA_DOWNLOAD: //wifi データダウンロード
        next_event = WIFI_BSUBWAY_EVENT_Start( work->gmSys,
            WIFI_BSUBWAY_MODE_GAMEDATA_DOWNLOAD, &dummy_ret );
        break;
      case DEBUG_BSWAY_WIFI_RIREKI_DOWNLOAD:  //wif 履歴ダウンロード
        next_event = WIFI_BSUBWAY_EVENT_Start( work->gmSys,
            WIFI_BSUBWAY_MODE_SUCCESSDATA_DOWNLOAD, &dummy_ret );
        break;
      case DEBUG_BSWAY_WIFI_UPLOAD: //wifi データアップロード
        next_event = WIFI_BSUBWAY_EVENT_Start( work->gmSys,
            WIFI_BSUBWAY_MODE_SCORE_UPLOAD, &dummy_ret );
        break;
      case DEBUG_BSWAY_ZONE_SINGLE: //シングル受付へ
        param = ZONE_ID_C04R0102;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
          work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_DOUBLE: //ダブル受付へ
        param = ZONE_ID_C04R0104;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
          work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_MULTI: //マルチ受付へ
        param = ZONE_ID_C04R0106;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
          work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_WIFI: //WiFi受付へ
        param = ZONE_ID_C04R0108;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
          work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_S_SINGLE: //Sシングル受付へ
        param = ZONE_ID_C04R0103;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
          work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_S_DOUBLE: //Sダブル受付へ
        param = ZONE_ID_C04R0105;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
          work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_ZONE_S_MULTI: //Sマルチ受付へ
        param = ZONE_ID_C04R0107;
        next_event = DEBUG_EVENT_QuickChangeMapAppoint(
          work->gmSys, work->fieldWork, param, &receipt_map_pos );
        break;
      case DEBUG_BSWAY_AUTO_SINGLE: //シングルオート戦闘
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
        debug_bsw_SetAuto( work->gmSys );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_AUTO_DOUBLE: //ダブルオート戦闘
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
        debug_bsw_SetAuto( work->gmSys );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_AUTO_MULTI: //マルチオート戦闘
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
        debug_bsw_SetAuto( work->gmSys );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_AUTO_S_SINGLE: //Sシングルオート戦闘
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_SINGLE );
        debug_bsw_SetAuto( work->gmSys );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_AUTO_S_DOUBLE: //Sダブルオート戦闘
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_DOUBLE );
        debug_bsw_SetAuto( work->gmSys );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_AUTO_S_MULTI: //Sマルチオート戦闘
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_MULTI );
        debug_bsw_SetAuto( work->gmSys );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_BTL_SINGLE_7: //シングル７戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_BTL_DOUBLE_7: //ダブル７戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_BTL_MULTI_7: //マルチ７戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_BTL_SINGLE_21: //シングル２１戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_BTL_DOUBLE_21: //ダブル２１戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_BTL_MULTI_21: //マルチ２１戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_SET_REGU_OFF: //レギュオフ
        {
          u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( work->gmSys );
          flag ^= BSW_DEBUG_FLAG_REGU_OFF;
          BSUBWAY_SCRWORK_DebugSetFlag( work->gmSys, flag );
        }
        break;
      case DEBUG_BSWAY_SET_BTL_SKIP: //バトルスキップ
        {
          u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( work->gmSys );
          flag ^= BSW_DEBUG_FLAG_BTL_SKIP;
          BSUBWAY_SCRWORK_DebugSetFlag( work->gmSys, flag );
        }
        break;
      case DEBUG_BSWAY_BTL_S_SINGLE_49: //Ｓシングル４９戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_SINGLE );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_BTL_S_DOUBLE_49: //Ｓダブル４９戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_DOUBLE );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_BTL_S_MULTI_49: //Ｓマルチ４９戦から
        BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_MULTI );
        BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
        SCRIPT_ChangeScript(
            event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        return( GMEVENT_RES_CONTINUE );
      case DEBUG_BSWAY_ANYSTAGE: //任意ステージ
        debugMenuCallProc_BSubwayAnyStage( event, work );
        return( GMEVENT_RES_CONTINUE );
      default:
        break;
      }

      if( next_event == NULL ){
        return( GMEVENT_RES_FINISH );
      }else{
        GMEVENT_CallEvent( event, next_event );
        (*seq)++;
      }
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_BSubway( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_BSUBWAY_EVENT_WORK  *work;

  GMEVENT_Change( event,
    debugMenuBSubwayEvent, sizeof(DEBUG_BSUBWAY_EVENT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BSUBWAY_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}

//======================================================================
//  デバッグメニュー　Gパワー
//======================================================================
#include "gamesystem/g_power.h"
#include "msg/msg_d_matsu.h"
//--------------------------------------------------------------
/// DEBUG_GPOWER_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;

  POWER_CONV_DATA *powerdata;
}DEBUG_GPOWER_EVENT_WORK;


//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuGPowerListEvent(
    GMEVENT *event, int *seq, void *work );

///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_GPowerList =
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
  16,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};

static const DEBUG_MENU_INITIALIZER DebugGPowerMenuListData = {
  NARC_message_power_dat,
  NULL,
  NULL,
  &DATA_DebugMenuList_GPowerList,
  1, 1, 24, 19,
  DEBUG_SetMenuWorkGPower,
  DEBUG_GetGPowerMax,
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　天気リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_GPowerList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_GPOWER_EVENT_WORK *work;

  GMEVENT_Change( event,
    debugMenuGPowerListEvent, sizeof(DEBUG_GPOWER_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_GPOWER_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：Gパワーリスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuGPowerListEvent(GMEVENT *event, int *seq, void *wk )
{
  DEBUG_GPOWER_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugGPowerMenuListData );
    work->powerdata = GPOWER_PowerData_LoadAlloc(work->heapID);
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      if(ret == FLDMENUFUNC_CANCEL){
        (*seq)++;
        break;
      }

      if(ret < GPOWER_ID_MAX){
        GPOWER_Set_OccurID(ret, work->powerdata, FALSE);
      }
      else{
        GPOWER_Clear_AllPower();
      }
    }
    break;
  case 2:
    FLDMENUFUNC_DeleteMenu( work->menuFunc );
    GPOWER_PowerData_Unload(work->powerdata);
    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * Gパワー用BMP_MENULIST_DATAセット
 * @param list  セット先BMP_MENULIST_DATA
 * @param heapID  文字列バッファ確保用HEAPID
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DEBUG_SetMenuWorkGPower(GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list,
  HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int id;
  STRBUF *strBuf = GFL_STR_CreateBuffer( 64, heapID );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMSGBG * msgBG = FIELDMAP_GetFldMsgBG( fieldmap );
  GFL_MSGDATA * pMsgDataDMatsu = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_d_matsu_dat );

  for( id = 0; id < GPOWER_ID_MAX; id++ ){
    GFL_MSG_GetString( msgData,  id, strBuf );
    FLDMENUFUNC_AddStringListData( list, strBuf, id, heapID );
  }
  GFL_MSG_GetString(pMsgDataDMatsu, DM_MSG_POWER_RESET, strBuf);
  FLDMENUFUNC_AddStringListData( list, strBuf, GPOWER_ID_MAX, heapID );

  GFL_MSG_Delete( pMsgDataDMatsu );
  GFL_HEAP_FreeMemory( strBuf );
}

//--------------------------------------------------------------
/**
 * @brief Gパワー用リスト最大値取得関数
 * @param fieldmap
 * @return  マップ最大数
 */
//--------------------------------------------------------------
static u16 DEBUG_GetGPowerMax( GAMESYS_WORK* gsys, void* cb_work )
{
  return GPOWER_ID_MAX + 1; // +1 = 「全てリセット」
}

//======================================================================
//  デバッグメニュー　フィールドわざ
//======================================================================
//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuFieldSkillListEvent(GMEVENT *event, int *seq, void *wk );

static const FLDMENUFUNC_LIST DATA_SubFieldSkillList[] =
{
  { DEBUG_FIELD_STR31, debugMenuCallProc_Naminori },              //波乗り
  { DEBUG_FIELD_STR45, debugMenuCallProc_Kairiki },               //怪力
  { DEBUG_FIELD_ANANUKENOHIMO, debugMenuCallProc_Ananukenohimo }, //穴ヌケの紐
  { DEBUG_FIELD_ANAWOHORU, debugMenuCallProc_Anawohoru },         //穴を掘る
  { DEBUG_FIELD_TELEPORT, debugMenuCallProc_Teleport },           //テレポート
  { DEBUG_FIELD_FSKILL_01, debugMenuCallProc_Diving },            //ダイビング
};

static const DEBUG_MENU_INITIALIZER DebugSubFieldSkillListSelectData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_SubFieldSkillList),
  DATA_SubFieldSkillList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　フィールド技リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_FieldSkillList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  DEBUG_MENU_EVENT_WORK   temp  = *wk;
  DEBUG_MENU_EVENT_WORK   *work;

  GMEVENT_Change( event,
    debugMenuFieldSkillListEvent, sizeof(DEBUG_MENU_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MENU_EVENT_WORK) );

  *work  = temp;
  work->call_proc = NULL;

  work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugSubFieldSkillListSelectData );

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：フィールド技リスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuFieldSkillListEvent(GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MENU_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }else if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        (*seq)++;
      }else if( ret != FLDMENUFUNC_CANCEL ){  //決定
        work->call_proc = (DEBUG_MENU_CALLPROC)ret;
        (*seq)++;
      }
    }
    break;

  case 2:
    {
      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( work->call_proc != NULL ){
        if( work->call_proc(work) == TRUE ){
          return( GMEVENT_RES_CONTINUE );
        }
      }

      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  デバッグメニュー　ふしぎなおくりもの作成
//======================================================================
//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
//#define MYSTERY_DLDATA_DEBUG_PRINT
static const FLDMENUFUNC_LIST DATA_SubMysteryCardMakeList[] =
{
  { DEBUG_FIELD_MYSTERY_01, debugMenuCallProc_MakeMysteryCardPoke },              //ポケモン作成
  { DEBUG_FIELD_MYSTERY_02, debugMenuCallProc_MakeMysteryCardItem },               //道具作成
  { DEBUG_FIELD_MYSTERY_03, debugMenuCallProc_MakeMysteryCardGPower },              //Gパワー作成
  { DEBUG_FIELD_MYSTERY_04, debugMenuCallProc_MakeMysteryCardGLiberty },              //リバティ作成
  { DEBUG_FIELD_MYSTERY_05, debugMenuCallProc_MakeMysteryCardEgg },              //タマゴ作成
};

static const DEBUG_MENU_INITIALIZER DebugSubMysteryCardMakeData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_SubMysteryCardMakeList),
  DATA_SubMysteryCardMakeList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};


static BOOL debugMenuCallProc_MakeMysteryCardList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event = wk->gmEvent;
  DEBUG_MENU_EVENT_WORK   temp  = *wk;
  DEBUG_MENU_EVENT_WORK   *work;

  //イベント流用
  GMEVENT_Change( event,
    debugMenuFieldSkillListEvent, sizeof(DEBUG_MENU_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MENU_EVENT_WORK) );

  *work  = temp;
  work->call_proc = NULL;

  work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugSubMysteryCardMakeData );

  return( TRUE );
}

static BOOL debugMenuCallProc_MakeMysteryCardPoke( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL,GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );

  for( i = 1; i < 2048; i++ )
  {
    if( !MYSTERYDATA_IsEventRecvFlag(p_mystery_sv, i) )
    {
      DEBUG_MYSTERY_SetGiftPokeData( &data );
      DEBUG_MYSTERY_SetGiftCommonData( &data, i, FALSE );
      MYSTERYDATA_SetCardData( p_mystery_sv, &data );

      DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
      dl_data.data  = data;
#ifdef MYSTERY_DLDATA_DEBUG_PRINT
      {
        int j;
        const u8 * cp_data  = (const u8*)&dl_data;
        for( j = 0; j < sizeof(DOWNLOAD_GIFT_DATA); j++ )
        {
          OS_TFPrintf( 3, "0x%x ", cp_data[j] );
          if( j % 0x10 == 0xF )
          {
            OS_TFPrintf( 3, "\n" );
          }
        }
      }
#endif
      OS_TPrintf( "ふしぎなカードをセットしました イベントID=[%d]\n", i );
      break;
    }
  }

  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}
static BOOL debugMenuCallProc_MakeMysteryCardItem( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL,GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );
  for( i = 1; i < 2048; i++ )
  {
    if( !MYSTERYDATA_IsEventRecvFlag(p_mystery_sv, i) )
    {
      DEBUG_MYSTERY_SetGiftItemData( &data, ITEM_MONSUTAABOORU );
      DEBUG_MYSTERY_SetGiftCommonData( &data, i, FALSE );
      MYSTERYDATA_SetCardData( p_mystery_sv, &data );

      DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
      dl_data.data  = data;
#ifdef MYSTERY_DLDATA_DEBUG_PRINT
      {
        int j;
        const u8 * cp_data  = (const u8*)&dl_data;
        for( j = 0; j < sizeof(DOWNLOAD_GIFT_DATA); j++ )
        {
          OS_TFPrintf( 3, "0x%x ", cp_data[j] );
          if( j % 0x10 == 0xF )
          {
            OS_TFPrintf( 3, "\n" );
          }
        }
      }
#endif
      OS_TPrintf( "ふしぎなカードをセットしました イベントID=[%d]\n", i );
      break;
    }
  }
  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}
static BOOL debugMenuCallProc_MakeMysteryCardGPower( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL, GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );
  for( i = 1; i < 2048; i++ )
  {
    if( !MYSTERYDATA_IsEventRecvFlag(p_mystery_sv, i) )
    {
      DEBUG_MYSTERY_SetGiftGPowerData( &data );
      DEBUG_MYSTERY_SetGiftCommonData( &data, i, FALSE );
      MYSTERYDATA_SetCardData( p_mystery_sv, &data );

      DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
      dl_data.data  = data;
#ifdef MYSTERY_DLDATA_DEBUG_PRINT
      {
        int j;
        const u8 * cp_data  = (const u8*)&dl_data;
        for( j = 0; j < sizeof(DOWNLOAD_GIFT_DATA); j++ )
        {
          OS_TFPrintf( 3, "0x%x ", cp_data[j] );
          if( j % 0x10 == 0xF )
          {
            OS_TFPrintf( 3, "\n" );
          }
        }
      }
#endif
      OS_TPrintf( "ふしぎなカードをセットしました イベントID=[%d]\n", i );
      break;
    }
  }
  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}
static BOOL debugMenuCallProc_MakeMysteryCardGLiberty( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL, GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );
  DEBUG_MYSTERY_SetGiftItemData( &data, ITEM_RIBATHITIKETTO );
  DEBUG_MYSTERY_SetGiftCommonData( &data, MYSTERY_DATA_EVENT_LIBERTY, FALSE );
  MYSTERYDATA_SetCardData( p_mystery_sv, &data );

  DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
  dl_data.data  = data;


  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}
static BOOL debugMenuCallProc_MakeMysteryCardEgg( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL,GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );

  for( i = 1; i < 2048; i++ )
  {
    if( !MYSTERYDATA_IsEventRecvFlag(p_mystery_sv, i) )
    {
      DEBUG_MYSTERY_SetGiftPokeData( &data );
      data.data.pokemon.egg = 1;
      DEBUG_MYSTERY_SetGiftCommonData( &data, i, FALSE );
      MYSTERYDATA_SetCardData( p_mystery_sv, &data );

      DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
      dl_data.data  = data;
#ifdef MYSTERY_DLDATA_DEBUG_PRINT
      {
        int j;
        const u8 * cp_data  = (const u8*)&dl_data;
        for( j = 0; j < sizeof(DOWNLOAD_GIFT_DATA); j++ )
        {
          OS_TFPrintf( 3, "0x%x ", cp_data[j] );
          if( j % 0x10 == 0xF )
          {
            OS_TFPrintf( 3, "\n" );
          }
        }
      }
#endif
      OS_TPrintf( "ふしぎなカードをセットしました イベントID=[%d]\n", i );
      break;
    }
  }

  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}

//======================================================================
//
//    マップ全チェック
//
//======================================================================

static GMEVENT * EVENT_DEBUG_AllMapCheck( GAMESYS_WORK * gsys, BOOL up_to_flag );
//--------------------------------------------------------------
/// デバッグメニュー：全マップチェック
//--------------------------------------------------------------
static BOOL debugMenuCallProc_AllMapCheck( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GMEVENT * new_event;
  new_event = EVENT_DEBUG_AllMapCheck( p_wk->gmSys, FALSE );
  GMEVENT_ChangeEvent( p_wk->gmEvent, new_event );
  return TRUE;
}
//--------------------------------------------------------------
/// デバッグメニュー：全接続チェック
//--------------------------------------------------------------
static BOOL debugMenuCallProc_AllConnectCheck( DEBUG_MENU_EVENT_WORK * p_wk )
{
  GMEVENT * new_event;
  new_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
      FS_OVERLAY_ID( debug_all_connect_check ), EVENT_DEBUG_AllConnectCheck, NULL );
  GMEVENT_ChangeEvent( p_wk->gmEvent, new_event );
  return TRUE;
}


//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  int check_zone_id;
  BOOL up_to_flag;
  int wait;
  GAMESYS_WORK * gsys;
}ALL_MAP_CHECK_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static const u16 ng_zone_table[] = {
  ZONE_ID_UNION,        //ユニオンルーム
  ZONE_ID_CLOSSEUM,
  ZONE_ID_CLOSSEUM02,
  ZONE_ID_PLD10,
  ZONE_ID_C04R0202,     //ミュージカル控え室
};

//#include "arc/fieldmap/map_matrix.naix"
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL checkNGZoneID( zone_id )
{
  int i;
#if 0
    if ( ZONEDATA_GetMatrixID(zone_id) == NARC_map_matrix_palacefield_mat_bin )
    {
      continue;
    }
#endif
  for ( i = 0; i < NELEMS(ng_zone_table); i++ )
  {
    if (ng_zone_table[i] == zone_id )
    {
      return TRUE;
    }
  }
  return FALSE;
}
static int getNextZoneID( ALL_MAP_CHECK_WORK* amcw )
{
  int zone_id = amcw->check_zone_id;
  while (TRUE)
  {
    if ( amcw->up_to_flag )
    {
      zone_id ++;
      if ( zone_id >= ZONE_ID_MAX ) break;
    } else {
      zone_id --;
      if ( zone_id < 0 ) break;
    }
    if ( checkNGZoneID( zone_id ) == TRUE ) continue;
    break;
  }
  amcw->check_zone_id = zone_id;
  return zone_id;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT allMapCheckEvent( GMEVENT * event, int *seq, void * wk )
{
  ALL_MAP_CHECK_WORK * amcw = wk;
  FIELDMAP_WORK * fieldmap;
  u16 zone_id;

  enum {
    SEQ_INIT,
    SEQ_SEEK_ID,
    SEQ_WAIT
  };
  switch ( *seq )
  {
  case SEQ_INIT:
    if ( amcw->up_to_flag )
    {
      amcw->check_zone_id = -1;
    } else {
      amcw->check_zone_id = ZONE_ID_MAX;
    }
    *seq = SEQ_SEEK_ID;
    /* FALL THROUGH */

  case SEQ_SEEK_ID:
    zone_id = getNextZoneID( amcw );
    if ( zone_id >= ZONE_ID_MAX || zone_id < 0 )
    {
      return GMEVENT_RES_FINISH;
    }
    {
      char buf[ZONEDATA_NAME_LENGTH];
      ZONEDATA_DEBUG_GetZoneName(buf, zone_id);
      OS_TPrintf( "\t[[ALL MAP CHECKING... :%s]]!!\n", buf );
    }
    fieldmap = GAMESYSTEM_GetFieldMapWork( amcw->gsys );
    GMEVENT_CallEvent( event,
        DEBUG_EVENT_ChangeMapDefaultPos( amcw->gsys, fieldmap, zone_id ) );
    *seq = SEQ_WAIT;
    break;

  case SEQ_WAIT:
    amcw->wait ++;
    if ( amcw->wait > 120 )
    {
      amcw->wait = 0;
      *seq = SEQ_SEEK_ID;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT * EVENT_DEBUG_AllMapCheck( GAMESYS_WORK * gsys, BOOL up_to_flag )
{
  GMEVENT * new_event;
  ALL_MAP_CHECK_WORK * amcw;
  new_event = GMEVENT_Create( gsys, NULL, allMapCheckEvent, sizeof(ALL_MAP_CHECK_WORK) );
  amcw = GMEVENT_GetEventWork( new_event );
  amcw->up_to_flag = up_to_flag;
  amcw->gsys = gsys;
  return new_event;
}



//======================================================================
//  デバッグメニュー ゾーンジャンプ
//======================================================================
#include "debug/debug_zone_jump.h"
FS_EXTERN_OVERLAY(debug_zone_jump);
static GMEVENT_RESULT debugMenuZoneJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//-------------------------------------
/// デバッグ国連データ作成用ワーク
//=====================================
typedef struct
{
  HEAPID HeapID;
  GAMESYS_WORK    *gsys;
  GMEVENT         *Event;
  FIELDMAP_WORK *FieldWork;
  PROCPARAM_DEBUG_ZONE_JUMP p_work;
} DEBUG_ZONE_JUMP_EVENT_WORK;

//----------------------------------------------------------------------------
/**
 *  @brief  ゾーンジャンプ
 *
 *  @param  DEBUG_MENU_EVENT_WORK *wk   ワーク
 *
 *  @return TRUEイベント継続  FALSE終了
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_DebugZoneJump( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *gsys  = p_wk->gmSys;
  GMEVENT       *event    = p_wk->gmEvent;
  FIELDMAP_WORK *fieldWork  = p_wk->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_ZONE_JUMP_EVENT_WORK *evt_work;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));

  //イベントチェンジ
  GMEVENT_Change( event, debugMenuZoneJump, sizeof(DEBUG_ZONE_JUMP_EVENT_WORK) );
  evt_work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( evt_work, sizeof(DEBUG_ZONE_JUMP_EVENT_WORK) );

  //ワーク設定
  evt_work->gsys = gsys;
  evt_work->Event = event;
  evt_work->FieldWork = fieldWork;
  evt_work->HeapID = heapID;

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグゾーンジャンプイベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuZoneJump( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_RESULT,
    SEQ_PROC_END,
  };

  DEBUG_ZONE_JUMP_EVENT_WORK *evt_work = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( evt_work->Event, EVENT_FieldSubProc( evt_work->gsys, evt_work->FieldWork,
        FS_OVERLAY_ID(debug_zone_jump), &ProcData_DebugZoneJump, &evt_work->p_work ) );
    *p_seq  = SEQ_PROC_RESULT;
    break;
  case SEQ_PROC_RESULT:
    if ( evt_work->p_work.Ret )
    {
      GMEVENT * mapchange_event;
      OS_Printf("ゾーンID　%d にジャンプ\n", evt_work->p_work.ZoneID);
      mapchange_event = DEBUG_EVENT_QuickChangeMapDefaultPos( evt_work->gsys, evt_work->FieldWork, evt_work->p_work.ZoneID );
      GMEVENT_ChangeEvent( p_event, mapchange_event );
    }
    else *p_seq  = SEQ_PROC_END;
    break;
  case SEQ_PROC_END:
    return GMEVENT_RES_FINISH;
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

//======================================================================================
//
//  デバッグメニュー　図鑑
//
//======================================================================================
//======================================================================
//======================================================================
typedef enum {
  DEBUG_ZKNCMD_GLOBAL_GET_FULL = 0,
  DEBUG_ZKNCMD_GLOBAL_SEE_FULL,
  DEBUG_ZKNCMD_RANDOM,
  DEBUG_ZKNCMD_FORM_FULL,
  DEBUG_ZKNCMD_ZENKOKU_FLAG,
  DEBUG_ZKNCMD_VERSION_UP,
  DEBUG_ZKNCMD_LOCAL_GET_FULL,
  DEBUG_ZKNCMD_LOCAL_SEE_FULL,   //7
  DEBUG_ZKNCMD_LANGUAGE_FULL,    //8
  DEBUG_ZKNCMD_LOCAL_SEE_25,
  DEBUG_ZKNCMD_LOCAL_SEE_60,
  DEBUG_ZKNCMD_LOCAL_SEE_120,
}DEBUG_ZKNCMD;

typedef struct {
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  ZUKAN_SAVEDATA * sv;
}DEBUG_ZUKAN_WORK;

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER ZukanMenuHeader =
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

///メニューリスト
static const FLDMENUFUNC_LIST ZukanMenuList[] =
{
  { DEBUG_FIELD_ZUKAN_01, (void*)DEBUG_ZKNCMD_GLOBAL_GET_FULL }, // 全国捕獲コンプ
  { DEBUG_FIELD_ZUKAN_07, (void*)DEBUG_ZKNCMD_GLOBAL_SEE_FULL }, // 全国見たコンプ
  { DEBUG_FIELD_ZUKAN_10, (void*)DEBUG_ZKNCMD_LOCAL_GET_FULL },  // 地方捕獲コンプ
  { DEBUG_FIELD_ZUKAN_11, (void*)DEBUG_ZKNCMD_LOCAL_SEE_FULL },  // 地方見たコンプ
  { DEBUG_FIELD_ZUKAN_09, (void*)DEBUG_ZKNCMD_RANDOM },          // ランダムセット
  { DEBUG_FIELD_ZUKAN_03, (void*)DEBUG_ZKNCMD_FORM_FULL },       // フォルムコンプ
  { DEBUG_FIELD_ZUKAN_12, (void*)DEBUG_ZKNCMD_LANGUAGE_FULL },   // 言語コンプ
  { DEBUG_FIELD_ZUKAN_05, (void*)DEBUG_ZKNCMD_ZENKOKU_FLAG },    // 全国フラグ
  { DEBUG_FIELD_ZUKAN_06, (void*)DEBUG_ZKNCMD_VERSION_UP },      // バージョンアップ
  { DEBUG_FIELD_ZUKAN_13, (void*)DEBUG_ZKNCMD_LOCAL_SEE_25 },    // 地方見た２５
  { DEBUG_FIELD_ZUKAN_14, (void*)DEBUG_ZKNCMD_LOCAL_SEE_60 },    // 地方見た６０
  { DEBUG_FIELD_ZUKAN_15, (void*)DEBUG_ZKNCMD_LOCAL_SEE_120 },   // 地方見た１２０
};

static const DEBUG_MENU_INITIALIZER DebugMenuZukanImitializer = {
  NARC_message_d_field_dat,
  NELEMS(ZukanMenuList),
  ZukanMenuList,
  &ZukanMenuHeader,
  1, 1, 16, 16,
  NULL,
  NULL
};

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZukanEvent( GMEVENT *event, int *seq, void *wk );
static void SetZukanDataOne( DEBUG_ZUKAN_WORK * wk, u16 mons, u16 form, u16 lang, u32 mode );
static void SetZukanLocal( DEBUG_ZUKAN_WORK * wk, u16 count, u32 mode );
static void SetZukanDataAll( DEBUG_ZUKAN_WORK * wk );


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　図鑑
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Zukan( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_ZUKAN_WORK *work;

  GMEVENT_Change( event, debugMenuZukanEvent, sizeof(DEBUG_ZUKAN_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_ZUKAN_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  work->sv = GAMEDATA_GetZukanSave( GAMESYSTEM_GetGameData(wk->gmSys) );

  return TRUE;
}

//--------------------------------------------------------------
/**
 * イベント：図鑑
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuZukanEvent( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ZUKAN_WORK *work = wk;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuZukanImitializer );
    (*seq)++;
    break;

  case 1:

    switch( FLDMENUFUNC_ProcMenu(work->menuFunc) ){
    case FLDMENUFUNC_NULL:    // 操作無し
      break;

    case FLDMENUFUNC_CANCEL:  // キャンセル
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_GLOBAL_GET_FULL:     // 全国捕獲
      {
        u32 i;
        for( i=1; i<=MONSNO_END; i++ ){
          SetZukanDataOne( wk, i, 0, PM_LANG, 0 );
        }
      }
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_GLOBAL_SEE_FULL:     // 全国見た
      {
        u32 i;
        for( i=1; i<=MONSNO_END; i++ ){
          SetZukanDataOne( wk, i, 0, PM_LANG, 1 );
        }
      }
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_RANDOM:     // ランダム
      {
        u32 rand;
        u32 max;
        u32 i;
        for( i=1; i<=MONSNO_END; i++ ){
          rand = GFL_STD_MtRand( 3 );
          if( rand == 2 ){ continue; }
          max = ZUKANSAVE_GetFormMax( i );
          SetZukanDataOne( wk, i, GFL_STD_MtRand(max), PM_LANG, rand );
        }
      }
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_FORM_FULL:     // フォルム
/*
      {
        u32 max;
        u16 i, j;
        for( i=1; i<=MONSNO_END; i++ ){
          max = ZUKANSAVE_GetFormMax( i );
          if( max != 1 ){
            for( j=0; j<max; j++ ){
              SetZukanDataOne( wk, i, j, PM_LANG, 0 );
            }
          }
        }
      }
*/
      SetZukanDataAll( wk );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_ZENKOKU_FLAG:     // 全国フラグ
      ZUKANSAVE_SetZenkokuZukanFlag( work->sv );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_VERSION_UP:     // バージョンアップ
      ZUKANSAVE_SetGraphicVersionUpFlag( work->sv );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_GET_FULL:     // 地方捕獲
      SetZukanLocal( wk, MONSNO_END, 0 );
#if 0
      {
        u16 * buf;
        u32 i;
        buf = POKE_PERSONAL_GetZenkokuToChihouArray( work->heapID, NULL );
        for( i=1; i<=MONSNO_END; i++ ){
          if( buf[i] != 0 ){
            SetZukanDataOne( wk, i, 0, PM_LANG, 0 );
          }
        }
        GFL_HEAP_FreeMemory( buf );
      }
#endif
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_FULL:     // 地方見た
      SetZukanLocal( wk, MONSNO_END, 1 );
#if 0
      {
        u16 * buf;
        u32 i;
        buf = POKE_PERSONAL_GetZenkokuToChihouArray( work->heapID, NULL );
        for( i=1; i<=MONSNO_END; i++ ){
          if( buf[i] != 0 ){
            SetZukanDataOne( wk, i, 0, PM_LANG, 1 );
          }
        }
        GFL_HEAP_FreeMemory( buf );
      }
#endif
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_25:
      SetZukanLocal( wk, 25, 1 );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_60:
      SetZukanLocal( wk, 60, 1 );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_120:
      SetZukanLocal( wk, 120, 1 );
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LANGUAGE_FULL:     // 言語
      {
        u32 i;
        for( i=1; i<=MONSNO_END; i++ ){
          SetZukanDataOne( wk, i, 0, LANG_JAPAN, 0 );
          SetZukanDataOne( wk, i, 0, LANG_ENGLISH, 0 );
          SetZukanDataOne( wk, i, 0, LANG_FRANCE, 0 );
          SetZukanDataOne( wk, i, 0, LANG_ITALY, 0 );
          SetZukanDataOne( wk, i, 0, LANG_GERMANY, 0 );
          SetZukanDataOne( wk, i, 0, LANG_SPAIN, 0 );
          SetZukanDataOne( wk, i, 0, LANG_KOREA, 0 );
        }
      }
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

static void SetZukanLocal( DEBUG_ZUKAN_WORK * wk, u16 count, u32 mode )
{
  u16 * buf;
  u32 i;
  buf = POKE_PERSONAL_GetZenkokuToChihouArray( wk->heapID, NULL );
  for( i=1; i<=MONSNO_END; i++ ){
    if( buf[i] != 0 ){
      SetZukanDataOne( wk, i, 0, PM_LANG, mode );
      count --;
    }
    if ( count == 0 ) break;
  }
  GFL_HEAP_FreeMemory( buf );
}

static void SetZukanDataOne( DEBUG_ZUKAN_WORK * wk, u16 mons, u16 form, u16 lang, u32 mode )
{
  POKEMON_PARAM * pp = PP_Create( mons, 50, 0, wk->heapID );
  PP_Put( pp, ID_PARA_form_no, form );
  PP_Put( pp, ID_PARA_country_code, lang );

  if( mode == 0 ){
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
  }else{
    ZUKANSAVE_SetPokeSee( wk->sv, pp );
  }

  GFL_HEAP_FreeMemory( pp );
}


static void SetZukanDataAllOne( DEBUG_ZUKAN_WORK * wk, POKEMON_PARAM * pp, u32 sex, BOOL rare, BOOL first )
{
  u32 mons;
  u32 id;
  u32 rand;
  u32 form_max;
  u32 i;

  mons = PP_Get( pp, ID_PARA_monsno, NULL );
  id   = PP_Get( pp, ID_PARA_id_no, NULL );
  rand = POKETOOL_CalcPersonalRandEx( id, mons, 0, sex, 0, rare );
  PP_SetupEx( pp, mons, 50, id, PTL_SETUP_POW_AUTO, rand );

  form_max = ZUKANSAVE_GetFormMax( mons );

  if( first == TRUE ){
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
  }else{
    ZUKANSAVE_SetPokeSee( wk->sv, pp );
  }

  for( i=1; i<form_max; i++ ){
    rand = POKETOOL_CalcPersonalRandEx( id, mons, i, sex, 0, rare );
    PP_Put( pp, ID_PARA_personal_rnd, rand );
    PP_Put( pp, ID_PARA_form_no, i );
    ZUKANSAVE_SetPokeSee( wk->sv, pp );
  }
}

static void SetZukanDataAll( DEBUG_ZUKAN_WORK * wk )
{
  POKEMON_PERSONAL_DATA * ppd;
  POKEMON_PARAM * pp;
  BOOL fast;
  u32 sex_vec;
  u32 rand;
  u16 i, j;

  for( i=1; i<=MONSNO_END; i++ ){
    // 性別ベクトル取得
    ppd = POKE_PERSONAL_OpenHandle( i, 0, wk->heapID );
    sex_vec = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );
    POKE_PERSONAL_CloseHandle( ppd );
    // POKEMON_PARAM作成
    pp = PP_Create( i, 50, 0, wk->heapID );
    fast = PP_FastModeOn( pp );
    PP_Put( pp, ID_PARA_form_no, 0 );
    PP_Put( pp, ID_PARA_country_code, LANG_JAPAN );

    // 性別なし
    if( sex_vec == POKEPER_SEX_UNKNOWN ){
      SetZukanDataAllOne( wk, pp, PTL_SEX_UNKNOWN, FALSE, TRUE );   // レアじゃない
      SetZukanDataAllOne( wk, pp, PTL_SEX_UNKNOWN, TRUE, FALSE );   // レア
    // 性別あり
    }else{
      // ♂もいる
      if( sex_vec != POKEPER_SEX_FEMALE ){
//        PP_Put( pp, ID_PARA_sex, PTL_SEX_MALE );
        SetZukanDataAllOne( wk, pp, PTL_SEX_MALE, FALSE, TRUE );    // レアじゃない
        SetZukanDataAllOne( wk, pp, PTL_SEX_MALE, TRUE, FALSE );    // レア
      }
      // ♀もいる
      if( sex_vec != POKEPER_SEX_MALE ){
//        PP_Put( pp, ID_PARA_sex, PTL_SEX_FEMALE );
        SetZukanDataAllOne( wk, pp, PTL_SEX_FEMALE, FALSE, TRUE );  // レアじゃない
        SetZukanDataAllOne( wk, pp, PTL_SEX_FEMALE, TRUE, FALSE );  // レア
      }
    }

    // 各言語セット
    PP_Put( pp, ID_PARA_country_code, LANG_ENGLISH );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_FRANCE );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_ITALY );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_GERMANY );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_SPAIN );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );
    PP_Put( pp, ID_PARA_country_code, LANG_KOREA );
    ZUKANSAVE_SetPokeGet( wk->sv, pp );

    PP_FastModeOff( pp, fast );
    GFL_HEAP_FreeMemory( pp );

    OS_Printf( "No.%d : 図鑑追加\n", i );
  }
}



//======================================================================================
//
//
//======================================================================================
//--------------------------------------------------------------
//イベントポケモン作成
//--------------------------------------------------------------
#include "poke_tool/poke_memo.h"
#include "field/evt_lock.h" //ロックカプセルのイベントロックのため
static GMEVENT_RESULT debugMenuEventpokeCreate( GMEVENT *event, int *seq, void *wk );


static const FLDMENUFUNC_LIST DATA_EventPokeCreate[] =
{
  { DEBUG_FIELD_EVEPOKE01, (void*)0 },
  { DEBUG_FIELD_EVEPOKE02, (void*)1 },
  { DEBUG_FIELD_EVEPOKE03, (void*)2 },
  { DEBUG_FIELD_EVEPOKE04, (void*)3 },
  { DEBUG_FIELD_EVEPOKE05, (void*)4 },
  { DEBUG_FIELD_EVEPOKE06, (void*)5 },
  { DEBUG_FIELD_EVEPOKE07, (void*)6 },
  { DEBUG_FIELD_EVEPOKE08, (void*)7 },
  { DEBUG_FIELD_EVEPOKE09, (void*)8 },
  { DEBUG_FIELD_EVEPOKE10, (void*)9 },
  { DEBUG_FIELD_EVEPOKE11, (void*)10 },
  { DEBUG_FIELD_EVEPOKE12, (void*)11 },
  { DEBUG_FIELD_EVEPOKE13, (void*)100 },
  { DEBUG_FIELD_EVEPOKE14, (void*)101 },
};

static const DEBUG_MENU_INITIALIZER DebugEventPokeCreateMenu = {
  NARC_message_d_field_dat,
  NELEMS(DATA_EventPokeCreate),
  DATA_EventPokeCreate,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

typedef struct
{
  HEAPID heapId;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FLDMENUFUNC *menuFunc;
  FIELDMAP_WORK *fieldWork;
}DEBUG_EVENT_POKE_CREATE;

static BOOL debugMenuCallProc_EventPokeCreate( DEBUG_MENU_EVENT_WORK * wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_EVENT_POKE_CREATE *work;

  GMEVENT_Change( event, debugMenuEventpokeCreate, sizeof(DEBUG_EVENT_POKE_CREATE) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_EVENT_POKE_CREATE) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->heapId = heapID;

  return TRUE;
}
static GMEVENT_RESULT debugMenuEventpokeCreate( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_EVENT_POKE_CREATE *work = wk;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapId, &DebugEventPokeCreateMenu );
    (*seq)++;
    break;

  case 1:
    {
      const u32 ret = FLDMENUFUNC_ProcMenu(work->menuFunc);
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(work->gmSys);

      if( ret == FLDMENUFUNC_NULL )
      {
        return GMEVENT_RES_CONTINUE;
      }
      else
      if( ret == FLDMENUFUNC_CANCEL )
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        return GMEVENT_RES_FINISH;
      }
      else
      {
        POKEPARTY *party = GAMEDATA_GetMyPokemon(gmData);
        BOX_MANAGER *boxData = GAMEDATA_GetBoxManager(gmData);
        u64 id = 0xbb76c1c5;
        u64 rnd;
        if( ret < 100 )
        {
          POKEMON_PARAM *pp;
          //ポケモン作成
          switch( ret )
          {
          case 0: //10えいがセレビィ
            pp = PP_Create( MONSNO_SEREBHI , 50 , PTL_SETUP_RND_AUTO , work->heapId );
            PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_SEREBIXI_BEFORE );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 1: //10えいがエンテイ
            rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_ENTEI , 0 , 0 , 0 , TRUE );
            pp = PP_CreateEx( MONSNO_ENTEI , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
            PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_BEFORE );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 2: //10えいがライコウ
            rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_RAIKOU , 0 , 0 , 0 , TRUE );
            pp = PP_CreateEx( MONSNO_RAIKOU , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
            PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_BEFORE );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 3: //10えいがスイクン
            rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_SUIKUN , 0 , 0 , 0 , TRUE );
            pp = PP_CreateEx( MONSNO_SUIKUN , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
            PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_BEFORE );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 4: //10えいがセレビィ(後
            pp = PP_Create( MONSNO_SEREBHI , 50 , PTL_SETUP_RND_AUTO , work->heapId );
            PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_SEREBIXI_AFTER );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 5: //10えいがエンテイ(後
            rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_ENTEI , 0 , 0 , 0 , TRUE );
            pp = PP_CreateEx( MONSNO_ENTEI , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
            PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_AFTER );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 6: //10えいがライコウ(後
            rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_RAIKOU , 0 , 0 , 0 , TRUE );
            pp = PP_CreateEx( MONSNO_RAIKOU , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
            PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_AFTER );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 7: //10えいがスイクン(後
            rnd = POKETOOL_CalcPersonalRandEx( id , MONSNO_SUIKUN , 0 , 0 , 0 , TRUE );
            pp = PP_CreateEx( MONSNO_SUIKUN , 50 , id , PTL_SETUP_POW_AUTO , rnd , work->heapId );
            PP_Put( pp , ID_PARA_birth_place , POKE_MEMO_PLACE_ENRAISUI_AFTER );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 8: //メロディア
            pp = PP_Create( MONSNO_655 , 50 , PTL_SETUP_RND_AUTO , work->heapId );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 9: //ダルタニス
            pp = PP_Create( MONSNO_654 , 50 , PTL_SETUP_RND_AUTO , work->heapId );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 10: //インセクタ
            pp = PP_Create( MONSNO_656 , 50 , PTL_SETUP_RND_AUTO , work->heapId );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          case 11: //はいふシェイミ
            pp = PP_Create( MONSNO_SHEIMI , 50 , PTL_SETUP_RND_AUTO , work->heapId );
            PP_Put( pp , ID_PARA_event_get_flag , TRUE );
            break;
          }
          {
            u16 oyaName[] = {L'イ',L'ベ',L'ン',L'ト',L'ぽ',L'け',0xFFFF};
            PP_Put( pp , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
            PP_Put( pp , ID_PARA_oyasex , PTL_SEX_MALE );
          }
          //手持ちに空きがあれば入れる
          if( PokeParty_GetPokeCount( party ) < 6 )
          {
            PokeParty_Add( party , pp );
          }
          else
          {
            BOXDAT_PutPokemon( boxData , PP_GetPPPPointer( pp ) );
          }
          GFL_HEAP_FreeMemory( pp );
        }
        else
        {
          switch( ret )
          {
          case 100://ロックカプセル
            {
              MYITEM_PTR myItem = GAMEDATA_GetMyItem( gmData );
              MYSTATUS *myStatus = GAMEDATA_GetMyStatus( gmData );
              MISC *miscData = GAMEDATA_GetMiscWork( gmData );

              EVTLOCK_SetEvtLock( miscData , EVT_LOCK_NO_LOCKCAPSULE , myStatus );
              MYITEM_AddItem( myItem , ITEM_ROKKUKAPUSERU , 1 , work->heapId );
            }
            break;
          case 101://ビクティチケット
            {
              MYITEM_PTR myItem = GAMEDATA_GetMyItem( gmData );
              MYSTATUS *myStatus = GAMEDATA_GetMyStatus( gmData );
              MISC *miscData = GAMEDATA_GetMiscWork( gmData );

              EVTLOCK_SetEvtLock( miscData , EVT_LOCK_NO_VICTYTICKET , myStatus );
              MYITEM_AddItem( myItem , ITEM_RIBATHITIKETTO , 1 , work->heapId );
            }
            break;


          }
        }
      }
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}


#include "event_debug_menu_symbol.h"
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　シンボルポケモン作成リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_SymbolPokeList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT * new_event;
  new_event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
    FS_OVERLAY_ID( debug_symbol ), EVENT_DEBUG_SymbolPokeList, wk );
  GMEVENT_ChangeEvent( wk->gmEvent, new_event );

  return TRUE;
}

//--------------------------------------------------------------
//ミュージカル関係デバッグ
//--------------------------------------------------------------
#include "savedata/musical_save.h"
#include "musical/musical_debug.h"
FS_EXTERN_OVERLAY( gds_debug );
static GMEVENT_RESULT debugMenuMusical( GMEVENT *event, int *seq, void *wk );


static const FLDMENUFUNC_LIST DATA_Musical[] =
{
  { DEBUG_FIELD_MUSICAL_01, (void*)0 },
  { DEBUG_FIELD_MUSICAL_02, (void*)1 },
  { DEBUG_FIELD_MUSICAL_03, (void*)2 },
};

static const DEBUG_MENU_INITIALIZER DebugMusicalMenu =
{
  NARC_message_d_field_dat,
  NELEMS(DATA_Musical),
  DATA_Musical,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

typedef struct
{
  HEAPID heapId;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FLDMENUFUNC *menuFunc;
  FIELDMAP_WORK *fieldWork;
}DEBUG_MUSICAL_CREATE;

static BOOL debugMenuCallProc_Musical( DEBUG_MENU_EVENT_WORK * wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_MUSICAL_CREATE *work;

  GMEVENT_Change( event, debugMenuMusical, sizeof(DEBUG_MUSICAL_CREATE) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MUSICAL_CREATE) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->heapId = heapID;

  return TRUE;
}

static GMEVENT_RESULT debugMenuMusical( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MUSICAL_CREATE *work = wk;

  switch( *seq ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapId, &DebugMusicalMenu );
    (*seq)++;
    break;

  case 1:
    {
      const u32 ret = FLDMENUFUNC_ProcMenu(work->menuFunc);
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(work->gmSys);
      MUSICAL_SAVE *musSave = GAMEDATA_GetMusicalSavePtr(gmData);
      if( ret == FLDMENUFUNC_NULL )
      {
        return GMEVENT_RES_CONTINUE;
      }
      else
      if( ret == FLDMENUFUNC_CANCEL )
      {
        FLDMENUFUNC_DeleteMenu( work->menuFunc );
        return GMEVENT_RES_FINISH;
      }
      else
      {
        switch( ret )
        {
        case 0:
          {
            u16 i;
            for( i=0;i<MUSICAL_ITEM_MAX_REAL;i++ )
            {
              MUSICAL_SAVE_AddItem( musSave , i );
            }
          }
          break;
        case 1:
          MUSICAL_SAVE_InitWork( musSave );
          break;
        case 2:
          {
            MUSICAL_SHOT_DATA *shotData = MUSICAL_SAVE_GetMusicalShotData( musSave );

            GFL_OVERLAY_Load( FS_OVERLAY_ID(gds_debug) );
            MUSICAL_DEBUG_CreateDummyData( shotData , 1 , work->heapId );
            GFL_OVERLAY_Unload( FS_OVERLAY_ID(gds_debug));
          }
          break;
        }
      }
    }
  }

  return GMEVENT_RES_CONTINUE;
}

static GMEVENT_RESULT debugCutin(
    GMEVENT *event, int *seq, void *wk );
//--------------------------------------------------------------
/**
 * カットインデバッグ開始
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_Cutin( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_ENCEFF_LIST_EVENT_WORK *work;

  GMEVENT_Change( event,
    debugCutin, sizeof(DEBUG_ENCEFF_LIST_EVENT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_ENCEFF_LIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  DbgCutinNo = 0;
  OS_Printf("----------DebugCutinStart----------\n");
  OS_Printf("DebugCutinNo %d\n",DbgCutinNo);
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：カットイン
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugCutin(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ENCEFF_LIST_EVENT_WORK *work = wk;

  if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_B )
  {
    OS_Printf("----------DebugCutinEnd----------\n");
    return( GMEVENT_RES_FINISH );
  }
  else if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
  {
    GMEVENT *call_evt;
    FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(work->fieldWork);
    call_evt = FLD3D_CI_CreateCutInEvt(work->gmSys, ptr, DbgCutinNo);
    GMEVENT_CallEvent( event, call_evt );
  }

  if ( GFL_UI_KEY_GetTrg() == PAD_KEY_UP )
  {
    DbgCutinNo++;
    if ( DbgCutinNo >= FLDCIID_MAX ) DbgCutinNo = 0;
    OS_Printf("DebugCutinNo %d\n",DbgCutinNo);
  }else if( GFL_UI_KEY_GetTrg() == PAD_KEY_DOWN )
  {
    DbgCutinNo--;
    if ( DbgCutinNo < 0  ) DbgCutinNo = FLDCIID_MAX-1;
    OS_Printf("DebugCutinNo %d\n",DbgCutinNo);
  }

  return( GMEVENT_RES_CONTINUE );
}



#endif  //  PM_DEBUG
