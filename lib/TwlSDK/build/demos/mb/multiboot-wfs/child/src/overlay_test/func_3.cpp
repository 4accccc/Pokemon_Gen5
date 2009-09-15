/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - child
  File:     func_3.cpp

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif

#include	"func.h"


/******************************************************************************/
/* local implement */

namespace
{

/* コンストラクタ / デストラクタのテスト用クラス */
    class   Foo
    {
      private:
        const int line_;
      public:
                Foo(int line):line_(line)
        {
            OS_TPrintf("func_3_cpp::Foo::constructor called. (object is at func_3_cpp:%d)\n",
                       line_);
        }
               ~Foo()
        {
            OS_TPrintf("func_3_cpp::Foo::destructor called. (object is at func_3_cpp:%d)\n", line_);
        }
    };

}                                      // unnamed namespace


/******************************************************************************/
/* variable */

static int i = 0;

/*
 * FS_StartOverlay() のタイミングで構築されます.
 * FS_EndOverlay() のタイミングで解体されます.
 */
static Foo foo1(__LINE__);


/******************************************************************************/
/* function */

#if	defined(__cplusplus)
extern  "C"
{
#endif


    void    func_3(char *dst)
    {
        /*
         * 初回呼び出し時のタイミングで構築されます.
         * 構築されていれば, FS_EndOverlay() のタイミングで解体されます.
         */
        static Foo foo2(__LINE__);

              ++i;
                (void)OS_SPrintf(dst, "func_3. called %d time%s.\n", i, (i == 1) ? "" : "s", &i);
    }


#if	defined(__cplusplus)
}                                      /* extern "C" */
#endif


/******************************************************************************/
