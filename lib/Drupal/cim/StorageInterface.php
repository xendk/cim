<?php

/**
 * @file
 * Definition of Drupal\cim\StorageInterface.
 */

namespace Drupal\cim;

/**
 * Defines an interface for storing cim configuration history.
 *
 * @todo Originally the plan was to use encrypted/signed files, as to satisfy
 * those who objected against the core configuration management system storing
 * configuration in plaintext on the server. However, things have been going
 * back and forth, so we concentrate on getting other things working instead,
 * until the dust settles.
 */
interface StorageInterface {
  // @todo Flesh out with what have been proven to work in DefaultStorage.
}
