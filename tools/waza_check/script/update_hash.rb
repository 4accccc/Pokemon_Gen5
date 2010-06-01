require "wazaoboe_lv_hash_conv.rb"
require "kowaza_hash_conv.rb"
require "machine_wazaname_hash_conv.rb"
require "machinewaza_hash_conv.rb"
require "oshiewaza_hash_conv.rb"
require "evolve_hash_conv.rb"
require "egg_group_hash_conv.rb"

CreateMachineWazaNameHash( "../data/wb_item.txt", "./hash/" )
CreateMachineWazaHash( "../data/personal_wb.csv", "./hash/" )
CreateKowazaHash( "../data/kowaza_table.txt", "./hash/" )
CreateWazaOboeLvHash( "../data/personal_wb.csv", "./hash/" )
CreateOshieWazaHash( "../data/personal_wb.csv", "./hash/" )
CreateEvolveHash( "../data/personal_wb.csv", "./hash/" )
CreateEggGroupHash( "../data/personal_wb.csv", "./hash/" )
