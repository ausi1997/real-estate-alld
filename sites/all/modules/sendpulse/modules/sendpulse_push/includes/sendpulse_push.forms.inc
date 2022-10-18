<?php

/**
 * @file
 * Contains sendpulse settings form.
 */

/**
 * Implements hook_form_FORM_ID_alter().
 */
function sendpulse_push_form_node_type_form_alter(&$form, &$form_state) {
  // Do not process node type form if it do not contain node type.
  if (empty($form['#node_type']->type) || !user_access('administer sendpulse push notifications')) {
    return;
  }

  $node_type = $form['#node_type']->type;

  $form['sendpulse_push'] = [
    '#type' => 'fieldset',
    '#title' => t('SendPulse Push Notifications settings'),
    '#group' => 'additional_settings',
  ];

  // Enable setting.
  $form['sendpulse_push']['sendpulse_push_node_enabled'] = [
    '#type' => 'checkbox',
    '#title' => t('Enable SendPulse Pushes'),
    '#default_value' => variable_get('sendpulse_push_node_enabled_' . $node_type),
    '#description' => t('Enable or disable pushes for this node type.')
  ];

  // Enable default setting.
  $form['sendpulse_push']['sendpulse_push_node_enabled_default'] = [
    '#type' => 'checkbox',
    '#title' => t('Checkbox "Push this node to subscribers" are checked by default'),
    '#default_value' => variable_get('sendpulse_push_node_enabled_default_' . $node_type),
    '#description' => t('Check this if you want checkbox "Push this node to subscribers" in node forms was checked by default.')
  ];

  $form['sendpulse_push']['sendpulse_push_node_enabled_prevent_double_post'] = [
    '#type' => 'checkbox',
    '#title' => t('Uncheck previous checkbox if current node has already pushed to subscribers'),
    '#description' => t('Prevent double pushing for nodes that already pushed to subscribers.'),
    '#default_value' => variable_get('sendpulse_push_node_enabled_prevent_double_post_' . $node_type),
    '#states' => [
      'visible' => [
        ':input[name="sendpulse_push_node_enabled_default"]' => [
          'checked' => TRUE
        ],
      ]
    ],
  ];

  // Message.
  $form['sendpulse_push']['sendpulse_push_message'] = [
    '#type' => 'fieldset',
    '#title' => t('Message'),
  ];

  $form['sendpulse_push']['sendpulse_push_message']['sendpulse_push_node_message_title'] = [
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => variable_get('sendpulse_push_node_message_title_' . $node_type, ''),
  ];

  $form['sendpulse_push']['sendpulse_push_message']['sendpulse_push_node_message_body'] = [
    '#type' => 'textarea',
    '#title' => t('Web push text'),
    '#description' => t('Web push notification content.'),
    '#default_value' => variable_get('sendpulse_push_node_message_body_' . $node_type, ''),
  ];

  $form['sendpulse_push']['sendpulse_push_message']['sendpulse_push_node_message_link'] = [
    '#type' => 'textfield',
    '#title' => t('Link'),
    '#description' => t('Navigation link; if it’s not specified, the website URL will be used'),
    '#default_value' => variable_get('sendpulse_push_node_message_link_' . $node_type, ''),
  ];

  if (module_exists('entity_token')) {
    $form['sendpulse_push']['sendpulse_push_message']['token_help'] = [
      '#theme' => 'token_tree_link',
      '#token_types' => ['node'],
      '#global_types' => TRUE,
      '#click_insert' => TRUE,
    ];
  }
  else {
    $form['sendpulse_push']['sendpulse_push_message']['token_help'] = [
      '#markup' => t('This module use entity_token module for configuration of text fields template. Please enable entity_token module for better configuration.')
    ];
  }

  $form['sendpulse_push']['sendpulse_push_message']['sendpulse_push_node_message_ttl'] = [
    '#type' => 'textfield',
    '#title' => t('Time to live'),
    '#description' => t('Push notification lifetime, in seconds. If not set, the default time (1 day) is applied.'),
    '#default_value' => variable_get('sendpulse_push_node_message_ttl_' . $node_type, ''),
  ];
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function sendpulse_push_form_node_form_alter(&$form, &$form_state) {
  // Do not process node form if it is not contains node type
  // or user doesn't have permissions.
  if (empty($form['#node']->type) || !user_access('sendpulse push notifications')) {
    return;
  }

  // Check node type and user access
  // to see whether we should process a node form.
  $type_enabled = variable_get('sendpulse_push_node_enabled_' . $form['#node']->type, 0);
  $user_access = user_access('sendpulse push notifications');

  // If node type matches all requirements we have to add new fields to form.
  if ($type_enabled && $user_access) {
    // Add SendPulse fieldset to node add/edit form.
    $form['sendpulse_push'] = [
      '#type' => 'fieldset',
      '#title' => t('SendPulse Push'),
      '#group' => 'additional_settings',
      '#weight' => -10,
    ];

    $form['sendpulse_push']['sendpulse_push_this_node'] = [
      '#type' => 'checkbox',
      '#title' => t('Push this node to subscribers'),
      '#default_value' => variable_get('sendpulse_push_node_enabled_default_' . $form['#node']->type, 0),
      '#description' => t('After you submit this node it will be pushed to your subscribers.'),
    ];

    // Disable push for node if sendpulse is not configured.
    if (!sendpulse_configured()) {
      $form['sendpulse_push']['sendpulse_push_this_node']['#disabled'] = TRUE;
      $form['sendpulse_push']['sendpulse_push_this_node']['#value'] = 0;
      $form['sendpulse_push']['sendpulse_push_this_node']['#description'] = t("You can't send push notifications until API keys is not properly configured on module settings page.");
    }
  }
}

/**
 * Form for sending single push.
 * 
 * @param array $form
 *   Form.
 * @param array $form_state
 *   Form state.
 * @return array
 *   Form processed.
 */
function sendpulse_send_single_push_form(array $form, array &$form_state) {
  $form['message_title'] = [
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#required' => TRUE,
  ];

  $form['message_body'] = [
    '#type' => 'textarea',
    '#title' => t('Web push text'),
    '#description' => t('Web push notification content.'),
    '#required' => TRUE,
  ];

  $form['message_link'] = [
    '#type' => 'textfield',
    '#title' => t('Link'),
    '#description' => t('Navigation link; if it’s not specified, the website URL will be used'),
  ];

  $form['message_ttl'] = [
    '#type' => 'textfield',
    '#title' => t('Time to live'),
    '#description' => t('Push notification lifetime, in seconds. If not set, the default time (1 day) is applied.'),
    '#element_validate' => ['element_validate_integer_positive'],
  ];

  $form['send'] = [
    '#type' => 'submit',
    '#value' => t('Send Push'),
  ];

  return $form;
}

/**
 * Submit for "sendpulse_send_single_push_form".
 * 
 * @param array $form
 *   Form.
 * @param array $form_state
 *   Form state.
 */
function sendpulse_send_single_push_form_submit(array $form, array &$form_state) {
  $values = &$form_state['values'];

  // Get node data that will be pushed.
  $title = sendpulse_push_process_push_title($values['message_title']);
  $body = sendpulse_push_process_push_body($values['message_body']);
  $link = $values['message_link'];
  $ttl = $values['message_ttl'];
  $website_id = sendpulse_push_get_website_id();

  if (empty($website_id)) {
    watchdog('sendpulse', 'It looks like your website is not configured to send push notifications through the sendpulse.com. You can clarify the details in the control panel https://login.sendpulse.com/push/');
    drupal_set_message(t('It looks like your website is not configured to send push notifications through the sendpulse.com. You can clarify the details in the <a href="https://login.sendpulse.com/push" target="_blank">control panel</a>.'), 'error');
  }
  else {
    try {
      // If ttl was not specified then let it be 1 day by default.
      if (empty($ttl)) {
        $ttl = SENDPULSE_PUSH_TTL_TIME_DEFAULT;
      }

      // Send info about push to SendPulse.
      $result = sendpulse_push_create_new_task($website_id, $title, $body, $ttl, [
        'link' => $link,
        'stretch_time' => SENDPULSE_PUSH_STRETCH_TIME,
      ]);

      if (!empty($result->is_error)) {
        watchdog('sendpulse', 'Error after trying to create Push: @message', ['@message' => $result->message]);
        drupal_set_message(t('Error after trying to create Push: @message', ['@message' => $result->message]), 'error');
      }
      else {
        drupal_set_message(t('Push notification was successfully send to subscribers'));
      }
    }
    catch (Exception $e) {
      watchdog_exception('sendpulse', $e);
      drupal_set_message(t('SendPulse: @message', ['@message' => $e->getMessage()]), 'error');
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function sendpulse_push_form_sendpulse_admin_main_settings_alter(&$form, &$form_state, $form_id) {
  if (!user_access('administer sendpulse push notifications')) {
    return;
  }

  $form['sendpulse_push'] = [
    '#type' => 'fieldset',
    '#title' => t('Sendpulse Push settings'),
  ];

  if (!empty($_SERVER['https'])) {
    $form['sendpulse_push']['sendpulse_push_files_installed'] = [
      '#type' => 'markup',
      '#title' => t('Chrome setup files'),
      '#description' => t("Download the setup files from SendPulse for the Chrome browser support. Unzip the archive and upload the files into the root folder of the Drupal installation."),
      '#theme_wrappers' => ['form_element'],
      '#markup' => file_exists('sp-push-manifest.json') && file_exists('sp-push-worker.js') ?
        '<span style="color:green;">' . t('Files sp-push-worker.js and sp-push-manifest.json are installed') . '</span>' :
        '<span style="color:red;">' . t('Files sp-push-worker.js and sp-push-manifest.json are not installed') . '</span>'
    ];
  }

  $form['sendpulse_push']['sendpulse_description'] = [
    '#type' => 'markup',
    '#markup' => t('Configure node type that should be pushed using SendPulse on <em>/admin/structure/types/manage/<b>{NODE_TYPE}</b></em> page.'),
  ];

  $form['#submit'][] = 'sendpulse_push_form_sendpulse_admin_main_settings_submit';
}

/**
 * Form submit for 'sendpulse_admin_main_settings' form.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   Form state.
 */
function sendpulse_push_form_sendpulse_admin_main_settings_submit(&$form, &$form_state) {
  $values = &$form_state['values'];

  // Remove spaces. Just in case of small user mistake.
  $values['sendpulse_id'] = trim($values['sendpulse_id']);
  $values['sendpulse_secret'] = trim($values['sendpulse_secret']);

  $client = sendpulse_get_client('session', $values['sendpulse_id'], $values['sendpulse_secret']);
  $website_id = sendpulse_push_get_website_id($client);
  if (empty($website_id)) {
    variable_set('sendpulse_push_code', '');

    drupal_set_message(t('It looks like your website is not configured to send push notifications through the sendpulse.com. You can clarify the details in the <a href="https://login.sendpulse.com/push" target="_blank">control panel</a>. <br /> The request to send push notifications will not be shown to your users until you have not added your site to the SendPulse push control panel. <br /> After that, you need to re-save this form.'), 'warning');
    watchdog('sendpulse', 'It looks like your website is not configured to send push notifications through the sendpulse.com. You can clarify the details in the control panel https://login.sendpulse.com/push');
  }
  else {
    $code = $client->getPushIntegrationCode($website_id);
    variable_set('sendpulse_push_code', $code->script_code);
  }
}
