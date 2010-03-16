//============================================================================================
/**
 * @file  sheimi_normalform.c
 * @bfief シェイミ時間越えフォルム戻り
 * @author  Saito
 */
//============================================================================================
#include <gflib.h>

#include "sheimi_normalform.h"
#include "poke_tool/monsno_def.h" //for MONSNO_SHEIMI FORMNO_～

//--------------------------------------------------------------
/**
 * @brief   POKEPARTYのシェイミを全てノーマルフォルムにする
 *
 * @param   ppt		POKEPARTYへのポインタ
 */
//--------------------------------------------------------------
void SHEIMI_NFORM_ChangeNormal(POKEPARTY *ppt)
{
	int pos, count, monsno, form_no;
	POKEMON_PARAM *pp;
	int set_form_no = FORMNO_SHEIMI_LAND;
	
	count = PokeParty_GetPokeCount(ppt);
	for(pos = 0; pos < count; pos++)
  {
		pp = PokeParty_GetMemberPointer(ppt,pos);
		monsno = PP_Get(pp, ID_PARA_monsno, NULL);
		form_no = PP_Get(pp, ID_PARA_form_no, NULL);
		if(monsno == MONSNO_SHEIMI && form_no == FORMNO_SHEIMI_SKY)
    {
      PP_ChangeFormNo( pp, set_form_no );
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief   時刻が夜に変わったタイミングならば、手持ちのシェイミを全てノーマルフォルムにする
 *
 * @param   ppt				POKEPARTYへのポインタ
 * @param   min_diff		経過した時間(分単位)
 * @param   now				現在時間
 *
 * @retval  TRUE:ノーマルフォルムへの変更を行った
 */
//--------------------------------------------------------------
BOOL SHEIMI_NFORM_ChangeNormal_TimeUpdate(POKEPARTY *ppt, int min_diff, const RTCTime * now)
{
	s32 hour, min_pos;
	
	//現在がNG時刻の場合(20:00 ～ 27:59)
	if(now->hour >= 20 || now->hour < 4){
		hour = now->hour;
		if(hour < 4){
			hour += 24;		//例)深夜2時ならば26時に変換する
		}
		hour -= 20;	//20時からのオフセットにする
		min_pos = now->minute + hour * 60;	//20時から何分経過しているか

		min_diff++;	//秒数の端数を考慮して切り上げ(端数が無いのが1/60なのでデフォで切り上げ

		if(min_pos < min_diff){
			//OS_TPrintf("シェイミ form 更新\n");
			SHEIMI_NFORM_ChangeNormal(ppt);
			return TRUE;
		}
		return FALSE;
	}
	else{
	//現在がOK時刻の場合(4:00 ～ 19:59)はmin_diffがNG時刻をまたいでいないかチェック
		min_pos = now->minute + (now->hour - 4) * 60;	//4時から何分経過しているか
		if(min_pos < min_diff){
			//OS_TPrintf("シェイミ form 更新\n");
			SHEIMI_NFORM_ChangeNormal(ppt);
			return TRUE;
		}
		return FALSE;
	}
}

