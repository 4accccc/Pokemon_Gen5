/*---------------------------------------------
  Utility program codes for particle demo .
  system.
 
 $Log: ptcl_demo_input.c,v $
 Revision 1.2  2004/12/27 04:32:20  okane_shinji
 (none)

 Revision 1.1  2004/09/15 01:23:09  okane_shinji
 初期登録

 
 *---------------------------------------------*/

#include<nnsys.h>
#include"ptcl_demo.h"

// キー入力
static u16      ptcl_demo_input_Cont, ptcl_demo_input_Trg;

/*-----------------------------------------------------------------
  Name       : PtclDemo_Key( int key_ )
  Description: 引数で指定されていたキーが押されているかどうかを判定します。
  Returns    : キーが押されていればTRUE。そうでなければFALSE
 *-----------------------------------------------------------------*/
BOOL PtclDemo_Key( int key_ )
{
    if( ptcl_demo_input_Cont & key_ ){
        return TRUE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_KeyOneshot( int key_ )
  Description: 引数で指定されていたキーがいま押されたかどうかを判定します。
  Returns    : キーが押されていればTRUE。そうでなければFALSE
 *-----------------------------------------------------------------*/
BOOL PtclDemo_KeyOneshot( int key_ )
{
    if( ptcl_demo_input_Trg & key_ ){
        return TRUE;
    }
    return FALSE;
}

/*-----------------------------------------------------------------
  Name       : PtclDemo_UpdateInput( )
  Description: 入力を更新。毎フレームに一回呼びだされる必要があります。
  Returns    : なし
 *-----------------------------------------------------------------*/
void PtclDemo_UpdateInput(void)
{
    u16 ReadData = PAD_Read();
    ptcl_demo_input_Trg  = (u16)(ReadData & (ReadData ^ ptcl_demo_input_Cont)); // トリガ 入力
    ptcl_demo_input_Cont = ReadData;                                    //   ベタ 入力
}






// end of file