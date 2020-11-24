/*
 * CircularBuffer.h
 *
 *  Created on: 6 nov. 2020
 *      Author: santi
 */

#ifndef __CIRCULARBUFFER_H_
#define __CIRCULARBUFFER_H_

/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Struct containing circular buffer internal variables.
 */
typedef struct
{
	void * const buffer;     /** Pointer to statically reserved memory array. */
	void * const buffer_end; /** Pointer to end of the array. */
	void * head;	         /** Pointer to the head of the buffer. */
	void * tail;	         /** Pointer to the tail of the buffer. */
	int capacity;            /** Maximum number of elements in the buffer. */
	int count;               /** Number of elements in the buffer. */
	int size;                /** Size of each element in the buffer. */
} CircularBuffer_t;

/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Macro to construct a circular buffer.
 *
 * It allocates static memory and creates a structure with the given name. This
 * name is meant to be passed as the "this" pointer to the buffer functions. *
 */
//TODO Sacar esto
// #define NEW_CIRCULAR_BUFFER(name,_capacity,_size)      	\
//         static char name##_arr[_capacity*_size];       	\
//         static CircularBuffer name =             		\
//         {                                				\
//             .buffer = name##_arr,     				    \
// 			.buffer_end = name##_arr + _capacity*_size,	\
//             .head = name##_arr,      				    \
//             .tail = name##_arr,       				    \
//             .capacity = _capacity,        				\
// 			.count = 0,                  				\
// 			.size = _size                 				\
//         }

/////////////////////////////////////////////////////////////////////////////////
//                         Global function prototypes                          //
/////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create a new buffer from an array of elements.
 * @param bufferArray Pointer to he array to convert to buffer
 * @param capacity Number of elements in the
 * @param elementSize size of every element.
 * @return CircularBuffer structure with the info needed to do stuff
 */
CircularBuffer_t newCircularBuffer(void* bufferArray, size_t capacity, size_t elementSize);

/**
 * @brief Push an element to the buffer.
 * @param this Pointer to the buffer structure.
 * @param data Pointer to the data to be pushed.
 * @return False if the buffer is full, true instead.
 */
bool push(CircularBuffer_t *this,void * data);

/**
 * @brief Pop an element from the buffer.
 * @param this Pointer to the buffer structure.
 * @param data Pointer to a variable to store the data.
 * @return False if the buffer is empty, true instead.
 */
bool pop(CircularBuffer_t *this,void * data);


/**
 * @brief Delete all elements from the buffer.
 * @param this Pointer to the buffer structure.
 */
void flush(CircularBuffer_t *this);

/**
 * @brief Number of elements currently stored in the buffer.
 * @param this Pointer to the buffer structure.
 * @return The number of elements.
 */
int numel(CircularBuffer_t *this);


/**
 * @brief Number of elements until the buffer gets full;
 * @param this Pointer to the buffer structure.
 * @return The number of elements left.
 */
int numberOfElementsLeft(CircularBuffer_t *this);

/**
 * @brief Check if the buffer is empty
 * @return True if the buffer is empty
 */
bool isEmpty(CircularBuffer_t *this);

/**
 * @brief Check if the buffer is full
 * @return True if the buffer is full
 */
bool isFull(CircularBuffer_t *this);

#endif // __CIRCULARBUFFER_H_
