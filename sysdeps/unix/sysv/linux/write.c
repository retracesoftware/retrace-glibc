/* Linux write syscall implementation.
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
#include <sys/syscall.h>
#include <pthread.h>

#include "../../../retrace/retrace-lib.h"


extern __thread Retrace_Log rlog;
extern __thread IntPair* fd_pair;

/* Write NBYTES of BUF to FD.  Return the number written, or -1.  */
ssize_t
__libc_write (int fd, const void *buf, size_t nbytes)
{
    int ret_val = -1;
    size_t syscall = __NR_write;
    time_t cur_time = 0;
    pthread_t thread_id = 0;

    if(fd < 3)
        return SYSCALL_CANCEL (write, fd, buf, nbytes);

    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall, sizeof(syscall));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        
        ret_val = SYSCALL_CANCEL (write, fd, buf, nbytes);

        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        IntPair* pair = Find_IntPair(fd_pair, fd);

        if(NULL == pair)
        {
            fprintf(stderr, "%s(), IntPair doesn't found!\n", __func__);
            abort();
        }
        
        if(nbytes != write(pair->value, buf, nbytes))
        {
            fprintf(stderr, "Write error!\n");
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

        ret_val = SYSCALL_CANCEL (write, fd, buf, nbytes);

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        return SYSCALL_CANCEL (write, fd, buf, nbytes);
    }
    
    return ret_val;

  //return SYSCALL_CANCEL (write, fd, buf, nbytes);
}
libc_hidden_def (__libc_write)

weak_alias (__libc_write, __write)
libc_hidden_weak (__write)
weak_alias (__libc_write, write)
libc_hidden_weak (write)
