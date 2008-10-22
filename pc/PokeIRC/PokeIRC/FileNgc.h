
#pragma once

#include <iostream>
using namespace std;

#include "FileNcl.h"

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
	 * NCG読み込みクラス
	 */
	//==============================================================================

	public ref class FileNCGRead : public FileBase
	{
	private:

		static const int HEADER_BYTE=16;
		static const int _NGC_BITMODE4 = 0; //ビットモード情報

		ref struct NCGHeader{
			unsigned long FormatID;
			unsigned short ByteOrderMark;
			unsigned short FormatVersion;
			unsigned long FileSize;
			unsigned short HeaderSize;
			unsigned short DataBlocks;
		};


		ref struct NCGDataHeader{
			unsigned long BlockType;
			unsigned long BlockSize;
			unsigned long CharWidth;
			unsigned long CharHeight;
			unsigned long BitMode;
			array<unsigned char>^ CharData;
		};

		ref struct NCGAttr{
			unsigned long BlockType;
			unsigned long BlockSize;
			unsigned long CharWidth;
			unsigned long CharHeight;
			array<unsigned char>^ AttrData;
		};

		ref struct NCGLinkFile{
			unsigned long BlockType;
			unsigned long BlockSize;
	//		array<unsigned char>^ FileName;
			String^ FileName;
		};


	private:
		NCGHeader^ ngcHead;
		NCGDataHeader^ ngcData;
		NCGAttr^   ngcAttr;
		NCGLinkFile^ ngcLink;
		FileNCLRead^ nclClass;
		int PalNo;
		int VramNo;

	public:
		virtual void read( String^ filename , String^ nowDir) override;
		virtual Bitmap^ PictureWrite(PictureBox^ pict) override;
		virtual String^ getMyFilter(void) override;
        virtual String^ getProjectDataString(void) override;
		virtual int getVRAMSize(void) override;


		void readWithNcl(String^ filename, String^ nowDir,String^ nclName);
		void setPalNo(int no){ PalNo = no; };
		void setVramNo(int no){ VramNo = no; };



		void setNCLHandle(FileNCLRead^ ncl);
		String^ getNCLName(void);
		void Write1Char(PictureBox^ pict,Bitmap^ bmp, int dotNo, int palNo,int XOf, int YOf);


	private:
		void ReadNGCDataBlock(void);
		void ReadNGCHeader(void);
		void ReadNGCAttr(void);
		void ReadNGCLink(void);

	public:


//		void WritePictureImage(PictureBox^ pict,NCGHeader^ head,NCGDataHeader^ dataHead);



	};



}
