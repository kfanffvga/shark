// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/shared_memory.h"

#include <sys/mman.h>

#include "base/logging.h"
#include "third_party/ashmem/ashmem.h"

namespace base {

// For Android, we use ashmem to implement SharedMemory. ashmem_create_region
// will automatically pin the region. We never explicitly call pin/unpin. When
// all the file descriptors from different processes associated with the region
// are closed, the memory buffer will go away.

bool SharedMemory::CreateNamed(const std::string& name,
                               bool open_existing, uint32 size) {
  DCHECK_EQ(-1, mapped_file_ );

  // "name" is just a label in ashmem. It is visible in /proc/pid/maps.
  mapped_file_ = ashmem_create_region(name.c_str(), size);
  if (-1 == mapped_file_) {
    DLOG(ERROR) << "Shared memory creation failed";
    return false;
  }

  int err = ashmem_set_prot_region(mapped_file_,
                                   PROT_READ | PROT_WRITE | PROT_EXEC);
  if (err < 0) {
    DLOG(ERROR) << "Error " << err << " when setting protection of ashmem";
    return false;
  }
  created_size_ = size;

  return true;
}

bool SharedMemory::Delete(const std::string& name) {
  // Like on Windows, this is intentionally returning true as ashmem will
  // automatically releases the resource when all FDs on it are closed.
  return true;
}

bool SharedMemory::Open(const std::string& name, bool read_only) {
  // ashmem doesn't support name mapping
  NOTIMPLEMENTED();
  return false;
}

}  // namespace base
