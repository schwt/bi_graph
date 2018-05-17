# note
An implementation of **bipartite graph(network)** algorithm.

## 1 usage:
#### 1.1 input format:
 `"uid,pid,rating,timestamp"`
- supported dilimeter: `",", " ", "\t"`, can be set in `config.ini`
- input data support one file or a whole directory (recurent child directory willnot be readed)

### 1.2 config setting
#### 1.2.1 train effect
- `lambda`: hot item punishing
- `rho`: hot user punishing
- `tau`: decay time scale(unit=second) between to behaviors
####1.2.2 application types
- `train_data_right`(optional): use for double behavior type
- `valid_reco_id`(optional): limit valid reco item set

### 1.3 running
- build: `make clean; make`
- running: `./graph config.ini`

## 2 stats of time used
```
data: 10000w
user:   100w
item:    10w
time: 16min

data: 1000w
user:  10w
item:   1w
time: 1:11s

data: 100w
user:  10w
item:   1w
time:   6s
```

## 3 note
1. source data config (`train_data`/`train_data_right`) support single file or multi files which in a same path.
2. for single behavior case, set `train_data_right` tobe empty in `config.ini`.
3. for double behavior case, `train_data` indicates left behavior, and `train_data_right` indicates right behavior data.
3. in double behavior case, the normalization (that $rho$, $lambda$ actions on) only consider the right behavior.

## 4 TODO
the function train on disk is not optimized.

