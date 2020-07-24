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

#include "../../../retrace/retrace-lib.h"

extern __thread Retrace_Log rlog;
extern __thread IntPair* fd_pair;

/* Close the file descriptor FD.  */
int
__close (int fd)
{
  int ret_val = -1;

  if (rlog.mode == Retrace_Record_Mode) 
  {
    rlog.mode = Retrace_Disabled_Mode;
    
    ret_val = SYSCALL_CANCEL (close, fd);

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
