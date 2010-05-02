//=============================================================================
/**
 * @file	  wifi_login.h
 * @bfief	  WIFI ログイン関連のヘッダー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/11/17
 */
//=============================================================================


#pragma once

#include <gflib.h>
#include "net/network_define.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

//-------------------------------------
///	WIFILOGIN終了
//=====================================
typedef enum
{
  WIFILOGIN_RESULT_LOGIN, //ログインした
  WIFILOGIN_RESULT_CANCEL,//キャンセルした
} WIFILOGIN_RESULT;

//-------------------------------------
///	WIFILOGIN表示方法 文字やはいいいえが表示されるディスプレイ
//=====================================
typedef enum
{
  WIFILOGIN_DISPLAY_DOWN, //下画面にテキストと選択肢を表示
  WIFILOGIN_DISPLAY_UP,   //上画面にテキストと選択肢を表示
} WIFILOGIN_DISPLAY;

//-------------------------------------
///	WIFILOGIN表示方法 
//=====================================
typedef enum
{
  WIFILOGIN_BG_NORMAL,      //通常用
  WIFILOGIN_BG_DREAM_WORLD, //ドリームワールド用
} WIFILOGIN_BG;

//-------------------------------------
///	WIFILOGIN_BGM設定
//=====================================
typedef enum
{
  WIFILOGIN_BGM_NORMAL,     //通常用（内部でSEQ_BGM_WIFI_ACCESSを呼びます）
  WIFILOGIN_BGM_NONE,       //BGM操作を行わない
} WIFILOGIN_BGM;

//-------------------------------------
///	WIFILOGIN起動モード
//=====================================
typedef enum
{
  WIFILOGIN_MODE_NORMAL,      //通常起動
  WIFILOGIN_MODE_ERROR,       //エラーで再びきたときの起動
  WIFILOGIN_MODE_NOTSAVE,     //初回WIFIでもセーブへ移行しない(不思議な贈り物でしか使わないはずです)
} WIFILOGIN_MODE;

//-------------------------------------
///	WIFILOGIN_FADE設定
//    以下の値をマスクとしてOR指定してください
//    ->現在はWIFILOGOUTだけです
//=====================================
typedef enum
{
  WIFILOGIN_FADE_DEFAULT,     //  ブラックインブラックアウト
  WIFILOGIN_FADE_BLACK_IN   = 1<<1,
  WIFILOGIN_FADE_BLACK_OUT  = 1<<2,
  WIFILOGIN_FADE_WHITE_IN   = 1<<3,
  WIFILOGIN_FADE_WHITE_OUT  = 1<<4,
  WIFILOGIN_FADE_BLACK      = WIFILOGIN_FADE_BLACK_IN|WIFILOGIN_FADE_BLACK_OUT,
  WIFILOGIN_FADE_WHITE      = WIFILOGIN_FADE_WHITE_IN|WIFILOGIN_FADE_WHITE_OUT,
} WIFILOGIN_FADE;


//-------------------------------------
///	前方宣言
//=====================================
typedef struct _WIFILOGIN_MESSAGE_WORK  WIFILOGIN_MESSAGE_WORK;

//-------------------------------------
///	WIFIログイン中に処理を追加する場合
//=====================================
//  WIFILOGINへメッセージをだすためにmessage_workを引数にしている
//  message_work使用関数はsrc\net_app\wifi_login\wifilogin_local.hをインクルードのする
typedef enum
{ 
  WIFILOGIN_CALLBACK_RESULT_SUCCESS,  //処理成功 -> resultにWIFILOGIN_RESULT_LOGINが入る
  WIFILOGIN_CALLBACK_RESULT_FAILED,   //処理失敗 -> resultにWIFILOGIN_RESULT_CANCELが入る
  WIFILOGIN_CALLBACK_RESULT_CONTINUE, //処理続行中
}WIFILOGIN_CALLBACK_RESULT;
typedef WIFILOGIN_CALLBACK_RESULT (*WIFILOGIN_CALLBACK_FUNCTION)( WIFILOGIN_MESSAGE_WORK * p_msg, void *p_callback_wk );

//-------------------------------------
///	WIFILOGINに渡す引数
//=====================================
typedef struct 
{
  GAMEDATA      *gamedata;    //[in ]ゲームデータ
  WIFILOGIN_BG  bg;           //[in ]使用する背景
  WIFILOGIN_DISPLAY display;  //[in ]どちらの画面を使うか
  NetworkServiceID_e nsid;    //[in ]ネットワークサービスID
  DWCSvlResult* pSvl;         //[in ]任天堂SVL認証構造体 DWCSvlResultは自前で確保してください
  WIFILOGIN_MODE    mode;     //[in ]起動モード
  WIFILOGIN_BGM     bgm;      //[in ]BGM設定
  WIFILOGIN_RESULT  result;   //[out]終了方法

  WIFILOGIN_CALLBACK_FUNCTION login_after_callback; //[in]コールバック(処理のタイミングはログイン後)
  void                        *p_callback_wk;     //[in]コールバック用ワーク
} WIFILOGIN_PARAM;


typedef struct _WIFILOGIN_WORK  WIFILOGIN_WORK;

extern const GFL_PROC_DATA WiFiLogin_ProcData;


FS_EXTERN_OVERLAY(wifi_login);

