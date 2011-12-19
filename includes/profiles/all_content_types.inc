<?php

/**
 * @file
 * Provides functions to enable nodes to be exported and imported.
 *
 * When importing the content type will need to be checked to make
 * sure that the nodes can be imported with all their data.
 */

/**
 * Add fieldset to the data export form.
 *
 */
function _all_content_types_add_fieldset_to_export_form(&$form) {

  $form['all_content_types'] = array(
                                     '#type' => 'fieldset',
                                     '#title' => 'All content types.',
                                     '#collapsible' => TRUE,
                                     '#collapsed' => FALSE, );

  $node_types = node_get_types();

  foreach ($node_types as $node_type) {

    $node_type_type = $node_type->type;
    $node_type_name = $node_type->name;

    $form['all_content_types'][$node_type_type] = array(
                                                        '#type' => 'checkbox',
                                                        '#title' => $node_type_name, );

  }
}

/**
 * Export the required dataset files.
 *
 * This function will look at which content types have been selected
 * for exporting to file and call a function to export thoese content
 * types.  The $dataset_files_created variable will hold the names of
 * all the dataset files which were created so it can be handed back
 * to the calling code for display to the user.
 *
 * @return
 *   TRUE if all ran OK.
 */
function _all_content_types_export_to_file($form_state, &$dataset_files_created) {

  $debug = FALSE;

  $string_to_output = var_export($form_state, TRUE);
  drupal_set_message("<pre>" . $string_to_output . "</pre>");

  // See if any content types were selected - if none then exist out
  // gracefully.
  if (!isset($form_state['clicked_button']['#post']['all_content_types'])) {
      drupal_set_message("No content types selected.");
      return TRUE;
  } 

  if ($debug) {
    drupal_set_message(var_dump($form_state['clicked_button']['#post']['all_content_types']));
  }

  // We can loop through which content types need to be exported.
  // The #post array will only contain values which have been set to
  // have a value of '1'.
  foreach ($form_state['clicked_button']['#post']['all_content_types'] as $content_type => $value) {

    drupal_set_message("content type: " . $content_type . " - value - " . $value);
    $dataset_files_created[] = data_export_import_export_nodes_for_a_content_type($content_type);
  }

  return TRUE;
  //  $dataset_files_created[] = data_export_import_export_all_content_types();
}

/**
 * Exports detailed node data.
 *
 * This function is going to use the API to extract the relevant
 * data. This data is then going to be output to a data file.
 *
 * @return
 *   The filename of the datafile which was created.
 */
function data_export_import_export_nodes_for_a_content_type($content_type) {

  // HERE HERE HERE Convert to run for the $content_type and not page.

  $debug = FALSE;

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  // $node_type = node_get_types('type', 'page');

  // Here we are getting all content types and selecting just the one
  // we need - this seems to give more consistent results than just
  // extracting the single node object.
  $node_types = node_get_types();
  if ($debug) {
    echo "node_type:\n";
    var_export($node_types['page']);
  }

  // Add the content type details to the array.
  // We are going to serialize the object here as it can then be
  // unserialized on the import and checked.  The normal
  // serialize/unserialize should cover this but the objects would not
  // be counted as matching.  Possibly related to:
  // https://bugs.php.net/bug.php?id=48016
  $dataset['content_type'] = serialize($node_types[$content_type]);

  //$type = 'page';

  // Get the node objects and add them to the array.
  $query = db_rewrite_sql("SELECT nid FROM {node} n WHERE type = '%s'", 'n');

  // $results = db_query_range($query, $type, $offset, $limit);
  $results = db_query($query, $content_type);

  while ($nid = db_result($results)) {
    $node = node_load($nid);
    $dataset['nodes'][$node->nid] = $node;
  }

  if ($debug) {
    echo "Dataset being exported ------------------------------->:\n";
    var_export($dataset);
  }

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  if ($debug) {
    echo "Dataset after being serialized ------------------------------->:\n";
    var_export($dataset_serialized);
  }

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = file_directory_path() . "/data_export_import";
  file_check_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  $dataset_directory = file_directory_path() . "/data_export_import/all_content_types";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_all_content_types_". $content_type .".dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;






  // --------------------------------------------------------------------

  $debug = FALSE;

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  $dataset[] = "a single content type exported";

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = file_directory_path() . "/data_export_import";
  file_check_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  // Create the default directory to hold the datasets.
  $dataset_directory = file_directory_path() . "/data_export_import/all_content_types";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_" . $content_type . ".dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;
}


/**
 * Exports detailed node data.
 *
 * This function is going to use the API to extract the relevant
 * data. This data is then going to be output to a data file.
 *
 * @return
 *   The filename of the datafile which was created.
 */
function data_export_import_export_all_content_types() {

  $debug = FALSE;

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  $dataset[] = "all content types exported data nodes etc";

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = file_directory_path() . "/data_export_import";
  file_check_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  // Create the default directory to hold the datasets.
  $dataset_directory = file_directory_path() . "/data_export_import/all_content_types";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_all_content_types.dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;
}
