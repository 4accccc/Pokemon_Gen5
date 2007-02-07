
//============================================================================================
/**
 * @file	yt_common.h
 * @brief	DS版ヨッシーのたまごcommonヘッダー
 * @date	2007.02.06
 */
//============================================================================================

#ifndef	__YT_COMMON_H__
#define	__YT_COMMON_H__

//ジョブナンバー定義
enum{
	YT_InitTitleNo=0,
	YT_MainTitleNo,
	YT_InitGameNo,
	YT_MainGameNo,
};

//ゲームパラメータ構造体宣言
typedef	struct
{
	int		job_no;			//ジョブナンバー
	TCBSYS	*tcbsys;
	void	*tcb_work;
	TCB		*tcb[3];
}GAME_PARAM;

#endif	__YT_COMMON_H__
