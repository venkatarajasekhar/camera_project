#ifndef _LINUX_LIMITS_H
#define _LINUX_LIMITS_H

#define NR_OPEN	        1024

#define NGROUPS_MAX       32	/* supplemental group IDs are available */
#define ARG_MAX       131072	/* # bytes of args + environ for exec() */
#define CHILD_MAX        999    /* no limit :-) */
#define OPEN_MAX         256	/* # open files a process may have */
#define LINK_MAX         127	/* # links a file may have */
#define MAX_CANON        255	/* size of the canonical input queue */
#define MAX_INPUT        255	/* size of the type-ahead buffer */
#define NAME_MAX         255	/* # chars in a file name */
#define PATH_MAX        4096	/* # chars in a path name including nul */

#ifdef CONFIG_PIPE_SIZE
#define PIPE_BUF    (PAGE_SIZE * (1 << CONFIG_PIPE_SIZE_ORD))
#else
#define PIPE_BUF        4096	/* # bytes in atomic write to a pipe */
#endif

#define RTSIG_MAX	  32

#endif
