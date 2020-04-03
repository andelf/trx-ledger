/*******************************************************************************
*   TRON Ledger
*   (c) 2018 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/


#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"
#include "base58.h"

#ifndef TARGET_BLUE
  #include "os_io_seproxyhal.h"
#else
  void io_seproxyhal_io_heartbeat(void) {}
#endif

#define _BE32_TO_LE32(bytes)                                                   \
  (((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |                   \
   ((uint32_t)bytes[2] << 8) | ((uint32_t)bytes[3]))

#define _BE64_TO_LE64(bytes)                                                   \
  (((uint64_t)bytes[0] << 56) | ((uint64_t)bytes[1] << 48) |                   \
   ((uint64_t)bytes[2] << 40) | ((uint64_t)bytes[3] << 32) |                   \
   ((uint64_t)bytes[4] << 24) | ((uint64_t)bytes[5] << 16) |                   \
   ((uint64_t)bytes[6] << 8) | ((uint64_t)bytes[7]))

#define _DISPLAY_WIDTH 14
#define _DISPLAY_NORMAL_DIGITS 12
#define _DISPLAY_EXPONENT_DIGITS 8

// src: uint256: BE u8[32], !destructive
// precision: 2, 6, 18
// dst: _DISPLAY_WIDTH + 1
void format_uint256_with_decimal(uint8_t *src, uint8_t precision, uint8_t *dst) {
  uint32_t *src_le32 = (uint32_t *)src;
  uint8_t buf[_DISPLAY_WIDTH];
  int buf_pos = 0;
  int cursor;

  // skip zero bits
  for (cursor = 0; src_le32[cursor] == 0 && cursor < 8; cursor++)
    ;
  // FIX endianess
  for (int i = cursor; i < 8; i++) {
    src_le32[i] = _BE32_TO_LE32((src + i * 4));
  }

  // base32 to base10
  while (src_le32[cursor] != 0 && cursor < 8) {
    uint64_t remainder = 0;
    for (int i = cursor; i < 8; i++) {
      lldiv_t output = lldiv((remainder << 32) + (uint64_t)src_le32[i], 10);
      src_le32[i] = output.quot;
      remainder = output.rem;
    }
    buf[buf_pos++ % _DISPLAY_WIDTH] = (uint8_t)remainder + '0';
    if (src_le32[cursor] == 0) {
      cursor++;
    }
  }

  // within normal range
  if (buf_pos < _DISPLAY_WIDTH && precision <= buf_pos) {
    for (int i = 0; i < buf_pos; i++) {
      if (buf_pos - i == precision) {
        *dst++ = '.';
      }
      *dst++ = buf[buf_pos - i - 1];
    }
    *dst++ = '\0';
    return;
  }

  // overflow, use exponent

  int exponent = 0;
  // adjust precision
  if (buf_pos >= _DISPLAY_WIDTH) {
    exponent = ((buf_pos - precision) / 3) * 3;

    // do not use exponent when the decimal point fits window
    if (exponent < _DISPLAY_NORMAL_DIGITS) {
      exponent = 0;
    }
  }
  // negative exponent
  if (buf_pos < precision) {
    exponent = -((precision - buf_pos) / 3 + 1) * 3;

    // do not use exponent for >= 1e-3
    if (exponent >= -3) {
      exponent = 0;
    }
  }
  int fixpoint = precision + exponent;

  // significant figures
  int figures = _DISPLAY_NORMAL_DIGITS;
  if (exponent != 0) {
    figures = _DISPLAY_EXPONENT_DIGITS;
  }

  // adjust for decimal point and leading zero
  if (buf_pos - fixpoint <= 0) {
    figures = figures + (buf_pos - fixpoint) - 2;
  }

  // leading zero and decimal point
  for (int i = 0; i >= buf_pos - fixpoint; i--) {
    *dst++ = '0';
    if (i == 0) {
      *dst++ = '.';
    }
  }

  for (int i = 0; i < figures && buf_pos - i - 1 >= 0; i++) {
    if (i == buf_pos - fixpoint && i > 0) {
      *dst++ = '.';
    }
    *dst++ = buf[(buf_pos - i - 1) % _DISPLAY_WIDTH];
  }

  if (exponent != 0) {
    snprintf((char *)dst, 8, "e%+d", exponent);
  } else {
    *dst++ = '\0';
  }
}

void getAddressFromKey(cx_ecfp_public_key_t *publicKey, uint8_t *address) {
  return getAddressFromPublicKey(publicKey->W, address);
}

void getAddressFromPublicKey(const uint8_t *publicKey, uint8_t *address) {
  uint8_t hashAddress[32];
  cx_sha3_t sha3;

  cx_keccak_init(&sha3, 256);
  cx_hash((cx_hash_t *)&sha3, CX_LAST, publicKey + 1, 64, hashAddress, 32);

  memmove(address, hashAddress + 11, ADDRESS_SIZE);
  address[0] = ADD_PRE_FIX_BYTE_MAINNET;
}

void getBase58FromAddress(uint8_t *address, uint8_t *out, cx_sha256_t *sha2, bool truncate) {
  uint8_t sha256[32];
  uint8_t addchecksum[ADDRESS_SIZE + 4];

  cx_sha256_init(sha2);
  cx_hash((cx_hash_t *)sha2, CX_LAST, address, 21, sha256, 32);
  cx_sha256_init(sha2);
  cx_hash((cx_hash_t *)sha2, CX_LAST, sha256, 32, sha256, 32);

  memmove(addchecksum, address, ADDRESS_SIZE);
  memmove(addchecksum + ADDRESS_SIZE, sha256, 4);

  encode_base_58(&addchecksum[0], 25, (char *)out, BASE58CHECK_ADDRESS_SIZE);
  out[BASE58CHECK_ADDRESS_SIZE] = '\0';
  if (truncate) {
    memmove((void *)out+5, "...", 3);
    memmove((void *)out+8,(const void *)(out+BASE58CHECK_ADDRESS_SIZE-5), 6); // include \0 char
  }
}

void transactionHash(uint8_t *raw, uint16_t dataLength,
                        uint8_t *out, cx_sha256_t* sha2) {

    cx_sha256_init(sha2);
    cx_hash((cx_hash_t*)sha2, CX_LAST, raw, dataLength, out, 32);
}

void signTransaction(transactionContext_t *transactionContext) {

    uint8_t privateKeyData[32];
    cx_ecfp_private_key_t privateKey;
    uint8_t rLength, sLength, rOffset, sOffset;
    uint8_t signature[100];
    unsigned int info = 0;

    // Get Private key from BIP32 path
    io_seproxyhal_io_heartbeat();
    os_perso_derive_node_bip32(
        CX_CURVE_256K1, transactionContext->bip32_path.indices,
        transactionContext->bip32_path.length, privateKeyData, NULL);
    cx_ecfp_init_private_key(CX_CURVE_256K1, privateKeyData, 32, &privateKey);
    os_memset(privateKeyData, 0, sizeof(privateKeyData));
    // Sign transaction hash
    io_seproxyhal_io_heartbeat();
    cx_ecdsa_sign(&privateKey, CX_RND_RFC6979 | CX_LAST, CX_SHA256,
                      transactionContext->hash, sizeof(transactionContext->hash),
                      signature, sizeof(signature), &info);

    io_seproxyhal_io_heartbeat();
    os_memset(&privateKey, 0, sizeof(privateKey));
    // recover signature
    rLength = signature[3];
    sLength = signature[4 + rLength + 1];
    rOffset = (rLength == 33 ? 1 : 0);
    sOffset = (sLength == 33 ? 1 : 0);
    os_memmove(transactionContext->signature, signature + 4 + rOffset, 32);
    os_memmove(transactionContext->signature + 32, signature + 4 + rLength + 2 + sOffset, 32);
    transactionContext->signature[64] = 0x00;
    if (info & CX_ECCINFO_PARITY_ODD) {
        transactionContext->signature[64] |= 0x01;
    }
    transactionContext->signatureLength = 65;

    return;

}
const unsigned char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void array_hexstr(char *strbuf, const void *bin, unsigned int len) {
    while (len--) {
        *strbuf++ = hex_digits[((*((char *)bin)) >> 4) & 0xF];
        *strbuf++ = hex_digits[(*((char *)bin)) & 0xF];
        bin = (const void *)((unsigned int)bin + 1);
    }
    *strbuf = 0; // EOS
}
