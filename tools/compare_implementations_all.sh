#!/bin/bash
./compare_implementations ../../benchmark/sigspatial/ ../../benchmark/sigspatial/dataset.txt ../test_data/benchmark_queries/sigspatial_query ../test_data/benchmark_queries/sigspatial_result light $1
./compare_implementations ../../benchmark/characters/data/ ../../benchmark/characters/data/dataset.txt ../test_data/benchmark_queries/characters_query ../test_data/benchmark_queries/characters_result light $1
./compare_implementations ../../benchmark/Geolife\ Trajectories\ 1.3/data/ ../../benchmark/Geolife\ Trajectories\ 1.3/data/dataset.txt ../test_data/benchmark_queries/geolife_query ../test_data/benchmark_queries/geolife_result light $1
