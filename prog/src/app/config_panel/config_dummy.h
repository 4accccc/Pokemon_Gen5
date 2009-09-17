

//------------------------------------------------------------------
//	カラー関連ダミー定義
//------------------------------------------------------------------
// 通常のフォントカラー
#define	FBMP_COL_NULL		(0)
#define	FBMP_COL_BLACK		(1)
#define	FBMP_COL_BLK_SDW	(2)
#define	FBMP_COL_RED		(3)
#define	FBMP_COL_RED_SDW	(4)
#define	FBMP_COL_GREEN		(5)
#define	FBMP_COL_GRN_SDW	(6)
#define	FBMP_COL_BLUE		(7)
#define	FBMP_COL_BLU_SDW	(8)
#define	FBMP_COL_PINK		(9)
#define	FBMP_COL_PNK_SDW	(10)
#define	FBMP_COL_WHITE		(15)
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//	SE関連ダミー定義
//------------------------------------------------------------------
enum{
	SEQ_SE_DP_SAVE = 10,
	SEQ_SE_DP_SELECT = 11,
};
static void Snd_PlaySE(int seqno) { /* dummy */ }
static void Snd_SetMonoFlag(int mode) { /* dummy */ }
static void Snd_SeStopBySeqNo(int seqno, int para) { /* dummy */ }

//------------------------------------------------------------------
//	ワイプ関連ダミー定義
//------------------------------------------------------------------
enum {
	COMM_BRIGHTNESS_SYNC = 1,
};

//------------------------------------------------------------------
//	MSG関連ダミー定義
//------------------------------------------------------------------
enum {
	MSG_SKIP_OFF = 0,
	MSG_SKIP_ON
};
static inline void MsgPrintSkipFlagSet(BOOL flag){/* */;}



