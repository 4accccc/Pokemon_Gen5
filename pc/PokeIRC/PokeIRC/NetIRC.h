#pragma once

#include <iostream>

using namespace std;



namespace PokeIRC {

	using namespace System;
	using namespace System::Diagnostics;
	using namespace System::IO;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;
	using namespace System::Text::RegularExpressions;


	//==============================================================================
	/**
	 * IRCライブラリラッパークラス
	 */
	//==============================================================================

	public ref class NetIRC
	{
	private:
		static bool isEmulate;


		static bool isSend;
		static bool isDataSend;



	public:
		static void Init(void);
		//static void callback(unsigned char *data, unsigned char size, unsigned char command, unsigned char value);
		static void draw(Form^ fm);
		static void connect(void);
		static void shutdown(void);
		static bool sendData(void);
		static bool SendLoop(void);
		static array<unsigned char>^ dataArray;

	private:


	};

}


