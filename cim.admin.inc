<?php

/**
 * @file
 * Admin page callbacks for the cim module.
 */

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
