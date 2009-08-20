//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		readme.txt
 *	@brief	narc_cpyフォルダの説明
 *	@author	Toru=Nagihashi
 *	@data		2009.08.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

●このフォルダについて
	このフォルダは、resourceフォルダからprog/arcフォルダへ
	narcをコピーするためだけのフォルダです。
	コンバート環境がresouce内にないなど、何らかの理由でnarc,naixのみコミットして
	いるものをprog/arcにコピーするためのフォルダです。

	なお、デバッグ用のファイルは、resource/debugフォルダにいれるべきですので、
	そちらもご考慮下さい。


●経緯
	2009.08.20以前、prog/arcの中に直接narcをコミットしているものがありました。
	しかし、その関係上、prog/arcの中のnarc,naixはmake cleanの対象になっておらず
	resourceからコピーしたファイルはprog/arc内に残ったままになっていました。

	これが原因で、resourceからコピー先を変更したりした場合でもprog/arcに変更前
	のファイルがのこり、makeが通らなくなるなどの問題が出てしまいました。

	例
	prog/arc/testにresouceからコピーしていたが、
	prog/arcにコピー先を変更した。
	しかしprog/arc/testのnaixをincludeしたままになっていた。
	prog/arcの中のnaixが消されないために、makeが通ってしまうが、
	新規環境を作った場合prog/arc/testには作られないためmake errorになった。

	これを防止するためにprog/arcの中のnarc,naixもmake cleanの対象にすることに
	なりました。


●このフォルダにnarcを追加する方へ
	makefile内で全てのnarcとnaixを集めているため、makefileに書き加える必要は
	ありません。
	もしnarc,naix以外のファイルを追加する場合は適宜ご変更下さい。
