<?php
/**
 * @file
 * Administration pages for DigitalOcean integration.
 */

 /**
 * Menu callback for admin/config/system/qa.
 */
function zenci_digitalocean_settings() {
  $config = config('zenci_digitalocean.settings');

  // Use setting.php values if exists.
  $api_token = settings_get('zenci_digitalocean_api_token');

  $form['api_token'] = array(
    '#type' => 'textfield',
    '#title' => t('DigitalOcean API token'),
    '#default_value' => !empty($api_token) ? $api_token : $config->get('api_token'),
    '#description' => t('You can obtain API Key on https://cloud.digitalocean.com/settings/api/tokens page'),
    '#disabled' => !empty($api_token) ? TRUE : FALSE,
  );
  
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Submit handler for the qa_settings() form.
 */
function zenci_digitalocean_settings_submit($form, &$form_state) {

  $config = config('zenci_digitalocean.settings');
  $api_token = settings_get('zenci_digitalocean_api_token');
  if (empty($api_token)) {
    $config->set('api_token', $form_state['values']['api_token']);
  }
  
  $config->save();
  
  backdrop_set_message(t('The configuration options have been saved.'));
}
