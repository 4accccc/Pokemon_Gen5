
//=============================================================================
/**
 * @file	network_define.h
 * @brief	WBのネットワーク定義を書くヘッダーファイル
 * @author	Katsumi Ohno
 * @date    2008.07.29
 */
//=============================================================================

#ifndef __NETWORK_DEFINE_H__
#define __NETWORK_DEFINE_H__

#define NET_WORK_ON (1)   // サンプルプログラム上で通信を動かす場合定義　動かさない場合未定義にする


//==============================================================================
//	定義
//==============================================================================

extern void FatalError_Disp(GFL_NETHANDLE* pNet,int errNo,void* pWork);
extern u32 GFLR_NET_GetGGID(void);

//==============================================================================
//	型宣言
//==============================================================================

/// ゲームの種類を区別する為の定義   初期化構造体のgsidとして渡してください
enum NetworkServiceID_e {
  WB_NET_NOP_SERVICEID = 0,
  WB_NET_BATTLE_SERVICEID,       ///< バトル用ID
  WB_NET_FIELDMOVE_SERVICEID,  ///< サンプルのフィールド移動
  WB_NET_BOX_DOWNLOAD_SERVICEID,  ///< ダウンロード通信のボックス取得
  WB_NET_DEBUG_OHNO_SERVICEID,
  WB_NET_SERVICEID_DEBUG_TAYA,
  WB_NET_DEBUG_MATSUDA_SERVICEID,
  WB_NET_TRADE_SERVICEID,			///<ポケモン交換
  WB_NET_SERVICEID_MAX   // 終端
};

///通信コマンドを区別する為の定義  コールバックテーブルを渡すときにIDとして定義してください
/// 上の定義と同じ並びで書いてください
enum NetworkCommandHeaderNo_e {
  GFL_NET_CMD_BASE = (WB_NET_NOP_SERVICEID<<8),  ///< ベースコマンド開始番号
  GFL_NET_CMD_BATTLE = (WB_NET_BATTLE_SERVICEID<<8),   ///< バトル開始番号
  GFL_NET_CMD_FIELD = (WB_NET_FIELDMOVE_SERVICEID<<8),   ///< フィールドのベース開始番号
  GFL_NET_CMD_BOX_DOWNLOAD = (WB_NET_BOX_DOWNLOAD_SERVICEID<<8),   ///< フィールドのベース開始番号
  GFL_NET_CMD_DEBUG_OHNO = (WB_NET_DEBUG_OHNO_SERVICEID<<8),
  GFL_NET_CMD_DEBUG_TAYA = (WB_NET_SERVICEID_DEBUG_TAYA<<8),
  GFL_NET_CMD_DEBUG_MATSUDA = (WB_NET_DEBUG_MATSUDA_SERVICEID<<8),
  GFL_NET_CMD_TRADE = (WB_NET_TRADE_SERVICEID<<8),		///<ポケモン交換の開始番号
};



#endif //__NETWORK_DEFINE_H__

