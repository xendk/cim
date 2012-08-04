<?php

/**
 * @file
 * Admin page callbacks for the cim module.
 */

use Drupal\cim\Difference;
use Drupal\cim\Changeset;
use Drupal\cim\ConfigArray;
use Drupal\cim\ConfigDrupalConfig;
use Drupal\cim\Crypt;
use Drupal\cim\Peer;

/**
 * Menu callback; Snapshot list page.
 */
function cim_admin_list($sha = NULL) {
  $build = array();
  // Pseudo paging by asking for one too many.
  $snapshots = cim_get_controller()->listing(21, $sha);
  $next_page = FALSE;

  if (sizeof($snapshots) > 20) {
    $next_page = TRUE;
    // Ignore the extra.
    array_pop($snapshots);
  }

  $rows = array();
  $header = array(t('Created'), t('Id'), t('+/X/-'));
  foreach ($snapshots as $snapshot) {
    $diff_stat = $snapshot->changeset()->diff3Stat();
    $diff_stat = $diff_stat[0] . '/' . $diff_stat[1] . '/' . $diff_stat[2];
    $rows[] = array(
      format_date($snapshot->created(), 'short'),
      theme('cim_admin_snapshot_overview', array('sha' => $snapshot->sha(), 'link' => 'admin/config/development/cim/snapshot/' . $snapshot->sha(), 'message' => $snapshot->message())),
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
    $last = end($snapshots);
    $build['next'] = array(
      '#markup' => l(t('Older'), 'admin/config/development/cim/list/' . $last->changeset_sha),
    );
  }
  return $build;
}

/**
 * Form callback; create snapshot.
 */
function cim_admin_create_snapshot_form($form, &$form_state) {
  // Include the utility drupal_var_export() function.
  include_once './core/includes/utility.inc';
  $snapshot = cim_get_controller()->create();

  if ($snapshot) {
    $form['intro'] = array(
      '#markup' => t('The new snapshot will consist of the changes below.'),
    );

    $form['message'] = array(
      '#type' => 'textfield',
      '#title' => t('Message'),
      '#default_value' => 'Manual snapshot without a message.',
      '#maxlength' => 255,
    );

    $form['changeset'] = array(
      '#type' => 'cim_changeset_view',
      '#changeset' => $snapshot->changeset(),
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
  $ssc = cim_get_controller();
  $snapshot = $ssc->create($form_state['values']['message']);
  $ssc->save($snapshot);
  $form_state['redirect'] = 'admin/config/development/cim';
  drupal_set_message(t('Snapshot "@message" saved.', array('@message' => $form_state['values']['message'])));
}

/**
 * Form callback; revert snapshot.
 */
function cim_admin_revert_snapshot_form($form, &$form_state, $snapshot) {
  // Include the utility drupal_var_export() function.
  include_once './core/includes/utility.inc';

  $config = new ConfigDrupalConfig();
  $changeset = cim_get_controller()->revert($snapshot);
  if ($changeset && $changeset->appliesTo($config)) {
    $form['changeset'] = array(
      '#type' => 'value',
      '#value' => $changeset,
    );

    $form['snapshot'] = array(
      '#type' => 'value',
      '#value' => $snapshot,
    );


    $form['intro'] = array(
      '#markup' => t('This will make the following changes.'),
    );

    $form['changeset_view'] = array(
      '#type' => 'cim_changeset_view',
      '#changeset' => $changeset,
    );

    return confirm_form($form, 'Revert snapshot?', 'admin/config/development/cim');
  }
  else {
    // @todo Conflict resolution.
    $form['message'] = array(
      '#markup' => t('Cannot revert this snapshot, conflicting changes.'),
    );

    return $form;
  }
}

/**
 * Submit handler.
 *
 * Reverts snapshot.
 */
function cim_admin_revert_snapshot_form_submit($form, &$form_state) {
  $snapshot = $form_state['values']['snapshot'];
  $message = 'Reverts "' . $snapshot->message() . '" (' . $snapshot->sha() . ')';
  cim_get_controller()->apply($form_state['values']['changeset'], $message);
  drupal_set_message(t('Snapshot reverted.'));
  $form_state['redirect'] = 'admin/config/development/cim';
}

/**
 * Form callback; Rollback snapshot.
 */
function cim_admin_rollback_snapshot_form($form, &$form_state, $snapshot) {
  // Include the utility drupal_var_export() function.
  include_once './core/includes/utility.inc';

  $config = new ConfigDrupalConfig();
  $changeset = cim_get_controller()->rollback($snapshot);
  if (!$changeset) {
    $form['message'] = array(
      '#markup' => t('No changes needed to roll back this snapshot.'),
    );

    return $form;
  }
  if ($changeset && $changeset->appliesTo($config)) {
    $form['changeset'] = array(
      '#type' => 'value',
      '#value' => $changeset,
    );

    $form['snapshot'] = array(
      '#type' => 'value',
      '#value' => $snapshot,
    );

    $form['intro'] = array(
      '#markup' => t('This will make the following changes.'),
    );

    $form['changes'] = array(
      '#type' => 'cim_changeset_view',
      '#changeset' => $changeset,
    );

    return confirm_form($form, 'Rollback snapshot?', 'admin/config/development/cim');
  }
  else {
    // @todo Conflict resolution.
    $form['message'] = array(
      '#markup' => t('Cannot rollback this snapshot, local changes conflicts.'),
    );

    return $form;
  }
}


/**
 * Submit handler.
 *
 * Reverts snapshot.
 */
function cim_admin_rollback_snapshot_form_submit($form, &$form_state) {
  $snapshot = $form_state['values']['snapshot'];
  $message = 'Rollback to "' . $snapshot->message() . '" (' . $snapshot->sha . ')';
  cim_get_controller()->apply($form_state['values']['changeset'], $message);
  drupal_set_message(t('Changes rolled back.'));
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

  $build['meta']['message'] = array(
    '#type' => 'item',
    '#title' => t('Message'),
    '#markup' => check_plain($snapshot->message()),
  );

  $build['meta']['changeset_sha'] = array(
    '#type' => 'item',
    '#title' => 'Changeset SHA',
    '#markup' => check_plain($snapshot->changeset_sha()),
  );

  $build['meta']['snapshot_parent'] = array(
    '#type' => 'item',
    '#title' => 'Parent snapshot SHA',
    '#markup' => $snapshot->parent_sha() ? check_plain($snapshot->parent_sha()) : t('(none, first snapshot)'),
  );

  $build['meta']['created'] = array(
    '#type' => 'item',
    '#title' => 'Creation time',
    '#markup' => format_date($snapshot->created()),
  );

  /* if (!empty($snapshot->dump)) { */
  /*   $build['meta']['dump'] = array( */
  /*     '#type' => 'markup', */
  /*     '#markup' => t('Includes full config dump.'), */
  /*   ); */
  /* } */

  $build['changeset'] = array(
    '#type' => 'cim_changeset_view',
    '#changeset' => $snapshot->changeset(),
  );

  return $build;
}

function cim_admin_authenticate_form($form, &$form_state) {
  global $user;

  if (empty($_GET['token']) || empty($_GET['sig'])) {
    $form['message'] = array(
      '#markup' => t('Missing parameters. Possibly incompatible version of CIM?'),
    );
    return $form;
  }
  $token = $_GET['token'];
  $signature = $_GET['sig'];
  $signature = base64_decode($signature);

  // We ensure that we're only run once by deleting the cache entry right away,
  // using locking to guard against race conditions.
  while (!lock_acquire('cim_admin_authenticate')) {
    if (lock_wait('cim_admin_authenticate', 15)) {
      $form['message'] = array(
        '#markup' => t('Could not acquire lock. Refresh to retry.'),
      );
      return $form;
    }
  }
  if ($cache = cache()->get('cim_auth_upstream:' . $token)) {
    cache()->delete('cim_auth_upstream:' . $token);
    lock_release('cim_admin_authenticate');
    $crypt = new Crypt(cim_keys(), $cache->data['public_key']);
    if (!$crypt->verify($token, $signature)) {
      $form['message'] = array(
        '#markup' => t('Signature did not verify.'),
      );
      return $form;
    }
    $form_state['cache'] = TRUE;
    $form_state['cim_peer_data'] = $cache->data;
    $form_state['cim_peer_data']['token'] = $token;
  }
  lock_release('cim_admin_authenticate');


  $name = $_SERVER['HTTP_HOST'];
  drupal_set_title(t("CIM authentication at @name", array('@name' => $name)));

  // Show a banner so it is obvious where the user are.
  $form['intro'] = array(
    '#markup' => '<p>' . t('This is @name', array('@name' => $name)) . '</p>',
  );

  if ($user->uid < 1) {
    // No user logged in, throw them past the login form.
    drupal_goto('user/login', array('query' => drupal_get_destination()));
  }
  else {
    // Check for permissions.
    if (user_access('administer snapshots', $user)) {
      $form['you'] = array(
        '#markup' => '<p>' . t('You are logged in as @name.', array('@name' => user_format_name($user))) . '</p>',
      );
    }
  }

  $form['downstream_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name for downstream'),
    '#description' => t('Provide a name that @name should know downstream by.', array('@name' => $name)),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 *
 */
function cim_admin_authenticate_form_validate($form, &$form_state) {
  // @todo check that $form_state['values']['downstream_name'] haven't been used.
  $directory = cim_directory();
  // @todo: remove
  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY)) {
    form_error($form, t("Error creating peers directory %directory", array('%directory' => $directory)));
  };
}

/**
 * Form submit.
 * Save peer data.
 */
function cim_admin_authenticate_form_submit($form, &$form_state) {
  global $user;
  $crypt = new Crypt(cim_keys());

  if (empty($form_state['cim_peer_data'])) {
    drupal_set_message(t('Token not active. Please retry authentication from the downstream.'), 'error');
    return;
  }

  $peer = new Peer($form_state['values']['downstream_name'], $form_state['cim_peer_data']['public_key'], $user->uid);
  $peer->save();

  $form_state['redirect'] = array(
    $form_state['cim_peer_data']['return_url'],
    array(
      'query' => array(
        'token' => $form_state['cim_peer_data']['token'],
        'sig' => base64_encode($crypt->sign($form_state['cim_peer_data']['token'])),
      ),
    ),
  );
}

function cim_admin_push_confirm_form($form, &$form_state, $snapshot) {
  $error = NULL;
  // Get the diff between the last pull and the given snapshot.
  $last = cim_get_storage()->readSigned('upstream_latest');
  if (empty($last)) {
    $error = t('Cannot push before pulling.');
  }
  else {
    if (!$last_pulled = cim_get_controller()->load($last['local'])) {
      $error = t('Cannot load local snapshot. Please pull to resync.');
    }
  }

  if ($error) {
    $form['error'] = array(
      '#markup' => $error,
    );
    return $form;
  }
  else {
    $new_changeset = Changeset::fromDiff($last_pulled->dump(), $snapshot->dump());

    if ($new_changeset) {
      $form['intro'] = array(
        '#markup' => t('Push the following configuration changes?'),
      );

      $upstream = cim_upstream();
      $form['message'] = array(
        '#type' => 'textfield',
        '#title' => t('Message'),
        '#default_value' => 'No message.',
        '#maxlength' => 255,
      );

      $form['upstream_sha'] = array(
        '#type' => 'value',
        '#value' => $last['upstream'],
      );

      $form['local_sha'] = array(
        '#type' => 'value',
        '#value' => $snapshot->sha(),
      );
      $form['changeset'] = array(
        '#type' => 'cim_changeset_view',
        '#changeset' => $new_changeset,
      );

      // @todo: Could the changeset element above not be an #input element?
      $form['new_changeset'] = array(
        '#type' => 'value',
        '#value' => $new_changeset,
      );

      return confirm_form($form, 'Push configuration?', 'admin/config/development/cim');
    }
    else {
      $form['message'] = array(
        '#markup' => t('Configuration is in sync with upstream.'),
      );
      return $form;
    }
  }
}

function cim_admin_push_confirm_form_submit($form, &$form_state) {
  $request = array(
    'action' => 'push',
    'base_sha' => $form_state['values']['upstream_sha'],
    'changeset' => $form_state['values']['new_changeset'],
    'message' => $form_state['values']['message'],
  );
  $response = cim_peer_request($request);

  if ($response['success'] && !empty($response['latest_sha']) && $response['latest_sha'] != $form_state['values']['upstream_sha']) {
    drupal_set_message('Configuration pushed to upstream. ');
    $data = array(
      'upstream' => $response['latest_sha'],
      'local' => $form_state['values']['local_sha'],
    );

    cim_get_storage()->writeSigned($data, 'upstream_latest');
    $form_state['redirect'] = 'admin/config/development/cim';
  }
  else {
    if ($response['message']) {
      drupal_set_message(t('Upstream rejected, message: "@message".', array('@message' => $response['message'])), 'error');
    }
    else {
      drupal_set_message(t('An error occurred, please check upstream.'), 'error');
    }
  }
}
