*This project has been created as part of the 42 curriculum by acaldeir*

# Codexion

Codexion is a multithreaded C simulation where coders compete for shared dongles under timing constraints.
This README focuses on:
- findings discovered during validation,
- improvements implemented,
- remaining tasks before final evaluation.

## Findings

### 1. Cooldown wait bug
- Symptom: a waiter could sleep forever after waking before cooldown expiry.
- Root cause: `pthread_cond_wait` had no timed wake aligned to cooldown availability.
- Impact: feasible scenarios could incorrectly end in burnout.

### 2. Shared coder state race risk
- Symptom: monitor reads fields while coder thread writes them.
- Fields affected: `state`, `last_compile_start_ms`, `compiles_done`.
- Impact: undefined behavior risk and non-deterministic decisions under load.

### 3. Tester quality issues
- `helgrind` mode used two `--tool` flags and could run the wrong tool.
- Tester did not consistently report pass/fail by expected exit status.

## Improvements Implemented

### Synchronization and cooldown
- Added per-coder mutex: `t_coder.mtx` in `coders/codexion.h`.
- Added synchronized state helpers in `coders/coder_state.c`:
  - `coder_set_compile_state`
  - `coder_set_simple_state`
  - `coder_get_last_compile_start`
  - `coder_get_compiles_done`
- Integrated these helpers in:
  - `coders/coder_routine.c`
  - `coders/monitor.c`
  - `coders/dongles.c`
- Reworked dongle waiting with timed wakeups in `coders/dongles.c`:
  - Added `pthread_cond_timedwait` path based on dongle cooldown timestamp.
  - Wait loop now wakes at cooldown deadline even without extra broadcast.

### Lifecycle updates
- Initialized/destroyed `t_coder.mtx` in:
  - `coders/simulation_init.c`
  - `coders/simulation_destroy.c`
- Added `coders/coder_state.c` to `Makefile` sources.

### Tester improvements
- `tester.sh` now:
  - fixes helgrind command (`valgrind --tool=helgrind`),
  - supports strict variable mode safely,
  - checks expected exit codes,
  - prints `[PASS]` or `[FAIL]` per test case.

## Verification Done

- Build: `make` passes with `-Wall -Wextra -Werror -pthread`.
- Cooldown regression scenario now progresses correctly:
  - `./codexion 2 5000 50 50 50 3 100 fifo`
- Tester smoke checks pass with explicit status reporting:
  - `./tester.sh 1`
  - `./tester.sh error_arg1`

## What Still Needs To Be Done

### Mandatory robustness hardening
1. Remove pending waiter requests safely when a stop occurs during wait.
2. Add rollback cleanup on partial init/thread-create failures.
3. Recheck burnout deadline precision under stress (10 ms requirement).
4. Validate liveness/fairness properties thoroughly for both FIFO and EDF.

### Validation and quality
5. Run full memory and race checks (valgrind memcheck + helgrind) on multiple scenarios.
6. Expand tester assertions beyond exit code:
- line format validation,
- two dongle-take lines before compile,
- single burnout line and no mixed output lines.
7. Decide policy for `number_of_compiles_required == 0` and align with evaluator expectations.

## Useful Commands

```bash
make
./tester.sh 1
./tester.sh error_arg1
./tester.sh 1 mem
./tester.sh 1 helgrind
```

## Notes

- The codebase is now stronger on synchronization correctness and cooldown behavior.
- Further evaluator-focused hardening is still recommended before final submission.
