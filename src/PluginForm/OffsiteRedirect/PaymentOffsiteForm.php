<?php

namespace Drupal\commerce_rbspayment\PluginForm\OffsiteRedirect;

use Drupal\commerce_payment\PluginForm\PaymentOffsiteForm as BasePaymentOffsiteForm;
use Drupal\commerce_price\Entity\Currency;
use Drupal\commerce_rbspayment\CommerceRbsPaymentApi;
use Drupal\Core\Form\FormStateInterface;

class PaymentOffsiteForm extends BasePaymentOffsiteForm {

  /**
   * {@inheritdoc}
   */
  public function buildConfigurationForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildConfigurationForm($form, $form_state);

    /** @var \Drupal\commerce_payment\Entity\PaymentInterface $payment */
    $payment = $this->entity;
    /** @var \Drupal\commerce_payment\Plugin\Commerce\PaymentGateway\OffsitePaymentGatewayInterface $payment_gateway_plugin */
    $payment_gateway_plugin = $payment->getPaymentGateway()->getPlugin();

    $data = [
//      'return' => $form['#return_url'],
//      'cancel' => $form['#cancel_url'],
//      'total' => $payment->getAmount()->getNumber(),
    ];

    $payment_gateway_configuration = $payment_gateway_plugin->getConfiguration();
    $user_name = $payment_gateway_configuration['username'];
    $password = $payment_gateway_configuration['password'];
    $double_staged = !$form['#capture'];
    $mode = $payment->getPaymentGatewayMode() == 'live' ? false : true;
    $logging = $payment_gateway_configuration['logging'] == 0 ? false : true;
    $timeout = $payment_gateway_configuration['timeout'];
    $url = $payment_gateway_configuration['server_url'];
    $test_url = $payment_gateway_configuration['server_test_url'];

    $rbs = new CommerceRbsPaymentApi($url, $test_url, $user_name, $password, $timeout, $double_staged, $mode, $logging);

    $order_number = $payment->getOrder()->getOrderNumber();
    $amount = $payment->getOrder()->getTotalPrice();
    $return_url = $GLOBALS['base_url'] . '?q=cart/rbspayment/complete/' . $order->order_id;

    $response = $rbs->registerOrder(
      $order_number,
      $amount->getNumber(),
      $this->getCurrencyNumericCode($amount->getCurrencyCode()),
      $return_url,
      $form['#exception_url'],
      '',
      'RU'
    );

    if (!isset($response['errorCode'])){
      $redirect_url = $form['#return_url'] ;
    } else {
      drupal_set_message(t('Error # %code: %message', [
        '%code' => $response['errorCode'],
        '%message' => $response['errorMessage']]), 'error');
      $redirect_url = $form['#exception_url'] ;
    }

    return $this->buildRedirectForm($form, $form_state, $redirect_url, $data, self::REDIRECT_GET);
  }

  /**
   * @param string $currency_code
   *
   * @return string
   */
  function getCurrencyNumericCode($currency_code) {
    $currency = Currency::load($currency_code);
    // RBS operates with RUR so we force RUB to be represented with RUR code of 810
    return $currency->getNumericCode() == 643 ? 810 : $currency->getNumericCode();
  }

}
