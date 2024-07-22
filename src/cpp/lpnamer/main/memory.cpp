/*
** This file is part of libyuni, a cross-platform C++ framework (http://libyuni.org).
**
** This Source Code Form is subject to the terms of the Mozilla Public License
** v.2.0. If a copy of the MPL was not distributed with this file, You can
** obtain one at http://mozilla.org/MPL/2.0/.
**
** github: https://github.com/libyuni/libyuni/
** gitlab: https://gitlab.com/libyuni/libyuni/ (mirror)
*/
// memory.h: The relative path is to avoid conflict with <memory.h>, which
// can sometimes occur...
#include "include/memory.h"

#if defined(__unix__)
#include <cstring>
#include <sys/sysinfo.h> // sysinfo (2)
#endif

#include <cstdio>
#include <cstdlib>

namespace Memory
{

  //! The default amount of available physical memory
  constexpr uint64_t defaultAvailable = 1024 * 1024 * 512; // 512Mo
                                         //! The default amount of total physical memory
  constexpr uint64_t defaultTotal = 1024 * 1024 * 1024;    // 1Go

#if defined(__unix__)
#define SYSTEM_MEMORY_IS_IMPLEMENTED

namespace // anonymous
{
//! Read a line from a file
static inline int fgetline(FILE* fp, char* s, int maxlen)
{
  int i = 0;
  char c;

  while ((c = (char)fgetc(fp)) != EOF)
  {
    if (c == '\n')
    {
      *s = '\0';
      return i;
    }
    if (i >= maxlen)
      return i;

    *s++ = c;
    ++i;
  }
  return (!i) ? EOF : i;
}

static inline uint64_t readvalue(char* line)
{
  // Here is a sample for /proc/meminfo :
  //
  // MemTotal:      1929228 kB
  // MemFree:         12732 kB
  // Buffers:         72176 kB
  // Cached:        1076572 kB
  // SwapCached:     151412 kB
  // Active:        1491184 kB
  // Inactive:       190832 kB
  // HighTotal:           0 kB
  // HighFree:            0 kB
  // LowTotal:      1929228 kB
  // LowFree:         12732 kB
  // SwapTotal:     2096472 kB
  // SwapFree:      1732964 kB
  // Dirty:             736 kB
  // Writeback:           0 kB
  // AnonPages:      512004 kB
  // Mapped:         702148 kB
  // Slab:           154320 kB
  // PageTables:      34712 kB
  // NFS_Unstable:        0 kB
  // Bounce:              0 kB
  // CommitLimit:   3061084 kB
  // Committed_AS:  1357596 kB
  // VmallocTotal: 34359738367 kB
  // VmallocUsed:    263492 kB
  // VmallocChunk: 34359474679 kB
  // HugePages_Total:     0
  // HugePages_Free:      0
  // HugePages_Rsvd:      0
  // Hugepagesize:     2048 kB

  // Trimming the string from the begining
  while (*line == ' ' and *line != '\0')
    ++line;
  const char* first = line;

  // Looking for the end of the number
  while (*line != ' ' and *line != '\0')
    ++line;
  // Tagging the end of the number
  *line = '\0';

#ifdef YUNI_OS_32
  return (uint64_t)atol(first) * 1024u;
#else
  return (uint64_t)atoll(first) * 1024u;
#endif
}

} // anonymous namespace

bool Usage::update()
{
  // The only good way to retrieve the memory usage is to read /proc/meminfo.
  // The routine sysinfo (2) is not enough since it does not take care of
  // the cache memory, returning a not valid amount of available physsical
  // memory for our uses.
  FILE* fd;
  if ((fd = fopen("/proc/meminfo", "r")))
  {
    // Resetting the amount of the total physical memory
    total = 0;
    // The amount of the available physical memory is the sum of 3 values :
    // MemFree, Cached and Buffers.
    available = 0;

    // A small buffer
    char line[90];
    // A counter to reduce to abort as soon as we have read all
    int remains = 8;

    // Analysing each line in /proc/meminfo, until the end-of-file or
    // until we have read the 4 values that interrest us.
    while (EOF != fgetline(fd, line, (uint)sizeof(line)))
    {
      if (!strncmp("MemTotal:", line, 9))
      {
        total = readvalue(line + 10);
        if (!(remains >> 1))
          break;
      }
      if (!strncmp("MemFree:", line, 8))
      {
        available += readvalue(line + 9);
        if (!(remains >> 1))
          break;
      }
      if (!strncmp("Cached:", line, 7))
      {
        available += readvalue(line + 8);
        if (!(remains >> 1))
          break;
      }
      if (!strncmp("Buffers:", line, 8))
      {
        available += readvalue(line + 9);
        if (!(remains >> 1))
          break;
      }
    }

    // Closing /proc/meminfo
    fclose(fd);

    // Checking the amount of the total physical memory, which can not be equal to 0
    if (!total)
    {
      total = (uint64_t)defaultTotal;
      return false;
    }
    return true;
  }

  // Error, using default values
  total = (uint64_t)defaultTotal;
  available = (uint64_t)defaultAvailable;
  return false;
}

uint64_t Available()
{
  return Usage().available;
}

uint64_t Total()
{
#ifdef __unix__
  {
    // Directly using sysinfo (2), which should be faster than parsing /proc/meminfo
    struct sysinfo s;
    return (!sysinfo(&s)) ? (s.mem_unit * s.totalram) / 1024 / 1024 / 1024 : (uint64_t)defaultTotal;
  }
#else
  {
    return Usage().total;
  }
#endif
}

#endif // YUNI_OS_LINUX

#ifndef SYSTEM_MEMORY_IS_IMPLEMENTED
#error Yuni::System::Memory: The implementation is missing for this operating system

uint64_t Total()
{
  return defaultTotal;
}

uint64_t Available()
{
  return defaultAvailable;
}

bool Usage::update()
{
  available = defaultAvailable;
  total = defaultTotal;
  return false;
}

#endif // Fallback

} // namespace Memory
