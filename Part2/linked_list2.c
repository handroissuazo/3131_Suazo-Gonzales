#include "linked_list2.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include "linked_list2.h"

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

// -----------------------------------------------------
// Linked List -----------------------------------------
typedef struct node
{	//This will hold our key, value, next node, and value_length (to know how much space to allocate and deallocate for some dynamic memory allocation! Yessss... )
	struct node *next;
	int key;
	int value_length;
	char *value;
} node;

typedef struct singlyLinkedList
{
	struct node *head;
	struct node *tail;
	int blockSizeInBytes;
	int memAllocInBytes;
} singlyLinkedList;

void InitSingleList (int M, int b, singlyLinkedList* theList)
{

}

void SingleListDestroy (singlyLinkedList theList)
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

void SingleListprintNode(struct node* nodePtr){
	printf("\tThat node points to: %p\n", (void *)nodePtr);
	printf("\tKey: %d, Value Length:%d, Value:%s\n",
		nodePtr->key, nodePtr->value_length, nodePtr->value);
}

int SingleListInsert (int key, char *value_ptr, int value_len, singlyLinkedList* theList)
{
	if(key < 0)
	{
		printf("\nInvalid key: %d\n", key);
		return 1;
	}

	struct node newNode;
	newNode.key = key;
	newNode.value_length = value_len;
	newNode.value = value_ptr;
	printf("\n%p--%d\n", theList->tail, theList->blockSizeInBytes);
	newNode.next = NULL;

	struct node *endOfPool = theList->head + (theList->memAllocInBytes/sizeof(struct node));
	if(newNode.next >= endOfPool)
	{
		printf("You did not allocate enough memory to insert more items. \nRun the program again with a larger memsize.\n");
		return 1;
	}

	struct node *savedNode;
	if(theList->head == theList->tail)
	{
		memcpy(theList->head, &newNode, theList->blockSizeInBytes);
		savedNode = theList->head;
	}
	else
	{
		memcpy(theList->tail, &newNode, theList->blockSizeInBytes);
		savedNode = theList->tail;
	}

	theList->tail += theList->blockSizeInBytes/sizeof(struct node);
	theList->tail->key = 0;
	theList->tail->next = NULL;
	theList->tail->value = NULL;
	theList->tail->value_length = 0;
	savedNode->next = theList->tail;
}

int SingleListDelete (int key, singlyLinkedList theList)
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

char* 	SingleListLookup (int key, singlyLinkedList theList)
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
		char* kv = &(current->key);
		printf ("Key = %d, Value Len = %d, Value = %s\n", *(int *) kv, *(int *) (kv+4), current->value);
		return kv;
	}
	return value;
}

void 	PrintSingleList (singlyLinkedList theList)
{
	struct node *iterator = theList.head;

	//loop through all the nodes in a list except for the last
	//node because it's next property should be null
	//TODO: Add a is_list_empty check for this stuff
	while (iterator->next != theList.tail)
	{
		printf("\tKey: %d, Value: %d\n", iterator->key, iterator->value_length);
		iterator = iterator->next;// + iterator->value_length;
	}

	printf("\tKey: %d, Value Length: %d\n", iterator->key, iterator->value_length);
	printf("-----------------------\n");
}

// ---------------------------------------------------
// Tiered List ---------------------------------------
typedef struct TieredList
{
	singlyLinkedList* tierListIterator;
	int numberOfTiers;
	int Range;
	singlyLinkedList* ListPtr[100000];
	singlyLinkedList* Root;

} TieredList;

// Globals
TieredList MasterList;

void Init (int M, int b, int t) // initializes the linked list, should be called once from the main
{
	if (t)
	{
		MasterList.numberOfTiers = t;
		MasterList.Range = INT_MAX / t;
	}

	MasterList.Root = (struct singlyLinkedList*) malloc(sizeof(singlyLinkedList)*MasterList.numberOfTiers + 1);
	MasterList.tierListIterator = MasterList.Root;

	for (int i = 0; i < MasterList.numberOfTiers; ++i)
	{
			MasterList.tierListIterator += i * sizeof(singlyLinkedList);
			MasterList.tierListIterator->memAllocInBytes = M;
			// Set the default byte size
				if (!b || b < 36)
				{
					// Set default to 128 bytes
					MasterList.tierListIterator->blockSizeInBytes = 128;
					printf("We changed your block size to %d because \n you need more than 32 bytes for each node.\n", MasterList.tierListIterator->blockSizeInBytes);
				}
				else
				{
					// If given, set blockSize to b
					MasterList.tierListIterator->blockSizeInBytes = b;
				}

			// Set the default byte size
				if (!M || M < 512000)
				{
					// Hey we need to have enough space for 11 nodes.
					MasterList.tierListIterator->memAllocInBytes = 512000;
					printf("We changed your allocation size to %d because \n we deem you need more room to fit items.\n", MasterList.tierListIterator->memAllocInBytes);
				}

			// Make the head node. FYI, malloc allocates a memory block of size m and returns a pointer to the start of the allocated block!
			MasterList.tierListIterator->head = (struct node*) malloc(MasterList.tierListIterator->memAllocInBytes);
			MasterList.tierListIterator->head->next = NULL;

			// Make the tail node
			MasterList.tierListIterator->tail = MasterList.tierListIterator->head;
			MasterList.ListPtr[i] = MasterList.tierListIterator;
	}
}

void Destroy () // destroys the linked list and cleans resources
{

}

int Insert (int key,char * value_ptr, int value_len) // inserts the key and copies the value to the payload
{
	int listPosition = 0;
	for (int i = 1; i < MasterList.numberOfTiers; ++i)
	{
		if (key < MasterList.Range * i + 1)
		{
			break;
		}
		else ++listPosition;
	}

	SingleListInsert(key, value_ptr, value_len, MasterList.ListPtr[listPosition]);


}

int Delete (int key) // delete the whole block containing that particular key. When multiple entries with the same key, delete only the first one
{
	int listPosition = 0;
	for (int i = 1; i < MasterList.numberOfTiers; ++i)
	{
		if (key < MasterList.Range * i + 1)
		{
			break;
		}
		else ++listPosition;
	}
}

char* Lookup (int key) // Looks up the first item with the given and returns a pointer to the value portion (the value length and the actual value)
{
	int listPosition = 0;
	for (int i = 1; i < MasterList.numberOfTiers; ++i)
	{
		if (key < MasterList.Range * i + 1)
		{
			break;
		}
		else ++listPosition;
	}
}
			    //(the user can read or modify after obtaining the pointer)

void PrintList () // prints the entire list by following the next pointers. Print the keys and the length of the value
{
	int listPosition = 0;
	for (int i = 1; i < MasterList.numberOfTiers; ++i)
	{
		if (key < MasterList.Range * i + 1)
		{
			break;
		}
		else ++listPosition;

		PrintSingleList(*(MasterList.ListPtr[listPosition]));
	}
}
