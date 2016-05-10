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
  $output['link'] = array(
    '#theme' => 'links__template_add',
    '#links' => array(
      'add' => array(
        'title' => t('Add template'),
        'href' => "admin/config/system/digitalocean/template_add",
      ),
    ),
    '#attributes' => array('class' => array('button-primary')),
  );
  $header = array(t('Name'), t('Region'), t('Size'), t('Image'), t('Operations'));

  $output['table'] = array(
    '#theme' => 'table__zenci_digitalocean_templates',
    '#rows' => $rows,
    '#header' => $header,
    '#empty' => t('No templates yet.'),
  );
  return $output;  
}

/**
 * Add template form.
 */
function zenci_digitalocean_template() {
  
  $digitalOceanAPI = zenci_digitalocean_get_class();
  
  $regions = $ZenCIAPI->getRegions();
  
  backdrop_set_message('<pre>' . print_r($regions, TRUE) . '</pre>');
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('This is a test task name.'),
    '#required' => TRUE,
  );
  $form['region'] = array(
    '#type' => 'textfield',
    '#title' => t('Region'),
    '#description' => t('The unique slug identifier for the region.'),
  );
  $form['size'] = array(
    '#type' => 'textfield',
    '#title' => t('Size'),
    '#description' => t('The unique slug identifier for the size.'),
  );
  $form['image'] = array(
    '#type' => 'textfield',
    '#title' => t('Image'),
    '#description' => t('The image ID of a public or private image, or the unique slug identifier for a public image.'),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add template'),
  );

  return $form;
}

/**
 * Validate handler for the zenci_digitalocean_template() form.
 */
function zenci_digitalocean_template_validate($form, &$form_state) {
  $template = db_select('zenci_digitalocean_templates', 'gh')
    ->fields('gh')
    ->condition('name', $form_state['values']['name'])
    ->execute()
    ->fetchObject();

  if ($template) {
    form_set_error('name', t('Name value should be unique.'));
  }
}

/**
 * Submit handler for the zenci_digitalocean_template() form.
 */
function zenci_digitalocean_template_submit($form, &$form_state) {
  $template = array(
    'name' => $form_state['values']['name'],
    'region' => $form_state['values']['region'],
    'size' => $form_state['values']['size'],
    'image' => $form_state['values']['image'],
  );
  if(FALSE !== backdrop_write_record('zenci_digitalocean_templates', $template)){
    backdrop_set_message(t('Template !name created', array(
      '!name' => $template['name'],
    )));
  }
  backdrop_goto('admin/config/system/digitalocean/list');  
}

/**
 * Remove template form.
 */
function zenci_digitalocean_template_remove($form, &$form_state, $template_id) {
  $template = db_select('zenci_digitalocean_templates', 'gh')
    ->fields('gh')
    ->condition('id', $template_id)
    ->execute()
    ->fetchObject();

  if (empty($template)) {
    backdrop_goto('admin/config/system/digitalocean/list');
  }

  $form_state['template'] = $template;

  $form['message'] = array(
    '#markup' => t('Are you sure want to delete this template?'),
  );
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );

  return $form;

}


/**
 * Submit handler for the zenci_digitalocean_template_remove() form.
 */
function zenci_digitalocean_template_remove_submit($form, &$form_state) {
  $template = $form_state['template'];
  db_delete('zenci_digitalocean_templates')
    ->condition('id', $template->id)
    ->execute();
  backdrop_set_message(t('Template deleted!'));
  backdrop_goto('admin/config/system/digitalocean/list');
}
