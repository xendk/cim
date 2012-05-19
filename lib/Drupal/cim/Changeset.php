<?php

/**
 * @file
 * Class for changesets.
 */

namespace Drupal\cim;

/**
 * Represents a set of changes to a configuration.
 */
class Changeset {
  /**
   * Parent snapshot.
   */
  protected $parent = "";

  /**
   * Array of Differences.
   */
  protected $changes;

  /**
   * Create a changeset from the difference between two snapshots.
   *
   * @return Changeset|NULL
   *   changeset or NULL if no changes detected.
   */
  public static function fromDiff($a, $b, $parent = "") {
    $changes = Changeset::diff($a, $b);
    if ($changes) {
      $changeset = new Changeset();
      $changeset->parent = $parent;
      $changeset->changes = $changes;
      return $changeset;
    }
    return NULL;
  }

  /**
   * Compute the difference between two snapshots.
   */
  public static function diff($a, $b, $prefix = array()) {
    $diff = array();
    $str_prefix = join('.', $prefix);
    foreach (array('a', 'b') as $var) {
      if (!is_array(${$var})) {
        if (${$var} instanceof ConfigInterface) {
          ${$var} = ${$var}->toArray();
        }
        else {
          throw new Exception("Argument to diff not an array or instance of ConfigInterface.");
        }
      }
    }
    $common_keys = array_intersect_key($a, $b);
    foreach ($common_keys as $key => $dummy) {
      if (is_array($a[$key]) && is_array($b[$key])) {
        $diff = array_merge($diff, Changeset::diff($a[$key], $b[$key], array_merge($prefix, array($key))));
      }
      else {
        if ($a[$key] !== $b[$key]) {
          $diff[] = new Difference(array_merge($prefix, array($key)), $a[$key], $b[$key]);
        }
      }
    }

    $a_keys = array_diff_key($a, $common_keys);
    foreach ($a_keys as $key => $value) {
      $diff[] = new Difference(array_merge($prefix, array($key)), $value, Difference::NULL);
    }

    $b_keys = array_diff_key($b, $common_keys);
    foreach ($b_keys as $key => $value) {
      $diff[] = new Difference(array_merge($prefix, array($key)), Difference::NULL, $value);
    }

    return array_filter($diff);
  }

  /**
   * Returns the parent of this changeset.
   */
  public function parent() {
    return $this->parent;
  }

  /**
   * Returns array of changes.
   */
  public function changes() {
    return $this->changes;
  }

  /**
   * Test if this changeset will apply to the given snapshot.
   */
  public function appliesTo(ConfigInterface $snapshot) {
    foreach ($this->changes as $change) {
      if (!$change->appliesTo($snapshot)) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Applies changeset to snapshot.
   */
  public function apply(ConfigInterface $snapshot) {
    foreach ($this->changes as $change) {
      if (!($snapshot = $change->apply($snapshot))) {
        return FALSE;
      }
    }
    return $snapshot;
  }

  /**
   * Return the sha for this changeset.
   */
  public function sha() {
    $string = $this->parent;
    foreach ($this->changes as $change) {
      $string .= serialize($change);
    }
    return hash('sha256', $string);
  }

  /**
   * Count the additions/changes/removals of changeset.
   */
  public function diff3Stat() {
    $total = array(0, 0, 0);
    foreach ($this->changes as $change) {
      $stat = $change->diff3Stat();
      $total[0] += $stat[0];
      $total[1] += $stat[1];
      $total[2] += $stat[2];
    }
    return $total;
  }
}
