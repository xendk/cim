<?php

/**
 * @file
 * Controller class for Snapshots.
 */

namespace Drupal\cim;

/**
 * Manages snapshots.
 */
class SnapshotController {
  /**
   * The number of changesets between saving a full dump of configuration.
   */
  const FULL_DUMP_FREQUENCY = 3;

  /**
   * Take a snapshot.
   */
  public function save() {
    $snapshot = $this->create();
    if ($snapshot) {
      drupal_write_record('cim', $snapshot);
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Create changeset from current configuration changes.
   */
  public function create($message = '') {
    $fulldump = FALSE;
    $config = new ConfigDrupalConfig();
    $latest_snapshot = $this->latest();
    if ($latest_snapshot) {
      list($depth, $previous_dump) = $this->latestDump($latest_snapshot);
      $changeset = Changeset::fromDiff($previous_dump, $config, $latest_snapshot->changeset_sha);
      if ($depth >= self::FULL_DUMP_FREQUENCY) {
        $fulldump = TRUE;
      }
    }
    else {
      $fulldump = TRUE;
      $changeset = Changeset::fromDiff(array(), $config);
    }
    if ($changeset) {
      $snapshot = new Snapshot($message);
      if ($fulldump) {
        $snapshot->setDump($config->toArray());
      }
      $snapshot->setChangeset($changeset);
      return $snapshot;
    }
    return FALSE;
  }

  public function loadCid($cid) {
    return db_select('cim', 'c', array('fetch' => 'Drupal\cim\Snapshot'))
      ->fields('c')
      ->condition('cid', $cid)
      ->execute()->fetch();
  }

  /**
   * Load a snapshot.
   */
  public function load($sha) {
    return db_select('cim', 'c', array('fetch' => 'Drupal\cim\Snapshot'))
      ->fields('c')
      ->condition('changeset_sha', $sha)
      ->execute()->fetch();
  }

  /**
   * List snapshots in reverse chronological order.
   *
   * @param int $num
   *   Number of changesets to list.
   * @param string $start
   *   SHA of the changeset to start from.
   */
  public function listing($num = 10, $start = NULL) {
    $result = array();
    if ($start) {
      $row = $this->load($start);
    }
    else {
      $row = $this->latest();
    }
    if (!$row) {
      return $result;
    }

    $result[] = $row;
    while (sizeof($result) < $num) {
      $row = $this->load($row->changeset_parent);
      if (!$row) {
        break;
      }
      $result[] = $row;
    }
    return $result;
  }

  /**
   * Get the latest changeset.
   */
  public function latest() {
    return db_select('cim', 'c', array('fetch' => 'Drupal\cim\Snapshot'))
      ->fields('c')
      ->orderBy('created', 'DESC')
      ->orderBy('cid', 'DESC')
      ->range(0, 1)
      ->execute()->fetch();
  }

  /**
   * Get a config dump from a changeset.
   *
   * Finds the last full dump and applies the following changesets, up to the
   * supplied changset.
   */
  public function latestDump($snapshot) {
    if (empty($snapshot->dump)) {
      $last_full = $this->loadCid($snapshot->previous_dump);

      if (empty($last_full)) {
        throw new Exception('CIM: Broken history, could not find base dump.');
      }

      // Load snapshots between last full dump and current.
      $snapshots = db_select('cim', 'c', array('fetch' => 'Drupal\cim\Snapshot'))
        ->fields('c')
        ->condition('previous_dump', $last_full->cid)
        ->execute()->fetchAllAssoc('changeset_parent');


      $depth = 0;
      $dump = new ConfigArray($last_full->dump);
      $current_snapshot = $last_full;
      do {
        if (!isset($snapshots[$current_snapshot->changeset_sha])) {
          throw new Exception('CIM: Broken history.');
        }
        $current_snapshot = $snapshots[$current_snapshot->changeset_sha];
        $dump = $current_snapshot->changeset->apply($dump);
        $depth++;
      } while ($current_snapshot->cid != $snapshot->cid);

      return array($depth, $dump);
    }
    return array(0, $snapshot->dump);
  }
}
