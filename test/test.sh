#!/bin/bash

test_count=1
failed=""

print_error() {
	echo "Test $test_count failed: ">&2
	for var in "$@"; do
		echo -n "$var" >&2
	done
	echo >&2
	failed="1"
}

check_file_exists() {
	if [ ! -e $1 ]; then
		print_error "file \"$1\" does not exist." 
	fi
}

test_case() {
	test_count=$((test_count+1))
	if ! cate $1; then
		print_error "cate failed to run \"$1\"" 
		return
	fi
	shift

	for file in "$@"; do
		check_file_exists $file
	done
}

check_cate_command_for() {
	test_count=$((test_count+1))
	text=`cate -d $1`

	if [ -z text ] ; then
		print_error "cate failed to run \"$1\"" 
		return
	fi
	shift

	for substring in "$@"; do
		if [[ ! "$text" == *"$substring"* ]]; then
			print_error "$substring was not found"
		fi
	done
}

test_case "" catel.out
test_case recursive recursive.out subrecursive.out
test_case library out/liblibrary_test.a out/liblibrary_test.so with_static.out with_dynamic.out with_ident.out

check_cate_command_for options " -lstdc++fs" " -std=c++17" " -pthread" 
check_cate_command_for smol "-ffunction-sections" "strip -S --strip-unneeded" "--remove-section=.comment"

#finally end
if [ ! -z $failed ]; then
	echo "some tests failed" >&2
	exit 1
fi

echo "All tests passed!"