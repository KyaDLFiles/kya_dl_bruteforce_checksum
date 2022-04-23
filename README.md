# kya_dl_bruteforce_checksum
Bruteforce all possible start and ending positions of a given known checksum for a Kya: Dark Lineage save file (or possibly any Eden Games game using the same base libraries)

# Usage
bfcsum(.exe) <h><filesize/auto> <h>known_checksum <h>start  
filesize can be set to "auto" to have the program find out on it's own  
Values are read as decimal by default, prepend lowercase "h" to have one read as hexadecimal
