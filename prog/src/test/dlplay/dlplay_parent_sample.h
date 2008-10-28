//============================================================================================
/**
 * @file	dlplay_parent_sample.c
 * @brief	ダウンロードプレイサンプル
 * @author	ariizumi
 * @date	2008.10.8
 */
//============================================================================================

#ifndef DLPLAY_PARENT_SAMPLE_H__
#define DLPLAY_PARENT_SAMPLE_H__

//BG面定義
#define DLPLAY_MSG_PLANE			(GFL_BG_FRAME3_M)
#define DLPLAY_MSG_PLANE_PRI		(0)

enum DLPLAY_PARENT_MODE
{
	DPM_SEND_IMAGE,	//子機にイメージを配布する
	DPM_SELECT_BOX,	//取るボックスを選択する
	DPM_ERROR,		//とりあえずエラー
	DPM_END,		//終了用
};

#endif //DLPLAY_PARENT_SAMPLE_H__


