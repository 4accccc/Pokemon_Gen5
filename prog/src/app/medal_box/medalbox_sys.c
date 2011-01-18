
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medalbox_sys.c
 *	@brief  メダルボックスメイン
 *	@author	Masayuki Onoue
 *	@data		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "app/medalbox_sys.h"
#include "medalbox_sys_local.h"

//============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GFL_PROC_RESULT MEDALBOXSYS_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MEDALBOXSYS_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MEDALBOXSYS_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA medalBox_ProcData =
{
  MEDALBOXSYS_ProcInit,
  MEDALBOXSYS_ProcMain,
  MEDALBOXSYS_ProcTerm
};


static GFL_PROC_RESULT MEDALBOXSYS_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MEDALBOXSYS_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MEDALBOXSYS_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  return GFL_PROC_RES_FINISH;
}

