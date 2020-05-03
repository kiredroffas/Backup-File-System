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
#define MAX_PATH_LENGTH 4096 // Max filepath length
#define DEBUG 0              // 0 = off, 1 = on

// Linked list stucture to store filepaths
typedef struct fplist{
    char *filepath;  // filepath to be backed up/restored
    int flag;        // 0 = backing up, 1 = restoring
    int threadNum;   // Identifying thread number
    int bytesCopied; // Number of bytes copied from file to backup
    struct fplist *next; // Pointer to next filepath
} fplist;

// Create a directory called .backup if one does not already exist
void createBackup() {
    const char *folder = ".backup";
    struct stat f;
    // If .backup directory already exists, no action is needed
    if(stat(folder, &f) == 0 && S_ISDIR(f.st_mode)) {
        if(DEBUG) { printf(".backup directory already exists\n\n"); }
    }
    // Else if backup directory does not exist, create a new .backup/ folder 
    else {
        if(DEBUG) { printf(".backup directory does not exist, creating new .backup\n\n"); }
        if(mkdir(".backup", 0700) != 0) {
            perror(".backup mkdir");
        }
    }
}

// Create a sentinel node for the beginning of the filepath linked list
fplist *createfplistSent() {
    fplist *sent = (fplist *)malloc(sizeof(fplist));
    sent->filepath = malloc(strlen("sentinel") + 1);
    strcpy(sent->filepath, "sentinel\0");
    sent->next = NULL;
    return(sent);
}

// Add a recursively read filepath to the fplist struct, and return the linked filepath
fplist *addList(fplist *prevfp, char *filePath) {
    // Due to recursive call, sometimes the passed fplist node wont be at the end of the list
    while(prevfp->next != NULL) {
        prevfp = prevfp->next;
    }
    // Create a new fplist filepath node
    fplist *p = (fplist *)malloc(sizeof(fplist));
    p->filepath = malloc(strlen(filePath) + 1);
    // Copy the filepath into the node
    strcpy(p->filepath, filePath);
    strcat(p->filepath, "\0");
    // Point the previous node to the newly created node
    prevfp->next = p;
    p->next = NULL;
    return(p);
}

// Create the name of the backup file with .backup appended into path
char *createBackupPath(char *f, char *backupPath, int flag) {
    // Get the current working directory to parse out of filePath
    char cwd[MAX_PATH_LENGTH];
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("createBackupPath getcwd");
    }
    // Make a copy of f->filepath so strtok does not destroy it
    char filepathCopy[MAX_PATH_LENGTH]; 
    strcpy(filepathCopy, f);
    char *token = strtok(filepathCopy, "/");

    // If flag = 0: Parse through filepath until it matches cwd and append ./backup
    if(flag == 0) {
        // Keep copying tokens while / delimiter is present in filepath 
        while (token != NULL) { 
            strcat(backupPath, "/");
            strcat(backupPath, token);
            if(strcmp(cwd, backupPath) == 0) {
                break;
            }
            token = strtok(NULL, "/");   
        } 
        // Append /.backup to the copied matching cwd backup path
        strcat(backupPath, "/.backup");
    }
    // Else if flag = 1: Parse through filepath until /.backup and then append the remaining filepath to the cwd
    else if(flag == 1) {
        // Keep copying tokens while / delimiter is present in filepath 
        while (token != NULL) { 
            if(strcmp(token, ".backup") == 0) {
                break;
            }
            token = strtok(NULL, "/");   
        } 
        // Copy the cwd to the backupPath
        strcpy(backupPath, cwd);
    }
    // Continue copying the rest of the original filepath into backup path
    token = strtok(NULL, "/");
    // Keep copying tokens while / delimiter is present in filepath 
    while(token != NULL) {
        strcat(backupPath, "/");
        strcat(backupPath, token);
        token = strtok(NULL, "/"); 
    }
    return(backupPath);
}

// Recursively list and add readable files to fplist struct, creating directories in .backup
int listFiles(char *directory, fplist *p, int flag) {
    // Check access to see if the directory/file exists (F_OK)
    if(access(directory,F_OK) != 0) { 
        fprintf(stderr,"error = %d : %s, %s doesn't exist\n",errno,strerror(errno), directory);
        return(0);
    }
    // Check access to see if the directory/file is readable (R_OK)
    if(access(directory,R_OK) != 0) { 
        fprintf(stderr,"error = %d : %s, %s isn't readable\n",errno,strerror(errno), directory);
        return(0);
    }
    if(DEBUG) { printf("--- Directory: %s\n" ,directory); }
    // flag = 0: If backing up dont want to backup cwd in .backup folder, so get cwd to compare with
    char cwd[MAX_PATH_LENGTH];
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("listFiles getcwd");
    }
    // flag = 1: If restoring don't want to restore the .backup folder, so append .backup to compare with
    if(flag == 1) {
        strcat(cwd, "/.backup");
    }

    // flag = 0: If directory isn't cwd, create new copy of directory in .backup
    // flag = 1: If directory isn't .backup folder create new copy of backup directory in cwd
    if(strcmp(directory, cwd) != 0) {
        // Create the name of the directory
        char *backupPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
        // Prevent conditional jump by intializing values
        memset(backupPath, '\0', sizeof(char) * MAX_PATH_LENGTH );
        // If flag = 0: Add /.backup into filepath
        if(flag == 0) {
            backupPath = createBackupPath(directory, backupPath, 0);
        }
        // If flag = 1: Remove /.backup from the filepath and append to cwd
        else if(flag == 1) {
            backupPath = createBackupPath(directory, backupPath, 1);
        }
        strcat(backupPath, "\0");
        struct stat f;
        // If backupPath directory already exists, no action is needed
        if(stat(backupPath, &f) == 0 && S_ISDIR(f.st_mode)) {
            if(DEBUG) { printf("%s directory already exists\n", backupPath); }
        }
        // Else if backupPath directory does not exist, it must be created 
        else {
            if(DEBUG) { printf("%s directory does not exist, creating\n", backupPath); }
            if(mkdir(backupPath, 0700) != 0) {
                perror("backupPath mkdir");
            }
        }
        free(backupPath);
    }
    else {
        if(DEBUG) {
            if(flag == 0) {
                printf("Not backing up cwd\n");
            }
            else if(flag == 1) {
                printf("Not restoring .backup\n");
            }
        }
    }

    DIR *dir;                   // Directory pointer
    struct dirent *entry;       // Structure with info referring to a directory entry
    char filePath[MAX_PATH_LENGTH]; // Buffer to hold filepaths
    int firstRun = 0;           // Flag to determine if it is the first filepath being recursivly read
    fplist *newp;               // fplist pointer to append new filepaths to the fplist struct

    // Attempt to open directory
    if( (dir = opendir(directory)) == NULL ) { 
        fprintf(stderr,"error = %d : %s, couldn't read %s\n",errno,strerror(errno), directory);
        return(0);
    }
    // Attempt to read file(s) in directory
    while( (entry = readdir(dir)) != NULL ) { 
        // If current read directory is current working directory/parent directory/.backup folder
        if( strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0 || strcmp(entry->d_name,".backup") == 0 || strcmp(entry->d_name,".git") == 0) {
            continue;  // Skip through the remaining part of the while loop for a single iteration, continue while with next file value
        }              
        // Append next file in directory to end of filepath
        snprintf(filePath,sizeof(filePath) - 1,"%s/%s",directory,entry->d_name);
        // If the file is a regular file
        if(entry->d_type == DT_REG) { 
            // And the file is readable
            if(access(filePath,R_OK) == 0) {  
                // If its the first read file append to the sentinel within struct fplist
                if(firstRun == 0) {
                    newp = addList(p, filePath);
                    firstRun++;
                }
                // Else if its the second read file onwards append to the previous read filepath in struct fplist
                else {
                    newp = addList(newp, filePath);
                    firstRun++;
                }
            }
            else {
                fprintf(stderr, "error: file %s isn't readable\n",filePath);
            }
        }
        // Else if the file is a directory
        else if(entry->d_type == DT_DIR) { 
            // If its the first read directory call listFiles again with the sentinel node
            if(firstRun == 0) {
                if(flag == 0) {
                    listFiles(filePath, p, 0);
                }
                else if(flag == 1) {
                    listFiles(filePath, p, 1);
                }
            }
            // Else if its the second read directory onwards call listFiles again with the previous filepath node
            else {
                if(flag == 0) {
                    listFiles(filePath, newp, 0);
                }
                else if(flag == 1) {
                    listFiles(filePath, newp, 1);
                }
            }
        }
    }
    // Close the directory we were working with
    if(closedir(dir) == -1) { 
        fprintf(stderr,"error = %d : %s\n",errno,strerror(errno));
    }	
    return(0);
}

// Check to see if a given backupPath matches the ./BackItUp executable path
int isBackItUpExecutable(char *backupPath) {
    char compare[MAX_PATH_LENGTH];
    if(getcwd(compare, sizeof(compare)) == NULL) {
        perror("getcwd");
    }
    strcat(compare, "/BackItUp");
    if(strcmp(backupPath, compare) == 0) {
        return(1);
    }
    else {
        return(0);
    }
}

// Check to see if original or backup file is more recently modified, or if backup file does not exist
int isBackupMostRecent(char *originalFile, char *backupFile) {
    // Get stat information for both the original and backup files
    struct stat originalFile_stat;
    int ret = stat(originalFile, &originalFile_stat);
    if (ret != 0) {
        return(-1);
    }
    struct stat backupFile_stat;
    ret = stat(backupFile, &backupFile_stat);
    // Return -1 if the backup file doesnt exist yet
    if (ret != 0) {
        return(-1);
    }
    // Return 1 if the backup file is more recently modified
    if(backupFile_stat.st_mtime > originalFile_stat.st_mtime) {
        return(1);
    }
    // Return 0 if the original file is more recently modified
    else {
        return(0);
    }
}

// Thread function is passed a filePath to copy into the .backup folder/current working directory
void *copyFile(void *file) {
    fplist *f = file;
    // Create the name of the backup file
    char *backupPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
    // Prevent conditional jump by intializing values
    memset(backupPath, '\0', sizeof(char) * MAX_PATH_LENGTH );
    // If backing up:
    if(f->flag == 0) {
        // Append /.backup into the filepath
        backupPath = createBackupPath(f->filepath, backupPath, 0);
        // Append .bak to the end of the backup filepath
        strcat(backupPath, ".bak\0");
    }
    // Else if restoring:
    else if(f->flag == 1) {
        // Remove /.backup from the filepath
        backupPath = createBackupPath(f->filepath, backupPath, 1);
        // Remove .bak from the filepath by setting null term 4 charaters from end of string
        backupPath[strlen(backupPath)-4] = 0;
    }
    // Check to see if the backupPath exists/is newer then the original filepath
    int ret = isBackupMostRecent(f->filepath, backupPath);
    // If backup file is more recently modified, no need to copy into .backup/cwd
    if(ret == 1) {
        if(f->flag == 0) {
            printf("[thread %d] NOTICE: %s is already the most current version\n", f->threadNum, f->filepath);
        }
        else if(f->flag == 1) {
            printf("[thread %d] NOTICE: %s is already the most current version\n", f->threadNum, backupPath);
        } 
    }
    // Else if original file is more recently modified or backup isnt created yet, copy into .backup/cwd
    else if(ret == 0 || ret == -1) {
        // If restoring, dont want to restore/overwrite the local ./BackItUp executable
        // Can cause undefined errors if the executable is overwritten mid process
        if(f->flag == 1) {
            int exception = isBackItUpExecutable(backupPath);
            if(exception == 1) {
                printf("[thread %d] NOTICE: Not overwriting the current version of %s\n", f->threadNum, backupPath);
                free(backupPath);
                return(0);
            }
        }
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
            fprintf(stderr, "[thread %d] fopen error with %s\n", f->threadNum, backupPath);
            perror("copyFile inputFile fopen");
            free(backupPath);
            return(0);
        }
        if((outputFile = fopen(backupPath, "wb")) == NULL) {
            fprintf(stderr, "[thread %d] fopen error with %s\n", f->threadNum, backupPath);
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
void createBackupThreads(fplist *sent, int flag) {
    int filePathCount = 0;  // Track how many files to copy
    // Don't want to include the sentinel in total count
    fplist *p = sent->next;
    // Get total filePath count so we know how many threads to spawn
    while(p) {
        filePathCount++;
        p = p->next;
    }
    if(DEBUG) { printf("\n"); }
    printf("Total files to be copied (threads to be created): %d\n", filePathCount);

    pthread_t threads[filePathCount]; // Pthread array to help create each filePath with a seperate thread
    int threadNumber = 1;             // Identify threads
    // Reset fplist pointer to the sentinel and skip the sentinel
    p = sent->next;
    // Copy each filePath into .backup/current working directory with a seperate thread
    while(p) {
        // Set values within the fplist node to be passed to the thread
        p->threadNum = threadNumber;
        p->flag = flag;
        p->bytesCopied = 0;
        // If backing up:
        if(flag == 0) {
            printf("[thread %d] Backing up %s\n", p->threadNum, p->filepath);
        }
        // Else if restoring:
        else if(flag == 1) {
            printf("[thread %d] Restoring %s\n", p->threadNum, p->filepath);
        }
        // Create thread for each filepath
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
    if(DEBUG) { printf("All threads have joined\n"); }

    // Reset fplist pointer and skip the sentinel
    p = sent->next;
    int totalBytes = 0;
    int copyFileCount = 0;
    // Calculate total bytes copied from each file and how many files were copied
    while(p) {
        if(p->bytesCopied != 0) {
            totalBytes += p->bytesCopied;
            copyFileCount++;
        }
        p = p->next;
    }
    printf("Successfully copied %d files (%d bytes)\n\n", copyFileCount, totalBytes);
}

// Print the fplist structure
void printList(fplist *sent) {
    fplist *p = sent;
    // Dont print the sentinel
    p = p->next;
    while(p) {
        printf("filepath: %s\n", p->filepath);
        p = p->next;
    }
}

// Free the fplist structure
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
        printf("Backing up cwd into .backup\n\n");
        // Create a directory called .backup if one does not already exist
        createBackup();
        // Create new filepath structure for read filepaths to be entered into
        fplist *sent = createfplistSent();
        // Get the current working directory to start recursing into
        char cwd[MAX_PATH_LENGTH];
        if(getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
        }
        // Recursively read and add filepaths into structure, creating directories in .backup
        listFiles(cwd, sent, 0);
        // Copy all of the regular filepaths in structure into .backup
        createBackupThreads(sent, 0);

        if(DEBUG) {
            printf("Files that should have been backed up:\n");
            printList(sent);
        }

        freeList(sent);
        return(0);
    }
    else if(argc == 2 && argv[1][0] == '-' && argv[1][1] == 'r') {
        printf("Restoring from .backup\n\n");
        // Create new filepath structure for read filepaths to be entered into
        fplist *sent = createfplistSent();
        // Get the .backup directory to start recursing into
        char backup[MAX_PATH_LENGTH];
        if(getcwd(backup, sizeof(backup)) == NULL) {
            perror("getcwd");
        }
        strcat(backup, "/.backup");
        // Recursively read and add filepaths into structure, creating directories in current working directory
        listFiles(backup, sent, 1);
        // Copy all of the regular filepaths in structure into current working directory
        createBackupThreads(sent, 1);

        if(DEBUG) {
            printf("Files that should have been restored:\n");
            printList(sent);
        }

        freeList(sent);
        return(0);
    }
    else {
        fprintf(stderr, "Back up directory with: ./BackItUp or Restore directory from .backup with: ./BackItUp -r\n");
        return(1);
    }
}
