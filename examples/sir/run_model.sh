#/bin/bash

./sir sample save_warmup=1 algorithm=hmc metric=dense_e adapt delta=0.95 data file=sir.data.R output file=$1.output.csv id=$1 random seed=55781900

# run in parallel
# seq 4 | parallel -j 4 --workdir $PWD ./run_model.sh {} ::: 0 1 2 3
