//============================================================================================
/**
 * @file  dummy3ds_savedata.c
 * @brief   3DS連動：内部確保領域
 * @date    2011.08.24
 * @author  tamada
 */
//============================================================================================

#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/dummy3ds_savedata.h"

//============================================================================================
//============================================================================================
//----------------------------------------------------------
//----------------------------------------------------------
struct _DUMMY3DS_SAVEDATA {
  u32 onetime_key;      //  ワンタイムパスワード
  u32 flag; //フラグ
  u8  is_played;   //一度でもやったかフラグ
  u8 dummy[231];
};

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief セーブワークサイズ取得
 */
//----------------------------------------------------------
int DUMMY3DS_SAVEDATA_GetWorkSize( void )
{
  return sizeof(DUMMY3DS_SAVEDATA);
}

//----------------------------------------------------------
/**
 * @brief 初期化
 */
//----------------------------------------------------------
void DUMMY3DS_SAVEDATA_Init( DUMMY3DS_SAVEDATA * sv )
{
  GFL_STD_MemClear( sv, DUMMY3DS_SAVEDATA_GetWorkSize() );
}


//----------------------------------------------------------
/**
 * @brief データ取得
 */
//----------------------------------------------------------
u32 DUMMY3DS_SAVEDATA_GetData( const DUMMY3DS_SAVEDATA * sv, DUMMY3DS_SAVEDATA_PARAM param )
{
  switch( param )
  {
  case DUMMY3DS_SAVEDATA_PARAM_IS_PLAYED:
    return sv->is_played;
  case DUMMY3DS_SAVEDATA_PARAM_ONETIME_KEY:
    return sv->onetime_key;
  case DUMMY3DS_SAVEDATA_PARAM_FLAG:
    return sv->flag;
  default:
    GF_ASSERT(0);
    return 0;
  }
}

//----------------------------------------------------------
/**
 * @brief データ設定
 */
//----------------------------------------------------------
void DUMMY3DS_SAVEDATA_SetData( DUMMY3DS_SAVEDATA * sv, DUMMY3DS_SAVEDATA_PARAM param, u32 data )
{
  switch( param )
  {
  case DUMMY3DS_SAVEDATA_PARAM_IS_PLAYED:
    sv->is_played = data;
    break;
  case DUMMY3DS_SAVEDATA_PARAM_ONETIME_KEY:
    sv->onetime_key = data;
    break;
  case DUMMY3DS_SAVEDATA_PARAM_FLAG:
    sv->flag = data;
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------
/**
 * @brief セーブデータ取得
 */
//----------------------------------------------------------
DUMMY3DS_SAVEDATA * SaveData_GetDummy3ds( SAVE_CONTROL_WORK * p_sv )
{
  return SaveControl_DataPtrGet(p_sv, GMDATA_ID_DUMMY3DS);
}
