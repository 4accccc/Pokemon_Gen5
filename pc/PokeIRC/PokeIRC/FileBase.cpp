//=============================================================================
/**
 * @file	FileBase.cpp
 * @brief	ファイルの基底クラス
 * @author	k.ohno
 * @date    2008.05.08
 */
//=============================================================================

//---------------------------------------------------------
// include 
//---------------------------------------------------------
#include "stdafx.h"
#include "FileBase.h"
//#include "FileManager.h"
//#include "ApplicationData.h"

using namespace PokeIRC;
using namespace System::Windows::Forms;

//--------------------------------------------------------------
/**
 * ファイル名の入力を行ってもらう
 * @param   nowDir 現状のDIR名
 * @retval  ファイル名文字列
 */
//--------------------------------------------------------------

String^ FileBase::inputFileName( void )
{

	int index = -1;
	System::IO::Stream^ myStream;
	OpenFileDialog^ openFileDialog1 = gcnew OpenFileDialog;

	openFileDialog1->InitialDirectory = "";
	openFileDialog1->Filter = getMyFilter();
	openFileDialog1->FilterIndex = 1;
	openFileDialog1->RestoreDirectory = true;

	if ( openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK )
	{
		if ( (myStream = openFileDialog1->OpenFile()) != nullptr )
		{
			myStream->Close();
			//ApplicationData::RWDir = System::IO::Path::GetDirectoryName(openFileDialog1->FileName);
			return openFileDialog1->FileName;
		}
	}
	return nullptr;
}



//--------------------------------------------------------------
/**
 * ファイルを読み込みStreamReaderを返す
 * @param   NowDir
 * @retval  BinaryReader^
 */
//--------------------------------------------------------------
bool FileBase::getStreamReader(String^ FileName)
{
	FileStream^ userStream;


	try {
		userStream = gcnew FileStream( FileName, FileMode::Open );
	}
	catch ( IOException^ e ) {
		MessageBox::Show( "load err " + dynamic_cast<String^>(e) + FileName );
		return false;
    }
	

	try {
		userReader = gcnew BinaryReader(userStream);
	}
	catch ( ArgumentException^ ){
		MessageBox::Show( "load err ");
		return false;
	}
	return true;
}

//--------------------------------------------------------------
/**
 * ファイルを書き込むStreamWriterを返す
 * @param   NowDir
 * @retval  BinaryReader^
 */
//--------------------------------------------------------------
bool FileBase::getStreamWriter(String^ FileName)
{
	FileStream^ userStream;


	if(true == System::IO::File::Exists(FileName)){
		System::IO::File::Copy(FileName, FileName + ".bak", true);
	}
	try {
		userStream = gcnew FileStream( FileName, FileMode::Create );
	}
	catch ( IOException^ e ) {
      MessageBox::Show( "load err " + dynamic_cast<String^>(e) + FileName );
	  return false;
    }
	try {
		userWriter = gcnew BinaryWriter(userStream);
	}
	catch ( ArgumentException^ ){
		MessageBox::Show( "load err ");
		return false;
	}
	return true;
}


//--------------------------------------------------------------
/**
 * 4Bit間隔のデータを取得
 * @param   data  バイナリデータ
 * @param   no    取り出す場所
 * @retval  int 取り出したデータ
 */
//--------------------------------------------------------------


unsigned int FileBase::Get4Bit(array<unsigned char>^ data,int no)
{
	unsigned int dataBit = data[no / 2];
	if((no % 2)){  //奇数の場合 上位4BITにデータがある
		dataBit = dataBit >> 4;
	}
	return dataBit & 0x0f;
}


//--------------------------------------------------------------
/**
 * ファイル名の出力を行ってもらう
 * @param   nowDir 現状のDIR名
 * @retval  ファイル名文字列
 */
//--------------------------------------------------------------

String^ FileBase::outputFileName( void )
{

	int index = -1;
	System::IO::Stream^ myStream;
	SaveFileDialog^ saveFileDialog1 = gcnew SaveFileDialog;

//	saveFileDialog1->InitialDirectory = ApplicationData::RWDir;
	saveFileDialog1->Filter = getMyFilter();
	saveFileDialog1->FilterIndex = 1;
	saveFileDialog1->RestoreDirectory = true;

	if ( saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK )
	{
		if ( (myStream = saveFileDialog1->OpenFile()) != nullptr )
		{
			myStream->Close();
//			ApplicationData::RWDir = System::IO::Path::GetDirectoryName(saveFileDialog1->FileName);
			return saveFileDialog1->FileName;
		}
	}
	return nullptr;
}



//--------------------------------------------------------------
/**
 * 自分自身を構造体に追加
 * @param   nowDir 現状のDIR名
 * @retval  ファイル名文字列
 */
//--------------------------------------------------------------

void FileBase::addArray(void)
{
//  FileManager::addReadFile(this);
}
