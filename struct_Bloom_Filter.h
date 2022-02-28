#ifndef struct_Bloom_Filter_h
#define struct_Bloom_Filter_h

#include <iostream>
#include <cstdio>
#include <cstring>

#define K 16  /* Number of hash functions utilized */
#define TRUE 1
#define FALSE 0

struct bloomFilter {
	private:
		char *bitArray;
		unsigned int size;
		
		bool BitIsSet(const unsigned int& pos, const unsigned int& index) const;
		
		void SetBit(const unsigned int& pos, const unsigned int& index);
		
		unsigned long djb2(unsigned char *str) const;
		
		unsigned long sdbm(unsigned char *str) const;
		
		unsigned long hash_i(unsigned char *str, unsigned int i) const;
		
	public:
		bloomFilter(const unsigned int& size);
		
		bloomFilter(const bloomFilter& bloom);
		
		~bloomFilter();
		
		unsigned int GetSize() const;
		
		const char *GetBitArray() const;
		
		void SetBitArray(const char* bitArray);
		
		bool Search(const std::string& key) const;
		
		void Insert(const std::string& key);
		
		void Update(const bloomFilter& bloom);
};

#endif
