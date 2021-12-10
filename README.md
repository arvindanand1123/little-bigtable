## Single Threaded Hashtable
+ `hash.c` contains a glib hashtable test that generates 4k entries, stores them, and prints out the contents of the hashtable in hashcout.txt.
+ Running it is a matter of running `./buildrunhash.sh`. First, you must change it's permissions to run as an executable via `chmod +x`.
## BigTable
+ Run `cmake .`, then run `make` to compile the code.
+ Running the binary can be done with `./bigtable` as the program is equipped with default values. However, the following parameters will affect the run time as such:
    + `--N=$value` specifies number of hashtables
    + `--n=$value` specifies the total items to be inserted into the BigTable such that the keys are `[0...n]`
    + `--t=$value` specifies the number of worker threads to spawn
    + `--p` specifies additional output: it allows for detailed output of keys and values stored
+ Running the test script is a matter of running `./buildrunbt.sh` and the output would be piped to `bigtablecout.txt`. Similar to `hash.c`, `chmod +x` is required. The parameters can be adjusted by altering the script.
+ Running the benchmark requires VTune setup.