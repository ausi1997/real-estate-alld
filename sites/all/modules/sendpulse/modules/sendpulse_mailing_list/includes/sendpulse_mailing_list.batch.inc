<?php

/**
 * @file
 * Contains sendpulse batch functions.
 */

/**
 * Batch process of synchronizing users in Mailing lists.
 */
function sendpulse_mailing_list_synchronize_users_process($mailing_lists, $user, &$context) {
  if (!isset($context['results']['user_names'])) {
    $context['results']['user_names'] = [];
  }

  $target_lists = sendpulse_mailing_list_get_mailing_lists_for_user($mailing_lists, $user);

  if (!empty($target_lists)) {
    foreach ($target_lists as $target_list) {
      $variables = sendpulse_mailing_list_get_user_variables_values($target_list, $user);
      $emails = [
        [
          'email' => $user->mail,
          'variables' => $variables,
        ]
      ];

      $client = sendpulse_get_client();
      $client->addEmails($target_list->sendpulse_abid, $emails);
      $context['results']['user_names'][] = $user->name;
    }
  }
}

/**
 * Finish callback after synchronizing users in Mailing lists
 */
function sendpulse_mailing_list_synchronize_users_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('Synchronized ' . count($results['user_names']) . ' users'));
  }
  else {
    drupal_set_message(t('An error has occurred.'), 'error');
  }

  drupal_goto('admin/structure/mailing_lists');
}
