//============================================================================================
/**
 * @file  straw_poke_form.c
 * @bfief 蓑ポケモン（ミノムッチ）フォールムチェンジ
 * @author  Saito
 */
//============================================================================================
#include <gflib.h>

#include "straw_poke_form.h"
#include "poke_tool/monsno_def.h"  // for MONSNO_モンスター名
#include "savedata/zukan_savedata.h"

#include "../../resource/battle/batt_bg_tbl/batt_bg_tbl.h"

#define BATT_BG_OBONID_MAX  (25)

const u16 StrawFromTbl[BATT_BG_OBONID_MAX][2] = {
  { BATT_BG_OBONID_00,FORMNO_412_SUNA},		    //地面（四季なし）            砂
  { BATT_BG_OBONID_01,FORMNO_412_SUNA},				//地面（四季あり）            砂
  { BATT_BG_OBONID_02,FORMNO_412_GOMI},				//床（屋外）                  ゴミ
	{ BATT_BG_OBONID_03,FORMNO_412_GOMI},				//床（屋内）                  ゴミ
	{ BATT_BG_OBONID_04,FORMNO_412_SUNA},				//洞窟                        砂
	{ BATT_BG_OBONID_05,FORMNO_412_KUSA},				//芝生（四季なし）            草
	{ BATT_BG_OBONID_06,FORMNO_412_KUSA},				//芝生（四季あり）            草
	{ BATT_BG_OBONID_07,FORMNO_412_SUNA},				//山肌                        砂
	{ BATT_BG_OBONID_08,FORMNO_412_KUSA},				//水上（屋外）                草
	{ BATT_BG_OBONID_09,FORMNO_412_KUSA},				//水上（屋内）                草
	{ BATT_BG_OBONID_10,FORMNO_412_SUNA},				//砂漠（屋外）                砂
	{ BATT_BG_OBONID_11,FORMNO_412_SUNA},				//砂漠（屋内）　　　　　　    砂
	{ BATT_BG_OBONID_12,FORMNO_412_KUSA},				//浅い湿地                    草
	{ BATT_BG_OBONID_13,FORMNO_412_KUSA},				//氷上（屋内）                草
	{ BATT_BG_OBONID_14,FORMNO_412_GOMI},       //パレス専用                  ゴミ <<野生戦闘が起きないので外から呼ばれない
	{ BATT_BG_OBONID_15,FORMNO_412_GOMI},				//四天王（ゴースト）専用　    ゴミ
	{ BATT_BG_OBONID_16,FORMNO_412_GOMI},				//四天王（格闘）専用　　　    ゴミ
	{ BATT_BG_OBONID_17,FORMNO_412_GOMI},				//四天王（悪）専用　　　　    ゴミ
	{ BATT_BG_OBONID_18,FORMNO_412_GOMI},				//四天王（エスパー）専用　    ゴミ
	{ BATT_BG_OBONID_19,FORMNO_412_GOMI},			  //N専用                       ゴミ
	{ BATT_BG_OBONID_20,FORMNO_412_GOMI},				//ゲーチス専用                ゴミ
	{ BATT_BG_OBONID_21,FORMNO_412_SUNA},				//チャンピオン専用            砂
	{ BATT_BG_OBONID_22,FORMNO_412_KUSA},				//エンカウント草（四季あり）  草
	{ BATT_BG_OBONID_23,FORMNO_412_KUSA},				//エンカウント草（四季なし）  草
	{ BATT_BG_OBONID_24,FORMNO_412_GOMI},				//アスファルト                ゴミ
};

//--------------------------------------------------------------
/**
 * @brief   蓑ポケモンを指定した季節のフォルムにする
 *
 * @param   gdata   ゲームデータポインタ
 * @param   ppt		POKEPARTYへのポインタ
 * @param   inEnvor     戦闘お盆ＩＤ
 */
//--------------------------------------------------------------
void STRAW_POKE_FORM_ChangeForm(GAMEDATA * gdata, POKEMON_PARAM *pp, const u32 inEnvor)
{
  int i;
  int monsno;
  int form;
  ZUKAN_SAVEDATA *zw = GAMEDATA_GetZukanSave( gdata );
	
	monsno = PP_Get(pp, ID_PARA_monsno, NULL);
  for (i=0;i<BATT_BG_OBONID_MAX;i++)
  {
    if ( StrawFromTbl[i][0] == inEnvor )
    {
      form = StrawFromTbl[i][1];
      if ( monsno == MONSNO_MINOMUTTI )
      {
        PP_ChangeFormNo( pp, form );
        //図鑑登録「見た」
        ZUKANSAVE_SetPokeSee(zw, pp);
      }
      break;
    }
  }
}

