//Created by Nick Stone
#pragma once
#include <iostream>
#include <cstddef>
#include <stdlib.h>
#include <atomic>
#include <cstdint>
#include <bitset>
#include <thread>
#include <mutex>
using namespace std;

#if 0

class BaseArena{
  // this would have common fields of arenas, such as size?
  int numbits;
  int chunksize;
  // ...
};

template<int NUMBITS, int CHUNKSIZE>
class Arena : public BaseArena {
  // this would have the bitmap, and the logic for atomically setting/clearing bits
  atomic<uint64_t> bits[NUMBITS/(8*sizeof(uint64_t))];
  char* chunks[NUMBITS*CHUNKSIZE];
};
#endif

//Creating A memory allocator that uses arena allocation methods
//This is done through a bit map of x bytes such that it is a multiple of 64 bits
//Each allocation doubles in size 

/// [nas] Rename from superultra to PersistentArenaManager
///
/// The PersistentArenaManager is a programmer-visible object that provides the ability to
/// carve out regions from a mmap'd file and use them as if they were the return value from
/// calls to malloc().  PersistentArenaManager also supports free().
///
/// The api consists of four functions:
/// - init(void* base, size_t offset): init() receives a previously mmap'd region of memory 
///   and its maximum size.  It finds the allocator within that region, initializes this 
///   PersistentArenaManager from that allocator, and prepares to serve future allocation
///   and free requests.
/// - destroy(): currently a no-op, but part of the API in case we ever decide that init() 
///   should be responsible for calling mmap... in that case, destroy() would call munmap()
/// - get_mem(size_t): rounds the given size up to the next size class, finds (or creates)
///   an arena that has a free block of that size, and reserves / returns that block
/// - return_mem(void*): Determines the arena from which the provided pointer was allocated,
///   and returns that pointer to the arena.
template <class T> class superultra {
public:
	//Using mutex locks in order to start with the easiest concurrency
	//End up using lock guard for spin lock 
	mutex mut;
	//Template variables Starting with the values that are supposed to be held by the class
	using value_type = T;
	using pointer = T *;
	using const_pointer = const T*;
	//These variables will be used for allocating the memory IE void * commonly used
	using size_type = size_t;
	using void_star = void*;
	//This struct is used to create a singly linked list that holds meta data for memory allocations
	//Each of these structs is used to form a simple linked list data structure
	//Each Arena is a given size of memory and points to the next arena in the list
	//Each arena also holds a size of the arena since this is dynamic as well as the location of the arena
	// [nas] switch to triple comments
	// [nas] it would help to have a picture of the structure here, and to increase locality if possible 
	//       by putting the bitset in the Arena.  That will require some ugly var-sized work.
	/// An Arena is a contiguous region of persistent memory, wholly contained within the bounds
	/// of the region with which the PersistentArenaManager was initialized.  It consists of a bitmap
	/// for tracking free blocks, an array of blocks, and other metadata.
	struct Arena {
		//Where the next arena is stored in the list 
		struct Arena* next;
		//The size of the arena... 64 128 256 ....
		// [nas] This is also actually the field that indicates the /type/ of the Arena.
		size_type Arenasize;
		//Location of where the arena starts this is used for allocating and sectioning off memory 
		// [nas] Switch to char* for easier pointer arithmetic?
		void_star startarena;
		//Not used yet but will be for accessing nodes... Concurrency
		atomic_flag lock;
		//This is a dynamic sized array that holds 0s and 1s
		// [nas] Consider fixed-size array of atomic uint64_ts?
		int* bitset;
		// [nas] Maybe all Arenas should be templated by size class, and inherit from a common base?
	};


private:
	//The number of allocations that occur in a given 
	// [nas] Probably not needed?!?
	int count = 0;
	//start location For the overall program
	// [nas] By our definition above, this is /base/
	void_star start;
	//Number of arenas in the list how many nodes are here and active
	// [nas] probably not needed
	int numarenas;
	//The size of the next block of memory ready to be allocated
	// [nas] Probably going away
	size_type chunk;
	//MALLOC
	//WHat is the total size we have allocated
	// [nas] Probably need two things here.  One is the /offset/ passed to init().  The other is
	//       the offset to reach the un-allocated portion of the region.
	size_type HeapSize;
	//Size of the region we are going to allocate next
	// [nas] probably goes away
	size_type nexts;
	//MALLOC
	//Where the start of the arena is goingto be
	// [nas] There are many options for how to do this, because there could be one arena list or 
	//       many, possibly segregated by size or organized in circular lists.
	Arena* Head_Arena;
	//Location of the next node that needs to be created
	// [nas] subsumed by the two offsets above
	Arena* Next_Arena;
	//For the self referential Bit map 

public:
	//THis is apart of the template easier to assign aa then arena * 
	using aa = Arena *;

	//These are a series of quick function calles mainly called getters or accessors
	//These were used to make sure the proper data was being collected and used
	int getnumallocs() { return count; }
	int getnumarenas() { return numarenas; }
	int getarenas() { return numarenas; }
	size_type getchunk() { return chunk; }
	void_star getstart() { return start; }
	Arena* gethead() { return Head_Arena; }
	//End QUick Functions

	//Malloc -> Moves the program counter 
	//The goal of the arena allocation is to actually allocate more memory as few as times as possible. 
	//Thus meaning that everytime we allocate we set our chunk size or the amount of memory that we are allocating
	//To be chunk * 2 or to appear in 64 bits then 128 bits then 256 bits then 512.... etc. 

	// [nas] should take a size_t
	void_star malloc() {

		//Take in no parameters
		//Create a new void star that points to the first position and adds the size of the arena to the void pointer
		//THen we increment the global counters and chunk for the next allocation to happen
		//Return the new pointer that is pointing to the new arena
		void_star retval = &start + chunk;
		//Add the chunk to the overall size of the heap 
		HeapSize += chunk;

		//Increase the number of times that we have malloced
		count = count + 1;
		//Set the chunk size to be double the last allocation
		chunk = chunk * 2;
		//Return the next location that we have allocated... Points to an arena!
		return retval;
	}

	//This functions goal is to be used to manipulate a void pointer to then be returned to the programmer 
	//The function will take in the size_t of the position and then it will take the size of the object that needs
	//to be allocated then it will take in the start of the arena that the object is being allocated too
	//Then a new void pointer will be created and returned that will be pointing to an object within the 
	//Range of bits in the given arena
	void_star lowlevelalloc(size_type posstart, size_type thisbig, void_star arenast) {
		//Make a new void * pointer that will point to the arena start + the positition of the next free spot in the bit map 
		// + the overall size that needs to be allocated. 
		void_star newplace = &arenast + posstart + thisbig;
		//Return new value 
		return newplace;

	}


/*
	This function takes in a given arena and the size of an object that needs to be allocated. Then We search the bit map
	to the corresponding arena that we take in and check to see if there is enough room to allocate the new object
	If yes we return else false
*/

	bool checkroom(Arena * temp, size_type big) {
		//Set a flag and a counter to be false and 0
		bool flag = false;
		size_type counter = 0;
		//A double for loop in order to iterate through the bit map 
		//We know the given bitmap size is Arena size
		for (int i = 0; i <= temp->Arenasize; i++) {

			//This inner loop is to check that there are enough positions in the list in order to allocate 
			for (int j = i; j <= temp-> Arenasize; j++) {

				//If the count is equal to the size of the thing being allocated we have enough room and we can exit the program 
				if (counter == big) {

					flag = true;
					return flag;
				}
				//New if chain we check the next spot in the array if its a 0 increment the counter
				if (temp->bitset[j] == 0)
				{
					//Increment counter by 1
					counter += 1;

				}
				else {
					//Reset the counter to 0
					counter = 0;
				}

			}

			counter = 0;

		}
		cout << "NO ROOM ";
		//Traverse whole list and found nothing... Return false NO room to allocate
		return flag;


	}

	////////////////////////////////////////////////////////////////////


	//Swap 0s to 1s
	//////////////////////////////////////////////////////////////////////

/*
This function takes in an arena and a size that needs to be allocated. Just like the function above this checks the arena bitmap to see 
if there is enough room to allocate the new object. If yes we track the position in the bit map that the object will be allocated in order
to offset the void star in the low level allocation and we set the bit map to be updated with 1s in the proper positions
Remember that 1 will occur in the sequential number of spots equal to the size of the object
*/
	size_type changeset(size_type needbig, Arena* e) {
		//// Set counter and position to 0... Counter is for the number of spots in the bit map 
		//The position is the overall position of the next free size
		size_type counter = 0;
		size_type positionchanging = 0;
		//Set a doube flag to both be false
		//Flag 1 ensures that we iterate through the entire bit map
		//Flag 2 ensures that there is enough room to allocate
		bool flag = false;
		bool flag2 = false;
		while (flag == false) {
			for (int i = 0; i <= e->Arenasize; i++) {
				positionchanging = i;
				for (int j = positionchanging; j <= e->Arenasize; j++) {
					if (counter == needbig) {
						//There is enoguh room quit the loops you are dones
						flag = true;
						flag2 = true;
						break;
					}
					else if (e->bitset[j] == 0)
					{
						//Increment counter by 1
						counter += 1;
					}
					else {
						//Reset the counter to 0
						counter = 0;
						break;
					}
				}
				//YOu have found the position quit the program
				if (flag2 == true) {
					break;
				}
				counter = 0;
			}
		}
		//Never found enough room 
		if (flag2 == false) {

			positionchanging = -1;

		}

		//Loop through and change the correct spots in the array to be 1s
		int space = positionchanging;
		for (int i = 0; i < needbig; i++) {

			e->bitset[space] = 1;
			space = space + 1;
		}
		//Return the position to be allocated
		return positionchanging;
	}

	//////////////////////////////////////////////////////////////////////

	//This hub function serves as the main function that controls the overall allocation process that is used by this program 
	//We first check to make sure that the arena has enough room to allocate memory If Not we return false otherwise 
	//We call low level alloc and we return the new void * back to the calling program 

	void_star hub(Arena* e, size_type needbig) {


		if (checkroom(e, needbig) == true) {
			size_type s = changeset(needbig, e);
			return lowlevelalloc(s, needbig, e->startarena);
		}
		else {

			return NULL; 
		}
	}
	//Bitallocate -> individually allocates based on bitmap..
	//Set the memory to false and keep this loop going 
	//We then iterate through the linked list if therre is no room or any arenas in the linked list
	//Then we allocate a new arena and make sure the arena is in the linked list
	//We then make sure that the arena has enough room to allocate the given memory that has been input by the programmer
	void_star bitallocate(size_type needbig) {
		while (1) {
			lock_guard <mutex> lock(mut);
			//if (mut.try_lock()) {
				bool memoryallocated = false;
				//We NEED to allocate memory, Continue until memory is allocated
				while (memoryallocated == false) {
					//No Arenas ... Make an arena
					if (Head_Arena == NULL) {
						allocate();
					}
					else {
						//Traverse all arenas and find a suitable place
						Arena* temp = Head_Arena;
						while (temp != NULL) {
							void_star store = hub(temp, needbig);
							if (store == NULL) {
								//Failed
								temp = temp->next;
							}
							else {
								return store;
								//Success!
							}
						}
						//Need another new arena
						if (memoryallocated == false) {
							allocate();
						}
					}
				}
			//}
		}
	}
	//Take in a struct and set ALL the values inside the struct
	Arena* arenainfo(Arena* temp) {
		//Arenasize  =chunk /2
		//startarena = temp
		temp->Arenasize = (chunk / 2);
		//cout << temp->Arenasize; 
		temp->startarena = temp;
		temp->bitset = new int[temp ->Arenasize];
		// traverse through array and set each element to 0
		for (int i = 0; i < temp->Arenasize; ++i) {
		
			temp->bitset[i] = 0;
		}
		//Return the updated Arena
		return temp;
	}


	//We use this function as the main function for creating and maintainning our linked list structure
	//This program is the ONLY program that calls malloc and that malloc belongs to an individual arena
	//Thus meaning that a large allocation should be maintained and kept up by the Arena Struct it belongs too
	void allocate() {
		
			//numallocations += 1; 
			numarenas = numarenas + 1;
			//0 Elements;
			if (Head_Arena == NULL) {
				Arena* e;
				e = reinterpret_cast<Arena*>(malloc());

				//Set all of Node values in e; 
				//SEt head Node to E; 
				e = arenainfo(e);
				//SEt Tail Not to HeadNode -> next
				Head_Arena = e;
				Head_Arena->next = NULL;
				start = Head_Arena;
				/*
				Call Functions to Establish Node
				*/
				//Head Node Now E
				Next_Arena = Head_Arena->next;
			}
			else if (Head_Arena->next == NULL) {
				//Temp Arena 
				
				Arena* te;
				//Allocate this new temp to be the malloc 
				te = reinterpret_cast<Arena*>(malloc());
				//te->startarena = te; 
				//Set Arena values
				te = arenainfo(te);
				//Place it in the list 
				Head_Arena->next = te;

			}
			else {
				//Arena* temp; 
				Arena* he;
				Next_Arena = Head_Arena;
				while (Next_Arena->next != NULL) {
					//Get to the last Node
					Next_Arena = Next_Arena->next;
				}
				//Once at end of list insert new node into list
				he = reinterpret_cast<Arena*>(malloc());
				//Set values of the new arena
				he = arenainfo(he);
				//Make sure it is in the list 
				Next_Arena->next = he;
			}
			//Function DOne
		}
	
	//First block is size 64 byte 
	//We start with no arenas and wait for the user to request memory before we alloate
	superultra()
	{
		numarenas = 0;
		HeapSize = 64;
		chunk = 64;
		Head_Arena = NULL;
		Next_Arena = NULL;
	}

	//Every destructor I write breaks the program/
	//Will wait for further instruction
	~superultra()
	{
	
		



	}


	// Frees a bit map in a given arenas
	void deallocate(void_star a) {
		free((Arena*) a);
	}


	//This function will iterate through the current linked list and will print the bit map in the arenas 
	//As well as some other information to ensure that the right information is being passed
	void print() {
		Arena* temp;
		temp = Head_Arena;
		int counter = 0;
		while (temp != NULL) {
			counter = counter + 1;
			cout << "Map Looks like this";
			cout << endl;
			// traverse through array and print each element
			cout << endl; 
			for (int i = temp->Arenasize-1; i >= 0; --i) {
				cout << temp->bitset[i];
				
			}
			cout << endl;
			temp = temp->next;
		}
	}

	//Set every value in the bitmap to 0
	//THis will allow the algorithm to revisit pointers
	//and reallocate all of the values
	void free(Arena* e) {
		// traverse through array and print each element
		for (int i = 0; i <= e->Arenasize; ++i) {
			e->bitset[i] =0;
		}
	}
	//Function designed to take in a superultra allocator and reconstruct the bit map 

	void recovery() {
		//Set a void star to equal the start of the original program
		//Then offset with 64 size and then check to see if a Arena exists.
		//If so THen we have found an old arena!
		//Else there was no arena in that location
		void_star test = Head_Arena -> startarena;
		size_type hit = 64; 
		void_star lets = &test + hit; 
		if (lets == NULL) {


			cout << "NO NODE";


		}
		//WE found a node
		else 
		{
			//Set the hit to be chunk size
			cout << "We have a Node hit";
			hit = hit * 2; 
			
			bool recover = true; 
			//Keep finding Nodes and offsetting the chunk value 
			while (recover == true) {
				lets = &lets + hit;
				cout << lets << "THIS VALUE"<< endl;
				cout << hit; 
				cout << endl; 
				cout << endl; 
				Arena* st = (Arena*)lets;
				if (st->bitset  == NULL) {
					//No Node
					recover = false; 

				}
				else {
					//NODE!
					hit = hit * 2; 
					cout << "NODE";
					//cout << lets; 


				}
			}
		}
	}

};
