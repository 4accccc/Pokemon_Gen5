//============================================================================================
/**
 * @file  kerudhio_normalform.c
 * @brief   ケルディオのわざ内容チェック＆フォルム戻し処理
 * @date    2011.12.21
 * @author  tamada GAMEFREAK inc.
 *
 * @note
 * swanではケルディオに新フォルム「かくごのすがた」（ソードフォルム）が追加されている。
 * 技「しんぴのつるぎ」を持っていない場合、フィールドの出入りでその姿を
 * 「いつものすがた」（ノーマルフォルム）に変更する
 */
//============================================================================================

#include "gflib.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "kerudhio_normalform.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "waza_tool/wazano_def.h"

//--------------------------------------------------------------
/**
 * @brief ケルディオのフォルム変更処理
 * @param gsys
 */
//--------------------------------------------------------------
void KERUDHIO_FORM_ChangeNormal( GAMESYS_WORK * gsys )
{
  POKEPARTY * party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData( gsys ) );
  int max = PokeParty_GetPokeCount(party);
  int poke_count;

  for ( poke_count = 0; poke_count < max; poke_count ++ )
  {
    POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, poke_count );
    u16 monsno = PP_Get(pp, ID_PARA_monsno, NULL);
    if ( PP_Get(pp, ID_PARA_tamago_flag, NULL) == FALSE
        && monsno == MONSNO_KERUDHIO
        && PP_Get(pp, ID_PARA_form_no, NULL) == FORMNO_KERUDHIO_SWORD )
    {//タマゴ以外でケルディオでソードフォルムだったらチェックする
      int waza_count;
      for ( waza_count = 0; waza_count < PTL_WAZA_MAX; waza_count ++ )
      {
        if (PP_Get(pp, ID_PARA_waza1 + waza_count, NULL) == WAZANO_SINPINOTURUGI )
        {
          break;
        }
      }
      if (waza_count == PTL_WAZA_MAX)
      {//「しんぴのつるぎ」が見つからなかったらフォルムをノーマルに戻す
        OS_Printf("kerudhio form change!(%d)\n", poke_count );
        PP_ChangeFormNo( pp, FORMNO_KERUDHIO_NORMAL );
      }
    }

    // ボスポケ潰し
    if(monsno>MONSNO_GENOSEKUTO){
      u8 *buf = (u8*)pp;
      buf[30] = 0xff;   // チェックサムを書き潰す
      buf[31] = 0xff;   // チェックサムを書き潰す
    }
  }
}
