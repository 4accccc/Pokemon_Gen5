require "mons_fullname_list_conv.rb"
require "form_name_hash_conv.rb"
require "eggwaza_hash_conv.rb"
require "levelwaza_hash_conv.rb"
require "levelwaza_learninglevel_hash_conv.rb"
require "machine_wazaname_hash_conv.rb"
require "machinewaza_hash_conv.rb"
require "teachwaza_hash_conv.rb"
require "evolve_hash_conv.rb"
require "egg_group_hash_conv.rb"
require "egg_group_poke_list_hash_conv.rb"

OutputMonsFullNameList( "../data/personal_wb.csv", "./hash/" )
OutputFormNameHash( "../data/personal_wb.csv", "./hash/" )
OutputEggWazaHash( "../data/kowaza_table.txt", "./hash/" )
OutputMachineWazaNameHash( "../data/wb_item.txt", "./hash/" )
OutputMachineWazaHash_from_PersonalDat( "../data/personal_wb.csv", "./hash/" )
OutputLevelWazaHash_form_PersonalData( "../data/personal_wb.csv", "./hash/" )
OutputLevelWazaLearningLevelHash_form_PersonalData( "../data/personal_wb.csv", "./hash/" )
OutputTeachWazaHash_from_PersonalData( "../data/personal_wb.csv", "./hash/" )
OutputEvolveHash_from_PersonalData( "../data/personal_wb.csv", "./hash/" )
OutputReverseEvolveHash_from_PersonalData( "../data/personal_wb.csv", "./hash/" )
OutputEggGroupHash( "../data/personal_wb.csv", "./hash/" )
OutputEggGroupPokeListHash( "../data/personal_wb.csv", "./hash/" )
