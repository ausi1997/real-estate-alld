<?php

/**
 * @file
 * Get Iframe url.
 */

/**
 * {@inheritdoc}
 */
function get_iframe_url() {
  $url = "";
  $authToken = variable_get('pushflew_section/pushflew_group/auth_token') ? variable_get('pushflew_section/pushflew_group/auth_token') : '';
  if ($authToken == NULL) {
    $url = register_website();
  } else {
    $websiteId = variable_get('pushflew_section/pushflew_group/website_id');
    $url = create_url($authToken, $websiteId);
  }

  return $url;
}

/**
 * Register new website - callback.
 */
function register_website() {
  global $user, $base_url;

  $data = array();
  $data['url'] = "https://app.pushflew.com/registerWebsite";
  $data['user_name'] = $user->name;
  $data['email'] = $user->mail;
  $data['company'] = variable_get('site_name');
  $data['website'] = $base_url;
  $parsedUrl = parse_url($data['website']);
  $host = explode('.', $parsedUrl['host']);
  if (count($host) > 1) {
    $subdomain = $host[0];
  } else {
    $subdomain = '';
  }
  $data['subdomain'] = $host[0];
  $data['host'] = $_SERVER['REQUEST_SCHEME'];
  $data['timezone'] = $user->timezone;

  $body = array(
    'ownedByUser' => $data['email'],
    'source' => 'drupal',
    'company' => $data['company'],
    'protocol' => $data['host'],
    'website' => $data['website'],
    'subdomain' => $data['subdomain'],
    'timezone' => $data['timezone']
  );

  $frameURL = "";
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $data['url']);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
  curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($body));
  curl_setopt($ch, CURLOPT_HTTPHEADER, array("Content-type: application/json"));
  $response = curl_exec($ch);

  if (curl_error($ch)) {
    echo '[PushFlew]Failed to register Pushflew, try again. <br> Error:' . curl_error($ch);
  } else {
    $result = json_decode($response, TRUE);
    save_website_data($result);
    register_logo($result['websiteId'], $result['auth']);
    $frameURL = create_url($result['auth'], $result['websiteId']);
  }
  curl_close($ch);
  return $frameURL;
}

/**
 * Save_website_data - callback.
 */
function save_website_data($data) {
  variable_set('pushflew_section/pushflew_group/auth_token', $data['auth']);
  variable_set('pushflew_section/pushflew_group/website_id', $data['websiteId']);
}

/**
 * Register_logo - callback.
 */
function register_logo($siteId, $auth) {
  $websiteId = ($siteId ? $siteId : variable_get('pushflew_section/pushflew_group/website_id'));
  $authToken = ($auth ? $auth : variable_get('pushflew_section/pushflew_group/auth_token'));

  if ($websiteId != NULL && $authToken != NULL) {
    global $base_url;
    global $theme_key;
    $logo = theme_get_setting('logo', $theme_key);
    variable_set('pushlogo', $logo);
    $url = "https://app.pushflew.com/saveSiteLogo";
    $body = array(
      'websiteId' => $websiteId,
      'logoURL' => $logo,
    );

    $chh = curl_init();
    curl_setopt($chh, CURLOPT_URL, $url);
    curl_setopt($chh, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($chh, CURLOPT_POST, 1);
    curl_setopt($chh, CURLOPT_SSL_VERIFYPEER, FALSE);
    curl_setopt($chh, CURLOPT_POSTFIELDS, json_encode($body));
    curl_setopt($chh, CURLOPT_HTTPHEADER, array("Content-type: application/json", "Authorization:" . $authToken));
    $response = curl_exec($chh);

    if (curl_error($chh)) {
      echo '[PushFlew]Failed to register Pushflew, try again. <br> Error:' . curl_error($chh);
    }
    curl_close($chh);
  }
}

/**
 * Create iframe URL.
 */
function create_url($auth, $websiteId) {
  global $user;

  $baseurl = "https://app.pushflew.com/third-party-view";
  $email = $user->mail;
  $url = sprintf("%s?currentuser=%s&websiteId=%s", $baseurl, $email, $websiteId);
  $url = $url . "&auth=" . $auth;

  return $url;
}

/**
 * Get User email.
 */
function getUser() {
  global $user;

  $user = $user->name;
  $email = $user->mail;

  return $email;
}