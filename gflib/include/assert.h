//=============================================================================================
/**
 * @file	assert.h
 * @date	2007.01.19
 * @author	GAME FREAK inc.
 */
//=============================================================================================

#ifndef	__ASSERT_H__
#define	__ASSERT_H__

#define	GF_ASSERT_ENABLE


#ifdef	GF_ASSERT_ENABLE

//ASSERTñ≥å¯ÇÃèÍçá
extern void GFL_Assert(const char * filename, unsigned int line_no, const char * exp);
extern void GFL_AssertMsg(const char * filename, unsigned int line_no, const char * fmt, ... );
extern void GFL_AssertPrintf(const char * fmt, ... );

#define GF_ASSERT(exp)		\
	(exp)? ((void)0): GFL_Assert(__FILE__, __LINE__, #exp);
#define GF_ASSERT_MSG(exp, ... )		\
	(exp)? ((void)0): GFL_AssertMsg(__FILE__, __LINE__, __VA_ARGS__ );
//#define GF_ASSERT_Printf( ... )		((exp)? ((void)0);


#else
//ASSERTóLå¯ÇÃèÍçá

#define GF_ASSERT(exp)				((void)0);
#define GF_ASSERT_MSG(exp, ...)		((void)0);
#define GF_ASSERT_Printf( ... )		((void)0);

#endif


#endif	/* __ASSERT_H__ */
