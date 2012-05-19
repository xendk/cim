<?php

/**
 * @file
 * Class for config differences.
 */

namespace Drupal\cim;

/**
 * Represents a single change in a changeset..
 */
class Difference {
  /**
   * Special NULL value so we can distinguish NULL and really non-existing.
   * md5('Difference::EMPTY')
   */
  const NULL = 'f8ce859e75fec78d568b2800b99c9854';

  /**
   * The config path changed.
   */
  public $path;

  /**
   * Existing value.
   */
  public $a;

  /**
   * New value.
   */
  public $b;

  /**
   * Create a new difference.
   */
  function __construct($path, $a, $b) {
    $this->path = $path;
    $this->a = $a;
    $this->b = $b;
  }

  /**
   * Test if difference applies to snapshot.
   */
  public function appliesTo(ConfigInterface $snapshot) {
    if ($this->noopCheck($snapshot)) {
      return TRUE;
    }
    if ($this->a === self::NULL) {
      return !$snapshot->exists($this->path);
    }
    return $snapshot->get($this->path) === $this->a;
  }

  /**
   * Apply difference to snapshot.
   */
  public function apply(ConfigInterface $snapshot) {
    if (!$this->appliesTo($snapshot)) {
      return FALSE;
    }
    if ($this->b === self::NULL) {
      $snapshot->clear($this->path);
    }
    else {
      $snapshot->set($this->path, $this->b);
    }
    return $snapshot;
  }

  /**
   * Count the additions/changes/removals of change.
   */
  public function diff3Stat() {
    $stat = array(0, 0, 0);
    if ($this->a === self::NULL) {
      // Additions.
      $stat[0] = sizeof($this->b);
    }
    elseif ($this->b === self::NULL) {
      // Removals.
      $stat[2] = sizeof($this->a);
    }
    else {
      // Counts as X changes and Y additions/removals.
      $a = sizeof($this->a);
      $b = sizeof($this->b);
      $changes = min($a, $b);
      $diff = max($a, $b) - $changes;
      if ($a > $b) {
        $stat[1] = $diff;
      }
      else {
        $stat[0] = $diff;
      }
      $stat[1] = $changes;
    }
    return $stat;
  }

  /**
   * Test if difference is already implemented.
   */
  protected function noopCheck(ConfigInterface $snapshot) {
    if ($this->b === self::NULL) {
      return !$snapshot->exists($this->path);
    }
    return $snapshot->get($this->path) === $this->b;
  }
}
