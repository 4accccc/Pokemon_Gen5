






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



/// 汎用通信コマンドの定義
enum NetworkServiceID_e {
  WB_NET_FIELDMOVE_SERVICEID = 1,  ///< サンプルのフィールド移動
  WB_NET_DEBUG_OHNO_SERVICEID,
  WB_NET_SERVICEID_DEBUG_TAYA,
  WB_NET_DEBUG_MATSUDA_SERVICEID,
  WB_NET_SERVICEID_MAX   // 終端
};


#endif //__NETWORK_DEFINE_H__

