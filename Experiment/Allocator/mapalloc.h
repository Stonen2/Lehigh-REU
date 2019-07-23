#pragma once
#include <iostream>
#include <stdlib.h>
#include <queue>
#include <sys/mman.h>


using namespace std;


class mapalloc {
	
private:
	
		void* ad; 
		size_t len; 
		int pro; 
		int flags; 
		int fd; 
		off_t offset;
		
		
		
		
		//void* base;
		//size_t regionsize;
		//size_t next;
		//int blocks;





		//Takes in all of the Parameters for a file to be Memory Mapped
		


	/*	mapalloc(void* aadr, size_t ilength, int iprot, int iflags, int ifd, off_t ioffset) {
			ad = aadr; 
			len = ilength;
			pro = iprot; 
			fd = ifd; 
			offset = ioffset; 

			ad = map();
			
			



			//regionsize = heap_size;
			//next = 0;
			//blocks = 0;
			//base = malloc(heap_size);



		}*/
		void* map() {
			void* temp;
			temp = mmap(nullptr,
				len,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1,
				0);
			return temp; 


		}
		void* maps(size_t f) {
			void* temp;
			temp = mmap(nullptr,
				f,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1,
				0);
			return temp;

		}

		void unmap() {

		}
		
		void free() {
			//No Op 
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





	





public:


	void* malloc(size_t s) {

		void* temp = maps(s);

		return temp;

	}

	mapalloc(size_t s) {
		len = s;
		ad = map();

	}


};

