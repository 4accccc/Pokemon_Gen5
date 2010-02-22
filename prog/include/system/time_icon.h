//============================================================================================
/**
 * @file		time_icon.h
 * @brief		セーブ中などに表示されるタイマーアイコン処理
 * @author	Hiroyuki Nakamura
 * @date		10.02.22
 *
 *	モジュール名：TIMEICON
 */
//============================================================================================
#pragma	once



// タイマーアイコンワーク
typedef struct _TIMEICON_WORK	TIMEICON_WORK;

#define	TIMEICON_DEFAULT_WAIT		( 16 )



extern TIMEICON_WORK * TIMEICON_Create(
												GFL_TCBSYS * tcbsys, GFL_BMPWIN * msg_win, u8 clear_color, u8 wait, HEAPID heapID );

extern void TILEICON_Exit( TIMEICON_WORK * wk );
