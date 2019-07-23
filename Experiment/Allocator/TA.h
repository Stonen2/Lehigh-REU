#pragma once
#include <stdlib.h>
#include <iostream>


using namespace std; 







class TA {

public: 
		size_t HeapSize;
		size_t next; 
		void* start;

		TA(size_t ih) {
			HeapSize = ih; 

			start = malloc(ih);

			next = 0; 


		}

		void* malloc(size_t s) {

				size_t pad_size = PAD(s);        // TODO: pad the size appropriately
				void* retval = &start + next;
				next += pad_size;
				HeapSize += pad_size; 
				return retval;
			

		}

		void free(void* n) {

			//No- OP 

		}


		size_t PAD(size_t s) {
			size_t temp = s;
			if (s % 8 == 0) {
				return temp;

			}
			else {
				temp = s / 8;
				size_t offset = 8 - temp;
				temp = s + offset;
				return temp;

			}

			//return temp; 
		}












};
