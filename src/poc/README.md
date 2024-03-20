# Apache Arrow and Parquet Proof of Concept

Program "arrow_poc.cpp" (arrow proof of concept) reads in a local CSV file
and writes a Parquet file.

It uses the following Arrow modules:

- COMPUTE (core in memory representation)
- CSV (CSV file format readers/writers)
- FILESYSTEM (local disk files)
- PARQUET (columnar data stuff)

To see how the C++ Apache Arrow libraries work, I built Arrow locally
following the instructions here:
https://arrow.apache.org/docs/developers/cpp/building.html#building-arrow-cpp

I started from the minimal release build instructions:

```
git clone https://github.com/apache/arrow.git
cd arrow/cpp
mkdir release
cd release
cmake ..    # too minimal
make        # ditto
```

Through trial and error, I tracked down the correct set of CMAKE config
I used CMAKE to create the set of local header and lib.a files needed
to compile and link the programs.

```
cmake .. \
	-DARROW_BUILD_STATIC=ON \
	-DARROW_COMPUTE=ON \
	-DARROW_CSV=ON \
	-DARROW_FILESYSTEM=ON \
	-DARROW_PARQUET=ON \
	-DARROW_DEPENDENCY_SOURCE=BUNDLED \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX:PATH=lib

make -j4 arrow
make -j4 parquet
make install   # into local dir "lib" (not /usr/local/lib)
```

Once installed, use GNU make to compile a program - see Makefile in this directory -
and run the program on one of the test file "stan_test.csv"

```
make
./arrow_poc stan_test.csv -o stan_test.parquet
```


