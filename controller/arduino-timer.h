/**
   arduino-timer - library for delaying function calls

   Copyright (c) 2018, Michael Contreras
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _CM_ARDUINO_TIMER_H__
#define _CM_ARDUINO_TIMER_H__

#include <Arduino.h>

#include <limits.h>

#ifndef TIMER_MAX_TASKS
#define TIMER_MAX_TASKS 0x10
#endif

#define _timer_foreach_task(T, task) \
  for (T task = tasks; task < tasks + max_tasks; ++task)

#define timer_foreach_task(T) \
  _timer_foreach_task(struct task *, T)

#define timer_foreach_const_task(T) \
  _timer_foreach_task(const struct task *, T)

template<
  size_t max_tasks = TIMER_MAX_TASKS,    /* max allocated tasks */
  unsigned long (*time_func)() = millis, /* time function for timer */
  typename T = void *                    /* handler argument type */
  >
class Timer {
public:

  typedef void *Task;                  /* public task handle */
  typedef bool (*handler_t)(T opaque); /* task handler func signature */

  /* Calls handler with opaque as argument in delay units of time */
  Task
  in(unsigned long delay, handler_t h, T opaque = T()) {
    return add_task(time_func(), delay, h, opaque);
  }

  /* Calls handler with opaque as argument at time */
  Task
  at(unsigned long time, handler_t h, T opaque = T()) {
    const unsigned long now = time_func();
    return add_task(now, time - now, h, opaque);
  }

  /* Calls handler with opaque as argument every interval units of time */
  Task
  every(unsigned long interval, handler_t h, T opaque = T()) {
    return add_task(time_func(), interval, h, opaque, interval);
  }

  /* Cancel the timer task */
  bool
  cancel(Task &task) {
    struct task *const t = static_cast<struct task *const>(task);

    if (t && (tasks <= t) && (t < tasks + max_tasks) && t->handler) {
      remove(t);
      task = static_cast<Task>(NULL);
      return true;
    }

    return false;
  }

  /* Cancel all timer tasks */
  void
  cancel() {
    timer_foreach_task(t) {
      remove(t);
    }
  }

  /* Ticks the timer forward - call this function in loop() */
  unsigned long
  tick() {
    tick<void>();
    return ticks();
  }

  template<typename R> void
  tick() {
    timer_foreach_task(task) {
      if (task->handler) {
        const unsigned long t = time_func();
        const unsigned long duration = t - task->start;

        if (duration >= task->expires) {
          task->repeat = task->handler(task->opaque) && task->repeat;

          if (task->repeat) task->start = t;
          else remove(task);
        }
      }
    }
  }

  /* Ticks until the next event */
  unsigned long
  ticks() const {
    unsigned long ticks = ULONG_MAX, elapsed;
    const unsigned long start = time_func();

    timer_foreach_const_task(task) {
      if (task->handler) {
        const unsigned long t = time_func();
        const unsigned long duration = t - task->start;

        if (duration >= task->expires) {
          ticks = 0;
          break;
        } else {
          const unsigned long remaining = task->expires - duration;
          ticks = remaining < ticks ? remaining : ticks;
        }
      }
    }

    elapsed = time_func() - start;

    if (elapsed >= ticks || ticks == ULONG_MAX) ticks = 0;
    else ticks -= elapsed;

    return ticks;
  }

  /* Number of active tasks in the timer */
  size_t
  size() const {
    size_t s = 0;

    timer_foreach_const_task(task) {
      if (task->handler) ++s;
    }

    return s;
  }

  /* True if there are no active tasks */
  bool
  empty() const {
    timer_foreach_const_task(task) {
      if (task->handler) return false;
    }

    return true;
  }

  Timer()
    : tasks{} {}

private:

  struct task {
    handler_t handler; /* task handler callback func */
    T opaque;          /* argument given to the callback handler */
    unsigned long start,
      expires, /* when the task expires */
      repeat;  /* repeat task */
  } tasks[max_tasks];

  inline void
  remove(struct task *task) {
    task->handler = NULL;
    task->opaque = T();
    task->start = 0;
    task->expires = 0;
    task->repeat = 0;
  }

  inline struct task *
  next_task_slot() {
    timer_foreach_task(slot) {
      if (slot->handler == NULL) return slot;
    }

    return NULL;
  }

  inline struct task *
  add_task(unsigned long start, unsigned long expires,
           handler_t h, T opaque, bool repeat = 0) {
    struct task *const slot = next_task_slot();

    if (!slot) return NULL;

    slot->handler = h;
    slot->opaque = opaque;
    slot->start = start;
    slot->expires = expires;
    slot->repeat = repeat;

    return slot;
  }
};

/* create a timer with the default settings */
inline Timer<>
timer_create_default() {
  return Timer<>();
}

#undef _timer_foreach_task
#undef timer_foreach_task
#undef timer_foreach_const_task

#endif
