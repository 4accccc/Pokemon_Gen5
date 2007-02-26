//===================================================================
/**
 * @file	main.h
 * @brief	アプリケーション　サンプルスケルトン
 * @author	GAME FREAK Inc.
 * @date	06.11.28
 *
 * $LastChangedDate$
 * $Id$
 */
//===================================================================

//親ヒープ用宣言
enum{
	GFL_HEAPID_APP = GFL_HEAPID_SYSTEM + 1,
	GFL_HEAPID_MAX
};

//子ヒープ用宣言
enum{
	HEAPID_BG=GFL_HEAPID_MAX,

	HEAPID_WATANABE_DEBUG,
	HEAPID_SOGABE_DEBUG,
	HEAPID_TAMADA_DEBUG,
	HEAPID_OHNO_DEBUG,
	HEAPID_TAYA_DEBUG,
	HEAPID_NAKAHIRO_DEBUG,
	HEAPID_TOMOYA_DEBUG,
    HEAPID_NETWORK,
    HEAPID_WIFI,

	HEAPID_CHILD_MAX,
};

#define	HEAPSIZE_SYSTEM	(0x08000)
#define	HEAPSIZE_APP	(0x80000)

