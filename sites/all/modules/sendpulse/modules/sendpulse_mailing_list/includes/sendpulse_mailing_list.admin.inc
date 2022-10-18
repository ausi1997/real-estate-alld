<?php

/**
 * @file
 * Contains sendpulse settings form.
 */

/**
 * Generates the domain type editing form.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   Form state.
 * @param \SendpulseMailingList $mailing_list
 *   Domain.
 * @param string $op
 *   Operation.
 *
 * @return array
 *   Built form.
 */
function sendpulse_mailing_list_form(array $form, array &$form_state, SendpulseMailingList $mailing_list, $op = 'edit') {
  $form['label'] = [
    '#title' => t('Mailing list name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => isset($mailing_list->label) ? $mailing_list->label : '',
    '#weight' => -20,
  ];

  $form['roles'] = [
    '#title' => t('Roles'),
    '#description' => t('Select user roles to synchronize with this mailing list.'),
    '#type' => 'checkboxes',
    '#options' => user_roles(TRUE),
    '#default_value' => isset($mailing_list->roles) ? $mailing_list->roles : [],
  ];

  if ($op != 'add') {
    $form['sendpulse_fields_mapping'] = [
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#title' => t('Mapping SendPulse mailing lists variables to Drupal Fields'),
      '#description' => t('Define which fields from Drupal (= Sources) map to which variables in SendPulse (= Targets).
The mapped target fields will be updated after user export.'
      ),
      '#prefix' => '<div id="sendpulse-fields-mapping-wrap">',
      '#suffix' => '</div>',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    ];
    $form['sendpulse_fields_mapping']['source'] = [
      '#prefix' => '<div class="sendpulse-mappings-add">',
      '#type' => 'select',
      '#title' => t('Source (Drupal)'),
      '#empty_option' => t('- Select -'),
      '#options' => sendpulse_get_drupal_user_fields_options($mailing_list),
    ];
    $form['sendpulse_fields_mapping']['target'] = [
      '#type' => 'select',
      '#title' => t('Target (SendPulse)'),
      '#empty_option' => t('- Create new -'),
      '#options' => sendpulse_mailing_list_get_mapping_options($mailing_list),
    ];
    $form['sendpulse_fields_mapping']['sendpulse_map_add'] = [
      '#prefix' => '<div class="sendpulse-add-button">',
      '#suffix' => '</div></div>',
      '#type' => 'submit',
      '#value' => t('Add'),
      '#ajax' => [
        'callback' => 'sendpulse_add_map',
        'wrapper' => 'sendpulse-fields-mapping-wrap',
        'method' => 'replace',
        'effect' => 'fade',
      ],
    ];
    $form['sendpulse_fields_mapping']['sendpulse_extra_mappings'] = sendpulse_build_sendpulse_map_tbl($mailing_list);
    $form['sendpulse_fields_mapping']['remove_mappings'] = [
      '#type' => 'submit',
      '#value' => t('Remove mappings'),
      '#ajax' => [
        'callback' => 'sendpulse_remove_mappings',
        'wrapper' => 'sendpulse-fields-mapping-wrap',
        'method' => 'replace',
        'effect' => 'fade',
      ],
    ];
  }

  $form['actions'] = ['#type' => 'actions'];
  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Save mailing list'),
    '#weight' => 40,
  ];

  $form['#attached']['css'] = [drupal_get_path('module', 'sendpulse_mailing_list') . '/css/sendpulse_mailing_list_admin.css'];

  return $form;
}

/**
 * Form API submit callback for the type form.
 *
 * @param array $form
 *   Input form.
 * @param array $form_state
 *   Input form state.
 */
function sendpulse_mailing_list_form_submit(array $form, array &$form_state) {
  if (!empty($form_state['triggering_element']['#value'])
    && $form_state['triggering_element']['#value'] != t('Save mailing list')) {
    return;
  }

  /** @var SendpulseMailingList $mailing_list */
  $mailing_list = entity_ui_form_submit_build_entity($form, $form_state);

  if ($form_state['op'] == 'add') {
    $client = sendpulse_get_client();

    $sendpulse_book = $client->createAddressBook($mailing_list->label());
    $mailing_list->sendpulse_abid = $sendpulse_book->id;

    drupal_set_message(t('Mailing list was added'));
    $form_state['redirect'] = 'admin/structure/mailing_lists/manage/' . $mailing_list->abid;
  }
  else {
    drupal_set_message(t('Mailing list was updated'));
    $form_state['redirect'] = 'admin/structure/mailing_lists';
  }

  // Save and go back.
  $mailing_list->save();
}

/**
 *
 *
 * @return array
 */
function sendpulse_get_drupal_user_fields() {
  $drupal_user_fields = [];
  $valid_fields = [
    'text',
    'varchar',
    'char',
    'blob',
    'integer',
    'decimal',
    'boolean',
    'date',
  ];

  $user_info = entity_get_property_info('user');

  $custom_fields = !empty($user_info['bundles']['user']['properties']) && is_array($user_info['bundles']['user']['properties'])
    ? $user_info['bundles']['user']['properties']
    : [];
  $base_fields = is_array($user_info['properties'])
    ? $user_info['properties']
    : [];
  foreach ($base_fields as $field_name => $field_info) {
    if (!empty($field_info['type']) && in_array($field_info['type'], $valid_fields)) {
      $drupal_user_fields['Base Fields'][$field_name] = [
        'label' => $field_info['label'],
        'type' => 'property',
      ];
    }
  }
  foreach ($custom_fields as $field_name => $field_info) {
    if (!empty($field_info['type']) && in_array($field_info['type'], $valid_fields)) {

      if ($field_info['type'] == 'date') {
        $field_info_extra = field_info_field($field_name);
        $field_type = $field_info_extra['type'];
      }
      else {
        $field_type = $field_info['type'];
      }

      $drupal_user_fields['Added Fields'][$field_name] = [
        'label' => $field_info['label'],
        'type' => $field_type,
      ];
    }
  }

  return $drupal_user_fields;
}

/**
 *
 *
 * @param \SendpulseMailingList $mailing_list
 * @return array
 */
function sendpulse_get_drupal_user_fields_options(SendpulseMailingList $mailing_list) {
  $options = [];

  $drupal_user_fields = sendpulse_get_drupal_user_fields();
  if (!empty($drupal_user_fields)) {
    foreach ($drupal_user_fields as $category_name => $field_category) {
      foreach ($field_category as $key => $drupal_user_field) {
        if (!in_array($key, $mailing_list->mapping)) {
          $options[$category_name][$key] = $drupal_user_field['label'];
        }
      }
    }
  }

  return $options;
}

/**
 *
 *
 * @param \SendpulseMailingList $mailing_list
 * @return array
 */
function sendpulse_build_sendpulse_map_tbl(SendpulseMailingList $mailing_list) {
  $header = [
    1 => t('Source (Drupal)'),
    2 => t('Target (SendPulse)'),
  ];
  $tbl_opts = [];
  if (!empty($mailing_list->mapping)) {
    $user_fields = sendpulse_get_drupal_user_fields();

    foreach ($mailing_list->mapping as $target => $source) {
      foreach ($user_fields as $category_name => $fields) {
        if (!empty($fields[$source])) {
          $source_label = $fields[$source]['label'];
          break;
        }
      }

      if (!empty($source_label)) {
        $tbl_opts[$target] = [
          1 => $source_label . ' <small>(' . $source . ')</small>',
          2 => $target,
        ];
      }
      else {
        unset($mailing_list->mapping[$target]);
        $mailing_list->save();
      }
    }
  }

  return [
    '#type' => 'tableselect',
    '#header' => $header,
    '#empty' => t('No items'),
    '#options' => $tbl_opts,
    '#attributes' => ['id' => 'sendpulse-mapping-tbl'],
  ];
}

/**
 * Ajax submit callback for add mapping.
 *
 * @param array $form
 * @param array $form_state
 * @return mixed
 */
function sendpulse_add_map(array $form, array &$form_state) {
  $mailing_list = &$form_state['sendpulse_mailing_list'];
  $errors = form_get_errors();
  if (empty($errors)) {
    $vals = $form_state['values'];
    $source = $vals['sendpulse_fields_mapping']['source'];
    if (!empty($source)) {
      if (empty($vals['sendpulse_fields_mapping']['target'])) {
        $user_fields = sendpulse_get_drupal_user_fields();

        foreach ($user_fields as $category_name => $fields) {
          if (!empty($fields[$source])) {
            $vals['sendpulse_fields_mapping']['target'] = $fields[$source]['label'];
            break;
          }
        }
      }
      $mailing_list->mapping[$vals['sendpulse_fields_mapping']['target']] = $source;
      $mailing_list->save();
    }
    else {
      drupal_set_message(t('Source field should be selected'), 'error');
    }
  }
  $form = drupal_rebuild_form('sendpulse_mailing_list_form', $form_state, $form);
  return $form['sendpulse_fields_mapping'];
}

/**
 * Ajax submit callback for removing selected mappings.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   Form state.
 * @return array
 *   Rebuilded form.
 */
function sendpulse_remove_mappings(array $form, array &$form_state) {
  $mailing_list = &$form_state['sendpulse_mailing_list'];
  $errors = form_get_errors();
  if (empty($errors)) {
    sendpulse_mailing_list_mapping_clean($mailing_list, $form_state);
    $mailing_list->save();
  }
  $form = drupal_rebuild_form('sendpulse_mailing_list_form', $form_state, $form);
  return $form['sendpulse_fields_mapping'];
}

/**
 *
 *
 * @param \SendpulseMailingList $mailing_list
 * @param $form_state
 */
function sendpulse_mailing_list_mapping_clean(SendpulseMailingList &$mailing_list, &$form_state) {
  if (!empty($form_state['values']['sendpulse_fields_mapping']['sendpulse_extra_mappings'])) {
    $vals = $form_state['values']['sendpulse_fields_mapping']['sendpulse_extra_mappings'];
    if (!empty($mailing_list->mapping)) {
      foreach ($vals as $target => $src) {
        if ($target === $src) {
          unset($mailing_list->mapping[$target]);
        }
        if (empty($target)) {
          unset($mailing_list->mapping[$target]);
        }
      }
    }
  }
}

/**
 *
 *
 * @param \SendpulseMailingList $mailing_list
 * @return array
 */
function sendpulse_mailing_list_get_mapping_options(SendpulseMailingList $mailing_list) {
  $client = sendpulse_get_client();
  $mapping_options = [];

  $book_variables = $client->getBookVariables($mailing_list->sendpulse_abid);
  foreach ($book_variables as $variable_data) {
    if (!isset($mailing_list->mapping[$variable_data->name])) {
      $mapping_options[$variable_data->name] = $variable_data->name;
    }
  }

  // Add SendPulse default variable 'Phone' in list.
  if (!isset($mailing_list->mapping['Phone'])) {
    $mapping_options['Phone'] = t('Phone');
  }

  return $mapping_options;
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function sendpulse_mailing_list_form_sendpulse_admin_main_settings_alter(&$form, &$form_state, $form_id) {
  if (!user_access('administer sendpulse mailing lists')) {
    return;
  }

  // Mailing lists settings.
  $form['sendpulse_mailing_list'] = [
    '#type' => 'fieldset',
    '#title' => t('Sendpulse Mailing lists settings'),
  ];

  $form['sendpulse_mailing_list']['sendpulse_mailing_list_sort_property'] = [
    '#type' => 'select',
    '#title' => t('Sorting property'),
    '#description' => t('Select the property by which you want to sort the the Mailing lists in the <a href="@table_path">table</a>.', [
      '@table_path' => url('admin/structure/mailing_lists'),
    ]),
    '#options' => [
      'creationdate' => t('Creation date'),
      'label' => t('Label'),
      'all_email_qty' => t('Total number of addresses'),
      'active_email_qty' => t('Active addresses'),
      'inactive_email_qty' => t('Inactive addresses'),
    ],
    '#default_value' => variable_get('sendpulse_mailing_list_sort_property', 'creationdate'),
  ];

  $form['sendpulse_mailing_list']['sendpulse_mailing_list_export_users'] = [
    '#type' => 'checkbox',
    '#title' => t('Automatically export users to SendPulse after their registration'),
    '#default_value' => variable_get('sendpulse_mailing_list_export_users', FALSE),
  ];

  // Subscribe at account registration time.
  $options = [
    'checked' => t('Default checked'),
    'unchecked' => t('Default unchecked'),
    'hidden' => t('Hidden'),
  ];
  $form['sendpulse_mailing_list']['sendpulse_mailing_list_subscribe_checkbox'] = [
    '#type' => 'select',
    '#title' => t('Subscription checkbox on registration form'),
    '#options' => $options,
    '#default_value' => variable_get('sendpulse_mailing_list_subscribe_checkbox', 'hidden'),
    '#description' => t('Default checked: Subscription checkbox is listed on the user registration page and is selected by default.<br />Default unchecked: Subscription checkbox is listed on the user registration page and is not selected by default.<br />Hidden: A new user is automatically exports in Sendpulse lists. Subscription checkbox is not listed on the user registration page.'),
    '#states' => [
      'visible' => [
        ':input[name="sendpulse_mailing_list_export_users"]' => [
          'checked' => TRUE
        ],
      ],
    ],
  ];
  $form['sendpulse_mailing_list']['sendpulse_mailing_list_subscribe_text'] = [
    '#type' => 'textfield',
    '#title' => t('Text for Subscription checkbox'),
    '#default_value' => variable_get('sendpulse_mailing_list_subscribe_text', t('Subscribe')),
    '#states' => [
      'visible' => [
        ':input[name="sendpulse_mailing_list_export_users"]' => [
          'checked' => TRUE
        ],
      ],
    ],
  ];

  $form['#submit'][] = 'sendpulse_mailing_list_form_sendpulse_admin_main_settings_submit';
}

function sendpulse_mailing_list_form_sendpulse_admin_main_settings_submit($form, &$form_state) {
  // Run cron for update/create mailing lists in Drupal
  // if api ID or SECRET was changed.
  if ($form['sendpulse']['sendpulse_id']['#default_value'] !== $form_state['values']['sendpulse_id']
    || $form['sendpulse']['sendpulse_secret']['#default_value'] !== $form_state['values']['sendpulse_secret']
  ) {
    sendpulse_mailing_list_cron();
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function sendpulse_mailing_list_form_user_register_form_alter(&$form, &$form_state) {
  if (variable_get('sendpulse_mailing_list_export_users', FALSE)
    && variable_get('sendpulse_mailing_list_subscribe_checkbox', 'hidden') != 'hidden')
  {
    $subscribe_checkbox = variable_get('sendpulse_mailing_list_subscribe_checkbox', 'hidden');

    // Add subscription checkbox to user register form.
    $form['sendpulse_mailing_list_subscribe'] = [
      '#type' => 'checkbox',
      '#title' => variable_get('sendpulse_mailing_list_subscribe_text', t('Subscribe')),
      '#default_value' => $subscribe_checkbox === 'checked' ? TRUE : FALSE,
    ];
  }
}
