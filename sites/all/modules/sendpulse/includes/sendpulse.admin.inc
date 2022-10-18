<?php

/**
 * @file
 * Contains sendpulse settings form.
 */

/**
 * Page callback. Return form with sendpulse main settings.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   Form state.
 *
 * @return mixed
 *   Result form.
 */
function sendpulse_admin_main_settings(array $form, array &$form_state) {
  // API settings.
  $form['sendpulse'] = [
    '#type' => 'fieldset',
    '#title' => t('Sendpulse REST API settings'),
  ];

  $form['sendpulse']['sendpulse_id'] = [
    '#type' => 'textfield',
    '#title' => t('ID'),
    '#default_value' => variable_get('sendpulse_id', ''),
    '#required' => TRUE,
  ];

  $form['sendpulse']['sendpulse_secret'] = [
    '#type' => 'textfield',
    '#title' => t('Secret'),
    '#default_value' => variable_get('sendpulse_secret', ''),
    '#required' => TRUE,
  ];

  return system_settings_form($form);
}

/**
 * Validates main settings form.
 */
function sendpulse_admin_main_settings_validate($form, &$form_state) {
  try {
    $values = &$form_state['values'];

    // Remove spaces. Just in case of small user mistake.
    $values['sendpulse_id'] = trim($values['sendpulse_id']);
    $values['sendpulse_secret'] = trim($values['sendpulse_secret']);

    sendpulse_get_client('session', $values['sendpulse_id'], $values['sendpulse_secret']);
  }
  catch (Exception $e) {
    watchdog_exception('sendpulse', $e);
    form_set_error('sendpulse_id', t('@message', ['@message' => $e->getMessage()]));
    form_set_error('sendpulse_secret');
  }
}
