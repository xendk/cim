<?php

/**
 * @file
 * Upstream administrative pages.
 */

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
      $peer = new Peer($cache->data['upstream_name'], $cache->data['public_key'], NULL, $cache->data['upstream_url']);
      $peer->save();
      // Really, there's no reason we shouldn't be able to support multiple
      // upstreams, but let's keep it simple to start with.
      cim_get_storage()->write($peer->id(), 'upstream');
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
  if (($peer_id = cim_get_storage()->read('upstream')) &&
    ($peer = Peer::load($peer_id))) {
      $name = $peer->name();
      $url = $peer->url();
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
   error_log($crypt->open(unserialize(base64_decode($auth_response))));
  if (!($auth_response = base64_decode($auth_response)) ||
    !($auth_response = unserialize($auth_response)) ||
    !($auth_response = $crypt->open($auth_response)) ||
    !($auth_response = unserialize($auth_response))) {
    form_error($form['url'], t('Could not decode response.'));
    return;
  }

  if ($auth_response['status'] == 'OK') {
    // Extra sanity check.
    if (!empty($auth_response['token']) && !empty($auth_response['public_key']) && !empty($auth_response['authenticate_url'])) {
      $auth_response['upstream_name'] = $form_state['values']['upstream_name'];
      $auth_response['upstream_url'] = $form_state['values']['url'];
      // Cache the response for later.
      cache()->set('cim_auth_downstream:' . $auth_response['token'], $auth_response, REQUEST_TIME + 3600);
      $form_state['cim_auth_res'] = $auth_response;
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
  $latest = cim_get_controller()->latest();

  if ($response['success'] && !empty($response['snapshot']) && ($response['snapshot']->dump_sha() != $latest->dump_sha())) {
    $blob = cim_peer_request(array('action' => 'blob', 'sha' => $response['snapshot']->dump_sha()));

    if ($blob['success']) {
      // Create a changeset we can use by taking the difference between our config
      // and upstreams.
      $config = new ConfigDrupalConfig();
      $new_changeset = Changeset::fromDiff($config, $blob['blob']);
    }
  }

  if (!empty($new_changeset)) {
    $form['intro'] = array(
      '#markup' => t('Pull the following configuration changes?'),
    );

    $upstream = cim_upstream();
    $form['message'] = array(
      '#type' => 'textfield',
      '#title' => t('Message'),
      '#default_value' => 'Pulled from ' . $upstream->name() . '.',
      '#maxlength' => 255,
    );

    $form['upstream_sha'] = array(
      '#type' => 'value',
      '#value' => $response['snapshot']->sha(),
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
    if ($response['success']) {
      $form['message'] = array(
        '#markup' => t('Configuration is in sync with upstream.'),
      );
    }
    else {
      if ($response['message']) {
        $message = t('Upstream reported an error: "@message"', array('@message' => $response['message']));
      }
      else {
        drupal_set_message(t('Upstream reported an error, but gave no error message'), 'error');
      }
      drupal_set_message($message, 'error');
      $form['message'] = array(
        '#markup' => t('Error fetching.'),
      );
    }
    return $form;
  }
}

function cim_admin_pull_confirm_form_submit($form, &$form_state) {
  $snapshot = cim_get_controller()->apply($form_state['values']['new_changeset'], $form_state['values']['message']);

  $data = array(
    'upstream' => $form_state['values']['upstream_sha'],
    'local' => $snapshot->sha(),
  );

  cim_get_storage()->writeSigned($data, 'upstream_latest');
  drupal_set_message('Configuration pulled from upstream.');
}
