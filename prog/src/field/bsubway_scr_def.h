//======================================================================
/**
 * @file  bsubway_scr_def.h
 * @bfief  バトルサブウェイ　スクリプト関連定数定義
 * @author kagaya
 * @date  07.05.28
 * @note プラチナより移植
 */
//======================================================================
#ifndef __SUBWAY_SCR_DEF_H__
#define __SUBWAY_SCR_DEF_H__

//======================================================================
//  define
//======================================================================
#ifdef DEBUG_ONLY_FOR_kagaya
//#define DEBUG_BSW_COMM_MULTI_BTL_SKIP //定義で通信マルチバトルをスキップ
//#define DEBUG_BSW_BTL_SKIP //定義で戦闘スキップ
//#define DEBUG_BSW_FORCE_BTL_WIN //定義で強制勝ち状態
//#define DEBUG_BSW_COMM_IGNORE_POKE_OVERLAP //定義で通信ポケモン被り無視
//#define DEBUG_BSW_REGU_NONE //定義でレギュ無視
#define DEBUG_BSW_PRINT //定義でデバッグ出力有効
//#define DEBUG_ROM_TR_NAME_CHECK //定義でトレーナー名表示
#endif

#define BSUBWAY_SCRWORK_MAGIC  (0x12345678) ///<マジックナンバー

#define BSWAY_NULL_POKE  (0xFF)
#define BSWAY_NULL_TRAINER  (0xFFFF)
#define BSWAY_NULL_PARAM  (0)

#define BSW_RENSHOU_MAX (9999)

///バトルサブウェイ初期化モード
#define BSWAY_PLAY_NEW (0)
#define BSWAY_PLAY_CONTINUE (1)

///バトルサブウェイ　クリアラウンド数
#define BSWAY_CLEAR_WINCNT  (7)

//バトルサブウェイ　連勝記録ラウンド数
#define BSWAY_20_RENSHOU_CNT      (20)
#define BSWAY_50_RENSHOU_CNT      (50)
#define BSWAY_100_RENSHOU_CNT    (100)

#define LEADER_SET_1ST  (20+1)  //21人目のとき現在の連勝数は20
#define LEADER_SET_2ND  (48+1)  //49人目のとき現在の連勝数は48

#define BSWAY_PARTNER_NUM (3)  ///<5人衆の数
#define BSWAY_PARTNER_DATA_START_NO_MAN (300)  ///<5人衆データのスタートindex
#define BSWAY_PARTNER_DATA_START_NO_WOMAN (303) ///<5人衆データのスタートindex

///バトルサブウェイプレイモードスクリプト用定義 BSWAY_PLAYMODEと同一
#define BSWAY_MODE_SINGLE  (0)
#define BSWAY_MODE_DOUBLE  (1)
#define BSWAY_MODE_MULTI  (2)
#define BSWAY_MODE_COMM_MULTI (3)
#define BSWAY_MODE_WIFI (4)
#define BSWAY_MODE_S_SINGLE (5)
#define BSWAY_MODE_S_DOUBLE (6)
#define BSWAY_MODE_S_MULTI (7)
#define BSWAY_MODE_S_COMM_MULTI (8)
#define BSWAY_MODE_MAX (9)
#define BSWAY_MODE_NULL (10)

///バトルサブウェイ　パートナーコード
#define BSWAY_PTCODE_OBJ    (2)
#define BSWAY_PTCODE_MINE2  (1)
#define BSWAY_PTCODE_MINE  (0)

///バトルサブウェイ WIFI接続モード定義
#define BSWAY_WIFI_DL_PLAY    (0)
#define BSWAY_WIFI_UP_PLAY    (1)
#define BSWAY_WIFI_DL_LEADER    (2)

///通信マルチ送受信バッファサイズ
#define BSWAY_SIO_BUF_LEN  (70)

//もらえるグッズ
#define BSWAY_GOODS_COPPER  (GOODS_UG_TROPH_14)
#define BSWAY_GOODS_SILVER  (GOODS_UG_TROPH_15)
#define BSWAY_GOODS_GOLD    (GOODS_UG_TROPH_16)

//もらえるバトルポイント
#define BTLPOINT_VAL_BSUBWAY_LEADER  (20)
#define BTLPOINT_VAL_BSUBWAY_WIFI1  (3)      //ダウンロード
#define BTLPOINT_VAL_BSUBWAY_WIFI2  (4)
#define BTLPOINT_VAL_BSUBWAY_WIFI3  (5)
#define BTLPOINT_VAL_BSUBWAY_WIFI4  (6)
#define BTLPOINT_VAL_BSUBWAY_WIFI5  (7)
#define BTLPOINT_VAL_BSUBWAY_WIFI6  (8)
#define BTLPOINT_VAL_BSUBWAY_WIFI7  (9)
#define BTLPOINT_VAL_BSUBWAY_WIFI8  (10)
#define BTLPOINT_VAL_BSUBWAY_WIFI9  (10)
#define BTLPOINT_VAL_BSUBWAY_WIFI10  (10)
#define BTLPOINT_VAL_BSUBWAY_STAGE1  (3)      //シングル、ダブル、AIマルチ
#define BTLPOINT_VAL_BSUBWAY_STAGE2  (3)
#define BTLPOINT_VAL_BSUBWAY_STAGE3  (4)
#define BTLPOINT_VAL_BSUBWAY_STAGE4  (4)
#define BTLPOINT_VAL_BSUBWAY_STAGE5  (5)
#define BTLPOINT_VAL_BSUBWAY_STAGE6  (5)
#define BTLPOINT_VAL_BSUBWAY_STAGE7  (7) //ここはプログラム的にDPから間違い
#define BTLPOINT_VAL_BSUBWAY_STAGE8  (7)
#define BTLPOINT_VAL_BSUBWAY_COMM1  (8)      //マルチ、WIFI
#define BTLPOINT_VAL_BSUBWAY_COMM2  (9)
#define BTLPOINT_VAL_BSUBWAY_COMM3  (11)
#define BTLPOINT_VAL_BSUBWAY_COMM4  (12)
#define BTLPOINT_VAL_BSUBWAY_COMM5  (14)
#define BTLPOINT_VAL_BSUBWAY_COMM6  (15)
#define BTLPOINT_VAL_BSUBWAY_COMM7  (18)
#define BTLPOINT_VAL_BSUBWAY_COMM8  (18)

///スコアフラグアクセスID
#define BSWAY_SF_SILVER  (0)
#define BSWAY_SF_GOLD  (1)
#define BSWAY_SF_S50    (2)
#define BSWAY_SF_S100  (3)
#define BSWAY_SF_D50    (4)
#define BSWAY_SF_M50    (5)
#define BSWAY_SF_CM50  (6)
#define BSWAY_SF_W50    (7)

//--------------------------------------------------------------
//  トレーナーデータ、ポケモンデータ
//--------------------------------------------------------------
#define BSW_TR_ARCDATANO_ORG (1) ///<トレーナーデータアーカイブ開始番号
#define BSW_PM_ARCDATANO_ORG (1) ///<ポケモンデータアーカイブ開始番号

//トレーナーデータ番号　ボス
#define BSW_TR_DATANO_SINGLE (306) //シングル
#define BSW_TR_DATANO_S_SINGLE (307) //スーパーシングル
#define BSW_TR_DATANO_DOUBLE (308) //ダブル
#define BSW_TR_DATANO_S_DOUBLE (309) //スーパーダブル
#define BSW_TR_DATANO_MULTI0 (310) //マルチ0
#define BSW_TR_DATANO_MULTI1 (311) //マルチ1
#define BSW_TR_DATANO_S_MULTI0 (312) //スーパーマルチ0
#define BSW_TR_DATANO_S_MULTI1 (313) //スーパーマルチ1

#define BSW_TR_DATANO_MAX (300) //0-299 通常のトレーナーNo

//--------------------------------------------------------------
/// バトルサブウェイシーン制御
//--------------------------------------------------------------
//受付制御
#define BSWAY_SCENE_RECEIPT_NON (0) //何も無し
#define BSWAY_SCENE_RECEIPT_CONTINUE (1) //コンテニュー
#define BSWAY_SCENE_RECEIPT_AFTER (2) //ゲーム後　クリア
#define BSWAY_SCENE_RECEIPT_AFTER_LOSE (3) //ゲーム後　負け、キャンセル
#define BSWAY_SCENE_RECEIPT_ERROR (4) //エラー

//列車内制御
#define BSWAY_SCENE_TRAIN_NON (0) //何も無し
#define BSWAY_SCENE_TRAIN_FIRST (1) //乗車初回
#define BSWAY_SCENE_TRAIN_CONTINUE (2) //乗車中

//ホーム制御
#define BSWAY_SCENE_HOME_NON (0) //何も無し
#define BSWAY_SCENE_HOME_GAME_CLEAR (1) //ゲームクリア直後
#define BSWAY_SCENE_HOME_STOPOVER (2) //途中駅
#define BSWAY_SCENE_HOME_TERMINUS (3) //ホーム終点

//--------------------------------------------------------------
/// バトルサブウェイコマンドツールID　開始番号
//--------------------------------------------------------------
#define BSWTOOL_START_NO (0)
#define BSWTOOL_WIFI_START_NO (100)
#define BSWTOOL_DEBUG_START_NO (200)
#define BSWSUB_START_NO (300)
#define BSWSUB_COMM_START_NO (400)
#define BSWSUB_DEBUG_START_NO (500)

//--------------------------------------------------------------
/// バトルサブウェイコマンドツールID
/// ワーク依存無しコマンド
//--------------------------------------------------------------
#define BSWTOOL_GET_ZONE_PLAY_MODE (BSWTOOL_START_NO+0)
#define BSWTOOL_SYSTEM_RESET (BSWTOOL_START_NO+1)
#define BSWTOOL_CLEAR_PLAY_DATA (BSWTOOL_START_NO+2)
#define BSWTOOL_IS_SAVE_DATA_ENABLE (BSWTOOL_START_NO+3)
#define BSWTOOL_PUSH_NOW_LOCATION (BSWTOOL_START_NO+4)
#define BSWTOOL_POP_NOW_LOCATION (BSWTOOL_START_NO+5)
#define BSWTOOL_GET_SAVE_RENSHOU (BSWTOOL_START_NO+6)
#define BSWTOOL_SET_NG_SCORE (BSWTOOL_START_NO+7)
#define BSWTOOL_GET_NOW_ROUND (BSWTOOL_START_NO+8)
//#define BSWTOOL_INC_ROUND (BSWTOOL_START_NO+9)
//#define BSWTOOL_GET_RENSHOU_CNT (BSWTOOL_START_NO+10)
#define BSWTOOL_GET_NEXT_ROUND (BSWTOOL_START_NO+11)
#define BSWTOOL_GET_MINE_OBJ (BSWTOOL_START_NO+12)
#define BSWTOOL_OBJ_VANISH (BSWTOOL_START_NO+13)
#define BSWTOOL_GET_BOSS_CLEAR_FLAG (BSWTOOL_START_NO+14)
#define BSWTOOL_SET_BOSS_CLEAR_FLAG (BSWTOOL_START_NO+15)
#define BSWTOOL_GET_SUPPORT_ENCOUNT_END (BSWTOOL_START_NO+16)
#define BSWTOOL_SET_SUPPORT_ENCOUNT_END (BSWTOOL_START_NO+17)
#define BSWTOOL_SET_TRAIN (BSWTOOL_START_NO+18)
#define BSWTOOL_SET_TRAIN_ANM (BSWTOOL_START_NO+19)
#define BSWTOOL_SET_TRAIN_VANISH (BSWTOOL_START_NO+20)
#define BSWTOOL_GET_DATA_PLAY_MODE (BSWTOOL_START_NO+21)
#define BSWTOOL_CHK_MODE_MULTI (BSWTOOL_START_NO+22)
#define BSWTOOL_OBJ_HEIGHT_OFF (BSWTOOL_START_NO+23)
#define BSWTOOL_OBJ_HEIGHT_ON (BSWTOOL_START_NO+24)
#define BSWTOOL_CHK_EXIST_OBJ (BSWTOOL_START_NO+25)
#define BSWTOOL_GET_HOME_NPC_MSGID (BSWTOOL_START_NO+26)
#define BSWTOOL_GET_WIFI_RANK (BSWTOOL_START_NO+27)
#define BSWTOOL_UP_WIFI_RANK (BSWTOOL_START_NO+28)
#define BSWTOOL_DOWN_WIFI_RANK (BSWTOOL_START_NO+29)
#define BSWTOOL_GET_STAGE_NO (BSWTOOL_START_NO+30)
#define BSWTOOL_GET_OBJCODE_PARTNER (BSWTOOL_START_NO+31)
#define BSWTOOL_CHK_BTL_BOX_IN (BSWTOOL_START_NO+32)
#define BSWTOOL_CHK_DEBUG_ROM (BSWTOOL_START_NO+33)
#define BSWTOOL_SET_TTOWN_TRAIN (BSWTOOL_START_NO+34)
#define BSWTOOL_SET_OBJ_DIR (BSWTOOL_START_NO+35)
#define BSWTOOL_OBJ_PAUSE (BSWTOOL_START_NO+36)
#define BSWTOOL_WIFI_HOME_NPC_MSG (BSWTOOL_START_NO+37)
#define BSWTOOL_CHK_EXIST_STAGE (BSWTOOL_START_NO+38)
#define BSWTOOL_CHK_HOME_NPC_GETITEM (BSWTOOL_START_NO+39)
#define BSWTOOL_GET_HOME_NPC_GETITEM_MSGID_BEFORE (BSWTOOL_START_NO+40)
#define BSWTOOL_GET_HOME_NPC_GETITEM_MSGID_AFTER (BSWTOOL_START_NO+41)
#define BSWTOOL_GET_HOME_NPC_GETITEM_ITEMNO (BSWTOOL_START_NO+42)
#define BSWTOOL_GET_LEADERDATA_DATANUM (BSWTOOL_START_NO+43)
#define BSWTOOL_YESNO_WIN (BSWTOOL_START_NO+44)
#define BSWTOOL_END_NO (BSWTOOL_START_NO+45)

#define BSWTOOL_WIFI_SET_UPLOAD_FLAG (BSWTOOL_WIFI_START_NO+0)
#define BSWTOOL_WIFI_GET_UPLOAD_FLAG (BSWTOOL_WIFI_START_NO+1)
#define BSWTOOL_WIFI_CONNECT (BSWTOOL_WIFI_START_NO+2)
#define BSWTOOL_WIFI_UPLOAD_SCORE (BSWTOOL_WIFI_START_NO+3)
#define BSWTOOL_WIFI_DOWNLOAD_GAMEDATA (BSWTOOL_WIFI_START_NO+4)
#define BSWTOOL_WIFI_DOWNLOAD_SCDATA (BSWTOOL_WIFI_START_NO+5)
#define BSWTOOL_WIFI_GET_RANK (BSWTOOL_WIFI_START_NO+6)
#define BSWTOOL_WIFI_CHK_DL_DATA (BSWTOOL_WIFI_START_NO+7)
#define BSWTOOL_WIFI_SET_SEL_DL_BTL (BSWTOOL_WIFI_START_NO+8)
#define BSWTOOL_WIFI_CHK_SEL_DL_BTL (BSWTOOL_WIFI_START_NO+9)
#define BSWTOOL_WIFI_CHK_DL_SCDATA (BSWTOOL_WIFI_START_NO+10)
#define BSWTOOL_WIFI_EV_READ_SCDATA (BSWTOOL_WIFI_START_NO+11)
#define BSWTOOL_WIFI_GET_DL_SCDATA_RANK (BSWTOOL_WIFI_START_NO+12)
#define BSWTOOL_WIFI_GET_DL_SCDATA_ROOM (BSWTOOL_WIFI_START_NO+13)
#define BSWTOOL_WIFI_END_NO (BSWTOOL_WIFI_START_NO+14)

#define BSWTOOL_DEBUG_CHK_FLAG (BSWTOOL_DEBUG_START_NO+0)
#define BSWTOOL_DEBUG_PRINT_NO (BSWTOOL_DEBUG_START_NO+1)
#define BSWTOOL_DEBUG_PRINT_HEAP_SIZE (BSWTOOL_DEBUG_START_NO+2)
#define BSWTOOL_DEBUG_CHK_DEBUG_ROM (BSWTOOL_DEBUG_START_NO+3)
#define BSWTOOL_DEBUG_END_NO (BSWTOOL_DEBUG_START_NO+4)

//--------------------------------------------------------------
/// バトルサブウェイコマンドツールID
/// ワーク依存コマンド
//--------------------------------------------------------------
#define BSWSUB_SELECT_POKE (BSWSUB_START_NO+0)
#define BSWSUB_GET_ENTRY_POKE (BSWSUB_START_NO+1)
#define BSWSUB_CHK_ENTRY_POKE (BSWSUB_START_NO+2)
#define BSWSUB_IS_CLEAR (BSWSUB_START_NO+3)
#define BSWSUB_SET_LOSE_SCORE (BSWSUB_START_NO+4)
#define BSWSUB_SET_CLEAR_SCORE (BSWSUB_START_NO+5)
#define BSWSUB_SAVE_REST_PLAY_DATA (BSWSUB_START_NO+6)
#define BSWSUB_CHOICE_BTL_PARTNER (BSWSUB_START_NO+7)
#define BSWSUB_GET_ENEMY_OBJ (BSWSUB_START_NO+8)
#define BSWSUB_LOCAL_BTL_CALL (BSWSUB_START_NO+9)
#define BSWSUB_GET_PLAY_MODE (BSWSUB_START_NO+10)
#define BSWSUB_SET_PLAY_BTL_BOSS (BSWSUB_START_NO+11)
#define BSWSUB_GET_PLAY_BTL_BOSS (BSWSUB_START_NO+12)
#define BSWSUB_ADD_BATTLE_POINT (BSWSUB_START_NO+13)
#define BSWSUB_SET_PARTNER_NO (BSWSUB_START_NO+14)
#define BSWSUB_GET_PARTNER_NO (BSWSUB_START_NO+15)
#define BSWSUB_BTL_TRAINER_SET (BSWSUB_START_NO+16)
#define BSWSUB_GET_SELPOKE_IDX (BSWSUB_START_NO+17)
#define BSWSUB_DUMMY00 (BSWSUB_START_NO+18)
#define BSWSUB_RECV_BUF_CLEAR (BSWSUB_START_NO+19)
#define BSWSUB_TRAINER_BEFORE_MSG_UR (BSWSUB_START_NO+20)
#define BSWSUB_SAVE_GAMECLEAR (BSWSUB_START_NO+21)
#define BSWSUB_SET_PLAY_MODE_LOCATION (BSWSUB_START_NO+22)
#define BSWSUB_LOAD_POKEMON_MEMBER (BSWSUB_START_NO+23)
#define BSWSUB_SET_USE_BTL_BOX_FLAG (BSWSUB_START_NO+24)
#define BSWSUB_CHG_WORK_COMM_MULTI_MODE (BSWSUB_START_NO+25)
#define BSWSUB_CHK_S_MODE (BSWSUB_START_NO+26)
#define BSWSUB_DUMMY01 (BSWSUB_START_NO+27)
#define BSWSUB_DUMMY02 (BSWSUB_START_NO+28)
#define BSWSUB_SET_HOME_OBJ (BSWSUB_START_NO+29)
#define BSWSUB_SET_COMM_FLAG (BSWSUB_START_NO+30)
#define BSWSUB_GET_COMM_FLAG (BSWSUB_START_NO+31)
#define BSWSUB_GET_BATTLE_RESULT (BSWSUB_START_NO+32)
#define BSWSUB_SET_HOME_WORK (BSWSUB_START_NO+33)
#define BSWSUB_SET_TRAIN_BGM_ROUND_SW (BSWSUB_START_NO+34)
#define BSWSUB_CHOICE_BTL_SEVEN (BSWSUB_START_NO+35)
#define BSWSUB_GET_MEMBER_POKENO (BSWSUB_START_NO+36) 
#define BSWSUB_SET_COMM_IRC_FLAG (BSWSUB_START_NO+37)
#define BSWSUB_GET_COMM_IRC_FLAG (BSWSUB_START_NO+38)
#define BSWSUB_CHK_REGULATION (BSWSUB_START_NO+39)
#define BSWSUB_PREPAR_BTL_BOX (BSWSUB_START_NO+40)
#define BSWSUB_GET_REGULATION_TYPE (BSWSUB_START_NO+41)
#define BSWSUB_GET_MEMBER_NUM (BSWSUB_START_NO+42)
#define BSWSUB_CALL_BTLREC_EXIST (BSWSUB_START_NO+43) //録画処理統一で不要
#define BSWSUB_CHK_BTLREC_EXIST (BSWSUB_START_NO+44) //録画処理統一で不要
#define BSWSUB_STORE_BTLREC (BSWSUB_START_NO+45)
#define BSWSUB_SAVE_BTLREC (BSWSUB_START_NO+46) //録画処理統一で不要
#define BSWSUB_FREE_BTLPRM (BSWSUB_START_NO+47)
#define BSWSUB_CALL_BTLREC (BSWSUB_START_NO+48)
#define BSWSUB_RESET_STAGE_ROUND (BSWSUB_START_NO+49)
#define BSWSUB_STOP_SHAKE_TRAIN (BSWSUB_START_NO+50)
#define BSWSUB_GET_TRAINER_NUM_ROUND_NOW (BSWSUB_START_NO+51)
#define BSWSUB_TRAINER_BEFORE_MSG_DR (BSWSUB_START_NO+52)
#define BSWSUB_UPDATE_WIFISCORE_BTLRESULT (BSWSUB_START_NO+53)
#define BSWSUB_SET_BTLREC_FINISH_FLAG (BSWSUB_START_NO+54)
#define BSWSUB_GET_BTLREC_FINISH_FLAG (BSWSUB_START_NO+55)
#define BSWSUB_GET_USE_BTL_BOX_FLAG (BSWSUB_START_NO+56)
#define BSWSUB_INC_ROUND (BSWSUB_START_NO+57)
#define BSWSUB_GET_NOW_RENSHOU (BSWSUB_START_NO+58)
#define BSWSUB_GAME_START_SCORE (BSWSUB_START_NO+59)

#define BSWSUB_END_NO (BSWSUB_START_NO+60)

#define BSWSUB_COMM_START (BSWSUB_COMM_START_NO+0)
#define BSWSUB_COMM_END (BSWSUB_COMM_START_NO+1)
#define BSWSUB_COMM_TIMSYNC (BSWSUB_COMM_START_NO+2)
#define BSWSUB_COMM_ENTRY_PARENT_MENU (BSWSUB_COMM_START_NO+3)
#define BSWSUB_COMM_ENTRY_CHILD_MENU (BSWSUB_COMM_START_NO+4)
#define BSWSUB_COMM_SEND_BUF (BSWSUB_COMM_START_NO+5)
#define BSWSUB_COMM_RECV_BUF (BSWSUB_COMM_START_NO+6)
#define BSWSUB_COMM_GET_CURRENT_ID (BSWSUB_COMM_START_NO+7)
#define BSWSUB_COMM_IRC_ENTRY (BSWSUB_COMM_START_NO+8)
#define BSWSUB_COMM_IRC_ENTRY_RESULT (BSWSUB_COMM_START_NO+9)
#define BSWSUB_COMM_SET_UNION_ACQ (BSWSUB_COMM_START_NO+10)
#define BSWSUB_COMM_GET_OBJCODE_OYA (BSWSUB_COMM_START_NO+11)
#define BSWSUB_COMM_GET_OBJCODE_KO (BSWSUB_COMM_START_NO+12)
#define BSWSUB_COMM_REQ_ERROR_DISP (BSWSUB_COMM_START_NO+13)
#define BSWSUB_COMM_REQ_ERROR_DISP_FLD (BSWSUB_COMM_START_NO+14)
#define BSWSUB_COMM_CHK_NET_ERROR (BSWSUB_COMM_START_NO+15)
#define BSWSUB_COMM_END_NO (BSWSUB_COMM_START_NO+16)

#define BSWSUB_DEBUG_SET_SELECT_POKE (BSWSUB_DEBUG_START_NO+1)
#define BSWSUB_DEBUG_END_NO (BSWSUB_DEBUG_START_NO+2)

//バトルサブウェイコマンド　引数NULL
#define BSW_NULL (0)

//--------------------------------------------------------------
//  通信マルチ　同期コード
//--------------------------------------------------------------
#define BSW_COMM_MULTI_RECEIPT_POKE_SELECT     1
#define BSW_COMM_MULTI_RECEIPT_BTL_TRAINER_SET 2
#define BSW_COMM_MULTI_RECEIPT_AUTO_SAVE       3
#define BSW_COMM_MULTI_RECEIPT_SIO_END         4
#define BSW_COMM_MULTI_RECEIPT_SEND_MYST       5
#define BSW_COMM_MULTI_RECEIPT_SEND_TRDATA     6
#define BSW_COMM_MULTI_RECEIPT_SEND_TRDATA_END 7
#define BSW_COMM_MULTI_RECEIPT_REGU_ERROR_MSG_WAIT 8
#define BSW_COMM_MULTI_RECEIPT_SEND_PLAY_MODE 9
#define BSW_COMM_MULTI_RECEIPT_PLAYMODE_ERROR_MSG_WAIT 10

#define BSW_COMM_MULTI_ROUND_BATTLE_START 50
#define BSW_COMM_MULTI_ROUND_MENU_START   51
#define BSW_COMM_MULTI_ROUND_MENU_SELECT  52
#define BSW_COMM_MULTI_ROUND_EXIT_WAIT    53
#define BSW_COMM_MULTI_ROUND_RETIRE_WAIT  54
#define BSW_COMM_MULTI_ROUND_SIO_END      52     
#define BSW_COMM_MULTI_ROUND_BEFORE_MSG_WAIT 53
#define BSW_COMM_MULTI_ROUND_DEBUG_BATTLE_RESULT 54

#define BSW_COMM_MULTI_HOME_MENU_SELECT     100
#define BSW_COMM_MULTI_HOME_RETIRE_WAIT     101
#define BSW_COMM_MULTI_HOME_BTL_TRAINER_SET 102
#define BSW_COMM_MULTI_HOME_AUTO_SAVE       103
#define BSW_COMM_MULTI_HOME_SIO_END         104
#define BSW_COMM_MULTI_HOME_SELPOKE0        105
#define BSW_COMM_MULTI_HOME_SELPOKE1        106
#define BSW_COMM_MULTI_HOME_SELPOKE2        107
#define BSW_COMM_MULTI_HOME_SELPOKE3        108
#define BSW_COMM_MULTI_HOME_SELPOKE4        109
#define BSW_COMM_MULTI_HOME_SELPOKE5        110
#define BSW_COMM_MULTI_HOME_SELPOKE6        111
#define BSW_COMM_MULTI_HOME_SELPOKE7        112
#define BSW_COMM_MULTI_HOME_SELPOKE8        113

//--------------------------------------------------------------
//  通信マルチ　参加メニュー結果
//--------------------------------------------------------------
#define BSWAY_COMM_PERENT_ENTRY_OK 0 //親メニュー　メンバーが集まった
#define BSWAY_COMM_PERENT_ENTRY_CANCEL 1 //親メニュー　キャンセル
#define BSWAY_COMM_PERENT_ENTRY_ERROR 2 //親メニュー　エラー

#define BSWAY_COMM_CHILD_ENTRY_OK 0 //子メニュー　OK
#define BSWAY_COMM_CHILD_ENTRY_NG 1 //子メニュー　NG

#define BSWAY_COMM_ROUND_SEL_NEXT   0
#define BSWAY_COMM_ROUND_SEL_RETIRE 1

#define BSWAY_COMM_HOME_SEL_NEXT 0
#define BSWAY_COMM_HOME_SEL_END  1

#define BSWAY_COMM_IRC_RESULT_OK    0
#define BSWAY_COMM_IRC_RESULT_EXIT  1
#define BSWAY_COMM_IRC_RESULT_RETRY 2

//--------------------------------------------------------------
//  スクリプト　飛び先座標指定系
//--------------------------------------------------------------
//自機 シングル/ダブル 受付飛び先指定
#define SB_GATE_ZONE  ZONEID_D31R0201

//自機 エレベーター 飛び先
#define SB_EV_ZONE    ZONEID_D31R0202
#define SB_EV_PLAYER_SX      (3)
#define SB_EV_PLAYER_SY      (6)

//自機 シングル通路 飛び先
#define SB_WAYS_ZONE  ZONEID_D31R0203
#define SB_WAYS_PLAYER_SX    (2)
#define SB_WAYS_PLAYER_SY    (1)

//自機 シングルバトルルーム 初期飛び先
#define SB_ROOMS_ZONE  ZONEID_D31R0205
#define SB_ROOMS_PLAYER_SX    (10)
#define SB_ROOMS_PLAYER_SY    (4)

//自機 マルチ通路 飛び先
#define SB_WAYD_ZONE  ZONEID_D31R0204
#define SB_WAYD_PLAYER_SX    (8)
#define SB_WAYD_PLAYER_SY    (3)

//自機 マルチバトルルーム 飛び先
#define SB_ROOMD_ZONE  ZONEID_D31R0206
#define SB_ROOMD_PLAYER_SX    (7)
#define SB_ROOMD_PLAYER_SY    (5)

//自機 サロン 飛び先
#define SB_SALON_ZONE  ZONEID_D31R0207
#define SB_SALON_PLAYER_SX    (8)
#define SB_SALON_PLAYER_SY    (2)

//通信定義(プラチナで追加)
#define BSWAY_COMM_PLAYER_DATA            (0)
#define BSWAY_COMM_TR_DATA                (1)
#define BSWAY_COMM_RETIRE_SELECT          (2)
#define BSWAY_COMM_MYSTATUS_DATA          (3)
#define BSWAY_COMM_PLAY_MODE              (4)
#define BSWAY_COMM_HOME_SELECT_BOX_TEMOTI (5)
#define BSWAY_COMM_HOME_SELECT_POKEMON    (6)
#define BSWAY_COMM_BATTLE_RESULT          (7)

//--------------------------------------------------------------
//  列車種類 FLDEFF_BTRAIN_TYPEと同一である事
//--------------------------------------------------------------
#if 0
#define BTRAIN_TYPE_01 (0) //シングルトレイン
#define BTRAIN_TYPE_02 (1) //スーパーシングルトレイン
#define BTRAIN_TYPE_03 (2) //ダブルトレイン
#define BTRAIN_TYPE_04 (3) //スーパーダブルトレイン
#define BTRAIN_TYPE_05 (4) //マルチトレイン
#define BTRAIN_TYPE_06 (5) //スーパーマルチトレイン
#define BTRAIN_TYPE_07 (6) //WiFiトレイン
#endif

//--------------------------------------------------------------
//  列車アニメ FLDEFF_BTRAIN_ANIME_TYPEと同一である事
//--------------------------------------------------------------
#define BTRAIN_ANIME_TYPE_START_GEAR (0) //ギアステ出発アニメ　扉締り、発車
#define BTRAIN_ANIME_TYPE_ARRIVAL (1)  //途中駅電車進入アニメ
#define BTRAIN_ANIME_TYPE_ARRIVAL_HOME (2) //ホーム進入、電車開くアニメ
#define BTRAIN_ANIME_TYPE_START_HOME (3)  //ホーム出発アニメ 扉締り、発車

//--------------------------------------------------------------
//  列車座標種類
//--------------------------------------------------------------
#define BTRAIN_POS_RECEIPT (0) //受付ホーム
#define BTRAIN_POS_HOME (1) //途中駅、終点
#define BTRAIN_POS_MAX (2) //最大

//--------------------------------------------------------------
/// バトルサブウェイOBJ ID
//--------------------------------------------------------------
#define BSW_TRAIN_OBJID_JIKI (0xe0)
#define BSW_TRAIN_OBJID_PARTNER (0xe1)
#define BSW_TRAIN_OBJID_TRAINER_0 (0xe2)
#define BSW_TRAIN_OBJID_TRAINER_1 (0xe3)
#define BSW_HOME_OBJID_NPC_FIRST (0x80)

//--------------------------------------------------------------
/// バトルサブウェイ専用　戦闘結果
//--------------------------------------------------------------
#define BSW_BTL_RESULT_LOSE (0) //負け
#define BSW_BTL_RESULT_WIN  (1) //勝ち
#define BSW_BTL_RESULT_COMM_ERROR (2) //通信バトル用 通信エラー

//--------------------------------------------------------------
//  デバッグ用フラグ
//--------------------------------------------------------------
#define BSW_DEBUG_FLAG_REGU_OFF (1<<0) //レギュ無視
#define BSW_DEBUG_FLAG_AUTO (1<<1) //オート
#define BSW_DEBUG_FLAG_BTL_SKIP (1<<2) //バトルスキップ

#endif //__SUBWAY_SCR_DEF_H__
