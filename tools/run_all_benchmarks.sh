#!/bin/bash

implementation="light"
run_sigspatial=true
run_characters=true
run_geolife=true

if [ "$#" -eq 0 ]; then
	number_of_runs=1
elif [ "$#" -eq 1  ]; then
	number_of_runs=$1
else
	echo "Wrong number of arguments passed."
	echo "USAGE: ./performance_test [number of rounds]"
	echo "Exiting..."
	exit
fi

# check if performance check binary exists
if [[ ! -f "../build/./performance_test" ]]; then
	echo "ERROR: The performance test binary does not exist!"
	echo "Build before executing this script."
	exit 1
fi

# check if benchmark dataset directories exist
if [[ ! -d "../../benchmark/sigspatial/" ]]; then
	echo "ERROR: The sigspatial dataset directory does not exist."
	echo "Fetch or create the benchmarks first (e.g. using benchmark/fetch_and_convert_data.py)."
	exit 1
fi
if [[ ! -d "../../benchmark/characters/data/" ]]; then
	echo "ERROR: The characters dataset directory does not exist."
	echo "Fetch or create the benchmarks first (e.g. using benchmark/fetch_and_convert_data.py)."
	exit 1
fi
if [[ ! -d "../../benchmark/Geolife Trajectories 1.3/data/" ]]; then
	echo "ERROR: The geolife dataset directory does not exist."
	echo "Fetch or create the benchmarks first (e.g. using benchmark/fetch_and_convert_data.py)."
	exit 1
fi


# run performance tests
echo "We will run $number_of_runs performance test(s)!"
echo ""

for i in `seq 1 $number_of_runs`; do
	echo "============================="
	echo "Starting round $i in the test."
	echo "============================="
	echo ""

	if ( $run_sigspatial ); then
		echo "=================================="
		echo "Start SIGSPATIAL performance test."
		echo "=================================="
		echo ""
		../build/./performance_test ../../benchmark/sigspatial/ ../../benchmark/sigspatial/dataset.txt ../test_data/benchmark_queries/sigspatial_query $implementation
		echo ""
		echo ""
	fi

	if ( $run_characters ); then
		echo "=================================="
		echo "Start CHARACTERS performance test."
		echo "=================================="
		echo ""
		../build/./performance_test ../../benchmark/characters/data/ ../../benchmark/characters/data/dataset.txt ../test_data/benchmark_queries/characters_query $implementation
		echo ""
		echo ""
	fi

	if ( $run_geolife ); then
		echo "==============================="
		echo "Start GEOLIFE performance test."
		echo "==============================="
		echo ""
		../build/./performance_test ../../benchmark/Geolife\ Trajectories\ 1.3/data/ ../../benchmark/Geolife\ Trajectories\ 1.3/data/dataset.txt ../test_data/benchmark_queries/geolife_query $implementation
		echo ""
		echo ""
	fi
done
