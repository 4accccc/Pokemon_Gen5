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
#define BSUBWAY_SCRWORK_MAGIC  (0x12345678)
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
#define BSWAY_MODE_NULL    (0xFFFF)
#define BSWAY_MODE_SINGLE  (0)
#define BSWAY_MODE_DOUBLE  (1)
#define BSWAY_MODE_MULTI  (2)
#define BSWAY_MODE_COMM_MULTI  (3)
#define BSWAY_MODE_WIFI (4)
#define BSWAY_MODE_WIFI_MULTI (5)

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

#define BSWTOOL_WIFI_RANK_DOWN (30)
#define BSWTOOL_GET_WIFI_RANK (31)
#define BSWTOOL_SET_WIFI_UPLOAD_FLAG (32)
#define BSWTOOL_GET_WIFI_UPLOAD_FLAG (33)
#define BSWTOOL_WIFI_CONNECT (34)

//--------------------------------------------------------------
/// バトルサブウェイコマンドツールID
/// ワーク依存コマンド
//--------------------------------------------------------------
#define BSWSUB_START (100)
#define BSWSUB_SELECT_POKE (BSWSUB_START+0)
#define BSWSUB_GET_ENTRY_POKE (BSWSUB_START+1)
#define BSWSUB_CHK_ENTRY_POKE (BSWSUB_START+2)
#define BSWSUB_IS_CLEAR (BSWSUB_START+3)
#define BSWSUB_SET_LOSE_SCORE (BSWSUB_START+4)
#define BSWSUB_SET_CLEAR_SCORE (BSWSUB_START+5)
#define BSWSUB_SAVE_REST_PLAY_DATA (BSWSUB_START+6)
#define BSWSUB_CHOICE_BTL_PARTNER (BSWSUB_START+7)
#define BSWSUB_GET_ENEMY_OBJ (BSWSUB_START+8)
#define BSWSUB_LOCAL_BTL_CALL (BSWSUB_START+9)
#define BSWSUB_GET_PLAY_MODE (BSWSUB_START+10)
#define BSWSUB_SET_LEADER_CLEAR_FLAG (BSWSUB_START+11)
#define BSWSUB_GET_LEADER_CLEAR_FLAG (BSWSUB_START+12)
#define BSWSUB_ADD_BATTLE_POINT (BSWSUB_START+13)
#define BSWSUB_SET_PARTNER_NO (BSWSUB_START+14)
#define BSWSUB_GET_PARTNER_NO (BSWSUB_START+15)
#define BSWSUB_BTL_TRAINER_SET (BSWSUB_START+16)
#define BSWSUB_GET_SELPOKE_IDX (BSWSUB_START+17)
#define BSWSUB_WIFI_RANK_UP (BSWSUB_START+18)
#define BSWSUB_RECV_BUF_CLEAR (BSWSUB_START+19)
#define BSWSUB_TRAINER_BEFORE_MSG (BSWSUB_START+20)
#define BSWSUB_SAVE_GAMECLEAR (BSWSUB_START+21)
#define BSWSUB_SET_PLAY_MODE_LOCATION (BSWSUB_START+22)
#define BSWSUB_LOAD_POKEMON_MEMBER (BSWSUB_START+23)
#define BSWSUB_END (BSWSUB_START+24)

//バトルサブウェイコマンド　引数NULL
#define BSW_NULL (0)

//--------------------------------------------------------------
//  通信マルチ　同期コード
//--------------------------------------------------------------
#define BSUBWAY_COMM_MULTI_POKE_SELECT    1
#define BSUBWAY_COMM_MULTI_BTL_TRAINER_SET  2
#define BSUBWAY_COMM_MULTI_AUTO_SAVE      3
#define BSUBWAY_COMM_MULTI_BATTLE_START    4
#define BSUBWAY_COMM_MULTI_NEXT_SELECT    5
#define BSUBWAY_COMM_MULTI_EXIT_WAIT      6
#define BSUBWAY_COMM_MULTI_RETIRE_WAIT    7
#define BSUBWAY_COMM_MULTI_SIO_END      8

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
#define BSUBWAY_COMM_PLAYER_DATA    (0)
#define BSUBWAY_COMM_TR_DATA      (1)
#define BSUBWAY_COMM_RETIRE_SELECT  (2)
//D31R0205_HERO kari
#define HERO_OBJ (0)

//--------------------------------------------------
//D31R0205_PCWOMAN2
#define PCWOMAN_OBJ (1)
//kari
#define TRAINER_OBJ (2)
//kari
#define OBJCODE_TRAINBOSS (TBOSS)

#endif //__SUBWAY_SCR_DEF_H__
