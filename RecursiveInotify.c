/*
*
* A program which recursively watches all directories under a directory specified in the command-line.
*
* Created by: Asad zia
*
*/

#define _XOPEN_SOURCE 500	/* Need this to be declared since some linux distributions consider nftw as an extension */

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/inotify.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static int inotifyFd;	/* global parameter for the inotfiy instance */

static int traverseDirectory(const char *pathname, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
       int wd, flags;
   
       if (! S_ISDIR(sb->st_mode))
       {
           return 0;               /* Ignore nondirectory files */
   	   }

       flags =     (IN_ACCESS |        /* File accessed */
				   IN_ATTRIB |        /* File attributes changed */
				   IN_OPEN   |        /* File was opened */
				   IN_CLOSE_WRITE |   /* Writtable File closed */
				   IN_CLOSE_NOWRITE | /* Unwrittable File closed */
				   IN_CREATE |        /* File created in directory */
				   IN_DELETE |        /* File deleted in directory */
				   IN_DELETE_SELF |   /* Directory deleted */
				   IN_MODIFY |        /* File modified */
				   IN_MOVE_SELF |     /* Directory moved */
				   IN_MOVED_FROM |    /* File moved away from the directory */
				   IN_MOVED_TO);      /* File moved into the directory */

   	   
   	   /* start watching the directory */
       wd = inotify_add_watch(inotifyFd, pathname, flags | IN_ONLYDIR);

       return 0;
}

/* 
*	The function for displaying a change in the directory
*/
static void           
displayInotifyEvent(struct inotify_event *event)
{
    printf("    wd =%2d; ", event->wd);
    if (event->cookie > 0)
        printf("cookie =%4d; ", event->cookie);

    printf("mask = ");
         if (event->mask & IN_ACCESS)
            printf ("\tIN_ACCESS\n");
          if (event->mask & IN_ATTRIB)
            printf ("\tIN_ATTRIB\n");
          if (event->mask & IN_OPEN)
            printf ("\tIN_OPEN\n");
          if (event->mask & IN_CLOSE_WRITE)
            printf ("\tIN_CLOSE_WRITE\n");
          if (event->mask & IN_CLOSE_NOWRITE)
            printf ("\tIN_CLOSE_NOWRITE\n");
          if (event->mask & IN_CREATE)
            printf ("\tIN_CREATE\n");
          if (event->mask & IN_DELETE)
            printf ("\tIN_DELETE\n");
          if (event->mask & IN_DELETE_SELF)
            printf ("\tIN_DELETE_SELF\n");
          if (event->mask & IN_MODIFY)
            printf ("\tIN_MODIFY\n");
          if (event->mask & IN_MOVE_SELF)
            printf ("\tIN_MOVE_SELF\n");
          if (event->mask & IN_MOVED_FROM)
            printf ("\tIN_MOVED_FROM (cookie: %d)\n",
                    event->cookie);
          if (event->mask & IN_MOVED_TO)
            printf ("\tIN_MOVED_TO (cookie: %d)\n",
                    event->cookie);

    printf("\n");

    if (event->len > 0)
        printf("        name = %s\n", event->name);
}

/*
*	The main function
*/
int main(int argc, char *argv[])
{
    int wd, i;
    char buf[BUF_LEN];
    ssize_t numRead;
    char *p;
    struct inotify_event *event;
    int flags = FTW_CHDIR | FTW_DEPTH | FTW_MOUNT;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        printf("%s pathname...\n", argv[0]);
		exit(5);
	}

    /* initializing inotify instance */
    inotifyFd = inotify_init ();
    if (inotifyFd == -1)
        exit(1);

    /* traversing all the directories */
    nftw(argv[1], traverseDirectory, 20, flags);


    for (;;) {                                  /* Read events forever */
        numRead = read(inotifyFd, buf, BUF_LEN);
        if (numRead == 0) {
			printf("read() from inotify fd returned 0!");
            exit(4);
		}

        if (numRead == -1)
            exit(3);

        printf("Read %ld bytes from inotify fd\n", (long) numRead);

        /* Process all of the events in buffer returned by read() */

        for (p = buf; p < buf + numRead; ) {
            event = (struct inotify_event *) p;
            displayInotifyEvent(event);

            p += sizeof(struct inotify_event) + event->len;
        }
    }

    exit(EXIT_SUCCESS);
}
