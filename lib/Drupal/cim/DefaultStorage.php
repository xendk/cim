<?php

/**
 * @file
 * Definition of Drupal\cim\DefaultStorage.
 */

namespace Drupal\cim;

/**
 * Defines the default cim storage controller.
 */
class DefaultStorage implements StorageInterface {

  /**
   * Constructs a storage object.
   *
   * @param string $directory
   *   The filesystem directory to use for storage.
   */
  public function __construct($crypt, $directory) {
    $this->crypt = $crypt;
    $this->dir = $directory;
  }

  /**
   * Read file contents from storage.
   *
   * @param string $file_name
   *   Name of the file.
   *
   * @return mixed
   *   File contents.
   */
  public function read($filename) {
     if (file_exists($this->dir . '/' . $filename)) {
      // @todo temporary compatibility.
      $data = file_get_contents($this->dir . '/' . $filename);
      $unserialized = unserialize($data);
      return $unserialized === FALSE ? $data : $unserialized;
    }
    return NULL;
  }

  /**
   * Write file content to storage.
   *
   * @param string $filename
   *   The file to write.
   * @param mixed $data
   *   Data to write.
   */
  public function write($data, $filename = NULL) {
    $data = serialize($data);
    if (!$filename) {
      $filename = hash('sha256', $data);
    }
    file_put_contents($filename, $data);
  }

  public function readSecure($filename) {
    $data = $this->read($filename);
    if ($data && ($data = $this->crypt->open($data)) && $data = unserialize($data)) {
      return $data;
    }
    return FALSE;
  }

  public function writeSecure($data, $filename) {
    $data = serialize($data);
    if (!$filename) {
      $filename = hash('sha256', $data);
    }
    return $this->write($this->crypt->seal($data), $filename);
  }
  /**
   * @todo These are currently not doing what they're saying, but we're keeping
   * them around so the callers can make their intent clear, should we later go
   * for crypting/signing the storage. This will be refactored before the 1.0
   * release.
   */
  public function readSigned($file_name) {
    return $this->read($file_name);
  }

  public function writeSigned($data, $file_name = NULL) {
    return $this->write($data, $file_name);
  }

  public function readCrypted($file_name) {
    return $this->read($file_name);
  }

  public function writeCrypted($data, $file_name = NULL) {
    return $this->write($data, $file_name);
  }
}
