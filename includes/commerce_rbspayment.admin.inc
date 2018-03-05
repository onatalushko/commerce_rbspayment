<?php

/**
 * @file
 * Contains admin area menu callbacks and forms.
 */

/**
 * Form that asks user confirmation for order refund.
 *
 * @param array $form
 * @param array $form_state
 * @param object $transaction
 *
 * @return array
 */
function commerce_rbspayment_transaction_refund_form($form, &$form_state, $transaction) {
  $form_state['transaction'] = $transaction;

  $form['#submit'][] = 'commerce_rbspayment_transaction_refund_form_submit';

  $form = confirm_form($form,
    t('Are you sure you want to refund this transaction?'),
    "admin/commerce/orders/{$transaction->order_id}/payment",
    '<p>' . t('@amount paid via RBS payment on @date.', array(
      '@amount' => commerce_currency_format($transaction->amount, $transaction->currency_code),
      '@date' => format_date($transaction->created, 'short')
    )) . '</p>',
    t('Refund'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Implements hook_FORM_ID_form_submit().
 */
function commerce_rbspayment_transaction_refund_form_submit($form, &$form_state) {
  $result = commerce_rbspayment_transaction_reverse($form_state['transaction']);
  if ($result) {
    drupal_set_message(t('Transaction refunded successfully'));
    $form_state['redirect'] = "admin/commerce/orders/{$form_state['transaction']->order_id}/payment";
  }
}

/**
 * Page that shows RBS extended transaction info.
 *
 * @param object $order
 * @param object $transaction
 *
 * @return array
 */
function commerce_rbspayment_transaction_info_page($order, $transaction) {
  return _commerce_rbspayment_transaction_info($transaction->remote_id);
}

/**
 * @param string $rbs_order_id
 *
 * @return array
 */
function _commerce_rbspayment_transaction_info($rbs_order_id) {
  $response = _commerce_rbspayment_api()->getOrderStatusByRBSOrderId($rbs_order_id);
  if ($response['errorCode']) {
    drupal_set_message(t('Error trying to get transaction info: !message', array(
      '!message' => $response['errorMessage']
    )), 'error');

    return array();
  }

  $build['merchant'] = _commerce_rbspayment_get_partial_info_table($response, 'merchantOrderParams',
    t('Merchant order parameters'), FALSE, 10);
  $build['cardAuth'] = _commerce_rbspayment_get_partial_info_table($response, 'cardAuthInfo',
    t('Card authorization info'), TRUE ,20);
  $build['paymentAmount'] = _commerce_rbspayment_get_partial_info_table($response, 'paymentAmountInfo',
    t('Payment amount info'), TRUE, 30);
  $build['bank'] = _commerce_rbspayment_get_partial_info_table($response, 'bankInfo',
    t('Bank info'), TRUE, 40);
  $build['bankInfo'] = _commerce_rbspayment_get_partial_info_table($response, 'attributes',
    t('Attributes'), FALSE, 50);

  $build['info'] = array(
    '#theme' => 'table',
    '#caption' => t('Basic info'),
    '#rows' => _commerce_rbspayment_convert_associative_to_rows($response),
    '#weight' => 0,
  );

  return $build;
}

/**
 * @param array $transaction_info
 * @param string $part_id
 * @param string $part_title
 * @param boolean $do_convert
 * @param int $weight
 *
 * @return array
 */
function _commerce_rbspayment_get_partial_info_table(array &$transaction_info, $part_id, $part_title, $do_convert, $weight) {
  $transaction_part_info = isset($transaction_info[$part_id]) ? $transaction_info[$part_id] : array();
  $build = array(
    '#theme' => 'table',
    '#caption' => $part_title,
    '#rows' => $do_convert ? _commerce_rbspayment_convert_associative_to_rows($transaction_part_info) : $transaction_part_info,
    '#weight' => $weight,
  );
  unset($transaction_info[$part_id]);

  return $build;
}

/**
 * Displays list of transactions for the given date (uses today as default date).
 *
 * @param array $form
 * @param array $form_state
 * @param string $date
 *
 * @return array
 */
function commerce_rbspayment_date_operations($form, $form_state, $date = '') {
  $date = isset($form_state['values']['date']) ? $form_state['values']['date'] : $date;
  if ($date === '') {
    $date = date('Y-m-d');
  }

  $form['filter'] = array(
    '#type' => 'container',
  );
  if (module_exists('date_popup')) {
    $form['filter']['date'] = array(
      '#type' => 'date_popup',
      '#title' => t('Transactions date'),
      '#default_value' => $date,
      '#date_type' => DATE_DATETIME,
      '#date_timezone' => date_default_timezone(),
      '#date_format' => 'd-m-Y',
      '#date_increment' => 1,
      '#date_year_range' => '-3:+1',
      '#date_label_position' => 'invisible',
      '#ajax' => array(
        'callback' => 'commerce_rbspayment_transactions_filter_form_submit',
        'wrapper' => 'operations-wrapper',
        'method' => 'replace',
      ),
    );
  }

  $form['filter']['state'] = array(
    '#type' => 'select',
    '#title' => t('Transactions states'),
    '#options' => array(
      '' => t('All'),
      'CREATED' => t('Created'),
      'APPROVED' => t('Approved'),
      'DEPOSITED' => t('Deposited'),
      'DECLINED' => t('Declined'),
      'REVERSED' => t('Reversed'),
      'REFUNDED' => t('Refunded'),
    ),
    '#default_value' => '',
    '#ajax' => array(
      'callback' => 'commerce_rbspayment_transactions_filter_form_submit',
      'wrapper' => 'operations-wrapper',
      'method' => 'replace',
    ),
  );

  $form['operations'] = array(
    '#prefix' => '<div id="operations-wrapper">',
    '#suffix' => '</div>',
  );
  if ($date > date('Y-m-d')) {
    drupal_set_message(t('Future date transactions inquiries are not supported.'), 'warning');
    return $form;
  }

  $state = isset($form_state['values']['state']) ? $form_state['values']['state'] : '';
  $operations_table = _commerce_rbspayment_get_operations_table($state, $date);
  $form['operations']['#markup'] = $operations_table;

  $form['#attached']['css'][] = drupal_get_path('module', 'commerce_rbspayment') .
    '/css/admin-operations-table.css';

  return $form;
}

/**
 * @param int $state
 * @param string $date
 *
 * @return string
 */
function _commerce_rbspayment_get_operations_table($state, $date) {
  $states = $state != '' ? array($state) : array();
  $date_time = DateTime::createFromFormat('Y-m-d', $date);
  if (!$date_time) {
    drupal_set_message(t('Incorrect date format'), 'error');
    return '';
  }

  $response = _commerce_rbspayment_get_operations_list_by_date($date_time, $states);

  if (!isset($response['totalCount']) || $response['totalCount'] == 0 || !isset($response['orderStatuses'])) {
    $message = array(
      '#theme_wrappers' => array('container'),
      '#attributes' => array('class' => array('messages', 'status')),
      '#markup' => t('No transactions found for selected filter values.'),
    );
    return drupal_render($message);
  }

  $bank_operations = $response['orderStatuses'];

  $rows = array_map(function ($operation) use ($date) {
    $operation['statusDescription'] = CommerceRBSPaymentAPI::$statusDescriptions[$operation['orderStatus']];
    $order_id = _commerce_rbspayment_trim_order_id_suffix($operation['orderNumber']);
    $order_payment_admin_link = "/admin/commerce/orders/{$order_id}/payment";
    $rbs_order_id = _commerce_rbspayment_get_rbs_order_id_attribute($operation['attributes']);
    $order_rbs_info_admin_link = "/admin/commerce/config/payment-methods/rbspayment/operations/{$date}/{$rbs_order_id}";
    return array(
      "<a href='{$order_payment_admin_link}'>{$order_id}</a>",
      date('Y-m-d H:i:s', substr($operation['date'], 0, 10)),
      commerce_currency_format($operation['amount'], _commerce_rbspayment_get_currency_code_by_iso($operation['currency'])),
      $operation['errorCode'],
      $operation['orderStatus'],
      $operation['statusDescription'],
      isset($operation['cardAuthInfo']) ? "{$operation['cardAuthInfo']['pan']} ({$operation['cardAuthInfo']['expiration']})" : '',
      "<a href='{$order_rbs_info_admin_link}'>".t('view').'</a>',
    );
  }, $bank_operations);

  $header = array(
    t('Order Id'),
    t('Time'),
    t('Sum'),
    t('Error code'),
    t('Status code'),
    t('Status'),
    t('Extra'),
    t('RBS info'),
  );
  $caption = t('Transactions for selected date: !date', array('!date' => $date));

  return theme('table', array(
    'rows' => $rows,
    'header' => $header,
    'caption' => $caption,
  ));
}

/**
 * @param array $attributes
 *
 * @return string
 */
function _commerce_rbspayment_get_rbs_order_id_attribute(array $attributes) {
  foreach ($attributes as $attribute) {
    if ($attribute['name'] == 'mdOrder') {
      return $attribute['value'];
    }
  }

  return '';
}

/**
 * @param DateTime $date_time
 * @param array $states
 *
 * @return array
 */
function _commerce_rbspayment_get_operations_list_by_date(DateTime $date_time, array $states) {
  $response = _commerce_rbspayment_api()->getOperationsList(date('Ymd', $date_time->getTimestamp()), $states);

  if (isset($response['errorCode']) && $response['errorCode'] != 0) {
    $error_message = t('Error occurred during communication with bank');
    if (isset($response['errorCode']) && $response['errorCode'] != 0 && isset($response['errorMessage'])) {
      $error_message = $response['errorMessage'];
    }
    drupal_set_message($error_message, 'error');
  }

  return $response;
}

/**
 * @param string $rbs_order_id
 *
 * @return array
 */
function commerce_rbspayment_detailed_transaction_info($rbs_order_id) {
  return _commerce_rbspayment_transaction_info($rbs_order_id);
}

/**
 * @param string $order_id
 *
 * @return string
 */
function _commerce_rbspayment_trim_order_id_suffix($order_id) {
  $suffix_pos = strpos($order_id, '/');
  return $suffix_pos !== FALSE ? substr($order_id, 0, $suffix_pos) : $order_id;
}

function commerce_rbspayment_transactions_filter_form_submit($form) {
  return $form['operations'];
}