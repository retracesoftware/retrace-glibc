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

#include <sys/socket.h>
#include <sysdep-cancel.h>
#include <socketcall.h>

#include "../../../retrace/retrace-lib.h"

int Retrace_Send(int fd, const void *buf, size_t len, int flags)
{
    int ret_val = -1;
    size_t syscall_num = __NR_sendto;
    pthread_t thread_id;
    time_t cur_time;

    if (rlog.mode == Retrace_Record_Mode)
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_SEND_SYSCALL
            ret_val = SYSCALL_CANCEL (send, fd, buf, len, flags);
        #elif defined __ASSUME_SENDTO_SYSCALL
            ret_val = SYSCALL_CANCEL (sendto, fd, buf, len, flags, NULL, 0);
        #else
            ret_val = SOCKETCALL_CANCEL (send, fd, buf, len, flags);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));

        IntPair* pair = Find_IntPair(sock_pair, fd);

        if(NULL == pair)
        {
            fprintf(stderr, "%s(), IntPair doesn't found!\n", __func__);
            abort();
        }

        if(len != write(pair->value, buf, len))
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

        if(fetched_syscall != syscall_num)
        {
            fprintf(stderr, "Fetched syscall not equal to current one!\n");
            abort();
        }

        RLog_Fetch(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Fetch(&rlog, &cur_time, sizeof(cur_time));
        RLog_Fetch(&rlog, &ret_val, sizeof(ret_val));

        ret_val = SYSCALL_CANCEL (write, fd, buf, len);

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_SEND_SYSCALL
            return SYSCALL_CANCEL (send, fd, buf, len, flags);
        #elif defined __ASSUME_SENDTO_SYSCALL
            return SYSCALL_CANCEL (sendto, fd, buf, len, flags, NULL, 0);
        #else
            return SOCKETCALL_CANCEL (send, fd, buf, len, flags);
        #endif
    }

    return ret_val;
}

ssize_t
__libc_send (int fd, const void *buf, size_t len, int flags)
{
    if (ptrRetraceSend == NULL) {
        #ifdef __ASSUME_SEND_SYSCALL
          return SYSCALL_CANCEL (send, fd, buf, len, flags);
        #elif defined __ASSUME_SENDTO_SYSCALL
          return SYSCALL_CANCEL (sendto, fd, buf, len, flags, NULL, 0);
        #else
          return SOCKETCALL_CANCEL (send, fd, buf, len, flags);
        #endif
    } else {
        return ptrRetraceSend(fd, buf, len, flags);
    }
}
weak_alias (__libc_send, send)
weak_alias (__libc_send, __send)
#ifdef HAVE_INTERNAL_SEND_SYMBOL
libc_hidden_def (__send)
#endif
