/* Copyright (C) 2015-2020 Free Software Foundation, Inc.
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

#include <errno.h>
#include <signal.h>
#include <sys/socket.h>

#include <socketcall.h>
#include <kernel-features.h>
#include <sys/syscall.h>

#include "../../../retrace/retrace-lib.h"

extern int (*ptrRetraceBind) (int fd, __CONST_SOCKADDR_ARG addr, socklen_t len) = NULL;

int Retrace_Bind(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len)
{
    int ret_val = -1
    size_t syscall_num = __NR_bind;
    pthread_t thread_id;
    time_t cur_time;

    if (rlog.mode == Retrace_Record_Mode)
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_BIND_SYSCALL
            ret_val = INLINE_SYSCALL (bind, 3, fd, addr.__sockaddr__, len);
        #else
            ret_val = SOCKETCALL (bind, fd, addr.__sockaddr__, len, 0, 0, 0);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        rlog.mode = Retrace_Record_Mode;
    }
    else if (rlog.mode == Retrace_Replay_Mode)
    {
        rlog.mode = Retrace_Disabled_Mode;

        size_t fetched_syscall;

        RLog_Fetch(&rlog, &fetched_syscall, sizeof(fetched_syscall));

        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_BIND_SYSCALL
            return INLINE_SYSCALL (bind, 3, fd, addr.__sockaddr__, len);
        #else
            return SOCKETCALL (bind, fd, addr.__sockaddr__, len, 0, 0, 0);
        #endif
    }

    return ret_val;
}

int
__bind (int fd, __CONST_SOCKADDR_ARG addr, socklen_t len)
{
    if (ptrRetraceBind == NULL) {
        #ifdef __ASSUME_BIND_SYSCALL
          return INLINE_SYSCALL (bind, 3, fd, addr.__sockaddr__, len);
        #else
          return SOCKETCALL (bind, fd, addr.__sockaddr__, len, 0, 0, 0);
        #endif
    } else {
        return ptrRetraceBind(fd,addr,len);
    }
}
weak_alias (__bind, bind)
