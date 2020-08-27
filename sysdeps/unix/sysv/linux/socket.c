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

__thread Retrace_Log rlog;
__thread IntPair* sock_pair = NULL;


int Retrace_Socket(int fd, int type, int domain)
{
    int ret_val = -1;
    size_t syscall_num = __NR_socket;
    pthread_t thread_id;
    time_t cur_time;

    if (rlog.mode == Retrace_Record_Mode)
    {
        rlog.mode = Retrace_Disabled_Mode;

        #ifdef __ASSUME_SOCKET_SYSCALL
            ret_val = INLINE_SYSCALL(socket, 3, fd, type, domain);
        #else
            ret_val = SOCKETCALL(socket, fd, type, domain);
        #endif

        thread_id = pthread_self();
        cur_time = time(NULL);

        char recorded_file_path[FILENAME_MAX];
        sprintf(recorded_file_path, "%s%d", RETRACE_DIR, ret_val);

        Check_Dir(RETRACE_DIR);

        int record_fd = open(recorded_file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);

        if (record_fd < 0)
        {
            fprintf(stderr, "Record file open error!\n");
            abort();
        }

        Insert_IntPair(&sock_pair, ret_val, record_fd);

        RLog_Push(&rlog, &syscall_num, sizeof(syscall_num));
        RLog_Push(&rlog, &thread_id, sizeof(pthread_t));
        RLog_Push(&rlog, &cur_time, sizeof(cur_time));
        RLog_Push(&rlog, &ret_val, sizeof(ret_val));
        RLog_Push(&rlog, recorded_file_path, strlen(recorded_file_path) + 1);

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

        size_t filename_len = RLog_Fetch_Length(&rlog);

        char* recorded_file_path = malloc(filename_len);

        if(NULL == recorded_file_path)
        {
            fprintf(stderr, "Allocation memory error!\n");
            abort();
        }

        RLog_Fetch(&rlog, recorded_file_path, filename_len);

        ret_val = open(recorded_file_path, O_RDWR);

        free(recorded_file_path);

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        #ifdef __ASSUME_SOCKET_SYSCALL
            return INLINE_SYSCALL (socket, 3, fd, type, domain);
        #else
            return SOCKETCALL (socket, fd, type, domain);
        #endif
    }

    return ret_val;
}

int
__socket (int fd, int type, int domain)
{
    if (ptrRetraceSocket == NULL) {
        #ifdef __ASSUME_SOCKET_SYSCALL
          return INLINE_SYSCALL (socket, 3, fd, type, domain);
        #else
          return SOCKETCALL (socket, fd, type, domain);
        #endif
    } else {
        return ptrRetraceSocket(fd,type,domain);
    }

}

libc_hidden_def (__socket)

weak_alias (__socket, socket)
