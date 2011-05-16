//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		net_common_download.h
 *	@brief  SWANネット共通ダウンロードモジュール
 *	@author	Toru=Nagihashi
 *	@date		2011.05.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "net/network_define.h"
#include <nd/dwc_nd.h>
#include "gamesystem/game_data.h"

#define COMMON_DOWNLOAD_WIFI_ND_DIRECT_DESIABLE

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	通信タイプ
//=====================================
typedef enum
{
  COMMON_DOWNLOAD_NET_TYPE_WIFI_ND,   //Wi-Fi任天堂ダウンロードサーバー
  COMMON_DOWNLOAD_NET_TYPE_BEACON,    //ビーコン受信

  COMMON_DOWNLOAD_NET_TYPE_MAX
} COMMON_DOWNLOAD_NET_TYPE;

//-------------------------------------
///	ダウンロード結果
//=====================================
typedef enum
{
  COMMON_DOWNLOAD_RESULT_NONE,      //なにも受信していない
  COMMON_DOWNLOAD_RESULT_SUCCESS,   //受信成功
  COMMON_DOWNLOAD_RESULT_EMPTY,     //受信するものがない  WiFiでしか返ってこない
  COMMON_DOWNLOAD_RESULT_TIMEOUT,   //タイムアウト(２分)  WiFiでしか返ってこない
  COMMON_DOWNLOAD_RESULT_CANCEL,    //キャンセルした
  COMMON_DOWNLOAD_RESULT_FAILED,    //通信がつながらなかった  WiFiでしか返ってこない
} COMMON_DOWNLOAD_RESULT;

//-------------------------------------
///	エラータイプ
//=====================================
typedef enum
{
  COMMON_DOWNLOAD_ERROR_TYPE_NONE,        //エラーが起こっていません。
  COMMON_DOWNLOAD_ERROR_TYPE_RETURN,      //再度通信をすればつながります。
  COMMON_DOWNLOAD_ERROR_TYPE_DISCONNECT,  //アプリ終了orWi-Fiから切断してください。
} COMMON_DOWNLOAD_ERROR_TYPE;

//-------------------------------------
///	状態リクエスト
//=====================================
typedef enum
{
  //COMMON_DOWNLOAD_NET_TYPE_WIFI_ND　Wi-Fiダウンロードサーバー
  COMMON_DOWNLOAD_REQ_WIFI_ND_START_SEARCH_DOWNLOAD,  //ダウンロード開始サーチ版  引数COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM
  COMMON_DOWNLOAD_REQ_WIFI_ND_START_DIRECT_DOWNLOAD,  //ダウンロード開始直指定版  引数COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM
  COMMON_DOWNLOAD_REQ_WIFI_ND_CANCEL_DOWNLOAD, //ダウンロード停止  引数NULL

  //COMMON_DOWNLOAD_NET_TYPE_BEACON ビーコン受け取り
  COMMON_DOWNLOAD_REQ_BEACON_START_DOWNLOAD, //ダウンロード開始  引数COMMON_DOWNLOAD_REQ_BEACON_PARAM
  COMMON_DOWNLOAD_REQ_BEACON_CANCEL_DOWNLOAD, //ダウンロード停止  引数NULL

  COMMON_DOWNLOAD_REQ_MAX
} COMMON_DOWNLOAD_REQ;

//-------------------------------------
///	内部状態
//=====================================
typedef enum
{
  COMMON_DOWNLOAD_STATE_WAIT, //何もしていない

  //COMMON_DOWNLOAD_NET_TYPE_WIFI_ND　Wi-Fiダウンロードサーバー
  COMMON_DOWNLOAD_STATE_WIFI_ND_SEARCH_DOWNLOAD,//検索ダウンロード中
  COMMON_DOWNLOAD_STATE_WIFI_ND_DIRECT_DOWNLOAD,//直接ダウンロード中

  //COMMON_DOWNLOAD_NET_TYPE_BEACON ビーコン受け取り
  COMMON_DOWNLOAD_STATE_BEACON_DOWNLOAD,//ダウンロード中

  COMMON_DOWNLOAD_STATE_MAX
} COMMON_DOWNLOAD_STATE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ハンドル
//=====================================
typedef struct _COMMON_DOWNLOAD COMMON_DOWNLOAD;

//-------------------------------------
///	初期化パラメータ
//=====================================
typedef struct
{
  COMMON_DOWNLOAD_NET_TYPE net_type;  //通信タイプ
  GAMEDATA *p_gamedata;               //ゲームデータ
  u32 buffer_size;                    //ダウンロードデータバッファサイズ
  HEAPID heapID;                      //ヒープID
} COMMON_DOWNLOAD_INIT_PARAM;

//=============================================================================
/**
 *					COMMON_DOWNLOAD_RequestStateに渡す引数構造体
*/
//=============================================================================
//-------------------------------------
///	COMMON_DOWNLOAD_REQ_WIFI_ND_START_SEARCH_DOWNLOAD用引数
//=====================================
typedef struct
{
  u32 download_search_num;      //ダウンロードサーバー検索数
  const char *cp_attribute_01;  //検索属性値１　""で指定しない
  const char *cp_attribute_02;  //検索属性値２　""で指定しない
  const char *cp_attribute_03;  //検索属性値３　""で指定しない
  //複数検索に引っかかった際にどれをダウンロードするか決めるコールバック
  u32 (*GetDownloadIndexCallback)( const DWCNdFileInfo *cp_fileInfo_array, u32 array_len );
  u32 file_size;              //ファイルサイズ
} COMMON_DOWNLOAD_REQ_WIFI_ND_SEARCH_PARAM;

//-------------------------------------
///	COMMON_DOWNLOAD_REQ_WIFI_ND_START_DIRECT_DOWNLOAD用引数
//=====================================
typedef struct
{
  const char *cp_file_name;   //ヌル文字終端した最長32文字のASCII文字列で表現されたファイル名
  u32 file_size;              //ファイルサイズ
} COMMON_DOWNLOAD_REQ_WIFI_ND_DIRECT_PARAM;

//-------------------------------------
///	COMMON_DOWNLOAD_REQ_BEACON_START_DOWNLOAD用引数
//=====================================
typedef struct
{
  NetworkServiceID_e  net_dev_id; //通信サービスID
  u32 language;     //言語コード
  u32 version_bit;  //1<<GET_VERSION()のようにビット指定にする
  u32 file_size;     //データサイズ
} COMMON_DOWNLOAD_REQ_BEACON_PARAM;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//初期化・破棄・メイン
extern COMMON_DOWNLOAD * COMMON_DOWNLOAD_Init( const COMMON_DOWNLOAD_INIT_PARAM *cp_param );
extern void COMMON_DOWNLOAD_Exit( COMMON_DOWNLOAD *p_wk );
extern void COMMON_DOWNLOAD_MainTop( COMMON_DOWNLOAD *p_wk );
extern void COMMON_DOWNLOAD_MainTail( COMMON_DOWNLOAD *p_wk );

//受信
extern BOOL COMMON_DOWNLOAD_RequestState( COMMON_DOWNLOAD *p_wk, COMMON_DOWNLOAD_REQ request, const void *cp_param_adrs );
extern COMMON_DOWNLOAD_STATE COMMON_DOWNLOAD_GetState( const COMMON_DOWNLOAD *cp_wk );

//受信データ取得
extern COMMON_DOWNLOAD_RESULT COMMON_DOWNLOAD_GetDownloadResult( const COMMON_DOWNLOAD *cp_wk );
extern const void * COMMON_DOWNLOAD_GetDownloadData( const COMMON_DOWNLOAD *cp_wk );

//エラー
extern COMMON_DOWNLOAD_ERROR_TYPE COMMON_DOWNLOAD_HandleError( COMMON_DOWNLOAD *p_wk );
extern void COMMON_DOWNLOAD_ClearError( COMMON_DOWNLOAD *p_wk );



//=============================================================================
/**
 *					使用例  メインは以下のようにしてください
 *
 *					詳細は　prog/src/test/nagihashi/debug_common_net.c　を御覧ください
*/
//=============================================================================
#if 0
static GFL_PROC_RESULT TestProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  //エラー処理
  switch( COMMON_DOWNLOAD_HandleError(p_dwn) )
  {
  case COMMON_DOWNLOAD_ERROR_TYPE_NONE:        //エラーが起こっていません。
    /*  なにもしない  */
    break;
  case COMMON_DOWNLOAD_ERROR_TYPE_RETURN:      //再度通信をすればつながります。
    /*   エラー時のシーケンス遷移など　 */
    COMMON_DOWNLOAD_ClearError( p_dwn );
    return GFL_PROC_RES_CONTINUE;
  case COMMON_DOWNLOAD_ERROR_TYPE_DISCONNECT:  //アプリ終了
    return GFL_PROC_RES_FINISH;
  default:
    GF_ASSERT(0);
  }
  
  COMMON_DOWNLOAD_MainTop( p_dwn );
  /*
   *  アプリケーションの処理
   *    この中でリクエスト処理を行う
   */
  COMMON_DOWNLOAD_MainTail( p_dwn );

  return GFL_PROC_RES_CONTINUE;
}
#endif
