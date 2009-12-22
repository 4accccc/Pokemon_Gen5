//============================================================================
/**
 *@file		tr_ai_zantei.h
 *@brief  MAKEを通すために暫定でdefine定義をしておくヘッダ（当然、置き換えをしなければいけない）
 *@author	HisashiSogabe
 *@data		2009.12.15
 */
//============================================================================

#define CONDITION_MAHI  ( 0 )
#define CONDITION_DOKUDOKU  ( 0 )
#define CONDITION2_KONRAN ( 0 )
#define CONDITION2_SHIME  ( 0 )       //WAZASICK_BIND
#define CONDITION2_NOROI  ( 0 )
#define WAZAKOUKA_AKUBI ( 0 )
#define KIND_TOKUSYU  ( 0 )
#define KIND_BUTSURI  ( 0 )
#define KIND_HENKA  ( 0 )

//置き換え対象がない
//#define CONDITION2_MIGAWARI ( 0 )   //コマンドに置き換える
//#define WAZAKOUKA_HUUIN_DID ( 0 )   //ふういんをためしたかどうか？これは、以前はServerだったがAI側でフラグをもって判断できそう
//#define WAZAKOUKA_HOGOSYOKU ( 0 )   //BPP_CONTFLG～に置き換える
//#define WAZAKOUKA_AQUARING  ( 0 )   //WAZASICK_～に置き換える
//#define SIDE_CONDITION_MIRAIYOCHI ( 0 ) //BPP_CONTFLG～に置き換える

//置き換え完了

//#define CONDITION_NEMURI      ( 0 )   //WAZASICK_NEMURI

//#define CONDITION2_AKUMU      ( 0 )   //WAZASICK_AKUMU
//#define CONDITION2_ICHAMON  ( 0 )     //WAZASICK_ICHAMON
//#define CONDITION2_MEROMERO ( 0 )     //WAZASICK_MEROMERO
//#define CONDITION2_MIYABURU ( 0 )     //WAZASICK_MIYABURU
//#define CONDITION2_KUROIMANAZASHI ( 0 ) //WAZASICK_TOOSENBOU
//#define CONDITION2_KIAIDAME ( 0 )       //BPP_CONTFLG_KIAIDAME

//#define WAZAKOUKA_HUUIN ( 0 )     //BTL_FLDEFF_FUIN
//#define WAZAKOUKA_HOROBINOUTA ( 0 ) //WAZASICK_HOROBINOUTA
//#define WAZAKOUKA_NEWOHARU  ( 0 )   //WAZASICK_NEWOHARU
//#define WAZAKOUKA_DOROASOBI ( 0 )   //BTL_FLDEFF_DOROASOBI
//#define WAZAKOUKA_MIZUASOBI ( 0 )   //BTL_FLDEFF_MIZUASOBI
//#define WAZAKOUKA_YADORARE  ( 0 )   //WAZASICK_YADORIGI
//#define WAZAKOUKA_LOCKON  ( 0 )     //ATTACKならWAZASICK_MUSTHIT DEFENCEならWAZASICK_MUSTHIT_TARGET
//#define WAZAKOUKA_MIRACLE_EYE ( 0 )   //WAZASICK_MIYABURU
//#define WAZAKOUKA_SHUTOUT ( 0 )       //WAZASICK_SASIOSAE
//#define WAZAKOUKA_HEALBLOCK ( 0 )     //WAZASICK_KAIHUKUHUUJI
//#define WAZAKOUKA_IEKI  ( 0 )         //WAZASICK_IEKI
//#define WAZAKOUKA_POWER_TRICK ( 0 )   //BPP_CONTFLG_POWERTRICK
//#define WAZAKOUKA_DENZIHUYUU  ( 0 )   //WAZASICK_FLYING

//#define SIDE_CONDITION_SHINPI ( 0 )       //BTL_SIDEEFF_SINPINOMAMORI
//#define SIDE_CONDITION_TOKUSYUGUARD ( 0 ) //BTL_SIDEEFF_HIKARINOKABE
//#define SIDE_CONDITION_SHIROIKIRI ( 0 )   //BTL_SIDEEFF_SIROIKIRI
//#define SIDE_CONDITION_BUTSURIGUARD ( 0 ) //BTL_SIDEEFF_REFRECTOR
//#define SIDE_CONDITION_OIKAZE ( 0 )       //BTL_SIDEEFF_OIKAZE
//#define SIDE_CONDITION_OMAZINAI ( 0 )     //BTL_SIDEEFF_OMAJINAI
//#define SIDE_CONDITION_MAKIBISHI  ( 0 )   //BTL_SIDEEFF_MAKIBISI
//#define SIDE_CONDITION_STEALTHROCK  ( 0 ) //BTL_SIDEEFF_STEALTHROCK
//#define SIDE_CONDITION_DOKUBISHI  ( 0 )   //BTL_SIDEEFF_DOKUBISI

//#define FIELD_CONDITION_TRICKROOM ( 0 )   //BTL_FLDEFF_TRICKROOM
//#define FIELD_CONDITION_JUURYOKU  ( 0 )   //BTL_FLDEFF_JURYOKU

//#define FIGHT_TYPE_2vs2 ( 0 )             //BTL_RULE_DOUBLE
//#define FIGHT_TYPE_TOWER  ( 0 )           //BTL_COMPETITOR_SUBWAY
