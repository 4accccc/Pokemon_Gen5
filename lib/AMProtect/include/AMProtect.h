/*-----------------------------------------------------------------*
  Project:  NetAgent AM - include
  File:     AMProtect.h

  Copyright NetAgent Co. Ltd., All rights reserved.

  Revision 2.05 2011/10/26
  一部の非公式カーネルに対応
  
  Revision 2.04 2011/08/24
  検出できないマジコンを検出できるように修正
  暗号化方式の改善
  内部処理を変更
  
  Revision 2.03 2010/01/25
  検出できないマジコンを検出できるように修正
  暗号化方式の改善
  内部処理を変更
   
  Revision 2.02 2010/10/19
  検出できないマジコンを検出できるように修正
  
  Revision 2.01 2010/06/30
  DSTWO,ezViに対応
  初期化をNitroStaticInitから明示的（AM_Init）に変更
  
  Revision 2.00 2010/06/14
  検知APIのインターフェース変更
  暗号化方式の改善
  内部処理を変更
  
  Revision 1.28 2010/06/02
  HDMA転送とのタイミングによる不都合修正
  検出できないマジコンを検出できるように修正
  
  Revision 1.27 2010/02/26
  暗号鍵変更の実施
  内部処理を変更
  
  Revision 1.26 2009/12/09
  暗号化方式の改良
  検出できないマジコンを検出できるように修正
  
  Revision 1.25 2009/11/18
  デバッグ・リリースビルド時にデバッガをマジコン判定してしまう不都合に対応
  
  Revision 1.24 2009/10/09
  暗号化方式の改良
  内部処理を変更
  若干の高速化を実施
  暗号鍵変更の実施
  
  Revision 1.23 2009/08/11
  暗号化方式の変更
  
  Revision 1.22 2009/07/01
  Thumb版Nitro/TwlSDKを使用した場合に誤検知するバグを修正
  
  Revision 1.20 2009/04/14
  TWLモード(HYBRID/LTD)に対応
  内部処理を改変
  暗号鍵変更の実施
  
  Revision 1.10 2009/03/10
  特定条件下でクラッシュする可能性があたバグを修正
  暗号鍵変更の実施
  
  Revision 1.08 2008/12/25
  暗号鍵変更の実施（これまでも実施されていたが非アナウンス）
  
  Revision 1.07 2008/12/14
  ヘッダファイルにエラーチェックを追記
  
  Revision 1.06 2008/11/12
  関数ポインタの保持方法および内部処理を改変

  Revision 1.05 2008/08/21
  バージョンの打ち直しを実施

  Revision 0.3  2008/07/25
  エミュレータ検出関数の追加
  
  Revision 0.2  2008/04/26
  DEBUG, RELEASEモード時でのensata,IS-NITRO-EMULATOR,
  IS-NITRO-DEBUGGERの検出抑制
  
  Revision 0.1  2008/03/16
  初期バージョン
  
 *-----------------------------------------------------------------*/

 
#ifndef _AM_PROTECT_H_
#define _AM_PROTECT_H_

#include <nitro.h>
#include <nitro/types.h>
#include <nitro/os.h>
#include <nitro/card.h>

#define AM_VERSION (205)


/* 引数に使用する関数ポインタの定義 */
#ifndef __AM_USER_CALLBACK__
# define __AM_USER_CALLBACK__
  typedef void* (*AM_USER_CALLBACK)(void*,void*);
# endif

/* API 呼び出しマクロ */
# define IMPORT_AM_API(__FUNC) Stub_##__FUNC

/* 実体関数をエクスポート */
#ifdef __cplusplus
extern "C" {
#endif

/* -- callbackポインタ用 --------------------------------------------------------
 * g_am_user_callback および g_am_user_callback_order の実体は
 * AMProtectR2a.a に定義されています。
 */
extern AM_USER_CALLBACK g_am_user_callback[2];
extern u32 g_am_user_callback_order;

/* ------------------------------------------------------------------------------
 * ライブラリ側関数定義
 * 実際には AM_IsMagiconB* 関数より呼び出される
 */
extern void* IMPORT_AM_API(ProtectB1)(void* param1, void* param2);
extern void* IMPORT_AM_API(ProtectB2)(void* param1, void* param2);
extern void* IMPORT_AM_API(ProtectB3)(void* param1, void* param2);
extern void* IMPORT_AM_API(ProtectB4)(void* param1, void* param2);

/* ------------------------------------------------------------------------------
 * 検知関数の改ざんチェック用マクロ
 */
#define CHECK_AND_INVOKE(func,p1,p2)											\
	u32* addr = (u32*)(func);													\
	u32 checksum, i;															\
	i = 37;																		\
	checksum = 0;																\
	do																			\
	{																			\
		checksum ^= ( ((*addr)>>i) | ((*addr)<<(32-i)) );						\
		++addr;																	\
	}while( --i );																\
	if( checksum == 0x9F75A8D6 )												\
	{																			\
		return (func)((p1),(p2));												\
	}																			\
	else																		\
	{																			\
		return (g_am_user_callback[ g_am_user_callback_order ^ 1 ])((p1),(p2));	\
	}

/*-----------------------------------------------------------------*
 * DSプロテクト ライブラリ API 定義
 *-----------------------------------------------------------------*/
 
/*
 * AM_IsMagiconB* 関数内部で使用する、反映関数のポインタをセットする
 *  [引数]
 *    legal_cb : 正規環境時に AM_IsMagiconB* 関数内部より呼び出される関数へのポインタ
 *    illegal_cb : 不正環境時に AM_IsMagiconB* 関数内部より呼び出される関数へのポインタ
 *  [備考]
 *    この関数は、実行時コスト低減のために inline 展開される
 *    
 */
inline void AM_SetCallback( AM_USER_CALLBACK legal_cb, AM_USER_CALLBACK illegal_cb )
{
	// VBlankCounterを使ってCB格納先を変化させる
	g_am_user_callback_order = OS_GetVBlankCount() & 1ul;
	g_am_user_callback[ g_am_user_callback_order ]       = legal_cb;
	g_am_user_callback[ g_am_user_callback_order ^ 1ul ] = illegal_cb;
}

#ifdef SDK_FINALROM
/* -----------------------------------------------------------------------------
 * AM_IsMagiconB*
 *   実際にマジコン検知を行います。
 *   AM_IsMagiconB1 : マジコン検知を行います
 *   AM_IsMagiconB2 : エミュレータ検知を行います
 *   AM_IsMagiconB3 : マジコン検知を行います
 *   AM_IsmagiconB4 : エミュレータ検知を行います
 *  [引数]
 *    param1, param2 : AM_SetCallback で設定した関数を実行する際に、
 *                     引数としてパススルーされます
 *                     内部での使用用途は定められていないので、
 *                     自由に使用する事が可能です
 *  [備考]
 *    AM_IsMagiconB1 と AM_IsMagiconB3 は、動作結果は同一ですが内部での実装は異なります
 *    AM_IsMagiconB2 と AM_IsMagiconB4 は、動作結果は同一ですが内部での実装は異なります
 *    Debug・Releaseビルド時には、公式エミュレータ・デバッガは検知対象外となり、
 *    常に正規環境となります。
 */
inline void* AM_IsMagiconB1( void* param1, void* param2 )
{
	CHECK_AND_INVOKE( IMPORT_AM_API(ProtectB1), param1, param2 );
}

inline void* AM_IsMagiconB2( void* param1, void* param2 )
{
	CHECK_AND_INVOKE( IMPORT_AM_API(ProtectB2), param1, param2 );
}

inline void* AM_IsMagiconB3( void* param1, void* param2 )
{
	CHECK_AND_INVOKE( IMPORT_AM_API(ProtectB3), param1, param2 );
}

inline void* AM_IsMagiconB4( void* param1, void* param2 )
{
	CHECK_AND_INVOKE( IMPORT_AM_API(ProtectB4), param1, param2 );
}
#else
/* DEBUG,RELEASE版では公式開発環境でのチェックを抑制する */
static void* AM_IsMagiconB1( void *param1, void* param2 ) {
	
	// LockIDの空き確認
	// LockIDを使い切っているようであればOS_Panic()
	s32 lockId = OS_GetLockID();
	if (OS_LOCK_ID_ERROR == lockId) {
		OS_Panic("lock ID get error.");
	}
	OS_ReleaseLockID((u16)lockId);


	switch (OS_GetConsoleType() & OS_CONSOLE_MASK) {
	case OS_CONSOLE_ENSATA:
	case OS_CONSOLE_ISEMULATOR:
	case OS_CONSOLE_ISDEBUGGER:
#ifdef OS_CONSOLE_TWLDEBUGGER
	case OS_CONSOLE_TWLDEBUGGER:
#endif
		return g_am_user_callback[ g_am_user_callback_order ]( param1, param2 );
	default:
		;
	}
	
	return IMPORT_AM_API(ProtectB1)( param1, param2 );
	
}

static void* AM_IsMagiconB2( void *param1, void* param2 ) {
	
	switch (OS_GetConsoleType() & OS_CONSOLE_MASK) {
	case OS_CONSOLE_ENSATA:
	case OS_CONSOLE_ISEMULATOR:
	case OS_CONSOLE_ISDEBUGGER:
#ifdef OS_CONSOLE_TWLDEBUGGER
	case OS_CONSOLE_TWLDEBUGGER:
#endif
		return g_am_user_callback[ g_am_user_callback_order ]( param1, param2 );
	default:
		;
	}
	
	return IMPORT_AM_API(ProtectB2)( param1, param2 );
	
}

static void* AM_IsMagiconB3( void *param1, void* param2 ) {
	
	// LockIDの空き確認
	// LockIDを使い切っているようであればOS_Panic()
	s32 lockId = OS_GetLockID();
	if (OS_LOCK_ID_ERROR == lockId) {
		OS_Panic("lock ID get error.");
	}
	OS_ReleaseLockID((u16)lockId);
	
	
	switch (OS_GetConsoleType() & OS_CONSOLE_MASK) {
	case OS_CONSOLE_ENSATA:
	case OS_CONSOLE_ISEMULATOR:
	case OS_CONSOLE_ISDEBUGGER:
#ifdef OS_CONSOLE_TWLDEBUGGER
	case OS_CONSOLE_TWLDEBUGGER:
#endif
		return g_am_user_callback[ g_am_user_callback_order ]( param1, param2 );
	default:
		;
	}
	
	return IMPORT_AM_API(ProtectB3)( param1, param2 );
	
}

static void* AM_IsMagiconB4( void *param1, void* param2 ) {
	
	switch (OS_GetConsoleType() & OS_CONSOLE_MASK) {
	case OS_CONSOLE_ENSATA:
	case OS_CONSOLE_ISEMULATOR:
	case OS_CONSOLE_ISDEBUGGER:
#ifdef OS_CONSOLE_TWLDEBUGGER
	case OS_CONSOLE_TWLDEBUGGER:
#endif
		return g_am_user_callback[ g_am_user_callback_order ]( param1, param2 );
	default:
		;
	}
	
	return IMPORT_AM_API(ProtectB4)( param1, param2 );
	
}

#endif /* SDK_FINALROM */

/*------------------------------------------------------------------------------
 * DSプロテクトライブラリの初期化を行います
 * 引数 : 無し
 * 戻り値 : 無し
 *
 * 各種検知関数を呼び出す前に一度だけ実行を行う必要があります
 * 二度目以降の呼び出しでは何も行わないため、複数回呼び出しても問題はありません
 ---------------------------------------------------------------------------- */
void AM_Init();


#ifdef __cplusplus
}
#endif


/* _AM_PROTECT_H_ */
#endif


/* --------------------------------------------------------- EOF --*/
