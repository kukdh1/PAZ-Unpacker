#include "Crypt.h"

namespace kukdh1 {
  Crypt::~Crypt() {
    // DO NOTHING
  }

  void Crypt::encrypt(uint8_t *in, uint32_t in_len, uint8_t **out, uint32_t *out_len) {
    if (out) {
      *out = NULL;
    }
    if (out_len) {
      out_len = 0;
    }
  }

  void Crypt::decrypt(uint8_t *in, uint32_t in_len, uint8_t **out, uint32_t *out_len) {
    if (out) {
      *out = NULL;
    }
    if (out_len) {
      out_len = 0;
    }
  }

  const int CryptICE::ice_smod[4][4] = {
    { 333, 313, 505, 369 },
    { 379, 375, 319, 391 },
    { 361, 445, 451, 397 },
    { 397, 425, 395, 505 }
  };

  const int CryptICE::ice_sxor[4][4] = {
    { 0x83, 0x85, 0x9b, 0xcd },
    { 0xcc, 0xa7, 0xad, 0x41 },
    { 0x4b, 0x2e, 0xd4, 0x33 },
    { 0xea, 0xcb, 0x2e, 0x04 }
  };

  const uint32_t CryptICE::ice_pbox[32] = {
    0x00000001, 0x00000080, 0x00000400, 0x00002000,
    0x00080000, 0x00200000, 0x01000000, 0x40000000,
    0x00000008, 0x00000020, 0x00000100, 0x00004000,
    0x00010000, 0x00800000, 0x04000000, 0x20000000,
    0x00000004, 0x00000010, 0x00000200, 0x00008000,
    0x00020000, 0x00400000, 0x08000000, 0x10000000,
    0x00000002, 0x00000040, 0x00000800, 0x00001000,
    0x00040000, 0x00100000, 0x02000000, 0x80000000
  };

  const uint32_t CryptICE::ice_keyrot[16] = {
    0, 1, 2, 3, 2, 1, 3, 0,
    1, 3, 2, 0, 3, 1, 0, 2
  };

  uint32_t CryptICE::gf_mult(uint32_t a, uint32_t b, uint32_t m) {
    uint32_t res = 0;

    while (b) {
      if (b & 1)
        res ^= a;

      a <<= 1;
      b >>= 1;

      if (a >= 256)
        a ^= m;
    }

    return res;
  }

  uint32_t CryptICE::gf_exp7(uint32_t b, uint32_t m) {
    uint32_t x;

    if (b == 0)
      return 0;

    x = gf_mult(b, b, m);
    x = gf_mult(b, x, m);
    x = gf_mult(x, x, m);

    return gf_mult(b, x, m);
  }

  uint32_t CryptICE::ice_perm32(uint32_t x) {
    uint32_t res = 0;
    const uint32_t *pbox = ice_pbox;

    while (x) {
      if (x & 1)
        res |= *pbox;
      pbox++;
      x >>= 1;
    }

    return res;
  }

  uint32_t CryptICE::ice_f(uint32_t p, subkey sk) {
    uint32_t tl, tr; /* Expanded 40-bit values */
    uint32_t al, ar; /* Salted expanded 40-bit values */

                     /* Left half expansion */
    tl = ((p >> 16) & 0x3ff) | (((p >> 14) | (p << 18)) & 0xffc00);

    /* Right half expansion */
    tr = (p & 0x3ff) | ((p << 2) & 0xffc00);

    /* Perform the salt permutation */
    /* al = (tr & sk[2]) | (tl & ~sk[2]); */
    /* ar = (tl & sk[2]) | (tr & ~sk[2]); */
    al = sk[2] & (tl ^ tr);
    ar = al ^ tr;
    al ^= tl;

    al ^= sk[0];/* XOR with the subkey */
    ar ^= sk[1];

    /* S-box lookup and permutation */
    return (ice_sbox[0][al >> 10] | ice_sbox[1][al & 0x3ff]
      | ice_sbox[2][ar >> 10] | ice_sbox[3][ar & 0x3ff]);
  }

  void CryptICE::ice_sbox_init() {
    for (int i = 0; i < 1024; i++) {
      int col = (i >> 1) & 0xff;
      int row = (i & 0x1) | ((i & 0x200) >> 8);
      unsigned long  x;

      x = gf_exp7(col ^ ice_sxor[0][row], ice_smod[0][row]) << 24;
      ice_sbox[0][i] = ice_perm32(x);

      x = gf_exp7(col ^ ice_sxor[1][row], ice_smod[1][row]) << 16;
      ice_sbox[1][i] = ice_perm32(x);

      x = gf_exp7(col ^ ice_sxor[2][row], ice_smod[2][row]) << 8;
      ice_sbox[2][i] = ice_perm32(x);

      x = gf_exp7(col ^ ice_sxor[3][row], ice_smod[3][row]);
      ice_sbox[3][i] = ice_perm32(x);
    }
  }

  void CryptICE::ice_key_sched_build(uint16_t *kb, uint32_t n, const uint32_t *keyrot) {
    for (int i = 0; i < 8; i++) {
      int j;
      int kr = keyrot[i];
      subkey *isk = &keysched[n + i];

      for (j = 0; j < 3; j++)
        (*isk)[j] = 0;

      for (j = 0; j < 15; j++) {
        int k;
        uint32_t *curr_sk = &(*isk)[j % 3];

        for (k = 0; k<4; k++) {
          uint16_t *curr_kb = &kb[(kr + k) & 3];
          uint32_t bit = *curr_kb & 1;

          *curr_sk = (*curr_sk << 1) | bit;
          *curr_kb = (*curr_kb >> 1) | ((bit ^ 1) << 15);
        }
      }
    }
  }

  void CryptICE::ice_key_set(uint8_t *key) {
    uint16_t kb[4];

    if (key_rounds == 8) {
      for (int i = 0; i < 4; i++)
        kb[3 - i] = (key[i * 2] << 8) | key[i * 2 + 1];

      ice_key_sched_build(kb, 0, ice_keyrot);
    }
    else {
      for (int i = 0; i < key_size; i++) {

        for (int j = 0; j < 4; j++)
          kb[3 - j] = (key[i * 8 + j * 2] << 8) | key[i * 8 + j * 2 + 1];

        ice_key_sched_build(kb, i * 8, ice_keyrot);
        ice_key_sched_build(kb, key_rounds - 8 - i * 8, &ice_keyrot[8]);
      }
    }
  }

  CryptICE::CryptICE(uint8_t *key, uint32_t keylen) {
    key_size = 1;
    key_rounds = keylen;
    keysched = NULL;

    if (keylen != 8) {
      if (keylen % 16 == 0) {
        key_size = keylen / 16;
      }
      else {
        throw CryptException(Crypt::ERROR_INVALID_LENGTH);
      }
    }

    keysched = (subkey *)calloc(key_rounds, sizeof(subkey));
    if (!keysched) {
      throw CryptException(Crypt::ERROR_OUT_OF_MEMORY);
    }

    ice_sbox_init();
    ice_key_set(key);
  }

  CryptICE::~CryptICE() {
    if (keysched) {
      free(keysched);
    }
  }

  void CryptICE::encrypt(uint8_t *in, uint32_t in_len, uint8_t **out, uint32_t *out_len) {
    if (out && out_len) {
      if (in_len % 8 != 0) {
        throw CryptException(Crypt::ERROR_INVALID_LENGTH);
      }
      else {
        uint32_t count = 0;
        uint8_t ptext[8];
        uint8_t ctext[8];
        uint32_t l, r;

        count = in_len >> 3;
        *out = (uint8_t *)calloc(in_len, 1);
        *out_len = in_len;

        if (!(*out)) {
          throw CryptException(Crypt::ERROR_OUT_OF_MEMORY);
        }

        for (uint32_t idx = 0; idx < count; idx++) {
          memcpy(ptext, in + idx * 8, 8);

          l = (((uint32_t)ptext[0]) << 24)
            | (((uint32_t)ptext[1]) << 16)
            | (((uint32_t)ptext[2]) << 8) | ptext[3];
          r = (((uint32_t)ptext[4]) << 24)
            | (((uint32_t)ptext[5]) << 16)
            | (((uint32_t)ptext[6]) << 8) | ptext[7];

          for (int i = 0; i < key_rounds; i += 2) {
            l ^= ice_f(r, keysched[i]);
            r ^= ice_f(l, keysched[i + 1]);
          }

          for (int i = 0; i < 4; i++) {
            ctext[3 - i] = r & 0xff;
            ctext[7 - i] = l & 0xff;

            r >>= 8;
            l >>= 8;
          }

          memcpy(*out + idx * 8, ctext, 8);
        }
      }
    }
    else {
      throw CryptException(Crypt::ERROR_INVALID_PARAMETERS);
    }
  }

  void CryptICE::decrypt(uint8_t *in, uint32_t in_len, uint8_t **out, uint32_t *out_len) {
    if (out && out_len) {
      if (in_len % 8 != 0) {
        throw CryptException(Crypt::ERROR_INVALID_LENGTH);
      }
      else {
        uint32_t count = 0;
        uint8_t ptext[8];
        uint8_t ctext[8];
        uint32_t l, r;

        count = in_len >> 3;
        *out = (uint8_t *)calloc(in_len, 1);
        *out_len = in_len;

        if (!(*out)) {
          throw CryptException(Crypt::ERROR_OUT_OF_MEMORY);
        }

        for (uint32_t idx = 0; idx < count; idx++) {
          memcpy(ctext, in + idx * 8, 8);

          l = (((uint32_t)ctext[0]) << 24)
            | (((uint32_t)ctext[1]) << 16)
            | (((uint32_t)ctext[2]) << 8) | ctext[3];
          r = (((uint32_t)ctext[4]) << 24)
            | (((uint32_t)ctext[5]) << 16)
            | (((uint32_t)ctext[6]) << 8) | ctext[7];

          for (int i = key_rounds - 1; i > 0; i -= 2) {
            l ^= ice_f(r, keysched[i]);
            r ^= ice_f(l, keysched[i - 1]);
          }

          for (int i = 0; i < 4; i++) {
            ptext[3 - i] = r & 0xff;
            ptext[7 - i] = l & 0xff;

            r >>= 8;
            l >>= 8;
          }

          memcpy(*out + idx * 8, ptext, 8);
        }
      }
    }
    else {
      throw CryptException(Crypt::ERROR_INVALID_PARAMETERS);
    }
  }

  /* CRC calculation codes from Blackdesert_Launcher.exe (KR Client) */
  uint32_t calculatePackCRC(uint8_t * data, uint32_t length) {
    int v2; // ST24_4@4
    unsigned int v3; // ST20_4@4
    unsigned int v4; // ST2C_4@4
    int v5; // ST20_4@4
    unsigned int v6; // ST24_4@4
    int v7; // ST2C_4@4
    unsigned int v8; // ST20_4@4
    int v9; // ST24_4@4
    unsigned int v10; // ST2C_4@4
    int v11; // ST20_4@4
    unsigned int v12; // ST24_4@4
    unsigned int result; // eax@18
    int v14; // ST24_4@23
    unsigned int v15; // ST20_4@23
    unsigned int v16; // ST2C_4@23
    int v17; // ST20_4@23
    unsigned int v18; // ST24_4@23
    int v19; // ST2C_4@23
    unsigned int v20; // ST20_4@23
    int v21; // ST24_4@23
    unsigned int v22; // ST2C_4@23
    int v23; // ST20_4@23
    unsigned int v24; // ST24_4@23
    int v25; // ST24_4@41
    unsigned int v26; // ST20_4@41
    unsigned int v27; // ST2C_4@41
    int v28; // ST20_4@41
    unsigned int v29; // ST24_4@41
    int v30; // ST2C_4@41
    unsigned int v31; // ST20_4@41
    int v32; // ST24_4@41
    unsigned int v33; // ST2C_4@41
    int v34; // ST20_4@41
    unsigned int v35; // ST24_4@41
    unsigned int v36; // ST20_4@56
    unsigned int v37; // ST2C_4@56
    unsigned int v38; // ST24_4@56
    unsigned int v39; // ST20_4@56
    unsigned int v40; // ST2C_4@56
    unsigned int v41; // ST24_4@56
    int *pdwData0; // [sp+Ch] [bp-24h]@39
    int nBeginValue0; // [sp+20h] [bp-10h]@1
    unsigned int nBeginValue1; // [sp+24h] [bp-Ch]@1
    int nBeginValue2; // [sp+2Ch] [bp-4h]@1

    nBeginValue0 = length - 558228019;
    nBeginValue1 = length - 558228019;
    nBeginValue2 = length - 558228019;
    pdwData0 = (int *)data;
    if (!((uint32_t)data & 3))
    {
      while (length > 0xC)
      {
        v2 = pdwData0[1] + nBeginValue1;
        v3 = pdwData0[2] + nBeginValue0;
        v4 = (*pdwData0 + nBeginValue2 - v3) ^ ((v3 >> 28) | 16 * v3);
        v5 = v2 + v3;
        v6 = (v2 - v4) ^ ((v4 >> 26) | (v4 << 6));
        v7 = v5 + v4;
        v8 = (v5 - v6) ^ ((v6 >> 24) | (v6 << 8));
        v9 = v7 + v6;
        v10 = (v7 - v8) ^ ((v8 >> 16) | (v8 << 16));
        v11 = v9 + v8;
        v12 = (v9 - v10) ^ ((v10 >> 13) | (v10 << 19));
        nBeginValue2 = v11 + v10;
        nBeginValue0 = (v11 - v12) ^ ((v12 >> 28) | 16 * v12);
        nBeginValue1 = nBeginValue2 + v12;
        length -= 12;
        pdwData0 += 3;
      }
      switch (length)
      {
        case 0xCu:
          nBeginValue0 += pdwData0[2];
          nBeginValue1 += pdwData0[1];
          nBeginValue2 += *pdwData0;
          break;
        case 0xBu:
          nBeginValue0 += pdwData0[2] & 0xFFFFFF;
          nBeginValue1 += pdwData0[1];
          nBeginValue2 += *pdwData0;
          break;
        case 0xAu:
          nBeginValue0 += pdwData0[2] & 0xFFFF;
          nBeginValue1 += pdwData0[1];
          nBeginValue2 += *pdwData0;
          break;
        case 9u:
          nBeginValue0 += pdwData0[2] & 0xFF;
          nBeginValue1 += pdwData0[1];
          nBeginValue2 += *pdwData0;
          break;
        case 8u:
          nBeginValue1 += pdwData0[1];
          nBeginValue2 += *pdwData0;
          break;
        case 7u:
          nBeginValue1 += pdwData0[1] & 0xFFFFFF;
          nBeginValue2 += *pdwData0;
          break;
        case 6u:
          nBeginValue1 += pdwData0[1] & 0xFFFF;
          nBeginValue2 += *pdwData0;
          break;
        case 5u:
          nBeginValue1 += pdwData0[1] & 0xFF;
          nBeginValue2 += *pdwData0;
          break;
        case 4u:
          nBeginValue2 += *pdwData0;
          break;
        case 3u:
          nBeginValue2 += *pdwData0 & 0xFFFFFF;
          break;
        case 2u:
          nBeginValue2 += *pdwData0 & 0xFFFF;
          break;
        case 1u:
          nBeginValue2 += *pdwData0 & 0xFF;
          break;
        case 0u:
          return nBeginValue0;
      }
    }
    else if (!((uint32_t)data & 1))
    {
      while (length > 0xC)
      {
        v14 = nBeginValue1 + (pdwData0[3] << 16) + pdwData0[2];
        v15 = nBeginValue0 + (pdwData0[5] << 16) + pdwData0[4];
        v16 = (nBeginValue2 + (pdwData0[1] << 16) + *pdwData0 - v15) ^ ((v15 >> 28) | 16 * v15);
        v17 = v14 + v15;
        v18 = (v14 - v16) ^ ((v16 >> 26) | (v16 << 6));
        v19 = v17 + v16;
        v20 = (v17 - v18) ^ ((v18 >> 24) | (v18 << 8));
        v21 = v19 + v18;
        v22 = (v19 - v20) ^ ((v20 >> 16) | (v20 << 16));
        v23 = v21 + v20;
        v24 = (v21 - v22) ^ ((v22 >> 13) | (v22 << 19));
        nBeginValue2 = v23 + v22;
        nBeginValue0 = (v23 - v24) ^ ((v24 >> 28) | 16 * v24);
        nBeginValue1 = nBeginValue2 + v24;
        length -= 12;
        pdwData0 += 6;
      }
      switch (length)
      {
        case 0xCu:
          nBeginValue0 += (pdwData0[5] << 16) + pdwData0[4];
          nBeginValue1 += (pdwData0[3] << 16) + pdwData0[2];
          nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
          break;
        case 0xBu:
          nBeginValue0 += *((uint8_t *)pdwData0 + 10) << 16;
        case 0xAu:
          nBeginValue0 += pdwData0[4];
          nBeginValue1 += (pdwData0[3] << 16) + pdwData0[2];
          nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
          break;
        case 9u:
          nBeginValue0 += *((uint8_t *)pdwData0 + 8);
        case 8u:
          nBeginValue1 += (pdwData0[3] << 16) + pdwData0[2];
          nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
          break;
        case 7u:
          nBeginValue1 += *((uint8_t *)pdwData0 + 6) << 16;
        case 6u:
          nBeginValue1 += pdwData0[2];
          nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
          break;
        case 5u:
          nBeginValue1 += *((uint8_t *)pdwData0 + 4);
        case 4u:
          nBeginValue2 += (pdwData0[1] << 16) + *pdwData0;
          break;
        case 3u:
          nBeginValue2 += *((uint8_t *)pdwData0 + 2) << 16;
        case 2u:
          nBeginValue2 += *pdwData0;
          break;
        case 1u:
          nBeginValue2 += *(uint8_t *)pdwData0;
          break;
        case 0u:
          return nBeginValue0;
      }
    }
    else {
      while (length > 0xC)
      {
        v25 = nBeginValue1
          + *((uint8_t *)pdwData0 + 4)
          + (*((uint8_t *)pdwData0 + 5) << 8)
          + (*((uint8_t *)pdwData0 + 6) << 16)
          + (*((uint8_t *)pdwData0 + 7) << 24);
        v26 = nBeginValue0
          + *((uint8_t *)pdwData0 + 8)
          + (*((uint8_t *)pdwData0 + 9) << 8)
          + (*((uint8_t *)pdwData0 + 10) << 16)
          + (*((uint8_t *)pdwData0 + 11) << 24);
        v27 = (nBeginValue2
          + *(uint8_t *)pdwData0
          + (*((uint8_t *)pdwData0 + 1) << 8)
          + (*((uint8_t *)pdwData0 + 2) << 16)
          + (*((uint8_t *)pdwData0 + 3) << 24)
          - v26) ^ ((v26 >> 28) | 16 * v26);
        v28 = v25 + v26;
        v29 = (v25 - v27) ^ ((v27 >> 26) | (v27 << 6));
        v30 = v28 + v27;
        v31 = (v28 - v29) ^ ((v29 >> 24) | (v29 << 8));
        v32 = v30 + v29;
        v33 = (v30 - v31) ^ ((v31 >> 16) | (v31 << 16));
        v34 = v32 + v31;
        v35 = (v32 - v33) ^ ((v33 >> 13) | (v33 << 19));
        nBeginValue2 = v34 + v33;
        nBeginValue0 = (v34 - v35) ^ ((v35 >> 28) | 16 * v35);
        nBeginValue1 = nBeginValue2 + v35;
        length -= 12;
        pdwData0 += 3;
      }
      switch (length)
      {
        case 0u:
          result = nBeginValue0;
          break;
        case 0xCu:
          nBeginValue0 += *((uint8_t *)pdwData0 + 11) << 24;
        case 0xBu:
          nBeginValue0 += *((uint8_t *)pdwData0 + 10) << 16;
        case 0xAu:
          nBeginValue0 += *((uint8_t *)pdwData0 + 9) << 8;
        case 9u:
          nBeginValue0 += *((uint8_t *)pdwData0 + 8);
        case 8u:
          nBeginValue1 += *((uint8_t *)pdwData0 + 7) << 24;
        case 7u:
          nBeginValue1 += *((uint8_t *)pdwData0 + 6) << 16;
        case 6u:
          nBeginValue1 += *((uint8_t *)pdwData0 + 5) << 8;
        case 5u:
          nBeginValue1 += *((uint8_t *)pdwData0 + 4);
        case 4u:
          nBeginValue2 += *((uint8_t *)pdwData0 + 3) << 24;
        case 3u:
          nBeginValue2 += *((uint8_t *)pdwData0 + 2) << 16;
        case 2u:
          nBeginValue2 += *((uint8_t *)pdwData0 + 1) << 8;
        case 1u:
          nBeginValue2 += *(uint8_t *)pdwData0;
      }
    }

    v36 = (nBeginValue1 ^ nBeginValue0) - ((nBeginValue1 >> 18) | (nBeginValue1 << 14));
    v37 = (v36 ^ nBeginValue2) - ((v36 >> 21) | (v36 << 11));
    v38 = (v37 ^ nBeginValue1) - ((v37 >> 7) | (v37 << 25));
    v39 = (v38 ^ v36) - ((v38 >> 16) | (v38 << 16));
    v40 = (v39 ^ v37) - ((v39 >> 28) | 16 * v39);
    v41 = (v40 ^ v38) - ((v40 >> 18) | (v40 << 14));

    return (v41 ^ v39) - ((v41 >> 8) | (v41 << 24));
  }

  /* Decompress codes from quickbms, refined by kukdh1 */
  uint32_t blackdesert_unpack_core(uint8_t *pInput, uint8_t *pOutput, uint32_t uiDecompressedLength, uint8_t *pOutput2, uint32_t pInput_size)
  {
    static const uint8_t blackdesert_unpack_dwTable[] = {
      0x4, 0x0, 0x1, 0x0, 0x2, 0x0, 0x1, 0x0, 0x3, 0x0, 0x1, 0x0, 0x2, 0x0, 0x1, 0x0
    };

    uint8_t *pOutputIndex;
    uint8_t *pInputIndex;
    uint32_t uiBlockGroupHeader;
    uint32_t uiCompressedLength;
    uint32_t uiBlockHeader;
    uint8_t *pLastInputIndex;
    uint8_t *pLastOutputIndex;

    pOutputIndex = pOutput;
    uiBlockGroupHeader = 1;
    pLastOutputIndex = uiDecompressedLength + pOutput - 1;

    if (pInput[0] & 0x02) {
      uiCompressedLength = (*(uint32_t *)(pInput + 1)) & 0xFFFFFFFF;
      pInputIndex = pInput + 9;
    }
    else {
      uiCompressedLength = pInput[1];
      pInputIndex = pInput + 3;
    }

    pLastInputIndex = pInput + uiCompressedLength - 1;

    while (1)
    {
      while (1)
      {
        uint32_t uiRepeatIndex;
        uint32_t uiBlockLength;

        if (uiBlockGroupHeader == 1)
        {
          if (pInputIndex + 3 > pLastInputIndex)
            return -1;

          // get value of block header
          uiBlockGroupHeader = *(uint32_t *)pInputIndex;
          pInputIndex += 4;
        }

        if (pInputIndex + 3 > pLastInputIndex)  // pInputIndex + 4 > pEndOfInput
          return -2;

        uiBlockHeader = *(uint32_t *)pInputIndex;

        if (!(uiBlockGroupHeader & 1))
          break;

        // get data from compression header and jump to block data?
        if ((uiBlockHeader & 0x03) == 0x03) {
          if ((uiBlockHeader & 0x7F) == 3)
          {
            uiRepeatIndex = uiBlockHeader >> 15;
            uiBlockLength = ((uiBlockHeader >> 7) & 0xFF) + 3;
            pInputIndex += 4;
          }
          else
          {
            uiRepeatIndex = (uiBlockHeader >> 7) & 0x1FFFF;
            uiBlockLength = ((uiBlockHeader >> 2) & 0x1F) + 2;
            pInputIndex += 3;
          }
        }
        else if ((uiBlockHeader & 0x03) == 0x02) {
          uiRepeatIndex = (uint16_t)uiBlockHeader >> 6;
          uiBlockLength = ((uiBlockHeader >> 2) & 0xF) + 3;
          pInputIndex += 2;
        }
        else if ((uiBlockHeader & 0x03) == 0x01) {
          uiRepeatIndex = (uint16_t)uiBlockHeader >> 2;
          uiBlockLength = 3;
          pInputIndex += 2;
        }
        else {
          uiRepeatIndex = (uint8_t)uiBlockHeader >> 2;
          uiBlockLength = 3;
          pInputIndex++;
        }

        // v16 < beginning of array || uiRepeatIndex > 3 || uiBlockLength > left bytes - 3
        if (pOutputIndex - uiRepeatIndex < pOutput2 || uiRepeatIndex < 3u || uiBlockLength > pLastOutputIndex - pOutputIndex - 3u)
          return -3;

        uint8_t *ptr = pOutputIndex;

        // Copy duplicated data
        for (uint32_t i = 0; i < uiBlockLength; i += 3) {
          *(uint32_t *)ptr = *(uint32_t *)(ptr - uiRepeatIndex);
          ptr += 3;
        }
        
        uiBlockGroupHeader >>= 1;
        pOutputIndex += uiBlockLength;
      }

      // if 11 ~ 0 bytes left
      if (pOutputIndex >= pLastOutputIndex - 10)
        break;

      int validDataLength = blackdesert_unpack_dwTable[uiBlockGroupHeader & 0xF];
      *(uint32_t *)pOutputIndex = uiBlockHeader;
      uiBlockGroupHeader >>= validDataLength;
      pOutputIndex += validDataLength;
      pInputIndex += validDataLength;
    }

    // Not finished (pOutputIndex == pLastOutputIndex means 1byte left)
    // Just copy last data
    if (pOutputIndex <= pLastOutputIndex)
    {
      uint8_t *pEndOfInput = pLastInputIndex + 1;

      while (1)
      {
        if (uiBlockGroupHeader == 1)
        {
          pInputIndex += 4;
          uiBlockGroupHeader = 0x80000000;
        }

        if (pInputIndex >= pEndOfInput)
          break;

        *(uint8_t *)pOutputIndex++ = *(uint8_t *)pInputIndex++;

        uiBlockGroupHeader >>= 1;

        // return written length
        if (pOutputIndex > pLastOutputIndex)
          return pOutputIndex - pOutput;
      }

      return -4;
    }

    // return written length
    return pOutputIndex - pOutput;
  }

  /* Decompress codes from quickbms, refined by kukdh1 */
  uint32_t decompress(uint8_t *pInput, uint8_t *pOutput) {
    uint32_t length;

    if (pInput[0] & 0x02) {
      length = (*(uint32_t *)(pInput + 5));
    }
    else {
      length = pInput[2];
    }

    if (pInput[0] & 0x01) {
      length = blackdesert_unpack_core(pInput, pOutput, length, pOutput, 0);
    }
    else {
      memcpy(pOutput, pInput + (pInput[0] & 0x02 ? 9 : 3), length);
    }

    return length;
  }
}
