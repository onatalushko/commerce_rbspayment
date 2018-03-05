<?php

/**
 * @param object $order
 */
function commerce_rbspayment_checkout_response_success($order) {
  $resulted_rbs_order_id = isset($_GET['orderId']) ? $_GET['orderId'] : '';

  $redirect_url = '/cart';
  $transaction = $order ? _commerce_rbspayment_get_order_rbs_transaction($order, $resulted_rbs_order_id) : NULL;
  if (!$transaction) {
    drupal_set_message(t('Inconsistent order data.'), 'error');
  }
  else {
    $response = _commerce_rbspayment_api()->getOrderStatusByRBSOrderId($resulted_rbs_order_id);
    if (!isset($response['errorCode']) || $response['errorCode'] == 0) {
      _commerce_rbspayment_set_transaction_status($transaction, $response['orderStatus']);
      _commerce_rbspayment_save_revisioned_payment_transaction($transaction, t('Success callback url invoked from RBS payment gate.'));

      if ($response['orderStatus'] == CommerceRBSPaymentAPI::orderStatusAuthorized) {
        commerce_payment_redirect_pane_next_page($order);

        drupal_set_message(t('Payment successful'), 'status');
        $redirect_url = commerce_checkout_order_uri($order);
      }
      else {
        $message = t('Order payment is not confirmed. !message Please contact us to check if payment was successful.',
          array('!message' => $response['actionCodeDescription']));
        // Workarounding commerce_payment losing messages on redirect.
        $order->data['commerce_rbspayment']['last_error_message'] = $message;
        _commerce_rbspayment_save_revisioned_order($order, $message);

        drupal_set_message($message, 'error');
        $redirect_url = _commerce_rbspayment_get_checkout_payment_back_url($order);
      }
    }
  }

  _commerce_rbspayment_do_back_redirect($redirect_url);
}

/**
 * @param string $redirect_url
 */
function _commerce_rbspayment_do_back_redirect($redirect_url) {
  $redirect_url_options = array();

  $suffix = variable_get_value('commerce_rbspayment_back_url_query');
  if (!empty($suffix)) {
    $suffix_options = drupal_parse_url("/home/{$suffix}");
    $redirect_url_options['query'] = $suffix_options['query'];
  }

  drupal_goto($redirect_url, $redirect_url_options);
}

/**
 * This callback url is only to be used when "Back to shop" link is exposed at RBS payment gate.
 *
 * @param object $order
 */
function commerce_rbspayment_checkout_response_fail($order) {
  global $user;
  $pending_transactions = $order ? _commerce_rbspayment_get_order_rbs_transactions($order, COMMERCE_PAYMENT_STATUS_PENDING) : NULL;
  if ($user->uid != $order->uid || empty($pending_transactions)) {
    drupal_set_message(t('Inconsistent order data.'), 'error');
    $redirect_url = '/cart';
  }
  else {
    // Workarounding commerce_payment losing messages on redirect.
    $order->data['commerce_rbspayment']['last_error_message'] = t('Payment canceled');
    _commerce_rbspayment_save_revisioned_order($order, t('Payment cancelled by user'));

    drupal_set_message(t('Payment canceled'), 'error');
    $redirect_url = _commerce_rbspayment_get_checkout_payment_back_url($order);
  }

  _commerce_rbspayment_do_back_redirect($redirect_url);
}

/**
 * Redirects user after trey returns from payment gate.
 * 
 * @param object $order
 */
function commerce_rbspayment_payment_back($order) {
  drupal_goto(_commerce_rbspayment_get_checkout_payment_back_url($order));
}

/**
 * @param array $form
 * @param array $form_state
 * @param object $user
 * @param object $order
 *
 * @return array
 */
function commerce_rbspayment_order_add_payment_form($form, &$form_state, $user, $order) {
  $form_state['order'] = $order;

  $balance = commerce_payment_order_balance($order);
  if ($balance['amount'] <= 0) {
    drupal_set_message(t('Order does not require additional payments'), 'info');
    drupal_goto(_commerce_rbspayment_get_add_payment_user_redirect_url($order));
  }

  $form_state['balance'] = $balance;

  $back_url = url("/user/{$user->uid}/orders/{$order->order_id}/rbspayment_payment_added", array('absolute' => TRUE));
  $fail_url = url("/user/{$user->uid}/orders/{$order->order_id}/rbspayment_payment_failed", array('absolute' => TRUE));
  $form = _commerce_rbspayment_payment_redirect_form($back_url, $fail_url);

  $message = t("Add %amount payment to your order", array(
    '%amount' => commerce_currency_format($balance['amount'], $balance['currency_code']),
  ));
  $form['message'] = array(
    '#markup' => "<div class='rbspayment-add-payment-message'>{$message}</div>",
    '#weight' => -10,
  );
  $form['#submit'] = ['commerce_rbspayment_order_add_payment_form_submit'];

  return $form;
}

/**
 * @param array $form
 * @param array $form_state
 */
function commerce_rbspayment_order_add_payment_form_submit($form, &$form_state) {
  $order = $form_state['order'];
  $rbs_terminal_url = _commerce_rbspayment_register_rbs_transaction($order, $form_state['balance']['amount'], $form_state['balance']['currency_code'],
    $form_state['values']['back_url'], $form_state['values']['fail_url']);

  if ($rbs_terminal_url){
    drupal_goto($rbs_terminal_url);
  }
  else {
    drupal_goto(_commerce_rbspayment_get_add_payment_user_redirect_url($order));
  }
}

/**
 * Processes order after payment has been processed.
 *
 * Updates order debit transaction and redirects user to the next page.
 * 
 * @param object $user
 * @param object $order
 */
function commerce_rbspayment_add_payment_response_success($user, $order) {
  $resulted_rbs_order_id = isset($_GET['orderId']) ? $_GET['orderId'] : '';

  $transaction = $order ? _commerce_rbspayment_get_order_rbs_transaction($order, $resulted_rbs_order_id) : NULL;
  if (!$transaction) {
    drupal_set_message(t('Inconsistent order data.'), 'error');
  }
  else {
    $response = _commerce_rbspayment_api()->getOrderStatusByRBSOrderId($resulted_rbs_order_id);
    if (!isset($response['errorCode']) || $response['errorCode'] == 0) {
      _commerce_rbspayment_set_transaction_status($transaction, $response['orderStatus']);
      _commerce_rbspayment_save_revisioned_payment_transaction($transaction, t('Success callback url invoked from RBS payment gate.'));

      if ($response['orderStatus'] == CommerceRBSPaymentAPI::orderStatusAuthorized) {
        drupal_set_message(t('Payment successful'), 'status');
      }
      else {
        $message = t('Order payment is not confirmed. !message Please contact us to check if payment was successful.',
          array('!message' => $response['actionCodeDescription']));
        // Workarounding commerce_payment losing messages on redirect.
        $order->data['commerce_rbspayment']['last_error_message'] = $message;
        _commerce_rbspayment_save_revisioned_order($order, $message);

        drupal_set_message($message, 'error');
      }
    }
  }

  $unfinished_statuses_transitions = commerce_rbspayment_unfinished_payments_statuses();
  _commerce_rbspayment_unfinished_order_status_change($order, $transaction->remote_status, $unfinished_statuses_transitions);

  drupal_goto(_commerce_rbspayment_get_add_payment_user_redirect_url($order));
}

/**
 * This callback url is only to be used when "Back to shop" link is exposed at RBS payment gate.
 *
 * @param object $user
 * @param object $order
 */
function commerce_rbspayment_add_payment_response_fail($user, $order) {
  global $user;
  $pending_transactions = $order ? _commerce_rbspayment_get_order_rbs_transactions($order, COMMERCE_PAYMENT_STATUS_PENDING) : NULL;
  if ($user->uid != $order->uid || empty($pending_transactions)) {
    drupal_set_message(t('Inconsistent order data.'), 'error');
  }
  else {
    // Workarounding commerce_payment losing messages on redirect.
    $order->data['commerce_rbspayment']['last_error_message'] = t('Payment canceled');
    _commerce_rbspayment_save_revisioned_order($order, t('Payment cancelled by user'));

    drupal_set_message(t('Payment canceled'), 'error');

    drupal_set_message(t('Payment canceled'), 'error');
  }

  drupal_goto(_commerce_rbspayment_get_add_payment_user_redirect_url($order));
}

/**
 * If current user is not the owner of the order we gonna redirect her to the front page
 * but not to the private area.
 *
 * @param object $order
 *
 * @return string
 */
function _commerce_rbspayment_get_add_payment_user_redirect_url($order) {
  global $user;
  return $user->uid == $order->uid ? "/user/{$order->uid}/orders/{$order->order_id}" : '<front>';
}
