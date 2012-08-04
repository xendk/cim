<?php

/**
 * @file
 * Class abstracting an peer.
 */

namespace Drupal\cim;

/**
 * Represents a peer and takes care of storage of data.
 */
class Peer {
  protected $name;
  protected $uid;
  protected $url;
  protected $publicKey;

  /**
   * Load a peer.
   */
  public static function load($id) {
    if ($peer = cim_get_storage()->readSigned($id)) {
      return $peer;
    }
    return FALSE;
  }

  public function save() {
    cim_get_storage()->writeSigned($this, hash('sha256', $this->publicKey));
  }

  public function __construct($name, $public_key, $uid = NULL, $url = NULL) {
    $this->publicKey = $public_key;
    $this->name = $name;
    $this->uid = $uid;
    $this->url = $url;
  }

  public function id() {
    return hash('sha256', $this->publicKey);
  }

  public function publicKey() {
    return $this->publicKey;
  }

  public function name() {
    return $this->name;
  }

  public function url() {
    return $this->url;
  }
}
