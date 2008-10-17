//======================================================================
/**
 *
 * @file	ari_debug.h
 * @brief	有泉汎用デバッグ
 * @author	ariizumi
 * @data	08.10.16
 */
//======================================================================
#ifndef ARI_DEBUG_H__
#define ARI_DEBUG_H__

//デバッグ有無のフラグ
#if DEBUG_ONLY_FOR_ariizumi_nobuhiko
#define DEB_ARI 1
#else
#define DEB_ARI 0
#endif

//デバッグ用マクロ
#if DEB_ARI
#define ARI_TPrintf(...) (void)((OS_TPrintf(__VA_ARGS__)))
#define ARI_Printf(...)  (void)((OS_Printf(__VA_ARGS__)))
#else
#define ARI_TPrintf(...) ((void)0)
#define ARI_Printf(...)  ((void)0)
#endif //DEB_ARI

#endif //ARI_DEBUG_H__
