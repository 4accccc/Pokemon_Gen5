//=============================================================================
/**
 * @file	FileNcl.cpp
 * @brief	パレットファイル管理
 * @author	k.ohno
 * @date    2008.05.12
 */
//=============================================================================

//---------------------------------------------------------
// include 
//---------------------------------------------------------
#include "stdafx.h"
#include "FileNcl.h"
//#include "FileManager.h"

using namespace PokeIRC;
using namespace System::Windows::Forms;



//---------------------------------------------------------
// ここからFileNCGReadクラス
//---------------------------------------------------------

//--------------------------------------------------------------
/**
 * フィルター種別を返す
 * @param   filename
 * @retval  StreamReader^
 */
//--------------------------------------------------------------

String^ FileNCLRead::getMyFilter(void)
{
	return "ncl files (*.ncl)|*.ncl|All files (*.*)|*.*";
}

//--------------------------------------------------------------
/**
 * ファイルを読み込んで格納
 * @param   filename
 * @retval  StreamReader^
 */
//--------------------------------------------------------------

void FileNCLRead::read(String^ filename, String^ nowDir)
{
	if(false == getStreamReader(filename)){
		return;
	}
	MyName = filename;
	ReadNCLHeader();
	ReadNCLDataBlock();
	userReader->Close();
	addArray();  // 自分を登録
	//FileManager::NCLNodeAdd(System::IO::Path::GetFileName(filename), this);
}


//--------------------------------------------------------------
/**
 * NCLヘッダブロック読み込み
 */
//--------------------------------------------------------------

void FileNCLRead::ReadNCLHeader(void)
{
	nclHead = gcnew NCLHeader;
	nclHead->FormatID = userReader->ReadUInt32();
	nclHead->ByteOrderMark = userReader->ReadUInt16();
	nclHead->FormatVersion = userReader->ReadUInt16();
	nclHead->FileSize = userReader->ReadUInt32();
	nclHead->HeaderSize = userReader->ReadUInt16();
	nclHead->DataBlocks = userReader->ReadUInt16();
}


//--------------------------------------------------------------
/**
 * NCLデータブロック読み込み
 */
//--------------------------------------------------------------

void FileNCLRead::ReadNCLDataBlock(void)
{
	nclData = gcnew NCLPalData;

	nclData->BlockType = userReader->ReadUInt32();
	nclData->BlockSize = userReader->ReadUInt32();
	nclData->PaletteColorNumber = userReader->ReadUInt32();
	nclData->PaletteNumber = userReader->ReadUInt32();
	nclData->PaletteData = userReader->ReadBytes(nclData->BlockSize-0x10);
}

//--------------------------------------------------------------
/**
 * NCLデータブロック読み込み
 */
//--------------------------------------------------------------

unsigned int FileNCLRead::getPaletteR(int line,int no)
{
	int pal = nclData->PaletteData[line*32+no*2]+nclData->PaletteData[line*32+no*2+1]*0x100;
	return (pal & 0x1f) << 3;
}
//--------------------------------------------------------------
/**
 * NCLデータブロック読み込み
 */
//--------------------------------------------------------------

unsigned int FileNCLRead::getPaletteG(int line,int no)
{
	int pal = nclData->PaletteData[line*32+no*2]+nclData->PaletteData[line*32+no*2+1]*0x100;
	return ((pal>>5) & 0x1f)<<3;
}

//--------------------------------------------------------------
/**
 * NCLデータブロック読み込み
 */
//--------------------------------------------------------------

unsigned int FileNCLRead::getPaletteB(int line,int no)
{
	int pal = nclData->PaletteData[line*32+no*2]+nclData->PaletteData[line*32+no*2+1]*0x100;
	return ((pal>>10) & 0x1f)<<3;
}

//--------------------------------------------------------------
/**
 * プロジェクト文字列を得る
 */
//--------------------------------------------------------------
String^ FileNCLRead::getProjectDataString(void)
{
	return String::Format("//RESOURCE NCL {0}", MyName);
}

