■ポケモンリスト用パーティクルデータコンバート手順

by 2011.06.01 iwasawa


1, pokelist/particle/spr 及び pokelist/particle/spt フォルダに素材を登録

2, C:\home\swan\tools\spaconv\SPAConv115\生成ファイル出力場所 フォルダが空であることを確認
   (空でなければ空にする)

3, SPAコンバーター C:\home\swan\tools\spaconv\SPAConv115\SPAConv.exe を起動

4, 「開くファイル」として pokelist/particle/pokelist_particle_lst.txt を指定
    (ドラッグ&ドロップでも可。「再読み込み」で再実行可能です)

5, コンバートが開始され、生成されたファイルが下記フォルダに出力されているのを確認
   ->C:\home\swan\tools\spaconv\SPAConv115\生成ファイル出力場所

6,  SPAコンバーターで「転送先1」に　C:\home\swan\resource\pokelist　フォルダを指定し
    「転送先1の場所へファイル転送」ボタンを押すとファイルがコピーされる

7, swanトップからmakeをかける(アーカイブを作り直すだけなら resource/pokelistフォルダでmakeでも可)

※SPAコンバーターから出力された *.spa ファイルは、素材に変更がなくても
  コンバートしなおすたびにバイナリ差分がでるので注意してください

※SPAコンバーターの詳しい使い方についてはC:\home\swan\tools\spaconv\SPAConv115\readme.txtを参照のこと

※コンバートする素材を増やしたい場合、pokelist_particle_lst.txt　を編集してください

※pokelist_particle_lst.datがpokelist_particle_lst.hと共に生成されますが
  現状プログラムでは使用していませんので、無視してください。
  SVNでは無視ファイル登録してあります。



