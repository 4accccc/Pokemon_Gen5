// PokeIRC.cpp : メイン プロジェクト ファイルです。

#include "stdafx.h"
#include "MainForm.h"

using namespace PokeIRC;
using namespace System::Threading;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// コントロールが作成される前に、Windows XP ビジュアル効果を有効にします
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// メイン ウィンドウを作成して、実行します認証
	//Application::Run(gcnew MainForm());

	MainForm^ form = gcnew MainForm();

	form->Show();

	while( form->Created ){
		Thread::Sleep( 10 );
		Application::DoEvents();
		form->Draw();
	}
	return 0;
}
