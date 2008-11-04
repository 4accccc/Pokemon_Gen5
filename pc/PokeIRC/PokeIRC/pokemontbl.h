#pragma once




namespace PokeIRC {

	using namespace System;


	typedef struct{
		int no;
		char* name;
	} PokeData;


	typedef struct{
		unsigned short dp_code;	
		unsigned short sjis_code;
	} DP2UTFCONV;



	public ref class PokemonDataTable{




	public:
		static int getPokemonNo(int i);
		static char* getPokemonName(int i);



		static String^ DPSpellConv(unsigned short* strcodebuff,int size);


	};
}

