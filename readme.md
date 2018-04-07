# note
An implementation of **bipartite graph(network)** algorithm.

### stats of time used
#### 1
- data: 10000w
- user:   100w
- item:    10w
- time: 16min

#### 2
- data: 1000w
- user:  10w
- item:   1w
- time: 1:11s

#### 3
- data: 100w
- user:  10w
- item:   1w
- time:   6s

### usage
1. source data config (`train_data`/`train_data_right`) support single file or multi files which in a same path.
2. for single behavior case, set `train_data_right` tobe empty in `config.ini`.
3. for double behavior case, `train_data` indicates left behavior, and `train_data_right` indicates right behavior data.
3. in double behavior case, the normalization (that $rho$, $lambda$ actions on) only consider the right behavior.

### TODO
the function train on disk is not optimized.
