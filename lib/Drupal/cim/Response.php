<?php

/**
 * @file
 * Simple class for handling responses.
 */

namespace Drupal\cim;

/**
 * Simple class to eliminate a lot of arrays.
 */
class Response {
  public $status;
  public $message;
  public $response;

  public static function error($message) {
    $respone = new self();
    $respone->status = 'ERR';
    $respone->message = $message;
    return $respone;
  }

  public static function success($result) {
    $respone = new self();
    $respone->status = 'OK';
    $respone->response = base64_encode($result);
    return $respone;
  }
}
