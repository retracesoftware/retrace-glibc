/* Linux open syscall implementation, LFS.
   Copyright (C) 1991-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <time.h>

#include <sysdep-cancel.h>

#include "../../../retrace/retrace-lib.h"

extern __thread Retrace_Log rlog;
extern __thread IntPair* fd_pair;

#ifdef __OFF_T_MATCHES_OFF64_T
# define EXTRA_OPEN_FLAGS 0
#else
# define EXTRA_OPEN_FLAGS O_LARGEFILE
#endif

/* Open FILE with access OFLAG.  If O_CREAT or O_TMPFILE is in OFLAG,
   a third argument is the file protection.  */
int
__libc_open64 (const char *file, int oflag, ...)
{
  int mode = 0;

  if (__OPEN_NEEDS_MODE (oflag))
  {
    va_list arg;
    va_start (arg, oflag);
    mode = va_arg (arg, int);
    va_end (arg);
  }

  pthread_t thread_id = 0;
  int ret_val = -1;
  size_t syscall = __NR_open;
  time_t cur_time;

  if (rlog.mode == Retrace_Record_Mode) 
  {
	rlog.mode = Retrace_Disabled_Mode;
		
	thread_id = pthread_self();
	cur_time = time(NULL);

	RLog_Push(&rlog, &syscall, sizeof(syscall));
	RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
	RLog_Push(&rlog, &cur_time, sizeof(cur_time));

	ret_val = SYSCALL_CANCEL (openat, AT_FDCWD, file, oflag | EXTRA_OPEN_FLAGS,
			mode);

	RLog_Push(&rlog, &ret_val, sizeof(ret_val));
	
	char recorded_file_path[FILENAME_MAX];
	
	sprintf(recorded_file_path, "%s%d", RETRACE_DIR, ret_val);
	
	struct stat stats;

	stat(RETRACE_DIR, &stats);

	if (!S_ISDIR(stats.st_mode))
	{
		if(mkdir(RETRACE_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
		{
			fprintf(stderr, "Cannot create %s dir\n", RETRACE_DIR);
			abort();
		}
	}

	int record_fd = open(recorded_file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);

	if (record_fd < 0)
	{
		fprintf(stderr, "Record file open error!\n");
		abort();
	}      

	Insert_IntPair(&fd_pair, ret_val, record_fd);

	RLog_Push(&rlog, file, strlen(file) + 1);
	RLog_Push(&rlog, recorded_file_path, strlen(recorded_file_path) + 1);

	rlog.mode = Retrace_Record_Mode;
  } 
  else if (rlog.mode == Retrace_Replay_Mode) 
  {
      rlog.mode = Retrace_Disabled_Mode;
      
      size_t fetched_syscall;

      RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));
      
      if(fetched_syscall != syscall)
      {
		  fprintf(stderr, "Fetched syscall not equal to current one!\n");
		  abort();
      }
      
      RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
      RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
	  RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

      char* file_name = malloc(strlen(file) + 1);
      
      RLog_Fetch(&rlog, file_name, strlen(file) + 1);

      if (strcmp(file, file_name))
      {
        fprintf(stderr, "Fetched filename isn't equal to current one!\n");
        abort();
      }
      
      size_t filename_len = RLog_Fetch_Length(&rlog);

      char* recorded_file_path = malloc(filename_len);

      RLog_Fetch(&rlog, recorded_file_path, filename_len);

      ret_val = open(recorded_file_path, O_RDWR);

      free(file_name);
      free(recorded_file_path);

      rlog.mode = Retrace_Replay_Mode;
  }
  else
  {
      return SYSCALL_CANCEL (openat, AT_FDCWD, file, oflag | EXTRA_OPEN_FLAGS,
			 mode);
  }  

  return ret_val;
}

strong_alias (__libc_open64, __open64)
libc_hidden_weak (__open64)
weak_alias (__libc_open64, open64)

#ifdef __OFF_T_MATCHES_OFF64_T
strong_alias (__libc_open64, __libc_open)
strong_alias (__libc_open64, __open)
libc_hidden_weak (__open)
weak_alias (__libc_open64, open)
#endif
