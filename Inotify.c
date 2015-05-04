/*
*
* A program which watches the events in the directory(s) specified in the command line.
* Command line execution: ./inotify <directory name> <directory name> ... <directory name>
*
* Created by: Asad Zia
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/inotify.h>
#include <limits.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

/* Setup inotify notifications (IN) mask. All these defined in inotify.h. */
static int event_mask =
  (IN_ACCESS |        /* File accessed */
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

/* 
*   The function for displaying a change in the directory
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

int
main(int argc, char *argv[])
{
    int inotifyFd, wd, i;
    char buf[BUF_LEN];
    ssize_t numRead;
    char *p;
    struct inotify_event *event;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        printf("%s pathname...\n", argv[0]);
		exit(5);
	}

    /* initializing inotify instance */
    inotifyFd = inotify_init ();
    if (inotifyFd == -1)
        exit(1);


      /* Loop all input directories, setting up watches */
       for (i = 1; i < argc; ++i)
        {

            wd = inotify_add_watch (inotifyFd, argv[i],event_mask);
                 if (wd == -1)
                     exit(2);

             printf("Watching %s using wd %d\n", argv[i], wd);
           
        }

    for (;;) {  /* Read events forever */
        
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
