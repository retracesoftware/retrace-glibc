/* Linux read syscall implementation.
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

#include <stdio.h>
#include <unistd.h>
#include <sysdep-cancel.h>

#include "../../../retrace/retrace-lib.h"

__thread Retrace_Log rlog;
__thread IntPair* fd_pair = NULL;

/* Read NBYTES into BUF from FD.  Return the number read or -1.  */
ssize_t
__libc_read (int fd, void *buf, size_t nbytes)
{
    int ret_val = -1;

    if (rlog.mode == Retrace_Record_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;
        
        ret_val = SYSCALL_CANCEL (read, fd, buf, nbytes);

        IntPair* pair = Find_IntPair(fd_pair, fd);

        if(NULL == pair)
        {
            fprintf(stderr, "IntPair doesn't found!\n");
            abort();
        }

        
        
        Record_Read(pair->value, buf, nbytes);

        rlog.mode = Retrace_Record_Mode;
    } 
    else if (rlog.mode == Retrace_Replay_Mode) 
    {
        rlog.mode = Retrace_Disabled_Mode;

        ret_val = SYSCALL_CANCEL (read, fd, buf, nbytes);

        rlog.mode = Retrace_Replay_Mode;
    }
    else
    {
        return SYSCALL_CANCEL (read, fd, buf, nbytes);
    }
    
    return ret_val;
}
libc_hidden_def (__libc_read)

libc_hidden_def (__read)
weak_alias (__libc_read, __read)
libc_hidden_def (read)
weak_alias (__libc_read, read)
