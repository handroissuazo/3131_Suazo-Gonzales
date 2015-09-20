#include <stdio.h>
#include <stdbool.h>
#include "linked_list.h"

//Empty Space Priority Queue Used for memory management
typedef struct EmptySpaceObject
{
	int BytesFromHeader;
	int sizeOfSpace;
} EmptySpaceObject;

typedef struct EmptySpaceQueue
{
	EmptySpaceObject *EmptySpaceList;
	int allocatedSpace;
	int usedSpace;
	int QueueSize;
} EmptySpaceQueue;

EmptySpaceQueue s_emptySpaceq;

int InsertIntoEmptySpaceQueue(int BytesFromHeader, int sizeOfSpace)
{
	// We keep track if we added a new spaceObject,  make a new spaceObject list to contain the new spacelist and order, and the newQueue Position for proper insertion.
	bool isNewSpaceInserted = false;
	int newQueuePos = 0;
	int newUsedSpace = 0;
	int newAllocatedSpace = 0;
	EmptySpaceObject *newQueue;

	// Check if empty space queue is full (This works because the empty list object will always be a set number of bytes)
	if (s_emptySpaceq.allocatedSpace <= s_emptySpaceq.usedSpace)
	{
		newAllocatedSpace = s_emptySpaceq.allocatedSpace * 2;
		newQueue = (EmptySpaceObject*)malloc(newAllocatedSpace);
	}
	else
	{
		newAllocatedSpace = s_emptySpaceq.allocatedSpace;
		newQueue = (EmptySpaceObject*)malloc(s_emptySpaceq.allocatedSpace);
	}


	//Priority Queue Logic.
	for (int i = 0; i < s_emptySpaceq.QueueSize; ++i)
	{
		EmptySpaceObject currentSpaceObject = *(s_emptySpaceq.EmptySpaceList + i);

		// If we find that the BytesFromHeader is less than one on the current queue we insert the new spaceObject into the newQueue
		if (BytesFromHeader < currentSpaceObject.BytesFromHeader && !isNewSpaceInserted){
			EmptySpaceObject newSpaceObject;
			newSpaceObject.BytesFromHeader = BytesFromHeader;
			newSpaceObject.sizeOfSpace = sizeOfSpace;

			*(newQueue + newQueuePos) = newSpaceObject;
			++newQueuePos;	//We keep track of the new insertion place for further insertions
			newUsedSpace += sizeof(EmptySpaceObject*);	//We keep track of the used space
			isNewSpaceInserted = true;
		}

		// Add the current space object to the new list.
		*(newQueue + newQueuePos) = currentSpaceObject;
		++newQueuePos;		//We keep track of the new insertion place for further insertions
		newUsedSpace += sizeof(EmptySpaceObject);	//We keep track of the used space
	}

	free(s_emptySpaceq.EmptySpaceList);

	if (!isNewSpaceInserted)
	{
		//Insert a newSpace Obeject into the queue
		EmptySpaceObject newSpaceObject;
		newSpaceObject.BytesFromHeader = BytesFromHeader;
		newSpaceObject.sizeOfSpace = sizeOfSpace;

		*(newQueue + newQueuePos) = newSpaceObject;
		++newQueuePos;
		newUsedSpace += sizeof(EmptySpaceObject);	//We keep track of the used space
	}

	s_emptySpaceq.EmptySpaceList = newQueue;
	s_emptySpaceq.QueueSize = newQueuePos;	//Queue Size needs to be 1 plus newQueuePos because it's zero indexed
	s_emptySpaceq.usedSpace = newUsedSpace;
	s_emptySpaceq.allocatedSpace = newAllocatedSpace;
}

int GetFirstAvailableSpace(int SizeOfSpaceRequired)
{
	bool isSpaceFound = false;
	int bytesFromHeader = -1;
	//This will remove the space returned as it assumes you will then use the space afterwards.
	for (int i = 0; i + 1 < s_emptySpaceq.QueueSize; ++i)
	{
		EmptySpaceObject currentSpaceObject = *(s_emptySpaceq.EmptySpaceList + i);

		// If we find that the BytesFromHeader is less than one on the current queue we insert the new spaceObject into the newQueue
		if (SizeOfSpaceRequired <= currentSpaceObject.sizeOfSpace && !isSpaceFound)
		{
			isSpaceFound = true;
			bytesFromHeader = currentSpaceObject.BytesFromHeader;
			s_emptySpaceq.usedSpace -= sizeof(EmptySpaceObject*);	//We keep track of the used space
			s_emptySpaceq.QueueSize--;
		}

		//Shift Over the remaning Entries
		if (isSpaceFound)
		{
			*(s_emptySpaceq.EmptySpaceList + i) = *(s_emptySpaceq.EmptySpaceList + i + 1);
		}
	}

	//In the future, this will need to account for remaining space (Since all of the items will be a fixed block size, this isn't necessary at this stage)

	return bytesFromHeader; //If no available space is found, the function returns -1. 
}

void DestroyEmptySpaceList() 
{
	free(s_emptySpaceq.EmptySpaceList);
}

void PrintEmptytSpaceList()
{
	for (int i = 0; i < s_emptySpaceq.QueueSize; ++i)
	{
		printf("Empty Space: %d, Bytes From Header: %d, Size Of Space %d\n", i, (s_emptySpaceq.EmptySpaceList + i)->BytesFromHeader, (s_emptySpaceq.EmptySpaceList + i)->sizeOfSpace);
	}

	printf("\n");
}

// Linked List 
typedef struct node
{	//This will hold our key, value, next node, and value_length (to know how much space to allocate and deallocate for some dynamic memory allocation! Yessss... )
	int key;
	char* value;
	int value_length;
	struct node *next;
} node;

typedef struct singlyLinkedList
{
	node *head;
	node *tail;
	int blockSizeInBytes;
} singlyLinkedList;

//Global Variables... I know... We don't have classes here so it's a bit tricky. 
singlyLinkedList s_theList;


void Init (int M, int b)
{
	//Setup the EmptySpaceQueue for custom DMA
	s_emptySpaceq.EmptySpaceList = (EmptySpaceObject*) malloc(M/b * sizeof(EmptySpaceObject));
	s_emptySpaceq.allocatedSpace = M;
	s_emptySpaceq.QueueSize = 0;
	s_emptySpaceq.usedSpace = 0;

	// Make the head node. FYI, malloc allocates a memory block of size m and returns a pointer to the start of the alloocated block!
	s_theList.head = (node *) malloc(M);
	s_theList.head->next = 0;

	// Make the tail node
	s_theList.tail = s_theList.head;

	// Set the default byte size
	if (b == NULL || b == 0)
	{
		// Set default to 128 bytes
		s_theList.blockSizeInBytes = 128;
	}
	else
	{	
		// If given, set blockSize to b
		s_theList.blockSizeInBytes = b;
	}

	for (int i = 0; i < 10000; ++i)
	{
		InsertIntoEmptySpaceQueue(i*128, 128);
	}

	PrintEmptytSpaceList();

	for (int i = 0; i < 9999; ++i)
	{
		GetFirstAvailableSpace(128);
	}

	PrintEmptytSpaceList();
}

void Destroy ()
{	// The destroy deletes s_theList from the head to tail
	node* current;
	while (s_theList.head != s_theList.tail)
	{	
		current = s_theList.head;
		s_theList.head = s_theList.head->next;
		free(current);
	}

	// This deletes the last node of the list after s_theList.head == s_theList.tail
	free(s_theList.head);

	DestroyEmptySpaceList();
}

int Insert (int key,char * value_ptr, int value_len)
{
		
}

int 	Delete (int key){}
char* 	Lookup (int key){return NULL;}
void 	PrintList (){}