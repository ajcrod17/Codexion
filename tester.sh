#!/bin/bash

set -u

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

PROG="./codexion"
PRECISION_TOLERANCE_MS=10

if [ -z "$1" ]; then
	echo -e "${RED}Missing test name.${NC}"
	echo "Usage: ./tester.sh <test_name> [mem|helgrind]"
	exit 1
fi

VALGRIND=""
case "${2-}" in
	"mem")
		VALGRIND="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes"
		;;
	"helgrind")
		VALGRIND="valgrind --tool=helgrind"
		;;
	"")
		VALGRIND=""
		;;
	*)
		echo -e "${RED}Unknown mode: $2. Use 'mem' or 'helgrind'.${NC}"
		exit 1
		;;
esac

print_pass()
{
	echo -e "${GREEN}[PASS] $1${NC}"
}

print_fail()
{
	echo -e "${RED}[FAIL] $1${NC}"
}

validate_log_semantics()
{
	local file="$1"
	local ncoders="$2"
	local required="$3"
	local expect_burnout="$4"
	local enforce_required="$5"

	awk -v ncoders="$ncoders" -v required="$required" \
		-v expect_burnout="$expect_burnout" \
		-v enforce_required="$enforce_required" '
	BEGIN {
		burned = 0;
	}
	{
		if ($0 !~ /^[0-9]+ [0-9]+ (has taken a dongle|is compiling|is debugging|is refactoring|burned out)$/)
			{
				print "invalid_line:" $0;
				exit 1;
			}
		id = $2 + 0;
		msg = substr($0, index($0, $3));
		if (msg == "has taken a dongle")
			takes[id]++;
		else if (msg == "is compiling")
		{
			if (takes[id] < 2)
			{
				print "compile_without_two_takes:" id;
				exit 1;
			}
			takes[id] -= 2;
			comp[id]++;
		}
		else if (msg == "burned out")
			burned++;
	}
	END {
		if (expect_burnout == 1 && burned != 1)
		{
			print "expected_one_burnout_got:" burned;
			exit 1;
		}
		if (expect_burnout == 0 && burned != 0)
		{
			print "unexpected_burnout:" burned;
			exit 1;
		}
		if (enforce_required == 1)
		{
			for (i = 1; i <= ncoders; i++)
				if (comp[i] < required)
				{
					print "coder_missing_required:" i;
					exit 1;
				}
		}
	}
	' "$file"
}

check_burnout_precision()
{
	local file="$1"
	local ttb="$2"
	local tolerance="$3"
	local ts
	local max

	ts=$(awk '$3 == "burned" && $4 == "out" {print $1; exit}' "$file")
	if [ -z "$ts" ]; then
		print_fail "burnout precision: missing burnout line"
		return 1
	fi
	max=$((ttb + tolerance))
	if [ "$ts" -gt "$max" ]; then
		print_fail "burnout precision: got ${ts}ms, max ${max}ms"
		return 1
	fi
	print_pass "burnout precision: ${ts}ms <= ${max}ms"
	return 0
}

run_case()
{
	local name="$1"
	local args="$2"
	local expected_exit="$3"
	local expect_burnout="$4"
	local enforce_required="$5"
	local check_precision="$6"
	local out_file="/tmp/codexion_${name}_$$.log"
	local status
	local ncoders
	local ttb
	local req

	echo -e "Test ${name}: ${args}\n"
	if [ -n "$VALGRIND" ]; then
		$VALGRIND $PROG $args >"$out_file" 2>&1
	else
		$PROG $args >"$out_file" 2>&1
	fi
	status=$?
	cat "$out_file"

	if [ "$status" -ne "$expected_exit" ]; then
		print_fail "exit=$status expected=$expected_exit"
		rm -f "$out_file"
		return 1
	fi

	set -- $args
	ncoders="$1"
	ttb="$2"
	req="$6"

	if [ "$expected_exit" -eq 0 ] && [ -z "$VALGRIND" ]; then
		if ! validate_log_semantics "$out_file" "$ncoders" "$req" "$expect_burnout" "$enforce_required"; then
			print_fail "semantic log validation"
			rm -f "$out_file"
			return 1
		fi
		print_pass "semantic log validation"
		if [ "$check_precision" -eq 1 ]; then
			if ! check_burnout_precision "$out_file" "$ttb" "$PRECISION_TOLERANCE_MS"; then
				rm -f "$out_file"
				return 1
			fi
		fi
	fi

	print_pass "exit=$status expected=$expected_exit"
	rm -f "$out_file"
	return 0
}

run_suite_mode()
{
	local mode="$1"
	local failures=0

	echo "Running suite in mode: $mode"
	VALGRIND="$mode"

	run_case suite_fifo "4 800 200 200 200 5 10 fifo" 0 -1 0 0 || failures=$((failures + 1))
	run_case suite_edf "4 800 200 200 200 5 10 edf" 0 -1 0 0 || failures=$((failures + 1))
	run_case suite_fair_fifo "5 5000 100 100 100 3 10 fifo" 0 0 1 0 || failures=$((failures + 1))
	run_case suite_fair_edf "5 5000 100 100 100 3 10 edf" 0 0 1 0 || failures=$((failures + 1))

	if [ "$failures" -eq 0 ]; then
		print_pass "suite ($mode)"
		return 0
	fi
	print_fail "suite ($mode) failures=$failures"
	return 1
}

case "$1" in
	1)
		run_case basic_fifo "4 800 200 200 200 5 10 fifo" 0 -1 0 0
		;;
	2)
		run_case basic_edf "4 800 200 200 200 5 10 edf" 0 -1 0 0
		;;
	3)
		run_case success_fifo "10 10000 100 100 100 5 50 fifo" 0 0 1 0
		;;
	4)
		run_case large_edf "20 5000 500 500 500 10 100 edf" 0 0 1 0
		;;
	5)
		run_case low_cooldown "5 2000 100 100 100 20 1 fifo" 0 0 1 0
		;;
	6)
		run_case long_actions "3 10000 2000 2000 2000 2 100 fifo" 0 0 1 0
		;;
	fair_fifo)
		run_case fairness_fifo "5 5000 100 100 100 3 10 fifo" 0 0 1 0
		;;
	fair_edf)
		run_case fairness_edf "5 5000 100 100 100 3 10 edf" 0 0 1 0
		;;
	precision_fifo)
		run_case precision_fifo "1 1000 200 200 200 5 50 fifo" 0 1 0 1
		;;
	precision_edf)
		run_case precision_edf "1 1000 200 200 200 5 50 edf" 0 1 0 1
		;;
	zero_compile)
		run_case zero_compiles "5 1000 200 200 200 0 10 fifo" 0 0 0 0
		;;
	plus_args)
		run_case plus_args "+4 +800 +200 +200 +200 +1 +10 fifo" 0 -1 0 0
		;;
	zero_times)
		run_case zero_times "3 0 0 0 0 0 0 fifo" 0 0 0 0
		;;
	one_compiler_fifo)
		run_case one_compiler_fifo "1 1000 200 200 200 5 50 fifo" 0 1 0 1
		;;
	one_compiler_edf)
		run_case one_compiler_edf "1 1000 200 200 200 5 50 edf" 0 1 0 1
		;;
	immediate_burnout)
		run_case immediate_burnout "2 1 200 200 200 5 10 fifo" 0 1 0 1
		;;
	cooldown_hell)
		run_case cooldown_hell "2 1000 100 100 100 5 2000 fifo" 0 1 0 0
		;;
	error_arg1)
		run_case error_coder "banana 200 300 400 500 5 10 fifo" 1 0 0 0
		;;
	error_arg2)
		run_case error_coder "10 banana 300 400 500 5 10 fifo" 1 0 0 0
		;;
	error_arg3)
		run_case error_coder "10 200 banana 400 500 5 10 fifo" 1 0 0 0
		;;
	error_arg4)
		run_case error_coder "10 200 300 banana 500 5 10 fifo" 1 0 0 0
		;;
	error_arg5)
		run_case error_coder "10 200 300 400 banana 5 10 fifo" 1 0 0 0
		;;
	error_arg6)
		run_case error_coder "10 200 300 400 500 banana 10 fifo" 1 0 0 0
		;;
	error_arg7)
		run_case error_coder "10 200 300 400 500 5 banana fifo" 1 0 0 0
		;;
	error_arg8)
		run_case error_coder "10 200 300 400 500 5 10 banana" 1 0 0 0
		;;
	error_arg9)
		run_case error_coder "10 200 300 -400 500 5 10 edf" 1 0 0 0
		;;
	error_arg10)
		run_case error_coder "too 10 200 300 400 500 5 10 edf" 1 0 0 0
		;;
	error_plus_only)
		run_case error_coder "+ 200 300 400 500 5 10 fifo" 1 0 0 0
		;;
	suite_mem)
		run_suite_mode "valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes"
		;;
	suite_helgrind)
		run_suite_mode "valgrind --tool=helgrind"
		;;
	suite)
		run_suite_mode ""
		;;
	*)
		echo -e "${RED}Unknown test.${NC}"
		exit 1
		;;
esac
