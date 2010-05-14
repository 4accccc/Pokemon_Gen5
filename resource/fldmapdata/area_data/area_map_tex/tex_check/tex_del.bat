@echo 以下のように記述すると　テクスチャと対応するパレットがimdから削除されます。 
@echo ただし、削除するテクスチャがマテリアルに参照されていると、imdファイルが
@echo 壊れてしまいますので慎重にお願いいたします。
@echo      imd名				テクスチャ名
@echo perl tex_del.pl out39_local_texset.imd fire.2

C:\cygwin\bin\perl tex_del.pl out39_local_texset.imd isi.2
C:\cygwin\bin\perl tex_del.pl out39_local_texset.imd isi.3
C:\cygwin\bin\perl tex_del.pl out39_local_texset.imd water.2
C:\cygwin\bin\perl tex_del.pl out39_local_texset.imd water.3
C:\cygwin\bin\perl tex_del.pl out39_local_texset.imd fire.2
C:\cygwin\bin\perl tex_del.pl out39_local_texset.imd fire.3
C:\cygwin\bin\perl tex_del.pl out39_local_texset.imd fire.4
C:\cygwin\bin\perl tex_del.pl out42_local_texset.imd denki.3
C:\cygwin\bin\perl tex_del.pl out42_local_texset.imd denki.4
C:\cygwin\bin\perl tex_del.pl in53_local_texset.imd c09_03kigo.1
C:\cygwin\bin\perl tex_del.pl in53_local_texset.imd c09_03kigo.2
C:\cygwin\bin\perl tex_del.pl in39_local_texset.imd taki_shibu.2
C:\cygwin\bin\perl tex_del.pl in39_local_texset.imd taki_shibu.3
C:\cygwin\bin\perl tex_del.pl in39_local_texset.imd taki_shibu.4
C:\cygwin\bin\perl tex_del.pl in31_local_texset.imd gym08_setuzoku


pause