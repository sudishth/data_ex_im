<?php
/**
 * @file
 * Enables users to be exported and imported.
 */

/**
 * Callback function to export users.
 */
function data_export_import_callback_export_users() {

  return drupal_get_form('data_export_import_export_users_form');
}

/**
 * Function to create form to export users.
 */
function data_export_import_export_users_form($form_state) {

  $form['description'] = array(
    '#type' => 'item', 
    '#title' => t('Export all users to a dataset file'),
  );

  // Adds a simple submit button that refreshes the form and clears its
  // contents. This is the default behavior for forms.
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Create dataset file',
  );
  return $form;
}

/**
 * Function to process form to export users.
 */
function data_export_import_export_users_form_submit($form, &$form_state) {

  // This is the function which will carry out the exporting of the
  // users to a dataset file.
  $dataset_file_created = data_export_import_export_users_to_file();

  drupal_set_message(t("The following dataset file was created:"));
  drupal_set_message(check_plain($dataset_file_created));

  return TRUE;
}

/**
 * Export the required dataset file.
 */
function data_export_import_export_users_to_file() {

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  // Here will add the list of roles to the dataset - this is so we can check
  // that the receiving instance has a matching set of roles.
  $dataset['roles'] = user_roles();

  $result = db_query("SELECT uid FROM {users}");

  foreach ($result as $user) {

    // We will not put the anonymous or admin users into the file.  These are
    // expected to exist as standard.
    if ($user->uid > 1) {

      // Attach the data for a user to the array.
      $dataset['users'][$user->uid] = user_load($user->uid);
    }
  }

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = variable_get('file_public_path', conf_path() . '/files') . "/data_export_import";
  file_prepare_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  $dataset_directory = variable_get('file_public_path', conf_path() . '/files') . "/data_export_import/users/";
  file_prepare_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_users.dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_unmanaged_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;
}

/**
 * Callback function to import users.
 */
function data_export_import_callback_import_users() {

  return drupal_get_form('data_export_import_import_users_form');
}

/**
 * Function to create form to import users.
 */
function data_export_import_import_users_form($form_state) {

  $form = array();

  $form['users'] = array(
    '#type' => 'fieldset',
    '#title' => t('Users'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  // Get the contents of the dataset directory and create a list of
  // links to dataset files.
  $directory = variable_get('file_public_path', conf_path() . '/files') . "/data_export_import/users";

  // $mask = '.dataset';
  $mask = '/.dataset/';

  $files = file_scan_directory($directory, $mask);

  // Sort them by the filename which is used as the key.  Since the
  // files are named using datetime stamps they will be listed in
  // date/time order.
  ksort($files);

  $options = array();
  $options['none'] = "None";
  foreach ($files as $file) {

    $options[$file->filename] = check_plain($file->filename);
  }

  $form['users']['users'] = array(
    '#type' => 'radios',
    '#title' => t('Please select file to import - NB You might want to switch off account activation email notification under the user settings'),
    '#default_value' => 'none',
    '#options' => $options,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Import dataset files',
  );

  return $form;
}

/**
 * Function to process form to import users.
 */
function data_export_import_import_users_form_submit($form, &$form_state) {



  drupal_set_message('<pre>' . var_export($form_state, TRUE) . '</pre>');


  if ($form_state['values']['users'] != 'none') {



    //    $result = data_export_import_import_users($form_state['clicked_button']['#post']['users']);

    //    drupal_set_message($form_state['clicked_button']['#post']['users']);
    drupal_set_message(var_dump($form_state));


    /*    
    if ($result) {
      drupal_set_message(t('The Users dataset file %dataset_file was imported.', array('%dataset_file' => $form_state['clicked_button']['#post']['users'])));
      drupal_set_message(t('Any required role settings for the users will now have to be set by using the users administration screen.'));
    }
    else {
      drupal_set_message(t('The Users dataset file %dataset_file was not imported.', array('%dataset_file' => $form_state['clicked_button']['#post']['users'])), 'error');
    }
*/

  }
}
