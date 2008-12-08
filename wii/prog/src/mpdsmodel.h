#ifndef MPDSMODEL_H__
#define MPDSMODEL_H__

#define WM_SIZE_USER_GAMEINFO (112)
#define _BEACON_SIZE_FIX (12)
#define _BEACON_FIXHEAD_SIZE (6)
#define _BEACON_USER_SIZE_MAX (WM_SIZE_USER_GAMEINFO-_BEACON_SIZE_FIX)

//アライメントを1バイト単位に変更
//後でpopするのを忘れないこと
#pragma pack(push,1)
typedef struct
{
	u8	  FixHead[_BEACON_FIXHEAD_SIZE];		 ///< 固定で決めた６バイト部分
	//GameServiceID
	u8		serviceNo; ///< 通信サービス番号
	u8	  debugAloneTest;	  ///< デバッグ用 同じゲームでもビーコンを拾わないように
	u8  	connectNum;		   ///< つながっている台数  --> 本親かどうか識別
	u8	  pause;			   ///< 接続を禁止したい時に使用する
	u8		dummy1;
	u8		dummy2;
	u8	  aBeaconDataBuff[_BEACON_USER_SIZE_MAX];
	
}DS_COMM_USERDATAINFO;
#pragma pack(pop)

#endif	//MPDSMODEL_H__