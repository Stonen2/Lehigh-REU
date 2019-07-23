#pragma once
#include <list>
#include <vector>
#include <iostream>
using namespace std; 


class basicbm {


	/*
	
	Add template stuff
	
	
	
	*/


private: 


	//Each Node will hold a bit map that corresponds to the number of bytes in that list... 

	//It will also be able to have next Fit best fit etc run against it

	//

	struct Node {
		vector<bool> bitmap; 
		size_t size; 
		void* address;
		size_t next; 
		Node(size_t s, void * ad) {
			this->address = ad; 
			this->size = s; 
			for (int i = 0; i < s; i++) {
				bitmap[i] = false; 

			}
		}



	};
	//64 byte maps
	list <Node> sf;
	//128 byte maps
	list<Node> ote;
	//256 byte maps
	list<Node> tfs;
	//512 byte maps
	list<Node> ftw;
	//1024 byte maps 
	list <Node> ttf;



public: 


	basicbm() {
		
	}

	~basicbm() {
		//If each list exits traverse and delete...
		//Operation Seek and Destroy

		while (!sf.empty()) {
			sf.pop_front();
		}
		while (!ote.empty()) {
			ote.pop_front();
		}
		while (!tfs.empty()) {
			tfs.pop_front();
		}
		while (!ftw.empty()) {
			ftw.pop_front();
		}
		while (!ttf.empty()) {
			ttf.pop_front();
		}
	}


	//Find all the fits based on the list
	//Done 


	bool test64(size_t s) {
		if (s > 64) {
			return false; 

		}
		return true; 
	}


	bool test128(size_t s) {
		if (s > 128) {
			return false;

		}
		return true;
	}

	bool test256(size_t s) {
		if (s > 256) {
			return false;

		}
		return true;
	}
	bool test512(size_t s) {
		if (s > 512) {
			return false;

		}
		return true;
	}
	bool test1024(size_t s) {
		if (s > 1024) {
			return false;

		}
		return true;
	}

	void addtolist(size_t ss) {

		if (test64(ss) == true) {
			//Do something
			//Try insert into existing Bitmap
			//If you Cannot then Build new bit map node and insert 

		}
		else if(test128(ss) == true){
			//Do SOmething 
		}
		else if(test256(ss) == true){
			//DO something
		}
		else if(test512(ss) == true){
			//Do something
		}
		else if(test1024(ss)) {
			//Do something 
		}
		else {

		cout << "Bigger than 1024 cannot help ";
		}


		//Add the new node to the corresponding list

	}

	//Generate some Bit maps??
	void recover(size_t s) {



	}
	//Program to run over a list and rebuild the bitmap 

    //Returns the position in the bit array that is next to be allocated
	int nextfit(vector<bool> test) {
		int nextfits = 0; 
		int temp = nextfits;
		if (nextfits == false) {
			for (int i = nextfits; i < test.size(); i++) {
				if (test[i] == false) {
					nextfits = i;
					break;
				}

			}

			return temp;

		}
		else {
			for (int i = nextfits; i < test.size(); i++) {
				if (test[i] == false) {
					return i;


				}



			}
		}

	}
	

	int firstfit(vector<bool> test) {
		for (int i = 0; i < test.size(); i++) {

			if (test[i] == false) {
				return i;

			}

		}

	}
	
	//Return the range of positions that are needed and free to allocate the given size too
	int findpos(int num,vector<bool> test)
	{
		int start = 0;
		int temp = 0;
		for (int i = 0; i < test.size(); i++) {
			temp = 0;
			if (test[i] == false) {
				start = i;

				for (int j = i; j < test.size(); j++) {
					if (temp == num) {
						return start;
						//break; 
					}
					else if (test[j] == false) {
						temp = temp + 1;
					}
					else {

						break;
					}

				}
				continue;



			}


		}

		cout << "not enough room to allocate";

	}

	
	//Flip The bit at a given position in the array
	vector<bool> flipbits(int pos,vector<bool> te) {
		vector<bool> test = te; 

		bool t = test.at(pos);
		if (t == false) {

			t = true;
			test[pos] = t;

		}
		else {
			t = false;
			test[pos] = t;

		}

	}

	


};

