<?php

/**
 * @file
 * Common interface for configuration.
 */

namespace Drupal\cim;

/**
 * Interface for interacting with configuration.
 */
interface ConfigInterface {
  /**
   * Checks if a given config path exists.
   */
  function exists($path);

  /**
   * Get the value of a config item.
   */
  function get($path);

  /**
   * Set the value of a config item.
   */
  function set($path, $value);

  /**
   * Clear (unset) a config item.
   */
  function clear($path);

  /**
   * Convert to the common array format.
   */
  function toArray();

  /* function commit(); */
}
