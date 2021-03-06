<?php

/**
 * Field handler to present a link to refund a payment transaction.
 */
class commerce_rbspayment_handler_field_rbspayment_transaction_link_refund extends commerce_payment_handler_field_payment_transaction_link {

  public function render($values) {
    // Ensure the user has access to refund this payment transaction.
    $transaction_id = $this->get_value($values, 'transaction_id');
    $order_id = $this->get_value($values, 'order_id');
    $transaction = commerce_payment_transaction_load($transaction_id);

    if (!commerce_rbspayment_transaction_refund_access($transaction)) {
      return '';
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('refund');
    return l($text, 'admin/commerce/orders/' . $order_id . '/payment/' . $transaction_id . '/rbspayment_refund', array('query' => drupal_get_destination()));
  }

}
