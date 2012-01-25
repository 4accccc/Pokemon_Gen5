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
  
  Revision 2.03 2011/01/28
  検出できないマジコンを検出できるように修正
  暗号化方式の改善
  内部処理を変更
  
  Revision 2.02 2010/10/22
  検出できないマジコンを検出できるように修正

  Revision 2.01 2010/06/30
  DSTWO/ezViに対応
  
  Revision 2.00 2010/06/14
  通常版より簡易版作成
  バージョン自体は2.00で開始

 *-----------------------------------------------------------------*/

 
#ifndef _AM_PROTECT_INSTANT_H_
#define _AM_PROTECT_INSTANT_H_

#include <nitro.h>
#include <nitro/types.h>
#include <nitro/os.h>
#include <nitro/card.h>

#define AM_VERSION (205)


/* 引数に使用する関数ポインタの定義 */
#ifndef __AM_USER_CALLBACK__
#define __AM_USER_CALLBACK__
typedef void* (*AM_USER_CALLBACK)(void*,void*);
#endif

/* API 呼び出しマクロ */
# define IMPORT_AM_API(__FUNC) Stub_##__FUNC

/* 実体関数をエクスポート */
#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------
 * ライブラリ側関数定義
 * 実際には AM_IsMagiconB* 関数より呼び出される
 */
extern void* IMPORT_AM_API(InstantDetect)(AM_USER_CALLBACK cb, void* param1, void* param2);
extern void* IMPORT_AM_API(InstantFreeze)(void* param1, void* param2 );

/*-----------------------------------------------------------------*
 * DSプロテクト ライブラリ API 定義
 *-----------------------------------------------------------------*/

#ifdef SDK_FINALROM
/* -----------------------------------------------------------------------------
 * AM_InstantPiracyDetect
 *  実際にマジコン検知を行います。
 *  [引数]
 *    cb : 正規環境時に呼びだされるコールバック関数
 *    param1, param2 : コールバック関数 cb を実行する際に、引数としてパススルーされます
 *                     内部での使用用途は定められていないので、自由に使用する事が可能です
 *  [備考]
 *    Debug・Releaseビルド時には、公式エミュレータ・デバッガは検知対象外となり、
 *    常に正規環境となります。
 */
inline void* AM_InstantPiracyDetect( AM_USER_CALLBACK cb, void* param1, void* param2 )
{
	u32* addr = (u32*)IMPORT_AM_API(InstantDetect);
	u32 checksum, i;
	i = 37;
	checksum = 0;
	do
	{
		checksum ^= ( ((*addr)>>i) | ((*addr)<<(32-i)) );
		++addr;
	}while( --i );
	if( checksum == 0x9F75A8D6 )
	{
		return IMPORT_AM_API(InstantDetect)( cb, param1, param2 );
	}
	else
	{
		return IMPORT_AM_API(InstantFreeze)( NULL, NULL );
	}
}

#else
/* DEBUG,RELEASE版では公式開発環境でのチェックを抑制する */
static void* AM_InstantPiracyDetect( AM_USER_CALLBACK cb, void *param1, void* param2 ) {
	
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
		if( cb )
		{
			return cb( param1, param2 );
		}
		else
		{
			return 0;
		}
	default:
		;
	}
	
	return IMPORT_AM_API(InstantDetect)( cb, param1, param2 );
	
}


#endif /* SDK_FINALROM */

#ifdef __cplusplus
}
#endif


/* _AM_PROTECT_INSTANT_H_ */
#endif


/* --------------------------------------------------------- EOF --*/
