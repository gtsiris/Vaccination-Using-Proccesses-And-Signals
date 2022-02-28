#include "struct_Bloom_Filter.h"

using namespace std;

bool bloomFilter::BitIsSet(const unsigned int& pos, const unsigned int& index) const {
	if ((bitArray[pos] >> index) & 1)  /* Shift it by index so it is either 0 or 1 then use mask to determine */
		return TRUE;
	return FALSE;
}

void bloomFilter::SetBit(const unsigned int& pos, const unsigned int& index) {
	bitArray[pos] |= (1 << index);  /* Shit 1 by index and then use OR. The bit will be set */
}

/*
This algorithm (k=33) was first reported by dan bernstein many years 
ago in comp.lang.c. 
The magic of number 33 (why it works better than many other constants, 
prime or not) has never been adequately explained.
*/
unsigned long bloomFilter::djb2(unsigned char *str) const {
	unsigned long hash = 5381;
	int c; 
	while (c = *str++) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;
}


/*
This algorithm was created for sdbm (a public-domain reimplementation of ndbm) 
database library. it was found to do well in scrambling bits, causing better 
distribution of the keys and fewer splits. it also happens to be a good 
general hashing function with good distribution. The actual function 
is hash(i) = hash(i - 1) * 65599 + str[i]; what is included below 
is the faster version used in gawk. There is even a faster, duff-device 
version. The magic constant 65599 was picked out of thin air while experimenting 
with different constants, and turns out to be a prime. this is one of the 
algorithms used in berkeley db (see sleepycat) and elsewhere.
*/
unsigned long bloomFilter::sdbm(unsigned char *str) const {
	unsigned long hash = 0;
	int c;

	while (c = *str++) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}


/* 
Return the result of the Kth hash funcation. This function uses djb2 and sdbm.
None of the functions used here is strong for cryptography purposes but they
are good enough for the purpose of the class.

The approach in this function is based on the paper:
https://www.eecs.harvard.edu/~michaelm/postscripts/rsa2008.pdf
*/
unsigned long bloomFilter::hash_i(unsigned char *str, unsigned int i) const {
	return djb2(str) + i*sdbm(str) + i*i;
}

bloomFilter::bloomFilter(const unsigned int& size) : size(size) {
	bitArray = new char[size];
	memset(bitArray, 0, size);
}

bloomFilter::bloomFilter(const bloomFilter& bloom) : size(bloom.size) {
	bitArray = new char[size];
	memcpy(bitArray, bloom.bitArray, size);
}

bloomFilter::~bloomFilter() {
	delete[] bitArray;
}

unsigned int bloomFilter::GetSize() const {
	return size;
}

const char *bloomFilter::GetBitArray() const {
	return bitArray;
}

void bloomFilter::SetBitArray(const char* bitArray) {
	memcpy(this->bitArray, bitArray, size);
}

bool bloomFilter::Search(const string& key) const {
	unsigned char *buffer = new unsigned char[key.length() + 1];  /* Allocate one more char for the null-terminated string */
	strcpy((char *)buffer, key.c_str());  /* Copy key to buffer (cannot be used as is, because of const specifier) */
	for (unsigned int i = 0; i < K; i++) {
		unsigned int pos = (hash_i(buffer, i) % (size * 8)) / (sizeof(char) * 8);  /* Position of the desired integer */
		unsigned int index = (hash_i(buffer, i) % (size * 8)) % (sizeof(char) * 8);  /* Index inside the desired integer */
		if (!BitIsSet(pos, index)) {
			delete[] buffer;
			return FALSE;
		}
	}
	delete[] buffer;  /* Free allocated memory */
	return TRUE;
}

void bloomFilter::Insert(const string& key) {
	unsigned char *buffer = new unsigned char[key.length() + 1];
	strcpy((char *)buffer, key.c_str());
	for (unsigned int i = 0; i < K; i++) {
		unsigned int pos = (hash_i(buffer, i) % (size * 8)) / (sizeof(char) * 8);
		unsigned int index = (hash_i(buffer, i) % (size * 8)) % (sizeof(char) * 8);
		SetBit(pos, index);
	}
	delete[] buffer;  /* Free allocated memory */
}

void bloomFilter::Update(const bloomFilter& bloom) {
	if (size != bloom.size) {
		cout << "Bloom sizes must match\n";
		return;
	}
	for (unsigned int i = 0; i < size; i++)
		bitArray[i] |= bloom.bitArray[i];  /* Bitwise OR doesn't violate the property of bloom to give definitive negative answers */
}
