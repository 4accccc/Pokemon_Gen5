//=============================================================================
/**
 * @file	net_command.h
 * @brief	データ共有を行う場合の通信システム
 *          使用する場合に書き込む必要があるものをまとめたヘッダー
 * @author	Katsumi Ohno
 * @date    2005.07.26
 */
//=============================================================================

#ifndef __NET_COMMAND_H__
#define __NET_COMMAND_H__


//==============================================================================
//	定義
//==============================================================================

//==============================================================================
//	型宣言
//==============================================================================

//#define   _SET(callfunc, getSize, name)       {callfunc,  getSize},


/// 汎用通信コマンドの定義
enum CommCommand_e {
  GFL_NET_CMD_NONE = 0,                ///< なにもしない
  GFL_NET_CMD_FREE = 0,                   ///< 空コマンド
  GFL_NET_CMD_COMMAND_MIN = 1,             ///< 最小値
  GFL_NET_CMD_EXIT_REQ = GFL_NET_CMD_COMMAND_MIN,            ///< 終了
  GFL_NET_CMD_EXIT,            ///< 自動終了
  GFL_NET_CMD_NEGOTIATION,  ///< 初期化時のネゴシエーション
  GFL_NET_CMD_NEGOTIATION_RETURN,  ///< ネゴシエーションの返事
  GFL_NET_CMD_DSMP_CHANGE,            ///< DSモード通信かMPモード通信かに切り替える許可を得る
  GFL_NET_CMD_DSMP_CHANGE_REQ,        ///< DSモード通信かMPモード通信かに切り替える指示を出す
  GFL_NET_CMD_DSMP_CHANGE_END,        ///< DSモード通信かMPモード通信かに切り替え完了したことを通知
  GFL_NET_CMD_TIMING_SYNC,            ///< 同期を取るコマンド
  GFL_NET_CMD_TIMING_SYNC_END,        ///< 同期が取れたことを返すコマンド
  //------------------------------------------------ここまで----------
  GFL_NET_CMD_COMMAND_MAX   // 終端--------------これは移動させないでください
};



/// コマンドテーブルの初期化をする
extern void GFL_NET_CommandInitialize(const NetRecvFuncTable* pCommPacketLocal,int listNum,void* pWork);
/// コマンドテーブルの開放処理
extern void GFL_NET_CommandFinalize( void );
/// データのサイズを得る
extern int GFL_NET_CommandGetPacketSize(int command);
///  コマンドテーブルを破棄する時の命令
extern BOOL GFL_NET_CommandThrowOut(void);
///  コマンドテーブルが破棄し終わったかどうか検査
extern BOOL GFL_NET_CommandIsThrowOuted(void);
///  コールバックを呼ぶ
extern void GFL_NET_CommandCallBack(int netID, int sendID, int command, int size, void* pData);
/// プリント
extern void GFL_NET_CommandDebugPrint(int command);

extern void* GFL_NET_CommandCreateBuffStart(int command, int netID, int size);
extern BOOL GFL_NET_CommandCreateBuffCheck(int command);


#endif// __NET_COMMAND_H__

