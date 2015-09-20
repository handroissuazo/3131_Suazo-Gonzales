#include "linked_list.h"

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
	while (theList.head != theList.tail)
	{
		current = theList.head;
		theList.head = theList.head->next;
		free(current);
	}

	// This deletes the last node of the list after theList.head == theList.tail
	free(theList.head);
}

int Insert (int key,char *value_ptr, int value_len)
{
	struct node *newNode = (struct node*) malloc(value_len);
	newNode->key = key;
	newNode->value = value_ptr;
	newNode->value_length = value_len;
	newNode->next = NULL;

	struct node *iterator = theList.head;
	while(iterator->next != NULL)
	{
		iterator = iterator->next;
	}

	iterator->next = newNode;
	// newNode->next = theList.tail;

}

int 	Delete (int key){}

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
