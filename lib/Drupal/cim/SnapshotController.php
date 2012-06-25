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
  protected $crypt;
  /**
   * The number of changesets between saving a full dump of configuration.
   */
  const FULL_DUMP_FREQUENCY = 3;

  public function __construct($crypt) {
    $this->crypt = $crypt;
  }

  /**
   * Save a snapshot.
   */
  public function save(Snapshot $snapshot) {
    // @todo: We should *really* have some way of making sure this was
    // successful.
    $snapshot->save($this);
    $head_file = cim_directory() . '/head';
    $sha = file_put_contents($head_file, $snapshot->ssc_sha);
    return $snapshot;
  }

  public function writeBlob($data) {
    if (isset($data->ssc_sha)) {
      unset($data->ssc_sha);
    }
    $blob = serialize($data);
    $sha = hash('sha256', $blob);
    $filename = cim_directory() . '/' . $sha;
    if (is_object($data)) {
      $data->ssc_sha = $sha;
    }
    return file_put_contents($filename, serialize($this->crypt->seal($blob)));
  }

  public function sha($data) {
    if (isset($data->ssc_sha)) {
      return $data->ssc_sha;
    }
    $blob = serialize($data);
    $sha = hash('sha256', $blob);
    if (is_object($data)) {
      // Cache sha on objects.
      $data->ssc_sha = $sha;
    }
    return $sha;
  }

  public function readBlob($sha) {
    $filename = cim_directory() . '/' . $sha;
    if (!file_exists($filename)) {
      return FALSE;
    }
    if ($blob = file_get_contents($filename)) {
      if (($blob = unserialize($blob)) && ($blob = $this->crypt->open($blob['data'], $blob['envelope']))) {
        return unserialize($blob);
      }
    }
    return FALSE;
  }

  /**
   * Create snapshot from current configuration changes.
   */
  public function create($message = '') {
    $config = new ConfigDrupalConfig();
    $last_snapshot = $this->latest();
    if ($last_snapshot) {
      $parent_sha = $this->sha($last_snapshot);
      $prevdump = $last_snapshot->dump($this);
    }
    else {
      $parent_sha = NULL;
      $prevdump = array();
    }
    $changeset = Changeset::fromDiff($prevdump, $config);
    if ($changeset) {
      $snapshot = new Snapshot($message, $this, $parent_sha, $changeset, $config);
      return $snapshot;
    }
    return FALSE;
  }

  /**
   * Applies the given changeset and creates a new snapshot.
   */
  public function apply(Changeset $changeset, $message = '') {
    $config = new ConfigDrupalConfig();
    if ($changeset->appliesTo($config)) {
      $changeset->apply($config);
      $config->commit();
      $snapshot = $this->create($message);
      return $this->save($snapshot);
    }
    else {
      throw new \Exception('Changeset doesn\'t apply');
    }
  }

  /**
   * Load a snapshot.
   */
  public function load($sha) {
    $snapshot = $this->readBlob($sha);
    if (!$snapshot) {
      // Who's been messing?
      print_r(debug_backtrace());
      die(); // todo: get rid of this, obviously.
    }
    return $snapshot;
  }

  function revert(Snapshot $snapshot) {
    $parent_snapshot = $snapshot->parent();
    return Changeset::fromDiff($snapshot->dump(), $parent_snapshot->dump());
  }

  function rollback(Snapshot $snapshot) {
    $latest_snapshot = $this->latest();
    return Changeset::fromDiff($latest_snapshot->dump(), $snapshot->dump());
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
      $row = $row->parent();
      if (!$row) {
        break;
      }
      $result[] = $row;
    }
    return $result;
  }

  /**
   * Get the latest Snapshot.
   */
  public function latest() {
    $head_file = cim_directory() . '/head';
    if (file_exists($head_file)) {
      $sha = file_get_contents($head_file);
      return $this->load($sha);
    }
    return FALSE;
  }

  /**
   * Get a config dump from a changeset.
   *
   * Finds the last full dump and applies the following changesets, up to and
   * including the supplied changset.
   */
  public function latestDump($snapshot) {
    if (empty($snapshot->dump)) {
      $last_full = $this->loadCid($snapshot->previous_dump);

      if (empty($last_full)) {
        throw new \Exception('CIM: Broken history, could not find base dump.');
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
          throw new \Exception('CIM: Broken history.');
        }
        $current_snapshot = $snapshots[$current_snapshot->changeset_sha];
        $dump = $current_snapshot->changeset->apply($dump);
        $depth++;
      } while ($current_snapshot->cid != $snapshot->cid);

      return array($depth, $dump);
    }
    return array(0, new ConfigArray($snapshot->dump));
  }
}
