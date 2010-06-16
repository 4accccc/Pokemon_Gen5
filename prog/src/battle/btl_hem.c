//=============================================================================================
/**
 * @file  btl_hem.c
 * @brief ポケモンWB バトルシステム サーバコマンド生成ハンドラの反応処理プロセスマネージャ
 * @author  taya
 *
 * @date  2010.05.28  作成
 */
//=============================================================================================
#include <gflib.h>


#include "btl_server_flow.h"

#include "btl_server_flow_local.h"
#include "btl_hem.h"

//--------------------------------------------------------------------------------------------------------
// HandEx
//--------------------------------------------------------------------------------------------------------
enum {
  PRINT_CHANNEL = 0,  // 0:NoPrint 1:StdPrint 2~:ExPrint
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/




void BTL_Hem_Init( HANDLER_EXHIBISION_MANAGER* wk )
{
  wk->state = 0;
  GFL_STD_MemClear( wk->workBuffer, sizeof(wk->workBuffer) );
}

u32 BTL_Hem_PushState_Impl( HANDLER_EXHIBISION_MANAGER* wk, u32 line )
{
  u32 state = wk->state;

  wk->useItem = ITEM_DUMMY_DATA;
  wk->read_ptr = wk->stack_ptr;
  wk->fSucceed = 0;
  wk->fPrevSucceed = 0;

  BTL_N_PrintfEx( PRINT_CHANNEL_HEMSYS, DBGSTR_HEM_Push, line, wk->stack_ptr, wk->read_ptr );

  return state;
}

u32 BTL_Hem_PushStateUseItem_Impl( HANDLER_EXHIBISION_MANAGER* wk, u16 itemNo, u32 line )
{
  u32 state = wk->state;

  wk->useItem = itemNo;
  wk->read_ptr = wk->stack_ptr;
  wk->fSucceed = 0;
  wk->fPrevSucceed = 0;

  BTL_N_PrintfEx( PRINT_CHANNEL_HEMSYS, DBGSTR_HEM_Push, line, wk->stack_ptr, wk->read_ptr );
  return state;
}

void BTL_Hem_PopState_Impl( HANDLER_EXHIBISION_MANAGER* wk, u32 state, u32 line )
{
  wk->state = state;
  BTL_N_PrintfEx( PRINT_CHANNEL_HEMSYS, DBGSTR_HEM_Pop, line, wk->stack_ptr, wk->read_ptr );
}

BTL_HANDEX_PARAM_HEADER* BTL_Hem_ReadWork( HANDLER_EXHIBISION_MANAGER* wk )
{
  if( wk->read_ptr < wk->stack_ptr )
//  if( (wk->read_ptr==0) && (wk->stack_ptr!=0) )
  {
    BTL_HANDEX_PARAM_HEADER* header = (BTL_HANDEX_PARAM_HEADER*)(&wk->workBuffer[wk->read_ptr]);
    wk->read_ptr += header->size;
//    wk->stack_ptr = 0;

    return header;
  }
  else
  {
    return NULL;
  }
}

BOOL BTL_Hem_IsExistWork( const HANDLER_EXHIBISION_MANAGER* wk )
{
  return ( wk->stack_ptr != 0 );
}

u16 BTL_Hem_GetUseItemNo( const HANDLER_EXHIBISION_MANAGER* wk )
{
  return wk->useItem;
}

BOOL BTL_Hem_IsUsed( const HANDLER_EXHIBISION_MANAGER* wk )
{
  return wk->fUsed;
}
void BTL_Hem_SetResult( HANDLER_EXHIBISION_MANAGER* wk, BOOL fSucceed  )
{
  if( fSucceed ){
    wk->fPrevSucceed = TRUE;
    wk->fSucceed = TRUE;
  }else{
    wk->fPrevSucceed = FALSE;
  }
  wk->fUsed = TRUE;
}
BOOL BTL_Hem_GetPrevResult( const HANDLER_EXHIBISION_MANAGER* wk  )
{
  return wk->fPrevSucceed;
}
BOOL BTL_Hem_GetTotalResult( const HANDLER_EXHIBISION_MANAGER* wk )
{
  return wk->fSucceed;
}

//=============================================================================================
/**
 * ワークメモリ１件確保
 *
 * @param   wk
 * @param   eq_type
 * @param   userPokeID
 *
 * @retval  BTL_HANDEX_PARAM_HEADER*
 */
//=============================================================================================
BTL_HANDEX_PARAM_HEADER* BTL_Hem_PushWork( HANDLER_EXHIBISION_MANAGER* wk, BtlEventHandlerExhibition eq_type, u8 userPokeID )
{
  static const struct {
    u8 eq_type;
    u8 size;
  }work_size_table[] = {
    { BTL_HANDEX_USE_ITEM,             sizeof(BTL_HANDEX_PARAM_USE_ITEM)            },
    { BTL_HANDEX_USE_ITEM_EFFECT,      sizeof(BTL_HANDEX_PARAM_HEADER)              },
    { BTL_HANDEX_RECOVER_HP,           sizeof(BTL_HANDEX_PARAM_RECOVER_HP)          },
    { BTL_HANDEX_DRAIN,                sizeof(BTL_HANDEX_PARAM_DRAIN)               },
    { BTL_HANDEX_SHIFT_HP,             sizeof(BTL_HANDEX_PARAM_SHIFT_HP)            },
    { BTL_HANDEX_RECOVER_PP,           sizeof(BTL_HANDEX_PARAM_PP)                  },
    { BTL_HANDEX_DECREMENT_PP,         sizeof(BTL_HANDEX_PARAM_PP)                  },
    { BTL_HANDEX_CURE_SICK,            sizeof(BTL_HANDEX_PARAM_CURE_SICK)           },
    { BTL_HANDEX_ADD_SICK,             sizeof(BTL_HANDEX_PARAM_ADD_SICK)            },
    { BTL_HANDEX_RANK_EFFECT,          sizeof(BTL_HANDEX_PARAM_RANK_EFFECT)         },
    { BTL_HANDEX_SET_RANK,             sizeof(BTL_HANDEX_PARAM_SET_RANK)            },
    { BTL_HANDEX_RECOVER_RANK,         sizeof(BTL_HANDEX_PARAM_RECOVER_RANK)        },
    { BTL_HANDEX_RESET_RANK,           sizeof(BTL_HANDEX_PARAM_RESET_RANK)          },
    { BTL_HANDEX_SET_STATUS,           sizeof(BTL_HANDEX_PARAM_SET_STATUS)          },
    { BTL_HANDEX_DAMAGE,               sizeof(BTL_HANDEX_PARAM_DAMAGE)              },
    { BTL_HANDEX_KILL,                 sizeof(BTL_HANDEX_PARAM_KILL)                },
    { BTL_HANDEX_CHANGE_TYPE,          sizeof(BTL_HANDEX_PARAM_CHANGE_TYPE)         },
    { BTL_HANDEX_MESSAGE,              sizeof(BTL_HANDEX_PARAM_MESSAGE)             },
    { BTL_HANDEX_TOKWIN_IN,            sizeof(BTL_HANDEX_PARAM_HEADER)              },
    { BTL_HANDEX_TOKWIN_OUT,           sizeof(BTL_HANDEX_PARAM_HEADER)              },
    { BTL_HANDEX_SET_TURNFLAG,         sizeof(BTL_HANDEX_PARAM_TURNFLAG)            },
    { BTL_HANDEX_RESET_TURNFLAG,       sizeof(BTL_HANDEX_PARAM_TURNFLAG)            },
    { BTL_HANDEX_SET_CONTFLAG,         sizeof(BTL_HANDEX_PARAM_SET_CONTFLAG)        },
    { BTL_HANDEX_RESET_CONTFLAG,       sizeof(BTL_HANDEX_PARAM_SET_CONTFLAG)        },
    { BTL_HANDEX_SIDEEFF_ADD,          sizeof(BTL_HANDEX_PARAM_SIDEEFF_ADD)         },
    { BTL_HANDEX_SIDEEFF_REMOVE,       sizeof(BTL_HANDEX_PARAM_SIDEEFF_REMOVE)      },
    { BTL_HANDEX_ADD_FLDEFF,           sizeof(BTL_HANDEX_PARAM_ADD_FLDEFF)          },
    { BTL_HANDEX_CHANGE_WEATHER,       sizeof(BTL_HANDEX_PARAM_CHANGE_WEATHER)      },
    { BTL_HANDEX_REMOVE_FLDEFF,        sizeof(BTL_HANDEX_PARAM_REMOVE_FLDEFF)       },
    { BTL_HANDEX_POSEFF_ADD,           sizeof(BTL_HANDEX_PARAM_POSEFF_ADD)          },
    { BTL_HANDEX_CHANGE_TOKUSEI,       sizeof(BTL_HANDEX_PARAM_CHANGE_TOKUSEI)      },
    { BTL_HANDEX_SET_ITEM,             sizeof(BTL_HANDEX_PARAM_SET_ITEM)            },
    { BTL_HANDEX_CHECK_ITEM_EQUIP,     sizeof(BTL_HANDEX_PARAM_CHECK_ITEM_EQUIP)    },
    { BTL_HANDEX_ITEM_SP,              sizeof(BTL_HANDEX_PARAM_ITEM_SP)             },
    { BTL_HANDEX_CONSUME_ITEM,         sizeof(BTL_HANDEX_PARAM_CONSUME_ITEM)        },
    { BTL_HANDEX_SWAP_ITEM,            sizeof(BTL_HANDEX_PARAM_SWAP_ITEM)           },
    { BTL_HANDEX_UPDATE_WAZA,          sizeof(BTL_HANDEX_PARAM_UPDATE_WAZA)         },
    { BTL_HANDEX_COUNTER,              sizeof(BTL_HANDEX_PARAM_COUNTER)             },
    { BTL_HANDEX_DELAY_WAZADMG,        sizeof(BTL_HANDEX_PARAM_DELAY_WAZADMG)       },
    { BTL_HANDEX_QUIT_BATTLE,          sizeof(BTL_HANDEX_PARAM_HEADER)              },
    { BTL_HANDEX_CHANGE_MEMBER,        sizeof(BTL_HANDEX_PARAM_CHANGE_MEMBER)       },
    { BTL_HANDEX_BATONTOUCH,           sizeof(BTL_HANDEX_PARAM_BATONTOUCH)          },
    { BTL_HANDEX_ADD_SHRINK,           sizeof(BTL_HANDEX_PARAM_ADD_SHRINK)          },
    { BTL_HANDEX_RELIVE,               sizeof(BTL_HANDEX_PARAM_RELIVE)              },
    { BTL_HANDEX_SET_WEIGHT,           sizeof(BTL_HANDEX_PARAM_SET_WEIGHT)          },
    { BTL_HANDEX_PUSHOUT,              sizeof(BTL_HANDEX_PARAM_PUSHOUT)             },
    { BTL_HANDEX_INTR_POKE,            sizeof(BTL_HANDEX_PARAM_INTR_POKE)           },
    { BTL_HANDEX_INTR_WAZA,            sizeof(BTL_HANDEX_PARAM_INTR_WAZA)           },
    { BTL_HANDEX_SEND_LAST,            sizeof(BTL_HANDEX_PARAM_SEND_LAST)           },
    { BTL_HANDEX_SWAP_POKE,            sizeof(BTL_HANDEX_PARAM_SWAP_POKE)           },
    { BTL_HANDEX_HENSIN,               sizeof(BTL_HANDEX_PARAM_HENSIN)              },
    { BTL_HANDEX_FAKE_BREAK,           sizeof(BTL_HANDEX_PARAM_FAKE_BREAK)          },
    { BTL_HANDEX_JURYOKU_CHECK,        sizeof(BTL_HANDEX_PARAM_HEADER)              },
    { BTL_HANDEX_TAMEHIDE_CANCEL,      sizeof(BTL_HANDEX_PARAM_TAMEHIDE_CANCEL)     },
    { BTL_HANDEX_ADD_EFFECT,           sizeof(BTL_HANDEX_PARAM_ADD_EFFECT)          },
    { BTL_HANDEX_CHANGE_FORM,          sizeof(BTL_HANDEX_PARAM_CHANGE_FORM)         },
    { BTL_HANDEX_SET_EFFECT_IDX,       sizeof(BTL_HANDEX_PARAM_SET_EFFECT_IDX)      },
    { BTL_HANDEX_WAZAEFFECT_ENABLE,    sizeof(BTL_HANDEX_PARAM_WAZAEFFECT_ENABLE)   },

  };
  u32 size, i;

  for(i=0, size=0; i<NELEMS(work_size_table); ++i)
  {
    if( work_size_table[i].eq_type == eq_type ){
      size = work_size_table[i].size;
      break;
    }
  }

  if( size )
  {
    while( size % 4 ){ ++size; }
    if( (wk->stack_ptr + size) <= sizeof(wk->workBuffer) )
    {
      BTL_HANDEX_PARAM_HEADER* header = (BTL_HANDEX_PARAM_HEADER*) &(wk->workBuffer[wk->stack_ptr]);

      for(i=0; i<size; ++i){
        wk->workBuffer[ wk->stack_ptr + i ] = 0;
      }
      header->equip = eq_type;
      header->size = size;
      header->userPokeID = userPokeID;
      header->tokwin_flag = 0;
      header->usingFlag = 1;

      wk->stack_ptr += size;
//      wk->read_ptr = wk->stack_ptr;

      BTL_N_PrintfEx( PRINT_CHANNEL_HEMSYS, DBGSTR_HEM_PushWork, eq_type, userPokeID, size, wk->stack_ptr );
      return header;
    }
    else
    {
      GF_ASSERT_MSG(0, "stack over flow ... eq_type=%d, pokeID=%d", eq_type, userPokeID);
    }
  }
  else
  {
    GF_ASSERT_MSG(0, "illegal eq_type = %d", eq_type);
  }
  return NULL;
}
//=============================================================================================
/**
 * ワークメモリ１件返却
 *
 * @param   wk
 * @param   exWork
 */
//=============================================================================================
void BTL_Hem_PopWork( HANDLER_EXHIBISION_MANAGER* wk, void* exWork )
{
  BTL_HANDEX_PARAM_HEADER* header = (BTL_HANDEX_PARAM_HEADER*) exWork;

  if( header->size <= wk->stack_ptr )
  {
    u32 basePos = ((u32)(exWork) - (u32)(wk->workBuffer));
    u32 btm = basePos + header->size;

    if( btm == wk->stack_ptr )
    {
      wk->stack_ptr -= header->size;

      BTL_N_PrintfEx( PRINT_CHANNEL_HEMSYS, DBGSTR_HEM_PophWork,
          header->equip, header->userPokeID, header->size, wk->stack_ptr, basePos );
    }
    else
    {
      GF_ASSERT_MSG(0, "exWork pop error! workSize=%d, endPtr=%d\n", header->size, btm );
    }
  }
  else
  {
    GF_ASSERT_MSG(0, "exWork pop over! workSize=%d, sp=%d\n", header->size, wk->stack_ptr );
  }
}
