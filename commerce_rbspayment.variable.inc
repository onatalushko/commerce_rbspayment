<?php

/**
 * @file
 * Lists module's variables.
 */

/**
 * Implements hook_group_info().
 */
function commerce_rbspayment_group_info() {
  $groups = array();

  $groups['commerce_rbspayment_main_settings'] = array(
    'title' => t('Main RBS payment payment settings'),
    'access' => 'configure commerce rbspayment',
  );
  $groups['commerce_rbspayment_order_tokens'] = array(
    'title' => t('Order info tokens settings'),
    'access' => 'configure commerce rbspayment',
  );

  return $groups;
}

/**
 * Implements hook_variable_info().
 */
function commerce_rbspayment_variable_info() {
  $variables = array();

  $variables['commerce_rbspayment_merchant_login'] = array(
    'title' => t('Merchant login'),
    'description' => t('This is the merchant login that RBS payment sent you when you set up your account.'),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'type' => 'string',
    'element' => array(
      '#type' => 'textfield',
      '#size' => 16,
    ),
    'default' => '',
    'required' => TRUE,
  );
  $variables['commerce_rbspayment_password'] = array(
    'title' => t('Password'),
    'description' => t('This is the hash password that RBS payment sent you.'),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'type' => 'string',
    'element' => array(
      '#type' => 'textfield',
      '#size' => 16,
    ),
    'required' => TRUE,
    'default' => '',
  );
  $variables['commerce_rbspayment_server_timeout'] = array(
    'title' => t('Server connection timeout'),
    'description' => t('Time to wait till bank server respond to API request.'),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'type' => 'number',
    'element' => array(
      '#type' => 'textfield',
      '#size' => 3,
      '#field_suffix' => t('seconds'),
    ),
    'required' => TRUE,
    'default' => 60,
  );
  $variables['commerce_rbspayment_stages'] = array(
    'type' => 'select',
    'title' => t('Payment staging'),
    'description' => t('Only single stage payments are supported by current version of this module.'),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'options' => array(
      0 => t('Single staged'),
      1 => t('Double staged'),
    ),
    'element' => array(
      '#disabled' => TRUE,
    ),
    'required' => TRUE,
    'default_value' => 0,
  );
  $variables['commerce_rbspayment_mode'] = array(
    'title' => t('Environment'),
    'description' => t('In testing environment no actual money transfer happens'),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'type' => 'select',
    'options' => array(
      1 => t('Testing'),
      0 => t('Production'),
    ),
    'required' => TRUE,
    'default_value' => 1,
  );
  $variables['commerce_rbspayment_payment_process_timeout'] = array(
    'title' => t('User payment session length'),
    'description' => t('Time given to the customer to complete payment process in RBS gate.'),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'type' => 'number',
    'element' => array(
      '#type' => 'textfield',
      '#size' => 3,
      '#field_suffix' => t('minutes'),
    ),
    'required' => TRUE,
    'default' => 25,
  );
  $variables['commerce_rbspayment_log_api_requests'] = array(
    'title' => t('Log all API requests in watchdog for debugging purposes.'),
    'description' => t("Each time module makes request to API regarding order's state, it can log request's info to watchdog."),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'type' => 'boolean',
    'element' => array(
        '#type' => 'checkbox',
    ),
    'default' => FALSE,
  );
  $variables['commerce_rbspayment_back_url_query'] = array(
    'title' => t('Bank payment back URL additional query'),
    'description' => t('Sessions in analytics systems may break when user moves from your site to bank page. You may want to add special query to the url when user returns from bank page to prevent session break (like "?utm_nooverride=1").'),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'type' => 'string',
    'element' => array(
      '#type' => 'textfield',
      '#size' => 30,
    ),
    'default' => '',
  );
  $variables['commerce_rbspayment_acquiring_fee'] = array(
    'title' => t('Bank acquiring fee'),
    'description' => t('Percent that the bank charges for every transaction.'),
    'group' => 'commerce_rbspayment_main_settings',
    'access' => 'configure commerce rbspayment',
    'type' => 'number',
    'element' => array(
      '#type' => 'textfield',
      '#size' => 4,
      '#field_suffix' => '%',
    ),
    'default' => '',
  );

  $variables['commerce_rbspayment_tokens_description'] = array(
    'title' => t('Order description'),
    'group' => 'commerce_rbspayment_order_tokens',
    'access' => 'configure commerce rbspayment',
    'type' => 'string',
    'element' => array(
      '#type' => 'textfield',
      '#size' => 120,
      '#maxlength' => 256,
    ),
    'default' => '[commerce-order:commerce-customer-billing:commerce-customer-address:name-line] - [commerce-order:commerce-customer-billing:field-customer-phone], [commerce-order:commerce-customer-billing:commerce-customer-address:locality]',
  );
  $variables['commerce_rbspayment_tokens_language'] = array(
    'title' => t('Language'),
    'group' => 'commerce_rbspayment_order_tokens',
    'access' => 'configure commerce rbspayment',
    'type' => 'string',
    'element' => array(
      '#type' => 'textfield',
      '#size' => 120,
    ),
    'default' => 'RU',
  );
  
  return $variables;
}
