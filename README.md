# Backup-File-System
* This C program uses multithreading and recursion to create a .backup folder with backups of every regular file/folder in a directory, overwriting previous backups within the .backup folder if modifications to a file are newer then the existing backup file. If ran with -r argument, all backup files in .backup are copied into the current working directory in a similar fashion.
* The ./BackItUp executable can be made by running make.
* Error checking is in place for all major system calls, as well as a #define DEBUG flag (0 = off, 1 = on) to see which directories/files are being created/restored.
# Backing Up
* Running the ./BackItUp executable with no arguments will back up all files in the directory (besides a preexisting .backup folder) into a .backup folder.
* BackItUp creates a directory .backup if one does not already exist. Files and directories that start with a '.' are hidden and in order to see them in the terminal you must use the ls -a command.
* BackItUp then creates a copy of all the regular files in the current working directory in the .backup directory. All the backup files have .bak appended to their name.
* If a backup file already exists within .backup, the program compares the last modification times of the original file and the backup file, and determines if a backup is required. If the existing .bak backup file is older, then the program overwrites it and prints out a warning when doing so. Otherwise, it does not overwrite the backup file, and notifies the user that the backup file is already the most current version.
* The BackItUp program allocates a new thread to copy each file. Therefore, if the current working directory contains five files, then the program creates five threads to backup each file. The program waits until all of the threads have finished (joined) before exiting.
* Subdirectories within the current working directory are recursively handled, with each directory that is read being created within the .backup folder. Each regular file that is read in this process is inserted into a fplist struct, so that all threads to create files can be made at once.
# Restoring
* Running the ./BackItUp executable with the -r argument will restore all files within the local .backup folder (besides the .backup folder itself) into the current working directory.
* The same functions used for backing up are also utilized for the restoring process with minor modifications through the use of a flag within the functions and within the fplist struct (0 = backing up, 1 = restoring).
* Like backing up restoring also recursively traverses the .backup folder, creating any directory that is read in the current working directory (without .bak), and storing any regular files read in a fplist struct.
* The BackItUp -r program then allocates a new thread to restore each file stored within the fplist struct without .bak, copying from the .backup folder and printing out a warning if it is overwriting a local file.
* Again, the restoring process does not restore any files that have a later modification time than the backed up copy, printing out a notice if the local file is already the most current version.
* Error checking is in place to not accidently overwrite the local ./BackItUp executable with a backed up copy, with a notice being printed if the user attempts to do so.
# Screenshots
## Backing Up
![Alt text](/screenshots/backup1.png?raw=true "backup1")
![Alt text](/screenshots/backup2.png?raw=true "backup2")
![Alt text](/screenshots/backup3.png?raw=true "backup3")
![Alt text](/screenshots/backup4.png?raw=true "backup4")
## Restoring
![Alt text](/screenshots/restore1.png?raw=true "restore1")
![Alt text](/screenshots/restore2.png?raw=true "restore2")
![Alt text](/screenshots/restore3.png?raw=true "restore3")
![Alt text](/screenshots/restore4.png?raw=true "restore4")
## ./BackItUp -r Error Checking
* If the user backs up the entire directory, restoring would cause the local executable to be overwritten...
![Alt text](/screenshots/both1.png?raw=true "both1")
* But instead a notice is printed, and the local executable is not overwritten.
![Alt text](/screenshots/both2.png?raw=true "both2")