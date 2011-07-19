//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wb_save_convert.c
 *	@brief  WBセーブのコンバート処理
 *	@author	Toru=Nagihashi
 *	@date		2011.07.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include <backup_system.h>
#include "arc_def.h"
#include "multiboot/wb_save.h"
#include "multiboot/wb_save_convert.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define BIT_ON( arr, idx )  (arr[idx/8] |= 1<<(idx%8) )
#define BIT_OFF( arr, idx )  (arr[idx/8] &= ~(1<<(idx%8)) )
#define BIT_CHECK( arr, idx )  ((arr[idx/8] & 1<<(idx%8) ) != 0)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================



//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
static void Wb_Save_Conv_ConvertOwner( const void *cp_data_adrs, WB_SAVE_CONV_OWNER *p_wk );
static void Wb_Save_Conv_ConvertData( const void *cp_data_adrs, WB_SAVE_CONV_DATA *p_wk );
//----------------------------------------------------------------------------
/**
 *	@brief  オーナーデータをコンバートしてアロケート
 *
 *	@param	const void *cp_data_adrs  データアドレス
 *	@param	heapID  ヒープID
 *
 *	@return オーナーデータ
 */
//-----------------------------------------------------------------------------
WB_SAVE_CONV_OWNER * WB_SAVE_CONV_AllocConvertOwner( const void *cp_data_adrs, HEAPID heapID )
{
  WB_SAVE_CONV_OWNER *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(WB_SAVE_CONV_OWNER) );

  Wb_Save_Conv_ConvertOwner( cp_data_adrs, p_wk );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データをコンバートしてアロケート
 *
 *	@param	const void *cp_data_adrs  データアドレス
 *	@param	heapID  ヒープID 
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
WB_SAVE_CONV_DATA * WB_SAVE_CONV_AllocConvertData( const void *cp_data_adrs, HEAPID heapID )
{
  WB_SAVE_CONV_DATA *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(WB_SAVE_CONV_DATA) );

  Wb_Save_Conv_ConvertData( cp_data_adrs, p_wk );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データすべてをコンバートしてアロケート
 *
 *	@param	const void *cp_data_adrs  データアドレス
 *	@param	heapID  ヒープID 
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
WB_SAVE_CONV_ALL_DATA * WB_SAVE_CONV_AllocConvertAllData( const void *cp_data_adrs, HEAPID heapID )
{
  WB_SAVE_CONV_ALL_DATA *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(WB_SAVE_CONV_ALL_DATA) );

  Wb_Save_Conv_ConvertOwner( cp_data_adrs, &p_wk->owner );
  Wb_Save_Conv_ConvertData( cp_data_adrs, &p_wk->data );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  オーナーデータへコンバート
 *
 *	@param	const void *cp_data_adrs  セーブデータ先頭アドレス
 *	@param	*p_wk   コンバート先データ
 */
//-----------------------------------------------------------------------------
static void Wb_Save_Conv_ConvertOwner( const void *cp_data_adrs, WB_SAVE_CONV_OWNER *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(WB_SAVE_CONV_OWNER) );

  {
    const WB_PLAYER_DATA *cp_player = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_PLAYER_DATA );
    const WB_MYSTATUS *cp_mystatus  = &cp_player->mystatus;

    p_wk->trainerID   = cp_mystatus->id;
    GFL_STD_MemCopy( cp_mystatus->name, p_wk->name, sizeof(STRCODE)*(PERSON_NAME_SIZE+EOM_SIZE) );
    p_wk->sex     = cp_mystatus->sex;
    p_wk->version = cp_mystatus->rom_code;
  }


  OS_TPrintf( "TrainerID=[%d]\n", p_wk->trainerID );
  OS_TPrintf( "name=[%x][%x][%x]\n", p_wk->name[0],p_wk->name[1],p_wk->name[2] );
  OS_TPrintf( "sex=[%d]\n", p_wk->sex );
  OS_TPrintf( "version=[%d]\n", p_wk->version );
}


//----------------------------------------------------------------------------
/**
 *	@brief  データすべてをコンバートしてアロケート
 *
 *	@param	const void *cp_data_adrs  データアドレス
 *	@param	heapID  ヒープID 
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
static void Wb_Save_Conv_ConvertData( const void *cp_data_adrs, WB_SAVE_CONV_DATA *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(WB_SAVE_CONV_DATA) );

  //フラグ変換
  {
    const WB_EVENTWORK *cp_ev = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_EVENT_WORK );
    {
      static const struct
      {
        u32 src_bit;
        u32 dst_bit;
      } sc_convert_flag_tbl[] =
      {
        { SYS_FLAG_GAME_CLEAR,    WB_SAVE_CONV_FLAG_GAME_CLEAR        },
        { SYS_FLAG_FIRST_POKE_GET,WB_SAVE_CONV_FLAG_FIRST_POKE_GET    },
        { SYS_FLAG_CHAMPION_WIN,  WB_SAVE_CONV_FLAG_CHAMPION_WIN      },
        { FV_T01R0102_AWARD_01,   WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_01  },
        { FV_T01R0102_AWARD_02,   WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_02  },
        { FV_T01R0102_TROPHY_01,  WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_01 },
        { FV_T01R0102_TROPHY_02,  WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_02 },
        { FV_T01R0102_TROPHY_03,  WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_03 },
      };

      int i;

      for( i = 0; i < NELEMS(sc_convert_flag_tbl); i++ )
      {
        if( BIT_CHECK( cp_ev->flag, sc_convert_flag_tbl[i].src_bit ) )
        {
          BIT_ON( p_wk->flag, sc_convert_flag_tbl[i].dst_bit );
          OS_TPrintf( "flag[%d->%d]=On\n", sc_convert_flag_tbl[i].src_bit, sc_convert_flag_tbl[i].dst_bit );
        }
        else
        {
          BIT_OFF( p_wk->flag, sc_convert_flag_tbl[i].dst_bit );
          OS_TPrintf( "flag[%d->%d]=Off\n", sc_convert_flag_tbl[i].src_bit, sc_convert_flag_tbl[i].dst_bit );
        }
      }
    }

    //フラグ変換その２（バッジはMISC）
    {
      const WB_MISC *cp_misc = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_MISC );
      static const struct
      {
        u32 src_bit;
        u32 dst_bit;
      } sc_convert_badge_tbl[] =
      {
        { 0,  WB_SAVE_CONV_FLAG_BADGE_01, },
        { 1,  WB_SAVE_CONV_FLAG_BADGE_02, },
        { 2,  WB_SAVE_CONV_FLAG_BADGE_03, },
        { 3,  WB_SAVE_CONV_FLAG_BADGE_04, },
        { 4,  WB_SAVE_CONV_FLAG_BADGE_05, },
        { 5,  WB_SAVE_CONV_FLAG_BADGE_06, },
        { 6,  WB_SAVE_CONV_FLAG_BADGE_07, },
        { 7,  WB_SAVE_CONV_FLAG_BADGE_08, },
      };
      
      int i;

      for( i = 0; i < NELEMS(sc_convert_badge_tbl); i++ )
      {
        if( cp_misc->badge & (1<<sc_convert_badge_tbl[i].src_bit) )
        {
          BIT_ON( p_wk->flag, sc_convert_badge_tbl[i].dst_bit );
          OS_TPrintf( "flag[%d->%d]=On\n", sc_convert_badge_tbl[i].src_bit, sc_convert_badge_tbl[i].dst_bit );
        }
        else
        {
          BIT_OFF( p_wk->flag, sc_convert_badge_tbl[i].dst_bit );
          OS_TPrintf( "flag[%d->%d]=Off\n", sc_convert_badge_tbl[i].src_bit, sc_convert_badge_tbl[i].dst_bit );
        }
      }
    }

    //ワーク変換
    {
      int i;

      static const struct
      {
        u32 src_work;
        u32 dst_work;
      } sc_convert_work_tbl[] =
      {
        { WK_SCENE_C04P01, WB_SAVE_CONV_WORK_N_BATTLE_RAIMON },
      };

      for( i = 0; i < NELEMS( sc_convert_work_tbl ); i++ )
      {
        p_wk->work[ sc_convert_work_tbl[i].dst_work ] = cp_ev->work[ sc_convert_work_tbl[i].src_work - WORKS_TOTAL_MIN ];
        OS_TPrintf( "work[%d->%d]=[%d]\n", sc_convert_work_tbl[i].src_work, sc_convert_work_tbl[i].dst_work, p_wk->work[ sc_convert_work_tbl[i].dst_work ] );
      }
    }
  }

  //ミュージカル
  {
    const WB_MUSICAL_SAVE *cp_musical = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_MUSICAL );
    GFL_STD_MemCopy( cp_musical->itemBit, p_wk->musical_item, 256/8 );
    p_wk->musical_sum_point = cp_musical->sumPoint;
    p_wk->musical_entry_num = cp_musical->entryNum;

    {
      int i;
      OS_TPrintf( "musical item_bit=\n" );
      for( i = 0; i < 256/8; i++ )
      {
        OS_TPrintf( "%x ",p_wk->musical_item[i] );
        if( i % 0x10 == 0x10-1 )
        {
          OS_TPrintf( "\n" );
        }
      }
    }
    OS_TPrintf( "musical point=%d\n",p_wk->musical_sum_point );
    OS_TPrintf( "musical entry=%d\n",p_wk->musical_entry_num );
  }

  //殿堂入り
  {
    const WB_DENDOU_RECORD *cp_dendou = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_MUSICAL );

    GFL_STD_MemCopy( cp_dendou->pokemon, p_wk->pokemon, sizeof(WB_DENDOU_POKEMON_DATA_INSIDE)*TEMOTI_POKEMAX );

    {
      int i;
      for( i = 0; i < TEMOTI_POKEMAX; i++ )
      {
        OS_TPrintf( "dendou poke[%d]\n", i );
        OS_TPrintf( "monsno[%d]\n", p_wk->pokemon[i].monsno );
        OS_TPrintf( "level [%d]\n", p_wk->pokemon[i].level );
        OS_TPrintf( "rand  [%d]\n", p_wk->pokemon[i].personalRandom );
        OS_TPrintf( "waza1 [%d]\n", p_wk->pokemon[i].waza[0] );
        OS_TPrintf( "waza2 [%d]\n", p_wk->pokemon[i].waza[1] );
        OS_TPrintf( "waza3 [%d]\n", p_wk->pokemon[i].waza[2] );
        OS_TPrintf( "waza4 [%d]\n", p_wk->pokemon[i].waza[3] );
      }
    }
  }
}
