#!/bin/bash
implementation="light"

./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/11661.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/5670.txt 0.0139176054 $implementation geolife-k1-1.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/10654.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/10728.txt 0.0004656258 $implementation geolife-k1-2.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/5562.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/5543.txt 0.1205100887 $implementation geolife-k1-3.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/3830.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/3981.txt 0.0183384918 $implementation geolife-k10-1.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/15378.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/14853.txt 0.0023537127 $implementation geolife-k10-2.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/8885.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/4878.txt 0.0316011510 $implementation geolife-k10-3.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/11058.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/10585.txt 0.0002481632 $implementation geolife-k10-4.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/849.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/3839.txt 0.0091283117 $implementation geolife-k100-1.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/1876.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/2875.txt 0.0866370933 $implementation geolife-k100-2.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/12984.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/6967.txt 0.0369174716 $implementation geolife-k100-3.svg
./test_curves ../../benchmark/Geolife\ Trajectories\ 1.3/data/4547.txt ../../benchmark/Geolife\ Trajectories\ 1.3/data/10811.txt 0.0070611380 $implementation geolife-k100-4.svg 
