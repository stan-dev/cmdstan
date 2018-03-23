#!/usr/bin/python

import argparse
import csv
from collections import defaultdict
import os
import re
import subprocess
from difflib import SequenceMatcher
from fnmatch import fnmatch
from functools import wraps
from multiprocessing.pool import ThreadPool
from time import time
import xml.etree.ElementTree as ET

GOLD_OUTPUT_DIR = "tests/golds/"

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
    return end-start, res

def shexec(command):
    print(command)
    returncode = subprocess.call(command, shell=True)
    if returncode != 0:
        raise Exception("{} from '{}'!".format(returncode, command))
    return returncode

def make(targets, j=8):
    shexec("make -j{} ".format(j) + " ".join(targets))

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
     , "examples/example-models/bugs_examples/vol2/pines/pines-4.stan"
    ])

def avg(coll):
    return float(sum(coll)) / len(coll)

def stdev(coll, mean):
    return (sum((x - mean)**2 for x in coll) / (len(coll) - 1)**0.5)

def csv_summary(csv_file):
    d = defaultdict(list)
    with open(csv_file, 'rb') as raw:
        headers = None
        for row in csv.reader(raw):
            if row[0].startswith("#"):
                continue
            if headers is None:
                headers = row
                continue
            for i in range(0, len(row)):
                d[headers[i]].append(float(row[i]))
    res = {}
    for k, v in d.items():
        if k.endswith("__"):
            continue
        mean = avg(v)
        res[k] = (mean, stdev(v, mean))
    return res

def format_summary_lines(summary):
    return ["{} {} {}\n".format(k, avg, stdev) for k, (avg, stdev) in summary.items()]

def parse_summary(f):
    d = {}
    for line in f:
        param, avg, stdev = line.split()
        d[param] = (float(avg), float(stdev))
    return d

def run(exe, data, overwrite, check_golds, check_golds_exact, runs):
    fails, errors = [], []
    gold = os.path.join(GOLD_OUTPUT_DIR, exe.replace("/", "_") + ".gold")
    tmp = gold + ".tmp"
    try:
        total_time = 0
        for i in range(runs):
            start = time()
            shexec("{} sample data file={} random seed=1234 output file={}"
                   .format(exe, data, tmp))
            end = time()
            total_time += end-start
    except Exception as e:
        return fails, errors + [str(e)]
    summary = csv_summary(tmp)
    with open(tmp, "w+") as f:
        f.writelines(format_summary_lines(summary))

    if overwrite:
        shexec("mv {} {}".format(tmp, gold))
    elif check_golds or check_golds_exact:
        gold_summary = {}
        with open(gold) as gf:
            gold_summary = parse_summary(gf)

        for k, (mean, stdev) in gold_summary.items():
            if stdev < 0.00001: #XXX Uh...
                continue
            err = abs(summary[k][0] - mean)
            if check_golds_exact and err > check_golds_exact:
                print("FAIL: {} param {} |{} - {}| not within {}"
                      .format(gold, k, summary[k][0], mean, check_golds_exact))
                fails.append((k, mean, stdev, summary[k][0]))
            elif err > 0.0001 and (err / stdev) > 0.5:
                print("FAIL: {} param {} not within ({} - {}) / {} < 0.5"
                      .format(gold, k, summary[k][0], mean, stdev))
                fails.append((k, mean, stdev, summary[k][0]))
    return total_time, (fails, errors)

def test_results_xml(tests):
    failures = str(sum(1 if x[2] else 0 for x in tests))
    time_ = str(sum(x[1] for x in tests))
    root = ET.Element("testsuite", failures=failures, name="Performance Tests",
                      tests=str(len(tests)), time=str(time_))
    for model, time_, fails, errors in tests:
        time_ = str(time_)
        testcase = ET.SubElement(root, "testcase", classname=model, time=time_)
        for fail in fails:
            testcase = ET.SubElement(root, "failure", type="param mismatch")
            testcase.text = ("param {} got mean {}, gold has mean {} and stdev {}"
                             .format(fail[0], fail[3], fail[1], fail[2]))
        for error in errors:
            testcase = ET.SubElement(root, "error", type="Exception")
            testcase.text = error
    return ET.ElementTree(root)

def parse_args():
    parser = argparse.ArgumentParser(description="Run gold tests and record performance.")
    parser.add_argument("directories", nargs="+")
    parser.add_argument("--check-golds", dest="check_golds", action="store_true",
                        help="Run the gold tests and check output within loose boundaries.")
    parser.add_argument("--check-golds-exact", dest="check_golds_exact", action="store",
                        help="Run the gold tests and check output to within specified tolerance",
                        type=float)
    parser.add_argument("--overwrite-golds", dest="overwrite", action="store_true",
                        help="Overwrite the gold test records.")
    parser.add_argument("--runs", dest="runs", action="store", type=int,
                        help="Number of runs per benchmark.")
    parser.add_argument("-j", dest="j", action="store", type=int)
    return parser.parse_args()

def process_test(overwrite, check_golds, check_golds_exact, runs):
    def process_test_wrapper(tup):
        # TODO: figure out the right place to compute the average or maybe don't compute the average.
        model, exe, data = tup
        time_, (fails, errors) = run(exe, data, overwrite, check_golds, check_golds_exact, runs)
        average_time = time_ / runs
        return (model, average_time, fails, errors)
    return process_test_wrapper

if __name__ == "__main__":
    args = parse_args()

    models = find_files("*.stan", args.directories)
    models = filter(model_name_re.match, models)
    models = list(filter(lambda m: not m in bad_models, models))
    executables = [m[:-5] for m in models]
    time_step("make_all_models", make, executables, args.j or 4)
    tests = [(model, exe, find_data_for_model(model))
             for model, exe in zip(models, executables)]
    tests = filter(lambda x: x[2], tests)
    tp = ThreadPool(args.j)
    results = tp.map(process_test(args.overwrite, args.check_golds,
                                            args.check_golds_exact, args.runs), tests)
    test_results_xml(results).write("performance.xml")
