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
	 * ファイル読み込み基底クラス
	 */
	//==============================================================================
	public ref class FileBase
	{


	public:
		String^ MyName;
		BinaryReader^ userReader;   // ファイルを読み込む為のハンドル
		BinaryWriter^ userWriter;   // ファイルを書き込む為のハンドル
		String^ inputFileName(void);
		String^ outputFileName(void);
	
		virtual void read(String^ filename, String^ nowDir){};
		virtual void write(void) {};
		virtual Bitmap^ PictureWrite(PictureBox^ pict) { return nullptr; };
		virtual String^ getMyFilter(void){ return "";};
		virtual unsigned short getCopyScreen(int x, int y){ return 0;};
		virtual void setPasteScreen(unsigned short scrdata, int x, int y){};
		virtual String^ getMyName(void){ return MyName; };
        virtual String^ getProjectDataString(void) { return ""; };
		virtual int outHeader(StreamWriter^ writer,int addnum) { return addnum; };
		virtual void outSource(StreamWriter^ writer, String^ Dir) {};
		virtual int getVRAMSize(void){ return 0; };
		virtual int ScanPalette(void){ return 0; };


      

		bool getStreamReader(String^ filename);
		bool getStreamWriter(String^ FileName);


		unsigned int Get4Bit(array<unsigned char>^ data,int no);


		void addArray(void);



	};







}
