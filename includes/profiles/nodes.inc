<?php
/**
 * @file
 * Enables nodes to be exported and imported.
 */

/**
 * Callback function to export nodes.
 */
function data_export_import_callback_export_nodes() {

  return drupal_get_form('data_export_import_export_nodes_form');
}

/**
 * Function to create form to export nodes.
 */
function data_export_import_export_nodes_form($form_state) {

  $form['export_nodes'] = array(
    '#type' => 'fieldset',
    '#title' => t('Export nodes'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );

  $form['export_nodes']['description'] = array(
    '#type' => 'item', 
    '#title' => t('Export all nodes to a dataset file'),
  );

  node_types_rebuild();
  $node_types = node_type_get_types();

  foreach ($node_types as $node_type) {

    $node_type_type = $node_type->type;
    $node_type_name = $node_type->name;

    //    $form['export_nodes'][$node_type_type] = array(
    $form['export_nodes']['content_types'][$node_type_type] = array(
      '#type' => 'checkbox',
      '#title' => check_plain($node_type_name),);
  }

  // Adds a simple submit button that refreshes the form and clears its
  // contents. This is the default behavior for forms.
  $form['export_nodes']['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Create dataset file',
  );

  return $form;
}

/**
 * Function to process form to export nodes.
 */
function data_export_import_export_nodes_form_submit($form, &$form_state) {

  // This is the function which will carry out the exporting of the
  // nodes to a dataset file.

  drupal_set_message("<pre>" . var_export($form_state, TRUE) . "</pre>");


  $dataset_file_created = data_export_import_export_nodes_to_file($form_state);

  drupal_set_message(t("The following dataset file was created:"));
  drupal_set_message(check_plain($dataset_file_created));

  return TRUE;
}

/**
 * Export the required dataset file.
 */
/*
function data_export_import_export_nodes_to_file() {

  return "DUMMY export.";
}
*/

/**
 * Export the required dataset files.
 *
 * This function will look at which content types have been selected
 * for exporting to file and call a function to export thoese content
 * types.  The $dataset_files_created variable will hold the names of
 * all the dataset files which were created so it can be handed back
 * to the calling code for display to the user.
 *
 * @param array $form_state
 *   Current values held in the form.
 *
 * @return bool
 *   TRUE if all ran OK.
 */
function data_export_import_export_nodes_to_file($form_state) {

  // See if any content types were selected - if none then exit out
  // gracefully.

  //NB - HERE WE NEED TO CHECK AT LEAST ON VALUE OF ['values']['content_types'] has a value of '1'.
  /*
  if (!isset($form_state['clicked_button']['#post']['all_content_types'])) {
    drupal_set_message(t("No content types selected."));
    return TRUE;
  }
  */

  /*
  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = file_directory_path() . "/data_export_import";
  file_check_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  $dataset_directory = file_directory_path() . "/data_export_import/all_content_types";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);
  */

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = variable_get('file_public_path', conf_path() . '/files') . "/data_export_import";
  file_prepare_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  $dataset_directory = variable_get('file_public_path', conf_path() . '/files') . "/data_export_import/nodes/";
  file_prepare_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);




  // Adding in the main values to the $batch variable.
  $batch = array();
  $batch['finished'] = 'data_export_import_batch_export_nodes_finished';
  $batch['title'] = t('Exporting nodes');
  $batch['init_message'] = t('The exportation of nodes starting.');
  $batch['progress_message'] = t('Processed @current out of @total.');
  $batch['error_message'] = t('Exporting nodes has encountered an error.');
  $batch['file'] = drupal_get_path('module', 'data_export_import') . '/includes/profiles/all_content_types.inc';

  // We can loop through which content types need to be exported.
  // The #post array will only contain values which have been set to
  // have a value of '1'. We will build up the $batch object with the
  // required operations.

  //  foreach ($form_state['clicked_button']['#post']['all_content_types'] as $content_type => $value) {
//  foreach ($form_state['values']['content_types'] as $content_type => $value) {
  foreach ($form_state['values']['export_nodes']['content_types'] as $content_type => $value) {

    // NB - HERE HERE HERE - TEST THIS WORKS OK.
    if ($value == 0) {
      continue;
    }

    // Get the filename we are going to save the data to.
    $file_name = format_date(REQUEST_TIME, 'custom', 'Ymd_His') . "_all_content_types_" . $content_type . ".dataset";
    $file_path_and_name = $dataset_directory . "/" . $file_name;

    // Here we are getting all content types and selecting just the one
    // we need - this seems to give more consistent results than just
    // extracting the single node object.

    // $node_types = node_get_types();
    node_types_rebuild();
    $node_types = node_type_get_types();

    // Save the content type variable to the file. By serializing the
    // variable we will change it to a character based format which is
    // safe to be output to a file.  This is made safer then by being
    // base64 encoded to make sure line endings and other characters
    // do not cause issues on importing the dataset.
    $content_type_data_serialized = serialize($node_types[$content_type]);
    $content_type_data_serialized_and_encoded = base64_encode($content_type_data_serialized);
    file_unmanaged_save_data($content_type_data_serialized_and_encoded . "\n", $file_path_and_name, FILE_EXISTS_REPLACE);

    // Each content type being exported to a dataset file will be run
    // as a batch to prevent timeouts.
    $batch['operations'][] = array(
      'data_export_import_batch_export_nodes_to_file',
      array(
        $content_type,
        $file_path_and_name,
      ),
    );
  }

  // This is the key function which will set the batch up to be processed.
  batch_set($batch);

  // Since we are not calling this batch processing from a form we will need to
  // request that the batch is processed.
  batch_process('admin/content/data_export_import/all_content_types');

  return TRUE;
}


/**
 * Batch function called to export the content type.
 *
 * @param string $content_type
 *   The dataset file which is being imported.
 *
 * @param string $file_path_and_name
 *   The path and filename to use for the dataset file created.
 *
 * @return bool
 *   TRUE if all ran OK.
 */
function data_export_import_batch_export_nodes_to_file($content_type, $file_path_and_name, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = db_result(db_query("SELECT COUNT(DISTINCT nid) FROM {node} WHERE type = '%s'", $content_type));
  }

  // We will set this to make one pass at a time to limit timeouts.
  $limit = 1;

  $result = db_query_range("SELECT nid FROM {node} WHERE nid > %d AND type = '%s' ORDER BY nid ASC", array($context['sandbox']['current_node'], $content_type), 0, $limit);

  while ($row = db_fetch_array($result)) {

    // Output one node to the file.
    $node = node_load($row['nid'], NULL, TRUE);

    // Here we will deal with any files attached to the node if the
    // upload module is being used.  What we are going to do is to
    // extract the file, convert it to a character encoding and then
    // attach it to the files array on the node.
    if (module_exists('upload')) {

      if (!empty($node->files) && is_array($node->files)) {

        // Loop through the files.
        foreach ($node->files as $attached_file_key => $attached_file) {

          // Get the file into a safe character based format and then
          // attach it to the array.
          $export_data = base64_encode(file_get_contents($attached_file->filepath));
          $node->files[$attached_file_key]->data_export_import_file_data = $export_data;
        }
      }
    }

    // Here we are looking for all the file data fields which are
    // attached using CCK.
    foreach ($node as $node_field_key => $node_field_value) {

      if (is_array($node_field_value)) {

        foreach ($node_field_value as $file_detail_key => $file_details_value) {

          // This looks for the correct type of value and if there is
          // a setting for fid, filename, filepath set then we will
          // assume this is a field of file(s) attached via CCK.
          if (is_array($file_details_value) && isset($file_details_value['fid']) && isset($file_details_value['filename']) &&isset($file_details_value['filepath'])) {

            // Get the file data in a serialized format and attach this as data
            // to the correct place on the $node object.
            $file_data = base64_encode(file_get_contents($file_details_value['filepath']));

            $node->{$node_field_key}[$file_detail_key]['cck_file_data'] = $file_data;
          }
        }
      }
    }

    // Here we will serialize the array to convert it to a string
    // which can then be output to a file.
    $node_serialized = serialize($node);

    // Encode the string to make sure the data does not contain line
    // endings and other characters which may cause problems when
    // reading the file during import.
    $node_serialized_and_base64_encoded = base64_encode($node_serialized);

    file_put_contents($file_path_and_name, $node_serialized_and_base64_encoded . "\n", FILE_APPEND | LOCK_EX);

    // Update our progress information.
    $context['sandbox']['progress']++;
    $context['sandbox']['current_node'] = $node->nid;
    $context['message'] = t('Now processing %node', array('%node' => $node->title));
  }

  // Store some result for post-processing in the finished callback.
  // We will use a trick by setting the key to be the file path so
  // this array value will be set again and again.  This way the results
  // array will not get larger and larger for each record which is processed
  // but will hold a single result for this batch.
  $context['results'][$file_path_and_name] = basename($file_path_and_name);

  // Inform the batch engine that we are not finished, and provide an
  // estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Batch 'finished' callback
 */
function data_export_import_batch_export_nodes_finished($success, $results, $operations) {
  if ($success) {

    // Here we do something meaningful with the results.
    $message = "The following dataset files were created:";
    $message .= theme('item_list', $results);
  }
  else {

    // An error occurred. $operations contains the operations that
    // remained unprocessed.
    $error_operation = reset($operations);
    $message = t(
      'An error occurred while processing %error_operation with arguments: @arguments',
      array(
        '%error_operation' => $error_operation[0],
        '@arguments' => print_r($error_operation[1], TRUE),
      )
    );
  }

  drupal_set_message($message);
}

















/**
 * Callback function to import nodes.
 */
function data_export_import_callback_import_nodes() {

  return drupal_get_form('data_export_import_import_nodes_form');
}

/**
 * Function to create form to import nodes.
 */
function data_export_import_import_nodes_form($form_state) {

  $form = array();

  $form['import_nodes'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import nodes'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  // Get the contents of the dataset directory and create a list of
  // links to dataset files.
  $directory = variable_get('file_public_path', conf_path() . '/files') . "/data_export_import/nodes";

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

  $form['import_nodes']['dataset_file'] = array(
    '#type' => 'radios',
    '#title' => t('Select file to import.'),
    '#default_value' => 'none',
    '#options' => $options,
  );

  $form['import_nodes']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Import dataset files',
  );

  return $form;
}

/**
 * Function to process form to import nodes.
 */
function data_export_import_import_nodes_form_submit($form, &$form_state) {

  if ($form_state['values']['dataset_file'] != 'none') {

    $result = data_export_import_import_nodes($form_state['values']['dataset_file']);

    if ($result) {
      drupal_set_message(t('The Nodes dataset file %dataset_file was imported.', array('%dataset_file' => $form_state['values']['dataset_file'])));
    }
    else {
      drupal_set_message(t('The Nodes dataset file %dataset_file was not imported.', array('%dataset_file' => $form_state['values']['dataset_file'])), 'error');
    }
  }
}

/**
 * Make nodes list match the nodes from a dataset file.
 *
 * @param string $file
 *   The dataset file which is being imported.
 *
 * @return bool
 *   TRUE if the import process ran without errors.
 */
function data_export_import_import_nodes($file) {

  return TRUE;
}
