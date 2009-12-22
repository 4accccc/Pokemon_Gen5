//======================================================================
/**
 * @file	bsubway_scr_def.h
 * @bfief	バトルサブウェイ　スクリプト関連定数定義
 * @author kagaya
 * @date	07.05.28
 * @note プラチナより移植
 */
//======================================================================
#ifndef	__SUBWAY_SCR_DEF_H__
#define __SUBWAY_SCR_DEF_H__

//======================================================================
//  define
//======================================================================
#define BSUBWAY_SCRWORK_MAGIC	(0x12345678)
#define BSWAY_NULL_POKE	(0xFF)
#define BSWAY_NULL_TRAINER	(0xFFFF)
#define BSWAY_NULL_PARAM	(0)
#define BSWAY_NULL (0)

///バトルタワー初期化モード
#define BSWAY_PLAY_NEW		(0)
#define BSWAY_PLAY_CONTINUE	(1)

///バトルタワー　クリアラウンド数
#define BSWAY_CLEAR_WINCNT	(7)

//バトルタワー　連勝記録ラウンド数
#define BSWAY_20_RENSHOU_CNT			(20)
#define BSWAY_50_RENSHOU_CNT			(50)
#define BSWAY_100_RENSHOU_CNT		(100)

#define LEADER_SET_1ST	(20+1)	//21人目のとき現在の連勝数は20
#define LEADER_SET_2ND	(48+1)	//49人目のとき現在の連勝数は48

#define BSWAY_FIVE_NUM		(5)	///<5人衆の数
#define BSUBWAY_FIVE_FIRST	(300)	///<5人衆データのスタートindex

///バトルタワープレイモードスクリプト用定義
#define BSWAY_MODE_NULL		(0xFFFF)
#define BSWAY_MODE_SINGLE	(0)
#define BSWAY_MODE_DOUBLE	(1)
#define BSWAY_MODE_MULTI		(2)
#define BSWAY_MODE_COMM_MULTI	(3)
#define BSWAY_MODE_WIFI			(4)
#define BSWAY_MODE_RETRY			(5)
#define BSWAY_MODE_WIFI_MULTI	(6)			//07.08.06 プラチナ追加(COMM_MULTIのWIFI版 友達手帳)

///バトルタワー　パートナーコード
#define BSWAY_PTCODE_OBJ		(2)
#define BSWAY_PTCODE_MINE2	(1)
#define BSWAY_PTCODE_MINE	(0)

///バトルタワー WIFI接続モード定義
#define BSWAY_WIFI_DL_PLAY		(0)
#define BSWAY_WIFI_UP_PLAY		(1)
#define BSWAY_WIFI_DL_LEADER		(2)

///通信マルチ送受信バッファサイズ
#define BSWAY_SIO_BUF_LEN	(70)

//もらえるグッズ
#define BSWAY_GOODS_COPPER	(GOODS_UG_TROPH_14)
#define BSWAY_GOODS_SILVER	(GOODS_UG_TROPH_15)
#define BSWAY_GOODS_GOLD		(GOODS_UG_TROPH_16)

//もらえるバトルポイント
#define BTLPOINT_VAL_BSUBWAY_LEADER	(20)
#define BTLPOINT_VAL_BSUBWAY_WIFI1	(3)			//ダウンロード
#define BTLPOINT_VAL_BSUBWAY_WIFI2	(4)
#define BTLPOINT_VAL_BSUBWAY_WIFI3	(5)
#define BTLPOINT_VAL_BSUBWAY_WIFI4	(6)
#define BTLPOINT_VAL_BSUBWAY_WIFI5	(7)
#define BTLPOINT_VAL_BSUBWAY_WIFI6	(8)
#define BTLPOINT_VAL_BSUBWAY_WIFI7	(9)
#define BTLPOINT_VAL_BSUBWAY_WIFI8	(10)
#define BTLPOINT_VAL_BSUBWAY_WIFI9	(10)
#define BTLPOINT_VAL_BSUBWAY_WIFI10	(10)
#define BTLPOINT_VAL_BSUBWAY_STAGE1	(3)			//シングル、ダブル、AIマルチ
#define BTLPOINT_VAL_BSUBWAY_STAGE2	(3)
#define BTLPOINT_VAL_BSUBWAY_STAGE3	(4)
#define BTLPOINT_VAL_BSUBWAY_STAGE4	(4)
#define BTLPOINT_VAL_BSUBWAY_STAGE5	(5)
#define BTLPOINT_VAL_BSUBWAY_STAGE6	(5)
#define BTLPOINT_VAL_BSUBWAY_STAGE7	(7) //ここはプログラム的にDPから間違い
#define BTLPOINT_VAL_BSUBWAY_STAGE8	(7)
#define BTLPOINT_VAL_BSUBWAY_COMM1	(8)			//マルチ、WIFI
#define BTLPOINT_VAL_BSUBWAY_COMM2	(9)
#define BTLPOINT_VAL_BSUBWAY_COMM3	(11)
#define BTLPOINT_VAL_BSUBWAY_COMM4	(12)
#define BTLPOINT_VAL_BSUBWAY_COMM5	(14)
#define BTLPOINT_VAL_BSUBWAY_COMM6	(15)
#define BTLPOINT_VAL_BSUBWAY_COMM7	(18)
#define BTLPOINT_VAL_BSUBWAY_COMM8	(18)

///スコアフラグアクセスID
#define BSWAY_SF_SILVER	(0)
#define BSWAY_SF_GOLD	(1)
#define BSWAY_SF_S50		(2)
#define BSWAY_SF_S100	(3)
#define BSWAY_SF_D50		(4)
#define BSWAY_SF_M50		(5)
#define BSWAY_SF_CM50	(6)
#define BSWAY_SF_W50		(7)

///バトルタワーコマンドツールID
#define BSWAY_TOOL_GET_MEMBER_NUM		(0)
#define BSWAY_TOOL_CHK_ENTRY_POKE_NUM	(1)
#define BSWAY_TOOL_SYSTEM_RESET			(2)
#define BSWAY_TOOL_CLEAR_PLAY_DATA		(3)
#define BSWAY_TOOL_IS_SAVE_DATA_ENABLE	(4)
#define BSWAY_TOOL_PUSH_NOW_LOCATION		(5)
#define BSWAY_TOOL_POP_NOW_LOCATION		(6)
#define BSWAY_TOOL_GET_RENSHOU_RECORD	(8)
#define BSWAY_TOOL_WIFI_RANK_DOWN		(9)
#define BSWAY_TOOL_GET_WIFI_RANK			(10)
#define BSWAY_TOOL_SET_WIFI_UPLOAD_FLAG	(11)
#define BSWAY_TOOL_GET_WIFI_UPLOAD_FLAG	(12)
#define BSWAY_TOOL_SET_NG_SCORE			(14)
#define BSWAY_TOOL_IS_PLAYER_DATA_ENABLE	(15)
#define BSWAY_TOOL_WIFI_CONNECT			(16)

#define BSWAY_SUB_START					(30)
#define BSWAY_SUB_SELECT_POKE			(BSWAY_SUB_START+0)
#define BSWAY_SUB_GET_ENTRY_POKE			(BSWAY_SUB_START+1)
#define BSWAY_SUB_CHK_ENTRY_POKE			(BSWAY_SUB_START+2)
#define BSWAY_SUB_GET_NOW_ROUND			(BSWAY_SUB_START+3)
#define BSWAY_SUB_INC_ROUND				(BSWAY_SUB_START+4)
#define BSWAY_SUB_IS_CLEAR				(BSWAY_SUB_START+5)
#define BSWAY_SUB_GET_RENSHOU_CNT		(BSWAY_SUB_START+6)
#define BSWAY_SUB_SET_LOSE_SCORE			(BSWAY_SUB_START+7)
#define BSWAY_SUB_SET_CLEAR_SCORE		(BSWAY_SUB_START+8)
#define BSWAY_SUB_SAVE_REST_PLAY_DATA	(BSWAY_SUB_START+9)
#define BSWAY_SUB_CHOICE_BTL_PARTNER		(BSWAY_SUB_START+10)
#define BSWAY_SUB_GET_ENEMY_OBJ			(BSWAY_SUB_START+11)
#define BSWAY_SUB_LOCAL_BTL_CALL			(BSWAY_SUB_START+12)
#define BSWAY_SUB_GET_PLAY_MODE			(BSWAY_SUB_START+13)
#define BSWAY_SUB_SET_LEADER_CLEAR_FLAG	(BSWAY_SUB_START+14)
#define BSWAY_SUB_GET_LEADER_CLEAR_FLAG	(BSWAY_SUB_START+15)
#define BSWAY_SUB_ADD_BATTLE_POINT		(BSWAY_SUB_START+16)
#define BSWAY_SUB_GOODS_FLAG_SET			(BSWAY_SUB_START+17)
#define BSWAY_SUB_LEADER_RIBBON_SET		(BSWAY_SUB_START+18)
#define BSWAY_SUB_RENSHOU_RIBBON_SET		(BSWAY_SUB_START+19)
#define BSWAY_SUB_SET_PARTNER_NO			(BSWAY_SUB_START+20)
#define BSWAY_SUB_GET_PARTNER_NO			(BSWAY_SUB_START+21)
#define BSWAY_SUB_BTL_TRAINER_SET		(BSWAY_SUB_START+22)
#define BSWAY_SUB_GET_SELPOKE_IDX		(BSWAY_SUB_START+23)
#define BSWAY_SUB_WIFI_RANK_UP			(BSWAY_SUB_START+24)
#define BSWAY_TOOL_GET_MINE_OBJ			(BSWAY_SUB_START+25)
#define BSWAY_SUB_CHOICE_BTL_SEVEN		(BSWAY_SUB_START+26)
#define BSWAY_SUB_UPDATE_RANDOM			(BSWAY_SUB_START+27)
#define BSWAY_SUB_RECV_BUF_CLEAR			(BSWAY_SUB_START+28)
#define BSWAY_SUB_SET_FIRST_BTL_FLAG		(BSWAY_SUB_START+29)
#define BSWAY_SUB_SET_SCRWORK_FIELDMAP (BSWAY_SUB_START+30)
#define BSWAY_SUB_GET_FIELDMAP_SCRWORK (BSWAY_SUB_START+31)
#define BSWAY_TOOL_PLAYER_VANISH (BSWAY_SUB_START+32)
#define BSWAY_SUB_TRAINER_BEFORE_MSG (BSWAY_SUB_START+33)
#define BSWAY_TOOL_CHK_REGULATION (BSWAY_SUB_START+34)
#define BSWAY_TOOL_GET_RENSHOU_CNT		(BSWAY_SUB_START+35)
#define BSWAY_SUB_GET_NEXT_CARNUM (BSWAY_SUB_START+36)
#define BSWAY_TOOL_GET_PLAY_MODE (BSWAY_SUB_START+37)
#define BSWAY_SUB_SAVE_GAMECLEAR (BSWAY_SUB_START+38)
#define BSWAY_SUB_SET_PLAY_MODE_LOCATION (BSWAY_SUB_START+39)
#define BSWAY_SUB_LOAD_POKEMON_MEMBER (BSWAY_SUB_START+40)
#define BSWAY_SUB_COUNT_NEXT_STAGE (BSWAY_SUB_START+41)

#define BSWAY_DEB_IS_WORK_NULL			(100)

//--------------------------------------------------------------
//  通信マルチ　同期コード
//--------------------------------------------------------------
#define BSUBWAY_COMM_MULTI_POKE_SELECT		1
#define BSUBWAY_COMM_MULTI_BTL_TRAINER_SET	2
#define BSUBWAY_COMM_MULTI_AUTO_SAVE			3
#define BSUBWAY_COMM_MULTI_BATTLE_START		4
#define BSUBWAY_COMM_MULTI_NEXT_SELECT		5
#define BSUBWAY_COMM_MULTI_EXIT_WAIT			6
#define BSUBWAY_COMM_MULTI_RETIRE_WAIT		7
#define BSUBWAY_COMM_MULTI_SIO_END			8

//--------------------------------------------------------------
//  スクリプト　飛び先座標指定系
//--------------------------------------------------------------
//自機 シングル/ダブル 受付飛び先指定
#define SB_GATE_ZONE	ZONEID_D31R0201

//自機 エレベーター 飛び先
#define SB_EV_ZONE		ZONEID_D31R0202
#define SB_EV_PLAYER_SX			(3)
#define SB_EV_PLAYER_SY			(6)

//自機 シングル通路 飛び先
#define SB_WAYS_ZONE	ZONEID_D31R0203
#define SB_WAYS_PLAYER_SX		(2)
#define SB_WAYS_PLAYER_SY		(1)

//自機 シングルバトルルーム 初期飛び先
#define SB_ROOMS_ZONE	ZONEID_D31R0205
#define SB_ROOMS_PLAYER_SX		(10)
#define SB_ROOMS_PLAYER_SY		(4)

//自機 マルチ通路 飛び先
#define SB_WAYD_ZONE	ZONEID_D31R0204
#define SB_WAYD_PLAYER_SX		(8)
#define SB_WAYD_PLAYER_SY		(3)

//自機 マルチバトルルーム 飛び先
#define SB_ROOMD_ZONE	ZONEID_D31R0206
#define SB_ROOMD_PLAYER_SX		(7)
#define SB_ROOMD_PLAYER_SY		(5)

//自機 サロン 飛び先
#define SB_SALON_ZONE	ZONEID_D31R0207
#define SB_SALON_PLAYER_SX		(8)
#define SB_SALON_PLAYER_SY		(2)

//通信定義(プラチナで追加)
#define BSUBWAY_COMM_PLAYER_DATA		(0)
#define BSUBWAY_COMM_TR_DATA			(1)
#define BSUBWAY_COMM_RETIRE_SELECT	(2)
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
