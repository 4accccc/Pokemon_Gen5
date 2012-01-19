//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		data_convert_save.c
 *	@brief  データコンバートセーブ
 *	@author	Toru=Nagihashi
 *	@data		2011.07.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/gfl_use.h"
#include "savedata/data_convert_save.h"
#include "savedata/musical_save.h"
#include "print/str_tool.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	データコンバートセーブデータ
//=====================================
struct _DATA_CONVERT_SAVE
{
  WB_SAVE_CONV_OWNER  owner[DATA_CONVERT_OWNER_MAX];
  WB_SAVE_CONV_DATA   data;
};

SDK_COMPILER_ASSERT( DATA_CONVERT_SAVE_WORK_SIZE == sizeof(DATA_CONVERT_SAVE) );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static int Data_Convert_Save_ConvertCore( DATA_CONVERT_SAVE *p_wk, const WB_SAVE_CONV_ALL_DATA *cp_all_data, const SAVE_CONTROL_WORK *cp_sv, BOOL data_in );

//----------------------------------------------------------------------------
/**
 *	@brief  ワークサイズ取得
 *
 *	@return ワークサイズ
 */
//-----------------------------------------------------------------------------
u32  DATA_CONVERT_SAVE_GetWorkSize( void )
{
  return sizeof(DATA_CONVERT_SAVE);
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワーク初期化
 *
 *	@param	DATA_CONVERT_SAVE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void DATA_CONVERT_SAVE_Init( DATA_CONVERT_SAVE *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(DATA_CONVERT_SAVE) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンバート
 *
 *	@param	DATA_CONVERT_SAVE *p_wk ワーク
 *	@param	void *cp_data_adrs      コンバート元データ
 *	@param  cp_sv                   セーブコントロールワーク
 *
 *	@retval TRUE一箇所でもコンバートした  FALSEコンバートしてない
 */
//-----------------------------------------------------------------------------
BOOL DATA_CONVERT_SAVE_Convert( DATA_CONVERT_SAVE *p_wk, const WB_SAVE_CONV_ALL_DATA *cp_all_data, const SAVE_CONTROL_WORK *cp_sv )
{
  return Data_Convert_Save_ConvertCore( p_wk, cp_all_data, cp_sv, TRUE ) != 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンバート元データの人と、既にコンバート済みデータの持ち主が同じか
 *	        もしコンバートを1回もしていなかったらTRUEが帰る
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	void *cp_data_adrs  コンバート元データ
 *
 *	@return TRUE同じ人  FALSE違う人
 */
//-----------------------------------------------------------------------------
BOOL DATA_CONVERT_SAVE_IsSameOwner( const DATA_CONVERT_SAVE *cp_wk, const WB_SAVE_CONV_ALL_DATA *cp_all_data, const SAVE_CONTROL_WORK *cp_sv )
{
  int bit = Data_Convert_Save_ConvertCore( (DATA_CONVERT_SAVE *)cp_wk, cp_all_data, cp_sv, FALSE );
  int i;
  BOOL same_owner = TRUE;


  //
  OS_TPrintf( "▼コンバート情報\n" );
  OS_TPrintf( "トレーナーID=%d\n", cp_all_data->owner.trainerID );
  OS_TPrintf( "性別=%d\n", cp_all_data->owner.sex );
  OS_TPrintf( "バージョン=%d\n", cp_all_data->owner.version );

  for( i = 0; i < DATA_CONVERT_OWNER_MAX; i++ )
  {
    if( bit & (1<<i) )
    {
      if( cp_wk->owner[i].trainerID != cp_all_data->owner.trainerID
        || STRTOOL_Comp( cp_wk->owner[i].name, cp_all_data->owner.name ) == FALSE
        || cp_wk->owner[i].sex != cp_all_data->owner.sex 
        || cp_wk->owner[i].version != cp_all_data->owner.version )
      {
        same_owner = FALSE;
        break;
      }
    }
  }

  return same_owner;
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンバート
 *
 *	@param	DATA_CONVERT_SAVE *p_wk ワーク
 *	@param	void *cp_data_adrs      コンバート元データ
 *	@param  cp_sv                   セーブコントロールワーク
 *	@param  data_in                 データをセーブにいれるかどうか
 *
 *	@retval bit 1<<DATA_CONVERT_OWNERの順番で変更された場合立っているビット
 */
//-----------------------------------------------------------------------------
static int Data_Convert_Save_ConvertCore( DATA_CONVERT_SAVE *p_wk, const WB_SAVE_CONV_ALL_DATA *cp_all_data, const SAVE_CONTROL_WORK *cp_sv, BOOL data_in )
{
  int i;
  const WB_SAVE_CONV_OWNER  *cp_src_owner = &cp_all_data->owner;
  const WB_SAVE_CONV_DATA   *cp_src_data  = &cp_all_data->data;
  WB_SAVE_CONV_DATA         *p_dst_data   = &p_wk->data;
  u32 cnt;

  BOOL is_convert[DATA_CONVERT_OWNER_MAX]  = { FALSE, FALSE, FALSE };

  //フラグの更新
  cnt = 0;
  for( i = 0; i < WB_SAVE_CONV_FLAG_MAX; i++ )
  {
    BOOL flag = (cp_src_data->flag[ i/8 ] & (1<<(i%8)) ) != 0;
    if( flag )
    {
      //自分が０で新しく１になったら変更された
      if( (p_dst_data->flag[ i/8 ] & (1<<(i%8)) ) == 0 )
      {
        is_convert[DATA_CONVERT_OWNER_SYS] = TRUE;
      }

      if( data_in )
      {
        DATA_CONVERT_SAVE_SetFlag( p_wk, i, TRUE );
      }
    }
  }

  //ワークの更新
  for( i = 0; i < WB_SAVE_CONV_WORK_MAX; i++ )
  {
    BOOL flag = TRUE;
    switch( i )
    {
    case WB_SAVE_CONV_WORK_SYS_FIRST_POKETYPE:
    case WB_SAVE_CONV_WORK_FURNITURE_SHOP:
      /*  なし  */
      break;
    case WB_SAVE_CONV_WORK_N_BATTLE_RAIMON:
      flag = cp_src_data->work[i] != 0;
      break;
    default:
      GF_ASSERT(0);
    }


    if( flag && cp_src_data->work[i] != p_dst_data->work[i] )
    {
      //自分が０で新しく１になったら変更された
      if( p_dst_data->work[i] == 0 )
      {
        is_convert[DATA_CONVERT_OWNER_SYS] = TRUE;
      }

      if( data_in )
      {
        DATA_CONVERT_SAVE_SetWork( p_wk, i, cp_src_data->work[i] );
      }
    }
  }

  //ミュージカルデータ
  if( cp_src_data->musical_entry_num > 0 )
  {
    const MUSICAL_SAVE* cp_musical = MUSICAL_SAVE_GetMusicalSave( (SAVE_CONTROL_WORK *)cp_sv );
    if( MUSICAL_SAVE_IsConvertItem( cp_musical ) == FALSE )
    {
      const u8* cp_have_item_bit = MUSICAL_SAVE_GetItemBitAdrs( cp_musical );
      if( GFL_STD_MemComp( cp_src_data->musical_item, cp_have_item_bit, WB_SAVE_CONV_MUSICAL_ITEM_BIT ) != 0 )
      {
        is_convert[DATA_CONVERT_OWNER_MUSICAL] = TRUE;
        if( data_in )
        {
          p_dst_data->musical_sum_point = cp_src_data->musical_sum_point;
          p_dst_data->musical_entry_num = cp_src_data->musical_entry_num;
          GFL_STD_MemCopy( cp_src_data->musical_item, p_dst_data->musical_item, WB_SAVE_CONV_MUSICAL_ITEM_BIT );
        }
      }
    }
  }

  //でんどうデータ
  {
    BOOL is_clear = (cp_src_data->flag[ WB_SAVE_CONV_FLAG_GAME_CLEAR/8 ] & (1<<(WB_SAVE_CONV_FLAG_GAME_CLEAR%8)) ) != 0;
    if( is_clear )
    {
      if( GFL_STD_MemComp( cp_src_data->pokemon, p_dst_data->pokemon, sizeof(WB_SAVE_CONV_DENDOU_POKEMON_DATA)*TEMOTI_POKEMAX ) != 0 )
      {
        if( data_in )
        {
          GFL_STD_MemCopy( cp_src_data->pokemon, p_dst_data->pokemon, sizeof(WB_SAVE_CONV_DENDOU_POKEMON_DATA)*TEMOTI_POKEMAX );
        }
        is_convert[DATA_CONVERT_OWNER_DENDOU] = TRUE;
      }
    }
  }

  //変更があった箇所のオーナーをセーブ
  if( data_in )
  {
    for( i = 0; i < DATA_CONVERT_OWNER_MAX; i++ )
    {
      if( is_convert[i] )
      {
        p_wk->owner[i]  = *cp_src_owner;
        p_wk->owner[i].is_exist = TRUE;
      }
    }
  }

  OS_TPrintf( "▼変更があったか\n" );
  OS_TPrintf( "SYSTEM   %d\n", is_convert[DATA_CONVERT_OWNER_SYS] );
  OS_TPrintf( "MUSICAL  %d\n", is_convert[DATA_CONVERT_OWNER_MUSICAL] );
  OS_TPrintf( "DENDOU   %d\n", is_convert[DATA_CONVERT_OWNER_DENDOU] );

  return (is_convert[DATA_CONVERT_OWNER_SYS]<<DATA_CONVERT_OWNER_SYS)
    | (is_convert[DATA_CONVERT_OWNER_MUSICAL]<<DATA_CONVERT_OWNER_MUSICAL)
    | (is_convert[DATA_CONVERT_OWNER_DENDOU]<<DATA_CONVERT_OWNER_DENDOU);
}


//----------------------------------------------------------------------------
/**
 *	@brief  データを取得
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	id            受け取りID
 *	@param	*p_data_adrs  データ受取りアドレス
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 DATA_CONVERT_SAVE_GetData( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_GET_ID id, void *p_data_adrs )
{
  switch( id )
  {
  case DATA_CONVERT_GET_ID_MUSICAL_POINT:
    return cp_wk->data.musical_sum_point;
  case DATA_CONVERT_GET_ID_MUSICAL_ENTRY:
    return cp_wk->data.musical_entry_num;
  case DATA_CONVERT_GET_ID_DENDOU_POKE_00: //p_data_adrsからWB_SAVE_CONV_DENDOU_POKEMON_DATA構造体で受け取る
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_01: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_02: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_03: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_04: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_05: //ゞ
    {
      u16 idx = id - DATA_CONVERT_GET_ID_DENDOU_POKE_00;
      GFL_STD_MemCopy( &cp_wk->data.pokemon[idx], p_data_adrs, sizeof(WB_SAVE_CONV_DENDOU_POKEMON_DATA) );
    }
    return 0;

  case DATA_CONVERT_GET_ID_OWNER_SYS:  //p_data_adrsからWB_SAVE_CONV_OWNER構造体で受け取る
  case DATA_CONVERT_GET_ID_OWNER_MUSICAL:
  case DATA_CONVERT_GET_ID_OWNER_DENDOU:
    if( p_data_adrs )
    {
      DATA_CONVERT_OWNER idx = id - DATA_CONVERT_GET_ID_OWNER_SYS;
      GFL_STD_MemCopy( &cp_wk->owner[idx], p_data_adrs, sizeof(WB_SAVE_CONV_OWNER) );
    }
    return (u32)&cp_wk->owner;
  case DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT:  //p_data_adrsからu8のWB_SAVE_CONV_MUSICAL_ITEM_BIT配列で受け取る
    if( p_data_adrs )
    {
      GFL_STD_MemCopy( cp_wk->data.musical_item, p_data_adrs, sizeof(u8)*WB_SAVE_CONV_MUSICAL_ITEM_BIT);
    }
    return (u32)cp_wk->data.musical_item;
  default:
    GF_ASSERT(0);
  }

  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フラグを取得
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	flag  チェックするフラグ
 *
 *	@return TRUEならばフラグON  FALSEならばフラグOFF
 */
//-----------------------------------------------------------------------------
BOOL DATA_CONVERT_SAVE_GetFlag( const DATA_CONVERT_SAVE *cp_wk, WB_SAVE_CONV_FLAG flag )
{
  GF_ASSERT( flag < WB_SAVE_CONV_FLAG_MAX );
  return (cp_wk->data.flag[ flag/8 ] & (1<<(flag%8)) ) != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワークの値をチェック
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	wk  調べるワーク
 *
 *	@return ワークの値
 */
//-----------------------------------------------------------------------------
u16 DATA_CONVERT_SAVE_GetWork( const DATA_CONVERT_SAVE *cp_wk, WB_SAVE_CONV_WORK wk )
{
  GF_ASSERT( wk < WB_SAVE_CONV_WORK_MAX );
  return cp_wk->data.work[ wk ];
}
//----------------------------------------------------------------------------
/**
 *	@brief  IDごとのコンバートデータがあるかどうか
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	id  ID
 *
 *	@return TRUEなら存在  FALSEならなし
 */
//-----------------------------------------------------------------------------
BOOL DATA_CONVERT_SAVE_IsExist( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_OWNER id )
{
  GF_ASSERT( id < DATA_CONVERT_OWNER_MAX );
  return cp_wk->owner[id].is_exist;
}
//----------------------------------------------------------------------------
/**
 *	@brief  性別を取得
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	id  ID
 *
 *	@return 性別
 */
//-----------------------------------------------------------------------------
u8 DATA_CONVERT_SAVE_GetSex( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_OWNER id )
{
  GF_ASSERT( id < DATA_CONVERT_OWNER_MAX );
  return cp_wk->owner[id].sex;
}
//----------------------------------------------------------------------------
/**
 *	@brief  名前を取得
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	id  ID
 *
 *	@return STRCODEのポインタ
 */
//-----------------------------------------------------------------------------
const STRCODE * DATA_CONVERT_SAVE_GetName( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_OWNER id )
{
  GF_ASSERT( id < DATA_CONVERT_OWNER_MAX );
  return cp_wk->owner[id].name;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ設定
 *
 *	@param	DATA_CONVERT_SAVE *p_wk ワーク
 *	@param	id    ID
 *	@param	data  データ
 */
//-----------------------------------------------------------------------------
void DATA_CONVERT_SAVE_SetData( DATA_CONVERT_SAVE *p_wk, DATA_CONVERT_GET_ID id, u32 data )
{
  switch( id )
  {
  case DATA_CONVERT_GET_ID_MUSICAL_POINT:
    p_wk->data.musical_sum_point  = data;
    break;
  case DATA_CONVERT_GET_ID_MUSICAL_ENTRY:
    p_wk->data.musical_entry_num  = data;
    break;
  case DATA_CONVERT_GET_ID_DENDOU_POKE_00: //p_data_adrsからWB_SAVE_CONV_DENDOU_POKEMON_DATA構造体で受け取る
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_01: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_02: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_03: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_04: //ゞ
    /* fall though */
  case DATA_CONVERT_GET_ID_DENDOU_POKE_05: //ゞ
    {
      u16 idx = id - DATA_CONVERT_GET_ID_DENDOU_POKE_00;
      GFL_STD_MemCopy( (void*)data, &p_wk->data.pokemon[idx], sizeof(WB_SAVE_CONV_DENDOU_POKEMON_DATA) );
    }
    break;
  case DATA_CONVERT_GET_ID_OWNER_SYS:  //p_data_adrsからWB_SAVE_CONV_OWNER構造体で受け取る
  case DATA_CONVERT_GET_ID_OWNER_MUSICAL:
  case DATA_CONVERT_GET_ID_OWNER_DENDOU:
    {
      DATA_CONVERT_OWNER idx = id - DATA_CONVERT_GET_ID_OWNER_SYS;
      GFL_STD_MemCopy( (void*)data, &p_wk->owner[idx], sizeof(WB_SAVE_CONV_OWNER) );
    }
    break;
  case DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT:  //p_data_adrsからu8のWB_SAVE_CONV_MUSICAL_ITEM_BIT配列で受け取る
    GFL_STD_MemCopy( (void*)data, p_wk->data.musical_item, sizeof(u8)*WB_SAVE_CONV_MUSICAL_ITEM_BIT);
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フラグを設定
 *
 *	@param	DATA_CONVERT_SAVE *p_wk ワーク
 *	@param	flag  フラグ
 *	@param	on_off フラグをON,OFFするかどうか
 */
//-----------------------------------------------------------------------------
void DATA_CONVERT_SAVE_SetFlag( DATA_CONVERT_SAVE *p_wk, WB_SAVE_CONV_FLAG flag, BOOL on_off )
{
  GF_ASSERT( flag < WB_SAVE_CONV_FLAG_MAX );

  if( on_off )
  {
    p_wk->data.flag[ flag/8 ] |= (1<<(flag%8));
  }
  else
  {
    p_wk->data.flag[ flag/8 ] &= ~(1<<(flag%8));
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワークに値を設定
 *
 *	@param	DATA_CONVERT_SAVE *p_wk ワーク
 *	@param	wk    ワーク番号
 *	@param	value 値
 */
//-----------------------------------------------------------------------------
void DATA_CONVERT_SAVE_SetWork( DATA_CONVERT_SAVE *p_wk, WB_SAVE_CONV_WORK wk, u16 value )
{
  GF_ASSERT( wk < WB_SAVE_CONV_WORK_MAX );
  p_wk->data.work[ wk ]  = value;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	const DATA_CONVERT_SAVE *cp_wk  ワーク
 *	@param	index イベント番号
 *
 *	@return TRUEオープン中  FALSEオープンしていない
 */
//-----------------------------------------------------------------------------
BOOL DATA_CONVERT_SAVE_IsEventOpen( const DATA_CONVERT_SAVE *cp_wk, u32 index )
{
  typedef BOOL (*GET_FLAG_FUNCTION)( const DATA_CONVERT_SAVE *cp_wk, int param );
  static const struct
  {
    int   param;
    GET_FLAG_FUNCTION GetFlagFunction;
  }sc_list_flag[] =
  {
    {
      WB_SAVE_CONV_FLAG_BADGE_01,
      (GET_FLAG_FUNCTION)DATA_CONVERT_SAVE_GetFlag,
    },
    {
      WB_SAVE_CONV_FLAG_BADGE_06,
      (GET_FLAG_FUNCTION)DATA_CONVERT_SAVE_GetFlag,
    },
    {
      WB_SAVE_CONV_FLAG_GAME_CLEAR,
      (GET_FLAG_FUNCTION)DATA_CONVERT_SAVE_GetFlag,
    },
    {
      WB_SAVE_CONV_FLAG_BADGE_08,
      (GET_FLAG_FUNCTION)DATA_CONVERT_SAVE_GetFlag,
    },
    {
      WB_SAVE_CONV_FLAG_BADGE_05,
      (GET_FLAG_FUNCTION)DATA_CONVERT_SAVE_GetFlag,
    },
    {
      WB_SAVE_CONV_FLAG_GAME_CLEAR,
      (GET_FLAG_FUNCTION)DATA_CONVERT_SAVE_GetFlag,
    },
    {
      DATA_CONVERT_OWNER_MUSICAL,
      (GET_FLAG_FUNCTION)DATA_CONVERT_SAVE_IsExist,
    },
    {
      WB_SAVE_CONV_FLAG_GAME_CLEAR,
      (GET_FLAG_FUNCTION)DATA_CONVERT_SAVE_GetFlag,
    }
  };
  GF_ASSERT( index < NELEMS(sc_list_flag) );

  return sc_list_flag[index].GetFlagFunction( cp_wk, sc_list_flag[index].param );
}
//----------------------------------------------------------------------------
/**
 *	@brief  イベント番号
 *
 *	@param	u32 index   番号
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
u32 DATA_CONVERT_SAVE_GetEventWork( u32 index )
{
  static const int sc_work[]  =
  {
    WK_SYS_LINK_EVENT_01,
    WK_SYS_LINK_EVENT_02,
    WK_SYS_LINK_EVENT_03,
    WK_SYS_LINK_EVENT_04,
    WK_SYS_LINK_EVENT_05,
    WK_SYS_LINK_EVENT_06,
    WK_SYS_LINK_EVENT_07,
    WK_SYS_LINK_EVENT_08,
  };
  GF_ASSERT( index < NELEMS(sc_work) );

  return sc_work[ index ];
}
