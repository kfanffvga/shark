// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task_queue.h"

#include "base/logging.h"
#include "base/stl_util.h"

TaskQueue::TaskQueue() {
}

TaskQueue::~TaskQueue() {
  // We own all the pointes in |queue_|.  It is our job to delete them.
  STLDeleteElements(&queue_);
}

void TaskQueue::Push(Task* task) {
  DCHECK(task);

  // Add the task to the back of the queue.
  queue_.push_back(task);
}

void TaskQueue::Clear() {
  // Delete all the elements in the queue and clear the dead pointers.
  STLDeleteElements(&queue_);
}

bool TaskQueue::IsEmpty() const {
  return queue_.empty();
}

void TaskQueue::Run() {
  // Nothing to run if our queue is empty.
  if (queue_.empty())
    return;

  std::deque<Task*> ready;
  queue_.swap(ready);

  // Run the tasks that are ready.
  std::deque<Task*>::const_iterator task;
  for (task = ready.begin(); task != ready.end(); ++task) {
    // Run the task and then delete it.
    (*task)->Run();
    delete (*task);
  }
}
