<?php

/**
 * @file
 * Class for handling the crypto/signing magic.
 */

namespace Drupal\cim;

/**
 * Simple proof of concept class for encryption. Will be rewritten to a more
 * general framework.
 */
class Crypt {
  protected $private_key;
  protected $public_key;
  protected $peer_public_key;

  public function __construct($key_pair = null, $peer_key = NULL) {
    if ($key_pair) {
      $this->private_key = $key_pair[0];
      $this->public_key = $key_pair[1];
    }
    if ($peer_key) {
      $this->peer_public_key = $peer_key;
    }
  }

  function getKeyPair() {
    return array(
      $this->private_key,
      $this->public_key,
    );
  }

  /**
   * Creates a new key pair for this object.
   */
  public function keyGen() {
    $key_pair = openssl_pkey_new();
    openssl_pkey_export($key_pair, $this->private_key);
    $pubkey = openssl_pkey_get_details($key_pair);
    $this->public_key = $pubkey['key'];
  }

  public function getPublicKey() {
    return $this->public_key;
  }

  /**
   * Set the public key of the peer.
   */
  public function setPeerKey($key) {
    $this->peer_public_key = $key;
  }

  /**
   * Sign a message and encrypt it with peer public key.
   */
  public function encrypt($data) {
    if (!($signature = $this->sign($data))) {
      return FALSE;
    }

    $signed_data = serialize(array(
                     'data' => $data,
                     'signature' => $signature,
                   ));
    if (!($res = $this->seal($signed_data))) {
      return FALSE;
    }
    return serialize($res);
  }

  /**
   * Decrypt a message and check the signature.
   */
  public function decrypt($data) {
    if (!($crypted_data = unserialize($data))) {
      return FALSE;
    }
    if (!($signed_data = $this->open($crypted_data)) || !($signed_data = unserialize($signed_data))) {
      return FALSE;
    }
    if (!$this->verify($signed_data['data'], $signed_data['signature'])) {
      return FALSE;
    }
    return $signed_data['data'];
  }

  /**
   * Sign a message.
   */
  public function sign($data) {
    if (!openssl_sign(serialize($data), $signature, $this->private_key)) {
      return FALSE;
    }
    return $signature;
  }

  /**
   * Verify a signature.
   */
  public function verify($data, $signature) {
    if (openssl_verify(serialize($data), $signature, $this->peer_public_key) != 1) {
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Encrypt a message with peer public key.
   */
  public function seal($data) {
    if (!openssl_seal($data, $crypted_data, $envelope, array($this->peer_public_key))) {
      return FALSE;
    }
    return array(
      'data' => $crypted_data,
      'envelope' => $envelope[0],
    );
  }

  /**
   * Decrypt a message.
   */
  public function open($data) {
    if (!openssl_open($data['data'], $uncrypted_data, $data['envelope'], $this->private_key)) {
      return FALSE;
    }

    return $uncrypted_data;
  }
}
