#include <stdio.h>
#include <stdbool.h>
#include <math.h>
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
	int memAllocInBytes;
} singlyLinkedList;

//Global Variables... I know... We don't have classes here so it's a bit tricky.
singlyLinkedList theList;

void Init (int M, int b)
{
	theList.memAllocInBytes = M;
	// Set the default byte size
		if (!b || b < 36)
		{
			// Set default to 128 bytes
			theList.blockSizeInBytes = 128;
			printf("We changed your block size to %d because \n you need more than 32 bytes for each node.\n", theList.blockSizeInBytes);
		}
		else
		{
			// If given, set blockSize to b
			theList.blockSizeInBytes = b;
		}

	// Set the default byte size
		if (!M || M < 512000)
		{
			// Hey we need to have enough space for 11 nodes.
			theList.memAllocInBytes = 512000;
			printf("We changed your allocation size to %d because \n we deem you need more room to fit items.\n", theList.memAllocInBytes);
		}

	// Make the head node. FYI, malloc allocates a memory block of size m and returns a pointer to the start of the allocated block!
	theList.head = (struct node*) malloc(theList.memAllocInBytes);
	theList.head->next = NULL;

	// Make the tail node
	theList.tail = theList.head;


}

void Destroy ()
{	// The destroy deletes theList from the head to tail
	struct node* current = theList.head;
	struct node* next;
	while (current != NULL)
	{
		next = current->next;
		current = NULL;
		current = next;
	}

	// This deletes the last node of the list after theList.head == theList.tail
	  theList.head = NULL;
}

void printNode(struct node* nodePtr){
	printf("\tThat node points to: %p\n", (void *)nodePtr);
	printf("\tKey: %d, Value Length:%d, Value:%s\n",
		nodePtr->key, nodePtr->value_length, nodePtr->value);
}

int Insert (int key,char *value_ptr, int value_len)
{
	struct node newNode;
	newNode.key = key;
	newNode.value_length = value_len;
	newNode.value = value_ptr;
	newNode.next = theList.tail + theList.blockSizeInBytes/ sizeof(struct node);

	struct node *endOfPool = theList.head + (theList.memAllocInBytes/sizeof(struct node));
	if(newNode.next >= endOfPool)
	{
		printf("You did not allocate enough memory to insert more items. \nRun the program again with a larger memsize.\n");
		return 1;
	}

	if(theList.head == theList.tail)
	{
		memcpy(theList.head, &newNode, theList.blockSizeInBytes);
	}
	else
	{
		memcpy(theList.tail, &newNode, theList.blockSizeInBytes);
	}

	theList.tail += theList.blockSizeInBytes/sizeof(struct node);
	theList.tail->key = 0;
	theList.tail->next = NULL;
	theList.tail->value = NULL;
	theList.tail->value_length = 0;
}

int Delete (int key)
{
	struct node* current;
	struct node* previous;
	bool isNodeFound = false;
	bool isNodeDeleted = false;
	current = theList.head;

	while (current != theList.tail)
	{
		if (key == current->key)
		{
			isNodeFound = true;
			break;
		}
		previous = current;
		current = current->next;
	}

	if(isNodeFound)
	{
		previous->next = current->next;
		InsertIntoEmptySpaceQueue(theList.head - current, theList.blockSizeInBytes);
		isNodeDeleted = true;
	}
	else if (!isNodeDeleted)
	{
		printf("Node of key %d not found. Try again with a key that actually exists!\n", key);
	}

	return isNodeDeleted;
}

char* 	Lookup (int key)
{
	struct node* current;
	current = theList.head;
	bool isNodeFound = false;

	while (current != theList.tail)
	{
		if (key == current->key)
		{
			isNodeFound = true;
			break;
		}

		current = current->next;
	}

	char* value = NULL;
	if(isNodeFound)
	{
		char keyString[4] = "0000";
		char keyPart[4];
		int digits = (current->key == 0 ? 1 : (int)(log10(current->key)+1));
		int count = 0;
		sprintf(keyPart, "%d", current->key);
		for (int i = 3; i > digits - 1; ++i)
		{
			keyString[i] = keyPart[digits - count];
			++count;
		}

		strcat(keyString, keyPart);

		char info[256];
		char value_length[4];

		sprintf(value_length, "%d", current->value_length);

		strcpy(info, key);
		strcat(info, value_length);
		strcat(info, current->value);
		value = info;
	}

	return value;
}

void 	PrintList ()
{
	struct node *iterator = theList.head;

	//loop through all the nodes in a list except for the last
	//node because it's next property should be null
	//TODO: Add a is_list_empty check for this stuff
	while (iterator->next != theList.tail)
	{
		printf("\tKey: %d, Value: %s\n", iterator->key, iterator->value);
		iterator = iterator->next;// + iterator->value_length;
	}

	printf("\tKey: %d, Value: %s\n", iterator->key, iterator->value);
	printf("-----------------------\n");
}
