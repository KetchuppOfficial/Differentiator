#ifndef HASH_PROTECTION_H_INCLUDED
#define HASH_PROTECTION_H_INCLUDED

#define CHECK_HASH(condition)               \
do                                          \
{                                           \
    if (Check_Hash ((condition)) == ERROR)  \
        return ERROR;                       \
}                                           \
while (0)

#define HASH_VAR(hash, var)                 \
do                                          \
{                                           \
    hash += (hash_t)(var);                  \
    hash += (hash << 10);                   \
    hash ^= (hash >> 6);                    \
}                                           \
while (0)

typedef long long int hash_t;

#endif // HASH_PROTECTION_H_INCLUDED
