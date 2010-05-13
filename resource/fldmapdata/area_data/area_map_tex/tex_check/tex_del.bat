@echo 以下のように記述すると　テクスチャと対応するパレットがimdから削除されます。 
@echo ただし、削除するテクスチャがマテリアルに参照されていると、imdファイルが
@echo 壊れてしまいますので慎重にお願いいたします。
@echo      imd名				テクスチャ名
@echo perl tex_del.pl out39_local_texset.imd fire.2

perl tex_del.pl out39_local_texset.imd isi.2
perl tex_del.pl out39_local_texset.imd isi.3
perl tex_del.pl out39_local_texset.imd water.2
perl tex_del.pl out39_local_texset.imd water.3


pause