#ifndef AES
#define AES

#include <stdint.h>  
#include <stddef.h> 
#include <string.h>
#include <stdlib.h>

#define AES_BLOCK_SIZE 16
#define AES_ROUNDS 10
#define AES_ROUND_KEYS_SIZE 176
#define MAX_BYTES 4096
// -> Function def
// Prepering a message block so it would 16 bytes.
void prepare_Block(const char *input, uint8_t **block, size_t *num_block);
// Reverting from hex to reading text
void revert_Hex(uint8_t **block);

// Prepering and expanding key 16 key + 160 (10 rounds) -> 176 bytes.
// roundKeys var is the key that was expanded.
void key_Expansion(const uint8_t *key, uint8_t *roundKeys);
void rotWord(uint8_t *word);
void subWord(uint8_t *word);

// Main function for encrypting 11 rounds
// Takes in the *block -> Hex message, *roundKey -> Key that was expanded.
void aes_Encrypt(uint8_t *block, uint8_t *roundKeys);
// Addit funct for encrypt
void add_Round(uint8_t *block, uint8_t *roundKeys);
void sub_Bytes(uint8_t *block);
void shift_Rows(uint8_t *block);
void mix_Column(uint8_t *block);

// Func to decrypt.
void de_Crypt(uint8_t *block, uint8_t *roundKeys);
void inv_Rows(uint8_t *block);
void inv_Bytes(uint8_t *block);
void inv_Column(uint8_t *block);

// Addit functions.
uint8_t gmul(uint8_t a, uint8_t b);

#endif