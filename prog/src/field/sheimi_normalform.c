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
#include "system/timezone.h"    //for TIMEZONE_～
#include "system/rtc_tool.h"  //for PM_RTC_GetTimeZoneChangeHour

//--------------------------------------------------------------
/**
 * @brief   POKEPARTYのシェイミを全てノーマルフォルムにする
 *
 * @param   gdata ゲームデータポインタ
 * @param   ppt		POKEPARTYへのポインタ
 */
//--------------------------------------------------------------
void SHEIMI_NFORM_ChangeNormal(GAMEDATA * gdata, POKEPARTY *ppt)
{
	int pos, count, monsno, form_no;
	POKEMON_PARAM *pp;
	int set_form_no = FORMNO_SHEIMI_LAND;
  ZUKAN_SAVEDATA *zw = GAMEDATA_GetZukanSave( gdata );
  BOOL see = FALSE;

	count = PokeParty_GetPokeCount(ppt);
	for(pos = 0; pos < count; pos++)
  {
		pp = PokeParty_GetMemberPointer(ppt,pos);
		monsno = PP_Get(pp, ID_PARA_monsno, NULL);
		form_no = PP_Get(pp, ID_PARA_form_no, NULL);
		if(monsno == MONSNO_SHEIMI && form_no == FORMNO_SHEIMI_SKY)
    {
      PP_ChangeFormNo( pp, set_form_no );
      //図鑑登録「見た」
      if (!see)
      {
        ZUKANSAVE_SetPokeSee(zw, pp);
        see = TRUE;
      }
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief   時刻が夜に変わったタイミングならば、手持ちのシェイミを全てノーマルフォルムにする
 *
 * @param   gdata     ゲームデータポインタ
 * @param   ppt				POKEPARTYへのポインタ
 * @param   min_diff		経過した時間(分単位)
 * @param   now				現在時間
 * @parm    inSeason    現在季節
 *
 * @retval  TRUE:ノーマルフォルムへの変更を行った
 */
//--------------------------------------------------------------
BOOL SHEIMI_NFORM_ChangeNormal_TimeUpdate(GAMEDATA * gdata, POKEPARTY *ppt, int min_diff, const RTCTime * now, int inSeason)
{
	s32 hour, min_pos;

  int night,morning;

  night = PM_RTC_GetTimeZoneChangeHour( inSeason, TIMEZONE_NIGHT );
  morning = PM_RTC_GetTimeZoneChangeHour( inSeason, TIMEZONE_MORNING );
	
	//現在がNG時刻の場合(春のとき20:00 ～ 27:59  night～morning)
	if(now->hour >= night || now->hour < morning){
		hour = now->hour;
		if(hour < morning){
			hour += 24;		//例)深夜2時ならば26時に変換する
		}
		hour -= night;	//nihgt時からのオフセットにする
		min_pos = now->minute + hour * 60;	//night時から何分経過しているか

		min_diff++;	//秒数の端数を考慮して切り上げ(端数が無いのが1/60なのでデフォで切り上げ

		if(min_pos < min_diff){
			//OS_TPrintf("シェイミ form 更新\n");
			SHEIMI_NFORM_ChangeNormal(gdata, ppt);
			return TRUE;
		}
		return FALSE;
	}
	else{
	//現在がOK時刻の場合(春のとき4:00 ～ 19:59 morning～night)はmin_diffがNG時刻をまたいでいないかチェック
		min_pos = now->minute + (now->hour - morning) * 60;	//morning時から何分経過しているか
		if(min_pos < min_diff){
			//OS_TPrintf("シェイミ form 更新\n");
			SHEIMI_NFORM_ChangeNormal(gdata, ppt);
			return TRUE;
		}
		return FALSE;
	}
}

