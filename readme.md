# note
An implementation of **bipartite graph(network)** algorithm.

mathimatical expression:

$$
sim(i \to j) = \frac 1 {(\sum_u r_{uj})^{\lambda}} \sum_u \frac {r_{ui}r_{uj}}{(\sum_k r_{uk})^{\rho}} T(t_{ui} - t_{uj}, \tau)
$$

where time decay factor has two configurable types: half-life decay, and Guassian decay,

$$
T_1(t, \tau) = 2^{-t / \tau}
T_2(t, \tau) = \mathcal{N}(t, \mu=0, \sigma=\tau)
$$


## 1 usage:
#### 1.1 input format:
- neccesary columns: `"uid,pid,rating,timestamp"`
- corrisponding columns are setted in `config.ini`
- supported dilimeter: `"\t", ",", " "`, can be set in `config.ini`
- input data support one file or a whole directory (recurent child directory will be ignored)

### 1.2 config setting
#### 1.2.1 train parameters
- `lambda`: hot item punishing
- `rho`: hot user punishing
- `tau`: decay time scale(unit=second) between to behaviors
####1.2.2 application types
- `train_data_right`(optional): use for double behavior type
- `valid_reco_id`(optional): limit valid reco items set
- `invalid_reco_id`(optional): blacklist reco items

### 1.3 running
- build: `make clean; make`
- running: `./graph config.ini`

## 2 performance
### 2.1 use single thread training in memory
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
### 2.2 use 30 threads training in memory
```
data: 98000w
user:  1200w
item:    52w
train file: 56GB
load time : 21min
train time: 13min
total time: 50min
```

## 3 note
1. multy threads mode is extreamly suggested.
2. for single behavior case, set `train_data_right` tobe empty in `config.ini`.
3. for double behavior case, `train_data` indicates left behavior, and `train_data_right` indicates right behavior data.
3. in double behavior case, the normalization (that $rho$, $lambda$ actions on) only consider the right behavior.
4. the function train on disk is not optimized.

