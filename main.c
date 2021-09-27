/*
  Program: Square waveform generator using the spacebar.
  
  The program is designed to be run on the command line. It receives as input
  the name of the file that will be written to and an integer representing the 
  amount of samples that should be generated per second.
  
  Example:
  ./program wave.dat 100

  Technique used: I/O Multiplexing

  Created by Kevin Perez
*/
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>

#define MAX_CHAR_PER_SEC 100

#define TARGET_KEY ' ' // ASCII character associated with spacebar

void initialize_input_buffer(char* buffer)
{
    for ( unsigned t = 0 ; t < MAX_CHAR_PER_SEC ; ++t )
	buffer[t] = '\0';
}

int main(int argc, char** argv)
{
    // Verify program input arguments
    if ( argc < 3 )
    {
	printf("Too few arguments provided.\n");
	return -1;
    }

    if ( argc > 3 )
    {
	printf("Too many arguments provided.\n");
	return -1;
    }

    int total_samples = atoi(argv[2]); // note: atoi not best for conversion.
    
    // Create wavefile. File overwritten if it exists.
    FILE * wavefile = fopen(argv[1], "w");
    
    char buffer[MAX_CHAR_PER_SEC];

    struct termios ttystate;

    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    // Set noncanonical input processing mode to prevent
    // terminal input from being processed.
    // i.e. input not required to be terminated by the \n or EOF delimiters
    ttystate.c_lflag &= ~ICANON;
    
    // set minimum number of bytes that must be available in the
    // (noncanonical) input queue in order for read to return. 
    ttystate.c_cc[VMIN] = 1;
    
    // set the terminal attributes associated with STDIN
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    while ( total_samples-- > 0 )
    {
	int space_bar_pressed = 0;
	sleep(1);

	// timeframe to block before stdin file descriptor becomes ready
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	fd_set readfds;
	FD_ZERO(&readfds); // clear file descriptor set
	FD_SET(STDIN_FILENO, &readfds); // add file descriptor to set
	
	select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

	// if STDIN is ready for reading
	if ( FD_ISSET(STDIN_FILENO, &readfds) )
	{
	    initialize_input_buffer(buffer);
	    read(STDIN_FILENO, buffer, 100);

	    // Check if spacebar has been pressed at least once
	    // within the past second
	    for ( int buf_it = 0; buf_it < MAX_CHAR_PER_SEC; ++buf_it)
	    {
		if ( buffer[buf_it] == TARGET_KEY )
		{
		    space_bar_pressed = 1;
		    break;
		}
	    }
	}

	if ( space_bar_pressed == 1 )
	{
	    fputs("1\n", wavefile);
	}
	else
	{
	    fputs("0\n", wavefile);
	}
    }
    
    return 0;
}


/*

Notes:


[1] I gave the buffer array a fixed size of 100 elements because it is not possible, at least on my computer, to type more than 100 characters into the terminal in one second when the keyboard repeat rate is maximized and repeat delay is minimized (via OS keyboard settings). 

[2] Realized the assignment instructions says "1's when a key is pressed" (i.e. when any key is pressed) yet I spent hours working on getting the program to output 1's only when the spacebar is pressed... 

[3] If I want to check if the spacebar has been pressed AT the edge of the waveform, then the for loop should iterate from 100 to 0 and it should be verified if the last non-NULL character is a whitespace. But this would not be very precise because the spacebar can be tapped at the start of the sleep cycle. A long-hold threshold can be added that would detect a long press by counting the amount of contiguous whitespaces starting from the end of the buffer; but the threshold would vary per computer because of the keyboard repeat rate mentioned in note [1].

References:
[1] https://www.gnu.org/software/libc/manual/html_node/Noncanonical-Input.html
[2] https://man7.org/linux/man-pages/man2/read.2.html
[3] https://man7.org/linux/man-pages/man2/select.2.html

 */

