<?php

/**
 * @file
 * Lists module's rules' actions.
 */

/**
 * Implements hook_rules_action_info().
 */
function commerce_rbspayment_rules_action_info() {
  $actions = array();

  $actions['commerce_rbspayment_refund_order'] = array(
    'label' => t('Refunds order paid by card via RBS payment'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
        'skip save' => TRUE,
      ),
    ),
    'group' => t('RBS payment'),
    'callbacks' => array(
      'execute' => 'commerce_rbspayment_refund_order_action',
    ),
  );

  return $actions;
}

/**
 * Implements rules order refund action.
 *
 * @param object $order
 * @param array $settings
 * @param RulesState $state
 * @param RulesPlugin $element
 */
function commerce_rbspayment_refund_order_action($order, $settings, RulesState $state, RulesPlugin $element) {
  $remote_reversible_statuses = array(CommerceRBSPaymentAPI::orderStatusAuthorized, CommerceRBSPaymentAPI::orderStatusPartlyRefunded);
  $reversible_transactions = _commerce_rbspayment_get_order_rbs_transactions($order, COMMERCE_PAYMENT_STATUS_SUCCESS, $remote_reversible_statuses);
  array_map('commerce_rbspayment_transaction_reverse', $reversible_transactions);
}

