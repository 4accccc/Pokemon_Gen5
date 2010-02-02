//==============================================================================
/**
 * @file  save.c
 * @brief WBセーブ
 * @author  matsuda
 * @date  2008.08.27(水)
 */
//==============================================================================
#include <gflib.h>
#include <backup_system.h>
#include "gamesystem/game_data.h"
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/box_savedata.h"
#include "savedata/myitem_savedata.h"
#include "poke_tool/pokeparty.h"
#include "playerdata_local.h"
#include "savedata/wifihistory.h"
#include "savedata/wifilist.h"
#include "savedata/c_gear_data.h"
#include "savedata/trainercard_data.h"
#include "savedata/mystery_data.h"
#include "savedata/dreamworld_data.h"
#include "savedata/situation.h"
#include "perapvoice_local.h"
#include "savedata/system_data.h"
#include "savedata/record.h"
#include "system/pms_word.h"
#include "savedata/mail_util.h"
#include "field/fldmmdl.h"
#include "savedata/musical_save.h"
#include "savedata/randommap_save.h"
#include "savedata/irc_compatible_savedata.h"
#include "savedata/worldtrade_data.h"
#include "savedata/regulation.h"
#include "field/eventwork.h"
#include "savedata/gimmickwork.h"
#include "savedata/battle_rec.h"
#include "savedata/battle_box_save.h"
#include "savedata/sodateya_work.h"
#include "savedata/wifi_negotiation.h"
#include "savedata/misc.h"
#include "savedata/intrude_save.h"
#include "savedata/shortcut.h"
#include "savedata/musical_dist_save.h"
#include "savedata/zukan_savedata.h"
#include "savedata/encount_sv.h"
#include "savedata/c_gear_picture.h"
#include "savedata/bsubway_savedata.h"
#include "savedata/rndmatch_savedata.h"
#include "savedata/radar_save.h"
#include "savedata/symbol_save.h"
#include "savedata/my_pms_data.h"
#include "savedata/battle_examination.h"
#include "savedata/anketo_save.h"
#include "savedata/un_savedata.h"

//==============================================================================
//  定数定義
//==============================================================================
#define MAGIC_NUMBER  (0x31053527)
#define SECTOR_MAX    (SAVE_PAGE_MAX)

///ノーマル領域で使用するセーブサイズ
#define SAVESIZE_NORMAL   (SAVE_SECTOR_SIZE * SECTOR_MAX)

///ミラーリング領域で使用するセーブ開始アドレス
#define MIRROR_SAVE_ADDRESS   (0x24000)
///外部セーブが使用するセーブ開始アドレス
#define EXTRA_SAVE_START_ADDRESS    (0x48000)

///外部セーブ：メモリマッピング
enum{
  EXTRA_MM_REC_MINE = EXTRA_SAVE_START_ADDRESS,
  EXTRA_MM_REC_MINE_MIRROR = EXTRA_MM_REC_MINE + SAVESIZE_EXTRA_BATTLE_REC,
  
  EXTRA_MM_REC_DL_0 = EXTRA_MM_REC_MINE_MIRROR + SAVESIZE_EXTRA_BATTLE_REC,
  EXTRA_MM_REC_DL_0_MIRROR = EXTRA_MM_REC_DL_0 + SAVESIZE_EXTRA_BATTLE_REC,
  
  EXTRA_MM_REC_DL_1 = EXTRA_MM_REC_DL_0_MIRROR + SAVESIZE_EXTRA_BATTLE_REC,
  EXTRA_MM_REC_DL_1_MIRROR = EXTRA_MM_REC_DL_1 + SAVESIZE_EXTRA_BATTLE_REC,
  
  EXTRA_MM_REC_DL_2 = EXTRA_MM_REC_DL_1_MIRROR + SAVESIZE_EXTRA_BATTLE_REC,
  EXTRA_MM_REC_DL_2_MIRROR = EXTRA_MM_REC_DL_2 + SAVESIZE_EXTRA_BATTLE_REC,
  
  EXTRA_MM_CGEAR = EXTRA_MM_REC_DL_2_MIRROR + SAVESIZE_EXTRA_BATTLE_REC,

  EXTRA_MM_BATTLE_EXAMINATION = EXTRA_MM_CGEAR + SAVESIZE_EXTRA_CGEAR_PICTURE,

  EXTRA_MM_STREAMING = EXTRA_MM_BATTLE_EXAMINATION + SAVESIZE_EXTRA_BATTLE_EXAMINATION,
};


//--------------------------------------------------------------
//  通常セーブデータのテーブル
//--------------------------------------------------------------
static const GFL_SAVEDATA_TABLE SaveDataTbl_Normal[] = {
  { //BOXデータ
    GMDATA_ID_BOXDATA,
    (FUNC_GET_SIZE)BOXDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXDAT_Init,
  },
  { //BOXトレーデータ01
    GMDATA_ID_BOXTRAY_01,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ02
    GMDATA_ID_BOXTRAY_02,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ03
    GMDATA_ID_BOXTRAY_03,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ04
    GMDATA_ID_BOXTRAY_04,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ05
    GMDATA_ID_BOXTRAY_05,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ06
    GMDATA_ID_BOXTRAY_06,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ07
    GMDATA_ID_BOXTRAY_07,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ08
    GMDATA_ID_BOXTRAY_08,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ09
    GMDATA_ID_BOXTRAY_09,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ10
    GMDATA_ID_BOXTRAY_10,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ11
    GMDATA_ID_BOXTRAY_11,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ12
    GMDATA_ID_BOXTRAY_12,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ13
    GMDATA_ID_BOXTRAY_13,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ14
    GMDATA_ID_BOXTRAY_14,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ15
    GMDATA_ID_BOXTRAY_15,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ16
    GMDATA_ID_BOXTRAY_16,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ17
    GMDATA_ID_BOXTRAY_17,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ18
    GMDATA_ID_BOXTRAY_18,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ19
    GMDATA_ID_BOXTRAY_19,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ20
    GMDATA_ID_BOXTRAY_20,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ21
    GMDATA_ID_BOXTRAY_21,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ22
    GMDATA_ID_BOXTRAY_22,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ23
    GMDATA_ID_BOXTRAY_23,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //BOXトレーデータ24
    GMDATA_ID_BOXTRAY_24,
    (FUNC_GET_SIZE)BOXTRAYDAT_GetTotalSize,
    (FUNC_INIT_WORK)BOXTRAYDAT_Init,
  },
  { //手持ちアイテム
    GMDATA_ID_MYITEM,
    (FUNC_GET_SIZE)MYITEM_GetWorkSize,
    (FUNC_INIT_WORK)MYITEM_Init,
  },
  { //手持ちポケモン
    GMDATA_ID_MYPOKE,
    (FUNC_GET_SIZE)PokeParty_GetWorkSize,
    (FUNC_INIT_WORK)PokeParty_InitWork,
  },
  { //プレイヤーデータ
    GMDATA_ID_PLAYER_DATA,
    (FUNC_GET_SIZE)PLAYERDATA_GetWorkSize,
    (FUNC_INIT_WORK)PLAYERDATA_Init,
  },
  { //状況データ
    GMDATA_ID_SITUATION,
    (FUNC_GET_SIZE)Situation_GetWorkSize,
    (FUNC_INIT_WORK)Situation_Init,
  },
  { //WiFi通信履歴データ
    GMDATA_ID_WIFIHISTORY,
    (FUNC_GET_SIZE)WIFIHISTORY_GetWorkSize,
    (FUNC_INIT_WORK)WIFIHISTORY_Init,
  },
  { //WiFi友達コードデータ
    GMDATA_ID_WIFILIST,
    (FUNC_GET_SIZE)WifiList_GetWorkSize,
    (FUNC_INIT_WORK)WifiList_Init,
  },
  { //WiFiネゴシエーション
    GMDATA_ID_WIFI_NEGOTIATION,
    (FUNC_GET_SIZE)WIFI_NEGOTIATION_SV_GetWorkSize,
    (FUNC_INIT_WORK)WIFI_NEGOTIATION_SV_Init,
  },
  { // C-GEAR
    GMDATA_ID_CGEAR,
    (FUNC_GET_SIZE)CGEAR_SV_GetWorkSize,
    (FUNC_INIT_WORK)CGEAR_SV_Init,
  },
  { //トレーナーカード(サイン
    GMDATA_ID_TRCARD,
    (FUNC_GET_SIZE)TRCSave_GetSaveDataSize,
    (FUNC_INIT_WORK)TRCSave_InitSaveData,
  },
  { //不思議な贈り物
    GMDATA_ID_MYSTERYDATA,
    (FUNC_GET_SIZE)MYSTERYDATA_GetWorkSize,
    (FUNC_INIT_WORK)MYSTERYDATA_Init,
  },
  { //PDW
    GMDATA_ID_DREAMWORLD,
    (FUNC_GET_SIZE)DREAMWORLD_SV_GetWorkSize,
    (FUNC_INIT_WORK)DREAMWORLD_SV_Init,
  },
  { //ぺラップボイス
    GMDATA_ID_PERAPVOICE,
    (FUNC_GET_SIZE)PERAPVOICE_GetWorkSize,
    (FUNC_INIT_WORK)PERAPVOICE_Init,
  },
  { //システムデータ
    GMDATA_ID_SYSTEM_DATA,
    (FUNC_GET_SIZE)SYSTEMDATA_GetWorkSize,
    (FUNC_INIT_WORK)SYSTEMDATA_Init,
  },
  { //レコード(スコア
    GMDATA_ID_RECORD,
    (FUNC_GET_SIZE)RECORD_GetWorkSize,
    (FUNC_INIT_WORK)RECORD_Init,
  },
  { //簡易文
    GMDATA_ID_PMS,
    (FUNC_GET_SIZE)PMSW_GetSaveDataSize,
    (FUNC_INIT_WORK)PMSW_InitSaveData,
  },
  { //メールデータ
    GMDATA_ID_MAILDATA,
    (FUNC_GET_SIZE)MAIL_GetBlockWorkSize,
    (FUNC_INIT_WORK)MAIL_Init,
  },
  {//動作モデル
    GMDATA_ID_MMDL,
    (FUNC_GET_SIZE)MMDL_SAVEDATA_GetWorkSize,
    (FUNC_INIT_WORK)MMDL_SAVEDATA_Init,
  },
  { //ミュージカル
    GMDATA_ID_MUSICAL,
    (FUNC_GET_SIZE)MUSICAL_SAVE_GetWorkSize,
    (FUNC_INIT_WORK)MUSICAL_SAVE_InitWork,
  },
  { //ランダム生成マップ
    GMDATA_ID_RANDOMMAP,  
    (FUNC_GET_SIZE)RANDOMMAP_SAVE_GetWorkSize,
    (FUNC_INIT_WORK)RANDOMMAP_SAVE_InitWork,
  },
  { //相性チェック
    GMDATA_ID_IRCCOMPATIBLE,
    (FUNC_GET_SIZE)IRC_COMPATIBLE_SV_GetWorkSize,
    (FUNC_INIT_WORK)IRC_COMPATIBLE_SV_Init,
  },
  { //イベントワーク
    GMDATA_ID_EVENT_WORK,
    (FUNC_GET_SIZE)EVENTWORK_GetWorkSize,
    (FUNC_INIT_WORK)EVENTWORK_InitWork,
  },
  { //GTS
    GMDATA_ID_WORLDTRADEDATA,
    (FUNC_GET_SIZE)WorldTradeData_GetWorkSize,
    (FUNC_INIT_WORK)WorldTradeData_Init,
  },
  { //REGULATION
    GMDATA_ID_REGULATION_DATA,
    (FUNC_GET_SIZE)RegulationData_GetWorkSize,
    (FUNC_INIT_WORK)RegulationData_Init,
  },
  { //フィールドギミックワーク
    GMDATA_ID_GIMMICK_WORK,
    (FUNC_GET_SIZE)GIMMICKWORK_GetWorkSize,
    (FUNC_INIT_WORK)GIMMICKWORK_Init,
  },
  { //バトルボックス
    GMDATA_ID_BATTLE_BOX,
    (FUNC_GET_SIZE)BATTLE_BOX_SAVE_GetWorkSize,
    (FUNC_INIT_WORK)BATTLE_BOX_SAVE_InitWork,
  },
  { // 育て屋ワーク
    GMDATA_ID_SODATEYA_WORK,
    (FUNC_GET_SIZE)SODATEYA_WORK_GetWorkSize,
    (FUNC_INIT_WORK)SODATEYA_WORK_InitWork,
  },
  { // かいりき岩場所テーブル
    GMDATA_ID_ROCKPOS,
    (FUNC_GET_SIZE)MMDL_ROCKPOS_GetWorkSize,
    (FUNC_INIT_WORK)MMDL_ROCKPOS_Init,
  },
  { //未分類ワーク
    GMDATA_ID_MISC,
    (FUNC_GET_SIZE)MISC_GetWorkSize,
    (FUNC_INIT_WORK)MISC_Init,
  },
  { //侵入セーブデータ
    GMDATA_ID_INTRUDE,
    (FUNC_GET_SIZE)IntrudeSave_GetWorkSize,
    (FUNC_INIT_WORK)IntrudeSave_WorkInit,
  },
  { //Yボタン登録セーブデータ
    GMDATA_ID_SHORTCUT,
    (FUNC_GET_SIZE)SHORTCUT_GetWorkSize,
    (FUNC_INIT_WORK)SHORTCUT_Init,
  },
  { //ポケモン図鑑セーブデータ
    GMDATA_ID_ZUKAN,
    (FUNC_GET_SIZE)ZUKANSAVE_GetWorkSize,
    (FUNC_INIT_WORK)ZUKANSAVE_Init,
  },
  { //エンカウント関連セーブデータ
    GMDATA_ID_ENCOUNT,
    (FUNC_GET_SIZE)EncDataSave_GetWorkSize,
    (FUNC_INIT_WORK)EncDataSave_Init,
  },
  { //バトルサブウェイ　プレイデータ
    GMDATA_ID_BSUBWAY_PLAYDATA,
    (FUNC_GET_SIZE)BSUBWAY_PLAYDATA_GetWorkSize,
    (FUNC_INIT_WORK)BSUBWAY_PLAYDATA_Init,
  },
  { //バトルサブウェイ　スコアデータ
    GMDATA_ID_BSUBWAY_SCOREDATA,
    (FUNC_GET_SIZE)BSUBWAY_SCOREDATA_GetWorkSize,
    (FUNC_INIT_WORK)BSUBWAY_SCOREDATA_Init,
  },
  { 
    //ランダムマッチセーブデータ
    GMDATA_ID_RNDMATCH,
    (FUNC_GET_SIZE)RNDMATCH_GetWorkSize,
    (FUNC_INIT_WORK)RNDMATCH_Init,
  },
  { //調査レーダー
    GMDATA_ID_RADAR,
    (FUNC_GET_SIZE)RadarSave_GetWorkSize,
    (FUNC_INIT_WORK)RadarSave_WorkInit,
  },
  { //シンボルエンカウント
    GMDATA_ID_SYMBOL,
    (FUNC_GET_SIZE)SymbolSave_GetWorkSize,
    (FUNC_INIT_WORK)SymbolSave_WorkInit,
  },
  { //主人公の挨拶文データ
    GMDATA_ID_MYPMS,
    (FUNC_GET_SIZE)MYPMS_GetWorkSize,
    (FUNC_INIT_WORK)MYPMS_Init,
  },
  { //すれ違いアンケート
    GMDATA_ID_ANKETO,
    (FUNC_GET_SIZE)AnketoSave_GetWorkSize,
    (FUNC_INIT_WORK)AnketoSave_WorkInit,
  },
  { //国連
    GMDATA_ID_UNSV,
    (FUNC_GET_SIZE)UNSV_GetWorkSize,
    (FUNC_INIT_WORK)UNSV_Init,
  },
};

//--------------------------------------------------------------
//  外部セーブデータのテーブル(ミラーリング有)
//--------------------------------------------------------------
///戦闘録画：自分 
///  ※GFL_SAVEDATA_TABLE全体でメモリは確保されます。
///    その為サイズが大きい録画データは一つ一つ別テーブルにしています)
static const GFL_SAVEDATA_TABLE SaveDataTbl_Extra_RecMine[] = {
  {
    EXGMDATA_ID_BATTLE_REC_MINE,
    (FUNC_GET_SIZE)BattleRec_GetWorkSize,
    (FUNC_INIT_WORK)BattleRec_WorkInit,
  },
};

///戦闘録画：ダウンロード0番
static const GFL_SAVEDATA_TABLE SaveDataTbl_Extra_RecDl_0[] = {
  {
    EXGMDATA_ID_BATTLE_REC_DL_0,
    (FUNC_GET_SIZE)BattleRec_GetWorkSize,
    (FUNC_INIT_WORK)BattleRec_WorkInit,
  },
};

///戦闘録画：ダウンロード1番
static const GFL_SAVEDATA_TABLE SaveDataTbl_Extra_RecDl_1[] = {
  {
    EXGMDATA_ID_BATTLE_REC_DL_1,
    (FUNC_GET_SIZE)BattleRec_GetWorkSize,
    (FUNC_INIT_WORK)BattleRec_WorkInit,
  },
};

///戦闘録画：ダウンロード2番
static const GFL_SAVEDATA_TABLE SaveDataTbl_Extra_RecDl_2[] = {
  {
    EXGMDATA_ID_BATTLE_REC_DL_2,
    (FUNC_GET_SIZE)BattleRec_GetWorkSize,
    (FUNC_INIT_WORK)BattleRec_WorkInit,
  },
};

//--------------------------------------------------------------
//  外部セーブデータのテーブル(ミラーリング無)
//--------------------------------------------------------------
///CGEAR絵のデータ
static const GFL_SAVEDATA_TABLE SaveDataTbl_Extra_CGEARPicture[] = {
  {
    EXGMDATA_ID_CGEAR_PICTURE,
    (FUNC_GET_SIZE)CGEAR_PICTURE_SAVE_GetWorkSize,
    (FUNC_INIT_WORK)CGEAR_PICTURE_SAVE_Init,
  },
};
///バトル検定
static const GFL_SAVEDATA_TABLE SaveDataTbl_Extra_BattleExamination[] = {
  {
    EXGMDATA_ID_BATTLE_EXAMINATION,
    (FUNC_GET_SIZE)BATTLE_EXAMINATION_SAVE_GetWorkSize,
    (FUNC_INIT_WORK)BATTLE_EXAMINATION_SAVE_Init,
  },
};
///ストリーミング
static const GFL_SAVEDATA_TABLE SaveDataTbl_Extra_Streaming[] = {
  {
    EXGMDATA_ID_STREAMING,
    (FUNC_GET_SIZE)MUSICAL_DIST_SAVE_GetWorkSize,
    (FUNC_INIT_WORK)MUSICAL_DIST_SAVE_InitWork,
  },
};


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   セーブパラメータテーブル
 */
//--------------------------------------------------------------
const GFL_SVLD_PARAM SaveParam_Normal = {
  SaveDataTbl_Normal,
  NELEMS(SaveDataTbl_Normal),
  0,                //バックアップ領域先頭アドレス
  MIRROR_SAVE_ADDRESS,      //ミラーリング領域先頭アドレス
  SAVESIZE_NORMAL,        //使用するバックアップ領域の大きさ
  MAGIC_NUMBER,
};

//--------------------------------------------------------------
/**
 * @brief   外部セーブパラメータテーブル
 *          ※SAVE_EXTRA_IDと並びを同じにしておくこと！！
 */
//--------------------------------------------------------------
const GFL_SVLD_PARAM SaveParam_ExtraTbl[] = {
  {//戦闘録画：自分
    SaveDataTbl_Extra_RecMine,
    NELEMS(SaveDataTbl_Extra_RecMine),
    EXTRA_MM_REC_MINE,              //バックアップ領域先頭アドレス
    EXTRA_MM_REC_MINE_MIRROR,       //ミラーリング領域先頭アドレス
    SAVESIZE_EXTRA_BATTLE_REC,      //使用するバックアップ領域の大きさ
    MAGIC_NUMBER,
  },
  {//戦闘録画：ダウンロード0番
    SaveDataTbl_Extra_RecDl_0,
    NELEMS(SaveDataTbl_Extra_RecDl_0),
    EXTRA_MM_REC_DL_0,              //バックアップ領域先頭アドレス
    EXTRA_MM_REC_DL_0_MIRROR,       //ミラーリング領域先頭アドレス
    SAVESIZE_EXTRA_BATTLE_REC,      //使用するバックアップ領域の大きさ
    MAGIC_NUMBER,
  },
  {//戦闘録画：ダウンロード1番
    SaveDataTbl_Extra_RecDl_1,
    NELEMS(SaveDataTbl_Extra_RecDl_1),
    EXTRA_MM_REC_DL_1,              //バックアップ領域先頭アドレス
    EXTRA_MM_REC_DL_1_MIRROR,       //ミラーリング領域先頭アドレス
    SAVESIZE_EXTRA_BATTLE_REC,      //使用するバックアップ領域の大きさ
    MAGIC_NUMBER,
  },
  {//戦闘録画：ダウンロード2番
    SaveDataTbl_Extra_RecDl_2,
    NELEMS(SaveDataTbl_Extra_RecDl_2),
    EXTRA_MM_REC_DL_2,              //バックアップ領域先頭アドレス
    EXTRA_MM_REC_DL_2_MIRROR,       //ミラーリング領域先頭アドレス
    SAVESIZE_EXTRA_BATTLE_REC,      //使用するバックアップ領域の大きさ
    MAGIC_NUMBER,
  },
  {//外部セーブパラメータテーブル：CGEARの絵(ミラーリング無)
    SaveDataTbl_Extra_CGEARPicture,
    NELEMS(SaveDataTbl_Extra_CGEARPicture),
    EXTRA_MM_CGEAR,             //バックアップ領域先頭アドレス
    EXTRA_MM_CGEAR,             //ミラーリング領域先頭アドレス ※ミラー無し指定
    SAVESIZE_EXTRA_CGEAR_PICTURE,       //使用するバックアップ領域の大きさ
    MAGIC_NUMBER,
  },
  {//外部セーブパラメータテーブル：バトル検定(ミラーリング無)
    SaveDataTbl_Extra_BattleExamination,
    NELEMS(SaveDataTbl_Extra_BattleExamination),
    EXTRA_MM_BATTLE_EXAMINATION,             //バックアップ領域先頭アドレス
    EXTRA_MM_BATTLE_EXAMINATION,             //ミラーリング領域先頭アドレス ※ミラー無し指定
    SAVESIZE_EXTRA_BATTLE_EXAMINATION,       //使用するバックアップ領域の大きさ
    MAGIC_NUMBER,
  },
  {//外部セーブパラメータテーブル：ストリーミング(ミラーリング無)
    SaveDataTbl_Extra_Streaming,
    NELEMS(SaveDataTbl_Extra_Streaming),
    EXTRA_MM_STREAMING,             //バックアップ領域先頭アドレス
    EXTRA_MM_STREAMING,             //ミラーリング領域先頭アドレス ※ミラー無し指定
    SAVESIZE_EXTRA_STREAMING,       //使用するバックアップ領域の大きさ
    MAGIC_NUMBER,
  },
};
SDK_COMPILER_ASSERT(NELEMS(SaveParam_ExtraTbl) == SAVE_EXTRA_ID_MAX);

