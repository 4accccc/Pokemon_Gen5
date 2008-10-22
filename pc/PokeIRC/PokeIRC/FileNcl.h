#pragma once

#include <iostream>
#include "FileBase.h"
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
	 * NCLì«Ç›çûÇ›ÉNÉâÉX
	 */
	//==============================================================================

	public ref class FileNCLRead : public FileBase
	{
	private:


		ref struct NCLHeader{
			unsigned long FormatID;
			unsigned short ByteOrderMark;
			unsigned short FormatVersion;
			unsigned long FileSize;
			unsigned short HeaderSize;
			unsigned short DataBlocks;
		};

		ref struct NCLPalData{
			unsigned long BlockType;
			unsigned long BlockSize;
			unsigned long PaletteColorNumber;
			unsigned long PaletteNumber;
			array<unsigned char>^ PaletteData;
		};

		NCLHeader^ nclHead;
		NCLPalData^ nclData;
		int PalPos;


	public:
		virtual void read( String^ filename , String^ nowDir) override;
		virtual String^ getMyFilter(void) override;
        virtual String^ getProjectDataString(void) override;

		unsigned int getPaletteR(int line,int no);
		unsigned int getPaletteG(int line,int no);
		unsigned int getPaletteB(int line,int no);

		void setPalPos(int no){ PalPos = no; };

	private:
		void ReadNCLDataBlock(void);
		void ReadNCLHeader(void);


	};

}
