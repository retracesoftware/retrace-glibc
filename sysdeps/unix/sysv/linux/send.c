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

extern __thread Retrace_Log rlog;
extern __thread IntPair* sock_pair;

ssize_t
__libc_send (int fd, const void *buf, size_t len, int flags)
{
  return Retrace_Send(fd, buf, len, flags);
}
weak_alias (__libc_send, send)
weak_alias (__libc_send, __send)
#ifdef HAVE_INTERNAL_SEND_SYMBOL
libc_hidden_def (__send)
#endif
