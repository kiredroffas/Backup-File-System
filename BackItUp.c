/* Erik Safford
   Backup and Restoration of File System
   Spring 2020   */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char** argv) {
    // Create a directory called .backup if one does not already exist
    const char *folder = ".backup";
    struct stat sb;
    
    // If .backup directory already exists, no action is needed
    if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        printf(".backup directory already exists\n");
    }
    // Else if backup directory does not exist, create a new .backup/ folder 
    else {
        //perror(".backup stat");
        printf(".backup directory does not exist, creating new .backup\n");
        if(mkdir(".backup", 0700) != 0) {
            perror(".backup mkdir");
        }
    }

}