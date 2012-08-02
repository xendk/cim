<?php

/**
 * @file
 * Implements ConfigInterface for the core config system.
 */

namespace Drupal\cim;

/**
 * ConfigInterface to the config system.
 */
class ConfigDrupalConfig implements ConfigInterface {
  /**
   * Array of DrupalConfig objects.
   */
  protected $configs = array();

  /**
   * Constructor.
   */
  function __construct() {
    $cnf = config_get_storage_names_with_prefix();
    foreach ($cnf as $key) {
      $this->configs[$key] = config($key);
    }
  }

  /**
   * Implements ConfigInterface::exists().
   */
  function exists($path) {
    $name = array_shift($path);
    if (!isset($this->configs[$name])) {
      return FALSE;
    }

    return $this->configs[$name]->get(join('.', $path)) !== NULL ? TRUE : FALSE;
  }

  /**
   * Implements ConfigInterface::get().
   */
  function get($path) {
    $name = array_shift($path);
    if (!isset($this->configs[$name])) {
      return NULL;
    }

    return $this->configs[$name]->get(join('.', $path));
  }

  /**
   * Implements ConfigInterface::set().
   */
  function set($path, $value) {
    $name = array_shift($path);
    if (!isset($this->configs[$name])) {
      throw new Exception(t('Unknow config group %name', array('%name' => $name)));
    }
    return $this->configs[$name]->set(join('.', $path), $value);
  }

  /**
   * Implements ConfigInterface::clear().
   */
  function clear($path) {
    $name = array_shift($path);
    if (!isset($this->configs[$name])) {
      return;
    }

    $this->configs[$name]->clear(join('.', $path));
  }

  /**
   * Commits changes to the config system.
   */
  function commit() {
    foreach ($this->configs as $name => $config) {
      $config->save();
    }
  }

  /**
   * Implements ConfigInterface::toArray().
   */
  function toArray() {
    $cnf = config_get_storage_names_with_prefix();
    $config = array();
    foreach ($this->configs as $key => $conf) {
      $config[$key] = $conf->get();
    }
    return $config;
  }
}
