<?php

/**
 * @file
 * Class representing a configuration snapshot.
 */

namespace Drupal\cim;

class Snapshot {
  // @todo these properties should be protected, but we have code depending on
  // them that needs other ways to get the required information then.
  // One being drupal_write_record. There seems to be no recommended way of
  // implementing objects with private properties.
  public $cid;
  public $changeset_sha;
  public $changeset_parent;
  public $changeset;
  // @todo drupal_write_record likes to serialize NULL instead of storing NULL. Figure out why.
  public $dump;
  public $previous_dump;
  public $created;
  public $uid;
  public $message;

  public function __construct($message = '') {
    global $user;
    // If these properties is already set, we were loaded from database.
    if (!empty($this->changeset)) {
      $this->changeset = unserialize($this->changeset);
    }
    if (!empty($this->dump)) {
      $this->dump = unserialize($this->dump);
    }
    if (empty($this->created)) {
      $this->created = REQUEST_TIME;
    }
    if (!isset($this->uid)) {
      $this->uid = $user->uid;
    }
    if (!isset($this->message)) {
      $this->message = $message;
    }
  }

  public function setChangeset($changeset) {
    $this->changeset = $changeset;
    $this->changeset_sha = $changeset->sha();
    $this->changeset_parent = $changeset->parent();
    if ($this->changeset_parent && empty($this->previous_dump)) {
      $previous_snapshot = cim_get_controller()->load($this->changeset_parent);
      if (!empty($previous_snapshot->dump)) {
        $this->previous_dump = $previous_snapshot->cid;
      }
      else {
        $this->previous_dump = $previous_snapshot->previous_dump;
      }
    }
  }

  public function setDump($dump) {
    $this->dump = $dump;
  }
}
