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
 * @return
 *   TRUE if all ran OK.
 */
function all_content_types_add_fieldset_to_export_form(&$form) {

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
  
  return TRUE;
}

/**
 * Exports detailed data about the nodes.
 *
 * This function is going to use the API to extract the relevant
 * data. This data is then going to be output to a data file.
 *
 * @return
 *   The filename of the datafile which was created.
 */
function data_export_import_export_nodes() {

  $debug = TRUE;

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  $dataset[] = "qwerty_articles";

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  // Create the default directory to hold the datasets.
  $dataset_directory = file_directory_path() . "/data_export_import/articles";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_articles.dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;
}

