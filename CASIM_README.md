# casim
Computer Architecture Simulation Infrastructure for CSCE 614 Computer Architecture



### 1. Environemnt setup

Everytime you want to build or run zsim, you need to setup the environment variables first.

```
$ source setup_env
```



#### 2. Download benchmark inputs

```
$ cd benchmarks/parsec-2.1
$ wget http://students.cse.tamu.edu/jyhuang/csce614/parsec smallinputs.tar.gz
$ tar xzvf parsec smallinputs.tar.gz
```



#### 3. Setup zsim

##### 3.1 Download config files and runscript

```
$ cd zsim
$ wget http://students.cse.tamu.edu/jyhuang/csce614/configs.tar.gz
$ wget http://students.cse.tamu.edu/jyhuang/csce614/hw2runscript
$ tar xzvf configs.tar.gz
```

##### 3.2 Compile zsim

```
$ scons -j4
```

##### 3.3 Launch a test to run

```
./build/opt/zsim tests/simple.cfg
```



###### For more information, check `zsim/README.md`
