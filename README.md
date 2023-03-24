# Online Turing Machine Simulator

This program simulates an online turing machine using the given config file.

Config files are written according to the following format:

*Note that this implementation does not currently support autonomous states.*

1. A single integer, `n`, denoting the number of states
2. Followed by `n` lines where each line contains the state name and whether it is accepting or not
   1. Example: `q0 accept`
3. A single integer, `m`, denoting the number of transitions
4. Followed by `m` lines describing each transition state
   1.  Format: `current input tape destination write direction`
   2.  Example: `q0 1 _ q1 1 R`
   3.  Use `_` for blanks, `L` for left, `R` for right, and `N` for hold.
5.  