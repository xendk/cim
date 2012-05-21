<?php

/**
 * @file
 * Configuration array class.
 */

namespace Drupal\cim;

/**
 * Implements ConfigInterface using an internal array for storage.
 */
class ConfigArray implements ConfigInterface {
  /**
   * The configuration.
   */
  protected $config = array();

  /**
   * Constructor. Takes an array in the common format.
   */
  function __construct($array) {
    if (!is_array($array)) {
      throw new \Exception('Non-array passed to ConfigArray::__construct.');
    }
    $this->config = $array;
  }

  /**
   * Implements ConfigInterface::exists().
   */
  function exists($path) {
    return drupal_array_nested_key_exists($this->config, $path);
  }

  /**
   * Implements ConfigInterface::get().
   */
  function get($path) {
    return drupal_array_get_nested_value($this->config, $path);
  }

  /**
   * Implements ConfigInterface::set().
   */
  function set($path, $value) {
    drupal_array_set_nested_value($this->config, $path, $value);
  }

  /**
   * Implements ConfigInterface::clear().
   */
  function clear($path) {
    $name = array_pop($path);
    $ref = &$this->config;
    foreach ($path as $parent) {
      $ref = &$ref[$parent];
    }
    unset($ref[$name]);
  }

  /**
   * Implements ConfigInterface::toArray().
   */
  function toArray() {
    return $this->config;
  }
}
