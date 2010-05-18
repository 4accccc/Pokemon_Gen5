bin_str = ARGV[0] + ".bin"
prm_str = ARGV[0] + ".prm"

org = File.open(bin_str,"ab")
add = File.open(prm_str,"rb")

org.write(add.read)

add.close
org.close

