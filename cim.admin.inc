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
  $peer_file = cim_directory() . '/' . hash('sha256', $form_state['cim_peer_data']['public_key']);

  $data = array(
    'name' => $form_state['values']['downstream_name'],
    'uid' => $user->uid,
    'public_key' => $form_state['cim_peer_data']['public_key'],
  );
  ksort($data);
  $data['signature'] = $crypt->sign($data);
  file_put_contents($peer_file, serialize($data));

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

function cim_admin_add_upstream_form($form, &$form_state) {
  // @todo this could do with a cleanup.
  if (!empty($_GET['token']) || !empty($_GET['sig'])) {
    $token = $_GET['token'];
    $signature = $_GET['sig'];
    $signature = base64_decode($signature);

    // Not as paranoid here as on upstream.
    if ($cache = cache()->get('cim_auth_downstream:' . $token)) {
      cache()->delete('cim_auth_downstream:' . $token);
      $crypt = new Crypt(cim_keys(), $cache->data['public_key']);
      if (!$crypt->verify($token, $signature)) {
        $form['message'] = array(
          '#markup' => t('Signature did not verify.'),
        );
        return $form;
      }
      // Verified, save it.
      $peer_id = hash('sha256', $cache->data['public_key']);
      $peer_file = cim_directory() . '/' . $peer_id;

      $data = array(
        'name' => $cache->data['upstream_name'],
        'public_key' => $cache->data['public_key'],
        'url' => $cache->data['upstream_url'],
      );
      $data['signature'] = $crypt->sign($data);
      file_put_contents($peer_file, serialize($data));
      // Really, there's no reason we shouldn't be able to support multiple
      // upstreams, but let's keep it simple to start with.
      file_put_contents(cim_directory() . '/upstream', $peer_id);
    }
    else {
      $form['message'] = array(
        '#markup' => t('Unknown token, it could be timed out?'),
      );
      return $form;
    }
  }

  $name = '';
  $url = '';
  if (file_exists(cim_directory() . '/upstream')) {
    $peer_id = file_get_contents(cim_directory() . '/upstream');
    if ($peer = cim_peer($peer_id)) {
      $name = $peer['name'];
      $url = $peer['url'];
    }
  }

  $form['upstream_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name for upstream.'),
    '#default_value' => $name,
  );

  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('Upstream url'),
    '#default_value' => $url,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    // @todo better name.
    '#value' => t('Authenticate at upstream'),
  );

  return $form;
}

function cim_admin_add_upstream_form_validate($form, &$form_state) {

  // Make sure that there's a peer directory we can use.
  // @todo: remove, deprecated.
  $directory = cim_directory();
  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY)) {
    form_error($form, t("Error creating peers directory %directory", array('%directory' => $directory)));
  };

  $crypt = new Crypt(cim_keys());

  $request = array(
    'action' => 'authenticate',
    'core' => 8,
    'version' => 1,
    'public_key' => $crypt->getPublicKey(),
    'return_url' => url('admin/config/development/cim/upstream', array('absolute' => TRUE)),
  );
  ksort($request);
  $request['signature'] = $crypt->sign(serialize($request));
  $auth_response = cim_request($form_state['values']['url'], $request);

  if ($auth_response['status'] == 'OK') {
    if (empty($auth_response['response']) || !($res = unserialize(base64_decode($auth_response['response'])))) {
      form_error($form['url'], t('Could not decode upstream response.'));
    }
    $res = unserialize($crypt->open($res['data'], $res['envelope']));
    // Extra sanity check.
    if (!empty($res['token']) && !empty($res['public_key']) && !empty($res['authenticate_url'])) {
      $res['upstream_name'] = $form_state['values']['upstream_name'];
      $res['upstream_url'] = $form_state['values']['url'];
      // Cache the response for later.
      cache()->set('cim_auth_downstream:' . $res['token'], $res, REQUEST_TIME + 3600);
      $form_state['cim_auth_res'] = $res;
    }
    else {
      // Include the utility drupal_var_export() function.
      include_once './core/includes/utility.inc';
      watchdog('cim', 'Malformed authentication response: @response', array(drupal_var_export($res)), WATCHDOG_ERROR);
      form_error($form['url'], t('Malformed upstream response.'));
    }
  }
  else {
    if (!empty($auth_response['message'])) {
      $message = t('Upstream reported an error: @message', array('@message' => $auth_response['message']));
    }
    else {
      $message = t("Upstream encountered an error, but didn't provide an error message");
    }
    form_error($form['url'], $message);
  }
}

function cim_admin_add_upstream_form_submit($form, &$form_state) {
  if ($auth_res = $form_state['cim_auth_res']) {
    $crypt = new Crypt(cim_keys());
    $form_state['redirect'] = array(
      $auth_res['authenticate_url'],
      array(
        'query' => array(
          'token' => $auth_res['token'],
          'sig' => base64_encode($crypt->sign($auth_res['token'])),
        ),
      ),
    );
  }
}

function cim_admin_pull_confirm_form($form, &$form_state) {
  $request = array(
    'action' => 'pull',
  );
  $response = cim_peer_request($request);
  $upstream = cim_upstream();
  // @todo: Way too much magic handwaving in these 2 lines...
  $crypt = new Crypt(cim_keys(), $upstream['public_key']);
  $response = $crypt->decrypt(base64_decode($response['response']));

  if (!empty($response['changeset'])) {
    // Create a ConfigArray corresponding to the upstream config.
    $upstream_conf = new ConfigArray(array());
    $response['changeset']->apply($upstream_conf);

    // Create a changeset we can use by taking the difference between our config
    // and upstreams.
    $config = new ConfigDrupalConfig();
    $new_changeset = Changeset::fromDiff($config, $upstream_conf);
  }

  if (!empty($new_changeset)) {
    $form['intro'] = array(
      '#markup' => t('Pull the following configuration changes?'),
    );

    $form['message'] = array(
      '#type' => 'textfield',
      '#title' => t('Message'),
      '#default_value' => 'Pulled from ' . $upstream['name'] . '.',
      '#maxlength' => 255,
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

    return confirm_form($form, 'Pull configuration?', 'admin/config/development/cim');
  }
  else {
    $form['message'] = array(
      '#markup' => t('Configuration is in sync with upstream.'),
    );
    return $form;
  }
}

function cim_admin_pull_confirm_form_submit($form, &$form_state) {
  cim_get_controller()->apply($form_state['values']['new_changeset'], $form_state['values']['message']);
  drupal_set_message('Configuration pulled from upstream.');
}

/**
 * Menu callback; Testing page.
 *
 * @todo not for release.
 */
function cim_testing() {
  /* dpm(cim_get_controller()->save('Test initial') ? 'saved' : 'nothing to save'); */
  /* $config = config('system.performance'); */
  /* for ($i = 1; $i < 5; $i++) { */
  /*   $config->set('cache', $i); */
  /*   $config->save(); */
  /*   dpm(cim_get_controller()->save('Set caching to ' . $i) ? 'saved' : 'nothing to save'); */
  /* } */

  $c1 = new Crypt();
  $c2 = new Crypt();
  $c1->keyGen(); $c2->keyGen();
  $c1->setPeerKey($c2->getPublicKey());
  $c2->setPeerKey($c1->getPublicKey());
  /* $c1->peer_public_key = $c2->public_key; */
  /* $c2->peer_public_key = $c1->public_key; */

  $message = $c1->encrypt(array('testdata' => 'this is a test'));
  dpm($c2->decrypt($message));

  $message2 = $c2->encrypt('this is another test');
  dpm($c1->decrypt($message2));

  /* $crypt->test(); */
  return "";
}
