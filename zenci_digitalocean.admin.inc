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

/**
 * Menu callback; Display a list of all Digital Ocean templates.
 */
function zenci_digitalocean_templates() {
  $templates = db_select('zenci_digitalocean_templates', 'wr')
    ->fields('wr')
    ->orderBy('name', 'DESC')
    ->execute()
    ->fetchAll();

  $rows = array();
  foreach ($templates as $template) {
    $row = array();
    $row[] = array('data' => $template->name);
    $row[] = array('data' => $template->region);
    $row[] = array('data' => $template->size);
    $row[] = array('data' => $template->image);
    $links['delete'] = array(
      'title' => t('Delete'),
      'href' => "admin/config/system/digitalocean/" . $template->id . "/delete",
    );
    $row[] = array(
      'data' => array(
        '#type' => 'operations',
        '#links' => $links,
      ),
    );
    $rows[] = $row;
  }
  $header = array(t('Name'), t('Region'), t('Size'), t('Image'), t('Operations'));

  $output['table'] = array(
    '#theme' => 'table__zenci_digitalocean_templates',
    '#rows' => $rows,
    '#header' => $header,
    '#empty' => t('No templates yet.'),
  );
  return $output;  
}
