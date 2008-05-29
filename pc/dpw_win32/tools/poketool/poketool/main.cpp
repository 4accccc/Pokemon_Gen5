/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     main.cpp

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include "stdafx.h"
#include "poketool.h"
#include "monsno.h"
#include "dpw_types.h"
#include "trans_pokecode.h"
#include <iostream>
#include "base64codec.h"
#include "illegal_check.h"
#include "itemsym.h"
#include <string>

#define WAZA_DATA_FILE_PATH "..\\poketool\\waza.bin"

#define foreach(a,b,c) \
    for(a::iterator c=(b).begin();(c)!=(b).end();(c)++)
#define const_foreach(a,b,c) \
    for(a::const_iterator c=(b).begin();(c)!=(b).end();(c)++)

static void fill_random_data(void *addr, u32 size);
static void print_unicode(wchar_t* wcs);
static std::vector<std::string> GetFileList(LPCWSTR path);
bool IsValidName(wchar_t* namelist, wchar_t* pname);

int _tmain(int argc, _TCHAR* argv[])
{
	srand(0x12345678);
	setlocale( LC_ALL, "Japanese" );

	printf("POKEMON_PASO_PARAM: %d\n", sizeof(POKEMON_PASO_PARAM));
	printf("B_TOWER_POKEMON: %d\n", sizeof(B_TOWER_POKEMON));
	printf("POKEMON_PARAM: %d\n", sizeof(POKEMON_PARAM));

	// personal_rndは暗号時の種になっている。値は32ビットのどれか。
	// 暗号化するときは、pokemon_paso_param::paradataのチェックサム
	// を取り、pokemon_paso_param::checksumへ入れる。
	// pokemon_calc_paramはpokemon_paso_param::personal_rndを使って
	// 暗号化。pokemon_paso_param::pp_fast_mode、
	// pokemon_paso_param::ppp_fast_modeは0でいいはず。

	{	// PokeTool_UnifyNameのテスト
		u16* in_name = (u16*)L"アイうエオ";
		u16 out_name[MONS_NAME_SIZE+EOM_SIZE];

		PokeTool_UnifyName(in_name, out_name);
		print_unicode((wchar_t*)out_name);

		s32 pokechar;
		u16 pokecode_buf[MONS_NAME_SIZE+EOM_SIZE];
		u16 unicode_buf[MONS_NAME_SIZE+EOM_SIZE];

		memset(pokecode_buf, 0xff, sizeof(pokecode_buf));
		memset(unicode_buf, 0, sizeof(unicode_buf));
		
		for (pokechar = 1; pokechar <= 0x200; pokechar++) {
			pokecode_buf[0] = (u16)pokechar;
			if (pokecode_to_unicode(pokecode_buf, (wchar_t*)unicode_buf, sizeof(unicode_buf)) >= 1) {
				PokeTool_UnifyName(unicode_buf, out_name);
				printf("%d: ", pokechar);
				print_unicode((wchar_t*)unicode_buf);
				print_unicode((wchar_t*)out_name);
			}
		}

		if (IsValidName(L"\tあいうえお\tかきくけこ\tあいうえおかきくけこ\t", (wchar_t*)in_name)) {
			printf("valid.\n");
		} else {
			printf("invalid.\n");
		}
	}

	// ダイヤモンドとパールでパラメータや成長テーブルが変わるの？

	{
		POKEMON_PARAM src;
		POKEMON_PARAM coded;

		memset(&src, 0, sizeof(POKEMON_PARAM));

		fill_random_data(src.ppp.paradata, sizeof(src.ppp.paradata));
		src.ppp.personal_rnd = rand();
		fill_random_data(&src.pcp, sizeof(src.pcp));

		PokeTool_EncodePokemonParam(&src, &coded);

		printf("%d\n", PokeTool_DecodePokemonParam(&coded, &src));
	}

    // デフォルトポケモン名を試す
    {
        wchar_t temp[1024];
        for(int i=1; i<=MONSNO_AUSU; i++){
            for(int j=1; j<3; j++){
                swprintf(temp, L"No.%d, %s" , i, (wchar_t*)PokeTool_GetDefaultName(i, j));
                print_unicode(temp);
            }
        }
    }
	{
		FILE* file = fopen("out/TR_UPLOAD_20060710_0110.bin", "rb");
		if (file == NULL) {
			printf("cannot open file.\n");
			return 1;
		}

		{
			Dpw_Tr_Data tr_data;
			POKEMON_PARAM pp;
			POKEMON_PARAM encoded;

			fread(&tr_data, sizeof(Dpw_Tr_Data), 1, file);

			printf("%d\n", PokeTool_DecodePokemonParam((POKEMON_PARAM*)&tr_data.postData.data, &pp));

			printf("%d\n", pp.pcp.level);

			printf("%d\n", PokeTool_GetNickNameFlag(&pp));
			PokeTool_SetNickNameFlag(&pp, TRUE);
			printf("%d\n", PokeTool_GetNickNameFlag(&pp));

			{
				u16 namebuf[MONS_NAME_SIZE+EOM_SIZE];

				PokeTool_GetNickName(&pp, namebuf);
				print_unicode((wchar_t*)namebuf);

				PokeTool_GetParentName(&pp, namebuf);
				print_unicode((wchar_t*)namebuf);

				PokeTool_GetMailName(&pp, namebuf);
				print_unicode((wchar_t*)namebuf);

				PokeTool_SetParentName(&pp, (u16*)L"しげまる！");
				PokeTool_GetParentName(&pp, namebuf);
				print_unicode((wchar_t*)namebuf);

				PokeTool_SetMailName(&pp, (u16*)L"メ－ル！");
				PokeTool_GetMailName(&pp, namebuf);
				print_unicode((wchar_t*)namebuf);
			}

			PokeTool_EncodePokemonParam(&pp, &encoded);

			printf("%d\n", memcmp(&tr_data.postData.data, &encoded, sizeof(POKEMON_PARAM)));
		}

		fclose(file);
	}

	{
		FILE* file = fopen("out/TR_UPLOAD_20060710_0110.bin", "rb");
		if (file == NULL) {
			printf("cannot open file.\n");
			return 1;
		}

		{
			Dpw_Tr_Data tr_data;
			POKEMON_PARAM pp;
			POKEMON_PARAM encoded;

			fread(&tr_data, sizeof(Dpw_Tr_Data), 1, file);

			printf("%d\n", PokeTool_DecodePokemonParam((POKEMON_PARAM*)&tr_data.postData.data, &pp));

			printf("%d\n", pp.pcp.level);

			printf("%d\n", PokeTool_GetNickNameFlag(&pp));
			PokeTool_SetNickNameFlag(&pp, TRUE);
			printf("%d\n", PokeTool_GetNickNameFlag(&pp));

			printf("monsno: %d\n", PokeTool_GetPokeParam(&pp, ID_PARA_monsno, NULL));

			{
				u16 namebuf[MONS_NAME_SIZE+EOM_SIZE];

				if (!PokeTool_GetNickName(&pp, namebuf)) {
					printf("PokeTool_GetNickName error.\n");
				}
				print_unicode((wchar_t*)namebuf);

				if (!PokeTool_GetParentName(&pp, namebuf)) {
					printf("PokeTool_GetNickName error.\n");
				}
				print_unicode((wchar_t*)namebuf);

				if (!PokeTool_GetMailName(&pp, namebuf)) {
					printf("PokeTool_GetNickName error.\n");
				}
				print_unicode((wchar_t*)namebuf);

				if (!PokeTool_SetParentName(&pp, (u16*)L"しげまる！")) {
					printf("PokeTool_GetNickName error.\n");
				}
				if (!PokeTool_GetParentName(&pp, namebuf)) {
					printf("PokeTool_GetNickName error.\n");
				}
				print_unicode((wchar_t*)namebuf);

				if (!PokeTool_SetMailName(&pp, (u16*)L"メ－ル！")) {
					printf("PokeTool_GetNickName error.\n");
				}
				if (!PokeTool_GetMailName(&pp, namebuf)) {
					printf("PokeTool_GetNickName error.\n");
				}
				print_unicode((wchar_t*)namebuf);

				printf("PokeTool_IsRare: %d\n", PokeTool_IsRare(&pp));

				printf("PokeTool_HasMail: %d\n", PokeTool_HasMail(&pp));
				{
					u16 item = ITEM_BURIKKUMEERU;
					PokeTool_SetPokeParam(&pp, ID_PARA_item, &item);
				}
				printf("PokeTool_HasMail: %d\n", PokeTool_HasMail(&pp));
				{
					u16 item = ITEM_SIRATAMA;
					PokeTool_SetPokeParam(&pp, ID_PARA_item, &item);
				}
				printf("PokeTool_HasMail: %d\n", PokeTool_HasMail(&pp));
				printf("PokeTool_GetMailLang: %d\n", PokeTool_GetMailLang(&pp));
/*
				for (u16 i = 0; i <= MONSNO_END + 1; i++) {
					PokeTool_SetPokeParam(&pp, ID_PARA_monsno, &i);

					for(u8 j = 0; j <= COUNTRY_CODE_MAX + 1; j++){
						PokeTool_SetNickNameFlag(&pp, TRUE);
						PokeTool_SetPokeParam(&pp, ID_PARA_country_code, &j);

						printf("monsno:%d countryCode:%d nickFlag:%d ", i, j, PokeTool_GetNickNameFlag(&pp));
						if (!PokeTool_SetNickNameDefault(&pp)) {
							printf("PokeTool_SetNickNameDefault error.\n");
							continue;
						}

						if (!PokeTool_GetNickName(&pp, namebuf)) {
							printf("PokeTool_GetNickName error.\n");
							continue;
						}
						print_unicode((wchar_t*)namebuf);
					}
				}
*/
				/*
				int monsno1 = 52;
				u16 waza[4];
				waza[0] = 47;
				waza[1] = 80;
				waza[2] = 85;
				waza[3] = 87;
				PokeTool_SetPokeParam(&pp, ID_PARA_waza1, &waza[0]);
				PokeTool_SetPokeParam(&pp, ID_PARA_waza2, &waza[1]);
				PokeTool_SetPokeParam(&pp, ID_PARA_waza3, &waza[2]);
				PokeTool_SetPokeParam(&pp, ID_PARA_waza4, &waza[3]);
				PokeTool_SetPokeParam(&pp, ID_PARA_monsno, &monsno1);
				if (!PokeTool_GetNickName(&pp, namebuf)) {
					printf("PokeTool_GetNickName error.\n");
				}
				print_unicode((wchar_t*)namebuf);
				PokeTool_CheckData(&pp);
				//PokeTool_PrintWazaData("wazadata.txt");
				*/
			}
			PokeTool_EncodePokemonParam(&pp, &encoded);

			printf("%d\n", memcmp(&tr_data.postData.data, &encoded, sizeof(POKEMON_PARAM)));
			fclose(file);
		}

		{
			FILE* file = fopen("out/BT_UPDATE_20070105_1200_2_10.bin", "rb");
			if (file == NULL) {
				printf("cannot open file.\n");
				return 1;
			}

			{
				Dpw_Bt_Player bt_player, bt_player2, bt_player3, bt_player4, bt_player5, bt_player6, bt_player7;
                Dpw_Bt_Leader bt_leader, bt_leader2;
				Dpw_Bt_PokemonData bt_data;
                size_t ret;
                int sizePly = sizeof(Dpw_Bt_Player);
                int sizeLeader = sizeof(Dpw_Bt_Leader);
                int sizePoke = sizeof(Dpw_Bt_PokemonData);

				ret = fread(&bt_player, sizeof(Dpw_Bt_Player), 1, file);//34
				ret = fread(&bt_leader, 34, 1, file);
				ret = fread(&bt_player3, sizeof(Dpw_Bt_Player), 1, file);
				ret = fread(&bt_player4, sizeof(Dpw_Bt_Player), 1, file);
				ret = fread(&bt_player5, sizeof(Dpw_Bt_Player), 1, file);
				ret = fread(&bt_player6, sizeof(Dpw_Bt_Player), 1, file);
				ret = fread(&bt_player7, sizeof(Dpw_Bt_Player), 1, file);
				ret = fread(&bt_leader, sizeof(Dpw_Bt_Leader), 1, file);
				ret = fread(&bt_leader2, 34, 1, file);
				bt_data = bt_player.pokemon[0];

				printf("PokeTool_BtIsRare: %d\n", PokeTool_BtIsRare(&bt_data.data));
			}
			fclose(file);
		}

		{
			{
				///////////////////////////////////////////
				// 技チェック
				u16 namebuf[MONS_NAME_SIZE+EOM_SIZE];
				std::string basePath = "..\\logdata_sample\\bindata\\";
				std::vector<std::string> files = GetFileList(L"..\\logdata_sample\\bindata\\*.*");
				foreach(std::vector<std::string>, files, it){
					if(*it == "." || *it == ".." || *it == "CVS" || *it == ".svn"){
						continue;
					}
					FILE* file = fopen((basePath + *it).c_str(), "rb");
					if (file == NULL) {
						printf("cannot open file.\n");
						return 1;
					}

					if(it->find("BT") == it->npos){
						Dpw_Tr_Data tr_data;
						POKEMON_PARAM pp;

						fread(&tr_data, sizeof(Dpw_Tr_Data), 1, file);

						printf("%d\n", PokeTool_DecodePokemonParam((POKEMON_PARAM*)&tr_data.postData.data, &pp));
						if (!PokeTool_GetNickName(&pp, namebuf)) {
							printf("PokeTool_GetNickName error.\n");
						}
						print_unicode((wchar_t*)namebuf);
						if(PokeTool_CheckData(&pp, WAZA_DATA_FILE_PATH) == POKETOOL_CHECKRESULT_OK){
							printf("Pokemon check OK.\n");
						}else{
							printf("Pokemon check NG.\n");
							PokeTool_CheckData(&pp, WAZA_DATA_FILE_PATH);
						}
					}else{
						/*
						Dpw_Bt_PokemonData bt_data;

						fread(&bt_data, sizeof(Dpw_Bt_PokemonData), 1, file);

						PokeTool_BtGetNickName(&bt_data.data, namebuf);
						print_unicode((wchar_t*)namebuf);
						if(PokeTool_BtCheckData(&bt_data.data, WAZA_DATA_FILE_PATH) == POKETOOL_CHECKRESULT_OK){
							printf("Pokemon check OK.\n");
						}else{
							printf("Pokemon check NG.\n");
							PokeTool_BtCheckData(&bt_data.data, WAZA_DATA_FILE_PATH);
						}
						*/
					}

					fclose(file);
				}
			}
		}
	}

	{
		static const char* base64data = "8F1s0gAAO8Qw_TBD2vovQ_xX72nYHtMvWh4Xw99Kdx8CWWDT7BQ6_6WIpgTIyQvRK8B3zGC4TgjigVJ3W_zgvtfSj9JS73t4EVr7Iqvfwu2PhbUfvKb_Kz3XxOK0BTPH09gtDbvsbhMejRp1yhAROIhEEroTcTDqKjAmTVy1jdtdNSHNrZcBPWM0U7o_jA5BY5Bp7A-x0BWd0eGrHqTwmTg-uedn9GEeaA0y3HeZIcgqweMH0zpom6p5cwOfL58etO48oqsrOc5wRuxZgT0rA0uAsIfRXik_WV3moC4MKQi-6G8-GuykSZmd7VuPAQEESgADAAAAAADWBwcMFRgqAAAAAAAAAAAAylFuBIkAhACSAHAA8QD__wAAAAA8yQAAMgALAQ==";
		u8 tr_data[sizeof(Dpw_Tr_Data)];
		char* encoded_data;
		BOOL result;

		result = base64decode(base64data, tr_data, (u32)strlen(base64data));
		encoded_data = (char*)malloc(base64size(sizeof(Dpw_Tr_Data)) + 1);

		memset(encoded_data, 0, base64size(sizeof(Dpw_Tr_Data)) + 1);

		base64encode(tr_data, encoded_data, sizeof(Dpw_Tr_Data));

		printf("strcmp: %d\n", strcmp(base64data, encoded_data));

		free(encoded_data);

		printf("result: %d\n", result);
	}

	{
		u16* unicode_str = (u16*)L"リザ－ドン";
		u16 pokecode_str[10];

		unicode_to_pokecode((const wchar_t*)unicode_str, pokecode_str, sizeof(pokecode_str));

		printf("0x%x\n", pokecode_str[2]);
	}

    puts("終了するにはエンターキーを押してください。");
	{
		char buf[2];
		gets_s(buf, 2);
	}

	return 0;
}

static void fill_random_data(void *addr, u32 size)
{
    u32 i;

    for (i = 0; i < size; i++) {
        ((u8*)addr)[i] = (u8)(rand() % 0x100);
    }
}

static void print_unicode(wchar_t* wcs)
{
	char buf[0x100];

	wcstombs(buf, wcs, sizeof(buf));

	printf("%s\n", buf);
}

std::vector<std::string> GetFileList(LPCWSTR path)
{

  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  std::vector<std::string> result;
  int nLen;
  char* mbcs;

  hFind = FindFirstFile(path, &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE) {
    printf ("Invalid File Handle. Get Last Error reports %d\n", GetLastError ());
  } else {
	do{
		nLen = WideCharToMultiByte(CP_THREAD_ACP, 0, FindFileData.cFileName, -1, NULL, 0, NULL, NULL );
		mbcs = new char[nLen];
		WideCharToMultiByte(CP_THREAD_ACP, 0, FindFileData.cFileName, (int)wcslen(FindFileData.cFileName)+1, mbcs, nLen, NULL, NULL );
		result.push_back(mbcs);
		delete mbcs;
	}while(FindNextFile(hFind, &FindFileData));

    FindClose(hFind);
  }

  return result;
}

bool IsValidName(wchar_t* namelist, wchar_t* pname){

	bool retval = true;
	wchar_t* buf;
	wchar_t* src;
	wchar_t name[MONS_NAME_SIZE+EOM_SIZE];

	PokeTool_UnifyName( (const u16*)pname, (u16*)name );

	buf = new wchar_t[wcslen(namelist) + 0xff];
	src = new wchar_t[wcslen(name) + 0xff];

	if( buf == NULL || src == NULL ){
	
		if(buf)
			delete [] buf;
		if(src)
			delete [] src;

		return false;
	}

	if( wcslen(name) > 0 ){

		swprintf(buf, L"\t%s\t", namelist);
		swprintf(src, L"\t%s\t", name);

		if( wcsstr(buf, src) != NULL )
			retval = false;

	}

	delete [] buf;
	delete [] src;

	return retval;
}