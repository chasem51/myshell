CS410 Assignment 2

Teammates -- UID:

Chase Maivald -- U18719879,
Kaelyn Shinbashi -- U6256519,
Jared Yellen -- U62909356
--

Setup:

`<make clean>`
>Cleans working dir from all executable, object, library files

`<make all>`
>Compiles myshell.c, matmult_t.c, matmult_p.c, matformatter.c into their respective executables

Usage:

`<./myshell>` 
>Starts an interative command line interpreter environment called "myshell". The prompt accepts certain cmd operators for purposes of:

1. Program file(s): 
`cmd` 
2. Sequence of commands: 
`cmd; cmd`
3. Stdout redirection: 
`cmd > output-file`
OR 
`cmd 1> output-file`
4. Stderror redirection: 
`cmd 2> output-file`
5. Stdout AND Stderror redirection: 
`cmd &> output-file`
6. Stdinput redirection: 
`cmd < input-file`
7. Unidirectional piping: 
`cmd1 | cmd2 | cmd3`
8. Background cmds: 
`cmd &`

`<./matformatter < input-matrices>` 
> Outputs matrice's transpose from input-matrices file by stdin stream

`<./matmult_p < input-matrices>` 
> Outputs multi-process matrix multiplication of input-matrix pairs from file by stdin stream

`<./matmult_t < input-matrices>` 
> Outputs multi-threaded matrix multiplication of input-matrix pairs from file by stdin stream

