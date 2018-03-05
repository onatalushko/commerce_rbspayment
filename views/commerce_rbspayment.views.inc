<?php

/**
 * Implements hook_views_data().
 */
function commerce_rbspayment_views_data() {
  $data = array();

  $data['commerce_payment_transaction']['refund_rbspayment_transaction'] = array(
    'field' => array(
      'title' => t('RBS Refund link'),
      'help' => t('Provide a simple link to refund RBS payment transaction.'),
      'handler' => 'commerce_rbspayment_handler_field_rbspayment_transaction_link_refund',
    ),
  );

  return $data;
}
