

ruby ../../tools/binlinker.rb map_a1.nsbmd map_a1.nsbtx NONE map_a1.bin map_a1.3dmd map_a1.3dppack
nnsarc -c -l -n -i test3dp -S data.list
ruby ../../tools/naixcutstamp.rb test3dp.naix
copy test3dp.naix ../../prog/arc/test_graphic
copy test3dp.narc ../../prog/arc/test_graphic
