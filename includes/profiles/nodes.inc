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

////  drupal_set_message("<pre>" . var_export($form_state, TRUE) . "</pre>");


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

////  $batch['file'] = drupal_get_path('module', 'data_export_import') . '/includes/profiles/all_content_types.inc';
  $batch['file'] = drupal_get_path('module', 'data_export_import') . '/includes/profiles/nodes.inc';

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
////    $file_name = format_date(REQUEST_TIME, 'custom', 'Ymd_His') . "_all_content_types_" . $content_type . ".dataset";
    $file_name = format_date(REQUEST_TIME, 'custom', 'Ymd_His') . "_nodes_" . $content_type . ".dataset";
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
////  batch_process('admin/content/data_export_import/all_content_types');
  batch_process('admin/content/data_export_import/nodes');

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
////    $context['sandbox']['max'] = db_result(db_query("SELECT COUNT(DISTINCT nid) FROM {node} WHERE type = '%s'", $content_type));
    $context['sandbox']['max'] = db_query("SELECT COUNT(DISTINCT nid) FROM {node} WHERE type = :content_type", array(':content_type' => $content_type))->fetchField();

  }

  // We will set this to make one pass at a time to limit timeouts.
  $limit = 1;

//  $result = db_query_range("SELECT nid FROM {node} WHERE nid > %d AND type = '%s' ORDER BY nid ASC", array($context['sandbox']['current_node'], $content_type), 0, $limit);
  $result = db_query_range("SELECT nid FROM {node} WHERE nid > :current_node AND type = :content_type ORDER BY nid ASC", 0, $limit, array(':current_node' => $context['sandbox']['current_node'], ':content_type' => $content_type));


// Changing two things here - the new D7 syntax returns an object not
// an array.

////  while ($row = db_fetch_array($result)) {
  foreach ($result as $row) {

    // Output one node to the file.
////    $node = node_load($row['nid'], NULL, TRUE);
    $node = node_load($row->nid, NULL, TRUE);

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

// DEBUG
  drupal_set_message("<pre>" . var_export($form_state, TRUE) . "</pre>");

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
/*
function data_export_import_import_nodes($file) {

  return TRUE;
}
*/


/**
 * Import a dataset file and make current nodes match exactly.
 *
 * The purpose of this function is to effectively 'import' the nodes
 * stored in a dataset file.
 *
 * NB - When this import has finished the nodes in the receiving
 * instance should be an exact match with the nodes in the imported
 * dataset file.  Think in terms of rsync with the '--delete'
 * option. This means that as well as importing new nodes we need to
 * delete nodes in the receiving instance which are not in the
 * imported dataset.
 *
 * This synchronization will be carried out by two passes.
 *
 * First we will loop through the nodes in the receiving instance  and
 * check against the the imported dataset. If the node exists in the
 * dataset then it will be updated in the receiving instance.  If it
 * doesn't exist in the dataset then it will be deleted from the
 * receiving Drupal instance.
 *
 * The second pass will be a loop through the dataset - any terms
 * which are in the dataset but are not in the receiving Drupal
 * instance will be created.
 *
 * This will effectively mean that the terms have been sychronised
 * completely.
 *
 * NB - There is some deeper logic here we need to be aware of.  The
 * node ID's need to match exactly otherwise the related items will
 * not match.  So when the new nodes are created they will need to
 * have their old ID's set to match exactly - again due to the related
 * terms.
 *
 * @param string $file
 *   The dataset file which is being imported.
 *
 * @return bool
 *   TRUE if the import process ran without errors.
 */
function data_export_import_import_nodes($file) {

  // Read the first line - decode the object and check that the content types
  // are an exact match.
  $file_path = file_directory_path() . "/data_export_import/nodes/" . $file;

  $handle = fopen($file_path, "r");
  if ($handle) {

    $content_type_line_from_file = fgets($handle);
    fclose($handle);
  }

  $node_content_type_object_from_file = unserialize(base64_decode($content_type_line_from_file));

  // Check that the content types match.
  $node_content_type = $node_content_type_object_from_file->type;

  node_types_rebuild();
  $node_types = node_type_get_types();
////  $node_types = node_get_types();

  // Check that the content type exists in the receiving instance.
  if (is_null($node_types[$node_content_type])) {

    drupal_set_message(t("The content type of the content being imported does not exist in the receiving site for content type of: %content_type", array('%content_type' => $node_content_type)), 'error');

    return FALSE;
  }

  // Here we are going to unset the description fields.  If the
  // description has been changed but everything else about the
  // content type is the same then we will assume that this is just an
  // edit to the description.
  unset($node_content_type_object_from_file->description);
  unset($node_types[$node_content_type]->description);

  if ($node_content_type_object_from_file != $node_types[$node_content_type]) {

    drupal_set_message(t("When the dataset was created the node content type was different from this site's node content type. Please manually compare the content type: %content_type", array('%content_type' => $node_content_type)), 'error');

    return FALSE;
  }

  // Loop through all the lines in the file and put the nid's into an array.
  // Then loop through all the current nodes (in the receiving instance)
  // and check the nid value.  If it doesn't match an nid from the dataset file
  // then delete the existing node in the receiving instance.  This will delete
  // test nodes which have been created.
  $line_number = 0;
  $handle = fopen($file_path, "r");
  if ($handle) {
    while (($buffer = fgets($handle)) !== FALSE) {

      $line_number++;
      if ($line_number > 1) {

        // @todo Here we will need to unencode the line extracted.
        $node_from_file = unserialize(base64_decode($buffer));
        $nids_from_file[] = $node_from_file->nid;
      }
    }

    if (!feof($handle)) {
      echo "Error: unexpected fgets() fail\n";
    }
    fclose($handle);

  }

  // Loop through the existing nodes (for that content type) and if
  // they don't exist in the dataset then delete them.

// Here we are going to be a bit simpler but harsher.  If a node of
// *any* content type extists with the matching ID then delete it.

// And we're going to alow the user running the import to be able to
// delete any nodes so that the data keeps consistent.

////  $query = db_rewrite_sql("SELECT n.nid FROM {node} n WHERE type = '%s'", 'n');
////  $results = db_query($query, $node_content_type);

  $results = db_query("SELECT nid FROM {node}");

////  while ($nid = db_result($results)) {
  foreach ($result as $nid) {

    if (!in_array($nid, $nids_from_file)) {

      // Delete node as it does not exist in the dataset being
      // imported.
      node_delete($nid);
    }
  }

  // Loop through all the lines in the dataset file and extract each
  // node line. If the receiving instance has a node with a matching
  // nid then update it - or if not then create a new node with the
  // same nid. We will use a batch function to process the lines from
  // the file.
  $batch = array(
    'operations' => array(
      array('data_export_import_batch_import_dataset_lines', array($file_path)),
    ),
    'finished' => 'data_export_import_batch_import_dataset_lines_finished',
    'title' => t('Processing Import of nodes from file Batch'),
    'init_message' => t('Batch of node importing starting.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Batch importing of nodes from file has encountered an error.'),
    'file' => drupal_get_path('module', 'data_export_import') . '/includes/profiles/nodes.inc',
  );

  batch_set($batch);

  // Since this is not called from a file then we need to call the
  // batch process function.
  batch_process('admin/content/data_export_import/nodes/import');

  return TRUE;
}

/**
 * Batch function to process lines from a dataset.
 *
 * This function will process one line at a time from a dataset file.
 * To stop the function from processing the first line each time we
 * will store the file pointer and then start the next file read from
 * that point.
 *
 * @param string $file_path
 *   File being processed.
 *
 * @return bool
 *   TRUE if the address is in a valid format, and FALSE if it isn't.
 */
function data_export_import_batch_import_dataset_lines($file_path, &$context) {
  if (!isset($context['sandbox']['progress'])) {

    $context['sandbox']['progress'] = 0;

    // Get the number of lines we will be processing.
    $line_count = 0;
    $handle = fopen($file_path, "r");

    if ($handle) {
      while (($buffer = fgets($handle)) !== FALSE) {

        $line_count++;
      }

      if (!feof($handle)) {
        drupal_set_message(t("Error: unexpected fgets() fail\n"), 'error');
      }
      fclose($handle);
    }
    $context['sandbox']['max'] = $line_count;
  }

  // Open the file for reading.
  $file_handle = fopen($file_path, 'r');

  // Check if file pointer position exists in the sandbox, and jump to
  // location in file.
  if ($context['sandbox']['file_pointer_position']) {
    fseek($file_handle, $context['sandbox']['file_pointer_position']);
  }

  // Get file line from the file.
  $line_from_file = fgets($file_handle);

  // The first line in the dataset file is a the content type data -
  // so we will ignore that line.
  if ($context['sandbox']['progress'] != 0) {

    $dataset_node = unserialize(base64_decode($line_from_file));

    // Find if there is an existing node and see if it matches what is
    // in the dataset. Note that node_load() returns FALSE if it can't
    // find a node.
    $existing_node = node_load($dataset_node->nid);

    // If node_load returns FALSE then it was not able to find a node
    // with the nid - therefore a new node needs to be created.
    // Otherwise the existing node may need to be updated with the
    // data from the dataset.
    if ($existing_node == FALSE) {

      // Saving data for a new node is relatively straight forward.
      // We just call a modified version of the node_save() function.
      // The difficult part is dealing with attached files and files
      // uploaded via CCK fields. First we will loop through the
      // fields looking for CCK fields which contain file data.  We
      // will then save these files. This means that we can then use
      // the new file ID in the node data and when we save the node
      // the link to the CCK file will be working correctly.
      foreach ($dataset_node as $node_field_key => $node_field_value) {

        if (is_array($node_field_value)) {

          foreach ($node_field_value as $file_detail_key => $file_details_value) {

            // This looks for the correct type of value and if there
            // is a setting fid/filename/filepath set then we will
            // assume this is a field of attached CCK files.
            if (is_array($file_details_value) && isset($file_details_value['fid']) && isset($file_details_value['filename']) &&isset($file_details_value['filepath'])) {

              if (isset($file_details_value['cck_file_data'])) {

                $directory = file_create_path(dirname($file_details_value['filepath']));

                if (file_check_directory($directory, TRUE)) {
                  if (file_put_contents($file_details_value['filepath'], base64_decode($file_details_value['cck_file_data']))) {
                    drupal_write_record('files', $file_details_value);

                    // As we've just saved the file it will probably have been
                    // given a new fid.  We are not going to force the file to
                    // be saved with the same ID as in the dataset as there may
                    // be another file which is using the ID value.  We need to
                    // get this new ID value and put that into the node data so
                    // the node links to the correct file.
                    $dataset_node->{$node_field_key}[$file_detail_key]['fid'] = $file_details_value['fid'];
                  }
                }
              }
            }
          }
        }
      }

      // Here we need to to deal with the files attached to a node via
      // the Upload module.  As they will have unusable fid values we
      // will first move the uploaded files to a separate variable,
      // we'll then create the node and then create the file and
      // records necessary for the node to see the file(s) as
      // attached.
      if (module_exists('upload')) {

        $a = 1;
        if (isset($dataset_node->files)) {

          $uploaded_files = $dataset_node->files;
          unset($dataset_node->files);
        }
      }

      // This is the important line where we actually save the node.
      // The node should now have correct fid values to link to the
      // CCK fields which have already been saved - and after saving
      // this node we will be able to use the correct revision vid to
      // link the node to the Upload module files which we will be
      // saving next.
      data_export_import_node_insert_with_defined_nid($dataset_node);

      if (module_exists('upload')) {

        if (!empty($uploaded_files)) {

          foreach ($uploaded_files as $uploaded_file) {

            $directory = file_create_path(dirname($uploaded_file->filepath));

            if (file_check_directory($directory, TRUE)) {
              if (file_put_contents($uploaded_file->filepath, base64_decode($uploaded_file->data_export_import_file_data))) {
                drupal_write_record('files', $uploaded_file);

                // Need to create the {upload} record which will
                // link the node to the data in the {files}
                // table.  We will use the new vid value which has
                // been generated when we saved the node.
                $upload_record = array();
                $upload_record['fid'] = $uploaded_file->fid;
                $upload_record['nid'] = $dataset_node->nid;
                $upload_record['vid'] = $dataset_node->vid;
                $upload_record['description'] = $uploaded_file->filename;
                $upload_record['list'] = $uploaded_file->list;
                $upload_record['weight'] = $uploaded_file->weight;

                drupal_write_record('upload', $upload_record);
              }
            }
          }
        }
      }
    }

    else {

      // If the nodes don't match then update the existing node with
      // the data from the dataset. Certain fields can be unset
      // because they would be expected to be different. We will store
      // the existing dataset node because it will be needed when
      // updating the node.
      $existing_node_vid = $existing_node->vid;

      // The node may have had new revisions created since the dataset
      // was created so we'll unset all the data about revisions.
      unset($dataset_node->vid);
      unset($existing_node->vid);
      unset($dataset_node->log);
      unset($existing_node->log);
      unset($dataset_node->revision_timestamp);
      unset($existing_node->revision_timestamp);

      // The changed date for the existing node could easily be
      // different from the dataset node - even from a previous import
      // of the dataset - or say a user saved the node without any
      // changes.
      unset($dataset_node->changed);
      unset($existing_node->changed);

      if ($dataset_node != $existing_node) {

        // Create a new revision. The dataset node may be from several
        // revisions back - and we don't want to reset back to a
        // previous revision - and we don't want to overwrite the
        // current revision - so creating a new revision is the best
        // option. The only other option is to delete the node (which
        // would delete all revisions) and then re-create it - but
        // this would lose all the revision history.
        $dataset_node->vid = $existing_node_vid;
        $dataset_node->revision = 1;
        $dataset_node->log = "Imported from dataset file: " . $file;

        // Here we will save the files attached to the node via CCK
        // fields and update the fid values stored in the
        // $dataset_node.
        foreach ($dataset_node as $node_field_key => $node_field_value) {

          if (is_array($node_field_value)) {

            foreach ($node_field_value as $file_detail_key => $file_details_value) {

              // This looks for the correct type of value and if there
              // is a setting fid/filename/filepath set then we will
              // assume this is a field of attached CCK files.
              if (is_array($file_details_value) && isset($file_details_value['fid']) && isset($file_details_value['filename']) &&isset($file_details_value['filepath'])) {

                if (isset($file_details_value['cck_file_data'])) {

                  $directory = file_create_path(dirname($file_details_value['filepath']));

                  if (file_check_directory($directory, TRUE)) {
                    if (file_put_contents($file_details_value['filepath'], base64_decode($file_details_value['cck_file_data']))) {
                      drupal_write_record('files', $file_details_value);

                      // As we've just saved the file it will probably
                      // have been given a new fid.  We are not going
                      // to force the file to be saved with the same
                      // ID as in the dataset as there may be another
                      // file which is using the ID value.  We need to
                      // get this new ID value and put that into the
                      // node data so the node links to the correct
                      // file.
                      $dataset_node->{$node_field_key}[$file_detail_key]['fid'] = $file_details_value['fid'];
                    }
                  }
                }
              }
            }
          }
        }

        // Here we need to to deal with the files attached to a node
        // via the Upload module.  As they will have unusable fid
        // values we will first move the uploaded files to a separate
        // variable, we'll then create the node and then create the
        // file and records necessary for the ndoe to see the file(s)
        // as attached.
        if (module_exists('upload')) {

          $a = 1;
          if (isset($dataset_node->files)) {

            $uploaded_files = $dataset_node->files;
            unset($dataset_node->files);
          }
        }

        node_save($dataset_node);

        if (module_exists('upload')) {

          if (!empty($uploaded_files)) {

            foreach ($uploaded_files as $uploaded_file) {

              $directory = file_create_path(dirname($uploaded_file->filepath));

              if (file_check_directory($directory, TRUE)) {

                if (file_put_contents($uploaded_file->filepath, base64_decode($uploaded_file->data_export_import_file_data))) {
                  drupal_write_record('files', $uploaded_file);

                  // Need to create the {upload} record which will
                  // link the node to the data in the {files}
                  // table.  We will use the new vid value which has
                  // been generated when we saved the node.
                  $upload_record = array();
                  $upload_record['fid'] = $uploaded_file->fid;
                  $upload_record['nid'] = $dataset_node->nid;
                  $upload_record['vid'] = $dataset_node->vid;
                  $upload_record['description'] = $uploaded_file->filename;
                  $upload_record['list'] = $uploaded_file->list;
                  $upload_record['weight'] = $uploaded_file->weight;

                  drupal_write_record('upload', $upload_record);
                }
              }
            }
          }
        }
      }
    }
  }

  $context['sandbox']['progress']++;

  $context['results'][$file_path] = basename($file_path);

  // Retain current file pointer position.
  $context['sandbox']['file_pointer_position'] = ftell($file_handle);

  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Batch 'finished' callback
 */
function data_export_import_batch_import_dataset_lines_finished($success, $results, $operations) {
  if ($success) {

    // Here we do something meaningful with the results.
    $message = "The following file was imported:";
    $message .= theme('item_list', $results);
  }
  else {

    // An error occurred. NB that $operations contains the operations
    // that remained unprocessed.
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
 * Implements saving a node with a defined nid value.
 *
 * This function is basically an override of the standard node_save()
 * function.
 */
function data_export_import_node_insert_with_defined_nid(&$node) {

  // Here we will remove the nid from the object to trick the rest of
  // the function into thinking it is dealing with a normal insert.
  // We will then re-introduce the nid when needed.
  $defined_nid = $node->nid;
  unset($node->nid);

  // Let modules modify the node before it is saved to the database.

////  node_invoke_nodeapi($node, 'presave');
module_invoke_all('node_' . 'presave', $node);

  global $user;

  // Insert a new node.
  $node->is_new = empty($node->nid);

  if ($node->is_new || !empty($node->revision)) {
    // When inserting a node, $node->log must be set because
    // {node_revisions}.log does not (and cannot) have a default
    // value.  If the user does not have permission to create
    // revisions, however, the form will not contain an element for
    // log so $node->log will be unset at this point.
    if (!isset($node->log)) {
      $node->log = '';
    }
  }

  // For the same reasons, make sure we have $node->teaser and
  // $node->body set.
  if (!isset($node->teaser)) {
    $node->teaser = '';
  }
  if (!isset($node->body)) {
    $node->body = '';
  }

////  $time = time();
  $time = REQUEST_TIME;


  if (empty($node->created)) {
    $node->created = $time;
  }

  // The changed timestamp is always updated for bookkeeping purposes
  // (revisions, searching, ...)
  $node->changed = $time;

  $node->timestamp = $time;
  $node->format = isset($node->format) ? $node->format : FILTER_FORMAT_DEFAULT;

  // Generate the node table query and the node_revisions table query.
  if ($node->is_new) {
    _node_save_revision($node, $user->uid);

    // Call our modified function after putting the nid back into the
    // object.
    $node->nid = $defined_nid;


//// HERE HERE HERE.
/// Hopefully we should be able to replace this function call with the
/// Drupal db_merge function.

    data_export_import_drupal_write_record_via_insert_with_defined_id('node', $node);

////    db_query('UPDATE {node_revisions} SET nid = %d WHERE vid = %d', $node->nid, $node->vid);
    db_query('UPDATE {node_revision} SET nid = %d WHERE vid = %d', $node->nid, $node->vid);
    $op = 'insert';
  }

  // Call the node specific callback (if any).
  node_invoke($node, $op);

////  node_invoke_nodeapi($node, $op);
  module_invoke_all('node_' . $op, $node);

  // Update the node access table for this node.
  node_access_acquire_grants($node);

  // Clear the page and block caches.
  cache_clear_all();
}

/**
 * Implements inserting a record with a defined id value.
 *
 * Overrides the standard drupal_write_record() function.
 */
function data_export_import_drupal_write_record_via_insert_with_defined_id($table, &$object, $update = array()) {

  // Standardize $update to an array.
  if (is_string($update)) {
    $update = array($update);
  }

  $schema = drupal_get_schema($table);
  if (empty($schema)) {
    return FALSE;
  }

  // Convert to an object if needed.
  if (is_array($object)) {
    $object = (object) $object;
    $array = TRUE;
  }
  else {
    $array = FALSE;
  }

  $fields = $defs = $values = $serials = $placeholders = array();

  // Go through our schema, build SQL, and when inserting, fill in
  // defaults for fields that are not set.
  foreach ($schema['fields'] as $field => $info) {

    // For inserts, populate defaults from Schema if not already
    // provided.
    if (!isset($object->$field) && !count($update) && isset($info['default'])) {
      $object->$field = $info['default'];
    }

    // Build arrays for the fields, placeholders, and values in our
    // query.
    if (isset($object->$field)) {
      $fields[] = $field;
      $placeholders[] = db_type_placeholder($info['type']);

      if (empty($info['serialize'])) {
        $values[] = $object->$field;
      }
      else {
        $values[] = serialize($object->$field);
      }
    }
  }

  // Build the SQL.
  $query = '';
  if (!count($update)) {
    $query = "INSERT INTO {" . $table . "} (" . implode(', ', $fields) . ') VALUES (' . implode(', ', $placeholders) . ')';
    $return = SAVED_NEW;
  }

  // Execute the SQL.
  if (!db_query($query, $values)) {
    $return = FALSE;
  }

  // If we began with an array, convert back so we don't surprise the
  // caller.
  if ($array) {
    $object = (array) $object;
  }

  return $return;
}