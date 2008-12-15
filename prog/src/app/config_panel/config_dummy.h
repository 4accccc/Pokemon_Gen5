//------------------------------------------------------------------
/*
 * 	簡単メモリリークチェック（現在は無効）
 */
//------------------------------------------------------------------
#define HeapStatePush()		/* */
#define HeapStatePop()		/* */
#define HeapStateCheck(h)	/* */


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
typedef struct _CATS_SYS * CATS_SYS_PTR;
typedef struct _CATS_RES * CATS_RES_PTR;
typedef struct _CATS_ACT * CATS_ACT_PTR;
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

enum {
	WIPE_TYPE_FADEOUT = 0,
	WIPE_TYPE_FADEIN = 1,

	WIPE_PATTERN_M = 0,

	WIPE_DISP_MAIN,
	WIPE_DISP_SUB,
};
static inline void WIPE_ResetWndMask(int sw) {/* */}
static inline void WIPE_SYS_Start(int pattern, int type1, int type2, u16 value,
		int timing, int sw, HEAPID heapID)
{
	if (type1 == WIPE_TYPE_FADEIN) {
		GFL_FADE_SetMasterBrightReq(
				GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,
				16, 0, 0);
	} else {
		GFL_FADE_SetMasterBrightReq(
				GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,
				0, 16, 0);
	}
}

static inline BOOL WIPE_SYS_EndCheck(void)
{
	return GFL_FADE_CheckFade();
}

//------------------------------------------------------------------
//	MSG関連ダミー定義
//------------------------------------------------------------------
enum {
	MSG_SKIP_OFF = 0,
	MSG_SKIP_ON
};
static inline void MsgPrintSkipFlagSet(BOOL flag){/* */;}



