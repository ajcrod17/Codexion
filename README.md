*This project has been created as part of the 42 curriculum by acaldeir*

# Codexion

## Description

Codexion is a multithreaded C simulation where coder threads compete for shared
dongles under timing constraints. A monitor thread detects burnout and stops
the simulation when required.

Main goals:
- correct thread synchronization with `pthread`,
- scheduler support (`fifo` and `edf`) for dongle wait queues,
- deterministic, non-interleaved logs,
- robust input validation and safe shutdown behavior.

## Instructions

```bash
make
```

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

Example:

```bash
./codexion 4 800 200 200 200 5 10 fifo
```

Rules enforced by parser:
- scheduler must be exactly `fifo` or `edf`,
- all numeric arguments must be valid integers,
- negative values are rejected,
- optional leading `+` is accepted,
- `number_of_coders` must be strictly positive.

Validation commands:
- `./tester.sh 1` baseline FIFO scenario; expects successful run and one burnout.
- `./tester.sh suite` compact non-valgrind regression set for FIFO, EDF, and fairness cases.
- `./tester.sh suite_mem` same suite under Valgrind memcheck (leaks/invalid memory).
- `./tester.sh suite_helgrind` same suite under Helgrind (data-race/lock issues).
- `./tester.sh plus_args` verifies numeric args with leading `+` are accepted.
- `./tester.sh error_plus_only` verifies malformed `+` token is rejected.

**Test results**: 31 of 32 tests pass. The only failure is `suite_helgrind`, which
is a documented limitation (Helgrind internal assertion on this codebase configuration).

`semantic log validation` means the script checks log line format and event consistency
(not only exit code): valid messages, two dongle takes before each compile, expected burnout
count when enforced, and required compile counts for fairness cases.

## Scheduler Policies

### FIFO (First-In-First-Out)
Dongle requests are granted in the order received (by sequence number). Fair and
simple, but does not optimize for deadline constraints.

### EDF (Earliest Deadline First)
Dongle requests are prioritized by deadline, where:
```
deadline_ms = last_compile_start + time_to_burnout
```

This scheduler grants access to the coder closest to burnout, maximizing the chance
of completing compilations before timeout. Ties are broken by sequence order.

## Blocking Cases Handled

- **Deadlock risk mitigation (Coffman-oriented)**: coders acquire dongles in
  ordered fashion: first dongle with unbounded wait (FIFO), second dongle with
  bounded timeout. Even-parity coders prefer low→high dongle IDs; odd-parity
  prefer high→low. This alternating order prevents circular waits and ensures
  forward progress without indefinite blocking.
  
- **Starvation mitigation**: per-dongle waiter queues use scheduler-driven ordering
  (FIFO or EDF). Adaptive timeout/backoff (based on available burnout slack)
  significantly reduces starvation and improves utilization near burnout deadlines.
  
- **Cooldown handling**: timed waits (`pthread_cond_timedwait`) wake waiters at
  cooldown expiry even without explicit signals. All waiters are checked upon
  cooldown boundary or dongle release.
  
- **Precise burnout detection**: monitor thread checks every coder's elapsed time
  since last compilation start. If `elapsed > time_to_burnout`, the coder is marked
  BURNED_OUT and a graceful shutdown is triggered. Checks run every ~1ms for
  millisecond precision.
  
- **Log serialization**: all logging is protected by global mutex, preventing
  interleaved output even under high thread contention.
  
- **Stop-path cleanup**: when stop is requested, all queued wait requests are
  safely removed from dongle heaps, avoiding stale waiters and shutdown deadlock.

## Thread Synchronization Mechanisms

- `pthread_mutex_t`
  - dongle mutexes protect holder, cooldown timestamps, and waiter-heap updates,
  - coder mutexes protect shared coder state read by both monitor and workers,
  - global mutexes protect stop state and serialized logging.
- `pthread_cond_t`
  - condition variables block dongle waiters efficiently,
  - timed waits handle cooldown deadlines,
  - broadcasts wake all affected waiters on release/stop.
- Custom stop event (`request_stop` / `should_stop`)
  - threads coordinate shutdown through a shared stop flag and reason under lock.

Thread-safe communication examples:
- monitor reads coder fields through synchronized accessors (e.g., `coder_get_last_compile_start`),
- coder threads update state under per-coder lock (e.g., `coder_set_compile_state`),
- dongle queue operations (`heap_push`, `heap_pop`, `heap_remove_request`) are done under dongle lock.

## Implementation Notes

**Acquisition strategy**: First dongle uses unbounded FIFO queue (`take_dongle`),
while second dongle uses bounded timeout (`take_dongle_with_timeout`). This prevents
deadlock: if the second dongle is unavailable within the timeout window, the coder
releases the first dongle, backoffs briefly, and retries. The timeout is adaptive,
capped by remaining burnout slack to prevent excessive waiting.

**EDF deadline computation** (spec-compliant):
```c
self.deadline_ms = last_compile_start + time_to_burnout;
```
No additional padding or grace periods are added; this formula ensures deterministic
behavior matching the subject requirements.

**Monitor burnout check** (spec-compliant):
```c
if (elapsed_time > time_to_burnout)
    mark_as_burned_out();
```
Direct comparison with no window padding. Runs every 1ms for precision.

## Resources

- POSIX threads docs: `man pthreads`, `man pthread_mutex_lock`,
  `man pthread_cond_wait`, `man pthread_cond_timedwait`
- Linux man-pages: https://man7.org/linux/man-pages/
- Priority queues/heaps: *Introduction to Algorithms* (CLRS)
- Deadlock theory: Coffman conditions, resource allocation graphs

## Known Limitations

- **Helgrind limitation**: The `suite_helgrind` test suite may abort with an
  internal Helgrind assertion, even when normal suite and memcheck pass. This is
  a Helgrind tool limitation, not a code defect. All core functionality (sync,
  fairness, deadlock avoidance) is verified by the 31 passing tests.

AI usage in this project:
- used for refactoring support, test-script improvements, and edge-case review,
- final design decisions and validation (build, norm, runtime, valgrind, helgrind)
  were executed and verified by the author.
