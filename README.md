# SimpleComputerSystemSimulation
Simulation of a simple computer system that consists of a CPU and Memory. This program is written in C and it uses Unix fork for processes and Unix pipe for communication.
This is a project I worked on for an Operating Systems class. 
*****************************************************************************************************************************************
### Memory
Memory can hold up to 2000 integer entries. First half is used for the user program and the other half is for the system.
It supports a readMem and a writeMem operation to read from a specific address and to write to a specific address with the address passed as a parameter. 
### CPU
Registers: PC, SP, IR, AC, X, Y.
It runs the user program at address 0.
Instructions are fetched from memory and stored into the register IR and operands are stored in a local variable.
Each instruction is executed one at a time.
The user stack is stored at the end of user memory while the system stack is stored at the end of system memory. 
The program ends after the execution of the END instruction.
User program cannot access system memory. 
It supports the following instructions:
1 = Load value,
2 = Load addr,
3 = LoadInd addr,
4 = LoadIdxX addr,
5 = LoadIdxY addr,
6 = LoadSpX,
7 = Store addr,
8 = Get,
9 = Put port,
10 = AddX,
11 = AddY,
12 = SubX,
13 = SubY,
14 = CopyToX,
15 = CopyFromX,
16 = CopyToY,
17 = CopyFromY,
18 = CopyToSp,
19 = CopyFromSp,
20 = Jump addr,
21 = JumpIfEqual addr,
22 = JumpIfNotEqual addr,
23 = Call addr,
24 = Ret,
25 = IncX,
26 = DecX,
27 = Push,
28 = Pop,
29 = Int,
30 = IRet,
50 = End

### Program Execution
To execute the program you first need to compile it by using a C compiler of your choice
When executing the program you need to include 2 parameters, the first is a text file which contains the user program 
and the second is a positive number which will be used to interrupt the processor after this number of executions. 
