#pragma once

#ifndef _CRYPT_H_
#define _CRYPT_H_

#include <iostream>
#include <string>
#include <exception>

namespace kukdh1 {
  class CryptException : public std::exception {
    private:
      int code;
    public:
      CryptException(int n) : code(n) {}
      int what() { return code; }
  };

  class Crypt {
    public:
      enum {
        ERROR_INVALID_LENGTH,
        ERROR_OUT_OF_MEMORY,
        ERROR_INVALID_PARAMETERS
      };

      virtual ~Crypt();
      virtual void encrypt(uint8_t *in, uint32_t in_len, uint8_t **out, uint32_t *out_len);
      virtual void decrypt(uint8_t *in, uint32_t in_len, uint8_t **out, uint32_t *out_len);
  };

  class CryptICE : public Crypt {
    public:
      typedef uint32_t subkey[3];

    private:
      static const int ice_smod[4][4];
      static const int ice_sxor[4][4];
      static const uint32_t ice_pbox[32];
      static const uint32_t ice_keyrot[16];
      uint32_t ice_sbox[4][1024];

      int key_size;
      int key_rounds;
      subkey *keysched;

      uint32_t gf_mult(uint32_t a, uint32_t b, uint32_t m);
      uint32_t gf_exp7(uint32_t b, uint32_t m);
      uint32_t ice_perm32(uint32_t x);
      uint32_t ice_f(uint32_t p, subkey sk);
      void ice_key_sched_build(uint16_t *kb, uint32_t n, const uint32_t *keyrot);
      void ice_key_set(uint8_t *key);
      void ice_sbox_init();
      
    public:
      CryptICE(uint8_t *key, uint32_t keylen);
      ~CryptICE();
      void encrypt(uint8_t *in, uint32_t in_len, uint8_t **out, uint32_t *out_len);
      void decrypt(uint8_t *in, uint32_t in_len, uint8_t **out, uint32_t *out_len);
  };

  uint32_t calculatePackCRC(uint8_t *data, uint32_t length);
  uint32_t decompress(uint8_t *in, uint8_t *out);
}

#endif
