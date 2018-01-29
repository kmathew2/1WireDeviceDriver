#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include<fcntl.h> //wrapper
#include <errno.h> //system error
#include <sys/ioctl.h> //ioctl
#include <string.h>
#define DRIVER_NAME "/dev/delayLED"	//setting the device name

//struct that holds the LED hex pixel data 
typedef struct LEDdata {   
	char LEDhexData[16*6];
	} LED_data;

int fd;

int main()
{
	int count,choice,i;
	fd = open(DRIVER_NAME, O_RDWR);	//opening the device driver
	if (fd == -1)
	{
  		printf("Error in opening file \n");
 
	}	
	
	printf("Enter 1 for custom pattern or 2 for predefined pattern :\n");
	scanf("%d", &choice);

	if(choice == 1)
	{
	printf("Enter the number of LEDs :\n");
	scanf("%d", &count);
	printf("number of LEDs %d\n",count);

	char* buffer;
	LED_data *ptr = malloc(sizeof(LED_data));	
	buffer = (char*)ptr;

	printf("Enter the LED data:");
	scanf("%s", ptr->LEDhexData);

	ssize_t wr= write(fd, buffer, count);
	if(wr != 0)
	{
		printf("Write not successful !!!");
	}
	free(ptr);
	}
	else
	{
		char* buffer1;
		LED_data *ptr1 = malloc(sizeof(LED_data));	
		buffer1 = (char*)ptr1;
		char* buffer2;
		LED_data *ptr2 = malloc(sizeof(LED_data));	
		buffer2 = (char*)ptr2;
		char* buffer3;
		LED_data *ptr3 = malloc(sizeof(LED_data));	
		buffer3 = (char*)ptr3;
		strcpy(ptr1->LEDhexData,"ff000000ff000000ffff000000ff000000ffff000000ff000000ffff000000ff000000ffff000000ff000000ff");
		strcpy(ptr2->LEDhexData,"00ff000000ffff000000ff000000ffff000000ff000000ffff000000ff000000ffff000000ff000000ffff0000");
		strcpy(ptr3->LEDhexData,"0000ffff000000ff000000ffff000000ff000000ffff000000ff000000ffff000000ff000000ffff000000ff00");

		for(i=1;i<10;i++)
		{
		ssize_t wr1= write(fd, buffer1, 15);
		if(wr1 != 0)
		{
			printf("Write1 not successful !!!");
		}
		

		sleep(1);

		
		ssize_t wr2= write(fd, buffer2, 15);
		if(wr2 != 0)
		{
			printf("Write2 not successful !!!");
		}
		

		sleep(1);

		
		ssize_t wr3= write(fd, buffer3, 15);
		if(wr3 != 0)
		{
			printf("Write3 not successful !!!");
		}
		sleep(1);
		}
		
		free(ptr1);
		free(ptr2);
		free(ptr3);
	}
	printf("LED data successfully passed !!!");

	return 0;
}
