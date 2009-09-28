//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：育て屋関連
 * @file   scrcmd_sodateya.c
 * @author obata
 * @date   2009.09.24
 *
 */
//////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "scrcmd_sodateya.h"
#include "savedata/sodateya_work.h"


//--------------------------------------------------------------------
/**
 * @brief 育て屋のポケモンに卵が生まれたかどうかのチェック
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdCheckSodateyaHaveEgg( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA*         gdata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*  scw = GAMEDATA_GetSaveControlWork( gdata );
  SODATEYA_WORK* sodateya = SODATEYA_WORK_GetSodateyaWork( scw );

  *ret_wk = (u16)SODATEYA_WORK_HaveEgg( sodateya );
  OBATA_Printf( "EvCmdCheckSodateyaHaveEgg : %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋のタマゴを受け取る
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaEgg( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMEDATA*         gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  SAVE_CONTROL_WORK*  scw = GAMEDATA_GetSaveControlWork( gdata );
  SODATEYA_WORK* sodateya = SODATEYA_WORK_GetSodateyaWork( scw );

  SODATEYA_WORK_AddEggToParty( sodateya, party ); 
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 育て屋のタマゴを削除する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdDeleteSodateyaEgg( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMEDATA*         gdata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*  scw = GAMEDATA_GetSaveControlWork( gdata );
  SODATEYA_WORK* sodateya = SODATEYA_WORK_GetSodateyaWork( scw );

  SODATEYA_WORK_DeleteEgg( sodateya );
  OBATA_Printf( "EvCmdDeleteSodateyaEgg\n" );
  return VMCMD_RESULT_CONTINUE;
} 

//--------------------------------------------------------------------
/**
 * @breif 育て屋に預けているポケモンの数を取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetSodateyaPokemonNum( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA*         gdata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*  scw = GAMEDATA_GetSaveControlWork( gdata );
  SODATEYA_WORK* sodateya = SODATEYA_WORK_GetSodateyaWork( scw );

  *ret_wk = (u16)SODATEYA_WORK_GetPokemonNum( sodateya );
  OBATA_Printf( "EvCmdGetSodateyaPokemonNum : %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @breif 預けポケモン2体の相性をチェックする
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdSodateyaLoveCheck( VMHANDLE* core, void* wk )
{
  int i;
  u32 monsno[2];
  u32 id_no[2];
  u32 form[2];
  u32 egg_group1[2];
  u32 egg_group2[2];
  POKEMON_PASO_PARAM* paso_param[2];
  POKEMON_PERSONAL_DATA* personal_data[2];
  SCRCMD_WORK*          work = (SCRCMD_WORK*)wk;
  u16*                ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA*            gdata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK*     scw = GAMEDATA_GetSaveControlWork( gdata );
  SODATEYA_WORK*    sodateya = SODATEYA_WORK_GetSodateyaWork( scw );
  HEAPID             heap_id = SCRCMD_WORK_GetHeapID( work );

  // 2体それぞれのパラメータを取得
  paso_param[0] = SODATEYA_WORK_GetPokemonPasoParam( sodateya, SODATE_POKE_FIRST );
  paso_param[1] = SODATEYA_WORK_GetPokemonPasoParam( sodateya, SODATE_POKE_SECOND );
  for( i=0; i<2; i++ )
  {
    monsno[i] = PPP_Get( paso_param[i], ID_PARA_monsno, NULL );
    id_no[i]  = PPP_Get( paso_param[i], ID_PARA_id_no, NULL );
    form[i]   = PPP_Get( paso_param[i], ID_PARA_form_no, NULL );
    personal_data[i] = POKE_PERSONAL_OpenHandle( monsno[i], form[i], GFL_HEAP_LOWID(heap_id) );
    egg_group1[i] = POKE_PERSONAL_GetParam( personal_data[i], POKEPER_ID_egg_group1 );
    egg_group2[i] = POKE_PERSONAL_GetParam( personal_data[i], POKEPER_ID_egg_group2 );
  }

  // 相性を決定
  // グループが違う ==> 0%
  if( ( egg_group1[0] != egg_group1[1] ) &&
      ( egg_group1[0] != egg_group2[1] ) &&
      ( egg_group2[0] != egg_group1[1] ) &&
      ( egg_group2[0] != egg_group2[1] )  )
  {
    *ret_wk = 0;
  }
  // グループが同じ
  else
  {
    // ポケモンの種類が違う
    if( monsno[0] != monsno[1] )
    {
      // IDが同じ ==> 20%
      if( id_no[0] == id_no[1] )
      {
        *ret_wk = 20;
      }
      // IDが違う ==> 50%
      else
      {
        *ret_wk = 50;
      }
    }
    // ポケモンの種類が同じ
    else
    {
      // IDが同じ ==> 50%
      if( id_no[0] == id_no[1] )
      {
        *ret_wk = 50;
      }
      // IDが違う ==> 70%
      else
      {
        *ret_wk = 70;
      }
    }
  }

  return VMCMD_RESULT_CONTINUE;
}
