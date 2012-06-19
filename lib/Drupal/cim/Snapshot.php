<?php

/**
 * @file
 * Class representing a configuration snapshot.
 */

namespace Drupal\cim;
use \Serializable;

class Snapshot implements Serializable {
  protected $ssc;
  protected $parent_sha;
  protected $changeset;
  protected $changeset_sha;
  protected $dump;
  protected $dump_sha;
  protected $created;
  /* protected $uid; */
  protected $message;

  public function __construct($message, $ssc, $parent_sha, Changeset $changeset, ConfigInterface $config) {
    $this->ssc = $ssc;
    $this->message = $message;
    $this->parent_sha = $parent_sha;
    $this->changeset = $changeset;
    $this->changeset_sha = hash('sha256', serialize($changeset));
    $dump = $config->toArray();
    $this->dump = $dump;
    $this->dump_sha = hash('sha256', serialize($dump));
    $this->created = REQUEST_TIME;
  }

  public function controller($controller) {
    // todo: really, this isn't dependency injection, but lets let the dust
    // settle on that one first.
    $this->ssc = $controller;
  }

  /**
   *
   */
  function save(SnapshotController $ssc) {
    // @todo: should let dump and changeset save themselves.
    $ssc->writeBlob($this->changeset);
    if ($this->dump) {
      $ssc->writeBlob($this->dump);
    }
    $ssc->writeBlob($this);
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
      return $this->ssc->load($this->parent_sha);
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
      $this->changeset = $this->ssc->readBlob($this->changeset_sha);
    }
    return $this->changeset;
  }

  function dump_sha() {
    return $this->dump_sha;
  }

  function dump() {
    if ($this->dump = $this->ssc->readBlob($this->dump_sha)) {
      return $this->dump;
    }
    else {
      if (empty($this->parent_sha)) {
        return;
      }
      // Get the parent snapshot dump and apply our changes.
      $parent = $this->ssc->load($this->parent_sha);
      if (empty($this->changeset)) {
        $this->changeset = $this->ssc->readBlob($this->changeset_sha);
      }
      return $this->changeset->apply($parent->dump());
    }
  }

  function sha() {
    return $this->ssc->sha($this);
  }
}
