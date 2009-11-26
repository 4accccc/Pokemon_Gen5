#pragma once 

/////////////////////////////////////////////////////////////////////////////////// 
/**
 *
 * @brief  カレンダーのエンカウント率データ
 * @author obata
 * @date   2009.08.05
 *
 */
/////////////////////////////////////////////////////////////////////////////////// 

#include <gflib.h>


// 不完全型の宣言
typedef struct _CALENDER_ENCOUNT_INFO CALENDER_ENCOUNT_INFO;


// 出現率番号
enum 
{
  ENCOUNT_NO_0,
  ENCOUNT_NO_1,
  ENCOUNT_NO_2,
  ENCOUNT_NO_3,
  ENCOUNT_NO_4,
  ENCOUNT_NO_MAX
};


//---------------------------------------------------------------------------------
/**
 * @brief エンカウント・データを作成する
 *
 * @param heap_id 使用するヒープID
 *
 * @return 作成したエンカウント・データ
 */
//---------------------------------------------------------------------------------
extern CALENDER_ENCOUNT_INFO* CALENDER_ENCOUNT_INFO_Create( HEAPID heap_id );

//---------------------------------------------------------------------------------
/**
 * @brief 指定日の出現率変化番号を取得する
 *
 * @param p_info データ
 * @parma p_date 日付
 *
 * @return 指定した日付の出現率変化番号( ENCOUNT_NO_x )
 */
//---------------------------------------------------------------------------------
extern u8 CALENDER_ENCOUNT_INFO_GetEncountNo( CALENDER_ENCOUNT_INFO* p_info, const RTCDate* p_date ); 

//---------------------------------------------------------------------------------
/**
 * @brief エンカウント・データを破棄する
 *
 * @param p_info 破棄するデータ
 */ 
//---------------------------------------------------------------------------------
extern void CALENDER_ENCOUNT_INFO_Delete( CALENDER_ENCOUNT_INFO* p_info ); 
