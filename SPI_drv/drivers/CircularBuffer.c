#include "CircularBuffer.h"
#include <string.h>

CircularBuffer_t newCircularBuffer(void* bufferArray, size_t arraySize, size_t elementSize)
{
	CircularBuffer_t buffer =             		
        {                                				
           	.buffer = bufferArray,     				    
			.buffer_end = (char*)bufferArray + arraySize * elementSize,	
            .head = bufferArray,      				  
            .tail = bufferArray,       				    
            .capacity = arraySize,        				
			.count = 0,                  				
			.size = elementSize                 				
        };
	return buffer;
}

bool push(CircularBuffer_t *this,void* data)
{
	if(this->count == this->capacity)
		return false;
	else
	{
		memcpy(this->head, data, this->size);
		this->head = (char*)(this->head) + this->size;
	    if(this->head == this->buffer_end)
	    	this->head = this->buffer;
	    this->count++;
	    return true;
	}
}

bool pop(CircularBuffer_t *this,void * data)
{
	if(this->count == 0)
		return false;
	else
	{
		memcpy(data, this->tail, this->size);
		this->tail = (char*)(this->tail) + this->size;
		if(this->tail == this->buffer_end)
			this->tail = this->buffer;
		this->count--;
		return true;
	}
}

void flush(CircularBuffer_t * this)
{
	this->head = this->tail;
	this->count = 0;
}

int numel(CircularBuffer_t *this)
{
	return this->count;
}

int numberOfElementsLeft(CircularBuffer_t *this)
{
	return this->capacity - this->count;
}

bool isEmpty(CircularBuffer_t *this)
{
	return (this->count == 0);
}

bool isFull(CircularBuffer_t *this)
{
	return (this->count == this->capacity);
}
