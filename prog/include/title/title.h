//==============================================================================
/**
 * @file	title.h
 * @brief	タイトル画面のヘッダ
 * @author	matsuda
 * @date	2008.12.03(水)
 */
//==============================================================================
#ifndef __TITLE_H__
#define __TITLE_H__

//==============================================================================
//	定数定義
//==============================================================================
FS_EXTERN_OVERLAY(title);

typedef struct {
	// [ in ]
#ifdef	PM_DEBUG
	u32	skipMode;			// タイトルスキップ（デバッグ用）
#endif	// PM_DEBUG
	// [ out ]
	u32	endMode;			// 終了モード
}TITLE_PARAM;

// 終了モード
enum {
	TITLE_END_SELECT = 0,		// ゲーム開始
	TITLE_END_TIMEOUT,			// タイトルループ
#ifdef	PM_DEBUG
	TITLE_END_DEBUG_CALL,		// デバッグメニュー呼び出し
#endif	// PM_DEBUG
};


//==============================================================================
//	外部データ
//==============================================================================
extern const GFL_PROC_DATA TitleProcData;
extern const GFL_PROC_DATA TitleControlProcData;


#endif	//__TITLE_H__
