//============================================================================================
/**
 * @file	test_rand.c
 * @brief	乱数動作テスト用関数
 * @author	ohno
 * @date	2006.12.18
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "gf_standard.h"
#include "test_rand.h"




static void _coinTest(u8* pBuff,char* str,int num)
{
    int ans1=0,ans2=0,ans3=0,i;

    for(i = 0;i < num;i++){
        if(0 == pBuff[i]){
            ans1++;
            pBuff[i]='x';
        }
        else{
            ans2++;
            pBuff[i]='o';
        }
    }
    OS_TPrintf("%sコイン%d回 表=%d 裏=%d \n",str,num,ans1,ans2);
    OS_TPrintf("%s\n",pBuff);
}

static void _gvwTest(u8* pBuff,char* str,int num)
{
    int ans1=0,ans2=0,ans3=0,i;

    for(i = 0;i < num;i++){
        if(0 == pBuff[i]){
            ans1++;
            pBuff[i]='g';
        }
        else if(1 == pBuff[i]){
            ans2++;
            pBuff[i]='v';
        }
        else{
            ans3++;
            pBuff[i]='w';
        }
    }
    OS_TPrintf("%sグーチョキパ%d回 ｇ=%d ｖ=%d ｗ=%d \n",str,num,ans1,ans2,ans3);
    OS_TPrintf("%s\n",pBuff);
}

//------------------------------------------------------------------
// RANDのテスト
//------------------------------------------------------------------

#define _RETRY_NUM (50)

void TEST_RAND_Main(u32 initseed)
{
    int i;
    GFL_STD_RandContext randContext;
    u64 seed;
    u8 buff[_RETRY_NUM+1];
    
    GFL_STD_MTRandInit(initseed);

    for(i = 0;i < _RETRY_NUM;i++){
        buff[i] = GFL_STD_MTRand( 2 );
    }
    _coinTest(buff,"MT",_RETRY_NUM);
    for(i = 0;i < _RETRY_NUM;i++){
        buff[i] = GFL_STD_MTRand( 3 );
    }
    _gvwTest(buff,"MT",_RETRY_NUM);

    // 乱数の種をメルセンヌツイスターから得ます。
    // 固定乱数の場合この値をSAVEしてください
    seed = GFL_STD_MTRand( 0 );
    seed = (seed << 32) + GFL_STD_MTRand( 0 );
    
    GFL_STD_RandInit( &randContext, seed );

    for(i = 0;i < _RETRY_NUM;i++){
        buff[i] = GFL_STD_Rand( &randContext,2 );
    }
    _coinTest(buff,"LCM",_RETRY_NUM);
    for(i = 0;i < _RETRY_NUM;i++){
        buff[i] = GFL_STD_Rand( &randContext,3 );
    }
    _gvwTest(buff,"LCM",_RETRY_NUM);

}


