cd data/conv_gra

nnsarc -c -i -n balance_ball.narc *.*

move *.narc ../../
move *.naix ../../

cd ..
rd /q /s conv_gra
cd ..

