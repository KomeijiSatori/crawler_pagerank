#include <bitset>

#include "bloom.h"

bitset<capacity> bit1;
unsigned int (*hash_funcs[hash_func_num])(string) = {RSHash, JSHash, PJWHash, ELFHash, BKDRHash, SDBMHash, DJBHash, DEKHash, BPHash};

void init_bloom(void)
{
	bit1.reset();
}

unsigned int RSHash(string str)//1
{
   unsigned int b = 378551;
   unsigned int a = 63689;
   unsigned int hash = 0;

   for (int i = 0; i < str.length(); i++)
   {
      hash = hash * a + str[i];
      a = a * b;
   }

   return hash;
}

unsigned int JSHash(string str)//2
{
   unsigned int hash = 1315423911;

   for (int i = 0; i < str.length(); i++)
   {
      hash ^= ((hash << 5) + str[i] + (hash >> 2));
   }

   return hash;
}

unsigned int PJWHash(string str)//3
{
   unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
   unsigned int ThreeQuarters = (unsigned int)((BitsInUnsignedInt  * 3) / 4);
   unsigned int OneEighth = (unsigned int)(BitsInUnsignedInt / 8);
   unsigned int HighBits = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
   unsigned int hash = 0;
   unsigned int test = 0;

   for (int i = 0; i < str.length(); i++)
   {
      hash = (hash << OneEighth) + str[i];

      if ((test = hash & HighBits) != 0)
      {
         hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
      }
   }

   return hash;
}

unsigned int ELFHash(string str)//4
{
   unsigned int hash = 0;
   unsigned int x = 0;

   for (int i = 0; i < str.length(); i++)
   {
      hash = (hash << 4) + str[i];
      if ((x = hash & 0xF0000000L) != 0)
      {
         hash ^= (x >> 24);
      }
      hash &= ~x;
   }

   return hash;
}

unsigned int BKDRHash(string str)//5
{
   unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
   unsigned int hash = 0;

   for (int i = 0; i < str.length(); i++)
   {
      hash = (hash * seed) + str[i];
   }

   return hash;
}

unsigned int SDBMHash(string str)//6
{
   unsigned int hash = 0;

   for (int i = 0; i < str.length(); i++)
   {
      hash = str[i] + (hash << 6) + (hash << 16) - hash;
   }

   return hash;
}

unsigned int DJBHash(string str)//7
{
   unsigned int hash = 5381;

   for (int i = 0; i < str.length(); i++)
   {
      hash = ((hash << 5) + hash) + str[i];
   }

   return hash;
}

unsigned int DEKHash(string str)//8
{
   unsigned int hash = static_cast<unsigned int>(str.length());

   for (int i = 0; i < str.length(); i++)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ str[i];
   }

   return hash;
}

unsigned int BPHash(string str)//9
{
   unsigned int hash = 0;
   for (int i = 0; i < str.length(); i++)
   {
      hash = hash << 7 ^ str[i];
   }

   return hash;
}

bool in_set(string url)
{
	for (int i = 0; i < hash_func_num; i++)
	{
		int bit = hash_funcs[i](url) % capacity;
		if (!bit1.test(bit))
		{
			return false;
		}
	}
	return true;
}

void set_in(string url)
{
	for (int i = 0; i < hash_func_num; i++)
	{
		bit1.set(hash_funcs[i](url) % capacity);
	}
}

