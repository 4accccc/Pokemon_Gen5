//============================================================================================
/**
	* @file	pmsi_sound_def.h
	* @bfief	ボックス画面サウンドナンバー定義
	* @author	taya
	* @date	06.02.15
	*/
//============================================================================================

#ifndef __PMSI_SOUND_DEF_H__
#define __PMSI_SOUND_DEF_H__

#include "sound\pm_sndsys.h"

#define PMS_USE_SND (1)

#if PMS_USE_SND
#define  SOUND_MOVE_CURSOR		(SEQ_SE_SELECT1)		// カーソル移動

#define  SOUND_DECIDE			(SEQ_SE_DECIDE1)		// 決定音

#define  SOUND_CANCEL			(SEQ_SE_CANCEL1)		// キャンセル音

#define  SOUND_CHANGE_SENTENCE	(SEQ_SE_SELECT1)		// 文章変更

#define  SOUND_CHANGE_CATEGORY	(SEQ_SE_SELECT4)		// カテゴリ変更
//#define  SOUND_CHANGE_CATEGORY	(SEQ_SE_DP_BUTTON9)		// カテゴリ変更

#define  SOUND_DISABLE_CATEGORY	(SEQ_SE_BEEP)	// 無効なカテゴリ選択
//#define  SOUND_DISABLE_CATEGORY	(SEQ_SE_DP_CUSTOM06)	// 無効なカテゴリ選択

#define  SOUND_DISABLE_BUTTON	(SEQ_SE_BEEP)	// 無効なボタンタッチ
//#define  SOUND_DISABLE_BUTTON	(SEQ_SE_DP_CUSTOM06)	// 無効なボタンタッチ

#define SOUND_WORD_INPUT (SEQ_SE_DECIDE1)  // 文字入力
#define SOUND_WORD_DELETE (SEQ_SE_CANCEL3)  // 文字削除

#define SOUND_SEARCH_DISABLE (SEQ_SE_BEEP) // 検索失敗

#define SOUND_TOUCH_FLIPBUTTON (SEQ_SE_SELECT1) // センテンス変更ボタンタッチ

#define SOUND_TOUCH_SLIDEBAR (SEQ_SE_SYS_06)    // スライドバーをタッチ

#endif //PMS_USE_SND

#endif	//__PMSI_SOUND_DEF_H__
