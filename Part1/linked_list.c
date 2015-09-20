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
	char *value;
	int value_length;
	struct node *next;
} node;

typedef struct singlyLinkedList
{
	struct node *head;
	struct node *tail;
	int blockSizeInBytes;
} singlyLinkedList;

//Global Variables... I know... We don't have classes here so it's a bit tricky.
singlyLinkedList theList;

void Init (int M, int b)
{
	// Make the head node. FYI, malloc allocates a memory block of size m and returns a pointer to the start of the alloocated block!
	theList.head = (struct node*) malloc(M);
	theList.head->next = NULL;

	// Make the tail node
	theList.tail = theList.head;

	// Set the default byte size
	if (!b || b == 0)
	{
		// Set default to 128 bytes
		theList.blockSizeInBytes = 128;
	}
	else
	{
		// If given, set blockSize to b
		theList.blockSizeInBytes = b;
	}
}

void Destroy ()
{	// The destroy deletes theList from the head to tail
	struct node* current;
	while (theList.head->next != NULL)
	{
		current = theList.head;
		theList.head = theList.head->next;
		free(current);
	}

	// This deletes the last node of the list after theList.head == theList.tail
	free(theList.head);
}

void printNode(struct node* nodePtr){
	printf("\tThat node points to: %p\n", (void *)nodePtr);
	printf("\tKey: %d, Value Length:%d, Value:%s\n",
		nodePtr->key, nodePtr->value_length, nodePtr->value);
}

int Insert (int key,char *value_ptr, int value_len)
{
	struct node *newNode = (struct node*)malloc(theList.blockSizeInBytes);
	newNode->key = key;
	newNode->value_length = value_len;
	newNode->value = value_ptr;
	int *nextPrt = theList.tail;
	nextPrt += theList.blockSizeInBytes/ sizeof(int);
	newNode->next = nextPrt;

	if(theList.head == theList.tail)
	{
		memcpy(theList.head, newNode, theList.blockSizeInBytes);
		printf("The head points to: %p\n", (void *)theList.head);

		printNode(theList.head);
	}
	else
	{
		memcpy(theList.tail, newNode, theList.blockSizeInBytes);

		printf("The head points to: %p, The iterator points to: %p, The newNode points to: %p\n",
				(void *)theList.head, (void *)iterator, (void *)newNode);

		printf("\tkey:%d\n", key);
	}

	theList.tail += 128/sizeof(struct node);


	free(newNode);
}

int Delete (int key)
{

}

char* 	Lookup (int key){return NULL;}

void 	PrintList ()
{
	struct node *iterator = theList.head;

	//loop through all the nodes in a list except for the last
	//node because it's next property should be null
	//TODO: Add a is_list_empty check for this stuff
	while (iterator->next != NULL)
	{
		printf("\tKey: %d, Value: %s\n", iterator->key, iterator->value);
		iterator = iterator->next;// + iterator->value_length;
	}

	printf("\tKey: %d, Value: %s\n", iterator->key, iterator->value);
}
