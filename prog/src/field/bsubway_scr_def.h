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
#define BSUBWAY_SCRWORK_MAGIC  (0x12345678) ///<マジックナンバー

#define BSWAY_NULL_POKE  (0xFF)
#define BSWAY_NULL_TRAINER  (0xFFFF)
#define BSWAY_NULL_PARAM  (0)

///バトルサブウェイ初期化モード
#define BSWAY_PLAY_NEW    (0)
#define BSWAY_PLAY_CONTINUE  (1)

///バトルサブウェイ　クリアラウンド数
#define BSWAY_CLEAR_WINCNT  (7)

//バトルサブウェイ　連勝記録ラウンド数
#define BSWAY_20_RENSHOU_CNT      (20)
#define BSWAY_50_RENSHOU_CNT      (50)
#define BSWAY_100_RENSHOU_CNT    (100)

#define LEADER_SET_1ST  (20+1)  //21人目のとき現在の連勝数は20
#define LEADER_SET_2ND  (48+1)  //49人目のとき現在の連勝数は48

#define BSWAY_FIVE_NUM    (5)  ///<5人衆の数
#define BSUBWAY_FIVE_FIRST  (300)  ///<5人衆データのスタートindex

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
/// バトルサブウェイシーン制御
//--------------------------------------------------------------
//受付制御
#define BSWAY_SCENE_RECEIPT_NON (0) //何も無し
#define BSWAY_SCENE_RECEIPT_CONTINUE (1) //コンテニュー
#define BSWAY_SCENE_RECEIPT_CANCEL (2) //キャンセル
#define BSWAY_SCENE_RECEIPT_AFTER (3) //ゲームクリア
#define BSWAY_SCENE_RECEIPT_ERROR (4) //エラー

//列車内制御
#define BSWAY_SCENE_TRAIN_NON (0) //何も無し
#define BSWAY_SCENE_TRAIN_FIRST (1) //乗車初回
#define BSWAY_SCENE_TRAIN_CONTINUE (2) //乗車中

//ホーム制御
#define BSWAY_SCENE_HOME_NON (0) //何も無し
#define BSWAY_SCENE_HOME_GAME_CLEAR (1) //ゲームクリア
#define BSWAY_SCENE_HOME_TERMINUS (2) //ホーム終点

//--------------------------------------------------------------
/// バトルサブウェイコマンドツールID
/// ワーク依存無しコマンド
//--------------------------------------------------------------
#define BSWTOOL_GET_ZONE_PLAY_MODE (0)
#define BSWTOOL_SYSTEM_RESET (1)
#define BSWTOOL_CHK_ENTRY_POKE_NUM (2)
#define BSWTOOL_CLEAR_PLAY_DATA (3)
#define BSWTOOL_IS_SAVE_DATA_ENABLE (4)
#define BSWTOOL_PUSH_NOW_LOCATION (5)
#define BSWTOOL_POP_NOW_LOCATION (6)
#define BSWTOOL_GET_RENSHOU_RECORD (7)
#define BSWTOOL_SET_NG_SCORE (8)
#define BSWTOOL_GET_NOW_ROUND (9)
#define BSWTOOL_INC_ROUND (10)
#define BSWTOOL_GET_RENSHOU_CNT (11)
#define BSWTOOL_GET_NEXT_ROUND (12)
#define BSWTOOL_GET_MINE_OBJ (13)
#define BSWTOOL_PLAYER_VANISH (14)
#define BSWTOOL_CHK_REGULATION (15)
#define BSWTOOL_GET_PLAY_MODE (16)
#define BSWTOOL_GET_BOSS_CLEAR_FLAG (17)
#define BSWTOOL_GET_S_BOSS_CLEAR_FLAG (18)
#define BSWTOOL_SET_BOSS_CLEAR_FLAG (19)
#define BSWTOOL_SET_S_BOSS_CLEAR_FLAG (20)
#define BSWTOOL_GET_SUPPORT_ENCOUNT_END (21)
#define BSWTOOL_SET_SUPPORT_ENCOUNT_END (22)
#define BSWTOOL_SET_TRAIN (23)
#define BSWTOOL_SET_TRAIN_ANM (24)
#define BSWTOOL_SET_TRAIN_VANISH (25)
#define BSWTOOL_GET_DATA_PLAY_MODE (26)
#define BSWTOOL_END_NO (27)

#define BSWTOOL_WIFI_START_NO (40)
#define BSWTOOL_SET_WIFI_UPLOAD_FLAG (BSWTOOL_WIFI_START_NO+0)
#define BSWTOOL_GET_WIFI_UPLOAD_FLAG (BSWTOOL_WIFI_START_NO+1)
#define BSWTOOL_WIFI_CONNECT (BSWTOOL_WIFI_START_NO+2)
#define BSWTOOL_WIFI_END_NO (BSWTOOL_WIFI_START_NO+3)

//--------------------------------------------------------------
/// バトルサブウェイコマンドツールID
/// ワーク依存コマンド
//--------------------------------------------------------------
#define BSWSUB_START_NO (100)
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
#define BSWSUB_SET_BOSS_CLEAR_FLAG (BSWSUB_START_NO+11)
#define BSWSUB_GET_BOSS_CLEAR_FLAG (BSWSUB_START_NO+12)
#define BSWSUB_ADD_BATTLE_POINT (BSWSUB_START_NO+13)
#define BSWSUB_SET_PARTNER_NO (BSWSUB_START_NO+14)
#define BSWSUB_GET_PARTNER_NO (BSWSUB_START_NO+15)
#define BSWSUB_BTL_TRAINER_SET (BSWSUB_START_NO+16)
#define BSWSUB_GET_SELPOKE_IDX (BSWSUB_START_NO+17)
#define BSWSUB_WIFI_RANK_UP (BSWSUB_START_NO+18)
#define BSWSUB_RECV_BUF_CLEAR (BSWSUB_START_NO+19)
#define BSWSUB_TRAINER_BEFORE_MSG (BSWSUB_START_NO+20)
#define BSWSUB_SAVE_GAMECLEAR (BSWSUB_START_NO+21)
#define BSWSUB_SET_PLAY_MODE_LOCATION (BSWSUB_START_NO+22)
#define BSWSUB_LOAD_POKEMON_MEMBER (BSWSUB_START_NO+23)
#define BSWSUB_SET_USE_BBOX (BSWSUB_START_NO+24)
#define BSWSUB_CHG_WORK_COMM_MULTI_MODE (BSWSUB_START_NO+25)
#define BSWSUB_CHK_S_MODE (BSWSUB_START_NO+26)
#define BSWSUB_WIFI_RANK_DOWN (BSWSUB_START_NO+27)
#define BSWSUB_GET_WIFI_RANK (BSWSUB_START_NO+28)
#define BSWSUB_END_NO (BSWSUB_START_NO+29)

#define BSWSUB_COMM_START_NO (200)
#define BSWSUB_COMM_START (BSWSUB_COMM_START_NO+0)
#define BSWSUB_COMM_END (BSWSUB_COMM_START_NO+1)
#define BSWSUB_COMM_TIMSYNC (BSWSUB_COMM_START_NO+2)
#define BSWSUB_COMM_ENTRY_PARENT_MENU (BSWSUB_COMM_START_NO+3)
#define BSWSUB_COMM_ENTRY_CHILD_MENU (BSWSUB_COMM_START_NO+4)
#define BSWSUB_COMM_SEND_BUF (BSWSUB_COMM_START_NO+5)
#define BSWSUB_COMM_RECV_BUF (BSWSUB_COMM_START_NO+6)
#define BSWSUB_COMM_GET_CURRENT_ID (BSWSUB_COMM_START_NO+7)
#define BSWSUB_COMM_END_NO (BSWSUB_COMM_START_NO+8)

//バトルサブウェイコマンド　引数NULL
#define BSW_NULL (0)

//--------------------------------------------------------------
//  通信マルチ　同期コード
//--------------------------------------------------------------
#define BSW_COMM_MULTI_POKE_SELECT    1
#define BSW_COMM_MULTI_BTL_TRAINER_SET  2
#define BSW_COMM_MULTI_AUTO_SAVE      3
#define BSW_COMM_MULTI_BATTLE_START    4
#define BSW_COMM_MULTI_NEXT_SELECT    5
#define BSW_COMM_MULTI_EXIT_WAIT      6
#define BSW_COMM_MULTI_RETIRE_WAIT    7
#define BSW_COMM_MULTI_SIO_END      8

//--------------------------------------------------------------
//  通信マルチ　参加メニュー結果
//--------------------------------------------------------------
#define BSWAY_COMM_PERENT_ENTRY_OK 0 //親メニュー　メンバーが集まった
#define BSWAY_COMM_PERENT_ENTRY_CANCEL 1 //親メニュー　キャンセル
#define BSWAY_COMM_PERENT_ENTRY_ERROR 2 //親メニュー　エラー

#define BSWAY_COMM_CHILD_ENTRY_OK 0 //子メニュー　OK
#define BSWAY_COMM_CHILD_ENTRY_NG 1 //子メニュー　NG

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
#define BSWAY_COMM_PLAYER_DATA    (0)
#define BSWAY_COMM_TR_DATA      (1)
#define BSWAY_COMM_RETIRE_SELECT  (2)

//D31R0205_HERO kari
#define HERO_OBJ (0)

//--------------------------------------------------
//D31R0205_PCWOMAN2
#define PCWOMAN_OBJ (1)
//kari
#define TRAINER_OBJ (2)
//kari
#define OBJCODE_TRAINBOSS (TBOSS)

//--------------------------------------------------------------
//  列車種類 FLDEFF_BTRAIN_TYPEと同一である事
//--------------------------------------------------------------
#define BTRAIN_TYPE_01 //シングルトレイン
#define BTRAIN_TYPE_02 //スーパーシングルトレイン
#define BTRAIN_TYPE_03 //ダブルトレイン
#define BTRAIN_TYPE_04 //スーパーダブルトレイン
#define BTRAIN_TYPE_05 //マルチトレイン
#define BTRAIN_TYPE_06 //スーパーマルチトレイン
#define BTRAIN_TYPE_07 //WiFiトレイン

//--------------------------------------------------------------
//  列車アニメ FLDEFF_BTRAIN_ANIME_TYPEと同一である事
//--------------------------------------------------------------
#define BTRAIN_ANIME_TYPE_START_GEAR, //ギアステ出発アニメ　扉締り、発車
#define BTRAIN_ANIME_TYPE_ARRIVAL, //途中駅電車進入アニメ
#define BTRAIN_ANIME_TYPE_ARRIVAL_HOME, //ホーム進入、電車開くアニメ
#define BTRAIN_ANIME_TYPE_START_HOME,  //ホーム出発アニメ 扉締り、発車

//--------------------------------------------------------------
//  列車座標種類
//--------------------------------------------------------------
#define BTRAIN_POS_RECEIPT (0) //受付ホーム
#define BTRAIN_POS_HOME (1) //途中駅、終点
#define BTRAIN_POS_MAX (2) //最大

#endif //__SUBWAY_SCR_DEF_H__
