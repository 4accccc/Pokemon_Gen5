//============================================================================================
/**
 * @file	wifihistory.h
 * @brief
 * @date	2006.03.25
 */
//============================================================================================

#ifndef	__WIFIHISTORY_H__
#define	__WIFIHISTORY_H__

#include "savedata/save_control.h"
#include "savedata/mystatus.h"

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief	WiFi通信履歴データの不完全型宣言
 */
//----------------------------------------------------------
typedef struct _WIFI_HISTORY WIFI_HISTORY;

///WiFi通信履歴を残す国の数
#define	WIFI_NATION_MAX		(256)

///WiFi通信履歴を残す国のそれぞれの地域コードの最大数
#define	WIFI_AREA_MAX		(64)


///日本の地球儀定義(本当はcountry103なんだけ参照できないので）
#define WIFI_NATION_JAPAN	( 103 ) 

///国名のある国の数（wifi_earth/wifi_place_msg_world.gmm）
#define	WIFI_COUNTRY_MAX		(234)

///地域名のある地域の数（wifi_earth/wifi_place_msg_USA.gmm）
#define	WIFI_AREADATA_MAX		(52)

//----------------------------------------------------------
/**
 * @brief	WiFi通信履歴データの状態定義
 */
//----------------------------------------------------------
typedef enum {
	WIFIHIST_STAT_NODATA = 0,	///<まだ通信したことがない
	WIFIHIST_STAT_NEW = 1,		///<本日初めて通信した
	WIFIHIST_STAT_EXIST = 2,	///<通信したことがある
	WIFIHIST_STAT_MINE = 3,		///<自分の場所

	WIFIHIST_STAT_MAX,
}WIFIHIST_STAT;

typedef enum {
  UN_INFO_RECV_POKE,       //もらったポケモン
  UN_INFO_SEND_POKE,      //あげたポケモン
  UN_INFO_FAVORITE,       //趣味
  UN_INFO_COUNTRY_NUM,    //交換した国の数
  UN_INFO_NATURE,         //性格
  UN_INFO_TALK,           //話したことあるか？
  UN_INFO_VALID,          //データは有効か？
}UN_INFO_TYPE;

//----------------------------------------------------------
//----------------------------------------------------------
//ワークサイズ取得（セーブシステムから呼ばれる）
extern int WIFIHISTORY_GetWorkSize(void);
//初期化（セーブシステムから呼ばれる）
extern void WIFIHISTORY_Init(WIFI_HISTORY * hist);
//セーブデータ取得（使う箇所で呼ぶ）
extern WIFI_HISTORY * SaveData_GetWifiHistory(SAVE_CONTROL_WORK * sv);

//自分の国と地域登録
extern void WIFIHISTORY_SetMyNationArea(WIFI_HISTORY * wh, MYSTATUS* my, int nation, int area);

extern void WIFIHISTORY_SetStat(WIFI_HISTORY * wh, int nation, int area, WIFIHIST_STAT stat);
extern WIFIHIST_STAT WIFIHISTORY_GetStat(const WIFI_HISTORY * wh, int nation, int area);

extern BOOL WIFIHISTORY_GetWorldFlag(const WIFI_HISTORY * wh);
extern void WIFIHISTORY_SetWorldFlag(WIFI_HISTORY * wh, BOOL flag);

//日付による更新処理
extern void WIFIHISTORY_Update(WIFI_HISTORY * wh);

extern u8 WIFIHISTORY_GetMyCountryCount(WIFI_HISTORY * wh);

extern int WIFIHISTORY_GetUnInfo(WIFI_HISTORY * wh, const int inIdx, const UN_INFO_TYPE inType);

#endif	/* __WIFIHISTORY_H__ */

