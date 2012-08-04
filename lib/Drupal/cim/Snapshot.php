<?php

/**
 * @file
 * Class representing a configuration snapshot.
 */

namespace Drupal\cim;
use \Serializable;

class Snapshot implements Serializable {
  protected $parent_sha;
  protected $changeset;
  protected $changeset_sha;
  protected $dump;
  protected $dump_sha;
  protected $created;
  /* protected $uid; */
  protected $message;

  public function __construct($message, $parent_sha, Changeset $changeset, ConfigInterface $config) {
    $this->message = $message;
    $this->parent_sha = $parent_sha;
    $this->changeset = $changeset;
    $this->changeset_sha = hash('sha256', serialize($changeset));
    $dump = $config->toArray();
    $this->dump = $dump;
    $this->dump_sha = hash('sha256', serialize($dump));
    $this->created = REQUEST_TIME;
  }

  /**
   *
   */
  function save() {
    // @todo: should let dump and changeset save themselves.
    $storage = cim_get_storage();
    $storage->writeSecure($this->changeset);
    if ($this->dump) {
      $storage->writeSecure($this->dump);
    }
    $storage->writeSecure($this);
  }

  function serialize() {
    return serialize(array($this->message, $this->parent_sha, $this->created, $this->changeset_sha, $this->dump_sha));
  }

  function unserialize($blob) {
    list($this->message, $this->parent_sha, $this->created, $this->changeset_sha, $this->dump_sha) = unserialize($blob);
  }

  function parent_sha() {
    return $this->parent_sha;
  }

  function parent() {
    if (!empty($this->parent_sha)) {
      return cim_get_controller()->load($this->parent_sha);
    }
    return FALSE;
  }

  function message() {
    return $this->message;
  }

  function created() {
    return $this->created;
  }

  function changeset_sha() {
    return $this->changeset_sha;
  }

  function changeset() {
    if (empty($this->changeset)) {
      $this->changeset = cim_get_storage()->readSecure($this->changeset_sha);
    }
    return $this->changeset;
  }

  function dump_sha() {
    return $this->dump_sha;
  }

  function dump() {
    if ($this->dump = cim_get_storage()->readSecure($this->dump_sha)) {
      return $this->dump;
    }
    else {
      if (empty($this->parent_sha)) {
        return;
      }
      // Get the parent snapshot dump and apply our changes.
      $parent = cim_get_storage()->readSecure($this->parent_sha);
      if (empty($this->changeset)) {
        $this->changeset = cim_get_storage()->readSecure($this->changeset_sha);
      }
      return $this->changeset->apply($parent->dump());
    }
  }

  function sha() {
    return hash('sha256', serialize($this));
  }
}
