# kya_dl_bruteforce_checksum
Bruteforce all possible start and ending positions of a given known checksum for a Kya: Dark Lineage save file (or possibly any Eden Games game using the same base libraries)

# Usage
bfcsum(.exe) \<h>\<filesize/auto> \<h>known\_checksum \<h>start\_offset  
filesize can be set to "auto" to have the program find out on it's own  
Values are read as decimal by default, prepend lowercase "h" to have one read as hexadecimal  
The program will print the starting offset every time it's increased (meaning it has tried all combinations to start:start√+1 to start:EOF)  
Upon finding a matching checksum, it will print the checksum, the starting offset, the ending offset, and exit
