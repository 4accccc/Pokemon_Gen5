/*-----------------------------------------------------------------*
  Project:  NetAgent AM - include
  File:     AMProtect.h

  Copyright NetAgent Co. Ltd., All rights reserved.

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

#define AM_VERSION (127)

/* -- 関数ポインタ操作用定義 ------------------------------------- */
#define _ADD_VAL        ((u32)(8 * 100))
#define _SET_FUNC(x)    (((u32)(x)) + (_ADD_VAL))
#define _GET_FUNC(x)    (((u32)(x)) - (_ADD_VAL))

/* 引数に使用する関数ポインタの定義 */
typedef void (*AM_PTR_FUNC)(void);
typedef u32 (*AM_CALLBACK_PTR)(void*);

/* 実体関数をエクスポート */
#ifdef __cplusplus
extern "C" {
#endif


#ifndef __UTILS_H__
# define IMPORT_AM_API(__FUNC) Stub_##__FUNC
#endif /* __UTILS_H__ */


// API定義
extern u32 IMPORT_AM_API(ProtectB1)(AM_CALLBACK_PTR addr, void* param, u32 reserve);
extern u32 IMPORT_AM_API(NotProtectB1)(AM_CALLBACK_PTR addr, void* param, u32 reserve);
extern u32 IMPORT_AM_API(ProtectB2)(AM_CALLBACK_PTR addr, void* param, u32 reserve);
extern u32 IMPORT_AM_API(NotProtectB2)(AM_CALLBACK_PTR addr, void* param, u32 reserve);
extern u32 IMPORT_AM_API(ProtectB3)(AM_CALLBACK_PTR addr, void* param, u32 reserve);
extern u32 IMPORT_AM_API(NotProtectB3)(AM_CALLBACK_PTR addr, void* param, u32 reserve);


// ver 1.24 において ver 1.23以前とのAPI互換性をとるための関数
static u32 __wrap_func( void* param ){
	if( param ){
		(*((AM_PTR_FUNC)param))();
	}
	return ~(u32)param;
}


/*-----------------------------------------------------------------*
  undocumented
 *-----------------------------------------------------------------*/
#ifdef SDK_FINALROM
#define AM_IsMagiconB1(func,arg,reserve)	(IMPORT_AM_API(ProtectB1)(func,arg,reserve))
#define AM_IsMagiconB2(func,arg,reserve)	(IMPORT_AM_API(ProtectB2)(func,arg,reserve))
#define AM_IsMagiconB3(func,arg,reserve)	(IMPORT_AM_API(ProtectB3)(func,arg,reserve))
#else
/* DEBUG,RELEASE版では公式開発環境でのチェックを抑制する */
static u32 AM_IsMagiconB1(AM_CALLBACK_PTR func, void *arg, u32 reserve) {
	
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
		return (u32)arg;
	default:
		;
	}
	
	return IMPORT_AM_API(ProtectB1)( func, arg, reserve );
	
}

static u32 AM_IsMagiconB2(AM_CALLBACK_PTR func, void *arg, u32 reserve) {
	
	switch (OS_GetConsoleType() & OS_CONSOLE_MASK) {
	case OS_CONSOLE_ENSATA:
	case OS_CONSOLE_ISEMULATOR:
	case OS_CONSOLE_ISDEBUGGER:
#ifdef OS_CONSOLE_TWLDEBUGGER
	case OS_CONSOLE_TWLDEBUGGER:
#endif
		return (u32)arg;
	default:
		;
	}
	
	return IMPORT_AM_API(ProtectB2)( func, arg, reserve );
	
}

static u32 AM_IsMagiconB3(AM_CALLBACK_PTR func, void *arg, u32 reserve) {
	
	switch (OS_GetConsoleType() & OS_CONSOLE_MASK) {
	case OS_CONSOLE_ENSATA:
	case OS_CONSOLE_ISEMULATOR:
	case OS_CONSOLE_ISDEBUGGER:
#ifdef OS_CONSOLE_TWLDEBUGGER
	case OS_CONSOLE_TWLDEBUGGER:
#endif
		return (u32)arg;
	default:
		;
	}
	
	return IMPORT_AM_API(ProtectB3)( func, arg, reserve );
	
}

#endif /* SDK_FINALROM */


/*-----------------------------------------------------------------*
  undocumented
 *-----------------------------------------------------------------*/
#ifdef SDK_FINALROM
#define AM_IsNotMagiconB1(func,arg,seed)	(IMPORT_AM_API(NotProtectB1)(func,arg,seed))
#define AM_IsNotMagiconB2(func,arg,seed)	(IMPORT_AM_API(NotProtectB2)(func,arg,seed))
#define AM_IsNotMagiconB3(func,arg,seed)	(IMPORT_AM_API(NotProtectB3)(func,arg,seed))
#else
/* DEBUG,RELEASE版では公式開発環境でのチェックを抑制する */
static u32 AM_IsNotMagiconB1(AM_CALLBACK_PTR func, void *arg, u32 reserve) {
	
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
		if (NULL != func) {
			(*func)( arg );
		}
		return ~(u32)arg;
	default:
		;
	}
	
	return IMPORT_AM_API(NotProtectB1)( func, arg, reserve );
	
}

static u32 AM_IsNotMagiconB2(AM_CALLBACK_PTR func, void *arg, u32 reserve) {
	
	switch (OS_GetConsoleType() & OS_CONSOLE_MASK) {
	case OS_CONSOLE_ENSATA:
	case OS_CONSOLE_ISEMULATOR:
	case OS_CONSOLE_ISDEBUGGER:
#ifdef OS_CONSOLE_TWLDEBUGGER
	case OS_CONSOLE_TWLDEBUGGER:
#endif
		if (NULL != func) {
			(*func)( arg );
		}
		return ~(u32)arg;
	default:
		;
	}
	
	return IMPORT_AM_API(NotProtectB2)( func, arg, reserve );
	
}


static u32 AM_IsNotMagiconB3(AM_CALLBACK_PTR func, void *arg, u32 reserve) {
	
	switch (OS_GetConsoleType() & OS_CONSOLE_MASK) {
	case OS_CONSOLE_ENSATA:
	case OS_CONSOLE_ISEMULATOR:
	case OS_CONSOLE_ISDEBUGGER:
#ifdef OS_CONSOLE_TWLDEBUGGER
	case OS_CONSOLE_TWLDEBUGGER:
#endif
		if (NULL != func) {
			(*func)( arg );
		}
		return ~(u32)arg;
	default:
		;
	}
	
	return IMPORT_AM_API(NotProtectB3)( func, arg, reserve );
	
}

#endif /* SDK_FINALROM */


/*
  version 1.23以前のAPIとの互換性のための関数定義
  インライン関数定義を使用してユーザーコードエリアに展開させる
*/
inline static u32 AM_IsMagiconA1( AM_PTR_FUNC func ){ return AM_IsMagiconB1(__wrap_func, func,0) == ~(u32)func; }
inline static u32 AM_IsMagiconA2( AM_PTR_FUNC func ){ return AM_IsMagiconB2(__wrap_func, func,0) == ~(u32)func; }
inline static u32 AM_IsMagiconA3( AM_PTR_FUNC func ){ return AM_IsMagiconB3(__wrap_func, func,0) == ~(u32)func; }
inline static u32 AM_IsNotMagiconA1( AM_PTR_FUNC func ){ return AM_IsNotMagiconB1(__wrap_func, func,0) == ~(u32)func; }
inline static u32 AM_IsNotMagiconA2( AM_PTR_FUNC func ){ return AM_IsNotMagiconB2(__wrap_func, func,0) == ~(u32)func; }
inline static u32 AM_IsNotMagiconA3( AM_PTR_FUNC func ){ return AM_IsNotMagiconB3(__wrap_func, func,0) == ~(u32)func; }

#ifdef __cplusplus
}
#endif


/* _AM_PROTECT_H_ */
#endif


/* --------------------------------------------------------- EOF --*/
