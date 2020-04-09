/* Erik Safford
   Backup and Restoration of File System
   Spring 2020   */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#define MAX_PATH_LENGTH 1000 //Max filepath length

typedef struct fplist{
    char *filepath;
    int type; // 0 = directory, 1 = file, 2 = sentinel
    struct fplist *next;
} fplist;

// Create a directory called .backup if one does not already exist
void createBackup() {
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
    return;
}

// Add a recursively read filepath to the fplist struct, and return the previous linked filepath
fplist *addList(fplist *prevfp, char *filePath, int type) {
    while(prevfp->next != NULL) {
        prevfp = prevfp->next;
        printf("Going to next\n");
    }
    fplist *p = malloc(sizeof(fplist));
    p->filepath = malloc(strlen(filePath) + 1);
    strcpy(p->filepath, filePath);
    p->type = type; 
    p->next = NULL;

    prevfp->next = p;
    return(p);
}

// Recursively list and add readable files and directories to fplist struct
int listFiles(char *directory, fplist *p) {
    if(access(directory,F_OK) != 0) { //Check access to see if the directory/file exists (F_OK)
		                              //0 = exists, 1 = doesnt exist
        fprintf(stderr,"error = %d : %s, %s doesn't exist\n",errno,strerror(errno), directory);
        return(0);
    }

    if(access(directory,R_OK) != 0) { //Check access to see if the directory/file is readable (R_OK)
		                              //0 = readable, 1 = not readable
        fprintf(stderr,"error = %d : %s, %s isn't readable\n",errno,strerror(errno), directory);
        return(0);
    }
	
    printf("%s\n",directory); //Print the current working directory

    fplist *newp = addList(p, directory, 0);
    printf("filepath: %s, type: %d\n", newp->filepath, newp->type);

    DIR *dir;                   //Directory pointer
    struct dirent *entry;       //Structure with info referring to a directory entry
    char filePath[MAX_PATH_LENGTH]; //Buffer to hold filepaths

    if( (dir = opendir(directory)) == NULL ) { //Attempt to open directory
        fprintf(stderr,"error = %d : %s, couldn't read %s\n",errno,strerror(errno), directory);
        return(0);
    }
	
    int firstRun = 0;
    fplist *newnewp;
    while( (entry = readdir(dir)) != NULL ) { //Attempt to read file(s) in directory
        //If current read directory is current working directory/parent directory
        if( strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0 || strcmp(entry->d_name,".backup") == 0 || strcmp(entry->d_name,".git") == 0) {
            continue;  //Skip through the remaining part of the while loop
        }              //for a single iteration, continue while with next file value
		
        //Append next file in directory to end of filepath
        //snprintf() so we don't have 'accidental' buffer overwrite
        snprintf(filePath,sizeof(filePath) - 1,"%s/%s",directory,entry->d_name);
        //sprintf(filePath,"%s/%s",directory,entry->d_name); <- not as safe
        
        //Check dirent flags of current directory/file
        if(entry->d_type == DT_REG) { //If the file is a regular file
            if(access(filePath,R_OK) == 0) {  //And the file is readable
                //Print the filepath we have/the file we checked
                printf("%s\n",filePath);
                //printf("%s/%s\n",directory,entry->d_name); <- could have done this way

                if(firstRun == 0) {
                    newnewp = addList(newp, filePath, 1);
                    firstRun++;
                    printf("filepath: %s, type: %d\n", newnewp->filepath, newnewp->type);
                }
                else {
                    firstRun++;
                    newnewp = addList(newnewp, filePath, 1);
                    printf("filepath: %s, type: %d, firstRun: %d\n", newnewp->filepath, newnewp->type, firstRun);
                }
            }
            else {
                fprintf(stderr, "error: file %s isn't readable\n",filePath);
            }
        }
        else if(entry->d_type == DT_DIR) { //Else if the file is a directory
            if(firstRun == 0) {
                listFiles(filePath, newp);
            }
            else {
                listFiles(filePath, newnewp); //Recursivly list the files in the directory
            }
        }
    }
    if( closedir(dir) == -1 ) { //Close the directory we were working with
        fprintf(stderr,"error = %d : %s\n",errno,strerror(errno));
    }	
}

void printList(fplist *sent) {
    fplist *p = sent;
    while(p) {
        printf("filepath: %s, type: %d\n", p->filepath, p->type);
        p = p->next;
    }
}

void freeList(fplist *sent) {
    fplist *tmp;
    while(sent) {
        tmp = sent->next;
        free(sent->filepath);   
        free(sent);
        sent = tmp;
    }
}

int main(int argc, char** argv) {
    // Create a directory called .backup if one does not already exist
    createBackup();

    //Recursively read in all readable/regular files/directories into filepath structure
    char directory[MAX_PATH_LENGTH];
    if(getcwd(directory, sizeof(directory)) == NULL) {
        perror("getcwd");
    }
    // Create new filepath structure for read filepaths to be entered into
    fplist *sent = malloc(sizeof(fplist));
    sent->filepath = malloc(strlen("sentinel") + 1);
    strcpy(sent->filepath, "sentinel\0");
    sent->type = 2;
    sent->next = NULL;
    listFiles(directory, sent);

    printList(sent);
    freeList(sent);
    
}