//=============================================================================================
/**
 * @file	assert.h
 * @date	2007.01.19
 * @author	GAME FREAK inc.
 */
//=============================================================================================

#ifndef	__ASSERT_H__
#define	__ASSERT_H__

#ifdef __cplusplus
extern "C" {
#endif

//デバッグ定義がなければ
#ifdef PM_DEBUG
#define	GF_ASSERT_ENABLE
#endif//PM_DEBUG


#ifdef	GF_ASSERT_ENABLE

// ASSERT有効の場合 --------------------------------------------------------------------------
extern void GFL_Assert(const char * filename, unsigned int line_no, const char * exp);
extern void GFL_AssertMsg(const char * filename, unsigned int line_no, const char * fmt, ... );

#define GF_ASSERT(exp)		\
	(exp)? ((void)0): GFL_Assert(__FILE__, __LINE__, #exp);
#define GF_ASSERT_MSG(exp, ... )		\
	(exp)? ((void)0): GFL_AssertMsg(__FILE__, __LINE__, __VA_ARGS__ );
//#define GF_ASSERT_Printf( ... )		((exp)? ((void)0);
//
#define GF_PANIC(...)         GF_ASSERT_MSG(0,__VA_ARGS__)


/* ASSERT情報出力をユーザ定義するための関数型定義 */

typedef void (*GFL_ASSERT_DispInitFunc)(void);
typedef void (*GFL_ASSERT_DispMainFunc)( const char* );
typedef void (*GFL_ASSERT_DispFinishFunc)(void);

extern void GFL_ASSERT_SetDisplayFunc( GFL_ASSERT_DispInitFunc initFunc, GFL_ASSERT_DispMainFunc mainFunc, GFL_ASSERT_DispFinishFunc finishFunc );


#else
// ASSERT無効の場合 --------------------------------------------------------------------------

#define GF_ASSERT(exp)				((void)0);
#define GF_ASSERT_MSG(exp, ...)		((void)0);

#define GF_PANIC(...)         OS_Panic(__VA_ARGS__)

#endif

#ifdef __cplusplus
}/* extern "C" */
#endif


#endif	/* __ASSERT_H__ */
