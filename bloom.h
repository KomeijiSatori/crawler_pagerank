#include <string>
using namespace std;

const int hash_func_num = 9;
const int capacity= 2080000;

unsigned int RSHash(string);
unsigned int JSHash(string);
unsigned int PJWHash(string);
unsigned int ELFHash(string);
unsigned int BKDRHash(string);
unsigned int SDBMHash(string);
unsigned int DJBHash(string);
unsigned int DEKHash(string);
unsigned int BPHash(string);
void init_bloom(void);
bool in_set(string);
void set_in(string);


