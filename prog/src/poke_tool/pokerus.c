//============================================================================================
/**
 * @file    pokerus.c
 * @bfief   ポケルス関連関数群
 * @author  HisashiSogabe
 * @date    09.12.02
 */
//============================================================================================
#include <gflib.h>

#include "poke_tool/pokerus.h"

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  inline  u32 No2Bit( u32 no );

//==============================================================================
/**
 *	ポケモンにポケルスが感染するかチェック
 *
 * @param	ppt		POKEPARTY構造体へのポインタ
 *
 * @retval  none		
 */
//==============================================================================
void	POKERUS_CheckCatchPokerus( POKEPARTY *ppt )
{
	POKEMON_PARAM	*pp;
	u16				rnd;
	u8				pokerus;
	int				count;
	int				pos;
  u8        infect;

	count = PokeParty_GetPokeCount( ppt );
	rnd = GFL_STD_MtRand( 0x10000 );
	infect = POKERUS_CheckInfected( ppt );

	if( ( rnd == 16384 ) || ( rnd == 32768 ) || ( rnd == 49152 ) )
  {
		do{
			pos = GFL_STD_MtRand( count );
			pp = PokeParty_GetMemberPointer( ppt, pos );
			if( ( PP_Get( pp, ID_PARA_monsno, NULL ) ) && ( PP_Get( pp, ID_PARA_tamago_flag, NULL ) == 0 ) )
      {
				break;
			}
			else
      {
				pos = count;
			}
		}while( pos == count );
		if( ( infect & No2Bit( pos ) ) == 0 )
    {
			do{
				pokerus = GFL_STD_MtRand( 0x100 );
			}while( ( pokerus & 0x07 ) == 0 );
			if( pokerus & 0xf0 )
      {
				pokerus &= 0x07;
			}
			pokerus |= ( pokerus << 4 );
			pokerus &= 0xf3;
			pokerus++;
			PP_Put( pp, ID_PARA_pokerus, pokerus );
		}
	}
}

//============================================================================================
/**
 *	ポケルスの日数カウンターをデクリメントする
 *
 * @param[in]	ppt		チェックするPOKEPARTY構造体のポインタ
 * @param[in]	day		経過日数
 *
 * @retval	none
 */
//============================================================================================
void	POKERUS_DecCounter( POKEPARTY *ppt, s32 day )
{
	POKEMON_PARAM	*pp;
	int	pos;
	int	count;
	u8	pokerus;

	count = PokeParty_GetPokeCount( ppt );

	for( pos = 0 ; pos < count ; pos++ )
  {
		pp = PokeParty_GetMemberPointer( ppt, pos );
		if( PP_Get( pp, ID_PARA_monsno, NULL ) )
    {
			pokerus = PP_Get( pp, ID_PARA_pokerus, NULL );
			if( pokerus & 0x0f )
      {
				if( ( ( pokerus & 0x0f ) < day ) || ( day > 4 ) )
        {
					pokerus &= 0xf0;
				}
				else
        {
					pokerus -= day;
				}
				if( pokerus == 0 )
        {
					pokerus = 0x10;
				}
				PP_Put( pp, ID_PARA_pokerus, pokerus );
			}
		}
	}
}

//============================================================================================
/**
 *	ポケルスの伝染チェックルーチン（ポケモンからポケモンへの感染）
 *
 * @param[in]	ppt	伝染をチェックするPOKEPARTY構造体のポインタ
 */
//============================================================================================
void	POKERUS_CheckContagion( POKEPARTY *ppt )
{
	POKEMON_PARAM	*pp;
	int	pos;
	int	count;
	u8	pokerus;

	count = PokeParty_GetPokeCount( ppt );

	if( GFL_STD_MtRand( 3 ) == 0 )
  { 
		for( pos = 0 ; pos < count ; pos++ )
    {
			pp = PokeParty_GetMemberPointer( ppt, pos );
			if( PP_Get( pp, ID_PARA_monsno, NULL ) )
      {
				pokerus = PP_Get( pp, ID_PARA_pokerus, NULL );
				if( pokerus & 0x0f )
        {
					if( pos != 0 )
          {
						pp = PokeParty_GetMemberPointer( ppt, pos - 1 );
						if( ( PP_Get( pp, ID_PARA_pokerus, NULL ) & 0xf0 ) == 0 )
            {
							PP_Put( pp, ID_PARA_pokerus, pokerus );
						}
					}
					if( pos < count - 1 )
          {
						pp = PokeParty_GetMemberPointer( ppt, pos + 1 );
						if( ( PP_Get( pp, ID_PARA_pokerus, NULL ) & 0xf0 ) == 0 )
            {
							PP_Put( pp, ID_PARA_pokerus, pokerus );
							pos++;
						}
					}
				}
			}
		}
	}
}

//============================================================================================
/**
 *	ポケモンがポケルスに感染しているかチェック
 *
 * @param[in]	ppt 感染をチェックするPOKEPARTY構造体のポインタ
 *
 * @retval	感染しているポケモンをビットで返す
 */
//============================================================================================
u8	POKERUS_CheckInfect( POKEPARTY *ppt )
{
	POKEMON_PARAM	*pp;
	int	pos;
	int	count;
	u8	infect = 0;

	count = PokeParty_GetPokeCount( ppt );

	for( pos = 0 ; pos < count ; pos++)
  {
		pp = PokeParty_GetMemberPointer( ppt, pos );
		if( POKERUS_CheckInfectPP( pp ) )
    { 
			infect |= No2Bit( pos );
		}
	}

	return infect;
}

//============================================================================================
/**
 *	ポケモンがポケルスに感染しているかチェック（過去に感染していたことも込み）
 *
 * @param[in]	ppt 感染をチェックするPOKEPARTY構造体のポインタ
 *
 * @retval	感染しているポケモンをビットで返す
 */
//============================================================================================
u8	POKERUS_CheckInfected( POKEPARTY *ppt )
{
	POKEMON_PARAM	*pp;
	int	pos;
	int	count;
	u8	infect = 0;

	count = PokeParty_GetPokeCount( ppt );

	for( pos = 0 ; pos < count ; pos++)
  {
		pp = PokeParty_GetMemberPointer( ppt, pos );
		if( PP_Get( pp, ID_PARA_pokerus, NULL ) )
    { 
			infect |= No2Bit( pos );
		}
	}

	return infect;
}

//============================================================================================
/**
 *	ポケルスに感染しているかチェック
 *
 * @param[in]	pp	感染をチェックするPOKEMON_PARAM構造体のポインタ
 *
 * @retval FALSE:感染していない　TRUE:感染している
 */
//============================================================================================
BOOL	POKERUS_CheckInfectPP( POKEMON_PARAM *pp )
{
	return POKERUS_CheckInfectPPP( PP_GetPPPPointer( pp ) );
}

//============================================================================================
/**
 *	ポケルスに感染しているかチェック
 *
 * @param[in]	ppp	感染をチェックするPOKEMON_PASO_PARAM構造体のポインタ
 *
 * @retval FALSE:感染していない　TRUE:感染している
 */
//============================================================================================
BOOL	POKERUS_CheckInfectPPP( POKEMON_PASO_PARAM *ppp )
{
	return ( ( PPP_Get( ppp, ID_PARA_pokerus, NULL ) & 0x0f ) != 0 );
}

//============================================================================================
/**
 *	過去にポケルスに感染していたかチェック
 *
 * @param[in]	pp	感染をチェックするPOKEMON_PARAM構造体のポインタ
 *
 * @retval FALSE:感染していない　TRUE:感染していた
 */
//============================================================================================
BOOL	POKERUS_CheckInfectedPP( POKEMON_PARAM *pp )
{ 
	return POKERUS_CheckInfectedPPP( PP_GetPPPPointer( pp ) );
}

//============================================================================================
/**
 *	過去にポケルスに感染していたかチェック
 *
 * @param[in]	ppp	感染をチェックするPOKEMON_PASO_PARAM構造体のポインタ
 *
 * @retval FALSE:感染していない　TRUE:感染していた
 */
//============================================================================================
BOOL	POKERUS_CheckInfectedPPP( POKEMON_PASO_PARAM *ppp )
{
	u8	pokerus;

	pokerus = PPP_Get( ppp, ID_PARA_pokerus, NULL );
	
	//今、かかっている場合は、FALSE
	if( pokerus & 0x0f )
  {
		return FALSE;
	}

	return ( ( pokerus & 0xf0 ) != 0 );
}

//============================================================================================
/**
 *	ナンバーをビットに変換
 */
//============================================================================================
static  inline  u32 No2Bit( u32 no )
{ 
  GF_ASSERT( no < 32 );

  return ( 1 << ( no ) );
}

