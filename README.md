# kya_dl_bruteforce_checksum
Bruteforce all possible start and ending positions of a given known checksum for a Kya: Dark Lineage save file (and possibly any Eden Games game using the same base libraries)

# Usage
bfcsum(.exe) /path/to/save/file.dat \<h\>known\_hash \<--start=\<h\>start\_offset\> \<--stop=\<h\>stop\_offset\>  
Values are read as decimal by default, prepend lowercase "h" to have one read as hexadecimal  
The program will print the starting offset every time it's increased (meaning it has tried all combinations to start:start+1 to start:EOF)  
Upon finding a matching checksum, it will print the checksum, the starting offset, the ending offset, and exit  
## Return codes
*0: match found*  
*1: error opening file*  
*2: missing argument(s)*  
*3: program finished succesfully, but no match found*  
*4: value(s) of argument(s) out of range*  
*5: invalid argument(s)*  
