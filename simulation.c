/*
* Isaac Cardoso
* This program simulates a simple computer system that consists of a CPU and memory.
* It uses the fork function to create a new process which simulates the CPU
* while the parent process simulates the memory.
* The program ends when the END(50) instruction is encountered or an error ocurrs.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

//function declarations
void error_exit(char *s);
int readMem(int arr[], int address);
void writeMem(int arr[], int address, int data);

/*
********************************************************************************
********************************** main ****************************************
********************************************************************************
*/
int main(int argc, char *argv[])
{
	//check if number of arguments is 3
	if(argc != 3){//if not, exit with error message
		error_exit("Invalid number of arguments");
	}

	//check if input file exists
	if(access(argv[1], F_OK) == -1){//if not, exit with error message
		error_exit("Input file does not exist");
	}

	//variables
	int timeToInterrupt = atoi(argv[2]);//time to interrupt 
	int interruptCounter = 0;//increases after execution of an instruction
	int result;//to store the result of the fork


	//create pipes to share data between processes
	int pipe1[2];
	int pipe2[2];
	//create pipe, exit if error occurs 
	if(pipe(pipe1) == -1 || pipe(pipe2) == -1){
		error_exit("pipe() failed");
	}

	//Create another process for the CPU function
	result = fork();

	//**********************************************************************************
	//****************************** FORK ERROR ****************************************
	if(result == -1)
		error_exit("The fork failed!");

	//**********************************************************************************
	//*************************** CHILD PROCESS (CPU) **********************************
	else if( result == 0){

		//child reads from pipe1 and writes to pipe2
		//close pipes not needed by the child
		close(pipe1[1]);
		close(pipe2[0]);

		//child variables
		int mode; // kernel mode(0), user mode(1)
		int inInterrupt;//block interrupts when equal to 1
		int SP, PC, IR, AC, X, Y;//CPU registers
		int operand;//to store other data from program
		int tempSP;//tem holder for stack pointer
		int r,w;//read and write signals to send to parent
		int endSignal = -1;//lets the parent know it's done sending signals

		//initialize registers and variables
		PC = 0;//point to the first instruction of program
		SP = 999; //point to the begining of the user stack
		AC = 0;
		X = 0;
		Y = 0;
		operand = 0;
		mode = 1; //user mode
		inInterrupt = 0;//in interrupt to false
		r = 0;//read signal
		w = 1; //write signal

		//fetch the first instruction
		write(pipe2[1], &r, sizeof(int));//send the read signal
		write(pipe2[1], &PC, sizeof(int));//send the location
		read(pipe1[0], &IR, sizeof(int));//read the response and store into IR

		//exit loop when the END(50) instruction is reached
		while(IR != 50){
			//do according to the instruction number
			switch(IR){
				//********************************************************
				//		1.	Load value:	Load the value into the AC
				//********************************************************
				case 1: 
					PC++; //increase PC by 1
					//get the value to load into AC
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &AC, sizeof(int));//store value into AC
					PC++;
					break;

				//*********************************************************
				//	2. Load addr: Load the value at the address into the AC
				//*********************************************************
				case 2:
					PC++; //increase PC by 1
					//get the address
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand

					//check for memory violation
					if(mode && (operand >= 1000)){
						//send end signal so parent can stop waiting for signals
						write(pipe2[1], &endSignal, sizeof(int));
						//display error message
						printf("Memory violation: accessing system address %d in user mode\n", operand);
						_exit(0);//terminate child process
					}

					//get the value to store into AC
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &operand, sizeof(int));//send the location
					read(pipe1[0], &AC, sizeof(int));//store value into AC
					PC++;
					break;

				//********************************************************
				//	3. LoadInd addr: Load the value from the address found 
				// 	    in the given address into the AC
				//********************************************************
				case 3:
					PC++; //increase PC by 1
					//get the address
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand

					//check for memory violation
					if(mode && (operand >= 1000)){
						//send end signal so parent can stop waiting for signals
						write(pipe2[1], &endSignal, sizeof(int));
						//display error message
						printf("Memory violation: accessing system address %d in user mode\n", operand);
						_exit(0);//terminate child process
					}

					//get the value at address stored in operand
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &operand, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand again
					//get the value at location stored in operand
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &operand, sizeof(int));//send the location
					read(pipe1[0], &AC, sizeof(int));//store value into AC
					PC++;
					break;

				//********************************************************
				//	4. LoadIdxX addr: Load the value at (address+X) 
				//	   into the AC
				//********************************************************
				case 4:
					PC++; //increase PC by 1
					//get the address
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand

					//check for memory violation
					if(mode && (operand >= 1000)){
						//send end signal so parent can stop waiting for signals
						write(pipe2[1], &endSignal, sizeof(int));
						//display error message
						printf("Memory violation: accessing system address %d in user mode\n", operand);
						_exit(0);//terminate child process
					}

					//(address+X)
					operand = operand + X;

					//get the value at location operand from memory
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &operand, sizeof(int));//send the location
					read(pipe1[0], &AC, sizeof(int));//store value into AC
					PC++;
					break;

				//********************************************************
				//	5. LoadIdxY addr: Load the value at (address+Y)
				//     into the AC
				//********************************************************
				case 5:
					PC++; //increase PC by 1
					//get the address
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand

					//check for memory violation
					if(mode && (operand >= 1000)){
						//send end signal so parent can stop waiting for signals
						write(pipe2[1], &endSignal, sizeof(int));
						//display error message
						printf("Memory violation: accessing system address %d in user mode\n", operand);
						_exit(0);//terminate child process
					}
					
					operand = operand + Y; ////(address+Y)

					//get the value at location operand from memory
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &operand, sizeof(int));//send the location
					read(pipe1[0], &AC, sizeof(int));//store value into AC
					PC++;
					break;

				//********************************************************
				//	6. LoadSpX: Load from (SP+X) into the AC
				//********************************************************
				case 6:
					PC++; //increase PC by 1
					operand = SP + X; // (SP + X)
					//get the value at location operand from memory
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &operand, sizeof(int));//send the location
					read(pipe1[0], &AC, sizeof(int));//store value into AC
					break;

				//***********************************************************
				//	7. Store addr: Store the value in the AC into the address
				//***********************************************************
				case 7:
					PC++; //increase PC by 1
					//get the addres
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand

					//check for memory violation
					if(mode && (operand >= 1000)){
						//send end signal so parent can stop waiting for signals
						write(pipe2[1], &endSignal, sizeof(int));
						printf("Memory violation: accessing system address %d in user mode\n", operand);
						_exit(0);
					}

					//send address and data to be written into memory to the parent process
					write(pipe2[1], &w, sizeof(int));//send write signal
					write(pipe2[1], &operand, sizeof(int));//send the address
					write(pipe2[1], &AC, sizeof(int));//send data stored in AC
					PC++;
					break;

				//********************************************************
				//	8. Get: Gets a random int from 1 to 100 into the AC
				//********************************************************
				case 8: 
					PC++; //increase PC by 1
					srand(time(NULL));
					AC = rand() % 101;//generate random number
					if(AC == 0)//if 0 was generated change to 1
						AC = 1;
					break;

				//********************************************************
				//	9. Put port: Write AC as int or char depending on port
				//********************************************************
				case 9:
					PC++; //increase PC by 1
					//get the port
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand

					//If port=1, write AC as an int to the screen
					if(operand == 1){
						printf("%d", AC);
					}
					//If port=2, write AC as a char to the screen
					else if(operand == 2){
						printf("%c", (char)AC);
					}
					PC++;
					break;

				//********************************************************
				//	10. AddX: Add the value in X to the AC
				//********************************************************
				case 10: 
					PC++; //increase PC by 1
					AC = AC + X;
					break;

				//********************************************************
				//	11. AddY: Add the value in Y to the AC
				//********************************************************
				case 11:
					PC++; //increase PC by 1
					AC = AC + Y;
					break;

				//********************************************************
				//	12. SubX: Subtract the value in X from the AC
				//********************************************************
				case 12: 
					PC++; //increase PC by 1
					AC = AC - X;
					break;

				//********************************************************
				//	13. SubY: Subtract the value in Y from the AC
				//********************************************************
				case 13: 
					PC++; //increase PC by 1
					AC = AC - Y;
					break;

				//********************************************************
				//	14. CopyToX: Copy the value in the AC to X
				//********************************************************
				case 14:
					PC++; //increase PC by 1
					X = AC;
					break;

				//********************************************************
				//	15. CopyFromX : Copy the value in X to the AC
				//********************************************************
				case 15:
					PC++; //increase PC by 1
					AC = X;
					break;

				//********************************************************
				//	16. CopyToY: Copy the value in the AC to Y
				//********************************************************
				case 16: 
					PC++; //increase PC by 1
					Y = AC;
					break;

				//********************************************************
				//	17. CopyFromY: Copy the value in Y to the AC
				//********************************************************
				case 17: 
					PC++; //increase PC by 1
					AC = Y;
					break;

				//********************************************************
				//	18. CopyToSp: Copy the value in AC to the SP
				//********************************************************
				case 18:
					PC++; //increase PC by 1
					SP = AC;
					break;

				//********************************************************
				//	19. CopyFromSp: Copy the value in SP to the AC
				//********************************************************
				case 19: 
					PC++; //increase PC by 1
					AC = SP;
					break;

				//********************************************************
				//	20. Jump addr: Jump to the address
				//********************************************************
				case 20: 
					PC++; //increase PC by 1
					//get the address
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand
					PC = operand; //value in operand is the new PC
					break;

				//********************************************************
				//	21. JumpIfEqual addr: Jump to the address only 
				//      if the value in the AC is zero
				//********************************************************
				case 21:
					PC++; //increase PC by 1
					if(AC == 0){
						//get the address
						write(pipe2[1], &r, sizeof(int));//send read signal
						write(pipe2[1], &PC, sizeof(int));//send the location
						read(pipe1[0], &operand, sizeof(int));//store value into operand
						PC = operand;
					}
					else{
						PC++; //increase PC by 1
					}
					break;

				//********************************************************
				//	22. JumpIfNotEqual addr: Jump to the address only 
				//      if the value in the AC is not zero
				//********************************************************
				case 22: 
					PC++; //increase PC by 1
					//get the address
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand
					if(AC != 0)
						PC = operand;
					else
						PC++;
					break;

				//********************************************************
				//	23. Call addr: Push return address onto stack, 
				//      jump to the address
				//********************************************************
				case 23: 
					PC++; //increase PC by 1
					//get the address
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &PC, sizeof(int));//send the location
					read(pipe1[0], &operand, sizeof(int));//store value into operand

					PC++; //return addres

					//push return address onto user stack
					SP--;//decrement stack pointer before push
					write(pipe2[1], &w, sizeof(int));//send write signal
					write(pipe2[1], &SP, sizeof(int));//send the address
					write(pipe2[1], &PC, sizeof(int));//send data stored in PC

					PC = operand; // update PC to the intruction to jump to
					break;

				//********************************************************
				//	24. Ret: Pop return address from the stack, 
				//      jump to the address
				//********************************************************
				case 24:
					PC++; //increase PC by 1
					//pop return address from location at SP
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &SP, sizeof(int));//send the location
					read(pipe1[0], &PC, sizeof(int));//store into PC 
					SP++;//increment stack pointer after pop
					break;

				//********************************************************
				//	25. IncX: Increment the value in X
				//********************************************************
				case 25: 
					PC++; //increase PC by 1
					X = X + 1;
					break;

				//********************************************************
				//	26. DecX: Decrement the value in X
				//********************************************************
				case 26: 
					PC++; //increase PC by 1
					X = X - 1;
					break;

				//********************************************************
				//	27. Push: Push AC onto stack
				//********************************************************
				case 27: 
					PC++; //increase PC by 1
					SP--;//decrement stack pointer before push
					write(pipe2[1], &w, sizeof(int));//send write signal
					write(pipe2[1], &SP, sizeof(int));//send the address
					write(pipe2[1], &AC, sizeof(int));//send data stored in AC
					break;

				//********************************************************
				//	28. Pop: Pop from stack into AC
				//********************************************************
				case 28: 
					PC++; //increase PC by 1
					//pop value from stack at location SP and store it in AC
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &SP, sizeof(int));//send the location
					read(pipe1[0], &AC, sizeof(int));//store value into AC
					SP++;//increment stack pointer after pop
					break;

				//********************************************************
				//	29. Int: Perform system call
				//********************************************************
				case 29: 
					PC++; //increase PC by 1
					if(inInterrupt)//avoid nested interrupts
						break;

					mode = 0; //enter kernel mode
					tempSP = SP; //temporarily hold current stack pointer value
					SP = 1999; //point to the system stack
					inInterrupt = 1; //set in interrupt flag to avoid nested interrupts

					//Save SP, PC onto the system stack
					//push current SP value temporarily held in tempSP onto sys stack
					SP--;//decrement stack pointer before push
					write(pipe2[1], &w, sizeof(int));//send write signal
					write(pipe2[1], &SP, sizeof(int));//send the address
					write(pipe2[1], &tempSP, sizeof(int));//send data stored in tempSP

					//push current PC value onto sys stack
					SP--;//decrement stack pointer before push
					write(pipe2[1], &w, sizeof(int));//send read signal
					write(pipe2[1], &SP, sizeof(int));//send the address
					write(pipe2[1], &PC, sizeof(int));//send data stored in PC

					PC = 1500; //execute from address 1500
					break;

				//********************************************************
				//	30. IRet: Return from interrupt
				//********************************************************
				case 30: 
					//pop PC from sys stack and store in PC
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &SP, sizeof(int));//send the location
					read(pipe1[0], &PC, sizeof(int));//store value into PC
					SP++;//increment stack pointer after pop

					//pop user SP from sys stack and store in tempSP
					write(pipe2[1], &r, sizeof(int));//send read signal
					write(pipe2[1], &SP, sizeof(int));//send the location
					read(pipe1[0], &tempSP, sizeof(int));//store value into tempSP
					SP++;//increment stack pointer after pop

					SP = tempSP;//point to the user stack

					mode = 1; //chage to user mode
					inInterrupt = 0; //enable interrupts
					break;

					default:
						//invalid instruction
						//send end signal
						write(pipe2[1], &endSignal, sizeof(int));
						//print error message
						error_exit("Invalid instruction");

			}//end switch case statements

			//if not currently executing an interrupt, increase counter
			if(!inInterrupt)
				interruptCounter++;

			//check for timer interrupts
			if(interruptCounter == timeToInterrupt){

				if(inInterrupt){//avoid nested interrupts
					//do nothing
				}
				else{
					interruptCounter = -1;//reset counter
					mode = 0; //enter kernel mode
					tempSP = SP; //temporarily hold current stack pointer value
					SP = 1999; //point to the system stack
					inInterrupt = 1; //set in interrupt flag to avoid nested interrupts

					//Save SP, PC and the system stack
					//push current SP value temporarily held in tempSP onto sys stack
					SP--; //decrement stack pointer before push
					write(pipe2[1], &w, sizeof(int));//send write signal
					write(pipe2[1], &SP, sizeof(int));//send the address
					write(pipe2[1], &tempSP, sizeof(int));//send data stored in operand

					//push current PC value onto sys stack
					SP--; //decrement stack pointer before push
					write(pipe2[1], &w, sizeof(int));//send read signal
					write(pipe2[1], &SP, sizeof(int));//send the address
					write(pipe2[1], &PC, sizeof(int));//send data stored in PC

					PC = 1000; //execute from address 1000
				}
			}

			//fetch the next instruction
			write(pipe2[1], &r, sizeof(int));//send the read signal
			write(pipe2[1], &PC, sizeof(int));//send the location
			read(pipe1[0], &IR, sizeof(int));//read the response and store it in IR

		}//end while loop

		//send end signal so parent can stop waiting for signals
		write(pipe2[1], &endSignal, sizeof(int));
	}//end of child process
	//****************************************************************************************


	//****************************************************************************************
	//*************************** PARENT PROCESS (MEMORY) ************************************
	else{
		
		//parent reads from pipe2 and writes to pipe1
		//close pipes not needed by the parent
		close(pipe1[0]);//
		close(pipe2[1]);

		//variables
		int memory[2000] = {0};//to store instructions,memory data, and stacks
		char *fileName;//to store the input file name

		//get the input file name from argument list
		//store it in fileName
		fileName = malloc(strlen(argv[1]));
		strcat(fileName, argv[1]);

		//open file for reading
		FILE *file;
		char buff[255];
		file = fopen(fileName, "r");

		/*
		* INITIALIZE MEMORY
		* Read the file until the end is encountered. 
		* The instruction number should be the first word in each line.
		* If a blank line is encountered, skip it and continue. 
		* If a period is encountered, move to that position in the 
		* array and continue reading the file and storing the 
		* instruction number at that position.
		*/
		int position = 0;
	    while(fgets(buff, 255, (FILE*)file) != NULL){
	    	//skip empty lines
	    	if(buff[0] == '\n' || buff[0] == '\t' || buff[0] == ' '){
	    		continue;
	    	}
	    	//jump to another position in the memory array
	    	else if(buff[0] == '.'){
	   			memmove(&buff[0], &buff[1], strlen(buff) - 0);// remove the period 
	   			position = atoi(buff);// cast to an int and update position
	   		}
	    	else{
	    		sscanf(buff, "%d", &memory[position]);
	    		position++;
	    	}
	    }//end while

	    //local variables to store values to pass as parameters to read/write function
	    int signal; 
	    int addr; 
	    int data;

	    //get signal from child
	    read(pipe2[0], &signal, sizeof(int));

	    while(signal != -1){
	    	if(signal == 0){ //read from memory
	    		read(pipe2[0], &addr, sizeof(int));
	    		if(addr > 1999)
	    			error_exit("Memory Violation. Out of bounds");
	    		else{
	    			data = readMem(memory, addr);
	    			write(pipe1[1], &data, sizeof(int));
	    	}

	    	}
	    	else if(signal == 1){ // write to memory
	    		read(pipe2[0], &addr, sizeof(int));
	    		if(addr > 1999)
	    			error_exit("Memory Violation. Out of bounds");
	    		else{
		    		read(pipe2[0], &data, sizeof(int));
		    		writeMem(memory, addr, data);
		    	}
	    	}
	    	//get next signal from child
	    	read(pipe2[0], &signal, sizeof(int));
	    }//end while

	}

	//wait for child process to end
	waitpid(-1, NULL, 0);
	return 0;
}
//***************************** End of main ****************************************
//**********************************************************************************

//Print error message on the screen and exit
void error_exit(char *s){
   fprintf(stderr,"\nERROR: %s\n", s);
      exit(1);
}

/* 
* Takes an int array and an integer as its parameters 
* and returns the data from the array at that integer location. 
*/
int readMem(int arr[], int address){
	return arr[address];

}

/*
* Takes 3 paramters, an int array, an integer to point
* to the location where the third parameter, data, will be writen. 
*/
void writeMem(int arr[], int address, int data){
	arr[address] = data;
}
