#include "scheduler.h"
#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>




ssize_t read_wrap(int fd, void * buf, size_t count) {
	ssize_t returnValue;
	int readResult;
	struct aiocb myaiocb;

	// in case aiocb is not initialized as 0 
	memset(&myaiocb, 0, sizeof(struct aiocb));

	// initialize fields in aiocb
	myaiocb.aio_fildes = fd;
	myaiocb.aio_buf = buf;
	myaiocb.aio_nbytes = count;
	myaiocb.aio_sigevent.sigev_notify = SIGEV_NONE;

	// careful about offset
	if(fd){
		myaiocb.aio_offset = lseek(fd, 0, SEEK_CUR);
	}	
	else{
		myaiocb.aio_offset = 0;
	}

	readResult = aio_read(&myaiocb);

	int counter =0;
	while(aio_error(&myaiocb) == EINPROGRESS){
		yield();
		++counter;
	}

	printf("Finished!!!\n");
	printf("Tried %d times to poll\n", counter);

	returnValue = aio_return(&myaiocb);

	// set offset after read operation
	// current offset should be old offset + how many bytes read
	if(returnValue >= 0){
		myaiocb.aio_offset = lseek(fd, myaiocb.aio_offset + returnValue, SEEK_SET);
	}



	return returnValue;
}





