#!/bin/sh

# dependencies: octave, wget, python3

# download

echo "Downloading all the data..."
wget 'https://download.microsoft.com/download/F/4/8/F4894AA5-FDBC-481E-9285-D5F8C4C4F039/Geolife%20Trajectories%201.3.zip'
# TODO: replace by MPI link to spare Martin some bandwidth!
wget 'https://www.martinwerner.de/files/shortest-sf.tgz'
wget 'https://archive.ics.uci.edu/ml/machine-learning-databases/character-trajectories/mixoutALL_shifted.mat'

# extract

echo "Extracting all the data..."
unzip Geolife\ Trajectories\ 1.3.zip
tar xf shortest-sf.tgz
mkdir characters
cp 'mixoutALL_shifted.mat' 'characters'

# convert

echo "Converting all the data... (this might take some time)"
cp 'geolife_converter.py' 'Geolife Trajectories 1.3'
cd Geolife\ Trajectories\ 1.3
mkdir data
python3 geolife_converter.py
cd data
dataset=$(ls *.txt | sort -n)
echo "$dataset" > dataset.txt
cd ../..

cp 'character_converter.m' 'characters'
cd characters
mkdir data
octave character_converter.m
cd data
dataset=$(ls *.txt | sort -n)
echo "$dataset" > dataset.txt
cd ../..

mv files sigspatial
cd sigspatial
rm dataset.txt
for filename in file*; do
    [ -e "$filename"  ] || continue
    echo "$(tail -n +2 ${filename})" > ${filename}
done
ls file* | sort -n > dataset.txt
