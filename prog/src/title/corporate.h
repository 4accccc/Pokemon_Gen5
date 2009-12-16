//============================================================================================
/**
 * @file		corporate.h
 * @brief		会社名表示
 * @author	Hiroyuki Nakamura
 * @date		2009.12.16
 */
//============================================================================================
#pragma	once


#ifdef	PM_DEBUG
// 終了モード
enum {
	CORPORATE_RET_NORMAL = 0,		// 通常終了
	CORPORATE_RET_SKIP,					// スキップ
	CORPORATE_RET_DEBUG,				// デバッグへ
};
#endif


// PROCデータ
extern const GFL_PROC_DATA CorpProcData;
