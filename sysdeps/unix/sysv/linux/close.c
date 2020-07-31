/* Linux close syscall implementation.
   Copyright (C) 2017-2020 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <sysdep-cancel.h>
#include <not-cancel.h>
#include <sys/syscall.h>
#include <pthread.h>

#include "../../../retrace/retrace-lib.h"

extern __thread Retrace_Log rlog;
extern __thread IntPair* fd_pair;

/* Close the file descriptor FD.  */
int
__close (int fd)
{
  pthread_t thread_id = 0;
  int ret_val = -1;
  size_t syscall = __NR_close;
  time_t cur_time;
  
  if (rlog.mode == Retrace_Record_Mode) 
  {
    rlog.mode = Retrace_Disabled_Mode;
    
    thread_id = pthread_self();
    cur_time = time(NULL);

    RLog_Push(&rlog, &syscall, sizeof(syscall));
    RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
    RLog_Push(&rlog, &cur_time, sizeof(cur_time));

    ret_val = SYSCALL_CANCEL (close, fd);

    RLog_Push(&rlog, &ret_val, sizeof(ret_val));

    IntPair* pair = Find_IntPair(fd_pair, fd);

    if(NULL == pair)
    {
        fprintf(stderr, "IntPair doesn't found!\n");
        abort();
    }

    if(-1 == close(pair->value))
    {
        fprintf(stderr, "Closing error!\n");
        abort();
    }

          
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

      ret_val = SYSCALL_CANCEL (close, fd);

      rlog.mode = Retrace_Replay_Mode;
  }
  else
  {
      return SYSCALL_CANCEL (close, fd);
  }  

  return ret_val;
}
libc_hidden_def (__close)
strong_alias (__close, __libc_close)
weak_alias (__close, close)
