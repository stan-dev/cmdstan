#/bin/bash

./sblrc-blr sample save_warmup=1 max_num_warmup=1100 algorithm=hmc metric=dense_e adapt delta=0.95 data file=sblrc-blr.data.R output file=$1.output.csv id=$1 random seed=8857113

# run in parallel
# seq 4 | parallel -j 4 --workdir $PWD ./run_model.sh {} ::: 0 1 2 3
