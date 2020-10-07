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

int default_syscall_connect(int fd, __CONST_SOCKADDR_ARG addr, socklen_t len)
{
#ifdef __ASSUME_CONNECT_SYSCALL
  return SYSCALL_CANCEL (connect, fd, addr.__sockaddr__, len);
#else
  return SOCKETCALL_CANCEL (connect, fd, addr.__sockaddr__, len);
#endif
}

int
__libc_connect (int fd, __CONST_SOCKADDR_ARG addr, socklen_t len)
{
    return syscall_connect(fd,addr,len);
}
__thread weak_variable int (* syscall_connect)(int,__CONST_SOCKADDR_ARG,socklen_t) = default_syscall_connect;
weak_alias (__libc_connect, connect)
weak_alias (__libc_connect, __connect)
libc_hidden_weak (__connect)
