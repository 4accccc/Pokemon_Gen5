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
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"

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
#include  "msg/debug/msg_d_tomoya.h"

#include "field_debug.h" 
#include "field_event_check.h"

#include "event_debug_menu_connect_check.h"
#include "event_debug_menu_research.h"
#include "event_debug_item.h" //EVENT_DebugItemMake
#include "event_debug_numinput.h"
#include "event_debug_numinput_research.h"
#include "event_debug_numinput_record.h"
#include "event_debug_menu_fskill.h"
#include "event_debug_menu_mystery_card.h"
#include "event_debug_bbd_color.h"
#include "event_debug_make_poke.h"
#include "event_debug_rewrite_poke.h"
#include "event_debug_menu_zone_jump.h"
#include "event_debug_menu_mmdl_list.h"
#include "event_debug_livecomm.h"

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
#include "savedata/wifihistory.h"

#include "event_debug_local.h"

#include "event_debug_demo3d.h" // for DEBUG_EVENT_FLDMENU_Demo3DSelect
#include "event_debug_sodateya.h" // for DEBUG_EVENT_DebugMenuSodateya
#include "event_debug_menu_make_egg.h"  // for DEBUG_EVENT_FLDMENU_MakeEgg
#include "event_debug_menu_intrude.h"   // for DEBUG_EVENT_FLDMENU_Intrude

#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SJIStoStrcode

#include "event_debug_wifimatch.h"
#include "event_battlerecorder.h"
#include "event_debug_mvpoke.h"
#include "field_bbd_color.h"

#include "gamesystem/pm_weather.h"

#include "debug/debug_makepoke.h"
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
#include "field/field_comm/intrude_mission.h" //MISSION_LIST_Create

#include "../../../resource/fld3d_ci/fldci_id_def.h"  // for FLDCIID_MAX

#include "system/main.h" //for HEAPID_FIELDMAP

#include "debug_wfbc.h"

#include "savedata/irc_compatible_savedata.h"

FS_EXTERN_OVERLAY( d_iwasawa );

//======================================================================
//  global
//======================================================================
int DbgCutinNo = 0;

//======================================================================
//  extern
//======================================================================
//デバッグ用変数実体はgamesystem.cにあります
extern u32 DbgFldHeapRest;
extern u32 DbgVramRest;
extern u32 DbgFldHeapUseMaxZone;
extern u32 DbgVramUseMaxZone;

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
static BOOL debugMenuCallProc_DisguiseList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_ForceSave( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_ControlLight( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ControlFog( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_WeatherList( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_CaptureList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_FieldAveStress( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_FieldAveStressPalace( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_FieldPosData( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_FieldHitchCheckData( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_ControlRtcList( DEBUG_MENU_EVENT_WORK *wk );

static BOOL debugMenuCallProc_DebugMakePoke( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_DebugReWritePoke( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_DebugItem( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_DebugSecretItem( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_DebugPDWItem( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_BoxMax( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_1BoxMax( DEBUG_MENU_EVENT_WORK *wk );
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

static BOOL debugMenuCallProc_ControlLinerCamera( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_ControlDelicateCamera( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_ControlShortCut( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_CreateMusicalShotData( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_CreateFeelingData( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_BeaconFriendCode( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_WifiBattleMatch( DEBUG_MENU_EVENT_WORK *wk );


static BOOL debugMenuCallProc_UseMemoryDump( DEBUG_MENU_EVENT_WORK *p_wk );

static BOOL debugMenuCallProc_DebugSake( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_DebugAtlas( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_Geonet( DEBUG_MENU_EVENT_WORK * p_wk );
static BOOL debugMenuCallProc_BattleRecorder( DEBUG_MENU_EVENT_WORK *p_wk );
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
static BOOL debugMenu_ClearWifiHistory( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_LiveComm( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_MissionReset( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenu_Intrude( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenu_WfbcDebug( DEBUG_MENU_EVENT_WORK *wk );

static void DbgRestDataUpdate(const u32 inZone);
static void VramDumpCallBack( u32 addr, u32 szByte, void* pUserData );

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
  { DEBUG_FIELD_STR17_01, debugMenuCallProc_FieldPosData },    //座標をみる
  { DEBUG_FIELD_STR17_02, debugMenuCallProc_FieldHitchCheckData },    //当たり判定をみる
  { DEBUG_FIELD_STR02, debugMenuCallProc_ControlLinerCamera },  //カメラ簡単操作
  { DEBUG_FIELD_STR52, debugMenuCallProc_ControlDelicateCamera }, //カメラ全部操作
  { DEBUG_FIELD_EVENT_CONTROL, debugMenuCallProc_EventFlagScript }, //イベント操作
  { DEBUG_FIELD_STR03, debugMenuCallProc_ScriptSelect },    //スクリプト実行
  { DEBUG_FIELD_STR13, debugMenuCallProc_MMdlList },        //モデルリスト
  { DEBUG_FIELD_DISGUISE, debugMenuCallProc_DisguiseList },  //変装
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
  { DEBUG_FIELD_STR72, debugMenuCallProc_1BoxMax },         //ボックス１つ分最大
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
//  { DEBUG_FIELD_STR74, debugMenu_CreateMusicalShotData },           //ミュージカルデータ作成
  { DEBUG_FIELD_STR75, debugMenu_CreateFeelingData },           //フィーリング作成

  { DEBUG_FIELD_TITLE_06, (void*)BMPMENULIST_LABEL },       //○つうしん
  { DEBUG_FIELD_STR19, debugMenuCallProc_OpenClubMenu },      //WIFIクラブ
  { DEBUG_FIELD_GPOWER, debugMenuCallProc_GPowerList},      //Gパワー
  { DEBUG_FIELD_STR49, debugMenuCallProc_BeaconFriendCode },  //ともだちコード配信
  { DEBUG_FIELD_STR57, debugMenuCallProc_DebugSake },             //サケ操作
//  { DEBUG_FIELD_STR58, debugMenuCallProc_DebugAtlas },            //アトラス操作
  { DEBUG_FIELD_GEONET, debugMenuCallProc_Geonet },         //ジオネット呼び出し
  { DEBUG_FIELD_GEONET_CLEAR, debugMenu_ClearWifiHistory }, //ジオネット情報クリア
  { DEBUG_FIELD_LIVE_COMM, debugMenu_LiveComm },  //ライブ通信デバッグ
  //{ DEBUG_FIELD_MISSION_RESET, debugMenu_MissionReset },  //ミッションリスト再作成
  { DEBUG_FIELD_INTRUDE,    debugMenu_Intrude },      //ハイリンク

  { DEBUG_FIELD_TITLE_04, (void*)BMPMENULIST_LABEL },       //○アプリ
  { DEBUG_FIELD_STR44, debugMenuCallProc_UITemplate },        //UIテンプレート
  { DEBUG_FIELD_DEMO3D,   debugMenuCallProc_Demo3d },             //３Dデモ

  { DEBUG_FIELD_TITLE_05, (void*)BMPMENULIST_LABEL },       //○スタッフ用
  { DEBUG_FIELD_STR70, debugMenuCallProc_PCScript },        //パソコン呼び出し
  { DEBUG_FIELD_STR47, debugMenu_ControlShortCut },           //Yボタン登録最大
  { DEBUG_FIELD_STR51  , debugMenuCallProc_OpenGTSNegoMenu }, //GTSネゴ
//  { DEBUG_FIELD_STR42, debugMenuCallProc_WifiGts },           //GTS
//  { DEBUG_FIELD_STR48, debugMenuCallProc_GDS },               //GDS
//  { DEBUG_FIELD_STR56, debugMenuCallProc_WifiBattleMatch },   //WIFI世界対戦
//  { DEBUG_FIELD_STR59, debugMenuCallProc_BattleRecorder },        //バトルレコーダー
  { DEBUG_FIELD_BSW_00, debugMenuCallProc_BSubway },                //バトルサブウェイ
  { DEBUG_FIELD_ZUKAN_04, debugMenuCallProc_Zukan },            //ずかん
  { DEBUG_FIELD_STR66,  debugMenuCallProc_RingTone },           //着信音
  { DEBUG_FIELD_STR73, debugMenu_WfbcDebug }                   //WFBCオートチェック
};


//------------------------------------------------------------------------
/*
 *  QUICKSJUMP
 *  上記のリストのSTRIDを書くと、STARTボタンをおしたときに、その位置までカーソルが動きます
 */
//------------------------------------------------------------------------
#if defined DEBUG_ONLY_FOR_toru_nagihashi
  #define QuickJumpStart   DEBUG_FIELD_STR41
#elif defined DEBUG_ONLY_FOR_matsumiya
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_masafumi_saitou
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_suginaka_katsunori
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_mizuguchi_mai || defined DEBUG_ONLY_FOR_mai_ando
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_murakami_naoto
  #define QuickJumpStart   DEBUG_FIELD_STR03
  #define QuickJumpSelect  DEBUG_FIELD_STR17_01
#elif defined DEBUG_ONLY_FOR_ohno
  #define QuickJumpStart   DEBUG_FIELD_STR19
//#elif defined DEBUG_ONLY_FOR_
//  #define QuickJumpStart
#elif defined DEBUG_ONLY_FOR_iwasawa
  #define QuickJumpStart   DEBUG_FIELD_STR41
  #define QuickJumpSelect  DEBUG_FIELD_STR65
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
  NARC_debug_message_d_field_dat,
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
  return DebugMenuInitCore( fieldmap, heapID, init, NULL, 0, 0, ARCID_DEBUG_MESSAGE );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static FLDMENUFUNC * DEBUGFLDMENU_InitArcID(
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
static FLDMENUFUNC * DEBUGFLDMENU_InitExPos(
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
  EV_WIFICLUB_PARAM param;

  param.fieldmap = fieldWork;
  param.bFieldEnd = TRUE;
  

  next_event = GMEVENT_CreateOverlayEventCall( gameSys, 
      FS_OVERLAY_ID(event_wificlub), 
      EVENT_CallWiFiClub, &param );

  GMEVENT_ChangeEvent( event, next_event );

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
  GMEVENT *next_event;

  next_event = GMEVENT_CreateOverlayEventCall( gameSys, 
      FS_OVERLAY_ID(event_gtsnego), EVENT_GTSNegoChangeDebug, fieldWork );
  GMEVENT_ChangeEvent( event, next_event );
  return( TRUE );
}


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
    FS_OVERLAY_ID( d_numinput_research ), DEBUG_EVENT_DebugMenu_Research, wk );

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
  { DEBUG_FIELD_STR_SUBSCRN02, (void*)setupSoundViewerSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN03, (void*)setupNormalSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN04, (void*)setupTopMenuSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN01, (void*)setupDebugLightSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN05, (void*)setupUnionSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN06, (void*)setupCGearFirstSubscreen },
  { DEBUG_FIELD_STR_SUBSCRN07, (void*)setupNoneSubscreen },
};

static const DEBUG_MENU_INITIALIZER DebugSubscreenSelectData = {
  NARC_debug_message_d_field_dat,
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
/**
 * デバッグメニュー呼び出し　動作モデル一覧
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MMdlList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
    FS_OVERLAY_ID( debug_menu_mmdl_list ), DEBUG_EVENT_DebugMenu_MmdlList, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE ); 
}

static BOOL debugMenuCallProc_DisguiseList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
    FS_OVERLAY_ID( debug_menu_mmdl_list ), DEBUG_EVENT_DebugMenu_DisguiseList, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE ); 
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
  NARC_debug_message_d_tomoya_dat,
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
  FIELD_DEBUG_SetPrint( debug, FIELD_DEBUG_PRINT_TYPE_POSITION );

  return( FALSE );
}
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　フィールド当たり判定情報
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_FieldHitchCheckData( DEBUG_MENU_EVENT_WORK *wk )
{
  FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( wk->fieldWork );
  FIELD_DEBUG_SetHitchCheckPrint( debug );

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
  NARC_debug_message_d_field_dat,
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
//  デバッグメニュー ポケモン作成
//======================================================================

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
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
    FS_OVERLAY_ID( debug_menu_makepoke ), DEBUG_EVENT_DebugMenu_MakePoke, p_wk );

  GMEVENT_ChangeEvent( p_wk->gmEvent, event );

  return( TRUE );
}

//======================================================================
//  デバッグメニュー ポケモン書き換え
//======================================================================

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
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
    FS_OVERLAY_ID( debug_menu_rewrite_poke ), DEBUG_EVENT_DebugMenu_ReWritePoke, p_wk );

  GMEVENT_ChangeEvent( p_wk->gmEvent, event );

  return( TRUE );
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

static BOOL debugMenuCallProc_BoxMax( DEBUG_MENU_EVENT_WORK *wk )
{
#if 1
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  MYSTATUS *myStatus;
  POKEMON_PARAM *pp;
  POKEMON_PASO_PARAM  *ppp;
  const STRCODE *name;

  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gameSys));
  name = MyStatus_GetMyName( myStatus );

  pp = PP_Create(100, 100, 123456, HEAPID_FIELDMAP);

  {
    int i,j,k=BOX_MAX_TRAY;
    int monsno=1;
    BOX_MANAGER* pBox = GAMEDATA_GetBoxManager(GAMESYSTEM_GetGameData(gameSys));

    BOXDAT_AddTrayMax( pBox );
    BOXDAT_AddTrayMax( pBox );
    for(i=0;i < k;i++){
      for(j=0;j < 30;j++){
        OS_TPrintf("%d  %d %d作成\n",monsno, i, j);
        PP_Setup(pp,  monsno , 100, MyStatus_GetID( myStatus ));

        ppp = (POKEMON_PASO_PARAM  *)PP_GetPPPPointerConst( pp );
        PPP_Put( ppp , ID_PARA_oyaname_raw , (u32)name );
        PPP_Put( ppp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );

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
 * @brief   ポケモンボックス１つ分をポケモンでいっぱいにする
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_1BoxMax( DEBUG_MENU_EVENT_WORK *wk )
{
#if 1
  GAMESYS_WORK  *gameSys  = wk->gmSys;
  MYSTATUS *myStatus;
  POKEMON_PARAM *pp;
  POKEMON_PASO_PARAM  *ppp;
  const STRCODE *name;

  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gameSys));
  name = MyStatus_GetMyName( myStatus );

  pp = PP_Create(100, 100, 123456, HEAPID_FIELDMAP);

  {
    int i,j,k=1;
    int monsno=1;
    BOX_MANAGER* pBox = GAMEDATA_GetBoxManager(GAMESYSTEM_GetGameData(gameSys));

    BOXDAT_AddTrayMax( pBox );
    BOXDAT_AddTrayMax( pBox );
    for(i=0;i < k;i++){
      for(j=0;j < 30;j++){
        OS_TPrintf("%d  %d %d作成\n",monsno, i, j);
        PP_Setup(pp,  monsno , 100, MyStatus_GetID( myStatus ));

        ppp = (POKEMON_PASO_PARAM  *)PP_GetPPPPointerConst( pp );
        PPP_Put( ppp , ID_PARA_oyaname_raw , (u32)name );
        PPP_Put( ppp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
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

  for( i = 0; i <= ITEM_DATA_MAX; i++ ){
    if( ITEM_CheckEnable( i ) ){
      MYITEM_AddItem(myitem , i, 999, GFL_HEAPID_APP );
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

  // 性別をひっくり返す
  sex = MyStatus_GetMySex( mystatus );
  sex ^= 1;
  MyStatus_SetMySex( mystatus, sex );
  MyStatus_SetMySex( s_mystatus, sex );

  // ユニオン見た目もひっくり返す
  MyStatus_SetTrainerView( mystatus,   sex*MYSTATUS_UNIONVIEW_MAX );
  MyStatus_SetTrainerView( s_mystatus, sex*MYSTATUS_UNIONVIEW_MAX );

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
  EV_GTS_PARAM param;

  param.fieldmap = p_wk->fieldWork;
  param.is_debug = TRUE;
  
  
  
  GMEVENT_ChangeEvent( p_wk->gmEvent, 
      GMEVENT_CreateOverlayEventCall( p_wk->gmSys, 
        FS_OVERLAY_ID(event_gts), EVENT_CallGts, &param ) );
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

  if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    //空を飛ぶときのカメラアングルにする
    VecFx32 add = {0,16384,0}; 
    FIELD_CAMERA_SetTargetOffset(cam, &add );
    FIELD_CAMERA_SetFovy(cam, 3640 );
    FIELD_CAMERA_SetAngleLen(cam, 970752 );
    FIELD_CAMERA_SetAnglePitch(cam, 9688 );
    FIELD_CAMERA_SetAngleYaw(cam,0);
    return GMEVENT_RES_CONTINUE;
  }


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
 *	@brief  フィーリングデータを１つ追加
 *
 *	@param	p_sv_ctrl セーブコントロール
 */
//-----------------------------------------------------------------------------
static void FEELINGCHECK_AddRankData( SAVE_CONTROL_WORK *p_sv_ctrl )
{	
	IRC_COMPATIBLE_SAVEDATA *p_sv	= IRC_COMPATIBLE_SV_GetSavedata( p_sv_ctrl );
	{	
		STRCODE	str[128];
		u16	strlen;
		u16 idx;
		static const struct
		{	
			u16 * p_str;
      u8  str_len;
      u8  sex;
      u8  month;
      u8  day;
			u32 ID;
      u16 mons_no;
		} scp_debug_rank_data[]	=
		{	
			{	
				L"いちたろう",
        5,
        1,
        10,
        22,
				0x573,
        1,
			},
			{	
				L"じろう",
        3,
        1,
        1,
        27,
				0x785,
        2,
			},
			{	
				L"さぶろう",
        4,
        1,
        4,
        8,
				0x123,
        3,
			},
			{	
				L"しろう",
        3,
        1,
        7,
        14,
				0x987,
        4,
			},
			{	
				L"ごろう",
        3,
        0,
        2,
        15,
				0x782,
        5,
			},
			{	
				L"ろくろう",
        4,
        0,
        8,
        2,
				0x191,
        6,
			},
			{	
				L"ななお",
        3,
        0,
        1,
        7,
				0x232,
        7,
			},
			{	
				L"はちべえ",
        4,
        0,
        11,
        27,
				0x595,
        8,
			},
			{	
				L"くろう",
        3,
        0,
        10,
        18,
				0x999,
        9,
			}
		};
		u16 *p_str;

		idx	= GFUser_GetPublicRand( NELEMS(scp_debug_rank_data) );

		p_str	= scp_debug_rank_data[idx].p_str;

		strlen	= scp_debug_rank_data[idx].str_len;
		GFL_STD_MemCopy(p_str, str, strlen*2);
		str[strlen]	= GFL_STR_GetEOMCode();

		//セーブする
		IRC_COMPATIBLE_SV_AddRanking( p_sv, str,
        GFUser_GetPublicRand( 101 ), scp_debug_rank_data[idx].sex, scp_debug_rank_data[idx].month, scp_debug_rank_data[idx].day, scp_debug_rank_data[idx].ID,
  scp_debug_rank_data[idx].mons_no, 0, PTL_SEX_MALE, 0 );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  
 *
 *	@param	DEBUG_MENU_EVENT_WORK *wk ワーク
 *	@retval TRUE続行  FALSE終了
 */
//-----------------------------------------------------------------------------
static BOOL debugMenu_CreateFeelingData( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  FEELINGCHECK_AddRankData( p_sv );

  return FALSE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー ジオネット情報削除（データズレ対策）
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenu_ClearWifiHistory( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys      = wk->gmSys;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(wk->gdata);
  WIFI_HISTORY *wifiHistory = SaveData_GetWifiHistory(p_sv);
  MYSTATUS *mystatus        = GAMEDATA_GetMyStatus( wk->gdata );

  WIFIHISTORY_Init( wifiHistory );
  MyStatus_SetMyNationArea( mystatus, 0, 0 );
  OS_Printf("mystatus adr=%08x\n", (u32)mystatus);

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
  EV_WIFIBATTLEMATCH_PARAM ev_btl_param;
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

  ev_btl_param.fieldmap = wk->fieldWork;
  ev_btl_param.mode     = DEBUG_MODE(mode);
  ev_btl_param.poke     = WIFIBATTLEMATCH_POKE_BTLBOX;
  ev_btl_param.btl_rule = btl_rule;

  GMEVENT_ChangeEvent( wk->gmEvent, 
      GMEVENT_CreateOverlayEventCall( wk->gmSys, FS_OVERLAY_ID(event_wifibtlmatch), EVENT_CallWifiBattleMatch, &btl_rule ) );
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
  GMEVENT* set_event;
  EV_DEBUG_WIFIMATCH param;
  
  param.fieldmap = p_wk->fieldWork;

  set_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys, 
        FS_OVERLAY_ID(event_debug_wifimatch), EVENT_DEBUG_CallWifiMatch, &param );
  
  GMEVENT_ChangeEvent( p_wk->gmEvent, set_event );
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
  GMEVENT* set_event;
  EV_DEBUG_WIFIMATCH param;
  
  param.fieldmap = p_wk->fieldWork;

  set_event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys, 
        FS_OVERLAY_ID(event_debug_wifimatch), EVENT_DEBUG_CallWifiMatch, &param );
  
  GMEVENT_ChangeEvent( p_wk->gmEvent, set_event );
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

//--------------------------------------------------------------
/**
 * デバッグメニュー　ライブ通信デバッグ 
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenu_LiveComm( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
              FS_OVERLAY_ID( d_livecomm ), DEBUG_EVENT_LiveCommFromField, NULL );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
  return TRUE;
}

//--------------------------------------------------------------
/**
 * デバッグメニュー　ミッションリスト再作成
 * @param wk  DEBUG_MENU_EVENT_WORK*  ワーク
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenu_MissionReset( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMEDATA *gmData = GAMESYSTEM_GetGameData(wk->gmSys);
  OCCUPY_INFO *occupy = GAMEDATA_GetMyOccupyInfo(gmData);
  MISSION_LIST_Create( occupy );
  return FALSE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー　ハイリンク
 */
//--------------------------------------------------------------
static BOOL debugMenu_Intrude( DEBUG_MENU_EVENT_WORK * wk )
{
  GMEVENT* event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
              FS_OVERLAY_ID( debug_menu_intrude ), DEBUG_EVENT_FLDMENU_Intrude, wk->fieldWork );
  GMEVENT_ChangeEvent( wk->gmEvent, event );
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

  FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( p_wk->fieldWork );
  FIELD_DEBUG_SetPrint( debug, FIELD_DEBUG_PRINT_TYPE_MEMORY );

  return( FALSE );

#if 0
  GMEVENT_Change( p_event, debugMenuUseMemoryDump, sizeof(DEBUG_USEMEMORY_EVENT_WORK) );
  p_work = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_work, sizeof(DEBUG_USEMEMORY_EVENT_WORK) );

  p_work->p_field   = p_fieldWork;
  p_work->heapID    = heapID;

  // MESSAGE情報読み込み
  {
    // ワードセット作成
    p_work->p_debug_wordset = WORDSET_Create( heapID );
    p_work->p_debug_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_field_dat, heapID );

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
#endif

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




//-----------------------------------------------------------------------------
/**
 * @brief 3Dデモ再生
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY( debug_demo3d );
static BOOL debugMenuCallProc_Demo3d( DEBUG_MENU_EVENT_WORK* wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_demo3d ), DEBUG_EVENT_FLDMENU_Demo3DSelect, NULL );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

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

  msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_field_dat, heapID );

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
  NARC_debug_message_d_field_dat,
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

//----------------------------------------------------------------------------
/**
 *  @brief  ビルボードカラーを指定
 */
//-----------------------------------------------------------------------------
static BOOL debugMenuCallProc_BBDColor( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_bbd_color ), DEBUG_EVENT_DebugMenu_BBD_Color, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE; 
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
  NARC_debug_message_d_field_dat,
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
  DEBUG_BSWAY_CHANGE_WIFI_RANK,
  DEBUG_BSWAY_CLEAR_WIFI_ROOM,
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
  
  { DEBUG_FIELD_BSW_41, (void*)DEBUG_BSWAY_CHANGE_WIFI_RANK },
  { DEBUG_FIELD_BSW_42, (void*)DEBUG_BSWAY_CLEAR_WIFI_ROOM },
};

#define DEBUG_BSUBWAY_LIST_MAX ( NELEMS(DATA_BSubwayMenuList) )

static const DEBUG_MENU_INITIALIZER DebugBSubwayMenuData = {
  NARC_debug_message_d_field_dat,
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
  NARC_debug_message_d_field_dat,
  DEBUG_BSUBWAY_ANYSTAGE_LIST_MAX,
  DATA_BSubwayAnyStageMenuList,
  &DATA_DebugMenuList_BSubway,
  1, 1, 20, 12,
  NULL,
  NULL
};

//DEBUG_BSUBWAY_EVENT_WORK
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *fldMsgBG;
  
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
  FLDSYSWIN *sysWin;
}DEBUG_BSUBWAY_EVENT_WORK;

typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *fldMsgBG;
  
  int play_mode;
  int game_round;
  int before_round;
  int key_repeat;
  int key_repeat_wait;
  
  STRBUF *strBuf;
  STRBUF *strTempBuf;
  GFL_MSGDATA *msgData;
  WORDSET *wordSet;
  FLDMSGWIN *msgWin;
  FLDSYSWIN *sysWin;
  FLDMENUFUNC *menuFunc;
}DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK;

static void debug_bsw_SetAuto( GAMESYS_WORK *gsys )
{
  u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( gsys );
  flag |= BSW_DEBUG_FLAG_AUTO;
  BSUBWAY_SCRWORK_DebugSetFlag( gsys, flag );
}

#define PAD_KEY_ALL (PAD_KEY_UP|PAD_KEY_DOWN|PAD_KEY_LEFT|PAD_KEY_RIGHT)

//バトルサブウェイデバッグメニュー派生　任意のステージに
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
        return( GMEVENT_RES_FINISH );
      }
      
      work->play_mode = ret;
      (*seq)++;
      break;
    }
  case 2:
    {
      FLDMSGBG * msgbg = FIELDMAP_GetFldMsgBG( work->fieldWork );
        
      work->strBuf = GFL_STR_CreateBuffer( 128, work->heapID );
      work->strTempBuf = GFL_STR_CreateBuffer( 128, work->heapID );
      work->msgData = DEBUGFLDMENU_CreateMSGDATA( work->fieldWork,
          NARC_debug_message_d_field_dat );
      work->wordSet = WORDSET_Create( work->heapID );
      work->sysWin = FLDSYSWIN_AddTalkWin( msgbg, NULL );
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

      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        FLDSYSWIN_Delete( work->sysWin );
        FLDMSGWIN_Delete( work->msgWin );
        WORDSET_Delete( work->wordSet );
        GFL_MSG_Delete( work->msgData );
        GFL_STR_DeleteBuffer( work->strBuf );
        GFL_STR_DeleteBuffer( work->strTempBuf );
       
        if( trg & PAD_BUTTON_B ){
          return( GMEVENT_RES_FINISH );
        }
        
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
        
        {
          int stage = 0,round = 0;
          
          stage = work->game_round / 7;
          round = work->game_round % 7;
          
          if( work->game_round % 7 ){
            stage++;
          }
          
          if( round == 0 ){
            round = 7;
          }
          
          STRTOOL_SetNumber( work->strTempBuf, work->game_round, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterWord(
              work->wordSet, 0, work->strTempBuf, 0, TRUE, PM_LANG );
          STRTOOL_SetNumber( work->strTempBuf, stage, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterWord(
              work->wordSet, 1, work->strTempBuf, 0, TRUE, PM_LANG );
          STRTOOL_SetNumber( work->strTempBuf, round, 1,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
          WORDSET_RegisterWord(
              work->wordSet, 2, work->strTempBuf, 0, TRUE, PM_LANG );
          
            GFL_MSG_GetString(
              work->msgData, DEBUG_FIELD_BSW_40, work->strTempBuf );
          WORDSET_ExpandStr(
              work->wordSet, work->strBuf, work->strTempBuf );
          FLDSYSWIN_ClearWindow( work->sysWin );
          FLDSYSWIN_PrintStrBuf( work->sysWin, 1, 0, work->strBuf );
        }
      }
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

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
  GFL_STD_MemClear( work, sizeof(DEBUG_BSUBWAY_ANYSTAGE_EVENT_WORK) );
  
  work->heapID = heapID;
  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->fldMsgBG = FIELDMAP_GetFldMsgBG( fieldWork );
}

static BOOL bsubway_CheckBattleZoneID( DEBUG_BSUBWAY_EVENT_WORK *work )
{
  u16 zone_id = FIELDMAP_GetZoneID( work->fieldWork );
  
  if( ZONEDATA_IsBattleSubway(zone_id) == TRUE ){
    if( zone_id != ZONE_ID_C04R0110 &&
        zone_id != ZONE_ID_C04R0111 ){
      return( TRUE );
    }
  }
  
  FLDSYSWIN_ClearWindow( work->sysWin );
  FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_37 );
  return( FALSE );
}

//バトルサブウェイデバッグメニュー派生　wifiランク変更
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *fldMsgBG;
  
  int rank;
  int before_rank;
  
  STRBUF *strBuf;
  STRBUF *strTempBuf;
  GFL_MSGDATA *msgData;
  WORDSET *wordSet;
  FLDMSGWIN *msgWin;
  FLDSYSWIN *sysWin;
  FLDMENUFUNC *menuFunc;
}DEBUG_BSW_CHANGE_WIFI_RANK;

static GMEVENT_RESULT debugMenuBSubwayChangeWifiRank(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSW_CHANGE_WIFI_RANK *work = wk;
  
  switch( (*seq) ){
  case 0:
    {
      FLDMSGBG * msgbg = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->strBuf = GFL_STR_CreateBuffer( 8, work->heapID );
      work->msgWin = FLDMSGWIN_Add( msgbg, NULL, 1, 1, 8, 2 );
      work->rank = BSUBWAY_SCRWORK_DebugGetWifiRank( work->gmSys );
      work->before_rank = -1;
    }
    (*seq)++;
    break;
  case 1:
    {
      int trg = GFL_UI_KEY_GetTrg();
      int cont = GFL_UI_KEY_GetCont();
      int repeat = GFL_UI_KEY_GetRepeat();
      
      if( trg & (PAD_BUTTON_A|PAD_BUTTON_B) ){
        FLDMSGWIN_Delete( work->msgWin );
        GFL_STR_DeleteBuffer( work->strBuf );
        
        if( trg & PAD_BUTTON_A ){
          BSUBWAY_SCRWORK_DebugSetWifiRank( work->gmSys, work->rank );
        }
        
        return( GMEVENT_RES_FINISH );
      }
      
      if( repeat & PAD_KEY_UP ){
        work->rank--;
        if( work->rank < 1 ){ work->rank = 1; }
      }else if( repeat & PAD_KEY_DOWN ){
        work->rank++;
        if( work->rank > 10 ){ work->rank = 10; }
      }else if( repeat & PAD_KEY_LEFT ){
        work->rank -= 2;
        if( work->rank < 1 ){ work->rank = 1; }
      }else if( repeat & PAD_KEY_RIGHT ){
        work->rank += 2;
        if( work->rank > 10 ){ work->rank = 10; }
      }
      
      if( work->rank != work->before_rank ){
        work->before_rank = work->rank;
        STRTOOL_SetNumber( work->strBuf, work->rank, 5,
            STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT  );
        FLDMSGWIN_ClearWindow( work->msgWin );
        FLDMSGWIN_PrintStrBuf( work->msgWin, 16, 1, work->strBuf );
      }
    }
  }

  return( GMEVENT_RES_CONTINUE );
}

static void debugMenuCallProc_BSubwayChangeWifiRank(
    GMEVENT *event, DEBUG_BSUBWAY_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_BSW_CHANGE_WIFI_RANK *work;
  
  GMEVENT_Change( event, debugMenuBSubwayChangeWifiRank,
      sizeof(DEBUG_BSW_CHANGE_WIFI_RANK) );
  work = GMEVENT_GetEventWork( event );
 GFL_STD_MemClear( work, sizeof(DEBUG_BSW_CHANGE_WIFI_RANK) );
  
  work->heapID = heapID;
  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->fldMsgBG = FIELDMAP_GetFldMsgBG( fieldWork );
}

//イベント：バトルサブウェイデバッグメニュー
static GMEVENT_RESULT debugMenuBSubwayEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BSUBWAY_EVENT_WORK  *work = wk;

  switch( (*seq) ){
  case 0:
    work->msgData = DEBUGFLDMENU_CreateMSGDATA( work->fieldWork,
        NARC_debug_message_d_field_dat );
    work->sysWin = FLDSYSWIN_AddTalkWin( work->fldMsgBG, work->msgData );
    work->menuFunc = DEBUGFLDMENU_Init(
        work->fieldWork, work->heapID,  &DebugBSubwayMenuData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret,param;
      u16 dummy_ret;
      BOOL chg_event = FALSE;
      VecFx32 receipt_map_pos = {GRID_SIZE_FX32(9),0,GRID_SIZE_FX32(12)};
      VecFx32 home_map_pos = {GRID_SIZE_FX32(27),0,GRID_SIZE_FX32(15)};
      GFL_MSGDATA *msgData = work->msgData;
      FLDSYSWIN *sysWin = work->sysWin;
      FLDMENUFUNC *menuFunc = work->menuFunc;
      GMEVENT *next_event = NULL;
      EV_WIFIBSUBWAY_PARAM wifi_bsubway_param;
      
      ret = FLDMENUFUNC_ProcMenu( menuFunc );
      
      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      
      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        FLDSYSWIN_Delete( sysWin );
        FLDMENUFUNC_DeleteMenu( menuFunc );
        GFL_MSG_Delete( msgData );
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
        wifi_bsubway_param.mode = WIFI_BSUBWAY_MODE_GAMEDATA_DOWNLOAD;
        wifi_bsubway_param.ret_wk = &dummy_ret;
        
        next_event = GMEVENT_CreateOverlayEventCall( work->gmSys, 
                      FS_OVERLAY_ID(event_wifibsubway), 
                      WIFI_BSUBWAY_EVENT_CallStart, &wifi_bsubway_param );
        break;
      case DEBUG_BSWAY_WIFI_RIREKI_DOWNLOAD:  //wif 履歴ダウンロード
        wifi_bsubway_param.mode = WIFI_BSUBWAY_MODE_SUCCESSDATA_DOWNLOAD;
        wifi_bsubway_param.ret_wk = &dummy_ret;
        
        next_event = GMEVENT_CreateOverlayEventCall( work->gmSys, 
                      FS_OVERLAY_ID(event_wifibsubway), 
                      WIFI_BSUBWAY_EVENT_CallStart, &wifi_bsubway_param );
        break;
      case DEBUG_BSWAY_WIFI_UPLOAD: //wifi データアップロード
        wifi_bsubway_param.mode = WIFI_BSUBWAY_MODE_SCORE_UPLOAD;
        wifi_bsubway_param.ret_wk = &dummy_ret;
        
        next_event = GMEVENT_CreateOverlayEventCall( work->gmSys, 
                      FS_OVERLAY_ID(event_wifibsubway), 
                      WIFI_BSUBWAY_EVENT_CallStart, &wifi_bsubway_param );
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
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_DOUBLE: //ダブルオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_MULTI: //マルチオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_S_SINGLE: //Sシングルオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_SINGLE );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_S_DOUBLE: //Sダブルオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_DOUBLE );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_AUTO_S_MULTI: //Sマルチオート戦闘
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_MULTI );
          debug_bsw_SetAuto( work->gmSys );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_SINGLE_7: //シングル７戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_DOUBLE_7: //ダブル７戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_MULTI_7: //マルチ７戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 7 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_SINGLE_21: //シングル２１戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_SINGLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_DOUBLE_21: //ダブル２１戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_DOUBLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_MULTI_21: //マルチ２１戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_MULTI );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 21 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_SET_REGU_OFF: //レギュオフ
        {
          u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( work->gmSys );
          flag ^= BSW_DEBUG_FLAG_REGU_OFF;
          BSUBWAY_SCRWORK_DebugSetFlag( work->gmSys, flag );
          
          FLDSYSWIN_ClearWindow( work->sysWin );

          if( (flag & BSW_DEBUG_FLAG_REGU_OFF) ){
            FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_25 );
          }else{
            FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_38 );
          }
        }
        break;
      case DEBUG_BSWAY_SET_BTL_SKIP: //バトルスキップ
        {
          u8 flag = BSUBWAY_SCRWORK_DebugGetFlag( work->gmSys );
          flag ^= BSW_DEBUG_FLAG_BTL_SKIP;
          BSUBWAY_SCRWORK_DebugSetFlag( work->gmSys, flag );
          
          FLDSYSWIN_ClearWindow( work->sysWin );

          if( (flag & BSW_DEBUG_FLAG_BTL_SKIP) ){
            FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_39);
          }else{
            FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_24);
          }
        }
        break;
      case DEBUG_BSWAY_BTL_S_SINGLE_49: //Ｓシングル４９戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_SINGLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_S_DOUBLE_49: //Ｓダブル４９戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_DOUBLE );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_BTL_S_MULTI_49: //Ｓマルチ４９戦から
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          BSUBWAY_SCRWORK_DebugCreateWork( work->gmSys, BSWAY_MODE_S_MULTI );
          BSUBWAY_SCRWORK_DebugFightAnyRound( work->gmSys, 49 );
          SCRIPT_ChangeScript(
              event, SCRID_BSW_DEBUG_MAP_CHG_TRAIN, NULL, HEAPID_PROC );
        }
        break;
      case DEBUG_BSWAY_ANYSTAGE: //任意ステージ
        if( bsubway_CheckBattleZoneID(work) == TRUE ){
          chg_event = TRUE;
          debugMenuCallProc_BSubwayAnyStage( event, work );
        }
        break;
      case DEBUG_BSWAY_CHANGE_WIFI_RANK: //wifiランク変更
        chg_event = TRUE;
        debugMenuCallProc_BSubwayChangeWifiRank( event, work );
        break;
      case DEBUG_BSWAY_CLEAR_WIFI_ROOM: //wifiルームデータクリア
        BSUBWAY_SCRWORK_DebugClearWifiRoomData( work->gmSys );
        FLDSYSWIN_ClearWindow( work->sysWin );
        FLDSYSWIN_Print( work->sysWin, 1, 0, DEBUG_FIELD_BSW_43 );
        break;
      default:
        break;
      }
      
      if( chg_event == TRUE || next_event != NULL ){
        FLDSYSWIN_Delete( sysWin );
        FLDMENUFUNC_DeleteMenu( menuFunc );
        GFL_MSG_Delete( msgData );
        
        if( next_event != NULL ){
          GMEVENT_CallEvent( event, next_event );
          (*seq)++;
        }
      }
    }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  
  return( GMEVENT_RES_CONTINUE );
}

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
  work->fldMsgBG = FIELDMAP_GetFldMsgBG( fieldWork );
  
  return( TRUE );
}

//======================================================================
//  デバッグメニュー　Gパワー
//======================================================================
#include "gamesystem/g_power.h"
#include "msg/debug/msg_d_matsu.h"
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
    work->menuFunc = DEBUGFLDMENU_InitArcID(
        work->fieldWork, work->heapID,  &DebugGPowerMenuListData, ARCID_MESSAGE );
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
  GFL_MSGDATA * pMsgDataDMatsu = DEBUGFLDMENU_CreateMSGDATA( fieldmap, NARC_debug_message_d_matsu_dat );

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
/**
 * デバッグメニュー呼び出し　フィールド技リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_FieldSkillList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_fskill ), DEBUG_EVENT_DebugMenu_Fskill, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE; 
}


//======================================================================
//  デバッグメニュー　不思議データ作成
//======================================================================

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　不思議データ作成
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MakeMysteryCardList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_mystery_card ), DEBUG_EVENT_DebugMenu_MakeMysteryCard, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE;
}

//======================================================================
//
//    マップ全チェック
//
//======================================================================

//static GMEVENT * EVENT_DEBUG_AllMapCheck( GAMESYS_WORK * gsys, BOOL up_to_flag );
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
      FS_OVERLAY_ID( debug_connect_check ), DEBUG_EVENT_DebugMenu_ConnectCheck, NULL );

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
extern PLC_WP_CHK_CheckDebugPrint(GAMESYS_WORK * gsys);
static GMEVENT_RESULT allMapCheckEvent( GMEVENT * event, int *seq, void * wk )
{
  ALL_MAP_CHECK_WORK * amcw = wk;
  FIELDMAP_WORK * fieldmap;
  u16 zone_id;

  enum {
    SEQ_INIT,
    SEQ_SEEK_ID,
    SEQ_WAIT,
    SEQ_FLY_CHECK,
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
      OS_Printf( "FieldHeap restmin = 0x%x: zone = %d\n",DbgFldHeapRest, DbgFldHeapUseMaxZone );
      OS_Printf( "3DVram restmin = 0x%x: zone = %d\n",DbgVramRest, DbgVramUseMaxZone );
      OS_Printf( "HUDSON: ALL MAP CHECK SUCCESS!!\n" ); // hudson 検出用
      return GMEVENT_RES_FINISH;
    }
    {
      char buf[ZONEDATA_NAME_LENGTH];
      ZONEDATA_DEBUG_GetZoneName(buf, zone_id);
      OS_TPrintf( "\t[[ALL MAP CHECKING... :%s]]!!\n", buf );
    }
    OS_Printf( "ALL MAP CHECK: zone_id = %d\n", zone_id );
    fieldmap = GAMESYSTEM_GetFieldMapWork( amcw->gsys );
    GMEVENT_CallEvent( event,
        DEBUG_EVENT_ChangeMapDefaultPos( amcw->gsys, fieldmap, zone_id ) );
    *seq = SEQ_WAIT;
    break;
  case SEQ_WAIT:
    amcw->wait ++;
    if ( amcw->wait > 120 )
    {
      fieldmap = GAMESYSTEM_GetFieldMapWork( amcw->gsys );
      //フィールドヒープの残りとＶＲＡＭの残りを計測
      DbgRestDataUpdate( FIELDMAP_GetZoneID( fieldmap ) );
      amcw->wait = 0;

      //パレスワープ禁止場所をダンプ
      PLC_WP_CHK_CheckDebugPrint(amcw->gsys);

      *seq = SEQ_FLY_CHECK/*SEQ_SEEK_ID*/;
    }
    break;
  case SEQ_FLY_CHECK:
    fieldmap = GAMESYSTEM_GetFieldMapWork( amcw->gsys );
    {
      GMEVENT *call_evt;
      FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(fieldmap);
      call_evt = FLD3D_CI_CreateCutInEvt(amcw->gsys, ptr, FLDCIID_FLY_OUT);
      GMEVENT_CallEvent( event, call_evt );
      *seq = SEQ_SEEK_ID;
    }
    break;  
  }

  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * EVENT_DEBUG_AllMapCheck( GAMESYS_WORK * gsys, BOOL up_to_flag )
{
  GMEVENT * new_event;
  ALL_MAP_CHECK_WORK * amcw;
  new_event = GMEVENT_Create( gsys, NULL, allMapCheckEvent, sizeof(ALL_MAP_CHECK_WORK) );
  amcw = GMEVENT_GetEventWork( new_event );
  amcw->up_to_flag = up_to_flag;
  amcw->gsys = gsys;
  return new_event;
}




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
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( p_wk->gmSys,
      FS_OVERLAY_ID( debug_menu_zone_jump ), DEBUG_EVENT_DebugMenu_ZoneJump, p_wk );

  GMEVENT_ChangeEvent( p_wk->gmEvent, event );

  return TRUE;
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
  DEBUG_ZKNCMD_MANUAL_SET,
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

typedef struct {
  FIELDMAP_WORK * fieldWork;
  ZUKAN_SAVEDATA * sv;
  HEAPID  heapID;

  GFL_MSGDATA * msgData;
  FLDSYSWIN * win;

  u16 mode;
  u16 pos;
  s16 start;
  s16 end;

}DEBUG_ZUKAN_MANUAL_WORK;

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
  { DEBUG_FIELD_ZUKAN_01, (void*)DEBUG_ZKNCMD_GLOBAL_GET_FULL },  // 全国捕獲コンプ
  { DEBUG_FIELD_ZUKAN_07, (void*)DEBUG_ZKNCMD_GLOBAL_SEE_FULL },  // 全国見たコンプ
  { DEBUG_FIELD_ZUKAN_10, (void*)DEBUG_ZKNCMD_LOCAL_GET_FULL },   // 地方捕獲コンプ
  { DEBUG_FIELD_ZUKAN_11, (void*)DEBUG_ZKNCMD_LOCAL_SEE_FULL },   // 地方見たコンプ
  { DEBUG_FIELD_ZUKAN_09, (void*)DEBUG_ZKNCMD_RANDOM },           // ランダムセット
  { DEBUG_FIELD_ZUKAN_03, (void*)DEBUG_ZKNCMD_FORM_FULL },        // フォルムコンプ
  { DEBUG_FIELD_ZUKAN_12, (void*)DEBUG_ZKNCMD_LANGUAGE_FULL },    // 言語コンプ
  { DEBUG_FIELD_ZUKAN_05, (void*)DEBUG_ZKNCMD_ZENKOKU_FLAG },     // 全国フラグ
  { DEBUG_FIELD_ZUKAN_06, (void*)DEBUG_ZKNCMD_VERSION_UP },       // バージョンアップ
  { DEBUG_FIELD_ZUKAN_13, (void*)DEBUG_ZKNCMD_LOCAL_SEE_25 },     // 地方見た２５
  { DEBUG_FIELD_ZUKAN_14, (void*)DEBUG_ZKNCMD_LOCAL_SEE_60 },     // 地方見た６０
  { DEBUG_FIELD_ZUKAN_15, (void*)DEBUG_ZKNCMD_LOCAL_SEE_120 },    // 地方見た１２０
  { DEBUG_FIELD_ZUKAN_16, (void*)DEBUG_ZKNCMD_MANUAL_SET },       // 登録切り替え
};

static const DEBUG_MENU_INITIALIZER DebugMenuZukanImitializer = {
  NARC_debug_message_d_field_dat,
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
static GMEVENT_RESULT ZukanManualSet( GMEVENT *event, int *seq, void *wk );


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
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      return GMEVENT_RES_FINISH;

    case DEBUG_ZKNCMD_LOCAL_SEE_FULL:     // 地方見た
      SetZukanLocal( wk, MONSNO_END, 1 );
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

    case DEBUG_ZKNCMD_MANUAL_SET:       // 登録切り替え
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      {
        FIELDMAP_WORK * fieldWork = work->fieldWork;
        ZUKAN_SAVEDATA * sv = work->sv;
        HEAPID  heapID = work->heapID;
        DEBUG_ZUKAN_MANUAL_WORK * mwk;

        GMEVENT_Change( event, ZukanManualSet, sizeof(DEBUG_ZUKAN_MANUAL_WORK) );

        mwk = GMEVENT_GetEventWork( event );
        mwk->sv = sv;
        mwk->fieldWork = fieldWork;
        mwk->heapID = heapID;
      }
      return GMEVENT_RES_CONTINUE;
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
    if( buf[i] != POKEPER_CHIHOU_NO_NONE ){
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

static void DZM_ModePut( DEBUG_ZUKAN_MANUAL_WORK * wk )
{
  FLDSYSWIN_FillClearWindow( wk->win, 0, 40, 22*8, 16 );

  if( wk->mode == 0 ){
    FLDSYSWIN_Print( wk->win, 0, 40, DEBUG_FIELD_ZUKAN_17 );
  }else if( wk->mode == 1 ){
    FLDSYSWIN_Print( wk->win, 0, 40, DEBUG_FIELD_ZUKAN_18 );
  }else{
    FLDSYSWIN_Print( wk->win, 0, 40, DEBUG_FIELD_ZUKAN_19 );
  }
}

static void DZM_MonsPut( DEBUG_ZUKAN_MANUAL_WORK * wk, u32 pos )
{
  STRBUF * str;
  STRBUF * exp;
  WORDSET * wset;

  FLDSYSWIN_FillClearWindow( wk->win, 0, 56+pos*16, 22*8, 16 );

  wset = WORDSET_Create( wk->heapID );
  exp  = GFL_STR_CreateBuffer( 128, wk->heapID );

  if( pos == 0 ){
    str = GFL_MSG_CreateString( wk->msgData, DEBUG_FIELD_ZUKAN_20 );
    WORDSET_RegisterNumber( wset, 0, wk->start, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterPokeMonsNameNo( wset, 1, wk->start );
  }else{
    str = GFL_MSG_CreateString( wk->msgData, DEBUG_FIELD_ZUKAN_21 );
    WORDSET_RegisterNumber( wset, 0, wk->end, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterPokeMonsNameNo( wset, 1, wk->end );
  }

  WORDSET_ExpandStr( wset, exp, str );

  if( wk->pos == pos ){
    FLDSYSWIN_PrintStrBufColor( wk->win, 0, 56+pos*16, exp, PRINTSYS_LSB_Make(3,4,15) );
  }else{
    FLDSYSWIN_PrintStrBufColor( wk->win, 0, 56+pos*16, exp, PRINTSYS_LSB_Make(1,2,15) );
  }

  GFL_STR_DeleteBuffer( str );
  GFL_STR_DeleteBuffer( exp );
  WORDSET_Delete( wset );
}

static void ZukanManualCountUp( DEBUG_ZUKAN_MANUAL_WORK * wk, s16 val )
{
  s16 * dat;

  if( wk->pos == 0 ){
    dat = &wk->start;
  }else{
    dat = &wk->end;
  }

  *dat += val;

  if( *dat > MONSNO_END ){
    *dat = *dat - MONSNO_END;
  }else if( *dat < 1 ){
    *dat = *dat + MONSNO_END;
  }

  DZM_MonsPut( wk, wk->pos );
}

static void DZM_DataMake( DEBUG_ZUKAN_MANUAL_WORK * wk )
{
  u16 start, end;

  if( wk->start > wk->end ){
    start = wk->end;
    end   = wk->start;
  }else{
    start = wk->start;
    end   = wk->end;
  }

  // みた
  if( wk->mode == 0 ){
    ZUKANSAVE_DebugDataSetSee( wk->sv, start, end, wk->heapID );
  // ほかく
  }else if( wk->mode == 1 ){
    ZUKANSAVE_DebugDataSetGet( wk->sv, start, end, wk->heapID );
  // 範囲クリア
  }else{
    ZUKANSAVE_DebugDataClear( wk->sv, start, end );
  }
}

static GMEVENT_RESULT ZukanManualSet( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ZUKAN_MANUAL_WORK * work = wk;

  switch( (*seq) ){
  case 0:
    work->mode  = 0;
    work->pos   = 0;
    work->start = 1;
    work->end   = 1;
    {
      FLDMSGBG * msgBG;
      msgBG = FIELDMAP_GetFldMsgBG( work->fieldWork );
      work->msgData = DEBUGFLDMENU_CreateMSGDATA(
          work->fieldWork, NARC_debug_message_d_field_dat );
      work->win = FLDSYSWIN_AddEx( msgBG, work->msgData, 1, 1, 22, 11 );

      FLDSYSWIN_Print( work->win, 0, 0, DEBUG_FIELD_ZUKAN_22 );
      DZM_ModePut( work );
      DZM_MonsPut( work, 0 );
      DZM_MonsPut( work, 1 );
    }
    (*seq)++;
    break;

  case 1:
    // モード切り替え
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      work->mode += 1;
      if( work->mode >= 3 ){
        work->mode = 0;
      }
      DZM_ModePut( work );
      break;
    }
    // 反映
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
      DZM_DataMake( work );
      PMSND_PlaySE( SEQ_SE_SAVE );
      break;
    }
    // 決定
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      work->pos ^= 1;
      DZM_MonsPut( work, 0 );
      DZM_MonsPut( work, 1 );
      break;
    }
    // 終了
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
      FLDSYSWIN_ClearWindow( work->win );
      FLDSYSWIN_Delete( work->win );
      GFL_MSG_Delete( work->msgData );
      return GMEVENT_RES_FINISH;
    }
    // +1
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
      ZukanManualCountUp( work, 1 );
      break;
    }
    // -1
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
      ZukanManualCountUp( work, -1 );
      break;
    }
    // +10
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
      ZukanManualCountUp( work, 10 );
      break;
    }
    // -10
    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
      ZukanManualCountUp( work, -10 );
      break;
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
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
  NARC_debug_message_d_field_dat,
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
  NARC_debug_message_d_field_dat,
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
            //一番重いのはペンドラー
            MUSICAL_DEBUG_CreateDummyData( shotData , 545 , work->heapId );
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

static void DbgRestDataUpdate(const u32 inZone)
{
  u32 heap_rest = GFL_HEAP_GetHeapFreeSize( HEAPID_FIELDMAP );
  u32 vram_rest = 0;

  NOZOMU_Printf("Zone %d seach..\n", inZone );

  if ( DbgFldHeapRest >= heap_rest )
  {
    DbgFldHeapRest = heap_rest;
    DbgFldHeapUseMaxZone = inZone;
  }
  
  NNS_GfdDumpLnkTexVramManagerEx( VramDumpCallBack, VramDumpCallBack, &vram_rest );

  if ( DbgVramRest >= vram_rest )
  {
    DbgVramRest = vram_rest;
    DbgVramUseMaxZone = inZone;
  }
  OS_Printf( "FieldHeap restmin = 0x%x: zone = %d\n",DbgFldHeapRest, DbgFldHeapUseMaxZone );
  OS_Printf( "3DVram restmin = 0x%x: zone = %d\n",DbgVramRest, DbgVramUseMaxZone );
}

static void VramDumpCallBack( u32 addr, u32 szByte, void* pUserData )
{
    // 合計サイズを計算。
    (*((u32*)pUserData)) += szByte;
    // 情報をデバックコンソールに出力
    NOZOMU_Printf("adr=0x%08x:  size=0x%08x    \n", addr, szByte );
    NOZOMU_Printf("Free_total 0x%x \n", (*((u32*)pUserData)) );
    
}




//-----------------------------------------------------------------------------
/**
 *    WFBC自動チェック
 */
//-----------------------------------------------------------------------------
//-------------------------------------
/// イベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK* p_gsys;
  FIELDMAP_WORK* p_fieldmap;
} EV_DEBUGMENU_WFBC;


static GMEVENT_RESULT EV_debugMenu_WfbcDebug( GMEVENT *event, int *seq, void *wk );
//----------------------------------------------------------------------------
/**
 *  @brief  WFBCオートデバック
 */
//-----------------------------------------------------------------------------
static BOOL debugMenu_WfbcDebug( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  EV_DEBUGMENU_WFBC *work;

  GMEVENT_Change( event, EV_debugMenu_WfbcDebug, sizeof(EV_DEBUGMENU_WFBC) );

  work = GMEVENT_GetEventWork( event );

  work->p_gsys = gsys;
  work->p_fieldmap = fieldWork;

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  自動動作
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EV_debugMenu_WfbcDebug( GMEVENT *event, int *seq, void *wk )
{
  EV_DEBUGMENU_WFBC *work = wk;

  
  switch( *seq ){
  case 0:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(debug_menu_wfbc_check) );
#ifdef DEBUG_ONLY_FOR_tomoya_takahashi
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){
      GMEVENT_CallEvent( event, DEBUG_WFBC_PeopleCheck( work->p_gsys ) );
    }
    else
#endif //DEBUG_ONLY_FOR_tomoya_takahashi
    {
      GMEVENT_CallEvent( event, DEBUG_WFBC_View( work->p_gsys ) );
    }
    (*seq)++;
    break;

  case 1:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(debug_menu_wfbc_check) );
    return ( GMEVENT_RES_FINISH );
  }

  return ( GMEVENT_RES_CONTINUE );
}



#endif  //  PM_DEBUG

