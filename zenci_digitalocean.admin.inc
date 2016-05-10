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
    $row[] = array('data' => $template->size);
    $row[] = array('data' => $template->image_name);
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
  $header = array(t('Name'), t('Size'), t('Image'), t('Operations'));

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
function zenci_digitalocean_template($form, &$form_state) {
  
  $digitalOceanAPI = zenci_digitalocean_get_class();
  $images = $digitalOceanAPI->getImages(TRUE);
  
  $form_state['images'] = $images;
  
  $image_options = array();
  foreach($images as $image){
    $image_options[$image->id] = $image->name;
  }
  
  $digitalOceanAPI->reInitCurl();
  
  $sizes =  $digitalOceanAPI->getSizes();
  
  $size_options = array();
  foreach($sizes as $size){
    $size_options[$size->slug] = $size->slug;
  }
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('This is a test task name.'),
    '#required' => TRUE,
  );
  $form['image'] = array(
    '#type' => 'select',
    '#title' => t('Image'),
    '#options' => $image_options,
    '#description' => t('Please select image.'),
  );

  $form['size'] = array(
    '#type' => 'select',
    '#title' => t('Size'),
    '#options' => $size_options,
    '#description' => t('Please select image.'),
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

  $image_name = '';
  $images = $form_state['images'];
  foreach($images as $image){
    if($image->id == $form_state['values']['image']){
      $image_name = $image->name;
    }
  }
    
  $template = array(
    'name' => $form_state['values']['name'],
    'size' => $form_state['values']['size'],
    'image_id' => $form_state['values']['image'],
    'image_name' => $image_name,
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

/**
 * Add instance form.
 */
function zenci_digitalocean_instance($form, &$form_state) {

  $templates = db_select('zenci_digitalocean_templates', 'wr')
    ->fields('wr')
    ->orderBy('name', 'DESC')
    ->execute()
    ->fetchAll();

  $options = array();
  foreach ($templates as $template) {
    $options[$template->name] = $template->name . " " . $template->image_name . " " . $template->size;
  }  
  $form['name'] = array(
    '#type' => 'select',
    '#title' => t('Template name'),
    '#options' => $options,
    '#description' => t('Please select template name.'),
  );


  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add instance'),
  );

  return $form;
}

/**
 * Submit handler for the zenci_digitalocean_instance() form.
 */
function zenci_digitalocean_instance_submit($form, &$form_state) {
  
  $template = db_select('zenci_digitalocean_templates', 'gh')
    ->fields('gh')
    ->condition('name', $form_state['values']['name'])
    ->execute()
    ->fetchObject();
  
  $digitalOceanAPI = zenci_digitalocean_get_class();
  $image = $digitalOceanAPI->getImage($template->image_id);
  
  $digitalOceanAPI->reInitCurl();
  
  $settings = array(
    'name' => 'Test ' . $template->name,
    'region' => $image->regions[0],
    'size' => $template->size,
    'image' => $template->image_id,    
  );
  $digitalOceanAPI->setDebug(TRUE);
  $droplet =  $digitalOceanAPI->createDroplet($settings);
  backdrop_set_message('<pre>' . print_r($droplet,true). '</pre>');
   
}
