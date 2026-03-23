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

Known status notes:
- Helgrind may abort with an internal assertion on this project setup, even when
  normal suite and memcheck pass.

`semantic log validation` means the script checks log line format and event consistency
(not only exit code): valid messages, two dongle takes before each compile, expected burnout
count when enforced, and required compile counts for fairness cases.

## Blocking Cases Handled

- Deadlock risk mitigation (Coffman-oriented): coders avoid indefinite waiting
  for the second resource by using bounded second-dongle acquisition with
  retry/release behavior.
- Starvation mitigation: per-dongle waiter queues use policy-driven ordering
  (`fifo` or `edf`) plus adaptive retry timing. This significantly reduces
  starvation frequency but does not guarantee zero burnout in all stress runs.
- Cooldown handling: timed waits (`pthread_cond_timedwait`) wake waiters at
  cooldown expiry even without extra signals.
- Precise burnout detection: monitor checks elapsed time against burnout
  deadline with millisecond precision.
- Log serialization: logging is protected so concurrent threads cannot interleave
  output lines.
- Stop-path cleanup: queued wait requests are removed when stop is requested,
  avoiding stale waiters and shutdown blocking.

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
- monitor reads coder fields through synchronized accessors,
- coder threads update state under per-coder lock,
- dongle queue operations (`push`, `pop`, `remove`) are done under dongle lock.

## Resources

- POSIX threads docs: `man pthreads`, `man pthread_mutex_lock`,
  `man pthread_cond_wait`, `man pthread_cond_timedwait`
- Linux man-pages: https://man7.org/linux/man-pages/
- Priority queues/heaps: *Introduction to Algorithms* (CLRS)
- Deadlock theory: Coffman conditions

AI usage in this project:
- used for refactoring support, test-script improvements, and edge-case review,
- used to propose safer rollback/synchronization patterns,
- final design decisions and validation (build, norm, runtime, valgrind,
  helgrind) were executed and verified by the author (with the Helgrind
  limitation above).
