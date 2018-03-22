#!/usr/bin/python

import argparse
import os
import re
import subprocess
from difflib import SequenceMatcher
from fnmatch import fnmatch
from functools import wraps
from time import time

TIME_FILE = "times.csv"
PERF_MODEL_DIRS = ("examples/example-models/bugs_examples/vol1",)

def find_files(pattern, dirs):
    res = []
    for pd in dirs:
        for d, _, flist in os.walk(pd):
            for f in flist:
                if fnmatch(f, pattern):
                    res.append(os.path.join(d, f))
    return res

def str_dist(target):
    def str_dist_internal(candidate):
        return SequenceMatcher(None, candidate, target).ratio()
    return str_dist_internal

def closest_string(target, candidates):
    if candidates:
        return max(candidates, key=str_dist(target))

def find_data_for_model(model):
    d = os.path.dirname(model)
    data_files = find_files("*.data.R", [d])
    if len(data_files) == 1:
        return data_files[0]
    else:
        return closest_string(model, data_files)

def time_step(name, fn, *args, **kwargs):
    start = time()
    res = fn(*args, **kwargs)
    end = time()
    with open(TIME_FILE, "a") as f:
        f.write("{}, {}\n".format(name, end-start))
    return res

def shexec(command):
    returncode = subprocess.call(command, shell=True)
    if returncode != 0:
        raise Exception("{} from '{}'!".format(returncode, command))
    return returncode

def make(targets):
    shexec("make -j8 " + " ".join(targets))

model_name_re = re.compile(".*/[A-z_][^/]+\.stan$")

bad_models = frozenset(
    ["examples/example-models/ARM/Ch.21/finite_populations.stan"
     , "examples/example-models/ARM/Ch.21/multiple_comparison.stan"
     , "examples/example-models/ARM/Ch.21/r_sqr.stan"
     , "examples/example-models/ARM/Ch.23/electric_1a.stan"
     , "examples/example-models/ARM/Ch.23/educational_subsidy.stan"
     , "examples/example-models/bugs_examples/vol2/pines/pines-3.stan"
     , "examples/example-models/bugs_examples/vol3/fire/fire.stan"
     # The following have data issues
              , "examples/example-models/ARM/Ch.10/ideo_two_pred.stan"
     , "examples/example-models/ARM/Ch.16/radon.1.stan"
     , "examples/example-models/ARM/Ch.16/radon.2.stan"
     , "examples/example-models/ARM/Ch.16/radon.2a.stan"
     , "examples/example-models/ARM/Ch.16/radon.2b.stan"
     , "examples/example-models/ARM/Ch.16/radon.3.stan"
     , "examples/example-models/ARM/Ch.16/radon.nopooling.stan"
     , "examples/example-models/ARM/Ch.16/radon.pooling.stan"
     , "examples/example-models/ARM/Ch.18/radon.1.stan"
     , "examples/example-models/ARM/Ch.18/radon.2.stan"
     , "examples/example-models/ARM/Ch.18/radon.nopooling.stan"
     , "examples/example-models/ARM/Ch.18/radon.pooling.stan"
     , "examples/example-models/ARM/Ch.19/item_response.stan"
     , "examples/example-models/bugs_examples/vol1/dogs/dogs.stan"
     , "examples/example-models/bugs_examples/vol1/rats/rats_stanified.stan"
    ])

def run(exe, data, overwrite=False):
    gold = exe + ".csv"
    tmp = exe + "_tmp.csv"
    shexec("{} sample data file={} random seed=1234 output file={}"
           .format(exe, data, gold if overwrite else tmp))
    if not overwrite:
        if shexec("diff {} {}".format(gold, tmp)) != 0:
            print("FAIL: {} not matched by output.".format(gold))
            return False
    return True

def parse_args():
    parser = argparse.ArgumentParser(description="Run gold tests and record performance.")
    parser.add_argument("--overwrite", dest="overwrite", action="store_true",
                        help="Overwrite the gold test records.")
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    with open(TIME_FILE, "w+") as f: f.write("function, time\n")

    models = find_files("*.stan", PERF_MODEL_DIRS)
    models = filter(model_name_re.match, models)
    models = list(filter(lambda m: not m in bad_models, models))
    executables = [m[:-5] for m in models]
    time_step("make_all_models", make, executables)
    #found = False
    for model, exe in zip(models, executables):
        #if not found and model != "examples/example-models/bugs_examples/vol1/rats/rats_stanified.stan":
        #    continue
        #if not found:
        #    found = True
        #    continue
        data = find_data_for_model(model)
        if not data:
            continue
        print(model, data)
        time_step(model, run, exe, data, args.overwrite)
