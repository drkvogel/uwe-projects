#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

#define PAGELENGTH 22

int level = 0;

static int pagecount = 0;
int rflag = 0;
int iflag = 0;
int sflag = 0;
int tflag = 0;
int pflag = 0;

void indent(int level)
{
    int i;
    if (tflag == 1) {
        for (i=0; i<level; i++) {
            printf("\t");
        }
    }
}

void page()
{
    pagecount++;
    if (pflag == 1 && pagecount >= PAGELENGTH) 
    {
        puts("Press enter...");
        while (getchar() == NULL) {
            // do nothing
        }
        pagecount = 0;
    }
}

// error handling
void argerror(char *cmdname, int errtype, char *err)
{
    switch (errtype)
    {
        case 1:
            fprintf(stderr, "%s: no such file or directory\n", err); break;
        case 2:
            fprintf(stderr, "-%c: not an option\n", err); 
            fprintf(stderr, "\tdo %s -\? for list of options", err); break;
        case 3:
            fprintf(stderr, "%s usage: %s [-rtisp] <pathname>\n", cmdname, cmdname); break;
        case 4:
            fprintf(stderr, "%S usage: %s [-rtisp] <pathname>\n", cmdname, cmdname);
            fprintf(stderr, "\t-r: recurse subdirectories\n");
            fprintf(stderr, "\t-t: indent subdirectories\n");
            fprintf(stderr, "\t-i: show inodes\n");
            fprintf(stderr, "\t-s: show blocks\n");
            fprintf(stderr, "\t-p: page output\n");
        // default:
        //     break;
    }
    exit(EXIT_FAILURE);
} // argerror

void rls(char *curdir) // walk dirs and print file info
{
    DIR *dir;
    struct dirent *direntp;
    struct stat buf;
    char *pathname;
    char *mode;
    int i, 
        totalsize = 0,  // total of file sizes in dir
        numfiles = 0,   // total number of files
        numdirs = 0;    // total number of dirs
    
    if ((dir = opendir(curdir)) != NULL)
    {
        indent(level);
        printf("directory %s\n", curdir);
        page();

        while ((direntp = readdir(dir)) != NULL)
        {
            pathname = (char *)malloc(sizeof(char) * strlen(curdir) + strlen(direntp->d_name) + 3);
            mode = (char *)malloc(1); // ???
            sprintf(pathname, "%s/%s", curdir, direntp->d_name);

            if (stat(pathname, &buf) != -1)
            {
                if ((S_IFDIR & buf.st_mode) != 0)
                {
                    strcpy(mode, "d");
                    numdirs++;

                    // don't recurse current and parent dirs
                    if ((rflag == 1) && (strcmp(direntp->d_name, ".") != 0)
                        && strcmp(direntp->d_name, "..") != 0)
                    {
                        level++;
                        rls(pathname);
                        free(pathname);
                        printf("\n");
                        page();
                        level--;
                    }
                } // if isdir
                else
                {
                    strcpy(mode, "-");
                    totalsize += buf.st_size;
                    numfiles++;
                }
                
                indent(level);
                printf("%s ", mode);

                if (iflag == 1) // print inode
                    printf("%d\t", buf.st_ino);
                if (sflag == 1) // print size in blocks
                    printf("%d\t", buf.st_blocks);

                printf("%s\n", direntp->d_name);
                page();
            } // stat call
        } // directory read

        // summarise
        indent(level);
        printf("%d blocks, %d files and %d dirs\n", totalsize, numfiles, numdirs);
    } // directory open
    else
    {
        argerror("rls", 1, curdir); // no such dir ???
    }
} // rls

main (int argc, char * argv[])
{
    char *startdir;
    int i;

    switch (argc)
    {
        case 1:
            strcpy(startdir, "."); break; // default dir is '.'
        case 3:
            if ((char)argv[1][0] != '-')
            {
                argerror(argv[0], 3, (char *)""); // usage error - more than one dir specified
            }
            else
                startdir =  argv[2];

        /* read options in next case statement... */
        case 2: if ((char) argv[1][0] == '-')
            {
                for (i=1; i<strlen(argv[1]); i++)
                {
                    switch ((char) argv[1][1])
                    {
                        case 'r': rflag = 1; break; // recurse subdirectories
                        case 'i': iflag = 1; break; // show inodes
                        case 's': sflag = 1; break; // show blocks ???
                        case 't': tflag = 1; break; // indent subdirectories
                        case 'p': pflag = 1; break; // page output
                        
                        // help option
                        case '\?': argerror(argv[0], 4, (char *)""); break;

                        // option error
                        default: argerror(argv[0], 2, (char *)argv[1][1]); break;
                    }
                }
                if (argc == 2)
                    strcpy(startdir, ".");
                break;
            }
            else
                startdir = argv[1]; break;
            
    
        default: argerror(argv[0], 3, (char *)""); // usage error
    }
    rls(startdir);
}