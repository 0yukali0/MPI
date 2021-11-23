# computation time
1. matrix A 10k * 10k
2. vector B 10k
3. result C 10k
```
core   |     1    |     10    |      10      |     10    |
----------------------------------------------------------
method | row-wise |  row-wise | column-wise  |   hybrid  |
----------------------------------------------------------
time   | 0.363186 |  0.325665 | 0.315914     |   working |
```
