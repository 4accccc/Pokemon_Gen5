//==============================================================================
/**
 * @file    etc_save.c
 * @brief   第2のMISCセーブ
 * @author  matsuda
 * @date    2010.04.06(火)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/etc_save.h"
#include "savedata/person_state.h"
#include "buflen.h"
#include "print/str_tool.h"


//==============================================================================
//  定数定義
//==============================================================================
///知り合い登録最大数
#define ACQUAINTANCE_ID_MAX     (32)

///マニュアルフラグ用
#define MANUAL_SECTOR_BIT       ( 32 )
#define MANUAL_FLAG_WORK_MAX    ( MANUAL_FLAG_MAX/MANUAL_SECTOR_BIT ) ///< 1bit対応なので

#define ANIMAL_TRAIL_WORK_SIZE  (44)  // けものみちセーブデータサイズ

//-------------------------------------
///	けものみち：テーブルごとのオブジェクトタイプ
//=====================================
static const AT_OBJ_TYPE sc_obj_type_table[ANIMAL_TRAIL_TABLE_NUM] =
{
  AT_OBJ_TYPE_MONSTER,
  AT_OBJ_TYPE_MONSTER,
  AT_OBJ_TYPE_MONSTER,
  AT_OBJ_TYPE_ITEM,
  AT_OBJ_TYPE_ITEM,
  AT_OBJ_TYPE_ITEM,
  AT_OBJ_TYPE_ITEM,
  AT_OBJ_TYPE_HIDE_ITEM,
  AT_OBJ_TYPE_HIDE_ITEM,
  AT_OBJ_TYPE_HIDE_ITEM,
  AT_OBJ_TYPE_HIDE_ITEM,
};

//==============================================================================
//  構造体定義
//==============================================================================

//-------------------------------------
///	獣道セーブデータ
//=====================================
typedef struct {
  u16 trail_data[ANIMAL_TRAIL_NUM];
  u8 now_trail_id; // 現在のけものみち番号
  u8 padding[3];
}ANIMAL_TRAIL_SAVE;

SDK_COMPILER_ASSERT( ANIMAL_TRAIL_WORK_SIZE == sizeof(ANIMAL_TRAIL_SAVE) );

//-------------------------------------
///	獣道データ
//=====================================
typedef struct{
  u16 is_set:1;         // 置かれている
  u16 table_index:4;    // テーブルのインデックス
  u16 data_index:3;     // データテーブルのインデックス
  u16 female_num:7;     // メスかどうか
  u16 pad:1;            // パディング
  u16 dummy;
}ANIMAL_TRAIL_DATA;

//-------------------------------------
///	データ格納用共用体
//=====================================
typedef union{
  ANIMAL_TRAIL_DATA at;
  u16               data;
}ANIMAL_TRAIL_UNION;

struct _ETC_SAVE_WORK{
  u32 acquaintance_trainer_id[ACQUAINTANCE_ID_MAX];
  u32 acquaintance_occ_num;   ///<acquaintance_trainer_idの何番目までデータが入っているか
  
  u32 manual_flag[MANUAL_FLAG_WORK_MAX]; ///< ゲーム内マニュアルの項目を読んだかどうか(1bit単位
  
  STRCODE rival_name[PERSON_NAME_SIZE + EOM_SIZE];    // 16

  PERSON_STATE  person_state;

  ANIMAL_TRAIL_SAVE animal_trail;

  u8 dummy[72-PERSON_STATE_WORK_SIZE - ANIMAL_TRAIL_WORK_SIZE];    ///<将来用に1ブロック分のアライメント全てを確保
};


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void GetTrailData( const ANIMAL_TRAIL_SAVE *p_at ,ANIMAL_TRAIL_DATA *p_trail ,u8 index);
static void SetTrailData( ANIMAL_TRAIL_SAVE *p_at ,const ANIMAL_TRAIL_DATA *p_trail ,u8 index);

//==============================================================================
//
//  
//
//==============================================================================
//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int   
 */
//------------------------------------------------------------------
u32 EtcSave_GetWorkSize( void )
{
  return sizeof(ETC_SAVE_WORK);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec   
 */
//--------------------------------------------------------------
void EtcSave_WorkInit(void *work)
{
  ETC_SAVE_WORK *intsave = work;
  
  GFL_STD_MemClear(intsave, sizeof(ETC_SAVE_WORK));
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 未分類データその２のセーブデータポインタ取得
 *
 * @param   p_sv    
 *
 * @retval  ETC_SAVE_WORK *   
 */
//==================================================================
ETC_SAVE_WORK * SaveData_GetEtc( SAVE_CONTROL_WORK * p_sv )
{
  return SaveControl_DataPtrGet(p_sv, GMDATA_ID_ETC);
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * ユニオン知り合いグループとして登録します
 *
 * @param   etcsave   
 * @param   trainer_id    トレーナーID
 */
//==================================================================
void EtcSave_SetAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id)
{
  int i, copy_num;
  
//【ユニオン友達リストの追加が排他的になっていない】
// 同じ人とユニオン接続し続けていても３２人に到達してしまいます。
#ifdef BUGFIX_AF_GFGTS2032_20101028
  // ユニオン友達リストは必ず32以下のはず
  GF_ASSERT( etcsave->acquaintance_occ_num<=ACQUAINTANCE_ID_MAX );
  
  for(i = 0; i < etcsave->acquaintance_occ_num; i++){
    if(etcsave->acquaintance_trainer_id[i] == trainer_id){

      //既に登録済みの人なので最後尾に付け直す(寿命更新)
      copy_num = (etcsave->acquaintance_occ_num - 1 - i);
      if(copy_num <= 0){
        return; //既に最後尾
      }
      GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[i + 1], 
        &etcsave->acquaintance_trainer_id[i], sizeof(u32) * copy_num);
      etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num - 1] = trainer_id;
      return;
    }
  }
  
  //新規登録
  if(etcsave->acquaintance_occ_num < ACQUAINTANCE_ID_MAX){
    etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num] = trainer_id;
    etcsave->acquaintance_occ_num++;
  }
  else{
    GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[1], 
      etcsave->acquaintance_trainer_id, sizeof(u32) * (ACQUAINTANCE_ID_MAX - 1));
    etcsave->acquaintance_trainer_id[ACQUAINTANCE_ID_MAX - 1] = trainer_id;
    etcsave->acquaintance_occ_num = ACQUAINTANCE_ID_MAX; //念のため友達人数を32人と上書きする
  }

#else

  for(i = 0; i < etcsave->acquaintance_occ_num; i++){
    if(etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num] == trainer_id){

      //既に登録済みの人なので最後尾に付け直す(寿命更新)
      copy_num = (etcsave->acquaintance_occ_num - 1 - i);
      if(copy_num <= 0){
        return; //既に最後尾
      }
      GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[i + 1], 
        &etcsave->acquaintance_trainer_id[i], sizeof(u32) * copy_num);
      etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num - 1] = trainer_id;
      return;
    }
  }
  
  //新規登録
  if(etcsave->acquaintance_occ_num < ACQUAINTANCE_ID_MAX){
    etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num] = trainer_id;
    etcsave->acquaintance_occ_num++;
  }
  else{
    GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[1], 
      etcsave->acquaintance_trainer_id, sizeof(u32) * (ACQUAINTANCE_ID_MAX - 1));
    etcsave->acquaintance_trainer_id[ACQUAINTANCE_ID_MAX - 1] = trainer_id;
  }

#endif
}

//==================================================================
/**
 * ユニオン知り合いグループに登録されている人物か調べる
 *
 * @param   etcsave   
 * @param   trainer_id    トレーナーID
 *
 * @retval  BOOL    TRUE:登録されている　　FALSE:登録されていない
 */
//==================================================================
BOOL EtcSave_CheckAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id)
{
  int i;
  for(i = 0; i < etcsave->acquaintance_occ_num; i++){
    if(etcsave->acquaintance_trainer_id[i] == trainer_id){
      return TRUE;
    }
  }
  return FALSE;
}

#ifdef PM_DEBUG
void EtcSave_DebugPrintOutAcuaintance( ETC_SAVE_WORK *etcsave )
{
  int i;
  OS_Printf("Acuaintance...\n");
  for(i=0;i<ACQUAINTANCE_ID_MAX;i++){
    OS_Printf("%02i:%08x\n", i, etcsave->acquaintance_trainer_id[i]);
  }
  OS_Printf("A-count=%d\n",  etcsave->acquaintance_occ_num );
}
#endif

//=============================================================================================
/**
 * @brief マニュアルフラグの取得
 *
 * @param   etcsave   ETC_SAVE_WORK
 * @param   id        フラグ番号（maxはMANUAL_FLAG_MAX(256)
 *
 * @retval  BOOL      TRUE:読んだ FALSE:まだ
 */
//=============================================================================================
BOOL EtcSave_GetManualFlag( const ETC_SAVE_WORK *etcsave, int id )
{
  int sector, bit;
  GF_ASSERT( id < MANUAL_FLAG_MAX );

  sector = id/MANUAL_SECTOR_BIT;
  bit    = id%MANUAL_SECTOR_BIT;

  return (etcsave->manual_flag[sector]>>bit)&1; 

}

//----------------------------------------------------------------------------
/**
 *	@brief  ライバルの名前をセット
 *
 *	@param	ETC_SAVE_WORK *etcsave  セーブデータ
 *	@param	STRCODE *name 名前文字列へのポインタ
 */
//-----------------------------------------------------------------------------
void EtcSave_SetRivalName( ETC_SAVE_WORK *etcsave, const STRCODE *name )
{
  STRTOOL_Copy( name, etcsave->rival_name, PERSON_NAME_SIZE + EOM_SIZE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライバルの名前を取得
 *
 *	@param	const ETC_SAVE_WORK *etcsave セーブデータ
 *
 *	@return STRCODE 名前文字列へのポインタ
 */
//-----------------------------------------------------------------------------
const STRCODE * EtcSave_GetRivalName( const ETC_SAVE_WORK *etcsave )
{
  return etcsave->rival_name;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物状態セーブデータ取得
 *
 *	@param	ETC_SAVE_WORK *etcsave セーブデータ
 *
 *	@return 人物状態セーブデータワーク
 *
 *	@note TODO セーブデータが空き次第、ETCとPERSON_STATEは分離する
 */
//-----------------------------------------------------------------------------
extern void *EtcSave_GetPersonStatePtr( ETC_SAVE_WORK *etcsave )
{
  return (void *)&etcsave->person_state;
}

//=============================================================================================
/**
 * @brief マニュアルフラグを立てる
 *
 * @param   etcsave   ETC_SAVE_WORK
 * @param   id        フラグ番号（maxはMANUAL_FLAG_MAX(256)
 *
 * @retval  BOOL      TRUE:読んだ FALSE:まだ
 */
//=============================================================================================
void EtcSave_SetManualFlag( ETC_SAVE_WORK *etcsave, int id )
{
  int sector, bit;
  
  // フラグは1bit単位で管理
  if(id<MANUAL_FLAG_MAX){

    sector = id/MANUAL_SECTOR_BIT;
    bit    = id%MANUAL_SECTOR_BIT;
  
    etcsave->manual_flag[sector] |= 1<<bit; 

  }
}


//=============================================================================
/**
 *					けものみち：非公開関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  獣道データを取得する
 *
 *	@param	*p_at     獣道セーブデータ
 *	@param	*p_trail  獣道データ
 *	@param	index     獣道ID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetTrailData( const ANIMAL_TRAIL_SAVE *p_at ,ANIMAL_TRAIL_DATA *p_trail ,u8 index)
{
  ANIMAL_TRAIL_UNION at_union;  
  at_union.data = p_at->trail_data[index];
  *p_trail = at_union.at;
}

//----------------------------------------------------------------------------
/**
 *	@brief  獣道データを保存する
 *
 *	@param	*p_at 
 *	@param	*p_trail 
 *	@param	index
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetTrailData( ANIMAL_TRAIL_SAVE *p_at ,const ANIMAL_TRAIL_DATA *p_trail ,u8 index)
{
  ANIMAL_TRAIL_UNION at_union;  
  at_union.at   = *p_trail;
  p_at->trail_data[index] = (u16)at_union.data;
}

//=============================================================================
/**
 *					けものみち：公開関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  配置されているオブジェクトタイプを取得
 *
 *	@param	*p_trail  獣道セーブデータ
 *	@param	index     獣道の番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
AT_OBJ_TYPE ANIMAL_TRAIL_SAVE_GetObjType( const ETC_SAVE_WORK *p_save, u8 index)
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,index);

  if( trail_data.is_set == FALSE )
  {
    return AT_OBJ_TYPE_NONE;
  }

  return sc_obj_type_table[trail_data.table_index];
}

//----------------------------------------------------------------------------
/**
 *	@brief  獣道のメインテーブルインデックスを設定する
 *
 *	@param	*p_save 
 *	@param	trail_id    獣道のID
 *	@param	table_index テーブルインデックス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void ANIMAL_TRAIL_SAVE_SetMainTableIndex( ETC_SAVE_WORK *p_save ,u8 trail_id ,u8 table_index)
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
  trail_data.table_index = table_index;
  SetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
}

//----------------------------------------------------------------------------
/**
 *	@brief  獣道のデータテーブルインデックスを設定する
 *
 *	@param	*p_save 
 *	@param	trail_id    獣道のID
 *	@param	table_index テーブルインデックス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void ANIMAL_TRAIL_SAVE_SetDataTableIndex( ETC_SAVE_WORK *p_save ,u8 trail_id ,u8 table_index)
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
  trail_data.data_index = table_index;
  SetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
}

//----------------------------------------------------------------------------
/**
 *	@brief  けものみちに物が置かれているか設定する
 *
 *	@param	*p_save 
 *	@param	trail_id  獣道のID
 *	@param	is_set    オブジェクトがセットされている = TRUE
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void ANIMAL_TRAIL_SAVE_SetObjectSetFlag( ETC_SAVE_WORK *p_save ,u8 trail_id ,BOOL is_set)
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
  trail_data.is_set = is_set;
  SetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
}

//----------------------------------------------------------------------------
/**
 *	@brief  けものみちに物が置かれているか取得する
 *
 *	@param	*p_save 
 *	@param	trail_id  獣道のID
 *
 *	@retval TRUE = セットされている
 */
//-----------------------------------------------------------------------------
BOOL ANIMAL_TRAIL_SAVE_GetObjectSetFlag( ETC_SAVE_WORK *p_save ,u8 trail_id )
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
  return trail_data.is_set;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メインテーブルインデックスを取得
 *
 *	@param	*p_save 
 *	@param	trail_id  獣道のID
 *
 *	@retval メインテーブルのインデックス
 */
//-----------------------------------------------------------------------------
u8 ANIMAL_TRAIL_SAVE_GetMainTableIndex( ETC_SAVE_WORK *p_save ,u8 trail_id )
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
  return trail_data.table_index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データテーブルインデックスを取得
 *
 *	@param	*p_save 
 *	@param	trail_id  獣道のID
 *
 *	@retval データテーブルのインデックス
 */
//-----------------------------------------------------------------------------
u8 ANIMAL_TRAIL_SAVE_GetDataTableIndex( ETC_SAVE_WORK *p_save ,u8 trail_id )
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
  return trail_data.data_index;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メスになるかどうか判定用パラメータを取得
 *
 *	@param	*p_save 
 *	@param	trail_id  獣道のID
 *
 *	@retval 判定用パラメータ
 */
//-----------------------------------------------------------------------------
u8 ANIMAL_TRAIL_SAVE_GetFemaleNum( ETC_SAVE_WORK *p_save ,u8 trail_id )
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
  return trail_data.female_num;
}

//----------------------------------------------------------------------------
/**
 *	@brief  けものみちに物が置かれているか設定する
 *
 *	@param	*p_save 
 *	@param	trail_id  獣道のID
 *	@param	is_set    オブジェクトがセットされている = TRUE
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void ANIMAL_TRAIL_SAVE_SetFemaleNum( ETC_SAVE_WORK *p_save ,u8 trail_id ,u8 female_num)
{
  ANIMAL_TRAIL_DATA trail_data;
  GetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
  trail_data.female_num = female_num;
  SetTrailData( &p_save->animal_trail ,&trail_data ,trail_id);
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在の獣道IDを取得
 *
 *	@param	*p_save 
 *
 *	@retval 獣道ID
 */
//-----------------------------------------------------------------------------
u8 ANIMAL_TRAIL_SAVE_GetNowAnimalTrailID( ETC_SAVE_WORK *p_save )
{
  return p_save->animal_trail.now_trail_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在の獣道IDを設定
 *
 *	@param	*p_save 
 *	@param	trail_id  獣道ID
 *
 *	@retval 獣道ID
 */
//-----------------------------------------------------------------------------
void ANIMAL_TRAIL_SAVE_SetNowAnimalTrailID( ETC_SAVE_WORK *p_save ,u8 trail_id)
{
  p_save->animal_trail.now_trail_id = trail_id;
}

