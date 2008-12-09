//======================================================================
/**
 * @file	field_comm_def.h
 * @brief	フィールド通信用定義群
 * @author	ariizumi
 * @data	08/11/27
 */
//======================================================================

#ifndef FIELD_COMM_DEF_H__
#define FIELD_COMM_DEF_H__

#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
#define FIELD_COMM_MEMBER_MAX (4)
#define FCM_BGPLANE_YESNO_WINDOW (GFL_BG_FRAME1_M)	//使用フレーム(DEBUG_BGFRAME_MENU 
#define FCM_BGPLANE_MSG_WINDOW (GFL_BG_FRAME2_M)	//使用フレーム(DEBUG_BGFRAME_MENU 

//自キャラのデータ用インデックス
#define FCD_SELF_INDEX (0xFF)
//======================================================================
//	enum
//======================================================================
//キャラの状態
typedef enum
{
	FCCS_NONE,		//存在なし
	FCCS_CONNECT,	//接続直後
	FCCS_REQ_DATA,	//キャラ個別データ受信中
	FCCS_EXIST_DATA,	//キャラ個別データ受信後
	FCCS_FIELD,		//フィールド中(会話も含む
}F_COMM_CHARA_STATE;

//キャラの会話管理用
typedef enum
{
	FCTS_NONE,
	FCTS_REPLY_TALK,	//会話の返事
	FCTS_WAIT_TALK,		//会話相手の停止待ち
	FCTS_UNPOSSIBLE,	//↑の返事(無理
	FCTS_ACCEPT,		//↑の返事(OK
	FCTS_RESERVE_TALK,	//会話予約
	FCTS_TALKING,		//会話中
	FCTS_WAIT_END,		//会話終了待ち
}F_COMM_TALK_STATE;

//フィールド会話でできる行動リスト
typedef enum
{
	FCAL_TRAINERCARD,	//トレーナーカード
	FCAL_BATTLE,		//バトル
	FCAL_END,			//終了
	FCAL_MAX,

	FCAL_SELECT,		//選択行動ではないが、選択中用
}F_COMM_ACTION_LIST;

//汎用フラグの送信
typedef enum
{
	FCCF_TALK_REQUEST,		//会話要求
	FCCF_TALK_UNPOSSIBLE,	//要求に対して無理
	FCCF_TALK_ACCEPT,		//要求に対して許可

	FCCF_ACTION_SELECT,		//親が選んだ行動
	FCCF_ACTION_RETURN,		//行動に対する返事

	FCCF_SYNC_TYPE,			//同期処理用関数

	FCCF_TEST,				//実験用
	
	FCCF_FLG_MAX,			//不正値として使う
}F_COMM_COMMON_FLG;

//同期コマンド種類
typedef enum
{
	FCST_START_INIT_ACTION, //行動前の準備同期
	FCST_START_CHANGE_PROC,	//パート遷移のタイミング
	FCST_WAIT_RETURN_PROC,	//パート遷移後の待ち

	FCST_MAX,
}F_COMM_SYNC_TYPE;

//FIELD_COMM_DATAで確保できるユーザーデータの種類
typedef enum
{
	FCUT_TRAINERCARD,	//トレーナーカード
	FCUT_BATTLE,		//バトル

	FCUT_MAX,
	FCUT_NO_INIT,	//初期化前

}F_COMM_USERDATA_TYPE;

//======================================================================
//	typedef struct
//======================================================================
typedef struct _FIELD_COMM_MAIN FIELD_COMM_MAIN;
typedef struct _FIELD_COMM_EVENT FIELD_COMM_EVENT;
typedef struct _FIELD_COMM_MENU FIELD_COMM_MENU;
typedef struct _FIELD_COMM_FUNC FIELD_COMM_FUNC;
typedef struct _FIELD_COMM_DEBUG_WORK FIELD_COMM_DEBUG_WORK;

//通信時のユーザーデータ
typedef struct
{
	char	name_[128];
	char	id_[128];
	char	pad_[768];
}FIELD_COMM_USERDATA_TRAINERCARD;

//======================================================================
//	proto
//======================================================================

#endif //FIELD_COMM_DEF_H__

