//=============================================================================
/**
 * @file  amprotect.c
 * @brief マジコン対策ライブラリ用ソース
 * @author  2010.06.19 mori / GAME FREAK Inc. 
 */
//=============================================================================
#include <gflib.h>

static const u8 *protect_adr = NULL;
void test_protect(void);

void test_protect(void)
{
  // このコードに意味は無い（!NULLにならないのでOS_Haltは実行されない）
  if(protect_adr!=NULL){
    OS_Halt();
  }
  return;
}
