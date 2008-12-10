//===================================================================
/**
 * @file	performance.h
 * @brief	パフォーマンスメーター　デバッグ用
 * @author	GAME FREAK Inc.
 * @date	08.08.01
 */
//===================================================================

#ifdef PM_DEBUG


//==============================================================================
//	定数定義
//==============================================================================
///パフォーマンスメーターID
typedef enum{
	PERFORMANCE_ID_MAIN,				///<メインループ
	PERFORMANCE_ID_VBLANK,				///<Vブランク
	
	PERFORMANCE_ID_USER_A,				///<ユーザー使用A
	PERFORMANCE_ID_USER_VBLANK_A,		///<ユーザー使用A(VBlank処理負荷用)
	PERFORMANCE_ID_USER_B,				///<ユーザー使用B
	PERFORMANCE_ID_USER_VBLANK_B,		///<ユーザー使用B(VBlank処理負荷用)

	PERFORMANCE_ID_MAX,
}PERFORMANCE_ID;

///数値の負荷表示をするID(VBLANK負荷計測は未対応)
#define PERFORMANCE_NUM_PRINT_ID		(PERFORMANCE_ID_MAIN)


//==============================================================================
//	外部関数宣言
//==============================================================================
extern void DEBUG_PerformanceInit(void);
extern void DEBUG_PerformanceMain(void);
extern void DEBUG_PerformanceStartLine(PERFORMANCE_ID id);
extern void DEBUG_PerformanceEndLine(PERFORMANCE_ID id);
extern void DEBUG_PerformanceSetActive( BOOL isActive );


#endif //PM_DEBUG
