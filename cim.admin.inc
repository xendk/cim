<?php

/**
 * @file
 * Admin page callbacks for the cim module.
 */

/**
 * Menu callback; Snapshot list page.
 */
function cim_admin_list($identifier = NULL) {
  $build = array();
  // Pseudo paging by asking for one too many.
  $changesets = cim_get_controller()->listing(11, $identifier);
  $next_page = FALSE;

  if (sizeof($changesets) > 10) {
    $next_page = TRUE;
    // Ignore the extra.
    array_pop($changesets);
  }

  $rows = array();
  $header = array(t('Num'), t('Id'), t('+/X/-'));
  foreach ($changesets as $changeset_row) {
    $diff_stat = $changeset_row->changeset->diff3Stat();
    $diff_stat = $diff_stat[0] . '/' . $diff_stat[1] . '/' . $diff_stat[2];
    $rows[] = array(
      $changeset_row->cid,
      l($changeset_row->identifier, 'admin/config/development/cim/snapshot/' . $changeset_row->cid),
      $diff_stat,
    );
  }
  $build['listing'] = array(
    '#theme' => 'table',
    '#title' => t('Snapshots'),
    '#header' => $header,
    '#rows' => $rows,
  );
  if ($next_page) {
    $last = end($changesets);
    $build['next'] = array(
      '#markup' => l('Older', 'admin/config/development/cim/list/' . $last->identifier),
    );
  }
  return $build;
}

/**
 * Form callback; create snapshot.
 */
function cim_admin_create_snapshot_form($form, &$form_state) {
  $snapshot = cim_get_controller()->create();

  if ($snapshot) {
    $changes = $snapshot->changeset->changes();
    $rows = array();
    foreach ($changes as $change) {
      $rows[] = array(
        join('.', $change->path),
        $change->a === Difference::NULL ? check_plain('<unset>') : (is_array($change->a) ? '<pre>' . drupal_var_export($change->a) . '</pre>' : check_plain($change->a)),
        $change->b === Difference::NULL ? check_plain('<unset>') : (is_array($change->b) ? '<pre>' . drupal_var_export($change->b) . '</pre>' : check_plain($change->b)),
      );
    }

    $form['intro'] = array(
      '#markup' => t('The new snapshot will consist of the changes below.'),
    );

    $form['changes'] = array(
      '#type' => 'fieldset',
      '#title' => t('Changes'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['changes']['details'] = array(
      '#theme' => 'table',
      '#header' => array(t('Configuration key'), t('From'), t('To')),
      '#rows' => $rows,
    );
    return confirm_form($form, 'Create a new snapshot?', 'admin/config/development/cim');
  }
  else {
    $form['message'] = array(
      '#markup' => t('No configuration changes, not creating a snapshot.'),
    );

    return $form;
  }
}

/**
 * Submit handler.
 *
 * Creates a new snapshot.
 */
function cim_admin_create_snapshot_form_submit($form, &$form_state) {
  cim_get_controller()->save();
  $form_state['redirect'] = 'admin/config/development/cim';
}

/**
 * Menu callback; Show snapshot page.
 *
 * @todo Shared changes display logic.
 */
function cim_admin_snapshot_view($snapshot) {
  // Include the utility drupal_var_export() function.
  include_once './core/includes/utility.inc';
  $build['meta'] = array(
    '#type' => 'fieldset',
    '#title' => t('Snapshot information'),
  );

  $build['meta']['identifier'] = array(
    '#type' => 'item',
    '#title' => 'Changeset id',
    '#markup' => $snapshot->identifier,
  );

  $build['meta']['parent'] = array(
    '#type' => 'item',
    '#title' => 'Parent changeset id',
    '#markup' => $snapshot->parent,
  );

  $build['meta']['created'] = array(
    '#type' => 'item',
    '#title' => 'Creation time',
    '#markup' => format_date($snapshot->created),
  );

  if (!empty($snapshot->dump)){
    $build['meta']['dump'] = array(
      '#type' => 'markup',
      '#markup' => t('Includes full config dump.'),
    );
  }

  $changes = $snapshot->changeset->changes();
  $rows = array();
  foreach ($changes as $change) {
    $rows[] = array(
      join('.', $change->path),
      $change->a === Difference::NULL ? check_plain('<unset>') : (is_array($change->a) ? '<pre>' . drupal_var_export($change->a) . '</pre>' : check_plain($change->a)),
      $change->b === Difference::NULL ? check_plain('<unset>') : (is_array($change->b) ? '<pre>' . drupal_var_export($change->b) . '</pre>' : check_plain($change->b)),
    );
  }

  $build['changes'] = array(
    '#type' => 'fieldset',
    '#title' => t('Changes'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $build['changes']['details'] = array(
    '#theme' => 'table',
    '#header' => array(t('Configuration key'), t('From'), t('To')),
    '#rows' => $rows,
  );

  return $build;
}

/**
 * Menu callback; Testing page.
 *
 * @todo not for release.
 */
function cim_testing() {
  /* $config_snapshot = new Config(); */
  /* $b = $config_snapshot->toArray(); */
  /* $a = cim_take_snapshot(); */
  /* $b = cim_take_snapshot(); */
  /* $b['image.style.thumbnail']['effects'] = array( */
  /*   'image_scale_200_200_1' => array( */
  /*     'name' => 'image_scale', */
  /*     'ieid' => 'image_scale_200_200_1', */
  /*     'data' => array( */
  /*       'width' => '200', */
  /*       'height' => '200', */
  /*       'upscale' => '1', */
  /*     ), */
  /*     'weight' => '0', */
  /*   ), */
  /* ); */
  /* $b['system.performance']['cache'] = 1; */
  /* $b_snapshot = new ArraySnapshot($b); */

  /* $b['system.performance']['cache'] = "5"; */
  /* $c_snapshot = new ArraySnapshot($b); */
  /* /\* dpm(cim_diff($a, $b)); *\/ */
  /* /\* dpm(cim_diff(array(), $b)); *\/ */
  /* /\* dpm($a); *\/ */
  /* $cs = Changeset::fromDiff($config_snapshot->toArray(), $b_snapshot->toArray()) */;
  /* dpm($cs); */
  /* dpm($cs->appliesTo($config_snapshot) ? 'true' : 'false'); */
  /* dpm($cs->appliesTo($b_snapshot) ? 'true' : 'false'); */
  /* dpm($cs->appliesTo($c_snapshot) ? 'true' : 'false'); */

  /* $test_snapshot = new ArraySnapshot($config_snapshot->toArray()); */
  /* $test_snapshot = $config_snapshot; */
  /* dpm($test_snapshot->toArray()); */
  /* dpm($cs); */
  /* $cs->apply($test_snapshot); */
  /* dpm($test_snapshot->toArray()); */

  /* $config = config('system.performance'); */
  /* $config->set('cache', "30"); */
  /* $config_snapshot->configs['system.performance']->set('cache', '30'); */

  /* dpm($cs->appliesTo($config_snapshot) ? 'true' : 'false'); */
  /* $b['system.performance']['cache_lifetime'] = '300'; */
  /* $cs3 = Changeset::fromDiff($a, $b); */
  /* dpm($cs->appliesTo($a) ? 'true' : 'false'); */
  /* dpm($cs->appliesTo($b) ? 'true' : 'false'); */
  /* $c = $cs->apply($a); */
  /* dpm($cs->appliesTo($c) ? 'true' : 'false'); */
  /* dpm($c); */
  /* dpm($b); */
  /* dpm($cs); */
  /* $cs2 = Changeset::fromDiff(array(), $b); */
  /* dpm($cs2->appliesTo($c) ? 'true' : 'false'); */
  /* dpm($cs3->appliesTo($c) ? 'true' : 'false'); */
  /* /\* dpm($cs2); *\/ */
  /* dpm($cs->identifier()); */
  /* dpm($cs2->identifier()); */
  /* dpm($cs3->identifier()); */

  $ssc = new SnapshotController();
  dpm($ssc->save() ? 'saved' : 'nothing to save');
  $config = config('system.performance');
  for ($i = 1; $i < 10; $i++) {
    $config->set('cache', $i);
    $config->save();
    dpm($ssc->save() ? 'saved' : 'nothing to save');
  }
  /* config('system.performance')-> */
  /* dpm($ssc->latest()); */
  return "";
}
