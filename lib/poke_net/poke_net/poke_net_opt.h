//============================================================================
//                         Pokemon Network Library
//============================================================================
#ifndef ___POKE_NET_OPT___
#define ___POKE_NET_OPT___

#include "poke_net_dscomp.h"

//==============================================
//                 置き換え文字
//==============================================

//==============================================
//              クラス/構造体定義
//==============================================

//==============================================
//                   関数定義
//==============================================
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void POKE_NET_OptEncode(const unsigned char *_In, long _Size, unsigned char *_Out);	// オプションエンコード
void POKE_NET_OptDecode(const unsigned char *_In, long _Size, unsigned char *_Out);	// オプションデコード
long POKE_NET_GetRequestSize(long _reqno);											// リクエストサイズ取得
long POKE_NET_GetResponseMaxSize(long _reqno);										// レスポンス最大サイズ取得
u64 POKE_NET_EncodePlayDataID(u64 _id);												// プレイデータＩＤの暗号化
u64 POKE_NET_DecodePlayDataID(u64 _id);												// プレイデータＩＤの複合化

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ___POKE_NET_OPT___
