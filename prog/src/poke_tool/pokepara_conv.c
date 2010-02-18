//============================================================================================
/**
 * @file  pokepara_conv.c
 * @bfief ポケモンパラメータコンバータツール群
 * @author  HisashiSogabe
 * @date  10.02.18
 */
//============================================================================================
#include    <gflib.h>

#include    "poke_tool/poke_tool.h"

//============================================================================================
/**
 * @brief   ポケシフター経由でシャチに持ってきたポケモンパラメータをコンバート
 *
 * @param ppp PokePasoParam
 */
//============================================================================================
void  PPCONV_ConvertPPPFromPokeShifter( POKEMON_PASO_PARAM* ppp )
{ 
  //性格を個性乱数から導いてワークに代入
  { 
    u8 seikaku = PPP_Get( ppp, ID_PARA_personal_rnd, NULL ) % 25;
    
    PPP_Put( ppp, ID_PARA_seikaku, seikaku );
  }

  //HGSSの捕獲ボールをシャチの捕獲ボール領域にコピー
  { 
    u8 get_cassette = PPP_Get( ppp, ID_PARA_get_cassette, NULL );
    if( ( get_cassette == VERSION_GOLD ) || ( get_cassette == VERSION_SILVER ) )
    { 
      u8  get_ball = PPP_Get( ppp, ID_PARA_dummy_p4_1, NULL ) & 0xff;
      PPP_Put( ppp, ID_PARA_get_ball, get_ball );
    }
  }

  //シャチになって未使用となった領域を０クリア
  PPP_Put( ppp, ID_PARA_dummy_p2_3, 0 );  //プラチナ new_get_place new_birth_place
  PPP_Put( ppp, ID_PARA_dummy_p4_1, 0 );  //HGSS get_ball humor
}
