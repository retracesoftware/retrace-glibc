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

#ifndef weak_variable
# define weak_variable weak_function
#endif

ssize_t default_syscall_sent(int fd, const void *buf, size_t len, int flags)
{
#ifdef __ASSUME_SEND_SYSCALL
  return SYSCALL_CANCEL (send, fd, buf, len, flags);
#elif defined __ASSUME_SENDTO_SYSCALL
  return SYSCALL_CANCEL (sendto, fd, buf, len, flags, NULL, 0);
#else
  return SOCKETCALL_CANCEL (send, fd, buf, len, flags);
#endif
}


ssize_t
__libc_send (int fd, const void *buf, size_t len, int flags)
{
    return syscall_sent(fd,buf,len, flags);
}

__thread weak_variable ssize_t (* syscall_sent)(int, const void*, size_t, int ) = default_syscall_sent;
weak_alias (__libc_send, send)
weak_alias (__libc_send, __send)
#ifdef HAVE_INTERNAL_SEND_SYMBOL
libc_hidden_def (__send)
#endif
