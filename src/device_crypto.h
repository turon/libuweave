// Copyright 2015 The Weave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIBUWEAVE_SRC_DEVICE_CRYPTO_H_
#define LIBUWEAVE_SRC_DEVICE_CRYPTO_H_

#include <stdbool.h>
#include <stdint.h>

#include "src/crypto_spake.h"
#include "src/macaroon.h"
#include "uweave/status.h"

#define UW_DEVICE_CRYPTO_BUFFER_LEN 128

#define UW_DEVICE_CRYPTO_DEVICE_ID_LEN 4

#define UW_DEVICE_CRYPTO_MACAROON_KEY_LEN UW_MACAROON_MAC_LEN

#define UW_DEVICE_CRYPTO_KEY_DEVICE_AUTH_KEY 1
#define UW_DEVICE_CRYPTO_KEY_CLIENT_AUTHZ_KEY 2
#define UW_DEVICE_CRYPTO_KEY_DEVICE_ID 3

typedef struct {
  // The device_id is unique per-pairing, and is reset when the device is
  // claimed.
  bool has_device_id;
  uint8_t device_id[UW_DEVICE_CRYPTO_DEVICE_ID_LEN];

  // To identify the device.
  bool has_device_auth_key;
  uint8_t device_authentication_key[UW_DEVICE_CRYPTO_MACAROON_KEY_LEN];

  // To authorize the client.
  bool has_client_authz_key;
  uint8_t client_authorization_key[UW_DEVICE_CRYPTO_MACAROON_KEY_LEN];

  // The key generated by the pairing process to bootstrap into /auth and
  // proceed with /accessControl.
  uint64_t ephemeral_issue_timestamp;
  uint8_t ephemeral_pairing_key[UW_SPAKE_P224_POINT_SIZE];

  // The new client_authz key is held in memory until it has been
  // correctly acknowledged by the client.  This is to ensure that communication
  // errors do not render the device inoperable without repeating pairing
  bool has_pending_client_authz_key;
  uint8_t pending_client_authorization_key[UW_DEVICE_CRYPTO_MACAROON_KEY_LEN];
} UwDeviceCrypto;

/**
 * Initializes the device into the base state.  Sets and saves the device
 * authentication key if unset.
 */
UwStatus uw_device_crypto_init_(UwDeviceCrypto* device_crypto);

/**
 * Reset the crypto state.
 */
void uw_device_crypto_reset_(UwDeviceCrypto* device_crypto);

/**
 * Records the pairing key computed in /pairing/confirm in memory for future
 * authentication. Sets a timestamp to enable expiration.
 */
UwStatus uw_device_crypto_remember_pairing_key_(UwDeviceCrypto* device_crypto,
                                                uint8_t* pairing_key,
                                                size_t pairing_key_len,
                                                uint64_t timestamp);

/**
 * Generates a new client authorization key without overriding the existing one
 * for use in the /accessControl/claim flow.
 */
UwStatus uw_device_crypto_generate_pending_client_authz_key_(
    UwDeviceCrypto* device_crypto,
    uint8_t* key_data);

/**
 * Commits the new client authorization key overriding the existing one.  This
 * invalidates all existing client tokens.  For use in the
 * /accessControl/confirm flow.
 */
UwStatus uw_device_crypto_commit_pending_client_authz_key_(
    UwDeviceCrypto* device_crypto);

#endif  // LIBUWEAVE_SRC_DEVICE_CRYPTO_H_