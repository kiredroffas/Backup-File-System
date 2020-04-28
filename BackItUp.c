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
#include <pthread.h>
#define MAX_PATH_LENGTH 1024 //Max filepath length

typedef struct fplist{
    char *filepath;
    int type; // 0 = sentinel, 1 = file
    int threadNum;  //Identifying thread number
    int bytesCopied; //Number of bytes copied from file to backup
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
        //printf("Going to next\n");
    }
    fplist *p = (fplist *)malloc(sizeof(fplist));
    p->filepath = malloc(strlen(filePath) + 1);
    strcpy(p->filepath, filePath);
    strcat(p->filepath, "\0");
    p->type = type; 
    p->next = NULL;

    prevfp->next = p;
    return(p);
}

// Create the name of the backup file with .backup appended into path
char *createBackupPath(char *f, char *backupPath) {
    // Get the current working directory to parse out of filePath
    char directory[MAX_PATH_LENGTH];
    if(getcwd(directory, sizeof(directory)) == NULL) {
        perror("createBackupPath getcwd");
    }
    //printf("directory: %s\n", directory);

    // Iterate and copy filepath into backup path until it matches cwd
    // Make a copy of f->filepath so strtok does not destroy it
    char filepathCopy[MAX_PATH_LENGTH]; 
    strcpy(filepathCopy, f);
    char *token = strtok(filepathCopy, "/");
    // Keep copying tokens while / delimiter is present in filepath 
    while (token != NULL) { 
        //printf("token: %s\n", token); 
        strcat(backupPath, "/");
        strcat(backupPath, token);
        //printf("backup path now: %s\n", backupPath);
        if(strcmp(directory, backupPath) == 0) {
            //printf("match\n");
            break;
        }
        token = strtok(NULL, "/");   
    } 
    // Append /.backup to the copied matching cwd backup path
    strcat(backupPath, "/.backup");
    // Continue copying the rest of the original filepath into backup path
    token = strtok(NULL, "/");
    // Keep copying tokens while / delimiter is present in filepath 
    while( token != NULL) {
        //printf("token: %s\n", token); 
        strcat(backupPath, "/");
        strcat(backupPath, token);
        //printf("backup path now: %s\n", backupPath);
        token = strtok(NULL, "/"); 
    }
    //printf("backup path now: %s\n", backupPath);
    return(backupPath);
}

// Recursively list and add readable files to fplist struct, creating directories in .backup
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
	
    printf("--- %s\n" ,directory);
    // Dont want to backup cwd in .backup folder, so get cwd to compare with
    char cwd[MAX_PATH_LENGTH];
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("listFiles getcwd");
    }

    // If directory isn't cwd, create new copy of directory in .backup
    if(strcmp(directory, cwd) != 0) {
        char *backupPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
        memset(backupPath, '\0', sizeof(char) * MAX_PATH_LENGTH );
        backupPath = createBackupPath(directory, backupPath);
        strcat(backupPath, "\0");

        int ret;
        ret = mkdir(backupPath, 0700);
        if(ret != 0) {
            fprintf(stderr, "copyFile mkdir %s\n", backupPath);
            perror("mkdir");
        }
        else {
            printf("Created %s\n", backupPath);
        }
        free(backupPath);
    }
    else {
        printf("Not backing up cwd\n");
    }
    
    printf("%s\n",directory); //Print the current working directory

    DIR *dir;                   //Directory pointer
    struct dirent *entry;       //Structure with info referring to a directory entry
    char filePath[MAX_PATH_LENGTH]; //Buffer to hold filepaths

    if( (dir = opendir(directory)) == NULL ) { //Attempt to open directory
        fprintf(stderr,"error = %d : %s, couldn't read %s\n",errno,strerror(errno), directory);
        return(0);
    }
	
    int firstRun = 0;
    fplist *newp;
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
                    newp = addList(p, filePath, 1);
                    firstRun++;
                    //printf("filepath: %s, type: %d\n", newnewp->filepath, newnewp->type);
                }
                else {
                    newp = addList(newp, filePath, 1);
                    firstRun++;
                    //printf("filepath: %s, type: %d, firstRun: %d\n", newnewp->filepath, newnewp->type, firstRun);
                }
            }
            else {
                fprintf(stderr, "error: file %s isn't readable\n",filePath);
            }
        }
        else if(entry->d_type == DT_DIR) { //Else if the file is a directory
            if(firstRun == 0) {
                listFiles(filePath, p);
            }
            else {
                listFiles(filePath, newp); //Recursivly list the files in the directory
            }
        }
    }
    if(closedir(dir) == -1) { //Close the directory we were working with
        fprintf(stderr,"error = %d : %s\n",errno,strerror(errno));
    }	
    return(0);
}

fplist *createfplistSent() {
    fplist *sent = (fplist *)malloc(sizeof(fplist));
    sent->filepath = malloc(strlen("sentinel") + 1);
    strcpy(sent->filepath, "sentinel\0");
    sent->type = 0;
    sent->next = NULL;
    return(sent);
}

int isBackupMostRecent(char *originalFile, char *backupFile) {
    struct stat originalFile_stat;
    int ret = stat(originalFile, &originalFile_stat);
    if (ret != 0) {
        //perror("isBackupMostRecent originalFile stat");
        return(-1);
    }

    struct stat backupFile_stat;
    ret = stat(backupFile, &backupFile_stat);
    // Return -1 if the backup file doesnt exist yet
    if (ret != 0) {
        //perror("isBackupMostRecent backupFile stat");
        return(-1);
    }
    
    // Return 1 if the backup file is more recently modified
    if(backupFile_stat.st_mtime > originalFile_stat.st_mtime) {
        //printf("Original: %ld, Backup: %ld, backup most recent, return 1\n", originalFile_stat.st_mtime, backupFile_stat.st_mtime);
        return(1);
    }
    // Return 0 if the original file is more recently modified
    else {
        //printf("Original: %ld, Backup: %ld, original most recent, return 0\n", originalFile_stat.st_mtime, backupFile_stat.st_mtime);
        return(0);
    }
}

// Thread function is passed a filePath to copy into the .backup folder with .bak appended
void *copyFile(void *file) {
    fplist *f = file;
    //printf("copyFile::: filepath: %s, threadNum: %d\n", ((struct fplist *)file)->filepath, ((struct fplist *)file)->threadNum);
    //printf("copyFile::: filepath: %s, threadNum: %d\n", f->filepath, f->threadNum);

    // Create the name of the backup file with .backup appened
    char *backupPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
    memset(backupPath, '\0', sizeof(char) * MAX_PATH_LENGTH );
    backupPath = createBackupPath(f->filepath, backupPath);
    
    //Append .bak to the end of the backup file
    strcat(backupPath, ".bak\0");
    //printf("copyFile: filepath: %s, backuppath: %s\n", f->filepath, backupPath);

    // Check to see if the backupPath exists/is newer then the original filepath
    int ret = isBackupMostRecent(f->filepath, backupPath);
    //printf("[thread %d] copyFile ret = %d\n", f->threadNum, ret);
    // If backup file is more recently modified, no need to copy into .backup
    if(ret == 1) {
        printf("[thread %d] NOTICE: %s is already the most current version\n", f->threadNum, f->filepath);
        f->bytesCopied = 0;
    }
    // Else if original file is more recently modified or backup isnt created yet, copy into .backup
    else if(ret == 0 || ret == -1) {
        // If backup file already exists in .backup, print warning
        if(ret == 0) {
            printf("[thread %d] WARNING: Overwriting %s\n", f->threadNum, backupPath);
        }
        char buffer[MAX_PATH_LENGTH];
        size_t bytes;
        int totalBytes = 0;
        FILE *inputFile, *outputFile;

        // Attempt to open input/output files
        if((inputFile = fopen(f->filepath, "rb")) == NULL) {
            perror("copyFile inputFile fopen");
            free(backupPath);
            return(0);
        }
        if((outputFile = fopen(backupPath, "wb")) == NULL) {
            perror("copyFile outputFile fopen");
            free(backupPath);
            return(0);
        }

        // Copy the inputFile into the outputFile
        while ((bytes = fread(buffer, 1, MAX_PATH_LENGTH, inputFile)) != 0) {
            if(fwrite(buffer, 1, bytes, outputFile) != bytes) {
                fprintf(stderr, "[thread %d] copyFile fwrite wrote %ld bytes\n", f->threadNum, bytes);
                free(backupPath);
                return(0);
            }
            totalBytes += bytes;
        }
        
        // Set the bytesCopied part of fplist struct to number of bytes copied from file
        f->bytesCopied = totalBytes;
        printf("[thread %d] Copied %d bytes from %s to %s\n", f->threadNum, f->bytesCopied, f->filepath, backupPath);

        // Close the opened files and return
        if(fclose(inputFile) != 0) {
            perror("copyFile inputFile fclose");
            free(backupPath);
            return(0);
        }
        if(fclose(outputFile) != 0) {
            perror("copyFile outputFile fclose");
            free(backupPath);
            return(0);
        }
    }

    free(backupPath);
    return(0);
}

// Create threads for file copying, first checking if they are the most recent version
void createBackupThreads(fplist *sent) {
    int filePathCount = 0;
    fplist *p = sent;
    // Don't want to include the sentinel in total count
    p = p->next;
    // Get total filePath count so we know how many threads to spawn
    while(p) {
        //printf("filepath: %s ... type: %d\n", p->filepath, p->type);
        filePathCount++;
        p = p->next;
    }
    printf("Total files to be copied (threads to be created): %d\n", filePathCount);

    //Create pthread array to help create each filePath with a seperate thread
    pthread_t threads[filePathCount];
    int threadNumber = 1;
    
    // Reset fplist pointer to the sentinel
    p = sent;
    // Skip the sentinel
    p = p->next;
    // Copy each filePath into .backup with a seperate thread
    while(p) {
        p->threadNum = threadNumber;
        //printf("filepath: %s, threadNum: %d\n", p->filepath, p->threadNum);
        printf("[thread %d] Backing up %s\n", p->threadNum, p->filepath);
        if(pthread_create(&threads[threadNumber-1], NULL, copyFile, p) != 0) {
            perror("createBackupThreads pthread_create error");
        }
        threadNumber++;
        p = p->next;
    }
    //Wait until all threads have exited (joined)
    for(int i = 0; i < filePathCount; i++) {
        if(pthread_join(threads[i], NULL) != 0) {
            perror("createBackupThreads pthread_join error");
        } 
    }
    printf("All threads have joined\n");

    // Rest fplist pointer to the sentinel
    p = sent;
    // Skip the sentinel
    p = p->next;
    int totalBytes = 0;
    int copyFileCount = 0;
    while(p) {
        if(p->bytesCopied != 0) {
            totalBytes += p->bytesCopied;
            copyFileCount++;
        }
        p = p->next;
    }
    printf("Successfully copied %d files (%d bytes)\n", copyFileCount, totalBytes);
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
    if(argc == 1) {
        // Create a directory called .backup if one does not already exist
        createBackup();

        // Recursively read in all readable regular files/directories into filepath structure
        // Create new filepath structure for read filepaths to be entered into
        fplist *sent = createfplistSent();
        // Get the current working directory to start recursing into
        char directory[MAX_PATH_LENGTH];
        if(getcwd(directory, sizeof(directory)) == NULL) {
            perror("getcwd");
        }
        // Recursively read and add filepaths into structure, creating directories in .backup
        listFiles(directory, sent);

        // Copy all of the regular filepaths in structure into .backup
        createBackupThreads(sent);

        //printList(sent);
        freeList(sent);
        return(0);
    }
    else if(argc == 2 && argv[1][0] == '-' && argv[1][1] == 'r') {
        printf("Restoring from backup\n");
        return(0);
    }
    else {
        fprintf(stderr, "Back up directory with: ./BackItUp or Restore directory from .backup with: ./BackItUp -r\n");
        return(1);
    }
}